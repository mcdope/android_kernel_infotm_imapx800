/*
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
 * Revision History:
 *  v1.0.2	leo@2012/06/05: tidy up the code.
 *  v1.0.3	leo@2012/06/18: add suspend&resume support.
 *  		leo@2012/07/05: add utlz support.
 */

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>    
#include <linux/init.h>      
#include <linux/string.h>
#include <linux/mm.h>             
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/semaphore.h>
#include "venc.h"

#include <mach/pad.h>
#include <mach/io.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/items.h>

#include <InfotmMedia.h>
#include <IM_utlzapi.h>
#include <utlz_lib.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"VENCDRV_I:"
#define WARNHEAD	"VENCDRV_W:"
#define ERRHEAD		"VENCDRV_E:"
#define TIPHEAD		"VENCDRV_T:"


static int venc_open_count; 
static struct mutex venc_lock;
static struct semaphore venc_sema;
static venc_param_t venc_param;		/* global variables structure */
/* for poll system call */
static wait_queue_head_t wait_encode;
static volatile unsigned int encode_poll_mark = 0;

typedef struct{
 	bool reservedHW;
}venc_instance_t;


/* 
 * Function declaration.
 */
static int venc_open(struct inode *inode, struct file *file);
static int venc_release(struct inode *inode, struct file *file);
static long venc_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int venc_mmap(struct file *file, struct vm_area_struct *vma);
static unsigned int venc_poll(struct file *file, poll_table *wait);
static int venc_probe(struct platform_device *pdev);
static int venc_remove(struct platform_device *pdev);
static int venc_suspend(struct platform_device *pdev, pm_message_t state);
static int venc_resume(struct platform_device *pdev);
static void venc_module_enable(void);
static void venc_module_disable(void);
static void venc_module_reset(void);

//
// DVFS
//
#ifdef CONFIG_VENC_DVFS_SUPPORT
extern utlz_handle_t utlzdrv_register_module(utlz_module_t *um);
extern IM_RET utlzdrv_unregister_module(utlz_handle_t hdl);
extern IM_RET utlzdrv_notify(utlz_handle_t hdl, IM_INT32 code);

typedef struct{
	int	coeffi;		// N/256
	int	up_wnd;
	int	down_wnd;

	long	clkfreq;	// Khz
}dvfs_clock_table_t;

static dvfs_clock_table_t gDvfsClkTbl[] = 
{
	{60, 60, 20, 70000},
	{90, 10, 20, 100000},
	{120, 10, 20, 120000},
	{150, 10, 20, 150000},
	{180, 10, 20, 180000},
	{210, 10, 20, 200000},
	{240, 10, 26, 250000},
};
#define DVFS_LEVEL_MAX	(sizeof(gDvfsClkTbl) / sizeof(gDvfsClkTbl[0]) - 1)
#define DVFS_LEVEL_MIN	(0)

static utlz_handle_t gUtlzHdl = NULL;
static int gDvfsLevel;

inline static IM_RET dvfs_set_level(IM_INT32 lx)
{
	long rate;
	IM_INFOMSG((IM_STR("%s(lx=%d)"), _IM_FUNC_, lx));

	rate = clk_set_rate(venc_param.busClock, gDvfsClkTbl[lx].clkfreq);
	if(rate < 0){
		IM_ERRMSG((IM_STR("clk_set_rate(%ldKhz) failed)"), gDvfsClkTbl[lx].clkfreq));
		return IM_RET_FAILED;
	}
		
	// record the actual clock for this level.
	gDvfsClkTbl[lx].clkfreq = rate;
	gDvfsLevel = lx;	
	
	IM_TIPMSG((IM_STR("level=%d, rate=%ldKhz"), gDvfsLevel, gDvfsClkTbl[lx].clkfreq));

	return IM_RET_OK;
}

static void venc_utlz_listener(int coeffi)
{
	int level;
	IM_INFOMSG((IM_STR("%s(coeffi=%d)"), _IM_FUNC_, coeffi));
	IM_TIPMSG((IM_STR("%s(coeffi=%d)"), _IM_FUNC_, coeffi));

	if(coeffi > gDvfsClkTbl[gDvfsLevel].coeffi + gDvfsClkTbl[gDvfsLevel].up_wnd){
		level = gDvfsLevel;
		if(level < DVFS_LEVEL_MAX){	// fast up.
			dvfs_set_level(DVFS_LEVEL_MAX);
		}
		else if (level == DVFS_LEVEL_MAX){
			IM_TIPMSG((IM_STR("fslevel is already the max value")));
		}
	}else if((coeffi < gDvfsClkTbl[gDvfsLevel].coeffi - gDvfsClkTbl[gDvfsLevel].down_wnd) && (gDvfsLevel > DVFS_LEVEL_MIN)){
		dvfs_set_level(gDvfsLevel - 1);	// smooth down.
	}
}

