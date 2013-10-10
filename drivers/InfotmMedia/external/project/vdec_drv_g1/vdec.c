/*
 * vdec.c
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
 * IMAPX video dec driver process.
 *
 * Author:
 *	ayakashi<eric.xu@infotmic.com.cn>.
 *
 * Revision History:
 * 1.0.0   04/05/2012  Created for kernel3.0.8 by ayakashi.
 * 1.0.1   04/14/2012 Released by ayakashi .
 * 1.0.2   05/03/2012 This version support mmu
 * 1.0.3   06/05/2012 The latest stable version,up sema if it needed when release. 
 * 1.0.4   06/29/2012 workaround for mmu disable failed.
 * 1.0.5   07/03/2012 add mutex for mmu resource
 * 1.0.6   07/09/2012 add  devfs
 * 1.0.7   07/12/2012 add ioctl for rest mmu
 * 1.0.8   09/02/2012 optimize for dvfs
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
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/items.h>

#include <InfotmMedia.h>
#include <IM_utlzapi.h>
#include <utlz_lib.h>
#include "vdec.h"

#define INFO	0
#define ERR		1
#define TIP		1

#define DEBUG
#ifdef DEBUG
#define DBGMSG(on, msg, ...)	if(on)	printk(KERN_ALERT msg, ##__VA_ARGS__)
#else
#define DBGMSG(on, msg, ...)
#endif

/* 
 * Function declaration.
 */
static int vdec_open(struct inode *inode, struct file *file);
static int vdec_release(struct inode *inode, struct file *file);
static ssize_t vdec_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
static ssize_t vdec_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int vdec_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long vdec_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif  
static int vdec_mmap(struct file *file, struct vm_area_struct *vma);
static unsigned int vdec_poll(struct file *file, poll_table *wait);
static int vdec_probe(struct platform_device *pdev);
static int vdec_remove(struct platform_device *pdev);
static int vdec_suspend(struct platform_device *pdev, pm_message_t state);
static int vdec_resume(struct platform_device *pdev);
static void vdec_module_enable(void);
static void vdec_module_disable(void);
static void vdec_module_reset(void);
static int vdec_mmu_init(pmm_handle_t *phandle);
static int vdec_mmu_deinit(pmm_handle_t phandle);
static int vdec_mmu_enable(pmm_handle_t phandle);
static int vdec_mmu_disable(pmm_handle_t phandle);
#define USE_MMU

/*
 * this structure include global varaibles
 */
vdec_param_t vdec_param;			/* global variables group */
static unsigned int vdec_open_count;		/* record open syscall times */
static struct mutex vdec_lock;		/* mainly lock for decode instance count */
#ifdef CONFIG_VDEC_POLL_MODE
static wait_queue_head_t wait_vdec;		/* a wait queue for poll system call */
static volatile unsigned int dec_poll_mark = 0;
static volatile unsigned int pp_poll_mark = 0;
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

