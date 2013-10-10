/*
 *  drivers/mtd/infotm/nand_mtd.c
 *
 *  Overview:
 *   This is the generic MTD driver for NAND flash devices. It should be
 *   capable of working with almost all NAND chips currently available.
 *   Basic support for AG-AND chips is provided.
 *
 *	Additional technical information is available on
 *	http://www.linux-mtd.infradead.org/doc/nand.html
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
 *		  2002-2006 Thomas Gleixner (tglx@linutronix.de)
 *
 *  Credits:
 *	David Woodhouse for adding multichip support
 *
 *	Aleph One Ltd. and Toby Churchill Ltd. for supporting the
 *	rework for 2K page size chips
 *
 *  TODO:
 *	Enable cached programming for 2k page size chips
 *	Check, if mtd->ecctype should be set to MTD_ECC_HW
 *	if we have HW ecc support.
 *	The AG-AND chips have nice features for speed improvement,
 *	which are not supported yet. Read / program 4 pages in one go.
 *	BBT table is not serialized, has to be fixed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/nand_bch.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/leds.h>
#include <linux/io.h>
#include <linux/mtd/partitions.h>
#include <linux/crc32.h>
#include <mach/items.h>
#include <mach/pad.h>

#include <mach/mem-reserve.h>
#include "imapx800.h"

#define BBT_PAGE_MASK	0xffffff3f
#define CDBG_NAND_ID_COUNT (8)

#define BBT_BLOCK_OFS	0x10
#define BBT_MAGIC	(0x56d5a58d)
#define ERASE_WRITE_BUG (16)
#define BBT_OVER_BLOCK	(8)

//#define INFOTM_NAND_DEBUG

unsigned int g_retrylevel;
unsigned int g_retrylevel_chip1;

unsigned char * g_otp_buf;
void    __iomem     *sys_rtc_regs;
void    *resved_mem;
unsigned char    *resved_mem_u8;
unsigned char	retry_reg_buf[8];
unsigned char   eslc_reg_buf[8];

#define RETRYLEVEL_COUNT_DEBUG  (0)

#if RETRYLEVEL_COUNT_DEBUG
unsigned int    retrylevel_count[30];
unsigned int    readpage_count;
#endif
struct nand_config nf_cfg;

static struct nand_ecclayout infotm_oob_layout = {
	.oobfree = { {
		.offset = 4,
		.length = 20}, },
	.oobavail = 20
};

struct nand_bad_block_uint{

	loff_t ofs;
	int bad_mark;
	int reserved;
	       
};

struct nand_bad_block_info{
	struct nand_bad_block_uint uint[32767];	
	int max_blk_num;
	int bad_mark_magic;
	int reserved[2];
};

struct nand_bad_block_info *g_bbt;
//struct nand_bad_block_info gt_bbt;

spinlock_t infotm_mtd_lock;
wait_queue_head_t infotm_mtd_wq;
flstate_t infotm_mtd_state;
spinlock_t nand_mtd_lock;
static int m_lock;
int power_index, cs1_index, cs2_index, cs3_index;

static int nand_block_isbad(struct mtd_info *mtd, loff_t offs);
static int nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, uint8_t *buf);
static int nand_erase(struct mtd_info *mtd, struct erase_info *instr);
static int nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const uint8_t *buf);
static int nand_do_read_ops(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops);
static int nand_do_write_ops(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops);
static int nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
			       int allowbbt);
static int erase_write_bug(void );

static int write_erase = 0;

/*for write or erase over 10 block, get panic*/
static int erase_write_bug(void )
{

	write_erase++;
		
	if(write_erase >= ERASE_WRITE_BUG)
	{	
		printk(KERN_ERR "\n write_erase faild over 10 blocks, panic!!\n");
		BUG();
	}
}

