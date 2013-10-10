/***************************************************************************** 
 * ** drivers/video/infotm_HDMI/imap_HDMI.c
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Implementation file of Infotm HDMI.
 * **
 * ** Author:
 * **     Alex Zhang <alex.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.0  06/11/2010 Alex Zhang 
 * ** 1.1  06/18/2010 Alex Zhang 
 * ** 2.0  06/21/2010 Alex Zhang 
 * ** 2.1  06/23/2010 Alex Zhang 
 * ** 2.2  06/24/2010 Alex Zhang 
 * *****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/wakelock.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/gpio.h>
#include <plat/imapx.h>
#include "../infotm/imapfb.h"
#include "hdmi.h"
#include "imapx200_ids.h"

#define HDMI_MAJOR               84
#define HDMI_POLL_MAJOR               85
#define HDMI_MINOR               0
#define i2c_device_address  0x70

#ifdef CONFIG_HDMI_EP932
extern struct hdmi_ops ep932m_ops;
#endif

#ifdef CONFIG_HDMI_SI9022
extern struct hdmi_ops sii9022_ops;
#endif

#define turn_on_backlight() 	\
({				\
	imapx_gpio_setcfg(__imapx_name_to_gpio(CONFIG_IG_LCD_33), IG_OUTPUT, IG_NORMAL); \
	imapx_gpio_setpin(__imapx_name_to_gpio(CONFIG_IG_LCD_33), 1, IG_NORMAL); \
 	msleep(10);	\
 	imapx_gpio_setcfg(__imapx_name_to_gpio(CONFIG_IG_LCD_BACKLIGHT), IG_CTRL0, IG_NORMAL); \
})

#define turn_off_backlight()    \
({                              \
	imapx_gpio_setcfg(__imapx_name_to_gpio(CONFIG_IG_LCD_BACKLIGHT), IG_OUTPUT, IG_NORMAL); \
	imapx_gpio_setpin(__imapx_name_to_gpio(CONFIG_IG_LCD_BACKLIGHT), 0, IG_NORMAL); \
 	msleep(10);	\
	imapx_gpio_setcfg(__imapx_name_to_gpio(CONFIG_IG_LCD_33), IG_OUTPUT, IG_NORMAL); \
	imapx_gpio_setpin(__imapx_name_to_gpio(CONFIG_IG_LCD_33), 0, IG_NORMAL); \
})

#ifdef CONFIG_SND_SOC_ALC5621
extern void externel_rt5621_AudioOutEnable(int mute);
#else
extern void imap_iokey_spken_hdmi(int);
#endif

struct hdmi_ops *hdmi_con;

static struct class *HDMI_class;

//I2C_CLIENT_INSMOD;

wait_queue_head_t       HDMI_wait;
spinlock_t              lock;
struct completion	Monitor_Wait;
struct wake_lock hdmi_wake_lock;

int	HDMI_MENU_SWITCH = -1; 
int	HDMI_HOTPLUG_IN = -1;
int    	HDMI_HOTPLUG_RESULT = 0;
int 	HDMI_QUERY_MONITOR_FLAG = -1;
int	HDMI_POLL_FLAG = 0;
/*now support 3 modes*/
unsigned int VIDEO_MODE = 0;
EXPORT_SYMBOL(VIDEO_MODE);
unsigned int DOUBLE_DISPLAY_MODE = 0;
EXPORT_SYMBOL(DOUBLE_DISPLAY_MODE);
unsigned int PANAUISION_MODE = 0;
EXPORT_SYMBOL(PANAUISION_MODE);

static const struct i2c_device_id HDMI_i2c_id[] = {
	{ "imap_HDMI", 0 },
	{ }
};

static struct i2c_client gHDMIClient;
static struct mutex HDMI_lock;

struct imap_HDMI_info {
	struct device *dev;
	struct i2c_client *client;
	struct delayed_work    delayed_work;
};

