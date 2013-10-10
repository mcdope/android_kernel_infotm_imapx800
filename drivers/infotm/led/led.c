/*
 * led.c
 *
 * Copyright (c) 2012~2014 ShangHai InfoTM Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sololz <sololz.luo@gmail.com> or <sololz.luo@infotmic.com.cn>.
 */

#include "led.h"

static struct led_light gled_light[] = {
    [LED_LIGHT_WORKING] = {     /* Working light. */
        .trigger = LED_TRIGGER_WORKING | LED_TRIGGER_PIN_HIGH,
        .gpio = CONFIG_LED_WORKING_PIN,
        .level = LED_MUTEX_LEVEL_RUN,
    }, 
    [LED_LIGHT_SUSPEND] = {     /* Suspending light. */
        .trigger = LED_TRIGGER_SUSPEND | LED_TRIGGER_PIN_HIGH,
        .gpio = CONFIG_LED_SUSPEND_PIN,
        .level = LED_MUTEX_LEVEL_RUN,
    },
};

static struct led gled = {
    .light_cnt = 0,
    .flashing = 0,
    .flashing_trigger = 0,
};

static void led_otguart2(int on)
{
    static void __iomem * u2base = NULL;

    if(!u2base) {
        u2base = ioremap(IMAP_UART2_BASE, SZ_4K);
        if(!u2base)
            return ;

        writel(1, IO_ADDRESS(SYSMGR_OTG_BASE) + 0x68);
        writel(1, IO_ADDRESS(SYSMGR_OTG_BASE) + 0x6c);
        writel(1, u2base + 0x80);
    }

    writel(!!on, u2base + 0x88);
}

static void led_gpio_init(void)
{
    int i = 0;
    if(gled_light[0].gpio == -2)
        return ;

    for (i = 0; i < gled.light_cnt; i++) {
        imapx_pad_set_dir(DIRECTION_OUTPUT, 1, gled_light[i].gpio);
        /* Set light dark at first. */
        imapx_pad_set_outdat((gled_light[i].trigger & LED_TRIGGER_PIN_HIGH) ? 
                OUTPUT_0 : OUTPUT_1, 1, gled_light[i].gpio);
    }

    led_working_light();
}

/***********************************************************************************************/
/*
 * LED flashing support. At first, this LED flashing is designed for factory burn 
 * finishing check. Because in factory, workers who burn boards do not know when 
 * a board is fully finished burning. So, while burning, application level code 
 * calls flashing interface to set led keep flashing. Once finish buring, set it
 * light.
 */

inline static int led_is_flashing(void)
{
    return !!gled.flashing;
}

static void led_set_flash(int val)
{
    if (val) {
        if (!gled.flashing) {
            gled.flashing = val;
            schedule_delayed_work(&gled.flashing_work, 0);
        }
    } else {
        gled.flashing = 0;
        cancel_delayed_work_sync(&gled.flashing_work);
    }
}

static void do_flashing_work(struct work_struct *work)
{
    if (led_is_flashing()) {
        schedule_delayed_work(&gled.flashing_work, msecs_to_jiffies(gled.flashing));
        gled.flashing_trigger ? led_working_light() : led_working_dark();
        gled.flashing_trigger = !gled.flashing_trigger;
    }
}

static ssize_t attr_led_is_flashing(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s", led_is_flashing() ? "enabled" : "disabled");
}

static ssize_t attr_led_set_flash(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
	int freq = simple_strtol(buf, NULL, 10);
	if(freq == 1)
		led_working_light();
	else if(!freq)
		led_working_dark();
	else {
		led_set_flash(freq); 
		goto end;
	}

	led_set_flash(0); 
end:
	return count;
}

static DEVICE_ATTR(flash_led, 0666, attr_led_is_flashing, attr_led_set_flash);

static struct attribute *led_attrs[] = {
    &dev_attr_flash_led.attr,
    NULL,
};

static struct attribute_group led_attr_group = {
        .attrs = led_attrs,
};

/***********************************************************************************************/