int infotm_nand_lock(void){

	spin_lock(&nand_mtd_lock);
	m_lock = 1;
	spin_unlock(&nand_mtd_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(infotm_nand_lock);

int infotm_nand_unlock(void){

	spin_lock(&nand_mtd_lock);
	m_lock = 0;
	spin_unlock(&nand_mtd_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(infotm_nand_unlock);

int infotm_nand_wp(void){
	/*add for sys shut down*/
	int lock_state = 0;

	while(1){
		if(m_lock == 1){
			printk("nand m_lock = 1\n");
			msleep(100);
		}else{
			if(lock_state == 1){
				lock_state = 0;
				printk("nand m_lock = 0\n");
			}
			break;
		}
	}
	return 0;	
}

int update_bbt(loff_t ofs){

	int mark_bad;
	int bbt_num;
	int j;
	
	mark_bad = 1;
	bbt_num = g_bbt->max_blk_num;
	for(j=0; j<bbt_num; j++){
		if(g_bbt->uint[j].ofs == ofs){
			mark_bad = 0;
			break;
		}
	}
	if(mark_bad == 1){
		g_bbt->uint[bbt_num].ofs = ofs;
		g_bbt->uint[bbt_num].bad_mark = 1;
		g_bbt->max_blk_num = bbt_num + 1;
		printk("======badblock 0x%llx======\n", ofs);
	}

	return 0;
}

int table_need_block(struct mtd_info *mtd){

	struct nand_config *nand_cfg = mtd->priv;
	int single_table_size;
	int need_blk;
	int i, j;

	single_table_size = sizeof(struct nand_bad_block_info);
	printk("table_size = %d\n", single_table_size);
	i=1;
	j=1;

	while(1){
		if((j*single_table_size*8) == (i*(nand_cfg->blocksize))){
			break;
		}

		if((j*single_table_size*8) > (i*(nand_cfg->blocksize))){
			i++;
		}else{
			j++;
		}
	}

	if(i>j){
		need_blk = i/j;
	}else{
		need_blk = 1;	
	}

	return need_blk;
}

int get_bad_block_table(struct mtd_info *mtd, struct nand_bad_block_info *table_buf){

	struct nand_config *nand_cfg = mtd->priv;
	loff_t ofs, page_ofs;
	int len;
	int retlen;
	int blk_num;
	int ret = 0;
	int i = 0;
	uint32_t crc;
	struct mtd_oob_ops ops;

	len = sizeof(struct nand_bad_block_info);
	ofs = mtd->size - mtd->erasesize;

	while(nand_block_checkbad(mtd, ofs, 1, 1)){
		ofs -= mtd->erasesize;
	}

	blk_num = ofs >> (nand_cfg->block_shift);
	page_ofs = ofs;
	for(i=0; i<8; i++){
		if((page_ofs >> (nand_cfg->block_shift)) != blk_num){
			ofs -= mtd->erasesize;
			while(nand_block_checkbad(mtd, ofs, 1, 1)){
				ofs -= mtd->erasesize;
			}
			page_ofs = ofs;
			blk_num = ofs >> (nand_cfg->block_shift);
		}

		ops.len    = len;
		ops.datbuf = (uint8_t *)table_buf;
		ops.oobbuf = NULL;
		ops.mode = MTD_OOB_AUTO;

		printk("read bbt %llx\n", page_ofs);
		ret = nand_do_read_ops(mtd, page_ofs, &ops);

		retlen   = ops.retlen;
		//ret = nand_read(mtd, page_ofs, len, &retlen, (uint8_t *)&table_buf);
		crc = crc32(~0, (u8 *) table_buf, len - 8);
#ifdef INFOTM_NAND_DEBUG
		printk("crc = 0x%x, ori crc = 0x%x\n", crc, table_buf->reserved[0]);
		printk("ret = %x, retlen = 0x%x\n", ret, retlen);
#endif
		if((ret != 0) || (len != retlen)){
			page_ofs += len;
		}else{
			if((table_buf->bad_mark_magic == BBT_MAGIC) && (crc == table_buf->reserved[0])){
				return 0;
			}else{
				page_ofs += len;
				g_bbt->max_blk_num = 0;
				g_bbt->bad_mark_magic = 0;
				printk("BBT MAGIC 0x%x != 0x%x\n", table_buf->bad_mark_magic, BBT_MAGIC);
			}
		}	
	}

	return -1;
}

int bad_block_table_erase(struct mtd_info *mtd){

	struct nand_config *nand_cfg = mtd->priv;
	int i = 0, j = 0;
	int len;
	int need_block;
	int ret;
	int bbt_state;
	struct erase_info instr;
	struct mtd_oob_ops ops;
	loff_t offs;

	bbt_state = nand_cfg->bbt;
	need_block = table_need_block(mtd);
	len = sizeof(struct nand_bad_block_info);
	printk("table_size = %d, need_block = %d\n", len, need_block);
	offs = mtd->size - mtd->erasesize;
	
	printk("offs = %llx\n", offs);
	
	nand_cfg->bbt = 0;
	for(i=0; i<need_block; i++){
		

		while(nand_block_checkbad(mtd, offs, 1, 1)){
			offs -= mtd->erasesize;
		}

		update_bbt(offs);
		
		instr.addr = offs;
		instr.len = mtd->erasesize;
		ret = nand_erase_nand(mtd, &instr, 0);
		printk("bbt erase 0x%llx\n", offs);
		if(ret != 0){
		//TODO
			need_block += 1;
			/*bbt block account over 10block, do not creat bbt!*/	
			if (need_block >= BBT_OVER_BLOCK)
				return -EIO;
	
#ifdef INFOTM_NAND_DEBUG
			printk("==bbt erase failed 0x%llx==\n", offs);
			printk("==mark 0x%llx bad==\n", offs);
#endif
			imapx800_nand_block_markbad(mtd, offs + nand_cfg->badpagemajor * nand_cfg->pagesize);
			if(nand_cfg->badpageminor != nand_cfg->badpagemajor){
				imapx800_nand_block_markbad(mtd, offs + nand_cfg->badpageminor * nand_cfg->pagesize);
			}
			//read back check marking, if marked update bbt
			if(nand_block_checkbad(mtd, offs, 1, 0) != 0){
				printk("mark bad block 0x%llx\n", offs);
				update_bbt(offs);
				printk("update bbt\n");
			}
		}
		
		offs -= mtd->erasesize;
	}
	nand_cfg->bbt = bbt_state;
	return 0;
}

int bad_block_table_write(struct mtd_info *mtd, struct nand_bad_block_info *table){

	struct nand_config *nand_cfg = mtd->priv;
	int i = 0;
	int blk_num;
	int len, retlen;
	int ret = 0;
	loff_t ofs, page_ofs;
	struct mtd_oob_ops ops;
	int bbt_state;

	bbt_state = nand_cfg->bbt;
	nand_cfg->bbt = 0;
	len = sizeof(struct nand_bad_block_info);
	ofs = mtd->size - mtd->erasesize;
	while(nand_block_checkbad(mtd, ofs, 1, 1)){
		ofs -= mtd->erasesize;
	}
			
	g_bbt->bad_mark_magic = BBT_MAGIC;
	g_bbt->reserved[0] = crc32(~0, (u8 *) table, len - 8);	
	printk("write crc32 = 0x%x\n", g_bbt->reserved[0]);
	blk_num = ofs >> (nand_cfg->block_shift);
	page_ofs = ofs;	
	for(i=0; i<8; i++){
		if((page_ofs >> (nand_cfg->block_shift)) != blk_num){
			ofs -= mtd->erasesize;
			while(nand_block_checkbad(mtd, ofs, 1, 1)){
				ofs -= mtd->erasesize;
			}
			page_ofs = ofs;
			blk_num = ofs >> (nand_cfg->block_shift);
		}

		ops.len = len;
		ops.datbuf = (uint8_t *)table;
		ops.oobbuf = NULL;
		ops.mode = MTD_OOB_AUTO;

		ret = nand_do_write_ops(mtd, page_ofs, &ops);

		retlen = ops.retlen;
		//ret = nand_write(mtd, page_ofs, len, &retlen, (uint8_t*)&table);
		if((ret == 0) && (len == retlen)){
			printk("table write 0x%llx\n", page_ofs);
		}else{
			//TODO: write next back pages
			printk("create table write failed 0x%llx\n", page_ofs);
		}
		page_ofs += len;
	}
	nand_cfg->bbt = bbt_state;

	return 0;
}

int create_bad_block_table(struct mtd_info *mtd, struct nand_bad_block_info *table){

	int ret = 0;

	ret = bad_block_table_erase(mtd);
	if (ret != 0)
	{
		printk(KERN_ALERT "bbt error over 10 block, do not create bbt, read bbt from DRAM!");
		return 0;
	}

	bad_block_table_write(mtd, table);
	
#if 0
	struct nand_config *nand_cfg = mtd->priv;
	int need_block;
	int i, j;
	int len;
	int ret;
	int retlen;
	int bbt_num;
	int blk_num;
	int mark_bad;
	loff_t ofs, offs, page_ofs;
	struct erase_info instr;
	struct mtd_oob_ops ops;

	need_block = table_need_block(mtd);
	
	len = sizeof(struct nand_bad_block_info);
	printk("table_size = %d, need_block = %d\n", len, need_block);
	ofs = mtd->size - mtd->erasesize;
	offs = ofs;	
	
	printk("ofs = %llx, offs = %llx\n", ofs, offs);
	for(i=0; i<need_block; i++){
		mark_bad = 1;
		while(nand_block_checkbad(mtd, offs, 1, 0)){
			offs -= mtd->erasesize;
		}
		bbt_num = g_bbt->max_blk_num;
		for(j=0; j<bbt_num; j++){
			if(g_bbt->uint[j].ofs == offs){
				mark_bad = 0;
				break;
			}
		}
		if(mark_bad == 1){
			g_bbt->uint[bbt_num].ofs = offs;
			g_bbt->uint[bbt_num].bad_mark = 1;
			g_bbt->max_blk_num = bbt_num + 1;
			g_bbt->bad_mark_magic = BBT_MAGIC;
			printk("bbt_blk 0x%llx\n", offs);
		}
		offs -= mtd->erasesize;
	}

	while(nand_block_checkbad(mtd, ofs, 1, 0)){
		ofs -= mtd->erasesize;
	}

	blk_num = ofs >> (nand_cfg->block_shift);
	page_ofs = ofs;
	instr.addr = ofs;
	instr.len = mtd->erasesize;
	//nand_erase(mtd, &instr);
	ret = nand_erase_nand(mtd, &instr, 0);
	printk("table erase 0x%llx\n", instr.addr);

	for(i=0; i<8; i++){
		if((page_ofs >> (nand_cfg->block_shift)) != blk_num){
			ofs -= mtd->erasesize;
			while(nand_block_checkbad(mtd, ofs, 1, 0)){
				ofs -= mtd->erasesize;
			}
			page_ofs = ofs;
			blk_num = ofs >> (nand_cfg->block_shift);
			instr.addr = ofs;
			instr.len = mtd->erasesize;
			//ret = nand_erase(mtd, &instr);
			ret = nand_erase_nand(mtd, &instr, 0);
			printk("table erase 0x%llx\n", instr.addr);
			if(ret != 0){
				printk("create table erase failed 0x%llx\n", ofs);
			}
		}

		ops.len = len;
		ops.datbuf = (uint8_t *)table;
		ops.oobbuf = NULL;
		ops.mode = MTD_OOB_AUTO;

		ret = nand_do_write_ops(mtd, page_ofs, &ops);

		retlen = ops.retlen;
		//ret = nand_write(mtd, page_ofs, len, &retlen, (uint8_t*)&table);
		if((ret == 0) && (len == retlen)){
			printk("table write 0x%llx\n", page_ofs);
			page_ofs += len;
		}else{
			printk("create table write failed 0x%llx\n", page_ofs);
		}
	}
#endif
	return 0;
}

int bbt_table_create(struct mtd_info *mtd){

	struct nand_config *nand_cfg = mtd->priv;
	loff_t ofs, mtd_size;

	nand_get_device(mtd, FL_READING);
	
	if(get_bad_block_table(mtd, g_bbt) == 0){
		nand_cfg->bbt = 1;
		printk("BBT_MAGIC match 0x%x\n", g_bbt->bad_mark_magic);
	}else{
		ofs = 0ll;
		mtd_size = mtd->size;
		while(mtd_size){
			//printk("ofs = 0x%llx\n", ofs);
			nand_block_checkbad(mtd, ofs, 1, 1);
			//printk("%d\n", __LINE__);
			ofs += mtd->erasesize;
			mtd_size -= mtd->erasesize;
		}
		create_bad_block_table(mtd, g_bbt);
		nand_cfg->bbt = 1;
		printk("create bbt ok\n");
	}
	
	nand_release_device(mtd);

	return 0;
}

static int check_offs_len(struct mtd_info *mtd,
		loff_t ofs, uint64_t len)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret = 0;

	/* Start address must align on block boundary */
	if (ofs & ((1 << nand_cfg->block_shift) - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __func__);
		ret = -EINVAL;
	}

	/* Length must align on block boundary */
	if (len & ((1 << nand_cfg->block_shift) - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Length not block aligned\n",
					__func__);
		ret = -EINVAL;
	}

	/* Do not allow past end of device */
	if (ofs + len > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Past end of device\n",
					__func__);
		ret = -EINVAL;
	}

	return ret;
}

/**
 * nand_select_chip - [DEFAULT] control CE line
 * @mtd:	MTD device structure
 * @chipnr:	chipnumber to select, -1 for deselect
 *
 * Default select function for 1 chip devices.
 */
static void nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	/* TODO: -1 de-select all chips
	 * 0: select the 0 chip
	 * 1: select the 1 chip
	 */

	return ;
}

/**
 * nand_check_wp - [GENERIC] check if the chip is write protected
 * @mtd:	MTD device structure
 * Check, if the device is write protected
 *
 * The function expects, that the device is already selected
 */
static int nand_check_wp(struct mtd_info *mtd)
{
	uint32_t status = 0;

	/* Check the WP bit */
	imapx800_nand_read_status(mtd, &status);
	//chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	return (status & NAND_STATUS_WP) ? 0 : 1;
}

/**
 * nand_block_bad - [DEFAULT] Read bad block marker from the chip
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int page, ret = 0;
	int chipnr, chip_page, die_page;
	struct nand_config *nand_cfg = mtd->priv;
		
	chipnr = (int)(ofs >> nand_cfg->chip_shift);
	page = (int)(ofs >> nand_cfg->page_shift);
	chip_page = page & (nand_cfg->pagemask);
	die_page = chip_page & (nand_cfg->die_pagemask);
	if(nand_cfg->ddp == 1){
		if(chip_page >= (nand_cfg->pages_pre_die)){
			die_page = die_page | 0x100000;
		}
	}
	nand_cfg->chipnr = chipnr;
	
#if 1
#if NAND_BADBLOCK_MAJOR
	ret = imapx800_nand_get_badmark(mtd, die_page + nand_cfg->badpagemajor);
	if(ret != 0){
		return 1;
	}
#endif
	
#if NAND_BADBLOCK_MINOR
	ret = imapx800_nand_get_badmark(mtd, die_page + nand_cfg->badpageminor);
	if(ret != 0){
		return 1;
	}
#endif
#endif
	//printk("ret = 0x%x\n", ret);
	return ret;
}

/**
 * nand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 * @allowbbt:	1, if its allowed to access the bbt area
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
static int nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
			       int allowbbt)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret = 0;
	int retrylevel = 0;
	int i = 0;

	nand_cfg->chipnr = (int)(ofs >> nand_cfg->chip_shift);
	if(nand_cfg->chipnr == 0){
		retrylevel = g_retrylevel;
	}else{
		retrylevel = g_retrylevel_chip1;
	}

	if (!nand_cfg->bbt){
retry_5_start:
		//printk("%d\n", __LINE__);
		ret = nand_block_bad(mtd, ofs, getchip);
		//printk("%d\n", __LINE__);

		if(ret > 0){
			if(nand_cfg->read_retry == 1){
				retrylevel++;
				if(retrylevel > nand_cfg->retry_level){
					retrylevel = 0;
				}
				if(nand_cfg->chipnr == 0){
					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
						imap_set_retry_param_26(mtd, retrylevel, g_otp_buf);

					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
						imap_set_retry_param_20(mtd, retrylevel, g_otp_buf, retry_reg_buf);

					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
						imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

					if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
							imap_set_retry_param_micro_20(mtd, retrylevel);
					
					if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
							imap_set_retry_param_sandisk19(mtd, retrylevel);
					
					if(g_retrylevel == retrylevel){
						printk("======retry badblock 0x%llx======\n", ofs);
						goto retry_5_end;
					}else{
						(mtd->ecc_stats.failed)--;
						goto retry_5_start;
					}
				}
				else if(nand_cfg->chipnr == 1){
					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
							imap_set_retry_param_26(mtd, retrylevel, &g_otp_buf[10]);
					
					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
							imap_set_retry_param_20(mtd, retrylevel, &g_otp_buf[1024], retry_reg_buf);
					
					if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
						imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

					if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
							imap_set_retry_param_micro_20(mtd, retrylevel);
					
					if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
						imap_set_retry_param_sandisk19(mtd, retrylevel);
					
					if(g_retrylevel_chip1 == retrylevel){
						printk("======retry badblock 0x%llx======\n", ofs);
						goto retry_5_end;
					}else{
						(mtd->ecc_stats.failed)--;
						goto retry_5_start;
					}
				}
			}
		}

		//TODO
		if(retrylevel != 0 && nand_cfg->read_retry == 1){
retry_5_end:
			retrylevel = 0;
			g_retrylevel = 0;
			g_retrylevel_chip1 = 0;

			if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26){
				if(nand_cfg->chipnr == 0){
					imap_set_retry_param_26(mtd, retrylevel, g_otp_buf);
				}else{
					imap_set_retry_param_26(mtd, retrylevel, &g_otp_buf[10]);
				}
			}
			if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20){
				if(nand_cfg->chipnr == 0){
					imap_set_retry_param_20(mtd, retrylevel, g_otp_buf, retry_reg_buf);
				}else{
					imap_set_retry_param_20(mtd, retrylevel, &g_otp_buf[1024], retry_reg_buf);
				}
			}        
			if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
				imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

			if(nand_cfg->nand_param0 == 0x75  && nand_cfg->nand_param1 == 20)
				imap_set_retry_param_micro_20(mtd, retrylevel);

			if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
				imap_set_retry_param_sandisk19(mtd, retrylevel);
		}

		g_retrylevel = 0;
		g_retrylevel_chip1 = 0;

	}	//no bbt end
	else{
		for(i=0; i<(g_bbt->max_blk_num); i++){
			if(g_bbt->uint[i].ofs == ofs){
				printk("bad 0x%llx from bbt\n", ofs);
				return 1;
			}
		}
	}

	if(allowbbt == 1){
		if(ret != 0){
			update_bbt(ofs);
		}
	}
	//printk("%d\n", __LINE__);
	/* Return info from the table */
	return ret;//nand_isbad_bbt(mtd, ofs, allowbbt);
}