int HDMI_dev_init(struct i2c_client *client)
{
	return 0;
}

unsigned int hdmi_count = 0;

static void hp_delayed_work(struct work_struct *work)
{
	uint32_t ret;

	struct imap_HDMI_info *HDMI_info =
		        container_of(to_delayed_work(work), struct imap_HDMI_info, delayed_work);
	
	detect_overflow();

	ret = hdmi_con->get_connection_status();

	hdmi_debug("ret %d HDMI_QUERY_MONITOR_FLAG %d \
			VIDEO_MODE %d DOUBLE_DISPLAY_MODE %d PANAUISION_MODE %d HDMI_HOTPLUG_RESULT %d \
			hdmi_count %d\n",ret, HDMI_QUERY_MONITOR_FLAG, VIDEO_MODE,\
		       				DOUBLE_DISPLAY_MODE, PANAUISION_MODE, HDMI_HOTPLUG_RESULT, hdmi_count);

#if defined(CONFIG_DOUBLE_DISPLAY_MODE) || defined(CONFIG_PANAUISION_MODE)
	if((ret == 1) && \
		(HDMI_QUERY_MONITOR_FLAG == 1) && \
//		(VIDEO_MODE == 0 || DOUBLE_DISPLAY_MODE == 0 || PANAUISION_MODE == 0) && 
	       	HDMI_HOTPLUG_RESULT == 0 && \
		hdmi_count >5)
#else
	if((ret == 1) && \
		(HDMI_QUERY_MONITOR_FLAG == 1) && \
	       	HDMI_HOTPLUG_RESULT == 0 && \
		hdmi_count >5)
#endif
	{
		HDMI_QUERY_MONITOR_FLAG = -1;
		HDMI_HOTPLUG_IN = 1;
		HDMI_HOTPLUG_RESULT = 1;
		hdmi_count = 0;
		HDMI_POLL_FLAG = 1;
		wake_up(&HDMI_wait);
//		complete(&Monitor_Wait);
	}
#if defined(CONFIG_DOUBLE_DISPLAY_MODE) || defined(CONFIG_PANAUISION_MODE)
	else if((ret == 0) && \
		(HDMI_QUERY_MONITOR_FLAG == 1) && \
//		(VIDEO_MODE == 1 || DOUBLE_DISPLAY_MODE == 1 || PANAUISION_MODE == 1) && 
		HDMI_HOTPLUG_RESULT == 1 &&\
		hdmi_count >5)
#else
	else if((ret == 0) && \
		(HDMI_QUERY_MONITOR_FLAG == 1) && \
		HDMI_HOTPLUG_RESULT == 1 &&\
		hdmi_count >5)
#endif
	{
		HDMI_QUERY_MONITOR_FLAG = -1;
		HDMI_HOTPLUG_IN= 0;
		HDMI_HOTPLUG_RESULT = 0;
		hdmi_count = 0;
		HDMI_POLL_FLAG = 1;
		wake_up(&HDMI_wait);
//		complete(&Monitor_Wait);
	}

	hdmi_count ++ ;

	schedule_delayed_work(&HDMI_info->delayed_work,
			msecs_to_jiffies(HDMI_HP_POLL_DELAY));
}



static int HDMI_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct device *dev = NULL;

	printk(KERN_INFO "Device for HDMI will be Initializad\n");

	/* register this i2c device with the driver core */
	dev = device_create(HDMI_class, NULL, MKDEV(HDMI_MAJOR, HDMI_MINOR), NULL, "HDMI");
	if (IS_ERR(dev))
	{
		ret = PTR_ERR(dev);
		ret = -EFAULT;
		goto exit;
	}

	dev = device_create(HDMI_class, NULL, MKDEV(HDMI_POLL_MAJOR, HDMI_MINOR), NULL, "HDMI_poll");
	if (IS_ERR(dev))
	{
		ret = PTR_ERR(dev);
		ret = -EFAULT;
		goto exit;
	}

	memcpy(&gHDMIClient, client, sizeof(struct i2c_client));

	mutex_init(&HDMI_lock);

	/* HDMI device initialization */
	if(unlikely(hdmi_con->chip_init())){
		printk(KERN_ERR "HDMI_i2c_probe: failed to initialise chip\n");
		ret = -EFAULT;
		goto exit;
	}

	printk(KERN_INFO "Init HDMI device OK\n");

	return 0;

