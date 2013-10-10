/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5316.h"
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>

#include <mach/pad.h>       
#include <mach/power-gate.h>
#include <mach/imap-iomap.h>
#include <mach/items.h>     

static struct i2c_client *this_client;
static unsigned int _sui_irq_num;
static int tsp_irq_index;
static int SCREEN_MAX_X=1024;
static int SCREEN_MAX_Y=600;

extern int get_lcd_width(void);
extern int get_lcd_height(void);

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ft5x0x_early_suspend(struct early_suspend *handler);
static void ft5x0x_late_resume(struct early_suspend *handler);
#endif

//#define FTS_CTL_IIC
//#define FTS_APK_DEBUG
//#define SYSFS_DEBUG
#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif
#ifdef SYSFS_DEBUG
#include "ft5x06_ex_fun.h"
#endif
struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x0x_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	//struct ft5x0x_platform_data *pdata;
#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
	struct early_suspend early_suspend;
#endif

    struct work_struct      pen_event_work;
    struct workqueue_struct *ts_workqueue;
};

#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02


/*
*ft5x0x_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}

/*release the point*/
static void ft5x0x_ts_release(struct ft5x0x_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x0x_read_Touchdata(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}

	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void ft5x0x_report_value(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;
	int up_point = 0;
	//int touch_point = 0;

	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */
		if (event->au16_x[i] < data->x_max
		    && event->au16_y[i] < data->y_max) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 event->pressure);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
					 event->au8_finger_id[i]);
			if (event->au8_touch_event[i] == FTS_POINT_DOWN
			    || event->au8_touch_event[i] == FTS_POINT_CONTACT)\
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
			else {
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
				up_point++;
			}
			//touch_point ++;
		}

		input_mt_sync(data->input_dev);

        //printk("report key-%d: x==%d, y==%d\n", i, event->au16_x[i], event->au16_y[i]);
	}
	input_sync(data->input_dev);

	if (event->touch_point == 0)
		ft5x0x_ts_release(data);

}

static void ft5x0x_work_func(struct work_struct *work)
{
    struct ft5x0x_ts_data *ft5x0x_ts = i2c_get_clientdata(this_client);
    int ret;

    ret = ft5x0x_read_Touchdata(ft5x0x_ts);
    if(ret == 0)
        ft5x0x_report_value(ft5x0x_ts);
}

static int cmd_write(u8 cmd, u8 para0, u8 para1, u8 para2, u8 num)
{
    u8 write_cmd[4] = {0};
    int ret;

    write_cmd[0] = cmd;
    write_cmd[1] = para0;
    write_cmd[2] = para1;
    write_cmd[3] = para2;

    ret = i2c_master_send(this_client, write_cmd, num);
    if(ret <= 0){
        printk("[TSP] write cmd error\n");
        return 0;
    }

    return 1;
}

static void fts_reset(void)
{
    int index = item_integer("ts.reset", 1);

    imapx_pad_set_mode(1, 1, index);/*gpio*/
    imapx_pad_set_dir(0, 1, index);/*output*/
    imapx_pad_set_outdat(1, 1, index);/*output 1*/
    msleep(10);
    imapx_pad_set_outdat(0, 1, index);/*output 0*/
    msleep(5);
    imapx_pad_set_outdat(1, 1, index);/*output 1*/
    msleep(10);

    cmd_write(0x07,0x00,0x00,0x00,1);
    msleep(200);
}

/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;

    if(imapx_pad_irq_pending(tsp_irq_index)){
        imapx_pad_irq_clear(tsp_irq_index);

        if(!work_pending(&ft5x0x_ts->pen_event_work)){
            queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
        }
    }

        return IRQ_HANDLED;

//	disable_irq_nosync(ft5x0x_ts->irq);
/*
	ret = ft5x0x_read_Touchdata(ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(ft5x0x_ts);
*/
//	enable_irq(ft5x0x_ts->irq);

//	return IRQ_HANDLED;
}

