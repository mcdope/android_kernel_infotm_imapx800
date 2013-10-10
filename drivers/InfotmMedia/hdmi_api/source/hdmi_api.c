/***************************************************************************** 
 ** 
 ** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** Use of Infotm's code is governed by terms and conditions 
 ** stated in the accompanying licensing statement. 
 ** 
 ** Description: Implementation file of Hdmi API 
 **
 ** Author:
 **     Sam Ye<weize_ye@infotm.com>
 **      
 ** Revision History: 
 ** ----------------- 
 ** 1.0.1	 Sam@2012/3/20 :  first commit
 **
 *****************************************************************************/
#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <ids_pwl.h>

#include <hdmi_api.h>
#include <hdmi_lib.h>
#include <lcd_lib.h>

#define DBGINFO 	0
#define DBGWARN 	1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"HDMIAPI_I:"
#define WARNHEAD	"HDMIAPI_W:"
#define ERRHEAD		"HDMIAPI_E:"
#define TIPHEAD		"HDMIAPI_T:"

static fcbk_dispdev_listener_t gHdmiListener;

typedef struct{
    IM_INT32 idsx;
    IM_BOOL  StateOn;
    void *HdmiInst;
    ids_display_device_t config;
}hdmi_handle;

static hdmi_handle gHdmiHandle = {0};
static void hdmi_hpd_cb(void *param);

IM_RET hdmi_init(dispdev_handle_t *handle , IM_INT32 idsx)
{
    IM_RET ret;
    IM_UINT32 resclk;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    *handle = (dispdev_handle_t)&gHdmiHandle;
    if (gHdmiHandle.HdmiInst != IM_NULL){
        IM_INFOMSG((IM_STR(" multi init avoided")));
        return IM_RET_OK;
    }

    memset((void*)&gHdmiHandle, 0 , sizeof(hdmi_handle));
    gHdmiHandle.idsx= idsx;
    gHdmiHandle.StateOn = IM_FALSE;
    ret = hdmilib_Handle(&gHdmiHandle.HdmiInst);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_Handle failed - ")));
        return IM_RET_FAILED;
    }

    idspwl_module_enable(MODULE_HDMI);
    idspwl_module_reset(MODULE_HDMI);
    ret = idspwl_module_request_frequency(MODULE_HDMI, 27000000, &resclk);
    if (ret != IM_RET_OK){
        IM_ERRMSG((IM_STR(" idspwl_module_request_frequency(MODULE_HDMI) failed - ")));
        return IM_RET_FAILED;
    }
    IM_TIPMSG((IM_STR(" hdmi module actual clock = %d - "),resclk));

    return IM_RET_OK;
}

IM_RET hdmi_deinit(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = hdmilib_deinit(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    ret = lcdlib_close(HdmiHandle->idsx);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }

    lcdlib_deinit(HdmiHandle->idsx);
    return IM_RET_OK;
}

IM_RET hdmi_set_video_basic_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix)
{
    IM_UINT32 index, num, mCode;
    IM_RET ret;
    lcdc_config_t lcdc;
    hdmi_video_t hdvid_t;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    IM_INFOMSG((IM_STR("hdmi_set_video_basic_config type=%d,id=%d ,width=%d,height=%d - "),config->type,config->id,config->width,config->height));

    hdmilib_get_config_num(HdmiHandle->HdmiInst,&num);
    for(index =0; index < num ; index++){
        hdmilib_index_to_mcode(HdmiHandle->HdmiInst, index, &mCode);
        if(mCode == config->id) 
            break;
    }
    IM_INFOMSG((IM_STR("sam :%s index = %d , num = %d  - config->id=%d "),__func__,index,num,config->id));
    if(index == num)
        return IM_RET_FAILED;
    if(hdmilib_get_config(HdmiHandle->HdmiInst, index, &hdvid_t) != IM_RET_OK){
        return IM_RET_FAILED;
    }
    if(hdmilib_set_basic_video_config(HdmiHandle->HdmiInst, index) != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_set_basic_video_config -failed  ")));
        return IM_RET_FAILED;
    }
    // If not to close lcd first , IDS would be dead during the second time's operation.
    ret = lcdlib_close(HdmiHandle->idsx);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    lcdc.width = hdvid_t.mHActive;
    lcdc.height = hdvid_t.mVActive;
    lcdc.VSPW = hdvid_t.mVSyncPulseWidth;
    lcdc.VBPD = hdvid_t.mVBlanking - hdvid_t.mVSyncOffset - hdvid_t.mVSyncPulseWidth;
    lcdc.VFPD = hdvid_t.mVSyncOffset;
    lcdc.HSPW = hdvid_t.mHSyncPulseWidth;
    lcdc.HBPD = hdvid_t.mHBlanking - hdvid_t.mHSyncOffset - hdvid_t.mHSyncPulseWidth;
    lcdc.HFPD = hdvid_t.mHSyncOffset;
    lcdc.signalPwrenEnable = 1;
    lcdc.signalPwrenInverse = 0;
    lcdc.signalVclkInverse = 1;
    lcdc.signalHsyncInverse = (hdvid_t.mHSyncPolarity) ? 0 : 1;
    lcdc.signalVsyncInverse = (hdvid_t.mVSyncPolarity) ? 0 : 1;
    lcdc.signalDataInverse = 0;
    lcdc.signalVdenInverse = 0; 
    lcdc.SignalDataMapping = 0x06;
    lcdc.fpsx1000 = hdvid_t.mFpsx1000;
    ret = lcdlib_init(HdmiHandle->idsx,&lcdc);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    ret = lcdlib_open(HdmiHandle->idsx);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    ret = hdmilib_init(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_init failed  - ")));
        return IM_RET_FAILED;
    }

    memcpy((void*)&gHdmiHandle.config, config, sizeof(ids_display_device_t)); 

    return IM_RET_OK;
}

