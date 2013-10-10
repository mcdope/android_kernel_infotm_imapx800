/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_pwl.h
--
--  Description :
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

#ifndef __ISP_PWL_H__
#define __ISP_PWL_H__

#define ISPPWL_BUFFER_ALIGN_8BYTES		1
#define ISPPWL_BUFFER_ALIGN_16BYTES		2

#define ISPPWL_INTR_DMA_SYNC	(1<<0)
#define ISPPWL_INTR_DMA_Y		(1<<1)
#define ISPPWL_INTR_DMA_U		(1<<2)
#define ISPPWL_INTR_DMA_V		(1<<3)
#define ISPPWL_INTR_DMA_BDC		(1<<4)

typedef IM_RET (*func_irq_callback_t)(void);

typedef struct{
	IM_UINT32 offset;
	IM_UINT32 value;
}isppwl_reg_val_t;
//==============================interface======================================
IM_RET isppwl_init(void);
IM_RET isppwl_deinit(void);

IM_RET isppwl_register_irq(int irq, func_irq_callback_t irq_callback);

IM_RET isppwl_write_reg(IM_UINT32 offset, IM_UINT32 value);

/*
 *FUNC: write registers.
 *PARAM:
 * 	rv, reg&value pair pointer.
 *	num, item number of the rv pair.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET isppwl_write_regs(isppwl_reg_val_t *rv, IM_UINT32 num);

/*
 *FUNC: write register bits.
 *PARAM:
 * 	reg_ofst, register offset with regbase.
 * 	bit_ofst, bit offset with the register.
 * 	bit_width, valid bit width.
 * 	val, value to write.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET isppwl_write_regbit(IM_UINT32 reg_ofst, IM_UINT32 bit_ofst, IM_UINT32 bit_width, IM_UINT32 value);

IM_RET isppwl_read_reg(IM_UINT32 offset, IM_UINT32 *value);

/*
 *FUNC: read registers.
 *PARAM: 
 *	rv, reg&value pair pointer.
 *	num, item number of the r&v pair.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET isppwl_read_regs(isppwl_reg_val_t *rv, IM_UINT32 num);

/*
 *FUNC: wait hw ready.
 *PARAM: 
 *	intr,
 *	timeout, timeout.
 *RETURN: IM_RET_OK succeed, IM_RET_TIMEOUT and other failed.
 */
IM_RET isppwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout);

/*
 *FUNC:  frame sync.
 *PARAM: 
 *	timeout, timeout.
 *RETURN: IM_RET_OK succeed, IM_RET_TIMEOUT and other failed.
 *it should be called only when ISP enable and ISP state change(param or module) 
 */
IM_RET isppwl_sync(IM_INT32 timeout);


/*
 *FUNC: allocate linear memory, it has physical and virtual address.
 *PARAM: 
 *	buffer, save allocated memory.
 *	flag, properties of the requested buffer.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET isppwl_alloc_linear_memory(IM_Buffer *buffer, IM_UINT32 flag);


/*
 *FUNC: free linear memory.
 *PARAM:
 *	buffer, free this buffer.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET isppwl_free_linear_memory(IN IM_Buffer *buffer);


/*
 * FUNC: general memory operation.
 */
void *isppwl_malloc(IM_UINT32 size);
void isppwl_free(void *mem);
void isppwl_memcpy(void *dst, void *src, IM_UINT32 size);
void isppwl_memset(void *dst, IM_CHAR c, IM_UINT32 size);




IM_RET isppwl_lock_access(void);
IM_RET isppwl_unlock_access(void);

typedef void * isppwl_lock_t;
IM_RET isppwl_lock_init(isppwl_lock_t *lck);
IM_RET isppwl_lock_deinit(isppwl_lock_t lck);
IM_RET isppwl_lock(isppwl_lock_t lck);
IM_RET isppwl_unlock(isppwl_lock_t lck);

//spin lock
typedef void * isppwl_spinlock_t;
IM_RET isppwl_spinlock_init(isppwl_spinlock_t *lck);
IM_RET isppwl_spinlock_deinit(isppwl_spinlock_t lck);
IM_RET isppwl_spinlock(isppwl_spinlock_t lck);
IM_RET isppwl_spinunlock(isppwl_spinlock_t lck);
IM_RET isppwl_spinlock_irq(isppwl_spinlock_t lck);
IM_RET isppwl_spinunlock_irq(isppwl_spinlock_t lck);


typedef void * isppwl_signal_t;
IM_RET isppwl_sig_init(isppwl_signal_t *sig, IM_BOOL manualReset);
IM_RET isppwl_sig_deinit(isppwl_signal_t sig);
IM_RET isppwl_sig_set(isppwl_signal_t sig);
IM_RET isppwl_sig_reset(isppwl_signal_t sig);
IM_RET isppwl_sig_wait(isppwl_signal_t sig, isppwl_lock_t *lck, IM_INT32 timeout);

typedef void * isppwl_thread_t;
typedef void (*isppwl_func_thread_entry_t)(void *data);
//typedef void (*isppwl_func_thread_entry_t)(struct work_struct *data);
IM_RET isppwl_thread_init(isppwl_thread_t *thread, isppwl_func_thread_entry_t func, void *p);
IM_RET isppwl_thread_deinit(isppwl_thread_t thread);
void isppwl_msleep(IM_UINT32 ms);

#endif	// __ISP_PWL_H__