/**
 * nand_get_device - [GENERIC] Get chip for selected access
 * @chip:	the nand chip descriptor
 * @mtd:	MTD device structure
 * @new_state:	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
	int
nand_get_device(struct mtd_info *mtd, int new_state)
{
	spinlock_t *lock = &infotm_mtd_lock;
	wait_queue_head_t *wq = &infotm_mtd_wq;
	DECLARE_WAITQUEUE(wait, current);
retry:
	spin_lock(lock);

	if (infotm_mtd_state == FL_READY) {
		infotm_mtd_state = new_state;
		spin_unlock(lock);
		return 0;
	}
	if (new_state == FL_PM_SUSPENDED) {
		if (infotm_mtd_state == FL_PM_SUSPENDED) {
			spin_unlock(lock);
			return 0;
		}
	}
	set_current_state(TASK_UNINTERRUPTIBLE);
	add_wait_queue(wq, &wait);
	spin_unlock(lock);
	schedule();
	remove_wait_queue(wq, &wait);
	goto retry;
}


/**
 * nand_release_device - [GENERIC] release chip
 * @mtd:	MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
void nand_release_device(struct mtd_info *mtd)
{
	/* De-select the NAND device */
	nand_select_chip(mtd, -1);

	/* Release the controller and the chip */
	spin_lock(&infotm_mtd_lock);
	infotm_mtd_state = FL_READY;
	wake_up(&infotm_mtd_wq);
	spin_unlock(&infotm_mtd_lock);
}


/**
 * nand_do_read_ops - [Internal] Read data with ECC
 *
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob ops structure
 *
 * Internal function. Called with chip held.
 */

static int last_chipnr = 0, last_die_page = 0; 
static int buf_en_flag = 0;

