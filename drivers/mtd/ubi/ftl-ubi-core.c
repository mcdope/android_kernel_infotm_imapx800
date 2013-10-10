/*
 * FTL based on UBI.
 *
 * Author:besto<bestoapache@gmail.com> 
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2008 Embedded Alley Solutions, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Based on mtdblock by:
 * (C) 2000-2003 Nicolas Pitre <nico@cam.org>
 * (C) 1999-2003 David Woodhouse <dwmw2@infradead.org>
 */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mtd/ubi.h>
#include <linux/mutex.h>
#include "ubi.h"

#if 0
#define DBG_S
#endif
#if 1
#define DBG_L
#endif

#ifdef DBG_S
#define dbg_s printk("FTL: %s\n", __func__)
#else
#define dbg_s
#endif

#ifdef DBG_L
#define dbg_l printk("FTL: %d\n", __LINE__)
#else
#define dbg_l
#endif

#define BUF_LEN_MAX 32 

#define OOB_SIZE 16
#define SECTOR_SIZE 512

static short magic_num = 722;

#define PAGE_USING  1
#define PAGE_EMPTY  2
#define	PAGE_DISCARD  3

/**
 * block state index
 * @page_state: the state (USING, DISCARD OR EMPTY) of the page in this block.
 * @using_page: the num of using page numbers of the block.
 */
struct _block_index{
	char *page_state;
	int using_page;
};
/**
 * page buffer
 * @sector: the virtual sector store in this buffer.
 * @read: whether the buffer have been read form nand.
 * @data_buf: data buffer, include page data and oob data.
 * @sector_state: the state of the sectors(whether been changed)
 * @jiffies: the last jiffies time to touch the buffer.
 */
struct _page_buf{
	int sector;
	char read;
	char *data_buf;
	int sector_state;
	unsigned long jiffies;
};

static int leb_num;

static int leb_page_size;
static int leb_page_num;

static int ftl_page_num;
/*
 * the sector numbers of one page
 */
static int page_sector_num;

static struct _block_index *block_index;
static int *sector_index;
static int *sector_id;

static struct _page_buf *page_buf;

static char *buf_tmp;

static int last_using_block;
/*
 * the state when the sectors in the page were all changed
 */
static int full_state;
/*
 * block id 
 * a number to make us know which page contained useful data when more two pages record the same virtual sector.
 */
static int block_id;

static int checking_count;

static int import_sector;

static char *block_record;

/*
 * create block, page, sector index in memory.
 */