IM_RET hdmi_get_video_basic_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix)
{
    IM_UINT32 index, num, mCode;
    hdmi_video_t hdvid_t;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;

    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    hdmilib_get_config_num(HdmiHandle->HdmiInst,&num);
    for(index =0; index < num ; index++){
        hdmilib_index_to_mcode(HdmiHandle->HdmiInst, index, &mCode);
        if(mCode == config->id) 
            break;
    }
    if(index == num)
        return IM_RET_FAILED;
    if(hdmilib_get_config(HdmiHandle->HdmiInst, index, &hdvid_t) != IM_RET_OK){
        return IM_RET_FAILED;
    }
    config->width = hdvid_t.mHActive;
    config->height = hdvid_t.mVActive;
    config->fpsx1000 = hdvid_t.mFpsx1000;

    if (info != IM_NULL) 
    {
        info->width = hdvid_t.mHActive;
        info->height = hdvid_t.mVActive;
        info->dataport = DISPDEV_DATAPORT_RGB888;
        info->hspw = hdvid_t.mHSyncPulseWidth;
        info->hbpd = hdvid_t.mHBlanking - hdvid_t.mHSyncOffset - hdvid_t.mHSyncPulseWidth;
        info->hfpd = hdvid_t.mHSyncOffset;
        info->vspw = hdvid_t.mVSyncPulseWidth;;
        info->vbpd = hdvid_t.mVBlanking - hdvid_t.mVSyncOffset - hdvid_t.mVSyncPulseWidth;
        info->vfpd = hdvid_t.mVSyncOffset;
        info->fpsx1000 = hdvid_t.mFpsx1000;
    }

    return IM_RET_OK;
}

IM_RET hdmi_set_video_encodings(dispdev_handle_t handle, IM_VI_ENCODING in, IM_VI_ENCODING out)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = hdmilib_set_video_encodings(HdmiHandle->HdmiInst, in, out);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_set_video_encodings- failed  - ")));
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

/*
 *  audio setting is done by IIS or SPDIF driver, they haven't got the handle.
 */
IM_RET hdmi_set_audio_basic_config(hdmi_audio_t *audio_t)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = &gHdmiHandle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
    audio_t->sampleSize = audio_t->sampleSize << 3;
    IM_INFOMSG((IM_STR(" interface=%d - codingType=%d - ch=%d, sampleSize=%d -, sf=%d - "),audio_t->aInterfaceType,
                audio_t->aCodingType,audio_t->channelAllocation,audio_t->sampleSize,audio_t->samplingFreq));

    if (HdmiHandle->HdmiInst == IM_NULL){
        IM_WARNMSG((IM_STR(" hdmi not inited yet ")));
        return IM_RET_OK;
    }
    ret = hdmilib_set_basic_audio_config(HdmiHandle->HdmiInst, audio_t);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_set_basic_audio_config- failed  - ")));
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

