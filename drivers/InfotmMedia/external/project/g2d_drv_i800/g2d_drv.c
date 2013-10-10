/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --

-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: g2d_drv.c
--
--  Description :
--		IMAPX800 G2D driver process.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8.
--
------------------------------------------------------------------------------*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/mman.h>
#include <linux/ioctl.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/atomic.h>

#include <mach/irqs.h>
#include <mach/power-gate.h>

#include <InfotmMedia.h> 
#include <IM_g2dapi.h> 

#include <g2d_pwl.h>
#include <g2d_kui.h>
#include "g2d_drv.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"G2DDRV_I:"
#define WARNHEAD	"G2DDRV_W:"
#define ERRHEAD		"G2DDRV_E:"
#define TIPHEAD		"G2DDRV_T:"

/* 
 * Function declaration.
 */
static int g2ddrv_open(struct inode *inode, struct file *file);
static int g2ddrv_release(struct inode *inode, struct file *file);
static ssize_t g2ddrv_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
static ssize_t g2ddrv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int g2ddrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int g2ddrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int g2ddrv_mmap(struct file *file, struct vm_area_struct *vma);
static unsigned int g2ddrv_poll(struct file *file, poll_table *wait);

static int g2ddrv_probe(struct platform_device *pdev);
static int g2ddrv_remove(struct platform_device *pdev);
static int g2ddrv_suspend(struct platform_device *pdev, pm_message_t state);
static int g2ddrv_resume(struct platform_device *pdev);

static void g2ddrv_module_enable(void);
static void g2ddrv_module_disable(void);
static void g2ddrv_module_reset(void);

/*
 * this structure include global varaibles
 */
static unsigned int g2ddrv_open_count = 0;/* record open syscall times */
struct mutex g2ddrv_lock;

/***********************************************************************************/
/*
 * Tutorial:
 * File operation driver interface in Linux kernel. Most api of *struct
 * file_operations* are system calls which are called by user space
 * application entering kernel trap. And most of them have corresponding
 * user space POSIX api, etc(first kernel, then POSIX), open() via open(), 
 * release() via close(), write() via write(), read() via read(), ioctl()
 * via ioctl().
 *
 * And also attention that, most apis in kernel return 0 means success,
 * else return a negative number(normally, return errno type, EINVAL etc).
 *
 * The misc device driver provides convenient driver node register.
 */
static struct file_operations g2ddrv_fops = {
    .owner		= THIS_MODULE,
    .open		= g2ddrv_open,
    .release	= g2ddrv_release,
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
    .unlocked_ioctl = g2ddrv_ioctl,
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
	.ioctl = g2ddrv_ioctl,
#endif
    .read 		= g2ddrv_read,
    .write		= g2ddrv_write,
    .mmap		= g2ddrv_mmap,
    .poll		= g2ddrv_poll,
};
static struct miscdevice g2ddrv_miscdev = {
    .minor	= MISC_DYNAMIC_MINOR,
    .fops	= &g2ddrv_fops,
    .name	= "g2d",
};

//#####################################################################################

/*
 * g2d kernel api for g2d kernel app call, now only memory operator support,
 * and support sync call only for fast memory operator
 *
 * some config struct must define in g2d_drv.h if other operator need support,
 * and kernel app must include this headfile(#include <g2d_drv.h>) at this condition
 */

void *g2dkapi_init(void)
{
	void *inst = IM_NULL;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(g2ddrv_open_count == 0)
	{
		//g2d module enable
		g2ddrv_module_enable();
	}
	g2ddrv_open_count++;

	inst = g2dkui_init();
	if(inst == NULL){
		IM_ERRMSG((IM_STR("@g2dkapi_init(), g2dkui_init() failed!")));
		return IM_NULL;
	}

	return inst;
}
EXPORT_SYMBOL(g2dkapi_init);

IM_RET g2dkapi_deinit(void *inst)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	/*if(inst == NULL){
		IM_ERRMSG((IM_STR("@g2dkapi_deinit(), inst is NULL!")));
		return IM_RET_FAILED;
	}*/

	ret = g2dkui_deinit(inst);
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("@g2dkapi_deinit(), g2dkui_deinit() failed!")));
		return IM_RET_FAILED;
	}

	g2ddrv_open_count--;
	if(g2ddrv_open_count == 0)
	{
		g2ddrv_module_disable();
	}

	return IM_RET_OK;
}
EXPORT_SYMBOL(g2dkapi_deinit);

