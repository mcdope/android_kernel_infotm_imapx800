/*
 * camera.h
 *
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Head file of IMAPX platform camera driver.
 *
 * Author:
 *	Sololz<sololz.luo@gmail.com>.
 *
 * Revision History:
 * 1.0  04/25/2011 Sololz.
 * 	Create this file.
 */

#if !defined(__CAMERA_H__)
#define __CAMERA_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/list.h>

#include <asm/io.h>
#include <asm/atomic.h>

#include "api_camera.h"
#include "cfg_camera.h"

#define IC_ERR(err, ...)	do { \
	printk(KERN_ERR "%s %d: " err, \
			__FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)
#if defined(CONFIG_IMAPX_CAMERA_DEBUG_ENABLE)
#define IC_DBG(dbg, ...)	do { \
	printk(KERN_ALERT dbg, ##__VA_ARGS__); \
} while (0)
#else
#define IC_DBG(dbg, ...)	do {} while (0)
#endif

#endif	/* __CAMERA_H__ */