static int nand_do_read_ops(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	/* TODO: read data, put the result length into *retlen
	 * must update ecc status here
	 *
	 * if ecc error happened: mtd->ecc_stats.failed increases
	 * if ecc happened: mtd->ecc_stats.corrected increases
	 */
	/* return -EBADMSG if ecc error happened
	 * return -EUCLEAN if ecc corrections happened
	 * return -ENOTSUPP if mode is not recognized
	 * return -ETIME if timeout, maybe READ IO ERROR
	 * return 0 if performed a perfect reading
	 */
	int chipnr, page, realpage, col = 0, bytes = 0, oobbytes = 0, aligned = 0, alignedoob = 0;
	struct nand_config *nand_cfg = mtd->priv;
	struct mtd_ecc_stats stats;
	//int blkcheck = (1 << (nand_cfg->block_shift - nand_cfg->page_shift)) - 1;
	int ret = 0;
	uint32_t readlen = ops->len;
	uint32_t oobreadlen = ops->ooblen;
	//uint32_t max_oobsize = ops->mode == MTD_OOB_AUTO ? mtd->oobavail : mtd->oobsize;

	uint8_t *bufpoi, *bufoob, *oob, *buf;
	uint32_t retrylevel = 0;
	int chip_page, die_page;
	int last_byte;
        int i = 0;

	stats = mtd->ecc_stats;

	buf = ops->datbuf;
	oob = ops->oobbuf;

	while (1) {

		chipnr = (int)(from >> nand_cfg->chip_shift);
		page = (int)(from >> nand_cfg->page_shift);
		chip_page = page & (nand_cfg->pagemask);
		die_page = chip_page & (nand_cfg->die_pagemask);
		if(nand_cfg->ddp == 1){
			if(chip_page >= (nand_cfg->pages_pre_die)){
				die_page = die_page | 0x100000;
			}
		}
		nand_cfg->chipnr = chipnr;

		col = from & (nand_cfg->pagesize - 1);
		last_byte = nand_cfg->pagesize - col;

		if(nand_cfg->chipnr == 0){
			retrylevel = g_retrylevel;
		}else{
			retrylevel = g_retrylevel_chip1;
		}
		if(buf != NULL){
			bytes = min(mtd->writesize, readlen);;
			bytes = min(last_byte, bytes);
			aligned = (bytes == mtd->writesize);
			if(imap_nand_valied_vm((uint32_t)buf, bytes)!=0){
				aligned = 0;	
			}
		}

		if(oob != NULL){
			oobbytes = min(mtd->oobsize, oobreadlen);
			alignedoob = (oobbytes == mtd->oobavail); 
			if(imap_nand_valied_vm((uint32_t)oob, oobbytes + 4)!=0){
				alignedoob = 0;	
			}
		}
		if((oobbytes != 16 || alignedoob) && (oob != NULL)){
			printk("oobbytes = 0x%x, alignedoob =%d, oob=0x%x\n", oobbytes, alignedoob, oob);
		}

		/* Is the current page in the buffer ? */
		if (buf || oob) {
			if(last_chipnr == chipnr && last_die_page == die_page && buf_en_flag == 1)
			{
				//get data from buffer, improve read speed
			}
			else
			{
				last_chipnr = chipnr;
				last_die_page = die_page;			
			if(buf != NULL){
				bufpoi = aligned ? buf : (nand_cfg->buffers->databuf);
			}else{
				bufpoi = NULL;
			}
			alignedoob = 0;
			if(oob != NULL){
				bufoob = alignedoob ? oob : (nand_cfg->oob_poi);
				//bufoob = (uint8_t *)((int)(bufoob) & (int)(~0xf));
			}else{
				bufoob = NULL;
				bufoob = nand_cfg->oob_poi;
				oobbytes = 16;
			}

			if(buf != NULL){	
				if (ops->mode == MTD_OOB_AUTO){
					/*read page and sysinfo*/
					//retrylevel = g_retrylevel;
					if(nand_cfg->chipnr == 0){
						retrylevel = g_retrylevel;
					}else{
						retrylevel = g_retrylevel_chip1;
					}
#if RETRYLEVEL_COUNT_DEBUG
                                        if(nand_cfg->read_retry == 1)
                                                readpage_count++;
#endif
retry_1_start:	
#if 1				
					if(bytes <= 1024 && col == 0)
					{
						buf_en_flag = 0;
						ret = imap_nand_read_page_random(mtd, bufpoi, die_page, 1024);
					}else{
						buf_en_flag = 1;
						ret = imap_nand_read_page(mtd, bufpoi, bufoob, die_page, mtd->writesize, oobbytes+4);
					}			
#endif
#if 0
					ret = imap_nand_read_page(mtd, bufpoi, bufoob, die_page, mtd->writesize, oobbytes+4);
#endif
					if(ret != 0){
						printk("imap_nand_read_page ret = 0x%x\n", ret);
						break;
					}
#if RETRYLEVEL_COUNT_DEBUG
                                        if(nand_cfg->read_retry == 1 && (mtd->ecc_stats.failed == stats.failed)){
                                                retrylevel_count[retrylevel] += 1;
                                        }
                                        if(nand_cfg->read_retry == 1 && readpage_count % 1000 == 0){
                                                
                                                printk("=============read retry debug===========\n");
                                                for(i=0; i< nand_cfg->retry_level; i++){
                                                        printk("read retry pass level %d = %d\n", i, retrylevel_count[i]);
                                                }
                                        }
#endif
					if(nand_cfg->read_retry == 1 && (mtd->ecc_stats.failed - stats.failed)){
						retrylevel++;
                                                //printk("$$$$$$page retrylevel is %d.\n", retrylevel);
						if(retrylevel > nand_cfg->retry_level)
							retrylevel = 0;
						if(nand_cfg->chipnr == 0){
							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
								imap_set_retry_param_26(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_20(mtd, retrylevel, g_otp_buf, retry_reg_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
								imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_micro_20(mtd, retrylevel);

							if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_sandisk19(mtd, retrylevel);

							if(g_retrylevel == retrylevel){
								printk("======first chip page read retry failed 0x%x======\n", page);
								goto retry_2_end;
							}else{
								(mtd->ecc_stats.failed)--;
								goto retry_1_start;
							}
						}
						else if(nand_cfg->chipnr == 1){
							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
								imap_set_retry_param_26(mtd, retrylevel, &g_otp_buf[10]);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_20(mtd, retrylevel, &g_otp_buf[1024], retry_reg_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
								imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_micro_20(mtd, retrylevel);

							if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_sandisk19(mtd, retrylevel);
							
							if(g_retrylevel_chip1 == retrylevel){
								printk("======second chip page read retry failed 0x%x======\n", page);
								goto retry_2_end;
							}else{
								(mtd->ecc_stats.failed)--;
								goto retry_1_start;
							}
						}
					}else{
						if(mtd->ecc_stats.failed - stats.failed)
							printk("=====page read failed 0x%x=====\n", page);
					}
				}
				else if(ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE){
					/*read page and sysinfo & rsvd(4byte)*/
					//printk("read_raw bytes = %d, oobbytes = %d\n", ops->len, ops->ooblen);
					ret = imap_nand_read_raw(mtd, bufpoi, NULL, die_page, ops->len, 0);
					if(ret != 0){
						printk("imap_nand_read_raw ret = 0x%x\n", ret);
						break;
					}
				}
				else{
					printk("ops->mode = %d, bufpoi = 0x%x, bufoob = 0%x, bytes = %d, oobbytes = %d\n", ops->mode, (unsigned int)bufpoi, (unsigned int)bufoob, bytes, oobbytes);
				}
			}else{
				if(ops->mode == MTD_OOB_AUTO){
					retrylevel = g_retrylevel;
retry_2_start:					
					ret = imap_nand_read_oob(mtd, bufoob, die_page, oobbytes+4);
					if(ret != 0){
						printk("imap_nand_read_oob ret = 0x%x\n", ret);
						break;
					}

					if(nand_cfg->read_retry == 1 && (mtd->ecc_stats.failed - stats.failed)){
						retrylevel++;
                                                //printk("$$$$$$oob retrylevel is %d.\n", retrylevel);
						if(retrylevel > nand_cfg->retry_level)
							retrylevel = 0;
						if(nand_cfg->chipnr == 0){
							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
								imap_set_retry_param_26(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_20(mtd, retrylevel, g_otp_buf, retry_reg_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
								imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_micro_20(mtd, retrylevel);

							if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_sandisk19(mtd, retrylevel);

							if(g_retrylevel == retrylevel){
								printk("======first chip oob read retry failed 0x%x======\n", page);
								goto retry_2_end;
							}else{
								(mtd->ecc_stats.failed)--;
								goto retry_2_start;
							}
						}
						else if(nand_cfg->chipnr == 1){
							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26)
								imap_set_retry_param_26(mtd, retrylevel, &g_otp_buf[10]);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_20(mtd, retrylevel, &g_otp_buf[1024], retry_reg_buf);

							if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
								imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

							if(nand_cfg->nand_param0 == 0x75 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_micro_20(mtd, retrylevel);

							if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
								imap_set_retry_param_sandisk19(mtd, retrylevel);

							if(g_retrylevel_chip1 == retrylevel){
								printk("======second chip oob read retry failed 0x%x======\n", page);
								goto retry_2_end;
							}else{
								(mtd->ecc_stats.failed)--;
								goto retry_2_start;
							}
						}

					}else{

						if(mtd->ecc_stats.failed - stats.failed)
							printk("=====oob read failed 0x%x, from = 0x%llx=====\n", page, from);
					}
				}
				else if(ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE){
					//printk("read_raw bytes = %d, oobbytes = %d\n", ops->len, ops->ooblen);
					ret = imap_nand_read_raw(mtd, NULL, bufoob, die_page, 0, ops->ooblen);
				}
			}

retry_2_end:
			if(retrylevel != 0 && nand_cfg->read_retry == 1){
				retrylevel = 0;
				g_retrylevel = 0;
				g_retrylevel_chip1 = 0;

				if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 26){
					if(nand_cfg->chipnr == 0){
						imap_set_retry_param_26(mtd, retrylevel, g_otp_buf);
					}else{
						imap_set_retry_param_26(mtd, retrylevel, &g_otp_buf[10]);
					}
				}
				if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 20){
					if(nand_cfg->chipnr == 0){
						imap_set_retry_param_20(mtd, retrylevel, g_otp_buf, retry_reg_buf);
					}else{
						imap_set_retry_param_20(mtd, retrylevel, &g_otp_buf[1024], retry_reg_buf);
					}
				}        
				if(nand_cfg->nand_param0 == 0x55 && nand_cfg->nand_param1 == 21)
					imap_set_retry_param_21(mtd, retrylevel, g_otp_buf);

				if(nand_cfg->nand_param0 == 0x75  && nand_cfg->nand_param1 == 20)
					imap_set_retry_param_micro_20(mtd, retrylevel);
							
				if(nand_cfg->nand_param0 == 0x95 && nand_cfg->nand_param1 == 20)
					imap_set_retry_param_sandisk19(mtd, retrylevel);
			}
			}
			g_retrylevel = 0;
			g_retrylevel_chip1 = 0;
			/* Transfer not aligned data */
			if(buf != NULL){
				if(!aligned){
					if (!(mtd->ecc_stats.failed - stats.failed)){
						nand_cfg->pagebuf = page;
						memcpy(buf, nand_cfg->buffers->databuf + col, bytes);
					}
				}
			}

			if(oob != NULL){
				if(!alignedoob){	
					memcpy(oob, bufoob + 4, oobbytes);	
				}
			}


			if(buf != NULL){
				buf += bytes;
			}
			if(oob != NULL){
				bufoob += oobbytes;
			}

		}else{

			//TODO:
			printk("buf and oobbuf is also NULL\n");
			memcpy(buf, nand_cfg->buffers->databuf + col, bytes);
			buf += bytes;

			if(oob != NULL){
				memcpy(oob, bufoob + 4, oobbytes);		
			}
		}

		if(buf)
			readlen -= bytes;
		if(oob)
			oobreadlen -= oobbytes;

		if(buf != NULL){
			if (!readlen)
				break;
		}else{
			if(!oobreadlen)
				break;
		}

//		printk("goto next while\n");
		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		realpage++;
		//from += nand_cfg->pagesize; 
		from += bytes; 
		
		//printk("oobreadlen = 0x%x, oobbytes = 0x%x, page = 0x%x\n", oobreadlen, oobbytes, page);
		/* Check, if we cross a chip boundary */
#if 0
		if (!page) {
			chipnr++;
		//	chip->select_chip(mtd, -1);
		//	chip->select_chip(mtd, chipnr);
		}
#endif
	}	

	if(buf){
		ops->retlen = ops->len - (size_t) readlen;
		//printk("ops->retlen = 0x%x\n", ops->retlen);
	}
	if (oob){
		ops->oobretlen = ops->ooblen - oobreadlen;
		//printk("ops->oobretlen = 0x%x\n", ops->oobretlen);
		//printk("%x%x%x%x, %x%x%x%x, %x%x%x%x, %x%x%x%x\n", oob[0], oob[1], oob[2], oob[3], oob[4], oob[5], oob[6], oob[7], oob[8], oob[9], oob[10], oob[11], oob[12], oob[13], oob[14], oob[15]);
	}

	if (ret)
		return -ETIME;

	//printk("mtd->ecc_stats.failed = 0x%x, stats.failed = 0x%x\n", mtd->ecc_stats.failed, stats.failed);
	//printk("mtd->ecc_stats.corrected = 0x%x, stats.corrected = 0x%x\n", mtd->ecc_stats.corrected, stats.corrected);

	if (mtd->ecc_stats.failed - stats.failed){
		printk("read ops mtd->ecc_stats.failed = 0x%x, stats.failed = 0x%x, page = 0x%x\n", mtd->ecc_stats.failed, stats.failed, page);
		return -EBADMSG;	
	}

	if(mtd->ecc_stats.corrected - stats.corrected){
		//printk("read ops mtd->ecc_stats.corrected = 0x%x, stats.corrected = 0x%x, page = 0x%x\n", mtd->ecc_stats.corrected, stats.corrected, page);
		return -EUCLEAN;	
	}

	return 0;

}

/**
 * nand_read - [MTD Interface] MTD compatibility function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, uint8_t *buf)
{
	struct mtd_oob_ops ops;
	int ret;

	nand_get_device(mtd, FL_READING);
	//printk("nand_read\n");

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	ops.len    = len;
	ops.datbuf = (uint8_t *)buf;
	ops.oobbuf = NULL;
	ops.mode = MTD_OOB_AUTO;

	ret = nand_do_read_ops(mtd, from, &ops);

	*retlen    = ops.retlen;

	nand_release_device(mtd);

	return ret;
}

/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int nand_read_oob(struct mtd_info *mtd, loff_t from,
			 struct mtd_oob_ops *ops)
{
	int ret;

	nand_get_device(mtd, FL_READING);
	//printk("nand_read_oob offs = 0x%llx\n", from);
	ops->retlen = 0;

	/* Do not allow reads past end of device */
	if (ops->datbuf && (from + ops->len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt read "
				"beyond end of device\n", __func__);
		return -EINVAL;
	}

	ret = nand_do_read_ops(mtd, from, ops);

	nand_release_device(mtd);

	return ret;
}


/**
 * nand_fill_oobbuf - [Internal] Transfer client buffer to oob
 * @chip:	nand chip structure
 * @oob:	oob data buffer
 * @len:	oob data write length
 * @ops:	oob ops structure
 */
static uint8_t *nand_fill_oobbuf(struct nand_config *nand_cfg, uint8_t *oob, size_t len,
						struct mtd_oob_ops *ops){

	switch (ops->mode) {

	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		memcpy(nand_cfg->oob_poi + ops->ooboffs, oob, len);
		return oob + len;

	case MTD_OOB_AUTO: 
		memcpy(nand_cfg->oob_poi + ops->ooboffs + 4, oob, len);
		return oob + len;

	default:
		BUG();
	}
	return NULL;	
}

/**
 * nand_do_write_ops - [Internal] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operations description structure
 *
 * NAND write with ECC
 */
#define NOTALIGNED(x)	((x & (mtd->writesize - 1)) != 0)
static int nand_do_write_ops(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int chipnr, realpage, page, blockmask, column;
	struct nand_config *nand_cfg = mtd->priv;
	
	uint32_t writelen = ops->len;
	uint32_t oobwritelen = ops->ooblen;
	uint32_t oobmaxlen = ops->mode == MTD_OOB_AUTO ?
				mtd->oobavail : mtd->oobsize;

	uint8_t *oob = ops->oobbuf;
	uint8_t *buf = ops->datbuf;
	uint8_t *bufpoi, *bufoob;
	int ret, subpage;
	int bytes;
	int cached;
	uint8_t *wbuf;
	int chip_page, die_page;
	int col = 0;

	//printk("nand_do_write_ops\n");

	infotm_nand_wp();

	ops->retlen = 0;
	if (!writelen)
		return 0;

	//printk("writelen = %d\n", writelen);
	/* reject writes, which are not page aligned */
	if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
		printk(KERN_NOTICE "%s: Attempt to write not "
				"page aligned data\n", __func__);
		return -EINVAL;
	}

	column = to & (mtd->writesize - 1);
	subpage = column || (writelen & (mtd->writesize - 1));

	//printk("column = %d, subpage = %d\n", column, subpage);

	if (subpage && oob)
		return -EINVAL;

	//chip->select_chip(mtd, chipnr);

	//printk("chipnr = %d\n", chipnr);
	/* Check, if it is write protected */
#if 0
	if (nand_check_wp(mtd))
		return -EIO;
#endif
	//realpage = (int)(to >> nand_cfg->page_shift);
	//page = (int)(to >> nand_cfg->page_shift);
	blockmask = (1 << (nand_cfg->block_shift - nand_cfg->page_shift)) - 1;
	
	//printk("realpage = 0x%x, page = 0x%x, blockmask = 0x%x\n", realpage, page, blockmask);
	/* Invalidate the page cache, when we write to the cached page */
	if (to <= (nand_cfg->pagebuf << nand_cfg->page_shift) &&
	    (nand_cfg->pagebuf << nand_cfg->page_shift) < (to + ops->len))
		nand_cfg->pagebuf = -1;

	/* If we're not given explicit OOB data, let it be 0xFF */
	if (likely(!oob))
		memset(nand_cfg->oob_poi, 0xff, oobmaxlen);
	
	/* Don't allow multipage oob writes with offset */
	if (oob && ops->ooboffs && (ops->ooboffs + ops->ooblen > oobmaxlen))
		return -EINVAL;

	while (1) {
		chipnr = (int)(to >> nand_cfg->chip_shift);
		page = (int)(to >> nand_cfg->page_shift);
		chip_page = page & (nand_cfg->pagemask);
		die_page = chip_page & (nand_cfg->die_pagemask);
		if(nand_cfg->ddp == 1){
			if(chip_page >= (nand_cfg->pages_pre_die)){
				die_page = die_page | 0x100000;
			}
		}
	
		last_chipnr = 0;
		last_die_page = 0;
		
		col = to & (nand_cfg->page_shift) - 1;

		if(col != 0){
			printk("=============write col = 0x%x==========\n", col);
		}

		nand_cfg->chipnr = chipnr;	
		bytes = mtd->writesize;
		cached = writelen > bytes && page != blockmask;
		
		if(imap_nand_valied_vm((uint32_t)buf, bytes)!=0){
			memcpy(&nand_cfg->buffers->databuf[0], buf, bytes);
			wbuf = &nand_cfg->buffers->databuf[0];
		}else{
			wbuf = buf;
		}

		/* Partial page write ? */
		if (unlikely(column || writelen < (mtd->writesize - 1))) {
			cached = 0;
			bytes = min_t(int, bytes - column, (int) writelen);
			nand_cfg->pagebuf = -1;
			memset(nand_cfg->buffers->databuf, 0xff, mtd->writesize);
			memcpy(&nand_cfg->buffers->databuf[column], buf, bytes);
			wbuf = nand_cfg->buffers->databuf;
		}

		bufpoi = wbuf;
		//bufoob = (uint8_t *)((int)(nand_cfg->oob_poi) & (int)(~0xf));
		bufoob = (uint8_t *)((int)(nand_cfg->oob_poi));

		memset(nand_cfg->oob_poi, 0xff, mtd->oobavail);
		if (oob) {
			size_t len = min(oobwritelen, oobmaxlen);
			//oob = nand_fill_oobbuf(nand_cfg, oob, len, ops);
			memcpy(bufoob + 4, oob, len);
			oob += len;
			oobwritelen -= len;
		}

		//printk("bufpoi = 0x%x, bufoob = 0x%x, page = 0x%x, ops->mode = 0x%x\n", bufpoi, bufoob, page, ops->mode);
		if (ops->mode == MTD_OOB_AUTO){
				/*write page and sysinfo*/
				//printk("MTD_OOB_AUTO\n");
				//printk("bufpoi = 0x%x, bytes = 0x%x\n", bufpoi, bytes);
				
				ret = imap_nand_write_page(mtd, bufpoi, bufoob, die_page, bytes, mtd->oobavail);
				if(ret != 0)
				  printk("imap_nand_write_page err\n");
					
		}
		else if(ops->mode == MTD_OOB_RAW || ops->mode == MTD_OOB_PLACE){
				//printk("MTD_OOB_RAW\n");
				ret = imap_nand_write_raw(mtd, bufpoi, bufoob, die_page, bytes, mtd->oobavail);
				if(ret != 0)
				  printk("imap_nand_write_raw err\n");
		
		}
		if(ret)
			break;
		
		writelen -= bytes;
		if (!writelen)
			break;
		//printk("curwritelen = %d, bytes = %d\n", writelen, bytes);

		column = 0;
		buf += bytes;
		//to += nand_cfg->pagesize;
		to += bytes;

	}
	/* return -EIO if write failed 
	 * return -ENOTSUPP if mode is not recognized
	 * return 0 if performed a perfect writing
	 * do not forget to put result into ops->retlen & ops->oobretlen
	 */
	ops->retlen = ops->len - writelen;
	if (unlikely(oob))
		ops->oobretlen = ops->ooblen;

	//printk("ops->retlen = 0x%x, ops->oobretlen = 0x%x, ret = 0x%x\n", ops->retlen, ops->oobretlen, ret);

	if(ret != 0)
		return -EIO;
	else
		return 0;
}


/**
 * nand_erase_nand - [Internal] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 * @allowbbt:	allow erasing the bbt area
 *
 * Erase one ore more blocks
 */
int nand_erase_nand(struct mtd_info *mtd, struct erase_info *instr,
		    int allowbbt)
{
	int page, status, pages_per_block, ret, chipnr, erase_io_ret;
	struct nand_config *nand_cfg = mtd->priv;
	loff_t rewrite_bbt[NAND_MAX_CHIPS] = {0};
	unsigned int bbt_masked_page = 0xffffffff;
	loff_t len;
	loff_t start;
	int chip_page, die_page;
	int erase_retry_cnt;

	infotm_nand_wp();

	DEBUG(MTD_DEBUG_LEVEL3, "%s: start = 0xllx, len = %llx\n",
				__func__, (unsigned long long)instr->addr,
				(unsigned long long)instr->len);

	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

	start = instr->addr;
#if 0
	printk("nand_cfg->page_shift = 0x%x\n", nand_cfg->page_shift);
	printk("nand_cfg->block_shift = 0x%x\n", nand_cfg->block_shift);
	printk("nand_cfg->chip_shift = 0x%x\n", nand_cfg->chip_shift);
#endif
	/* Shift to get first page */

	/* Calculate pages in each block */
	pages_per_block = 1 << (nand_cfg->block_shift - nand_cfg->page_shift);
#if 0
	printk("nstr->addr = 0x%llx\n", instr->addr);
	printk("page = 0x%x\n", page);
	printk("chipnr = 0x%x\n", chipnr);
	printk("nand_cfg->block_shift = 0x%x\n", nand_cfg->block_shift);
	printk("nand_cfg->page_shift = 0x%x\n", nand_cfg->page_shift);
	printk("pages_per_block = 0x%x\n", pages_per_block);
#endif
#if 0
	/* Check, if it is write protected */
	if (nand_check_wp(mtd)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Device is write protected!!!\n",
					__func__);
		instr->state = MTD_ERASE_FAILED;
		goto erase_exit;
	}
#endif

	/* Loop through the pages */
	len = instr->len;

	instr->state = MTD_ERASING;

	while (len) {
		/*
		 * heck if we have a bad block, we do not erase bad blocks !
		 */
		//printk("len = 0x%llx, page = 0x%x\n", len, page);

		/*
		 * Invalidate the page cache, if we erase the block which
		 * contains the current cached page
		 */
	
		chipnr = (int)(start >> (nand_cfg->chip_shift));
		page = (int)(start >> nand_cfg->page_shift);
		chip_page = page & (nand_cfg->pagemask);
		die_page = chip_page & (nand_cfg->die_pagemask);
		nand_cfg->chipnr = chipnr;
		if(nand_cfg->ddp == 1){
			if(chip_page >= (nand_cfg->pages_pre_die)){
				die_page = die_page | 0x100000;
			}
		}
	
		nand_cfg->chipnr = chipnr;	
		//printk("erase chipnr %d, page 0x%x\n", chipnr, die_page);	
		if (page <= nand_cfg->pagebuf && nand_cfg->pagebuf <
		    (page + pages_per_block))
			nand_cfg->pagebuf = -1;
		
		erase_retry_cnt = 0;
erase_retry:		
		erase_io_ret = imapx800_nand_erase(mtd, die_page, nand_cfg->cycle, &status);

#if 0	
		/*
		 * See if operation failed and additional status checks are
		 * available
		 */
		if (((erase_io_ret != 0) || (status & NAND_STATUS_FAIL)) && (chip->errstat))
			status = chip->errstat(mtd, chip, FL_ERASING,
					       status, page);
#endif
		/* See if block erase succeeded */
#if 1
		if ((erase_io_ret != 0) || (status & NAND_STATUS_FAIL)) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: Failed erase, "
					"page 0x%08x\n", __func__, page);
			printk("MTD_ERASE_FAILED page = %x\n", page);

			if(erase_retry_cnt >= 3){
				erase_write_bug( );
				instr->state = MTD_ERASE_FAILED;
				instr->fail_addr =
					((loff_t)page << nand_cfg->page_shift);
				goto erase_exit;
			}
			//try to reset nandflash
			nf2_asyn_reset(mtd);
			erase_retry_cnt++;
			goto erase_retry;

		}
