/*
 * api_camera.h
 *
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Head file of IMAPX platform camera driver interfaces to user space.
 *
 * Author:
 *	Sololz<sololz.luo@gmail.com>.
 *
 * Revision History:
 * 1.0  04/26/2011 Sololz.
 * 	Create this file.
 */

#if !defined(__API_CAMERA_H__)
#define __API_CAMERA_H__

#if defined(__KERNEL__)
#define IC_DEV_NAME		"icam"
#else
#define IC_DEV_NAME		"/dev/icam"
#endif

//
// camera module id, range 0--15
//
#define DWL_MODULE_CAM_0	(0)
#define DWL_MODULE_COUNT	(1)

//
//
//
#define DWL_CAM_RES_QQCIF	(1<<0)	// 88*72
#define DWL_CAM_RES_SUB_QCIF	(1<<1)	// 128*96
#define DWL_CAM_RES_QQVGA	(1<<2)	// 160*120
#define DWL_CAM_RES_QCIF	(1<<3)	// 174*144
#define DWL_CAM_RES_QVGA	(1<<4)	// 320*240
#define DWL_CAM_RES_CIF		(1<<5)	// 352*288
#define DWL_CAM_RES_VGA		(1<<6)	// 640*480
#define DWL_CAM_RES_PAL		(1<<7)	// 768*576
#define DWL_CAM_RES_SVGA	(1<<8)	// 800*600
#define DWL_CAM_RES_XVGA	(1<<9)	// 1024*768
#define DWL_CAM_RES_720P	(1<<10)	// 1280*720
#define DWL_CAM_RES_SXGA	(1<<11)	// 1280*1024
#define DWL_CAM_RES_SXGAPlus	(1<<12)	// 1400*1050
#define DWL_CAM_RES_UXGA	(1<<13)	// 1600*1200
#define DWL_CAM_RES_1080P	(1<<14)	// 1920*1080
#define DWL_CAM_RES_WQXGA	(1<<15)	// 2560*1600
#define DWL_CAM_RES_QUXGA	(1<<16)	// 3200*2400
#define DWL_CAM_RES_WQXGA_U	(1<<17)	// 3840*2400

#define DWL_CAM_FPS_10		(1<<0)
#define DWL_CAM_FPS_12		(1<<1)
#define DWL_CAM_FPS_15		(1<<2)
#define DWL_CAM_FPS_18		(1<<3)
#define DWL_CAM_FPS_20		(1<<4)
#define DWL_CAM_FPS_22		(1<<5)
#define DWL_CAM_FPS_24		(1<<6)
#define DWL_CAM_FPS_25		(1<<7)
#define DWL_CAM_FPS_27		(1<<8)
#define DWL_CAM_FPS_29		(1<<9)
#define DWL_CAM_FPS_30		(1<<10)
#define DWL_CAM_FPS_60		(1<<11)
#define DWL_CAM_FPS_DEFAULT	(DWL_CAM_FPS_30 | DWL_CAM_FPS_25 | \
		DWL_CAM_FPS_22 | DWL_CAM_FPS_18)

//
// sensor light mode (white balance)
//
#define DWL_CAM_LIGHT_MODE_AUTO 		(1<<0)
#define DWL_CAM_LIGHT_MODE_INCANDESCENT  	(1<<1)
#define DWL_CAM_LIGHT_MODE_FLUORESCENT   	(1<<2)
#define DWL_CAM_LIGHT_MODE_DAYLIGHT  	 	(1<<3)
#define DWL_CAM_LIGHT_MODE_WARM_FLUORECENT	(1<<4)
#define DWL_CAM_LIGHT_MODE_CLOUDY_DAYLIGHT	(1<<5)
#define DWL_CAM_LIGHT_MODE_TWILIGHT		(1<<6)
#define DWL_CAM_LIGHT_MODE_SHADE  		(1<<7)

//
// sensor special effect
//
#define DWL_CAM_SPECIAL_EFFECT_NONE 	  	(1<<0)
#define DWL_CAM_SPECIAL_EFFECT_MONO 	  	(1<<1)
#define DWL_CAM_SPECIAL_EFFECT_NEGATIVE   	(1<<2)
#define DWL_CAM_SPECIAL_EFFECT_SOLARIZE	  	(1<<3)
#define DWL_CAM_SPECIAL_EFFECT_PASTEL 	  	(1<<4)
#define DWL_CAM_SPECIAL_EFFECT_MOSAIC 	  	(1<<5)
#define DWL_CAM_SPECIAL_EFFECT_RESIZE	  	(1<<6)
#define DWL_CAM_SPECIAL_EFFECT_SEPIA 	 	(1<<7)
#define DWL_CAM_SPECIAL_EFFECT_POSTERIZE  	(1<<8)
#define DWL_CAM_SPECIAL_EFFECT_WHITEBOARD	(1<<9)
#define DWL_CAM_SPECIAL_EFFECT_BLACKBOARD	(1<<10)
#define DWL_CAM_SPECIAL_EFFECT_AQUA 	  	(1<<11)

