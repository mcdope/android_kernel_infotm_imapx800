/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of HDMI APIs
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/
#ifndef __HDMI_API_H__
#define __HDMI_API_H__

//
//
//
typedef enum
{
	IM_RGB = 0,
	IM_YCC444,
	IM_YCC422
} IM_VI_ENCODING;

typedef enum
{
	IM_I2S = 0, 
	IM_SPDIF
} IM_AU_INTERFACE_TYPE;

typedef enum
{
	IM_PCM = 1,
	IM_AC3,
	IM_MPEG1,
	IM_MP3,
	IM_MPEG2,
	IM_AAC,
	IM_DTS,
	IM_ATRAC,
	IM_ONE_BIT_AUDIO,
	IM_DOLBY_DIGITAL_PLUS,
	IM_DTS_HD,
	IM_MAT,
	IM_DST,
	IM_WMAPRO
} IM_AU_ENCODING;

typedef struct {
	IM_AU_INTERFACE_TYPE aInterfaceType;
	IM_AU_ENCODING 		aCodingType;
	IM_UINT32 	channelAllocation;
	IM_UINT32 	sampleSize;
	IM_UINT32 samplingFreq;
}hdmi_audio_t;


typedef struct {
	IM_UINT32 mCode;
	IM_UINT32 mPixelRepetitionInput;
	IM_UINT32 mPixelClock;
	IM_BOOL mInterlaced;
	IM_UINT32 mHActive;
	IM_UINT32 mHBlanking;
	IM_UINT32 mHBorder;
	IM_UINT32 mHImageSize;
	IM_UINT32 mHSyncOffset;
	IM_UINT32 mHSyncPulseWidth;
	IM_BOOL mHSyncPolarity;
	IM_UINT32 mVActive;
	IM_UINT32 mVBlanking;
	IM_UINT32 mVBorder;
	IM_UINT32 mVImageSize;
	IM_UINT32 mVSyncOffset;
	IM_UINT32 mVSyncPulseWidth;
	IM_BOOL mVSyncPolarity;
	IM_UINT32 mFpsx1000;
}hdmi_video_t;

// DI stardard interface.
IM_RET hdmi_init(dispdev_handle_t *handle, IM_INT32 idsx);
IM_RET hdmi_deinit(dispdev_handle_t handle);
IM_RET hdmi_set_video_basic_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix);
IM_RET hdmi_get_video_basic_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix);
IM_RET hdmi_set_video_encodings(dispdev_handle_t handle, IM_VI_ENCODING in, IM_VI_ENCODING out);
IM_RET hdmi_set_audio_basic_config(hdmi_audio_t *audio_t);
IM_RET hdmi_set_hdcp_basic_config(dispdev_handle_t handle);
IM_RET hdmi_set_product_basic_config(dispdev_handle_t handle);
IM_RET hdmi_open(dispdev_handle_t handle);
IM_RET hdmi_close(dispdev_handle_t handle);
IM_RET hdmi_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener);
IM_RET hdmi_suspend(dispdev_handle_t handle);
IM_RET hdmi_resume(dispdev_handle_t handle);

// Extended interface.


#endif	// __HDMI_API_H__