exit:
	return ret;
}

static int HDMI_i2c_remove(struct i2c_client *client)
{
	printk(KERN_INFO "Remove HDMI device driver\n");

	mutex_destroy(&HDMI_lock);

	return 0;
}

static unsigned int 
HDMI_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	HDMI_QUERY_MONITOR_FLAG = 1;

	poll_wait(file, &HDMI_wait, wait);

	if(HDMI_POLL_FLAG == 1)
	{
		mask |= POLLIN | POLLRDNORM;
		HDMI_POLL_FLAG = 0;
	}

	return mask;
}

static int HDMI_open(struct inode *inode, struct file *file)
{
	file->private_data = &gHDMIClient;
	return 0;
}

static int HDMI_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static ssize_t
HDMI_poll_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	HDMI_QUERY_MONITOR_FLAG = 1;
	wait_for_completion(&Monitor_Wait);
	hdmi_debug("Monitor wait completion\n");

	return 0;
}

static ssize_t
HDMI_poll_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned int HDMI_HOTPLUG_TYPE = 0;
	int ret = 0;

	if(HDMI_HOTPLUG_IN == -1)
	{
		if(HDMI_MENU_SWITCH == -1)
			HDMI_HOTPLUG_TYPE = 1;	/*No hotplug	No Menu button pressed*/
		else if(HDMI_MENU_SWITCH == 0)
			HDMI_HOTPLUG_TYPE = 2;	/*No hotplug	Menu Button Pressed to LCD*/
		else if(HDMI_MENU_SWITCH == 1)
			HDMI_HOTPLUG_TYPE = 3;	/*No hotplug	Menu Button Pressed to TV*/
	}
	else if(HDMI_HOTPLUG_IN == 0)
	{
		if(HDMI_MENU_SWITCH == -1)
			HDMI_HOTPLUG_TYPE = 4;	/*hotplug out detected No menu button pressed*/
		else if (HDMI_MENU_SWITCH == 0)
			HDMI_HOTPLUG_TYPE = 5 ;	/*hotplug out detected menu button pressed to LCD*/
		else if (HDMI_MENU_SWITCH == 1)
			HDMI_HOTPLUG_TYPE = 6 ;	/*hotplug out detected menu button pressed to TV*/
	}
	else if(HDMI_HOTPLUG_IN == 1)
	{
		if(HDMI_MENU_SWITCH == -1)
			HDMI_HOTPLUG_TYPE = 7;	/*hotplug in detected No menu button pressed*/
		else if (HDMI_MENU_SWITCH == 0)
			HDMI_HOTPLUG_TYPE = 8 ;	/*hotplug in detected menu button pressed to LCD*/
		else if (HDMI_MENU_SWITCH == 1)
			HDMI_HOTPLUG_TYPE = 9 ;	/*hotplug in detected menu button pressed to TV*/
	}

	if(copy_to_user((void __user *)buf, &HDMI_HOTPLUG_TYPE, sizeof(unsigned int)))
	{
		printk(KERN_ERR "[HDMI_ioctl]: copy to user space error\n");
		ret = -EFAULT;
	}
	HDMI_HOTPLUG_IN = -1;
	HDMI_MENU_SWITCH = -1;

	return ret;
}