static int create_index(struct ubi_volume_desc *ubi, int sector_num){

	int i, err;
	int block, page,old_page;
	int num,id;
	char oob_buf[OOB_SIZE];
	short magic;

	printk("FTL: create ftl index...\n");
        printk("FTL: TEST 12\n");

	for(i = 0; i < sector_num; i++){
		sector_id[i] = -1;
		sector_index[i] = -1;
	}
	for(block = 0; block < leb_num; block++){
		memset(block_index[block].page_state, PAGE_EMPTY, ftl_page_num);
	}

	for(block = 0; block < leb_num; block++){
		if(ubi_page_read(ubi, block, buf_tmp, ftl_page_num*leb_page_size, 0, 1)){
			dbg_l;
			return -EIO;
		}
		/*
		 * the magic num will store after all the data in the last page.
		 */
		if(*(short *)(buf_tmp+ftl_page_num*sizeof(int)) ==  magic_num){
			/*
			 * the id will store after the magic num.
			 */
			memcpy(&id, buf_tmp+ftl_page_num*sizeof(int)+sizeof(short), sizeof(int));
			for(page = 0; page < ftl_page_num; page++){
				memcpy(&num, buf_tmp+page*sizeof(int), sizeof(int));
				if(sector_id[num] <= id){
					block_id = block_id<id?id:block_id;
					old_page = sector_index[num];
					if(old_page != -1){
						block_index[old_page/ftl_page_num].page_state[old_page%ftl_page_num] = PAGE_DISCARD;
					}
					sector_id[num] = id;
					sector_index[num] = page+block*ftl_page_num;
					block_index[block].page_state[page] = PAGE_USING;

				}else{
					block_index[block].page_state[page] = PAGE_DISCARD;
				}
			}
		}else{
			for(page = 0; page < ftl_page_num; page++){
				/* read oob data first*/
				if(ubi_page_read(ubi, block, oob_buf, page*leb_page_size, 0, 0)){
					dbg_l;
					return -EIO;
				}
				/* oob data after the page data,
				 * oob data contain 11 bytes,the first 2 bytes is the magic number to make sure this page is using
				 * the next 4 bytes is the virtual sector number.
				 * the next 4 bytes is the block id.
				 * last 1 byte is not been used.
				 */
				memcpy(&magic, oob_buf, sizeof(short));
				memcpy(&num, oob_buf + sizeof(short), sizeof(int));
				memcpy(&id, oob_buf + sizeof(short) + sizeof(int), sizeof(int));
				if(magic == magic_num){
					if(sector_id[num] <= id){
						block_id = block_id<id?id:block_id;
						old_page = sector_index[num];
						if(old_page != -1){
							block_index[old_page/ftl_page_num].page_state[old_page%ftl_page_num] = PAGE_DISCARD;
						}
						sector_id[num] = id;
						sector_index[num] = page+block*ftl_page_num;
						block_index[block].page_state[page] = PAGE_USING;

					}else{
						block_index[block].page_state[page] = PAGE_DISCARD;
					}
				}else{
					break;
				}
			}
		}
	}

	/* 
	 * create the block using page numbers and store them in the memory
	 */
	for(block = 0; block < leb_num; block++){
		block_index[block].using_page = 0;
		for(page = 0; page < ftl_page_num; page++){
			if(block_index[block].page_state[page] == PAGE_USING) block_index[block].using_page++;
		}
		if(block_index[block].using_page == 0){
			err = ubi_leb_erase(ubi, block);
			if(err){
				dbg_l;
				return err;
			}
			memset(block_index[block].page_state, PAGE_EMPTY, ftl_page_num);
		}

	}

	printk("FTL: create index ok!\n");

	return 0;
}
/*
 * ftl init.
 * malloc some memory
 */
int ftl_core_io_init(struct ubi_volume_desc *ubi, int _leb_num, int _leb_page_num, int _leb_page_size, int sector_num){

	int i, ret;
	int block,page;
	char oob_buf[OOB_SIZE];

	if(block_index != NULL) return 0;

	block_id = 0;

	leb_num = _leb_num;
	leb_page_num = _leb_page_num;
	leb_page_size = _leb_page_size;

	ftl_page_num = leb_page_num - 1;

	page_sector_num =  leb_page_size/SECTOR_SIZE;

	sector_num = sector_num / page_sector_num;

	import_sector = sector_num >> 9;

	sector_index = kmalloc(sector_num*sizeof(int), GFP_KERNEL);
	sector_id = kmalloc(sector_num*sizeof(int), GFP_KERNEL);
	block_index = kmalloc(leb_num*sizeof(struct _block_index), GFP_KERNEL);
	page_buf = kmalloc(BUF_LEN_MAX*sizeof(struct _page_buf), GFP_KERNEL);

	block_record = kmalloc(leb_page_size+OOB_SIZE, GFP_KERNEL);

	buf_tmp = kmalloc(leb_page_size+OOB_SIZE, GFP_KERNEL);

	if(block_index == NULL || sector_index == NULL || page_buf == NULL 
                || buf_tmp == NULL || sector_id == NULL || block_record == NULL) return -ENOMEM;

	for(i = 0; i < leb_num; i++){
		block_index[i].page_state = kmalloc(ftl_page_num*sizeof(char), GFP_KERNEL);
		if(block_index[i].page_state == NULL) return -ENOMEM;
	}

	for(i = 0; i < BUF_LEN_MAX; i++){
		page_buf[i].data_buf = kmalloc(leb_page_size+OOB_SIZE, GFP_KERNEL);
		if (page_buf[i].data_buf ==  NULL) return -ENOMEM;
		page_buf[i].read = 0;
		page_buf[i].sector = -1;
		page_buf[i].sector_state = 0;
	}

	ret = create_index(ubi, sector_num);
	if (ret) return ret;

	kfree(sector_id);

	/*
	 * find the first empty page to save time.
	 */

	for(block = 0; block < leb_num; block++){
		for(page = 0; page < ftl_page_num; page++){
			if(block_index[block].page_state[page] == PAGE_EMPTY) break;
		}
		if(page != ftl_page_num && page != 0){
			last_using_block = block;
			for(page = 0; block_index[block].page_state[page] != PAGE_EMPTY; page++){
				if(ubi_page_read(ubi, block, oob_buf, page*leb_page_size, 0, 0)){
					return -EIO;
				}
				memcpy(block_record + sizeof(int)*(page%ftl_page_num), oob_buf+sizeof(short), sizeof(int));
			}
			goto end;
		}
	}
	for(block = 0; block < leb_num; block++){
		if(block_index[block].page_state[0] == PAGE_EMPTY){
			last_using_block = block;
			block_id++;
			break;
		}
	}
end:
	checking_count = 0;
	full_state = 0;
	for(i = 0; i < page_sector_num; i++){
		int mask = 1<<i;
		full_state |= mask;
	}
	return 0;
}
/*
 * set buffer state
 */
