/************************************************************
**   driver/video/backlight/imapx800_bl.c
**  
**   Copyright (c) 2009~2014 ShangHai Infotm .Ltd all rights reserved.
**
**   Use of infoTM's code  is governed by terms and conditions
**   stated in the accompanying licensing statment.
**   
**   Description: backlight control driver for imapx800 SOC
** 
**
**   AUTHOR:
**   Haixu Fu	 <haixu_fu@infotm.com>
**   warits		 <warits.wang@infotm.com>
**
**   
**   Revision History:
**  ---------------------------------
**   1.1  03/06/2010   Haixu Fu
**   1.2  04/01/2010   warits (add android features)
**   1.3  05/25/2012   bob (for imapx800)
*******************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/sysdev.h>
#include <linux/regulator/consumer.h>
#include <mach/items.h>
#include <mach/pad.h>

#include <linux/gpio.h>
#ifdef CONFIG_FAKE_PM
#include <fake_pm.h>
#endif

#define	IMAP_MAX_INTENSITY (0xff)
#define	IMAP_DEFAULT_INTENSITY IMAP_MAX_INTENSITY

#define BL_DEBUG 0

/* debug code */
#define bl_dbg(debug_level, msg...) do\
{\
	if (debug_level > 0)\
	{\
		printk(msg);\
	}\
} while (0)

struct backlight_device *gbd;
int g_intensity = 0;
int g_pwm_chn = -1;
static int imapbl_suspended = 0;
static int imapbl_cur_brightness = IMAP_DEFAULT_INTENSITY;

struct imapbl_data {
	int current_intensity;
	int suspend;
	struct regulator *regulator;
};

static DEFINE_MUTEX(bl_mutex);
static void imapbl_write_intensity(int intensity);
static void imapbl_send_intensity(struct backlight_device *bd);

extern int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long gtcmp);
extern int pwm_change_width(int channel, int intensity);
extern int imap_pwm_suspend(struct sys_device *pdev, pm_message_t pm);
extern int imap_pwm_resume(struct sys_device *pdev);
extern int imap_back_poweron();
extern void imapbl_power_on();
extern void imapbl_shut_down();

void imapbl_power_on()
{
	int m;

	//printk("imapbl_power_on\n");	
	imap_back_poweron();
	if (item_exist("bl.msleep"))
		m = item_integer("bl.msleep",0);
	else
		m  = 0;
	msleep(m);

    switch(g_pwm_chn)
    {
        case 0:
            imapx_pad_cfg(IMAPX_PWM, 1);
            break;
        case 1:
            imapx_pad_cfg(IMAPX_PWM1, 1);
            break;
        case 2:
            imapx_pad_cfg(IMAPX_PWM2, 1);
            break;
        default:
            break;
    }
	return;

}

void imapbl_shut_down()
{
    int g_tcmp, tout;

   // printk("imapbl_shut_down\n");	

	switch(g_pwm_chn)
    {
        case 0:
            imapx_pad_cfg(IMAPX_PWM, 0);
            break;
        case 1:
            imapx_pad_cfg(IMAPX_PWM1, 0);
            break;
        case 2:
            imapx_pad_cfg(IMAPX_PWM2, 0);
            break;
        default:
            break;
    }

	return;
}
EXPORT_SYMBOL(imapbl_shut_down);

static void imapbl_write_intensity(int intensity)
{
	int g_tcmp, tout;
	int tmp_start, tmp_end;
	unsigned long gpio;
	
	//printk("%s: intensity=%d\n", __func__, intensity);	

	//tout = 0;
	//tmp_start = tmp_end = 0;
	//tmp_start = item_integer("bl.start",0);
	//tmp_end = item_integer("bl.end",0);

	//g_tcmp = (tmp_start - tmp_end) * intensity / 0xff + tmp_end;

	//printk("===========start = %d, end = %d, %d, %d, %d\n", tmp_start, tmp_end, intensity, tmp_start, g_tcmp);
	//imap_timer_setup(tout, tmp_start, g_tcmp);

	pwm_change_width(g_pwm_chn, intensity);
	return;
}
	
static void imapbl_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);
	
	g_intensity = bd->props.brightness;

	if(bd->props.power != FB_BLANK_UNBLANK)
		intensity = 0;
	if(bd->props.fb_blank != FB_BLANK_UNBLANK)
		intensity = 0;
	if(imapbl_suspended)
		intensity = 0;
	mutex_lock(&bl_mutex);
	imapbl_write_intensity(intensity);
	mutex_unlock(&bl_mutex);
	devdata->current_intensity = intensity ;
}

static int imapbl_set_intensity(struct backlight_device *bd)
{
	imapbl_send_intensity(bd);
	return 0;
}

