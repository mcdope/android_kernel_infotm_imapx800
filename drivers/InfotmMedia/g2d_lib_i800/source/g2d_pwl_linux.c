/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_pwl_linux.c
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

#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>

#include <InfotmMedia.h>
#include <IM_buffallocapi.h>
#include <IM_devmmuapi.h>
#include <pmm_lib.h>

#include "g2d_pwl.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"G2DPWL_I:"
#define WARNHEAD	"G2DPWL_W:"
#define ERRHEAD		"G2DPWL_E:"
#define TIPHEAD		"G2DPWL_T:"

#define INTERRUPT_USED	1

typedef struct{
	struct mutex	accessLock;
	IM_BOOL			irqRegister;
	IM_UINT32		regBasePhy;
	IM_UINT32		*regBaseVir;
	IM_INT32		regSize;
	IM_UINT32		resetPhy;
	IM_UINT32		*resetVir;
	IM_UINT32		mmuPhy;
	IM_UINT32		*mmuVir;
 	pmm_handle_t 	mmu_handle;
}pwl_context_t;

static pwl_context_t *gPwl = IM_NULL;
static IM_UINT32 pwl_count = 0;

static wait_queue_head_t wq_g2d;
static IM_UINT32 gG2DWait = 0;

extern IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner);
extern IM_RET pmmdrv_release(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid);
extern IM_RET pmmdrv_dmmu_deinit(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_enable(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_disable(IN pmm_handle_t handle);

//g2d interrupt function
static irqreturn_t g2dpwl_irq_handle(int irq, void *p)
{
	IM_INT32 status = 0;
	//IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	g2dpwl_read_reg(G2D_GLB_INTS, &status);
	if(status & G2DGLBINTS_0VER){
		IM_INFOMSG((IM_STR("%s(), interrupt status is ok!"), IM_STR(_IM_FUNC_)));
		gG2DWait = 1;
		wake_up_interruptible(&wq_g2d);
	}

	g2dpwl_write_reg(G2D_GLB_INTS, status);

	//IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IRQ_NONE;
}

IM_RET g2dpwl_init(void)
{
#if INTERRUPT_USED
	IM_UINT32 tmp = 0;
#endif
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	if(pwl_count == 0){
		IM_ASSERT(gPwl == IM_NULL);
		gPwl = g2dpwl_malloc(sizeof(pwl_context_t));
		if(gPwl == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dpwl_init(), g2dpwl_malloc(gPwl) failed!")));
			return IM_RET_FAILED;
		}
		g2dpwl_memset((void *)gPwl, 0, sizeof(pwl_context_t));

		//
		mutex_init(&gPwl->accessLock);

		//
		gPwl->regBasePhy = G2D_BASE_ADDR;
		gPwl->regSize = 0x1000;
		gPwl->regBaseVir = (IM_UINT32 *)ioremap_nocache(gPwl->regBasePhy, gPwl->regSize);
		if(gPwl->regBaseVir == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dpwl_init(), ioremap(G2D) regBaseVir failed!")));
			goto Fail;
		}

		gPwl->resetPhy = G2D_RESET_ADDR;
		gPwl->resetVir = (IM_UINT32 *)ioremap_nocache(gPwl->resetPhy, 0x100);
		if(gPwl->resetVir == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dpwl_init(), ioremap(G2D) resetVir failed!")));
			goto Fail;
		}
		//it can only remap 4k aligment
		gPwl->mmuPhy = G2D_MMU_BASE_ADDR;
		gPwl->mmuVir = (IM_UINT32 *)ioremap_nocache(gPwl->mmuPhy, 0x100);
		if(gPwl->mmuVir == IM_NULL){
			IM_ERRMSG((IM_STR("@g2dpwl_init(), ioremap(G2D) mmuVir failed!")));
			goto Fail;
		}
		gPwl->mmuVir = (void*)((IM_UINT32)gPwl->mmuVir);
#if INTERRUPT_USED
		/* initualize wait queue */
		init_waitqueue_head(&wq_g2d);

		//set interrupt mask disable
		g2dpwl_read_reg(G2D_GLB_INTM, &tmp);
		tmp &= (~G2DGLBINTM_OVER);
		g2dpwl_write_reg(G2D_GLB_INTM, tmp);
		//register g2d irq
		if(request_irq(IRQ_G2D, g2dpwl_irq_handle, IRQF_DISABLED, "g2d_irq", (void *)gPwl) != 0){
			IM_ERRMSG((IM_STR("request_irq() failed")));
			goto Fail;
		}
		//enable g2d irq
		//enable_irq(IRQ_G2D);

		gPwl->irqRegister = IM_TRUE;
#endif
		gG2DWait = 0;
	}

	pwl_count++;

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
Fail:
	if(gPwl->regBaseVir != IM_NULL){
		iounmap((void *)gPwl->regBaseVir);
	}

	if(gPwl->resetVir != IM_NULL){
		iounmap((void *)gPwl->resetVir);
	}

	if(gPwl->mmuVir != IM_NULL){
		iounmap((void *)((IM_UINT32)gPwl->mmuVir));
	}

#if INTERRUPT_USED
	if(gPwl->irqRegister == IM_TRUE){
		//disable_irq(IRQ_G2D);
		free_irq(IRQ_G2D, (void *)gPwl);
		gPwl->irqRegister = IM_FALSE;
	}
#endif

	mutex_destroy(&gPwl->accessLock);
	g2dpwl_free((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET g2dpwl_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	pwl_count--;
	if(pwl_count == 0){
		IM_ASSERT(gPwl != IM_NULL);
		if(gPwl->regBaseVir != IM_NULL){
			iounmap((void *)gPwl->regBaseVir);
		}

		if(gPwl->resetVir != IM_NULL){
			iounmap((void *)gPwl->resetVir);
		}

		if(gPwl->mmuVir != IM_NULL){
			iounmap((void *)((IM_UINT32)gPwl->mmuVir));
		}

		mutex_destroy(&gPwl->accessLock);
		//unregister g2d irq
		if(gPwl->irqRegister == IM_TRUE){
			//disable_irq(IRQ_G2D);
			free_irq(IRQ_G2D, (void *)gPwl);
			gPwl->irqRegister = IM_FALSE;
		}
		g2dpwl_free((void *)gPwl);
		gPwl = IM_NULL;
	}
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_reset(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	writel(0xff, (IM_UINT32)gPwl->resetVir);
	writel(0x0, (IM_UINT32)gPwl->resetVir);
	return IM_RET_OK;
}

IM_RET g2dpwl_write_reg(IM_UINT32 addr, IM_UINT32 val)
{
	IM_INFOMSG((IM_STR("%s(base_addr=0x%x, offset=0x%x, value=0x%x)"), IM_STR(_IM_FUNC_), (IM_UINT32)gPwl->regBaseVir, addr, val));
	writel(val, (IM_UINT32)gPwl->regBaseVir + addr);
	return IM_RET_OK;
}

IM_RET g2dpwl_read_reg(IM_UINT32 addr, IM_UINT32 *val)
{
	*val = readl((IM_UINT32)gPwl->regBaseVir/*IMAP_VA_FB*/ + addr);
	return IM_RET_OK;
}


void *g2dpwl_malloc(IM_UINT32 size)
{
	return kmalloc(size,GFP_KERNEL);
}

void g2dpwl_free(void *p)
{
	if(p != IM_NULL) kfree(p);
}

void g2dpwl_memcpy(void *dst, void *src, IM_UINT32 size)
{
	memcpy(dst, src, size);
}

void g2dpwl_memset(void *dst, IM_INT8 c, IM_UINT32 size)
{
	memset(dst, c, size);
}

IM_RET g2dpwl_module_enable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_module_disable(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_module_reset(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_module_request_frequency(IM_FLOAT freq, IM_UINT32 flag)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_lock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_lock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET g2dpwl_unlock_access(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_unlock(&gPwl->accessLock);
	return IM_RET_OK;
}

IM_RET g2dpwl_lock_init(g2dpwl_lock_t *lck)
{
	struct mutex *mtx = (struct mutex *)g2dpwl_malloc(sizeof(struct mutex));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(mtx != IM_NULL);
	mutex_init(mtx);
	*lck = (g2dpwl_lock_t)mtx;
	return IM_RET_OK;
}

IM_RET g2dpwl_lock_deinit(g2dpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_destroy((struct mutex *)lck);
	g2dpwl_free((void *)lck);
	return IM_RET_OK;
}

IM_RET g2dpwl_lock(g2dpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_lock((struct mutex *)lck);
	return IM_RET_OK;
}

IM_RET g2dpwl_unlock(g2dpwl_lock_t lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_unlock((struct mutex *)lck);
	return IM_RET_OK;
}

typedef struct{
	IM_BOOL manualReset;
	struct semaphore sem;
}signal_t;

IM_RET g2dpwl_sig_init(g2dpwl_signal_t *sig, IM_BOOL manualReset)
{
	signal_t *signal = (signal_t *)g2dpwl_malloc(sizeof(signal_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(signal != IM_NULL);
	signal->manualReset = manualReset;
	sema_init(&signal->sem, 0);
	*sig = (g2dpwl_signal_t)signal;
	return IM_RET_OK;
}

IM_RET g2dpwl_sig_deinit(g2dpwl_signal_t sig)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	g2dpwl_free((void *)sig);
	return IM_RET_OK;
}

IM_RET g2dpwl_sig_set(g2dpwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	IM_INFOMSG((IM_STR("g2dpwl_sig_set---1, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
	IM_INFOMSG((IM_STR("set signal(sig=0x%x)"), (int)sig));
	up(&signal->sem);
	IM_INFOMSG((IM_STR("g2dpwl_sig_set---2, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
	return IM_RET_OK;
}

IM_RET g2dpwl_sig_reset(g2dpwl_signal_t sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	while(down_trylock(&signal->sem) == 0);
	return IM_RET_OK;
}

IM_RET g2dpwl_sig_wait(g2dpwl_signal_t sig, g2dpwl_lock_t *lck, IM_INT32 timeout)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	if(lck != IM_NULL){		
		g2dpwl_unlock(*lck);
	}

	IM_INFOMSG((IM_STR("wait signal(sig=0x%x)++"), (int)sig));
	if(timeout == -1){
		IM_INFOMSG((IM_STR("g2dpwl_sig_wait---1, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
		if(down_interruptible(&signal->sem) != 0){
			if(lck != IM_NULL){
				g2dpwl_lock(*lck);
			}
			return IM_RET_FAILED;
		}
	}else{
		if(down_timeout(&signal->sem, (long)timeout * HZ / 1000000) == -ETIME){
			if(lck != IM_NULL){
				g2dpwl_lock(*lck);
			}
			IM_INFOMSG((IM_STR("g2dpwl_sig_wait timeout!!!")));
			return IM_RET_TIMEOUT;
		}
	}
	IM_INFOMSG((IM_STR("wait signal(sig=0x%x)--"), (int)sig));
	IM_INFOMSG((IM_STR("g2dpwl_sig_wait---2, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
	if(signal->manualReset == IM_TRUE){
		IM_INFOMSG((IM_STR("g2dpwl_sig_wait---3, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
		up(&signal->sem);
		IM_INFOMSG((IM_STR("g2dpwl_sig_wait---4, signal=0x%x, sem->count=%d"), (int)signal, signal->sem.count));
	}
	if(lck != IM_NULL){
		g2dpwl_lock(*lck);
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

typedef struct {
	struct work_struct work;
	struct workqueue_struct *wq;
	void *data;
	g2dpwl_func_thread_entry_t func;
}thread_t;

static void work_handle(struct work_struct *p)
{
	thread_t *thrd = container_of(p, thread_t, work);
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd->func(thrd->data);
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
}

IM_RET g2dpwl_thread_init(g2dpwl_thread_t *thread, g2dpwl_func_thread_entry_t func, void *data)
{
	int ret = 1;
	thread_t *thrd = IM_NULL;

	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	thrd = (thread_t *)g2dpwl_malloc(sizeof(thread_t));
	if(thrd == IM_NULL){
		IM_ERRMSG((IM_STR("@g2dpwl_thread_init(), g2dpwl_malloc(thread_t) failed!")));
		return IM_RET_FAILED;
	}

	thrd->wq = create_singlethread_workqueue("g2dpwl_wq");
	if(thrd->wq == NULL){
		IM_ERRMSG((IM_STR("@g2dpwl_thread_init(), create_singlethread_workqueue() failed!")));
		g2dpwl_free((void *)thrd);
		return IM_RET_FAILED;
	}

	thrd->data = data;
	thrd->func = func;
	INIT_WORK(&thrd->work, work_handle);

	//INIT_WORK(&thrd->work, func);
	ret = queue_work(thrd->wq, &thrd->work);
	if(ret != 1){
		destroy_workqueue(thrd->wq);
		g2dpwl_free((void *)thrd);
		return IM_RET_FAILED;
	}
	
	*thread = (g2dpwl_thread_t)thrd;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_thread_deinit(g2dpwl_thread_t thread)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(thread != IM_NULL);
	destroy_workqueue(((thread_t *)thread)->wq);
	g2dpwl_free((void *)thread);
	return IM_RET_OK;
}

void g2dpwl_msleep(IM_UINT32 ms)
{
	msleep(ms);
}

/*IM_RET g2dpwl_register_isr(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(request_irq(IRQ_G2D, g2dpwl_irq_handle, IRQF_DISABLED, "g2d_irq", (void *)gPwl) != 0){
		IM_ERRMSG((IM_STR("request_irq() failed")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET g2dpwl_unregister_isr(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	free_irq(IRQ_G2D, (void *)gPwl);
	return IM_RET_OK;
}*/

IM_RET g2dpwl_enable_intr(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	enable_irq(IRQ_G2D);
	return IM_RET_OK;
}

IM_RET g2dpwl_disable_intr(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	disable_irq(IRQ_G2D);
	return IM_RET_OK;
}

IM_RET g2dpwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout)
{
#if INTERRUPT_USED	//use interrupt
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	wait_event_interruptible(wq_g2d, gG2DWait != 0);
	if(intr != IM_NULL){
		*intr = gG2DWait;
	}
	gG2DWait = 0;
#else	//use inquire
	IM_UINT32 status = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	do{
		g2dpwl_read_reg(G2D_GLB_INTS, &status);
		if(status & G2DGLBINTS_0VER){
			gG2DWait = 1;
		}
	}while(gG2DWait == 0);

	if(intr != IM_NULL){
		*intr = gG2DWait;
	}
	gG2DWait = 0;
	g2dpwl_write_reg(G2D_GLB_INTS, status);
#endif
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET g2dpwl_mmu_init(void)
{
	IM_RET ret;
	ret = pmmdrv_open(&gPwl->mmu_handle, "G2D");
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_init(), pmmdrv_open() failed!")));
		return IM_RET_FAILED;
	}
	ret = pmmdrv_dmmu_init(gPwl->mmu_handle, DMMU_DEV_G2D);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_init(), pmmdrv_dmmu_init() failed!")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET g2dpwl_mmu_deinit(void)
{
	IM_RET ret;
	ret = pmmdrv_dmmu_deinit(gPwl->mmu_handle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_deinit(), pmmdrv_dmmu_deinit() failed!")));
		return IM_RET_FAILED;
	}
	ret = pmmdrv_release(gPwl->mmu_handle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_deinit(), pmmdrv_release() failed!")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET g2dpwl_mmu_enable(void)
{
	IM_RET ret;
	IM_UINT32 temp;
	ret = pmmdrv_dmmu_enable(gPwl->mmu_handle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_enable(), pmmdrv_release() failed!")));
		return IM_RET_FAILED;
	}
	//g2d mmu write/read enable
	temp = readl((IM_UINT32)gPwl->mmuVir + G2D_MMU_SWITCH_OFFSET);
	temp |= (1<<1) | (1<<0);
	writel(temp, (IM_UINT32)gPwl->mmuVir + G2D_MMU_SWITCH_OFFSET);
	return IM_RET_OK;
}

IM_RET g2dpwl_mmu_disable(void)
{
	IM_RET ret;
	IM_UINT32 temp;
	ret = pmmdrv_dmmu_disable(gPwl->mmu_handle);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR("@g2dpwl_mmu_disable(), pmmdrv_release() failed!")));
		return IM_RET_FAILED;
	}
	//g2d mmu write/read disable
	temp = readl((IM_UINT32)gPwl->mmuVir + G2D_MMU_SWITCH_OFFSET);
	temp &= 0xfffffffc;
	writel(temp, (IM_UINT32)gPwl->mmuVir + G2D_MMU_SWITCH_OFFSET);
	return IM_RET_OK;
}