static int led_probe(struct platform_device *pdev)
{
    int ret = 0;

    gled.light_cnt = sizeof(gled_light) / sizeof(gled_light[0]);
    led_gpio_init();

    INIT_DELAYED_WORK(&gled.flashing_work, do_flashing_work);

    ret = sysfs_create_link(NULL, &pdev->dev.kobj, "led");
    if (ret) {
        LED_ERR("sysfs_create_link() for led error, %d!\n", ret);
        return ret;
    }

    ret = sysfs_create_group(&pdev->dev.kobj, &led_attr_group);
    if (ret) {
        LED_ERR("sysfs_create_group() for led error %d!\n", ret);
        return ret;
    }

    LED_DBG("LED probe finish.\n");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    LED_DBG("LED removed.\n");
    return 0;
}

static int led_suspend(struct platform_device *pdev, pm_message_t state)
{
    led_suspending_light();
    return 0;
}

static int led_resume(struct platform_device *pdev)
{
    led_gpio_init();
    return 0;
}

static struct platform_driver led_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "led",
    },
    .probe = led_probe,
    .remove = led_remove,
    .suspend = led_suspend, 
    .resume = led_resume,
};

static int led_lights_adjust(void)
{
    if (item_exist("led.working.pin")) {
        if(item_equal("led.working.pin", "otguart2", 0)) {
            gled_light[LED_LIGHT_WORKING].gpio = -2;
            return 0;
        }
        gled_light[LED_LIGHT_WORKING].gpio = item_integer("led.working.pin", 0);
    }else{
        return 1;
    }
    if (item_exist("led.suspend.pin")) {
        gled_light[LED_LIGHT_SUSPEND].gpio = item_integer("led.suspend.pin", 0);
    }
    return 0;
}

static int __init led_init(void)
{
    //Be5t0 modify it
    if (led_lights_adjust()) {
        return 0;
    }

    return platform_driver_register(&led_driver);
}

static void __exit led_exit(void)
{
    platform_driver_unregister(&led_driver);
}

/*********************************************************************/

static void led_dark(int level)
{
    int i = 0;

    if(gled_light[0].gpio == -2) {
        led_otguart2(0);
        return ;
    }

    for (i = 0; i < gled.light_cnt; i++) {
        if (gled_light[i].level == level) {
            imapx_pad_set_outdat((gled_light[i].trigger & LED_TRIGGER_PIN_HIGH) ? 
                    OUTPUT_0 : OUTPUT_1, 1, gled_light[i].gpio);
        }
    }
}

void led_working_light(void)
{
    int i = 0;

    if(gled_light[0].gpio == -2) {
        led_otguart2(1);
        return ;
    }

    led_dark(LED_MUTEX_LEVEL_RUN);
    for (i = 0; i < gled.light_cnt; i++) {
        if ((gled_light[i].trigger & LED_TRIGGER_TYPE_MASK) == LED_TRIGGER_WORKING) {
            imapx_pad_set_outdat((gled_light[i].trigger & LED_TRIGGER_PIN_HIGH) ? 
                    OUTPUT_1 : OUTPUT_0, 1, gled_light[i].gpio);
        }
    }
}
EXPORT_SYMBOL(led_working_light);

void led_working_dark(void)
{
    led_dark(LED_MUTEX_LEVEL_RUN);
}
EXPORT_SYMBOL(led_working_dark);

void led_suspending_light(void)
{
    int i = 0;

    if(gled_light[0].gpio == -2)
	    return ;

    led_dark(LED_MUTEX_LEVEL_RUN);
    for (i = 0; i < gled.light_cnt; i++) {
        if ((gled_light[i].trigger & LED_TRIGGER_TYPE_MASK) == LED_TRIGGER_SUSPEND) {
            imapx_pad_set_outdat((gled_light[i].trigger & LED_TRIGGER_PIN_HIGH) ? 
                    OUTPUT_1 : OUTPUT_0, 1, gled_light[i].gpio);
        }
    }
}
EXPORT_SYMBOL(led_suspending_light);

void led_suspending_dark(void)
{
    led_dark(LED_MUTEX_LEVEL_RUN);
}
EXPORT_SYMBOL(led_suspending_dark);

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz Luo");
MODULE_DESCRIPTION("LED driver on IMAPX platform");
