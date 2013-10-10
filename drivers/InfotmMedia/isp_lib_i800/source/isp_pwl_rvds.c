/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_pwl_rvds.c
--
--  Description :
--		isp rvds platform wrapper layer.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#include <imapx_system.h>
#include <cmnlib.h>
#include <InfotmMedia.h>
#include <interrupt.h>
#include <rtl_system.h>
#include <rtl_trace.h>
#include "isp_pwl.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"PWL_I:"
#define WARNHEAD	"PWL_W:"
#define ERRHEAD		"PWL_E:"
#define TIPHEAD		"PWL_T:"

#define INTERRUPT_USED	1

typedef struct{
	IM_UINT32 module;
	IM_UINT32 regBase;
	rtl_trace_callback_t rt;
}PWL_CONTEXT;

IM_UINT32 gIspWait = 0;


#define ISP_GLB_INTS	(0x18)
#define ISPGLBINTS_SYNC	(1<<0)
#define ISPGLBINTS_Y	(1<<1)
#define ISPGLBINTS_U	(1<<2)
#define ISPGLBINTS_V	(1<<3)
#define ISPGLBINTS_BDC	(1<<4)

#define ISP_GLB_INTM	(0x1C)
#define ISPGLBINTM_SYNC	(1<<0)
#define ISPGLBINTM_Y	(1<<1)
#define ISPGLBINTM_U	(1<<2)
#define ISPGLBINTM_V	(1<<3)
#define ISPGLBINTM_BDC	(1<<4)

//use imapx200(arm11) base addr
#define IMAPX200_ADDR	0


#if IMAPX200_ADDR
#define ISP_INTR		54//IMAPX200_ISP
#define ISP_REG_BASE	0x20f00000
#else
#define ISP_INTR		ISP_INT_ID
#define ISP_REG_BASE	ISP_BASE_ADDR
#endif

static IM_UINT32 gCount = 0;


#if defined(_IMAPX_ARM1136JFS_)
__irq
#endif
void isp_isr(void)
{

	IM_INT32 status;
	gCount++;

	//IM_TIPMSG((IM_STR("interrupt ........")));
	//intr_mask(ISP_INTR);
	
	status = *(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS);
	
	gIspWait = 0;
	if(status & ISPGLBINTS_SYNC){
		gIspWait |= ISPPWL_ISP_INTR_DMA_SYNC;
	}
	if(status & ISPGLBINTS_Y){
		gIspWait |= ISPPWL_ISP_INTR_DMA_Y;
	}
	if(status & ISPGLBINTS_U){
		gIspWait |= ISPPWL_ISP_INTR_DMA_U;
	}
	if(status & ISPGLBINTS_V){
		gIspWait |= ISPPWL_ISP_INTR_DMA_V;
	}
	if(status & ISPGLBINTS_BDC){
		gIspWait |= ISPPWL_ISP_INTR_DMA_BDC;
	}

	*(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS) = status;

	//intr_clear(ISP_INTR);
	//intr_unmask(ISP_INTR);	
}

// ============================================================================

IM_RET isppwl_clear_intr(IN ISPPWLCTX pwl)
{
	IM_INT32 status;	
	status = *(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS);
	*(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS) = status;

	return IM_RET_OK;
}

IM_RET isppwl_init(IN IM_UINT32 module, OUT ISPPWLCTX *pwl)
{
	PWL_CONTEXT *lpwl;
	IM_UINT32 tmp;

	IM_INFOMSG((IM_STR("%s(module=%d)"), IM_STR(_IM_FUNC_), module));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = mm_malloc(sizeof(PWL_CONTEXT), 0);
	if(lpwl == IM_NULL){
		IM_ERRMSG((IM_STR("mm_malloc() failed")));
		return IM_RET_NOMEMORY;
	}

	isppwl_memset((ISPPWLCTX)lpwl, (void *)lpwl, 0, sizeof(PWL_CONTEXT));

	lpwl->module = ISPPWL_MODULE_ISP;
	lpwl->regBase = ISP_REG_BASE;

	//reset gpu(isp) system control
	tmp = *((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x21c));
	tmp |= (1<<4);
	*((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x21c)) = tmp;

	tmp = *((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x21c));
	tmp &= ~(1<<4);
	*((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x21c)) = tmp;	

	//enable gpu(isp) system control
	tmp = *((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x218));
	tmp &= ~(1<<4);
	*((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x218)) = tmp;

	//set interrupt mask, only sync interrupt enable
	tmp = *(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTM);
	tmp |= (ISPGLBINTM_Y | ISPGLBINTM_U | ISPGLBINTM_V |ISPGLBINTM_BDC);
	tmp &= (~ISPGLBINTM_SYNC);
	*(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTM) = tmp;
	
	//
	intr_register(ISP_INTR, (INTR_ISR)isp_isr, 0);
	//intr_clear(ISP_INT_ID);
#if INTERRUPT_USED
	intr_enable(ISP_INTR);
#endif
	gIspWait = 0;

	*pwl = (ISPPWLCTX)lpwl;
	return IM_RET_OK;
}