#endif
		/*
		 * If BBT requires refresh, set the BBT rewrite flag to the
		 * page being erased
		 */
		if (bbt_masked_page != 0xffffffff &&
		    (page & BBT_PAGE_MASK) == bbt_masked_page)
			    rewrite_bbt[chipnr] =
					((loff_t)page << nand_cfg->page_shift);

		/* Increment page address and decrement length */
		len -= (1 << nand_cfg->block_shift);
		//page += pages_per_block;
		start += nand_cfg->blocksize;

	}
	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;


	/*
	 * If BBT requires refresh and erase was successful, rewrite any
	 * selected bad block tables
	 */
	if (bbt_masked_page == 0xffffffff || ret)
		return ret;
#if 0
	for (chipnr = 0; chipnr < chip->numchips; chipnr++) {
		if (!rewrite_bbt[chipnr])
			continue;
		/* update the BBT for chip */
		DEBUG(MTD_DEBUG_LEVEL0, "%s: nand_update_bbt "
			"(%d:0x%0llx 0x%0x)\n", __func__, chipnr,
			rewrite_bbt[chipnr], chip->bbt_td->pages[chipnr]);
		nand_update_bbt(mtd, rewrite_bbt[chipnr]);
	}
#endif
	/* Return more or less happy */
	return ret;
}

/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 */
static int nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const uint8_t *buf)
{
	struct mtd_oob_ops ops;
	int ret;

	nand_get_device(mtd, FL_WRITING);
	//printk("nand_write\n");
	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	ops.len = len;
	ops.datbuf = (uint8_t *)buf;
	ops.oobbuf = NULL;
	ops.mode = MTD_OOB_AUTO;

	ret = nand_do_write_ops(mtd, to, &ops);

	*retlen = ops.retlen;

	nand_release_device(mtd);
	
	if(ret != 0)
		erase_write_bug( );

	return ret;
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int nand_write_oob(struct mtd_info *mtd, loff_t to,
			  struct mtd_oob_ops *ops)
{
	int ret;

	nand_get_device(mtd, FL_WRITING);
//	printk("nand_write_oob\n");
	ops->retlen = 0;

	/* Do not allow writes past end of device */
	if (ops->datbuf && (to + ops->len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt write beyond "
				"end of device\n", __func__);
		return -EINVAL;
	}

	ret = nand_do_write_ops(mtd, to, ops);

	nand_release_device(mtd);

	return ret;
}

/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret = 0;
	/* TODO:
	 * else if chip reports bad return -EIO
	 * do NOT skip bad blocks, but you have to check bad block, if bad block
	 * found return -EIO.
	 * else return 0 for successful
	 */

	/* Grab the lock and see if the device is available */
	nand_get_device(mtd, FL_ERASING);
	DEBUG(MTD_DEBUG_LEVEL3, "%s: start = 0x%012llx, len = %llu\n",
				__func__, (unsigned long long)instr->addr,
				(unsigned long long)instr->len);

	//printk("erase instr->addr = 0x%llx, instr->len = 0x%llx\n", instr->addr, instr->len);
	if (check_offs_len(mtd, instr->addr, instr->len))
		return -EINVAL;
	
	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

	/* TODO: instr->fail_addr should set to the failure block addr */
	ret = nand_erase_nand(mtd, instr, 0);
	/* Deselect and wake up anyone waiting on the device */
	nand_release_device(mtd);

	if(!ret)
	  mtd_erase_callback(instr);

	/* TODO: update the bbt */

	return 0;
}

/**
 * nand_sync - [MTD Interface] sync
 * @mtd:	MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void nand_sync(struct mtd_info *mtd)
{
	printk("nand_sync\n");
	DEBUG(MTD_DEBUG_LEVEL3, "%s: called\n", __func__);

	/* Grab the lock and see if the device is available */
	nand_get_device(mtd, FL_SYNCING);
	/* Release it and go back */
	nand_release_device(mtd);
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
static int nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	int ret;

	nand_get_device(mtd, FL_READING);
	//printk("nand_block_isbad\n");
	/* Check for invalid offset */
	if (offs > mtd->size)
		return -EINVAL;

	/* TODO: 
	 * set ret to  1 if bad, 0 if good
	 * if bbt is availiable, check from bbt
	 * else check the coresponding spare areas.
	 */
	/* Check for invalid offset */
	if (offs > mtd->size)
		return -EINVAL;

	//printk("isbad offs = 0x%llx\n", offs);
	ret = nand_block_checkbad(mtd, offs, 1, 1);


	nand_release_device(mtd);

	//printk("check ret = %d\n", ret);
	return ret;
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	int page, chipnr, ret = 0;
	struct nand_config *nand_cfg = mtd->priv;
	int status;
	int chip_page, die_page;
	int bbt_state;

	printk("nand_block_markbad\n");
	ret = nand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing. */
		if (ret > 0)
			return 0;
		return ret;
	}

	/* TODO: mark this block as bad, aslo update the bbt */
	if(!nand_cfg)
	  return -1;
		

	nand_get_device(mtd, FL_WRITING);

	infotm_nand_wp();

	bbt_state = nand_cfg->bbt;
	
	bad_block_table_erase(mtd);
	
	chipnr = (int)(ofs >> (nand_cfg->chip_shift));
	page = (int)(ofs >> nand_cfg->page_shift);
	chip_page = page & (nand_cfg->pagemask);
	die_page = chip_page & (nand_cfg->die_pagemask);
	if(nand_cfg->ddp == 1){
		if(chip_page >= (nand_cfg->pages_pre_die)){
			die_page = die_page | 0x100000;
		}
	}

	nand_cfg->chipnr = chipnr;
	//page = (int)(ofs >> nand_cfg->page_shift) & nand_cfg->pagemask;
	printk("marking offset: 0x%llx as bad., page = 0x%x\n", ofs, page);

	//page = (int)(ofs >> nand_cfg->page_shift);
	//chipnr = (int)(ofs >> nand_cfg->chip_shift);
	ret = imapx800_nand_erase(mtd, die_page, nand_cfg->cycle, &status);
	if(ret)
	  printk("warning: erase failed.\n");

	imapx800_nand_block_markbad(mtd, ofs + nand_cfg->badpagemajor * nand_cfg->pagesize);
	if(nand_cfg->badpageminor != nand_cfg->badpagemajor)
		imapx800_nand_block_markbad(mtd, ofs + nand_cfg->badpageminor * nand_cfg->pagesize);

	//read back, check is marking , if ok, update bbt
	nand_cfg->bbt = 0;
	if(nand_block_checkbad(mtd, ofs, 1, 0) != 0){
		printk("mark bad block 0x%llx\n", ofs);
		update_bbt(ofs);
		bad_block_table_write(mtd, g_bbt);
		printk("update bbt\n");
	}
	nand_cfg->bbt = bbt_state;
	
	nand_release_device(mtd);
	return ret;
}