static utlz_module_t gUtlzModule = {
	.name = "venc",
	.period = 1,
	.callback = venc_utlz_listener
};

static IM_RET dvfs_init(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	gUtlzHdl = utlzdrv_register_module(&gUtlzModule);
	if(gUtlzHdl == IM_NULL){
		IM_ERRMSG((IM_STR("utlzdrv_register_module() failed")));
		return IM_RET_FAILED;
	}

	dvfs_set_level(DVFS_LEVEL_MAX);
	return IM_RET_OK;
}

static IM_RET dvfs_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(gUtlzHdl){
		utlzdrv_unregister_module(gUtlzHdl);
		gUtlzHdl = IM_NULL;
	}
	return IM_RET_OK;
}
#endif



/***********************************************************************************/
static struct file_operations venc_fops = {
    .owner = THIS_MODULE,
    .open = venc_open,
    .release = venc_release,
    .unlocked_ioctl = venc_ioctl,
    .mmap = venc_mmap,
    .poll = venc_poll,
};
static struct miscdevice venc_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &venc_fops,
    .name = "venc",
};

static irqreturn_t venc_irq_handle(int irq, void *dev_id)
{
	venc_param_t *dev = (venc_param_t *)dev_id;
	unsigned int irq_status = 0;

	/* get interrupt register status */
	irq_status = readl(dev->reg_base_virt_addr + 0x04);

	if (irq_status & 0x01) {
		writel(irq_status & (~0x01), dev->reg_base_virt_addr + 0x04);

		encode_poll_mark = 1;
		wake_up(&wait_encode);
		return IRQ_HANDLED;
	} else {
		IM_ERRMSG((IM_STR("venc interrupt has a error!")));
		return IRQ_HANDLED;
	}
}

static int venc_open(struct inode *inode, struct file *file)
{
	venc_instance_t *inst = (venc_instance_t *)kmalloc(sizeof(venc_instance_t), GFP_KERNEL);
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(inst == IM_NULL){
		IM_ERRMSG((IM_STR("kmalloc(venc_instance_t) failed")));
		return -EFAULT;
	}
	memset((void *)inst, 0, sizeof(venc_instance_t));

	//
	mutex_lock(&venc_lock);
	if(venc_open_count == 0){
		venc_module_enable();
	}
	venc_open_count++;
	mutex_unlock(&venc_lock);
	
	file->private_data = (void *)inst;

	return 0;
}

static int venc_release(struct inode *inode, struct file *file)
{
	venc_instance_t *inst = (venc_instance_t *)file->private_data;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(inst->reservedHW == true){
		up(&venc_sema);
		inst->reservedHW = false;
	}
	kfree((void *)inst);
	
	mutex_lock(&venc_lock);
	venc_open_count--;
	if(venc_open_count == 0){
		venc_module_disable();
	}
	mutex_unlock(&venc_lock);
	
	return 0;
}

static long venc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	venc_instance_t *inst = (venc_instance_t *)file->private_data;
	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

	switch (cmd) {
	case HX280ENC_IOCGHWOFFSET:
		__put_user(venc_param.reg_base_phys_addr, (unsigned int *)arg);
		break;
	case HX280ENC_IOCGHWIOSIZE:
		__put_user(VENC_ACT_REG_SIZE, (unsigned int *)arg);
		break;
	case HX280ENC_IOC_RESERVE_HW:
		down_interruptible(&venc_sema);
		inst->reservedHW = true;	
#ifdef CONFIG_VENC_DVFS_SUPPORT
		if(gUtlzHdl != IM_NULL){
			utlzdrv_notify(gUtlzHdl, UTLZ_NOTIFY_CODE_RUN);
		}
#endif	
		break;
	case HX280ENC_IOC_RELEASE_HW:
		up(&venc_sema);
		inst->reservedHW = false;
#ifdef CONFIG_VENC_DVFS_SUPPORT
		if(gUtlzHdl != IM_NULL){
			utlzdrv_notify(gUtlzHdl, UTLZ_NOTIFY_CODE_STOP);
		}
#endif	
		break;
	default:
		IM_ERRMSG((IM_STR("unknown cmd 0x%x"), cmd));
		return -EFAULT;
	}

	return 0;
}

static int venc_mmap(struct file *file, struct vm_area_struct *vma)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(unlikely(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot))){
		return -EAGAIN;
	}

	return 0;
}

