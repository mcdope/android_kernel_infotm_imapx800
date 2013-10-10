/***************************************************************************** 
 ** 
 ** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** Use of Infotm's code is governed by terms and conditions 
 ** stated in the accompanying licensing statement. 
 ** 
 ** Description: Implementation file of ids lib, in Kernel call
 **
 ** Author:
 **     Sam Ye<weize_ye@infotm.com>
 **      
 ** Revision History: 
 ** ----------------- 
 ** v1.0.1	Sam@2012/3/20 :  first commit : 
 **   		假设fblayer如果使用，必然调用是在所有其他wlayer之前，这样w0就可以保证
 **     		仅仅会被fblayer使用了。如果不使用fblayer, w0就可以用于wlayer的调用；
 ** v1.0.2	Sam@2012/4/09 :  Test stable except for mmu
 ** v1.0.3	Sam@2012/4/18 :  
 **			1. mmu OK && local layer alpha set supported.
 **			2. add async-flush safe-management.				
 ** v1.0.4	sam@2012/05/08:
 ** 			make device open after IDS open, for HDMI PHY lock need. 
 ** v1.0.6	sam@2012/6/18:
 **			1. hdmi register listener call after vmode init. 
 **			2. add resume&suspend support.
 ** v1.0.7	leo@2012/08/11: readjust.
 **
 ** v1.0.7	leo@2012/08/11: readjust.
 **
 ** v2.0.1   sam@2013/02/25 : structure readjust, move core vmode ids structure to kernel and add extlayer 
 **          support.
 **
 *****************************************************************************/


#include <InfotmMedia.h>
#include <IM_idsapi.h>

#include <ids_lib.h>
#include <ids_pwl.h>
#include <ie_lib.h>

#include <lcd_api.h>
#include <hdmi_api.h>
#include <dsi_api.h>

#include <ids_internal.h>

#include <IM_g2dapi.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IDSLIB_I:"
#define WARNHEAD	"IDSLIB_W:"
#define ERRHEAD		"IDSLIB_E:"
#define TIPHEAD		"IDSLIB_T:"

// if WINMIX used, the picformat should be RGB
#define USE_WINMIX

static IM_UINT32 idslib_get_bpp_from_pixel(IM_UINT32 pixfmt);
static IM_RET idshw_init(IM_INT32 idsx);
static IM_RET idshw_deinit(IM_INT32 idsx);
static void idshw_async_config(IM_INT32 idsx, IM_INT32 wx);
static void idshw_flush_config(IM_INT32 idsx, IM_INT32 wx);
static IM_RET idshw_pm_save_regs(IM_INT32 idsx);
static IM_RET idshw_pm_restore_regs(IM_INT32 idsx);

static IM_RET wlayer_init_internal(wlayer_t *wlayer, IM_INT32 idsx, IM_INT32 wx, IM_BOOL isfblayer);
static IM_RET wlayer_deinit_internal(wlayer_t *wlayer);
static IM_RET wlayer_open_internal(wlayer_t *wlayer);
static IM_RET wlayer_close_internal(wlayer_t *wlayer);

static IM_RET wlayer_open(IM_INT32 idsx, IM_INT32 wx);
static IM_RET wlayer_prepare_close(IM_INT32 idsx, IM_INT32 wx);
static IM_RET wlayer_close(IM_INT32 idsx, IM_INT32 wx);
static IM_RET wlayer_set_canvas(IM_INT32 idsx, ids_wlayer_canvas_t *canvas);
static IM_RET make_default_canvas(IM_INT32 idsx, ids_display_device_t *dev, ids_wlayer_canvas_t *canvas);
static IM_RET device_set_default(IM_INT32 idsx);
static IM_RET device_get_default(IM_INT32 idsx, ids_display_device_t *dev, ids_wlayer_canvas_t *canvas);
static IM_RET device_init(IM_INT32 idsx, IM_INT32 devtype, IM_INT32 devid);
static IM_RET device_register_listener(IM_INT32 idsx, fcbk_dispdev_listener_t listener);
static IM_RET device_open(IM_INT32 idsx);
static IM_RET device_close(IM_INT32 idsx, IM_INT32 ignoreWx, IM_BOOL force);
static IM_RET device_suspend(IM_INT32 idsx);
static IM_RET device_resume(IM_INT32 idsx);
static IM_RET device_listener(dispdev_handle_t handle, IM_UINT32 evt, void *p);
static void  vmode_trigger_event(IM_INT32 evt, IM_INT32 idsx, IM_INT32 wx, void *p, IM_INT32 size);
static void vmode_server_looper(void *p);
static IM_RET fblayer_vmode_listener(IM_UINT32 evt, void *p);
static IM_RET ext_fblayer_create(vmode_t *vmode, ids_vmode_config_t *vmcfg);
static IM_RET ext_fblayer_destroy(vmode_t *vmode);

#ifdef USE_WINMIX
IM_RET idslib_fblayer_winmix_init(fblayer_handle_t handle, ids_wlayer_panorama_t *pano, IM_UINT32 num, IM_INT32 pixfmt);
IM_RET idslib_fblayer_winmix_deinit(fblayer_handle_t handle);
IM_RET idslib_fblayer_winmix_set_panorama(fblayer_handle_t handle,ids_wlayer_canvas_t *canvas);
IM_RET idslib_winmix_reserve_windows(IM_INT32 idsx, IM_INT32 wxs);
IM_RET idslib_winmix_release_windows(IM_INT32 idsx, IM_INT32 wxs);
IM_RET idslib_winmix_open(IM_INT32 idsx, IM_INT32 wxs);
IM_RET idslib_winmix_close(IM_INT32 idsx, IM_INT32 wxs);
#endif

extern IM_RET idsdrv_bank_check_2(bank_check *bh);
extern void* g2dkapi_init(void);
extern IM_RET g2dkapi_deinit(void * inst);
extern IM_RET g2dkapi_do_scale(void *inst, g2d_image_t *src, g2d_image_t *dst, IM_INT32 edgeSharpValue);
static void *g2dinst = IM_NULL;


static ids_instance_t *gIdsInst = IM_NULL;


static dispdev_interface_t DEVINTFTBL[IDS_DISPDEV_TYPE_MAX] = {
    {// lcd
        lcd_init,
        lcd_deinit,
        lcd_set_config,
        lcd_get_config,
        lcd_open,
        lcd_close,
        lcd_register_listener,
        lcd_suspend,
        lcd_resume
    },
    {// tv
        0
    },
    {// i80
        0
    },
    {// dsi
        dsi_init,
        dsi_deinit,
        dsi_set_config,
        dsi_get_config,
        dsi_open,
        dsi_close,
        dsi_register_listener,
        dsi_suspend,
        dsi_resume
    },
    {// hdmi
        hdmi_init,
        hdmi_deinit,
        hdmi_set_video_basic_config,
        hdmi_get_video_basic_config,
        hdmi_open,
        hdmi_close,
        hdmi_register_listener,
        hdmi_suspend,
        hdmi_resume
    },
};


#define MODULE_LOCIDS		((gIdsInst->locidsx == 0) ? MODULE_IDS0 : MODULE_IDS1)
#define MODULE_EXTIDS		((gIdsInst->extidsx == 0) ? MODULE_IDS0 : MODULE_IDS1)
#define MODULE_IDSx(idsx)	(((idsx) == 0) ? MODULE_IDS0 : MODULE_IDS1)

#define VALID_IDSx(idsx)	((((idsx)==0) || ((idsx==1))) ? IM_TRUE : IM_FALSE)
#define VALID_Wx(wx)		(((wx < 0) || (wx > 3)) ? IM_FALSE : IM_TRUE)

#define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#define MIN(a, b)		(((a) < (b)) ? (a) : (b))


IM_UINT32 idslib_version(void)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    return IM_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_PATCH);
}

IM_RET idslib_init(idslib_init_config_t *config)
{
    IM_INT32 i, j;
    dispdev_t *dev;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsInst == IM_NULL);

    // check parameter.
    if(config == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer!")));
        return IM_RET_INVALID_PARAMETER;
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
        IM_INFOMSG((IM_STR("locDevs[%d].type=%d, id=0x%x"), i, config->locDevs[i].type, config->locDevs[i].id));
    }
    IM_INFOMSG((IM_STR("extDevNum=%d"), config->extDevNum));
    IM_INFOMSG((IM_STR("extDevPrefer=%d"), config->extDevPrefer));
    for(i=0; i<config->extDevNum; i++){
        IM_INFOMSG((IM_STR("extDevs[%d].type=%d, id=0x%x"), i, config->extDevs[i].type, config->extDevs[i].id));
    }

    // check config's validation.
    if(!VALID_IDSx(config->locidsx)){
        IM_ERRMSG((IM_STR("invalid parameter, config->locidsx=%d"), config->locidsx));
        return IM_RET_INVALID_PARAMETER;
    }
    if((config->locDevNum < 0) || (config->locDevPrefer < 0)){
        IM_ERRMSG((IM_STR("invalid parameter, config->locDevNum=%d, config->locDevPrefer=%d"), config->locDevNum, config->locDevPrefer));
        return IM_RET_INVALID_PARAMETER;
    }else if((config->locDevNum > 0) && (config->locDevPrefer >= config->locDevNum)){
        IM_ERRMSG((IM_STR("invalid parameter, config->locDevNum=%d, config->locDevPrefer=%d"), config->locDevNum, config->locDevPrefer));
        return IM_RET_INVALID_PARAMETER;
    }

    if(config->extDevNum < 0){
        IM_ERRMSG((IM_STR("invalid parameter, config->extDevNum=%d, config->extDevPrefer=%d"), config->extDevNum, config->extDevPrefer));
        return IM_RET_INVALID_PARAMETER;
    }else if((config->extDevNum > 0) && (config->extDevPrefer >= config->extDevNum)){
        IM_ERRMSG((IM_STR("invalid parameter, config->extDevNum=%d, config->extDevPrefer=%d"), config->extDevNum, config->extDevPrefer));
        return IM_RET_INVALID_PARAMETER;
    }

    // init pwl.
    if(idspwl_init() != IM_RET_OK){
        IM_ERRMSG((IM_STR("idspwl_init() failed!")));
        return IM_RET_FAILED;
    }

    // initialize ids instance.
    gIdsInst = (ids_instance_t *)idspwl_malloc(sizeof(ids_instance_t));
    if(gIdsInst == IM_NULL){
        IM_ERRMSG((IM_STR("malloc(gIdsInst) failed")));
        goto Fail;
    }
    idspwl_memset((void *)gIdsInst, 0, sizeof(ids_instance_t));
    gIdsInst->mode |= config->resvFblayer ? IDS_MODE_FB_L_to_E : IDS_MODE_FB_to_L_E;
    gIdsInst->locidsx = config->locidsx;
    gIdsInst->extidsx = (config->locidsx == 0) ? 1: 0;

    // init mempool.
    gIdsInst->mpl = im_mpool_init((func_mempool_malloc_t)idspwl_malloc, (func_mempool_free_t)idspwl_free);
    if(gIdsInst->mpl == IM_NULL){
        IM_ERRMSG((IM_STR("im_mpool_init() failed")));
        goto Fail;
    }

    // init idsMgr.
    for(i=0; i<2; i++){
        gIdsInst->idsMgr[i].resource = RES_ALL;
    }	

    // init vmode.
    gIdsInst->vmlock = idspwl_lock_init();
    if(gIdsInst->vmlock == IM_NULL){
        IM_ERRMSG((IM_STR("idspwl_lock_init() failed")));
        goto Fail;
    }

    // init devs.
    for(i=0; i<config->locDevNum; i++){
        if((config->locDevs[i].type < IDS_DISPDEV_TYPE_LCD) || (config->locDevs[i].type > IDS_DISPDEV_TYPE_HDMI)){
            IM_ERRMSG((IM_STR("invalid parameter, devtype=%d"), config->locDevs[i].type));
            continue;
        }

        // now we only support these dev type.
        if((config->locDevs[i].type != IDS_DISPDEV_TYPE_LCD) && (config->locDevs[i].type != IDS_DISPDEV_TYPE_HDMI) &&
                (config->locDevs[i].type != IDS_DISPDEV_TYPE_DSI)){
            IM_ERRMSG((IM_STR("Now don't support this devtype=%d"), config->locDevs[i].type));
            continue;	
        }

        dev = &gIdsInst->devs[gIdsInst->locidsx][config->locDevs[i].type];
        if(dev->intf == IM_NULL){
            dev->type = config->locDevs[i].type;
            dev->idsx = gIdsInst->locidsx;
            dev->dynamic = (config->locDevs[i].id == IDS_DISPDEV_ID_DYNAMIC) ? IM_TRUE : IM_FALSE;
            dev->suffixStart = i;
            dev->intf = &DEVINTFTBL[dev->type];
            if(dev->intf->device_init(&dev->handle, gIdsInst->locidsx) != IM_RET_OK){
                IM_ERRMSG((IM_STR("devtype=%d device_init() failed"), dev->type));
                goto Fail;
            }
            dev->flag |= DISPDEV_FLAG_INITED;
            dev->cfglst = im_list_init(sizeof(ids_display_device_t), gIdsInst->mpl);
            if(dev->cfglst == IM_NULL){
                IM_ERRMSG((IM_STR("im_list_init() failed")));
                goto Fail;
            }
        }
        if(!dev->dynamic){
            if(dev->intf->device_get_config(dev->handle, &config->locDevs[i], IM_NULL, i) != IM_RET_OK){
                IM_ERRMSG((IM_STR("devtype=%d device_get_config(id=0x%x) failed"), config->locDevs[i].type, config->locDevs[i].id));
                goto Fail;
            }
            im_list_put_back(dev->cfglst, (void *)&config->locDevs[i]);
        }
    }
    for(i=0; i<config->extDevNum; i++){
        if((config->extDevs[i].type < IDS_DISPDEV_TYPE_LCD) || (config->extDevs[i].type > IDS_DISPDEV_TYPE_HDMI)){
            IM_ERRMSG((IM_STR("invalid parameter, devtype=%d"), config->extDevs[i].type));
            continue;	
        }

        // now we only support these dev type.
        if((config->extDevs[i].type != IDS_DISPDEV_TYPE_LCD) && (config->extDevs[i].type != IDS_DISPDEV_TYPE_HDMI)){
            IM_ERRMSG((IM_STR("Now don't support this devtype=%d"), config->extDevs[i].type));
            continue;
        }

        dev = &gIdsInst->devs[gIdsInst->extidsx][config->extDevs[i].type];
        if(dev->intf == IM_NULL){
            dev->type = config->extDevs[i].type;
            dev->idsx = gIdsInst->extidsx;
            dev->dynamic = (config->extDevs[i].id == IDS_DISPDEV_ID_DYNAMIC) ? IM_TRUE : IM_FALSE;
            dev->suffixStart = i;
            dev->intf = &DEVINTFTBL[dev->type];
            if(dev->intf->device_init(&dev->handle, gIdsInst->extidsx) != IM_RET_OK){
                IM_ERRMSG((IM_STR("devtype=%d device_init() failed"), dev->type));
                goto Fail; // sam : If extIdsx = 0, hdmi_init failed. just goto Fail ? 
            }
            dev->flag |= DISPDEV_FLAG_INITED;
            dev->cfglst = im_list_init(sizeof(ids_display_device_t), gIdsInst->mpl);
            if(dev->cfglst == IM_NULL){
                IM_ERRMSG((IM_STR("im_list_init() failed")));
                goto Fail;
            }
        }
        if(!dev->dynamic){
            if(dev->intf->device_get_config(dev->handle, &config->extDevs[i], IM_NULL, i) != IM_RET_OK){
                IM_ERRMSG((IM_STR("devtype=%d device_get_config(id=0x%x) failed"), config->extDevs[i].type, config->extDevs[i].id));
                goto Fail;
            }
            im_list_put_back(dev->cfglst, (void *)&config->extDevs[i]);
        }
    }

    //
    gIdsInst->effic.busEffic = config->busEffic;
    gIdsInst->effic.workMode[0] = IDSLIB_WORKMODE_IDLE;
    gIdsInst->effic.workMode[1] = IDSLIB_WORKMODE_IDLE;

    //
    idspwl_memcpy((void *)&gIdsInst->config, (void *)config, sizeof(idslib_init_config_t));
    return IM_RET_OK;

Fail:
    for(i=0; i<2; i++){
        for(j=0; j<IDS_DISPDEV_TYPE_MAX; j++){
            dev = &gIdsInst->devs[i][j];
            if(dev->flag & DISPDEV_FLAG_INITED){
                dev->intf->device_deinit(dev->handle);
                dev->handle = IM_NULL;
                dev->flag &= ~DISPDEV_FLAG_INITED;
            }
            if(gIdsInst->devs[i][j].cfglst){
                im_list_clear(gIdsInst->devs[i][j].cfglst);
                im_list_deinit(gIdsInst->devs[i][j].cfglst);
            }
        }
    }
    if(gIdsInst->vmlock){
        idspwl_lock_deinit(gIdsInst->vmlock);
    }
    if(gIdsInst->mpl){
        im_mpool_deinit(gIdsInst->mpl);
    }
    idspwl_free(gIdsInst);
    gIdsInst = IM_NULL;
    idspwl_deinit();

    return IM_RET_FAILED;
}

IM_RET idslib_deinit(void)
{
    IM_INT32 i, j;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(gIdsInst == IM_NULL){
        return IM_RET_OK;
    }

    for(i=0; i<4; i++){
        wlayer_deinit_internal((wlayer_handle_t)&gIdsInst->idsMgr[0].wlayer[i]);
        wlayer_deinit_internal((wlayer_handle_t)&gIdsInst->idsMgr[1].wlayer[i]);
    }

    idslib_vmode_deinit((vmode_handle_t)&gIdsInst->vmode);

    for(i=0; i<2; i++){
        for(j=0; j<IDS_DISPDEV_TYPE_MAX; j++){
            if(gIdsInst->devs[i][j].cfglst){
                im_list_clear(gIdsInst->devs[i][j].cfglst);
                im_list_deinit(gIdsInst->devs[i][j].cfglst);
            }
        }
    }

    if(gIdsInst->vmlock){
        idspwl_lock_deinit(gIdsInst->vmlock);
    }
    if(gIdsInst->mpl){
        im_mpool_deinit(gIdsInst->mpl);
    }
    idspwl_free(gIdsInst);
    gIdsInst = IM_NULL;
    idspwl_deinit();

    return IM_RET_OK;
}

IM_RET idslib_get_local_ids(IM_INT32 *idsx)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsInst != IM_NULL);

    // check parameter.
    if(idsx == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }

    //
    *idsx = gIdsInst->locidsx;
    return IM_RET_OK;
}

IM_RET idslib_vsync(IM_INT32 idsx)
{
    if(gIdsInst->idsMgr[idsx].dev && (gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED) &&
            !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_BREAKED)){
        return idspwl_vsync(idsx);
    }
    return IM_RET_FAILED;
}