static long HDMI_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int mode = 0;
	void __user *argp = (void __user *)arg;
	int HDMI_TIMING_TYPE = 0 ;
	uint32_t control = 0;

	mutex_lock(&HDMI_lock);

	switch(cmd)
	{
		case HDMI_CHECK_MODE:
			mode = 1;
#ifdef CONFIG_DOUBLE_DISPLAY_MODE
			mode |= 1<<1;
#endif
#ifdef CONFIG_PANAUISION_MODE
			mode |= 1<<2;
#endif
			hdmi_debug("%s commond :HDMI_CHECK mode %d\n",__func__, mode);
			if(copy_to_user((void __user *)argp, &mode, sizeof(unsigned int)))
			{
				printk(KERN_ERR "[HDMI_ioctl]: copy to user space error\n");
				ret = -EFAULT;
			}
			break;

		case HDMI_CHECK_CONNECT:
			hdmi_debug("%s commond :HDMI_CHECK_CONNECT\n",__func__);
			if(copy_to_user((void __user *)argp, &HDMI_HOTPLUG_RESULT, sizeof(unsigned int)))
			{
				printk(KERN_ERR "[HDMI_ioctl]: copy to user space error\n");
				ret = -EFAULT;
			}
			break;

		case HDMI_SET_NOTMAL_TIMING:  //return
			if(VIDEO_MODE == 1 || DOUBLE_DISPLAY_MODE == 1 || PANAUISION_MODE == 1)
			{
				hdmi_debug("%s commond : HDMI_SET_NOTMAL_TIMING\n",__func__);
#ifdef CONFIG_UI_MAP_COLOR
				if(VIDEO_MODE == 1)
				{
					__raw_writel(~(0x1<<24), IMAP_OVCW0CMR);
					__raw_writel(~(0x1<<24), IMAP_OVCW3CMR);
				}
#endif
				if(VIDEO_MODE == 0)
				{
					turn_off_backlight();
					hdmi_con->mute_output(0, 1);
				}
				lcd_change_timing(LCD, 1, 0);
				if(VIDEO_MODE == 0)
				{
					msleep(1000);
					turn_on_backlight();
				}

				DOUBLE_DISPLAY_MODE = 0;
				PANAUISION_MODE = 0;
				if(VIDEO_MODE == 0)
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(0);
#else
				imap_iokey_spken_hdmi(1);
#endif
				VIDEO_MODE = 0;
				wake_unlock(&hdmi_wake_lock);
			}
			break;
		case HDMI_SET_VIDEO_TIMING:  //enter hdmi mode
			if(copy_from_user(&HDMI_TIMING_TYPE, argp, sizeof(unsigned int)))
			{
				printk(KERN_ERR "[HDMI_ioctl]: copy from user space error\n");
				ret = -EFAULT;
			}

			if(VIDEO_MODE == 0)
			{
				hdmi_debug("%s commond : HDMI_SET_VIDEO_TIMING %d\n",__func__,HDMI_TIMING_TYPE);
				wake_lock(&hdmi_wake_lock);
//				imap_iokey_spken_hdmi(0);
//				turn_off_backlight();
//				hdmi_con->mute_output(0, 1);
#ifdef CONFIG_UI_MAP_COLOR
				__raw_writel(0x1<<24 | ((0x0 & 0xffffff)<<0), IMAP_OVCW0CMR);
				__raw_writel(0x1<<24 | ((0x0 & 0xffffff)<<0), IMAP_OVCW3CMR);
#endif
				lcd_change_timing(HDMI_TIMING_TYPE - HDMI_640_480, 1, 0);
				osd_excursion_bottom(HDMI_TIMING_TYPE - HDMI_640_480);
				msleep(200);
				hdmi_con->mute_output(HDMI_TIMING_TYPE - HDMI_640_480, 0);
				PANAUISION_MODE = 0;
				DOUBLE_DISPLAY_MODE = 0;
				VIDEO_MODE = 1;
			}
			break;
		case HDMI_SET_DOUBLE_DISPLAY_TIMING:
#ifdef CONFIG_DOUBLE_DISPLAY_MODE
#ifdef CONFIG_UI_MAP_COLOR
			if(VIDEO_MODE == 1)
			{
				__raw_writel(~(0x1<<24), IMAP_OVCW0CMR);
				__raw_writel(~(0x1<<24), IMAP_OVCW3CMR);
			}
#endif
			if(DOUBLE_DISPLAY_MODE == 0)
			{
				hdmi_debug("%s commond :HDMI_SET_DOUBLE_DISPLAY_TIMING\n",__func__);
				wake_lock(&hdmi_wake_lock);
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(1);
#else
				imap_iokey_spken_hdmi(0);
#endif
				turn_off_backlight();
				hdmi_con->mute_output(0, 1);
				if(imapfb_fimd.xres == 800){
					lcd_change_timing(HDMI_800_600, 1, 1);
					msleep(3000);
					hdmi_con->mute_output(HDMI_800_600, 0);
				}else if(imapfb_fimd.xres == 1024){
					lcd_change_timing(HDMI_1024_768, 1, 1);
					msleep(3000);		// modify - Eric.Lu 
					hdmi_con->mute_output(HDMI_1024_768, 0);
				}
				msleep(500);
				turn_on_backlight();
				DOUBLE_DISPLAY_MODE = 1;
				PANAUISION_MODE = 0;
				VIDEO_MODE = 0;
			}
#else
			hdmi_debug("double display mode not supported\n");
			if(VIDEO_MODE == 1|| PANAUISION_MODE == 1 || DOUBLE_DISPLAY_MODE == 1)
			{
				hdmi_con->mute_output(0, 1);
				lcd_change_timing(LCD, 1, 0);
				turn_on_backlight();
				VIDEO_MODE = 0;
				DOUBLE_DISPLAY_MODE = 0;
				PANAUISION_MODE = 0;
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(0);
#else
				imap_iokey_spken_hdmi(1);
#endif
			}
			ret = -1;
#endif
			break;
		case HDMI_SET_PANAUISION_TIMING:
#ifdef CONFIG_PANAUISION_MODE
#ifdef CONFIG_UI_MAP_COLOR
			if(VIDEO_MODE == 1)
			{
				__raw_writel(~(0x1<<24), IMAP_OVCW0CMR);
				__raw_writel(~(0x1<<24), IMAP_OVCW3CMR);
			}
#endif
			if(PANAUISION_MODE == 0)
			{
				hdmi_debug("%s commond :HDMI_PANAUISION_TIMING\n",__func__);
				wake_lock(&hdmi_wake_lock);
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(1);
#else
				imap_iokey_spken_hdmi(0);
#endif
				turn_off_backlight();
				hdmi_con->mute_output(0, 1);
				if(imapfb_fimd.xres == 800){
					lcd_change_timing(HDMI_720P, 1, 1);
					osd_excursion(HDMI_720P);
					//lcd_change_timing(HDMI_1024_768, 1, 1);
					//osd_excursion(HDMI_1024_768);
					msleep(3000);
					hdmi_con->mute_output(HDMI_720P, 0);
					//hdmi_con->mute_output(HDMI_1024_768, 0);
				}else if(imapfb_fimd.xres == 1024){
					lcd_change_timing(HDMI_720P, 1, 1);
					osd_excursion(HDMI_720P);
					msleep(3000);		// modify - Eric.Lu 
					hdmi_con->mute_output(HDMI_720P, 0);
				}

				PANAUISION_MODE = 1;
				DOUBLE_DISPLAY_MODE = 0;
				VIDEO_MODE = 0;
			}
#else
			hdmi_debug("panauision mode not supported\n");
			if(VIDEO_MODE == 1|| PANAUISION_MODE == 1 || DOUBLE_DISPLAY_MODE == 1)
			{
				hdmi_con->mute_output(0, 1);
				lcd_change_timing(LCD, 1, 0);
				turn_on_backlight();
				VIDEO_MODE = 0;
				DOUBLE_DISPLAY_MODE = 0;
				PANAUISION_MODE = 0;
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(0);
#else
				imap_iokey_spken_hdmi(1);
#endif
			}
			ret = -1;
#endif
			break;
		case HDMI_TV_SUPPORT_V_MODE:                                                    
			break;                                                                  

		case HDMI_DISCONNECT:
			hdmi_debug("%s commond :HDMI_DISCONNECT\n",__func__);
			hdmi_con->mute_output(0, 1);
			break;
			
		case HDMI_BACKLIGHT_CONTROL:
			hdmi_debug("%s commond :HDMI_BACKLIGHT_CONTROL\n",__func__);
			if(copy_from_user(&control, argp, sizeof(unsigned int)))
			{
				printk(KERN_ERR "[HDMI_ioctl]: copy from user space error\n");
				ret = -EFAULT;
			}

			if(control == 0)
				turn_off_backlight();
			else if(control == 1)
			{
				msleep(100);
				turn_on_backlight();
			}

			break;

		case HDMI_SPEAKER_CONTROL:
			hdmi_debug("%s commond :HDMI_SPEAKER_CONTROL\n",__func__);
			if(copy_from_user(&control, argp, sizeof(unsigned int)))
			{
				printk(KERN_ERR "[HDMI_ioctl]: copy from user space error\n");
				ret = -EFAULT;
			}

			msleep(100);
			if(control == 0)
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(1);
#else
				imap_iokey_spken_hdmi(0);
#endif
			else if(control == 1)
#ifdef CONFIG_SND_SOC_ALC5621
				externel_rt5621_AudioOutEnable(0);
#else
				imap_iokey_spken_hdmi(1);
#endif
			break;

		default:
			printk(KERN_ERR "[HDMI_ioctl]: unknown command type\n");
			ret = -EFAULT;
			break;
	}
	mutex_unlock(&HDMI_lock);
	return ret;
}

