/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: camif_pwl.h
--
--  Description :
--
--	Author:
--     Jimmy Shu   <jimmy.shu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	Jimmy@2012/10/17: first commit.
--
------------------------------------------------------------------------------*/

#ifndef __CAMIF_PWL_H__
#define __CAMIF_PWL_H__

#define CAMIFPWL_BUFFER_ALIGN_8BYTES		1
#define CAMIFPWL_BUFFER_ALIGN_16BYTES		2

#define CAMIFPWL_INTR_DMA_SYNC	(1<<0)
#define CAMIFPWL_INTR_DMA_Y		(1<<1)
#define CAMIFPWL_INTR_DMA_U		(1<<2)
#define CAMIFPWL_INTR_DMA_V		(1<<3)

typedef IM_RET (*func2_irq_callback_t)(void);

typedef struct{
	IM_UINT32 offset;
	IM_UINT32 value;
}camifpwl_reg_val_t;
//==============================interface======================================
IM_RET camifpwl_init(void);
IM_RET camifpwl_deinit(void);

IM_RET camifpwl_register_irq(int irq, func2_irq_callback_t irq_callback);

IM_RET camifpwl_write_reg(IM_UINT32 offset, IM_UINT32 value);

/*
 *FUNC: write registers.
 *PARAM:
 * 	rv, reg&value pair pointer.
 *	num, item number of the rv pair.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camifpwl_write_regs(camifpwl_reg_val_t *rv, IM_UINT32 num);

/*
 *FUNC: write register bits.
 *PARAM:
 * 	reg_ofst, register offset with regbase.
 * 	bit_ofst, bit offset with the register.
 * 	bit_width, valid bit width.
 * 	val, value to write.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camifpwl_write_regbit(IM_UINT32 reg_ofst, IM_UINT32 bit_ofst, IM_UINT32 bit_width, IM_UINT32 value);

IM_RET camifpwl_read_reg(IM_UINT32 offset, IM_UINT32 *value);

/*
 *FUNC: read registers.
 *PARAM: 
 *	rv, reg&value pair pointer.
 *	num, item number of the r&v pair.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camifpwl_read_regs(camifpwl_reg_val_t *rv, IM_UINT32 num);

/*
 *FUNC: wait hw ready.
 *PARAM: 
 *	intr,
 *	timeout, timeout.
 *RETURN: IM_RET_OK succeed, IM_RET_TIMEOUT and other failed.
 */
IM_RET camifpwl_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout);

/*
 *FUNC:  frame sync.
 *PARAM: 
 *	timeout, timeout.
 *RETURN: IM_RET_OK succeed, IM_RET_TIMEOUT and other failed.
 *it should be called only when CAMIF enable and CAMIF state change(param or module) 
 */
IM_RET camifpwl_sync(IM_INT32 timeout);


/*
 *FUNC: allocate linear memory, it has physical and virtual address.
 *PARAM: 
 *	buffer, save allocated memory.
 *	flag, properties of the requested buffer.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camifpwl_alloc_linear_memory(IM_Buffer *buffer, IM_UINT32 flag);


/*
 *FUNC: free linear memory.
 *PARAM:
 *	buffer, free this buffer.
 *RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camifpwl_free_linear_memory(IN IM_Buffer *buffer);


/*
 * FUNC: general memory operation.
 */
void *camifpwl_malloc(IM_UINT32 size);
void camifpwl_free(void *mem);
void camifpwl_memcpy(void *dst, void *src, IM_UINT32 size);
void camifpwl_memset(void *dst, IM_CHAR c, IM_UINT32 size);




IM_RET camifpwl_lock_access(void);
IM_RET camifpwl_unlock_access(void);

typedef void * camifpwl_lock_t;
IM_RET camifpwl_lock_init(camifpwl_lock_t *lck);
IM_RET camifpwl_lock_deinit(camifpwl_lock_t lck);
IM_RET camifpwl_lock(camifpwl_lock_t lck);
IM_RET camifpwl_unlock(camifpwl_lock_t lck);

//spin lock
typedef void * camifpwl_spinlock_t;
IM_RET camifpwl_spinlock_init(camifpwl_spinlock_t *lck);
IM_RET camifpwl_spinlock_deinit(camifpwl_spinlock_t lck);
IM_RET camifpwl_spinlock(camifpwl_spinlock_t lck);
IM_RET camifpwl_spinunlock(camifpwl_spinlock_t lck);
IM_RET camifpwl_spinlock_irq(camifpwl_spinlock_t lck);
IM_RET camifpwl_spinunlock_irq(camifpwl_spinlock_t lck);


typedef void * camifpwl_signal_t;
IM_RET camifpwl_sig_init(camifpwl_signal_t *sig, IM_BOOL manualReset);
IM_RET camifpwl_sig_deinit(camifpwl_signal_t sig);
IM_RET camifpwl_sig_set(camifpwl_signal_t sig);
IM_RET camifpwl_sig_reset(camifpwl_signal_t sig);
IM_RET camifpwl_sig_wait(camifpwl_signal_t sig, camifpwl_lock_t *lck, IM_INT32 timeout);

typedef void * camifpwl_thread_t;
typedef void (*camifpwl_func_thread_entry_t)(void *data);
//typedef void (*camifpwl_func_thread_entry_t)(struct work_struct *data);
IM_RET camifpwl_thread_init(camifpwl_thread_t *thread, camifpwl_func_thread_entry_t func, void *p);
IM_RET camifpwl_thread_deinit(camifpwl_thread_t thread);
void camifpwl_msleep(IM_UINT32 ms);

#endif	// __CAMIF_PWL_H__