IM_RET idslib_suspend(void)
{
    IM_INT32 i, j;
    pm_t *pm = &gIdsInst->pm;
    dispdev_t *dd;
    wlayer_t *wlayer;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    idspwl_lock_lock(gIdsInst->vmlock);

    gIdsInst->suspendFlag = IM_TRUE;
    // first all init to 0.
    idspwl_memset((void *)pm, 0, sizeof(pm_t));

#ifdef USE_WINMIX
    idslib_winmix_close(gIdsInst->fblayer.wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
    if ((gIdsInst->mode & IDS_MODE_VMODE_USE) && VMODE_FLAG_CHECK_EXT_VALID(gIdsInst->vmode.flag)){
        idslib_winmix_close(gIdsInst->vmode.extfblayer->wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
    }
#endif
    // wlayer prepare close.
    for(i=0; i<2; i++){
        for(j=0; j<4; j++){
            wlayer = &gIdsInst->idsMgr[i].wlayer[j];
            if(wlayer->flag & WLAYER_FLAG_OPENED){
                wlayer_prepare_close(wlayer->idsx, wlayer->wx);
            }
        }
    }

    // device suspend.
    for(i=0; i<2; i++){
        dd = gIdsInst->idsMgr[i].dev;
        if(dd && (dd->flag & DISPDEV_FLAG_CONFIGURED)){
            device_suspend(i);// suspend will do close.
            if(dd->flag & DISPDEV_FLAG_OPENED){
                // here we don't call device_close(), but interrupt must be disabled 
                // for enable interrupt in resume(indirectly called from device_open().
                idspwl_disable_frame_intr(i);

                dd->flag &= ~DISPDEV_FLAG_OPENED;
                pm->devFlagStore[i] |= PM_DEVFLAG_OPEN;
            }
            dd->flag |= DISPDEV_FLAG_SUSPEND;
        }
    }

    // dynamic device which is in listening do suspend().
    for(i=0; i<2; i++){
        for(j=0; j<IDS_DISPDEV_TYPE_MAX; j++){
            dd = &gIdsInst->devs[i][j];
            if(dd && (dd->flag & DISPDEV_FLAG_LISTENING) && !(dd->flag & DISPDEV_FLAG_SUSPEND)){
                dd->intf->device_suspend(dd->handle);
                dd->flag |= DISPDEV_FLAG_SUSPEND;
            }
        }
    }

    // wlayer.
    for(i=0; i<2; i++){
        for(j=0; j<4; j++){
            wlayer = &gIdsInst->idsMgr[i].wlayer[j];
            if(wlayer->flag & WLAYER_FLAG_OPENED){
                wlayer_close(wlayer->idsx, wlayer->wx);
                wlayer->flag &= ~WLAYER_FLAG_OPENED;

                pm->wlFlagStore[i][j] |= PM_WLFLAG_OPEN;
            }
            if(wlayer->rsvdRes & (RES_DEVMMU_W0 << j)){
                idspwl_deinit_mmu(wlayer->idsx, wlayer->wx);
                wlayer->flag &= ~WLAYER_FLAG_MMU_INITED;
            }
        }
    }

    // ie's suspend().
    for(i = 0; i < 2; i ++){
        if(gIdsInst->ie[i].flag & IE_FLAG_INITED){
            ielib_suspend(gIdsInst->ie[i].handle);	
        }
    }

    // idshw deinit.
    for(i=0; i<2; i++){
        if(gIdsInst->idsMgr[i].flag & IDSMGR_FLAG_HW_INITED){
            idshw_pm_save_regs(i);
            idshw_deinit(i);
            gIdsInst->idsMgr[i].flag &= ~IDSMGR_FLAG_HW_INITED;
            pm->idshwFlagStore[i] |= PM_IDSHWFLAG_INIT;
        }
    }
    idspwl_suspend();

    if (g2dinst != IM_NULL){
        g2dkapi_deinit(g2dinst);
        g2dinst = IM_NULL;
        gIdsInst->G2DSuspendInit = IM_TRUE;
    }

    idspwl_lock_unlock(gIdsInst->vmlock);

    return IM_RET_OK;
}

IM_RET idslib_resume(void)
{
    IM_INT32 i, j;
    pm_t *pm = &gIdsInst->pm;
    dispdev_t *dd;
    wlayer_t *wlayer;
    IM_BOOL mapEnable[2] = {IM_FALSE, IM_FALSE};
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    idspwl_lock_lock(gIdsInst->vmlock);

    if (g2dinst == IM_NULL && gIdsInst->G2DSuspendInit){
        g2dinst = g2dkapi_init();
        if (g2dinst == IM_NULL){
            IM_ERRMSG((IM_STR(" iidslib_resume : g2dkapi_init failed -!!!")));
        }
        gIdsInst->G2DSuspendInit = IM_FALSE;
    }

    idspwl_resume();
    // idshw init.
    for(i=0; i<2; i++){
        if(pm->idshwFlagStore[i] & PM_IDSHWFLAG_INIT){
            idshw_init(i);
            gIdsInst->idsMgr[i].flag |= IDSMGR_FLAG_HW_INITED;
            idshw_pm_restore_regs(i);		
            pm->idshwFlagStore[i] &= ~PM_IDSHWFLAG_INIT;
        }
    }

    //
    if(pm->otherFlag & PM_OTHERFLAG_FB_SWAP_BUFFER){
        vmode_trigger_event(IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER, -1, -1, 
                (void *)&gIdsInst->fblayer.showBuffer, sizeof(IM_Buffer));
        pm->otherFlag &= ~PM_OTHERFLAG_FB_SWAP_BUFFER;
    }

    // wlayer.
    for(i=0; i<2; i++){
        for(j=0; j<4; j++){
            wlayer = &gIdsInst->idsMgr[i].wlayer[j];
            if(wlayer->rsvdRes & (RES_DEVMMU_W0 << j)){
                idspwl_init_mmu(wlayer->idsx, wlayer->wx, wlayer->rsvdRes & RES_YUVtoRGB_MATRIX);
                wlayer->flag |= WLAYER_FLAG_MMU_INITED;
            }
            if(pm->wlFlagStore[i][j] & PM_WLFLAG_OPEN){
                wlayer_open(wlayer->idsx, wlayer->wx);
                wlayer->flag |= WLAYER_FLAG_OPENED;

                pm->wlFlagStore[i][j] &= ~PM_WLFLAG_OPEN;
            }
            if (wlayer->flag & WLAYER_FLAG_MMU_INITED){
                // Test shows if open device immediately after mmu enable. mmu will not work.
                // so map color for a while (for one frame period.)
                idspwl_write_reg(MODULE_IDSx(i), OVCW1CMR , 0x1000000);
                if (i == 0){
                    mapEnable[0] = IM_TRUE;
                }else {
                    mapEnable[1] = IM_TRUE;
                }
            }
        }
    }
#ifdef USE_WINMIX
    idslib_winmix_open(gIdsInst->fblayer.wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
    if (gIdsInst->mode & IDS_MODE_VMODE_USE && VMODE_FLAG_CHECK_EXT_VALID(gIdsInst->vmode.flag)){
        idslib_winmix_open(gIdsInst->vmode.extfblayer->wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
    }
#endif

    // device resume.
    for(i=0; i<2; i++){
        dd = gIdsInst->idsMgr[i].dev;
        if(dd && (dd->flag & DISPDEV_FLAG_CONFIGURED)){
            if(dd->flag & DISPDEV_FLAG_SUSPEND){
                device_resume(i);
                dd->flag &= ~DISPDEV_FLAG_SUSPEND;
            }
        }
    }

    // dynamic device which is in listening do resume().
    for(i=0; i<2; i++){
        for(j=0; j<IDS_DISPDEV_TYPE_MAX; j++){
            dd = &gIdsInst->devs[i][j];
            if(dd && (dd->flag & DISPDEV_FLAG_LISTENING) && (dd->flag & DISPDEV_FLAG_SUSPEND)){
                dd->intf->device_resume(dd->handle);
                dd->flag &= ~DISPDEV_FLAG_SUSPEND;
            }
        }
    }

    // device open.
    for(i=0; i<2; i++){
        dd = gIdsInst->idsMgr[i].dev;
        if(dd && (dd->flag & DISPDEV_FLAG_CONFIGURED)){
            if(pm->devFlagStore[i] & PM_DEVFLAG_OPEN){
                if(device_open(i) != IM_RET_OK){
                    dd->flag |= DISPDEV_FLAG_BREAKED;
                }
                dd->flag |= DISPDEV_FLAG_OPENED; // still OPENED.
                pm->devFlagStore[i] &= ~PM_DEVFLAG_OPEN;
            }
        }
    }

    // ie's resume().
    for(i = 0; i < 2; i ++){
        if (gIdsInst->ie[i].flag & IE_FLAG_INITED){
            ielib_resume(gIdsInst->ie[i].handle);
        }
        if (mapEnable[i] == IM_TRUE){
            idspwl_vsync(i);
            idspwl_write_reg(MODULE_IDSx(i), OVCW1CMR , 0);
        }
    }

    gIdsInst->suspendFlag = IM_FALSE;
    idspwl_lock_unlock(gIdsInst->vmlock);

    return IM_RET_OK;
}

IM_RET idslib_fblayer_init(fblayer_handle_t *handle)
{
    fblayer_t *fblayer;
    ids_wlayer_panorama_t pano;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsInst != IM_NULL);
    IM_ASSERT(gIdsInst->config.resvFblayer);

    // check parameter.
    if(handle == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    // check if fblayer was employed.
    fblayer = &gIdsInst->fblayer;
    if(fblayer->flag & FBLAYER_FLAG_INITED){
        IM_ERRMSG((IM_STR("fblayer has been employed")));
        return IM_RET_FAILED;
    }

    //
    idspwl_memset((void *)fblayer, 0, sizeof(fblayer_t));
    if(wlayer_init_internal(&gIdsInst->idsMgr[gIdsInst->locidsx].wlayer[0], gIdsInst->locidsx, 0/*fixed to 0*/, IM_TRUE) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_init_internal() failed")));
        return IM_RET_FAILED;
    }
    fblayer->wlayer = &gIdsInst->idsMgr[gIdsInst->locidsx].wlayer[0];

    memset((void*)&pano, 0, sizeof(ids_wlayer_panorama_t));
    pano.imgWidth = gIdsInst->config.fbWidth;
    pano.imgHeight = gIdsInst->config.fbHeight;
    pano.pos.left = 0;
    pano.pos.top = 0;
    pano.pos.width = pano.imgWidth;
    pano.pos.height = pano.imgHeight;
    if(idslib_wlayer_set_panorama((wlayer_handle_t)fblayer->wlayer, gIdsInst->config.pixfmt, &pano) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_panorama() failed")));
        goto Fail;
    }

    // mark resource is reserved.
    fblayer->flag = FBLAYER_FLAG_INITED;

#ifdef USE_WINMIX
    //  1<<1(win1) & 1<<2(win2) & 1<<3(win3);
    if (idslib_fblayer_winmix_init((fblayer_handle_t)fblayer, &pano, 3, gIdsInst->config.pixfmt) != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_init failed - \n")));
        return IM_RET_FAILED;
    }
    if (idslib_fblayer_winmix_set_panorama((fblayer_handle_t)fblayer, IM_NULL) != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_set_panorama failed - \n")));
        return IM_RET_FAILED;
    }
#endif

    *handle = (fblayer_handle_t)fblayer;
    return IM_RET_OK;
Fail:
    wlayer_deinit_internal((wlayer_handle_t)fblayer->wlayer);
    fblayer->wlayer = IM_NULL;
    *handle = IM_NULL;
    return IM_RET_FAILED;
}

IM_RET idslib_fblayer_deinit(fblayer_handle_t handle)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((fblayer == IM_NULL) || (fblayer != &gIdsInst->fblayer)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(fblayer->flag & FBLAYER_FLAG_INITED)){
        IM_INFOMSG((IM_STR("fblayer is not inited")));
        return IM_RET_OK;
    }

    //
    if(fblayer->flag & FBLAYER_FLAG_OPENED){
        if(idslib_fblayer_close(handle)){
            IM_ERRMSG((IM_STR("idslib_fblayer_close() failed")));
            return IM_RET_FAILED;
        }
    }

    if(wlayer_deinit_internal((wlayer_handle_t)fblayer->wlayer) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_deinit_internal() failed")));
        return IM_RET_FAILED;	
    }

    fblayer->wlayer = IM_NULL;
    fblayer->flag = 0;

    return IM_RET_OK;
}

IM_RET idslib_fblayer_open(fblayer_handle_t handle)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((fblayer == IM_NULL) || (fblayer != &gIdsInst->fblayer)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(fblayer->flag & FBLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("fblayer has not been inited")));
        return IM_RET_FAILED;
    }

    //
    if(fblayer->flag & FBLAYER_FLAG_OPENED){
        return IM_RET_OK;
    }
    if(!(fblayer->flag & FBLAYER_BUFFER_SET)){
        IM_ERRMSG((IM_STR("buffer not setup")));
        return IM_RET_FAILED;
    }

    if(wlayer_open_internal((wlayer_handle_t)fblayer->wlayer) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_open_internal() failed")));
        return IM_RET_FAILED;
    }
    fblayer->flag |= FBLAYER_FLAG_OPENED;

#ifdef USE_WINMIX 
    idslib_winmix_open(fblayer->wlayer->idsx, fblayer->wmix.wxs);
#endif

    return IM_RET_OK;
}

IM_RET idslib_fblayer_close(fblayer_handle_t handle)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((fblayer == IM_NULL) || (fblayer != &gIdsInst->fblayer)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(fblayer->flag & FBLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("fblayer has not been inited")));
        return IM_RET_FAILED;
    }

#ifdef USE_WINMIX 
    idslib_winmix_close(fblayer->wlayer->idsx, fblayer->wmix.wxs);
#endif
    //
    if(fblayer->flag & FBLAYER_FLAG_OPENED){
        if(wlayer_close_internal((wlayer_handle_t)fblayer->wlayer)){
            IM_ERRMSG((IM_STR("wlayer_close_internal() for fblayer failed")));
            return IM_RET_FAILED;	
        }
        fblayer->flag &= ~FBLAYER_FLAG_OPENED;
    }

    return IM_RET_OK;	
}

static IM_RET fblayer_hide(IM_BOOL hide)
{
    wlayer_t *wlayer = gIdsInst->fblayer.wlayer;
    IM_INFOMSG((IM_STR("%s(hide=%d)"), IM_STR(_IM_FUNC_), hide));

    if(hide == IM_TRUE){
        if(wlayer->flag & WLAYER_FLAG_OPENED){
#ifdef USE_WINMIX
            idslib_winmix_close(gIdsInst->fblayer.wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
#endif
            wlayer_prepare_close(wlayer->idsx, wlayer->wx);

            if(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED){
                if(device_close(wlayer->idsx, wlayer->wx, IM_FALSE) == IM_RET_OK){
                    gIdsInst->idsMgr[wlayer->idsx].dev->flag &= ~DISPDEV_FLAG_OPENED;
                }else{
                    IM_ERRMSG((IM_STR("device_open() failed")));
                    return IM_RET_FAILED;
                }
            }

            if(wlayer_close(wlayer->idsx, wlayer->wx) == IM_RET_OK){
                wlayer->flag &= ~WLAYER_FLAG_OPENED;
            }else{
                IM_ERRMSG((IM_STR("wlayer_open() failed")));
                return IM_RET_FAILED;
            }
        }
    }else{
        if(!(wlayer->flag & WLAYER_FLAG_OPENED)){
            if(wlayer_open(wlayer->idsx, wlayer->wx) != IM_RET_OK){
                IM_ERRMSG((IM_STR("wlayer_open() failed")));
                goto Fail;
            }
            wlayer->flag |= WLAYER_FLAG_OPENED;
#ifdef USE_WINMIX
            idslib_winmix_open(gIdsInst->fblayer.wlayer->idsx, gIdsInst->fblayer.wmix.wxs);
#endif

            if(!(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED)){
                if(device_open(wlayer->idsx) != IM_RET_OK){
                    IM_ERRMSG((IM_STR("device_open() failed")));
                    goto Fail;
                }
                gIdsInst->idsMgr[wlayer->idsx].dev->flag |= DISPDEV_FLAG_OPENED;
            }
        }
    }

    return IM_RET_OK;

Fail:
    if(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED){
        if(device_close(wlayer->idsx, wlayer->wx, IM_FALSE) == IM_RET_OK){
            gIdsInst->idsMgr[wlayer->idsx].dev->flag &= ~DISPDEV_FLAG_OPENED;
        }
    }
    if(wlayer->flag & WLAYER_FLAG_OPENED){
        wlayer_close(wlayer->idsx, wlayer->wx);
        wlayer->flag &= ~WLAYER_FLAG_OPENED;
    }

    return IM_RET_FAILED;
}

IM_RET idslib_fblayer_swap_buffer(fblayer_handle_t handle, IM_Buffer *buffer)
{
    IM_INT32 OVCWxB0SAR=0;
    IM_UINT32 i, phy_addr=0x40000000;
    fblayer_t *fblayer = (fblayer_t *)handle;

    //IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // use assert but check for performance consideration.
    IM_ASSERT(buffer != IM_NULL);
    IM_ASSERT(buffer->phy_addr != 0);
    IM_ASSERT((fblayer != IM_NULL) && (fblayer == &gIdsInst->fblayer));
    IM_ASSERT(fblayer->flag & FBLAYER_FLAG_INITED);

    //
    if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
        gIdsInst->pm.osdRegStore[fblayer->wlayer->idsx][OVCW0B0SAR] = buffer->phy_addr;
    }else{
        idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCW0B0SAR, buffer->phy_addr);
    }
    idspwl_memcpy((void *)&fblayer->showBuffer, (void *)buffer, sizeof(IM_Buffer));
    fblayer->flag |= FBLAYER_BUFFER_SET;

#ifdef USE_WINMIX
    for ( i = 0; i < fblayer->wmix.num; i ++){
        switch(i){
            case 0:
                OVCWxB0SAR = OVCW1B0SAR;
                phy_addr = buffer->phy_addr + fblayer->wmix.vmoffset[0];
                break;
            case 1:
                OVCWxB0SAR = OVCW2B0SAR;
                phy_addr = buffer->phy_addr + fblayer->wmix.vmoffset[1];
                break;
            case 2:
                OVCWxB0SAR = OVCW3B0SAR;
                phy_addr = buffer->phy_addr + fblayer->wmix.vmoffset[2];
                break;
            default:
                IM_ASSERT(IM_FALSE);
                break;
        }
        if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
            gIdsInst->pm.osdRegStore[fblayer->wlayer->idsx][OVCWxB0SAR] = phy_addr;
        }else{
            idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxB0SAR, phy_addr);
        }
    }
#endif
    if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
        return IM_RET_OK;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);
    if(VMODE_FLAG_CHECK_EXT_VALID(gIdsInst->vmode.flag))
    {
        if((gIdsInst->idsMgr[gIdsInst->extidsx].dev->flag & DISPDEV_FLAG_BREAKED)){
            IM_INFOMSG((IM_STR("BREAKED or !EXTFB_OK, don't swap extfb buffer")));
            idspwl_lock_unlock(gIdsInst->vmlock);
            return IM_RET_OK;
        }else if(gIdsInst->idsMgr[gIdsInst->extidsx].dev->flag & DISPDEV_FLAG_SUSPEND){
            IM_INFOMSG((IM_STR("SUSPEND, don't swap extfb buffer")));
            gIdsInst->pm.otherFlag |= PM_OTHERFLAG_FB_SWAP_BUFFER;
            idspwl_lock_unlock(gIdsInst->vmlock);
            return IM_RET_OK;
        }
        vmode_trigger_event(IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER, -1, -1, (void *)buffer, sizeof(IM_Buffer)); // if IM_RET_FALSE, user must wait ext-fblayer swap buffer finish.
        gIdsInst->pm.otherFlag &= ~PM_OTHERFLAG_FB_SWAP_BUFFER;
        idspwl_lock_unlock(gIdsInst->vmlock);
        return IM_RET_FALSE;
    }
    idspwl_lock_unlock(gIdsInst->vmlock);

    return IM_RET_OK;
}

IM_RET idslib_fblayer_vsync(fblayer_handle_t handle)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    //IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters, use assert but "if".
    IM_ASSERT((fblayer != IM_NULL) && (fblayer == &gIdsInst->fblayer));
    IM_ASSERT(fblayer->flag & FBLAYER_FLAG_INITED);

    return idslib_wlayer_vsync((wlayer_handle_t)fblayer->wlayer);
}

#ifdef USE_WINMIX
IM_RET idslib_fblayer_winmix_init(fblayer_handle_t handle, ids_wlayer_panorama_t *pano, IM_UINT32 num, IM_INT32 pixfmt) 
{
    IM_INT32 wxs, bpp;
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s(),num=%d"), IM_STR(_IM_FUNC_),num));
    IM_INFOMSG((IM_STR(" pixfmt=%d,pano->imgWidth=%d, imgHeight=%d"),pixfmt,pano->imgWidth,pano->imgHeight));

    if (num == 3){
        wxs = (1<<1) | (1<<2) | (1<<3);
    }else if (num == 2){
        wxs = (1<<1) | (1<<2);
    }else if (num == 1){
        wxs = 1<<1;
    } else {
        return IM_RET_INVALID_PARAMETER;
    }
    if (idslib_winmix_reserve_windows(fblayer->wlayer->idsx, wxs) != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idslib_winmix_reserve_windows failed - \n")));
        return IM_RET_FAILED;
    }

    fblayer->wmix.wxs = wxs;
    fblayer->wmix.num = num;
    fblayer->wmix.imgWidth = pano->imgWidth;
    fblayer->wmix.pixfmt = pixfmt;

    if (pixfmt == IDSLIB_PIXFMT_16BPP_RGB565){
        bpp = 16;
    }else if (pixfmt == IDSLIB_PIXFMT_32BPP_RGB0888 || pixfmt == IDSLIB_PIXFMT_32BPP_ARGBA888){
        bpp = 32;
    }else{
        return IM_RET_INVALID_PARAMETER;
    }

    //
    // Todo : add local canvas offset later :
    // 
    switch (num){
        case 1:
            if(pano->imgWidth > 1792)
            {
                fblayer->wmix.vmoffset[0]     = 640 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 640;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 640;
                fblayer->wmix.coords[0].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 1120){
                fblayer->wmix.vmoffset[0]     = 416 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 416;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 416;
                fblayer->wmix.coords[0].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 720){
                fblayer->wmix.vmoffset[0]     = 256 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 256;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 256;
                fblayer->wmix.coords[0].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 640){
                fblayer->wmix.vmoffset[0]     = 228 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 228;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 228;
                fblayer->wmix.coords[0].height= pano->imgHeight;
            }
            else {
                fblayer->wmix.vmoffset[0]     = ((pano->imgWidth / 3) & 0xFFE0) * (bpp >> 3);
                fblayer->wmix.coords[0].left  = fblayer->wmix.vmoffset[0];
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = fblayer->wmix.vmoffset[0];
                fblayer->wmix.coords[0].height= pano->imgHeight;
            }
            break;
        case 2:
            if(pano->imgWidth > 1792)
            {
                fblayer->wmix.vmoffset[0]     = 384 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 384;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 1152;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 768 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 768;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 384;
                fblayer->wmix.coords[1].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 1120){
                fblayer->wmix.vmoffset[0]     = 256 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 256;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 768;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 512 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 512;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 256;
                fblayer->wmix.coords[1].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 720){
                fblayer->wmix.vmoffset[0]     = 160 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 160;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 480;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 320 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 320;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 160;
                fblayer->wmix.coords[1].height= pano->imgHeight;
            }
            else if (pano->imgWidth > 640){
                fblayer->wmix.vmoffset[0]     = 128 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 128;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 384;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 256 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 256;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 128;
                fblayer->wmix.coords[1].height= pano->imgHeight;
            }
            else {
                fblayer->wmix.vmoffset[0]     = ((pano->imgWidth / 5) & 0xFFE0) * (bpp >> 3);
                fblayer->wmix.coords[0].left  = fblayer->wmix.vmoffset[0];
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = fblayer->wmix.vmoffset[0] * 3;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = fblayer->wmix.vmoffset[0] * 2 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = fblayer->wmix.vmoffset[1];
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = fblayer->wmix.vmoffset[0];
                fblayer->wmix.coords[1].height= pano->imgHeight;
            }
            break;
        case 3:
            if(pano->imgWidth > 1792)
            {
                fblayer->wmix.vmoffset[0]     = 288 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 288;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = 1376;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 576 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 576;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 800;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 832 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 832;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 288;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            else if (pano->imgWidth >= 1344)
            {
                fblayer->wmix.vmoffset[0]     = 256 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 256;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = (1536 > pano->imgWidth)? (pano->imgWidth-256) : 1280;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 512 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 512;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 768;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 768 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 768;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 256;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            else if (pano->imgWidth >= 1120)
            {
                fblayer->wmix.vmoffset[0]     = 192 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 192;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = (1152 > pano->imgWidth)? (pano->imgWidth - 192) : 960;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 384 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 384;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 576;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 576 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 576;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 188;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            else if (pano->imgWidth >= 896)
            {
                fblayer->wmix.vmoffset[0]     = 160 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 160;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = (960 > pano->imgWidth)? (pano->imgWidth - 160) : 800;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 320 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 320;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 480;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 480 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 480;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 160;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            else if (pano->imgWidth >= 448)
            {
                fblayer->wmix.vmoffset[0]     = 128 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 128;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = (768 > pano->imgWidth)? (pano->imgWidth-128) : 640;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 256 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 256;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 384;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 384 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 384;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 128;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            else
            {
                fblayer->wmix.vmoffset[0]     = 64 * (bpp >> 3);
                fblayer->wmix.coords[0].left  = 64;
                fblayer->wmix.coords[0].top   = 0;
                fblayer->wmix.coords[0].width = (384 > pano->imgWidth)? (pano->imgWidth-64) : 320;
                fblayer->wmix.coords[0].height= pano->imgHeight;

                fblayer->wmix.vmoffset[1]     = 128 * (bpp >> 3);
                fblayer->wmix.coords[1].left  = 128;
                fblayer->wmix.coords[1].top   = 0;
                fblayer->wmix.coords[1].width = 192;
                fblayer->wmix.coords[1].height= pano->imgHeight;

                fblayer->wmix.vmoffset[2]     = 192 * (bpp >> 3);
                fblayer->wmix.coords[2].left  = 192;
                fblayer->wmix.coords[2].top   = 0;
                fblayer->wmix.coords[2].width = 65;
                fblayer->wmix.coords[2].height= pano->imgHeight;
            }
            break;
        default:
            IM_ASSERT(IM_FALSE);
            break;
    }
    return IM_RET_OK;
}

IM_RET idslib_fblayer_winmix_deinit(fblayer_handle_t handle)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    if (fblayer->wmix.wxs){
        idslib_winmix_close(fblayer->wlayer->idsx, fblayer->wmix.wxs);
        idslib_winmix_release_windows(fblayer->wlayer->idsx, fblayer->wmix.wxs);
    }
    return IM_RET_OK;
}

IM_RET idslib_fblayer_winmix_set_panorama(fblayer_handle_t handle,ids_wlayer_canvas_t *canvas)
{
    IM_INT32 i, pixfmt, left, top, right, bottom;
    IM_INT32 OVCWxCR=0, OVCWxVSSR=0, OVCWxPCAR=0, OVCWxPCBR=0, OVCWxPCCR=0;
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    pixfmt = fblayer->wmix.pixfmt;

    // 0 -> win1, 1-> win2, 2-> win3
    for (i = 0; i < fblayer->wmix.num; i++){
        switch(i){
            case 0:
                OVCWxCR = OVCW1CR;
                OVCWxVSSR = OVCW1VSSR;
                OVCWxPCAR = OVCW1PCAR;
                OVCWxPCBR = OVCW1PCBR;
                OVCWxPCCR = OVCW1PCCR;
                break;
            case 1:
                OVCWxCR = OVCW2CR;
                OVCWxVSSR = OVCW2VSSR;
                OVCWxPCAR = OVCW2PCAR;
                OVCWxPCBR = OVCW2PCBR;
                OVCWxPCCR = OVCW2PCCR;
                break;
            case 2:
                OVCWxCR = OVCW3CR;
                OVCWxVSSR = OVCW3VSSR;
                OVCWxPCAR = OVCW3PCAR;
                OVCWxPCBR = OVCW3PCBR;
                OVCWxPCCR = OVCW3PCCR;
                break;
            default:
                IM_ASSERT(IM_FALSE);
                break;
        }
        if (canvas == IM_NULL){
            left = fblayer->wmix.coords[i].left ;
            top = fblayer->wmix.coords[i].top ;
            right = fblayer->wmix.coords[i].left + fblayer->wmix.coords[i].width ;
            bottom = fblayer->wmix.coords[i].top + fblayer->wmix.coords[i].height;
        }
        else {
            left = fblayer->wmix.coords[i].left + canvas->xOffset;
            top = fblayer->wmix.coords[i].top + canvas->yOffset;
            right = left + fblayer->wmix.coords[i].width;
            bottom = top + fblayer->wmix.coords[i].height;
        }
        idspwl_write_regbit(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxCR , OVCWxCR_BPPMODE, 5, pixfmt);
        idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxVSSR, fblayer->wmix.imgWidth);
        idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxPCAR, (left << OVCWxPCAR_LeftTopX) | (top << OVCWxPCAR_LeftTopY));
        idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxPCBR,
                ((right - 1) << OVCWxPCBR_RightBotX) | ((bottom - 1) << OVCWxPCBR_RightBotY));
        idspwl_write_reg(MODULE_IDSx(fblayer->wlayer->idsx), OVCWxPCCR, 0xFFF000);
    }
    return IM_RET_OK;
}

IM_RET idslib_winmix_reserve_windows(IM_INT32 idsx, IM_INT32 wxs)
{
    IM_INT32 i;
    IM_INFOMSG((IM_STR("%s(idsx=%d, wxs=0x%x)"), IM_STR(_IM_FUNC_), idsx, wxs));
    IM_ASSERT(gIdsInst != IM_NULL);

    // check.
    /*for(i=0; i<4; i++){
        if((wxs & (1<<i)) && !(gIdsInst->idsMgr[idsx].resource & (RES_WLAYER_0 << i))){
            IM_ERRMSG((IM_STR("no w%d resource"), i));
            return IM_RET_FAILED;
        }
    }*/

    // reserve.
    for(i=0; i<4; i++){
        if(wxs & (1<<i)){
            gIdsInst->idsMgr[idsx].resource &= ~(RES_WLAYER_0 << i);
        }
    }

    return IM_RET_OK;
}

IM_RET idslib_winmix_release_windows(IM_INT32 idsx, IM_INT32 wxs)
{
    IM_INT32 i;
    IM_INFOMSG((IM_STR("%s(idsx=%d, wxs=0x%x)"), IM_STR(_IM_FUNC_), idsx, wxs));
    IM_ASSERT(gIdsInst != IM_NULL);

    // release.
    for(i=0; i<4; i++){
        if(wxs & (1<<i)){
            idspwl_write_regbit(MODULE_IDSx(idsx), OVCW0CR + i * 0x80, OVCWxCR_ENWIN, 1, 0);
            gIdsInst->idsMgr[idsx].resource |= RES_WLAYER_0 << i;
        }
    }

    return IM_RET_OK;
}

IM_RET idslib_winmix_open(IM_INT32 idsx, IM_INT32 wxs)
{
    IM_INT32 i;
    IM_INFOMSG((IM_STR("%s(idsx=%d, wxs=0x%x)"), IM_STR(_IM_FUNC_), idsx, wxs));
    IM_ASSERT(gIdsInst != IM_NULL);

    // open
    for(i=0; i<4; i++){
        if(wxs & (1<<i)){
            if(i == 0){
                idspwl_write_reg(MODULE_IDSx(idsx), OVCW0CMR, 0x0000000);
            }else{
                idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CMR + 0x80 * (i - 1), 0x0000000);
            }
            idspwl_write_regbit(MODULE_IDSx(idsx), OVCW0CR + i * 0x80, OVCWxCR_ENWIN, 1, 1);
        }
    }

    return IM_RET_OK;
}

