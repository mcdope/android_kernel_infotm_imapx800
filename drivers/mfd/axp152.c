#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/axp152.h>
#include <linux/mfd/axp152_imapx800.h>
#include <mach/items.h>
#include <mach/pad.h>

//#define AXP152_TEST

static struct mfd_cell axp152s[] = {
	{
		.name = "axp152-pmic",
	},
};

static int axp152_i2c_read(struct axp152 *axp152, u8 reg,
				int bytes, void *dest)
{
	struct i2c_client *i2c = axp152->i2c_client;
	struct i2c_msg xfer[2];
	int ret;

	/* Write register */
	xfer[0].addr = i2c->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	/* Read data */
	xfer[1].addr = i2c->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = bytes;
	xfer[1].buf = dest;

	ret = i2c_transfer(i2c->adapter, xfer, 2);
	if(ret == 2)
		ret = 0;
	else if(ret < 0)
		ret = -EIO;

	return ret;
}

static int axp152_i2c_write(struct axp152 *axp152, u8 reg,
				int bytes, void *src)
{
	struct i2c_client *i2c = axp152->i2c_client;
	/* we add 1 byte for device register */
	u8 msg[AXP152_MAX_REGISTER + 1];
	int ret;

	if(bytes > AXP152_MAX_REGISTER)
		return -EINVAL;

	msg[0] = reg;
	memcpy(&msg[1], src, bytes);

	ret = i2c_master_send(i2c, msg, bytes + 1);
	if(ret < 0)
		return ret;
	if(ret != bytes + 1)
		return -EIO;

	return 0;
}

#ifdef AXP152_TEST

static u8 axp_vreg[] = {
	0x01, 0x12, 0x13, 0x15, 0x23, 0x25,
	0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x31, 0x32, 0x36, 0x37, 0x80, 0x81,
	0x8a, 0x8f, 0x90, 0x91, 0x92, 0x93,
	0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
	0x9c, 0x9d, 0x40, 0x41, 0x42, 0x48,
	0x49, 0x4a,
};

#endif

static u8 axp_reg_init[][2] = {
	//{AXP152_DCDC4, 0x68},//3.3V
	//{AXP152_ALDO, 0xf9}, //aldo1 3.3V, aldo2 2.5V
	//{AXP152_DLDO1, 0x15},//2.8V
	//{AXP152_DLDO2, 0x0b},//1.8V
	//{AXP152_LDO0, 0x23}, //2.8V, 500mA limit

    {AXP152_VOFF, 0x00},
	{AXP152_IRQ_EN1, 0x00},//all irqs disabled
	{AXP152_IRQ_EN2, 0x00},
	{AXP152_IRQ_EN3, 0x00},
	{AXP152_IRQ_ST1, 0xff},//clear all irq status
	{AXP152_IRQ_ST2, 0xff},
	{AXP152_IRQ_ST3, 0xff},
};

const u8 axp_reg_init_num = ARRAY_SIZE(axp_reg_init);