//extern func used 
extern IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner);
extern IM_RET pmmdrv_release(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid);
extern IM_RET pmmdrv_dmmu_deinit(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_reset(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_enable(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_disable(IN pmm_handle_t handle);

//
// DVFS
//
#ifdef CONFIG_VDEC_DVFS_SUPPORT
extern utlz_handle_t utlzdrv_register_module(utlz_module_t *um);
extern IM_RET utlzdrv_unregister_module(utlz_handle_t hdl);
extern IM_RET utlzdrv_notify(utlz_handle_t hdl, IM_INT32 code);
static spinlock_t dvfslock;
typedef struct{
    int	coeffi;		// N/256
    int	up_wnd;
    int	down_wnd;

    long	clkfreq;	// Khz
}dvfs_clock_table_t;

static dvfs_clock_table_t gDvfsClkTbl[] = 
{
    //	{40, 10, 10, 50000},
    {20, 20, 20, 37000},
    {65, 15, 15, 55500},
    {95, 15, 15, 74000},
    {120, 10, 10, 98800},
    {140, 10, 10, 126000},
    {160, 10, 10, 148000},
    {180, 10, 10, 178000},
    {205, 15, 15, 222000},
    {240, 15, 20, 296000},
};

#define DVFS_LEVEL_MAX	(sizeof(gDvfsClkTbl) / sizeof(gDvfsClkTbl[0]) - 1)
#define DVFS_LEVEL_MIN	(0)
static utlz_handle_t gUtlzHdl = NULL;
static int  gDvfsLevel;
static int gDvfsRequestLevel;
static int gVdecDvfsCount;
static int extern_ctrl_dvfs;
extern void imapx800_freq_message(int module, int freq);
void imapx800_vpufreq_setlvl(int level)
{
    unsigned long flags;
    spin_lock_irqsave(&dvfslock, flags);
    if(level < 0)
    {
        extern_ctrl_dvfs = 0;
    }
    else
    {
        extern_ctrl_dvfs = 1;
        gDvfsRequestLevel = level > DVFS_LEVEL_MAX ? DVFS_LEVEL_MAX : level;
        imapx800_freq_message(2, gDvfsClkTbl[gDvfsRequestLevel].clkfreq / 1000);
    }
    spin_unlock_irqrestore(&dvfslock, flags);
    return;
}
EXPORT_SYMBOL(imapx800_vpufreq_setlvl);

int imapx800_vpufreq_getval(void)
{
    unsigned long flags;
    int hz;
    spin_lock_irqsave(&dvfslock, flags);
    if(extern_ctrl_dvfs)
    {
        if(gDvfsRequestLevel != -1)
            hz = gDvfsClkTbl[gDvfsRequestLevel].clkfreq / 1000;
        else
            hz = 0;
    }
    else
    {
        if(gDvfsLevel != -1)
            hz = gDvfsClkTbl[gDvfsLevel].clkfreq / 1000;
        else
            hz = 0;	
    }
    spin_unlock_irqrestore(&dvfslock, flags);
    return hz;
}
EXPORT_SYMBOL(imapx800_vpufreq_getval);

inline static IM_RET dvfs_set_level(IM_INT32 lx)
{
    long rate;
    DBGMSG(INFO,"%s(lx=%d)\n", _IM_FUNC_, lx);

    rate = clk_set_rate(vdec_param.busClock, gDvfsClkTbl[lx].clkfreq);
    if(rate < 0){
        DBGMSG(ERR, "clk_set_rate(%ldKhz) failed\n", gDvfsClkTbl[lx].clkfreq);
        return IM_RET_FAILED;
    }

    // record the actual clock for this level.
    gDvfsClkTbl[lx].clkfreq = rate;
    gDvfsLevel = lx;

    //printk(KERN_DEBUG "vpuf-->%ldM \n", rate/1000);
    imapx800_freq_message(2, rate/1000);

    return IM_RET_OK;
}

static void vdec_utlz_listener(int coeffi)
{
    int level;
    unsigned long flags;
    DBGMSG(INFO,"%s(coeffi=%d)\n", _IM_FUNC_, coeffi);
    spin_lock_irqsave(&dvfslock, flags);
    if(extern_ctrl_dvfs == 0)
    {
        if(coeffi == 0)
        {
            if(gDvfsLevel != DVFS_LEVEL_MIN)
            {
                gDvfsRequestLevel = DVFS_LEVEL_MAX;
                dvfs_set_level(DVFS_LEVEL_MIN);
            }
        }else if(coeffi > gDvfsClkTbl[gDvfsLevel].coeffi + gDvfsClkTbl[gDvfsLevel].up_wnd)
        {
            level = gDvfsLevel;
            while(level < DVFS_LEVEL_MAX)
            {	// fast up.
                if(gDvfsClkTbl[level].coeffi + gDvfsClkTbl[level].up_wnd >= coeffi)
                {
                    break;
                }
                level++;
            }	

            if(level > DVFS_LEVEL_MAX)
            {
                level = DVFS_LEVEL_MAX;
            }
            if(level > gDvfsLevel)
            {
                gDvfsRequestLevel = level;
            }
        }
        else if((coeffi < gDvfsClkTbl[gDvfsLevel].coeffi - gDvfsClkTbl[gDvfsLevel].down_wnd) && gDvfsLevel > DVFS_LEVEL_MIN)
        {
            gDvfsRequestLevel = gDvfsLevel - 1;
        }
    }
    spin_unlock_irqrestore(&dvfslock, flags);
}

static utlz_module_t gUtlzModule = {
    .name = "vdec",
    .period = 1,
    .callback = vdec_utlz_listener
};

static IM_RET dvfs_init(void)
{
    gUtlzHdl = utlzdrv_register_module(&gUtlzModule);
    if(gUtlzHdl == IM_NULL){
        DBGMSG(ERR, "utlzdrv_register_module() failed\n");
        return IM_RET_FAILED;
    }
    gDvfsRequestLevel = DVFS_LEVEL_MAX;
    dvfs_set_level(DVFS_LEVEL_MAX);
    return IM_RET_OK;
}

static IM_RET dvfs_deinit(void)
{
    if(gUtlzHdl){
        utlzdrv_unregister_module(gUtlzHdl);
        gUtlzHdl = IM_NULL;
    }
    gDvfsRequestLevel = -1;
    gDvfsLevel = -1;
    return IM_RET_OK;
}
#endif


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
static struct file_operations vdec_fops = {
    .owner = THIS_MODULE,
    .open = vdec_open,
    .release = vdec_release,
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = vdec_ioctl,
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
    .unlocked_ioctl = vdec_ioctl,
#endif
    .read = vdec_read,
    .write = vdec_write,
    .mmap = vdec_mmap,
    .poll = vdec_poll,
};
static struct miscdevice vdec_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &vdec_fops,
    .name = "vdec",
};


/* 
 * This irq handle function should never be reexecute at the same
 * time. Pp and decode share the same interrupt signal thread, in 
 * pp external mode, a finished interruption refer to be a pp irq, 
 * but in pp pipeline mode(decode pipeline with decode), recieved 
 * irq refers to be a decode interruption.
 * In this driver, we use System V signal asynchronization thread
 * communication. So you should set System V in you kernel make 
 * menuconfig.
 * ATTENTION: if your application runs in multi-thread mode, be 
 * aware that if you set your application getting this signal
 * as process mode, the signal will be send to your application's
 * main thread, and signal will never be processed twice. So in 
 * this case you won't get anything in your derived thread.
 */
static irqreturn_t vdec_irq_handle(int irq, void *dev_id)
{
    unsigned int handled;
    unsigned int irq_status_dec;
    unsigned int irq_status_pp;
    vdec_param_t *dev;

    handled = 0;
    dev = (vdec_param_t *)dev_id;

    /* interrupt status register read */
    irq_status_dec = readl(dev->reg_base_virt_addr + VDEC_IRQ_STAT_DEC * 4);
    irq_status_pp = readl(dev->reg_base_virt_addr + VDEC_IRQ_STAT_PP * 4);

    /* this structure is to enable irq in irq */
    if((irq_status_dec & VDEC_IRQ_BIT_DEC) || (irq_status_pp & VDEC_IRQ_BIT_PP))
    {
        if(irq_status_dec & VDEC_IRQ_BIT_DEC)
        {
#ifdef CONFIG_VDEC_HW_PERFORMANCE
            do_gettimeofday(&(dev->end_time));
#endif
            /* clear irq */
            writel(irq_status_dec & (~VDEC_IRQ_BIT_DEC), \
                    dev->reg_base_virt_addr + VDEC_IRQ_STAT_DEC * 4);

#ifdef CONFIG_VDEC_SIGNAL_MODE
            /* fasync kill for decode instances to send signal */
            if(dev->async_queue_dec != NULL)
                kill_fasync(&(dev->async_queue_dec), SIGIO, POLL_IN);
            else
                DBGMSG(ERR,"IMAPX vdec received w/o anybody waiting for it\n");
#endif	/* CONFIG_VDEC_SIGNAL_MODE */

#ifdef CONFIG_VDEC_POLL_MODE
            dec_poll_mark = 1;
            wake_up(&wait_vdec);
#endif	/* CONFIG_VDEC_POLL_MODE */

            DBGMSG(INFO,"IMAPX vdec get dec irq\n");
        }

        /* In pp pipeline mode, only one decode interrupt will be set */
        if(irq_status_pp & VDEC_IRQ_BIT_PP)
        {
#ifdef CONFIG_VDEC_HW_PERFORMANCE
            do_gettimeofday(&(dev->end_time));
#endif
            /* clear irq */
            writel(irq_status_dec & (~VDEC_IRQ_BIT_PP), \
                    dev->reg_base_virt_addr + VDEC_IRQ_STAT_PP * 4);

#ifdef CONFIG_VDEC_SIGNAL_MODE
            /* fasync kill for pp instance */
            if(dev->async_queue_pp != NULL)
                kill_fasync(&dev->async_queue_pp, SIGIO, POLL_IN);
            else
                DBGMSG(ERR,"IMAPX vdec pp received w/o anybody waiting for it\n");
#endif	/* CONFIG_VDEC_SIGNAL_MODE */

#ifdef CONFIG_VDEC_POLL_MODE
            pp_poll_mark = 1;
            wake_up(&wait_vdec);
#endif	/* CONFIG_VDEC_POLL_MODE */

            DBGMSG(INFO,"IMAPX vdec get pp irq\n");
        }

        handled = 1;
    }
    else
        DBGMSG(ERR,"IMAPX vdec Driver get an unknown irq\n");

    return IRQ_RETVAL(handled);
}

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
/*
 * File operations, this system call should be called before
 * any interrupt occurs. You can call fcntl system call to set 
 * driver node file property in order to get asynchronization
 * signal.
 */
static int vdec_fasync(int fd, struct file *file, int mode)
{
    vdec_param_t *dev;
    struct fasync_struct **async_queue;

    dev = &vdec_param;
    async_queue = NULL;

    /* select whitch interrupt this instance will sign up for */
    if((unsigned int *)(file->private_data) == &(vdec_param.dec_instance))
    {
        DBGMSG(INFO,"IMAPX vdec fasync called\n");
        async_queue = &(vdec_param.async_queue_dec);
    }
    else if((unsigned int *)(file->private_data) == &(vdec_param.pp_instance))
    {
        DBGMSG(INFO,"IMAPX vdec pp fasync called\n");
        async_queue = &(vdec_param.async_queue_pp);
    }
    else
        DBGMSG(ERR,"IMAPX vdec wrong fasync called\n");

    return fasync_helper(fd, file, mode, async_queue);
}
#endif	/* CONFIG_VDEC_SIGNAL_MODE */

/*
 * Tutorial:
 * Each open() system call returns a file descriptor if success. And as a
 * user space application, open() is a must step to access further kernel
 * module interfaces.
 */
static int vdec_open(struct inode *inode, struct file *file)
{
    DBGMSG(INFO, "%s(vdec_open_count=%d)\n", __func__, vdec_open_count);

    mutex_lock(&vdec_lock);
    if(vdec_open_count == 0)
    {
#ifdef CONFIG_VDEC_DVFS_SUPPORT
        gVdecDvfsCount = 0;
        gDvfsLevel = -1;
        gDvfsRequestLevel = -1;
#endif

        vdec_module_enable();
#ifdef USE_MMU
        if(vdec_mmu_init(&(vdec_param.pmm_handle)) == 0)
        {
            vdec_param.mmu_inited = 1;
            vdec_param.mmu_state = 	VDEC_MMU_OFF;
            vdec_param.wait_mmu = 0;
        }
#endif 
    }
    vdec_open_count++;
    mutex_unlock(&vdec_lock);

    /* dec instance by default, you can change it by ioctl pp instance */
    file->private_data = &(vdec_param.dec_instance);


    DBGMSG(INFO,"IMAPX vdec open OK\n");

    return VDEC_RET_OK;
}

/*
 * Tutorial:
 * After open() a driver node, the file descriptor must be closed if you
 * don't use it anymore.
 */
static int vdec_release(struct inode *inode, struct file *file)
{
    DBGMSG(INFO, "%s(vdec_open_count=%d)\n", __func__, vdec_open_count);

#ifdef CONFIG_VDEC_SIGNAL_MODE
    /* reset decode driver node file property */
    if(file->f_flags & FASYNC)
        vdec_fasync(-1, file, 0);
#endif	/* CONFIG_VDEC_SIGNAL_MODE */

    mutex_lock(&vdec_lock);
    if(vdec_param.dec_instance == (unsigned  int )file || vdec_param.pp_instance == (unsigned  int )file )
    {
        DBGMSG(ERR, "application data abort, vdec drv release the source\n");
        if(file->private_data == &(vdec_param.dec_instance))
        {
            vdec_param.dec_instance = 0;
            up(&vdec_param.dec_sema);
        }
        else if(file->private_data == &(vdec_param.pp_instance))
        {
            vdec_param.pp_instance = 0;
            up(&vdec_param.pp_sema);
        }
    }

    if(vdec_param.wait_mmu > 0 && vdec_param.wait_mmu_instance == (unsigned int)file)
    {
        DBGMSG(ERR, "application data abort, vdec drv release mmu source\n");
        up(&vdec_param.mmu_sema);
        vdec_param.wait_mmu--;

    }
    vdec_open_count--;

    if(vdec_open_count == 0)
    {
#ifdef USE_MMU
        if(vdec_param.mmu_state == VDEC_MMU_ON)
        {
            vdec_mmu_disable(vdec_param.pmm_handle);
            vdec_param.mmu_state = 	VDEC_MMU_OFF;
        }
        if(vdec_mmu_deinit(vdec_param.pmm_handle) == 0)
        {
            vdec_param.mmu_inited = 0;
            vdec_param.pmm_handle = NULL;
        }
#endif 
        vdec_module_disable();
    }
    mutex_unlock(&vdec_lock);

    DBGMSG(INFO,"IMAPX vdec release OK\n");
    return VDEC_RET_OK;
}

/*
 * Tutorial:
 * Normally, the *read* and *write* apis are not widely used because
 * most driver use *mmap* to map a kernel memory region to user space
 * in order to avoid memory copy. 
 * And something special with read & write is the return value is the
 * actual accessed data length.
 */
static ssize_t vdec_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    DBGMSG(INFO, "%s()\n", __func__);
    return VDEC_RET_OK;
}

static ssize_t vdec_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    DBGMSG(INFO, "%s()\n", __func__);
    return VDEC_RET_OK;
}