static void set_page_buf_state(int buf_pos, int sector_pos){
	int mask = 1<<sector_pos;
	page_buf[buf_pos].sector_state |= mask;
}
/*
 * get buffer state
 */
static int get_page_buf_state(int buf_pos, int sector_pos){
	int mask = 1<<sector_pos;
	return page_buf[buf_pos].sector_state & mask;
}

static int is_sector_in_buf(long sector){
	int i;
	int virtual_sector = sector / page_sector_num;

	for(i = 0; i < BUF_LEN_MAX; i++){
		if(virtual_sector == page_buf[i].sector) return i;
	}
	return -1;
}

static int get_new_buf(void){
	int i;
	for(i = 0; i < BUF_LEN_MAX; i++){
		if(page_buf[i].sector == -1){
			return i;
		}
	}
	return -1;
}
/*
 * get new empty to write
 */
static int get_new_page(void){

	int block, page;

	for(page = 0; page < ftl_page_num; page++){
		if(block_index[last_using_block].page_state[page] == PAGE_EMPTY) return last_using_block*ftl_page_num+page;
	}

	for(block = 0; block < leb_num; block++){
		if(block_index[block].page_state[0] == PAGE_EMPTY){
			last_using_block = block;
			block_id++;
			return block*ftl_page_num;
		}
	}
	return -1;
}

static int get_empty_block_num(void){

	int block;
	int n = 0;

	for(block = 0; block < leb_num; block++){
		if(block_index[block].page_state[0] ==  PAGE_EMPTY) n++;
	}
	return n;
}
/*
 * return the using page number of the block.
 * if the block have empty page will return the max number to avoid it to be recycle.
 */
static int get_block_using_page_num(int block){

	if((block == last_using_block && block_index[block].page_state[ftl_page_num-1] == PAGE_EMPTY) 
			|| block_index[block].page_state[0] == PAGE_EMPTY)
		return ftl_page_num;
	return block_index[block].using_page;
}

static int recycle_block(struct ubi_volume_desc *ubi, int block);
static int garbage_checking(struct ubi_volume_desc *ubi, int low_block_num);
/*
 * sync one page buffer.
 * write one page buffer data to the nand.
 */