/* set and get funcs for ANDROID */
static void imapbl_leds_set_brightness(struct led_classdev *led_cdev,
   int brightness)
{
	mutex_lock(&bl_mutex);
	imapbl_write_intensity(brightness);
	imapbl_cur_brightness = brightness;
	mutex_unlock(&bl_mutex);
}   

static int imapbl_leds_get_brightness(struct led_classdev *led_cdev)
{
	return imapbl_cur_brightness;
}

static int imapbl_get_intensity(struct backlight_device *bd)
{
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);
	return devdata->current_intensity;
}

static struct backlight_ops imapbl_ops = {
	.get_brightness = imapbl_get_intensity,
	.update_status 	= imapbl_set_intensity,
};

#ifdef CONFIG_PM

static int imapbl_suspend(struct platform_device *pdev,pm_message_t state)
{
	/* XXX ANDROID XXX */
	int v33;
	struct backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	printk("++ %s ++\n", __func__);
	//imap_pwm_suspend(NULL, state);

	if(devdata->regulator)
		regulator_disable(devdata->regulator);
	return 0;
}

static int imapbl_resume(struct platform_device *pdev)
{
	/* XXX ANDROID XXX */
	int v33;
	struct backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	printk("++ %s ++\n", __func__);
	if(devdata->regulator)
		regulator_enable(devdata->regulator);

	imap_pwm_resume(NULL);
	return 0;
}

#else

#define  imapbl_suspend NULL
#define	 imapbl_resume  NULL

#endif
static struct led_classdev imap_bl_cdev = {
	.name = "lcd-backlight",
	.brightness_set = imapbl_leds_set_brightness,
	.brightness_get = imapbl_leds_get_brightness,
};

void imapbl_lowlevel_blctrl(int on)
{
}
EXPORT_SYMBOL(imapbl_lowlevel_blctrl);

static  int imapbl_probe(struct platform_device *pdev)
{
	int error;
	struct backlight_device *bd;
	struct imapbl_data *devdata;
	
#ifdef CONFIG_FAKE_PM
	if_in_suspend = 0;
#endif

	devdata = kzalloc(sizeof(struct imapbl_data),GFP_KERNEL);
	if(!devdata)
		return -ENOMEM;
    if(item_equal("bl.ctrl", "pwm", 0))
    {
        g_pwm_chn = item_integer("bl.ctrl",1);
    } else 
        g_pwm_chn = 0;

	if(item_exist("bl.power")) {
		struct regulator *regulator;
		char buf[ITEM_MAX_LEN];

		item_string(buf, "bl.power", 1);

		regulator = regulator_get(&pdev->dev, buf);
		if(IS_ERR(regulator)) {
			printk("%s: get regulator fail\n", __func__);
			return -1;
		}
		regulator_enable(regulator);

		devdata->regulator = regulator;
	}

	bd = backlight_device_register ("lcd-backlight",&pdev->dev,devdata,
			&imapbl_ops, NULL);

	if(IS_ERR(bd))
		return PTR_ERR(bd);

	gbd = bd;
	/* Register LEDS for ANDROID GUI backlight control */
	/**                                                                                           
	 * led_classdev_register - register a new object of led_classdev class.                       
	 * @parent: The device to register.                                                           
	 * @led_cdev: the led_classdev structure for this device.                                     
	 */                                                                                           
	error = led_classdev_register(&pdev->dev, &imap_bl_cdev);
	if(error)
	{
		printk(KERN_INFO "Registe leds lcd-backlight failed.\n");
	}

	platform_set_drvdata(pdev,bd);
	bd->props.max_brightness = IMAP_MAX_INTENSITY;
	bd->props.brightness 	 = IMAP_DEFAULT_INTENSITY;

	g_intensity = bd->props.brightness; 
	return 0;
}

static int imapbl_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	if(devdata->regulator)
		regulator_put(devdata->regulator);	
	kfree(devdata);
	bd->props.brightness = 0;
	bd->props.power	= 0;
	imapbl_send_intensity(bd);
	backlight_device_unregister(bd);

	return 0;
}

static struct platform_driver imapbl_driver = {
	.probe		= imapbl_probe,
	.remove		= imapbl_remove,
	.suspend	= imapbl_suspend,
	.resume		= imapbl_resume,
	.driver		= {
		.name	= "imap-backlight",
		.owner  = THIS_MODULE,
	},
};

static int __init imapbl_init(void)
{
	int ret;
	ret = platform_driver_register(&imapbl_driver);
	if(ret)
		return ret;
	return 0;
}

static void __exit imapbl_exit(void)
{
	platform_driver_unregister(&imapbl_driver);
}
module_init(imapbl_init);
module_exit(imapbl_exit);

MODULE_AUTHOR("ronaldo, warits, bob");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IMAPX800 Backlight Driver");