IM_RET g2dkapi_memcpy(void *inst, IM_UINT32 dst_phy, IM_UINT32 src_phy, IM_UINT32 size)
{
	IM_RET ret = IM_RET_OK;
	g2d_config_t config;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	/*if(inst == NULL){
		IM_ERRMSG((IM_STR("@g2dkapi_set_memory_config(), inst is NULL!")));
		return IM_RET_FAILED;
	}*/
 	g2dpwl_memset((void *)&config, 0x0, sizeof(g2d_config_t));
	config.property = G2D_OPP_MEM;
	config.useDevMMU = IM_FALSE;
	config.memCfg.type = G2D_MEM_CPY;
	config.memCfg.size = size;
	config.memCfg.srcBuf.phy_addr = src_phy;
	config.memCfg.dstBuf.phy_addr = dst_phy;

	ret = g2dkui_set_config(inst, &config);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_memcpy(), g2dkui_set_config() failed!")));
		return IM_RET_FAILED;
	}
	//only support sync call, so taskId = -1
	ret = g2dkui_do(inst, -1);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_memcpy(), g2dkui_do() failed!")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}
EXPORT_SYMBOL(g2dkapi_memcpy);

//set one byte unit
IM_RET g2dkapi_memset(void *inst, IM_UINT32 dst_phy, IM_INT32 value, IM_UINT32 size)
{
	IM_RET ret = IM_RET_OK;
	g2d_config_t config;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	/*if(inst == NULL){
		IM_ERRMSG((IM_STR("@g2dkapi_set_memory_config(), inst is NULL!")));
		return IM_RET_FAILED;
	}*/
 	g2dpwl_memset((void *)&config, 0x0, sizeof(g2d_config_t));
	config.property = G2D_OPP_MEM;
	config.useDevMMU = IM_FALSE;
	config.memCfg.type = G2D_MEM_SET;
	config.memCfg.size = size;
	config.memCfg.setValueL = value&0xff;
	config.memCfg.setValueH = 0x0;
	config.memCfg.setUnitByte = G2D_MEMSET_1BYTE;
	config.memCfg.dstBuf.phy_addr = dst_phy;

	ret = g2dkui_set_config(inst, &config);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_memset(), g2dkui_set_config() failed!")));
		return IM_RET_FAILED;
	}
	//only support sync call, so taskId = -1
	ret = g2dkui_do(inst, -1);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_memset(), g2dkui_do() failed!")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}
EXPORT_SYMBOL(g2dkapi_memset);


IM_RET g2dkapi_do_scale(void *inst, g2d_image_t *src, g2d_image_t *dst, IM_INT32 edgeSharpValue)
{
	IM_RET ret = IM_RET_OK;
	g2d_config_t config;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	/*if(inst == NULL){
	  IM_ERRMSG((IM_STR("@g2dkapi_set_memory_config(), inst is NULL!")));
	  return IM_RET_FAILED;
	  }*/

	g2dpwl_memset((void *)&config, 0x0, sizeof(g2d_config_t));
	config.property = G2D_OPP_ALPHAROP;
	config.useDevMMU = IM_FALSE;
	config.alpharopCfg.type = G2D_AROP_SCL ;
	//config.alpharopCfg.alpha = ;
	//config.alpharopCfg.rop = ;
	//config.alpharopCfg.imgSrc1 = ;
	g2dpwl_memcpy((void *)&config.alpharopCfg.imgSrc1, src, sizeof(g2d_image_t));
	//config.alpharopCfg.imgSrc2 = ;
	//config.alpharopCfg.imgPat = ;
	//config.alpharopCfg.imgDst = ;
	g2dpwl_memcpy((void *)&config.alpharopCfg.imgDst, dst, sizeof(g2d_image_t));
	//config.alpharopCfg.dith = ; //default enable at g2d_lib
	if (edgeSharpValue != 0) 
	{
		config.alpharopCfg.edgeSharpValue = edgeSharpValue - 1;
		config.alpharopCfg.type |= G2D_AROP_EE;
	}

	ret = g2dkui_set_config(inst, &config);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_do_scale(), g2dkui_set_config() failed!")));
		return IM_RET_FAILED;
	}
	//only support sync call, so taskId = -1
	ret = g2dkui_do(inst, -1);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dkapi_do_scale(), g2dkui_do() failed!")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}