IM_RET idslib_winmix_close(IM_INT32 idsx, IM_INT32 wxs)
{
    IM_INT32 i;
    IM_INFOMSG((IM_STR("%s(idsx=%d, wxs=0x%x)"), IM_STR(_IM_FUNC_), idsx, wxs));
    IM_ASSERT(gIdsInst != IM_NULL);

    // open
    for(i=0; i<4; i++){
        if(wxs & (1<<i)){
            idspwl_write_regbit(MODULE_IDSx(idsx), OVCW0CR + i * 0x80, OVCWxCR_ENWIN, 1, 0);
        }
    }

    return IM_RET_OK;
}
#endif

IM_RET idslib_fblayer_get_device(fblayer_handle_t handle, ids_display_device_t *dev, dispdev_info_t *info, ids_wlayer_canvas_t *canvas)
{
    fblayer_t *fblayer = (fblayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((fblayer == IM_NULL) || (fblayer != &gIdsInst->fblayer)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(fblayer->flag & FBLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("fblayer has not been inited")));
        return IM_RET_FAILED;
    }

    //	
    return idslib_wlayer_get_device((wlayer_handle_t)fblayer->wlayer, dev, info, canvas);
}

static IM_RET wlayer_init_internal(wlayer_t *wlayer, IM_INT32 idsx, IM_INT32 wx, IM_BOOL isfblayer)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d, isfblayer=%d)"), IM_STR(_IM_FUNC_), idsx, wx, isfblayer));

    // check resource. fblayer fixed use 0.
    if(!isfblayer && ((idsx == gIdsInst->locidsx) && (wx == 0) && (gIdsInst->config.resvFblayer))){
        IM_ERRMSG((IM_STR("w0 reserved for fblayer")));
        return IM_RET_NORESOURCE;
    }
    if(!(gIdsInst->idsMgr[idsx].resource & (RES_WLAYER_0 << wx))){
        IM_ERRMSG((IM_STR("no w%d resource"), wx));
        return IM_RET_NORESOURCE;
    }

    // 
    if(!(gIdsInst->idsMgr[idsx].flag & IDSMGR_FLAG_HW_INITED)){
        if(idshw_init(idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idshw_init() failed")));	
            return IM_RET_FAILED;
        }
        gIdsInst->idsMgr[idsx].flag |= IDSMGR_FLAG_HW_INITED;
    }
    if((gIdsInst->idsMgr[idsx].dev == IM_NULL) || !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_CONFIGURED)){
        if(device_set_default(idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("device_set_default(idsx=%d) failed"), idsx));
            goto Fail;
        }
    }

    // init this wlayer.
    idspwl_memset((void *)wlayer, 0, sizeof(wlayer_t));
    wlayer->idsx = idsx;
    wlayer->wx = wx;
    wlayer->flag = WLAYER_FLAG_INITED;

    // mark resource is reserved.
    gIdsInst->idsMgr[idsx].resource &= ~(RES_WLAYER_0 << wx);
    return IM_RET_OK;
Fail:
    if(((gIdsInst->idsMgr[idsx].resource & RES_WLAYER_ALL) == RES_WLAYER_ALL) && 
            !(gIdsInst->ie[idsx].flag & (IE_FLAG_INITED | IE_FLAG_EMPLOYEED))){
        IM_ASSERT(gIdsInst->idsMgr[idsx].flag & IDSMGR_FLAG_HW_INITED);
        if(idshw_deinit(idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idshw_deinit() failed")));	
        }
        gIdsInst->idsMgr[idsx].flag &= IDSMGR_FLAG_HW_INITED;
    }
    return IM_RET_FAILED;
}

IM_RET idslib_extlayer_init(extlayer_handle_t *handle, idslib_func_idsdrv_listener_t listener)
{
    // currently extlayer is just for hdmi
    IM_UINT32 devtype = IDS_DISPDEV_TYPE_HDMI;
    dispdev_t *dd;
    vmode_t *vmode = &gIdsInst->vmode;
    dd = &gIdsInst->devs[gIdsInst->extidsx][devtype];
    if (dd->cfglst == IM_NULL || im_list_size(dd->cfglst) == 0){
        IM_ERRMSG((IM_STR(" extend device not connected!! ")));
        return IM_RET_FAILED;
    }

    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
    idspwl_lock_lock(gIdsInst->vmlock);
    if (gIdsInst->mode & IDS_MODE_VMODE_USE){
        IM_INFOMSG((IM_STR(" current vmode project mode, quit it, for extlayer use ")));
        vmode->flag &= ~VMODE_FLAG_HIDELOC;
        fblayer_hide(IM_FALSE);
        if (vmode->extfblayer != IM_NULL){
            ext_fblayer_destroy(vmode);
        }
        vmode->extDevSel = IM_FALSE;
        vmode->flag |= VMODE_FLAG_STABLE;
        gIdsInst->mode &= ~IDS_MODE_VMODE_USE;
        gIdsInst->vmode.flag &= ~VMODE_FLAG_EXT_SETUP;
        vmode->config.mode = IDS_VMODE_MODE_LOCAL;
        // notify the app to return to local mode
        vmode_trigger_event(IDS_VMODE_EVENT_BACK_TO_LOCAL, 0, 0 , IM_NULL, 0);
    }
    idspwl_lock_unlock(gIdsInst->vmlock);

    device_set_default(gIdsInst->extidsx);
    gIdsInst->extlayer.listener = listener;
    *handle = &gIdsInst->extlayer;
    gIdsInst->mode |= IDS_MODE_EXTLAYER_FLAG;
    return IM_RET_OK;
}

IM_RET idslib_extlayer_deinit(extlayer_handle_t handle)
{
    IM_INT32 evt;
    extlayer_t *extlayer = (extlayer_t *)handle;
    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

    if((handle == IM_NULL) || (extlayer != &gIdsInst->extlayer)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    extlayer->listener = IM_NULL;
    gIdsInst->mode &= ~IDS_MODE_EXTLAYER_FLAG;

    // change back to projection mode when extlayer mode is quit
    if(!(gIdsInst->idsMgr[1].dev->flag & DISPDEV_FLAG_BREAKED)){
        evt = IDS_VMODE_EVENT_EXTDEV_UPDATE;
        vmode_trigger_event(evt, -1, -1, IM_NULL, 0);
    }
    return IM_RET_OK;
}

IM_RET idslib_wlayer_init(wlayer_handle_t *handle, IM_INT32 idsx, IM_INT32 wx)
{
    IM_RET ret;
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));
    IM_ASSERT(gIdsInst != IM_NULL);

    // check parameters.	
    if((handle == IM_NULL) || (!VALID_IDSx(idsx)) || (!VALID_Wx(wx))){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    if (idsx == gIdsInst->extidsx && ((gIdsInst->mode & IDS_MODE_EXTLAYER_FLAG) == 0)){
        IM_ERRMSG((IM_STR(" extlayer use not registered , error ")));
        return IM_RET_FAILED;
    }
    // 
    if((ret = wlayer_init_internal(&gIdsInst->idsMgr[idsx].wlayer[wx], idsx, wx, IM_FALSE)) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_init_internal() failed")));
        return ret;
    }
    *handle = (wlayer_handle_t)&gIdsInst->idsMgr[idsx].wlayer[wx];

    if (idsx == gIdsInst->extidsx){
        gIdsInst->mode |= (IDS_MODE_EXTLAYER_W0_USE << wx);
    }
    return IM_RET_OK;
}

IM_RET idslib_wlayer_deinit(wlayer_handle_t handle)
{
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    return wlayer_deinit_internal(wlayer);
}

static IM_RET wlayer_deinit_internal(wlayer_t *wlayer)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    if(wlayer->flag & WLAYER_FLAG_OPENED){
        wlayer_close_internal(wlayer);
    }

    //
    if(wlayer->flag & WLAYER_FLAG_ASYNC_CONFIG){
        idshw_flush_config(wlayer->idsx, wlayer->wx);
    }

    // release resources.
    if(wlayer->rsvdRes & RES_RAMPAL_W0){
        idslib_wlayer_release_resource((wlayer_handle_t) wlayer, WLAYER_RES_RAMPAL, 0);
    }else if(wlayer->rsvdRes & RES_RAMPAL_W1){
        idslib_wlayer_release_resource((wlayer_handle_t) wlayer, WLAYER_RES_RAMPAL, 1);
    }

    if(wlayer->rsvdRes & RES_YUVtoRGB_MATRIX){
        idslib_wlayer_release_resource((wlayer_handle_t) wlayer, WLAYER_RES_YUVtoRGB_MATRIX, 0);
    }

    if(wlayer->rsvdRes & (RES_DEVMMU_W0 << wlayer->wx)){
        idslib_wlayer_release_resource((wlayer_handle_t) wlayer, WLAYER_RES_MMU, wlayer->wx);
    }

    // If all the wlayers are released, IE is useless now, deinit IE and deinit this ids ? 
    if(((gIdsInst->idsMgr[wlayer->idsx].resource & RES_WLAYER_ALL) == RES_WLAYER_ALL) && 
            !(gIdsInst->ie[wlayer->idsx].flag & (IE_FLAG_INITED | IE_FLAG_EMPLOYEED))){
        IM_ASSERT(gIdsInst->idsMgr[wlayer->idsx].flag & IDSMGR_FLAG_HW_INITED);
        if(idshw_deinit(wlayer->idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idshw_deinit() failed")));	
        }
        gIdsInst->idsMgr[wlayer->idsx].flag &= ~IDSMGR_FLAG_HW_INITED;
    }

    if (wlayer->idsx == gIdsInst->extidsx){
        gIdsInst->mode &= ~(IDS_MODE_EXTLAYER_W0_USE << wlayer->wx);
    }

    gIdsInst->idsMgr[wlayer->idsx].resource |= (RES_WLAYER_0 << wlayer->wx);
    idspwl_memset((void *)wlayer, 0, sizeof(wlayer_t));
    return IM_RET_OK;
}

IM_RET idslib_wlayer_query_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{	
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s(res=%d, param=%d)"), IM_STR(_IM_FUNC_), res, param));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    switch(res){
        case WLAYER_RES_RAMPAL:
            if(!(gIdsInst->idsMgr[wlayer->idsx].resource & ((param & 0x1) << RES_RAMPAL_W0))){
                return IM_RET_NORESOURCE;
            }
            break;
        case WLAYER_RES_YUVtoRGB_MATRIX:
            if((wlayer->wx == 3) || !(gIdsInst->idsMgr[wlayer->idsx].resource & RES_YUVtoRGB_MATRIX)){
                return IM_RET_NORESOURCE;
            }
            break;
        case WLAYER_RES_MMU:
            if(!(gIdsInst->idsMgr[wlayer->idsx].resource & (RES_DEVMMU_W0 << wlayer->wx))){
                return IM_RET_NORESOURCE;
            }
            break;
        default:
            return IM_RET_INVALID_PARAMETER;
    }

    return IM_RET_OK;
}

IM_RET idslib_wlayer_reserve_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{	
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s(res=%d, param=%d)"), IM_STR(_IM_FUNC_), res, param));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    switch(res){
        case WLAYER_RES_RAMPAL:
            if(!(gIdsInst->idsMgr[wlayer->idsx].resource & ((param & 0x1) << RES_RAMPAL_W0))){
                return IM_RET_NORESOURCE;
            }
            wlayer->rsvdRes |= (param & 0x1) << RES_RAMPAL_W0;
            gIdsInst->idsMgr[wlayer->idsx].resource &= ~((param & 0x1) << RES_RAMPAL_W0);
            break;
        case WLAYER_RES_YUVtoRGB_MATRIX:
            if((wlayer->wx == 3) || !(gIdsInst->idsMgr[wlayer->idsx].resource & RES_YUVtoRGB_MATRIX)){
                return IM_RET_NORESOURCE;
            }
            wlayer->rsvdRes |= RES_YUVtoRGB_MATRIX;
            gIdsInst->idsMgr[wlayer->idsx].resource &= ~RES_YUVtoRGB_MATRIX;
            break;
        case WLAYER_RES_MMU:
            if(!(gIdsInst->idsMgr[wlayer->idsx].resource & (RES_DEVMMU_W0 << wlayer->wx))){
                return IM_RET_NORESOURCE;
            }
            wlayer->rsvdRes |= RES_DEVMMU_W0 << wlayer->wx;
            gIdsInst->idsMgr[wlayer->idsx].resource &= ~(RES_DEVMMU_W0 << wlayer->wx);
            break;
        default:
            return IM_RET_INVALID_PARAMETER;
    }

    return IM_RET_OK;
}

IM_RET idslib_wlayer_release_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param)
{
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s(res=%d, param=%d)"), IM_STR(_IM_FUNC_), res, param));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    switch(res){
        case WLAYER_RES_RAMPAL:
            if(!(wlayer->rsvdRes & ((param & 0x1) << RES_RAMPAL_W0))){
                wlayer->rsvdRes &= ~((param & 0x1) << RES_RAMPAL_W0);
                gIdsInst->idsMgr[wlayer->idsx].resource |= (param & 0x1) << RES_RAMPAL_W0;
            }
            break;
        case WLAYER_RES_YUVtoRGB_MATRIX:
            if((wlayer->wx != 3) && !(gIdsInst->idsMgr[wlayer->idsx].resource & RES_YUVtoRGB_MATRIX)){
                wlayer->rsvdRes &= ~RES_YUVtoRGB_MATRIX;
                gIdsInst->idsMgr[wlayer->idsx].resource |= RES_YUVtoRGB_MATRIX;
            }
            break;
        case WLAYER_RES_MMU:
            if(wlayer->rsvdRes & (RES_DEVMMU_W0 << wlayer->wx)){
                gIdsInst->idsMgr[wlayer->idsx].resource |= (RES_DEVMMU_W0 << wlayer->wx);
                wlayer->rsvdRes &= ~(RES_DEVMMU_W0 << wlayer->wx);
            }
            break;
        default:
            return IM_RET_INVALID_PARAMETER;
    }

    return IM_RET_OK;
}

IM_RET idslib_wlayer_open(wlayer_handle_t handle)
{
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    if (wlayer->idsx == gIdsInst->extidsx && ((gIdsInst->mode & IDS_MODE_EXTLAYER_FLAG) == 0)){
        IM_INFOMSG((IM_STR(" extlayer use not registered, not to respond ")));
        return IM_RET_OK;
    }
    return wlayer_open_internal(wlayer);
}

static IM_RET wlayer_open_internal(wlayer_t *wlayer)
{
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    if(wlayer->flag & WLAYER_FLAG_OPENED){
        IM_INFOMSG((IM_STR("wlayer has been opened")));
        return IM_RET_OK;
    }

    //
    if(wlayer->rsvdRes & (RES_DEVMMU_W0 << wlayer->wx)){
        if(idspwl_init_mmu(wlayer->idsx, wlayer->wx, wlayer->rsvdRes & RES_YUVtoRGB_MATRIX) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idspwl_init_mmu() failed")));
            goto Fail;
        }
        wlayer->flag |= WLAYER_FLAG_MMU_INITED;
    }

    //
    if((gIdsInst->vmode.flag & VMODE_FLAG_INITED) && (wlayer->idsx == gIdsInst->locidsx) && 
            (wlayer->wx == 0) && (gIdsInst->fblayer.flag & FBLAYER_FLAG_INITED) && 
            !(gIdsInst->vmode.flag & VMODE_FLAG_HIDELOC))
    {
        IM_INFOMSG((IM_STR("fblayer hide")));
    }else{
        if(wlayer_open(wlayer->idsx, wlayer->wx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("wlayer_open() failed")));
            goto Fail;
        }
        wlayer->flag |= WLAYER_FLAG_OPENED;

        if(!(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED)){
            if(device_open(wlayer->idsx) != IM_RET_OK){
                IM_ERRMSG((IM_STR("device_open() failed")));
                goto Fail;
            }
            gIdsInst->idsMgr[wlayer->idsx].dev->flag |= DISPDEV_FLAG_OPENED;
        }
    }

    return IM_RET_OK;

Fail:
    if(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED){
        if(device_close(wlayer->idsx, wlayer->wx, IM_FALSE) == IM_RET_OK){
            gIdsInst->idsMgr[wlayer->idsx].dev->flag &= ~DISPDEV_FLAG_OPENED;
        }
    }
    if(wlayer->flag & WLAYER_FLAG_OPENED){
        wlayer_close(wlayer->idsx, wlayer->wx);
        wlayer->flag &= ~WLAYER_FLAG_OPENED;
    }
    if(wlayer->flag & WLAYER_FLAG_MMU_INITED){
        idspwl_deinit_mmu(wlayer->idsx, wlayer->wx);
        wlayer->flag &= ~WLAYER_FLAG_MMU_INITED;
    }

    return IM_RET_FAILED;
}

IM_RET idslib_wlayer_close(wlayer_handle_t handle)
{
    wlayer_t *wlayer = (wlayer_t *)handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    return wlayer_close_internal(wlayer);
}