/*
 * Tutorial:
 * IO control is the most widely used system call to driver because it
 * can almost implement all functionalities provided by others.
 */
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6 )
    static int vdec_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long vdec_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
    int ret;
    int mmu_enable;

#ifdef CONFIG_VDEC_HW_PERFORMANCE
    int out;	/* for copy from/to user */
    struct timeval end_time_arg;
    out = -1;
#endif
    //DBGMSG(INFO, "%s()\n", __func__);

    ret = 0;
    mmu_enable = 0;
    /* cmd check */
    if(_IOC_TYPE(cmd) != VDEC_MAGIC)
        return -ENOTTY;
    if(_IOC_NR(cmd) > VDEC_MAX_CMD)
        return -ENOTTY;

    /* check command by command feature READ/WRITE */
    if(_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd) & _IOC_WRITE)
        ret = !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));
    if(ret)
        return -EFAULT;

    switch(cmd)
    {
        case VDEC_IRQ_DISABLE:
            disable_irq(IRQ_VDEC);
            break;

        case VDEC_IRQ_ENABLE:
            enable_irq(IRQ_VDEC);
            break;

            /* we should return a physics address to application level */
        case VDEC_REG_BASE:
            __put_user(vdec_param.reg_base_phys_addr, (unsigned long *)arg);
            break;

            /* this is 101 * 4 by default */
        case VDEC_REG_SIZE:
            __put_user(VDEC_ACT_REG_SIZE, (unsigned int *)arg);
            break;

        case VDEC_PP_INSTANCE:
            file->private_data = &(vdec_param.pp_instance);
            break;
#ifdef CONFIG_VDEC_HW_PERFORMANCE	/* this ioctl command call is for hardware decode performance detect */
        case VDEC_HW_PERFORMANCE:
            end_time_arg.tv_sec	= vdec_param.end_time.tv_sec;
            end_time_arg.tv_usec	= vdec_param.end_time.tv_usec;

            out = copy_to_user((struct timeval *)arg, &end_time_arg, sizeof(struct timeval));
            break;
#endif
        case VDEC_INSTANCE_LOCK:
#ifdef USE_MMU
            __get_user(mmu_enable, (int *)arg);
            mutex_lock(&vdec_lock);
            if((vdec_param.mmu_state + mmu_enable == 1) && vdec_param.dec_working)
            {
                vdec_param.wait_mmu++;
                vdec_param.wait_mmu_instance = (unsigned int)file;
                mutex_unlock(&vdec_lock);
                if(down_interruptible(&vdec_param.mmu_sema))
                {
                    return -1;
                }
                mutex_lock(&vdec_lock);
            }
            if(vdec_param.mmu_state == VDEC_MMU_ON)
            {
                if(mmu_enable == 0)
                {
                    if(vdec_mmu_disable(vdec_param.pmm_handle) != 0)
                    {
                        vdec_module_reset();
                        pmmdrv_dmmu_reset(vdec_param.pmm_handle);
                    }
                    vdec_param.mmu_state = VDEC_MMU_OFF;
                }
            }else{
                if(mmu_enable == 1)
                {
                    //vdec_module_reset();
                    //pmmdrv_dmmu_reset(vdec_param.pmm_handle);
                    vdec_mmu_enable(vdec_param.pmm_handle);
                    vdec_param.mmu_state = VDEC_MMU_ON;
                }
            }
            vdec_param.dec_working++;
            mutex_unlock(&vdec_lock);
#endif 
 
            if(file->private_data == &(vdec_param.dec_instance))
            {
                if(down_interruptible(&vdec_param.dec_sema))
                {
                    return -1;
                }
                mutex_lock(&vdec_lock);
                vdec_param.dec_instance = (unsigned int)file;
                mutex_unlock(&vdec_lock);
            }
            else if(file->private_data == &(vdec_param.pp_instance))
            {
                if(down_interruptible(&vdec_param.pp_sema))
                {
                    return -1;
                }
                mutex_lock(&vdec_lock);
                vdec_param.pp_instance = (unsigned int)file;
                mutex_unlock(&vdec_lock);
            }

           break;
        case VDEC_INSTANCE_UNLOCK:
            mutex_lock(&vdec_lock);
            if(file->private_data == &(vdec_param.dec_instance))
            {
                vdec_param.dec_instance = 0;
                up(&vdec_param.dec_sema);
            }
            else if(file->private_data == &(vdec_param.pp_instance))
            {
                vdec_param.pp_instance = 0;
                up(&vdec_param.pp_sema);
            }

            vdec_param.dec_working--;
            if(vdec_param.dec_working == 0 && vdec_param.wait_mmu > 0)
            {
                vdec_param.wait_mmu_instance = 0;
                up(&vdec_param.mmu_sema);
                vdec_param.wait_mmu--;
            }
            mutex_unlock(&vdec_lock);

            break;
        case VDEC_CHECK_MMU_INIT:
            __put_user(vdec_param.mmu_inited, (unsigned int *)arg);
            break;
        case VDEC_RESET:
            vdec_module_reset();
            pmmdrv_dmmu_reset(vdec_param.pmm_handle);
            //vdec_mmu_deinit(vdec_param.pmm_handle);
            //vdec_mmu_init(&vdec_param.pmm_handle);
            mutex_lock(&vdec_lock);
            if(vdec_param.mmu_state == VDEC_MMU_ON)
            {
                vdec_mmu_enable(vdec_param.pmm_handle);
            }
            mutex_unlock(&vdec_lock);
            break;
#ifdef CONFIG_VDEC_DVFS_SUPPORT
        case VDEC_NOTIFY_HW_ENABLE:
            if(gUtlzHdl != IM_NULL)
            {
                unsigned long flags;
                spin_lock_irqsave(&dvfslock, flags);
                if((gDvfsRequestLevel >= 0 || extern_ctrl_dvfs) && gDvfsRequestLevel <= DVFS_LEVEL_MAX && gDvfsRequestLevel != gDvfsLevel)
                {
                    dvfs_set_level(gDvfsRequestLevel);
                    //gDvfsRequestLevel = -1;
                }
                spin_unlock_irqrestore(&dvfslock, flags);
                mutex_lock(&vdec_lock);
                gVdecDvfsCount++;
                if(gVdecDvfsCount == 1)
                {
                    mutex_unlock(&vdec_lock);
                    utlzdrv_notify(gUtlzHdl, UTLZ_NOTIFY_CODE_RUN);
                }
                else
                {
                    mutex_unlock(&vdec_lock);
                }
            }
            break;
        case VDEC_NOTIFY_HW_DISABLE:
            if(gUtlzHdl != IM_NULL){
                mutex_lock(&vdec_lock);
                gVdecDvfsCount--;
                if(gVdecDvfsCount == 0)
                {
                    mutex_unlock(&vdec_lock);
                    utlzdrv_notify(gUtlzHdl, UTLZ_NOTIFY_CODE_STOP);
                }
                else
                {
                    mutex_unlock(&vdec_lock);
                }
            }
            break;

        case VDEC_UPDATE_CLK:
            if(gUtlzHdl != IM_NULL){
                unsigned long flags;
                spin_lock_irqsave(&dvfslock, flags);
                if(extern_ctrl_dvfs == 0)
                {
                    gDvfsRequestLevel = DVFS_LEVEL_MAX;
                }
                spin_unlock_irqrestore(&dvfslock, flags);
            }
            break;
        case VDEC_GETDVFS:
            __put_user(vdec_param.dvfsEna, (unsigned int *)arg);
            break;
#endif
        default:
            DBGMSG(ERR,"IMAPX vdec unknown ioctl command\n");
            return -EFAULT;
    }

    return VDEC_RET_OK;
}

