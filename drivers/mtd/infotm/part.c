/***************************************************************************** 
** XXX driver/mtd/infotm/imapx800.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: iMAP NAND Flash platform driver.
**				TODO: Science spare ECC is used, read_oob, wirte_oob
**					should be applied to ensure the validity of OOB.
**
** Author:
**     warits   <warits.wang@infotmic.com.cn>
**     jay      <jay.hu@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.0  XXX 03/23/2012 XXX	file and function structures, by warits
** 1.0  XXX 03/23/2012 XXX	detailed implementation, by jay
*****************************************************************************/


#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/imapx_nand.h>
#include <mach/pad.h>
#include <mach/power-gate.h>
#include <mach/items.h>

static struct mtd_partition imap_nand_parts[] = {
	{
		.name       = "reserved",
		.offset     = 0  * SZ_1M,
		.size       = 112 * SZ_1M,
	}, {
		.name       = "system",
		.offset     = MTDPART_OFS_APPEND,
		.size       = 152 * SZ_1M,
	}, {
		.name       = "misc",
		.offset     = MTDPART_OFS_APPEND,
		.size       = 20 * SZ_1M,
	}, {
		.name       = "cache",
		.offset     = MTDPART_OFS_APPEND,
		.size       = 128 * SZ_1M,
	}, {
		.name       = "userdata",
		.offset     = MTDPART_OFS_APPEND,
		.size       = 164 * SZ_1M,
	}, {
		.name       = "local",
		.offset     = MTDPART_OFS_APPEND,
		.size       = MTDPART_SIZ_FULL,
	}
};

struct mtd_partition *
imap_auto_part(int *num) {
	char *parts[] = { "reserved", "part.system",
		"part.misc", "part.cache",
		"part.userdata", "part.local" };
	int i;

	for(i = 0; i < ARRAY_SIZE(parts); i++) {
		if(item_exist(parts[i])) {
			int64_t a = item_integer(parts[i], 0);
			if(a > 0)
			  imap_nand_parts[i].size = a * SZ_1M;
		}
	}
	
	*num = ARRAY_SIZE(imap_nand_parts);

	return imap_nand_parts;
}

EXPORT_SYMBOL(imap_auto_part);

