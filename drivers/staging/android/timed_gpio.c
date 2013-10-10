/* drivers/misc/timed_gpio.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/time.h>
//#include <plat/imapx.h>
#include <asm/io.h>

#include <mach/pad.h>
#include <mach/items.h>
#include <linux/regulator/consumer.h>

#include "timed_output.h"
#include "timed_gpio.h"

#ifdef CONFIG_FAKE_PM
#include <plat/fake_pm.h>
#endif

#define GPIO_MOTOR_EN	"cond17"

struct timed_gpio_data {
	struct timed_output_dev dev;
	struct hrtimer timer;
	spinlock_t lock;
	unsigned 	gpio;
	int 		max_timeout;
	u8 		active_low;
	struct regulator *regulator;
};

//static unsigned int motor_gpio;
static int motor_en_index;

static struct timed_gpio android_gpio = {
	.name		= "vibrator",
	.max_timeout	= 10000,
	.active_low	= 0,
};

static struct timed_gpio_platform_data android_vibrator_data = {
	.num_gpios      = 1,
	.gpios          = &android_gpio,
};

static void gpio_output_direction(int direction)
{
	unsigned int temp;

	if(motor_en_index == -1)
	{
		printk(KERN_ERR "motor gpio error.\n");
		return ;
	}

	if(direction > 0)
		imapx_pad_set_outdat(1, 1, motor_en_index);//output 1
//	  imapx_gpio_setpin(motor_gpio, 1, IG_NORMAL);
	else if(direction <= 0)
		imapx_pad_set_outdat(0, 1, motor_en_index);//output 0
//	  imapx_gpio_setpin(motor_gpio, 0, IG_NORMAL);

	//printk("gpio_output_direction set motor.en to %d\n", direction?1:0);
}

struct timeval time_start;
struct timeval time_end;
__kernel_time_t time;

static enum hrtimer_restart gpio_timer_func(struct hrtimer *timer)
{
	struct timed_gpio_data *data =
		container_of(timer, struct timed_gpio_data, timer);

	//	gpio_direction_output(data->gpio, data->active_low ? 1 : 0);
	gpio_output_direction(data->active_low ? 1 : 0);
	return HRTIMER_NORESTART;
}

static int gpio_get_time(struct timed_output_dev *dev)
{
	struct timed_gpio_data	*data =
		container_of(dev, struct timed_gpio_data, dev);

	if (hrtimer_active(&data->timer)) {
		ktime_t r = hrtimer_get_remaining(&data->timer);
		struct timeval t = ktime_to_timeval(r);
		return t.tv_sec * 1000 + t.tv_usec / 1000;
	} else
		return 0;
}

static void gpio_enable(struct timed_output_dev *dev, int value)
{
	struct timed_gpio_data	*data =
		container_of(dev, struct timed_gpio_data, dev);
	unsigned long	flags;

	spin_lock_irqsave(&data->lock, flags);

	/* cancel previous timer and set GPIO according to value */
	hrtimer_cancel(&data->timer);
	//	gpio_direction_output(data->gpio, data->active_low ? !value : !!value);
	gpio_output_direction(data->active_low ? !value : !!value);
	if (value > 0) {
		if (value > data->max_timeout)
			value = data->max_timeout;

		hrtimer_start(&data->timer,
				ktime_set(value /1000, (value % 1000) * 1000000),
				HRTIMER_MODE_REL);

	}

	spin_unlock_irqrestore(&data->lock, flags);
}