/*
 * Tutorial:
 * Map private memory to user space, you can define the operation of 
 * as you wish.
 */
static int vdec_mmap(struct file *file, struct vm_area_struct *vma)
{
    // size_t size = VDEC_ACT_REG_SIZE;	//vma->vm_end - vma->vm_start
    size_t size = vma->vm_end - vma->vm_start;

    DBGMSG(INFO, "%s(), size= %u, pgoff = %lu\n", __func__, size, vma->vm_pgoff);
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    //return remap_pfn_range(vma, vdec_param.reg_base_phys_addr,vma->vm_pgoff, size, vma->vm_page_prot);
    return remap_pfn_range(vma, vma->vm_start,vma->vm_pgoff, size, vma->vm_page_prot);
}

/*
 * Tutorial:
 * Poll is most used for user space application to wait for a kernel
 * driver event, like interrupt or harware mission completion.
 */

#ifdef CONFIG_VDEC_POLL_MODE
static unsigned int vdec_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;
    //	DBGMSG(INFO, "%s()\n", __func__);

    poll_wait(file, &wait_vdec, wait);

    if(file->private_data == &(vdec_param.dec_instance))
    {
        if(dec_poll_mark != 0)
        {
            DBGMSG(INFO, "dec irq coming\n");
            mask = POLLIN | POLLRDNORM;
            dec_poll_mark = 0;
        }
    }
    else if(file->private_data == &(vdec_param.pp_instance))
    {
        if(pp_poll_mark != 0)
        {
            DBGMSG(INFO, "pp irq coming\n");
            mask = POLLIN | POLLRDNORM;
            pp_poll_mark = 0;
        }
    }
    else
    {
        dec_poll_mark = 0;
        pp_poll_mark = 0;
        DBGMSG(INFO, "wait nothing\n");
        mask = POLLERR;
    }

    return mask;
}
#endif
/*
 * Tutorial:
 * Platform device driver provides suspend/resume support.
 */
