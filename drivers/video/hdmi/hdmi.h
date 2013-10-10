/***************************************************************************** 
 * ** drivers/video/infotm_HDMI/imap_HDMI.h
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Head file of Infotm HDMI.
 * **
 * ** Author:
 * **     Alex Zhang <alex.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.0  06/11/2010 Alex Zhang 
* *****************************************************************************/
#ifndef __HDMI_H__
#define __HDMI_H__

#undef HDMI_DEBUG
//#define HDMI_DEBUG

#ifdef HDMI_DEBUG
#define hdmi_debug(fmt, ...)	\
	printk(fmt, ##__VA_ARGS__)
#else
#define hdmi_debug(fmt, ...)	\
	do{} while(0)
#endif

#define HDMI_HP_POLL_DELAY      1000    

typedef enum
{
	LCD = 0,
	HDMI_1080P,
//	HDMI_1080I,
	HDMI_720P,
	HDMI_480P_16_9,
	HDMI_480P_4_3,
//	HDMI_480I_16_9,
//	HDMI_480I_4_3,
	HDMI_576P_16_9,
	HDMI_576P_4_3,
//	HDMI_576I_16_9,
//	HDMI_576I_4_3,
	HDMI_640_480,	
	HDMI_800_600,	
	HDMI_1024_768,	
	HDMI_1080P_TV,
//	HDMI_1080I,
	HDMI_720P_TV,
	HDMI_480P_16_9_TV,
	HDMI_480P_4_3_TV,
//	HDMI_480I_16_9,
//	HDMI_480I_4_3,
	HDMI_576P_16_9_TV,
	HDMI_576P_4_3_TV,
//	HDMI_576I_16_9,
//	HDMI_576I_4_3,
	HDMI_640_480_TV,	
	HDMI_800_600_TV,	
	HDMI_1024_768_TV,	
}video_timing;

#define HDMI_CHECK_HOTPLUG				_IOR('H', 301, uint32_t)
#define HDMI_SET_NOTMAL_TIMING				_IO('H', 302)
#define HDMI_SET_VIDEO_TIMING				_IOW('H', 303, uint32_t)
#define HDMI_QUERY_APP					_IO('H', 304)
#define HDMI_QUERY_RENDER				_IO('H', 305)
#define HDMI_CHECK_MENUSWITCH				_IOR('H', 306, uint32_t)
#define HDMI_CHECK_MODE					_IOR('H', 307, uint32_t)
#define HDMI_QUERY_MONITOR				_IO('H', 308)
#define HDMI_TV_SUPPORT_V_MODE  			_IOR('H', 310, uint32_t)
#define HDMI_SET_DOUBLE_DISPLAY_TIMING			_IO('H', 311)
#define HDMI_SET_PANAUISION_TIMING			_IO('H', 312)
#define HDMI_SET_MODE					_IOW('H', 313, uint32_t)
#define HDMI_CHECK_CONNECT				_IOR('H', 314, uint32_t)
#define HDMI_SET_MODE_FOR_MOUSE 			_IO('H', 315)
#define HDMI_DISCONNECT	 				_IO('H', 316)
#define	HDMI_BACKLIGHT_CONTROL				_IOW('H', 317, uint32_t)
#define	HDMI_SPEAKER_CONTROL				_IOW('H', 318, uint32_t)

struct hdmi_ops {
	int (*chip_init)(void);
	void (*power)(uint32_t on_off);
	int (*get_connection_status)(void);
	void (*mute_output)(video_timing timing, uint32_t mute);
};

#endif /*__HDMI_H__*/