static int timed_gpio_probe(struct platform_device *pdev)
{
	//struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
	struct timed_gpio_platform_data *pdata = &android_vibrator_data;
	struct timed_gpio *cur_gpio;
	struct timed_gpio_data *gpio_data, *gpio_dat;
	int i, j, ret = 0;
	struct regulator *regulator;
	int err;
	char buf[ITEM_MAX_LEN];

	printk("!!!!!!!!!!!!!!!!%s enter!\n", __func__);
//	if (!pdata)
//		return -EBUSY;

	gpio_data = kzalloc(sizeof(struct timed_gpio_data) * pdata->num_gpios,
			GFP_KERNEL);
	if (!gpio_data)
		return -ENOMEM;

	item_string(buf, "motor.power", 1);
	printk("motor.power is %s\n", buf);
	regulator = regulator_get(&pdev->dev, buf);
	if(IS_ERR(regulator))
	{
		printk("%s: get regulator fail\n", __func__);
		return -1;
	}
	err = regulator_set_voltage(regulator, 2700000, 2700000);
	if(err)
		printk("%s: set regulator fail\n", __func__);
	regulator_enable(regulator);

	gpio_data->regulator = regulator;

	motor_en_index = item_integer("motor.en", 1);;
	//if(motor_en_index != -1) {
		imapx_pad_set_mode(1, 1, motor_en_index);//set mode to gpio
		imapx_pad_set_dir(0, 1, motor_en_index);//set dir to output
	//}else {
	//	printk("motor_en_index error!\n");
	//	return -1;
	//}

	for (i = 0; i < pdata->num_gpios; i++) {
		cur_gpio = &pdata->gpios[i];
		gpio_dat = &gpio_data[i];

		hrtimer_init(&gpio_dat->timer, CLOCK_MONOTONIC,
				HRTIMER_MODE_REL);
		gpio_dat->timer.function = gpio_timer_func;
		spin_lock_init(&gpio_dat->lock);

		gpio_dat->dev.name = cur_gpio->name;
		gpio_dat->dev.get_time = gpio_get_time;
		gpio_dat->dev.enable = gpio_enable;
		ret = timed_output_dev_register(&gpio_dat->dev);
		if (ret < 0)
		{
//			gpio_free(cur_gpio->gpio);
		}

/*		
		ret = gpio_request(cur_gpio->gpio, cur_gpio->name);
		if (ret >= 0) {
			ret = timed_output_dev_register(&gpio_dat->dev);
			if (ret < 0)
				gpio_free(cur_gpio->gpio);
		}
		if (ret < 0) {
			for (j = 0; j < i; j++) {
				timed_output_dev_unregister(&gpio_data[i].dev);
				gpio_free(gpio_data[i].gpio);
			}
			kfree(gpio_data);
			return ret;
		}
*/
		gpio_dat->gpio = cur_gpio->gpio;
		gpio_dat->max_timeout = cur_gpio->max_timeout;
		gpio_dat->active_low = cur_gpio->active_low;
//		gpio_direction_output(gpio_dat->gpio, gpio_dat->active_low);
		gpio_output_direction(gpio_dat->active_low);
	}

	//motor_gpio = __imapx_name_to_gpio(CONFIG_IG_MOTOR_ENABLE);
	
	//imapx_pad_set_dir(0, 1, motor_en_index);//set dir to output

	platform_set_drvdata(pdev, gpio_data);

	printk("!!!!!!!!!!!!!!!!!!!!%s end\n", __func__);
	return 0;
}

static int timed_gpio_remove(struct platform_device *pdev)
{
//	struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
	struct timed_gpio_platform_data *pdata = &android_vibrator_data;
	struct timed_gpio_data *gpio_data = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < pdata->num_gpios; i++) {
		timed_output_dev_unregister(&gpio_data[i].dev);
		gpio_free(gpio_data[i].gpio);
	}

	regulator_put(gpio_data->regulator);
	kfree(gpio_data);

	return 0;
}

#ifdef CONFIG_PM
static int timed_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct timed_gpio_data *gpio_data = platform_get_drvdata(pdev);

	regulator_disable(gpio_data->regulator);

	return 0;
}

static int timed_gpio_resume(struct platform_device *pdev)
{
	struct timed_gpio_platform_data *pdata = &android_vibrator_data;
	struct timed_gpio_data *gpio_data = platform_get_drvdata(pdev);
	struct timed_gpio_data *gpio_dat;
	int i;

	regulator_enable(gpio_data->regulator);

	imapx_pad_set_mode(1, 1, motor_en_index);
	imapx_pad_set_dir(0, 1, motor_en_index);

	for (i = 0; i < pdata->num_gpios; i++) {
		gpio_dat = &gpio_data[i];
		gpio_output_direction(gpio_dat->active_low);
	}

	return 0;
}
#else
#define timed_gpio_suspend	NULL
#define timed_gpio_resume	NULL
#endif

static struct platform_driver timed_gpio_driver = {
	.probe		= timed_gpio_probe,
	.remove		= timed_gpio_remove,
	.suspend	= timed_gpio_suspend,
	.resume		= timed_gpio_resume,
	.driver		= {
		.name		= TIMED_GPIO_NAME,
		.owner		= THIS_MODULE,
	},
};

static int __init timed_gpio_init(void)
{
	if(item_exist("motor.en")) {
		return platform_driver_register(&timed_gpio_driver);
	}
	else {
		printk("There is no motor\n");
		return -1;
	}
}

static void __exit timed_gpio_exit(void)
{
	platform_driver_unregister(&timed_gpio_driver);
}

module_init(timed_gpio_init);
module_exit(timed_gpio_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("timed gpio driver");
MODULE_LICENSE("GPL");
