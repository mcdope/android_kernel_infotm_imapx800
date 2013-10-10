/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camisp_api.h
--
--  Description :
--		Head file for work model.
--
--	Author:
--  	Leo Zhang   <leo.zhang@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	leo@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __CAMISP_API_H__
#define __CAMISP_API_H__

#define CAMISP_INTERFACE_DVP			0
#define CAMISP_INTERFACE_MIPI			1



#define CAMISP_IOCTL_CMD_GET_MODULES	0x45674980
#define CAMISP_IOCTL_CMD_OPEN			0x45674981
#define CAMISP_IOCTL_CMD_CLOSE			0x45674982
#define CAMISP_IOCTL_CMD_SET_CFG		0x45674983
#define CAMISP_IOCTL_CMD_GET_CFG		0x45674984
#define CAMISP_IOCTL_CMD_SET_PRO		0x45674985
#define CAMISP_IOCTL_CMD_GET_PRO		0x45674986
#define CAMISP_IOCTL_CMD_ASSIGN_BUF		0x45674987
#define CAMISP_IOCTL_CMD_RELEASE_BUF	0x45674988
#define CAMISP_IOCTL_CMD_SEND_CTR		0x45674989
#define CAMISP_IOCTL_CMD_CANCEL_CTR		0x4567498a
#define CAMISP_IOCTL_CMD_START			0x4567498b
#define CAMISP_IOCTL_CMD_STOP			0x4567498c
#define CAMISP_IOCTL_CMD_GET_EVENT		0x4567498d

//ioctl struct
typedef struct{
	IM_UINT32	res;	// CAM_RES_xxx.
	IM_UINT32	fmt;	// CAM_PIXFMT_xxx.
	IM_UINT32	fps;	// CAM_FPS_xxx.
}ioctl_ds_set_config;

typedef struct{
	IM_INT32	code;
	IM_INT32	id;
}ioctl_ds_cancel_control;

typedef struct {
	IM_INT32 key;
	IM_INT32 size;
	IM_UINT32 param[128];// cast to appropriate property "value" pointer.
}ioctl_ds_key_property;


//
// control word format.
//
#define CTRLWORD_DATASTRUCT_MAX_SIZE	64
typedef struct{
	IM_INT32	code;	// CAM_CTRL_CODE_XXX.
	IM_BOOL		async;
	IM_INT32	id;		// the unique id of the ctrl word.
	IM_RET		ret;	// the excuted status of the ctrl word.

	IM_CHAR		ds[CTRLWORD_DATASTRUCT_MAX_SIZE];	// data struct.
}ctrl_word_t;


typedef ctrl_word_t ioctl_ds_send_control;

//
// 设计思路：
// 1. 首先control线程调用control_exec(done=false)函数执行该控制字；
// 2. 当control_exec函数执行到某一点时可以选择返回IM_RET_RETRY，这样给予线程机会可以作取消该控制的动作；
// 3. 如果线程未检测到取消该控制字的命令，则继续调用1；如果检测到，则调用control_exec(done=true)函数，这样将保留
// 在之前的状态，所以在实现该函数时，要选择合适的点作为返回；
// 4. 当控制执行结束时，control_exec返回最终的状态，然后control线程会调用controlDone通知调用者完成了该控制字的动作。
// 
// 被取消的控制字的cw.ret=IM_RET_FALSE.
//
typedef IM_RET (*func_control_exec_t)(ctrl_word_t *cw, IM_BOOL done);

//
// 支持创建多进行并行执行控制，一个线程可以串行执行多个控制字。这里通过ctCodeBox用来存放各线程应该响应的控制字，
// 同一线程支持的控制字将调用同一个control_exec()。
// 如果控制字不属于任何一个线程盒，将不执行。
//
#define CONTROL_THREAD_COUNT_MAX	4
typedef struct{
	IM_INT32		ctCnt;	// control thread count.
	IM_INT32		ctCodeBoxSize[CONTROL_THREAD_COUNT_MAX];	// number of control code in box.
	IM_INT32		ctCodeBox[CONTROL_THREAD_COUNT_MAX][CAM_CTRL_CODE_NUMBER];	// control code box, which codes use the same control thread.
	func_control_exec_t	control_exec[CONTROL_THREAD_COUNT_MAX];
}ctrlmodel_config_t;


//
// stream model.
//
typedef struct{
	IM_INT32	path;	// CAM_PATH_xxx.
	IM_INT32	id;
	IM_INT64    ts;
	IM_Buffer	buffer;
}strm_frame_t;

typedef void (*func_frame_done_t)(strm_frame_t *frm);
typedef struct{
	func_frame_done_t		frame_done;	// when camapi has captured a frame, it can call frame_done to notify the work model.
}strmmodel_config_t;

//
// device listener model.
// 设备检测的动作放到camera之外来处理，包括镜头转动，连接断开等，这样简化camera的设计，而且也显得更干净。
//

//
// camapi interface.
//
typedef struct{
	IM_CHAR		camsenName[256];
	IM_UINT32	itfType;	//interface type: CAMISP_INTERFACE_DVP(0) or CAMISP_INTERFACE_MIPI(1)
	IM_UINT32	dataType;
	IM_UINT32 	facing;		
	IM_INT32	reserved;
}camdev_config_t;

struct __workmodel_config_t;
typedef IM_RET (*func_camapi_init_t)(camdev_config_t *devcfg, struct __workmodel_config_t *wmcfg);
typedef IM_RET (*func_camapi_deinit_t)(void);
typedef IM_RET (*func_camapi_ioctl)(IM_INT32 cmd, void *p);
typedef IM_RET (*func_camapi_suspend_t)(void);
typedef IM_RET (*func_camapi_resume_t)(void);
typedef struct{
	func_camapi_init_t	camapi_init;
	func_camapi_deinit_t	camapi_deinit;
	func_camapi_ioctl	camapi_ioctl;
	func_camapi_suspend_t	camapi_suspend;
	func_camapi_resume_t	camapi_resume;
}camapi_interface_t;

IM_RET workmodel_register_camapi(camapi_interface_t *apis);	// camapi library implement this function to connect to the work model.


//
//
//
#define WORKMODEL_EVENT_CONTROL_DONE	1
#define WORKMODEL_EVENT_FRAME_DONE		2

typedef struct{
	IM_INT32	type;	// WORKMODEL_EVENT_xxx.
	union{
		ctrl_word_t	cw;
		strm_frame_t	frm;
	}contents;
}workmodel_event_t;

typedef struct __workmodel_config_t{
	ctrlmodel_config_t		cmcfg;
	strmmodel_config_t		smcfg;
}workmodel_config_t;




//================================================camera isp api==================================================

IM_RET camisp_init(camdev_config_t *config);
IM_RET camisp_deinit(void);
IM_RET camisp_ioctl(IM_INT32 cmd, void *p);
IM_RET camisp_suspend(void);
IM_RET camisp_resume(void);
IM_RET camisp_wait_event(void);
IM_RET camisp_get_event(workmodel_event_t *evt);


#endif	// __CAMISP_API_H__


