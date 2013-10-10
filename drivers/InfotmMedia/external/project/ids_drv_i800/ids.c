/******************************************************************************
 ** ids.c
 **
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 **
 ** Use of Infotm's code is governed by terms and conditions
 ** stated in the accompanying licensing statement.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** IMAPX display subsystem driver process.
 **
 **
 ** Revision History:
 ** -----------------
 ** v1.0.1	Sam@2012/03/20: first commit.
 ** v1.0.2	Sam@2012/04/09: test stable except for mmu.
 ** v1.0.3	leo@2012/04/09: modify idslib_async_config() and idslib_flush_config() call in ioctl().
 **		leo@2012/04/26: modify im_mpool_init() parameter, because kmalloc don't match func_mempool_malloc_t;
 ** v1.0.5	sam@2012/06/12: add item & pmu support.
 **		leo@2012/06/18: add suspend&resume support.
 ** v1.0.6	leo@2012/07/03: support early suspend.
 ** v1.0.7	leo@2012/08/11: readjust.
 ** v2.0.1	sam@2013/01/07: structure readjust, move core vmode ids structure to kernel and add extlayer
 **			support. Support double display
 **
 ******************************************************************************/


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
#include <linux/earlysuspend.h>
#include <linux/mutex.h>
#include <linux/mman.h>
#include <linux/completion.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/items.h>

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <ids_pwl.h>
#include "ids.h"
#include <linux/time.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IDSDRV_I:"
#define WARNHEAD	"IDSDRV_W:"
#define ERRHEAD		"IDSDRV_E:"
#define TIPHEAD		"IDSDRV_T:"


//
// the whole ids_drv management.
//
static idsdrv_global_t *gIdsdrvGlobal = IM_NULL;

//
// to share ids_drv resource with external, i.e, ids_pwl_linux, lcd_api ...
//
idsdrv_resource_t *gIdsdrvResource = IM_NULL;

//
//
#define IDS_HDMI_NO_LOCAL	(1) // 1<<0
#define IDS_BANK_RESERVE	(3) // (1<<0 | 1<<1)
static int gIDSMemCheckFlag = 0;

static int ids_open(struct inode *inode, struct file *file);
static int ids_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static long ids_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ids_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int ids_mmap(struct file *file, struct vm_area_struct *vma);
static unsigned int ids_poll(struct file *file, poll_table *wait);
static int ids_probe(struct platform_device *pdev);
static int ids_remove(struct platform_device *pdev);
#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ids_early_suspend(struct early_suspend *s);
static void ids_late_resume(struct early_suspend *s);
#else
static int ids_suspend(struct platform_device *pdev, pm_message_t state);
static int ids_resume(struct platform_device *pdev);
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
static struct file_operations ids_fops = {
    .owner = THIS_MODULE,
    .open = ids_open,
    .release = ids_release,
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
    .unlocked_ioctl = ids_ioctl,
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = ids_ioctl,
#endif
    .mmap = ids_mmap,
    .poll = ids_poll,
};

static struct miscdevice ids_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &ids_fops,
    .name = "ids",
};

//#############################################################################

#define AUTOLOCK_CALL_RETURN(func, ...)	\
    mutex_lock(&gIdsdrvGlobal->acslock);	\
ret = func(__VA_ARGS__);	\
mutex_unlock(&gIdsdrvGlobal->acslock);	\
return ret;

#define DUMMY_FBLAYER	((fblayer_handle_t)0x87654321)

IM_RET idsdrv_fblayer_init(fblayer_handle_t *handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsdrvGlobal);// MUST ENSURE ids_probe has been called and successfully.
    if(handle == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(! gIdsdrvGlobal->initcfg.resvFblayer){
        *handle	= DUMMY_FBLAYER;
        return IM_RET_OK;
    }
    gIdsdrvGlobal->fbl.extSwpDone = 0;
    sema_init(&gIdsdrvGlobal->fbl.extSwpSem, 0);
    AUTOLOCK_CALL_RETURN(idslib_fblayer_init, handle);
}
EXPORT_SYMBOL(idsdrv_fblayer_init);

IM_RET idsdrv_fblayer_deinit(fblayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    if(! gIdsdrvGlobal->initcfg.resvFblayer){
        return IM_RET_OK;
    }
    AUTOLOCK_CALL_RETURN(idslib_fblayer_deinit, handle);
}
EXPORT_SYMBOL(idsdrv_fblayer_deinit);

IM_RET idsdrv_fblayer_open(fblayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    if(! gIdsdrvGlobal->initcfg.resvFblayer){
        return IM_RET_OK;
    }
    AUTOLOCK_CALL_RETURN(idslib_fblayer_open, handle);
}
EXPORT_SYMBOL(idsdrv_fblayer_open);

IM_RET idsdrv_fblayer_close(fblayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    if(! gIdsdrvGlobal->initcfg.resvFblayer){
        return IM_RET_OK;
    }
    AUTOLOCK_CALL_RETURN(idslib_fblayer_close, handle);
}
EXPORT_SYMBOL(idsdrv_fblayer_close);

IM_RET idsdrv_fblayer_swap_buffer(fblayer_handle_t handle, IM_Buffer *buffer)
{
    IM_RET ret;
    //IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(! gIdsdrvGlobal->initcfg.resvFblayer){
        return IM_RET_OK;
    }
    /*if(gIdsdrvGlobal->suspended){
        return IM_RET_OK;
    }*/

    gIdsdrvGlobal->fbl.extSwpDone = 0;
    mutex_lock(&gIdsdrvGlobal->acslock);
    ret = idslib_fblayer_swap_buffer(handle, buffer);
    if((ret != IM_RET_OK && ret != IM_RET_FALSE)){
        IM_ERRMSG((IM_STR("idslib_fblayer_swap_buffer() failed")));
        mutex_unlock(&gIdsdrvGlobal->acslock);
        return IM_RET_FAILED;
    }
    mutex_unlock(&gIdsdrvGlobal->acslock);

    if(!gIdsdrvGlobal->disWaitPostVsync){
        mutex_lock(&gIdsdrvGlobal->acslock);
        if(gIdsdrvGlobal->waitVsync){
            mutex_unlock(&gIdsdrvGlobal->acslock);
            wait_for_completion_interruptible(&gIdsdrvGlobal->comple);
        }else{
            gIdsdrvGlobal->waitVsync = true;
            INIT_COMPLETION(gIdsdrvGlobal->comple);
            mutex_unlock(&gIdsdrvGlobal->acslock);

            idslib_fblayer_vsync(handle);

            mutex_lock(&gIdsdrvGlobal->acslock);
            complete_all(&gIdsdrvGlobal->comple);
            gIdsdrvGlobal->waitVsync = false;
            mutex_unlock(&gIdsdrvGlobal->acslock);
        }
    }

    if ( ret == IM_RET_FALSE){ // wait extend-fblayer completed.
        mutex_lock(&gIdsdrvGlobal->acslock);
        if (gIdsdrvGlobal->fbl.extSwpDone == 0){
            mutex_unlock(&gIdsdrvGlobal->acslock);
            while(! down_trylock(&gIdsdrvGlobal->fbl.extSwpSem));
            down_timeout(&gIdsdrvGlobal->fbl.extSwpSem, (long)20 * HZ / 1000);
            mutex_lock(&gIdsdrvGlobal->acslock);
        }
        mutex_unlock(&gIdsdrvGlobal->acslock);
    }

    return IM_RET_OK;
}
EXPORT_SYMBOL(idsdrv_fblayer_swap_buffer);

IM_RET idsdrv_fblayer_get_fbinfo(fblayer_handle_t handle, idslib_fb_info_t *fbinfo)
{
    if(fbinfo == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_FAILED;
    }

    fbinfo->pixfmt = gIdsdrvGlobal->initcfg.pixfmt;
    fbinfo->width = gIdsdrvGlobal->initcfg.fbWidth;
    fbinfo->height = gIdsdrvGlobal->initcfg.fbHeight;
    return IM_RET_OK;
}
EXPORT_SYMBOL(idsdrv_fblayer_get_fbinfo);

