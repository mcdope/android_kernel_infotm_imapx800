/***************************************************************************** 
 ** 
 ** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** Use of Infotm's code is governed by terms and conditions 
 ** stated in the accompanying licensing statement. 
 ** 
 ** Description: Implementation file of hdmi library 
 **
 ** Author:
 **     Sam Ye<weize_ye@infotm.com>
 **      
 ** Revision History: 
 ** ----------------- 
 ** 1.0.1	 Sam@2012/3/20 :  first commit
 **  v2.0.1	 Sam@2013/02/25:  support hdmi as the static device
 **
 *****************************************************************************/

#include <InfotmMedia.h>
#include <ids_pwl.h>

#include <api.h>
#include <hdmi_system.h>
#include <hdmi_log.h>
#include <error.h>
#include <edid.h>
#include <board.h>
#include <phy.h>
#include <access.h>

#include <IM_idsapi.h>
#include <ids_lib.h>
#include <hdmi_api.h>
#include <hdmi_lib.h>
#include <hdmi.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <mach/items.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"HDMILIB_I:"
#define WARNHEAD	"HDMILIB_W:"
#define ERRHEAD		"HDMILIB_E:"
#define TIPHEAD		"HDMILIB_T:"

IM_INT32 gHdmiStateOn = IM_FALSE;
IM_INT32 gHdmiOnBeforeSuspend = IM_FALSE;
// open & close local sound 
int imapx800_switch_spk_hdmi(int dir);

#define MAX_VID_INDEX 3
IM_UINT32 gVidNum_t = 0;
hdmi_video_t gVid_t[MAX_VID_INDEX];
static IM_UINT32 gVidNum_A = 0;
hdmi_video_t gVid_A[MAX_VID_INDEX];
static IM_BOOL gEdidEqual = IM_TRUE;

enum {HDMI_LOST, HDMI_CONNECTED};
IM_INT32 gHdmiLastState = HDMI_LOST;
static IM_INT32 gHdmiResumeState = 0;
static IM_INT32 gDeviceInitFlag;
static IM_INT32 gDeviceOpenFlag;
IM_ULONG hpdConnected = 0;
IM_INT32 edidDone = 0;
static handler_t gHpdListener=NULL;

static struct workqueue_struct *wq;
static struct delayed_work dwq;
static void delay_func(struct work_struct *p);
struct semaphore gSema; // used for hdmi_open after resume
struct semaphore gSemaEdid; // used for edid read never return bug
extern int gHdmiPcbTestOn;

static void hpdCallback(void *param);
static void edidCallback(void *param);
static IM_UINT32 mCodetoFpsx1000(IM_CHAR code);
static IM_RET apiSet(void *inst);
static IM_RET audioSet(void *inst);
IM_INT32 edid_video_cb(dtd_t *dtd);
IM_INT32 compliance_Standby(struct_hdmi_instance *hdmi_inst);

struct_hdmi_instance gHdmiInst;

IM_RET hdmilib_Handle(void **inst)
{
    IM_INT32 i;
    dtd_t dtd;
    char str[ITEM_MAX_LEN];
    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));
    gHdmiInst.videoOn = 0;
    gHdmiInst.audioOn = 0;
    gHdmiInst.hdcpOn = 0;
    gHdmiInst.productOn = 0;
    gHdmiInst.hdmiStatic = IM_FALSE;

    *inst = &gHdmiInst;

    if (!access_Initialize((IM_UCHAR*)HDMI_TX_CTRL_ADDRESS))
    {
        return IM_RET_FAILED;
    }
    gDeviceInitFlag = 0;
    gDeviceOpenFlag = 0;

    // Need to cancel workqueue later
    wq = create_workqueue("hdmi");
    INIT_DELAYED_WORK(&dwq, delay_func);

    sema_init(&gSema, 0);
    if (gHdmiPcbTestOn == 0)
    {
        sema_init(&gSemaEdid, 0);

        item_string(str, "ids.loc.dev0.interface", 0);
        if ((item_integer("ids.locidsx", 0) == 1) && (strcasecmp(str, "hdmi") == 0) )
        {
            gHdmiInst.hdmiStatic = IM_TRUE;
            dtd_Fill(&dtd,16,60000); // 1080P
            edid_video_cb(&dtd);
            dtd_Fill(&dtd,4,60000);  // 720P
            edid_video_cb(&dtd);
            dtd_Fill(&dtd,1,60000);  // VGA
            edid_video_cb(&dtd);
            for(i=0; i < gVidNum_t; i++){
                memcpy((void*)&gVid_A[i], (void*)&gVid_t[i], sizeof(hdmi_video_t));
            }
            gVidNum_A = gVidNum_t;
            gVidNum_t = 0;

        }
    }

    gHdmiStateOn = IM_FALSE;

    return IM_RET_OK;
}

