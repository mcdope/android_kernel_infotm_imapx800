/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file g2d_pwl.h
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

#ifndef __G2D_PWL_H__
#define __G2D_PWL_H__

#if defined(KERNEL_LINUX)
#include <mach/irqs.h>
#endif


#ifndef IRQ_G2D
#define IRQ_G2D (249)
#endif//IRQ_G2D

//g2d base reg address
#define G2D_BASE_ADDR	(0x29100000)

//g2d system reset address
#define G2D_RESET_ADDR (0x21e0c000)

//g2d mmu base address
#define G2D_MMU_BASE_ADDR 	(0x29000000)
#define G2D_MMU_SWITCH_OFFSET 	(0x8)

#define G2D_GLB_INTS		(0xE4)
#define G2DGLBINTS_0VER		(1<<0)

#define G2D_GLB_INTM		(0xE8)
#define G2DGLBINTM_OVER		(1<<0)

//==============================interface======================================
IM_RET g2dpwl_init(void);
IM_RET g2dpwl_deinit(void);

IM_RET g2dpwl_reset(void);
IM_RET g2dpwl_write_reg(IM_UINT32 addr, IM_UINT32 val);
IM_RET g2dpwl_read_reg(IM_UINT32 addr, IM_UINT32 *val);

void *g2dpwl_malloc(IM_UINT32 size);
void g2dpwl_free(void *p);
void g2dpwl_memcpy(void *dst, void *src, IM_UINT32 size);
void g2dpwl_memset(void *dst, IM_INT8 c, IM_UINT32 size);


IM_RET g2dpwl_module_enable(void);
IM_RET g2dpwl_module_disable(void);
IM_RET g2dpwl_module_reset(void);
IM_RET g2dpwl_module_request_frequency(IM_FLOAT freq, IM_UINT32 flag);

IM_RET g2dpwl_lock_access(void);
IM_RET g2dpwl_unlock_access(void);

typedef void * g2dpwl_lock_t;
IM_RET g2dpwl_lock_init(g2dpwl_lock_t *lck);
IM_RET g2dpwl_lock_deinit(g2dpwl_lock_t lck);
IM_RET g2dpwl_lock(g2dpwl_lock_t lck);
IM_RET g2dpwl_unlock(g2dpwl_lock_t lck);

typedef void * g2dpwl_signal_t;
IM_RET g2dpwl_sig_init(g2dpwl_signal_t *sig, IM_BOOL manualReset);
IM_RET g2dpwl_sig_deinit(g2dpwl_signal_t sig);
IM_RET g2dpwl_sig_set(g2dpwl_signal_t sig);
IM_RET g2dpwl_sig_reset(g2dpwl_signal_t sig);
IM_RET g2dpwl_sig_wait(g2dpwl_signal_t sig, g2dpwl_lock_t *lck, IM_INT32 timeout);

typedef void * g2dpwl_thread_t;
typedef void (*g2dpwl_func_thread_entry_t)(void *data);
//typedef void (*g2dpwl_func_thread_entry_t)(struct work_struct *data);
IM_RET g2dpwl_thread_init(g2dpwl_thread_t *thread, g2dpwl_func_thread_entry_t func, void *p);
IM_RET g2dpwl_thread_deinit(g2dpwl_thread_t thread);
void g2dpwl_msleep(IM_UINT32 ms);

//IM_RET g2dpwl_register_isr(void);
//IM_RET g2dpwl_unregister_isr(void);
IM_RET g2dpwl_enable_intr(void);
IM_RET g2dpwl_disable_intr(void);
IM_RET g2dpwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout);

//mmu interface
IM_RET g2dpwl_mmu_init(void);
IM_RET g2dpwl_mmu_deinit(void);
IM_RET g2dpwl_mmu_enable(void);
IM_RET g2dpwl_mmu_disable(void);

#endif	// __G2D_PWL_H__

