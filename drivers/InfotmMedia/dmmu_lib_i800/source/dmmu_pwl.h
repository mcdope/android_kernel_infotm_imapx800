/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.1.0	leo@2012/03/17: first commit.
** v1.2.2	leo@2012/04/20: add dmmupwl_flush_cache() function.
**
*****************************************************************************/ 

#ifndef __DMMU_PWL_H__
#define __DMMU_PWL_H__


//
// BASE must 4KB alignment.
//
#define DMMU_REG_SIZE		(0x1000)

#define G2D_MMU_BASE		(0x29000000)
#define G2D_MMU_OFFSET		(0x800)

#define IDS0_W0_MMU_BASE	(0x22006000)
#define IDS0_W0_MMU_OFFSET	(0x200)

#define IDS0_W1_MMU_BASE	(0x22006000)
#define IDS0_W1_MMU_OFFSET	(0x400)

#define IDS0_W2_MMU_BASE	(0x22006000)
#define IDS0_W2_MMU_OFFSET	(0x600)

#define IDS0_CBCR_MMU_BASE	(0x22006000)
#define IDS0_CBCR_MMU_OFFSET	(0x800)

#define IDS1_W0_MMU_BASE	(0x23006000)
#define IDS1_W0_MMU_OFFSET	(0x200)

#define IDS1_W1_MMU_BASE	(0x23006000)
#define IDS1_W1_MMU_OFFSET	(0x400)

#define IDS1_W2_MMU_BASE	(0x23006000)
#define IDS1_W2_MMU_OFFSET	(0x600)

#define IDS1_CBCR_MMU_BASE	(0x23006000)
#define IDS1_CBCR_MMU_OFFSET	(0x800)

#define VDEC_MMU_BASE		(0x25040000)
#define VDEC_MMU_OFFSET		(0x0)


IM_RET dmmupwl_init(void);
IM_RET dmmupwl_deinit(void);

void *dmmupwl_malloc(IM_INT32 size);
void dmmupwl_free(void *p);
void *dmmupwl_memcpy(void *dst, void *src, IM_INT32 size);
void *dmmupwl_memset(void *p, IM_CHAR c, IM_INT32 size);
IM_RET dmmupwl_alloc_linear_page_align_memory(OUT IM_Buffer *buffer);	// must noncache.
IM_RET dmmupwl_free_linear_page_align_memory(IN IM_Buffer *buffer);
void dmmupwl_flush_cache(void *virAddr, IM_UINT32 phyAddr, IM_INT32 size);

IM_RET dmmupwl_write_reg(IM_INT32 devid, IM_UINT32 addr, IM_UINT32 val);
IM_UINT32 dmmupwl_read_reg(IM_INT32 devid, IM_UINT32 addr);
IM_RET dmmupwl_write_regbit(IM_INT32 devid, IM_UINT32 addr, IM_INT32 bit, IM_INT32 width, IM_UINT32 val);

void dmmupwl_udelay(IM_INT32 us);

IM_RET dmmupwl_mmu_init(IM_INT32 devid);
IM_RET dmmupwl_mmu_deinit(IM_INT32 devid);

#endif	// __DMMU_PWL_H__

