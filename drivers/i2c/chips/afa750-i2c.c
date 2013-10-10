/*
 * AFA750 Three-Axis Digital Accelerometers (I2C Interface)
 *
 *
 * Copyright (C) 2009 Frances Chu, Afa Micro Corp.
 * Licensed under the GPL-2 or later.
 */

#include <linux/input.h>	/* BUS_I2C */
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/pm.h>
#include "afa750.h"
#include <mach/items.h>

static int afa750_smbus_read(struct device *dev, unsigned char reg)
{
	struct i2c_client *client = to_i2c_client(dev);

	return i2c_smbus_read_byte_data(client, reg);
}

static int afa750_smbus_write(struct device *dev,
			       unsigned char reg, unsigned char val)
{
	struct i2c_client *client = to_i2c_client(dev);

	return i2c_smbus_write_byte_data(client, reg, val);
}

static int afa750_smbus_read_block(struct device *dev,
				    unsigned char reg, int count,
				    void *buf)
{
        int ret;
	struct i2c_client *client = to_i2c_client(dev);

	//return i2c_smbus_read_i2c_block_data(client, reg, count, buf);
	ret = i2c_smbus_read_i2c_block_data(client, reg, count, buf);
        if(ret < 0){
            printk("FrancesLog****: smbus read block fialed %d\n", ret);
            while(1);
        }
        return ret;       
}

static int afa750_i2c_read_block(struct device *dev,
				  unsigned char reg, int count,
				  void *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret;

	ret = i2c_master_send(client, &reg, 1);
	if (ret < 0){
            printk("FrancesLog****: i2c read block send fialed %d\n", ret);
            while(1);
		return ret;
        }

	ret = i2c_master_recv(client, buf, count);
	if (ret < 0){
            printk("FrancesLog****: i2c read block recv fialed %d\n", ret);
            while(1);
		return ret;
        }

	if (ret != count){
            printk("FrancesLog****: i2c read block ret != count fialed \n");
            while(1);
		return -EIO;
        }

	return 0;
}

static const struct afa750_bus_ops afa750_smbus_bops = {
	.bustype	= BUS_I2C,
	.write		= afa750_smbus_write,
	.read		= afa750_smbus_read,
	.read_block	= afa750_smbus_read_block,
};

static const struct afa750_bus_ops afa750_i2c_bops = {
	.bustype	= BUS_I2C,
	.write		= afa750_smbus_write,
	.read		= afa750_smbus_read,
	.read_block	= afa750_i2c_read_block,
};

static int __devinit afa750_i2c_probe(struct i2c_client *client,
				       const struct i2c_device_id *id)
{
	struct afa750 *ac;
	int error;

	error = i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA);
	if (!error) {
		dev_err(&client->dev, "SMBUS Byte Data not Supported\n");
		return -EIO;
	}

	ac = afa750_probe(&client->dev, client->irq, false,
			   i2c_check_functionality(client->adapter,
						   I2C_FUNC_SMBUS_READ_I2C_BLOCK) ?
				&afa750_smbus_bops : &afa750_i2c_bops);
	if (IS_ERR(ac))
		return PTR_ERR(ac);

	i2c_set_clientdata(client, ac);

	return 0;
}

static int __devexit afa750_i2c_remove(struct i2c_client *client)
{
	struct afa750 *ac = i2c_get_clientdata(client);

	return afa750_remove(ac);
}

#ifdef CONFIG_PM
static int afa750_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct afa750 *ac = i2c_get_clientdata(client);

	afa750_suspend(ac);

	return 0;
}

static int afa750_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct afa750 *ac = i2c_get_clientdata(client);

	afa750_resume(ac);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(afa750_i2c_pm, afa750_i2c_suspend,
			 afa750_i2c_resume);

static const struct i2c_device_id afa750_id[] = {
	{ "afa750", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, afa750_id);

static struct i2c_driver afa750_driver = {
	.driver = {
		.name = "afa750",
		.owner = THIS_MODULE,
		.pm = &afa750_i2c_pm,
	},
	.probe    = afa750_i2c_probe,
	.remove   = __devexit_p(afa750_i2c_remove),
	.id_table = afa750_id,
};

static int __init afa750_i2c_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	printk("*****************%s run !***********\n", __func__);
	if(item_exist("sensor.grivaty") && item_equal("sensor.grivaty.model", "afa750", 0)){
	 	memset(&info, 0 ,sizeof(struct i2c_board_info));
		info.addr = 0x3d;
		strlcpy(info.type, "afa750", I2C_NAME_SIZE);
		
		adapter = i2c_get_adapter(item_integer("sensor.grivaty.ctrl", 1));
		if(!adapter){
			printk("***********get_adapter error!\n");
		}
		client = i2c_new_device(adapter, &info);	
		return i2c_add_driver(&afa750_driver);

	}else{
		printk("%s, G-sensor is not afa750 or exist\n", __func__);
		return -1;
	}
	
}
late_initcall(afa750_i2c_init);

static void __exit afa750_i2c_exit(void)
{
	i2c_del_driver(&afa750_driver);
}
module_exit(afa750_i2c_exit);

MODULE_AUTHOR("Frances Chu <franceschu@afamicro.com>");
MODULE_DESCRIPTION("AFA750 Three-Axis Digital Accelerometer I2C Bus Driver");
MODULE_LICENSE("GPL");