EXPORT_SYMBOL(g2dkapi_do_scale);


//#############################################################################



/*
 * Tutorial:
 * Each open() system call returns a file descriptor if success. And as a
 * user space application, open() is a must step to access further kernel
 * module interfaces.
 */
static int g2ddrv_open(struct inode *inode, struct file *file)
{
	IM_INT32  ret = 0;
	g2ddrv_context_t *g2dDrv = NULL;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	mutex_lock(&g2ddrv_lock);

	if(g2ddrv_open_count == 0)
	{
		//g2d module enable
		g2ddrv_module_enable();
	}
	g2ddrv_open_count++;

	g2dDrv = kmalloc(sizeof(g2ddrv_context_t), GFP_KERNEL);
	if(g2dDrv == NULL){
		IM_ERRMSG((IM_STR("@g2ddrv_open(), kmalloc(g2ddrv_context_t) failed!")));
		ret = -EFAULT;
		goto Fail;
	}

	g2dDrv->g2dInst = g2dkui_init();
	if(g2dDrv->g2dInst == NULL){
		IM_ERRMSG((IM_STR("@g2ddrv_open(), g2dkui_init() failed!")));
		kfree(g2dDrv);
		ret = -EFAULT;
		goto Fail;
	}

	file->private_data = g2dDrv;
	mutex_unlock(&g2ddrv_lock);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;

Fail:
	g2ddrv_open_count--;
	if(g2ddrv_open_count == 0)
	{
		g2ddrv_module_disable();
	}
	mutex_unlock(&g2ddrv_lock);
	return ret;
}

/*
 * Tutorial:
 * After open() a driver node, the file descriptor must be closed if you
 * don't use it anymore.
 */
static int g2ddrv_release(struct inode *inode, struct file *file)
{
	IM_INT32 res = 0;
	IM_RET ret = IM_RET_OK;
	g2ddrv_context_t *g2dDrv = file->private_data;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	mutex_lock(&g2ddrv_lock);

	ret = g2dkui_deinit(g2dDrv->g2dInst);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2ddrv_release(), g2dkui_deinit() failed!")));
		res = -EFAULT;
		goto Fail;
	}

	kfree(g2dDrv);
	g2dDrv = NULL;
	
Fail:	
	g2ddrv_open_count--;
	if(g2ddrv_open_count == 0)
	{
		//g2d module disable
		g2ddrv_module_disable();
	}
	mutex_unlock(&g2ddrv_lock);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return res;
}

/*
 * Tutorial:
 * Normally, the *read* and *write* apis are not widely used because
 * most driver use *mmap* to map a kernel memory region to user space
 * in order to avoid memory copy. 
 * And something special with read & write is the return value is the
 * actual accessed data length.
 */
static ssize_t g2ddrv_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    return 0;
}
static ssize_t g2ddrv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    return 0;
}

/*
 * Tutorial:
 * IO control is the most widely used system call to driver because it
 * can almost implement all functionalities provided by others.
 */
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int g2ddrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int g2ddrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	IM_RET ret = IM_RET_OK;
	g2d_config_t config;
	IM_INT32 taskId;
	g2ddrv_context_t *g2dDrv = file->private_data;

	IM_INFOMSG((IM_STR("%s(cmd = %d)++"), IM_STR(_IM_FUNC_), cmd));

	switch (cmd) {
		case G2DLIB_IOCTL_CMD_SETCFG:
			copy_from_user((void *)&config, (void *)arg, sizeof(config));
			ret = g2dkui_set_config(g2dDrv->g2dInst, &config);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("@g2ddrv_ioctl(), g2dkui_set_config() failed!")));
				return -EFAULT;
			}
			break;
		case G2DLIB_IOCTL_CMD_DO:
			__get_user(taskId, (IM_INT32 *)arg);
			ret = g2dkui_do(g2dDrv->g2dInst, taskId);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("@g2ddrv_ioctl(), g2dkui_do() failed!")));
				return -EFAULT;
			}
			break;
		case G2DLIB_IOCTL_CMD_GETID:
			taskId = g2dDrv->taskId;
			__put_user(taskId, (IM_INT32 *)arg);
			break;
		default:
			IM_ERRMSG((IM_STR("@g2ddrv_ioctl(), unknown ioctl command!")));
			return -EFAULT;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return 0;
}

