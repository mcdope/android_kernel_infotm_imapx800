/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: csi_pwl_linux.c
--
--  Description :
--		csi linux platform wrapper layer.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/09/05: first commit.
--
------------------------------------------------------------------------------*/

#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/clk.h>

#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
#include <mach/pad.h>
#include <mach/io.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#endif

#include <InfotmMedia.h>
#include "csi_pwl.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CSI_PWL_I:"
#define WARNHEAD	"CSI_PWL_W:"
#define ERRHEAD		"CSI_PWL_E:"
#define TIPHEAD		"CSI_PWL_T:"

#define CSI_BASE_ADDR	(0x26100000)
#define CSI_DMA_BASE_ADDR	(0x26100400)


typedef struct{
	IM_UINT32			regBasePhy;
	IM_UINT32			*regBaseVir;
	IM_INT32			regSize;
	//struct clk 			*csiclk;
}csipwl_context_t;

static csipwl_context_t *gPwl = IM_NULL;

IM_RET csipwl_init(void)
{
	IM_UINT32 tmp = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl == IM_NULL);
	gPwl = kmalloc(sizeof(csipwl_context_t), GFP_KERNEL);
	if(gPwl == IM_NULL){
		IM_ERRMSG((IM_STR("@csipwl_init(), kmalloc(gPwl) failed!")));
		return IM_RET_FAILED;
	}
	memset((void *)gPwl, 0, sizeof(csipwl_context_t));


	gPwl->regBasePhy = CSI_BASE_ADDR;
	gPwl->regSize = 0x100;
	gPwl->regBaseVir = (IM_UINT32 *)ioremap_nocache(gPwl->regBasePhy, gPwl->regSize);
	if(gPwl->regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("@csipwl_init(), ioremap(ISP) regBaseVir failed!")));
		goto Fail;
	}

	IM_INFOMSG((IM_STR("%s(gPwl->regBaseVir=0x%x)--"), IM_STR(_IM_FUNC_), (IM_UINT32)(gPwl->regBaseVir)));
	return IM_RET_OK;
Fail:
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	kfree((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET csipwl_deinit(void)
{
	IM_UINT32 tmp;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl != IM_NULL);
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	kfree((void *)gPwl);
	gPwl = IM_NULL;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET csipwl_write_reg(IM_UINT32 offset, IM_UINT32 value)
{
	IM_INFOMSG((IM_STR("%s(base_addr=0x%x, offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), (IM_UINT32)gPwl->regBaseVir, offset, value));
	writel(value, (IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}


IM_RET csipwl_read_reg(IM_UINT32 offset, IM_UINT32 *value)
{
	IM_INFOMSG((IM_STR("%s(base_addr=0x%x, offset=0x%x)"), IM_STR(_IM_FUNC_), (IM_UINT32)gPwl->regBaseVir, offset));
	*value = readl((IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}