static IM_RET wlayer_close_internal(wlayer_t *wlayer)
{
    IM_RET ret = IM_RET_OK;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    //
    if(!(wlayer->flag & WLAYER_FLAG_OPENED)){
        IM_INFOMSG((IM_STR("wlayer is not opened")));
        return IM_RET_OK;
    }

    //
    wlayer_prepare_close(wlayer->idsx, wlayer->wx);

    //
    if(gIdsInst->idsMgr[wlayer->idsx].dev->flag & DISPDEV_FLAG_OPENED){
        if(device_close(wlayer->idsx, wlayer->wx, IM_FALSE) == IM_RET_OK){
            gIdsInst->idsMgr[wlayer->idsx].dev->flag &= ~DISPDEV_FLAG_OPENED;
        }
    }

    if(wlayer_close(wlayer->idsx, wlayer->wx) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_close() failed")));
        ret = IM_RET_FAILED;
    }
    wlayer->flag &= ~WLAYER_FLAG_OPENED;

    if(wlayer->flag & WLAYER_FLAG_MMU_INITED){
        if(idspwl_deinit_mmu(wlayer->idsx, wlayer->wx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idspwl_deinit_mmu() failed")));
            ret = IM_RET_FAILED;
        }
        wlayer->flag &= ~WLAYER_FLAG_MMU_INITED;
    }

    return ret;
}

IM_RET idslib_wlayer_vsync(wlayer_handle_t handle)
{
    wlayer_t *wlayer = (wlayer_t *)handle;

    //IM_INFOMSG((IM_STR("%s,idsx=%d, wx=%d "), IM_STR(_IM_FUNC_), wlayer->idsx, wlayer->wx));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    //
    if(!(wlayer->flag & WLAYER_FLAG_OPENED)){
        return IM_RET_FAILED;
    }

    idspwl_vsync(wlayer->idsx);
    return IM_RET_OK;
}

IM_RET idslib_wlayer_get_device(wlayer_handle_t handle, ids_display_device_t *dev, dispdev_info_t *info, ids_wlayer_canvas_t *canvas)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_INT32 idsx = wlayer->idsx;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameter.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), idsx, wlayer->wx));

    //
    IM_ASSERT((gIdsInst->idsMgr[idsx].dev != IM_NULL) && (gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_CONFIGURED));
    if(dev){
        idspwl_memcpy((void *)dev, (void *)&gIdsInst->idsMgr[idsx].dev->config, sizeof(ids_display_device_t));
    }
    if(info){
        idspwl_memcpy((void *)info, (void *)&gIdsInst->idsMgr[idsx].dev->info, sizeof(dispdev_info_t));	
    }
    if(canvas){
        idspwl_memcpy((void *)canvas, (void *)&gIdsInst->idsMgr[idsx].dev->canvas, sizeof(ids_wlayer_canvas_t));	
    }

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_panorama(wlayer_handle_t handle, IM_INT32 pixfmt, ids_wlayer_panorama_t *pano)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    dispdev_t *dd;
    IM_INT32 OVCWxCR, OVCWxVSSR, OVCWxPCCR = 0;

    IM_INFOMSG((IM_STR("%s(pixfmt=%d)"), IM_STR(_IM_FUNC_), pixfmt));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("idsx=%d, wx=%d"), wlayer->idsx, wlayer->wx));

    IM_INFOMSG((IM_STR("pano->imgWidth=%d"), pano->imgWidth));
    IM_INFOMSG((IM_STR("pano->imgHeight=%d"), pano->imgHeight));
    IM_INFOMSG((IM_STR("pano->pos.left=%d"), pano->pos.left));
    IM_INFOMSG((IM_STR("pano->pos.top=%d"), pano->pos.top));
    IM_INFOMSG((IM_STR("pano->pos.width=%d"), pano->pos.width));
    IM_INFOMSG((IM_STR("pano->pos.height=%d"), pano->pos.height));
    IM_INFOMSG((IM_STR("pano->buffer.vir_addr=0x%x"), (IM_INT32)pano->buffer.vir_addr));    
    IM_INFOMSG((IM_STR("pano->buffer.phy_addr=0x%x"), pano->buffer.phy_addr));              
    IM_INFOMSG((IM_STR("pano->buffer.size=%d"), pano->buffer.size));                        
    IM_INFOMSG((IM_STR("pano->buffer.flag=0x%x"), pano->buffer.flag));                      
    IM_INFOMSG((IM_STR("pano->bufferUv.vir_addr=0x%x"), (IM_INT32)pano->bufferUv.vir_addr));
    IM_INFOMSG((IM_STR("pano->bufferUv.phy_addr=0x%x"), pano->bufferUv.phy_addr));          
    IM_INFOMSG((IM_STR("pano->bufferUv.size=%d"), pano->bufferUv.size));                    
    IM_INFOMSG((IM_STR("pano->bufferUv.flag=0x%x"), pano->bufferUv.flag));                  

    dd = gIdsInst->idsMgr[wlayer->idsx].dev;
    if ((dd->config.width < pano->imgWidth) || (dd->config.height < pano->imgHeight)){
        IM_ERRMSG((IM_STR("invalid parameter : dd->config.width=%d, height=%d, pano->imgWidth=%d, imgHeight=%d "),
                    dd->config.width,dd->config.height,pano->imgWidth,pano->imgHeight));
        return IM_RET_INVALID_PARAMETER;
    }
    //

    wlayer->pixfmt = pixfmt;

    // set canvas, to make the display area placed in the middle
    if(!(dd->flag & DISPDEV_FLAG_OPENED) && !(VMODE_FLAG_CHECK_EXT_SETUP(gIdsInst->vmode.flag))){
        dd->canvas.width = (pano->imgWidth + 0x7) & (~0x07);
        dd->canvas.height = (pano->imgHeight + 0x3) & (~0x03);
        dd->canvas.xOffset = (dd->config.width - dd->canvas.width) >> 1;
        dd->canvas.yOffset = (dd->config.height - dd->canvas.height) >> 1;
    }
    idspwl_memcpy((void *)&wlayer->pano, (void *)pano, sizeof(ids_wlayer_panorama_t));

    idshw_async_config(wlayer->idsx, wlayer->wx);
    switch (wlayer->wx){
        case 0:
            OVCWxCR = OVCW0CR;
            OVCWxVSSR = OVCW0VSSR;
            break;
        case 1:
            OVCWxCR = OVCW1CR;
            OVCWxVSSR = OVCW1VSSR;
            OVCWxPCCR = OVCW1PCCR;
            break;
        case 2:
            OVCWxCR = OVCW2CR;
            OVCWxVSSR = OVCW2VSSR;
            OVCWxPCCR = OVCW2PCCR;
            break;
        case 3:
            OVCWxCR = OVCW3CR;
            OVCWxVSSR = OVCW3VSSR;
            OVCWxPCCR = OVCW3PCCR;
            break;
        default :
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    idspwl_write_regbit(MODULE_IDSx(wlayer->idsx), OVCWxCR, OVCWxCR_BPPMODE, 5, pixfmt);
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxVSSR, pano->imgWidth);
    if (OVCWxPCCR != 0 ){
        idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxPCCR, 0xFFF000);
    }

    if (idslib_wlayer_set_buffer(handle, &pano->buffer, &pano->bufferUv) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_buffer failed")));
        return IM_RET_FAILED;
    }
    if (idslib_wlayer_set_position(handle, &pano->pos) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_position failed")));
        return IM_RET_FAILED;
    }
    if (idslib_wlayer_set_region(handle, &pano->region) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_region failed")));
        return IM_RET_FAILED;
    }
    idshw_flush_config(wlayer->idsx, wlayer->wx);

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_buffer(wlayer_handle_t handle, IM_Buffer *buffer, IM_Buffer *bufferUv)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_UINT32 i, reg_addr, OVCWxB0SAR = 0;

    //IM_INFOMSG((IM_STR("%s(), idsx=%d, wx=%d "), IM_STR(_IM_FUNC_), wlayer->idsx, wlayer->wx));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    switch(wlayer->wx){
        case 0:
            reg_addr = OVCW0B0SAR;
            break;
        case 1:
            reg_addr = OVCW1B0SAR;
            break;
        case 2:
            reg_addr = OVCW2B0SAR;
            break;
        case 3:
            reg_addr = OVCW3B0SAR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    //
    if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
        gIdsInst->pm.osdRegStore[wlayer->idsx][reg_addr] = buffer->phy_addr;
    }else{
        idspwl_write_reg(MODULE_IDSx(wlayer->idsx), reg_addr, buffer->phy_addr);
    }
    if (wlayer->pixfmt == IDSLIB_PIXFMT_12BPP_YUV420SP){
        if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
            gIdsInst->pm.osdRegStore[wlayer->idsx][OVCBRB0SAR] = bufferUv->phy_addr;
        }else{
            idspwl_write_reg(MODULE_IDSx(wlayer->idsx),OVCBRB0SAR , bufferUv->phy_addr);
        }
        wlayer->pano.bufferUv.phy_addr = bufferUv->phy_addr;
    }
#ifdef USE_WINMIX
    if ((wlayer->idsx == gIdsInst->extidsx) && (gIdsInst->mode & IDS_MODE_VMODE_USE)
            && VMODE_FLAG_CHECK_EXT_SETUP(gIdsInst->vmode.flag)){
        for ( i = 0; i < gIdsInst->vmode.extfblayer->wmix.num; i ++){
            switch(i){
                case 0:
                    OVCWxB0SAR = OVCW1B0SAR;
                    reg_addr = buffer->phy_addr + gIdsInst->vmode.extfblayer->wmix.vmoffset[0];
                    break;
                case 1:
                    OVCWxB0SAR = OVCW2B0SAR;
                    reg_addr = buffer->phy_addr + gIdsInst->vmode.extfblayer->wmix.vmoffset[1];
                    break;
                case 2:
                    OVCWxB0SAR = OVCW3B0SAR;
                    reg_addr = buffer->phy_addr + gIdsInst->vmode.extfblayer->wmix.vmoffset[2];
                    break;
                default:
                    IM_ASSERT(IM_FALSE);
                    break;
            }
            if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
                gIdsInst->pm.osdRegStore[wlayer->idsx][OVCWxB0SAR] = reg_addr;
            }else{
                idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxB0SAR, reg_addr);
            }
        }
    }
#endif
    if (wlayer->pano.region.enable){
        idslib_wlayer_set_region(handle, &wlayer->pano.region);
    }

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_region(wlayer_handle_t handle, ids_wlayer_region_t *region)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_UINT32 i, yOffset, cbcrOffset, reg_addr, OVCWxB0SAR=0;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    IM_INFOMSG((IM_STR("region->enable=%d"), (IM_INT32)region->enable));  
    IM_INFOMSG((IM_STR("region->xOffset=%d"), region->xOffset));
    IM_INFOMSG((IM_STR("region->yOffset=%d"), region->yOffset));
    IM_INFOMSG((IM_STR("region->width=%d"), region->width));    
    IM_INFOMSG((IM_STR("region->height=%d"), region->height));  

    if(region->enable == IM_FALSE){
        IM_INFOMSG((IM_STR(" region set not enable ")));
        idspwl_memcpy((void*)&wlayer->pano.region, (void *)region, sizeof (ids_wlayer_region_t));
        return IM_RET_OK;
    }

    if((region->xOffset < 0) || (region->xOffset >= wlayer->pano.imgWidth) ||
            (region->yOffset < 0) || (region->yOffset >= wlayer->pano.imgHeight) ||
            (region->width <= 0) || (region->width + region->xOffset > wlayer->pano.imgWidth) ||
            (region->height <= 0) || (region->height + region->yOffset > wlayer->pano.imgHeight))
    {
        IM_ERRMSG((IM_STR("invalid parameter region: xOffset=%d, yOffset=%d, width=%d, height=%d, imgWidth=%d, imgHeight=%d"),
                    region->xOffset, region->yOffset, region->width, region->height,
                    wlayer->pano.imgWidth, wlayer->pano.imgHeight));
        return IM_RET_INVALID_PARAMETER;
    }

    if((wlayer->pano.pos.width > region->width) || (wlayer->pano.pos.height > region->height)){
        IM_ERRMSG((IM_STR("invalid parameter, pos.width=%d, pos.height=%d, region.width=%d, pano->region.height=%d"),
                    wlayer->pano.pos.width, wlayer->pano.pos.height, region->width, region->height));
        return IM_RET_INVALID_PARAMETER;
    }

    switch(wlayer->wx){
        case 0:
            reg_addr = OVCW0B0SAR;
            break;
        case 1:
            reg_addr = OVCW1B0SAR;
            break;
        case 2:
            reg_addr = OVCW2B0SAR;
            break;
        case 3:
            reg_addr = OVCW3B0SAR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    idshw_async_config(wlayer->idsx, wlayer->wx);

    if (wlayer->pixfmt == IDSLIB_PIXFMT_12BPP_YUV420SP){
        yOffset = wlayer->pano.imgWidth * region->yOffset + region->xOffset + wlayer->pano.buffer.phy_addr;

        cbcrOffset = (region->yOffset >> 1) * wlayer->pano.imgWidth + region->xOffset + wlayer->pano.bufferUv.phy_addr;
        if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
            gIdsInst->pm.osdRegStore[wlayer->idsx][OVCBRB0SAR] = cbcrOffset;
        }else{
            idspwl_write_reg(MODULE_IDSx(wlayer->idsx),OVCBRB0SAR , cbcrOffset);
        }
    }
    else {
        yOffset = (wlayer->pano.imgWidth * region->yOffset + region->xOffset) * (idslib_get_bpp_from_pixel(wlayer->pixfmt) >> 3)
            + wlayer->pano.buffer.phy_addr;
    }
    if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
        gIdsInst->pm.osdRegStore[wlayer->idsx][reg_addr] = yOffset;
    }else{
        idspwl_write_reg(MODULE_IDSx(wlayer->idsx), reg_addr, yOffset);
    }
#ifdef USE_WINMIX
    if (wlayer->idsx == gIdsInst->extidsx && (gIdsInst->mode & IDS_MODE_VMODE_USE) 
            && VMODE_FLAG_CHECK_EXT_VALID(gIdsInst->vmode.flag)){
        for ( i = 0; i < gIdsInst->vmode.extfblayer->wmix.num; i ++){
            switch(i){
                case 0:
                    OVCWxB0SAR = OVCW1B0SAR;
                    reg_addr = yOffset + gIdsInst->vmode.extfblayer->wmix.vmoffset[0];
                    break;
                case 1:
                    OVCWxB0SAR = OVCW2B0SAR;
                    reg_addr = yOffset + gIdsInst->vmode.extfblayer->wmix.vmoffset[1];
                    break;
                case 2:
                    OVCWxB0SAR = OVCW3B0SAR;
                    reg_addr = yOffset + gIdsInst->vmode.extfblayer->wmix.vmoffset[2];
                    break;
                default:
                    IM_ASSERT(IM_FALSE);
                    break;
            }
            if(unlikely(gIdsInst->suspendFlag == IM_TRUE)){
                gIdsInst->pm.osdRegStore[wlayer->idsx][OVCWxB0SAR] = reg_addr;
            }else{
                idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxB0SAR, reg_addr);
            }
        }
    }
#endif
    idshw_flush_config(wlayer->idsx, wlayer->wx);

    idspwl_memcpy((void*)&wlayer->pano.region, (void *)region, sizeof (ids_wlayer_region_t));

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_position(wlayer_handle_t handle, ids_wlayer_position_t *pos)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    dispdev_t *dd;
    IM_INT32 x0, y0, x1, y1 ;
    IM_UINT32 reg_addr;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    IM_INFOMSG((IM_STR("pos->left=%d"), pos->left));
    IM_INFOMSG((IM_STR("pos->top=%d"), pos->top));
    IM_INFOMSG((IM_STR("pos->width=%d"), pos->width));
    IM_INFOMSG((IM_STR("pos->height=%d"), pos->height));

    dd = gIdsInst->idsMgr[wlayer->idsx].dev;

    if((pos->width > wlayer->pano.imgWidth) || (pos->height > wlayer->pano.imgHeight)){
        IM_ERRMSG((IM_STR("invalid parameter, pos->width=%d, wlayer->pano.imgWidth=%d,pos->height=%d,wlayer->pano.imgHeight=%d"),
                    pos->width,wlayer->pano.imgWidth,pos->height,wlayer->pano.imgHeight));
        return IM_RET_INVALID_PARAMETER;
    }
    if((pos->left < 0) || (pos->width < 0) || ((pos->left + pos->width) > dd->config.width) ||
            (pos->top < 0) || (pos->height < 0) || ((pos->top + pos->height) > dd->config.height)){
        IM_ERRMSG((IM_STR("invalid parameter, l=%d, w=%d, t=%d, h=%d, c_w=%d, c_h=%d"), pos->left, pos->width,
                    pos->top, pos->height, dd->config.width, dd->config.height));
        return IM_RET_INVALID_PARAMETER;
    }

    if (wlayer->pano.region.enable == IM_TRUE){
        if((pos->width > wlayer->pano.region.width) || (pos->height > wlayer->pano.region.height)){
            IM_ERRMSG((IM_STR("invalid parameter, pos->width=%d, pos->height=%d, wlayer->pano.region.width=%d, region.height=%d"),
                        pos->width, pos->height, wlayer->pano.region.width, wlayer->pano.region.height));
            return IM_RET_INVALID_PARAMETER;
        }
    }

    x0 = pos->left + dd->canvas.xOffset;
    y0 = pos->top + dd->canvas.yOffset;
    x1 = x0 + pos->width - 1;
    y1 = y0 + pos->height - 1;

    switch(wlayer->wx){
        case 0:
            reg_addr = OVCW0PCAR;
            break;
        case 1:
            reg_addr = OVCW1PCAR;
            break;
        case 2:
            reg_addr = OVCW2PCAR;
            break;
        case 3:
            reg_addr = OVCW3PCAR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    idshw_async_config(wlayer->idsx, wlayer->wx);
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), reg_addr, (x0 << OVCWxPCAR_LeftTopX) | (y0 << OVCWxPCAR_LeftTopY));
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), reg_addr + 0x04/*OVCWxPCBR*/, (x1 << OVCWxPCBR_RightBotX) | (y1 << OVCWxPCBR_RightBotY));
    idshw_flush_config(wlayer->idsx, wlayer->wx);

    idspwl_memcpy((void *)&wlayer->pano.pos, (void *)pos, sizeof(ids_wlayer_position_t));

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_mapclr(wlayer_handle_t handle, IM_BOOL enable, IM_INT32 clr)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_UINT32 reg_addr;

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    switch(wlayer->wx){
        case 0:
            reg_addr = OVCW0CMR;
            break;
        case 1:
            reg_addr = OVCW1CMR;
            break;
        case 2:
            reg_addr = OVCW2CMR;
            break;
        case 3:
            reg_addr = OVCW3CMR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), reg_addr, (enable << OVCWxCMR_MAPCOLEN) | (clr << OVCWxCMR_MAPCOLOR));

    wlayer->mapclrEnable = enable;
    wlayer->mapclr = clr;

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_alpha(wlayer_handle_t handle, ids_wlayer_alpha_t *alpha)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_UINT32 OVCWxCR, OVCWxPCCR, l1;

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    switch(wlayer->wx){
        case 1:
            OVCWxCR = OVCW1CR;
            OVCWxPCCR = OVCW1PCCR;
            break;
        case 2:
            OVCWxCR = OVCW2CR;
            OVCWxPCCR = OVCW2PCCR;
            break;
        case 3:
            OVCWxCR = OVCW3CR;
            OVCWxPCCR = OVCW3PCCR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    l1 = (alpha->alpha_r << OVCWxPCCR_ALPHA0_R) |
        (alpha->alpha_g << OVCWxPCCR_ALPHA0_G) |
        (alpha->alpha_b << OVCWxPCCR_ALPHA0_B);

    idshw_async_config(wlayer->idsx, wlayer->wx);
    idspwl_write_regbit(MODULE_IDSx(wlayer->idsx), OVCWxCR, OVCWxCR_BLD_PIX, 1 , alpha->pixelBlend);
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxPCCR, l1);
    idshw_flush_config(wlayer->idsx, wlayer->wx);

    idspwl_memcpy((void *)&wlayer->pano.alpha, (void *)alpha, sizeof(ids_wlayer_alpha_t));

    return IM_RET_OK;
}

IM_RET idslib_wlayer_set_clrkey(wlayer_handle_t handle, IM_BOOL enable, IM_BOOL matchForeground, IM_INT32 clr)
{
    wlayer_t *wlayer = (wlayer_t *)handle;
    IM_UINT32 OVCWxCKCR, OVCWxCKR, l1;

    // check parameters.
    if((wlayer == IM_NULL) || (wlayer != &gIdsInst->idsMgr[wlayer->idsx].wlayer[wlayer->wx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(wlayer->flag & WLAYER_FLAG_INITED)){
        IM_ERRMSG((IM_STR("this wlayer is not inited")));
        return IM_RET_FAILED;
    }

    switch(wlayer->wx){
        case 1:
            OVCWxCKCR = OVCW1CKCR;
            OVCWxCKR = OVCW1CKR;
            break;
        case 2:
            OVCWxCKCR = OVCW2CKCR;
            OVCWxCKR = OVCW2CKR;
            break;
        case 3:
            OVCWxCKCR = OVCW3CKCR;
            OVCWxCKR = OVCW3CKR;
            break;
        default:
            IM_ERRMSG((IM_STR("wrong window")));
            return IM_RET_FAILED;
    }
    l1 = (enable << OVCWxCKCR_KEYEN) | ((matchForeground ? 0 : 1) << OVCWxCKCR_DIRCON);

    idshw_async_config(wlayer->idsx, wlayer->wx);
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxCKCR, l1);
    idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCWxCKR, clr);
    idshw_flush_config(wlayer->idsx, wlayer->wx);

    wlayer->clrkeyEnable = enable;
    wlayer->clrkeyMatchForeground = matchForeground;
    wlayer->clrkey = clr;

    return IM_RET_OK;
}

