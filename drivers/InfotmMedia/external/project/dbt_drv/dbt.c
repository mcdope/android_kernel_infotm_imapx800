/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.0.1	karst@2012/08/22: first commit.
**
*****************************************************************************/ 


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/page.h>
#include <linux/delay.h>

#include <InfotmMedia.h>
#include <dbt_uk.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"DBTDRV_I:"
#define WARNHEAD	"DBTDRV_W:"
#define ERRHEAD		"DBTDRV_E:"
#define TIPHEAD		"DBTDRV_T:"


//static ipc_drv_context_t *gDbtDriver = IM_NULL;
static int dbt_open(struct inode *inode, struct file *file);
static int dbt_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int dbt_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long dbt_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif

static struct file_operations dbt_fops = {
    .owner = THIS_MODULE,
    .open = dbt_open,
    .release = dbt_release,
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = dbt_ioctl,
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
    .unlocked_ioctl = dbt_ioctl,
#endif
};

static struct platform_driver dbt_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-dbt",
	},
};
static struct miscdevice dbt_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &dbt_fops,
    .name = "dbt",
};


static int dbt_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int dbt_release(struct inode *inode, struct file *file)
{
	return 0;
}

#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int dbt_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long dbt_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	IM_INT32 index;
	IM_INT32 num;
    void *virAddr;
	IM_UINT32 phyAddr;

	__get_user(num, (IM_INT32 *)(&((dbt_ioctl_rw_t *)arg)->num));
	__get_user(phyAddr, (IM_UINT32 *)(&((dbt_ioctl_rw_t *)arg)->phyAddr));

	virAddr = (void *)ioremap_nocache(phyAddr, MAX_DATA_LENGTH);
    if(virAddr == IM_NULL){
        IM_ERRMSG((IM_STR("ioremap_nocache() failed")));
        return -EFAULT;
    }

	switch(cmd) {
	case DBT_IOCTL_REG_WRITE:
	case DBT_IOCTL_MEM_WRITE:
		for (index = 0; index < num; ++index) {
			__get_user(*((IM_UINT32 *)virAddr + index), (IM_UINT32 *)(&((dbt_ioctl_rw_t *)arg)->vals) + 1); 
		}
		break;
	case DBT_IOCTL_REG_READ:
	case DBT_IOCTL_MEM_READ:
		for (index = 0; index < num; ++index) {
			__put_user(*((IM_UINT32 *)virAddr + index), (IM_UINT32 *)(&((dbt_ioctl_rw_t *)arg)->vals) + 1);
		}
		break;
	default:
		break;
	}

    if(virAddr != IM_NULL){
        iounmap(virAddr);
    }

	return 0;
}

static int __init dbt_init(void)
{
	int ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* Register platform device driver. */
	ret = platform_driver_register(&dbt_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register dbt platform device driver error, %d."),ret));
		return ret;
	}

	/* Register misc device driver. */
	ret = misc_register(&dbt_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register dbt misc device driver error, %d."),ret));
		return ret;
	}

	return 0;
}
static void __exit dbt_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	misc_deregister(&dbt_miscdev);
	platform_driver_unregister(&dbt_plat);
}
module_init(dbt_init);
module_exit(dbt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karst.Liu of InfoTM");
MODULE_DESCRIPTION("DBT driver");
