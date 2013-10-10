/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: camif_pwl_linux.c
--
--  Description :
--		camif linux platform wrapper layer.
--
--	Author:
--     Jimmy Shu   <jimmy.shu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	Jimmy@2012/10/16: first commit.
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
#include "camif_pwl.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMIFPWL_I:"
#define WARNHEAD	"CAMIFPWL_W:"
#define ERRHEAD		"CAMIFPWL_E:"
#define TIPHEAD		"CAMIFPWL_T:"

#define CAMIF_BASE_ADDR	(0x26000000)
#define IRQ_CAMIF			(144)

#define CAMIF_GLB_INTS	(0xA4)

#define PR_DMA_SUCCESS	(1<<2)
#define CO_DMA_SUCCESS	(1<<1)


//camif module clock
#define CAMIF_CLOCKGATE_ADDRESS	(0x21e34004)
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
	func2_irq_callback_t irq_callback;
	struct clk 			*busclk;
	struct clk 			*osdclk;
	IM_UINT32			busclkFrq;
	IM_UINT32			osdclkFrq;
	//struct clk 			*csiclk;
	//struct clk 			*campclk;
	//struct clk 			*camoclk;
}camifpwl_context_t;

static wait_queue_head_t wq_camif;
IM_UINT32 gCamifWait = 0;

static camifpwl_context_t *gPwl = IM_NULL;

IM_RET camifpwl_register_irq(int irq, func2_irq_callback_t irq_callback)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(gPwl != IM_NULL);
	gPwl->irq_callback = irq_callback;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

//camif interrupt function
static irqreturn_t camifpwl_irq_handle(int irq, void *p)
{
	IM_INT32 status = 0;

	camifpwl_read_reg(CAMIF_GLB_INTS, &status);
    if (status & PR_DMA_SUCCESS) {
		gPwl->irq_callback();
    }
    if (status & CO_DMA_SUCCESS) {
    }
	camifpwl_write_reg(CAMIF_GLB_INTS, status);

	return IRQ_HANDLED;
}

IM_RET camif_clock_enable(void)
{
	IM_UINT32 temp;
	IM_UINT32 res;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(gPwl != IM_NULL);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	//open all camif clock gate
	temp = __raw_readl(IO_ADDRESS(CAMIF_CLOCKGATE_ADDRESS));
	temp |= 0xf;
	__raw_writel(temp, IO_ADDRESS(CAMIF_CLOCKGATE_ADDRESS));
	
	//enable bus clock
	gPwl->busclk = clk_get(NULL, "isp");
	if(gPwl->busclk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get camif busclk failed!")));
		goto Fail;
	}

	res = clk_set_rate(gPwl->busclk, 201000); //201MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("camif bus clk set rate failed! ")));
		goto Fail;
	}

	gPwl->busclkFrq = (IM_UINT32)clk_get_rate(gPwl->busclk);

	if(clk_enable(gPwl->busclk) != 0)
	{
		IM_ERRMSG((IM_STR("camif busclk enable failed!")));
		goto Fail;
	}


	IM_INFOMSG((IM_STR("camif busclk frq= %d!"), gPwl->busclkFrq));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;

Fail:
	if(gPwl->busclk != IM_NULL)
	{
		clk_disable(gPwl->busclk);
		clk_put(gPwl->busclk);
		gPwl->busclk = IM_NULL;
	}


	return IM_RET_FAILED;
}

IM_RET camif_clock_disable(void)
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

	temp = __raw_readl(IO_ADDRESS(CAMIF_CLOCKGATE_ADDRESS));
	temp &= ~(0xf);
	__raw_writel(temp, IO_ADDRESS(CAMIF_CLOCKGATE_ADDRESS));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camifpwl_init(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
    IM_ASSERT(gPwl == IM_NULL);
	
    gPwl = camifpwl_malloc(sizeof(camifpwl_context_t));
	if(gPwl == IM_NULL){
		IM_ERRMSG((IM_STR("@camifpwl_init(), camifpwl_malloc(gPwl) failed!")));
		return IM_RET_FAILED;
	}
	camifpwl_memset((void *)gPwl, 0, sizeof(camifpwl_context_t));

	//
	mutex_init(&gPwl->accessLock);

	/*if(camif_clock_enable() != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("@camifpwl_init(), clock enable failed!")));
	}*/

	gPwl->regBasePhy = CAMIF_BASE_ADDR;
	gPwl->regSize = 0x1000;
	gPwl->regBaseVir = (IM_UINT32 *)ioremap_nocache(gPwl->regBasePhy, gPwl->regSize);
	if(gPwl->regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("@camifpwl_init(), ioremap(ISP) regBaseVir failed!")));
		goto Fail;
	}

	/* initualize wait queue */
	init_waitqueue_head(&wq_camif);

	//register camif irq
	if(request_irq(IRQ_CAMIF, camifpwl_irq_handle, IRQF_DISABLED, "camif_irq", (void *)gPwl) != 0)    {
		IM_ERRMSG((IM_STR("request_irq() failed")));
		goto Fail;
	}

	gPwl->irqRegister = IM_TRUE;
	gCamifWait = 0;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
