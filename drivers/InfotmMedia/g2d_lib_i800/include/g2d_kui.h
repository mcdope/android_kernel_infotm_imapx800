/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_kui.h
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/03/16: first commit.
-- v1.0.2	arsor@2012/04/06: supports linux-3.0.8.
-- v1.0.3	arsor@2012/04/18: mmu test ok and fixed poll timeout bug.
--
------------------------------------------------------------------------------*/

#ifndef __G2D_KUI_H__
#define __G2D_KUI_H__

//#############################################################################
//g2d ioctl cmd and struct
#define G2DLIB_IOCTL_CMD_SETCFG		101
#define G2DLIB_IOCTL_CMD_DO			102
#define G2DLIB_IOCTL_CMD_GETID		103

//
// g2d config struct.
//
typedef struct{
	IM_UINT32		property;		//G2D_OPP_XXX
	IM_BOOL			useDevMMU;	
	g2d_mem_t		memCfg;
	g2d_draw_t		drawCfg;
	g2d_rotate_t	rotateCfg;
	g2d_alpharop_t	alpharopCfg;
}g2d_config_t;



//==============================interface=====================================
const void *g2dkui_init(void);
IM_RET g2dkui_deinit(void *instance);

IM_RET g2dkui_set_config(void *instance, g2d_config_t *config);
IM_RET g2dkui_do(void *instance, IM_INT32 taskId);
IM_RET g2dkui_wait(void *instance, IM_INT32 *taskId);
IM_RET g2dkui_suspend(void);
IM_RET g2dkui_resume(void);



//#############################################################################
#endif//__G2D_KUI_H__	