IM_RET idsdrv_fblayer_get_device(fblayer_handle_t handle, dispdev_info_t *info)
{
    IM_RET ret;
    ids_display_device_t dev;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(!gIdsdrvGlobal->initcfg.resvFblayer){
        if(info){
            info->width = gIdsdrvGlobal->initcfg.fbWidth;
            info->height = gIdsdrvGlobal->initcfg.fbHeight;
            info->dataport = DISPDEV_DATAPORT_RGB888;
            info->hspw = 1;
            info->hbpd = 1;
            info->hfpd = 1;
            info->vspw = 1;
            info->vbpd = 1;
            info->vfpd = 1;
            info->fpsx1000 = 60000;
            info->phyWidth = 0;
            info->phyHeight = 0;
        }
        return IM_RET_OK;
    }

    AUTOLOCK_CALL_RETURN(idslib_fblayer_get_device, handle, &dev, info, IM_NULL);
}
EXPORT_SYMBOL(idsdrv_fblayer_get_device);

IM_RET idsdrv_get_local_ids(IM_INT32 *idsx)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_get_local_ids, idsx);
}
EXPORT_SYMBOL(idsdrv_get_local_ids);

IM_RET idsdrv_vsync(IM_INT32 idsx)
{
    IM_RET ret = IM_RET_OK;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    if(idsx == 0){
        mutex_lock(&gIdsdrvGlobal->acslock);
        if(gIdsdrvGlobal->waitVsync){
            mutex_unlock(&gIdsdrvGlobal->acslock);
            wait_for_completion_interruptible(&gIdsdrvGlobal->comple);
            mutex_lock(&gIdsdrvGlobal->acslock);
        }else{
            gIdsdrvGlobal->waitVsync = true;
            INIT_COMPLETION(gIdsdrvGlobal->comple);
            mutex_unlock(&gIdsdrvGlobal->acslock);

            ret = idslib_vsync(idsx);

            mutex_lock(&gIdsdrvGlobal->acslock);
            complete_all(&gIdsdrvGlobal->comple);
            gIdsdrvGlobal->waitVsync = false;
        }
        mutex_unlock(&gIdsdrvGlobal->acslock);
    }else{
        idslib_vsync(idsx);
    }
    return ret;
}
EXPORT_SYMBOL(idsdrv_vsync);

IM_RET idsdrv_wlayer_init(wlayer_handle_t *handle, IM_INT32 idsx, IM_INT32 wx)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_init, handle, idsx, wx);
}
EXPORT_SYMBOL(idsdrv_wlayer_init);

IM_RET idsdrv_wlayer_deinit(wlayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_deinit, handle);
}
EXPORT_SYMBOL(idsdrv_wlayer_deinit);

IM_RET idsdrv_wlayer_query_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_query_resource, handle, res, param);
}
EXPORT_SYMBOL(idsdrv_wlayer_query_resource);

IM_RET idsdrv_wlayer_reserve_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_reserve_resource, handle, res, param);
}
EXPORT_SYMBOL(idsdrv_wlayer_reserve_resource);

IM_RET idsdrv_wlayer_release_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_release_resource, handle, res, param);
}
EXPORT_SYMBOL(idsdrv_wlayer_release_resource);

IM_RET idsdrv_wlayer_open(wlayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_open, handle);
}
EXPORT_SYMBOL(idsdrv_wlayer_open);

IM_RET idsdrv_wlayer_close(wlayer_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_close, handle);
}
EXPORT_SYMBOL(idsdrv_wlayer_close);

IM_RET idsdrv_wlayer_get_device(wlayer_handle_t handle, ids_display_device_t *dev, dispdev_info_t *info, ids_wlayer_canvas_t *canvas)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_get_device, handle, dev, info, canvas);
}
EXPORT_SYMBOL(idsdrv_wlayer_get_device);

IM_RET idsdrv_wlayer_set_panorama(wlayer_handle_t handle, IM_INT32 pixfmt, ids_wlayer_panorama_t *pano)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_wlayer_set_panorama, handle, pixfmt, pano);
}
EXPORT_SYMBOL(idsdrv_wlayer_set_panorama);

IM_RET idsdrv_wlayer_set_buffer(wlayer_handle_t handle, IM_Buffer *buffer, IM_Buffer *bufferUv)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    mutex_lock(&gIdsdrvGlobal->acslock);
    if(idslib_wlayer_set_buffer(handle, buffer, bufferUv) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_buffer() failed")));
        mutex_unlock(&gIdsdrvGlobal->acslock);
        return IM_RET_FAILED;
    }
    mutex_unlock(&gIdsdrvGlobal->acslock);
    idslib_wlayer_vsync(handle);
    return IM_RET_OK;
}
EXPORT_SYMBOL(idsdrv_wlayer_set_buffer);

IM_RET idsdrv_vmode_init(vmode_handle_t *handle, idslib_func_idsdrv_listener_t listener)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_init, handle, listener);
}
EXPORT_SYMBOL(idsdrv_vmode_init);

IM_RET idsdrv_vmode_deinit(vmode_handle_t handle)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_deinit, handle);
}
EXPORT_SYMBOL(idsdrv_vmode_deinit);

IM_RET idsdrv_vmode_get_mode(vmode_handle_t handle, ids_vmode_config_t *config)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_get_mode, handle, config);
}
EXPORT_SYMBOL(idsdrv_vmode_get_mode);

IM_RET idsdrv_vmode_get_devs(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 *num, ids_display_device_t *devs)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_get_devs, handle, idsx, num, devs);
}
EXPORT_SYMBOL(idsdrv_vmode_get_devs);

IM_RET idsdrv_vmode_set_dev(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 devtype, IM_INT32 devid)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_set_dev, handle, idsx, devtype, devid);
}
EXPORT_SYMBOL(idsdrv_vmode_set_dev);

IM_RET idsdrv_vmode_set_canvas(vmode_handle_t handle, IM_INT32 idsx, ids_wlayer_canvas_t *canvas)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    AUTOLOCK_CALL_RETURN(idslib_vmode_set_canvas, handle, idsx, canvas);
}
EXPORT_SYMBOL(idsdrv_vmode_set_canvas);

static int check_bank_need(IM_INT32 *bEn)
{
    IM_UINT32 memSize, m16;
    IM_UINT32 width, height;

    m16 = 0;
    memSize = 0;
    width = 0;
    height = 0;

    if(item_exist("memory.size")){
        memSize = item_integer("memory.size",0);
    }
    if(item_exist("ids.fb.width")){
        width = item_integer("ids.fb.width", 0);
    }
    if(item_exist("ids.fb.height")){
        height = item_integer("ids.fb.height", 0);
    }
    if (memSize != 768 || width < 1280 || height < 720){
        *bEn = 0;
    }
    else{
        *bEn = 1;
        gIDSMemCheckFlag |= IDS_BANK_RESERVE;
        return IM_RET_OK;
    }

    if(item_exist("memory.reduce_en")){
        m16 = item_integer("memory.reduce_en",0);
    }
    if (m16){
        gIDSMemCheckFlag |= IDS_HDMI_NO_LOCAL;
    }
    return IM_RET_OK;
}

IM_RET idsdrv_fblayer_bank_check(IM_Buffer *buf, IM_UINT32 size, IM_INT32 *bEn)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    check_bank_need(bEn);
    if (*bEn == 1)
    {
        if (buf != IM_NULL){
            buf->phy_addr = 0xB8000000 - size / CONFIG_FB_IMAP_BUFFER_NUM;
            buf->vir_addr = ioremap_nocache(buf->phy_addr , size);
            buf->flag = IM_BUFFER_FLAG_PHY;
        }
    }

    return IM_RET_OK;
}
EXPORT_SYMBOL(idsdrv_fblayer_bank_check);

/*
 *
 *  To check whether we need to reserve two banks for display:
 *  If all the conditions confirms , we choose the way :
 *   i)  DDR 1G, and memory.size 768, means two banks reserved(256M).
 *   ii) fb width >= 1280
 *   iii) fb height >= 720
 *
 *  idsdrv_bank_check_2 must be called before idsdrv_fblayer_bank_check(...)
 */
