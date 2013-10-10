/********************************************************************************
** linux-2.6.31.6/arch/arm/plat-imap/include/plat/mem_reserve.h
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.0  12/17/2009    Raymond Wang
********************************************************************************/
#ifndef _MEM_RESERVE_H
#define _MEM_RESERVE_H

#include <linux/types.h>

#define RESERVEMEM_DEV_ETH		0
#define RESERVEMEM_DEV_MEMALLOC		1
#define RESERVEMEM_DEV_PMM		2
#define RESERVEMEM_DEV_FTL              3
#define RESERVEMEM_DEV_MAX		4

struct imap_reservemem_device {
	int		id;
	const char 	*name;
	size_t		size;
	dma_addr_t	paddr;
};

extern dma_addr_t imap_get_reservemem_paddr(int dev_id);
extern size_t imap_get_reservemem_size(int dev_id);

/* these memorys is not reserved indeed. they contain value created in uboot.
 * driver read & save these values during boot time.
 * In fact, linux do not know we use these memorys for special purpose and is
 * still in charge of managing these memorys.
 *
 * -- by warits, Oct 18, 2012
 */

#define ___RBASE   (IMAP_SDRAM_BASE + 0x10800000)

#define RESERVED_BASE_LOGO          (___RBASE + 0xa000000)
#define RESERVED_BASE_UBOOT0        (___RBASE + 0xb000000)
#define RESERVED_BASE_DTRA          (___RBASE + 0x9000000)
#define RESERVED_BASE_RRTB          (___RBASE + 0x8000000)

#endif