IM_RET isppwl_deinit(IN ISPPWLCTX pwl)
{
	IM_UINT32 tmp;
	PWL_CONTEXT *lpwl = (PWL_CONTEXT *)pwl;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(lpwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	IM_INFOMSG((IM_STR("isppwl_deinit(module=%d)"), lpwl->module));
	mm_free((void *)lpwl);

	intr_disable(ISP_INTR);

	//disable
	tmp = *((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x218));
	tmp &= ~(1<<4);
	*((volatile IM_UINT32 *)(PERIPHERAL_BASE_ADDR + 0x218)) = tmp;

	return IM_RET_OK;
}

IM_RET isppwl_alloc_linear_memory(IN ISPPWLCTX pwl, OUT IM_Buffer *buffer, IN IM_UINT32 flag)
{
	IM_INT32 l1 = 0;
	void *p = (void *)0;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	if((pwl == IM_NULL) || (buffer == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	if(flag & ISPPWL_BUFFER_ALIGN_16BYTES){
		l1 |= MM_ALLOC_ALIGN_32BYTE;
	}else if(flag & ISPPWL_BUFFER_ALIGN_8BYTES){
		l1 |= MM_ALLOC_ALIGN_8BYTE;
	}

	p = mm_module_malloc(MODULE_ISP, buffer->size, l1);
	if(p == IM_NULL){
		IM_ERRMSG((IM_STR("mm_malloc() failed")));
		return IM_RET_NOMEMORY;
	}
	
	buffer->vir_addr = p;
	buffer->phy_addr = (IM_UINT32)p;
	buffer->flag = IM_BUFFER_FLAG_PHY;

	return IM_RET_OK;
}

IM_RET isppwl_free_linear_memory(IN ISPPWLCTX pwl, IN IM_Buffer *buffer)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((pwl == IM_NULL) || (buffer == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	mm_free(buffer->vir_addr);
	return IM_RET_OK;
}

IM_RET isppwl_write_regs(IN ISPPWLCTX pwl, IN ISPPWL_REG_VAL *rv, IN IM_UINT32 num)
{
	IM_INT32 i;
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((pwl == IM_NULL) || (rv == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	for(i=0; i<num; i++){
		ret = isppwl_write_reg(pwl, (rv+i)->offset, (rv+i)->value);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("isppwl_write_reg() failed, ret=%d"), ret));
			return ret;
		}
	}

	return IM_RET_OK;
}

IM_RET isppwl_write_reg(IN ISPPWLCTX pwl, IN IM_UINT32 offset, IN IM_UINT32 value)
{
	PWL_CONTEXT *lpwl;
	//IM_INFOMSG((IM_STR("%s(offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), offset, value));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = (PWL_CONTEXT *)pwl;
	*((volatile IM_UINT32 *)(lpwl->regBase + offset)) = value;

	if(lpwl->rt.write_reg != IM_NULL){
		lpwl->rt.write_reg(lpwl->regBase + offset, value);
	}
		
	return IM_RET_OK;
}

IM_RET isppwl_write_reg_notrace(IN ISPPWLCTX pwl, IN IM_UINT32 offset, IN IM_UINT32 value)
{
	PWL_CONTEXT *lpwl;
	//IM_INFOMSG((IM_STR("%s(offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), offset, value));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = (PWL_CONTEXT *)pwl;
	*((volatile IM_UINT32 *)(lpwl->regBase + offset)) = value;
	
	return IM_RET_OK;
}

IM_RET isppwl_write_regbit(IN ISPPWLCTX pwl, IN IM_UINT32 reg_ofst,
		IN IM_UINT32 bit_ofst, IN IM_UINT32 bit_width, IN IM_UINT32 value)
{	
	PWL_CONTEXT *lpwl;
	IM_UINT32 lval;
	IM_INFOMSG((IM_STR("%s(reg_ofst=%d, bit_ofst=%d, bit_width=%d, value=0x%x)"), 
		IM_STR(_IM_FUNC_), reg_ofst, bit_ofst, bit_width, value));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	if(value >= (1<<bit_width)){
		IM_ERRMSG((IM_STR("Invalid parameter, value is overflow, bit_width=%d, value=0x%x"), bit_width, value));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = (PWL_CONTEXT *)pwl;
	lval = *(volatile IM_UINT32 *)(lpwl->regBase + reg_ofst);
	lval &= ~(((1 << bit_width) - 1) << bit_ofst);
	lval |= (value << bit_ofst);
	*((volatile IM_UINT32 *)(lpwl->regBase + reg_ofst)) = lval;
	return IM_RET_OK;
}

IM_RET isppwl_read_regs(IN ISPPWLCTX pwl, INOUT ISPPWL_REG_VAL *rv, IN IM_UINT32 num)
{	
	IM_INT32 i;
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((pwl == IM_NULL) || (rv == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	for(i=0; i<num; i++){
		ret = isppwl_read_reg(pwl, (rv+i)->offset, &((rv+i)->value));
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("isppwl_read_reg() failed, ret=%d"), ret));
			return ret;
		}
	}

	return IM_RET_OK;
}

IM_RET isppwl_read_reg(IN ISPPWLCTX pwl, IN IM_UINT32 offset, OUT IM_UINT32 *value)
{
	PWL_CONTEXT *lpwl;
	//IM_INFOMSG((IM_STR("%s(offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), offset, value));
	if((pwl == IM_NULL) || (value == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	lpwl = (PWL_CONTEXT *)pwl;
	*value = *(volatile IM_UINT32 *)(lpwl->regBase + offset);

	if(lpwl->rt.read_reg != IM_NULL){
		lpwl->rt.read_reg(lpwl->regBase  + offset, *value);
	}
	return IM_RET_OK;
}

IM_RET isppwl_read_reg_notrace(IN ISPPWLCTX pwl, IN IM_UINT32 offset, OUT IM_UINT32 *value)
{
	PWL_CONTEXT *lpwl;
	//IM_INFOMSG((IM_STR("%s(offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), offset, value));
	if((pwl == IM_NULL) || (value == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	lpwl = (PWL_CONTEXT *)pwl;
	*value = *(volatile IM_UINT32 *)(lpwl->regBase + offset);
	
	return IM_RET_OK;
}

//static int malloc_num = 0;
void * isppwl_malloc(IN ISPPWLCTX pwl, IM_UINT32 size)
{
	IM_INFOMSG((IM_STR("%s(size=%d)"), IM_STR(_IM_FUNC_), size));
	//malloc_num++;
	//IM_TIPMSG((IM_STR("malloc num: %d)"), malloc_num));
	return mm_malloc(size, 0);
}

//static int free_num = 0;
void isppwl_free(IN ISPPWLCTX pwl, void *mem)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	//free_num++;
	//IM_TIPMSG((IM_STR("free num: %d)"), free_num));
	mm_free(mem);
}

void * isppwl_memcpy(IN ISPPWLCTX pwl, void *dst, void *src, IM_UINT32 size)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mm_memcpy(dst, src, size);
	return dst;
}

void * isppwl_memset(IN ISPPWLCTX pwl, void *dst, IM_CHAR c, IM_UINT32 size)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mm_memset(dst, size, c);
	return dst;
}

IM_RET isppwl_wait_hw_ready(IN ISPPWLCTX pwl, OUT IM_UINT32 *intr, IN IM_INT32 timeout)
{
	PWL_CONTEXT *lpwl;
	IM_INT32 status;
	//int i;
	
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = (PWL_CONTEXT *)pwl;

	if(lpwl->rt.wait_hw_ready != IM_NULL){
		lpwl->rt.wait_hw_ready();
	}

#if INTERRUPT_USED	//use interrupt
	while(gIspWait == 0);
	
	if(intr != IM_NULL){
		*intr = gIspWait;
		//IM_TIPMSG((IM_STR("gIspWait=0x%x"), gIspWait));
	}
	gIspWait = 0;
#else	//use inquire
	do{
		status = *(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS);
		
		if(status & ISPGLBINTS_SYNC){
			gIspWait |= ISPPWL_ISP_INTR_DMA_SYNC;
		}
		if(status & ISPGLBINTS_Y){
			gIspWait |= ISPPWL_ISP_INTR_DMA_Y;
		}
		if(status & ISPGLBINTS_U){
			gIspWait |= ISPPWL_ISP_INTR_DMA_U;
		}
		if(status & ISPGLBINTS_V){
			gIspWait |= ISPPWL_ISP_INTR_DMA_V;
		}
		if(status & ISPGLBINTS_BDC){
			gIspWait |= ISPPWL_ISP_INTR_DMA_BDC;
		}
	}while(!(gIspWait&ISPPWL_ISP_INTR_DMA_SYNC));

	if(intr != IM_NULL){
		*intr = gIspWait;
		//IM_TIPMSG((IM_STR("gIspWait=0x%x"), gIspWait));
	}
	gIspWait = 0;
	*(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS) = status;
	
#endif

	/*i = 10000;
	while(i-->0);
	*intr = ISPPWL_ISP_INTR_DMA_SYNC;*/
	
	return IM_RET_OK;
}

IM_RET isppwl_sync(IN ISPPWLCTX pwl, IN IM_INT32 timeout)
{
	IM_INT32 status;
	PWL_CONTEXT *lpwl;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl = (PWL_CONTEXT *)pwl;
	
	if(lpwl->rt.wait_hw_ready != IM_NULL){
		lpwl->rt.wait_hw_ready();
	}


#if INTERRUPT_USED	//use interrupt
	while(gIspWait == 0);

	gIspWait = 0;
#else	//use inquire
	do{
		status = *(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS);
		
		if(status & ISPGLBINTS_SYNC){
			gIspWait |= ISPPWL_ISP_INTR_DMA_SYNC;
		}
		if(status & ISPGLBINTS_Y){
			gIspWait |= ISPPWL_ISP_INTR_DMA_Y;
		}
		if(status & ISPGLBINTS_U){
			gIspWait |= ISPPWL_ISP_INTR_DMA_U;
		}
		if(status & ISPGLBINTS_V){
			gIspWait |= ISPPWL_ISP_INTR_DMA_V;
		}
		if(status & ISPGLBINTS_BDC){
			gIspWait |= ISPPWL_ISP_INTR_DMA_BDC;
		}
	}while(!(gIspWait&ISPPWL_ISP_INTR_DMA_SYNC));

	gIspWait = 0;
	*(volatile IM_UINT32 *)(ISP_REG_BASE + ISP_GLB_INTS) = status;
	
#endif
	
	return IM_RET_OK;
}


IM_RET isppwl_ioctl(IN ISPPWLCTX pwl, IN IM_UINT32 code, 
		IN void *inData, IN IM_UINT32 inLen,
		OUT void *outData, IN IM_UINT32 outLen, OUT IM_UINT32 *actualOutLen)
{
	PWL_CONTEXT *lpwl;
	lpwl = (PWL_CONTEXT *)pwl;
	IM_INFOMSG((IM_STR("%s(module=%d, code=%d)"), IM_STR(_IM_FUNC_), lpwl->module, code));
	if(pwl == IM_NULL){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(code == ISPPWL_ISP_IOCTRL_SET_PAL){
		mm_memcpy((void *)(ISP_REG_BASE + 0x400), inData, inLen);
	}	

	return IM_RET_OK;
}

IM_RET isppwl_register_rt(IN ISPPWLCTX pwl, IN void *rt)
{
	PWL_CONTEXT *lpwl = (PWL_CONTEXT *)pwl;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((pwl == IM_NULL) || (rt == IM_NULL)){
		IM_ERRMSG((IM_STR("Invalid parameter, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}

	lpwl->rt.read_reg = ((rtl_trace_callback_t *)rt)->read_reg;
	lpwl->rt.write_reg = ((rtl_trace_callback_t *)rt)->write_reg;
	lpwl->rt.enable_hw = ((rtl_trace_callback_t *)rt)->enable_hw;
	lpwl->rt.wait_hw_ready = ((rtl_trace_callback_t *)rt)->wait_hw_ready;

	return IM_RET_OK;
}

