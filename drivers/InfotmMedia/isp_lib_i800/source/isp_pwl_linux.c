/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_pwl_linux.c
--
--  Description :
--		isp linux platform wrapper layer.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
-- v1.0.2	arsor@2012/07/16: add isp all clock control function.
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
//#include <mach/irqs.h>
//#include <asm/irqs.h>

#include <InfotmMedia.h>
#include "isp_pwl.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"PWL_I:"
#define WARNHEAD	"PWL_W:"
#define ERRHEAD		"PWL_E:"
#define TIPHEAD		"PWL_T:"

#define ISP_BASE_ADDR	(0x26000000)
//#define ISP_BASE_CTR	(0x20000000)
#define IRQ_ISP			(144)

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

//isp module clock
#define ISP_CLOCKGATE_ADDRESS	(0x21e34004)
#define CLKGATE_BUS				0
#define CLKGATE_OSD				1
#define CLKGATE_CSI				2
#define CLKGATE_CAMPIX			3
#define CLKGATE_CAMOUT			4


typedef struct{
	struct mutex		accessLock;
	IM_BOOL				irqRegister;
	IM_UINT32			regBasePhy;
	IM_UINT32			*regBaseVir;
	IM_INT32			regSize;
	IM_UINT32			ctrBasePhy;
	IM_UINT32			*ctrBaseVir;
	func_irq_callback_t irq_callback;
	struct clk 			*busclk;
	struct clk 			*osdclk;
	IM_UINT32			busclkFrq;
	IM_UINT32			osdclkFrq;
	//struct clk 			*csiclk;
	//struct clk 			*campclk;
	//struct clk 			*camoclk;
}isppwl_context_t;

static wait_queue_head_t wq_isp;
IM_UINT32 gIspWait = 0;

static isppwl_context_t *gPwl = IM_NULL;

IM_RET isppwl_register_irq(int irq, func_irq_callback_t irq_callback)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl != IM_NULL);
	gPwl->irq_callback = irq_callback;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

//isp interrupt function
static irqreturn_t isppwl_irq_handle(int irq, void *p)
{
	IM_INT32 status = 0;

	isppwl_read_reg(ISP_GLB_INTS, &status);
	if(status & ISPGLBINTS_Y){
		gIspWait |= ISPPWL_INTR_DMA_Y;
	}
	if(status & ISPGLBINTS_U){
		gIspWait |= ISPPWL_INTR_DMA_U;
	}
	if(status & ISPGLBINTS_V){
		gIspWait |= ISPPWL_INTR_DMA_V;
	}
	if(status & ISPGLBINTS_BDC){
		gIspWait |= ISPPWL_INTR_DMA_BDC;
	}
	if(status & ISPGLBINTS_SYNC){
		gIspWait |= ISPPWL_INTR_DMA_SYNC;
		gPwl->irq_callback();
		//wake_up_interruptible(&wq_isp);
	}

	isppwl_write_reg(ISP_GLB_INTS, status);

	return IRQ_HANDLED;
}

IM_RET clock_enable(void)
{
	IM_UINT32 temp;
	IM_UINT32 res;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(gPwl != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	//open all isp clock gate
	temp = __raw_readl(IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));
	temp |= 0xf;
	__raw_writel(temp, IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));
	
	//enable bus clock
	gPwl->busclk = clk_get(NULL, "isp");
	if(gPwl->busclk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get isp busclk failed!")));
		goto Fail;
	}

	res = clk_set_rate(gPwl->busclk, 201000); //201MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("isp bus clk set rate failed! ")));
		goto Fail;
	}

	gPwl->busclkFrq = (IM_UINT32)clk_get_rate(gPwl->busclk);

	if(clk_enable(gPwl->busclk) != 0)
	{
		IM_ERRMSG((IM_STR("isp busclk enable failed!")));
		goto Fail;
	}

	//enable osd clock
	gPwl->osdclk = clk_get(NULL, "isp-osd");
	if(gPwl->osdclk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get isp osdclk failed!")));
		goto Fail;
	}

	res = clk_set_rate(gPwl->osdclk, 444000); //444MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("isp osd clk set rate failed! ")));
		goto Fail;
	}
	gPwl->osdclkFrq = (IM_UINT32)clk_get_rate(gPwl->osdclk);

	if(clk_enable(gPwl->osdclk) != 0)
	{
		IM_ERRMSG((IM_STR("isp osdclk enable failed!")));
		goto Fail;
	}

	//enable csi clock
	//TODO:
	
	//enable camp clock
	//TODO:
	
	//enable camo clock
	//TODO:

	IM_INFOMSG((IM_STR("isp busclk frq= %d, isp osdclk frq=%d!"), gPwl->busclkFrq, gPwl->osdclkFrq));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;