IM_RET idslib_ie_init(ie_handle_t *handle, IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
    IM_ASSERT(gIdsInst != IM_NULL);

    // check parameters.
    if(handle == IM_NULL){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    // support multiple instances
    /*if(gIdsInst->ie[idsx].flag & IE_FLAG_EMPLOYEED){
      IM_ERRMSG((IM_STR("ie%d has been employeed"), idsx));
      return IM_RET_FAILED;
      }*/

    // if already inited, no need to init ielib again.
    if(gIdsInst->ie[idsx].flag & IE_FLAG_INITED){
        *handle = (ie_handle_t)&gIdsInst->ie[idsx];
        gIdsInst->ie[idsx].flag |= IE_FLAG_EMPLOYEED;
        return IM_RET_OK;
    }

    //
    if(!(gIdsInst->idsMgr[idsx].flag & IDSMGR_FLAG_HW_INITED)){
        if(idshw_init(idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idshw_init() failed")));	
            return IM_RET_FAILED;
        }
        gIdsInst->idsMgr[idsx].flag |= IDSMGR_FLAG_HW_INITED;
    }

    gIdsInst->ie[idsx].flag |= IE_FLAG_EMPLOYEED;
    *handle = (ie_handle_t)&gIdsInst->ie[idsx];
    return IM_RET_OK;
}

IM_RET idslib_ie_deinit(ie_handle_t handle)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    //
    if(!(gIdsInst->ie[ie->idsx].flag & IE_FLAG_EMPLOYEED)){
        IM_WARNMSG((IM_STR("ie(idsx) is not employeed")));
        return IM_RET_OK;
    }

    //
    if(((gIdsInst->idsMgr[ie->idsx].resource & RES_WLAYER_ALL) == RES_WLAYER_ALL) && 
            !(gIdsInst->ie[ie->idsx].flag & IE_FLAG_INITED)){
        IM_ASSERT(gIdsInst->idsMgr[ie->idsx].flag & IDSMGR_FLAG_HW_INITED);
        if(idshw_deinit(ie->idsx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idshw_deinit() failed")));	
        }
        gIdsInst->idsMgr[ie->idsx].flag &= IDSMGR_FLAG_HW_INITED;
    }

    gIdsInst->ie[ie->idsx].flag &= ~IE_FLAG_EMPLOYEED;
    return IM_RET_OK;
}

IM_RET idslib_ie_acc_set(ie_handle_t handle, IM_INT32 val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s(val=%d)"), IM_STR(_IM_FUNC_), val));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_acc_set(ie->handle, val);
}

IM_RET idslib_ie_acc_get(ie_handle_t handle, IM_INT32 *val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_acc_get_status(ie->handle, val);
}

IM_RET idslib_ie_acm_set(ie_handle_t handle, IM_INT32 val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s(val=%d)"), IM_STR(_IM_FUNC_), val));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_acm_set(ie->handle, val);
}

IM_RET idslib_ie_acm_get(ie_handle_t handle, IM_INT32 *val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_acm_get_status(ie->handle, val);
}

IM_RET idslib_ie_gamma_set(ie_handle_t handle, IM_INT32 val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s(val=%d)"), IM_STR(_IM_FUNC_), val));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_gamma_set(ie->handle, val);
}

IM_RET idslib_ie_gamma_get(ie_handle_t handle, IM_INT32 *val)
{
    ie_t *ie = (ie_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    return ielib_gamma_get_status(ie->handle, val);
}

IM_RET idslib_ie_ee_set(ie_handle_t handle, IM_INT32 val)
{
    ie_t *ie = (ie_t *)handle;
    vmode_t *vm = &gIdsInst->vmode;
    IM_INFOMSG((IM_STR("%s(val=%d)"), IM_STR(_IM_FUNC_), val));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    vm->eeval = val;
    return IM_RET_OK;
}

IM_RET idslib_ie_ee_get(ie_handle_t handle, IM_INT32 *val)
{
    ie_t *ie = (ie_t *)handle;
    vmode_t *vm = &gIdsInst->vmode;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameters.
    if((handle == IM_NULL) || (handle != &gIdsInst->ie[ie->idsx])){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!(ie->flag & IE_FLAG_EMPLOYEED)){
        IM_ERRMSG((IM_STR("ie(idsx=%d) has not been employeed"), ie->idsx));
        return IM_RET_FAILED;
    }

    *val = vm->eeval;
    return IM_RET_OK;
}

IM_RET idslib_check_hdmi_on(IM_UINT32 *hdmiOn)
{
    dispdev_t *dd = gIdsInst->idsMgr[1].dev;
    //IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // just check ids1 device flag 
    if(dd && (dd->flag & DISPDEV_FLAG_OPENED)){
        *hdmiOn = 1;
    }
    else{
        *hdmiOn = 0;
    }
    return IM_RET_OK;
}

IM_RET idslib_vmode_init(vmode_handle_t *handle, idslib_func_idsdrv_listener_t listener)
{
    vmode_t *vm = &gIdsInst->vmode;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    IM_ASSERT(gIdsInst != IM_NULL);

    // check parameter.
    if((handle == IM_NULL) || (listener == IM_NULL)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_FAILED;
    }

    // check resource.
    if(vm->flag & VMODE_FLAG_INITED){
        IM_ERRMSG((IM_STR("no vmode resource")));
        return IM_RET_NORESOURCE;		
    }
    idspwl_memset((void *)vm, 0, sizeof(vmode_t));

    //
    vm->signal = idspwl_sig_init(IM_TRUE);
    if(vm->signal == IM_NULL){
        IM_ERRMSG((IM_STR("idspwl_sig_init() failed")));
        goto Fail;
    }
    vm->evtlst = im_list_init(sizeof(vmode_evt_internal_t), gIdsInst->mpl);
    if(vm->evtlst == IM_NULL){
        IM_ERRMSG((IM_STR("im_list_init(vm->evtlst) failed")));
        goto Fail;
    }
    vm->action = VM_THREAD_ACTION_START;
    vm->thread = idspwl_thread_init((idspwl_func_thread_entry_t)vmode_server_looper, (void *)vm);
    if(vm->thread == IM_NULL){
        IM_ERRMSG((IM_STR("idspwl_thread_init() failed")));
        goto Fail;
    }
    while(vm->stat != VM_THREAD_STAT_RUN){ // wait for thread running.
        idspwl_msleep(5);
    }

    //
    vm->listener = listener;
    if((gIdsInst->idsMgr[gIdsInst->locidsx].dev != IM_NULL)	&& 
            (gIdsInst->idsMgr[gIdsInst->locidsx].dev->flag & DISPDEV_FLAG_CONFIGURED)){
        idspwl_memcpy((void *)&vm->config.locDev, (void *)&gIdsInst->idsMgr[gIdsInst->locidsx].dev->config, sizeof(ids_display_device_t));
        idspwl_memcpy((void *)&vm->config.locCanvas, (void *)&gIdsInst->idsMgr[gIdsInst->locidsx].dev->canvas, sizeof(ids_wlayer_canvas_t));
    }else{
        device_get_default(gIdsInst->locidsx, &vm->config.locDev, &vm->config.locCanvas);
    }

    if((gIdsInst->idsMgr[gIdsInst->extidsx].dev != IM_NULL) &&
            (gIdsInst->idsMgr[gIdsInst->extidsx].dev->flag & DISPDEV_FLAG_CONFIGURED)){
        idspwl_memcpy((void *)&vm->config.extDev, (void *)&gIdsInst->idsMgr[gIdsInst->extidsx].dev->config, sizeof(ids_display_device_t));
        idspwl_memcpy((void *)&vm->config.extCanvas, (void *)&gIdsInst->idsMgr[gIdsInst->extidsx].dev->canvas, sizeof(ids_wlayer_canvas_t));
    }else{
        device_get_default(gIdsInst->extidsx, &vm->config.extDev, &vm->config.extCanvas);
    }

    //
    vm->config.mode = IDS_VMODE_MODE_LOCAL;
    vm->flag = VMODE_FLAG_INITED | VMODE_FLAG_STABLE ;

    // register dynamic device listener.
    device_register_listener(gIdsInst->locidsx, (fcbk_dispdev_listener_t)device_listener);
    device_register_listener(gIdsInst->extidsx, (fcbk_dispdev_listener_t)device_listener);

    *handle = (vmode_handle_t)vm;
    return IM_RET_OK;
Fail:
    if(vm->thread){
        vm->action = VM_THREAD_ACTION_STOP;
        while(vm->stat != VM_THREAD_STAT_UNKNOWN){
            idspwl_msleep(5);
        }
        idspwl_thread_deinit(vm->thread);
    }
    if(vm->evtlst){
        im_list_deinit(vm->evtlst);
    }
    if(vm->signal){
        idspwl_sig_deinit(vm->signal);
    }
    idspwl_memset((void *)vm, 0, sizeof(vmode_t));
    return IM_RET_FAILED;
}

IM_RET idslib_vmode_deinit(vmode_handle_t handle)
{
    vmode_t *vm = (vmode_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode)){
        IM_ERRMSG((IM_STR("invalid parameter")));
        return IM_RET_INVALID_PARAMETER;
    }

    //
    if(vm->flag & VMODE_FLAG_HIDELOC){
        fblayer_hide(IM_FALSE);
    }

    ext_fblayer_destroy(vm);

    idspwl_lock_lock(gIdsInst->vmlock);

    // release resources.
    IM_ASSERT(vm->thread);
    vm->action = VM_THREAD_ACTION_STOP;
    while(vm->stat != VM_THREAD_STAT_UNKNOWN){
        idspwl_lock_unlock(gIdsInst->vmlock);
        idspwl_msleep(20);
        idspwl_lock_lock(gIdsInst->vmlock);
    }
    idspwl_thread_deinit(vm->thread);

    IM_ASSERT(vm->evtlst);
    im_list_deinit(vm->evtlst);

    IM_ASSERT(vm->signal);
    idspwl_sig_deinit(vm->signal);

    idspwl_memset((void *)vm, 0, sizeof(vmode_t));
    vm->config.mode = IDS_VMODE_MODE_LOCAL;
    vm->flag = VMODE_FLAG_STABLE;

    idspwl_lock_unlock(gIdsInst->vmlock);    
    return IM_RET_OK;
}

IM_RET idslib_vmode_get_mode(vmode_handle_t handle, ids_vmode_config_t *config)
{
    vmode_t *vm = (vmode_t *)handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode) || (config == IM_NULL)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }

    // if vmode is not stable, don't allow get mode.
    if(!(vm->flag & VMODE_FLAG_STABLE)){
        IM_ERRMSG((IM_STR("vmode is not stable")));
        return IM_RET_FAILED;
    }

    //
    idspwl_memcpy((void *)config, (void *)&vm->config, sizeof(ids_vmode_config_t));

    return IM_RET_OK;
}

IM_RET idslib_vmode_set_mode(vmode_handle_t handle, IM_UINT32 mode)
{
    IM_RET ret;
    IM_INT32 i;
    vmode_t *vm = (vmode_t *)handle;
    wlayer_t *wlayer;
    IM_INFOMSG((IM_STR("%s(mode=0x%x)"), IM_STR(_IM_FUNC_), mode));
    IM_ASSERT(IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_L_to_E);

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }
    if((IDS_VMODE_MODE(mode) != IDS_VMODE_MODE_PROJECTION) && (IDS_VMODE_MODE(mode) != IDS_VMODE_MODE_LOCAL)){
        IM_ERRMSG((IM_STR("invalid parameter, mode=0x%x"), mode));
        return IM_RET_INVALID_PARAMETER;
    }

    // if vmode is not stable, don't allow set mode.
    if(!(vm->flag & VMODE_FLAG_STABLE)){
        IM_ERRMSG((IM_STR("vmode is not stable")));
        return IM_RET_FAILED;
    }

    // when in actual use, these could be removed.
    if (gIdsInst->mode & IDS_MODE_EXTLAYER_WALL_USE){
        vmode_trigger_event(IDS_EXTLAYER_EVENT_BACK_TO_LOCAL, 0, 0 , IM_NULL, 0);
        for (i = 0; i < 4; i ++){
            wlayer_deinit_internal(&gIdsInst->idsMgr[gIdsInst->extidsx].wlayer[i]);
        }
        if ((gIdsInst->mode & IDS_MODE_EXTLAYER_WALL_USE)){
            IM_ERRMSG((IM_STR(" ids mode error : extlayer is still using")));
            return IM_RET_FAILED;
        }
    }

    //
    if(vm->config.mode == mode){
        IM_INFOMSG((IM_STR("mode is already the same mode=0%x"), mode));
        return IM_RET_OK;
    }
    if(((IDS_VMODE_MODE(mode) == IDS_VMODE_MODE_LOCAL) && !(gIdsInst->idsMgr[gIdsInst->locidsx].dev->flag & DISPDEV_FLAG_CONFIGURED)) ||
            ((IDS_VMODE_MODE(mode) != IDS_VMODE_MODE_LOCAL) && !(gIdsInst->idsMgr[gIdsInst->extidsx].dev->flag & DISPDEV_FLAG_CONFIGURED))){
        IM_ERRMSG((IM_STR("device is not set")));
        return IM_RET_FAILED;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);
    if((IDS_VMODE_MODE(mode) == IDS_VMODE_MODE_LOCAL) || (mode & IDS_VMODE_BIT_SHOW_LOCAL)){
        vm->flag &= ~VMODE_FLAG_HIDELOC;
        fblayer_hide(IM_FALSE);
    }else{
        vm->flag |= VMODE_FLAG_HIDELOC; // hide loc delayed in vmode_notify_internal() implement.
    }

    if(IDS_VMODE_MODE(vm->config.mode) == IDS_VMODE_MODE(mode)){
        IM_INFOMSG((IM_STR("target mode is equal to current mode")));
        if(vm->flag & VMODE_FLAG_HIDELOC){
            fblayer_hide(IM_TRUE);
        }
        vm->config.mode = mode;
        idspwl_lock_unlock(gIdsInst->vmlock);
        return IM_RET_OK;
    }

    vm->config.mode = mode;
    if((gIdsInst->idsMgr[gIdsInst->locidsx].resource & RES_WLAYER_ALL) == RES_WLAYER_ALL){
        IM_INFOMSG((IM_STR("no wlayer used")));
        idspwl_lock_unlock(gIdsInst->vmlock);
        return IM_RET_OK;
    }

    // vmode only support fblayer : w0
    wlayer = &gIdsInst->idsMgr[gIdsInst->locidsx].wlayer[0];
    if((wlayer->wx == 0) && (gIdsInst->fblayer.flag & FBLAYER_FLAG_INITED)){
        if(vm->config.mode == IDS_VMODE_MODE_LOCAL){
            gIdsInst->vmode.flag &= ~VMODE_FLAG_EXT_SETUP;
        }else{
            gIdsInst->vmode.flag |= VMODE_FLAG_EXT_SETUP;
        }
        gIdsInst->vmode.flag &= ~VMODE_FLAG_STABLE ;
        vmode_trigger_event(IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE, wlayer->idsx, wlayer->wx, (void *)&vm->config, sizeof(ids_vmode_config_t));
    }

    if(! VMODE_FLAG_CHECK_STABLE(vm->flag)){
        vm->flag &= ~VMODE_FLAG_STABLE;
        ret = IM_RET_FALSE;
    }else{
        ret = IM_RET_OK;
    }

    idspwl_lock_unlock(gIdsInst->vmlock);

    return ret;
}

IM_RET idslib_vmode_get_devs(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 *num, ids_display_device_t *devs)
{
    vmode_t *vm = (vmode_t *)handle;
    ids_display_device_t *dev;
    IM_INT32 i, j = 0;

    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode) || (num == IM_NULL) || (devs == IM_NULL)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!VALID_IDSx(idsx)){
        IM_ERRMSG((IM_STR("invalid parameter, idsx=%d"), idsx));
        return IM_RET_INVALID_PARAMETER;
    }

    // if vmode is not stable, don't allow get devs.
    if(!(vm->flag & VMODE_FLAG_STABLE)){
        IM_ERRMSG((IM_STR("vmode is not stable")));
        return IM_RET_FAILED;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);

    *num = 0;
    for(i=0; i<IDS_DISPDEV_TYPE_MAX; i++){
        if (gIdsInst->devs[idsx][i].cfglst == IM_NULL){
            continue;
        }

        dev = (ids_display_device_t *)im_list_begin(gIdsInst->devs[idsx][i].cfglst);
        while((dev != IM_NULL) && (j<IDSLIB_SUPPORT_DEVS_MAX)){
            idspwl_memcpy((void *)&devs[j++], (void *)dev, sizeof(ids_display_device_t));
            (*num)++;
            dev = (ids_display_device_t *)im_list_next(gIdsInst->devs[idsx][i].cfglst);
        }
    }

    idspwl_lock_unlock(gIdsInst->vmlock);

    return IM_RET_OK;
}

IM_RET idslib_vmode_set_dev(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 devtype, IM_INT32 devid)
{
    IM_INT32 i;
    IM_RET ret;
    vmode_t *vm = (vmode_t *)handle;
    dispdev_t *dd;
    IM_INFOMSG((IM_STR("%s(idsx=%d, devtype=%d, devid=0x%x)"), IM_STR(_IM_FUNC_), idsx, devtype, devid));

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!VALID_IDSx(idsx)){
        IM_ERRMSG((IM_STR("invalid parameter, idsx=%d"), idsx));
        return IM_RET_INVALID_PARAMETER;
    }
    if((devtype < IDS_DISPDEV_TYPE_LCD) || (devtype > IDS_DISPDEV_TYPE_HDMI)){
        IM_ERRMSG((IM_STR("invalid parameter, devtype=%d"), devtype));
        return IM_RET_INVALID_PARAMETER;
    }

    // if vmode is not stable, don't allow set dev.
    if(!(vm->flag & VMODE_FLAG_STABLE)){
        IM_ERRMSG((IM_STR("vmode is not stable")));
        return IM_RET_FAILED;
    }

    if (gIdsInst->mode & IDS_MODE_EXTLAYER_WALL_USE){
        vmode_trigger_event(IDS_EXTLAYER_EVENT_BACK_TO_LOCAL, 0, 0 , IM_NULL, 0);
        for (i = 0; i < 4; i ++){
            wlayer_deinit_internal(&gIdsInst->idsMgr[gIdsInst->extidsx].wlayer[i]);
        }
        if ((gIdsInst->mode & IDS_MODE_EXTLAYER_WALL_USE)){
            IM_ERRMSG((IM_STR(" ids mode error : extlayer is still using")));
            return IM_RET_FAILED;
        }
    }

    //
    dd = gIdsInst->idsMgr[idsx].dev;
    if(dd && (dd->flag & DISPDEV_FLAG_OPENED)){
        IM_ERRMSG((IM_STR("must close dev first")));
        return IM_RET_FAILED;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);
    if(device_init(idsx, devtype, devid) != IM_RET_OK){
        IM_ERRMSG((IM_STR("device_init(devtype=%d, devid=0x%x) failed"), devtype, devid));
        device_set_default(idsx); // may it has no default config.
        ret = IM_RET_FAILED;
    }else{
        ret = IM_RET_OK;
    }

    dd = gIdsInst->idsMgr[idsx].dev;
    if((dd != IM_NULL) && (dd->flag & DISPDEV_FLAG_CONFIGURED)){
        if(idsx == gIdsInst->locidsx){
            idspwl_memcpy((void *)&vm->config.locDev, (void *)&dd->config, sizeof(ids_display_device_t));
            idspwl_memcpy((void *)&vm->config.locCanvas, (void *)&dd->canvas, sizeof(ids_wlayer_canvas_t));
        }
        else if(idsx == gIdsInst->extidsx){
            idspwl_memcpy((void *)&vm->config.extDev, (void *)&dd->config, sizeof(ids_display_device_t));
            idspwl_memcpy((void *)&vm->config.extCanvas, (void *)&dd->canvas, sizeof(ids_wlayer_canvas_t));
        }
    }
    idspwl_lock_unlock(gIdsInst->vmlock);

    vm->extDevSel = IM_TRUE;

    return ret;
}

IM_RET idslib_vmode_set_canvas(vmode_handle_t handle, IM_INT32 idsx, ids_wlayer_canvas_t *canvas)
{
    IM_RET ret = IM_RET_OK;
    vmode_t *vm = (vmode_t *)handle;
    dispdev_t *dd;
    wlayer_t *wlayer;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    // check parameter.
    if((vm == IM_NULL) || (vm != &gIdsInst->vmode)){
        IM_ERRMSG((IM_STR("invalid parameter, null pointer")));
        return IM_RET_INVALID_PARAMETER;
    }
    if(!VALID_IDSx(idsx)){
        IM_ERRMSG((IM_STR("invalid parameter, idsx=%d"), idsx));
        return IM_RET_INVALID_PARAMETER;
    }

    //
    dd = gIdsInst->idsMgr[idsx].dev;
    if((dd == IM_NULL) || !(dd->flag & DISPDEV_FLAG_CONFIGURED)){
        IM_ERRMSG((IM_STR("device is not set")));
        return IM_RET_FAILED;
    }

    //
    IM_INFOMSG((IM_STR("canvas->pixfmt=0x%x"), canvas->pixfmt));
    IM_INFOMSG((IM_STR("canvas->rotation=%d"), canvas->rotation));
    IM_INFOMSG((IM_STR("canvas->width=%d"), canvas->width));
    IM_INFOMSG((IM_STR("canvas->height=%d"), canvas->height));
    IM_INFOMSG((IM_STR("canvas->xOffset=%d"), canvas->xOffset));
    IM_INFOMSG((IM_STR("canvas->yOffset=%d"), canvas->yOffset));

    if((canvas->pixfmt != IM_PIC_FMT_16BITS_RGB_565) && (canvas->pixfmt != IM_PIC_FMT_32BITS_0RGB_8888)){
        IM_ERRMSG((IM_STR("invalid parameter, canvas->pixfmt=0x%x"), canvas->pixfmt));
        return IM_RET_INVALID_PARAMETER;
    }
    if((canvas->xOffset < 0) || (canvas->width < 0) || ((canvas->xOffset + canvas->width) > dd->config.width) ||
            (canvas->yOffset < 0) || (canvas->height < 0) || ((canvas->yOffset + canvas->height) > dd->config.height) ||
            (canvas->width & 0x7) || (canvas->height & 0x3)){
        IM_ERRMSG((IM_STR("invalid parameter, xOffset=%d, yOffset=%d, width=%d, height=%d"), 
                    canvas->xOffset, canvas->yOffset, canvas->width, canvas->height));
        return IM_RET_INVALID_PARAMETER;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);
    if(idsx == gIdsInst->locidsx){
        IM_ASSERT(IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_to_L_E);
        wlayer_set_canvas(gIdsInst->locidsx, canvas);
        idspwl_memcpy((void *)&vm->config.locCanvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
        idspwl_memcpy((void *)&dd->canvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
    }else{
        if(IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_to_L_E){
            wlayer_set_canvas(gIdsInst->extidsx, canvas);
        }else{
            wlayer = &gIdsInst->idsMgr[gIdsInst->extidsx].wlayer[0];
            if(vm->flag & VMODE_FLAG_EXT_SETUP){
                vmode_trigger_event(IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS, wlayer->idsx, wlayer->wx, (void *)canvas, sizeof(ids_wlayer_canvas_t));
                vm->flag &= ~VMODE_FLAG_STABLE ;
            }
            else {
                idspwl_memcpy((void *)&vm->config.extCanvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
                idspwl_memcpy((void *)&dd->canvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
            }
            if(! VMODE_FLAG_CHECK_STABLE(vm->flag)){
                ret = IM_RET_FALSE;
            }else{
                ret = IM_RET_OK;
            }
        }
    }
    idspwl_lock_unlock(gIdsInst->vmlock);

    return ret;
}

static IM_BOOL vmode_exception_process(void)
{
    vmode_t *vm = &gIdsInst->vmode;
    vmode_evt_internal_t vmevt;
    IM_INFOMSG((IM_STR("%s(vm->flag=0x%x)"), IM_STR(_IM_FUNC_), vm->flag));

    IM_ASSERT(vm->flag & VMODE_FLAG_STABLE);

    //
    if(vm->flag & VMODE_FLAG_HIDELOC){
        fblayer_hide(IM_FALSE);
    }

    //
    if(vm->config.mode == IDS_VMODE_MODE_LOCAL){
        IM_INFOMSG((IM_STR("vmode in local mode, exception process nothing to do.")));
        return IM_FALSE;
    }

    // first change back all extend-window to local.
    vm->config.mode = IDS_VMODE_MODE_LOCAL;
    vmevt.idsx = gIdsInst->locidsx;
    idspwl_memcpy((void *)vmevt.p, &vm->config, sizeof(ids_vmode_config_t));

    if(VMODE_FLAG_CHECK_EXT_VALID(vm->flag)){
        vmevt.wx = 0;
        vmevt.evt = IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE;
        vm->flag &= ~VMODE_FLAG_EXT_SETUP;
        vm->flag &= ~VMODE_FLAG_STABLE;
        IM_INFOMSG((IM_STR("trigger evt IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE")));
        im_list_put_front(gIdsInst->vmode.evtlst, (void *)&vmevt);
    }

    vm->flag &= ~VMODE_FLAG_EXT_SETUP;

    // then put exception to vmode.
    vmevt.evt = IDS_VMODE_EVENT_EXCEPTION;
    vmevt.idsx = -1;
    vmevt.wx = -1;
    im_list_put_back(gIdsInst->vmode.evtlst, (void *)&vmevt);

    return IM_TRUE;
}

static void vmode_trigger_event(IM_INT32 evt, IM_INT32 idsx, IM_INT32 wx, void *p, IM_INT32 size)
{
    vmode_t *vm = &gIdsInst->vmode;
    vmode_evt_internal_t vmevt;
    IM_INFOMSG((IM_STR("%s(evt=0x%x, idsx=%d, wx=%d, size=%d)"), IM_STR(_IM_FUNC_), evt, idsx, wx, size));
    IM_INFOMSG((IM_STR("vm->flag=0x%x"), vm->flag));

    // check vmode init state.
    if(! (vm->flag & VMODE_FLAG_INITED)){
        IM_WARNMSG((IM_STR("vmode is not inited")));
        return;
    }

    //
    vmevt.evt = evt;
    vmevt.idsx = idsx;
    vmevt.wx = wx;
    if(p != IM_NULL){
        IM_ASSERT(size <= sizeof(vmevt.p));
        idspwl_memcpy((void *)vmevt.p, p, size);
    }

    //
    if((vmevt.evt == IDS_VMODE_EVENT_SETMODE_STATUS) ||
            (vmevt.evt == IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE) ||
            (vmevt.evt == IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS) ||
            (vmevt.evt == IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS) ||
            (vmevt.evt == IDS_VMODE_NOTIFY_FBL_SWAPBF) ||
            (vmevt.evt == IDS_VMODE_EVENT_BACK_TO_LOCAL)  ||
            (vmevt.evt == IDS_EXTLAYER_EVENT_BACK_TO_LOCAL))
    { // these events must put to the front.
        im_list_put_front(gIdsInst->vmode.evtlst, (void *)&vmevt);
    }
    else
    {
        if((IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_L_to_E) && (vm->flag & VMODE_FLAG_STABLE) && 
                ((vmevt.evt == IDS_VMODE_EVENT_EXTDEV_BREAK) || (vmevt.evt == IDS_VMODE_EVENT_EXCEPTION)))
        {
            // vm->flag may be change just after call vmode_trigger_event, but the function change vm->flag and the function trigger BREAK&EXCEPTION never same, so it still safe.
            vmode_exception_process();
        }else{
            // if vmode is not stable, BREAK and EXCEPTION still put to the evtlst,
            // but in vmode_server_looper, we blocked these event until vmode be
            // stable. in vmode_server_looper, when received these event, if vmode
            // is stable, it will still do vmode_exception_process. 
            im_list_put_back(gIdsInst->vmode.evtlst, (void *)&vmevt);
        }
    }

    //
    if(im_list_size(gIdsInst->vmode.evtlst) >= 1){
        idspwl_sig_set(gIdsInst->vmode.signal);
    }
}

static void vmode_process_event(vmode_evt_internal_t *vmevt)
{
    vmode_t *vm = &gIdsInst->vmode;
    IM_INT32 mask = 0, size = 0;
    idslib_event_t EVT;
    IM_INFOMSG((IM_STR("%s(evt=0x%x, idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), vmevt->evt, vmevt->idsx, vmevt->wx));

    if(IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_L_to_E){
        if((vmevt->evt == IDS_VMODE_EVENT_LOCDEV_UPDATE) || (vmevt->evt == IDS_VMODE_EVENT_LOCDEV_BREAK)){
            IM_INFOMSG((IM_STR("IDS_MODE_FB_L_to_E ignore the evt 0x%x"), vmevt->evt));
            return;
        }
    }else{ // IDS_MODE_FB_to_L_E
        if((vmevt->evt == IDS_VMODE_EVENT_SETMODE_STATUS) || (vmevt->evt == IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS) ||
                (vmevt->evt == IDS_VMODE_EVENT_EXCEPTION) || (vmevt->evt == IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE) || 
                (vmevt->evt == IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER) ||
                (vmevt->evt == IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS)){
            IM_INFOMSG((IM_STR("IDS_MODE_FB_to_L_E ignore the evt 0x%x"), vmevt->evt));
            return;
        }
    }

    switch(vmevt->evt)	
    {
        case IDS_VMODE_EVENT_EXTDEV_UPDATE:
        case IDS_VMODE_EVENT_EXTDEV_BREAK:
        case IDS_VMODE_EVENT_LOCDEV_UPDATE:
        case IDS_VMODE_EVENT_LOCDEV_BREAK:
        case IDS_VMODE_EVENT_BACK_TO_LOCAL:
            mask |= IDSLIB_VMEVT_MASK_VMODE;
            break;
        case IDS_VMODE_EVENT_EXCEPTION:
        case IDS_VMODE_EVENT_SETMODE_STATUS:
        case IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS:
        case IDS_VMODE_NOTIFY_FBL_SWAPBF:
            mask |= IDSLIB_VMEVT_MASK_VMODE;
            size = sizeof(IM_INT32);
            break;
        case IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE:
        case IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER:
        case IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS:
            fblayer_vmode_listener(vmevt->evt,  vmevt->p);
            break;
        case IDS_EXTLAYER_EVENT_EXTDEV_BREAK:
        case IDS_EXTLAYER_EVENT_BACK_TO_LOCAL:
            mask |= IDSLIB_VMEVT_MASK_EXTLAYER;
            break;
        default:
            break;
    }

    if(mask){
        EVT.evt = vmevt->evt;
        if(size){
            idspwl_memcpy((void *)EVT.param, (void *)vmevt->p, size);
        }

        // to avoid got vmlock then request acslock(in ids_drv), it may be dead-lock,
        // here unlock before calling vm->listener.
        idspwl_lock_unlock(gIdsInst->vmlock);
        vm->listener(mask, vmevt->idsx, vmevt->wx, &EVT);
        idspwl_lock_lock(gIdsInst->vmlock);
    }
}

static void vmode_server_looper(void *p)
{
    IM_RET wait;
    vmode_t *vm = (vmode_t *)p;
    vmode_evt_internal_t *vmevt;
    IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

    idspwl_lock_lock(gIdsInst->vmlock);
    vm->stat = VM_THREAD_STAT_RUN;

    while(1)
    {
        // first check if has stop request.
        if(vm->action == VM_THREAD_ACTION_STOP){
            IM_INFOMSG((IM_STR("VM_THREAD_ACTION_STOP")));
            break;
        }

        //
        wait = idspwl_sig_wait(vm->signal, gIdsInst->vmlock, 500);
        if((wait != IM_RET_OK) && (wait != IM_RET_TIMEOUT)){
            IM_ERRMSG((IM_STR("idspwl_sig_wait() failed")));
            break;
        }else if(wait == IM_RET_TIMEOUT){
            continue;
        }

        //
        vmevt = (vmode_evt_internal_t *)im_list_begin(vm->evtlst);
        IM_INFOMSG((IM_STR("vmode_server_looper get evt=0x%x, vm->flag=0x%x"), vmevt->evt, vm->flag));
        // when vmode state is not stable, only allow send these EVENTs.
        if(!(vm->flag & VMODE_FLAG_STABLE) && 
                ((vmevt->evt != IDS_VMODE_EVENT_SETMODE_STATUS) && 
                 (vmevt->evt != IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS) && 
                 (vmevt->evt != IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE) && 
                 (vmevt->evt != IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS)))
        {
            IM_INFOMSG((IM_STR("vmode_server_looper blocked the event 0x%x"), vmevt->evt));
            idspwl_lock_unlock(gIdsInst->vmlock);
            idspwl_msleep(20);
            idspwl_lock_lock(gIdsInst->vmlock);
        }
        else{
            if((IDS_MODE_FB_MODE(gIdsInst->mode) == IDS_MODE_FB_L_to_E) && (vm->flag & VMODE_FLAG_STABLE) && 
                    ((vmevt->evt == IDS_VMODE_EVENT_EXTDEV_BREAK) || (vmevt->evt == IDS_VMODE_EVENT_EXCEPTION)))
            {
                IM_INFOMSG((IM_STR("vmode_server_looper do vmode_exception_process()")));
                if(vmode_exception_process() == IM_FALSE){
                    vmode_process_event(vmevt);
                    im_list_erase(vm->evtlst, vmevt);
                    if(im_list_size(vm->evtlst) == 0){
                        idspwl_sig_reset(vm->signal);
                    }
                }
            }
            else{
                vmode_process_event(vmevt);
                im_list_erase(vm->evtlst, vmevt);
                if(im_list_size(vm->evtlst) == 0){
                    idspwl_sig_reset(vm->signal);
                }
            }
        }
    }

    vm->stat = VM_THREAD_STAT_UNKNOWN;
    idspwl_lock_unlock(gIdsInst->vmlock);

    IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}

static IM_RET idshw_init(IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    // module enable and reset.
    if(idspwl_module_enable(MODULE_IDSx(idsx)) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idspwl_module_enable(%d) failed"), idsx));
        return IM_RET_FAILED;
    }
    if(idspwl_module_reset(MODULE_IDSx(idsx)) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idspwl_module_reset(%d) failed"), idsx));
        idspwl_module_disable(MODULE_IDSx(idsx));
        return IM_RET_FAILED;
    }

    // default, flush regs directly.
    idspwl_write_regbit(MODULE_IDSx(idsx), OVCDCR, OVCDCR_UpdateReg, 1, 1);

    // flush yuv2rgb matrix.
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOMC, (1<<OVCOMC_ToRGB) | (16<<OVCOMC_oft_b) | (0<<OVCOMC_oft_a));
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF11, 298);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF12, 0);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF13, 409);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF21, 298);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF22, (IM_UINT32)-100);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF23, (IM_UINT32)-208);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF31, 298);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF32, 516);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF33, 0);

    //
    if(ielib_init(&gIdsInst->ie[idsx].handle, idsx) != IM_RET_OK){
        IM_ERRMSG((IM_STR("ielib_init(idsx=%d) failed"), idsx));
        return IM_RET_FAILED;
    }
    gIdsInst->ie[idsx].idsx = idsx;
    gIdsInst->ie[idsx].flag |= IE_FLAG_INITED;

    return IM_RET_OK;
}