IM_RET hdmi_set_hdcp_basic_config(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if (HdmiHandle->HdmiInst == IM_NULL){
        IM_WARNMSG((IM_STR(" hdmi not inited yet ")));
        return IM_RET_OK;
    }
    ret = hdmilib_set_basic_hdcp_config(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_set_basic_hdcp_config - failed  - ")));
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

IM_RET hdmi_set_product_basic_config(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if (HdmiHandle->HdmiInst == IM_NULL){
        IM_WARNMSG((IM_STR(" hdmi not inited yet ")));
        return IM_RET_OK;
    }
    ret = hdmilib_set_basic_product_config(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_set_basic_product_config failed  - ")));
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

IM_RET hdmi_open(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = hdmilib_open(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("hdmilib_open failed  - ")));
        //return IM_RET_FAILED;
        return IM_RET_OK; // sam : for test
    }
    HdmiHandle->StateOn = IM_TRUE;

    return IM_RET_OK;
}

IM_RET hdmi_close(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = hdmilib_close(HdmiHandle->HdmiInst);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    idspwl_msleep(20); // why here must sleep for a while, or BUS could be dead !!! Technical trap ? 
    ret = lcdlib_close(HdmiHandle->idsx);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    HdmiHandle->StateOn = IM_FALSE;
    //idspwl_module_disable(MODULE_HDMI);

    return IM_RET_OK;
}

IM_RET hdmi_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if((void*)HdmiHandle != (void*)&gHdmiHandle)
        return IM_RET_FAILED;

    ret = hdmilib_hpd_detect_and_read_edid(hdmi_hpd_cb);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    gHdmiListener = listener;

    IM_INFOMSG((IM_STR("%s() end "), IM_STR(_IM_FUNC_)));
    return IM_RET_OK;
}

IM_RET hdmi_suspend(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    hdmilib_suspend(HdmiHandle->HdmiInst);
    if (HdmiHandle->StateOn == IM_TRUE){
        idspwl_msleep(10);
        ret = lcdlib_close(HdmiHandle->idsx);
        if(ret != IM_RET_OK){
            return IM_RET_FAILED;
        }
    }
    idspwl_module_disable(MODULE_HDMI);

    return IM_RET_OK;
}

IM_RET hdmi_resume(dispdev_handle_t handle)
{
    IM_RET ret;
    hdmi_handle *HdmiHandle = (hdmi_handle*) handle;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    idspwl_module_enable(MODULE_HDMI);
    idspwl_module_reset(MODULE_HDMI);

    // For time save, disable hpd debounce func during hdmi resume process
    ret = hdmilib_resume(HdmiHandle->HdmiInst, hdmi_hpd_cb);
    if(ret != IM_RET_OK){
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

void hdmi_hpd_cb(void *param)
{
    IM_CHAR hpd = *((IM_CHAR*)(param));
    IM_UINT32 index;
    hdmi_video_t hdvid_t;
    IM_RET ret;
    dispdev_evt_config_update_t dispdev_evt;
    ids_display_device_t *devs;

    IM_INFOMSG((IM_STR("%s(), hpd=%d"), IM_STR(_IM_FUNC_),hpd));

    if(hpd == 1){
        hdmilib_get_config_num((dispdev_handle_t) &gHdmiHandle, &dispdev_evt.num);
        devs = (ids_display_device_t*)idspwl_malloc( dispdev_evt.num * sizeof(ids_display_device_t));
        IM_INFOMSG((IM_STR("hdmi_hpd_cb: dispdev_evt.num = %d - "),dispdev_evt.num));
        for(index = 0; index < dispdev_evt.num; index ++){
            ret = hdmilib_get_config((dispdev_handle_t) &gHdmiHandle, index, &hdvid_t); 
            if(ret != IM_RET_OK){
                break;
            }
            devs[index].type = IDS_DISPDEV_TYPE_HDMI;
            devs[index].id  = hdvid_t.mCode; 
            devs[index].width = hdvid_t.mHActive;
            devs[index].height = hdvid_t.mVActive;
            devs[index].fpsx1000  = hdvid_t.mFpsx1000;
        }
        dispdev_evt.devs = devs;
        gHdmiListener((dispdev_handle_t) &gHdmiHandle, DISPDEV_EVT_CONFIG_UPDATE, &dispdev_evt);
        idspwl_free(devs);
    }
    else{
        gHdmiListener((dispdev_handle_t) &gHdmiHandle, DISPDEV_EVT_BREAK, IM_NULL);
    }
    return ;
}



