/*
 * led.h
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

#ifndef __LED_H__
#define __LED_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/led_export.h>

#include <mach/pad.h>
#include <mach/items.h>
#include <mach/imap-iomap.h>

#include <asm/io.h>

#define LED_ERR(err, ...)       do { \
    printk(KERN_ERR "[LED ERR] %s %d: " err, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)
#ifdef CONFIG_IMAPX_LED
#define LED_DBG(dbg, ...)       do { \
    printk(KERN_INFO "[LED DBG] " dbg, ##__VA_ARGS__); \
} while (0)
#else
#define LED_DBG(dbg, ...)       do {} while (0)
#endif

/* Trigger type. */
#define LED_TRIGGER_PIN_HIGH    (0x80000000)
#define LED_TRIGGER_WORKING     (0x00000001)
#define LED_TRIGGER_SUSPEND     (0x00000002)
#define LED_TRIGGER_BOOT        (0x00000003)
#define LED_TRIGGER_TYPE_MASK   (0x0000ffff)

/* LED mutex level. */
#define LED_MUTEX_LEVEL_RUN     (1)
#define LED_MUTEX_LEVEL_BOOT    (2)

enum {
    LED_LIGHT_WORKING = 0,
    LED_LIGHT_SUSPEND,
};

struct led_light {
    unsigned int trigger;
    int gpio;
    int level;
};

struct led {
    int light_cnt;
    int flashing;
    int flashing_trigger;
    struct delayed_work flashing_work;
};

#endif  /* __LED_H__ */
