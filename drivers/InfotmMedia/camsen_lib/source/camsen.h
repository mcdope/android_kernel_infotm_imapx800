/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: camsen.h
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
-- v1.0.3	arsor@2012/07/16: add api of power down & get pmu info.
--
------------------------------------------------------------------------------*/

#ifndef _CAMSEN_H_H_
#define _CAMSEN_H_H_


#define VER_MAJOR	1
#define VER_MINOR	0
#define VER_PATCH	3

#define VER_STRING	"1.0.3"

typedef struct camsen_ops {
	const char *name;		/* camera sensor module name */
	IM_UINT32 i2c_dev_addr;	//i2c device address(7bits address)

	IM_RET (*sen_pwdn)(IM_UINT32 padNum);
	IM_RET (*sen_get_pmu_info)(camsenpwl_pmu_info_t *pmuInfo);
	IM_RET (*sen_init)(pwl_handle_t pwl, IM_BOOL checkOnly);
	IM_RET (*sen_deinit)(void);
	IM_RET (*sen_start) (void); 
	IM_RET (*sen_stop)(void);
	IM_RET (*sen_get_caps)(camsen_caps_t *caps);
	IM_RET (*sen_get_out_mode)(camsen_out_mode_t *outMode);
	IM_RET (*sen_set_out_mode)(camsen_out_mode_t *outMode);
	IM_RET (*sen_set_property)(IM_UINT32 property, void *p);
	IM_RET (*sen_get_property)(IM_UINT32 property, void *p);

	struct camsen_ops *next;
}camsen_ops;

typedef struct{
	char name[256];
	pwl_handle_t pwl;
	camsen_out_mode_t outMode;
}camsen_context_t;

//sensor register and find api
void im_camsen_register_all(void);
camsen_ops *im_find_camsen(const char *name);

#endif /* #ifndef _CAMSEN_H_H_ */