#if defined(CONFIG_PM)
int HDMI_i2c_suspend(struct i2c_client *client, pm_message_t state)
{
//	struct imap_HDMI_info *HDMI_info = (struct imap_HDMI_info *)i2c_get_clientdata(client);

	printk(KERN_INFO "iic Suspend hdmi\n");


	if(HDMI_HOTPLUG_RESULT == 1 && (( VIDEO_MODE == 1) || (DOUBLE_DISPLAY_MODE == 1) || (PANAUISION_MODE == 1)))
	{
		lcd_change_timing(LCD, 1, 0);
	}

	if(hdmi_con->power)
		hdmi_con->power(0);

	return 0;
}
		
int HDMI_i2c_resume(struct i2c_client *client)
{
//	unsigned int ret = 0;
	struct imap_HDMI_info *HDMI_info = (struct imap_HDMI_info *)i2c_get_clientdata(client);

	printk(KERN_INFO "iic Resume hdmi\n");
	
	if(hdmi_con->power)
		hdmi_con->power(1);

	hdmi_con->chip_init();

	schedule_delayed_work(&HDMI_info->delayed_work,
			msecs_to_jiffies(HDMI_HP_POLL_DELAY));
#if 0
	ret = hdmi_con->get_connection_status();

	if(ret ==1)
	{
		if(VIDEO_MODE == 1)
		{
			turn_off_backlight();
			lcd_change_timing(HDMI_720P, 1);
			osd_excursion_bottom(HDMI_720P);
			msleep(100);		// modify - Eric.Lu 
			hdmi_con->mute_output(HDMI_720P, 0);
			VIDEO_MODE = 1;
			DOUBLE_DISPLAY_MODE = 0;
			PANAUISION_MODE = 0;
		}else if(DOUBLE_DISPLAY_MODE == 1){
			turn_off_backlight();
			if(imapfb_fimd.xres == 800){
				lcd_change_timing(HDMI_800_600, 1);
				msleep(100);		// modify - Eric.Lu 
				hdmi_con->mute_output(HDMI_800_600, 0);
			}else if(imapfb_fimd.xres == 1024){
				lcd_change_timing(HDMI_1024_768, 1);
				msleep(100);		// modify - Eric.Lu 
				hdmi_con->mute_output(HDMI_1024_768, 0);
			}
			turn_on_backlight();
			VIDEO_MODE = 0;
			DOUBLE_DISPLAY_MODE = 1;
			PANAUISION_MODE = 0;
		}else if(PANAUISION_MODE == 1){
			turn_off_backlight();
			if(imapfb_fimd.xres == 800){
				lcd_change_timing(HDMI_1024_768, 1);
				osd_excursion(HDMI_1024_768);
				msleep(100);		// modify - Eric.Lu 
				hdmi_con->mute_output(HDMI_1024_768, 0);
			}else if(imapfb_fimd.xres == 1024){
				lcd_change_timing(HDMI_720P, 1);
				osd_excursion(HDMI_720P);
				msleep(100);		// modify - Eric.Lu 
				hdmi_con->mute_output(HDMI_720P, 0);
			}
			VIDEO_MODE = 0;
			DOUBLE_DISPLAY_MODE = 0;
			PANAUISION_MODE = 1;
		}
	}
#endif

	return 0;
}		
#endif

