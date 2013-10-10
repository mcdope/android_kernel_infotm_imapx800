/*
 * sensor.h
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
 * Head file of IMAPX platform camera driver sensor level.
 *
 * Author:
 *	Sololz<sololz.luo@gmail.com>.
 *
 * Revision History:
 * 1.0  04/28/2011 Sololz.
 * 	Create this file.
 */

#if !defined(__SENSOR_H__)
#define __SENSOR_H__

#include <linux/delay.h>
#include <linux/i2c.h>

#include <plat/clock.h>

#include <asm/atomic.h>

#include <mach/imapx_base_reg.h>

#include "../camera.h"

/* ############################################################################## */

#define	INIT_SENSOR 			(1)
#define SWITCH_SENSOR_TO_HIGH_SVGA   	(2)
#define SWITCH_SENSOR_TO_LOW_SVGA 	(3)
#define	SWITCH_SENSOR_TO_HIGH_XUGA   	(4)
#define SWITCH_SENSOR_TO_UPMID_XUGA     (5)
#define	SWITCH_SENSOR_TO_MID_XUGA   	(6)
#define	SWITCH_SENSOR_TO_LOW_XUGA   	(7)
#define	SENSOR_TO_HIGH_PREVIEW		(8)
#define	SENSOR_TO_LOW_PREVIEW		(9)
#define	CLOSE_SENSOR			(10)

#define SENSOR_WB_AUTO 			DWL_CAM_LIGHT_MODE_AUTO
#define SENSOR_WB_INCANDESCENT		DWL_CAM_LIGHT_MODE_INCANDESCENT
#define	SENSOR_WB_FLUORESCENT		DWL_CAM_LIGHT_MODE_FLUORESCENT
#define	SENSOR_WB_DAYLIGHT		DWL_CAM_LIGHT_MODE_DAYLIGHT
#define	SENSOR_WB_WARM_FLUORECENT	DWL_CAM_LIGHT_MODE_WARM_FLUORECENT
#define	SENSOR_WB_CLOUDY	 	DWL_CAM_LIGHT_MODE_CLOUDY_DAYLIGHT
#define	SENSOR_WB_TWILIGHT		DWL_CAM_LIGHT_MODE_TWILIGHT
#define	SENSOR_WB_SHADE			DWL_CAM_LIGHT_MODE_SHADE

#define	SENSOR_EFFECT_OFF  		DWL_CAM_SPECIAL_EFFECT_NONE
#define SENSOR_EFFECT_MONO		DWL_CAM_SPECIAL_EFFECT_MONO
#define	SENSOR_EFFECT_NEGATIVE 		DWL_CAM_SPECIAL_EFFECT_NEGATIVE
#define	SENSOR_EFFECT_SOLARIZE		DWL_CAM_SPECIAL_EFFECT_SOLARIZE
#define	SENSOR_EFFECT_PASTEL		DWL_CAM_SPECIAL_EFFECT_PASTEL
#define	SENSOR_EFFECT_MOSAIC		DWL_CAM_SPECIAL_EFFECT_MOSAIC
#define	SENSOR_EFFECT_RESIZE		DWL_CAM_SPECIAL_EFFECT_RESIZE
#define	SENSOR_EFFECT_SEPIA		DWL_CAM_SPECIAL_EFFECT_SEPIA
#define	SENSOR_EFFECT_POSTERIZE		DWL_CAM_SPECIAL_EFFECT_POSTERIZE
#define	SENSOR_EFFECT_WHITEBOARD 	DWL_CAM_SPECIAL_EFFECT_WHITEBOARD
#define	SENSOR_EFFECT_BLACKBOARD	DWL_CAM_SPECIAL_EFFECT_BLACKBOARD
#define	SNESOR_EFFECT_AQUA		DWL_CAM_SPECIAL_EFFECT_AQUA

#define	SENSOR_BRIGHTNESS_0		(1)
#define	SENSOR_BRIGHTNESS_1		(2)
#define	SENSOR_BRIGHTNESS_2		(3)
#define	SENSOR_BRIGHTNESS_3		(4)
#define	SENSOR_BRIGHTNESS_4		(5)
#define	SENSOR_BRIGHTNESS_5		(6)
#define	SENSOR_BRIGHTNESS_6		(7)