IM_RET hdmilib_init(void **inst)
{
    IM_INT32 force = 0;
    IM_INFOMSG((IM_STR("sam : %s - gDeviceInitFlag = %d - %d - \n"),__func__,gDeviceInitFlag, __LINE__));

    if(gDeviceInitFlag == 0){
        if (gHdmiInst.hdmiStatic == 1){
            force = 1;
        }
        if (!api_Initialize(HDMI_BASE_ADDR, 1, 2500, force))
        {
            gDeviceInitFlag = 0;
            *inst = &gHdmiInst;
            return IM_RET_FAILED;
        }
    }
    gDeviceInitFlag = 1;
    return IM_RET_OK;
}

IM_RET hdmilib_deinit(void *inst)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));

    gDeviceInitFlag = 0;
    gDeviceOpenFlag = 0;
    hpdConnected = 0;
    edidDone = 0;
    if(!api_DeInit()) return IM_RET_FAILED;
    compliance_Standby(hdmi_inst);

    return IM_RET_OK;
}

IM_RET hdmilib_open(void *inst)
{
    IM_INFOMSG((IM_STR("sam : %s - gDeviceOpenFlag = %d - %d - \n"),__func__,gDeviceOpenFlag, __LINE__));
    if(gDeviceOpenFlag)
    {
        if(!api_exitStandby()) return IM_RET_FAILED;
        return IM_RET_OK;
    }

    if(apiSet(inst) != IM_RET_OK){
        return IM_RET_FAILED;
    }

    gHdmiStateOn = IM_TRUE;

    gDeviceOpenFlag = 1;
    return IM_RET_OK;
}

IM_RET hdmilib_close(void *inst)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;

    if (gHdmiOnBeforeSuspend != IM_TRUE){
        imapx800_switch_spk_hdmi(1);
    }
    if(!api_Standby()) {
        return IM_RET_FAILED;
    }
    gHdmiStateOn = IM_FALSE;

    hdmi_inst->videoOn = 0;
    gDeviceOpenFlag = 1;
    return IM_RET_OK;
}

IM_RET hdmilib_get_config_num(void *inst,IM_UINT32 *num)
{
    IM_INFOMSG((IM_STR("sam : %s - - %d - gVidNum_A = %d "),__func__,__LINE__,gVidNum_A));
    *num = gVidNum_A;
    return IM_RET_OK;
}

IM_INT32 hdmilib_index_to_mcode(void *inst,IM_UINT32 index, IM_INT32 *mCode)
{
    if(index >= gVidNum_A){
        return IM_RET_FAILED;
    }
    *mCode = gVid_A[index].mCode;
    return IM_RET_OK;
}

IM_RET hdmilib_get_config(void *inst, IM_UINT32 index, hdmi_video_t *hdvid_t)
{
    if(index >= gVidNum_A){
        return IM_RET_FAILED;
    }
    idspwl_memcpy((void*)hdvid_t, (void *)&gVid_A[index], sizeof(hdmi_video_t));
    return IM_RET_OK;
}