static struct platform_driver vdec_plat = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "imap-vdec",
    },
    .probe = vdec_probe,
    .remove = vdec_remove,
    .suspend = vdec_suspend,
    .resume = vdec_resume,
};

static int vdec_probe(struct platform_device *pdev)
{
    int		ret;

    //struct resource	res;
    DBGMSG(INFO, "%s()\n", __func__);

    /* initualize open count */
    vdec_open_count = 0;

    /* initualize instances just for mark */
    vdec_param.dec_instance	= 0;
    vdec_param.pp_instance	= 0;

#ifdef CONFIG_VDEC_POLL_MODE
    /* initualize wait queue */
    init_waitqueue_head(&wait_vdec);
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
    /* initualize async queue */
    vdec_param.async_queue_dec	= NULL;
    vdec_param.async_queue_pp	= NULL;
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

    /* request memory region for registers */
    vdec_param.reg_reserved_size = VDEC_ACT_REG_SIZE;
    vdec_param.resource_mem = NULL;
    vdec_param.reg_base_phys_addr = VDEC_BASE_REG_PA;
    vdec_param.reg_base_virt_addr = ioremap_nocache(VDEC_BASE_REG_PA, VDEC_ACT_REG_SIZE);
    if(vdec_param.reg_base_virt_addr == NULL)
    {
        DBGMSG(ERR,"Fail to ioremap IMAPX vdec register base address\n");
        return -EINVAL;
    }

    /*
     * decoder and pp shared one irq line, so we must register irq in share mode
     */
    ret = request_irq(IRQ_VDEC, vdec_irq_handle, IRQF_DISABLED, pdev->name, (void *)(&vdec_param));
    if(ret)
    {
        DBGMSG(ERR,"Fail to request irq for IMAPX vdec device\n");
        return ret;
    }

    //init vdec lock
    mutex_init(&vdec_lock);

    //init sema for race condition
    sema_init(&vdec_param.dec_sema, 1);
    sema_init(&vdec_param.pp_sema, 1);
    sema_init(&vdec_param.mmu_sema, 0);

    //map vdes base pa addr
    vdec_param.mempool_virt_addr = ioremap_nocache(VDEC_MEMPOOL_BASE_PA, VDEC_MEMPOOL_SIZE);

    // bus clock.
    vdec_param.busClock = clk_get(&pdev->dev, "vdec");
    if(vdec_param.busClock == NULL){
        DBGMSG(ERR, "clk_get(vdec) failed\n");
        return -EFAULT;	
    }
    vdec_module_disable();

    //
    vdec_param.dvfsEna = VDEC_DVFS_OFF;
#ifdef CONFIG_VDEC_DVFS_SUPPORT
    if(item_exist(DVFS_ENABLE_ITEM) == 1 && item_exist(DVFS_VPU_ENABLE_ITEM) == 1){
        if(item_integer(DVFS_ENABLE_ITEM, 0) == 1 && item_integer(DVFS_VPU_ENABLE_ITEM, 0) == 1){
            vdec_param.dvfsEna = VDEC_DVFS_ON;
            DBGMSG(TIP, "vdec DVFS enabled\n");
        }
    }
    extern_ctrl_dvfs = 0;
    gDvfsRequestLevel = -1;
    gDvfsLevel = -1;
    spin_lock_init(&dvfslock);
#endif

    DBGMSG(INFO,"IMAPX vdec Driver probe OK\n");

    return VDEC_RET_OK;
}