/*
 * Tutorial:
 * Map private memory to user space, you can define the operation of 
 * as you wish.
 */
static int g2ddrv_mmap(struct file *file, struct vm_area_struct *vma)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    /* Nocache. */
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    return remap_pfn_range(vma, /*vma->vm_start*/0x24000000, vma->vm_pgoff,
            /*size*/0x1000, vma->vm_page_prot);
}


/*
 * Tutorial:
 * Poll is most used for user space application to wait for a kernel
 * driver event, like interrupt or harware mission completion.
 */
static unsigned int g2ddrv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	IM_RET ret = IM_RET_OK;
	g2ddrv_context_t *g2dDrv = file->private_data;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	g2dDrv->taskId = -1;
	ret = g2dkui_wait(g2dDrv->g2dInst, &g2dDrv->taskId);
	if(ret == IM_RET_TIMEOUT)
	{
		IM_ERRMSG((IM_STR("@g2ddrv_poll(), g2dkui_wait() timeout!")));
		mask = 0;
	}
	else if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2ddrv_poll(), g2dkui_wait() failed!")));
		mask = POLLERR;
	}else{
		mask = POLLIN | POLLRDNORM;
	}
	IM_INFOMSG((IM_STR("%s(taskId=%d)--"), IM_STR(_IM_FUNC_), g2dDrv->taskId));
	return mask;
}



/*
 * Tutorial:
 * Platform device driver provides suspend/resume support.
 */
static struct platform_driver g2ddrv_plat = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "imapx-g2d",
    },
    .probe = g2ddrv_probe,
    .remove = g2ddrv_remove,
    .suspend = g2ddrv_suspend,
    .resume = g2ddrv_resume,
};
static int g2ddrv_probe(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	/* initualize open count */
	g2ddrv_open_count = 0;
	mutex_init(&g2ddrv_lock);
	return 0;
}

static int g2ddrv_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_destroy(&g2ddrv_lock);
	return 0;
}

static int g2ddrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	g2dkui_suspend();
	g2ddrv_module_disable();
	return 0;
}
static int g2ddrv_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s(g2ddrv_open_count=%d)"), IM_STR(_IM_FUNC_), g2ddrv_open_count));
	if(g2ddrv_open_count > 0)
	{
		//g2d module enable
		g2ddrv_module_enable();
		g2dkui_resume();
	}
	return 0;
}

void g2ddrv_module_enable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	module_power_on(SYSMGR_G2D_BASE);

	// module reset
	//g2ddrv_module_reset();
}

void g2ddrv_module_disable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	module_power_down(SYSMGR_G2D_BASE);
}

void g2ddrv_module_reset(void)
{
	module_reset(SYSMGR_G2D_BASE, 1);
}

/*
 * Tutorial:
 * An init function with prefix *__init* and called with *module_init*
 * represents it's a kernel module entrance. This function is called 
 * at kernel booting, while the corresponding function with *__exit*
 * prefix is called at current module removed from kernel.
 * If current kernel module is compiled into kernel image, the exit
 * api might never be called. BUT, DON'T ignore it's importance cuz
 * when your module is compiled to be a dynamic one(*.ko), the exit
 * will be called at removing module by command *rmmod xxx*.
 */
static int __init g2ddrv_init(void)
{
    int ret = 0;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    /* Register platform device driver. */
    ret = platform_driver_register(&g2ddrv_plat);
    if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR("@g2ddrv_init(), Register g2d platform device driver error = %d."), ret));
        return ret;
    }

    /* Register misc device driver. */
    ret = misc_register(&g2ddrv_miscdev);
    if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR("@g2ddrv_init(), Register g2d misc device driver error = %d."), ret));
        return ret;
    }

    /* TODO */
    return 0;
}
static void __exit g2ddrv_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    /* TODO */

    misc_deregister(&g2ddrv_miscdev);
    platform_driver_unregister(&g2ddrv_plat);
}
module_init(g2ddrv_init);
module_exit(g2ddrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arsor of InfoTM");
MODULE_DESCRIPTION("Imapx800 G2D device driver");
