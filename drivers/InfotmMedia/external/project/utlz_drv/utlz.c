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
** v1.0.1	leo@2012/05/04: first commit.
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
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/mman.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/delay.h>

#include <InfotmMedia.h>
#include <IM_utlzapi.h>
#include <utlz_lib.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"UTLZDRV_I:"
#define WARNHEAD	"UTLZDRV_W:"
#define ERRHEAD		"UTLZDRV_E:"
#define TIPHEAD		"UTLZDRV_T:"


//
//
//
typedef struct{
	struct mutex 	lock;
	int		refcnt;
}utlz_drv_context_t;

static bool gInited = false;
static utlz_drv_context_t *gUtlzDriver = IM_NULL;

//
// function description.
//
static int utlzdrv_open(struct inode *inode, struct file *file);
static int utlzdrv_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int utlzdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long utlzdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int utlzdrv_probe(struct platform_device *pdev);
static int utlzdrv_remove(struct platform_device *pdev);
static int utlzdrv_suspend(struct platform_device *pdev, pm_message_t state);
static int utlzdrv_resume(struct platform_device *pdev);


utlz_handle_t utlzdrv_register_module(utlz_module_t *um)
{
	utlz_handle_t utlz;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	if((um == IM_NULL) || (um->name == IM_NULL) || (um->callback == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_NULL;
	}

	if(gInited == false){
		utlzdrv_probe(NULL);
	}
	mutex_lock(&gUtlzDriver->lock);

	if((utlz = utlzlib_register_module(um)) == IM_NULL){
		IM_ERRMSG((IM_STR("utlzlib_register_module() failed")));
		mutex_unlock(&gUtlzDriver->lock);
		return IM_NULL;
	}
	mutex_unlock(&gUtlzDriver->lock);
	return utlz;
}
EXPORT_SYMBOL(utlzdrv_register_module);

IM_RET utlzdrv_unregister_module(utlz_handle_t hdl)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	if(hdl == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gUtlzDriver->lock);
	if(gInited == false){
		mutex_unlock(&gUtlzDriver->lock);
		return IM_RET_FAILED;
	}

	if(utlzlib_unregister_module(hdl) != IM_RET_OK){
		IM_ERRMSG((IM_STR("utlzlib_unregister_module() failed")));
		mutex_unlock(&gUtlzDriver->lock);
		return IM_RET_FAILED;
	}
	mutex_unlock(&gUtlzDriver->lock);
	return IM_RET_OK;
}
EXPORT_SYMBOL(utlzdrv_unregister_module);

IM_RET utlzdrv_notify(utlz_handle_t hdl, IM_INT32 code)
{
	IM_INFOMSG((IM_STR("%s(code=%d) "), IM_STR(_IM_FUNC_), code));
	if((hdl == IM_NULL) || ((code != UTLZ_NOTIFY_CODE_RUN) && (code != UTLZ_NOTIFY_CODE_STOP))){
        IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gUtlzDriver->lock);
	if(utlzlib_notify(hdl, code) != IM_RET_OK){
		IM_ERRMSG((IM_STR("utlzlib_notify() failed")));
		mutex_unlock(&gUtlzDriver->lock);
		return IM_RET_FAILED;
	}
	mutex_unlock(&gUtlzDriver->lock);
	return IM_RET_OK;
}
EXPORT_SYMBOL(utlzdrv_notify);


static struct file_operations utlzdrv_fops = {
    .owner = THIS_MODULE,
    .open = utlzdrv_open,
    .release = utlzdrv_release,
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = utlzdrv_ioctl,
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
    .unlocked_ioctl = utlzdrv_ioctl,
#endif
};

static struct miscdevice utlzdrv_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &utlzdrv_fops,
    .name = "utlz",
};

static int utlzdrv_open(struct inode *inode, struct file *file)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	mutex_lock(&gUtlzDriver->lock);
	gUtlzDriver->refcnt++;
	IM_INFOMSG((IM_STR("gUtlzDriver->refcnt=%d"), gUtlzDriver->refcnt));
	mutex_unlock(&gUtlzDriver->lock);
	return 0;
}

static int utlzdrv_release(struct inode *inode, struct file *file)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_lock(&gUtlzDriver->lock);
	gUtlzDriver->refcnt--;
	IM_INFOMSG((IM_STR("gUtlzDriver->refcnt=%d"), gUtlzDriver->refcnt));
	mutex_unlock(&gUtlzDriver->lock);
	return 0;
}

#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int utlzdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long utlzdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	IM_RET ret = IM_RET_OK;
	utlz_ioctl_ds_get_coefficient_t ds_gc;
	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

	mutex_lock(&gUtlzDriver->lock);

	switch(cmd)
	{
	case UTLZ_IOCTL_GET_COEFFICIENT:
		__get_user(ds_gc.module, (IM_CHAR *)(&((utlz_ioctl_ds_get_coefficient_t *)arg)->module));
		ds_gc.coeffi = utlzlib_get_coefficient(ds_gc.module);
		__put_user(ds_gc.coeffi, (IM_INT32 *)(&(((utlz_ioctl_ds_get_coefficient_t *)arg)->coeffi)));
		break;
	default:
		IM_ERRMSG((IM_STR("unsupport cmd 0x%x"), cmd));
		ret = IM_RET_FAILED;
		break;
	}

	mutex_unlock(&gUtlzDriver->lock);

	return IM_FAILED(ret) ? -EFAULT : 0;
}

static struct platform_driver utlzdrv_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-utlz",
	},
	.probe = utlzdrv_probe,
	.remove = utlzdrv_remove,
	.suspend = utlzdrv_suspend,
	.resume = utlzdrv_resume,
};

static int utlzdrv_probe(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	if(gInited == true){
		return 0;
	}

	//
	gUtlzDriver = (utlz_drv_context_t *)kmalloc(sizeof(utlz_drv_context_t), GFP_KERNEL);
	if(gUtlzDriver == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(gUtlzDriver) failed")));
		return -1;
	}
	memset((void *)gUtlzDriver, 0, sizeof(utlz_drv_context_t));

	//
	mutex_init(&gUtlzDriver->lock);

	//
	if(utlzlib_init() != IM_RET_OK){
		IM_ERRMSG((IM_STR("utlzlib_init() failed")));
		goto Fail;
	}
	
	gInited = true;
	return 0;
Fail:
	mutex_destroy(&gUtlzDriver->lock);
	kfree(gUtlzDriver);
	return -1;
}

static int utlzdrv_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	utlzlib_deinit();
	mutex_destroy(&gUtlzDriver->lock);
	kfree(gUtlzDriver);
	gInited = false;
	return 0;
}

static int utlzdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}

static int utlzdrv_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}

static int __init utlzdrv_init(void)
{
	int ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* Register platform device driver. */
	ret = platform_driver_register(&utlzdrv_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register utlz platform device driver error.")));
		return ret;
	}

	/* Register misc device driver. */
	ret = misc_register(&utlzdrv_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register utlz misc device driver error.")));
		return ret;
	}

	return 0;
}

static void __exit utlzdrv_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	misc_deregister(&utlzdrv_miscdev);
	platform_driver_unregister(&utlzdrv_plat);
}
module_init(utlzdrv_init);
module_exit(utlzdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leo.Zhang of InfoTM");
MODULE_DESCRIPTION("UTLZ driver");