static int sync_one_buf(struct ubi_volume_desc *ubi, int buf_pos){

	int i, new_page, old_page;
	int err = 0;

	if(page_buf[buf_pos].sector_state == 0){
		goto end;
	}

	/* 
	 * if the page buffer has not been read and not all sectors in this page have been change, will read the page from nand
	 * and set the sectors which have not been change the origin data
	 */
	if(page_buf[buf_pos].sector_state != full_state && !page_buf[buf_pos].read && sector_index[page_buf[buf_pos].sector] != -1){
		int block = sector_index[page_buf[buf_pos].sector] / ftl_page_num;
		int page = sector_index[page_buf[buf_pos].sector] % ftl_page_num;
		err = ubi_page_read(ubi, block, buf_tmp, page*leb_page_size, 0, 1);
		if(err){
			dbg_l;
			return err;
		}
		for(i = 0; i < page_sector_num; i++){
			if(!get_page_buf_state(buf_pos, i)){
				memcpy(page_buf[buf_pos].data_buf+SECTOR_SIZE*i, buf_tmp+SECTOR_SIZE*i, SECTOR_SIZE);
			}
		}
	}

	/*
	 * write the page buffer to the nand
	 */
	new_page = get_new_page();

	if(new_page == -1){

		return -EIO;
	}

	memcpy(page_buf[buf_pos].data_buf+leb_page_size, &magic_num, sizeof(short));
	memcpy(page_buf[buf_pos].data_buf+leb_page_size+sizeof(short), &page_buf[buf_pos].sector, sizeof(int));
	memcpy(page_buf[buf_pos].data_buf+leb_page_size+sizeof(short)+sizeof(int), &block_id, sizeof(int));

	err = ubi_page_write(ubi, new_page/ftl_page_num, page_buf[buf_pos].data_buf, new_page%ftl_page_num*leb_page_size, UBI_UNKNOWN);

	if(err) {
		dbg_l;
		return err;
	}

	memcpy(block_record + sizeof(int)*(new_page%ftl_page_num), &page_buf[buf_pos].sector, sizeof(int));
	if(new_page%ftl_page_num == ftl_page_num - 1){
		memcpy(block_record + ftl_page_num*sizeof(int), &magic_num, sizeof(short));
		memcpy(block_record + ftl_page_num*sizeof(int)+sizeof(short), &block_id, sizeof(int));
		err = ubi_page_write(ubi, new_page/ftl_page_num, block_record, ftl_page_num*leb_page_size, UBI_UNKNOWN);
		if(err) {
			return err;
		}
	}

	old_page = sector_index[page_buf[buf_pos].sector];

	block_index[new_page/ftl_page_num].page_state[new_page%ftl_page_num] = PAGE_USING;
	block_index[new_page/ftl_page_num].using_page++;
	sector_index[page_buf[buf_pos].sector] = new_page;

	/*
	 * if the block where the old page locate have no using page, will be recycle
	 */
	if(old_page != -1){
		int block = old_page/ftl_page_num;
		block_index[block].page_state[old_page%ftl_page_num] = PAGE_DISCARD;
		block_index[block].using_page--;
		if(get_block_using_page_num(block) == 0){
			err = recycle_block(ubi, block);
			if(err) return err;
		}
	}


	/*
	 * if write page count more than one number and empty leb number less than 5, garbage checking will run.
	 */
	checking_count++;
	if(checking_count > ftl_page_num && get_empty_block_num() < 5){
		err = garbage_checking(ubi, 10);
		if(err) return err;
		checking_count = 0;
	}
end:
	page_buf[buf_pos].sector = -1;
	page_buf[buf_pos].read = 0;
	page_buf[buf_pos].sector_state = 0;

	return 0;
}
/*
 * find the page buffer than not been used and sync it
 */
static int flush_one_buf(struct ubi_volume_desc *ubi){
	int i;
	int err = 0;
	int buf_pos = 0;
	unsigned long jiffies_tmp = jiffies;

	for(i = 0; i < BUF_LEN_MAX; i++){
		if(page_buf[i].jiffies < jiffies_tmp){
			jiffies_tmp = page_buf[i].jiffies;
			buf_pos = i;
		}
	}
	err = sync_one_buf(ubi, buf_pos);

	return err;
}