int axp152_set_bits(struct axp152 *axp152, u8 reg, u8 mask)
{
	u8 data;
	int err;

	mutex_lock(&axp152->io_mutex);
	err = axp152_i2c_read(axp152, reg, 1, &data);
	if(err) {
		dev_err(axp152->dev, "read from reg %x failed\n", reg);
		goto out;
	}

	data |= mask;
	err = axp152_i2c_write(axp152, reg, 1, &data);
	if(err)
		dev_err(axp152->dev, "write to reg %x failed\n", reg);

out:
	mutex_unlock(&axp152->io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(axp152_set_bits);

int axp152_clear_bits(struct axp152 *axp152, u8 reg, u8 mask)
{
	u8 data;
	int err;

	mutex_lock(&axp152->io_mutex);
	err = axp152_i2c_read(axp152, reg, 1, &data);
	if(err) {
		dev_err(axp152->dev, "read from reg %x failed\n", reg);
		goto out;
	}

	data &= ~mask;
	err = axp152_i2c_write(axp152, reg, 1, &data);
	if(err)
		dev_err(axp152->dev, "write to reg %x failed\n", reg);

out:
	mutex_unlock(&axp152->io_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(axp152_clear_bits);

struct axp_channel {
	const char *name;
	int found;
	uint8_t reg;
	uint8_t val;
};

static struct axp_channel channel_info[] = {
	{
		.name = "dcdc4",
		.reg = AXP152_DCDC4,
		.val = 0x68,
	},
	{
		.name = "aldo1",
		.reg = AXP152_ALDO,
		.val = 0xf9,
	},
	{
		.name = "dldo1",
		.reg = AXP152_DLDO1,
		.val = 0x15,
	},
	{
		.name = "dldo2",
		.reg = AXP152_DLDO2,
		.val = 0x0b,
	},
};

static int ch_num = ARRAY_SIZE(channel_info);

static const char *dev_item[7] = {
	"ids.loc.dev0.lvds.power",
	"ids.loc.dev0.lcd.power",
	"camera.front.power_iovdd",
	"camera.front.power_dvdd",
	"camera.rear.power_iovdd",
	"camera.rear.power_dvdd",
    "codec.power",
};

static int check_reg_num(const char *buf)
{
	int i;
	int ret = -1;

	for(i=0; i<ch_num; i++) {
		if(!strcmp(buf, channel_info[i].name)) {
			ret = i;
			break;
		}
	}

	return ret;
}

static int check_en_bit(int num)
{
	int ret;

	switch(num) {
	case 0:
		ret = 4;
        break;
	case 1:
		ret = 3;
        break;
	case 2:
		ret = 1;
        break;
	case 3:
		ret = 0;
        break;
	}

	return ret;
}

static int axp152_default_output(struct axp152 *axp152)
{
	int i;
	int num;
	int bit;
	int ret = 0;
	char buf[ITEM_MAX_LEN];
    //uint8_t dat;

	for(i=0; i<ch_num; i++) {
		channel_info[i].found = 0;
	}

	for(i=0;i<7;i++) {
		if(item_exist(dev_item[i])) {
			if(i == 1) {
				num = item_integer(dev_item[i], 1);
				imapx_pad_set_mode(1, 1, num);
				imapx_pad_set_dir(0, 1, num);
				imapx_pad_set_outdat(1, 1, num);
			}else {
				item_string(buf, dev_item[i], 1);
				num = check_reg_num(buf);
				if(num != -1) {
					if(channel_info[i].found == 0) {
						ret |= axp152_i2c_write(axp152, channel_info[num].reg,
													0x1, &channel_info[num].val);
                        //ret |= axp152_i2c_read(axp152, channel_info[num].reg, 
                        //                            0x1, &dat);
						bit = check_en_bit(num);
						ret |= axp152_set_bits(axp152, AXP152_POWER_CTL, (1 << bit));
						channel_info[i].found = 1;
                        //printk("num=%d, reg=0x%02x, dat=0x%02x, en_bit=%d\n", num, channel_info[num].reg, dat, bit);
					}else
						continue;
				}else
					continue;
			}
		}
	}

	return ret;
}


static int axp152_i2c_probe(struct i2c_client *i2c,
				const struct i2c_device_id *id)
{
	struct axp152 *axp152;
	int ret = 0;
	int i;
	int dat;

	axp152 = kzalloc(sizeof(struct axp152), GFP_KERNEL);
	if(axp152 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, axp152);
	axp152->dev = &i2c->dev;
	axp152->i2c_client = i2c;
	axp152->id = id->driver_data;
	axp152->read = axp152_i2c_read;
	axp152->write = axp152_i2c_write;
	mutex_init(&axp152->io_mutex);

#ifdef AXP152_TEST
	printk("for testing pmic, read all register\n");
	for(i = 0; i < 38; i++) {
		ret  = axp152_i2c_read(axp152, axp_vreg[i], 0x01, &dat);
		if(ret == 0)
			printk("axp152 read reg 0x%x =0x%x OK\n", axp_vreg[i], dat);
		else
			printk("axp152 read reg 0x%x =0x%x  Err\n", axp_vreg[i], dat);
	}
#endif

	for(i = 0; i < axp_reg_init_num; i++) {
		ret = axp152_i2c_write(axp152, axp_reg_init[i][0], 0x01, &axp_reg_init[i][1]);
		if(ret < 0) {
			printk("axp152 register init fail, regaddr = 0x%x\n", axp_reg_init[i][0]);
			goto err2;
		}
		ret = axp152_i2c_read(axp152, axp_reg_init[i][0], 0x01, &dat);
		//printk("i=%d, axp152 read result of register init, 0x%x = 0x%x\n", i, axp_reg_init[i][0], dat); 
	}

	if(!axp152_default_output(axp152))
		printk("axp152_default_output OK!\n");
	else
		printk("axp152_default_output ERROR!\n");

	ret = mfd_add_devices(axp152->dev, -1,
				axp152s, ARRAY_SIZE(axp152s), NULL, 0);

	if(ret < 0)
		goto err;

	return ret;

err:
	mfd_remove_devices(axp152->dev);
err2:
	kfree(axp152);
	printk("%s err\n", __func__);
	return ret;
}

static int axp152_i2c_remove(struct i2c_client *i2c)
{
	struct axp152 *axp152 = i2c_get_clientdata(i2c);
	u8 reg_dat;
	int ret;

	ret = axp152_i2c_read(axp152, AXP152_POWER_DOWN, 1, &reg_dat);
	if(ret < 0)
		return ret;

	reg_dat |= 1 << 7;
	axp152_i2c_write(axp152, AXP152_POWER_DOWN, 1, &reg_dat);

	mfd_remove_devices(axp152->dev);
	kfree(axp152);

	return 0;
}

static int axp152_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return 0;
}

static int axp152_i2c_resume(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id axp152_i2c_id[] = {
	{ "axp152", AXP152 },
	{}
};
MODULE_DEVICE_TABLE(i2c, axp152_i2c_id);

static struct i2c_driver axp152_i2c_driver = {
	.driver = {
		.name = "axp152",
		.owner = THIS_MODULE,
	},
	.probe = axp152_i2c_probe,
	.remove = axp152_i2c_remove,
	.suspend = axp152_i2c_suspend,
	.resume = axp152_i2c_resume,
	.id_table = axp152_i2c_id,
};

static int __init axp152_i2c_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	if(item_exist("pmu.model")) {
		if(item_equal("pmu.model", "axp152", 0)) {
			memset(&info, 0, sizeof(struct i2c_board_info));
			info.addr = 0x30;
			strlcpy(info.type, "axp152", I2C_NAME_SIZE);
			info.flags = I2C_CLIENT_WAKE;
			info.platform_data = &axp152_platform_data;

			adapter = i2c_get_adapter(item_integer("pmu.ctrl", 1));
			if(!adapter)
				printk(KERN_ERR "******i2c_get_adapter error!******\n");

			client = i2c_new_device(adapter, &info);

			return i2c_add_driver(&axp152_i2c_driver);
		}else
			printk(KERN_ERR "%s: pmu model is not axp152\n", __func__);
	}else
		printk(KERN_ERR "%s: pmu model is not exist\n", __func__);

	return -1;
}
module_init(axp152_i2c_init);

static void __exit axp152_i2c_exit(void)
{
	i2c_del_driver(&axp152_i2c_driver);
}
module_exit(axp152_i2c_exit);

MODULE_AUTHOR("zhanglei <lei_zhang@infotm.com>");
MODULE_DESCRIPTION("AXP152 chip multi-function driver");
MODULE_LICENSE("GPL");

