/*
 * ie_lib.h
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
 * Author:
 *	Sam<weize_ye@infotm.com>.
 *
 * Revision History:
 ** 1.0.1	 Sam@2012/5/11 :  first commit : 
 ** 1.0.5	 Sam@2012/6/05 :  add acc & acm support
 **
*****************************************************************************/


#ifndef __IE_LIB_H_
#define __IE_LIB_H_

/*************** acc&acm&gamma   ***************************/
#define ENH_CFG_BASE_OFFSET 	(0x5800)

#define ENH_FSR 			(ENH_CFG_BASE_OFFSET + 0x0000)
#define ENH_IECR 			(ENH_CFG_BASE_OFFSET + 0x0004)
#define ENH_ACCCOER 		(ENH_CFG_BASE_OFFSET + 0x0080)
#define ENH_ACCLUTR			(ENH_CFG_BASE_OFFSET + 0x0084)
#define ENH_ACMTHER     	(ENH_CFG_BASE_OFFSET + 0x008C)
#define ENH_ACMCOEP     	(ENH_CFG_BASE_OFFSET + 0x0090)
#define ENH_ACMCOER     	(ENH_CFG_BASE_OFFSET + 0x0094)
#define ENH_ACMCOEG     	(ENH_CFG_BASE_OFFSET + 0x0098)
#define ENH_ACMCOEB     	(ENH_CFG_BASE_OFFSET + 0x009C)
#define ENH_RGAMMA 			(0x5900)
#define ENH_GGAMMA 			(0x5980)
#define ENH_BGAMMA 			(0x5A00)

#define ENH_IECR_GAROUND		20
#define ENH_IECR_GAPASSBY		19
#define ENH_IECR_ACMROUND		18
#define ENH_IECR_ACMPASSBY 		17
#define ENH_IECR_ACCREADY		16
#define ENH_IECR_ACCROUND		15
#define ENH_IECR_ACCPASSBY		14
#define ENH_IECR_HISTNFMS		9
#define ENH_IECR_HISTROUND		8
#define ENH_IECR_HISTPASSBY		7
#define ENH_IECR_Y2RROUND		6
#define ENH_IECR_Y2RPASSBY		5
#define ENH_IECR_R2YROUND		4
#define ENH_IECR_R2YPASSBY		3
#define ENH_IECR_EEDNOEN		2
#define ENH_IECR_EEROUND		1
#define ENH_IECR_EEPASSBY		0

#define ENH_ACCCOER_ACCCOE 		16

#define ENH_ACMCOEST			0
#define ENH_ACCLUTR_ACCLUTA 	16
#define ENH_FSR_VRES			16
#define ENH_FSR_HRES			0

#define ENH_IE_ACC_LUTB_LENGTH 1792
#define ENH_IE_GAMMA_COEF_LENGTH 32

/******************** dither  ***********************/
#define DIT_CFG_BASE_OFFSET		(0x5C00)

#define DIT_FSR				(DIT_CFG_BASE_OFFSET + 0x0000)
#define DIT_DCR				(DIT_CFG_BASE_OFFSET + 0x0004)

//DIT_FSR
#define DIT_FSR_VRES			(16)		//[27:16]
#define DIT_FSR_HRES			(0)		//[11:0]
        
//DIT_DCR
#define DIT_DCR_RNB				(9)		//[11:9]
#define DIT_DCR_GNB				(6)		//[8:6]
#define DIT_DCR_BNB				(3)		//[5:3]
#define DIT_DCR_PASSBY			(2)		//[2]
#define DIT_DCR_TEMPO			(0)		//[0]
        
/*********************************************************/

IM_RET ielib_init(ie_handle_t *handle, IM_INT32 idsx);
IM_RET ielib_deinit(ie_handle_t handle);
IM_RET ielib_set_width_height(ie_handle_t handle, IM_INT32 width, IM_INT32 height);
IM_RET ielib_acc_set(ie_handle_t handle, IM_UINT32 acc);
IM_RET ielib_acc_get_status(ie_handle_t handle, IM_UINT32 *acc);
IM_RET ielib_acm_set(ie_handle_t handle, IM_UINT32 acm);
IM_RET ielib_acm_get_status(ie_handle_t handle, IM_UINT32 *acm);
IM_RET ielib_gamma_set(ie_handle_t handle, IM_UINT32 gamma);
IM_RET ielib_gamma_get_status(ie_handle_t handle, IM_UINT32 *gamma);
IM_RET ielib_dither_set(ie_handle_t handle, IM_INT32 portType, IM_BOOL en);
IM_RET ielib_dither_get_status(ie_handle_t handle, IM_BOOL *val);
IM_RET ielib_suspend(ie_handle_t handle);
IM_RET ielib_resume(ie_handle_t handle);


#endif // __IE_LIB_H_
