/***********************************************************************************
 * mmalcr.h
 *
 * Copyright (C) 2011 Sololz<sololz.luo@gmail.com>.
 * 
 * Head file of allocator, the coding style and API design strictly refers to 
 * the POSIX rules.
 *
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.0  2011/01/17 Sololz
 * 	Create this file.
 **********************************************************************************/

#if !defined(__MMALCR_H__)
#define __MMALCR_H__

#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/mutex.h>
#else	/* __KERNEL__ */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#endif	/* __KERNEL__ */

/* ############################################################################## */

/* Defines the max and min size count of asigned to be managed memory region. */
#define MMALCR_MAX_PROCESS_SIZE_ORDER	(31)	/* 2GB. */
#define MMALCR_MIN_PROCESS_SIZE_ORDER	(12)	/* 4KB. */
#if ((MMALCR_MAX_PROCESS_SIZE_ORDER > 32) || (MMALCR_MIN_PROCESS_SIZE_ORDER < 1))
#error "mmalcr size order error!"
#endif
#define MMALCR_MAX_PROCESS_SIZE	(1 << MMALCR_MAX_PROCESS_SIZE_ORDER)
#define MMALCR_MIN_PROCESS_SIZE	(1 << MMALCR_MIN_PROCESS_SIZE_ORDER)

/* Defines the max and min align count. */
#define MMALCR_MAX_ALIGN_ORDER	(12)
#define MMALCR_MIN_ALIGN_ORDER	(3)
#define MMALCR_MAX_ALIGN	(1 << MMALCR_MAX_ALIGN_ORDER)
#define MMALCR_MIN_ALIGN	(1 << MMALCR_MIN_ALIGN_ORDER)

/* 0xffffffff is supposed to be an error address because on 32bit platform, 
 * this address will never be a start of a memory region. */
#define MMALCR_ERROR_ADDRESS	(0xffffffff)
#define MMALCR_MAX_ADDRESS	(0xffffffff)

/* ############################################################################## */

/* Return value of some mmalcr function APIs. */
#define MMALCR_RET_OK		(0x00000000)
#define MMACLR_RET_EINVAL	(0x80000001)
#define MMACLR_RET_ENOMEM	(0x80000002)
#define MMALCR_RET_EFAULT	(0x80000004)
typedef int32_t mmalcr_ret_t;

/* ############################################################################## */

/**
 * The create function will allocate a memory region of mmalcr_t size to record
 * the parameters and initialize internal data.
 *
 * Params:
 * @ size - Defines the size of managed memory block.
 * @ mm_start - You may have notice that the data type of mm_start is *unsigned 
 * int32_t* but it does not necessarily means that this allocator is designed 
 * only to process physical address. This parameter can also be virtual address.
 * @ align_order - This align is the basic alignment of memory block allocated, 
 * the alloc API will also provide an align argument to set. The *align* here 
 * must be that *alloc-align* exactly divided. *align* must be 2E(x).
 *
 * Return:
 * The create function returns NULL if any error, else returns the allocator 
 * pointer.
 * Release function returns 0 represents success, else returns negative number.
 * @ -EINVAL - Invalid arguments.
 */
typedef void * mmalcr_t;
mmalcr_t mmalcr_create(uint32_t size, uint32_t mm_start, \
		uint32_t align_order);
mmalcr_ret_t mmalcr_release(mmalcr_t mmalcr);

/**
 * Set and get allocator configurations.
 */
#define MMALCR_CONFIG_FLAG_ALL	(0xffffffff)
typedef uint32_t mmalcr_config_flag_t;
typedef struct {
} mmalcr_config_t;

/**
 * TODO
 */
uint32_t mmalcr_alloc(mmalcr_t mmalcr, uint32_t size, \
		uint32_t align_order);
mmalcr_ret_t mmalcr_free(mmalcr_t mmalcr, uint32_t addr);

#endif	/* __MMALCR_H__ */