static int ft5x0x_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	//struct ft5x0x_platform_data *pdata =
	//    (struct ft5x0x_platform_data *)client->dev.platform_data;
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;

    tsp_irq_index = item_integer("ts.int", 1);
    _sui_irq_num = imapx_pad_irq_number(tsp_irq_index);
    if(!_sui_irq_num)
        return -EINVAL;

    imapx_pad_set_mode(1, 1, tsp_irq_index);/* gpio mode */
    imapx_pad_set_dir(1, 1, tsp_irq_index);/* input */
    imapx_pad_irq_config(tsp_irq_index, INTTYPE_FALLING, FILTER_MAX);/* set trigger mode and filter */

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);

	if (!ft5x0x_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	
	i2c_set_clientdata(client, ft5x0x_ts);
	ft5x0x_ts->irq = _sui_irq_num;
	ft5x0x_ts->client = client;
	//ft5x0x_ts->pdata = pdata;
	ft5x0x_ts->x_max = SCREEN_MAX_X - 1;
	ft5x0x_ts->y_max = SCREEN_MAX_Y - 1;
#ifdef CONFIG_PM
	//err = gpio_request(pdata->reset, "ft5x0x reset");
	//if (err < 0) {
	//	dev_err(&client->dev, "%s:failed to set gpio reset.\n",
	//		__func__);
	//	goto exit_request_reset;
	//}
#endif

    this_client = client;

    INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_work_func);
    ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
    if(!ft5x0x_ts->ts_workqueue){
        err = -ESRCH;
        goto exit_create_singlethread;
    }

	//err = request_threaded_irq(client->irq, NULL, ft5x0x_ts_interrupt,
	//			   pdata->irqflags, client->dev.driver->name,
	//			   ft5x0x_ts);

    err = request_irq(_sui_irq_num, ft5x0x_ts_interrupt, IRQF_SHARED, "qt602240_ts", ft5x0x_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	disable_irq(client->irq);

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_PRESSURE, input_dev->absbit);

	input_set_abs_params(input_dev, 
			     ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
	input_set_abs_params(input_dev, 
			     ABS_MT_POSITION_Y, 0, ft5x0x_ts->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);

	input_dev->name = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft5x0x_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
	/*make sure CTP already finish startup process */
	msleep(150);

	/*get some register information */
	uc_reg_addr = FT5x0x_REG_FW_VER;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_info(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT5x0x_REG_POINT_RATE;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_info(&client->dev, "[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);

	uc_reg_addr = FT5X0X_REG_THGROUP;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_info(&client->dev, "[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);
#ifdef SYSFS_DEBUG
	ft5x0x_create_sysfs(client);
#endif
#ifdef FTS_APK_DEBUG
	ft5x0x_create_apk_debug_channel(client);
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif
	enable_irq(client->irq);

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
    ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
    ft5x0x_ts->early_suspend.suspend = ft5x0x_early_suspend;
    ft5x0x_ts->early_suspend.resume = ft5x0x_late_resume;
    register_early_suspend(&ft5x0x_ts->early_suspend);
#endif

	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
	free_irq(client->irq, ft5x0x_ts);
//#ifdef CONFIG_PM
//exit_request_reset:
//	gpio_free(ft5x0x_ts->pdata->reset);
//#endif

exit_create_singlethread:
exit_irq_request_failed:
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ft5x0x_early_suspend(struct early_suspend *handler)
#else
static void ft5x0x_ts_suspend(struct early_suspend *handler)
#endif
{
	struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
						early_suspend);

	dev_info(&ts->client->dev, "[FTS]ft5x0x suspend\n");
	disable_irq(_sui_irq_num);
}

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ft5x0x_late_resume(struct early_suspend *handler)
#else
static void ft5x0x_ts_resume(struct early_suspend *handler)
#endif
{
	struct ft5x0x_ts_data *ts = container_of(handler, struct ft5x0x_ts_data,
						early_suspend);

	dev_info(&ts->client->dev, "[FTS]ft5x0x resume.\n");
	//gpio_set_value(ts->pdata->reset, 0);
	//msleep(20);
	//gpio_set_value(ts->pdata->reset, 1);
	//enable_irq(ts->pdata->irq);

    imapx_pad_set_mode(1, 1, tsp_irq_index);
    imapx_pad_irq_config(tsp_irq_index, INTTYPE_FALLING, FILTER_MAX);
    fts_reset();
    enable_irq(_sui_irq_num);
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	ft5x0x_ts = i2c_get_clientdata(client);
	input_unregister_device(ft5x0x_ts->input_dev);
#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
    unregister_early_suspend(&ft5x0x_ts->early_suspend);
#endif
	#ifdef CONFIG_PM
	//gpio_free(ft5x0x_ts->pdata->reset);
	#endif
	#ifdef FTS_APK_DEBUG
	ft5x0x_release_apk_debug_channel();
	#endif
	#ifdef SYSFS_DEBUG
	ft5x0x_release_sysfs(client);
	#endif
	#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
	#endif
	free_irq(client->irq, ft5x0x_ts);
	kfree(ft5x0x_ts);
    cancel_work_sync(&ft5x0x_ts->pen_event_work);
    destroy_workqueue(ft5x0x_ts->ts_workqueue);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{FT5X0X_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe = ft5x0x_ts_probe,
	.remove = __devexit_p(ft5x0x_ts_remove),
	.id_table = ft5x0x_ts_id,
#ifndef CONFIG_IDSDRV_EARLY_SUSPEND
	.suspend = ft5x0x_ts_suspend,
	.resume = ft5x0x_ts_resume,
#endif
	.driver = {
		   .name = FT5X0X_NAME,
		   .owner = THIS_MODULE,
		   },
};

static int __init ft5x0x_ts_init(void)
{
//	int ret;
//	ret = i2c_add_driver(&ft5x0x_ts_driver);
//	if (ret) {
//		printk(KERN_WARNING "Adding ft5x0x driver failed "
//		       "(errno = %d)\n", ret);
//	} else {
//		pr_info("Successfully added driver %s\n",
//			ft5x0x_ts_driver.driver.name);
//	}
//	return ret;

    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;

    printk("Enter FT 5316 %s\n", __func__);

    if(item_exist("ts.model")){
        if(item_equal("ts.model", "ft5316", 0)){
            SCREEN_MAX_X = get_lcd_width();
            SCREEN_MAX_Y = get_lcd_height();

            memset(&info, 0, sizeof(struct i2c_board_info));
            info.addr = 0x38;
            strlcpy(info.type, "ft5x0x_ts", I2C_NAME_SIZE);

            adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
            if(!adapter)
                printk("****** get_adapter error! ******\n");

            client = i2c_new_device(adapter, &info);

            return i2c_add_driver(&ft5x0x_ts_driver);
        }else
            printk("%s: touchscreen is not ft5316\n", __func__);
    }else
        printk("%s: touchscreen is not exist\n", __func__);

    return -1;
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