IM_RET hdmilib_set_basic_video_config(void *inst, IM_UINT32 index)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    dtd_t dtd;
    // if gHdmiOnBeforeSuspend true, this func must be called during resume process. 
    if (gHdmiOnBeforeSuspend == IM_TRUE){
        if (gHdmiResumeState){
            down_timeout(&gSema, (long)300 * HZ / 1000); // timeout : 300ms
        }
        gHdmiOnBeforeSuspend = IM_FALSE;
        if (gEdidEqual == IM_FALSE){
            IM_ERRMSG((IM_STR(" hdmi information updated, choose again-- ")));
            gEdidEqual = IM_TRUE;
            return IM_RET_FAILED;
        }
    }
    if(index >= gVidNum_A){
        return IM_RET_FAILED;
    }
    if(hdmi_inst->videoOn == 0){
        hdmi_inst->videoOn = 1;
    }
    gDeviceOpenFlag = 0;
    videoParams_Reset(&hdmi_inst->HdmiVideo);

    videoParams_SetHdmi(&hdmi_inst->HdmiVideo, 1);
    videoParams_SetEncodingIn(&hdmi_inst->HdmiVideo, RGB);
    videoParams_SetEncodingOut(&hdmi_inst->HdmiVideo, RGB); 
    videoParams_SetColorResolution(&hdmi_inst->HdmiVideo, 8);

    // make useless of the mCode , is it OK ? 
    //dtd_Fill(&dtd, gVid_A[index].mCode , gVid_A[index].mFpsx1000);
    memset((void*)&dtd,0, sizeof(dtd_t));
    dtd.mCode = gVid_A[index].mCode;
    dtd.mPixelRepetitionInput = gVid_A[index].mPixelRepetitionInput;
    dtd.mPixelClock = gVid_A[index].mPixelClock;
    dtd.mInterlaced = gVid_A[index].mInterlaced ;
    dtd.mHActive = gVid_A[index].mHActive;
    dtd.mHBlanking = gVid_A[index].mHBlanking;
    dtd.mHBorder = gVid_A[index].mHBorder;
    dtd.mHImageSize = gVid_A[index].mHImageSize;
    dtd.mHSyncOffset = gVid_A[index].mHSyncOffset;
    dtd.mHSyncPulseWidth = gVid_A[index].mHSyncPulseWidth;
    dtd.mHSyncPolarity = gVid_A[index].mHSyncPolarity;
    dtd.mVActive = gVid_A[index].mVActive;
    dtd.mVBlanking = gVid_A[index].mVBlanking;
    dtd.mVBorder = gVid_A[index].mVBorder;
    dtd.mVImageSize = gVid_A[index].mVImageSize;
    dtd.mVSyncOffset = gVid_A[index].mVSyncOffset;
    dtd.mVSyncPulseWidth = gVid_A[index].mVSyncPulseWidth;
    dtd.mVSyncPolarity = gVid_A[index].mVSyncPolarity;

    videoParams_SetDtd(&(hdmi_inst->HdmiVideo), &dtd);

    videoParams_SetPixelRepetitionFactor(&hdmi_inst->HdmiVideo, 0);
    videoParams_SetPixelPackingDefaultPhase(&hdmi_inst->HdmiVideo, 0);
    videoParams_SetColorimetry(&hdmi_inst->HdmiVideo, ITU601);

    videoParams_SetHdmiVideoFormat(&hdmi_inst->HdmiVideo, 0);
    videoParams_Set3dStructure(&hdmi_inst->HdmiVideo, 0);
    videoParams_Set3dExtData(&hdmi_inst->HdmiVideo, 0);
    return IM_RET_OK;
}

IM_RET hdmilib_set_video_encodings(void *inst, IM_VI_ENCODING In, IM_VI_ENCODING Out)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    encoding_t in, out;
    gDeviceOpenFlag = 0;

    switch (In){
        case IM_RGB :
            in = RGB;
            break;
        case IM_YCC444:
            in = YCC444;
            break;
        case IM_YCC422:
            in = YCC422;
            break;
        default :
            return IM_RET_FAILED;
    }
    switch (Out){
        case IM_RGB :
            out = RGB;
            break;
        case IM_YCC444:
            out = YCC444;
            break;
        case IM_YCC422:
            out = YCC422;
            break;
        default :
            return IM_RET_FAILED;
    }
    videoParams_SetEncodingIn(&hdmi_inst->HdmiVideo, in);
    videoParams_SetEncodingOut(&hdmi_inst->HdmiVideo, out); 
    return IM_RET_OK;	
}