static const struct file_operations HDMI_fops = {
	.owner                  = THIS_MODULE,
	.unlocked_ioctl = HDMI_ioctl,
	.open                   = HDMI_open,
	.poll		= HDMI_poll,
	.release                        = HDMI_release,
};

static const struct file_operations HDMI_poll_fops = {
	.owner                  = THIS_MODULE,
	.read		= HDMI_poll_read,
	.write		= HDMI_poll_write,
};

static struct i2c_driver HDMI_i2c_driver = {
	.driver = {
		.name = "imap_HDMI-i2c",
		.owner = THIS_MODULE,
	},
	.probe = HDMI_i2c_probe,
	.remove = HDMI_i2c_remove,
#if defined(CONFIG_PM) 
	.suspend = HDMI_i2c_suspend,
	.resume = HDMI_i2c_resume,
#endif
	.id_table = HDMI_i2c_id,
};

static int __init HDMI_probe(struct platform_device *pdev)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct imap_HDMI_info *HDMI_info;
	int ret;

	printk(KERN_INFO "HDMI Probe to add i2c driver for HDMI device\n");

#ifdef CONFIG_HDMI_EP932
	hdmi_con = &ep932m_ops;
#endif
#ifdef CONFIG_HDMI_SI9022
	hdmi_con = &sii9022_ops;