int ftl_core_io_read(struct ubi_volume_desc *ubi, long sector, char *buf){

	long buf_pos = is_sector_in_buf(sector);
	int err = 0;
	int block, page;

	/* if the sector is in page buffer and has been read from nand return it
	 * or read it to nand
	 */
	if(buf_pos == -1){

		if(sector_index[sector/page_sector_num] == -1) return 0;

		buf_pos = get_new_buf();
		if(buf_pos == -1){
			err = flush_one_buf(ubi);
			if(err) {
				dbg_l;
				return err;
			}
			buf_pos = get_new_buf();
			if(buf_pos == -1){
				dbg_l;
				return -ENOMEM;
			}
		}
		page_buf[buf_pos].sector = sector/page_sector_num;
		page_buf[buf_pos].read = 1;
		page_buf[buf_pos].sector_state = 0;
		block = sector_index[page_buf[buf_pos].sector] / ftl_page_num;
		page = sector_index[page_buf[buf_pos].sector] % ftl_page_num;
		err = ubi_page_read(ubi, block, page_buf[buf_pos].data_buf, page*leb_page_size, 0, 1);
		if(err) {
			printk("FTL: err = %d, block = %d, page = %d\n", err, block, page);
			dbg_l;
			return err;
		}
		memcpy(buf, page_buf[buf_pos].data_buf+sector%page_sector_num*SECTOR_SIZE, SECTOR_SIZE); 
		page_buf[buf_pos].jiffies = jiffies;
	}else{
		if(!get_page_buf_state(buf_pos, sector%page_sector_num) && !page_buf[buf_pos].read && sector_index[page_buf[buf_pos].sector] != -1){
			int i;
			block = sector_index[page_buf[buf_pos].sector] / ftl_page_num;
			page = sector_index[page_buf[buf_pos].sector] % ftl_page_num;
			err = ubi_page_read(ubi, block, buf_tmp, page*leb_page_size, 0, 1);
			if(err) {
				dbg_l;
				printk("FTL: err = %d, block = %d, page = %d\n", err, block, page);
				return err;
			}
			for(i = 0; i < page_sector_num; i++){
				if(!get_page_buf_state(buf_pos, i)){
					memcpy(page_buf[buf_pos].data_buf+SECTOR_SIZE*i, buf_tmp+SECTOR_SIZE*i, SECTOR_SIZE);
				}
			}
			page_buf[buf_pos].read = 1;
		}
		memcpy(buf, page_buf[buf_pos].data_buf+sector%page_sector_num*SECTOR_SIZE, SECTOR_SIZE); 
		page_buf[buf_pos].jiffies = jiffies;
	}
	return 0;
}


/*
 * recycle block
 * write the using pages other block
 */
static int recycle_block(struct ubi_volume_desc *ubi, int block){

	int page,new_page;
	int sector;
	int err = 0;

	if(block_index[block].using_page == 0) goto end;
	
	//printk("FTL: recycle block %d,using page %d\n", block, block_index[block].using_page);

	for(page = 0; page < ftl_page_num; page++){
		if(block_index[block].page_state[page] == PAGE_USING){
			if(ubi_page_read(ubi, block, buf_tmp, page*leb_page_size, 0, 1)){
				dbg_l;
				return -EIO;
			}
			memcpy(&sector, buf_tmp+leb_page_size+sizeof(short), sizeof(int));

			new_page = get_new_page();

			if (new_page < 0){
				return err;
			}

			sector_index[sector] = new_page;
			block_index[new_page/ftl_page_num].page_state[new_page%ftl_page_num] = PAGE_USING;
			block_index[new_page/ftl_page_num].using_page++;
			memcpy(buf_tmp+leb_page_size, &magic_num, sizeof(short));
			memcpy(buf_tmp+leb_page_size+sizeof(short)+sizeof(int), &block_id, sizeof(int));

			if(ubi_page_write(ubi, new_page/ftl_page_num, buf_tmp, 
						(new_page%ftl_page_num)*leb_page_size, UBI_UNKNOWN)){
				dbg_l;
				return -EIO;
			}	       

			memcpy(block_record + sizeof(int)*(new_page%ftl_page_num), &sector, sizeof(int));
			if(new_page%ftl_page_num == ftl_page_num - 1){
				memcpy(block_record + ftl_page_num*sizeof(int), &magic_num, sizeof(short));
				memcpy(block_record + ftl_page_num*sizeof(int) + sizeof(short), &block_id, sizeof(int));
				err = ubi_page_write(ubi, new_page/ftl_page_num, block_record, ftl_page_num *leb_page_size, UBI_UNKNOWN);

				if(err) {
					return err;
				}
			}
		}
	}
end:
	if(ubi_leb_erase(ubi, block)) {
		dbg_l;
		return -EIO;
	}
	for(page = 0; page < ftl_page_num; page++){
		block_index[block].page_state[page] = PAGE_EMPTY;
	}
	block_index[block].using_page = 0;

	return 0;
}