IM_RET hdmilib_set_basic_audio_config( void *inst, hdmi_audio_t *aud_t)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    interfaceType_t aIfType;
    codingType_t aCodeType;
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    gDeviceOpenFlag = 0;
    if(hdmi_inst->audioOn == 0){
        hdmi_inst->audioOn = 1;
    }
    switch(aud_t->aInterfaceType){
        case IM_I2S:
            aIfType = I2S;
            break;
        case IM_SPDIF:
            aIfType = SPDIF;
            break;
        default :
            IM_ERRMSG((IM_STR(" hdmi audio interface error -- ")));
            return IM_RET_FAILED;
    }
    switch (aud_t->aCodingType){
        case IM_PCM:
            aCodeType = PCM;
            break;
        case IM_AC3:
            aCodeType = AC3;
            break;
        case IM_MPEG1:
            aCodeType = MPEG1;
            break;
        case IM_MP3:
            aCodeType = MP3;
            break;
        case IM_MPEG2:
            aCodeType = MPEG2;
            break;
        case IM_AAC:
            aCodeType = AAC;
            break;
        case IM_DTS:
            aCodeType = DTS;
            break;
        case IM_ATRAC:
            aCodeType = ATRAC;
            break;
        case IM_ONE_BIT_AUDIO:
            aCodeType = ONE_BIT_AUDIO;
            break;
        case IM_DOLBY_DIGITAL_PLUS:
            aCodeType = DOLBY_DIGITAL_PLUS;
            break;
        case IM_DTS_HD:
            aCodeType = DTS_HD;
            break;
        case IM_MAT:
            aCodeType = MAT;
            break;
        case IM_DST:
            aCodeType = DST;
            break;
        case IM_WMAPRO:
            aCodeType = WMAPRO;
            break;
        default:
            IM_ERRMSG((IM_STR(" hdmi audio coding type error -- ")));
            return IM_RET_FAILED;
    }
    audioParams_SetInterfaceType(&hdmi_inst->HdmiAudio, aIfType);
    audioParams_SetCodingType(&hdmi_inst->HdmiAudio, aCodeType);
    audioParams_SetChannelAllocation(&hdmi_inst->HdmiAudio, aud_t->channelAllocation);
    audioParams_SetPacketType(&hdmi_inst->HdmiAudio, AUDIO_SAMPLE);
    audioParams_SetSampleSize(&hdmi_inst->HdmiAudio, aud_t->sampleSize);
    audioParams_SetSamplingFrequency(&hdmi_inst->HdmiAudio, aud_t->samplingFreq);
    audioParams_SetLevelShiftValue(&hdmi_inst->HdmiAudio, 0);
    audioParams_SetDownMixInhibitFlag(&hdmi_inst->HdmiAudio, 0);
    audioParams_SetClockFsFactor(&hdmi_inst->HdmiAudio, (aIfType == I2S)?64:128);// 64:I2S, 128:SPDIF

    if( audioSet(inst) != IM_RET_OK){
        return IM_RET_FAILED;
    }
    return IM_RET_OK;

}

IM_RET hdmilib_set_basic_hdcp_config(void *inst)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    if(hdmi_inst->hdcpOn == 0){
        hdmi_inst->hdcpOn = 1;
    }
    gDeviceOpenFlag = 0;
    hdcpParams_SetEnable11Feature(&hdmi_inst->HdmiHdcp, (HDCP_ENABLE_11_FEATURE));
    hdcpParams_SetRiCheck(&hdmi_inst->HdmiHdcp, (HDCP_RI_CHECK));
    hdcpParams_SetI2cFastMode(&hdmi_inst->HdmiHdcp, (HDCP_I2C_FASTMODE));
    hdcpParams_SetEnhancedLinkVerification(&hdmi_inst->HdmiHdcp, (HDCP_ENHANCED_LINK_VER));
    return IM_RET_OK;
}

IM_RET hdmilib_set_basic_product_config(void *inst)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    if(hdmi_inst->productOn == 0){
        hdmi_inst->productOn = 1;
    }
    gDeviceOpenFlag = 0;
    productParams_Reset(&hdmi_inst->HdmiProduct);
    productParams_SetVendorName(&hdmi_inst->HdmiProduct, (PRODUCT_VENDOR_NAME), 8);
    productParams_SetProductName(&hdmi_inst->HdmiProduct, (PRODUCT_NAME), 4);
    productParams_SetSourceType(&hdmi_inst->HdmiProduct, (PRODUCT_SOURCE_TYPE));
    productParams_SetOUI(&hdmi_inst->HdmiProduct, (PRODUCT_OUI));
    productParams_SetVendorPayload(&hdmi_inst->HdmiProduct, (PRODUCT_VENDOR_PAYLOAD), (PRODUCT_VENDOR_PAYLOAD_LENGTH));
    return IM_RET_OK;
}