#endif
	if(hdmi_con->power)
		hdmi_con->power(1);

	HDMI_info =(struct imap_HDMI_info *) kzalloc(sizeof(struct imap_HDMI_info), GFP_KERNEL);
	if(!HDMI_info)
	{
		printk(KERN_ERR "Cannot allocate for HDMI_info\n");
		return -ENOMEM;
	}

	wake_lock_init(&hdmi_wake_lock, WAKE_LOCK_SUSPEND, "hdmi");

	ret = register_chrdev(HDMI_MAJOR, "imap-HDMI", &HDMI_fops);
	if (ret)
		goto out;

	ret = register_chrdev(HDMI_POLL_MAJOR, "imap-HDMI_poll", &HDMI_poll_fops);
	if (ret)
		goto out;

	HDMI_class = class_create(THIS_MODULE, "HDMI_dev");
	if (IS_ERR(HDMI_class))
	{
		printk(KERN_ERR "HDMI_init: fail to create HDMI device class\n");
		ret = PTR_ERR(HDMI_class);
		goto out_unreg_chrdev;
	}

	/* Add i2c_driver */
	ret = i2c_add_driver(&HDMI_i2c_driver);
	if(ret)
	{
		printk(KERN_ERR "HDMI_init: fail to register i2c driver\n");
		goto out_unreg_class;
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = i2c_device_address;
	strlcpy(info.type, "imap_HDMI", I2C_NAME_SIZE);

#ifdef	CONFIG_HDMI_SI9022_I2C
	adapter = i2c_get_adapter(CONFIG_HDMI_SI9022_I2C + 1);
#endif
#ifdef   CONFIG_HDMI_EP932
	adapter = i2c_get_adapter(CONFIG_HDMI_EP932_I2C + 1);
#endif
	if (!adapter)
	{
		printk(KERN_ERR "HDMI_init: can't get i2c adapter\n");
		goto err_adapter;
	}

	HDMI_info->client = i2c_new_device(adapter, &info);
	i2c_put_adapter(adapter);
	if (!(HDMI_info->client))
	{
		printk(KERN_ERR "HDMI_init: can't add i2c device at 0x%x\n", (unsigned int)info.addr);
		goto err_adapter;
	}

	spin_lock_init(&lock);
//	init_completion(&Monitor_Wait); 
	init_waitqueue_head(&HDMI_wait);
	HDMI_info->dev = &(pdev->dev);

	INIT_DELAYED_WORK(&HDMI_info->delayed_work, hp_delayed_work);

	schedule_delayed_work(&HDMI_info->delayed_work,
	                      msecs_to_jiffies(HDMI_HP_POLL_DELAY));


	platform_set_drvdata(pdev, HDMI_info);
	i2c_set_clientdata(HDMI_info->client, HDMI_info);

	printk(KERN_INFO "HDMI device add i2c driver OK!\n");

	return 0;

err_adapter:
	wake_lock_destroy(&hdmi_wake_lock);
	i2c_del_driver(&HDMI_i2c_driver);
out_unreg_class:
	class_destroy(HDMI_class);
out_unreg_chrdev:
	unregister_chrdev(HDMI_MAJOR, "imap-HDMI");
	unregister_chrdev(HDMI_POLL_MAJOR, "imap-HDMI_poll");

out:
	printk(KERN_ERR "%s: Driver Initialisation failed\n", __FILE__);

	return ret;
}