static unsigned int venc_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	poll_wait(file, &wait_encode, wait);

	if (encode_poll_mark != 0) {
		mask = POLLIN | POLLRDNORM;
		encode_poll_mark = 0;
	}

	return mask;
}

static struct platform_driver venc_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imap-venc",
	},
	.probe = venc_probe,
	.remove = venc_remove,
	.suspend = venc_suspend,
	.resume = venc_resume,
};

static int venc_probe(struct platform_device *pdev)
{
	int ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	venc_open_count = 0;
	memset(&venc_param, 0x00, sizeof(venc_param_t));
	init_waitqueue_head(&wait_encode);
	mutex_init(&venc_lock);
	sema_init(&venc_sema, 1);

	venc_param.reg_reserved_size = VENC_ACT_REG_SIZE;
	venc_param.reg_base_phys_addr = VENC_ACT_REG_BASE;
	venc_param.reg_base_virt_addr = ioremap_nocache(VENC_ACT_REG_BASE, VENC_ACT_REG_SIZE);
	if(venc_param.reg_base_virt_addr == NULL){
		IM_ERRMSG((IM_STR("ioremap_nocache() failed!")));
		return -EINVAL;
	}

	// request for irq.
	ret = request_irq(VENC_ACT_INTR_ID, venc_irq_handle, IRQF_DISABLED, pdev->name, (void *)(&venc_param));
	if (ret) {
		IM_ERRMSG((IM_STR("request_irq() failed!")));
		return ret;
	}

	// bus clock.
	venc_param.busClock = clk_get(&pdev->dev, "venc");
	if(venc_param.busClock == NULL){
		IM_ERRMSG((IM_STR("clk_get() failed!")));
		return -EFAULT;	
	}

	//
	venc_param.dvfsEna = false;
#ifdef CONFIG_VENC_DVFS_SUPPORT
	if(item_exist(DVFS_ENABLE_ITEM) == 1){
		if(item_integer(DVFS_ENABLE_ITEM, 0) == 1){
			venc_param.dvfsEna = true;
			IM_INFOMSG((IM_STR("venc DVFS enabled")));
		}
	}
#endif

	//
	venc_module_disable();

	return 0;
}

static int venc_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	iounmap((void *)(venc_param.reg_base_virt_addr));

	/* release irq */
	free_irq(VENC_ACT_INTR_ID, pdev);

	mutex_destroy(&venc_lock);
	
	return 0;
}

static int venc_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(venc_open_count > 0){
		venc_module_disable();
	}
	return 0;
}

static int venc_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s(venc_open_count=%d)"), IM_STR(_IM_FUNC_), venc_open_count));
	mutex_lock(&venc_lock);
	if(venc_open_count > 0){
		venc_module_enable();
	}
	mutex_unlock(&venc_lock);

	return 0;
}

void venc_module_enable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if(clk_enable(venc_param.busClock) != 0){
		IM_ERRMSG((IM_STR("clk_enable() failed")));
	}

	module_power_on(SYSMGR_VENC_BASE);
	venc_module_reset();

	//
#ifdef CONFIG_VENC_DVFS_SUPPORT
	if(venc_param.dvfsEna){
		if(dvfs_init() != IM_RET_OK){
			IM_ERRMSG((IM_STR("dvfs_init() failed")));
		}
	}
#endif
}

void venc_module_disable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
#ifdef CONFIG_VENC_DVFS_SUPPORT
	if(venc_param.dvfsEna){
		if(dvfs_deinit() != IM_RET_OK){
			IM_ERRMSG((IM_STR("dvfs_deinit() failed")));
		}
	}
#endif

	module_power_down(SYSMGR_VENC_BASE);
	
	//
	clk_disable(venc_param.busClock);
}

void venc_module_reset(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	module_reset(SYSMGR_VENC_BASE, 1);	// 0--bus reset, 1--venc reset.
}

static int __init venc_init(void)
{
	int ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* Register platform device driver. */
	ret = platform_driver_register(&venc_plat);
	if (unlikely(ret != 0)) {
		printk(KERN_ALERT "Register venc platform device driver error, %d.\n", ret);
		return ret;
	}

	/* Register misc device driver. */
	ret = misc_register(&venc_miscdev);
	if (unlikely(ret != 0)) {
		printk(KERN_ALERT "Register venc misc device driver error, %d.\n", ret);
		return ret;
	}

	return 0;
}


static void __exit venc_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	misc_deregister(&venc_miscdev);
	platform_driver_unregister(&venc_plat);
}
module_init(venc_init);
module_exit(venc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rane of InfoTM");