IM_RET hdmilib_hpd_detect_and_read_edid(void *listener)
{
    IM_INT32 ret;
    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));

    if (gHdmiInst.hdmiStatic == IM_TRUE){
        // if hdmi as static device, no need to register interrupt listner, just return OK.
        return IM_RET_OK;
    }

    gVidNum_t = 0;
    gVidNum_A = 0;

    gHpdListener = listener;
    ret = api_hpd_detect(HDMI_BASE_ADDR,hpdCallback);
    if(ret == FALSE){
        return IM_RET_FAILED;
    }
    return IM_RET_OK;
}

IM_RET hdmilib_suspend(void *inst)
{
    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;
    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));

    if (gHdmiStateOn == IM_TRUE){
        /* hdmi still in display before suspend. we keep this state for two reasons:
           i)  hdmi_close won't be called before suspend.
           ii) BREAK signal must be noticed after resume when hdmi is on before suspend.
           */
        gHdmiOnBeforeSuspend = IM_TRUE;
        hdmilib_close(inst);
    }

    gDeviceInitFlag = 0;
    gDeviceOpenFlag = 0;
    hpdConnected = 0;
    edidDone = 0;
    gVidNum_t = 0;
    if(!api_DeInit()) 
    {
        return IM_RET_FAILED;
    }

    if (hdmi_inst->videoOn != 0)
    {
        videoParams_Reset(&hdmi_inst->HdmiVideo);
        hdmi_inst->videoOn = 0;
    }

    return IM_RET_OK;
}

IM_RET hdmilib_resume(void *inst, void *listener)
{
    IM_RET ret;
    IM_UINT32 hpdState;
    IM_INFOMSG((IM_STR("sam : %s - - %d - "),__func__,__LINE__));

    while(down_trylock(&gSema) == 0);

    gHdmiResumeState = 1;
    gHpdListener = listener;

    ret = api_hpd_detect(HDMI_BASE_ADDR,hpdCallback);
    if(ret == FALSE){
        IM_ERRMSG((IM_STR("sam :hdmilib_resume api_hpd_detect failed -")));
        return IM_RET_FAILED;
    }

    hpdState = (api_CoreRead(0x3004) & 0x02) >> 1;
    if (hpdState == 0){
        gVidNum_A = 0;
        gHdmiResumeState = 0;
        if(gHdmiLastState == HDMI_CONNECTED){
            queue_delayed_work(wq, &dwq, 0);
        }
    }

    IM_INFOMSG((IM_STR("sam : hdmilib_resume end - \n")));

    return IM_RET_OK;
}

IM_RET audioSet(void *inst)
{
    videoParams_t *pVid = NULL;
    audioParams_t *pAud = NULL;
    hdcpParams_t *pHdc = NULL;
    productParams_t *pPro = NULL;

    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;

    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));
    if(hdmi_inst->videoOn){
        pVid = &(hdmi_inst->HdmiVideo);
    }
    if(hdmi_inst->audioOn){
        pAud = &(hdmi_inst->HdmiAudio);
    }
    if(hdmi_inst->hdcpOn){
        pHdc = &(hdmi_inst->HdmiHdcp);
    }
    if(hdmi_inst->productOn){
        pPro = &(hdmi_inst->HdmiProduct);
    }

    if (pVid) {
        imapx800_switch_spk_hdmi(0);
        if(api_Audio_Cfg(pVid, pAud, pPro, pHdc) == 0){
            return IM_RET_FAILED;
        }
    }
    return IM_RET_OK;
}