static int garbage_checking(struct ubi_volume_desc *ubi, int low_block_num){

	int block;
	int err = 0;

	while(get_empty_block_num() < low_block_num){
		int min_using_page = ftl_page_num;
		int block_tmp = -1;
		int n;
		for(block = 0; block < leb_num; block++){
			n = get_block_using_page_num(block);
			if( n < min_using_page){
				min_using_page = n;	
				block_tmp = block;
			}
		}
		if(block_tmp == -1) {
			dbg_l;
			return -EIO;
		}
		err = recycle_block(ubi, block_tmp);
		if(err){
			dbg_l;
			return err;
		}
	}
	return 0;

}

int ftl_core_io_write(struct ubi_volume_desc *ubi, long sector, char *buf){


	long buf_pos;
	int err = 0;

	dbg_s;


	buf_pos = is_sector_in_buf(sector);

	if(buf_pos == -1){
		buf_pos = get_new_buf();
		if(buf_pos == -1){
			err = flush_one_buf(ubi);
			if(err) {
				dbg_l;
				return err;
			}
			buf_pos = get_new_buf();
			if(buf_pos == -1){
				dbg_l;
				return -EFAULT;
			}
		}
		page_buf[buf_pos].sector = sector/page_sector_num;
		page_buf[buf_pos].sector_state = 0;
		memcpy(page_buf[buf_pos].data_buf+sector%page_sector_num*SECTOR_SIZE, buf, SECTOR_SIZE); 
		set_page_buf_state(buf_pos, sector%page_sector_num);
		page_buf[buf_pos].jiffies = jiffies;

	}else{
		memcpy(page_buf[buf_pos].data_buf+sector%page_sector_num*SECTOR_SIZE, buf, SECTOR_SIZE); 
		set_page_buf_state(buf_pos, sector%page_sector_num);
		page_buf[buf_pos].jiffies = jiffies;
	}

	return 0;
}

int ftl_core_io_flush(struct ubi_volume_desc *ubi){	

	int buf_pos;
	int err = 0;

	for(buf_pos = 0; buf_pos < BUF_LEN_MAX; buf_pos++){
		err = sync_one_buf(ubi, buf_pos);
		if(err) return err;
	}
	return 0;
}
/*
 * this function will be used by the auto flush
 */

int ftl_core_io_auto_flush(struct ubi_volume_desc *ubi){

	int buf_pos;
	int err = 0;
	for(buf_pos = 0; buf_pos < BUF_LEN_MAX; buf_pos++){
		if(page_buf[buf_pos].sector == -1 || page_buf[buf_pos].sector_state == 0 ) continue;
		if((page_buf[buf_pos].sector < import_sector && page_buf[buf_pos].jiffies + 199 > jiffies)
				|| page_buf[buf_pos].jiffies + 399 > jiffies) {
			err = sync_one_buf(ubi, buf_pos);
			if(err) return err;
		}
	}
	return 0;
}