Fail:
	if(gPwl->busclk != IM_NULL)
	{
		clk_disable(gPwl->busclk);
		clk_put(gPwl->busclk);
		gPwl->busclk = IM_NULL;
	}

	if(gPwl->osdclk != IM_NULL)
	{
		clk_disable(gPwl->osdclk);
		clk_put(gPwl->osdclk);
		gPwl->osdclk = IM_NULL;
	}

	/*if(gPwl->csiclk != IM_NULL)
	{
		clk_disable(gPwl->csiclk);
		clk_put(gPwl->csiclk);
		gPwl->csiclk = IM_NULL;
	}*/

	return IM_RET_FAILED;
}

IM_RET clock_disable(void)
{
	IM_UINT32 temp;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(gPwl != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	//clock disable
	if(gPwl->busclk != IM_NULL)
	{
		clk_disable(gPwl->busclk);
		clk_put(gPwl->busclk);
		gPwl->busclk = IM_NULL;
	}

	if(gPwl->osdclk != IM_NULL)
	{
		clk_disable(gPwl->osdclk);
		clk_put(gPwl->osdclk);
		gPwl->osdclk = IM_NULL;
	}

	/*if(gPwl->csiclk != IM_NULL)
	{
		clk_disable(gPwl->csiclk);
		clk_put(gPwl->csiclk);
		gPwl->csiclk = IM_NULL;
	}*/

	temp = __raw_readl(IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));
	temp &= ~(0xf);
	__raw_writel(temp, IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET isppwl_init(void)
{
	IM_UINT32 tmp = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl == IM_NULL);
	gPwl = isppwl_malloc(sizeof(isppwl_context_t));
	if(gPwl == IM_NULL){
		IM_ERRMSG((IM_STR("@isppwl_init(), isppwl_malloc(gPwl) failed!")));
		return IM_RET_FAILED;
	}
	isppwl_memset((void *)gPwl, 0, sizeof(isppwl_context_t));

	//
	mutex_init(&gPwl->accessLock);

	/*if(clock_enable() != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("@isppwl_init(), clock enable failed!")));
	}*/

	gPwl->regBasePhy = ISP_BASE_ADDR;
	gPwl->regSize = 0x10000;
	gPwl->regBaseVir = (IM_UINT32 *)ioremap_nocache(gPwl->regBasePhy, gPwl->regSize);
	if(gPwl->regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("@isppwl_init(), ioremap(ISP) regBaseVir failed!")));
		goto Fail;
	}

	/* initualize wait queue */
	init_waitqueue_head(&wq_isp);

#if 0	//isp system control, not use now
	gPwl->ctrBasePhy = ISP_BASE_CTR;
	gPwl->ctrBaseVir = (IM_UINT32 *)ioremap_nocache(gPwl->ctrBasePhy, 0x1000);
	if(gPwl->ctrBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("@isppwl_init(), ioremap(ISP) ctrBaseVir failed!")));
		goto Fail;
	}
	//reset isp system control
	tmp = readl((IM_UINT32)gPwl->ctrBaseVir + 0x21c);
	tmp |= (1<<4);
	writel(tmp, (IM_UINT32)gPwl->ctrBaseVir + 0x21c);

	tmp = readl((IM_UINT32)gPwl->ctrBaseVir + 0x21c);
	tmp &= ~(1<<4);
	writel(tmp, (IM_UINT32)gPwl->ctrBaseVir + 0x21c);

	//enable isp system control
	tmp = readl((IM_UINT32)gPwl->ctrBaseVir + 0x218);
	tmp &= ~(1<<4);
	writel(tmp, (IM_UINT32)gPwl->ctrBaseVir + 0x218);
#endif

	//set interrupt mask, only sync interrupt enable
	isppwl_read_reg(ISP_GLB_INTM, &tmp);
	tmp |= (ISPGLBINTM_Y | ISPGLBINTM_U | ISPGLBINTM_V |ISPGLBINTM_BDC);
	tmp &= (~ISPGLBINTM_SYNC);
	isppwl_write_reg(ISP_GLB_INTM, tmp);

	//register isp irq
	if(request_irq(IRQ_ISP, isppwl_irq_handle, IRQF_DISABLED, "isp_irq", (void *)gPwl) != 0){
		IM_ERRMSG((IM_STR("request_irq() failed")));
		goto Fail;
	}

	gPwl->irqRegister = IM_TRUE;
	gIspWait = 0;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