IM_RET apiSet(void *inst)
{
    videoParams_t *pVid = NULL;
    audioParams_t *pAud = NULL;
    hdcpParams_t *pHdc = NULL;
    productParams_t *pPro = NULL;

    struct_hdmi_instance *hdmi_inst = (struct_hdmi_instance *) inst;

    IM_INFOMSG((IM_STR("sam : %s - - %d - \n"),__func__,__LINE__));
    if(hdmi_inst->videoOn){
        pVid = &(hdmi_inst->HdmiVideo);
    }
    if(hdmi_inst->audioOn){
        pAud = &(hdmi_inst->HdmiAudio);
    }
    if(hdmi_inst->hdcpOn){
        pHdc = &(hdmi_inst->HdmiHdcp);
    }
    if(hdmi_inst->productOn){
        pPro = &(hdmi_inst->HdmiProduct);
    }

    if (pVid){
        if (api_Configure(pVid, pAud, pPro, pHdc) == 0){
            return IM_RET_FAILED;
        }
        imapx800_switch_spk_hdmi(0);
    }
    return IM_RET_OK;
}

/* Checks for:
 *  1 . PixelClock valid
 *	2 . Max gVidNum_t 
 *	3 . Resolution + HZ 
 *	4 . Interlaced or not 
 **/
IM_INT32 check_dtd_valid(dtd_t *dtd)
{
    IM_BOOL valid = IM_TRUE;
    IM_INT32 index = -1;
    IM_INT32 cnt, freq1, freq2;

    if (dtd->mInterlaced){
        valid = IM_FALSE;
    }
    if (dtd->mPixelClock!=2520 && dtd->mPixelClock!=2700 && dtd->mPixelClock!=5400 && dtd->mPixelClock!=7200 &&
            dtd->mPixelClock!=7425 && dtd->mPixelClock!=10800 && dtd->mPixelClock!=14850){
        valid = IM_FALSE;
    }
    for(cnt = 0; cnt < gVidNum_t ; cnt++){
        if (dtd->mHActive==gVid_t[cnt].mHActive && dtd->mVActive==gVid_t[cnt].mVActive){
            freq1 = dtd->mPixelClock * 10000 / (dtd->mHActive + dtd->mHBlanking) / (dtd->mVActive + dtd->mVBlanking);
            freq2 = gVid_t[cnt].mPixelClock * 10000 / (gVid_t[cnt].mHActive + gVid_t[cnt].mHBlanking)
                / (gVid_t[cnt].mVActive + gVid_t[cnt].mVBlanking);
            if (freq1>freq2 && freq1<=60){
                index = cnt;
            }else {
                valid = IM_FALSE;
            }
        }
    }
    if (valid == IM_TRUE){
        if (gVidNum_t < MAX_VID_INDEX && index == -1){
            index = gVidNum_t;
        }
        else if((dtd->mHActive==1920&&dtd->mVActive==1080) || (dtd->mHActive==1280&&dtd->mVActive==720))
        {
            for (cnt=0; cnt<gVidNum_t; cnt++)
            {
                if ((gVid_t[cnt].mHActive!= 1280 || gVid_t[cnt].mVActive != 720) && (gVid_t[cnt].mHActive != 1920 ||
                            gVid_t[cnt].mVActive != 1080)){
                    index = cnt;
                    break;
                }
            }
        }
    }
    else{
        index = -1;
    }

    return index;
}

IM_INT32 edid_video_cb(dtd_t *dtd)
{
    IM_INT32 index;
    index = check_dtd_valid(dtd);
    if(index == -1){
        return -1;
    }
    //gVid_t[gVidNum_t].mCode = dtd->mCode;
    gVid_t[index].mCode = gVidNum_t;
    gVid_t[index].mPixelRepetitionInput = dtd->mPixelRepetitionInput;
    gVid_t[index].mPixelClock = dtd->mPixelClock;
    gVid_t[index].mInterlaced = dtd->mInterlaced;
    gVid_t[index].mHActive = dtd->mHActive;
    gVid_t[index].mHBlanking = dtd->mHBlanking;
    gVid_t[index].mHBorder = dtd->mHBorder;
    gVid_t[index].mHImageSize = dtd->mHImageSize;
    gVid_t[index].mHSyncOffset = dtd->mHSyncOffset;
    gVid_t[index].mHSyncPulseWidth = dtd->mHSyncPulseWidth;
    gVid_t[index].mHSyncPolarity = dtd->mHSyncPolarity;
    gVid_t[index].mVActive = dtd->mVActive;
    gVid_t[index].mVBlanking = dtd->mVBlanking;
    gVid_t[index].mVBorder = dtd->mVBorder;
    gVid_t[index].mVImageSize = dtd->mVImageSize;
    gVid_t[index].mVSyncOffset = dtd->mVSyncOffset;
    gVid_t[index].mVSyncPulseWidth = dtd->mVSyncPulseWidth;
    gVid_t[index].mVSyncPolarity = dtd->mVSyncPolarity;
    gVid_t[index].mFpsx1000 = dtd->mPixelClock * 10000 / (dtd->mHActive + dtd->mHBlanking) / (dtd->mVActive + dtd->mVBlanking) * 1000;
    //gVid_t[gVidNum_t].mFpsx1000 = mCodetoFpsx1000(dtd->mCode);
    if (index >= gVidNum_t)
        gVidNum_t ++;
    return 0;
}