/**
 * nand_suspend - [MTD Interface] Suspend the NAND flash
 * @mtd:	MTD device structure
 */
static int nand_suspend(struct mtd_info *mtd)
{
	printk("nand_suspend\n");
	return nand_get_device(mtd, FL_PM_SUSPENDED);
}

/**
 * nand_resume - [MTD Interface] Resume the NAND flash
 * @mtd:	MTD device structure
 */
static void nand_resume(struct mtd_info *mtd)
{
	struct nand_config * nand_cfg = NULL;
	int i = 0;
	nand_cfg = mtd->priv;
	
	printk("nand_resume\n");
	
	if(power_index != -1){
		imapx_pad_set_mode(1, 1, power_index); //gpio mode
		imapx_pad_set_outdat(1, 1, power_index); //set output = 1
		imapx_pad_set_dir(0, 1, power_index); //set dir output
	}

        g_retrylevel = 0;
        g_retrylevel_chip1 = 0;
        imap_nand_hwinit();
	nf2_timing_init(nand_cfg->interface, nand_cfg->timing, nand_cfg->rnbtimeout, nand_cfg->phyread, nand_cfg->phydelay, nand_cfg->busw);
	for(i=0; i<(nand_cfg->chipnum); i++){
		nand_cfg->chipnr = i;
		nf2_asyn_reset(mtd);
	}
	if (infotm_mtd_state == FL_PM_SUSPENDED)
	  nand_release_device(mtd);
	else
	  printk(KERN_ERR "%s called for a chip which is not "
				  "in suspended state\n", __func__);
}


int nand_get_item(void){

	char buf[64];

	if (item_exist("nand.1.cs")){
		cs1_index = item_integer("nand.1.cs", 1);
	}
	if (item_exist("nand.2.cs")){
		cs2_index = item_integer("nand.2.cs", 1);
	}
	if (item_exist("nand.3.cs")){
		cs3_index = item_integer("nand.3.cs", 1);
	}
	if (item_exist("nand.2.power")){
		item_string(buf, "nand.2.power", 0);
		if(!strncmp(buf,"pads",4)){
			power_index = item_integer("nand.2.power", 1);
		}
	}else if(!strncmp(buf,"pmu",3)){
	
	
	}

	return 0;
}

/**
 * nand_scan - [NAND Interface] Scan for the NAND device
 * @mtd:	MTD device structure
 * @maxchips:	Number of chips to scan for
 *
 * This fills out all the uninitialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 * The mtd->owner field must be set to the module of the caller
 *
 */