IM_RET idsdrv_bank_check_2(bank_check  *bh)
{
    IM_UINT32 memSize;
    IM_UINT32 width, height, fbSize;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    memSize = 0;
    width = 0;
    height = 0;

    if (gIDSMemCheckFlag != IDS_BANK_RESERVE){
        bh->bankNeeds = IM_FALSE;
        return IM_RET_OK;
    }

    if(item_exist("ids.fb.width")){
        width = item_integer("ids.fb.width", 0);
    }
    if(item_exist("ids.fb.height")){
        height = item_integer("ids.fb.height", 0);
    }
    if (width == 0 || height == 0){
        IM_ERRMSG((IM_STR(" framebuffer width or height error ")));	
        return IM_RET_FAILED;
    }
    fbSize = width * height * 4; // make it RGB888, or even if pixfmt is 565, it is also OK.
    bh->bankNeeds = IM_TRUE;
    bh->bufA.vir_addr = 0;
    bh->bufA.phy_addr = 0xB8000000 - fbSize - bh->size;
    bh->bufA.size = bh->size;
    bh->bufA.flag = IM_BUFFER_FLAG_PHY;

    bh->bufB.vir_addr = 0;
    bh->bufB.phy_addr = 0xB8000000 + fbSize * 3; // CONFIG_FB_IMAP_BUFFER_NUM - 1
    bh->bufB.size = bh->size;
    bh->bufB.flag = IM_BUFFER_FLAG_PHY;

    return IM_RET_OK;
}
EXPORT_SYMBOL(idsdrv_bank_check_2);

//#############################################################################

static void idsdrv_clear_event(im_list_handle_t evtlst, IM_INT32 evt1, IM_INT32 evt2, IM_INT32 evt3, IM_INT32 evt4)
{
    idslib_event_t *l = im_list_begin(evtlst);

    while(l){
        if(((evt1 != -1) && (l->evt == evt1)) || ((evt2 != -1) && (l->evt == evt2)) ||
                ((evt3 != -1) && (l->evt == evt3)) || ((evt4 != -1) && (l->evt == evt4))){
            l = im_list_erase(evtlst, l);
        }else{
            l = im_list_next(evtlst);
        }
    }
}

static void idsdrv_listener(IM_INT32 mask, IM_INT32 idsx, IM_INT32 wx, idslib_event_t *evt)
{
    idsdrv_instance_t *inst;
    IM_INFOMSG((IM_STR("%s(mask=0x%x, idsx=%d, wx=%d, evt=0x%x)"), IM_STR(_IM_FUNC_), mask, idsx, wx, evt->evt));

    mutex_lock(&gIdsdrvGlobal->acslock);
    inst = (idsdrv_instance_t *)im_list_begin(gIdsdrvGlobal->instlst);
    while(inst)
    {
        mutex_lock(&inst->lock);
        if((inst->type == IDSLIB_INST_VMODE) && (mask & IDSLIB_VMEVT_MASK_VMODE))
        {
            if(evt->evt == IDS_VMODE_EVENT_EXTDEV_UPDATE){
                idsdrv_clear_event(inst->evtlst, IDS_VMODE_EVENT_EXTDEV_UPDATE, -1, -1, -1);
            }
            else if(evt->evt == IDS_VMODE_EVENT_EXTDEV_BREAK){
                idsdrv_clear_event(inst->evtlst, IDS_VMODE_EVENT_EXTDEV_BREAK, IDS_VMODE_EVENT_EXTDEV_UPDATE, -1, -1);
            }
            else if(evt->evt == IDS_VMODE_EVENT_BACK_TO_LOCAL){
                idsdrv_clear_event(inst->evtlst, IDS_VMODE_EVENT_BACK_TO_LOCAL, -1, -1, -1);
            }
            else if(evt->evt == IDS_VMODE_EVENT_EXCEPTION){
                idsdrv_clear_event(inst->evtlst, IDS_VMODE_EVENT_EXCEPTION, -1, -1, -1);
            }
            else if(evt->evt == IDS_VMODE_NOTIFY_FBL_SWAPBF){
                gIdsdrvGlobal->fbl.extSwpDone = 1;
                up(&gIdsdrvGlobal->fbl.extSwpSem);
                mutex_unlock(&inst->lock);
                break;
            }
            im_list_put_back(inst->evtlst, evt);
            wake_up(&inst->waitQ);
            mutex_unlock(&inst->lock);
            break;
        }
        else if((inst->type == IDSLIB_INST_EXTLAYER	) && (mask & IDSLIB_VMEVT_MASK_EXTLAYER))
        {
            if(evt->evt == IDS_EXTLAYER_EVENT_EXTDEV_BREAK){
                idsdrv_clear_event(inst->evtlst, IDS_EXTLAYER_EVENT_EXTDEV_BREAK, IDS_EXTLAYER_EVENT_BACK_TO_LOCAL, -1, -1);
            }else if(evt->evt == IDS_EXTLAYER_EVENT_BACK_TO_LOCAL){
                idsdrv_clear_event(inst->evtlst, IDS_EXTLAYER_EVENT_BACK_TO_LOCAL, -1, -1, -1);
            }
            im_list_put_back(inst->evtlst, evt);
            wake_up(&inst->waitQ);
            mutex_unlock(&inst->lock);
            break;
        }

        mutex_unlock(&inst->lock);
        inst = (idsdrv_instance_t *)im_list_next(gIdsdrvGlobal->instlst);
    }
    mutex_unlock(&gIdsdrvGlobal->acslock);
}

/*
 * Tutorial:
 * Each open() system call returns a file descriptor if success. And as a
 * user space application, open() is a must step to access further kernel
 * module interfaces.
 */
static int ids_open(struct inode *inode, struct file *file)
{
    idsdrv_instance_t *inst = IM_NULL;
    IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

    //
    inst = (idsdrv_instance_t *)kmalloc(sizeof(idsdrv_instance_t), GFP_KERNEL);
    if(inst == IM_NULL){
        IM_ERRMSG((IM_STR("kmalloc(instance) failed!")));
        return -EFAULT;
    }
    memset((void *)inst, 0, sizeof(idsdrv_instance_t));
    inst->idsx = -1;
    inst->wx = -1;
    mutex_init(&inst->lock);
    init_waitqueue_head(&inst->waitQ);
    inst->evtlst = im_list_init(sizeof(idslib_event_t), gIdsdrvGlobal->mpl);
    if(inst->evtlst == IM_NULL){
        IM_ERRMSG((IM_STR("im_list_init() failed")));
        goto Fail;
    }

    //
    mutex_lock(&gIdsdrvGlobal->acslock);
    im_list_put_back(gIdsdrvGlobal->instlst, (void *)inst);
    IM_INFOMSG((IM_STR("gIdsdrvGlobal->instlst length=%d"), im_list_size(gIdsdrvGlobal->instlst)));
    file->private_data = (void *)inst;
    mutex_unlock(&gIdsdrvGlobal->acslock);
    return 0;
Fail:
    if(inst->evtlst){
        im_list_deinit(inst->evtlst);
    }
    mutex_destroy(&inst->lock);
    kfree((void *)inst);
    return -EFAULT;
}

/*
 * Tutorial:
 * After open() a driver node, the file descriptor must be closed if you
 * don't use it anymore.
 */
static int ids_release(struct inode *inode, struct file *file)
{
    idsdrv_instance_t *inst = (idsdrv_instance_t *)file->private_data;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(!inst->gotAcslock){
        mutex_lock(&gIdsdrvGlobal->acslock);
    }

    IM_INFOMSG((IM_STR(" inst->type = %d - "),inst->type));
    if((inst->type == IDSLIB_INST_WLAYER) && inst->wlhandle){
        idslib_wlayer_deinit((wlayer_handle_t)inst->wlhandle);
    }else if ((inst->type == IDSLIB_INST_EXTLAYER)){
        idslib_extlayer_deinit((extlayer_handle_t)inst->extlhandle);
    }else if((inst->type == IDSLIB_INST_VMODE) && inst->vmhandle){
        idslib_vmode_deinit((vmode_handle_t)inst->vmhandle);
        if(inst->iehandle){
            idslib_ie_deinit((ie_handle_t)inst->iehandle);
        }
    }else if((inst->type == IDSLIB_INST_IE) && inst->iehandle){
        idslib_ie_deinit((ie_handle_t *)inst->iehandle);
    }

    im_list_clear(inst->evtlst);
    im_list_deinit(inst->evtlst);
    mutex_destroy(&inst->lock);
    im_list_erase(gIdsdrvGlobal->instlst, (void *)inst);
    kfree((void *)inst);

    mutex_unlock(&gIdsdrvGlobal->acslock);
    return 0;
}

