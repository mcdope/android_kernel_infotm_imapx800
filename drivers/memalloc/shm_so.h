/*******************************************************************************
 * shm_so.h
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: 
 * 	Head file of shared memory support.
 *
 * Author:
 *	Sololz <sololz.luo@gmail.com>.
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.0  2011/01/03 Sololz
 * 	Create this file.
 ******************************************************************************/

#if !defined(__SHM_SO_H__)
#define __SHM_SO_H__

#include <linux/semaphore.h>

#include "memalloc.h"

/* ############################################################################## */

/* Parameter structure of IO control command to get shared memory. */
typedef struct {
	unsigned int size;	/* Required shared memory size. */
	unsigned int key;
} shm_so_get_t;

/* Shared memory data structure. */
typedef struct shm_so_t {
	struct shm_so_t *pre;
	struct shm_so_t *next;

	unsigned int size;	/* Size of shared memory. */
	unsigned int start;	/* Start access location of shared memory. */
	unsigned int phys;	/* FIXME: Not used yet. */
	void *virt;		/* Kernel level virtual address. */

	unsigned int key;	/* Shared memory named by key. */
	unsigned int deps;	/* Records user count of current shared memory. */
	struct semaphore sem;
} shm_so_t;

/* Global items pack structure. */
typedef struct {
	struct mutex gshms_lock; /* Global shared memory lock, for shm list. */
	shm_so_t *gshms;	/* Global shared memory list. */
} shm_so_global_t;

/* ############################################################################## */

#define SHM_SO_DEV_NAME		"shm_so"
#define SHM_SO_MAX_ALLOC_SIZE	(64 * 1024)

#define SHM_SO_IOCMD_MAGIC	's'
#define SHM_SO_IOCMD_ALLOC	_IOW(SHM_SO_IOCMD_MAGIC, 1, shm_so_get_t)
#define SHM_SO_IOCMD_MAX_NUM	(1)

#endif	/* __SHM_SO_H__ */
