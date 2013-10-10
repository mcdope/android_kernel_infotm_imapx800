/*
 * memalloc.h
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if !defined(__MEMALLOC_H__)
#define __MEMALLOC_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>

#include <mach/mem-reserve.h>

/* ############################################################################################# */

#define MEMALLOC_MAX_OPEN	(256)	/* Max instance. */
#define MEMALLOC_MAX_ALLOC	(2048)	/* Max allocation each open. */

#if defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_CLASS_MODE)
/* Origin driver register mode for memalloc. */
#define MEMALLOC_DYNAMIC_MAJOR	(0)	/* 0 means dynamic alloc by default. */
#define MEMALLOC_DYNAMIC_MINOR	(255)
#define	MEMALLOC_DEFAULT_MAJOR	(111)
#define MEMALLOC_DEFAULT_MINOR	(111)
#endif

#define IMAPX200_MEMORY_START	(0x40000000)

#define MEMALLOC_DEV_NAME	"memalloc"
#define MEMALLOC_PAGE_SIZE	(4096)	/* Page size defined internal. */
#define MEMALLOC_MAX_ALLOC_SIZE	(8 * 1024 * 1024)	/* 8MB is quite so enough, normally less then 3MB */

/* RESERVED MEMORY. */
#if defined(CONFIG_IMAP_MEMALLOC_MANUAL_RESERVE)
#define MEMALLOC_MEM_START	IMAPX200_MEMORY_START	/* Using board provide by infotm, memory start address must be this. */
#define MEMALLOC_MEM_END	(MEMALLOC_MEM_START + (CONFIG_IMAP_MEMALLOC_TOTAL_SIZE * 1024 * 1024))
#define MEMALLOC_RSV_SIZE	(CONFIG_IMAP_MEMALLOC_RSV_SIZE * 1024 * 1024)	/* Reserved size to be fix. */
#define MEMALLOC_RSV_ADDR	(MEMALLOC_MEM_END - MEMALLOC_RSV_SIZE)
#endif

/* Debug macros include debug alert error. */
#ifdef CONFIG_IMAP_MEMALLOC_DEBUG
#define MEMALLOC_DEBUG(debug, ...)	\
    printk(KERN_DEBUG "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)
#else
#define MEMALLOC_DEBUG(debug, ...)  	do{}while(0)
#endif

#define MEMALLOC_ALERT(alert, ...)	\
    printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)
#define MEMALLOC_ERROR(error, ...)	\
    printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#define memalloc_debug(debug, ...)	MEMALLOC_DEBUG(debug, ##__VA_ARGS__)
#define memalloc_alert(alert, ...)	MEMALLOC_ALERT(alert, ##__VA_ARGS__)
#define memalloc_error(error, ...)	MEMALLOC_ERROR(error, ##__VA_ARGS__)

/* IO control commands. */
#define MEMALLOC_MAGIC  	'k'
#define MEMALLOC_GET_BUF        _IOWR(MEMALLOC_MAGIC, 1, unsigned long)
#define MEMALLOC_FREE_BUF       _IOW(MEMALLOC_MAGIC, 2, unsigned long)
#define MEMALLOC_FLUSH_RSV	_IO(MEMALLOC_MAGIC, 6)
/* Cached memroy support. */
#define MEMALLOC_SET_CACHED	_IOW(MEMALLOC_MAGIC, 11, unsigned long)
/* Debugging tool. */
#define MEMALLOC_RESET		_IO(MEMALLOC_MAGIC, 15)
/* Export this interface for user space to get reserved memory 
 * size information. */
#define MEMALLOC_GET_TOTAL_SIZE	_IOR(MEMALLOC_MAGIC, 16, unsigned int)
#define MEMALLOC_GET_LEFT_SIZE	_IOR(MEMALLOC_MAGIC, 17, unsigned int)
/* Memory block with key asigned, to support sharing in different 
 * processes/tasks. */
#define MEMALLOC_ALLOC_WITH_KEY	_IOWR(MEMALLOC_MAGIC, 18, awk_param_t)
#define MEMALLOC_FREE_WITH_KEY	_IOW(MEMALLOC_MAGIC, 19, awk_param_t)

#define MEMALLOC_MAX_CMD	(19)

/* ############################################################################################# */

typedef struct {
    unsigned int paddr;
    unsigned int size;
} memalloc_param_t;

/* Special shared memory param struct. */
typedef struct {
    uint32_t key;
    uint32_t size;
    uint32_t addr;
    uint32_t fresh;
} awk_param_t;

typedef struct {
    uint32_t phys;
    uint32_t size;
} pmemblock_t;

/* This structure for open instance. */
typedef struct {
    /* Stores physics address alloc by ioctl for each instance. */
    pmemblock_t pmemblock[MEMALLOC_MAX_ALLOC];
    int alloc_count;
    uint32_t cached_mark;
} memalloc_inst_t;

/*
 * Reserved memory list structure, every memory node start address
 * is the end address + 1 of pre node memory, current end memory 
 * address is the start address of next node memory start address - 1.
 * It means that all memory block in list is linear address, this 
 * design will be very easy for unused block merge. Considering this
 * memory alloc might use O(n) time to get correspond memory, TODO, 
 * to be optimized.
 */
typedef struct rsv_mem_struct {
    unsigned int key;		        /* A key mark for sharing support. */
    unsigned int ref;		        /* Share count. */
    unsigned int mark;		        /* Mark for current memory block is used or not. */
    unsigned int phys;		        /* Physical start address of current memory block. */
    unsigned int size;		        /* Current memory block size. */
    struct rsv_mem_struct *pre;	        /* Pre memory node of current memory block. */
    struct rsv_mem_struct *next;	/* Next memory node of current memory block. */
} rsv_mem_t;

/* This structure stores global parameters. */
typedef struct {
    /* TODO FIXME:
     * While debuging this mutex, I found that spin_lock() does not work. */
    struct mutex m_lock;
#if defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_CLASS_MODE)
    int major;
    int minor;
    struct class *m_class;
#endif
    rsv_mem_t *rsv_head;		/* Head of reserved memory list. */
    unsigned int rsv_phys;		/* Reserved memory start physical address. */
    unsigned int rsv_phys_end;	        /* Reserved memory end physical address. */
    unsigned int rsv_size;		/* Reserved memory size. */
    int inst_count;
    memalloc_inst_t shared_inst;	/* Shared instance. */
    int sh_count;			/* For android share memory in different processes and threads. */
    unsigned int trace_memory;	        /* Trace memory usage. */
} memalloc_global_t;

/* Memory cached flush param. */
typedef struct {
    unsigned int size;
    void *virt;
} cacheflush_param_t;

#endif	/* __MEMALLOC_H__ */