Fail:
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	if(gPwl->irqRegister == IM_TRUE){
		free_irq(IRQ_ISP, (void *)gPwl);
		gPwl->irqRegister = IM_FALSE;
	}

	mutex_destroy(&gPwl->accessLock);
	isppwl_free((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET isppwl_deinit(void)
{
	IM_UINT32 tmp;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl != IM_NULL);
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

#if 0 //isp system control, not use now

	//disable
	tmp = readl((IM_UINT32)gPwl->ctrBaseVir + 0x218);
	//?????????? why disable and enable setting the same value
	tmp &= ~(1<<4);
	writel(tmp, (IM_UINT32)gPwl->ctrBaseVir + 0x218);

	if(gPwl->ctrBaseVir != IM_NULL){
		iounmap((void *)gPwl->ctrBaseVir);
	}
#endif

	//clock_disable();

	mutex_destroy(&gPwl->accessLock);
	//unregister isp irq
	if(gPwl->irqRegister == IM_TRUE){
		free_irq(IRQ_ISP, (void *)gPwl);
		gPwl->irqRegister = IM_FALSE;
	}
	isppwl_free((void *)gPwl);
	gPwl = IM_NULL;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET isppwl_write_reg(IM_UINT32 offset, IM_UINT32 value)
{
	IM_INFOMSG((IM_STR("%s(base_addr=0x%x, offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), (IM_UINT32)gPwl->regBaseVir, offset, value));
	writel(value, (IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}

IM_RET isppwl_write_regs(isppwl_reg_val_t *rv, IM_UINT32 num)
{
	return IM_RET_OK;
}

IM_RET isppwl_write_regbit(IM_UINT32 reg_ofst, IM_UINT32 bit_ofst, IM_UINT32 bit_width, IM_UINT32 value)
{
	return IM_RET_OK;
}

IM_RET isppwl_read_reg(IM_UINT32 offset, IM_UINT32 *value)
{
	*value = readl((IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}

IM_RET isppwl_read_regs(isppwl_reg_val_t *rv, IM_UINT32 num)
{
	return IM_RET_OK;
}

IM_RET isppwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
#if 1//use interrupt
	wait_event_interruptible(wq_isp, (gIspWait & ISPPWL_INTR_DMA_SYNC) != 0);
	IM_INFOMSG((IM_STR("%s(wait ok.....)"), IM_STR(_IM_FUNC_)));
	if(intr != IM_NULL){
		*intr = gIspWait;
	}
	gIspWait = 0;
#else//use inquire
	IM_INT32 status = 0;
	do{
		isppwl_read_reg(ISP_GLB_INTS, &status);
		if(status & ISPGLBINTS_Y){
			gIspWait |= ISPPWL_INTR_DMA_Y;
		}
		if(status & ISPGLBINTS_U){
			gIspWait |= ISPPWL_INTR_DMA_U;
		}
		if(status & ISPGLBINTS_V){
			gIspWait |= ISPPWL_INTR_DMA_V;
		}
		if(status & ISPGLBINTS_BDC){
			gIspWait |= ISPPWL_INTR_DMA_BDC;
		}
		if(status & ISPGLBINTS_SYNC){
			gIspWait |= ISPPWL_INTR_DMA_SYNC;
			IM_INFOMSG((IM_STR("%s(#######sync intr#####)"), IM_STR(_IM_FUNC_)));
		}
	}while(!(gIspWait&ISPPWL_INTR_DMA_SYNC));

	if(intr != IM_NULL){
		*intr = gIspWait;
	}
	gIspWait = 0;
	isppwl_write_reg(ISP_GLB_INTS, status);
#endif

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET isppwl_sync(/*IM_UINT32 *intr*/IM_INT32 timeout)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	wait_event_interruptible(wq_isp, (gIspWait & ISPPWL_INTR_DMA_SYNC) != 0);
	/*if(intr != IM_NULL){
		*intr = gIspWait;
	}*/
	gIspWait = 0;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}


IM_RET isppwl_alloc_linear_memory(IM_Buffer *buffer, IM_UINT32 flag)
{
	return IM_RET_OK;
}


IM_RET isppwl_free_linear_memory(IN IM_Buffer *buffer)
{
	return IM_RET_OK;
}


void *isppwl_malloc(IM_UINT32 size)
{
	return kmalloc(size,GFP_KERNEL);
}

void isppwl_free(void *mem)
{
	if(mem != IM_NULL) kfree(mem);
}

void isppwl_memcpy(void *dst, void *src, IM_UINT32 size)
{
	memcpy(dst, src, size);
}

void isppwl_memset(void *dst, IM_CHAR c, IM_UINT32 size)
{
	memset(dst, c, size);
}




IM_RET isppwl_lock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_lock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET isppwl_unlock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_unlock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET isppwl_lock_init(isppwl_lock_t *lck)
{
	struct mutex *mtx = (struct mutex *)isppwl_malloc(sizeof(struct mutex));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(mtx != IM_NULL);
	mutex_init(mtx);
	*lck = (isppwl_lock_t)mtx;
	return IM_RET_OK;
}

IM_RET isppwl_lock_deinit(isppwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_destroy((struct mutex *)lck);
	isppwl_free((void *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_lock(isppwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_lock((struct mutex *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_unlock(isppwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_unlock((struct mutex *)lck);
	return IM_RET_OK;
}

//spin lock
IM_RET isppwl_spinlock_init(isppwl_spinlock_t *lck)
{
	spinlock_t *spLock = (spinlock_t *)isppwl_malloc(sizeof(spinlock_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(spLock != IM_NULL);
	spin_lock_init(spLock);
	*lck = (isppwl_spinlock_t)spLock;
	return IM_RET_OK;
}

IM_RET isppwl_spinlock_deinit(isppwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	isppwl_free((void *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_spinlock(isppwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_lock((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_spinunlock(isppwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_unlock((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_spinlock_irq(isppwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_lock_irq((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET isppwl_spinunlock_irq(isppwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_unlock_irq((spinlock_t *)lck);
	return IM_RET_OK;
}


typedef struct{
	IM_BOOL manualReset;
	struct semaphore sem;
}signal_t;

IM_RET isppwl_sig_init(isppwl_signal_t *sig, IM_BOOL manualReset)
{
	signal_t *signal = (signal_t *)isppwl_malloc(sizeof(signal_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(signal != IM_NULL);
	signal->manualReset = manualReset;
	sema_init(&signal->sem, 0);
	*sig = (isppwl_signal_t)signal;
	return IM_RET_OK;
}

IM_RET isppwl_sig_deinit(isppwl_signal_t sig)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	isppwl_free((void *)sig);
	return IM_RET_OK;
}

IM_RET isppwl_sig_set(isppwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	up(&signal->sem);
	return IM_RET_OK;
}

IM_RET isppwl_sig_reset(isppwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	while(down_trylock(&signal->sem) == 0);
	return IM_RET_OK;
}

IM_RET isppwl_sig_wait(isppwl_signal_t sig, isppwl_lock_t *lck, IM_INT32 timeout)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	if(lck != IM_NULL){		
		isppwl_unlock(*lck);
	}

	if(timeout == -1){
		if(down_interruptible(&signal->sem) != 0){
			if(lck != IM_NULL){
				isppwl_lock(*lck);
			}
			return IM_RET_FAILED;
		}
	}else{
		if(down_timeout(&signal->sem, (long)timeout * HZ / 1000000) == -ETIME){
			if(lck != IM_NULL){
				isppwl_lock(*lck);
			}
			return IM_RET_TIMEOUT;
		}
	}
	if(signal->manualReset == IM_TRUE){
		up(&signal->sem);
	}
	if(lck != IM_NULL){
		isppwl_lock(*lck);
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

typedef struct {
	struct work_struct work;
	struct workqueue_struct *wq;
	void *data;
	isppwl_func_thread_entry_t func;
}thread_t;

static void work_handle(struct work_struct *p)
{
	thread_t *thrd = container_of(p, thread_t, work);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd->func(thrd->data);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}

IM_RET isppwl_thread_init(isppwl_thread_t *thread, isppwl_func_thread_entry_t func, void *data)
{
	int ret = 1;
	thread_t *thrd = IM_NULL;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd = (thread_t *)isppwl_malloc(sizeof(thread_t));
	if(thrd == IM_NULL){
		IM_ERRMSG((IM_STR("@isppwl_thread_init(), isppwl_malloc(thread_t) failed!")));
		return IM_RET_FAILED;
	}

	thrd->wq = create_singlethread_workqueue("isppwl_wq");
	if(thrd->wq == NULL){
		IM_ERRMSG((IM_STR("@isppwl_thread_init(), create_singlethread_workqueue() failed!")));
		isppwl_free((void *)thrd);
		return IM_RET_FAILED;
	}

	thrd->data = data;
	thrd->func = func;
	INIT_WORK(&thrd->work, work_handle);

	//INIT_WORK(&thrd->work, func);
	ret = queue_work(thrd->wq, &thrd->work);
	if(ret != 1){
		destroy_workqueue(thrd->wq);
		isppwl_free((void *)thrd);
		return IM_RET_FAILED;
	}
	
	*thread = (isppwl_thread_t)thrd;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET isppwl_thread_deinit(isppwl_thread_t thread)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(thread != IM_NULL);
	destroy_workqueue(((thread_t *)thread)->wq);
	isppwl_free((void *)thread);
	return IM_RET_OK;
}

void isppwl_msleep(IM_UINT32 ms)
{
	msleep(ms);
}