static int vdec_remove(struct platform_device *pdev)
{
    DBGMSG(INFO, "%s()\n", __func__);

    /* unmap register base */
    iounmap((void *)(vdec_param.mempool_virt_addr));
    iounmap((void *)(vdec_param.reg_base_virt_addr));

    /* release irq */
    free_irq(IRQ_VDEC, pdev);

    //
    mutex_destroy(&vdec_lock);

    return VDEC_RET_OK;
}

static int vdec_suspend(struct platform_device *pdev, pm_message_t state)
{
    DBGMSG(INFO, "%s(vdec_open_count=%d)\n", __func__, vdec_open_count);

    mutex_lock(&vdec_lock);
    if(vdec_open_count > 0)
    {
#ifdef USE_MMU
        if(vdec_param.mmu_state == VDEC_MMU_ON)
        {
            vdec_mmu_disable(vdec_param.pmm_handle);
            //vdec_param.mmu_state = 	VDEC_MMU_OFF;
        }

        if(vdec_mmu_deinit(vdec_param.pmm_handle) == 0)
        {
            vdec_param.mmu_inited = 0;
            vdec_param.pmm_handle = NULL;
        }
#endif 
        vdec_module_disable();
    }
    mutex_unlock(&vdec_lock);
    return VDEC_RET_OK;
}