Fail:
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	if(gPwl->irqRegister == IM_TRUE){
		free_irq(IRQ_CAMIF, (void *)gPwl);
		gPwl->irqRegister = IM_FALSE;
	}

	mutex_destroy(&gPwl->accessLock);
	camifpwl_free((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET camifpwl_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
    IM_ASSERT(gPwl != IM_NULL);
	
    if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	//camif_clock_disable();

	mutex_destroy(&gPwl->accessLock);
	//unregister camif irq
	if(gPwl->irqRegister == IM_TRUE){
		free_irq(IRQ_CAMIF, (void *)gPwl);
		gPwl->irqRegister = IM_FALSE;
	}
	camifpwl_free((void *)gPwl);
	gPwl = IM_NULL;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET camifpwl_write_reg(IM_UINT32 offset, IM_UINT32 value)
{
	IM_INFOMSG((IM_STR("%s(base_addr=0x%x, offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), (IM_UINT32)gPwl->regBaseVir, offset, value));
	writel(value, (IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}

IM_RET camifpwl_write_regs(camifpwl_reg_val_t *rv, IM_UINT32 num)
{
	return IM_RET_OK;
}

IM_RET camifpwl_write_regbit(IM_UINT32 reg_ofst, IM_UINT32 bit_ofst, IM_UINT32 bit_width, IM_UINT32 value)
{
	return IM_RET_OK;
}

IM_RET camifpwl_read_reg(IM_UINT32 offset, IM_UINT32 *value)
{
	*value = readl((IM_UINT32)gPwl->regBaseVir + offset);
	return IM_RET_OK;
}

IM_RET camifpwl_read_regs(camifpwl_reg_val_t *rv, IM_UINT32 num)
{
	return IM_RET_OK;
}

IM_RET camifpwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
#if 1//use interrupt
	wait_event_interruptible(wq_camif, (gCamifWait & CAMIFPWL_INTR_DMA_SYNC) != 0);
	IM_INFOMSG((IM_STR("%s(wait ok.....)"), IM_STR(_IM_FUNC_)));
	if(intr != IM_NULL){
		*intr = gCamifWait;
	}
	gCamifWait = 0;
#else//use inquire
	IM_INT32 status = 0;

	if(intr != IM_NULL){
		*intr = gCamifWait;
	}
	gCamifWait = 0;
	//camifpwl_write_reg(CAMIF_GLB_INTS, status);
#endif

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET camifpwl_sync(/*IM_UINT32 *intr*/IM_INT32 timeout)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	wait_event_interruptible(wq_camif, (gCamifWait & CAMIFPWL_INTR_DMA_SYNC) != 0);
	/*if(intr != IM_NULL){
		*intr = gCamifWait;
	}*/
	gCamifWait = 0;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}


IM_RET camifpwl_alloc_linear_memory(IM_Buffer *buffer, IM_UINT32 flag)
{
	return IM_RET_OK;
}


IM_RET camifpwl_free_linear_memory(IN IM_Buffer *buffer)
{
	return IM_RET_OK;
}


void *camifpwl_malloc(IM_UINT32 size)
{
	return kmalloc(size,GFP_KERNEL);
}

void camifpwl_free(void *mem)
{
	if(mem != IM_NULL) kfree(mem);
}

void camifpwl_memcpy(void *dst, void *src, IM_UINT32 size)
{
	memcpy(dst, src, size);
}

void camifpwl_memset(void *dst, IM_CHAR c, IM_UINT32 size)
{
	memset(dst, c, size);
}




IM_RET camifpwl_lock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_lock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET camifpwl_unlock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_unlock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET camifpwl_lock_init(camifpwl_lock_t *lck)
{
	struct mutex *mtx = (struct mutex *)camifpwl_malloc(sizeof(struct mutex));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(mtx != IM_NULL);
	mutex_init(mtx);
	*lck = (camifpwl_lock_t)mtx;
	return IM_RET_OK;
}

IM_RET camifpwl_lock_deinit(camifpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_destroy((struct mutex *)lck);
	camifpwl_free((void *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_lock(camifpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_lock((struct mutex *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_unlock(camifpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_unlock((struct mutex *)lck);
	return IM_RET_OK;
}

//spin lock
IM_RET camifpwl_spinlock_init(camifpwl_spinlock_t *lck)
{
	spinlock_t *spLock = (spinlock_t *)camifpwl_malloc(sizeof(spinlock_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(spLock != IM_NULL);
	spin_lock_init(spLock);
	*lck = (camifpwl_spinlock_t)spLock;
	return IM_RET_OK;
}

IM_RET camifpwl_spinlock_deinit(camifpwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	camifpwl_free((void *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_spinlock(camifpwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_lock((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_spinunlock(camifpwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_unlock((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_spinlock_irq(camifpwl_spinlock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	spin_lock_irq((spinlock_t *)lck);
	return IM_RET_OK;
}

IM_RET camifpwl_spinunlock_irq(camifpwl_spinlock_t lck)
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

IM_RET camifpwl_sig_init(camifpwl_signal_t *sig, IM_BOOL manualReset)
{
	signal_t *signal = (signal_t *)camifpwl_malloc(sizeof(signal_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(signal != IM_NULL);
	signal->manualReset = manualReset;
	sema_init(&signal->sem, 0);
	*sig = (camifpwl_signal_t)signal;
	return IM_RET_OK;
}

IM_RET camifpwl_sig_deinit(camifpwl_signal_t sig)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	camifpwl_free((void *)sig);
	return IM_RET_OK;
}

IM_RET camifpwl_sig_set(camifpwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	up(&signal->sem);
	return IM_RET_OK;
}

IM_RET camifpwl_sig_reset(camifpwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	while(down_trylock(&signal->sem) == 0);
	return IM_RET_OK;
}

IM_RET camifpwl_sig_wait(camifpwl_signal_t sig, camifpwl_lock_t *lck, IM_INT32 timeout)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	if(lck != IM_NULL){		
		camifpwl_unlock(*lck);
	}

	if(timeout == -1){
		if(down_interruptible(&signal->sem) != 0){
			if(lck != IM_NULL){
				camifpwl_lock(*lck);
			}
			return IM_RET_FAILED;
		}
	}else{
		if(down_timeout(&signal->sem, (long)timeout * HZ / 1000000) == -ETIME){
			if(lck != IM_NULL){
				camifpwl_lock(*lck);
			}
			return IM_RET_TIMEOUT;
		}
	}
	if(signal->manualReset == IM_TRUE){
		up(&signal->sem);
	}
	if(lck != IM_NULL){
		camifpwl_lock(*lck);
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

typedef struct {
	struct work_struct work;
	struct workqueue_struct *wq;
	void *data;
	camifpwl_func_thread_entry_t func;
}thread_t;

static void work_handle(struct work_struct *p)
{
	thread_t *thrd = container_of(p, thread_t, work);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd->func(thrd->data);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}

IM_RET camifpwl_thread_init(camifpwl_thread_t *thread, camifpwl_func_thread_entry_t func, void *data)
{
	int ret = 1;
	thread_t *thrd = IM_NULL;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd = (thread_t *)camifpwl_malloc(sizeof(thread_t));
	if(thrd == IM_NULL){
		IM_ERRMSG((IM_STR("@camifpwl_thread_init(), camifpwl_malloc(thread_t) failed!")));
		return IM_RET_FAILED;
	}

	thrd->wq = create_singlethread_workqueue("camifpwl_wq");
	if(thrd->wq == NULL){
		IM_ERRMSG((IM_STR("@camifpwl_thread_init(), create_singlethread_workqueue() failed!")));
		camifpwl_free((void *)thrd);
		return IM_RET_FAILED;
	}

	thrd->data = data;
	thrd->func = func;
	INIT_WORK(&thrd->work, work_handle);

	//INIT_WORK(&thrd->work, func);
	ret = queue_work(thrd->wq, &thrd->work);
	if(ret != 1){
		destroy_workqueue(thrd->wq);
		camifpwl_free((void *)thrd);
		return IM_RET_FAILED;
	}
	
	*thread = (camifpwl_thread_t)thrd;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET camifpwl_thread_deinit(camifpwl_thread_t thread)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(thread != IM_NULL);
	destroy_workqueue(((thread_t *)thread)->wq);
	camifpwl_free((void *)thread);
	return IM_RET_OK;
}

void camifpwl_msleep(IM_UINT32 ms)
{
	msleep(ms);
}