/*
 * Tutorial:
 * IO control is the most widely used system call to driver because it
 * can plement all functionalities provided by others.
 */
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
    static long ids_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ids_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    idsdrv_instance_t *inst = (idsdrv_instance_t *)file->private_data;
    IM_RET ret = IM_RET_OK;
    IM_UINT32 tmp, s;
    IM_INT32 idsx, wx, pixFormat, ieval;
    ioctl_ds_wlayer_get_device_t ds_wgd;
    void * handle = IM_NULL;
    ioctl_ds_resource_t ds_res;
    ids_vmode_config_t vmcfg;
    ioctl_ds_vmode_set_mode_t ds_vsm;
    ioctl_ds_vmode_get_devs_t ds_vgds;
    ioctl_ds_vmode_set_dev_t ds_vsd;
    ioctl_ds_vmode_set_canvas_t ds_vsc;
    ioctl_ds_panorama_t ds_pano;
    ioctl_ds_region_t ds_region;
    ioctl_ds_buffer_t ds_buf;
    ioctl_ds_position_t ds_pos;
    ioctl_ds_mapclr_t ds_mc;
    ioctl_ds_alpha_t ds_alpha;
    ioctl_ds_clrkey_t ds_ck;
    idslib_event_t *evt;
    bank_check bch;

    //IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

    mutex_lock(&gIdsdrvGlobal->acslock);
    inst->gotAcslock = IM_TRUE;

    switch(cmd){
        case IDSLIB_IOCTL_VERSION:
            tmp = idslib_version();
            __put_user(tmp, (IM_UINT32 *)arg);
            break;
        case IDSLIB_IOCTL_GET_LOCAL_IDS:
            ret = idslib_get_local_ids(&idsx);
            if(ret == IM_RET_OK){
                __put_user(idsx, (IM_INT32 *)arg);
            }
            break;
        case IDSLIB_IOCTL_VSYNC:
            __get_user(idsx, (IM_INT32 *)arg);
            if(idsx == 0){
                if(gIdsdrvGlobal->waitVsync){
                    mutex_unlock(&gIdsdrvGlobal->acslock);
                    wait_for_completion_interruptible(&gIdsdrvGlobal->comple);
                    mutex_lock(&gIdsdrvGlobal->acslock);
                }else{
                    gIdsdrvGlobal->waitVsync = true;
                    INIT_COMPLETION(gIdsdrvGlobal->comple);
                    mutex_unlock(&gIdsdrvGlobal->acslock);

                    idslib_vsync(idsx);

                    mutex_lock(&gIdsdrvGlobal->acslock);
                    complete_all(&gIdsdrvGlobal->comple);
                    gIdsdrvGlobal->waitVsync = false;
                }
            }else{
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_vsync(idsx);
                mutex_lock(&gIdsdrvGlobal->acslock);
            }   
            break;
        case IDSLIB_IOCTL_EXTLAYER_INIT:
            if (gIDSMemCheckFlag & IDS_BANK_RESERVE){
                // if no local show in projection mode, we just doesn't support extlayer use.
                ret = IM_RET_FAILED;
            }
            else{
                ret = idslib_extlayer_init((extlayer_handle_t*)&handle,(idslib_func_idsdrv_listener_t)idsdrv_listener);
                if (ret == IM_RET_OK)
                    inst->extlhandle = handle;
            }
            break;
        case IDSLIB_IOCTL_EXTLAYER_DEINIT:
            ret = idslib_extlayer_deinit((extlayer_handle_t)inst->extlhandle);
            break;
        case IDSLIB_IOCTL_WLAYER_INIT:
            __get_user(idsx, (IM_INT32 *)(&((ioctl_ds_wlayer_init_t *)arg)->idsx));
            __get_user(wx, (IM_INT32 *)(&((ioctl_ds_wlayer_init_t *)arg)->wx));
            __get_user(pixFormat, (IM_INT32 *)(&((ioctl_ds_wlayer_init_t*)arg)->pixFormat));
            if (pixFormat == IM_PIC_FMT_16BITS_RGB_565){
                inst->pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
            }
            else if (pixFormat == IM_PIC_FMT_32BITS_0RGB_8888){
                inst->pixfmt = IDSLIB_PIXFMT_32BPP_RGB0888;
            }
            else if (pixFormat == IM_PIC_FMT_32BITS_ARGB_8888){
                inst->pixfmt = IDSLIB_PIXFMT_32BPP_ARGBA888;
            }
            else if ( pixFormat == IM_PIC_FMT_12BITS_YUV420SP){
                inst->pixfmt = IDSLIB_PIXFMT_12BPP_YUV420SP;
            }
            else{
                IM_ERRMSG((IM_STR(" IDSLIB_IOCTL_WLAYER_INIT pixfmt not supported(0x%x)"),pixFormat));
                ret = IM_RET_FAILED;
                __put_user(ret, (IM_RET *)(&((ioctl_ds_wlayer_init_t *)arg)->ret));
                break;
            }
            ret = idslib_wlayer_init((wlayer_handle_t *)&handle, idsx, wx);
            __put_user(ret, (IM_RET *)(&((ioctl_ds_wlayer_init_t *)arg)->ret));
            inst->wlhandle = handle;
            inst->idsx = idsx;
            inst->wx = wx;

            if(ret == IM_RET_NORESOURCE){
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_DEINIT:
            ret = idslib_wlayer_deinit((wlayer_handle_t)inst->wlhandle);
            inst->wlhandle = IM_NULL;
            break;
        case IDSLIB_IOCTL_WLAYER_QUERY_RESOURCE:
            s = copy_from_user((void *)&ds_res, (void *)arg, sizeof(ioctl_ds_resource_t));
            ret = idslib_wlayer_query_resource((wlayer_handle_t)inst->wlhandle, ds_res.res, ds_res.param);
            __put_user(ret, (IM_RET *)(&((ioctl_ds_resource_t *)arg)->ret));
            ret = IM_RET_OK;
            break;
        case IDSLIB_IOCTL_WLAYER_RESERVE_RESOURCE:
            s = copy_from_user((void *)&ds_res, (void *)arg, sizeof(ioctl_ds_resource_t));
            ret = idslib_wlayer_reserve_resource((wlayer_handle_t)inst->wlhandle, ds_res.res, ds_res.param);
            __put_user(ret, (IM_RET *)(&((ioctl_ds_resource_t *)arg)->ret));
            ret = IM_RET_OK;
            break;
        case IDSLIB_IOCTL_WLAYER_RELEASE_RESOURCE:
            s = copy_from_user((void *)&ds_res, (void *)arg, sizeof(ioctl_ds_resource_t));
            ret = idslib_wlayer_release_resource((wlayer_handle_t)inst->wlhandle, ds_res.res, ds_res.param);
            __put_user(ret, (IM_RET *)(&((ioctl_ds_resource_t *)arg)->ret));
            ret = IM_RET_OK;
            break;
        case IDSLIB_IOCTL_WLAYER_OPEN:
            ret = idslib_wlayer_open((wlayer_handle_t)inst->wlhandle);
            break;
        case IDSLIB_IOCTL_WLAYER_CLOSE:
            ret = idslib_wlayer_close((wlayer_handle_t)inst->wlhandle);
            break;
        case IDSLIB_IOCTL_WLAYER_GET_DEVICE:
            ret = idslib_wlayer_get_device((wlayer_handle_t)inst->wlhandle, &ds_wgd.dev, &ds_wgd.info, &ds_wgd.canvas);
            if(ret == IM_RET_OK){
                s = copy_to_user((void *)arg, (void *)&ds_wgd, sizeof(ioctl_ds_wlayer_get_device_t));
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_PANORAMA:
            s = copy_from_user((void*)&ds_pano, (void*)arg, sizeof(ioctl_ds_panorama_t));
            ds_pano.ret = idslib_wlayer_set_panorama((wlayer_handle_t)inst->wlhandle, inst->pixfmt, &(ds_pano.panorama));
            if (ds_pano.ret == IM_RET_OK){
                __put_user(ds_pano.ret, (IM_RET *)(&((ioctl_ds_panorama_t*)arg)->ret));
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_BUFFER:
            s = copy_from_user((void*)&ds_buf, (void*)arg, sizeof(ioctl_ds_buffer_t));
            ds_buf.ret = idslib_wlayer_set_buffer((wlayer_handle_t)inst->wlhandle, &(ds_buf.buffer),&(ds_buf.bufferUv));
            if (ds_buf.ret == IM_RET_OK){
                __put_user(ds_buf.ret, (IM_RET *)(&((ioctl_ds_buffer_t*)arg)->ret));
                ret = IM_RET_OK;
            }
            // test shows if frame intr waited, hdmi video would be stucked once in a while
            /*
               mutex_unlock(&gIdsdrvGlobal->acslock);
               idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
               mutex_lock(&gIdsdrvGlobal->acslock);*/
            break;
        case IDSLIB_IOCTL_WLAYER_SET_REGION:
            s = copy_from_user((void*)&ds_region, (void*)arg, sizeof(ioctl_ds_region_t));
            ds_region.ret = idslib_wlayer_set_region((wlayer_handle_t)inst->wlhandle, &(ds_region.region));
            if (ds_region.ret == IM_RET_OK){
                __put_user(ds_region.ret, (IM_RET *)(&((ioctl_ds_region_t*)arg)->ret));
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
                mutex_lock(&gIdsdrvGlobal->acslock);
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_POSITION:
            s = copy_from_user((void*)&ds_pos, (void*)arg, sizeof(ioctl_ds_position_t));
            ds_pos.ret = idslib_wlayer_set_position((wlayer_handle_t)inst->wlhandle, &(ds_pos.position));
            if (ds_pos.ret == IM_RET_OK){
                __put_user(ds_pos.ret, (IM_RET *)(&((ioctl_ds_position_t*)arg)->ret));
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
                mutex_lock(&gIdsdrvGlobal->acslock);
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_MAPCLR:
            s = copy_from_user((void*)&ds_mc, (void*)arg, sizeof(ioctl_ds_mapclr_t));
            ds_mc.ret = idslib_wlayer_set_mapclr((wlayer_handle_t)inst->wlhandle, ds_mc.mapclr.enable,ds_mc.mapclr.clr);
            if (ds_mc.ret == IM_RET_OK){
                __put_user(ds_mc.ret, (IM_RET *)(&((ioctl_ds_mapclr_t*)arg)->ret));
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
                mutex_lock(&gIdsdrvGlobal->acslock);
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_ALPHA:
            s = copy_from_user((void*)&ds_alpha, (void*)arg, sizeof(ioctl_ds_alpha_t));
            ds_alpha.ret = idslib_wlayer_set_alpha((wlayer_handle_t)inst->wlhandle, &(ds_alpha.alpha));
            if (ds_mc.ret == IM_RET_OK){
                __put_user(ds_alpha.ret, (IM_RET *)(&((ioctl_ds_alpha_t*)arg)->ret));
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
                mutex_lock(&gIdsdrvGlobal->acslock);
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_WLAYER_SET_CLRKEY:
            s = copy_from_user((void*)&ds_ck, (void*)arg, sizeof(ioctl_ds_clrkey_t));
            ds_ck.ret = idslib_wlayer_set_clrkey((wlayer_handle_t)inst->wlhandle, ds_ck.enable,ds_ck.matchForeground,ds_ck.clr);
            if (ds_mc.ret == IM_RET_OK){
                __put_user(ds_ck.ret, (IM_RET *)(&((ioctl_ds_clrkey_t*)arg)->ret));
                mutex_unlock(&gIdsdrvGlobal->acslock);
                idslib_wlayer_vsync((wlayer_handle_t)inst->wlhandle);
                mutex_lock(&gIdsdrvGlobal->acslock);
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_VMODE_INIT:
            ret = idslib_vmode_init((vmode_handle_t *)&handle, (idslib_func_idsdrv_listener_t)idsdrv_listener);
            inst->vmhandle = handle;
            break;
        case IDSLIB_IOCTL_VMODE_DEINIT:
            ret = idslib_vmode_deinit((vmode_handle_t)inst->vmhandle);
            inst->vmhandle = IM_NULL;
            break;
        case IDSLIB_IOCTL_VMODE_GET_MODE:
            ret = idslib_vmode_get_mode((vmode_handle_t)inst->vmhandle, &vmcfg);
            if(ret == IM_RET_OK){
                s = copy_to_user((void *)arg, (void *)&vmcfg, sizeof(ids_vmode_config_t));
            }
            break;
        case IDSLIB_IOCTL_VMODE_SET_MODE:
            __get_user(ds_vsm.mode, (IM_UINT32 *)(&((ioctl_ds_vmode_set_mode_t *)arg)->mode));
            ds_vsm.ret = idslib_vmode_set_mode((vmode_handle_t)inst->vmhandle, ds_vsm.mode);
            if((ds_vsm.ret == IM_RET_OK) || (ds_vsm.ret == IM_RET_FALSE)){
                __put_user(ds_vsm.ret, (IM_RET *)(&((ioctl_ds_vmode_set_mode_t *)arg)->ret));
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_VMODE_GET_DEVS:
            __get_user(ds_vgds.idsx, (IM_INT32 *)(&((ioctl_ds_vmode_get_devs_t *)arg)->idsx));
            ret = idslib_vmode_get_devs((vmode_handle_t)inst->vmhandle, ds_vgds.idsx, &ds_vgds.num, ds_vgds.devs);
            if(ret == IM_RET_OK){
                s = copy_to_user((void *)(&((ioctl_ds_vmode_get_devs_t *)arg)->devs), (void *)&(ds_vgds.devs), sizeof(ds_vgds.devs));
                __put_user(ds_vgds.num, (IM_INT32 *)(&((ioctl_ds_vmode_get_devs_t *)arg)->num));
            }
            break;
        case IDSLIB_IOCTL_VMODE_SET_DEV:
            s = copy_from_user((void *)&ds_vsd, (void *)arg, sizeof(ioctl_ds_vmode_set_dev_t));
            ret = idslib_vmode_set_dev((vmode_handle_t)inst->vmhandle, ds_vsd.idsx, ds_vsd.devtype, ds_vsd.devid);
            break;
        case IDSLIB_IOCTL_VMODE_SET_CANVAS:
            s = copy_from_user((void *)&ds_vsc, (void *)arg, sizeof(ioctl_ds_vmode_set_canvas_t));
            ds_vsc.ret = idslib_vmode_set_canvas((vmode_handle_t)inst->vmhandle, ds_vsc.idsx, &ds_vsc.canvas);
            if((ds_vsc.ret == IM_RET_OK) || (ds_vsc.ret == IM_RET_FALSE)){
                __put_user(ds_vsc.ret, (IM_RET *)(&((ioctl_ds_vmode_set_canvas_t *)arg)->ret));
                ret = IM_RET_OK;
            }
            break;
        case IDSLIB_IOCTL_IE_INIT:
            __get_user(idsx, (IM_INT32 *)arg);
            ret = idslib_ie_init((ie_handle_t *)&handle, idsx);
            inst->iehandle = handle;
            break;
        case IDSLIB_IOCTL_IE_DEINIT:
            ret = idslib_ie_deinit((ie_handle_t)inst->iehandle);
            inst->iehandle = IM_NULL;
            break;
        case IDSLIB_IOCTL_IE_ACC_SET:
            __get_user(ieval, (IM_INT32 *)arg);
            ret = idslib_ie_acc_set((ie_handle_t)inst->iehandle, ieval);
            break;
        case IDSLIB_IOCTL_IE_ACC_GET:
            ret = idslib_ie_acc_get((ie_handle_t)inst->iehandle, &ieval);
            if(ret == IM_RET_OK){
                __put_user(ieval, (IM_INT32 *)arg);
            }
            break;
        case IDSLIB_IOCTL_IE_ACM_SET:
            __get_user(ieval, (IM_INT32 *)arg);
            ret = idslib_ie_acm_set((ie_handle_t)inst->iehandle, ieval);
            break;
        case IDSLIB_IOCTL_IE_ACM_GET:
            ret = idslib_ie_acm_get((ie_handle_t)inst->iehandle, &ieval);
            if(ret == IM_RET_OK){
                __put_user(ieval, (IM_INT32 *)arg);
            }
            break;
        case IDSLIB_IOCTL_IE_GAMMA_SET:
            __get_user(ieval, (IM_INT32 *)arg);
            ret = idslib_ie_gamma_set((ie_handle_t)inst->iehandle, ieval);
            break;
        case IDSLIB_IOCTL_IE_GAMMA_GET:
            ret = idslib_ie_gamma_get((ie_handle_t)inst->iehandle, &ieval);
            if(ret == IM_RET_OK){
                __put_user(ieval, (IM_INT32 *)arg);
            }
            break;
        case IDSLIB_IOCTL_IE_EE_SET:
            __get_user(ieval, (IM_INT32 *)arg);
            ret = idslib_ie_ee_set((ie_handle_t)inst->iehandle, ieval);
            break;
        case IDSLIB_IOCTL_IE_EE_GET:
            ret = idslib_ie_ee_get((ie_handle_t)inst->iehandle, &ieval);
            if(ret == IM_RET_OK){
                __put_user(ieval, (IM_INT32 *)arg);
            }
            break;
        case IDSLIB_IOCTL_REGISTER_INSTANCE:
            __get_user(tmp, (IM_INT32 *)arg);
            inst->type = tmp;
            break;
        case IDSLIB_IOCTL_GET_EVENT:
            evt = (idslib_event_t *)im_list_begin(inst->evtlst);
            if(evt != IM_NULL){
                s = copy_to_user((void *)arg, (void *)evt, sizeof(idslib_event_t));
                im_list_erase(inst->evtlst, evt);
            }else{
                ret = IM_RET_FAILED;
            }
            break;
        case IDSLIB_IOCTL_SHOW_LOCAL:
            tmp = (gIDSMemCheckFlag & IDS_HDMI_NO_LOCAL) ? 0 : 1;
            __put_user(tmp, (IM_INT32*)arg);
            ret = IM_RET_OK;
            break;
        case IDSLIB_IOCTL_BANK_CHECK:
            s = copy_from_user((void *)&bch, (void *)arg, sizeof(bank_check));
            ret = idsdrv_bank_check_2(&bch);	
            s = copy_to_user((void *)arg, (void *)&bch, sizeof(bank_check));
            break;
        case IDSLIB_IOCTL_BUFFER_REALLOC_CHECK:
            ret = IM_RET_OK;
            tmp = 0;
            if (gIDSMemCheckFlag == IDS_BANK_RESERVE){
                ret = idslib_check_hdmi_on(&tmp);
            }
            __put_user(tmp, (IM_INT32 *)arg);
            break;
        case IDSDRV_IOCTL_ENABLE_WAIT_POST_VSYNC:
            gIdsdrvGlobal->disWaitPostVsync = false;
            ret = IM_RET_OK;
            break;
        case IDSDRV_IOCTL_DISABLE_WAIT_POST_VSYNC:
            gIdsdrvGlobal->disWaitPostVsync = true;
            ret = IM_RET_OK;
            break;
        default:
            IM_ERRMSG((IM_STR("invalid ctrl code 0x%x"), cmd));
            ret = IM_RET_FAILED;
            break;
    }

    mutex_unlock(&gIdsdrvGlobal->acslock);
    inst->gotAcslock = IM_FALSE;

    return (ret==IM_RET_OK)?0:-EFAULT;
}

/*
 * Tutorial:
 * Map private memory to user space, you can define the operation of
 * as you wish.
 */
static int ids_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned int size, addr;
    size = vma->vm_end - vma->vm_start;
    addr = vma->vm_pgoff;	
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    return remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot);
}

/*
 * Tutorial:
 * Poll is most used for user space application to wait for a kernel
 * driver event, like interrupt or harware mission completion.
 */
static unsigned int ids_poll(struct file *file, poll_table *wait)
{
    int mask = 0;
    idsdrv_instance_t *inst = (idsdrv_instance_t *)file->private_data;

    //IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    mutex_lock(&inst->lock);
    if(im_list_size(inst->evtlst) > 0){
        mask = POLLIN | POLLRDNORM;
        goto Quit;
    }
    mutex_unlock(&inst->lock);

    poll_wait(file, &inst->waitQ, wait);

    mutex_lock(&inst->lock);
    if(im_list_size(inst->evtlst) > 0){
        mask = POLLIN | POLLRDNORM;
    }
Quit:
    mutex_unlock(&inst->lock);
    return mask;
}

/*
 * Tutorial:
 * Platform device driver provides suspend/resume support.
 */
static struct platform_driver ids_plat = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "imapx-ids",
    },
    .probe = ids_probe,
    .remove = ids_remove,
#ifndef CONFIG_IDSDRV_EARLY_SUSPEND
    .suspend = ids_suspend,
    .resume = ids_resume,
#endif
};


extern dispdev_cfg_t gLCDConf[];
extern dispdev_cfg_t gDSIConf[];
static IM_RET ids_get_id_from_idstr(IM_INT32 type, char *str, IM_UINT32 *id)
{
    dispdev_cfg_t *tbl;
    IM_INFOMSG((IM_STR("%s(type=%d, str=%s)"), IM_STR(_IM_FUNC_), type, str));

    switch(type){
        case IDS_DISPDEV_TYPE_LCD:
            tbl = gLCDConf;
            break;
        case IDS_DISPDEV_TYPE_DSI:
            tbl = gDSIConf;
            break;
        case IDS_DISPDEV_TYPE_HDMI:
            if (gIdsdrvGlobal->initcfg.locidsx == 0){
                return IM_RET_FAILED;
            }
            if(strcasecmp("HDMI_1080P_60HZ",str) == 0){
                *id = 0;
            }
            else if(strcasecmp("HDMI_720P_60HZ",str) == 0){
                *id = 1;
            }
            else if(strcasecmp("HDMI_VGA_60HZ",str) == 0){
                *id = 2;
            }
            else{
                IM_ERRMSG((IM_STR(" ids hdmi item error ")));
                return IM_RET_FAILED;
            }
            return IM_RET_OK;
        case IDS_DISPDEV_TYPE_TV:
        case IDS_DISPDEV_TYPE_I80:
            return IM_RET_FAILED;
        default:
            return IM_RET_FAILED;
    }

    //
    while(strcasecmp(tbl->idstr, "none")){
        IM_INFOMSG((IM_STR("tbl->idstr = %s"), tbl->idstr));
        if(!strcasecmp(str, tbl->idstr)){
            *id = tbl->devid;
            return IM_RET_OK;
        }
        tbl++;
    }

    return IM_RET_FAILED;
}

static IM_RET ids_init_config(idslib_init_config_t *config)
{
    char keystr[ITEM_MAX_LEN], valstr[ITEM_MAX_LEN];
    int val, i;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //
    if(item_exist("ids.locidsx")){
        val = item_integer("ids.locidsx", 0);
        IM_INFOMSG((IM_STR("ids.locidsx = %d"), val));
        config->locidsx = (val == 0) ? 0 : 1;
    }else{
        IM_WARNMSG((IM_STR("no item --> ids.locidsx, default set to 0")));
        config->locidsx = 0;
    }

    //
    if(item_exist("ids.fb.resvFblayer")){
        val = item_integer("ids.fb.resvFblayer", 0);
        IM_INFOMSG((IM_STR("ids.fb.resvFblayer = %d"), val));
        config->resvFblayer = (val == 0) ? IM_FALSE : IM_TRUE;
    }else{
        IM_WARNMSG((IM_STR("no item --> ids.fb.resvFblayer, default set to 0")));
        config->resvFblayer = IM_FALSE;
    }

    if(config->resvFblayer){
        if(item_exist("ids.fb.pixfmt")){
            item_string(valstr, "ids.fb.pixfmt", 0);
            IM_INFOMSG((IM_STR("ids.fb.pixfmt = %s"), valstr));
            if(!strcasecmp(valstr, "565")){
                config->pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
            }else if(!strcasecmp(valstr, "888")){
                config->pixfmt = IDSLIB_PIXFMT_32BPP_RGB0888;
            }else{
                IM_ERRMSG((IM_STR("invalid item --> ids.fb.pixfmt = %s"), valstr));
                return IM_RET_FAILED;
            }
        }else{
            IM_ERRMSG((IM_STR("no item --> ids.fb.pixfmt")));
            return IM_RET_FAILED;
        }

        if(item_exist("ids.fb.width")){
            val = item_integer("ids.fb.width", 0);
            IM_INFOMSG((IM_STR("ids.fb.width = %d"), val));
            config->fbWidth = val & ~0x07;
        }else{
            IM_ERRMSG((IM_STR("no item --> ids.fb.width")));
            return IM_RET_FAILED;
        }

        if(item_exist("ids.fb.height")){
            val = item_integer("ids.fb.height", 0);
            IM_INFOMSG((IM_STR("ids.fb.height = %d"), val));
            config->fbHeight = val & ~0x03;
        }else{
            IM_ERRMSG((IM_STR("no item --> ids.fb.height")));
            return IM_RET_FAILED;
        }
    }

    //
    if(item_exist("ids.loc.dev.num")){
        val = item_integer("ids.loc.dev.num", 0);
        IM_INFOMSG((IM_STR("ids.loc.dev.num = %d"), val));
        config->locDevNum = ((val < 0) || (val > IDSLIB_DEV_CONFIG_MAX)) ? 1 : val;

        if(item_exist("ids.loc.dev.prefer")){
            val = item_integer("ids.loc.dev.prefer", 0);
            IM_INFOMSG((IM_STR("ids.loc.dev.prefer = %d"), val));
            config->locDevPrefer = ((val < 0) || (val > config->locDevNum)) ? 0 : val;
        }else{
            IM_WARNMSG((IM_STR("no item --> ids.loc.dev.prefer, default set to 0")));
            config->locDevPrefer = 0;
        }
    }else{
        IM_WARNMSG((IM_STR("no item --> ids.loc.dev.num, default set to 0")));
        config->locDevNum = 0;
        config->locDevPrefer = 0;
    }

    //
    for(i=0; i < config->locDevNum; i++){
        sprintf(keystr, "ids.loc.dev%d.interface", i);
        if(item_exist(keystr)){
            item_string(valstr, keystr, 0);
            IM_INFOMSG((IM_STR("%s = %s"), keystr, valstr));
            if(!strcasecmp(valstr, "lcd")){
                config->locDevs[i].type = IDS_DISPDEV_TYPE_LCD;
            }else if(!strcasecmp(valstr, "tvif")){
                config->locDevs[i].type = IDS_DISPDEV_TYPE_TV;
            }else if(!strcasecmp(valstr, "i80")){
                config->locDevs[i].type = IDS_DISPDEV_TYPE_I80;
            }else if(!strcasecmp(valstr, "dsi")){
                config->locDevs[i].type = IDS_DISPDEV_TYPE_DSI;
            }else if(!strcasecmp(valstr, "hdmi")){
                config->locDevs[i].type = IDS_DISPDEV_TYPE_HDMI;
            }else{
                IM_ERRMSG((IM_STR("invalid item --> %s = %s"), keystr, valstr));
                return IM_RET_FAILED;
            }
        }else{
            IM_ERRMSG((IM_STR("no item --> %s"), keystr));
            return IM_RET_FAILED;
        }

        sprintf(keystr, "ids.loc.dev%d.type", i);
        if(item_exist(keystr)){
            item_string(valstr, keystr, 0);
            IM_INFOMSG((IM_STR("%s = %s"), keystr, valstr));
            if(!strcasecmp(valstr, "dynamic")){
                config->locDevs[i].id = IDS_DISPDEV_ID_DYNAMIC;
            }else{
                if(ids_get_id_from_idstr(config->locDevs[i].type, valstr, &config->locDevs[i].id) != IM_RET_OK){
                    IM_ERRMSG((IM_STR("invalid item --> %s = %s"), keystr, valstr));
                    return IM_RET_FAILED;
                }
            }
        }else{
            IM_ERRMSG((IM_STR("no item --> %s"), keystr));
            return IM_RET_FAILED;
        }
    }

    //
    if(item_exist("ids.ext.dev.num")){
        val = item_integer("ids.ext.dev.num", 0);
        IM_INFOMSG((IM_STR("ids.ext.dev.num = %d"), val));
        config->extDevNum = ((val < 0) || (val > IDSLIB_DEV_CONFIG_MAX)) ? 1 : val;

        if(item_exist("ids.ext.dev.prefer")){
            val = item_integer("ids.ext.dev.prefer", 0);
            IM_INFOMSG((IM_STR("ids.ext.dev.prefer = %d"), val));
            config->extDevPrefer = ((val < 0) || (val > config->extDevNum)) ? 0 : val;
        }else{
            IM_WARNMSG((IM_STR("no item --> ids.ext.dev.prefer, default set to 0")));
            config->extDevPrefer = 0;
        }
    }else{
        IM_WARNMSG((IM_STR("no item --> ids.ext.dev.num, default set to 0")));
        config->extDevNum = 0;
        config->extDevPrefer = 0;
    }

    //
    for(i=0; i < config->extDevNum; i++){
        sprintf(keystr, "ids.ext.dev%d.interface", i);
        if(item_exist(keystr)){
            item_string(valstr, keystr, 0);
            IM_INFOMSG((IM_STR("%s = %s"), keystr, valstr));
            if(!strcasecmp(valstr, "lcd")){
                config->extDevs[i].type = IDS_DISPDEV_TYPE_LCD;
            }else if(!strcasecmp(valstr, "tvif")){
                config->extDevs[i].type = IDS_DISPDEV_TYPE_TV;
            }else if(!strcasecmp(valstr, "i80")){
                config->extDevs[i].type = IDS_DISPDEV_TYPE_I80;
            }else if(!strcasecmp(valstr, "dsi")){
                config->extDevs[i].type = IDS_DISPDEV_TYPE_DSI;
            }else if(!strcasecmp(valstr, "hdmi")){
                config->extDevs[i].type = IDS_DISPDEV_TYPE_HDMI;
            }else{
                IM_ERRMSG((IM_STR("invalid item --> %s = %s"), keystr, valstr));
                return IM_RET_FAILED;
            }
        }else{
            IM_ERRMSG((IM_STR("no item --> %s"), keystr));
            return IM_RET_FAILED;
        }

        sprintf(keystr, "ids.ext.dev%d.type", i);
        if(item_exist(keystr)){
            item_string(valstr, keystr, 0);
            IM_INFOMSG((IM_STR("%s = %s"), keystr, valstr));
            if(!strcasecmp(valstr, "dynamic")){
                config->extDevs[i].id = IDS_DISPDEV_ID_DYNAMIC;
            }else{
                if(ids_get_id_from_idstr(config->extDevs[i].type, valstr, &config->extDevs[i].id) != IM_RET_OK){
                    IM_ERRMSG((IM_STR("invalid item --> %s = %s"), keystr, valstr));
                    return IM_RET_FAILED;
                }
            }
        }else{
            IM_ERRMSG((IM_STR("no item --> %s"), keystr));
            return IM_RET_FAILED;
        }
    }

    //
    if(item_exist("memory.freq")){
        val = item_integer("memory.freq", 0);
        IM_INFOMSG((IM_STR("memory.freq = %d"), val));
        if(val >= 533){
            config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_1;
        }else if(val >= 444){
            config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_2;
        }else if (val >= 396){
            config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_3;
        }else{
            config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_4;
        }

        if(item_exist("memory.reduce_en")){
            val = item_integer("memory.reduce_en", 0);
            IM_INFOMSG((IM_STR("memory.reduce_en = %d"), val));
            if(val != 0){
                config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_4;
            }
        }else{
            IM_WARNMSG((IM_STR("no item --> memory.reduce_ne, ignore it")));
        }
    }else{
        IM_WARNMSG((IM_STR("no item --> memory.freq, set busEffic to normal")));
        config->busEffic = IDSLIB_BUS_EFFIC_LEVEL_3;
    }

    //
    IM_INFOMSG((IM_STR("locidsx=%d"), config->locidsx));
    IM_INFOMSG((IM_STR("resvFblayer=%d"), config->resvFblayer));
    IM_INFOMSG((IM_STR("pixfmt=%d"), config->pixfmt));
    IM_INFOMSG((IM_STR("fbWidth=%d"), config->fbWidth));
    IM_INFOMSG((IM_STR("fbHeight=%d"), config->fbHeight));
    IM_INFOMSG((IM_STR("locDevNum=%d"), config->locDevNum));
    IM_INFOMSG((IM_STR("locDevPrefer=%d"), config->locDevPrefer));
    for(i=0; i<config->locDevNum; i++){
        IM_INFOMSG((IM_STR("locDevs[%d].type=%d"), i, config->locDevs[i].type));
        IM_INFOMSG((IM_STR("locDevs[%d].id=0x%x"), i, config->locDevs[i].id));
    }
    IM_INFOMSG((IM_STR("extDevNum=%d"), config->extDevNum));
    IM_INFOMSG((IM_STR("extDevPrefer=%d"), config->extDevPrefer));
    for(i=0; i<config->extDevNum; i++){
        IM_INFOMSG((IM_STR("extDevs[%d].type=%d"), i, config->extDevs[i].type));
        IM_INFOMSG((IM_STR("extDevs[%d].id=0x%x"), i, config->extDevs[i].id));
    }
    IM_INFOMSG((IM_STR("busEffic=%d"), config->busEffic));

    return IM_RET_OK;
}

static void *ids_malloc(int size)
{
    return kmalloc(size, GFP_KERNEL);
}

static int ids_probe(struct platform_device *pdev)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // if initted, retrun directly.
    if(gIdsdrvGlobal != IM_NULL){
        return 0;
    }

    // alloc gIdsdrvGlobal and gIdsdrvResource.
    gIdsdrvGlobal = (idsdrv_global_t *)kmalloc(sizeof(idsdrv_global_t), GFP_KERNEL);
    if(gIdsdrvGlobal == IM_NULL){
        IM_ERRMSG((IM_STR("kmalloc(gIdsdrvGlobal) failed")));
        return IM_RET_FAILED;
    }
    memset((void *)gIdsdrvGlobal, 0, sizeof(idsdrv_global_t));

    gIdsdrvGlobal->disWaitPostVsync = false;
    gIdsdrvGlobal->waitVsync = false;
    init_completion(&gIdsdrvGlobal->comple);

    IM_ASSERT(gIdsdrvResource == IM_NULL);
    gIdsdrvResource = (idsdrv_resource_t *)kmalloc(sizeof(idsdrv_resource_t), GFP_KERNEL);
    if(gIdsdrvResource == IM_NULL){
        IM_ERRMSG((IM_STR("kmalloc(gIdsdrvResource) failed")));
        goto Fail;
    }
    memset((void *)gIdsdrvResource, 0, sizeof(idsdrv_resource_t));
    gIdsdrvResource->pdev = pdev;

    //
    mutex_init(&gIdsdrvGlobal->acslock);
    gIdsdrvGlobal->mpl = im_mpool_init((func_mempool_malloc_t)ids_malloc, (func_mempool_free_t)kfree);
    if(gIdsdrvGlobal->mpl == IM_NULL){
        IM_ERRMSG((IM_STR("im_mpool_init() failed")));
        goto Fail;
    }

    gIdsdrvGlobal->instlst = im_list_init(0, gIdsdrvGlobal->mpl);
    if(gIdsdrvGlobal->instlst == IM_NULL){
        IM_ERRMSG((IM_STR("im_list_init() failed")));
        goto Fail;
    }

    //
    if(ids_init_config(&gIdsdrvGlobal->initcfg) != IM_RET_OK){
        IM_ERRMSG((IM_STR("ids_init_config() failed")));
        goto Fail;
    }
    if(idslib_init(&gIdsdrvGlobal->initcfg) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_init() failed!")));
        goto Fail;
    }

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
    gIdsdrvGlobal->elySuspnd.level = EARLY_SUSPEND_LEVEL_DISABLE_FB; // don't modify carelessly.
    gIdsdrvGlobal->elySuspnd.suspend = ids_early_suspend;
    gIdsdrvGlobal->elySuspnd.resume = ids_late_resume;
    register_early_suspend(&gIdsdrvGlobal->elySuspnd);
#endif

    return 0;
Fail:
    idslib_deinit();
    if(gIdsdrvGlobal->instlst){
        im_list_deinit(gIdsdrvGlobal->instlst);
    }
    if(gIdsdrvGlobal->mpl){
        im_mpool_deinit(gIdsdrvGlobal->mpl);
    }
    if(gIdsdrvResource){
        kfree(gIdsdrvResource);
        gIdsdrvResource = IM_NULL;
    }
    mutex_destroy(&gIdsdrvGlobal->acslock);
    kfree(gIdsdrvGlobal);
    gIdsdrvGlobal = IM_NULL;
    return -EFAULT;
}

static int ids_remove(struct platform_device *pdev)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsdrvGlobal);

    idslib_deinit();
    if(gIdsdrvGlobal->instlst){
        im_list_deinit(gIdsdrvGlobal->instlst);
    }
    if(gIdsdrvGlobal->mpl){
        im_mpool_deinit(gIdsdrvGlobal->mpl);
    }
    if(gIdsdrvResource){
        kfree(gIdsdrvResource);
        gIdsdrvResource = IM_NULL;
    }
    mutex_destroy(&gIdsdrvGlobal->acslock);
    kfree(gIdsdrvGlobal);
    gIdsdrvGlobal = IM_NULL;

    return 0;
}

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ids_early_suspend(struct early_suspend *s)
{
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
    mutex_lock(&gIdsdrvGlobal->acslock);
    gIdsdrvGlobal->suspended = true;
    idslib_suspend();
    mutex_unlock(&gIdsdrvGlobal->acslock);
    IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}
#else
static int ids_suspend(struct platform_device *pdev, pm_message_t state)
{
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
    gIdsdrvGlobal->suspended = true;
    idslib_suspend();
    IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
    return 0;
}
#endif

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ids_late_resume(struct early_suspend *s)
{
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
    mutex_lock(&gIdsdrvGlobal->acslock);
    idslib_resume();
    gIdsdrvGlobal->suspended = false;
    mutex_unlock(&gIdsdrvGlobal->acslock);
    IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}
#else
static int ids_resume(struct platform_device *pdev)
{
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
    idslib_resume();
    gIdsdrvGlobal->suspended = false;
    IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
    return 0;
}
#endif

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
static int __init ids_init(void)
{
    int ret = 0;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    /* Register platform device driver. */
    ret = platform_driver_register(&ids_plat);
    if (unlikely(ret != 0)) {
        IM_ERRMSG((IM_STR(" Register ids platform device driver error, %d."),ret));
        return ret;
    }

    /* Register misc device driver. */
    ret = misc_register(&ids_miscdev);
    if (unlikely(ret != 0)) {
        IM_ERRMSG((IM_STR(" Register ids misc device driver error, %d."),ret));
        return ret;
    }

    return 0;
}
static void __exit ids_exit(void)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    misc_deregister(&ids_miscdev);
    platform_driver_unregister(&ids_plat);
}
module_init(ids_init);
module_exit(ids_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sam of InfoTM");