static int vdec_resume(struct platform_device *pdev)
{
    DBGMSG(INFO, "%s(vdec_open_count=%d)\n", __func__, vdec_open_count);
    mutex_lock(&vdec_lock);
    if(vdec_open_count > 0)
    {
        vdec_module_enable();
#ifdef USE_MMU
        if(vdec_mmu_init(&(vdec_param.pmm_handle)) == 0)
        {
            vdec_param.mmu_inited = 1;
        }
        // assume vdec_param.mmu_state = VDEC_MMU_OFF;
        if(vdec_param.mmu_state == VDEC_MMU_ON){
            DBGMSG(ERR, "vdec_resume(), the mmu_state = VDEC_MMU_ON");
            vdec_mmu_enable(vdec_param.pmm_handle);
        }
#endif 
    }
    mutex_unlock(&vdec_lock);

    return VDEC_RET_OK;
}

void vdec_module_enable(void)
{
    //
    if(clk_enable(vdec_param.busClock) != 0){
        DBGMSG(ERR, "clk_enable(vdec) failed");
    }

    // TODO, enable vdec module.
    module_power_on(SYSMGR_VDEC_BASE);

    // enable mempool
    writel(1, vdec_param.mempool_virt_addr + 0xc820);	// mempool vdec mode.
    writel(1, vdec_param.mempool_virt_addr + 0xd000);	// trigger efuse read.
    //writel(1, vdec_param.mempool_virt_addr + 0x9000);	// trigger efuse read.

    // modle reset
    vdec_module_reset();

    //
#ifdef CONFIG_VDEC_DVFS_SUPPORT
    if(vdec_param.dvfsEna == VDEC_DVFS_ON){
        if(dvfs_init() != IM_RET_OK){
            DBGMSG(ERR, "dvfs_init() failed");
        }
    }
#endif
}