static int HDMI_remove(struct platform_device *pdev)
{
	struct imap_HDMI_info *info = (struct imap_HDMI_info *)platform_get_drvdata(pdev);

	wake_lock_destroy(&hdmi_wake_lock);
	i2c_unregister_device(info->client);
	i2c_del_driver(&HDMI_i2c_driver);
	class_destroy(HDMI_class);
	unregister_chrdev(HDMI_MAJOR, "imap-HDMI");
	unregister_chrdev(HDMI_POLL_MAJOR, "imap-HDMI_poll");

	return 0;
}

#if defined(CONFIG_PM)
int HDMI_suspend(struct platform_device *dev, pm_message_t state)
{
	struct imap_HDMI_info *info = (struct imap_HDMI_info *)platform_get_drvdata(dev);
	printk(KERN_INFO "Suspend HDMI\n");
	cancel_delayed_work_sync(&info->delayed_work);
	return 0;
}
		
int HDMI_resume(struct platform_device *dev)
{
        printk(KERN_INFO "Resume HDMI\n");
	return 0;
}		
#endif

static struct platform_driver HDMI_driver = {
	.driver = {
		.name = "imap-HDMI",
		.owner = THIS_MODULE,
	},
	.probe = HDMI_probe,
	.suspend = HDMI_suspend,
	.resume = HDMI_resume,
	.remove = HDMI_remove,
};

static int __init HDMI_init(void)
{
	return platform_driver_register(&HDMI_driver);
}

static void __exit HDMI_exit(void)
{
	platform_driver_unregister(&HDMI_driver);
}

module_init(HDMI_init);
module_exit(HDMI_exit);

MODULE_DESCRIPTION("Infotm HDMI driver");
MODULE_AUTHOR("Alex Zhang, <alex.zhang@infotmic.com.cn>");
MODULE_LICENSE("GPL");