int nand_scan(struct mtd_info *mtd, int maxchips)
{
	int ret;
	uint8_t id[CDBG_NAND_ID_COUNT];
	uint8_t id1[CDBG_NAND_ID_COUNT];
	uint8_t id2[CDBG_NAND_ID_COUNT];
	uint8_t id3[CDBG_NAND_ID_COUNT];
	//int timing = 0x51ffffff;
	int timing = 0x31ffffff;
	//int rnbtimeout = 0xfffaf0;
	int rnbtimeout = 0xfffa00;
	int phyread = 0x4023;
	//int phydelay = 192<<8 | 192;
	int phydelay = 0x400;
	int busw = 0;
	int i, j, k;	
	struct nand_config * nand_cfg = NULL;
	int bad_mark = 0;
	int otp_start = 0;
	int otp_len = 0;
	uint32_t retry_param_magic = 0;
	int block, page, skip;
	int pages_pre_chip;
	int len = 0, len1;
	loff_t start, ofs;

	int check_second_chip;
	power_index = -1; 
	cs1_index = -1; 
	cs2_index = -1; 
	cs3_index = -1;

	g_retrylevel = 0;	
	g_retrylevel_chip1 = 0;	

	nand_get_item();	
	
#ifdef INFOTM_NAND_DEBUG
	printk("power_index = %d, cs1_index = %d, cs2_index = %d, cs3_index = %d\n", power_index, cs1_index, cs2_index, cs3_index);
#endif
#if RETRYLEVEL_COUNT_DEBUG
        for(i=0; i<30; i++){
                retrylevel_count[i] = 0;
        }
        readpage_count = 0;
#endif
	m_lock = 0;	
        //len = sizeof(gt_bbt);
        len = sizeof(struct nand_bad_block_info);
	printk("kmalloc len = 0x%x\n", len);
	g_bbt = kmalloc(len, GFP_KERNEL);
	if(g_bbt == NULL){
		printk("kmalloc gbbt failed\n");
	}
	g_bbt->max_blk_num = 0;
	g_bbt->bad_mark_magic = 0;

	memset(g_bbt, len, 0xff);

	g_otp_buf = NULL;
	g_otp_buf = kmalloc(2048, GFP_KERNEL);
	if(g_otp_buf == NULL){
		printk("====OTP BUF ALLOC FAILED=====\n");
	}

	sys_rtc_regs = ioremap(SYSMGR_RTC_BASE, 0x1000);
	//resved_mem = ioremap(0x88000000, 0x1000);
	//resved_mem = phys_to_virt(0x88000000);
	resved_mem = phys_to_virt(RESERVED_BASE_RRTB);
	if(resved_mem == NULL){
		printk("resved_mem is NULL\n");
	}
	resved_mem_u8 = (unsigned char *)(resved_mem + 0x100);
	mtd->priv = &nf_cfg; 
	nand_cfg = mtd->priv;
	/* read id from nand device:
	 * two kind of interface should be tried. (legacy & toggle)
	 * then, fill the following values: XXX
	 */
	nand_cfg->buffers = kmalloc(sizeof(*nand_cfg->buffers), GFP_KERNEL);
	if (!nand_cfg->buffers){
		printk("nand_scan kmalloc nand buffer error\n");
		return -ENOMEM;
	}

	nand_cfg->pagebuf = -1;
	nand_cfg->chipnr = 0;
	nand_cfg->bbt = 0;
#if IMAPX800_NAND_16BIT
	busw = 16;
	ret = nand_readid(mtd, id, 0, timing, rnbtimeout, phyread, phydelay, busw);
#else
	busw = 8;
	ret = nand_readid(mtd, id, 0, timing, rnbtimeout, phyread, phydelay, busw);
#endif
	
#ifdef INFOTM_NAND_DEBUG
	printk("got id: %02x %02x %02x %02x %02x %02x %02x %02x\n",
					id[0], id[1], id[2],
					id[3], id[4], id[5],
					id[6], id[7]);
#endif
	nand_cfg->pagesize = 0;
	for(i=0; ;i++){
		if(0 == infotm_nand_idt[i].pagesize)
			break;
	
		for(j = 0; j < 6; j++){
			if(infotm_nand_idt[i].id[j] != id[j]){
#ifdef INFOTM_NAND_DEBUG
				printk("id%d do not match, %x<=>%x\n", j, id[j], (unsigned int)infotm_nand_idt[i].id[j]);
#endif
				break;
			}
		}

		if((CDBG_NAND_ID_COUNT - 2) == j){
			printk("nand configuratino matched.\n");
			nand_cfg->interface = infotm_nand_idt[i].interface;
			nand_cfg->pagesize = infotm_nand_idt[i].pagesize;	
			nand_cfg->sparesize = infotm_nand_idt[i].oobsize;	
			nand_cfg->chipsize = infotm_nand_idt[i].chipsize;
			nand_cfg->blocksize = infotm_nand_idt[i].erasesize;
			nand_cfg->cycle = infotm_nand_idt[i].cycle;
			nand_cfg->mecclvl = infotm_nand_idt[i].mecclvl;
			nand_cfg->secclvl = infotm_nand_idt[i].secclvl;
			nand_cfg->badpageminor = infotm_nand_idt[i].bad0;
			nand_cfg->badpagemajor = infotm_nand_idt[i].bad1;
			nand_cfg->sysinfosize = infotm_nand_idt[i].sysinfo;
			nand_cfg->timing = infotm_nand_idt[i].timing;
			nand_cfg->rnbtimeout = infotm_nand_idt[i].rnbtimeout;
			nand_cfg->phyread = infotm_nand_idt[i].phyread;
			nand_cfg->phydelay = infotm_nand_idt[i].phydelay;
			nand_cfg->busw = infotm_nand_idt[i].busw;
			nand_cfg->randomizer = infotm_nand_idt[i].randomizer;
			nand_cfg->polynomial = infotm_nand_idt[i].poly;
			nand_cfg->seed[0] = infotm_nand_idt[i].seed[0];	
			nand_cfg->seed[1] = infotm_nand_idt[i].seed[1];	
			nand_cfg->seed[2] = infotm_nand_idt[i].seed[2];	
			nand_cfg->seed[3] = infotm_nand_idt[i].seed[3];	
			nand_cfg->seed[4] = infotm_nand_idt[i].seed[4];	
			nand_cfg->seed[5] = infotm_nand_idt[i].seed[5];	
			nand_cfg->seed[6] = infotm_nand_idt[i].seed[6];	
			nand_cfg->seed[7] = infotm_nand_idt[i].seed[7];	
			nand_cfg->active_async = infotm_nand_idt[i].active_async;
			nand_cfg->read_retry = infotm_nand_idt[i].read_retry;
			nand_cfg->retry_level = infotm_nand_idt[i].retry_level;
			nand_cfg->nand_param0 = infotm_nand_idt[i].nand_param0;
			nand_cfg->nand_param1 = infotm_nand_idt[i].nand_param1;
			nand_cfg->nand_param2 = infotm_nand_idt[i].nand_param2;
			//nand_cfg->chipnum = infotm_nand_idt[i].chipnum;
			nand_cfg->chipnum = 1;
			nand_cfg->chip_pre_device = infotm_nand_idt[i].chipnum;
			nand_cfg->chipmap[0] = infotm_nand_idt[i].chipmap[0];
			nand_cfg->chipmap[1] = infotm_nand_idt[i].chipmap[1];
			nand_cfg->chipmap[2] = infotm_nand_idt[i].chipmap[2];
			nand_cfg->chipmap[3] = infotm_nand_idt[i].chipmap[3];
			nand_cfg->single_chipsize = infotm_nand_idt[i].single_chipsize;
			nand_cfg->ddp = infotm_nand_idt[i].ddp;
			nand_cfg->diesize = infotm_nand_idt[i].diesize;
			nand_cfg->max_correct_bits = infotm_nand_idt[i].max_correct_bits;

			nand_cfg->page_shift = ffs(nand_cfg->pagesize) - 1;
			nand_cfg->block_shift = ffs(nand_cfg->blocksize) - 1;
			nand_cfg->pagemask = 0xffffffff;
			nand_cfg->die_pagemask = 0xffffffff;
			//nand_cfg->chip_shift = ffs(nand_cfg->chipsize << 20);
			nand_cfg->chip_shift = 32; //TODO:
			if(nand_cfg->chip_pre_device > 1){
				nand_cfg->chip_shift = ffs(nand_cfg->single_chipsize) + 19; 
				pages_pre_chip = ((nand_cfg->single_chipsize) * 0x100000ll) >> (nand_cfg->page_shift);
			}else{
				nand_cfg->chip_shift = ffs(nand_cfg->chipsize) + 19;
				pages_pre_chip = ((nand_cfg->chipsize) * 0x100000ll) >> (nand_cfg->page_shift);
			}		
			nand_cfg->pagemask = pages_pre_chip - 1;
			if(nand_cfg->ddp == 1){
				nand_cfg->pages_pre_die = ((nand_cfg->diesize) * 0x100000ll) >> (nand_cfg->page_shift);
				nand_cfg->die_pagemask = nand_cfg->pages_pre_die - 1;
				printk("pages_pre_die = 0x%x, die_pagemask = 0x%x\n", nand_cfg->pages_pre_die, nand_cfg->die_pagemask);
			}

#ifdef INFOTM_NAND_DEBUG
			printk("chipnum = %d, chip_shift = %d, pages_pre_chip = 0x%x, pagemask = 0x%x\n", nand_cfg->chip_pre_device, nand_cfg->chip_shift, pages_pre_chip, nand_cfg->pagemask);	
#endif
			if(nand_cfg->mecclvl != 0){
			    nand_cfg->eccen = 1;
			}

			memcpy(nand_cfg->name, infotm_nand_idt[i].name, 20);

			if(nand_cfg->randomizer){
			    nand_init_randomizer(nand_cfg);

#ifdef INFOTM_NAND_DEBUG
			    for(k=0; k<8; k++){
				printk("seed[%x] = 0x%x, sysinfo_seed[%x] = 0x%x, ecc_seed[%x] = 0x%x, secc_seed[%x] = 0x%x\n", k, nand_cfg->seed[k], 
					k, nand_cfg->sysinfo_seed[k], k, nand_cfg->ecc_seed[k], k, nand_cfg->secc_seed[k]);
			    }
#endif
			}
			break;	
		}
	}

	if(0 == nand_cfg->pagesize){
	
#if IMAPX800_NAND_16BIT
			busw = 16;
			ret = nand_readid(mtd, id, 2, timing, rnbtimeout, phyread, phydelay, busw);
#else
			busw = 8;
			ret = nand_readid(mtd, id, 2, timing, rnbtimeout, phyread, phydelay, busw);
#endif
			
#ifdef INFOTM_NAND_DEBUG
	        	printk("got id: %02x %02x %02x %02x %02x %02x %02x %02x\n",
					id[0], id[1], id[2],
					id[3], id[4], id[5],
					id[6], id[7]);
#endif
			nand_cfg->pagesize = 0;
			for(i=0; ;i++){
				if(0 == infotm_nand_idt[i].pagesize)
					break;
			
				for(j = 0; j < 6; j++){
					if(infotm_nand_idt[i].id[j] != id[j]){
						break;
					}
				}
		
				if(CDBG_NAND_ID_COUNT == j){
					printk("nand configuratino matched.\n");
					nand_cfg->interface = infotm_nand_idt[i].interface;
					nand_cfg->pagesize = infotm_nand_idt[i].pagesize;	
					nand_cfg->sparesize = infotm_nand_idt[i].oobsize;	
					nand_cfg->chipsize = infotm_nand_idt[i].chipsize;
					nand_cfg->blocksize = infotm_nand_idt[i].erasesize;
					nand_cfg->cycle = infotm_nand_idt[i].cycle;
					nand_cfg->mecclvl = infotm_nand_idt[i].mecclvl;
					nand_cfg->secclvl = infotm_nand_idt[i].secclvl;
					nand_cfg->badpageminor = infotm_nand_idt[i].bad0;
					nand_cfg->badpagemajor = infotm_nand_idt[i].bad1;
					nand_cfg->sysinfosize = infotm_nand_idt[i].sysinfo;
					nand_cfg->timing = infotm_nand_idt[i].timing;
					nand_cfg->rnbtimeout = infotm_nand_idt[i].rnbtimeout;
					nand_cfg->phyread = infotm_nand_idt[i].phyread;
					nand_cfg->phydelay = infotm_nand_idt[i].phydelay;
					nand_cfg->busw = infotm_nand_idt[i].busw;
					nand_cfg->active_async = infotm_nand_idt[i].active_async;
					nand_cfg->read_retry = infotm_nand_idt[i].read_retry;
					nand_cfg->retry_level = infotm_nand_idt[i].retry_level;
					nand_cfg->nand_param0 = infotm_nand_idt[i].nand_param0;
					nand_cfg->nand_param1 = infotm_nand_idt[i].nand_param1;
					nand_cfg->nand_param2 = infotm_nand_idt[i].nand_param2;

					nand_cfg->page_shift = ffs(nand_cfg->pagesize) - 1;
					nand_cfg->block_shift = ffs(nand_cfg->blocksize) - 1;
					nand_cfg->pagemask = 0xffffffff;
					nand_cfg->die_pagemask = 0xffffffff;
					//nand_cfg->chip_shift = ffs(nand_cfg->chipsize << 20) - 1;
					nand_cfg->chip_shift = 32; //TODO:
					if(nand_cfg->chip_pre_device > 1){
						nand_cfg->chip_shift = ffs(nand_cfg->single_chipsize) + 19; 
						pages_pre_chip = ((nand_cfg->single_chipsize) * 0x100000ll) >> (nand_cfg->page_shift);
					}else{
						nand_cfg->chip_shift = ffs(nand_cfg->chipsize) + 19;
						pages_pre_chip = ((nand_cfg->chipsize) * 0x100000ll) >> (nand_cfg->page_shift);
					}
					nand_cfg->pagemask = pages_pre_chip - 1;
					if(nand_cfg->ddp == 1){
						nand_cfg->pages_pre_die = ((nand_cfg->diesize) * 0x100000ll) >> (nand_cfg->page_shift);
						nand_cfg->die_pagemask = nand_cfg->pages_pre_die - 1;
						printk("pages_pre_die = 0x%x, die_pagemask = 0x%x\n", nand_cfg->pages_pre_die, nand_cfg->die_pagemask);
					}
#ifdef INFOTM_NAND_DEBUG
					printk("chipnum = %d, chip_shift = %d, pages_pre_chip = 0x%x, pagemask = 0x%x\n", nand_cfg->chip_pre_device, nand_cfg->chip_shift, pages_pre_chip, nand_cfg->pagemask);	
#endif
					memcpy(nand_cfg->name, infotm_nand_idt[i].name, 20);
					break;		
				}
			}
	}

	nand_cfg->sync_mode = 1;//TODO: NEED DEFINE CONFIG
	/* to be filled::: */
	//mtd->name = ".xxx";
	mtd->name = nand_cfg->name;
	//memcpy(mtd->name, nand_cfg->name, 20);
	mtd->writesize = nand_cfg->pagesize;		// page size
	mtd->oobsize   = nand_cfg->sparesize;		// spare size
	mtd->erasesize = nand_cfg->blocksize;		// block size
	mtd->size = (nand_cfg->chipsize) * 0x100000ll;
	//mtd->erasesize_shift = 0;
	//mtd->writesize_shift = nand_cfg->page_shift;
	//mtd->erasesize_mask = 0;
	//mtd->writesize_mask = nand_cfg->pagemask;
#ifdef INFOTM_NAND_DEBUG
	printk("chipsize = 0x%x, mtd->size = 0x%llx, mtd->page_shift = 0x%x, mtd->block_shift = 0x%x, nand_cfg->chip_shift = 0x%x\n", nand_cfg->chipsize, mtd->size, nand_cfg->page_shift, nand_cfg->block_shift, nand_cfg->chip_shift);
	printk("mtd->oobsize = %d\n", mtd->oobsize);
#endif
	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->panic_write = NULL;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->panic_write = NULL;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = nand_suspend;
	mtd->resume = nand_resume;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;
	mtd->writebufsize = mtd->writesize;

	/* propagate the special oob_layout to mtd_info */
	mtd->ecclayout = &infotm_oob_layout;
	mtd->oobavail  = infotm_oob_layout.oobavail;
	mtd->subpage_sft = 0;
	mtd->priv = &nf_cfg; 

	nand_cfg->oob_poi = nand_cfg->buffers->databuf + mtd->writesize;

#ifdef INFOTM_NAND_DEBUG
	printk("writesize = 0x%x, oobsize = 0x%x, erasesize = 0x%x, totalsize = 0x%llx, mtd = 0x%x\n", mtd->writesize, mtd->oobsize, mtd->erasesize, mtd->size, (unsigned int)mtd);
	printk("mtd->read_oob = 0x%x, nand_cfg->phydelay = 0x%x\n", (unsigned int)mtd->read_oob, nand_cfg->phydelay);
	printk("mtd->block_isbad = 0x%x\n", (unsigned int)mtd->block_isbad);
#endif
	/* TODO: check if there is a second chip (CE)
	 * different chips at the same time is not supported.
	 */

	if(nand_cfg->active_async == 1)	
		nf2_active_async(mtd);

	if(nand_cfg->interface == 1){
		printk("nand onfi mode\n");
		nf2_active_sync(mtd);
	}
	
	if(power_index != -1){
		imapx_pad_set_mode(1, 1, power_index); //gpio mode
		imapx_pad_set_outdat(1, 1, power_index); //set output = 1
		imapx_pad_set_dir(0, 1, power_index); //set dir output
	}

	if(nand_cfg->chip_pre_device != 2){
		/* check the second nand chip*/
		if(cs1_index != -1){
			printk("check cs1_index\n");
			nand_cfg->chipmap[1] = 1;	
			nand_cfg->chipnr = 1;		
#if IMAPX800_NAND_16BIT
			busw = 16;
			nand_readid(mtd, id1, 0, timing, rnbtimeout, phyread, phydelay, busw);
#else
			busw = 8;
			nand_readid(mtd, id1, 0, timing, rnbtimeout, phyread, phydelay, busw);
#endif
			for(j = 0; j < 6; j++){
				if(id1[j] != id[j]){
					//TODO: set pad default
#ifdef INFOTM_NAND_DEBUG
					printk("id%d do not match, %x<=>%x\n", j, id1[j], id[j]);
#endif
					goto check_cs1_end;
				}
			}
			nand_cfg->chipnum++;
			printk("the second chip is found\n");
		}
	}else{
		nand_cfg->chipnum = 2;
	}
check_cs1_end:

	/* check the third nand chip*/
	if(cs2_index != -1){
		printk("check cs2_index\n");
		imapx_pad_set_mode(1, 1, cs2_index); //gpio mode
		imapx_pad_set_outdat(1, 1, cs2_index); //set output = 1
		imapx_pad_set_dir(0, 1, cs2_index); //set dir output
		nand_cfg->chipmap[2] = 2;	
		nand_cfg->chipnr = 2;		
#if IMAPX800_NAND_16BIT
			busw = 16;
			nand_readid(mtd, id2, 0, timing, rnbtimeout, phyread, phydelay, busw);
#else
			busw = 8;
			nand_readid(mtd, id2, 0, timing, rnbtimeout, phyread, phydelay, busw);
#endif
			for(j = 0; j < 6; j++){
				if(id2[j] != id[j]){
					//TODO: set pad default
#ifdef INFOTM_NAND_DEBUG
					printk("id%d do not match, %x<=>%x\n", j, id2[j], id[j]);
#endif
					goto check_cs2_end;
				}
			}
			nand_cfg->chipmap[nand_cfg->chipnum] = 2;
			printk("chipmap[%d] = 2\n", nand_cfg->chipnum);
			nand_cfg->chipnum++;
			printk("the third chip is found\n");
	}
check_cs2_end:
	
	/* check the fourth nand chip*/
	if(cs3_index != -1){
		printk("check cs3_index\n");
		imapx_pad_set_mode(1, 1, cs3_index); //gpio mode
		imapx_pad_set_outdat(1, 1, cs3_index); //set output = 1
		imapx_pad_set_dir(0, 1, cs3_index); //set dir output
		nand_cfg->chipmap[3] = 3;	
		nand_cfg->chipnr = 3;		
#if IMAPX800_NAND_16BIT
			busw = 16;
			nand_readid(mtd, id3, 0, timing, rnbtimeout, phyread, phydelay, busw);
#else
			busw = 8;
			nand_readid(mtd, id3, 0, timing, rnbtimeout, phyread, phydelay, busw);
#endif
			for(j = 0; j < 6; j++){
				if(id3[j] != id[j]){
					//TODO: set pad default
#ifdef INFOTM_NAND_DEBUG
					printk("id%d do not match, %x<=>%x\n", j, id3[j], id[j]);
#endif
					goto check_cs3_end;
				}
			}
			nand_cfg->chipmap[nand_cfg->chipnum] = 3;
			nand_cfg->chipnum++;
			printk("the fourth chip is found\n");
	}
check_cs3_end:	
	nf2_timing_init(nand_cfg->interface, nand_cfg->timing, nand_cfg->rnbtimeout, nand_cfg->phyread, nand_cfg->phydelay, nand_cfg->busw);

	for(i=0; i<(nand_cfg->chipnum); i++){
		nand_cfg->chipnr = i;
		nf2_asyn_reset(mtd);
	}

	if(nand_cfg->chip_pre_device != 2){
		mtd->size = mtd->size * nand_cfg->chipnum;
	}else{
		mtd->size = mtd->size * nand_cfg->chipnum / 2;	
	}
#ifdef INFOTM_NAND_DEBUG
	printk("total mtd->size = 0x%llx, nand_cfg->chipnum = %d\n", mtd->size, nand_cfg->chipnum);
#endif

	if(nand_cfg->read_retry == 1){
		if(nand_cfg->nand_param0 == 0x55){
			if(nand_cfg->nand_param1 == 20){
				if(nand_cfg->nand_param2 == 35){	
					start = 4100ull*256*8192;
					retry_reg_buf[0] = 0xcc;
					retry_reg_buf[1] = 0xbf;
					retry_reg_buf[2] = 0xaa;
					retry_reg_buf[3] = 0xab;
					retry_reg_buf[4] = 0xcd;
					retry_reg_buf[5] = 0xad;
					retry_reg_buf[6] = 0xae;
					retry_reg_buf[7] = 0xaf;
					eslc_reg_buf[0] = 0xb0;
					eslc_reg_buf[1] = 0xb1;
					eslc_reg_buf[2] = 0xa0;
					eslc_reg_buf[3] = 0xa1;
				}
				if(nand_cfg->nand_param2 == 36){
					start = 2052ull*256*8192;
					retry_reg_buf[0] = 0xb0;
					retry_reg_buf[1] = 0xb1;
					retry_reg_buf[2] = 0xb2;
					retry_reg_buf[3] = 0xb3;
					retry_reg_buf[4] = 0xb4;
					retry_reg_buf[5] = 0xb5;
					retry_reg_buf[6] = 0xb6;
					retry_reg_buf[7] = 0xb7;
					eslc_reg_buf[0] = 0xa0;
					eslc_reg_buf[1] = 0xa1;
					eslc_reg_buf[2] = 0xa7;
					eslc_reg_buf[3] = 0xa8;
				}
				retry_param_magic = readl(resved_mem + 8);
				if(retry_param_magic != 0x8a7a6a5a){
					g_retrylevel = 0;

					block = nand_cfg->blocksize;
					nand_cfg->chipnr = 0;
					for(skip = 0; isbad(mtd, (start & ~(block - 1)));
							skip++, start += block) {
							printk("bad block skipped @ 0x%llx\n", start & ~(block - 1));
						if(skip > 50) {
							printk("two many bad blocks skipped"
									"before getting the corrent data.\n");
							return -1;
						}
					}
					page = start >> 13;
					imap_get_retry_param_20_from_page(mtd, (unsigned int *)g_otp_buf, 1024, page);
					otp_start = 0;
					otp_len = 64;
otp_check_again:
					for(k = otp_start,j = (otp_start+otp_len); k<(otp_start + otp_len) && otp_start<1026; k++, j++){
						if((((g_otp_buf[k] | g_otp_buf[j]) & 0xff) != 0xff) && (((g_otp_buf[k] & g_otp_buf[j]) & 0xff) != 0x0)){
							printk("%d, %d, 0x%x, 0x%x\n", k, j, g_otp_buf[k], g_otp_buf[j]);
							otp_start += otp_len * 2;
							if(otp_start >= 896){
								printk("kernel: get nand param failed\n");
								return -1;
							}
							goto otp_check_again;
						}
					}
					printk("otp_start = %d, 0x%x, 0x%x, 0x%x, 0x%x\n", otp_start, g_otp_buf[0], g_otp_buf[1], g_otp_buf[2], g_otp_buf[3]);
					g_otp_buf += otp_start;
					if(bad_mark == 0){
						printk("default param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[0], g_otp_buf[1],g_otp_buf[2],g_otp_buf[3], g_otp_buf[4], g_otp_buf[5],g_otp_buf[6],g_otp_buf[7]);
						printk("1st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[8], g_otp_buf[9],g_otp_buf[10],g_otp_buf[11], g_otp_buf[12], g_otp_buf[13],g_otp_buf[14],g_otp_buf[15]);
						printk("2st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[16], g_otp_buf[17],g_otp_buf[18],g_otp_buf[19], g_otp_buf[20], g_otp_buf[21],g_otp_buf[22],g_otp_buf[23]);
						printk("3st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[24], g_otp_buf[25],g_otp_buf[26],g_otp_buf[27], g_otp_buf[28], g_otp_buf[29],g_otp_buf[30],g_otp_buf[31]);
						printk("4st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[32], g_otp_buf[33],g_otp_buf[34],g_otp_buf[35], g_otp_buf[36], g_otp_buf[37],g_otp_buf[38],g_otp_buf[39]);
						printk("5st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[40], g_otp_buf[41],g_otp_buf[42],g_otp_buf[43], g_otp_buf[44], g_otp_buf[45],g_otp_buf[46],g_otp_buf[47]);
						printk("6st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[48], g_otp_buf[49],g_otp_buf[50],g_otp_buf[51], g_otp_buf[52], g_otp_buf[53],g_otp_buf[54],g_otp_buf[55]);
						printk("7st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[56], g_otp_buf[57],g_otp_buf[58],g_otp_buf[59], g_otp_buf[60], g_otp_buf[61],g_otp_buf[62],g_otp_buf[63]);

					}
				}else{
					printk("get retry param from uboot1\n");
					for(i=0; i<64; i++){
						g_otp_buf[i] = *(resved_mem_u8 + i);	
					}
					printk("default param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[0], g_otp_buf[1],g_otp_buf[2],g_otp_buf[3], g_otp_buf[4], g_otp_buf[5],g_otp_buf[6],g_otp_buf[7]);
					printk("1st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[8], g_otp_buf[9],g_otp_buf[10],g_otp_buf[11], g_otp_buf[12], g_otp_buf[13],g_otp_buf[14],g_otp_buf[15]);
					printk("2st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[16], g_otp_buf[17],g_otp_buf[18],g_otp_buf[19], g_otp_buf[20], g_otp_buf[21],g_otp_buf[22],g_otp_buf[23]);
					printk("3st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[24], g_otp_buf[25],g_otp_buf[26],g_otp_buf[27], g_otp_buf[28], g_otp_buf[29],g_otp_buf[30],g_otp_buf[31]);
					printk("4st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[32], g_otp_buf[33],g_otp_buf[34],g_otp_buf[35], g_otp_buf[36], g_otp_buf[37],g_otp_buf[38],g_otp_buf[39]);
					printk("5st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[40], g_otp_buf[41],g_otp_buf[42],g_otp_buf[43], g_otp_buf[44], g_otp_buf[45],g_otp_buf[46],g_otp_buf[47]);
					printk("6st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[48], g_otp_buf[49],g_otp_buf[50],g_otp_buf[51], g_otp_buf[52], g_otp_buf[53],g_otp_buf[54],g_otp_buf[55]);
					printk("7st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[56], g_otp_buf[57],g_otp_buf[58],g_otp_buf[59], g_otp_buf[60], g_otp_buf[61],g_otp_buf[62],g_otp_buf[63]);
				}
				
				if(nand_cfg->chipnum == 2){
					printk("the second H27UBG8T2CTR or H27UCG8T2ATR found, and get retry table\n");
					nand_cfg->chipnr = 1;
					get_second_otp_table(mtd, start);
				}



			}
			if(nand_cfg->nand_param1 == 26){
				//TODO:
				retry_param_magic = readl(resved_mem + 8);
				g_otp_buf[0] = readl(resved_mem) & 0xff;
				if((retry_param_magic != 0x8a7a6a5a) || (g_otp_buf[0] == 0)){
					g_retrylevel = 0;
					imap_get_retry_param_26(mtd, g_otp_buf);
				}else{
					g_otp_buf[0] = readl(resved_mem) & 0xff;
					g_otp_buf[1] = (readl(resved_mem)>>8) & 0xff;
					g_otp_buf[2] = (readl(resved_mem)>>16) & 0xff;
					g_otp_buf[3] = (readl(resved_mem)>>24) & 0xff;
					g_retrylevel = readl(resved_mem + 4);
				}
				printk("kernel: imap_get_retry_param_26 0x%x, 0x%x, 0x%x, 0x%x, %d\n", g_otp_buf[0], g_otp_buf[1], g_otp_buf[2], g_otp_buf[3], g_retrylevel);
				//imap_get_retry_param_26(mtd, g_otp_buf);	
				if(nand_cfg->chipnum == 2){
					printk("the second H27UBG8T2BTR-BC found, and get retry table\n");
					nand_cfg->chipnr = 1;
					imap_get_retry_param_26(mtd, &g_otp_buf[10]);
					printk("second H27UBG8T2BTR-BC retry table 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[10], g_otp_buf[11], g_otp_buf[12], g_otp_buf[13]);
				}
		
			}
			if(nand_cfg->nand_param1 == 21){
				printk("SAMSUNG 32GB K9GBG08U0B or 64GB K9LCG08U0B nand retry found.\n");
				g_retrylevel = 0;
				imap_get_retry_param_21(mtd, g_otp_buf);
			}


		}
		if(nand_cfg->nand_param0 == 0x75){
			if(nand_cfg->nand_param1 == 20){
				g_retrylevel = 0;
				printk("MT29F64G08CBABA retry find\n");
			}
		}
		if(nand_cfg->nand_param0 == 0x95){
			if(nand_cfg->nand_param1 == 20){
				g_retrylevel = 0;
				printk("SDTNQFAMA-004G or SDTNQFAMA-008G retry find\n");
//				init_retry_param_sandisk19(mtd);
			}
		}
	}

	last_chipnr = 0;
	last_die_page = 0;

	//bbt_table_create(mtd);
	
	/* TODO: finaly, intialize the bbt */
	spin_lock_init(&infotm_mtd_lock);
	init_waitqueue_head(&infotm_mtd_wq);
	infotm_mtd_state = FL_READY;
	
	spin_lock_init(&nand_mtd_lock);

	bbt_table_create(mtd);

	return ret;
}
EXPORT_SYMBOL(nand_scan);

/* infotm nand base:
 * to enable sequence page reading, we implemented a
 * nand driver separate from the nand_base standard.
 * the file structures is ported by warits from the following
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven J. Hill <sjhill@realitydiluted.com>");
MODULE_AUTHOR("Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("Generic NAND flash driver code");

