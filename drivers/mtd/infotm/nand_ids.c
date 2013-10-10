/*
 *  drivers/mtd/nandids.c
 *
 *  Copyright (C) 2002 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/mtd/nand.h>

const struct nand_idt_table infotm_nand_idt[] = {
	{
		.name       = "H27UBG8T2BTR",
		.id         = { 0xad, 0xd7, 0x94, 0xda, 0x74, 0xc3, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 255,     
		.sysinfo    = 16,  
		//.timing     = 0x16212,   
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		.seed       = { 0x1a00, 0x3ffe, 0x2eee, 0x3fff,
			0x2aaa, 0x1555, 0xe5a5, 0x25a5 },
		//.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
		//	0x347e, 0x1698, 0x2c80, 0x3fff },
		.nand_param0  = 0x55,
		.nand_param1  = 26,
		.read_retry    = 1,
		.retry_level = 6,
		.max_correct_bits = 30, //return -UCLEAN for move block

	}, {
		.name       = "H27UCG8T2ATR",
		.id         = { 0xad, 0xde, 0x94, 0xda, 0x74, 0xc4, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0xec80, 0x7fff },
		.nand_param0  = 0x55,
		.nand_param1  = 20,
		.nand_param2  = 35,
		.read_retry    = 1,
		.retry_level = 7,
		.max_correct_bits = 34, //return -UCLEAN for move block

	}, {
		.name       = "H27UBG8T2CTR",
		.id         = { 0xad, 0xd7, 0x94, 0x91, 0x60, 0x44, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0xec80, 0x7fff },
		.nand_param0  = 0x55,
		.nand_param1  = 20,
		.nand_param2  = 36,
		.read_retry    = 1,
		.retry_level = 7,
		.max_correct_bits = 30, //return -UCLEAN for move block

	}, {
		.name       = "MT29F64G08CBABA",
		.id         = { 0x2c, 0x64, 0x44, 0x4b, 0xa9, 0, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 744,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 7,     /* 48 bit */  
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0xec80, 0xbfff },
		.nand_param0  = 0x75,
		.nand_param1  = 20,
		.read_retry    = 1,
		.retry_level = 7,
		.max_correct_bits = 36, //return -UCLEAN for move block
	}, {
		.name       = "SDTNQFAMA-004G",
		.id         = { 0x45, 0xd7, 0x84, 0x93, 0x72, 0x57, 0x08, 0x04 },
		.pagesize   = 0x4000,
		.oobsize    = 1280,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x400000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */  
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x747e, 0x1698, 0xec80, 0x7fff },
		.nand_param0  = 0x95,
		.nand_param1  = 20,
		.read_retry    = 1,
		.retry_level = 20,
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, {
		.name       = "SDTNQFAMA-008G",
		.id         = { 0x45, 0xde, 0x94, 0x93, 0x76, 0x57, 0x09, 0x04 },
		.pagesize   = 0x4000,
		.oobsize    = 1280,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x400000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */  
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x747e, 0x1698, 0xec80, 0x7fff },
		.nand_param0  = 0x95,
		.nand_param1  = 20,
		.read_retry    = 1,
		.retry_level = 20,
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, {
		.name       = "MT29F32G08CBABA",
		.id         = { 0x2c, 0x68, 0x04, 0x46, 0x89, 0, 0, 0 },
		.pagesize   = 0x1000,
		.oobsize    = 224,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /* 24 bit */  
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffa40,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0x2c80, 0x3fff },
		.max_correct_bits = 18, //return -UCLEAN for move block


	}, {
		.name       = "TC58NVG6DCJTA00",
		.id         = { 0x98, 0xde, 0x84, 0x93, 0x72, 0x57, 0, 0 },
		.pagesize   = 0x4000,
		.oobsize    = 1280,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x400000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 255,     
		.sysinfo    = 16,  
		.timing     = 0x6212,   
		.rnbtimeout = 0xfffa10,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 30, //return -UCLEAN for move block

	}, {
		.name       = "K9GBC08U0A",
		.id         = { 0xec, 0xd7, 0x94, 0x7a, 0x54, 0x43, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 127,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, {
		.name       = "K9GAG08U0E",
		.id         = { 0xec, 0xd5, 0x84, 0x72, 0x50, 0x42, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 436,
		.chipsize   = 2048,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /* 24 bits */
		.secclvl    = 1,  
		.bad0       = 0,       //
		.bad1       = 127,     //
		.sysinfo    = 16,  
		.timing     = 0x16323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 0,
		.poly	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 18, //return -UCLEAN for move block
	}, {
		.name       = "K9HDG08U1A",
		.id         = { 0xec, 0xde, 0xd5, 0x7a, 0x58, 0x43, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 16384,  /* Mega bytes */
		//.chipsize   = 12288,  /* Mega bytes */
		//.chipsize   = 10240,  /* Mega bytes */
		//.chipsize   = 8192,  /* Mega bytes */
		//.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 127,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 1,
		.poly	    = 0x994,	
		//.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
		//	0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0x2c80, 0x3fff },
		.chipnum    = 2,
		.chipmap[0] = 0,
		.chipmap[1] = 1,
		.single_chipsize = 8192, /* Mega bytes */
		.ddp	    = 1,
		.diesize    = 4096, /* Mega bytes */
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, {
		.name       = "K9GAG08U0M",
		.id         = { 0xec, 0xd5, 0x14, 0, 0, 0, 0, 0 },
		.pagesize   = 0x1000,
		.oobsize    = 128,
		.chipsize   = 2048,  /* Mega bytes */
		.erasesize  = 0x80000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 2,     /* 8 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 127,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 0,
		.poly	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 6, //return -UCLEAN for move block

	}, {
		.name       = "H27UBG8T2ATR",
		.id         = { 0xad, 0xd7, 0x94, 0x9a, 0x74, 0x42, 0, 0 },
		.pagesize   = 0x2000,
		.oobsize    = 436,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /*24 bit*/ 
		.secclvl    = 1,  
		.bad0       = 255,     
		.bad1       = 254,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,   
	        .randomizer = 0,
		.poly	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 18, //return -UCLEAN for move block

	}, {
		.name       = "TH58TVG7D2GBAxx",
		.id         = { 0x98, 0xde, 0x94, 0x82, 0x76, 0xd6, 0x01, 0x20 },
		.pagesize   = 0x2000,
		.oobsize    = 640,
		.chipsize   = 2048,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 2,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit*/ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 255,     
		.sysinfo    = 16,  
		.timing     = 0x51022222,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3636, 
		.busw       = 8,     
	        .randomizer = 1,
		.poly 	    = 0x994,	
		.seed       = { 0xfffe, 0xda00, 0xffff, 0xeeee,
			0x5555, 0xaaaa, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, 
	{
		.name       = "K9F1G08U0B",
		.id         = { 0xec, 0xf1, 0x00, 0x95, 0x00, 0, 0, 0 },
		.pagesize   = 0x800,
		.oobsize    = 64,
		.chipsize   = 2048,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,     /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 1023,     
		.sysinfo    = 16,  
		.timing     = 0x51022222,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x0700, 
		.busw       = 8,     
	        .randomizer = 0,
		.poly 	    = 0x994,	
		.seed       = { 0xfffe, 0xda00, 0xffff, 0xeeee,
			0x5555, 0xaaaa, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, 	
	
	{
		.name       = "MT29F64G08CBAAB",
		.id         = {0x2C, 0x88, 0x04, 0x4b, 0xa9, 0, 0, 0},
		.pagesize   = 0x2000,
		.oobsize    = 448,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /* 24 bit */  
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 0,     
		.sysinfo    = 16,  
		//.timing     = 0x1ff7b,   
		.timing     = 0x18323,   
		//.timing     = 0x6312,   
		//.timing     = 0x6211,   
		//.timing     = 0x7322,   
		//.timing     = 0x17423,   
		//.rnbtimeout = 0xfffaf0,
		.rnbtimeout = 0xfffa10,
		.phyread    = 0x4023,  
		.phydelay   = 0x3636, 
		.busw       = 8,     
	        .randomizer = 1,
		.poly 	    = 0x994,	
		.seed       = { 0x1a00, 0x3ffe, 0x2eee, 0x3fff,
			0x2aaa, 0x1555, 0x25a5, 0x25a5 },
		.active_async = 1,
		.max_correct_bits = 18, //return -UCLEAN for move block
	}, 	
	{
		.name       = "MT29F8G08ABABA",
		.id         = {0x2C, 0x38, 0x00, 0x26, 0x85, 0, 0, 0},
		.pagesize   = 0x1000,
		.oobsize    = 224,
		.chipsize   = 2048,  /* Mega bytes */
		.erasesize  = 0x80000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /* 24 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 127,     
		.sysinfo    = 16,  
		.timing     = 0x31022222,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x5134,  
		.phydelay   = 0x3818, 
		.busw       = 8,     
	        .randomizer = 0,
		.poly 	    = 0x994,	
		.seed       = { 0xfffe, 0xda00, 0xffff, 0xeeee,
			0x5555, 0xaaaa, 0xa5a5, 0xa5a5 },
		.max_correct_bits = 18, //return -UCLEAN for move block
	},
	{
		.name       = "MT29F32G08CBACA",
		.id         = {0x2c, 0x68, 0x04, 0x4a, 0xa9, 0, 0, 0},
		.pagesize   = 0x1000,
		.oobsize    = 224,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 4,     /* 24 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 255,     
		.sysinfo    = 16,  
		.timing     = 0x18323,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x5134,  
		.phydelay   = 0x3818, 
		.busw       = 8,     
	        .randomizer = 1,
		.poly 	    = 0x994,	
		.seed       = { 0xda00, 0xfffe, 0xeeee, 0xffff,
			0xaaaa, 0x5555, 0xa5a5, 0xa5a5 },
		.active_async = 1,
		.max_correct_bits = 18, //return -UCLEAN for move block
	}, 	
	{
		.name       = "K9GBG08U0B",
		.id         = {0xEC, 0xD7, 0x94, 0x7E, 0x64, 0x44, 0, 0},
		.pagesize   = 0x2000,
		.oobsize    = 1024,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,    /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 127,     
		.sysinfo    = 16,  
		.timing     = 0x18324,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x5134,  
		.phydelay   = 0x3818, 
		.busw       = 8,     
	        .randomizer = 1,
		.poly 	    = 0x994,	
		//.seed       = { 0xfffe, 0xda00, 0xffff, 0xeeee,
		//	0x5555, 0xaaaa, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0xec80, 0x3fff },
		.nand_param0  = 0x55,
		.nand_param1  = 21,
		.read_retry    = 1,
		.retry_level = 14,
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, 	
	{
		.name       = "K9LCG08U0B",
		.id         = {0xEC, 0xDE, 0xD5, 0x7E, 0x68, 0x44, 0, 0},
		.pagesize   = 0x2000,
		.oobsize    = 1024,
		.chipsize   = 8192,  /* Mega bytes */
		.erasesize  = 0x100000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.mecclvl    = 6,    /* 40 bit */ 
		.secclvl    = 1,  
		.bad0       = 0,     
		.bad1       = 127,     
		.sysinfo    = 16,  
		.timing     = 0x18324,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x5134,  
		.phydelay   = 0x3818, 
		.busw       = 8,     
	        .randomizer = 1,
		.poly 	    = 0x994,	
		//.seed       = { 0xfffe, 0xda00, 0xffff, 0xeeee,
		//	0x5555, 0xaaaa, 0xa5a5, 0xa5a5 },
		.seed       = { 0x3550, 0x1c72, 0x074e, 0x20fe,
			0x347e, 0x1698, 0xec80, 0x3fff },
		.nand_param0  = 0x55,
		.nand_param1  = 21,
		.read_retry    = 1,
		.retry_level = 14,
		.max_correct_bits = 30, //return -UCLEAN for move block
	}, 	
	{
		.name       = "tcgvrPBA",
		.id         = {0x98, 0xde, 0x94, 0x82, 0xf4, 0x56, 1, 0},
		.pagesize   = 0x2000,
		.oobsize    = 32,
		.chipsize   = 4096,  /* Mega bytes */
		.erasesize  = 0x200000,
		.interface  = 0,     /* legacy=0, onfi_sync=1, toggle=2 */
		.cycle      = 5,     /* unlikely(4) */
		.bad0       = 0,     
		.bad1       = 255,     
		.sysinfo    = 16,  
		.timing     = 0x51022222,   
		.rnbtimeout = 0xfffaf0,
		.phyread    = 0x4023,  
		.phydelay   = 0x3818, 
		.busw       = 8,     
		.randomizer = 0,
	}, 	
	{
		.pagesize = 0x0, //end
	},

};

EXPORT_SYMBOL(infotm_nand_idt);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("Nand device & manufacturer IDs");

