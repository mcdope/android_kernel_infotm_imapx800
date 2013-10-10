/********************************************************************************
** linux-2.6.31.6/arch/arm/plat-imap/mem_reserve.c
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
**     1.1  12/17/2009    remove x200 token, these code will be
**                        used on every imap platforms.
**  leo@2012/09/22: add pmm.reserve.size item support for RESERVEMEM_DEV_PMM.
********************************************************************************/

#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/swap.h>
#include <asm/setup.h>
#include <linux/io.h>
#include <mach/memory.h>
#include <mach/items.h>

#include <mach/mem-reserve.h>

static struct imap_reservemem_device reservemem_devs[RESERVEMEM_DEV_MAX] = {
    {
	.id = RESERVEMEM_DEV_ETH,
	.name = "iMAP_Ethernet",

#ifdef CONFIG_IMAP_RESERVEMEM_SIZE_ETH
	.size = CONFIG_IMAP_RESERVEMEM_SIZE_ETH * SZ_1K,
#else
	.size = 0,
#endif
	.paddr = 0,
    },
    {
	.id	= RESERVEMEM_DEV_MEMALLOC,
	.name	= "memalloc",
#ifdef CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE
	.size	= CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE * SZ_1M,
#else
	.size = 0,
#endif	/* CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE */
	.paddr	= 0,
    },
    {
	.id	= RESERVEMEM_DEV_PMM,
	.name	= "imapx-pmm",
#ifdef CONFIG_PMM_RESERVE_MEM_SIZE_MB
	.size	= CONFIG_PMM_RESERVE_MEM_SIZE_MB * SZ_1M,
#else
	.size = 0,
#endif // CONFIG_PMM_RESERVE_MEM_SIZE_MB
	.paddr	= 0,
    },
};

static struct imap_reservemem_device *get_reservemem_device(int dev_id)
{
    struct imap_reservemem_device *dev = NULL;
    int i, found;

    if (dev_id < 0 || dev_id >= RESERVEMEM_DEV_MAX)
	return NULL;

    i = 0;
    found = 0;
    while (!found && (i < RESERVEMEM_DEV_MAX)) {
	dev = &reservemem_devs[i];
	if (dev->id == dev_id)
	    found = 1;
	else
	    i++;
    }

    if (!found)
	dev = NULL;

    return dev;
}

dma_addr_t imap_get_reservemem_paddr(int dev_id)
{
    struct imap_reservemem_device *dev;

    dev = get_reservemem_device(dev_id);
    if (!dev){
	printk(KERN_ERR "invalid device!\n");
	return 0;
    }

    if (!dev->paddr) {
	printk(KERN_ERR "no memory for %s\n", dev->name);
	return 0;
    }

    return dev->paddr;
}
EXPORT_SYMBOL(imap_get_reservemem_paddr);

size_t imap_get_reservemem_size(int dev_id)
{
    struct imap_reservemem_device *dev;

    dev = get_reservemem_device(dev_id);
    if (!dev){
	printk(KERN_ERR "invalid device!\n");
	return 0;
    }

    return dev->size;
}
EXPORT_SYMBOL(imap_get_reservemem_size);

void imap_mem_reserve(void)
{
    struct imap_reservemem_device *dev;
    int i, tmp=-1;

    for(i = 0; i < sizeof(reservemem_devs) / sizeof(reservemem_devs[0]); i++) {
        dev = &reservemem_devs[i];

        // we need aujust the order to put the item access function front of 
        // imap_mem_reserve(), else it has no any effect. 
        if(dev->id == RESERVEMEM_DEV_PMM){
            if(item_exist("pmm.reserve.size")){
                tmp = item_integer("pmm.reserve.size", 0);
                if((tmp >= 0) && (tmp <= 256)){
                    dev->size = tmp * SZ_1M;
                }
            }
            printk(KERN_INFO "pmm.reserve.size=%d, dev->size=%d\n", tmp, dev->size);
        }

        if (dev->size > 0) {
            dev->paddr = virt_to_phys(alloc_bootmem_low(dev->size));
            printk(KERN_INFO \
                    "imap: %luMB memory reserved for %s.\n",
                    (unsigned long) (dev->size >> 20),
                    dev->name);
        }
    }
}

EXPORT_SYMBOL(imap_mem_reserve);