static IM_RET idshw_deinit(IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    //
    if(ielib_deinit(&gIdsInst->ie[idsx].handle) != IM_RET_OK){
        IM_ERRMSG((IM_STR("ielib_deinit(idsx=%d) failed"), idsx));
    }
    gIdsInst->ie[idsx].flag &= ~IE_FLAG_INITED;

    // module disable.
    if(idspwl_module_disable(MODULE_IDSx(idsx)) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idspwl_module_disable(%d) failed"), idsx));
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

static void idshw_async_config(IM_INT32 idsx, IM_INT32 wx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d,asyncConfigRefcnt=%d)"), IM_STR(_IM_FUNC_), idsx, wx,gIdsInst->idsMgr[idsx].asyncConfigRefcnt));
    gIdsInst->idsMgr[idsx].wlayer[wx].flag |= WLAYER_FLAG_ASYNC_CONFIG;
    gIdsInst->idsMgr[idsx].asyncConfigRefcnt++;
    if(gIdsInst->idsMgr[idsx].asyncConfigRefcnt == 1){
        idspwl_write_regbit(MODULE_IDSx(idsx), OVCDCR, OVCDCR_UpdateReg, 1, 0);
    }
}

static void idshw_flush_config(IM_INT32 idsx, IM_INT32 wx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d,asyncConfigRefcnt=%d)"), IM_STR(_IM_FUNC_), idsx, wx, gIdsInst->idsMgr[idsx].asyncConfigRefcnt));
    gIdsInst->idsMgr[idsx].wlayer[wx].flag &= ~WLAYER_FLAG_ASYNC_CONFIG;
    gIdsInst->idsMgr[idsx].asyncConfigRefcnt--;
    if(gIdsInst->idsMgr[idsx].asyncConfigRefcnt == 0){
        idspwl_write_regbit(MODULE_IDSx(idsx), OVCDCR, OVCDCR_UpdateReg, 1, 1);
    }
}

