/***************************************************************************** 
** XXX driver/power/imapx820_battery.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: imapx820 battery driver.
**
** Author:
**     jack   <jack.zhang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 0.1  XXX 10/17/2012 XXX	jack
*****************************************************************************/

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/ioport.h>

#include <asm/io.h>

#include "imapx820_battery.h"
#include "mp2611_charger.h"



static int __devinit mp2611_probe(struct platform_device *pdev)
{
	
}

static int __devexit  mp2611_remove(struct platform_device *pdev)
{

}

#ifdef CONFIG_PM
static int
mp2611_suspend(struct platform_device *pdev, pm_message_t state) 
{
}

static int
mp2611_resume(struct platform_device *pdev) 
{

}
#else
#define  mp2611_suspend NULL
#define  mp2611_resume  NULL
#endif


static struct platform_driver mp2611_driver = {
	.probe = mp2611_probe,
	.remove = __devexit_p(mp2611_remove),
	.suspend = mp2611_suspend,
	.resume = mp2611_resume,
	.driver = {
		.name = "mp2611_charger",
		.owner = THIS_MODULE,
	}
}

static int __init mp2611_init(void)
{
	return platform_driver_register(mp2611_driver);
}

static void __exit mp2611_exit(void)
{
	return platform_driver_unregister(mp2611_driver);
}

module_init(mp2611_init);
module_exit(mp2611_exit);

MODULE_AUTHOR("jack <jack.zhang@infotmic.com.cn>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mp2611 charger driver for iMAPx820");
