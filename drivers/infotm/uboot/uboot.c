/***************************************************************************** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: Infotm board configuration system.
**
** Author:
**     peter <peter.fu@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  07/19/2012
*****************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/rtc.h>
#include <linux/gpio.h>


#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/mem-reserve.h>

#define UBOOT_MAJOR  168
#define UBOOT_NAME   "uboot0"
#define PHY_MEM_BASE_UBOOT0     RESERVED_BASE_UBOOT0
#define SIZE_32K                0x8000


static int uboot_pos = 0;
static int uboot0_len = 0;
static void *uboot0 = NULL;


int uboot_export(char *buf , size_t len)
{
    int _l; 

    if(!buf) {
        uboot_pos = 0;
        return 0;
    }   

    if(uboot_pos >= uboot0_len)
        return 0;

    _l = (len > (uboot0_len - uboot_pos))?
        (uboot0_len- uboot_pos): len;

    if(_l) {
        memcpy(buf, uboot0 + uboot_pos, _l);
        uboot_pos += _l; 
    }   

    return _l; 


}

int uboot_dev_open(struct inode *fi, struct file *fp)
{
    uboot_export(NULL,0);
    return 0;
}

int uboot_dev_close(struct inode *fi, struct file *fp)
{
    return 0;
}

static int uboot_dev_read(struct file *filp,
   char __user *buf, size_t length, loff_t *offset)
{
    return uboot_export(buf, length);
}

static const struct file_operations uboot_dev_fops = {
	.read = uboot_dev_read,
	.open = uboot_dev_open,
    //.close = uboot_dev_close,
};

static int __init uboot_dev_init(void)
{
	struct class *cls;
	int ret;
    uint8_t * tmp = NULL;

    
	printk(KERN_INFO "uboot driver (c) 2009, 2014 InfoTM\n");

	/* create gps dev node */
	ret = register_chrdev(UBOOT_MAJOR, UBOOT_NAME, &uboot_dev_fops);
	if(ret < 0)
	{
		printk(KERN_ERR "Register char device for uboot failed.\n");
		return -EFAULT;
	}

	cls = class_create(THIS_MODULE, UBOOT_NAME);
	if(!cls)
	{
		printk(KERN_ERR "Can not register class for uboot .\n");
		return -EFAULT;
	}

	/* create device node */
	device_create(cls, NULL, MKDEV(UBOOT_MAJOR, 0), NULL, UBOOT_NAME);

    uboot0 = (uint8_t *) kmalloc(SIZE_32K, GFP_KERNEL);
    tmp = phys_to_virt(PHY_MEM_BASE_UBOOT0);
    uboot0_len = ((tmp[15] << 24) | (tmp[14] << 16) | (tmp[13] << 8) | tmp[12]) + 512;
    if(uboot0_len < SIZE_32K)
        memcpy(uboot0, tmp, uboot0_len);
    
	return 0;
}

static void __exit uboot_dev_exit(void) {}

module_init(uboot_dev_init);
module_exit(uboot_dev_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("uboot interface device export");