static IM_RET idshw_pm_save_regs(IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    idspwl_read_reg(MODULE_IDSx(idsx), OVCDCR, &gIdsInst->pm.osdRegStore[idsx][OVCDCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCPCR, &gIdsInst->pm.osdRegStore[idsx][OVCPCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCBKCOLOR, &gIdsInst->pm.osdRegStore[idsx][OVCBKCOLOR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0CR, &gIdsInst->pm.osdRegStore[idsx][OVCW0CR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0PCAR, &gIdsInst->pm.osdRegStore[idsx][OVCW0PCAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0PCBR, &gIdsInst->pm.osdRegStore[idsx][OVCW0PCBR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0VSSR, &gIdsInst->pm.osdRegStore[idsx][OVCW0VSSR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0CMR, &gIdsInst->pm.osdRegStore[idsx][OVCW0CMR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0B0SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW0B0SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0B1SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW0B1SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0B2SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW0B2SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW0B3SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW0B3SAR]);
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1CR, &gIdsInst->pm.osdRegStore[idsx][OVCW1CR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1PCAR, &gIdsInst->pm.osdRegStore[idsx][OVCW1PCAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1PCBR, &gIdsInst->pm.osdRegStore[idsx][OVCW1PCBR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1PCCR, &gIdsInst->pm.osdRegStore[idsx][OVCW1PCCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1VSSR, &gIdsInst->pm.osdRegStore[idsx][OVCW1VSSR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1CKCR, &gIdsInst->pm.osdRegStore[idsx][OVCW1CKCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1CKR, &gIdsInst->pm.osdRegStore[idsx][OVCW1CKR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1CMR, &gIdsInst->pm.osdRegStore[idsx][OVCW1CMR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1B0SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW1B0SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1B1SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW1B1SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1B2SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW1B2SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW1B3SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW1B3SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2CR, &gIdsInst->pm.osdRegStore[idsx][OVCW2CR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2PCAR, &gIdsInst->pm.osdRegStore[idsx][OVCW2PCAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2PCBR, &gIdsInst->pm.osdRegStore[idsx][OVCW2PCBR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2PCCR, &gIdsInst->pm.osdRegStore[idsx][OVCW2PCCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2VSSR, &gIdsInst->pm.osdRegStore[idsx][OVCW2VSSR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2CKCR, &gIdsInst->pm.osdRegStore[idsx][OVCW2CKCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2CKR, &gIdsInst->pm.osdRegStore[idsx][OVCW2CKR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2CMR, &gIdsInst->pm.osdRegStore[idsx][OVCW2CMR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2B0SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW2B0SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2B1SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW2B1SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2B2SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW2B2SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW2B3SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW2B3SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3CR, &gIdsInst->pm.osdRegStore[idsx][OVCW3CR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3PCAR, &gIdsInst->pm.osdRegStore[idsx][OVCW3PCAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3PCBR, &gIdsInst->pm.osdRegStore[idsx][OVCW3PCBR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3PCCR, &gIdsInst->pm.osdRegStore[idsx][OVCW3PCCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3VSSR, &gIdsInst->pm.osdRegStore[idsx][OVCW3VSSR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3CKCR, &gIdsInst->pm.osdRegStore[idsx][OVCW3CKCR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3CKR, &gIdsInst->pm.osdRegStore[idsx][OVCW3CKR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3CMR, &gIdsInst->pm.osdRegStore[idsx][OVCW3CMR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3B0SAR, &gIdsInst->pm.osdRegStore[idsx][OVCW3B0SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCW3SABSAR, &gIdsInst->pm.osdRegStore[idsx][OVCW3SABSAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCBRB0SAR, &gIdsInst->pm.osdRegStore[idsx][OVCBRB0SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCBRB1SAR, &gIdsInst->pm.osdRegStore[idsx][OVCBRB1SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCBRB2SAR, &gIdsInst->pm.osdRegStore[idsx][OVCBRB2SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCBRB3SAR, &gIdsInst->pm.osdRegStore[idsx][OVCBRB3SAR]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOMC, &gIdsInst->pm.osdRegStore[idsx][OVCOMC]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF11, &gIdsInst->pm.osdRegStore[idsx][OVCOEF11]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF12, &gIdsInst->pm.osdRegStore[idsx][OVCOEF12]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF13, &gIdsInst->pm.osdRegStore[idsx][OVCOEF13]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF21, &gIdsInst->pm.osdRegStore[idsx][OVCOEF21]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF22, &gIdsInst->pm.osdRegStore[idsx][OVCOEF22]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF23, &gIdsInst->pm.osdRegStore[idsx][OVCOEF23]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF31, &gIdsInst->pm.osdRegStore[idsx][OVCOEF31]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF32, &gIdsInst->pm.osdRegStore[idsx][OVCOEF32]);	
    idspwl_read_reg(MODULE_IDSx(idsx), OVCOEF33, &gIdsInst->pm.osdRegStore[idsx][OVCOEF33]);	

    return IM_RET_OK;
}

static IM_RET idshw_pm_restore_regs(IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    idspwl_write_reg(MODULE_IDSx(idsx), OVCDCR, gIdsInst->pm.osdRegStore[idsx][OVCDCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCPCR, gIdsInst->pm.osdRegStore[idsx][OVCPCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCBKCOLOR, gIdsInst->pm.osdRegStore[idsx][OVCBKCOLOR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0CR, gIdsInst->pm.osdRegStore[idsx][OVCW0CR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0PCAR, gIdsInst->pm.osdRegStore[idsx][OVCW0PCAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0PCBR, gIdsInst->pm.osdRegStore[idsx][OVCW0PCBR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0VSSR, gIdsInst->pm.osdRegStore[idsx][OVCW0VSSR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0CMR, gIdsInst->pm.osdRegStore[idsx][OVCW0CMR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0B0SAR, gIdsInst->pm.osdRegStore[idsx][OVCW0B0SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0B1SAR, gIdsInst->pm.osdRegStore[idsx][OVCW0B1SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0B2SAR, gIdsInst->pm.osdRegStore[idsx][OVCW0B2SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW0B3SAR, gIdsInst->pm.osdRegStore[idsx][OVCW0B3SAR]);
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CR, gIdsInst->pm.osdRegStore[idsx][OVCW1CR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1PCAR, gIdsInst->pm.osdRegStore[idsx][OVCW1PCAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1PCBR, gIdsInst->pm.osdRegStore[idsx][OVCW1PCBR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1PCCR, gIdsInst->pm.osdRegStore[idsx][OVCW1PCCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1VSSR, gIdsInst->pm.osdRegStore[idsx][OVCW1VSSR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CKCR, gIdsInst->pm.osdRegStore[idsx][OVCW1CKCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CKR, gIdsInst->pm.osdRegStore[idsx][OVCW1CKR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CMR, gIdsInst->pm.osdRegStore[idsx][OVCW1CMR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1B0SAR, gIdsInst->pm.osdRegStore[idsx][OVCW1B0SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1B1SAR, gIdsInst->pm.osdRegStore[idsx][OVCW1B1SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1B2SAR, gIdsInst->pm.osdRegStore[idsx][OVCW1B2SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW1B3SAR, gIdsInst->pm.osdRegStore[idsx][OVCW1B3SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2CR, gIdsInst->pm.osdRegStore[idsx][OVCW2CR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2PCAR, gIdsInst->pm.osdRegStore[idsx][OVCW2PCAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2PCBR, gIdsInst->pm.osdRegStore[idsx][OVCW2PCBR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2PCCR, gIdsInst->pm.osdRegStore[idsx][OVCW2PCCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2VSSR, gIdsInst->pm.osdRegStore[idsx][OVCW2VSSR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2CKCR, gIdsInst->pm.osdRegStore[idsx][OVCW2CKCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2CKR, gIdsInst->pm.osdRegStore[idsx][OVCW2CKR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2CMR, gIdsInst->pm.osdRegStore[idsx][OVCW2CMR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2B0SAR, gIdsInst->pm.osdRegStore[idsx][OVCW2B0SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2B1SAR, gIdsInst->pm.osdRegStore[idsx][OVCW2B1SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2B2SAR, gIdsInst->pm.osdRegStore[idsx][OVCW2B2SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW2B3SAR, gIdsInst->pm.osdRegStore[idsx][OVCW2B3SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3CR, gIdsInst->pm.osdRegStore[idsx][OVCW3CR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3PCAR, gIdsInst->pm.osdRegStore[idsx][OVCW3PCAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3PCBR, gIdsInst->pm.osdRegStore[idsx][OVCW3PCBR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3PCCR, gIdsInst->pm.osdRegStore[idsx][OVCW3PCCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3VSSR, gIdsInst->pm.osdRegStore[idsx][OVCW3VSSR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3CKCR, gIdsInst->pm.osdRegStore[idsx][OVCW3CKCR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3CKR, gIdsInst->pm.osdRegStore[idsx][OVCW3CKR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3CMR, gIdsInst->pm.osdRegStore[idsx][OVCW3CMR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3B0SAR, gIdsInst->pm.osdRegStore[idsx][OVCW3B0SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCW3SABSAR, gIdsInst->pm.osdRegStore[idsx][OVCW3SABSAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCBRB0SAR, gIdsInst->pm.osdRegStore[idsx][OVCBRB0SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCBRB1SAR, gIdsInst->pm.osdRegStore[idsx][OVCBRB1SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCBRB2SAR, gIdsInst->pm.osdRegStore[idsx][OVCBRB2SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCBRB3SAR, gIdsInst->pm.osdRegStore[idsx][OVCBRB3SAR]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOMC, gIdsInst->pm.osdRegStore[idsx][OVCOMC]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF11, gIdsInst->pm.osdRegStore[idsx][OVCOEF11]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF12, gIdsInst->pm.osdRegStore[idsx][OVCOEF12]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF13, gIdsInst->pm.osdRegStore[idsx][OVCOEF13]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF21, gIdsInst->pm.osdRegStore[idsx][OVCOEF21]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF22, gIdsInst->pm.osdRegStore[idsx][OVCOEF22]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF23, gIdsInst->pm.osdRegStore[idsx][OVCOEF23]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF31, gIdsInst->pm.osdRegStore[idsx][OVCOEF31]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF32, gIdsInst->pm.osdRegStore[idsx][OVCOEF32]);	
    idspwl_write_reg(MODULE_IDSx(idsx), OVCOEF33, gIdsInst->pm.osdRegStore[idsx][OVCOEF33]);	

    return IM_RET_OK;
}

static IM_RET wlayer_open(IM_INT32 idsx, IM_INT32 wx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));

    //
    if(wx == 0){
        idspwl_write_reg(MODULE_IDSx(idsx), OVCW0CMR, 0x0000000);
    }else{
        idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CMR + 0x80 * (wx - 1), 0x0000000);
    }

    //
    if(gIdsInst->idsMgr[idsx].wlayer[wx].flag & WLAYER_FLAG_MMU_INITED){
        if(idspwl_enable_mmu(idsx, wx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idspwl_enable_mmu() failed")));
            return IM_RET_FAILED;
        }
        gIdsInst->idsMgr[idsx].wlayer[wx].flag |= WLAYER_FLAG_MMU_ENABLED;
    }
    idspwl_write_regbit(MODULE_IDSx(idsx), OVCW0CR + 0x80 * wx, OVCWxCR_ENWIN, 1, 1);

    if((gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED) && 
            !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_BREAKED)){
        idslib_vsync(idsx);
    }
    return IM_RET_OK;
}

static IM_RET wlayer_prepare_close(IM_INT32 idsx, IM_INT32 wx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));
    if(wx == 0){
        idspwl_write_reg(MODULE_IDSx(idsx), OVCW0CMR, 0x1000000);
    }else{
        idspwl_write_reg(MODULE_IDSx(idsx), OVCW1CMR + 0x80 * (wx - 1), 0x1000000);
    }

    if((gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED) &&
            !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_BREAKED)){
        idslib_vsync(idsx);
    }

    return IM_RET_OK;
}

static IM_RET wlayer_close(IM_INT32 idsx, IM_INT32 wx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));

    idspwl_write_regbit(MODULE_IDSx(idsx), OVCW0CR + 0x80 * wx, OVCWxCR_ENWIN, 1, 0);

    // here, we must wait at least 1 frame sync before disable MMU.
    /*if((gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED) &&
      !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_BREAKED)){
      idslib_vsync(idsx);
      }*/

    if(gIdsInst->idsMgr[idsx].wlayer[wx].flag & WLAYER_FLAG_MMU_ENABLED){
        // if not sleep for a while , sometimes mmu disable would be failed
        // But how ? Before wlayer_close, we has just call wlayer_prepare_close to disable ddr reading.
        idspwl_msleep(20); 
        if(idspwl_disable_mmu(idsx, wx) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idspwl_disable_mmu() failed")));
            return IM_RET_FAILED;
        }
        gIdsInst->idsMgr[idsx].wlayer[wx].flag &= ~WLAYER_FLAG_MMU_ENABLED;
    }
    return IM_RET_OK;
}

static IM_RET wlayer_set_canvas(IM_INT32 idsx, ids_wlayer_canvas_t *canvas)
{
    IM_INT32 i, x0, y0, x1, y1;
    wlayer_t *wlayer;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    for(i=0; i<4; i++){
        wlayer = &gIdsInst->idsMgr[idsx].wlayer[i];
        if(wlayer->flag & WLAYER_FLAG_OPENED){
            x0 = wlayer->pano.pos.left + canvas->xOffset;
            y0 = wlayer->pano.pos.top + canvas->yOffset;
            x1 = x0 + wlayer->pano.pos.width - 1;
            y1 = y0 + wlayer->pano.pos.height - 1;

            idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCW0PCAR, (x0 << OVCWxPCAR_LeftTopX) | (y0 << OVCWxPCAR_LeftTopY));
            idspwl_write_reg(MODULE_IDSx(wlayer->idsx), OVCW0PCBR, (x1 << OVCWxPCBR_RightBotX) | (y1 << OVCWxPCBR_RightBotY));	
        }
    }

    return IM_RET_OK;	
}

static void effic_init(IM_INT32 idsx)
{
    IM_BOOL is16bit;
    effic_t *effic = &gIdsInst->effic;
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    //
    if((dd == IM_NULL) || !(dd->flag & DISPDEV_FLAG_CONFIGURED)){
        effic->workMode[idsx] = IDSLIB_WORKMODE_IDLE;
        return;
    }

    //
    is16bit = (dd->canvas.pixfmt == IM_PIC_FMT_16BITS_RGB_565) ? IM_TRUE : IM_FALSE;

    //
    if((dd->config.width > 1280) || (dd->config.height > 720)){
        effic->workMode[idsx] = is16bit ? IDSLIB_WORKMODE_1080P_16BIT : IDSLIB_WORKMODE_1080P_32BIT;
    }else if((dd->config.width > 720) || (dd->config.height > 576)){
        effic->workMode[idsx] = is16bit ? IDSLIB_WORKMODE_720P_16BIT : IDSLIB_WORKMODE_720P_32BIT;
    }else if((dd->config.width > 640) || (dd->config.height > 480)){
        effic->workMode[idsx] = is16bit ? IDSLIB_WORKMODE_576P_16BIT : IDSLIB_WORKMODE_576P_32BIT;
    }else{
        effic->workMode[idsx] = is16bit ? IDSLIB_WORKMODE_VGA_16BIT : IDSLIB_WORKMODE_VGA_32BIT;
    }

    return;
}

static void effic_deinit(IM_INT32 idsx)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
    gIdsInst->effic.workMode[idsx] = IDSLIB_WORKMODE_IDLE;
}

IM_INLINE static IM_RET effic_bus_config(void)
{
    effic_t *effic = &gIdsInst->effic;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(idspwl_set_bus_config(effic->busEffic, gIdsInst->idsMgr[0].dev ? gIdsInst->idsMgr[0].dev->type : -1, effic->workMode[0], gIdsInst->idsMgr[1].dev ? gIdsInst->idsMgr[1].dev->type : -1, effic->workMode[1]) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idspwl_set_bus_config() failed")));
        return IM_RET_FAILED;
    }

    return IM_RET_OK;
}

static IM_RET make_default_canvas(IM_INT32 idsx, ids_display_device_t *dev, ids_wlayer_canvas_t *canvas)
{
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    if(idsx == gIdsInst->extidsx){
        if(gIdsInst->effic.busEffic == IDSLIB_BUS_EFFIC_LEVEL_1){
            if((dev->width > 1280) || (dev->height > 720)){
                canvas->pixfmt = IM_PIC_FMT_16BITS_RGB_565;
            }else{
                canvas->pixfmt = IM_PIC_FMT_32BITS_0RGB_8888;
            }
        }
        else if(gIdsInst->effic.busEffic == IDSLIB_BUS_EFFIC_LEVEL_2){
            if((dev->width >= 1280) || (dev->height >= 720)){
                canvas->pixfmt = IM_PIC_FMT_16BITS_RGB_565;
            }else{
                canvas->pixfmt = IM_PIC_FMT_32BITS_0RGB_8888;
            }
        }
        else if(gIdsInst->effic.busEffic == IDSLIB_BUS_EFFIC_LEVEL_3){
            if((dev->width > 720) || (dev->height > 576)){
                canvas->pixfmt = IM_PIC_FMT_16BITS_RGB_565;
            }else{
                canvas->pixfmt = IM_PIC_FMT_32BITS_0RGB_8888;
            }
        }
        else {
            canvas->pixfmt = IM_PIC_FMT_16BITS_RGB_565;
        }
    }else{
        canvas->pixfmt = IM_PIC_FMT_32BITS_0RGB_8888;
    }

    canvas->rotation = IDS_ROTATION_0;
    canvas->width = dev->width & ~0x07;
    canvas->height = dev->height & ~0x3;
    canvas->xOffset = (dev->width - canvas->width) / 2;
    canvas->yOffset = (dev->height & 0x3) >> 1;

    IM_INFOMSG((IM_STR("default canvas: pixfmt=0x%x, rotation=%d, width=%d, height=%d, xOffset=%d, yOffset=%d"), canvas->pixfmt, canvas->rotation, canvas->width, canvas->height, canvas->xOffset, canvas->yOffset));

    return IM_RET_OK;
}

static IM_RET device_set_default(IM_INT32 idsx)
{
    IM_INT32 prefer;
    dispdev_t *dd;
    ids_display_device_t *cfg;
    IM_INT32 type, devid;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    if((idsx == gIdsInst->locidsx) && (gIdsInst->config.locDevNum > 0)){
        prefer = gIdsInst->config.locDevPrefer;
        if(gIdsInst->config.locDevs[prefer].id != IDS_DISPDEV_ID_DYNAMIC){
            return device_init(idsx, gIdsInst->config.locDevs[prefer].type, gIdsInst->config.locDevs[prefer].id);
        }
    }else if((idsx == gIdsInst->extidsx) && (gIdsInst->config.extDevNum > 0)){
        prefer = gIdsInst->config.extDevPrefer;
        if(gIdsInst->config.extDevs[prefer].id != IDS_DISPDEV_ID_DYNAMIC){
            return device_init(idsx, gIdsInst->config.extDevs[prefer].type, gIdsInst->config.extDevs[prefer].id);
        }
        else{
            dd = &gIdsInst->devs[idsx][gIdsInst->config.extDevs[prefer].type];
            if (dd->cfglst != IM_NULL && (im_list_size(dd->cfglst) != 0)){
                cfg = (ids_display_device_t *)im_list_begin(dd->cfglst);
                type = cfg->type;
                devid = cfg->id;
                while (cfg != IM_NULL){
                    if (cfg->width >= 1280 && cfg->height >= 720){
                        devid = cfg->id;
                    }
                    if (cfg->width == 1280 && cfg->height == 720 && dd->type == IDS_DISPDEV_TYPE_HDMI){
                        break;
                    }
                    cfg = (ids_display_device_t *)im_list_next(dd->cfglst);
                }
                return device_init(idsx, type, devid);
            }
        }
    }

    return IM_RET_FAILED;
}

static IM_RET device_get_default(IM_INT32 idsx, ids_display_device_t *dev, ids_wlayer_canvas_t *canvas)
{
    IM_INT32 prefer;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    if((idsx == gIdsInst->locidsx) && (gIdsInst->config.locDevNum > 0)){
        prefer = gIdsInst->config.locDevPrefer;
        if(gIdsInst->config.locDevs[prefer].id != IDS_DISPDEV_ID_DYNAMIC){
            idspwl_memcpy((void *)dev, (void *)&gIdsInst->config.locDevs[prefer], sizeof(ids_display_device_t));
            make_default_canvas(idsx, dev, canvas);
            return IM_RET_OK;
        }
    }else if((idsx == gIdsInst->extidsx) && (gIdsInst->config.extDevNum > 0)){
        prefer = gIdsInst->config.extDevPrefer;
        if(gIdsInst->config.extDevs[prefer].id != IDS_DISPDEV_ID_DYNAMIC){
            idspwl_memcpy((void *)dev, (void *)&gIdsInst->config.extDevs[prefer], sizeof(ids_display_device_t));
            make_default_canvas(idsx, dev, canvas);
            return IM_RET_OK;
        }
    }

    return IM_RET_FAILED;
}

static IM_RET device_init(IM_INT32 idsx, IM_INT32 devtype, IM_INT32 devid)
{
    ids_display_device_t *cfg;
    IM_INT32 suffix;
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;
    IM_INFOMSG((IM_STR("%s(idsx=%d, devtype=%d, devid=0x%x)"), IM_STR(_IM_FUNC_), idsx, devtype, devid));

    // if set the same config, don't do anything.
    if((dd != IM_NULL) && (dd->flag & DISPDEV_FLAG_CONFIGURED) && (dd->type == devtype) && (dd->config.id == devid)){
        IM_INFOMSG((IM_STR("ids%d has selected devtype=%d, devid=0x%x"), idsx, devtype, devid));
        return IM_RET_OK;
    }
    else if((dd != IM_NULL) && (dd->type != devtype)){
        // if dev calss is different, first deinit current device class, then init the new device class.
        IM_ASSERT(dd->handle != IM_NULL);
        IM_ASSERT(!(dd->flag & DISPDEV_FLAG_OPENED));
        if(!(dd->flag & DISPDEV_FLAG_LISTENING)){
            if(dd->intf->device_deinit(dd->handle) != IM_RET_OK){
                IM_ERRMSG((IM_STR("ids%d devtype=%d device_deinit() failed"), idsx, dd->type));
                return IM_RET_FAILED;
            }
            dd->handle = IM_NULL;
            dd->flag &= ~DISPDEV_FLAG_INITED;
        }
        dd = IM_NULL;
        gIdsInst->idsMgr[idsx].dev = IM_NULL;
    }

    if(dd == IM_NULL){
        dd = &gIdsInst->devs[idsx][devtype];
        dd->flag &= ~DISPDEV_FLAG_CONFIGURED;
        if(!(dd->flag & DISPDEV_FLAG_INITED)){
            if(dd->intf->device_init(&dd->handle, idsx) != IM_RET_OK){
                IM_ERRMSG((IM_STR("ids%d devtype=%d device_init() failed"), idsx, devtype));
                return IM_RET_FAILED;
            }
            dd->flag |= DISPDEV_FLAG_INITED;
        }
        gIdsInst->idsMgr[idsx].dev = dd;
    }

    //
    suffix = dd->suffixStart;
    cfg = (ids_display_device_t *)im_list_begin(dd->cfglst);
    while(cfg != IM_NULL){
        if(cfg->id == devid){
            break;
        }
        cfg = (ids_display_device_t *)im_list_next(dd->cfglst);
        suffix++;
    }
    if(cfg == IM_NULL){ // has not got the devid.
        IM_ERRMSG((IM_STR("ids%d devtype=%d don't found devid=0x%x"), idsx, devtype, devid));
        return IM_RET_FAILED;
    }

    //
    if(dd->dynamic){
        suffix = dd->suffixStart;
    }
    if(dd->intf->device_get_config(dd->handle, cfg, &dd->info, suffix) != IM_RET_OK){
        IM_ERRMSG((IM_STR("ids%d devtype=%d devid=0x%x device_get_config() failed"), idsx, devtype, devid));
        return IM_RET_FAILED;
    }
    IM_INFOMSG((IM_STR("device_init: idsx=%d, type=%d, id=0x%x, width=%d, height=%d"), idsx, cfg->type, cfg->id, cfg->width, cfg->height));
    make_default_canvas(idsx, cfg, &dd->canvas);
    idspwl_memcpy((void *)&dd->config, (void *)cfg, sizeof(ids_display_device_t));
    dd->flag |= DISPDEV_FLAG_CONFIGURED;
    dd->suffix = suffix;

    return IM_RET_OK;
}

static IM_RET device_register_listener(IM_INT32 idsx, fcbk_dispdev_listener_t listener)
{
    IM_INT32 j;
    dispdev_t *dd;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

    for(j=0; j<IDS_DISPDEV_TYPE_MAX; j++)
    {
        dd = &gIdsInst->devs[idsx][j];
        if(dd->dynamic && !(dd->flag & DISPDEV_FLAG_LISTENING))
        {
            if(!(gIdsInst->idsMgr[idsx].flag & IDSMGR_FLAG_HW_INITED)){
                if(idshw_init(idsx) != IM_RET_OK){
                    IM_ERRMSG((IM_STR("idshw_init(idsx=%d) failed"), idsx));	
                    return IM_RET_FAILED;
                }
                gIdsInst->idsMgr[idsx].flag |= IDSMGR_FLAG_HW_INITED;
            }

            if(dd->handle == IM_NULL){
                if(dd->intf->device_init(&dd->handle, idsx) != IM_RET_OK){
                    IM_ERRMSG((IM_STR("ids%d devtype=%d device_init() failed"), idsx, j));
                    continue;
                }
                dd->flag |= DISPDEV_FLAG_INITED;
            }
            if(dd->intf->device_register_listener(dd->handle, listener) != IM_RET_OK){
                IM_ERRMSG((IM_STR("ids%d devtype=%d device_register_listener() failed"), idsx, j));
                continue;
            }
            dd->flag |= DISPDEV_FLAG_LISTENING;
        }
    }

    return IM_RET_OK;
}

static IM_RET device_open(IM_INT32 idsx)
{
    //effic_t *effic = &gIdsInst->effic;
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
    IM_ASSERT((dd != IM_NULL) && (dd->handle != IM_NULL)); 
    IM_ASSERT(dd->flag & DISPDEV_FLAG_CONFIGURED)

        //
        effic_init(idsx);
    if(effic_bus_config() != IM_RET_OK){
        IM_ERRMSG((IM_STR("effic_bus_config() failed")));
        effic_deinit(idsx);
        return IM_RET_FAILED;
    }

    //
    if(dd->intf->device_set_config(dd->handle, &dd->config, dd->suffix) != IM_RET_OK){
        IM_ERRMSG((IM_STR("device_set_config() failed, type=%d, id=0x%x"), dd->config.type, dd->config.id));
        goto Fail;
    }
    if(dd->intf->device_open(dd->handle) != IM_RET_OK){
        IM_ERRMSG((IM_STR("device_open() failed, type=%d, id=0x%x"), dd->config.type, dd->config.id));
        goto Fail;
    }

    // ie setting.
    IM_ASSERT(gIdsInst->ie[idsx].flag & IE_FLAG_INITED);
    ielib_set_width_height(gIdsInst->ie[idsx].handle, dd->config.width, dd->config.height);

    // dither.
    if((dd->canvas.pixfmt == IM_PIC_FMT_32BITS_0RGB_8888) && (dd->info.dataport != DISPDEV_DATAPORT_RGB888)){
        if(ielib_dither_set(gIdsInst->ie[idsx].handle, dd->info.dataport, IM_TRUE) != IM_RET_OK){
            IM_ERRMSG((IM_STR("ielib_dither_set(enable) failed")));
        }else{
            IM_INFOMSG((IM_STR("ie set dither(enable, dataport=%d)"), dd->info.dataport));
            gIdsInst->ie[idsx].flag |= IE_FLAG_DITHER_OPENED;
        }
    }else{
        if(ielib_dither_set(gIdsInst->ie[idsx].handle, 0, IM_FALSE) != IM_RET_OK){
            IM_ERRMSG((IM_STR("ielib_dither_set(disable) failed")));
        }else{
            gIdsInst->ie[idsx].flag &= ~IE_FLAG_DITHER_OPENED;
        }
    }
    // acm 
    if (ielib_acm_set(gIdsInst->ie[idsx].handle, dd->info.dftacm) != IM_RET_OK){
        IM_ERRMSG((IM_STR("ie set dither(disable) failed")));
    }

    //
    idspwl_enable_frame_intr(idsx);

    return IM_RET_OK;
Fail:
    effic_deinit(idsx);
    effic_bus_config();
    return IM_RET_FAILED;
}

static IM_RET device_close(IM_INT32 idsx, IM_INT32 ignoreWx, IM_BOOL force)
{
    IM_INT32 i;
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;

    IM_INFOMSG((IM_STR("%s(idsx=%d, ignoreWx=%d)"), IM_STR(_IM_FUNC_), idsx, ignoreWx));
    IM_ASSERT((dd != IM_NULL) && (dd->handle != IM_NULL));    

    if(!force){
        for(i=0; i<4; i++){
            if(i != ignoreWx){
                if(gIdsInst->idsMgr[idsx].wlayer[i].flag & WLAYER_FLAG_OPENED){
                    return IM_RET_FALSE;
                }
            }
        }
    }

    //
    if(dd->intf->device_close(dd->handle) != IM_RET_OK){
        IM_ERRMSG((IM_STR("device_close() failed")));
        return IM_RET_FAILED;				
    }
    if(dd->flag & DISPDEV_FLAG_BREAKED){
        dd->flag &= ~DISPDEV_FLAG_BREAKED;
    }

    //
    if(gIdsInst->ie[idsx].flag & IE_FLAG_DITHER_OPENED){
        if(ielib_dither_set(gIdsInst->ie[idsx].handle, 0, IM_FALSE) != IM_RET_OK){
            IM_ERRMSG((IM_STR("ielib_dither_set(disable) failed")));
        }else{
            gIdsInst->ie[idsx].flag &= ~IE_FLAG_DITHER_OPENED;
        }
    }

    //
    effic_deinit(idsx);
    effic_bus_config();

    idspwl_disable_frame_intr(idsx);

    return IM_RET_OK;
}

static IM_RET device_suspend(IM_INT32 idsx)
{
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
    dd->intf->device_suspend(dd->handle);
    return IM_RET_OK;
}

static IM_RET device_resume(IM_INT32 idsx)
{
    dispdev_t *dd = gIdsInst->idsMgr[idsx].dev;
    IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));
    dd->intf->device_resume(dd->handle);
    return IM_RET_OK;
}

static IM_RET device_listener(dispdev_handle_t handle, IM_UINT32 evt, void *p)
{
    IM_BOOL extTrigger; 
    IM_INT32 VEevt;
    IM_INT32 idsx=3/*invalid idsx*/, devtype, i, num;
    ids_display_device_t *devs;
    dispdev_t *dd;
    IM_INFOMSG((IM_STR("%s(evt=0x%x)"), IM_STR(_IM_FUNC_), evt));

    extTrigger = IM_FALSE;
    //
    if((evt != DISPDEV_EVT_BREAK) && (evt != DISPDEV_EVT_CONFIG_UPDATE)){
        IM_ERRMSG((IM_STR("invalid dev evt 0x%x"), evt));
        return IM_RET_FAILED;
    }

    //
    idspwl_lock_lock(gIdsInst->vmlock);

    for(i=0; i<2; i++){
        for(devtype=0; devtype<IDS_DISPDEV_TYPE_MAX; devtype++){
            if(handle == gIdsInst->devs[i][devtype].handle){
                idsx = i;
                break;
            }
        }
    }
    if(idsx >= 2){
        IM_ERRMSG((IM_STR("invalid handle")));
        goto Fail;
    }
    IM_INFOMSG((IM_STR("device_listener, idsx=%d, devtype=%d"), idsx, devtype));

    //
    if(evt == DISPDEV_EVT_BREAK){
        if((gIdsInst->idsMgr[idsx].dev == IM_NULL) || !(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED)){
            IM_WARNMSG((IM_STR("ids%d is not opened, so don't do BREAK notify"), idsx));
            goto Fail;
        }
        VEevt = (idsx == gIdsInst->locidsx) ? IDS_VMODE_EVENT_LOCDEV_BREAK : IDS_VMODE_EVENT_EXTDEV_BREAK;
        if(gIdsInst->idsMgr[idsx].dev->flag & DISPDEV_FLAG_OPENED){
            gIdsInst->idsMgr[idsx].dev->flag |= DISPDEV_FLAG_BREAKED;
        }
    }
    else if(evt == DISPDEV_EVT_CONFIG_UPDATE){
        IM_ASSERT(gIdsInst->devs[idsx][devtype].flag & DISPDEV_FLAG_LISTENING);
        num = ((dispdev_evt_config_update_t *)p)->num;
        devs = ((dispdev_evt_config_update_t *)p)->devs;
        dd = &gIdsInst->devs[idsx][devtype];
        im_list_clear(dd->cfglst);
        for(i=0; i<num; i++){
            IM_INFOMSG((IM_STR("devs[%d].type=%d, id=0x%x, width=%d, height=%d, fpsx1000=%d"),
                        i, devs[i].type, devs[i].id, devs[i].width, devs[i].height, devs[i].fpsx1000));
            if(idsx == gIdsInst->extidsx){
                if(gIdsInst->effic.busEffic == IDSLIB_BUS_EFFIC_LEVEL_4){
                    if((devs[i].width > 1280) || (devs[i].height > 720)){
                        continue;
                    }
                }
            }
            im_list_put_back(dd->cfglst, (void *)&devs[i]);
        }
        VEevt = (idsx == gIdsInst->locidsx) ? IDS_VMODE_EVENT_LOCDEV_UPDATE : IDS_VMODE_EVENT_EXTDEV_UPDATE;
        if (num == 0 && VEevt == IDS_VMODE_EVENT_EXTDEV_UPDATE){
            extTrigger = IM_TRUE;
        }
    }

    vmode_trigger_event(VEevt, -1, -1, IM_NULL, 0);
    if (extTrigger && (gIdsInst->mode & IDS_MODE_EXTLAYER_WALL_USE)){
        VEevt = IDS_EXTLAYER_EVENT_EXTDEV_BREAK;
        vmode_trigger_event(VEevt, -1, -1, IM_NULL, 0);
    }
    idspwl_lock_unlock(gIdsInst->vmlock);
    return IM_RET_OK;
Fail:
    idspwl_lock_unlock(gIdsInst->vmlock);
    return IM_RET_FAILED;
}

static IM_RET fblayer_vmode_listener(IM_UINT32 evt, void *p)
{
    ids_vmode_config_t *vmcfg;
    IM_BOOL ignore = IM_FALSE;
    IM_INT32 pixfmt, mode, status, size = 0;
    IM_Buffer *oriBuffer, *dstBuffer;
    ids_wlayer_canvas_t *canvas;
    IM_BOOL realloc = IM_FALSE;
    ids_wlayer_panorama_t pano;
    vmode_t * vmode = &gIdsInst->vmode;
    bank_check bch;
    dispdev_t *dd = gIdsInst->idsMgr[gIdsInst->extidsx].dev;
    g2d_image_t src, dst;

    IM_INFOMSG((IM_STR("%s() evt=0x%x"), IM_STR(_IM_FUNC_),evt));

    if(evt == IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE){
        vmcfg = (ids_vmode_config_t *)p;
        mode = IDS_VMODE_MODE(vmcfg->mode);
        if (mode == IDS_VMODE_MODE_LOCAL){
            printk("sam : fblayer_vmode_listener back to local ##### \n");
            if (vmode->extfblayer != IM_NULL){
                ext_fblayer_destroy(vmode);
            }
            vmode->extDevSel = IM_FALSE;
            gIdsInst->mode &= ~IDS_MODE_VMODE_USE;
        }
        else if (mode == IDS_VMODE_MODE_PROJECTION){
            IM_ASSERT(vmode->extfblayer == IM_NULL);
            if (ext_fblayer_create(vmode, vmcfg) != IM_RET_OK){
                IM_ERRMSG((IM_STR(" ext_fblayer_create() failed - ")));
                goto Fail;
            }
            gIdsInst->mode |= IDS_MODE_VMODE_USE;
        }
        vmode->flag |= VMODE_FLAG_STABLE;
        if ( ignore == IM_FALSE){
            status = IDS_VMODE_SETMODE_OK;
            vmode_trigger_event(IDS_VMODE_EVENT_SETMODE_STATUS, -1, -1 , &status, sizeof(IM_INT32));
        }
    }
    else if (evt == IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER){
        IM_ASSERT(vmode->extfblayer);

        oriBuffer = (IM_Buffer *)p;
        if (vmode->shareBuffer){
            dstBuffer = oriBuffer;
        }
        else{
            if (vmode->buffAIsBack){
                dstBuffer = &vmode->buffA;
                vmode->buffAIsBack = IM_FALSE;
            }
            else {
                dstBuffer = &vmode->buffB;
                vmode->buffAIsBack = IM_TRUE;
            }
            memset((void*)&src, 0, sizeof(g2d_image_t));
            memset((void*)&dst, 0, sizeof(g2d_image_t));
            src.format = vmode->config.locCanvas.pixfmt;
            src.width = vmode->config.locCanvas.width;
            src.height = vmode->config.locCanvas.height;
            src.clipRect.left = 0;
            src.clipRect.top = 0;
            src.clipRect.width = src.width;;
            src.clipRect.height = src.height;
            memcpy((void*)&(src.buffer[0]), (void*)oriBuffer, sizeof(IM_Buffer));
            dst.format = vmode->config.extCanvas.pixfmt;
            dst.width = vmode->config.extCanvas.width;
            dst.height = vmode->config.extCanvas.height;
            dst.clipRect.left = 0;
            dst.clipRect.top = 0;
            dst.clipRect.width = dst.width;
            dst.clipRect.height = dst.height;
            idspwl_memcpy((void *)&(dst.buffer[0]),(void *)dstBuffer, sizeof (IM_Buffer));
            if (g2dkapi_do_scale(g2dinst, &src, &dst, vmode->eeval) != IM_RET_OK){
                IM_ERRMSG((IM_STR("##### g2dkapi_do_scale failed, check it !!!!!")));
                goto Fail;
            }
        }
        idspwl_memcpy((void*)&vmode->extfblayer->showBuffer, dstBuffer, sizeof(IM_Buffer));

        idshw_async_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);
        idslib_wlayer_set_buffer(vmode->extfblayer->wlayer, dstBuffer, IM_NULL);
        idshw_flush_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);

        if (vmode->waitSwapFbBuffer){
            if (vmode->flag & VMODE_FLAG_HIDELOC){
                fblayer_hide(IM_TRUE);
            }
            if (wlayer_open_internal(vmode->extfblayer->wlayer) != IM_RET_OK){
                IM_ERRMSG((IM_STR(" wlayer_open_internal failed ")));
                goto Fail;
            }
#ifdef USE_WINMIX 
            idslib_winmix_open(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wmix.wxs);
#endif
            vmode->waitSwapFbBuffer = IM_FALSE;
        }
        status = IDS_VMODE_FBL_SWPBF_OK;
        vmode_trigger_event(IDS_VMODE_NOTIFY_FBL_SWAPBF, vmode->extfblayer->wlayer->idsx, 
                vmode->extfblayer->wlayer->wx, &status, sizeof(IM_INT32));
    }
    else if (evt == IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS)
    {

        IM_ASSERT((vmode->config.mode & 0xff) != IDS_VMODE_MODE_LOCAL);
        IM_ASSERT(vmode->extfblayer != IM_NULL);

        canvas = (ids_wlayer_canvas_t *)p;
        IM_INFOMSG((IM_STR("canvas->pixfmt=0x%x"), canvas->pixfmt));
        IM_INFOMSG((IM_STR("canvas->rotation=%d"), canvas->rotation));
        IM_INFOMSG((IM_STR("canvas->width=%d"), canvas->width));
        IM_INFOMSG((IM_STR("canvas->height=%d"), canvas->height));
        IM_INFOMSG((IM_STR("canvas->xOffset=%d"), canvas->xOffset));
        IM_INFOMSG((IM_STR("canvas->yOffset=%d"), canvas->yOffset));
        IM_ASSERT((canvas->pixfmt == IM_PIC_FMT_16BITS_RGB_565) || (canvas->pixfmt == IM_PIC_FMT_32BITS_0RGB_8888) );

        if((canvas->pixfmt == vmode->config.extCanvas.pixfmt) && (canvas->rotation == vmode->config.extCanvas.rotation) &&
                (canvas->width == vmode->config.extCanvas.width) && (canvas->height == vmode->config.extCanvas.height) &&
                (canvas->xOffset == vmode->config.extCanvas.xOffset) && (canvas->yOffset == vmode->config.extCanvas.yOffset)){
            IM_WARNMSG((IM_STR("the set canvas is same as current setting")));
            vmode->flag |= VMODE_FLAG_STABLE;
            status = IDS_VMODE_SETCANVAS_OK;
            vmode_trigger_event(IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS	, -1, -1 , &status, sizeof(IM_INT32));
            return IM_RET_OK;
        }
        if((canvas->pixfmt != vmode->config.extCanvas.pixfmt) || (canvas->width != vmode->config.extCanvas.width) ||
                (canvas->height != vmode->config.extCanvas.height)){
            size = canvas->width * canvas->height * (IM_PIC_BITS(canvas->pixfmt) >> 3);
            realloc = IM_TRUE;
        }

        idspwl_memset((void *)&bch, 0, sizeof(bank_check));
        bch.size = size;
        idsdrv_bank_check_2(&bch);
        if (realloc == IM_TRUE){

            if (vmode->waitSwapFbBuffer == IM_FALSE){
                memset((void*)&src, 0, sizeof(g2d_image_t));
                memset((void*)&dst, 0, sizeof(g2d_image_t));
                src.format = vmode->config.extCanvas.pixfmt;
                src.width = vmode->config.extCanvas.width;
                src.height = vmode->config.extCanvas.height;
                src.clipRect.left = 0;
                src.clipRect.top = 0;
                src.clipRect.width = src.width;
                src.clipRect.height = src.height;
                dst.format = canvas->pixfmt;
                dst.width = canvas->width;
                dst.height = canvas->height;
                dst.clipRect.left = 0;
                dst.clipRect.top = 0;
                dst.clipRect.width = dst.width;
                dst.clipRect.height = dst.height;
                if (vmode->buffAIsBack == IM_TRUE){
                    if (unlikely(bch.bankNeeds)){
                        // use reserve memory address, fixed, just change the buffer size, 
                        vmode->buffA.size = size;
                    }
                    else {
                        idspwl_free_memory_block(&vmode->buffA);
                        vmode->buffA.vir_addr = IM_NULL;
                        IM_JIF(idspwl_alloc_memory_block(&vmode->buffA, size, IM_TRUE));
                    }
                    memcpy((void*)&(src.buffer[0]), (void*)&vmode->buffB, sizeof(IM_Buffer));
                    memcpy((void*)&(dst.buffer[0]), (void*)&vmode->buffA, sizeof(IM_Buffer));
                    if (g2dkapi_do_scale(g2dinst, &src, &dst, vmode->eeval) != IM_RET_OK){
                        IM_ERRMSG((IM_STR("##### g2dkapi_do_scale failed, check it !!!!!")));
                        goto Fail;
                    }
                    vmode->buffAIsBack = IM_FALSE;
                }
                else {
                    if (unlikely(bch.bankNeeds)){
                        vmode->buffB.size = size;
                    }
                    else {
                        idspwl_free_memory_block(&vmode->buffB);
                        vmode->buffB.vir_addr = IM_NULL;
                        IM_JIF(idspwl_alloc_memory_block(&vmode->buffB, size, IM_TRUE));
                    }
                    memcpy((void*)&(src.buffer[0]), (void*)&vmode->buffA, sizeof(IM_Buffer));
                    memcpy((void*)&(dst.buffer[0]), (void*)&vmode->buffB, sizeof(IM_Buffer));
                    if (g2dkapi_do_scale(g2dinst, &src, &dst, vmode->eeval) != IM_RET_OK){
                        IM_ERRMSG((IM_STR("##### g2dkapi_do_scale failed, check it !!!!!")));
                        goto Fail;
                    }
                    vmode->buffAIsBack = IM_TRUE;
                }
            }
            else{
                if (unlikely(bch.bankNeeds)){
                    vmode->buffA.size = size;
                    vmode->buffB.size = size;
                }
                else{
                    idspwl_free_memory_block(&vmode->buffA);
                    vmode->buffA.vir_addr = IM_NULL;
                    IM_JIF(idspwl_alloc_memory_block(&vmode->buffA, size, IM_TRUE));

                    idspwl_free_memory_block(&vmode->buffB);
                    vmode->buffB.vir_addr = IM_NULL;
                    IM_JIF(idspwl_alloc_memory_block(&vmode->buffB, size, IM_TRUE));
                }
            }
        }
        if((canvas->pixfmt == vmode->config.locCanvas.pixfmt) && 
                (canvas->rotation == vmode->config.locCanvas.rotation) &&
                (canvas->width == vmode->config.locCanvas.width) && 
                (canvas->height == vmode->config.locCanvas.height)){
            vmode->shareBuffer = IM_TRUE;
        }else{
            vmode->shareBuffer = IM_FALSE;
        }

        memset((void*)&pano, 0, sizeof(ids_wlayer_panorama_t));
        pano.imgWidth = canvas->width;
        pano.imgHeight = canvas->height;
        pano.pos.left = canvas->xOffset;
        pano.pos.top = canvas->yOffset;
        pano.pos.width = canvas->width;
        pano.pos.height = canvas->height;
        pano.buffer.vir_addr = (vmode->buffAIsBack==IM_TRUE) ? vmode->buffB.vir_addr : vmode->buffA.vir_addr;
        pano.buffer.phy_addr = (vmode->buffAIsBack==IM_TRUE) ? vmode->buffB.phy_addr : vmode->buffA.phy_addr;
        pano.buffer.size = canvas->width * canvas->height * (IM_PIC_BITS(canvas->pixfmt) >> 3);

        switch (canvas->pixfmt){
            case IM_PIC_FMT_16BITS_RGB_565:
                pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
                break;
            case IM_PIC_FMT_32BITS_0RGB_8888:
                pixfmt = IDSLIB_PIXFMT_32BPP_RGB0888;
                break;
            case IM_PIC_FMT_32BITS_ARGB_8888:
                pixfmt = IDSLIB_PIXFMT_32BPP_ARGBA888;
                break;
            default :
                pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
                break;
        }
        // remove device canvas info, idslib_wlayer_set_position() need.
        idspwl_memset((void *)&dd->canvas, 0, sizeof(ids_wlayer_canvas_t));
        idshw_async_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);

#ifdef USE_WINMIX
        //  1<<1(win1) & 1<<2(win2) & 1<<3(win3);
        if (idslib_fblayer_winmix_init((fblayer_handle_t)vmode->extfblayer, &pano, 3, pixfmt) != IM_RET_OK){
            IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_init(extfblayer) failed - \n")));
            idshw_flush_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);
            return IM_RET_FAILED;
        }
        if (idslib_fblayer_winmix_set_panorama((fblayer_handle_t)vmode->extfblayer, canvas) != IM_RET_OK){
            IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_set_panorama(extfblayer) failed - \n")));
            idshw_flush_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);
            return IM_RET_FAILED;
        }
#endif
        if(idslib_wlayer_set_panorama((wlayer_handle_t)vmode->extfblayer->wlayer, pixfmt, &pano) != IM_RET_OK){
            IM_ERRMSG((IM_STR(" idslib_wlayer_set_panorama failed ")));
            goto Fail;
        }

        idshw_flush_config(vmode->extfblayer->wlayer->idsx, vmode->extfblayer->wlayer->wx);

        idspwl_msleep(18); // Wait for one frame time. For vysnc call has some bugs, so just sleep a while here.
        /*if (vmode->extfblayer->wlayer->opened){
            idspwl_vsync(vmode->extfblayer->wlayer->idsx); // intr wait may have some problems, log shows no time wasted.
        }*/

        // the last, we can release the previous show buffer.
        if((realloc == IM_TRUE) && (vmode->waitSwapFbBuffer == IM_FALSE))
        {
            if(vmode->buffAIsBack == IM_TRUE){
                if (unlikely(bch.bankNeeds)){
                    vmode->buffA.size = size;
                }
                else {
                    idspwl_free_memory_block(&vmode->buffA);
                    vmode->buffA.vir_addr = IM_NULL;
                    IM_JIF(idspwl_alloc_memory_block(&vmode->buffA, size, IM_TRUE));
                }
            }else{
                if (unlikely(bch.bankNeeds)){
                    vmode->buffB.size = size;
                }
                else {
                    idspwl_free_memory_block(&vmode->buffB);
                    vmode->buffB.vir_addr = IM_NULL;
                    IM_JIF(idspwl_alloc_memory_block(&vmode->buffB, size, IM_TRUE));
                }
            }
        }
        vmode->flag |= VMODE_FLAG_STABLE ;

        status = IDS_VMODE_SETCANVAS_OK;
        vmode_trigger_event(IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS	, -1, -1 , &status, sizeof(IM_INT32));
        idspwl_memcpy((void *)&vmode->config.extCanvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
        idspwl_memcpy((void *)&dd->canvas, (void *)canvas, sizeof(ids_wlayer_canvas_t));
    }

    return IM_RET_OK;

Fail:

#ifdef USE_WINMIX
    if (vmode->extfblayer->wmix.wxs){
        idslib_fblayer_winmix_deinit((fblayer_handle_t)vmode->extfblayer);
    }
#endif
    if(evt == IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE){
        status = IDS_VMODE_FBL_SWPBF_FAILED ;
        vmode_trigger_event(IDS_VMODE_NOTIFY_FBL_SWAPBF, vmode->extfblayer->wlayer->idsx, 
                vmode->extfblayer->wlayer->wx, &status, sizeof(IM_INT32));
    }
    else if (evt == IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS){
        status = IDS_VMODE_SETCANVAS_FAILED;
        vmode_trigger_event(IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS, -1, -1 , &status, sizeof(IM_INT32));
    }
    else if (evt == IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER){
        status = IDS_VMODE_FBL_SWPBF_FAILED	;
        vmode_trigger_event(IDS_VMODE_NOTIFY_FBL_SWAPBF , -1, -1 , &status, sizeof(IM_INT32));
    }

    // Warning : Once failed, trigger EXCEPTION immediately or just skip the error ?
    //status = IDS_VMODE_EXCEPTION_UNKNOWN;
    //vmode_trigger_event(IDS_VMODE_EVENT_EXCEPTION, -1, -1, (void *)&status, sizeof(IM_INT32));

    return IM_RET_FAILED;
}

static IM_RET ext_fblayer_create(vmode_t *vmode, ids_vmode_config_t *vmcfg)
{
    IM_UINT32 size, pixfmt;
    fblayer_t  *extfblayer;
    ids_wlayer_panorama_t pano;
    bank_check bch;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    IM_INFOMSG((IM_STR(" vmcfg->mode=%d "), vmcfg->mode));
    IM_INFOMSG((IM_STR(" vmcfg->locDev.type=%d "),vmcfg->locDev.type));
    IM_INFOMSG((IM_STR(" vmcfg->locDev.id  =%d "),vmcfg->locDev.id));
    IM_INFOMSG((IM_STR(" vmcfg->locDev.width=%d "),vmcfg->locDev.width));
    IM_INFOMSG((IM_STR(" vmcfg->locDev.height=%d "),vmcfg->locDev.height));
    IM_INFOMSG((IM_STR(" vmcfg->locDev.fpsx1000=%d "),vmcfg->locDev.fpsx1000));
    IM_INFOMSG((IM_STR(" vmcfg->locCanvas.pixfmt=0x%x "),vmcfg->locCanvas.pixfmt));
    IM_INFOMSG((IM_STR(" vmcfg->locCanvas.width=%d "),vmcfg->locCanvas.width));
    IM_INFOMSG((IM_STR(" vmcfg->locCanvas.height=%d "),vmcfg->locCanvas.height));
    IM_INFOMSG((IM_STR(" vmcfg->locCanvas.xOffset=%d "),vmcfg->locCanvas.xOffset));
    IM_INFOMSG((IM_STR(" vmcfg->locCanvas.yOffset=%d "),vmcfg->locCanvas.yOffset));
    IM_INFOMSG((IM_STR(" vmcfg->extDev.type=%d "),vmcfg->extDev.type));
    IM_INFOMSG((IM_STR(" vmcfg->extDev.id  =%d "),vmcfg->extDev.id));
    IM_INFOMSG((IM_STR(" vmcfg->extDev.width=%d "),vmcfg->extDev.width));
    IM_INFOMSG((IM_STR(" vmcfg->extDev.height=%d "),vmcfg->extDev.height));
    IM_INFOMSG((IM_STR(" vmcfg->extDev.fpsx1000=%d "),vmcfg->extDev.fpsx1000));
    IM_INFOMSG((IM_STR(" vmcfg->extCanvas.pixfmt=0x%x "),vmcfg->extCanvas.pixfmt));
    IM_INFOMSG((IM_STR(" vmcfg->extCanvas.width=%d "),vmcfg->extCanvas.width));
    IM_INFOMSG((IM_STR(" vmcfg->extCanvas.height=%d "),vmcfg->extCanvas.height));
    IM_INFOMSG((IM_STR(" vmcfg->extCanvas.xOffset=%d "),vmcfg->extCanvas.xOffset));
    IM_INFOMSG((IM_STR(" vmcfg->extCanvas.yOffset=%d "),vmcfg->extCanvas.yOffset));


    extfblayer = idspwl_malloc(sizeof(fblayer_t));
    if (extfblayer == IM_NULL){
        IM_ERRMSG((IM_STR(" extfblayer malloc falied")));
        return IM_RET_FAILED;
    }
    idspwl_memset((void*)extfblayer, 0 , sizeof(fblayer_t));

    if(wlayer_init_internal(&gIdsInst->idsMgr[gIdsInst->extidsx].wlayer[0], gIdsInst->extidsx, 0/*fixed to 0*/, IM_TRUE) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_init_internal() failed")));
        return IM_RET_FAILED;
    }
    extfblayer->wlayer = &gIdsInst->idsMgr[gIdsInst->extidsx].wlayer[0];

    if ((vmcfg->extCanvas.pixfmt == vmcfg->locCanvas.pixfmt) &&
            (vmcfg->extCanvas.rotation == vmcfg->locCanvas.rotation) &&
            (vmcfg->extCanvas.width == vmcfg->locCanvas.width) && 
            (vmcfg->extCanvas.height == vmcfg->locCanvas.height)){
        vmode->shareBuffer = IM_TRUE;
    }else {
        vmode->shareBuffer = IM_FALSE;
    }

    if (vmode->shareBuffer == IM_FALSE){
        size = vmcfg->extCanvas.width * vmcfg->extCanvas.height * (IM_PIC_BITS(vmcfg->extCanvas.pixfmt) >> 3);
        idspwl_memset((void *)&bch, 0, sizeof (bank_check));
        bch.size = size;
        idsdrv_bank_check_2(&bch);
        if (unlikely(bch.bankNeeds)){
            IM_INFOMSG((IM_STR(" use reserved bank memory ")));
            idspwl_memcpy((void*)&vmode->buffA, (void*)&bch.bufA, sizeof(IM_Buffer));
            idspwl_memcpy((void*)&vmode->buffB, (void*)&bch.bufB, sizeof(IM_Buffer));
        }
        else {
            IM_INFOMSG((IM_STR(" use normally allocated memory ")));
            if (idspwl_alloc_memory_block(&vmode->buffA, size, IM_TRUE) != IM_RET_OK){
                IM_ERRMSG((IM_STR("idspwl_alloc_memory_block(size=%d) failed"),size));
                goto Fail;
            }
            if (idspwl_alloc_memory_block(&vmode->buffB, size, IM_TRUE) != IM_RET_OK){
                IM_ERRMSG((IM_STR("idspwl_alloc_memory_block(size=%d) failed"),size));
                goto Fail;
            }
            vmode->buffAIsBack = IM_TRUE;
        }
    }

    memset((void*)&pano, 0, sizeof(ids_wlayer_panorama_t));
    pano.imgWidth = vmcfg->extCanvas.width;
    pano.imgHeight = vmcfg->extCanvas.height;
    pano.pos.left = 0;   // idslib_wlayer_set_position would take dd->canvas.x(y)Offset into consideration
    pano.pos.top = 0;
    pano.pos.width = vmcfg->extCanvas.width;
    pano.pos.height = vmcfg->extCanvas.height;

    switch (vmcfg->extCanvas.pixfmt){
        case IM_PIC_FMT_16BITS_RGB_565:
            pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
            break;
        case IM_PIC_FMT_32BITS_0RGB_8888:
            pixfmt = IDSLIB_PIXFMT_32BPP_RGB0888;
            break;
        case IM_PIC_FMT_32BITS_ARGB_8888:
            pixfmt = IDSLIB_PIXFMT_32BPP_ARGBA888;
            break;
        default :
            pixfmt = IDSLIB_PIXFMT_16BPP_RGB565;
            break;
    }

    if(idslib_wlayer_set_panorama((wlayer_handle_t)extfblayer->wlayer, pixfmt, &pano) != IM_RET_OK){
        IM_ERRMSG((IM_STR("idslib_wlayer_set_panorama() failed")));
        goto Fail;
    }

#ifdef USE_WINMIX
    //  1<<1(win1) & 1<<2(win2) & 1<<3(win3);
    if (idslib_fblayer_winmix_init((fblayer_handle_t)extfblayer, &pano, 3, pixfmt) != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_init(extfblayer) failed - \n")));
        return IM_RET_FAILED;
    }
    if (idslib_fblayer_winmix_set_panorama((fblayer_handle_t)extfblayer, &vmcfg->extCanvas) != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idslib_fblayer_winmix_set_panorama(extfblayer) failed - \n")));
        return IM_RET_FAILED;
    }
#endif

    vmode->rotate = ((vmcfg->extCanvas.rotation + 360) - vmcfg->extCanvas.rotation) % 360;
    vmode->waitSwapFbBuffer = IM_TRUE;
    vmode->extfblayer = extfblayer;

    if (g2dinst == IM_NULL){
        g2dinst = g2dkapi_init();
        if (g2dinst == IM_NULL){
            IM_ERRMSG((IM_STR(" g2dkapi_init failed -!!!")));
            goto Fail;
        }
    }

    return IM_RET_OK;
Fail:
#ifdef USE_WINMIX
    if (vmode->extfblayer && vmode->extfblayer->wmix.wxs){
        idslib_fblayer_winmix_deinit((fblayer_handle_t)vmode->extfblayer);
    }
#endif
    if (extfblayer != IM_NULL){
        ext_fblayer_destroy(vmode);
        idspwl_free(extfblayer);
    }

    if (vmode->buffA.vir_addr != IM_NULL){
        idspwl_free_memory_block(&vmode->buffA);
        vmode->buffA.vir_addr = IM_NULL;
    }
    if (vmode->buffB.vir_addr != IM_NULL){
        idspwl_free_memory_block(&vmode->buffB);
        vmode->buffB.vir_addr = IM_NULL;
    }
    return IM_RET_FAILED;
}

static IM_RET ext_fblayer_destroy(vmode_t *vmode)
{
    fblayer_t  *extfblayer;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    extfblayer = vmode->extfblayer;

#ifdef USE_WINMIX
    idslib_fblayer_winmix_deinit((fblayer_handle_t)extfblayer);
#endif

    wlayer_close_internal((wlayer_handle_t)extfblayer->wlayer);

    if (g2dinst != IM_NULL){
        g2dkapi_deinit(g2dinst);
        g2dinst = IM_NULL;
    }

    if(wlayer_deinit_internal((wlayer_handle_t)extfblayer->wlayer) != IM_RET_OK){
        IM_ERRMSG((IM_STR("wlayer_deinit_internal() failed")));
        return IM_RET_FAILED;	
    }
    if (vmode->buffA.vir_addr != IM_NULL){
        idspwl_free_memory_block(&vmode->buffA);
        vmode->buffA.vir_addr = IM_NULL;
    }
    if (vmode->buffB.vir_addr != IM_NULL){
        idspwl_free_memory_block(&vmode->buffB);
        vmode->buffB.vir_addr = IM_NULL;
    }
    idspwl_free(extfblayer);
    vmode->extfblayer = IM_NULL;
    return IM_RET_OK;
}

static IM_UINT32 idslib_get_bpp_from_pixel(IM_UINT32 pixfmt)
{
    IM_UINT32 bpp;

    switch (pixfmt){
        case IDSLIB_PIXFMT_16BPP_RGB565:
            bpp = 16;
            break;
        case IDSLIB_PIXFMT_32BPP_RGB0888:
        case IDSLIB_PIXFMT_32BPP_ARGBA888:
            bpp = 32;
            break;
        case IDSLIB_PIXFMT_12BPP_YUV420SP:
            bpp = 12;
            break;
        default :
            bpp = 0;
            break;
    }
    IM_INFOMSG((IM_STR("%s() pixfmt=%d, bpp=%d"), IM_STR(_IM_FUNC_),pixfmt,bpp));
    return bpp;
}