//
// [31:24] reserved
// [22] write
// [21] read 
// [20] 0--property, 1--control
// [19:16] reserved
// [15--0] code
//
#define DWL_CAM_IOCTRL_SENSOR_INIT			((0<<22) | (0<<21) | (1<<20) | (0))
#define DWL_CAM_IOCTRL_SENSOR_DEINIT			((0<<22) | (0<<21) | (1<<20) | (1))
#define DWL_CAM_IOCTRL_SENSOR_START			((0<<22) | (0<<21) | (1<<20) | (2))
#define DWL_CAM_IOCTRL_SENSOR_STOP			((0<<22) | (0<<21) | (1<<20) | (3))
#define DWL_CAM_IOCTRL_SENSOR_CHECK_CONNECT		((0<<22) | (0<<21) | (1<<20) | (100))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_RESOLUTION	((0<<22) | (1<<21) | (0<<20) | (101))
#define DWL_CAM_IOCTRL_SENSOR_GET_RESOLUTION		((0<<22) | (1<<21) | (0<<20) | (102))
#define DWL_CAM_IOCTRL_SENSOR_SET_RESOLUTION		((1<<22) | (0<<21) | (0<<20) | (103))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_FPS		((0<<22) | (1<<21) | (0<<20) | (104))
#define DWL_CAM_IOCTRL_SENSOR_GET_FPS			((0<<22) | (1<<21) | (0<<20) | (105))
#define DWL_CAM_IOCTRL_SENSOR_SET_FPS			((1<<22) | (0<<21) | (0<<20) | (106))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_LIGHT_MODE	((0<<22) | (1<<21) | (0<<20) | (200))
#define DWL_CAM_IOCTRL_SENSOR_GET_LIGHT_MODE		((0<<22) | (1<<21) | (0<<20) | (201))
#define DWL_CAM_IOCTRL_SENSOR_SET_LIGHT_MODE		((1<<22) | (0<<21) | (0<<20) | (202))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_COLOR_SATURATION ((0<<22) | (1<<21) | (0<<20) | (300))
#define DWL_CAM_IOCTRL_SENSOR_GET_COLOR_SATURATION	((0<<22) | (1<<21) | (0<<20) | (301))
#define DWL_CAM_IOCTRL_SENSOR_SET_COLOR_SATURATION	((1<<22) | (0<<21) | (0<<20) | (302))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_BRIGHTNESS	((0<<22) | (1<<21) | (0<<20) | (400))
#define DWL_CAM_IOCTRL_SENSOR_GET_BRIGHTNESS		((0<<22) | (1<<21) | (0<<20) | (401))
#define DWL_CAM_IOCTRL_SENSOR_SET_BRIGHTNESS		((1<<22) | (0<<21) | (0<<20) | (402))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_CONTRAST	((0<<22) | (1<<21) | (0<<20) | (500))
#define DWL_CAM_IOCTRL_SENSOR_GET_CONTRAST		((0<<22) | (1<<21) | (0<<20) | (501))
#define DWL_CAM_IOCTRL_SENSOR_SET_CONTRAST		((1<<22) | (0<<21) | (0<<20) | (502))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_SPECIAL_EFFECT ((0<<22) | (1<<21) | (0<<20) | (600))
#define DWL_CAM_IOCTRL_SENSOR_GET_SPECIAL_EFFECT	((0<<22) | (1<<21) | (0<<20) | (601))
#define DWL_CAM_IOCTRL_SENSOR_SET_SPECIAL_EFFECT	((1<<22) | (0<<21) | (0<<20) | (602))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_EXPOSURE	((0<<22) | (1<<21) | (0<<20) | (700))
#define DWL_CAM_IOCTRL_SENSOR_GET_EXPOSURE		((0<<22) | (1<<21) | (0<<20) | (701))
#define DWL_CAM_IOCTRL_SENSOR_SET_EXPOSURE		((1<<22) | (0<<21) | (0<<20) | (702))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_SHARPNESS	((0<<22) | (1<<21) | (0<<20) | (800))
#define DWL_CAM_IOCTRL_SENSOR_GET_SHARPNESS		((0<<22) | (1<<21) | (0<<20) | (801))
#define DWL_CAM_IOCTRL_SENSOR_SET_SHARPNESS		((1<<22) | (0<<21) | (0<<20) | (802))
#define DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_MIRRORANDFLIP	((0<<22) | (1<<21) | (0<<20) | (900))
#define DWL_CAM_IOCTRL_SENSOR_GET_MIRRORANDFLIP		((0<<22) | (1<<21) | (0<<20) | (901))
#define DWL_CAM_IOCTRL_SENSOR_SET_MIRRORANDFLIP		((1<<22) | (0<<21) | (0<<20) | (902))

//
// macro operations of ioctl command.
//
#define DWL_IOCTRL_WRITE(cmd)	(cmd & (1 << 22))
#define DWL_IOCTRL_READ(cmd)	(cmd & (1 << 21))
#define DWL_IOCTRL_CTRL(cmd)	(cmd & (1 << 20))
#define DWL_IOCTRL_CODE(cmd)	(cmd & 0xffff)

// CamIF features.
#define CAMIF_REGISTER_PHYS_BASE (0x20cc0000)
#define CAMIF_REGISTER_SIZE	(4096)

#endif	/* __API_CAMERA_H__ */
