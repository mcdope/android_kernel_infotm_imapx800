/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camsen_pwl.h
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/04/05: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/16: changed sensor pmu manager to this layer.
--
------------------------------------------------------------------------------*/

#ifndef __CAMSEN_PWL_H__
#define __CAMSEN_PWL_H__

#if defined(KERNEL_LINUX)
#include <mach/irqs.h>
#endif

typedef void * pwl_handle_t;

#define PMU_CHANNEL_MAX		3
//camera sensor(power manager unit)pmu struct
typedef struct{
	IM_CHAR pwName[128];	//power name need to provide
	IM_INT32 volt;		//volt need to set, unit is mv	
}camsenpwl_pmu_channel_t;

typedef struct{
	IM_UINT32 useChs;	//real pmu channels number used
	camsenpwl_pmu_channel_t channel[PMU_CHANNEL_MAX];
}camsenpwl_pmu_info_t;

//==============================interface======================================
void camsenpwl_set_pmu_handle(void *pmuHandle);
IM_RET camsenpwl_pwdn(IM_CHAR *moduleName, IM_UINT32 facing);

IM_RET camsenpwl_init(pwl_handle_t *pwl, IM_CHAR *name, IM_UINT32 facing, IM_BOOL checkOnly);
IM_RET camsenpwl_deinit(pwl_handle_t pwl);
IM_RET camsenpwl_start(pwl_handle_t pwl);
IM_RET camsenpwl_stop(pwl_handle_t pwl);
IM_RET camsenpwl_get_caps(pwl_handle_t pwl, camsen_caps_t *caps);
IM_RET camsenpwl_get_out_mode(pwl_handle_t pwl, camsen_out_mode_t *outMode);
IM_RET camsenpwl_set_out_mode(pwl_handle_t pwl, camsen_out_mode_t *outMode);
IM_RET camsenpwl_get_property(pwl_handle_t pwl, IM_UINT32 property, void *p);
IM_RET camsenpwl_set_property(pwl_handle_t pwl, IM_UINT32 property, void *p);

IM_RET camsenpwl_clock_enable(pwl_handle_t pwl, IM_UINT32 requestFrq); /*request freq unit:Hz*/
IM_RET camsenpwl_clock_disable(pwl_handle_t pwl);
IM_UINT32 camsenpwl_clock_get_freq(pwl_handle_t pwl);

IM_UINT32 camsenpwl_get_pwdn_padnum(pwl_handle_t pwl);
IM_UINT32 camsenpwl_get_reset_padnum(pwl_handle_t pwl);
IM_UINT32 camsenpwl_get_flash_light_padnum(pwl_handle_t pwl);

IM_INT32 camsenpwl_i2c_read(pwl_handle_t pwl, IM_UINT8 *buf,
		IM_UINT8 *addr, IM_UINT32 size, IM_UINT32 len);
IM_INT32 camsenpwl_i2c_write(pwl_handle_t pwl, IM_UINT8 *buf, IM_UINT32 len);

IM_INT32 camsenpwl_io_set_mode(IM_INT32 index, IM_UINT32 mode);//0:function mode, 1:gpio mode
IM_INT32 camsenpwl_io_set_dir(IM_INT32 index, IM_UINT32 dir);//0:output, 1:input
IM_INT32 camsenpwl_io_set_outdat(IM_INT32 index, IM_UINT32 data);

void *camsenpwl_malloc(IM_UINT32 size);
void camsenpwl_free(void *p);
void camsenpwl_memcpy(void *dst, void *src, IM_UINT32 size);
void camsenpwl_memset(void *dst, IM_INT8 c, IM_UINT32 size);

#endif	// __CAMSEN_PWL_H__

