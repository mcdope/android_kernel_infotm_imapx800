/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of hmid internal configuration definitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __HDMI_H_
#define __HDMI_H_

#define HDMI_TX_CTRL_ADDRESS	(0x23040000)

#define HDMI_BASE_ADDR     (0)

// Audio MODULE 
#define AUDIO_IF_TYPE 			(SPDIF) // interfaceType_t
#define AUDIO_CODE_TYPE			(PCM) // codingType_t
#define AUDIO_CHANNEL_ALLOC 	(0x02)
#define AUDIO_SAMPLE_SIZE 		(16)
#define AUDIO_SAMPLE_FREQ       (44100)
#define AUDIO_LEVEL_SHIFT_VAL   (0)
#define AUDIO_DOWNMIX_INHIBIT_FLAG      (0)
#define AUDIO_ORIGINAL_SF		(44100) // 32000
#define AUDIO_IEC_COPYRIGHT		(1)
#define AUDIO_IEC_CGMSA			(3)
#define AUDIO_IEC_PCM_MODE		(0)
#define AUDIO_IEC_CATEGORY_CODE (0)
#define AUDIO_IEC_SOURCE_NUM    (1)
#define AUDIO_IEC_CLK_ACCURACY  (0)
#define AUDIO_PACKET_TYPE 		(AUDIO_SAMPLE)  // packet_t
#define AUDIO_CLK_FS_FACTOR     (512)

// VIDEO MODULE
#define VIDEO_HDMI_MODE 		(1)
#define VIDEO_ENCODE_OUT		(RGB) // encoding_t
#define VIDEO_ENCODE_IN			(RGB) // encoding_t
#define VIDEO_COLOR_RES			(8)
#define VIDEO_PIXEL_REP_FAC		(0)
#define VIDEO_PIXEL_PACK_PHASE  (0)
#define VIDEO_COLORMETRY		(ITU601) // colorimetry_t
#define VIDEO_CSC_FILTER 		(0)
#define VIDEO_CSC_SCALE 		(0)
#define VIDEO_CSC_A_0			(0)
#define VIDEO_CSC_A_1			(0)
#define VIDEO_CSC_A_2			(0)
#define VIDEO_CSC_A_3			(0)
#define VIDEO_CSC_B_0			(0)
#define VIDEO_CSC_B_1			(0)
#define VIDEO_CSC_B_2			(0)
#define VIDEO_CSC_B_3			(0)
#define VIDEO_CSC_C_0			(0)
#define VIDEO_CSC_C_1			(0)
#define VIDEO_CSC_C_2			(0)
#define VIDEO_CSC_C_3			(0)
#define VIDEO_SCAN_INFO			(0)
#define VIDEO_AFAR_RATIO			(8)
#define VIDEO_NON_UNIFORM_SCALE (0)
#define VIDEO_RGB_QUAN_RANGE	(0)
#define VIDEO_EXT_COLORIMETRY	(~0)
#define VIDEO_IT_COTENT			(0)
#define VIDEO_END_TOP_BAR		(~0)
#define VIDEO_START_BOTTOM_BAR	(~0)
#define VIDEO_END_LEFT_BAR		(~0)
#define VIDEO_START_RIGHT_BAR	(~0)
#define VIDEO_PIXEL_REP_INPUT 	(0)
#define VIDEO_DTD_FROM_EDID		(0)
#define VIDEO_DTD_EDID_NUM		(0)
#define VIDEO_DTD_REFRESH_RATE  (6000)
#define VIDEO_DTD_CODE			(1)
#define VIDEO_3D_ON				(0)
#define VIDEO_3D_STRUCTURE		(0)

// HDCP MODULE 
#define HDCP_ENABLE_11_FEATURE	(0)
#define HDCP_RI_CHECK 			(0)
#define HDCP_I2C_FASTMODE		(0)
#define HDCP_ENHANCED_LINK_VER	(0)

// PRODUCT MODULE 
#define PRODUCT_VENDOR_NAME 		("synopsys")
#define PRODUCT_NAME				("HDMI")
#define PRODUCT_SOURCE_TYPE			(-1)
#define PRODUCT_OUI					(~1)
#define PRODUCT_VENDOR_PAYLOAD 		(NULL)
#define PRODUCT_VENDOR_PAYLOAD_LENGTH	(0)

typedef struct hdmi_audio_conf{
	interfaceType_t mInterfaceType;
	codingType_t mCodingType; /** (audioParams_t *params, see InfoFrame) */
	IM_UINT8 mChannelAllocation; /** channel allocation (audioParams_t *params, 
						   see InfoFrame) */
	IM_UINT8 mSampleSize; /**  sample size (audioParams_t *params, 16 to 24) */
	IM_UINT32 mSamplingFrequency; /** sampling frequency (audioParams_t *params, Hz) */
	IM_UINT32 mOriginalSamplingFrequency; /** Original sampling frequency */ // sam : what means ? 
	packet_t mPacketType; /** packet type. currently only Audio Sample (AUDS) 
						  and High Bit Rate (HBR) are supported */
	IM_UINT16 mClockFsFactor; /** Input audio clock Fs factor used at the audop 
						  packetizer to calculate the CTS value and ACR packet insertion rate */
}hdmi_audio_conf;

typedef struct hdmi_video_conf{
	IM_UINT8 mHdmi;
	encoding_t mEncodingOut;
	encoding_t mEncodingIn;
	IM_UINT8 mColorResolution;
	IM_UINT8 mPixelRepetitionFactor;
	IM_UINT8 CeaCode; // For dtd set.
	IM_UINT32 refreshRate;
	u8	mColorimetry;
	IM_UINT8 mPixelPackingDefaultPhase;
	IM_UINT8 mHdmiVideoFormat; // 3D hdmi set
	IM_UINT8 m3dStructure;
	IM_UINT8 m3dExtData;
}hdmi_video_conf;

typedef struct {
	/*
	IM_INT32 hpdConnected ;
	IM_INT32 reconfig ;
	IM_INT32 edidDone ;
	*/

	/* compliance mode */
	/* options: hdcp, 3d X, next(mode), modes(all), mode X*/
	IM_INT32 videoOn;
	IM_INT32 audioOn;
	IM_INT32 hdcpOn;
	IM_INT32 productOn;

	videoParams_t HdmiVideo ;
	audioParams_t HdmiAudio;
	hdcpParams_t HdmiHdcp;
	productParams_t HdmiProduct;

	IM_BOOL hdmiStatic;
}struct_hdmi_instance;

#endif // __HDMI_H_
