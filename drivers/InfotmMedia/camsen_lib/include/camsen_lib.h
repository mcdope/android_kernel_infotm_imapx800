/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: camsen_lib.h
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/05: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/16: add api of pmu set handle and power down.
--
------------------------------------------------------------------------------*/

#ifndef _CAMERA_SENSOR_LIB_H_
#define _CAMERA_SENSOR_LIB_H_


typedef struct {
	IM_UINT32 res;
	IM_UINT32 fmt;
	IM_UINT32 fps;
}camsen_out_mode_t;

//typedef cam_preview_config_t camsen_caps_t;

typedef struct {
	IM_UINT32 supportRes;
	IM_UINT32 maxRes;
	IM_UINT32 initRes;
}camsen_caps_t;

typedef void * camsen_handle_t;

/*------------------------------------------------------------------------------
    Function prototypes                                                         
------------------------------------------------------------------------------*/
IM_UINT32 camsen_version(void);
void camsen_register_all(void); //this function must be called before follow api function called
void camsen_set_pmu_handle(void *pmuHandle);
IM_RET camsen_pwdn(IM_CHAR *moduleName, IM_UINT32 facing);

IM_RET camsen_init(camsen_handle_t *handle, IM_CHAR *module_name, IM_UINT32 facing, IM_BOOL checkOnly);
IM_RET camsen_deinit(camsen_handle_t handle);
IM_RET camsen_start(camsen_handle_t handle);
IM_RET camsen_stop(camsen_handle_t handle);
IM_RET camsen_get_caps(camsen_handle_t handle, camsen_caps_t *caps);
IM_RET camsen_get_out_mode(camsen_handle_t handle, camsen_out_mode_t *outMode);
IM_RET camsen_set_out_mode(camsen_handle_t handle, camsen_out_mode_t *outMode);
IM_RET camsen_get_property(camsen_handle_t handle, IM_UINT32 property, void *p);
IM_RET camsen_set_property(camsen_handle_t handle, IM_UINT32 property, void *p);

#endif /* #ifndef _CAMERA_SENSOR_LIB_H_ */