IM_INT32 compliance_Standby(struct_hdmi_instance *hdmi_inst)
{
    if (hdmi_inst->videoOn != 0)
    {
        videoParams_Reset(&hdmi_inst->HdmiVideo);
        hdmi_inst->videoOn = 0;
    }
    if (hdmi_inst->audioOn != 0)
    {
        audioParams_Reset(&hdmi_inst->HdmiAudio);
        hdmi_inst->audioOn = 0;
    }
    if (hdmi_inst->productOn != 0)
    {
        productParams_Reset(&hdmi_inst->HdmiProduct);
        hdmi_inst->productOn = 0;
    }
    if (hdmi_inst->hdcpOn != 0)
    {
        hdcpParams_Reset(&hdmi_inst->HdmiHdcp);
        hdmi_inst->hdcpOn = 0;
    }
    return 0;
}

/***********************************************
 * callbacks
 **********************************************/
static void delay_func(struct work_struct *p)
{
    IM_UINT8 hpdState;
    IM_INT32 param, mask = 0;
    IM_INFOMSG((IM_STR("sam : delay_func ")));

    hpdState = (api_CoreRead(0x3004) & 0x02) >> 1;
    IM_INFOMSG((IM_STR("sam : hpdConnected =%lu, hpdState=%d, edidDone=%d \n"),hpdConnected,hpdState,edidDone));
    if (edidDone == 1){
        //  If true, must be state after suspend and HDMI is in display before suspend.
        if (gHdmiOnBeforeSuspend == IM_TRUE && gEdidEqual == IM_FALSE)
        {
            param = 0;
            gHpdListener((void*)&param);
            param = 1;
            gHpdListener((void*)&param);
        }
        else if (gEdidEqual == IM_FALSE || gHdmiLastState != HDMI_CONNECTED)
        {
            param = 1;
            gHpdListener((void*)&param);
        }
        gHdmiLastState = HDMI_CONNECTED;
        if (gHdmiResumeState == 1){
            up(&gSema); // edid done
        }
        gHdmiResumeState = 0;
    }
    else if (hpdConnected == 1 && hpdState == 1)
    {
        gVidNum_t = 0;
        mask = 1; // reading edid process, mask hpd.
        api_EdidRead(edidCallback);
        IM_INFOMSG((IM_STR("sam : waiting for edid done\n")));
        if (gHdmiPcbTestOn == 0){
            while(down_trylock(&gSemaEdid) == 0);
            down_timeout(&gSemaEdid, (long)300 * HZ / 1000); // timeout : 300ms
            IM_INFOMSG((IM_STR("sam : ### waiting done. edidDone=%d - \n"),edidDone));
        }
        if (edidDone == 0 && hpdConnected == 1){
            api_edid_done();
            param = 0;
            edidCallback(&param);
        }
    }
    else if(hpdConnected == 0 && hpdState == 0)
    {
        gVidNum_t = 0;
        gVidNum_A = 0;
        if (gHdmiOnBeforeSuspend == IM_TRUE || gHdmiStateOn == IM_TRUE){
            param = 0;
            gHpdListener((void*)&param);
            param = 1;
            gHpdListener((void*)&param);
        }
        else if (gHdmiLastState != HDMI_LOST)
        {
            param = 1;
            gHpdListener((void*)&param);
            if (gHdmiResumeState == 1){
                up(&gSema);  // hpd lost
            }
            gHdmiResumeState = 0;
        }
        gHdmiLastState = HDMI_LOST;
    }

    if (mask == 0){
        phy_InterruptEnable(0, ~0x02);
    }
}

