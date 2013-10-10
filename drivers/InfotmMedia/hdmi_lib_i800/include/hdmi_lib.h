/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of hdmi library 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __HDMI_LIB_H_
#define __HDMI_LIB_H_

IM_RET hdmilib_Handle(void **inst);
IM_RET hdmilib_init(void **inst);
IM_RET hdmilib_deinit(void *inst);
IM_RET hdmilib_open(void *inst );
IM_RET hdmilib_close(void *inst);
IM_RET hdmilib_get_config_num(void *inst,IM_UINT32 *num);
IM_RET hdmilib_index_to_mcode(void *inst,IM_UINT32 index, IM_INT32 *mCode);
IM_RET hdmilib_get_config(void *inst, IM_UINT32 index, hdmi_video_t *hdvid_t);
IM_RET hdmilib_set_basic_video_config(void *inst, IM_UINT32 index);
IM_RET hdmilib_set_video_encodings(void *inst, IM_VI_ENCODING In, IM_VI_ENCODING Out);
IM_RET hdmilib_set_basic_audio_config(void *inst,hdmi_audio_t *audio_t);
IM_RET hdmilib_set_basic_hdcp_config(void *inst);
IM_RET hdmilib_set_basic_product_config(void *inst);
IM_RET hdmilib_hpd_detect_and_read_edid(void *listener);
IM_RET hdmilib_suspend(void *inst);
IM_RET hdmilib_resume(void *inst, void *listener);

#endif // __HDMI_LIB_H_hdm