void vdec_module_disable(void)
{
    //
#ifdef CONFIG_VDEC_DVFS_SUPPORT
    if(vdec_param.dvfsEna == VDEC_DVFS_ON){
        if(dvfs_deinit() != IM_RET_OK){
            DBGMSG(ERR, "dvfs_deinit() failed");
        }
    }
#endif

    //
    writel(0, vdec_param.mempool_virt_addr + 0xc820);	// mempool vdec mode.
    writel(0, vdec_param.mempool_virt_addr + 0xd000);	// trigger efuse read.
    //writel(0, vdec_param.mempool_virt_addr + 0x9000);	// trigger efuse read.
    module_power_down(SYSMGR_VDEC_BASE);

    //
    clk_disable(vdec_param.busClock);
}

void vdec_module_reset(void)
{
    module_reset(SYSMGR_VDEC_BASE, 0x7);
}

int vdec_mmu_init(pmm_handle_t *pphandle)
{
    if(pmmdrv_open(pphandle, VDEC_MODULE_NAME) != IM_RET_OK)
    {
        DBGMSG(ERR,"vdec open pmmdrv failed \n");
        return -1;
    }

    if(pmmdrv_dmmu_init(*pphandle, DMMU_DEV_VDEC) != IM_RET_OK)
    {
        DBGMSG(ERR,"vdec  pmmdrv_dmmu_init failed \n");
        return -1;
    }

    return 0;

}

int vdec_mmu_deinit(pmm_handle_t phandle)
{
    if(pmmdrv_dmmu_deinit(phandle) != IM_RET_OK)
    {
        DBGMSG(ERR,"vdec  pmmdrv_dmmu_init failed \n");
    }

    if(pmmdrv_release(phandle) != IM_RET_OK)
    {
        DBGMSG(ERR,"vdec open pmmdrv failed \n");
        return -1;
    }

    return 0;
}

int vdec_mmu_enable(pmm_handle_t phandle)
{
    if(pmmdrv_dmmu_enable(phandle) != IM_RET_OK)
    {
        DBGMSG(ERR, "vdec Enable dmmu failed\n");
        return -1;
    }
    writel(1, vdec_param.mempool_virt_addr + 0x30020);	// mmu enable

    //*(volatile IM_UINT32 *)0x21e30020 = 0x1;	

    return 0;
}

int vdec_mmu_disable(pmm_handle_t phandle)
{
    writel(0, vdec_param.mempool_virt_addr + 0x30020);	// mmu disable

    if(pmmdrv_dmmu_disable(phandle) != IM_RET_OK)
    {
        DBGMSG(ERR, "vdec Disable dmmu failed\n");
        return -1;
    }

    //*(volatile IM_UINT32 *)0x21e30020 = 0x0;	
    return 0;
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
static int __init vdec_init(void)
{
    int ret = 0;
    DBGMSG(INFO, "%s()\n", __func__);

    /* Register platform device driver. */
    ret = platform_driver_register(&vdec_plat);
    if (unlikely(ret != 0)) {
        DBGMSG(ERR,"Register vdec platform device driver error, %d.\n", ret);
        return ret;
    }

    /* Register misc device driver. */
    ret = misc_register(&vdec_miscdev);
    if (unlikely(ret != 0)) {
        DBGMSG(ERR,"Register vdec misc device driver error, %d.\n", ret);
        return ret;
    }

    return 0;
}


static void __exit vdec_exit(void)
{
    DBGMSG(INFO, "%s()\n", __func__);

    misc_deregister(&vdec_miscdev);
    platform_driver_unregister(&vdec_plat);
}
module_init(vdec_init);
module_exit(vdec_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ayakashi of InfoTM");
MODULE_DESCRIPTION("vdec subsystem basic level driver");