static void edidCallback(void *param)
{
    IM_INT32 i;
    dtd_t dtd;
    IM_CHAR edid = *((IM_CHAR*)(param));
    IM_INFOMSG((IM_STR("sam : edidCallback :  ")));

    if (gHdmiPcbTestOn == 0){
        up(&gSemaEdid); // edid done
        // If only edid read error happens , return 720P support to user. 
        if (edid == 0 && hpdConnected){
            IM_INFOMSG((IM_STR("sam : edid read error -- \n")));
            dtd_Fill(&dtd,16,60000);
            edid_video_cb(&dtd); // 1080P 60Hz
            dtd_Fill(&dtd,4,60000);
            edid_video_cb(&dtd); // 720P 60Hz
        }
    }

    if (gVidNum_t == 0){
        dtd_Fill(&dtd,1,60000);
        edid_video_cb(&dtd); // VGA 60Hz
    }
    edidDone = 1;

    gEdidEqual = IM_TRUE;
    if (gVidNum_t == gVidNum_A)
    {
        for (i=0; i < gVidNum_A; i++)
        {
            if (gVid_A[i].mHActive != gVid_t[i].mHActive || gVid_A[i].mVActive != gVid_t[i].mVActive
                    || gVid_A[i].mFpsx1000 != gVid_t[i].mFpsx1000)
            {
                gEdidEqual = IM_FALSE;
                break;
            }
        }
    }else{
        gEdidEqual = IM_FALSE;
    }

    if (gEdidEqual == IM_FALSE){
        for(i=0; i < gVidNum_t; i++){
            memcpy((void*)&gVid_A[i], (void*)&gVid_t[i], sizeof(hdmi_video_t));
        }
        gVidNum_A = gVidNum_t;
        gVidNum_t = 0;
    }

    // 1jiffies = 5ms 
    if (gHdmiResumeState == 0){
        queue_delayed_work(wq, &dwq, 20);
    }
    else {
        queue_delayed_work(wq, &dwq, 0);
    }
}

static void hpdCallback(void *param)
{
    IM_CHAR hpd = *((IM_CHAR*)(param));
    hpdConnected = hpd;
    IM_INFOMSG((IM_STR("sam : hpdCallback: hpdConnected = %lu - "),hpdConnected));

    if (hpdConnected == 0 && gHdmiPcbTestOn ==0 )
        up(&gSemaEdid); // hpd lost
    edidDone = 0;

    // 1 jiffies = 5ms 
    if (gHdmiResumeState == 0){
        queue_delayed_work(wq, &dwq, 40);
    }
    else {
        queue_delayed_work(wq, &dwq, 0);
    }
}

IM_UINT32 mCodetoFpsx1000(IM_CHAR code)
{
    IM_UINT32 fpsx1000;
    switch (code){
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            fpsx1000 = 60000;
            break;
        case 8:
            fpsx1000 = 60054;
            break;
        case 9:
            fpsx1000 = 60054;
            break;
        case 10:
        case 11:
            fpsx1000 = 60000;
            break;
        case 12:
        case 13:
            fpsx1000 = 60054;
            break;
        case 14:
        case 15:
        case 16:
            fpsx1000 = 60000;
            break;
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            fpsx1000 = 50000;
            break;
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            fpsx1000 = 50000;
            break;
        case 32:
            fpsx1000 = 24000;
            break;
        case 33:
            fpsx1000 = 25000;
            break;
        case 34:
            fpsx1000 = 30000;
            break;
        case 35:
        case 36:
            fpsx1000 = 60000;
            break;
        case 37:
        case 38:
        case 39:
            fpsx1000 = 50000;
            break;
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
            fpsx1000 = 100000;
            break;
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
            fpsx1000 = 120000;
            break;
        case 52:
        case 53:
        case 54:
        case 55:
            fpsx1000 = 200000;
            break;
        case 56:
        case 57:
        case 58:
        case 59:
            fpsx1000 = 240000;
            break;
        default :
            fpsx1000 = 0;
            break;
    }
    return fpsx1000;
}