#define SENSOR_SCENCE_AUTO		(1)
#define SENSOR_SCENCE_ACTION		(2)
#define	SENSOR_SCENCE_PORTRAIT		(3)
#define	SENSOR_SCENCE_LANDSCAPE		(4)
#define	SENSOR_SCENCE_NIGHT		(5)

/*
 * Add sensor delay for cpu switching 
 * in i2c register configuration process.
 */
#define SENSOR_DELAY()	\
({	\
	if(i%5 == 0) \
		msleep(1); \
})

/* ############################################################################## */

struct sensor_ops {
	const uint32_t s_res;		/* Support resolution. */
	uint32_t (*get_fps)(uint32_t res);
	const uint32_t s_lmode;		/* Support light mode. */
	const uint32_t s_seffect;	/* Support spec effect. */

	/* int (*detect)(void __iomem *reg); */
	int (*power_on)(void);
	int (*power_off)(void);
	int (*reset) (void __iomem *reg); 
	int (*set_mode)(int value, void __iomem *reg);
	int (*set_effect)(int value);
	int (*set_wb)(int value);
	uint32_t (*get_id)(void);
	int (*i2c_read)(uint8_t *buf, uint8_t *addr, uint32_t size, uint32_t len);
	int (*i2c_write)(uint8_t *buf, uint32_t len);

#if 0
	int (*switch_low_svga)(void);
	int (*switch_high_svga)(void);
	int (*switch_high_xuga)(void);
	int (*switch_upmid_xuga)(void);
	int (*switch_mid_xuga)(void);
	int (*switch_low_xuga)(void);
	int (*svga_to_xuga)(void);
	int (*to_preview_320_240)(void);
	int (*to_preview_640_480)(void);
	int (*set_sepia)(void);
	int (*set_bluish)(void);
	int (*set_greenish)(void);
	int (*set_reddish)(void);
	int (*set_yellowish)(void);
	int (*set_bandw)(void);
	int (*set_negative)(void);
	int (*set_normal)(void);
	int (*set_auto)(void);
	int (*set_sunny)(void);
	int (*set_cloudy)(void);
	int (*set_office)(void);
	int (*set_home)(void);
	int (*saturation_0)(void);
	int (*saturation_1)(void);
	int (*saturation_2)(void);
	int (*saturation_3)(void);
	int (*saturation_4)(void);
	int (*brightness_0)(void);
	int (*brightness_1)(void);
	int (*brightness_2)(void);
	int (*brightness_3)(void);
	int (*brightness_4)(void);
	int (*brightness_5)(void);
	int (*brightness_6)(void);
	int (*contrast_0)(void);
	int (*contrast_1)(void);
	int (*contrast_2)(void);
	int (*contrast_3)(void);
	int (*contrast_4)(void);
	int (*contrast_5)(void);
	int (*contrast_6)(void);
	int (*sharpness_0)(void);
	int (*sharpness_1)(void);
	int (*sharpness_2)(void);
	int (*sharpness_3)(void);
	int (*sharpness_4)(void);
	int (*sharpness_auto)(void);
	int night_mode_on(void);
	int night_mode_off(void);
#endif

	/* this enables sensor_ops
	 * to be listed in a link list
	 */
	char name[64];			/* module name */
	struct i2c_adapter *adapter;	/* i2c adapter */
	struct list_head link;		/* list truct, enable the struct can be linked together */
	uint8_t	addr;			/* i2c slave address */
	uint8_t	pwdn;			/* whether pull down pwdn when power on. 1 indicates pull low while power on */
	uint8_t	hwid;			/* the relevant hwid */
	uint32_t *idlist;		/* supported idlist, which will be used during cmos id detection */
};

struct res_to_fps_map {
	uint32_t res;
	uint32_t fps;
};

#define MAX_SEN_NUM	(25)
int imapx200_cam_sensor_register(struct sensor_ops *ops);
int imapx200_cam_sensor_unregister(struct  sensor_ops *ops);

#endif	/* __SENSOR_H__ */
