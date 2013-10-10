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
#include <mach/items.h>
#include <mach/imapx_nand.h>
#include "imapx800.h"
#include <mach/pad.h>
#include <mach/power-gate.h>

#define PAD_SYSM_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000))
#define ECC_CLK_SEL_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0xA270))
#define ECC_CLK_DIV_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0xA274))

#define RETRY_TABLE_MAGIC	(0x6756572b)
int nand_debug_en;

int g_otp_start;
unsigned char *g_debug_buf;
unsigned int *g_debug_wbuf;
unsigned char * g_second_otp_buf;
extern unsigned char * g_otp_buf;
extern struct nand_config nf_cfg;
extern unsigned int g_retrylevel;
extern void    __iomem     *sys_rtc_regs;
extern int power_index, cs1_index, cs2_index, cs3_index;
extern unsigned char	retry_reg_buf[8];
extern unsigned char   eslc_reg_buf[8];
extern unsigned int g_retrylevel_chip1;

extern int add_mtd_partitions(struct mtd_info *master, const struct mtd_partition *parts, int nbparts);
extern struct mtd_partition *
imap_auto_part(int *num);
#define __nand_msg(args...) printk(KERN_ERR "infotm-nand: " args)
//#define __nand_dbg(args...) printk(KERN_ERR "infotm-nand: " args)
#define __nand_dbg(args...)
//#define __nand_msg(args...)

#if defined(CONFIG_MTD_NAND_IMAPX800_SDMA)
#define NAND_USE_IRQ
#else
#undef NAND_USE_IRQ
#endif

/* Time out to wait DMA to finish(ms) */
#define NAND_SDMA_TIMEOUT 5000

#if 0
static void inline __nand_dump(const uint8_t *dat, int len)
{
	char str[64];
	int i;

	str[0] = 0;
	for(i = 0; i < len; i++)
	{
		if(!(i & 0xf))
		  __nand_msg("%s\n", str);
		sprintf(str + 3 * (i & 0xf), "%02x ", dat[i]);
	}
}
#endif

//#undef CONFIG_IMAPX800_FPGA_PLATFORM

#undef  pr_fmt
#define pr_fmt(fmt) "NAND DBG: " fmt

/* 0: (0,  2]
 * 1: (2,  4]
 * 2: (4,  8]
 * 3: (8,  16]
 * 4: (16, 20]
 * 5: (20, 24]
 * 6: (24, 32]
 * 7: (32, 40]
 * 8: (40, 127]
 * 9: (unfixed)
 * 10:(normal)
 */
static int mecc_stat[12];

/* 0: (0, 4]
 * 1: (4, 8]
 * 2: (unfixed)
 * 3: (normal)
 */
static int secc_stat[4];

unsigned int nf2_secc_cap (unsigned secc_type);
static void ecc_add_stat(int type, int bits);
int nf2_check_irq(void);
void trx_nand_cs(struct mtd_info *mtd, int valid, int chipnr);
void trx_afifo_intr_clear(void);

struct imap_nand_dma {
	uint32_t dma_addr;
	uint32_t len;
	struct page *page;
};

struct imap_nand_dma imap_nand_dma_list[MAX_DMABUF_CNT];

const char *part_probes[] = { "cmdlinepart", NULL };

/* The structions used by NAND driver */
struct imap_nand_device {

	/* basic */
	struct	mtd_info	*mtd;
	struct	nand_chip	*chip;

	/* resource */
	struct	device		*dev;
    struct  resource    *area;
    struct  clk         *clk;
    void    __iomem     *regs;
	int		irqno;
	wait_queue_head_t	wq;

	/* device flags */
	int		cur_ecc_mode;
};

static struct imap_nand_device imap_nand;
static int g_nand_state;
static int irq_condition;
static int imap_nand_inited = 0;

#if defined(NAND_USE_IRQ)
static irqreturn_t imap_nand_irq(int irq, void *id)
{
	/* TODO: irq ... */
	
	
	unsigned int cInt = 0;
	unsigned int val0 = 0, val1 = 0;

	val0 = readl(imap_nand.regs + NF2INTR);	
	val1 = readl(imap_nand.regs + NF2INTE);
    	cInt = val0 & val1 ;    // valid flag

	//printk("irq 0x%x 0x%x\n", val0, val1);	
	if(g_nand_state == IMAPX800_NAND_WRITE){
		if(cInt & 0x1){
			irq_condition = 1;
			wake_up(&imap_nand.wq);			
		}	
	}
	if(g_nand_state == IMAPX800_NAND_READ){
		if(val0 & (0x1<<13)){
			printk("read RNB timeout\n");
		}
		
		if(cInt & (0x1<<5)){
	//		printk("readirq 0x%x\n", cInt);
			irq_condition = 1;
			wake_up(&imap_nand.wq);
		}	
	}
	trx_afifo_intr_clear();
	
	return IRQ_HANDLED;
}
#endif

int imap_nand_valied_vm(uint32_t vm_addr, int len){
	
	if(virt_addr_valid(vm_addr) && virt_addr_valid(vm_addr + len))
	{
		return 0;
	}else{
		return -1;
	}

}

static int imap_nand_map_vm(uint32_t vm_addr, int len,
   int index, int *curindex, enum dma_data_direction dir)
{
	int page_tail = 0;
	uint32_t logic_addr = 0;
	if(virt_addr_valid(vm_addr) && virt_addr_valid(vm_addr + len))
	{
		if(index >= MAX_DMABUF_CNT){
			printk("index > MAX_DMABUF_CNT\n");
			return 0;
		}
		/* This is a valid kernel logical address */
		imap_nand_dma_list[index].dma_addr = dma_map_single(NULL,
		   (void *)vm_addr, len, dir);
		imap_nand_dma_list[index].len = len;
	}
	else if(is_vmalloc_addr((void *)vm_addr))
	{
		/* This is a vmalloc address */
		/* FIXME: I can't handle longer then 12KB, and this
		   will never happen */
		BUG_ON(len > (12 << 10));
		while(len)
		{
			if(index >= MAX_DMABUF_CNT){
				printk("too many buf alloc\n");
				return 0;
			}
			page_tail = PAGE_SIZE - (vm_addr & (PAGE_SIZE - 1));

			imap_nand_dma_list[index].page = vmalloc_to_page((void *)vm_addr);
			if(len > page_tail)
			  imap_nand_dma_list[index].len = PAGE_SIZE - (vm_addr & (PAGE_SIZE - 1));
			else
			  imap_nand_dma_list[index].len = len;

			/* Map page to kernel */
			logic_addr = (uint32_t)kmap(imap_nand_dma_list[index].page);
			/* add offset */
			logic_addr += vm_addr & (PAGE_SIZE - 1);
			
			imap_nand_dma_list[index].dma_addr = dma_map_single(NULL,
			   (void *)logic_addr, imap_nand_dma_list[index].len, dir);

			vm_addr += imap_nand_dma_list[index].len;
			len		-= imap_nand_dma_list[index].len;
			++index;
		}
	}
	else
	{
		return 0;
		/*FIXME: I suppose User-Space buffer will not get here. */
		BUG();
	}

	*curindex = index;
	/* Map successed */
	return 1;
}



static void imap_nand_unmap_vm(enum dma_data_direction dir)
{
	int i;
	for (i = 0; i < MAX_DMABUF_CNT; i++)
	{
		if(imap_nand_dma_list[i].dma_addr)
		  dma_unmap_single(NULL, imap_nand_dma_list[i].dma_addr,
			 imap_nand_dma_list[i].len, dir);
		if(imap_nand_dma_list[i].page)
		  kunmap(imap_nand_dma_list[i].page);

		imap_nand_dma_list[i].dma_addr = 0;
		imap_nand_dma_list[i].len = 0;
		imap_nand_dma_list[i].page = NULL;
	}
}

int nf2_set_polynomial(int polynomial)
{
	writel(polynomial, imap_nand.regs + NF2RANDMP);	
	return 1;
}


unsigned int nf2_randc_seed_hw (unsigned int raw_seed, unsigned int cycle, int polynomial)
{
	unsigned int result;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;

	writel(cycle << 16 | raw_seed, imap_nand.regs + NF2RAND0);
	writel(0x1, imap_nand.regs + NF2RAND1);
	writel(0x0, imap_nand.regs + NF2RAND1);
	
        tend = cpu_clock(UINT_MAX) + 20000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
		result = readl(imap_nand.regs + NF2RAND2);
		if((result & 0x10000) == 0){
		    timeout = 0;
		    break;
		}
	}
    	
	result = readl(imap_nand.regs + NF2RAND2) & 0xffff;
	
	if(timeout != 0){
    	    	result = -1;
		printk("nf2_randc_seed_hw timeout\n");
	}
	return result;

}

int nand_init_randomizer(struct nand_config *cfg)
{
	int ecc_num, i;
	int pagesize, sysinfosize;

	if(cfg->randomizer) {
		ecc_num = nf2_ecc_num(cfg->mecclvl, cfg->pagesize, 0);
		printk("i: randomizer (%d)\n", ecc_num);
//		printf("ecc_num = %d\n", ecc_num);
	
		pagesize = cfg->pagesize;
		sysinfosize = cfg->sysinfosize;
		if(cfg->busw == 16){
			pagesize = (cfg->pagesize) >> 1;
			sysinfosize = (cfg->sysinfosize) >> 1;
			ecc_num = ecc_num >> 1;	
		}

		nf2_set_polynomial(cfg->polynomial);
		for(i = 0; i < 8; i++)
		{
			cfg->sysinfo_seed[i] = nf2_randc_seed_hw(cfg->seed[i] & 0x7fff,
						(pagesize - 1), cfg->polynomial);
			if(cfg->busw == 16){
				cfg->ecc_seed[i] = nf2_randc_seed_hw(cfg->sysinfo_seed[i],
							(sysinfosize + 2 - 1), cfg->polynomial);	
			}else{
				cfg->ecc_seed[i] = nf2_randc_seed_hw(cfg->sysinfo_seed[i],
							(sysinfosize + 4 - 1), cfg->polynomial);
			}
			cfg->secc_seed[i] = nf2_randc_seed_hw(cfg->ecc_seed[i], (ecc_num - 1), cfg->polynomial);
		}

	}

	return 0;
}


unsigned int nf2_ecc_cap (unsigned int ecc_type)
{
	unsigned int ecc_cap = 0;
	
    switch (ecc_type) {
        case 0 : {  ecc_cap = 2;} break;		//2bit
        case 1 : {  ecc_cap = 4;} break;		//4bit
        case 2 : {  ecc_cap = 8;} break;		//8bit
        case 3 : {  ecc_cap = 16;} break;		//16bit
        case 4 : {  ecc_cap = 24;} break;		//24bit
        case 5 : {  ecc_cap = 32;} break;		//32bit
        case 6 : {  ecc_cap = 40;} break;		//40bit
        case 7 : {  ecc_cap = 48;} break;		//48bit
        case 8 : {  ecc_cap = 56;} break;		//56bit
        case 9 : {  ecc_cap = 60;} break;		//60bit
        case 10 : {  ecc_cap = 64;} break;		//64bit
        case 11 : {  ecc_cap = 72;} break;		//72bit
        case 12 : {  ecc_cap = 80;} break;		//80bit
        case 13 : {  ecc_cap = 96;} break;		//96bit
        case 14 : {  ecc_cap = 112;} break;		//112bit
        case 15 : {  ecc_cap = 127;} break;		//127bit
        default : break;
    }	
    
    return ecc_cap;
}

int nf2_soft_reset(int num)
{
    	volatile unsigned int tmp = 0;
    	int ret = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;

        tend = cpu_clock(UINT_MAX) + 20000000;

    	writel(num, imap_nand.regs + NF2SFTR);
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2SFTR) & (0x1f);
    		if(tmp == 0){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
    	    	ret = -1;
    		tmp = readl(imap_nand.regs + NF2SFTR) & (0x1f);
		printk("nf2_soft_reset timeout, tmp = 0x%x, num = %d\n", tmp, num);
	}

    	return ret;
    
}

int nf2_ecc_num (int ecc_type, int page_num, int half_page_en)
{
    int kb_cnt = 0;
    int ecc_num = 0;

    kb_cnt = (page_num >> 10);

    switch (ecc_type) {
        case 0 : {  ecc_num = 4*kb_cnt;} break;			//2bit
        case 1 : {  ecc_num = 8*kb_cnt;} break;			//4bit
        case 2 : {  ecc_num = 16*kb_cnt;} break;		//8bit
        case 3 : {  ecc_num = 28*kb_cnt;} break;		//16bit
        case 4 : {  ecc_num = 44*kb_cnt;} break;		//24bit
        case 5 : {  ecc_num = 56*kb_cnt;} break;		//32bit
        case 6 : {  ecc_num = 72*kb_cnt;} break;		//40bit
        case 7 : {  ecc_num = 84*kb_cnt;} break;		//48bit
        case 8 : {  ecc_num = 100*kb_cnt;} break;		//56bit
        case 9 : {  ecc_num = 108*kb_cnt;} break;		//60bit
        case 10 : {  ecc_num = 112*kb_cnt;} break;		//64bit
        case 11 : {  ecc_num = 128*kb_cnt;} break;		//72bit
        case 12 : {  ecc_num = 140*kb_cnt;} break;		//80bit
        case 13 : {  ecc_num = 168*kb_cnt;} break;		//96bit
        case 14 : {  ecc_num = 196*kb_cnt;} break;		//112bit
        case 15 : {  ecc_num = 224*kb_cnt;} break;		//127bit
        default : break;
    }
    
    if(half_page_en == 1)
    {
    	ecc_num = ecc_num << 1;
    }


    return ecc_num;
}

void nf2_ecc_cfg(int ecc_enable, int page_mode, int ecc_type, int eram_mode, 
		 int tot_ecc_num, int tot_page_num,  int trans_dir, int half_page_en)
{
	unsigned int tmp = 0;
	unsigned int ecc_cap = 0;	
	unsigned int val = 0;
	
	val  = (half_page_en<<18) | (eram_mode <<12) | (page_mode<<11) | (3<<8) | (ecc_type<<4) | (trans_dir<<1) | ecc_enable;
    	writel(val, imap_nand.regs + NF2ECCC);
    
	val  = ((tot_ecc_num)<<16) | (tot_page_num);
	writel(val, imap_nand.regs + NF2PGEC);
	
	ecc_cap = nf2_ecc_cap(ecc_type);
	tmp = readl(imap_nand.regs + NF2ECCLEVEL);
	tmp &= ~(0x7f<<8);
	tmp |= (ecc_cap << 8);
	writel(tmp, imap_nand.regs + NF2ECCLEVEL);
	//TODO
	//nf_dbg("NF2ECCLEVEL = 0x%x\n", tmp);
}

void nf2_secc_cfg(int secc_used, int secc_type, int rsvd_ecc_en)
{
	volatile unsigned int tmp;
	unsigned int secc_cap;
	
	tmp = readl(imap_nand.regs + NF2ECCC);
	tmp |= (secc_type <<17) | (secc_used <<16) | (rsvd_ecc_en<<3);
	writel(tmp, imap_nand.regs + NF2ECCC);
	
	secc_cap = nf2_secc_cap (secc_type);
	tmp = readl(imap_nand.regs + NF2ECCLEVEL);
	tmp &= ~(0xf<<16);
	tmp |= (secc_cap << 16);
	writel(tmp, imap_nand.regs + NF2ECCLEVEL);
}

void nf2_sdma_cfg(int ch0_adr, int ch0_len, int ch1_adr, int ch1_len, int sdma_2ch_mode)
{
	int dma_bst_len     = 1024;
	int blk_number		= 1;
	int adma_mode		= 0;	//SDMA
    	int bufbound        = 0;
    	int bufbound_chk    = 0;    // not check page address boundary
    	int dma_enable      = 1;
	unsigned int value = 0;

	writel(ch0_adr, imap_nand.regs + NF2SADR0);
	writel(ch1_adr, imap_nand.regs + NF2SADR1);
	writel((ch1_len<<16 | ch0_len), imap_nand.regs + NF2SBLKS);
   
	value = (dma_bst_len<<16) | blk_number;
       	writel(value, imap_nand.regs + NF2SBLKN);	
    
	value = (bufbound<<4) | (bufbound_chk<<3) | (sdma_2ch_mode<<2) | (adma_mode<<1) | dma_enable;
	writel(value, imap_nand.regs + NF2DMAC);
    
   	return;
}

void nf2_addr_cfg(int row_addr0, int col_addr, int ecc_offset, int busw)
{
	unsigned int tmp;
       	
	if(busw == 16){
		col_addr = col_addr >> 1;
		ecc_offset = ecc_offset >> 1;
	}

	tmp = (ecc_offset<<16) | col_addr;

	writel(row_addr0, imap_nand.regs + NF2RADR0);
	writel(0x0, imap_nand.regs + NF2RADR1);
	writel(0x0, imap_nand.regs + NF2RADR2);
	writel(0x0, imap_nand.regs + NF2RADR3);
	writel(tmp, imap_nand.regs + NF2CADR);	
}

/*
    @func   void | trx_afifo_ncmd | 
    @param	NULL
    @comm	nand Normal CMD/ADR/DAT op
    @xref
    @return NULL
*/
void trx_afifo_ncmd (unsigned int flag, unsigned int type, unsigned int wdat)
{
	unsigned int value = 0;
	
	value = (0x1<<14) | ((flag & 0xf) <<10) | (type<<8) | wdat;
	writel(value, imap_nand.regs + NF2TFPT);
}

void trx_afifo_reg (unsigned int flag, unsigned int addr, unsigned int data)
{
	unsigned int value = 0;

	if (flag==0){   // 32-bit register write
		value = (0xf<<12) | (flag<<8) | addr;
		writel(value, imap_nand.regs + NF2TFPT);
	    	value = data & 0xffff;
	    	writel(value, imap_nand.regs + NF2TFPT);
	    	value = (data>>16) & 0xffff;
	    	writel(value, imap_nand.regs + NF2TFPT);
	    }
	else if (flag==1){   // 16-bit register write
	    	value = (0xf<<12) | (flag<<8) | addr;
	    	writel(value, imap_nand.regs + NF2TFPT);
	    	value = data & 0xffff;
	    	writel(value, imap_nand.regs + NF2TFPT);
	    }

    	else {
        	value = (0xf<<12) | (flag<<8) | addr;
		writel(value, imap_nand.regs + NF2TFPT);
	    }

}

/*
    @func   void | trx_afifo_nop | 
    @param	NULL
    @comm	nand nop cmd
    @xref
    @return NULL
*/
void trx_afifo_nop (unsigned int flag, unsigned int nop_cnt)
{
	unsigned int value = 0;

	value = ((flag & 0xf)<<10) | (nop_cnt & 0x3ff);
	writel(value, imap_nand.regs + NF2TFPT);
}

int nf2_check_irq(void)
{
	unsigned int cInt = 0;
	unsigned int val0 = 0;
	
	val0 = readl(imap_nand.regs + NF2INTR);


	if(g_nand_state == IMAPX800_NAND_WRITE){
		cInt = val0 & 0x1001;	
	}
	if(g_nand_state == IMAPX800_NAND_READ){
		cInt = val0 & 0x20;
	}

    if (cInt & (1<<18)){    // cpu free int
        val0 = readl(imap_nand.regs + NF2INTE);
	val0 &= ~(0x1<<18); // disable free int enable
	writel(val0, imap_nand.regs + NF2INTE);
	val0  = (1<<18);
        writel(val0, imap_nand.regs + NF2INTR);
        cInt &= ~(1<<18);
    }
    else if (cInt & (1<<17)){    // cpu word int
        val0 = readl(imap_nand.regs + NF2INTE);
	val0 &= ~(1<<17);    // disable word int enable
	writel(val0, imap_nand.regs + NF2INTE);
	val0 = (1<<17);
        writel(val0, imap_nand.regs + NF2INTR);
        cInt &= ~(1<<17);
    }
    else if ((cInt & (1<<10)) && (cInt & (1<<19))) {    // cpu data int and last data int
        val0 = readl(imap_nand.regs + NF2INTE);
	val0 &= ~((1<<19) | (1<<10));    // disable data int and last data int enable
	writel(val0, imap_nand.regs + NF2INTE);
        val0 = (1<<19) | (1<<10);
	writel(val0, imap_nand.regs + NF2INTR);
        cInt &= ~(1<<19 | 1<<10);
    }
    else if (cInt & (1<<10)){    // cpu data int
        val0 = readl(imap_nand.regs + NF2INTE);
	val0 &= ~(1<<10);    // disable data int enable
	writel(val0, imap_nand.regs + NF2INTE);
        val0 = (1<<10);
	writel(val0, imap_nand.regs + NF2INTR);
        cInt &= ~(1<<10);
    }
    else if(cInt & (1<<12)){
   	printk("write error state\n");
        writel(0x1000, imap_nand.regs + NF2INTE);	
    }	    	    
    else if (cInt & (1<<0)) {   // trxf-empty int
	writel(cInt, imap_nand.regs + NF2INTR);
        //val0 = 0xf;  // cs0 invalid
	//writel(val0, imap_nand.regs + NF2CSR);
    }
    else if (cInt & (1<<1)) {   // trxf-Line int
        val0 = (1<<1);
	writel(val0, imap_nand.regs + NF2INTR);
        val0  = 1<<0;     // trxf-line int ack.
	writel(val0, imap_nand.regs + NF2INTA);
    }
    else if (cInt & (1<<15)) {   // trxf-repeat nested error int
        val0 = (1<<15);
        writel(val0, imap_nand.regs + NF2INTR);
        cInt &= ~(1<<15);
    }
    else if (cInt & (1<<8)) {   // dec error number threshold
        val0 = (1<<8);
        writel(val0, imap_nand.regs + NF2INTR);
        cInt        &= ~(1<<8);
    }
    else if (cInt & (1<<9)) {   // dec uncorrectable
        val0 = (1<<9);
        writel(val0, imap_nand.regs + NF2INTR);
	(*(volatile int *)(0x43ffF088)) = 0x39;	//step
        cInt        &= ~(1<<9);
    }
    else if (cInt & (1<<4)) {   // ADMA Line int
        val0 = (1<<4);
        writel(val0, imap_nand.regs + NF2INTR);
	// set new adma address
        val0 = readl(imap_nand.regs + NF2ACADR);
	writel(val0, imap_nand.regs + NF2AADR);
	val0 = 1<<1;     // adma int ack.
        writel(val0, imap_nand.regs + NF2INTA);
        cInt &= ~(1<<4);
    }
    else if (cInt & (1<<7)) {   // ADMA valid error int
        val0 = (1<<7);
        writel(val0, imap_nand.regs + NF2INTR);
	// set new adma address
        val0 = readl(imap_nand.regs + NF2ACADR) + 8;
	writel(val0, imap_nand.regs + NF2AADR);	
	nf2_soft_reset(8);  // reset dma
        val0 = 1<<1;     // adma int ack.
        writel(val0, imap_nand.regs + NF2INTA);
	cInt        &= ~(1<<7);
    }
    else if (cInt & (1<<6)) {   // SDMA page boundary  int
        val0 = (1<<6);
        writel(val0, imap_nand.regs + NF2INTR);
	// set new sdma address
        val0 = 1<<2;     // sdma page boundary int ack.
        writel(val0, imap_nand.regs + NF2INTA);
	(*(volatile int *)(0x43ffF088)) = 0x38;	//step
        cInt        &= ~(1<<6);
    }
    else if (cInt & (1<<16)) {   // SDMA block int
        val0 = (1<<16);
        writel(val0, imap_nand.regs + NF2INTR);
	val0 = 1<<3;     // sdma block int ack.
        writel(val0, imap_nand.regs + NF2INTA);
	cInt        &= ~(1<<16);
    }
    
    return cInt;
}


int nf2_check_ecc(int page_num, int *fixed_bits)
{
	int nf2_ecc_info[10];
	int block_cnt = 0;
	int i = 0, j = 0, k = 0;
	int ret = 0;
	int fixed_bit = 0;

	nf2_ecc_info[0] = readl(imap_nand.regs + NF2ECCINFO0);
	nf2_ecc_info[1] = readl(imap_nand.regs + NF2ECCINFO1);
	nf2_ecc_info[2] = readl(imap_nand.regs + NF2ECCINFO2);
	nf2_ecc_info[3] = readl(imap_nand.regs + NF2ECCINFO3);
	nf2_ecc_info[4] = readl(imap_nand.regs + NF2ECCINFO4);
	nf2_ecc_info[5] = readl(imap_nand.regs + NF2ECCINFO5);
	nf2_ecc_info[6] = readl(imap_nand.regs + NF2ECCINFO6);
	nf2_ecc_info[7] = readl(imap_nand.regs + NF2ECCINFO7);
	nf2_ecc_info[8] = readl(imap_nand.regs + NF2ECCINFO8);

	*fixed_bits = 0;	
	block_cnt = page_num>>10;
	for(i=0; i<block_cnt; i++)
	{
		//TODO
		//nf_dbg("nf2_ecc_info[8] = 0x%x\n", nf2_ecc_info[8]);
		if(nf2_ecc_info[8] & (1<<i))
		{
			ret = -1;
			ecc_add_stat(0, -1);		// register mecc bits
			break;
		}
		else
		{
			j = i>>2;
			k = i%4;
			switch(k)
			{
			case 0:
				fixed_bit = nf2_ecc_info[j]& 0x7f;
				break;
			case 1:
				fixed_bit = (nf2_ecc_info[j]& 0x7f00) >> 8;
				break;
			case 2:
				fixed_bit = (nf2_ecc_info[j]& 0x7f0000) >> 16;
				break;
			case 3:
				fixed_bit = (nf2_ecc_info[j]& 0x7f000000) >> 24;
				break;	
			}
			ecc_add_stat(0, fixed_bit);		// register mecc bits
		}
		//choose the max fixed_bit	
		*fixed_bits = *fixed_bits > fixed_bit ? *fixed_bits : fixed_bit;	
	}
	
	return ret;
}

int nf2_check_secc(int *fixed_bits)
{
	int nf2_secc_info = 0;
	
	nf2_secc_info = readl(imap_nand.regs + NF2ECCINFO9);
	if(nf2_secc_info & 0x40) {
		ecc_add_stat(1, -1);		// register secc bits
		return -1;
	}
	else
	{
		if(nf2_secc_info & 0x80)
		{
			*fixed_bits = nf2_secc_info & 0xf;
		}	
		ecc_add_stat(1, *fixed_bits);		// register secc bits
	}	
	
	return 0;
}
		
int nf2_set_busw(int busw){

	if(busw == 16){
		writel(0x1, imap_nand.regs + NF2BSWMODE);
	}else{
		writel(0x0, imap_nand.regs + NF2BSWMODE);
	}	

	return 0;
}

unsigned int nf2_secc_cap (unsigned secc_type)
{
	unsigned int secc_cap = 0;
	
	switch(secc_type)
	{
	case 0: { secc_cap = 4;} break;
	case 1: { secc_cap = 8;} break;
	default: break;
	}
	
	return secc_cap;
}

static void ecc_add_stat(int type, int bits)
{
	int mecc_seg[9] = {2, 4, 8, 16, 20, 24, 32, 40, 127};
	int secc_seg[2] = {4, 8}, i;

	if(type) {
		/* secc */
		if(bits < 0) secc_stat[2]++;
		else if(bits == 0) secc_stat[3]++;
		else if(bits > 8)
		  printk("x800 SHOULD NOT fix so much secc bits: %d\n", bits);
		else {
			for (i = 0; i < 2; i++)
			  if(bits <= secc_seg[i]) break;
			secc_stat[i] ++;
		}
	} else {
		/* mecc */
		if(bits < 0) mecc_stat[9]++;
		else if(bits == 0) mecc_stat[10]++;
		else if(bits > 127)
		  printk("x800 SHOULD NOT fix so much mecc bits: %d\n", bits);
		else {
			for (i = 0; i < 9; i++)
			  if(bits <= mecc_seg[i]) break;
			mecc_stat[i] ++;
		}
	}
}

int nf2_active_async(struct mtd_info *mtd) {

	struct nand_config *nand_cfg = mtd->priv;
	int tmp = 0;
	int ret = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
	//printk("===========nf2_active_async=========\n");    

	nf2_soft_reset(1);

    	writel(0x0, imap_nand.regs + NF2PSYNC);    // set host to async mode
    	//step : program trx-afifo
    	writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo

    	trx_afifo_ncmd(0x0, 0x0, 0xef);    // set features CMD
    	trx_afifo_ncmd(0x0, 0x1, 0x01);    // set features ADDR 01H
    	//trx_afifo_ncmd(0x0, 0x2, 0x10 + nand_cfg->sync_mode);     // set features P1
    	trx_afifo_ncmd(0x0, 0x2, 0x00 + 0x5);     // set features P5
    	trx_afifo_ncmd(0x0, 0x2, 0x00);    // set features P2
    	trx_afifo_ncmd(0x0, 0x2, 0x00);    // set features P3
    	trx_afifo_ncmd(0x4, 0x2, 0x00);    // set features P4, check rnb status
    	trx_afifo_reg(0x2, 0xf, 0x0);      // cs invalid
    	//trx_afifo_nop(0x0, 0x100);         // nop
    	//trx_afifo_reg(0x3, 0x1, 0x0);      // set sync mode

    	writel(0x0, imap_nand.regs + NF2FFCE);     // disable write trx-afifo

    	//writel(0xe, imap_nand.regs + NF2CSR);	// cs0 valid
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
    	writel(0x1, imap_nand.regs + NF2STRR);	// start trx-afifo
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
    	writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status

	if(timeout != 0){
		ret  = -1;
    	    	printk("nf2_active_async timeout\n");
	}
	return ret;
}

int nf2_active_sync(struct mtd_info *mtd) {

	struct nand_config *nand_cfg = mtd->priv;
	int tmp = 0;
	int ret = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
    	
	nf2_soft_reset(1);

    	writel(0x0, imap_nand.regs + NF2PSYNC);    // set host to async mode
    	//step : program trx-afifo
    	writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo

    	trx_afifo_ncmd(0x0, 0x0, 0xef);    // set features CMD
    	trx_afifo_ncmd(0x0, 0x1, 0x01);    // set features ADDR 01H
    	trx_afifo_ncmd(0x0, 0x2, 0x10 + nand_cfg->sync_mode);     // set features P1
    	trx_afifo_ncmd(0x0, 0x2, 0x00);    // set features P2
    	trx_afifo_ncmd(0x0, 0x2, 0x00);    // set features P3
    	trx_afifo_ncmd(0x4, 0x2, 0x00);    // set features P4, check rnb status
    	trx_afifo_reg(0x2, 0xf, 0x0);      // cs invalid
    	trx_afifo_nop(0x0, 0x100);         // nop
    	trx_afifo_reg(0x3, 0x1, 0x0);      // set sync mode

    	writel(0x0, imap_nand.regs + NF2FFCE);     // disable write trx-afifo

    	//writel(0xe, imap_nand.regs + NF2CSR);	// cs0 valid
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
    	writel(0x1, imap_nand.regs + NF2STRR);	// start trx-afifo
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
    	writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status

	if(timeout != 0){
		ret  = -1;
    	    	printk("nf2_active_sync timeout\n");
	}
	return ret;
}

int nf2_asyn_reset(struct mtd_info *mtd)
{
	struct nand_config *nand_cfg = mtd->priv;
	int tmp = 0;
	int ret = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
     	
	//step 1 : Reset internal logic
     	nf2_soft_reset(1);

     	//step : program trx-afifo
     	writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo
     	trx_afifo_ncmd(0x4, 0x0, 0xff);    // reset CMD, wait for rnb ready
     	writel(0x0, imap_nand.regs + NF2FFCE);     // disable write trx-afifo

		//printk("chipnr = %d\n", nand_cfg->chipnr);
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
     	
     	writel(0x1, imap_nand.regs + NF2STRR); // start trx-afifo

        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret  = -1;
    	    	printk("nf2_asyn_reset timeout\n");
	}
	
	trx_afifo_intr_clear();	
     	
	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
 
     	return ret; 
}

/*
    @func   void | trx_afifo_enable | 
    @param	NULL
    @comm	allow write trx-afifo
    @xref
    @return NULL
*/
void trx_afifo_enable(void)
{
	volatile unsigned int tmp = 0;
	tmp = readl(imap_nand.regs + NF2FFCE);
	tmp |= 0x1;     // allow write trx-afifo
	writel(tmp, imap_nand.regs + NF2FFCE);
}

/*
    @func   void | trx_nand_cs | 
    @param	NULL
    @comm	nand cs select
    @xref
    @return 1: finish, 0: not finish
*/
void trx_nand_cs(struct mtd_info *mtd, int valid, int chipnr)
{
	unsigned int val = 0;
	struct nand_config *nand_cfg = mtd->priv;

	switch(nand_cfg->chipmap[chipnr]){
		case 0:
			if(valid == 1)
			{
				val = ~(1<<0);  // cs0 valid
				writel(val, imap_nand.regs + NF2CSR);
			}
			else
			{
				val = 0xf;
				writel(val, imap_nand.regs + NF2CSR);
			}	
			
			break;
		case 1:
			//printk("chip 1\n");
			if(valid == 1)
			{
				val = ~(1<<1);  // cs0 valid
				writel(val, imap_nand.regs + NF2CSR);
			}
			else
			{
				val = 0xf;
				writel(val, imap_nand.regs + NF2CSR);
			}	
			break;
		case 2:
			if(valid == 1)
			{
				imapx_pad_set_outdat(0, 1, cs2_index); //set output = 0
			}
			else
			{
				imapx_pad_set_outdat(1, 1, cs2_index); //set output = 1
			}
			break;
		case 3:
			if(valid == 1)
			{
				imapx_pad_set_outdat(0, 1, cs3_index); //set output = 0
			}
			else
			{
				imapx_pad_set_outdat(1, 1, cs3_index); //set output = 1
			}
			break;
	
		default:
			printk("invailed chipnr\n");
	}

}

/*
    @func   void | trx_afifo_disable | 
    @param	NULL
    @comm	disable write trx-afifo
    @xref
    @return NULL
*/
void trx_afifo_disable(void)
{
	writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo
}

/*
    @func   void | trx_afifo_start | 
    @param	NULL
    @comm	start trx-afifo
    @xref
    @return NULL
*/
void trx_afifo_start(void)
{
	writel(0x1, imap_nand.regs + NF2STRR); // start trx-afifo
}

/*
    @func   void | trx_afifo_is_finish | 
    @param	which your major data
    @comm	start trx-afifo
    @xref
    @return 1: finish, 0: not finish
*/
int trx_afifo_is_finish(int major)
{
	unsigned int val = 0;

	val = readl(imap_nand.regs + NF2INTR) & major;
	return val;    
}

/*
    @func   void | trx_afifo_intr_clear | 
    @param	NULL
    @comm	clear nand all interrupt
    @xref
    @return 1: finish, 0: not finish
*/
void trx_afifo_intr_clear(void)
{
	writel(0xffffffff, imap_nand.regs + NF2INTR); // clear status
}

/*
    @func   void | trx_afifo_read_state | 
    @param	index
    @comm	return NF2STAR0 or NF2STAR1
    @xref
    @return 1: finish, 0: not finish
*/
int trx_afifo_read_state(int index)
{
	switch(index)
	{	
	case 0:
		return readl(imap_nand.regs + NF2STSR0);
	case 1:
		return readl(imap_nand.regs + NF2STSR1);
	case 2:
		return readl(imap_nand.regs + NF2STSR2);
	case 3:
		return readl(imap_nand.regs + NF2STSR3);
	}
	return -1;	
}

/**
 * return 0 successful, return -1 failed
*/
int nf2_page_op(struct mtd_info *mtd, int ecc_enable, int page_mode, int ecc_type, int rsvd_ecc_en,
                    int page_num, int sysinfo_num, int trans_dir, int row_addr,
		    int ch0_adr, int ch0_len, int secc_used, int secc_type,
                    int half_page_en, int main_part, int cycle, int randomizer,
		    uint32_t *data_seed, uint16_t *sysinfo_seed,  uint16_t *ecc_seed, uint16_t *secc_seed,
		    uint16_t *data_last1K_seed, uint16_t *ecc_last1K_seed, int *ecc_unfixed, int *secc_unfixed,
		    int *corrected_fixed, int ch1_adr, int ch1_len, int sdma_2ch_mode, int busw)
{ 
	struct nand_config *nand_cfg = mtd->priv;
	int ecc_num = 0;
	int secc_num = 0;
	int spare_num = 0;
	int rsvd_num = 4;
	int ecc_offset = 0;
	int secc_offset = 0;
	int tot_ecc_num = 0;
	int tot_page_num = 0;
	int cur_ecc_offset = 0;
	int col_addr = 0;
	int eram_mode = 0;
	int fixed_bits = 0;
	int ecc_ret = 0;
	int ret = 0;
	int result = 0;
	int ecc_cap = 0;
	int secc_cap = 0;
	int dat = 0;
	int iostat = 0;
#if !defined(NAND_USE_IRQ)
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
#endif
	nand_debug_en = 0;
	irq_condition = 0;

	*ecc_unfixed = 0x0;
	*secc_unfixed = 0x0;
	*corrected_fixed = 0x0;

	ecc_cap = nf2_ecc_cap(ecc_type);
	secc_cap = nf2_secc_cap(secc_type);

	ret = nf2_soft_reset(1);
	if(ret != 0)
		return -1;
		
	spare_num = sysinfo_num + rsvd_num;

	if(randomizer == 1)
	{
		//TODO
		//nf_dbg("randomizer enable\n");
		writel(0x1, imap_nand.regs + NF2RANDME);	
	}
	else
	{
		//TODO
		//nf_dbg("randomizer disable\n");
		writel(0x0, imap_nand.regs + NF2RANDME);	
	}	

	if(page_mode == 0) //whole page op
	{
		col_addr = 0;
    	
		if(ecc_enable == 1)
		{
			if(randomizer == 1)
			{
				writel((((data_seed[1] & 0xffff)<<16) | (data_seed[0] & 0xffff)) ,(imap_nand.regs + NF2DATSEED0));
				writel((((data_seed[3] & 0xffff)<<16) | (data_seed[2] & 0xffff)) ,(imap_nand.regs + NF2DATSEED1));
				writel((((data_seed[5] & 0xffff)<<16) | (data_seed[4] & 0xffff)) ,(imap_nand.regs + NF2DATSEED2));
				writel((((data_seed[7] & 0xffff)<<16) | (data_seed[6] & 0xffff)) ,(imap_nand.regs + NF2DATSEED3));
				writel((ecc_seed[1]<<16 | ecc_seed[0]) ,imap_nand.regs + NF2ECCSEED0);
				writel((ecc_seed[3]<<16 | ecc_seed[2]) ,imap_nand.regs + NF2ECCSEED1);
				writel((ecc_seed[5]<<16 | ecc_seed[4]) ,imap_nand.regs + NF2ECCSEED2);
				writel((ecc_seed[7]<<16 | ecc_seed[6]) ,imap_nand.regs + NF2ECCSEED3);
			}	
			ecc_num = nf2_ecc_num (ecc_type, page_num, half_page_en);
			ecc_offset = page_num + spare_num;
    			ch0_len = page_num + spare_num;
			tot_page_num = ch0_len;
			cur_ecc_offset = ecc_offset;
			if(secc_used == 1)
			{
				if(secc_type == 0)
				{
					secc_num = 8;
				}
				else
				{
					secc_num = 12;
				}	
				tot_ecc_num = ecc_num + secc_num;
				secc_offset = ecc_offset + ecc_num;

			}
			else
			{
				tot_ecc_num = ecc_num;
				secc_num = 0;
				secc_offset = 0;
			}	
    		}
		else
		{
			if(randomizer == 1)
			{
				writel((((data_seed[1] & 0xffff)<<16) | (data_seed[0] & 0xffff)) ,(imap_nand.regs + NF2DATSEED0));
				writel((((data_seed[3] & 0xffff)<<16) | (data_seed[2] & 0xffff)) ,(imap_nand.regs + NF2DATSEED1));
				writel((((data_seed[5] & 0xffff)<<16) | (data_seed[4] & 0xffff)) ,(imap_nand.regs + NF2DATSEED2));
				writel((((data_seed[7] & 0xffff)<<16) | (data_seed[6] & 0xffff)) ,(imap_nand.regs + NF2DATSEED3));
			}	
			ecc_num = 0;
			ecc_offset = 0;
			secc_num = 0;
			secc_offset = 0;
			ch0_len = page_num + spare_num;
			tot_page_num = ch0_len;
			tot_ecc_num = 0;
			secc_used = 0;
			cur_ecc_offset = ecc_offset;
		}	
    	}		
	else // main page or spare op
	{
    
		if(main_part == 1)//main range
		{
			col_addr = 0;
			if(ecc_enable == 1)				
			{
				if(randomizer == 1)
				{
					writel((((data_seed[1] & 0xffff)<<16) | (data_seed[0] & 0xffff)) ,(imap_nand.regs + NF2DATSEED0));
					writel((((data_seed[3] & 0xffff)<<16) | (data_seed[2] & 0xffff)) ,(imap_nand.regs + NF2DATSEED1));
					writel((((data_seed[5] & 0xffff)<<16) | (data_seed[4] & 0xffff)) ,(imap_nand.regs + NF2DATSEED2));
					writel((((data_seed[7] & 0xffff)<<16) | (data_seed[6] & 0xffff)) ,(imap_nand.regs + NF2DATSEED3));
					writel((ecc_seed[1]<<16 | ecc_seed[0]) ,imap_nand.regs + NF2ECCSEED0);
					writel((ecc_seed[3]<<16 | ecc_seed[2]) ,imap_nand.regs + NF2ECCSEED1);
					writel((ecc_seed[5]<<16 | ecc_seed[4]) ,imap_nand.regs + NF2ECCSEED2);
					writel((ecc_seed[7]<<16 | ecc_seed[6]) ,imap_nand.regs + NF2ECCSEED3);
				}	
				ecc_num = nf2_ecc_num (ecc_type, page_num, half_page_en);
				ecc_offset = page_num + spare_num;
				//ch0_len  = page_num;
				tot_page_num = ch0_len;
				tot_ecc_num = ecc_num / (page_num/ch0_len);
				secc_used = 0;
				secc_num = 0;
				secc_offset = 0;
				cur_ecc_offset = ecc_offset;
				//nand_debug_en = 1;
				page_mode = 0;
				//printk("tot_page_num = %d, tot_ecc_num = %d, cur_ecc_offset = 0x%x\n", tot_page_num, tot_ecc_num, cur_ecc_offset);
			}
			else
			{
				if(randomizer == 1)
				{
					writel((((data_seed[1] & 0xffff)<<16) | (data_seed[0] & 0xffff)) ,(imap_nand.regs + NF2DATSEED0));
					writel((((data_seed[3] & 0xffff)<<16) | (data_seed[2] & 0xffff)) ,(imap_nand.regs + NF2DATSEED1));
					writel((((data_seed[5] & 0xffff)<<16) | (data_seed[4] & 0xffff)) ,(imap_nand.regs + NF2DATSEED2));
					writel((((data_seed[7] & 0xffff)<<16) | (data_seed[6] & 0xffff)) ,(imap_nand.regs + NF2DATSEED3));
				}	
				ecc_num = 0;
				ecc_offset = 0;
				ch0_len  = page_num;
				tot_page_num = ch0_len;
				tot_ecc_num = 0;
				secc_used = 0;
				secc_num = 0;
				secc_offset = 0;
				cur_ecc_offset = ecc_offset;		
			}	
		}
		else //spare range
		{
			if(ecc_enable == 1)
			{
				ecc_num = nf2_ecc_num (ecc_type, page_num, half_page_en);
				ecc_offset = page_num + spare_num;
				if(secc_used == 1)
				{
					if(randomizer == 1)
					{
						writel((sysinfo_seed[1]<<16 | sysinfo_seed[0]) ,imap_nand.regs + NF2DATSEED0);
						writel((sysinfo_seed[3]<<16 | sysinfo_seed[2]) ,imap_nand.regs + NF2DATSEED1);
						writel((sysinfo_seed[5]<<16 | sysinfo_seed[4]) ,imap_nand.regs + NF2DATSEED2);
						writel((sysinfo_seed[7]<<16 | sysinfo_seed[6]) ,imap_nand.regs + NF2DATSEED3);
						writel((secc_seed[1]<<16 | secc_seed[0]) ,imap_nand.regs + NF2ECCSEED0);
						writel((secc_seed[3]<<16 | secc_seed[2]) ,imap_nand.regs + NF2ECCSEED1);
						writel((secc_seed[5]<<16 | secc_seed[4]) ,imap_nand.regs + NF2ECCSEED2);
						writel((secc_seed[7]<<16 | secc_seed[6]) ,imap_nand.regs + NF2ECCSEED3);
					}	

					if(secc_type == 0)
					{
						secc_num = 8;
					}
					else
					{
						secc_num = 12;
					}	
					ch0_len  = spare_num;
					tot_page_num = ch0_len;
					tot_ecc_num = secc_num;
					secc_offset = ecc_offset + ecc_num;
					cur_ecc_offset = secc_offset;
					col_addr = page_num;
#if 0
					printk("ch0_len = 0x%x\n", ch0_len);
					printk("tot_page_num = 0x%x\n", tot_page_num);
					printk("tot_ecc_num = 0x%x\n", tot_ecc_num);
					printk("secc_offset = 0x%x\n", secc_offset);
					printk("cur_ecc_offset = 0x%x\n", cur_ecc_offset);
					printk("col_addr = 0x%x\n", page_num);
#endif
				}
				else //use last 1K ecc
				{
					if(randomizer == 1)
					{
						writel((data_last1K_seed[1]<<16 | data_last1K_seed[0]) ,imap_nand.regs + NF2DATSEED0);
						writel((data_last1K_seed[3]<<16 | data_last1K_seed[2]) ,imap_nand.regs + NF2DATSEED1);
						writel((data_last1K_seed[5]<<16 | data_last1K_seed[4]) ,imap_nand.regs + NF2DATSEED2);
						writel((data_last1K_seed[7]<<16 | data_last1K_seed[6]) ,imap_nand.regs + NF2DATSEED3);
						writel((ecc_last1K_seed[1]<<16 | ecc_last1K_seed[0]) ,imap_nand.regs + NF2ECCSEED0);
						writel((ecc_last1K_seed[3]<<16 | ecc_last1K_seed[2]) ,imap_nand.regs + NF2ECCSEED1);
						writel((ecc_last1K_seed[5]<<16 | ecc_last1K_seed[4]) ,imap_nand.regs + NF2ECCSEED2);
						writel((ecc_last1K_seed[7]<<16 | ecc_last1K_seed[6]) ,imap_nand.regs + NF2ECCSEED3);
					}	

					ch0_len = spare_num;
					tot_page_num = 1024 + spare_num;
					tot_ecc_num = ecc_num / (page_num>>10);
					cur_ecc_offset = ecc_offset + tot_ecc_num * ((page_num>>10) - 1);
					col_addr = page_num - 1024;	
				}	
			}
			else
			{
				if(randomizer == 1)
				{
					writel((sysinfo_seed[1]<<16 | sysinfo_seed[0]) ,imap_nand.regs + NF2DATSEED0);
					writel((sysinfo_seed[3]<<16 | sysinfo_seed[2]) ,imap_nand.regs + NF2DATSEED1);
					writel((sysinfo_seed[5]<<16 | sysinfo_seed[4]) ,imap_nand.regs + NF2DATSEED2);
					writel((sysinfo_seed[7]<<16 | sysinfo_seed[6]) ,imap_nand.regs + NF2DATSEED3);
				}	
				ch0_len = spare_num;
				tot_page_num = ch0_len;
				tot_ecc_num = 0;
				secc_used = 0;
				ecc_num = 0;
				ecc_offset = 0;
				secc_num = 0;
				secc_offset = 0;
				cur_ecc_offset = secc_offset;
				col_addr = page_num;
#if 0
				printk("ch0_len = 0x%x\n", ch0_len);
					printk("tot_page_num = 0x%x\n", tot_page_num);
					printk("tot_ecc_num = 0x%x\n", tot_ecc_num);
					printk("secc_offset = 0x%x\n", secc_offset);
					printk("cur_ecc_offset = 0x%x\n", cur_ecc_offset);
					printk("col_addr = 0x%x\n", page_num);
#endif
			}	

		}
    	}

	if(sdma_2ch_mode == 1)
	{
		ch0_len -= ch1_len;
	}	

	nf2_ecc_cfg(ecc_enable, page_mode, ecc_type, eram_mode, 
		 	tot_ecc_num, tot_page_num, trans_dir, half_page_en);
	//TODO
	if(nand_debug_en){
		printk("nf2_ecc_cfg %d, %d, %d, %d, %d, %d, %d, %d\n", ecc_enable, page_mode, ecc_type, eram_mode, tot_ecc_num, tot_page_num, trans_dir, half_page_en);
	}
	nf2_secc_cfg(secc_used, secc_type, rsvd_ecc_en);
	//TODO
	if(nand_debug_en){
		printk("nf2_secc_cfg %d, %d, %d\n", secc_used, secc_type, rsvd_ecc_en);
	}
   	nf2_sdma_cfg(ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);
	//TODO
	if(nand_debug_en){
		printk("nf2_sdma_cfg 0x%x, %d, 0x%x, %d, %d\n", ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);
	}
	nf2_addr_cfg(row_addr, col_addr, cur_ecc_offset, busw);
	//TODO
	if(nand_debug_en){
		printk("nf2_addr_cfg 0x%x, 0x%x, %d, %d\n", row_addr, col_addr, cur_ecc_offset, busw);
	}
#if 0
	if(trans_dir == 1){
	//write op
		g_debug_wbuf[20] = ecc_enable;
		g_debug_wbuf[21] = page_mode;
		g_debug_wbuf[22] = ecc_type;
		g_debug_wbuf[23] = tot_ecc_num;
		g_debug_wbuf[24] = tot_page_num;
		g_debug_wbuf[25] = trans_dir;
		g_debug_wbuf[26] = half_page_en;
		g_debug_wbuf[27] = secc_used;
		g_debug_wbuf[28] = secc_type;
		g_debug_wbuf[29] = rsvd_ecc_en;
		g_debug_wbuf[30] = ch0_adr;
		g_debug_wbuf[31] = ch0_len;
		g_debug_wbuf[32] = ch1_adr;
		g_debug_wbuf[33] = ch1_len;
		g_debug_wbuf[34] = sdma_2ch_mode;
		g_debug_wbuf[35] = row_addr;
		g_debug_wbuf[36] = col_addr;
		g_debug_wbuf[37] = cur_ecc_offset;
		g_debug_wbuf[38] = busw;

	}else{
	//read op
		if(ecc_enable == 1){
		g_debug_wbuf[50] = ecc_enable;
		g_debug_wbuf[51] = page_mode;
		g_debug_wbuf[52] = ecc_type;
		g_debug_wbuf[53] = tot_ecc_num;
		g_debug_wbuf[54] = tot_page_num;
		g_debug_wbuf[55] = trans_dir;
		g_debug_wbuf[56] = half_page_en;
		g_debug_wbuf[57] = secc_used;
		g_debug_wbuf[58] = secc_type;
		g_debug_wbuf[59] = rsvd_ecc_en;
		g_debug_wbuf[60] = ch0_adr;
		g_debug_wbuf[61] = ch0_len;
		g_debug_wbuf[62] = ch1_adr;
		g_debug_wbuf[63] = ch1_len;
		g_debug_wbuf[64] = sdma_2ch_mode;
		g_debug_wbuf[65] = row_addr;
		g_debug_wbuf[66] = col_addr;
		g_debug_wbuf[67] = cur_ecc_offset;
		g_debug_wbuf[68] = busw;
		g_debug_wbuf[69] = 0x0;
		g_debug_wbuf[70] = 0x0;
		}
	
	}
#endif	
	if(trans_dir == 0)//read op
	{
		g_nand_state = IMAPX800_NAND_READ;
#if defined(NAND_USE_IRQ)	
		writel(0x20, imap_nand.regs + NF2INTE); // enable sdma finished interrupt
#else
		writel(0x0, imap_nand.regs + NF2INTE); // disable sdma finished interrupt
#endif		
		writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		//trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
#if 0
		trx_afifo_ncmd(0x4, 0x0, 0x30);    // page read CMD1, check rnb
		trx_afifo_nop(0x5, 0x7);           // nop and then read whole page, 7nop > 20ns for tRR PARAM
#else
		trx_afifo_ncmd(0x0, 0x0, 0x30);
		trx_afifo_nop(0x7, 100);	   // 100 cycle nop for tWB
		trx_afifo_nop(0x5, 0x7);           // nop and then read whole page, 7nop > 20ns for tRR PARAM
#endif
		trx_afifo_reg(0x8, 0x1, 0x0);      // row_addr_0 add 1	
		writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo

		trx_nand_cs(mtd, 1, nand_cfg->chipnr);

    		if (ecc_enable) {
			writel(0x7, imap_nand.regs + NF2STRR); // start trx-afifo, ecc and dma
    		} else {
    		 	writel(0x5, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 
    		}


#if defined(NAND_USE_IRQ)
			//printk("wait event\n");
		ret = wait_event_timeout(imap_nand.wq,
				irq_condition,
				NAND_SDMA_TIMEOUT * HZ / 1000);
		if(!ret){
			result = -1;
		  __nand_msg("SDMA read timeout\n");
		}
		writel(0x0, imap_nand.regs + NF2INTE); // disable interrupt
#else
        	tend = cpu_clock(UINT_MAX) + 200000000;
        	while(1){
        	        tstart = cpu_clock(UINT_MAX);
		        if(tstart > tend){
        	                timeout = -1;
        	                break;
        	        } 
			if(nf2_check_irq() == 0x1<<5){
				timeout = 0;
				break;
			}
		}
			
		if(timeout != 0){
			result = -1;
    		    	printk("nf2_read timeout\n");
		}
		
		trx_afifo_intr_clear();
		//nf_dbg("nf2_read_op time = %d\n", time_tick_start);
#endif
		
		trx_nand_cs(mtd, 0, nand_cfg->chipnr);
		
		if(main_part == 1 || page_mode == 0){
			ecc_ret = nf2_check_ecc(page_num, &fixed_bits);
			if(ecc_ret != 0)
			{
				*ecc_unfixed = 0xff;
//				printk("ecc unfixed\n");
			}	
			else if(fixed_bits >= nand_cfg->max_correct_bits){
				printk("==fixed bit = %d\n", fixed_bits);
				*corrected_fixed = 1;
			}
			//g_debug_wbuf[69] = *ecc_unfixed;
		}
		if(secc_used == 1)
		{
			ecc_ret = nf2_check_secc(&fixed_bits);
			if(ecc_ret != 0)
			{
				*secc_unfixed = 0xff;
//				printk("secc unfixed 0x%x, 0x%x, 0x%x, 0x%x\n", readl(imap_nand.regs + NF2DEBUG8), readl(imap_nand.regs + NF2DEBUG9), readl(imap_nand.regs + NF2DEBUG10), readl(imap_nand.regs + NF2DEBUG11));
			}	
			else if(fixed_bits > (secc_cap >> 1)){
				*corrected_fixed = 1;
			}	
			//g_debug_wbuf[70] = *secc_unfixed;
		}
	}	
	else //write op
	{
		g_nand_state = IMAPX800_NAND_WRITE;
#if defined(NAND_USE_IRQ)
		writel(0x1001, imap_nand.regs + NF2INTE); // enable trx-empty interrupt.
#else
		writel(0x0, imap_nand.regs + NF2INTE); // disable trx-empty interrupt.
#endif		
		writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x80);    // page program CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		//trx_afifo_nop(0x3, 0x0);           // nop and then write whole page
		trx_afifo_nop(0x3, 70);           // nop and then write whole page, 70nop > 200ns for tADL PARAM
#if 0
		trx_afifo_ncmd(0x2, 0x0, 0x10);    // page program CMD1,wait for rnb ready and check status
#else
		trx_afifo_ncmd(0x0, 0x0, 0x10);		//send 10h cmd
		trx_afifo_nop(0x7, 100);		//100 nop cycle for tWB, then wait R/B high
		trx_afifo_nop(0x0, 0x300);  		//3ns * 0x300 = 2.4us
		trx_afifo_nop(0x0, 0x300);
		trx_afifo_nop(0x0, 0x300);
		trx_afifo_ncmd(0x6, 0x0, 0x70);		//send 70h cmd, then read to status reg
#endif
		trx_afifo_reg(0x8, 0x1, 0x0);      // row_addr_0 add 1
		writel(0x0, imap_nand.regs + NF2FFCE); //disable write trx-afifo

		trx_nand_cs(mtd, 1, nand_cfg->chipnr);

    		if (ecc_enable) {
			writel(0x7, imap_nand.regs + NF2STRR);//start trx-afifo, ecc and dma
    		} else {
			writel(0x5, imap_nand.regs + NF2STRR);//start trx-afifo,  and dma
    		}
  		
#if defined(NAND_USE_IRQ)
		ret = wait_event_timeout(imap_nand.wq,
				irq_condition,
				NAND_SDMA_TIMEOUT * HZ / 1000);

		if(!ret){
			result = -1;
		  __nand_msg("SDMA write timeout\n");
		}
		writel(0x0, imap_nand.regs + NF2INTE); // disable interrupt
#else
        	tend = cpu_clock(UINT_MAX) + 200000000;
        	while(1){
        	        tstart = cpu_clock(UINT_MAX);
		        if(tstart > tend){
        	                timeout = -1;
        	                break;
        	        } 
			if(nf2_check_irq() == 0x1){
				timeout = 0;
				break;
			}
		}
			
		if(timeout != 0){
    			result = -1;
			printk("nf2_write timeout\n");
		}
	
		
		trx_afifo_intr_clear();
		//TODO
		//nf_dbg("nf2_read_op time = %d\n", time_tick_start);
#endif
		
		trx_nand_cs(mtd, 0, nand_cfg->chipnr);

		iostat = trx_afifo_read_state(0) & 0xff;
		dat = iostat & 0xdf;
		if(dat != 0xC0){
			result = -1;
			printk("program debuginfo, 0x%x, 0x%x\n", row_addr, dat);
		}

	}	

	g_nand_state = IMAPX800_NAND_IDLE;
	
	return result;

}

int nf2_timing_init(int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw)
{
	unsigned int pclk_cfg = 0;
#if 0	
	/* init nf pads */ 
	module_enable(NF2_SYSM_ADDR);
	pads_chmod(PADSRANGE(0, 15), PADS_MODE_CTRL, 0);
	if(ecfg_check_flag(ECFG_ENABLE_PULL_NAND)) {
		pads_pull(PADSRANGE(0, 15), 1, 1);
		pads_pull(PADSRANGE(10, 11), 1, 0);		/* cle, ale */
		pads_pull(15, 1, 0);					/* dqs */
	}
	if(busw == 16){
		pads_chmod(PADSRANGE(18, 19), PADS_MODE_CTRL, 0);
		pads_chmod(PADSRANGE(25, 30), PADS_MODE_CTRL, 0);
		if(ecfg_check_flag(ECFG_ENABLE_PULL_NAND)){
	  		pads_pull(PADSRANGE(18, 19), 1, 1);
	  		pads_pull(PADSRANGE(25, 30), 1, 1);
		}
	}
#endif
	nf2_set_busw(busw);

	pclk_cfg = readl(imap_nand.regs + NF2PCLKM);
	pclk_cfg &= ~(0x3<<4);
	pclk_cfg |= (0x3<<4); //ecc_clk/4
	writel(pclk_cfg, imap_nand.regs + NF2PCLKM);	

	if(interface == 0)//async
	{
		writel(0x0, imap_nand.regs + NF2PSYNC);
		pclk_cfg = readl(imap_nand.regs + NF2PCLKM);
		pclk_cfg |= (0x1); //sync clock gate
		writel(pclk_cfg, imap_nand.regs + NF2PCLKM);	
	}
	else if(interface == 1)// onfi sync
	{
		writel(0x1, imap_nand.regs + NF2PSYNC);
		pclk_cfg = readl(imap_nand.regs + NF2PCLKM);
		pclk_cfg &= ~(0x1); //sync clock pass
		writel(pclk_cfg, imap_nand.regs + NF2PCLKM);	
	}
	else	//toggle
	{
		writel(0x2, imap_nand.regs + NF2PSYNC);
		pclk_cfg = readl(imap_nand.regs + NF2PCLKM);
		pclk_cfg &= ~(0x1); //sync clock pass
		writel(pclk_cfg, imap_nand.regs + NF2PCLKM);	
	}	
	
	writel((timing & 0x1fffff), imap_nand.regs + NF2AFTM);
	writel(((timing & 0xff000000) >> 24), imap_nand.regs + NF2SFTM);
	//writel(0xe001, imap_nand.regs + NF2STSC);	
	writel(0x0001, imap_nand.regs + NF2STSC);	
	//writel(0x4023, NF2PRDC);	
	writel(phyread, imap_nand.regs + NF2PRDC);	
	//writel(0xFFFA00, NF2TOUT);
	writel(rnbtimeout, imap_nand.regs + NF2TOUT);
	//writel(0x3818, NF2PDLY);
	writel(phydelay, imap_nand.regs + NF2PDLY);


	return 1;	
}

int imap_get_retry_param_20(struct mtd_info *mtd, unsigned int *buf, int len, int param){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int index = 0, curindex = 0, tmp = 0;

	int ecc_enable = 0;
	int page_mode = 0;
	int ecc_type = 0;
	int eram_mode = 0;
	int tot_ecc_num = 0;
	int tot_page_num = 0;
	int trans_dir = 0;
	int half_page_en = 0;
	int secc_used = 0;
	int secc_type = 0;
	int rsvd_ecc_en = 0;
	int ch0_adr = 0;
	int ch0_len = 0;
	int ch1_adr = 0;
	int ch1_len = 0;
	int sdma_2ch_mode = 0;
	int row_addr = 0;
	int col_addr = 0;
	int cur_ecc_offset = 0;
	int busw = 8;


	tot_page_num = len;
	if(!imap_nand_map_vm((uint32_t)buf, len, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("imap_get_retry_param_20 dma pagebuf map error\n");
		return -1;
	}
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;

	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;
	nf2_ecc_cfg(ecc_enable, page_mode, ecc_type, eram_mode, 
		 	tot_ecc_num, tot_page_num, trans_dir, half_page_en);
	//TODO
	//printk("nf2_ecc_cfg %d, %d, %d, %d, %d, %d, %d, %d\n", ecc_enable, page_mode, ecc_type, eram_mode, tot_ecc_num, tot_page_num, trans_dir, half_page_en);

	nf2_secc_cfg(secc_used, secc_type, rsvd_ecc_en);
	//TODO
	//printk("nf2_secc_cfg %d, %d, %d\n", secc_used, secc_type, rsvd_ecc_en);

   	nf2_sdma_cfg(ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);
	//TODO
	//printk("nf2_sdma_cfg 0x%x, %d, 0x%x, %d, %d\n", ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);

	nf2_addr_cfg(row_addr, col_addr, cur_ecc_offset, busw);
	//TODO

	trx_afifo_enable(); 
	
	trx_afifo_ncmd(0x4, 0x0, 0xff);    // set CMD ff, check rnb
	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set CMD 36
	if(param == 0x35){
		trx_afifo_ncmd(0x0, 0x1, 0xff);    // set ADDR ff
		trx_afifo_ncmd(0x0, 0x2, 0x40);    // write data 40
		trx_afifo_ncmd(0x0, 0x1, 0xcc);    // set ADDR cc
	}
	if(param == 0x36){
		trx_afifo_ncmd(0x0, 0x1, 0xae);    // set ADDR ae
		trx_afifo_ncmd(0x0, 0x2, 0x00);    // write data 00
		trx_afifo_ncmd(0x0, 0x1, 0xb0);    // set ADDR b0
	}
	trx_afifo_ncmd(0x0, 0x2, 0x4d);    // write data 4d
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set CMD 16
	trx_afifo_ncmd(0x0, 0x0, 0x17);    // set CMD 17
	trx_afifo_ncmd(0x0, 0x0, 0x04);    // set CMD 04
	trx_afifo_ncmd(0x0, 0x0, 0x19);    // set CMD 19
	trx_afifo_ncmd(0x0, 0x0, 0x00);    // set CMD 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x0, 0x1, 0x02);    // set ADDR 02
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // set ADDR 00
	trx_afifo_ncmd(0x1, 0x0, 0x30);    // set CMD 30, check rnb and read page data
	trx_afifo_ncmd(0x4, 0x0, 0xff);    // set CMD ff, check rnb
	trx_afifo_ncmd(0x4, 0x0, 0x38);    // set CMD 38, check rnb
		
	writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo

	trx_nand_cs(mtd, 1, nand_cfg->chipnr);

    	writel(0x5, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 
        
	tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<5);
    		if(tmp == (0x1<<5)){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand set OTP param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
	imap_nand_unmap_vm(DMA_FROM_DEVICE);

	return 0;
}	

int imap_program_otp(struct mtd_info *mtd, int page, uint8_t * buf, int len){

	struct nand_config *nand_cfg  = mtd->priv;
	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	int i = 0;
	int index = 0, curindex = 0;
	int ch0_adr = 0;
	int ch0_len = 0;
	
	if(!imap_nand_map_vm((uint32_t)buf, len, index, &curindex, DMA_TO_DEVICE)){
		//TODO:
		printk("imap_get_retry_param_20 dma pagebuf map error\n");
		return -1;
	}
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;

	for(i=0; i<4; i++){	
		
		nf2_soft_reset(1);

		val  = (3<<8) | (15<<4) | (0x1<<1) | 0x1;//127bit
		writel(val, imap_nand.regs + NF2ECCC);

		val  = (224<<16) | (len); //127bit
		writel(val, imap_nand.regs + NF2PGEC);

		writel((int)ch0_adr, imap_nand.regs + NF2SADR0);
		writel(len, imap_nand.regs + NF2SBLKS);

		val = (1024<<16) | 1;
		writel(val, imap_nand.regs + NF2SBLKN);	

		val = 1;
		writel(val, imap_nand.regs + NF2DMAC);

		writel(page, imap_nand.regs + NF2RADR0);

		val = 1024<<16; //ecc offset
		writel(val, imap_nand.regs + NF2CADR);

		writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo

		writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x80);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		trx_afifo_nop(0x3, 0x70);
		trx_afifo_ncmd(0x2, 0x0, 0x10);    // page read CMD1, check rnb and read whole page
		writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo
		trx_nand_cs(mtd, 1, nand_cfg->chipnr);

		//writel(0x5, NF2STRR); // start trx-afifo,  and dma 
		writel(0x7, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 

		while(1)
		{
			//TODO add time out check	    
			tmp = readl(imap_nand.regs + NF2INTR) & (0x1);
			if(tmp == (0x1))break;
		}

		trx_nand_cs(mtd, 0, nand_cfg->chipnr);
		writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status

		page += 1;
	}
	
	imap_nand_unmap_vm(DMA_TO_DEVICE);
	//buf[1024] = (readl(NF2STSR0) & 0xff00) >> 8;
	//buf[1025] = (readl(NF2STSR0) & 0xff);
	return ret;

}

int imap_otp_check(uint8_t * buf){
	
	int i = 0;
	int offset = g_otp_start;

check_otp:
	for(i=g_otp_start; i<(g_otp_start+64); i++){
		if(((buf[i] | buf[i+64])!= 0xff) || ((buf[i] & buf[i+64])!= 0x0)){
			printk("===buf[%d] = 0x%x, buf[%d+64] = 0x%x\n", i, buf[i], i, buf[i+64]);
			break;	
		}
	}
	if(i != (g_otp_start + 64)){
		//TODO reboot
		g_otp_start += 128;
		if(g_otp_start >= (896 + offset)){
			//TODO:
			printk("get nand param failed\n");
			return -1;
		}
		goto check_otp;
	}

	return 0;
}

int imap_otp_copy(uint8_t *buf){

	int i = 0, j = 0;
	unsigned char nand_param_copy[128];
	unsigned char *pbuf;

	pbuf = (unsigned char *)g_second_otp_buf;
	for(i=0; i<128; i++){
		nand_param_copy[i] = buf[i];
	}

	for(i=0,j=0; i<1024; i++,j++){
		if(j>=128){
			j=0;
		}
		pbuf[i] = nand_param_copy[j];
	}

	return 0;
}	

void get_eslc_param_20(struct mtd_info *mtd, uint8_t *buf, uint8_t *regbuf){

	struct nand_config *nand_cfg  = mtd->priv;
	int tmp = 0;
	int retry_param = 0;
	
	nf2_soft_reset(1);
	
	writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo
	
	trx_afifo_ncmd(0x0, 0x0, 0x37);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, regbuf[0]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[1]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[2]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, regbuf[3]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	
	writel(0x0, imap_nand.regs + NF2FFCE);     // disable write trx-afifo
	
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	writel(0x1, imap_nand.regs + NF2STRR);   // start trx-afifo
	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
    	writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status

	retry_param = trx_afifo_read_state(0);
	buf[0] = (retry_param >> 24) & 0xff;
	buf[1] = (retry_param >> 16) & 0xff;
	buf[2] = (retry_param >> 8) & 0xff;
	buf[3] = (retry_param >> 0) & 0xff;
	printk("get eslc param 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3]);	
}

void set_eslc_param_20(struct mtd_info *mtd, uint8_t *buf, int start, uint8_t *regbuf){

	struct nand_config *nand_cfg  = mtd->priv;
	int tmp = 0;
	
	nf2_soft_reset(1);
	
	writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo
	
	if(start == 1){
		trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
		trx_afifo_ncmd(0x0, 0x1, regbuf[0]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[0] + 0xa);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[1]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[1] + 0xa);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[2]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[2] + 0xa);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[3]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[3] + 0xa);    // data
		trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
	}else{
		trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
		trx_afifo_ncmd(0x0, 0x1, regbuf[0]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[0]);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[1]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[1]);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[2]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[2]);    // data
		trx_afifo_ncmd(0x0, 0x1, regbuf[3]);    // ADDRS
		trx_afifo_ncmd(0x0, 0x2, buf[3]);    // data
		trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
	
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // set param CMD0
		trx_afifo_ncmd(0x0, 0x1, 0x00);    // ADDRS
		trx_afifo_ncmd(0x0, 0x0, 0x30);    // set param CMD0
	
	}
	writel(0x0, imap_nand.regs + NF2FFCE);     // disable write trx-afifo
	
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	writel(0x1, imap_nand.regs + NF2STRR);   // start trx-afifo
	
    	while(1)
    	{
    		//TODO add time out check	    
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1)break;
    	}

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
    	writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status
}

int create_otp_table(struct mtd_info *mtd, int page){
	struct nand_config *nand_cfg  = mtd->priv;
	int cycle = 5;
	unsigned int status;
	unsigned int *pbuf;
	unsigned char *pbuf_8;
	unsigned char eslc_buf[4];
	unsigned int ret;

	pbuf = (unsigned int *)g_second_otp_buf;
	pbuf_8 = (unsigned char *)g_second_otp_buf;

	nand_cfg->chipnr = 1;
	g_otp_start = 2;
get_retry_table_again:	
	imap_get_retry_param_20(mtd, (unsigned int *)g_second_otp_buf, 1024, nand_cfg->nand_param2);
			
	//check otp head	
	if(pbuf_8[0] != 0x8 || pbuf_8[1] != 0x8){
		goto get_retry_table_again;
	}
	ret = imap_otp_check(g_second_otp_buf);
	if(ret != 0){
		goto get_retry_table_again;
	}

	imap_otp_copy((uint8_t *)(g_second_otp_buf) + g_otp_start);

	imapx800_nand_erase(mtd, page, cycle, &status);

	pbuf[255] = RETRY_TABLE_MAGIC;

	get_eslc_param_20(mtd, eslc_buf, eslc_reg_buf);
	set_eslc_param_20(mtd, eslc_buf, 1, eslc_reg_buf);
	imap_program_otp(mtd, page, (uint8_t *)g_second_otp_buf, 1024);
	set_eslc_param_20(mtd, eslc_buf, 0, eslc_reg_buf);
	rtc_reboot();
	printk("kernel rtc reboot ......\n");
	while(1);
	//TODO:
	//rtc reset
}

int get_second_otp_table(struct mtd_info *mtd, loff_t start){

	struct nand_config *nand_cfg  = mtd->priv;
	uint32_t *pbuf, *pbuf1, *pbuf2;
	int block;
	int skip;
	int page;
	int i;

	g_second_otp_buf = kmalloc(1024, GFP_KERNEL);

	g_retrylevel_chip1 = 0;

	block = nand_cfg->blocksize;
	nand_cfg->chipnr = 1;
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
	nand_cfg->chipnr = 1;
	imap_get_retry_param_20_from_page(mtd, (unsigned int *)g_second_otp_buf, 1024, page);
	
	pbuf = (uint32_t *)g_second_otp_buf;

	if(pbuf[255] != RETRY_TABLE_MAGIC){
		printk("second magic not match %x != %x\n", pbuf[255], RETRY_TABLE_MAGIC);
		create_otp_table(mtd, page);
	}

	pbuf1 = (unsigned int *)&g_otp_buf[1024];
	pbuf2 = (unsigned int *)g_second_otp_buf;
	for(i=0; i<16; i++){
		pbuf1[i] = pbuf2[i];
	}

	printk("the second retry table\n");
	printk("default param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[1024], g_otp_buf[1025],g_otp_buf[1026],g_otp_buf[1027], g_otp_buf[1028], g_otp_buf[1029],g_otp_buf[1030],g_otp_buf[1031]);
	printk("1st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[8 + 1024], g_otp_buf[9 + 1024],g_otp_buf[10 + 1024],g_otp_buf[11 + 1024], g_otp_buf[12 + 1024], g_otp_buf[13 + 1024],g_otp_buf[14 + 1024],g_otp_buf[15 + 1024]);
	printk("2st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[16 + 1024], g_otp_buf[17 + 1024],g_otp_buf[18 + 1024],g_otp_buf[19 + 1024], g_otp_buf[20 + 1024], g_otp_buf[21 + 1024],g_otp_buf[22 + 1024],g_otp_buf[23 + 1024]);
	printk("3st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[24 + 1024], g_otp_buf[25 + 1024],g_otp_buf[26 + 1024],g_otp_buf[27 + 1024], g_otp_buf[28 + 1024], g_otp_buf[29 + 1024],g_otp_buf[30 + 1024],g_otp_buf[31 + 1024]);
	printk("4st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[32 + 1024], g_otp_buf[33 + 1024],g_otp_buf[34 + 1024],g_otp_buf[35 + 1024], g_otp_buf[36 + 1024], g_otp_buf[37 + 1024],g_otp_buf[38 + 1024],g_otp_buf[39 + 1024]);
	printk("5st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[40 + 1024], g_otp_buf[41 + 1024],g_otp_buf[42 + 1024],g_otp_buf[43 + 1024], g_otp_buf[44 + 1024], g_otp_buf[45 + 1024],g_otp_buf[46 + 1024],g_otp_buf[47 + 1024]);
	printk("6st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[48 + 1024], g_otp_buf[49 + 1024],g_otp_buf[50 + 1024],g_otp_buf[51 + 1024], g_otp_buf[52 + 1024], g_otp_buf[53 + 1024],g_otp_buf[54 + 1024],g_otp_buf[55 + 1024]);
	printk("7st     param 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_otp_buf[56 + 1024], g_otp_buf[57 + 1024],g_otp_buf[58 + 1024],g_otp_buf[59 + 1024], g_otp_buf[60 + 1024], g_otp_buf[61 + 1024],g_otp_buf[62 + 1024],g_otp_buf[63 + 1024]);

	kfree(g_second_otp_buf);
	return 0;
}

int imapx_read_spare(struct mtd_info *mtd, int page, int pagesize){

	struct nand_config *nand_cfg  = mtd->priv;
	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	unsigned long long tstart, tend;		/* 20ms timeout */
	uint8_t buf[20];
	int curindex = 0;
	int index = 0;
	int ch0_adr;
	int ch0_len;
	int timeout = -1;
	unsigned char *offsets;

	offsets = kmalloc(nand_cfg->sysinfosize + 4, GFP_KERNEL);
	if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sysinfosize + 4, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("imapx_read_spare dma pagebuf map error\n");
		return -1;
	}
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	
	nf2_soft_reset(1);

	val  = (1<<11) | (3<<8);
    	writel(val, imap_nand.regs + NF2ECCC);
    
	val  = (20); 
	writel(val, imap_nand.regs + NF2PGEC);
	
	writel((int)ch0_adr, imap_nand.regs + NF2SADR0);
	writel(20, imap_nand.regs + NF2SBLKS);
   
	val = (1024<<16) | 1;
       	writel(val, imap_nand.regs + NF2SBLKN);	
    
	val = 1;
	writel(val, imap_nand.regs + NF2DMAC);
	
	writel(page, imap_nand.regs + NF2RADR0);

	val = pagesize; //ecc offset
	writel(val, imap_nand.regs + NF2CADR);

	writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
	trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
	trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
	trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
	writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo
	//writel(0xe, NF2CSR); // cs0 valid
	
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);

    	writel(0x5, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 
    	//writel(0x7, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 

	tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<5);
    		if(tmp == (0x1<<5)){
			timeout = 0;
			break;
		}
	}
	if(timeout != 0){
		ret = -1;
    	    	printk("imapx_read_spare timeout\n");
	}
	
	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	//writel(0xf, NF2CSR);    // cs0 invalid
    	writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status
	imap_nand_unmap_vm(DMA_FROM_DEVICE);

	//spl_printf("0x%x, 0x%x, 0x%x, 0x%x\n", buf[0],buf[1],buf[2],buf[3]);	
	//buf[1024] = (readl(NF2STSR0) & 0xff00) >> 8;
	//buf[1025] = (readl(NF2STSR0) & 0xff);
	printk("offsets[0] = 0x%x\n", offsets[0]);
	if(offsets[0] != 0xff){
		ret = 1;
	}else{
		ret = 0;
	}

	kfree(offsets);
	return ret;
}

int isbad(struct mtd_info *mtd, loff_t start){

	struct nand_config *nc  = mtd->priv;
	int ret = 0;
	int page = 0;

	page = (start >> 13) + nc->badpagemajor;
	printk("check bad 0x%x\n", page);
	ret = imapx_read_spare(mtd, page, nc->pagesize);
	if(ret != 0){
		return ret;
	}
	
	page = (start >> 13) + nc->badpageminor;
	printk("check bad 0x%x\n", page);
	ret = imapx_read_spare(mtd, page, nc->pagesize);
		
	return ret;
}

#if 0
int imap_get_retry_param_20_from_page(struct mtd_info *mtd, unsigned int *buf, int len, int page){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int index = 0, curindex = 0, tmp = 0;

	int ecc_enable = 1;
	int page_mode = 0;
	int ecc_type = 15;
	int eram_mode = 0;
	int tot_ecc_num = 224;
	int tot_page_num = 1024;
	int trans_dir = 0;
	int half_page_en = 0;
	int secc_used = 0;
	int secc_type = 0;
	int rsvd_ecc_en = 0;
	int ch0_adr = 0;
	int ch0_len = 0;
	int ch1_adr = 0;
	int ch1_len = 0;
	int sdma_2ch_mode = 0;
	int row_addr = 0;
	int col_addr = 0;
	int cur_ecc_offset = 1024;
	int busw = 8;
	int cycle = 5;
	int ecc_info = 0;
	int i = 0;

	row_addr = page;

	for(i=0; i<4; i++){
		tot_page_num = len;
		if(!imap_nand_map_vm((uint32_t)buf, len, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("imap_get_retry_param_20 dma pagebuf map error\n");
			return -1;
		}
		ch0_adr = imap_nand_dma_list[0].dma_addr;
		ch0_len = imap_nand_dma_list[0].len;

		ret = nf2_soft_reset(1);
		if(ret != 0)
			return -1;
		nf2_ecc_cfg(ecc_enable, page_mode, ecc_type, eram_mode, 
				tot_ecc_num, tot_page_num, trans_dir, half_page_en);
		//TODO
		//printk("nf2_ecc_cfg %d, %d, %d, %d, %d, %d, %d, %d\n", ecc_enable, page_mode, ecc_type, eram_mode, tot_ecc_num, tot_page_num, trans_dir, half_page_en);

		nf2_secc_cfg(secc_used, secc_type, rsvd_ecc_en);
		//TODO
		//printk("nf2_secc_cfg %d, %d, %d\n", secc_used, secc_type, rsvd_ecc_en);

		nf2_sdma_cfg(ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);
		//TODO
		//printk("nf2_sdma_cfg 0x%x, %d, 0x%x, %d, %d\n", ch0_adr, ch0_len, ch1_adr, ch1_len, sdma_2ch_mode);

		nf2_addr_cfg(row_addr, col_addr, cur_ecc_offset, busw);
		//TODO

		trx_afifo_enable(); 

		writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		//trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
		trx_afifo_ncmd(0x4, 0x0, 0x30);    // page read CMD1, check rnb
		trx_afifo_nop(0x5, 0x7);           // nop and then read whole page, 7nop > 20ns for tRR PARAM
		trx_afifo_reg(0x8, 0x1, 0x0);      // row_addr_0 add 1	
		writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo

		trx_nand_cs(mtd, 1, nand_cfg->chipnr);

		writel(0x5, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 

		tend = cpu_clock(UINT_MAX) + 200000000;
		while(1){
			tstart = cpu_clock(UINT_MAX);
			if(tstart > tend){
				timeout = -1;
				break;
			} 
			tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<5);
			if(tmp == (0x1<<5)){
				timeout = 0;
				break;
			}
		}

		if(timeout != 0){
			ret = -1;
			printk("nand set OTP param timeout\n");
		}

		ecc_info = readl(NF2ECCINFO8);
		trx_afifo_intr_clear();

		trx_nand_cs(mtd, 0, nand_cfg->chipnr);

		imap_nand_unmap_vm(DMA_FROM_DEVICE);

		row_addr += 1;
		if(ecc_info & 0x1){
			printk("kernel: read otp table from page ecc failed, try next page 0x%x\n", row_addr);
		}else{
			break;
		}
	}
	return 0;
}	

#else
int imap_get_retry_param_20_from_page(struct mtd_info *mtd, unsigned int *buf, int len, int page){

	struct nand_config *nand_cfg  = mtd->priv;
	int val = 0;
	int ret = 0;
	int tmp = 0;
	int cycle = 5;
	int ecc_info = 0;
	int i = 0;
	int index = 0, curindex = 0;
	int ch0_adr = 0;
	int ch0_len = 0;

	for(i=0; i<4; i++){
		if(!imap_nand_map_vm((uint32_t)buf, len, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("imap_get_retry_param_20 dma pagebuf map error\n");
			return -1;
		}
		ch0_adr = imap_nand_dma_list[0].dma_addr;
		ch0_len = imap_nand_dma_list[0].len;
		
		nf2_soft_reset(1);

		val  = (3<<8) | (15<<4) | 0x1;//127bit
		writel(val, imap_nand.regs + NF2ECCC);

		val  = (224<<16) | (len); //127bit
		writel(val, imap_nand.regs + NF2PGEC);

		writel((int)ch0_adr, imap_nand.regs + NF2SADR0);
		writel(ch0_len, imap_nand.regs + NF2SBLKS);

		val = (1024<<16) | 1;
		writel(val, imap_nand.regs + NF2SBLKN);	

		val = 1;
		writel(val, imap_nand.regs + NF2DMAC);

		writel(page, imap_nand.regs + NF2RADR0);

		val = 1024<<16; //ecc offset
		writel(val, imap_nand.regs + NF2CADR);


		writel(0x1, imap_nand.regs + NF2FFCE);     // allow write trx-afifo

		writel(0x1, imap_nand.regs + NF2FFCE); // allow write trx-afifo
		trx_afifo_ncmd(0x0, 0x0, 0x00);    // page read CMD0
		trx_afifo_ncmd((0x8 | (cycle -1)), 0x1, 0x0);    // 5 cycle addr of row_addr_0 & col_addr
		trx_afifo_ncmd(0x1, 0x0, 0x30);    // page read CMD1, check rnb and read whole page
		writel(0x0, imap_nand.regs + NF2FFCE); // disable write trx-afifo

		trx_nand_cs(mtd, 1, nand_cfg->chipnr);
		//writel(0x5, NF2STRR); // start trx-afifo,  and dma 
		writel(0x7, imap_nand.regs + NF2STRR); // start trx-afifo,  and dma 

		while(1)
		{
			//TODO add time out check	    
			tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<5);
			if(tmp == (0x1<<5))break;
		}
		ecc_info = readl(imap_nand.regs + NF2ECCINFO8);

		trx_nand_cs(mtd, 1, nand_cfg->chipnr);
		writel(0xffffff, imap_nand.regs + NF2INTR);	// clear status

		imap_nand_unmap_vm(DMA_FROM_DEVICE);
		page += 1;
		if(ecc_info & 0x1){
			printk("read otp table from page ecc failed, try next page 0x%x\n", page);
		}else{
			break;
		}
	}

	return ret;
}
#endif

#if 0
int imap_set_retry_param_default_20(){
	
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */

	printk("imap_set_retry_param_default\n");

	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	trx_afifo_enable(); 

	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, 0xcc);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[0]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xbf);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[1]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xaa);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[2]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xab);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[3]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xcd);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[4]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[5]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[6]);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, g_otp_buf[7]);    // data
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1	
	
	trx_afifo_disable();
	trx_nand_cs(1, 0);
	
	trx_afifo_start();
	
	tend = cpu_clock(UINT_MAX) + 200000000;
	
	while(1){
		tstart = cpu_clock(UINT_MAX);
		if(tstart > tend){
			timeout = -1;
			break;
		} 
		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
	
	if(timeout != 0){
		ret = -1;
    	    	printk("nand set OTP param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(0, 0);

	return 0;

}
#endif
int imap_set_retry_param_micro_20(struct mtd_info *mtd, int retrylevel){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */
	
	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	trx_afifo_enable(); 

	trx_afifo_ncmd(0x0, 0x0, 0xef);    // set EFH CMD0
	trx_afifo_ncmd(0x0, 0x1, 0x89);    // ADDRS
	trx_afifo_nop(0x0, 0x100);// for tadl delay
	trx_afifo_ncmd(0x0, 0x2, retrylevel);    // P1
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P2
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P3
	trx_afifo_ncmd(0x0, 0x2, 0x0);    // P4
	trx_afifo_nop(0x7, 100);// for twb delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	trx_afifo_nop(0x0, 0x300);// 2.4us delay
	
	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	
	trx_afifo_start();
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand set OTP param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	return 0;
}

int imap_set_retry_param_20(struct mtd_info *mtd, int retrylevel, unsigned char * otp_table, unsigned char * reg_buf){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */
	
	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	trx_afifo_enable(); 
	
	
	trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, reg_buf[0]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[0 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[1]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[1 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[2]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[2 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[3]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[3 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[4]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[4 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[5]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[5 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[6]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[6 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x1, reg_buf[7]);    // ADDRS
	trx_afifo_ncmd(0x0, 0x2, otp_table[7 + retrylevel * 8]);    // data
	trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1	
	
	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	
	trx_afifo_start();
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand set OTP param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	return 0;
}	

int imap_get_retry_param_21(struct mtd_info *mtd, unsigned char *buf)
{
	/*set retry param by willie*/
	buf[0] = 0xa1;  //CMD: set register data 
	buf[1] = 0x00;
	buf[2] = 0xa7;  //read level for P0     reg or data?
	buf[3] = 0xa4;  //read level for P1
	buf[4] = 0xa5;  //read level for P2
	buf[5] = 0xa6;  //read level for P3
	/*retry table*/
	buf[6] = 0x00;  //default 0, retry end need do default
	buf[7] = 0x00;
	buf[8] = 0x00;
	buf[9] = 0x00;

	buf[10] = 0x05; //retry table 1, all 14 times;
	buf[11] = 0x0a;
	buf[12] = 0x00;
	buf[13] = 0x00;

	buf[14] = 0x28; //retry table 2;
	buf[15] = 0x00;
	buf[16] = 0xec;
	buf[17] = 0xd8;

	buf[18] = 0xed; //retry table 3;
	buf[19] = 0xf5;
	buf[20] = 0xed;
	buf[21] = 0xe6;

	buf[22] = 0x0a; //retry table 4;
	buf[23] = 0x0f;
	buf[24] = 0x05;
	buf[25] = 0x00;

	buf[26] = 0x0f; //retry table 5;
	buf[27] = 0x0a;
	buf[28] = 0xfb;
	buf[29] = 0xec;

	buf[30] = 0xe8; //retry table 6;
	buf[31] = 0xef;
	buf[32] = 0xe8;
	buf[33] = 0xdc;

	buf[34] = 0xf1; //retry table 7;
	buf[35] = 0xfb;
	buf[36] = 0xfe;
	buf[37] = 0xf0;

	buf[38] = 0x0a; //retry table 8;
	buf[39] = 0x00;
	buf[40] = 0xfb;
	buf[41] = 0xec;

	buf[42] = 0xd0; //retry table 9;
	buf[43] = 0xe2;
	buf[44] = 0xd0;
	buf[45] = 0xc2;

	buf[46] = 0x14; //retry table 10;
	buf[47] = 0x0f;
	buf[48] = 0xfb;
	buf[49] = 0xec;

	buf[50] = 0xe8; //retry table 11;
	buf[51] = 0xfb;
	buf[52] = 0xe8;
	buf[53] = 0xdc;

	buf[54] = 0x1e; //retry table 12;
	buf[55] = 0x14;
	buf[56] = 0xfb;
	buf[57] = 0xec;

	buf[58] = 0xfb; //retry table 13;
	buf[59] = 0xff;
	buf[60] = 0xfb;
	buf[61] = 0xf8;

	buf[62] = 0x07; //retry table 14;
	buf[63] = 0x0c;
	buf[64] = 0x02;
	buf[65] = 0x00;

	return 0;
}

int imap_get_retry_param_26(struct mtd_info *mtd, unsigned char *buf){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */
	unsigned int param = 0;

	printk("imap_get_retry_param_26\n");

	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	trx_afifo_enable(); 

	trx_afifo_ncmd(0x0, 0x0, 0x37);    // set param CMD0
	trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
	trx_afifo_ncmd(0x0, 0x3, 0x00);    // data
	
	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	
	trx_afifo_start();
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand get param timeout 26\n");
	}

	trx_afifo_intr_clear();
	trx_nand_cs(mtd, 0, nand_cfg->chipnr);	
	param = trx_afifo_read_state(0);
	buf[0] = (param >> 24) & 0xff;
	buf[1] = (param >> 16) & 0xff;
	buf[2] = (param >> 8) & 0xff;
	buf[3] = (param >> 0) & 0xff;

	printk("0xa7 = 0x%x, 0xad = 0x%x, 0xae = 0x%x, 0xaf = 0x%x\n", buf[0], buf[1], buf[2], buf[3]);	
	
	return 0;
}

int imap_set_retry_param_26(struct mtd_info *mtd, int retrylevel, unsigned char *buf){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */

	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	//printk("a7 = 0x%x, ad = 0x%x, ae = 0x%x, af = 0x%x\n", buf[0], buf[1], buf[2], buf[3]);
	trx_afifo_enable(); 
	switch(retrylevel){
		case 0:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[0]);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[1]);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2]);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3]);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 1:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[0]);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[1]+0x6);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2]+0xa);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3]+0x6);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 2:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[1] - 0x3);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2] - 0x7);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3] - 0x8);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 3:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[1] - 0x6);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2] - 0xd);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3] - 0xf);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 4:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[1] - 0x9);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2] - 0x14);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3] - 0x17);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 5:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2] - 0x1a);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3] - 0x1e);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		case 6:
			trx_afifo_ncmd(0x0, 0x0, 0x36);    // set param CMD0
			trx_afifo_ncmd(0x0, 0x1, 0xa7);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xad);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, 0x00);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xae);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[2] - 0x20);    // data
			trx_afifo_ncmd(0x0, 0x1, 0xaf);    // ADDRS
			trx_afifo_ncmd(0x0, 0x2, buf[3] - 0x25);    // data
			trx_afifo_ncmd(0x0, 0x0, 0x16);    // set param CMD1
			break;
		default:
			printk("set retry level is not allow\n");
			break;
	}	

	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	
	trx_afifo_start();
	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand set param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
	
	
	
	return 0;
}

unsigned char buf_crd_param19 [63] = {
	0x00, 0x00, 0x00,
	0xf0, 0xf0, 0xf0,
	0xef, 0xe0, 0xe0,
	0xdf, 0xd0, 0xd0,
	0x1e, 0xe0, 0x10,
	0x2e, 0xd0, 0x20,
	0x3d, 0xf0, 0x30,
	0xcd, 0xe0, 0xd0,
	0x0d, 0xd0, 0x10,
	0x01, 0x10, 0x20,
	0x12, 0x20, 0x20,
	0xb2, 0x10, 0xd0,
	0xa3, 0x20, 0xd0,
	0x9f, 0x00, 0xd0,
	0xbe, 0xf0, 0xc0,
	0xad, 0xc0, 0xc0,
	0x9f, 0xf0, 0xc0,
	0x01, 0x00, 0x00,
	0x02, 0x00, 0x00,
	0x0d, 0xb0, 0x00,
	0x0c, 0xa0, 0x00,
};

int init_retry_param_sandisk19(struct mtd_info *mtd)
{
	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	int i = 0;
	unsigned long long tstart, tend;                /* 20ms timeout */

	ret = nf2_soft_reset(1);
	if(ret != 0)
		return -1;

	 trx_afifo_enable();
	
	trx_afifo_ncmd(0x0, 0x0, 0xB6);
	trx_afifo_ncmd(0x0, 0x0, 0x3b);
	trx_afifo_ncmd(0x0, 0x0, 0xb9);

	for(i=4; i<13; i++){
		trx_afifo_ncmd(0x0, 0x0, 0x53);
		trx_afifo_ncmd(0x0, 0x1, i);    // set ADDR
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x64 = 300ns
		trx_afifo_ncmd(0x0, 0x2, 0x0);
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x23 = 105ns
	}

	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	trx_afifo_start();

	tend = cpu_clock(UINT_MAX) + 200000000;
	while(1){
		tstart = cpu_clock(UINT_MAX);
		if(tstart > tend){
			timeout = -1;
			break;
		}
		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
		if(tmp == 1){
			timeout = 0;
			break;
		}
	}

	if(timeout != 0){
		ret = -1;
		printk("nand set param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	return 0;
}

int imap_set_retry_param_sandisk19(struct mtd_info *mtd, int retrylevel)
{

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	int i = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */

	//printk("sandisk get retry level %d\n", retrylevel);
	/*sandisk special need: enable*/
	if(retrylevel == 1)
	{
		init_retry_param_sandisk19(mtd);
	}

	ret = nf2_soft_reset(1);
	if(ret != 0)
		return -1;
	
	trx_afifo_enable();

	trx_afifo_ncmd(0x0, 0x0, 0x3b);
	trx_afifo_ncmd(0x0, 0x0, 0xb9);
	for(i=0; i<3; i++){
		trx_afifo_ncmd(0x0, 0x0, 0x53);
		if(i == 2)
			trx_afifo_ncmd(0x0, 0x1, 0x04 + i + 1);    // set ADDR ff
		else
			trx_afifo_ncmd(0x0, 0x1, 0x04 + i);    // set ADDR ff
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x64 = 300ns
		trx_afifo_ncmd(0x0, 0x2, buf_crd_param19[retrylevel * 3 + i]);    // set data
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x64 = 300ns
	}
	/*sandisk special need: disable*/
	if(retrylevel == 0)
		trx_afifo_ncmd(0x0, 0x0, 0xD6);

	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);

	trx_afifo_start();

	tend = cpu_clock(UINT_MAX) + 200000000;
	while(1){
		tstart = cpu_clock(UINT_MAX);
		if(tstart > tend){
			timeout = -1;
			break;
		}
		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
		if(tmp == 1){
			timeout = 0;
			break;
		}
	}

	if(timeout != 0){
		ret = -1;
		printk("nand set param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	return 0;

}

int imap_set_retry_param_21(struct mtd_info *mtd, int retrylevel, unsigned char *buf){

	struct nand_config *nand_cfg  = mtd->priv;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	int i = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */

	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	nand_debug("retrylevel %d \n", retrylevel);
	nand_debug("retry tab 0x%x, 0x%x 0x%x, 0x%x\n",
			buf[(retrylevel + 1)*4 + 2], buf[(retrylevel + 1)*4 + 3], buf[(retrylevel + 1)*4 + 4], buf[(retrylevel + 1)*4 + 5]);
	trx_afifo_enable();

	for(i=0; i<4; i++)
	{
		trx_afifo_nop(0x0, 0x64);       //3ns * 0x64 = 300ns
		trx_afifo_ncmd(0x0, 0x0, buf[0]);
		trx_afifo_ncmd(0x0, 0x2, buf[1]);
		trx_afifo_ncmd(0x0, 0x2, buf[i + 2]);
		trx_afifo_ncmd(0x0, 0x2, buf[(retrylevel + 1)*4 + i + 2]);
	}

	trx_afifo_disable();
	trx_nand_cs(mtd, 1, nand_cfg->chipnr);

	trx_afifo_start();

	tend = cpu_clock(UINT_MAX) + 200000000;
	while(1){
		tstart = cpu_clock(UINT_MAX);
		if(tstart > tend){
			timeout = -1;
			break;
		} 
		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
		if(tmp == 1){
			timeout = 0;
			break;
		}
	}

	if(timeout != 0){
		ret = -1;
		printk("nand set param timeout\n");
	}
	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);



	return 0;
}

int imap_set_retry_param_default(){

	if(!imap_nand_inited)
		return 0;

	nand_get_device(imap_nand.mtd, FL_READING);
	
	if(nf_cfg.nand_param0 == 0x55){
		if(nf_cfg.nand_param1 == 26){
			printk("nf_cfg.nand_param1 = 26, g_retrylevel = %d\n", g_retrylevel);	
			while(1){
				if(g_retrylevel == 0)
					break;
				g_retrylevel++;
				if(g_retrylevel > (nf_cfg.retry_level))
					g_retrylevel=0;
				printk("set retrylevel %d\n", g_retrylevel);
				imap_set_retry_param_26(imap_nand.mtd, g_retrylevel, g_otp_buf);	

			}

			writel(g_otp_buf[0], sys_rtc_regs + 0x30);
			writel(g_otp_buf[1], sys_rtc_regs + 0x34);
			writel(g_otp_buf[2], sys_rtc_regs + 0x38);
			writel(g_otp_buf[3], sys_rtc_regs + 0x40);
			writel(0x71, sys_rtc_regs + 0x2c);
		}

		if(nf_cfg.nand_param1 == 20){
			nf_cfg.chipnr = 0;	
			imap_set_retry_param_20(imap_nand.mtd, 0, g_otp_buf, retry_reg_buf);
			if(nf_cfg.chipnum == 2){
				nf_cfg.chipnr = 1;
				imap_set_retry_param_20(imap_nand.mtd, 0, &g_otp_buf[1024], retry_reg_buf);
			}
		}
	}
	if(nf_cfg.nand_param0 == 0x75){
		if(nf_cfg.nand_param1 == 20){
			imap_set_retry_param_micro_20(imap_nand.mtd, 0);
		}
	}
	nand_release_device(imap_nand.mtd);
	return 0;
}

int nand_readid(struct mtd_info *mtd, uint8_t buf[], int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw)
{
	/* Read CDBG_NAND_ID_COUNT IDs into buffer
	 * If any error happened, return -1;
	 */

	struct nand_config *nand_cfg  = mtd->priv;
	int read_id = 0;
	int ret = 0;
	int tmp = 0;
	int timeout = -1;
	unsigned long long tstart, tend;		/* 20ms timeout */

	/* interface is set to 0 when reading ID, warit oct31,2011 */
	interface = 0;

	//TODO:
	//module_reset(NF2_SYSM_ADDR);
	nf2_timing_init(interface, timing, rnbtimeout, phyread, phydelay, busw);
	ret = nf2_asyn_reset(mtd);
	if(ret != 0)
		return -1;
	ret = nf2_soft_reset(1);
	if(ret != 0)
		return -1;
	
	trx_afifo_enable();
	trx_afifo_ncmd(0x0, 0x0, 0x90);    // read id CMD
	trx_afifo_ncmd(0x0, 0x1, 0x00);    // ADDRS
	trx_afifo_nop(0, 8);			   // NOP
	trx_afifo_ncmd(0xf, 0x3, 0x00);    // 8 read data    
	trx_afifo_disable();

	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	
	trx_afifo_start();

        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & 0x1;
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
		ret = -1;
    	    	printk("nand_readid timeout\n");
	}

	trx_afifo_intr_clear();

	trx_nand_cs(mtd, 0, nand_cfg->chipnr);

	read_id = trx_afifo_read_state(0);
	printk("read_id = 0x%x\n", read_id);
	buf[7] = read_id & 0xff;
	buf[6] = (read_id & 0xff00) >> 8;
	buf[5] = (read_id & 0xff0000) >> 16;
	buf[4] = (read_id & 0xff000000) >> 24;

	read_id = trx_afifo_read_state(1);
	printk("read_id = 0x%x\n", read_id);
	buf[3] = read_id & 0xff;
	buf[2] = (read_id & 0xff00) >> 8;
	buf[1] = (read_id & 0xff0000) >> 16;
	buf[0] = (read_id & 0xff000000) >> 24;

#if 0
	printk("ID: %02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
#endif
	return ret;
}

int imapx800_nand_erase(struct mtd_info *mtd, int page, int cycle, unsigned int *status)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret = 0;
	int i = 0;
	int tmp = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
	int iostat;
	
	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	//printk("erase 0x%x\n", page);
	writel(page, imap_nand.regs + NF2RADR0); // row addr 0
 
	//step : program trx-afifo
    
	trx_afifo_enable(); 
	trx_afifo_ncmd(0x0, 0x0, 0x60);    // ERASE CMD0
#if 0
	trx_afifo_ncmd((0x8 | (cycle - 3)), 0x1, 0x04);    // 3 cycle addr of row_addr_0
#else
	for(i=0; i<(cycle-2); i++){
		trx_afifo_ncmd(0x0, 0x1, ((page & (0xff << (i * 8))) >> i * 8));    // write addr
		
	}
#endif

#if 0	
	trx_afifo_ncmd(0x2, 0x0, 0xD0);    // erase CMD1,wait for rnb ready and check status
#else
	trx_afifo_ncmd(0x0, 0x0, 0xD0);
	trx_afifo_nop(0x7, 100);	//100 nop cycle for tWB, then wait R/B high
	trx_afifo_nop(0x0, 0x300);  	//3ns * 0x300 = 2.4us
	trx_afifo_nop(0x0, 0x300);  	//3ns * 0x300 = 2.4us
	trx_afifo_nop(0x0, 0x300);  	//3ns * 0x300 = 2.4us
	trx_afifo_nop(0x0, 0x300);  	//3ns * 0x300 = 2.4us
	trx_afifo_nop(0x0, 0x300);
	trx_afifo_nop(0x0, 0x300);
	trx_afifo_ncmd(0x6, 0x0, 0x70); //send 70h cmd, then read to status reg
#endif
	trx_afifo_disable();

    	trx_nand_cs(mtd, 1, nand_cfg->chipnr);  // cs0 valid
    	
	trx_afifo_start(); // start trx-afifo

        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<0);
    		if(tmp == 0x1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
    	    	ret = -1;
		printk("nand_erase timeout\n");
	}
    
    	trx_afifo_intr_clear();
    	
	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
	
	iostat = trx_afifo_read_state(0) & 0xff;
	*status = iostat & 0xdf;

	if(*status != 0xC0){
		printk("erase debuginfo 0x%x, 0x%x\n", page, iostat);
		ret = -1;
	}

	return ret;
}

int imapx800_nand_read_status(struct mtd_info *mtd, unsigned int *status){

	struct nand_config *nand_cfg  = mtd->priv;
	uint32_t ret = 0;
	int tmp = 0;
	unsigned long long tstart, tend;		/* 20ms timeout */
	int timeout = -1;
	
	ret = nf2_soft_reset(1);
    	if(ret != 0)
		return -1;

	trx_afifo_enable(); 
	trx_afifo_ncmd(0x0, 0x0, 0x70); //read status cmd
	trx_afifo_nop(0x0, 0xa); //nop cnt = 10
	trx_afifo_ncmd(0x0, 0x3, 0x0); //read to status register
	trx_afifo_disable();

	trx_nand_cs(mtd, 1, nand_cfg->chipnr);
	trx_afifo_start();	
        tend = cpu_clock(UINT_MAX) + 200000000;
        while(1){
                tstart = cpu_clock(UINT_MAX);
	        if(tstart > tend){
                        timeout = -1;
                        break;
                } 
    		tmp = readl(imap_nand.regs + NF2INTR) & (0x1<<0);
    		if(tmp == 1){
			timeout = 0;
			break;
		}
	}
		
	if(timeout != 0){
    	    	ret = -1;
		printk("nand_read_status timeout\n");
	}
	
	trx_afifo_intr_clear();
	trx_nand_cs(mtd, 0, nand_cfg->chipnr);
	*status = trx_afifo_read_state(0) & 0xff;
//	printk("status = 0x%x\n", *status);
	return ret;
}

int imapx800_nand_get_badmark(struct mtd_info *mtd, int page)
{
	struct nand_config *nand_cfg = mtd->priv;

	int page_mode = 1; // not whole page
	int main_part = 0; // spare part
	int trans_dir = 0; //read op
	int ecc_en = 0;
	int secc_used = 0;
	int secc_type = 0;
	int rsvd_ecc_en = 1;
	int half_page_en = 0;
	int ret = 0;
	int result = 0;
	int ecc_unfixed = 0;
	int secc_unfixed = 0;
	int ch0_adr = 0;
	int ch0_len = 0;
	int ch1_adr = 0;
	int ch1_len = 0;
	int sdma_2ch_mode = 0;
	int corrected_fixed = 0;
	int randomizer = 0;
	int index, curindex;
	unsigned char *offsets;

	//printk("check bad block 0x%x\n", page);

	offsets = kmalloc(nand_cfg->sysinfosize + 4, GFP_KERNEL);
	if(offsets == 0){
		printk("offset kmalloc error\n");
		return -1;
	}

	index = 0;
	if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sysinfosize + 4, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("get badmake dma pagebuf map error\n");
		result = -1;
		goto __exit_err__;
	}
	
	if(imap_nand_dma_list[0].len != (nand_cfg->sysinfosize + 4)){
		printk("get badmake dma pagebuf len error\n");	
		result = -1;
		goto __exit_err__;
	}
	
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	ecc_en = 0;
	secc_used = 0;
	ret = nf2_page_op(mtd, ecc_en, page_mode, nand_cfg->mecclvl, rsvd_ecc_en,
				nand_cfg->pagesize, nand_cfg->sysinfosize, trans_dir, page,
				ch0_adr, ch0_len, secc_used, nand_cfg->secclvl,
				half_page_en, main_part, nand_cfg->cycle, randomizer,
				&(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
				&(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &ecc_unfixed, &secc_unfixed,
				&corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, nand_cfg->busw);
	
	imap_nand_unmap_vm(DMA_FROM_DEVICE);
	
	//printk("ch0_adr = 0x%x, ch0_len = %d, offsets = 0x%x\n", ch0_adr, ch0_len, offsets);
	//printk("page = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", page, offsets[0], offsets[1], offsets[2], offsets[3]);
	if(ret){
		printk("nand read spare IO error\n");
		result = -1;
		goto __exit_err__;
	}

	//printk("offsets[0] = 0x%x, page 0x%x.\n", offsets[0], page);
	if(offsets[0] == 0xff){
		result = 0;	
	}else{
		result = 1;
	}

	if(nand_cfg->eccen || nand_cfg->randomizer){
	index = 0;
	if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sysinfosize + 4, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("get badmake dma pagebuf map error\n");
		result = -1;
		goto __exit_err__;
	}
	if(imap_nand_dma_list[0].len != (nand_cfg->sysinfosize + 4)){
		printk("get badmake dma pagebuf len error\n");	
		result = -1;
		goto __exit_err__;
	}
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	ecc_en = nand_cfg->eccen;
	randomizer = nand_cfg->randomizer;
	sdma_2ch_mode = 0;

#if defined(CONFIG_MTD_NAND_IMAPX800_SECC)
	secc_used = nand_cfg->eccen;
	secc_type = nand_cfg->secclvl;
#else		
	secc_used = 0;
	secc_type = 0;
#endif
	//printk("ecc = %d, secc = %d, randomizer = %d\n", ecc_en, secc_used, randomizer);
	//printk("offsets = 0x%x, ch0_adr = 0x%x, ch0_len = 0x%x\n", offsets, ch0_adr, ch0_len);
	ret = nf2_page_op(mtd, ecc_en, page_mode, nand_cfg->mecclvl, rsvd_ecc_en,
				nand_cfg->pagesize, nand_cfg->sysinfosize, trans_dir, page,
				ch0_adr, ch0_len, secc_used, secc_type,
				half_page_en, main_part, nand_cfg->cycle, randomizer,
				&(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
				&(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &ecc_unfixed, &secc_unfixed,
				&corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, nand_cfg->busw);
	
	imap_nand_unmap_vm(DMA_FROM_DEVICE);

	//printk("secc_unfixed = 0x%x, page 0x%x.\n", secc_unfixed, page);
	if(!secc_unfixed){
		if(offsets[0] == 0xff){
			result = 0;
		}else{
			result = 1;
		}
	}
	else if(result == 1){
//		printk("a bad block found page = 0x%x\n", page);
	}
	}

__exit_err__:
	kfree(offsets);
	return result;
}

int imapx800_nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{

	struct nand_config *nand_cfg = mtd->priv;
	int page;
	int meccstat, seccstat, ret;
	int corrected_fixed;
	int secc_used;
	int secc_type;
	unsigned char *offsets;
	int chipnr, chip_page, die_page;

	if(!nand_cfg)
	  return -1;

#if defined(CONFIG_MTD_NAND_IMAPX800_SECC)
		secc_used = nand_cfg->eccen;
		secc_type = nand_cfg->secclvl;
		offsets = kmalloc(4, GFP_KERNEL);
		memset(offsets, 0x0, 4);
#else	
		offsets = kmalloc(1024 + 4, GFP_KERNEL);
		memset(offsets, 0x0, 1024 + 4);
		secc_used = 0;
		secc_type = 0;
#endif
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
	ret = nf2_page_op(mtd, nand_cfg->eccen, 1, nand_cfg->mecclvl, 1,			/* rsvd ecc on */
			nand_cfg->pagesize, nand_cfg->sysinfosize, 1, die_page,					/* write */
			0, nand_cfg->pagesize, secc_used, secc_type,
			0, 0, nand_cfg->cycle, nand_cfg->randomizer,
			&(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			&(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &meccstat, &seccstat,
			&corrected_fixed, (uint32_t)offsets, 4, 1, nand_cfg->busw);

	kfree(offsets);
	return ret;
}
 
/***********************************************************************
 * -Function:
 *    imap_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
 *                    uint8_t *buf)
 *
 * -Description:
 *    Read all data in current page in to buf, and all data in current
 *    oob in to chip->oop_poi.
 *    Data in main area of page is protected by MECC in oob area.
 *    MECC(if this is a SLC flash) and the first seccsteps * seccsize
 *    bytes in oob is protected by SECC.
 *    The rest of OOB and SECC has no protection.
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer to store read data
 *                
 * -Return
 *    0
 *
 * -Others
 *    Science we now need to read the ECC codes out of OOB before any
 *    page data, so none of the default read_page method(HW, SW, SYND)
 *    can be used. This special odd function is applied here.
 ***********************************************************************
 */
int imap_nand_read_oob(struct mtd_info *mtd, uint8_t *oobbuf, int page, int oobbytes)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	int result = 0;
	uint32_t ecc_unfixed = 0, secc_unfixed = 0;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, cycle, randomizer, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int index = 0, curindex = 0;
	unsigned char *offsets;
	int i = 0;

	offsets = kmalloc(nand_cfg->sparesize, GFP_KERNEL);
	if(offsets == 0){
		printk("offset kmalloc error\n");
		return -1;
	}

	//printk("oob read 0x%x\n", page);
#if 0
	if((uint32_t)oobbuf & 0x3)
		printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)oobbuf);
	else if((uint32_t)oobbuf & 0x1f)
		printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)oobbuf);
#endif
	index = 0;
	if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sparesize, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("dma oobbuf map error\n");
		result = -1;
		goto __read_oob_exit__;
	}
			
	ecc_enable = 0;
	page_mode = 1;// not whole page
	ecc_type = 0;
	rsvd_ecc_en = 1;
	page_num = nand_cfg->pagesize;
	//sysinfo_num = nand_cfg->sysinfosize;
	sysinfo_num = nand_cfg->sparesize - 4;
	trans_dir = 0;// read op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	secc_used = 0;
	secc_type = 0;
	half_page_en = 0;
	main_part = 0; //spare part
	cycle = nand_cfg->cycle;
	randomizer = 0;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;
	
	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        	            page_num, sysinfo_num, trans_dir, row_addr,
			    ch0_adr, ch0_len, secc_used, secc_type,
        	            half_page_en, main_part, cycle, randomizer,
			    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
			    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_FROM_DEVICE);
	if(ret != 0){
		//TODO
		result = -1;
		goto __read_oob_exit__;
	}	
	
	//printk("len = %d\n", ch0_len);
	for(i=0; i<ch0_len; i++){
		//printk("0x%x ", offsets[i]);
		if(offsets[i] != 0xff){
	//		printk("oob read offset[%d] = 0x%x\n", i, offsets[i]);
			goto __read_oob_again__;	
		}
	}

	//printk("emtpy page find 0x%x\n", row_addr);
	memset((void *)oobbuf, 0xff, oobbytes);
	result = 0;
	goto __read_oob_exit__;
#if 0
	printk("imap_nand_read_oob ecc = 0\n");
		printk("ecc = 0, ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
		printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
		printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
		printk("oobbuf = 0x%x\n", oobbuf);
		printk("oobbuf[0] = 0x%x, oobbuf[1] = 0x%x, oobbuf[2] = 0x%x, oobbuf[3] = 0x%x\n", oobbuf[0], oobbuf[1], oobbuf[2], oobbuf[3]);
#endif

__read_oob_again__:		
	if(nand_cfg->eccen || nand_cfg->randomizer){
		index = 0;
		if(!imap_nand_map_vm((uint32_t)offsets, oobbytes, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("dma oobbuf map error\n");
			result = -1;
			goto __read_oob_exit__;
		}
		ecc_enable = nand_cfg->eccen;
		ecc_type = nand_cfg->mecclvl;
		sysinfo_num = nand_cfg->sysinfosize;

#if defined(CONFIG_MTD_NAND_IMAPX800_SECC)
		secc_used = nand_cfg->eccen;
		secc_type = nand_cfg->secclvl;
#else		
		secc_used = 0;
		secc_type = 0;
#endif
		randomizer = nand_cfg->randomizer;
		ch0_adr = imap_nand_dma_list[0].dma_addr;
		ch0_len = imap_nand_dma_list[0].len;
		ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        		            page_num, sysinfo_num, trans_dir, row_addr,
				    ch0_adr, ch0_len, secc_used, secc_type,
        		            half_page_en, main_part, cycle, randomizer,
				    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
				    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
				    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

		imap_nand_unmap_vm(DMA_FROM_DEVICE);
#if 0
		printk("imap_nand_read_oob ecc = 1\n");
		printk("ecc = 1, ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
		printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
		printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
		printk("oobbuf = 0x%x\n", oobbuf);
		printk("oobbuf[0] = 0x%x, oobbuf[1] = 0x%x, oobbuf[2] = 0x%x, oobbuf[3] = 0x%x\n", oobbuf[0], oobbuf[1], oobbuf[2], oobbuf[3]);
#endif
		if(ret != 0){
			//TODO
			result = -1;
			goto __read_oob_exit__;
		}		
		
		if(secc_unfixed == 0){
			//printk("oob secc_unfixed = 0, 0x%x\n", row_addr);	
			memcpy((void *)oobbuf, offsets, oobbytes);
		}else{
			printk("===========oob read secc unfixed 0x%x============\n", row_addr);
			mtd->ecc_stats.failed++;
		}	
		if(corrected_fixed){
			printk("read oob corrected_fixed = %d\n", corrected_fixed);
//			mtd->ecc_stats.corrected++;
		}
	}

__read_oob_exit__:	
	//printk("read oob ok\n");
	kfree(offsets);
	return result;
}


int imap_nand_read_page_random(struct mtd_info *mtd, uint8_t *pagebuf, int page, int bytes){

	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	uint32_t ecc_unfixed = 0, secc_unfixed = 0;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, cycle, randomizer, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int index = 0, curindex = 0, tmp = 0;
	unsigned char *offsets;
	unsigned int *offsets_u32;
	int i = 0;

	ecc_unfixed = 0;
	secc_unfixed = 0;
	corrected_fixed = 0;
	index = 0;
	if(!imap_nand_map_vm((uint32_t)pagebuf, bytes, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("dma pagebuf map error\n");
		return -1;
	}
	if(curindex >= index + 1){
		printk("waring , not support 2 part not contion memory\n");
	}
	
	if(imap_nand_dma_list[0].len != bytes){
		printk("dma pagebuf map len error\n");
		return -1;	
	}
			
	ecc_enable = nand_cfg->eccen;
	page_mode = 1;// part page
	ecc_type = nand_cfg->mecclvl;
	rsvd_ecc_en = 1;
	page_num = nand_cfg->pagesize;
	sysinfo_num = nand_cfg->sysinfosize;
	trans_dir = 0;// read op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	secc_used = nand_cfg->eccen;
	secc_type = nand_cfg->secclvl;
	half_page_en = 0;
	main_part = 1; //page part
	cycle = nand_cfg->cycle;
	randomizer = nand_cfg->randomizer;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;
	secc_used = 0;
	secc_type = 0;

	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        	            page_num, sysinfo_num, trans_dir, row_addr,
			    ch0_adr, ch0_len, secc_used, secc_type,
        	            half_page_en, main_part, cycle, randomizer,
			    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
			    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_FROM_DEVICE);
	
	if(ret != 0){
		//TODO
		printk("imap_nand_read_page_random failed\n");
		return -1;
	}		
	
	if(ecc_unfixed != 0){
		offsets = kmalloc(nand_cfg->sparesize, GFP_KERNEL);
		if(offsets == 0){
			printk("offset kmalloc error\n");
			return -1;
		}
		offsets_u32 = (unsigned int *)offsets;

		memset(offsets, 0xff, nand_cfg->sparesize);
		index = 0;
		if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sparesize, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("get badmake dma pagebuf map error\n");
			return -1;
		}
		
		if(imap_nand_dma_list[0].len != (nand_cfg->sparesize)){
			printk("get badmake dma pagebuf len error\n");	
			return -1;
		}
		
		ch0_adr = imap_nand_dma_list[0].dma_addr;
		ch0_len = imap_nand_dma_list[0].len;
		page_mode = 1; // not whole page
		main_part = 0; // spare part
		trans_dir = 0; //read op
		ecc_enable = 0;
		secc_used = 0;
		sdma_2ch_mode = 0;
		randomizer = 0;
		ret = nf2_page_op(mtd, ecc_enable, page_mode, nand_cfg->mecclvl, rsvd_ecc_en,
					nand_cfg->pagesize, (nand_cfg->sparesize - 4), trans_dir, row_addr,
					ch0_adr, ch0_len, secc_used, nand_cfg->secclvl,
					half_page_en, main_part, nand_cfg->cycle, randomizer,
					&(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
					&(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &tmp, &tmp,
					&corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, nand_cfg->busw);
		
		imap_nand_unmap_vm(DMA_FROM_DEVICE);
		
		//printk("ch0_adr = 0x%x, ch0_len = %d, offsets = 0x%x\n", ch0_adr, ch0_len, offsets);
		//printk("page = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", page, offsets[0], offsets[1], offsets[2], offsets[3]);
		if(ret){
			printk("nand read spare IO error\n");
			kfree(offsets);
			return -1;
		}

		for(i=0; i<ch0_len; i++){
			if(offsets[i] != 0xff){
				mtd->ecc_stats.failed++;
				//printk("random read ECC unfixed: page=0x%x (%d)\n", page, !!ecc_unfixed);
				goto imap_nand_read_page_random_end;
			}
		}
			
		if(nand_cfg->randomizer == 1){
			memset(pagebuf, 0xff, bytes);
		}
imap_nand_read_page_random_end:	
	kfree(offsets);
	}

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
 *                    uint8_t *buf)
 *
 * -Description:
 *    Read all data in current page in to buf, and all data in current
 *    oob in to chip->oop_poi.
 *    Data in main area of page is protected by MECC in oob area.
 *    MECC(if this is a SLC flash) and the first seccsteps * seccsize
 *    bytes in oob is protected by SECC.
 *    The rest of OOB and SECC has no protection.
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer to store read data
 *                
 * -Return
 *    0
 *
 * -Others
 *    Science we now need to read the ECC codes out of OOB before any
 *    page data, so none of the default read_page method(HW, SW, SYND)
 *    can be used. This special odd function is applied here.
 ***********************************************************************
 */
int imap_nand_read_page(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes)
{

	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	uint32_t ecc_unfixed = 0, secc_unfixed = 0;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, cycle, randomizer, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int index = 0, curindex = 0, tmp = 0;
	unsigned char *offsets;
	unsigned int *offsets_u32;
	int i = 0;
#if 0
	if((uint32_t)pagebuf & 0x3)
		//printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)pagebuf);
	else if((uint32_t)pagebuf & 0x1f)
		//printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)pagebuf);
#endif
	//printk("r p = 0x%x\n", page);

	ecc_unfixed = 0;
	secc_unfixed = 0;
	corrected_fixed = 0;
	index = 0;
	if(!imap_nand_map_vm((uint32_t)pagebuf, bytes, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("dma pagebuf map error\n");
		return -1;
	}
	if(curindex >= index + 1){
		printk("waring , not support 2 part not contion memory\n");
	}
	
	if(imap_nand_dma_list[0].len != mtd->writesize){
		printk("dma pagebuf map len error\n");
		return -1;	
	}
			
	ecc_enable = nand_cfg->eccen;
	page_mode = 0;// whole page
	ecc_type = nand_cfg->mecclvl;
	rsvd_ecc_en = 1;
	page_num = nand_cfg->pagesize;
	sysinfo_num = nand_cfg->sysinfosize;
	trans_dir = 0;// read op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	secc_used = nand_cfg->eccen;
	secc_type = nand_cfg->secclvl;
	half_page_en = 0;
	main_part = 0;
	cycle = nand_cfg->cycle;
	randomizer = nand_cfg->randomizer;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;

	if(oobbuf == NULL){
//		printk("nand read page oobbuf = NULL\n");
	}

	//if(oobbuf != NULL){
#if 0		
		if((uint32_t)oobbuf & 0x3)
			//printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)oobbuf);
		else if((uint32_t)oobbuf & 0x1f)
			//printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)oobbuf);
#endif
	memset(oobbuf, 0xff, 20);
		index = curindex + 1;		
		if(!imap_nand_map_vm((uint32_t)oobbuf, oobbytes, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("dma oobbuf map error\n");
			return -1;
		}
		if(curindex >= index + 1){
			printk("waring , not support 2 part not contion memory\n");
		}
		if(imap_nand_dma_list[1].len != oobbytes){
			printk("dma oobbuf map len error\n");
			return -1;	
		}
		ch1_adr = imap_nand_dma_list[1].dma_addr;
		ch1_len = imap_nand_dma_list[1].len;
		sdma_2ch_mode = 1;
	//}

#if defined(CONFIG_MTD_NAND_IMAPX800_SECC)
	secc_used = nand_cfg->eccen;
	secc_type = nand_cfg->secclvl;
#else
	secc_used = 0;
	secc_type = 0;
#endif	
	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        	            page_num, sysinfo_num, trans_dir, row_addr,
			    ch0_adr, ch0_len, secc_used, secc_type,
        	            half_page_en, main_part, cycle, randomizer,
			    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
			    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_FROM_DEVICE);
#if 0
	printk("imap_nand_read_page\n");
	printk("ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
	printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
	printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
	printk("pagebuf = 0x%x, oobbuf = 0x%x\n", pagebuf, oobbuf);
	printk("pagebuf[0] = 0x%x, pagebuf[1] = 0x%x, pagebuf[2] = 0x%x, pagebuf[3] = 0x%x\n", pagebuf[0], pagebuf[1], pagebuf[2], pagebuf[3]);
#endif
	if(oobbuf != NULL){
		//printk("oobbuf[0] = 0x%x, oobbuf[1] = 0x%x, oobbuf[2] = 0x%x, oobbuf[3] = 0x%x\n", oobbuf[0], oobbuf[1], oobbuf[2], oobbuf[3]);
	}
	if(ret != 0){
		//TODO
		return -1;
	}		
	
	if(corrected_fixed){
		//printk("corrected_fixed = %d\n", corrected_fixed);
		// FIXME: do not tell this info
		mtd->ecc_stats.corrected++;
	}

	//if(ecc_unfixed != 0 || secc_unfixed != 0){
	if(ecc_unfixed != 0){
#if 0
	    if((secc_unfixed == 0xff && ecc_unfixed == 0x0) || (secc_unfixed == 0x0 && ecc_unfixed == 0xff))
			printk("ecc_unfixed = 0x%x, secc_unfixed = 0x%x, page = 0x%x\n", ecc_unfixed, secc_unfixed, row_addr);	
#endif		
		//offsets = kmalloc(nand_cfg->sysinfosize + 4, GFP_KERNEL);
		offsets = kmalloc(nand_cfg->sparesize, GFP_KERNEL);
		if(offsets == 0){
			printk("offset kmalloc error\n");
			return -1;
		}
		offsets_u32 = (unsigned int *)offsets;

		memset(offsets, 0xff, nand_cfg->sparesize);
		index = 0;
		if(!imap_nand_map_vm((uint32_t)offsets, nand_cfg->sparesize, index, &curindex, DMA_FROM_DEVICE)){
			//TODO:
			printk("get badmake dma pagebuf map error\n");
			return -1;
		}
		
		if(imap_nand_dma_list[0].len != (nand_cfg->sparesize)){
			printk("get badmake dma pagebuf len error\n");	
			return -1;
		}
		
		ch0_adr = imap_nand_dma_list[0].dma_addr;
		ch0_len = imap_nand_dma_list[0].len;
		page_mode = 1; // not whole page
		main_part = 0; // spare part
		trans_dir = 0; //read op
		ecc_enable = 0;
		secc_used = 0;
		sdma_2ch_mode = 0;
		randomizer = 0;
		ret = nf2_page_op(mtd, ecc_enable, page_mode, nand_cfg->mecclvl, rsvd_ecc_en,
					nand_cfg->pagesize, (nand_cfg->sparesize - 4), trans_dir, row_addr,
					ch0_adr, ch0_len, secc_used, nand_cfg->secclvl,
					half_page_en, main_part, nand_cfg->cycle, randomizer,
					&(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
					&(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &tmp, &tmp,
					&corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, nand_cfg->busw);
		
		imap_nand_unmap_vm(DMA_FROM_DEVICE);
		
		//printk("ch0_adr = 0x%x, ch0_len = %d, offsets = 0x%x\n", ch0_adr, ch0_len, offsets);
		//printk("page = 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", page, offsets[0], offsets[1], offsets[2], offsets[3]);
		if(ret){
			printk("nand read spare IO error\n");
			kfree(offsets);
			return -1;
		}

		for(i=0; i<ch0_len; i++){
			if(offsets[i] != 0xff){
				mtd->ecc_stats.failed++;
				//printk("ECC unfixed: page=0x%x (%d, %d)\n", page, !!ecc_unfixed, !!secc_unfixed);
				goto imap_nand_read_page_end;
			}
		}
			
		if(nand_cfg->randomizer == 1){
			memset(pagebuf, 0xff, bytes);
			if(oobbuf != NULL){
				memset(oobbuf, 0xff, oobbytes);
			}
		}
#if 0
		//printk("offsets[0] = 0x%x\n", offsets[0]);
		if(offsets[1] != 0xff){
			mtd->ecc_stats.failed++;
			printk("ECC unfixed: page=0x%x (%d, %d)\n", page, !!ecc_unfixed, !!secc_unfixed);
#if 0
			printk("write oob 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
						g_debug_buf[0], g_debug_buf[1], g_debug_buf[2], g_debug_buf[3],
						g_debug_buf[4], g_debug_buf[5], g_debug_buf[6], g_debug_buf[7],
						g_debug_buf[8], g_debug_buf[9], g_debug_buf[10], g_debug_buf[11],
						g_debug_buf[12], g_debug_buf[13], g_debug_buf[14], g_debug_buf[15],
						g_debug_buf[16], g_debug_buf[17], g_debug_buf[18], g_debug_buf[19], g_debug_wbuf[80]);
			printk("page = 0x%x, offsets 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
						page, offsets[0], offsets[1], offsets[2], offsets[3],
						offsets[4], offsets[5], offsets[6], offsets[7],
						offsets[8], offsets[9], offsets[10], offsets[11],
						offsets[12], offsets[13], offsets[14], offsets[15],
						offsets[16], offsets[17], offsets[18], offsets[19]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[20], g_debug_wbuf[21],g_debug_wbuf[22],g_debug_wbuf[23],g_debug_wbuf[24]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[25], g_debug_wbuf[26],g_debug_wbuf[27],g_debug_wbuf[28],g_debug_wbuf[29]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[30], g_debug_wbuf[31],g_debug_wbuf[32],g_debug_wbuf[33],g_debug_wbuf[34]);
			printk("0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[35], g_debug_wbuf[36],g_debug_wbuf[37],g_debug_wbuf[38]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[50], g_debug_wbuf[51],g_debug_wbuf[52],g_debug_wbuf[53],g_debug_wbuf[54]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[55], g_debug_wbuf[56],g_debug_wbuf[57],g_debug_wbuf[58],g_debug_wbuf[59]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[60], g_debug_wbuf[61],g_debug_wbuf[62],g_debug_wbuf[63],g_debug_wbuf[64]);
			printk("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", g_debug_wbuf[65], g_debug_wbuf[66],g_debug_wbuf[67],g_debug_wbuf[68],g_debug_wbuf[69],g_debug_wbuf[70]);
#endif
		
			//goto read_again_full;
			//printk("ecc_unfixed = 0x%x, secc_unfixed = 0x%x, offset[1] = 0x%x\n", ecc_unfixed, secc_unfixed, offsets[1]);
		}else{
			if(nand_cfg->randomizer == 1){
				memset(pagebuf, 0xff, bytes);
				if(oobbuf != NULL){
					memset(oobbuf, 0xff, oobbytes);
				}
			}
		}
#endif
imap_nand_read_page_end:	
	kfree(offsets);
	}
	return 0;
}

/**
 * nand_read_page_raw - [Intern] read raw page data without ecc
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @page:	page number to read
 *
 * Not for syndrome calculating ecc controllers, which use a special oob layout
 */
int imap_nand_read_raw(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	uint32_t ecc_unfixed = 0, secc_unfixed = 0;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, cycle, randomizer, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int index, curindex;
	uint8_t *readbuf = NULL;
	int readlen = 0;
#if 0
	if((uint32_t)pagebuf & 0x3)
		printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)pagebuf);
	else if((uint32_t)pagebuf & 0x1f)
		printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)pagebuf);
#endif
	//printk("imap_nand_read_raw\n");

	if(pagebuf != NULL){
		readbuf = pagebuf;
		readlen =  bytes;
		main_part = 1;
	}else if(oobbuf != NULL){
		readbuf = oobbuf;
		readlen = oobbytes;
		main_part = 0;
	}
	
	if(readbuf == NULL)
		return -1;

	index = 0;
	if(!imap_nand_map_vm((uint32_t)readbuf, readlen, index, &curindex, DMA_FROM_DEVICE)){
		//TODO:
		printk("dma pagebuf map error\n");
		return -1;
	}

	ecc_enable = 0;
	page_mode = 1;// part page
	ecc_type = 0;
	rsvd_ecc_en = 0;
	page_num = nand_cfg->pagesize;
	sysinfo_num = nand_cfg->sysinfosize;
	trans_dir = 0;// read op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	secc_used = 0;
	secc_type = 0;
	half_page_en = 0;
	cycle = nand_cfg->cycle; 
	randomizer = 0;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;

	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
                    page_num, sysinfo_num, trans_dir, row_addr,
		    ch0_adr, ch0_len, secc_used, secc_type,
                    half_page_en, main_part, cycle, randomizer,
		    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
		    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
		    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_FROM_DEVICE);
#if 0
	printk("imap_nand_read_raw\n");
	printk("ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
	printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
	printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
	printk("pagebuf = 0x%x, oobbuf = 0x%x\n", pagebuf, oobbuf);
	printk("pagebuf[0] = 0x%x, pagebuf[1] = 0x%x, pagebuf[2] = 0x%x, pagebuf[3] = 0x%x\n", pagebuf[0], pagebuf[1], pagebuf[2], pagebuf[3]);
#endif
	if(ret != 0){
		//TODO
		return -1;
	}	
			
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_write_page(struct mtd_info *mtd, struct nand_chip * chip,
 *                    const uint8_t *buf)
 *
 * -Description:
 *    write pagelength bytes from buf to current page, 
 *    fill SECC MECC to oob_poi, and write oob_poi to OOB
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer contains data to write
 *                
 * -Return
 *    none
 ***********************************************************************
 */
int imap_nand_write_page(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, cycle, randomizer, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int ecc_unfixed, secc_unfixed;
	int index, curindex;
	int i;
#if 0
	if((uint32_t)pagebuf & 0x3)
		//printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)pagebuf);
	else if((uint32_t)pagebuf & 0x1f)
		//printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)pagebuf);
#endif

	//printk("w p = 0x%x\n", page);
	index = 0;
	if(!imap_nand_map_vm((uint32_t)pagebuf, mtd->writesize, index, &curindex, DMA_TO_DEVICE)){
		//TODO:
		printk("dma pagebuf map error\n");
		return -1;
	}
	if(curindex >= index + 1){
		printk("waring , not support 2 part not contion memory\n");
	}
	if(imap_nand_dma_list[0].len != mtd->writesize){
		printk("dma pagebuf map len error\n");
		return -1;	
	}
	
	ecc_enable = nand_cfg->eccen;
	page_mode = 0;// whole page
	ecc_type = nand_cfg->mecclvl;
	rsvd_ecc_en = 1;
	page_num = nand_cfg->pagesize;
	sysinfo_num = nand_cfg->sysinfosize;
	trans_dir = 1;// write op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	half_page_en = 0;
	main_part = 0;
	cycle = nand_cfg->cycle;
	randomizer = nand_cfg->randomizer;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;
#if defined(CONFIG_MTD_NAND_IMAPX800_SECC)
	secc_used = nand_cfg->eccen;
	secc_type = nand_cfg->secclvl;
#else
	secc_used = 0;
	secc_type = 0;
#endif	
#if 0
		if((uint32_t)oobbuf & 0x3)
			printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)oobbuf);
		else if((uint32_t)oobbuf & 0x1f)
			printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)oobbuf);
#endif

		oobbuf[1] = 0x0; //write flag
#if 0
	for(i=0; i<20; i++){
		g_debug_buf[i] = oobbuf[i];
	}
	g_debug_wbuf[80] = row_addr;
#endif

	index = curindex + 1;	
		if(!imap_nand_map_vm((uint32_t)oobbuf, oobbytes, index, &curindex, DMA_TO_DEVICE)){
			//TODO:
			printk("dma oobbuf map error\n");
			return -1;
		}
		if(curindex >= index + 1){
			printk("waring , not support 2 part not contion memory\n");
		}
		if(imap_nand_dma_list[1].len != oobbytes){
			printk("dma oobbuf map len error\n");
			return -1;	
		}
		ch1_adr = imap_nand_dma_list[1].dma_addr;
		ch1_len = imap_nand_dma_list[1].len;
		sdma_2ch_mode = 1;

	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        	            page_num, sysinfo_num, trans_dir, row_addr,
			    ch0_adr, ch0_len, secc_used, secc_type,
        	            half_page_en, main_part, cycle, randomizer,
			    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
			    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_TO_DEVICE);
#if 0
	printk("imap_nand_write_page\n");
	printk("ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
	printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
	printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
	printk("pagebuf = 0x%x, oobbuf = 0x%x\n", pagebuf, oobbuf);
	printk("pagebuf[0] = 0x%x, pagebuf[1] = 0x%x, pagebuf[2] = 0x%x, pagebuf[3] = 0x%x\n", pagebuf[0], pagebuf[1], pagebuf[2], pagebuf[3]);
	printk("oobbuf[0] = 0x%x, oobbuf[1] = 0x%x, oobbuf[2] = 0x%x, oobbuf[3] = 0x%x\n", oobbuf[0], oobbuf[1], oobbuf[2], oobbuf[3]);
#endif	
	if(ret != 0){
		//TODO
		return -1;
	}		
#if 0
	printk("write oobbuf 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
				oobbuf[0], oobbuf[1], oobbuf[2], oobbuf[3],
				oobbuf[4], oobbuf[5], oobbuf[6], oobbuf[7],
				oobbuf[8], oobbuf[9], oobbuf[10], oobbuf[11],
				oobbuf[12], oobbuf[13], oobbuf[14], oobbuf[15],
				oobbuf[16], oobbuf[17], oobbuf[18], oobbuf[19]);
#endif
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_write_raw(struct mtd_info *mtd, struct nand_chip * chip,
 *                    const uint8_t *buf)
 *
 * -Description:
 *    write pagelength bytes from buf to current page, 
 *    fill SECC MECC to oob_poi, and write oob_poi to OOB
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer contains data to write
 *                
 * -Return
 *    none
 ***********************************************************************
 */
int imap_nand_write_raw(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes)
{
	struct nand_config *nand_cfg = mtd->priv;
	int ret;
	int corrected_fixed = 0;
	int ecc_enable, page_mode, ecc_type, rsvd_ecc_en, page_num, sysinfo_num;
	int trans_dir, row_addr, ch0_adr, ch0_len, secc_used, secc_type, half_page_en;
	int main_part, randomizer, cycle, ch1_adr, ch1_len, sdma_2ch_mode, busw;
	int ecc_unfixed, secc_unfixed;
	int index, curindex;

	printk("imap_nand_write_raw\n");

	index = 0;
	if(!imap_nand_map_vm((uint32_t)pagebuf, mtd->writesize, index, &curindex, DMA_TO_DEVICE)){
		//TODO:
		printk("dma pagebuf map error\n");
		return -1;
	}
	if(curindex >= index + 1){
		printk("waring , not support 2 part not contion memory\n");
	}
	if(imap_nand_dma_list[0].len != mtd->writesize){
		printk("dma pagebuf map len error\n");
		return -1;	
	}
	
	ecc_enable = 0;
	page_mode = 0;// whole page
	ecc_type = 0;
	rsvd_ecc_en = 1;
	page_num = nand_cfg->pagesize;
	sysinfo_num = nand_cfg->sysinfosize;
	trans_dir = 1;// write op
	row_addr = page;
	ch0_adr = imap_nand_dma_list[0].dma_addr;
	ch0_len = imap_nand_dma_list[0].len;
	secc_used = 0;
	secc_type = 0;
	half_page_en = 0;
	main_part = 0;
	cycle = nand_cfg->cycle;
	randomizer = nand_cfg->randomizer;
	ch1_adr = 0;
	ch1_len = 0;
	sdma_2ch_mode = 0;
	busw = nand_cfg->busw;

	if(oobbuf != NULL){
		if((uint32_t)oobbuf & 0x3)
			printk("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)oobbuf);
		else if((uint32_t)oobbuf & 0x1f)
			printk("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)oobbuf);
	
		index = curindex + 1;	
		if(!imap_nand_map_vm((uint32_t)oobbuf, mtd->oobsize, index, &curindex, DMA_TO_DEVICE)){
			//TODO:
			printk("dma oobbuf map error\n");
			return -1;
		}
		if(curindex >= index + 1){
			printk("waring , not support 2 part not contion memory\n");
		}
		if(imap_nand_dma_list[1].len != mtd->oobsize){
			printk("dma oobbuf map len error\n");
			return -1;	
		}
		ch1_adr = imap_nand_dma_list[1].dma_addr;
		ch1_len = imap_nand_dma_list[1].len;
		sdma_2ch_mode = 1;
	}

	ret = nf2_page_op(mtd, ecc_enable, page_mode, ecc_type, rsvd_ecc_en,
        	            page_num, sysinfo_num, trans_dir, row_addr,
			    ch0_adr, ch0_len, secc_used, secc_type,
        	            half_page_en, main_part, cycle, randomizer,
			    &(nand_cfg->seed[0]), &(nand_cfg->sysinfo_seed[0]), &(nand_cfg->ecc_seed[0]), &(nand_cfg->secc_seed[0]),
			    &(nand_cfg->data_last1K_seed[0]), &(nand_cfg->ecc_last1K_seed[0]), &(ecc_unfixed), &(secc_unfixed),
			    &corrected_fixed, ch1_adr, ch1_len, sdma_2ch_mode, busw);

	imap_nand_unmap_vm(DMA_TO_DEVICE);
	printk("imap_nand_write_raw\n");
	printk("ch0_adr = 0x%x, ch0_len = %d\n", ch0_adr, ch0_len);
	printk("ch1_adr = 0x%x, ch1_len = %d\n", ch1_adr, ch1_len);
	printk("page = 0x%x, page_num = %d, sysinfo_num = %d\n", row_addr, page_num, sysinfo_num);
	printk("pagebuf = 0x%x, oobbuf = 0x%x\n", (unsigned int)pagebuf, (unsigned int)oobbuf);
	printk("pagebuf[0] = 0x%x, pagebuf[1] = 0x%x, pagebuf[2] = 0x%x, pagebuf[3] = 0x%x\n", pagebuf[0], pagebuf[1], pagebuf[2], pagebuf[3]);
	if(ret != 0){
		//TODO
		return -1;
	}		
	
	return 0;
}

/* bad block handling */
/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_hwinit(void)
 *
 * -Description:
 *    this is used before nand_scan to config base status.
 *
 * -Input Param
 *    none
 *                
 * -Return
 *    none
 ***********************************************************************
 */
int imap_nand_hwinit(void)
{
	/* TODO: clock enable, pads init, module enable */
	
	module_power_on(SYSMGR_NAND_BASE);
	module_reset(SYSMGR_NAND_BASE, 0);

	//writel(0x7, PAD_SYSM_VA);

	if(imapx_pad_cfg(IMAPX_NAND, 1)!=0){
		printk("nand io config error\n");
		return -ENOENT;
	}

	/* demo */
	imap_nand.clk = clk_get(NULL, "nandflash");
	if(IS_ERR(imap_nand.clk))
	{
		printk("iMAP NAND: Failed to get bus clock.\n");
		return -ENOENT;
	}

	clk_enable(imap_nand.clk);
	/* Get clk source and enable it */
	imap_nand.clk = clk_get(NULL, "nand-ecc");
	if(IS_ERR(imap_nand.clk))
	{
		printk("iMAP NAND: Failed to get ecc clock.\n");
		return -ENOENT;
	}

	clk_enable(imap_nand.clk);
	printk("imap_nand.clk = %d\n", clk_get_rate(imap_nand.clk));
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_probe(struct platform_device *pdev)
 *
 * -Description:
 *    platform_driver probe
 *
 * -Input Param
 *    pdev    platform device pointer
 *                
 * -Return
 *    0 on success
 *    errno is any error happens
 ***********************************************************************
 */
static int imap_nand_probe(struct platform_device *pdev)
{
	struct mtd_partition *parts;
	int err = 0, ret = 0, nr_part;
        cpumask_var_t mask;
        mask->bits[0] = 0x2;
	pr_debug("In probe function.\n");

	nand_debug_en = 0;

	if(!item_equal("board.disk", "nnd", 0)) {
		char __bd[256];
		item_string(__bd, "board.disk", 0);
		printk(KERN_ERR "board.disk: %s\n", __bd);
		printk(KERN_ERR "NAND driver is not loaded.\n");
		return -ENOENT;
	}


	//g_debug_buf = kmalloc(524288, GFP_KERNEL);
	//memset(g_debug_buf, 0, 524288);

	//g_debug_wbuf = (unsigned int *)g_debug_buf;

	g_nand_state = IMAPX800_NAND_IDLE;
	/* Allocate and remap the resources */
	imap_nand.dev = &pdev->dev;
	imap_nand.regs = ioremap(pdev->resource->start,
				pdev->resource->end - pdev->resource->start + 1);

	if(imap_nand.regs == NULL)
	{
		dev_err(&pdev->dev, "Can not remap register address.\n");
		err = -EIO;
		goto exit_error;
	}

	/* Allocate memory for MTD, structure and private data */
	imap_nand.mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);

	if(imap_nand.mtd == NULL)
	{
		dev_err(&pdev->dev, "Unable to allocate NAND MTD structure\n");
		return -ENOMEM;
	}

	/* Set zero to the structures */
	memset((char *)(imap_nand.mtd), 0, sizeof(struct mtd_info));

	/* Link nand_chip to private data of mtd */
	(imap_nand.mtd)->name = "infotm-nand";

	/* Initialize MTD */
	imap_nand.mtd->owner = THIS_MODULE;

	ret = imap_nand_hwinit();
        if(ret != 0){
		goto exit_error;
	}

#if defined(NAND_USE_IRQ)
	/* Get IRQ Number */
	imap_nand.irqno = platform_get_irq(pdev, 0);
	if (imap_nand.irqno < 0) {
		dev_err(&pdev->dev, "Get NAND IRQ No. failed.\n");
		return -ENOENT;
	}

	err = request_irq(imap_nand.irqno, imap_nand_irq,
				 IRQF_DISABLED, "imap-nand", &imap_nand);

	if (err) {
		dev_err(&pdev->dev, "IRQ%d error %d\n", imap_nand.irqno, ret);
		return err;
	}

    ret = irq_set_affinity(imap_nand.irqno, mask);
    if(ret){
	    dev_err(&pdev->dev, "acq irq set affinity cpu1 error \n");
		return err;
	}

	init_waitqueue_head(&imap_nand.wq);
#endif

	/* Scan NAND device to see if every thing OK */
	if (nand_scan(imap_nand.mtd, 1))
	{
		ret = -ENXIO;
		goto exit_error;
	}

	/* Register the partitions */
	nr_part = parse_mtd_partitions(imap_nand.mtd, part_probes, &parts, 0);

	__nand_msg("nr_part is %d\n", nr_part);

	if(nr_part <= 0)
	{
		__nand_msg("Get cmdline partitions failed, part automatic.\n");
		parts = imap_auto_part(&nr_part);
	} else
	  __nand_msg("Using cmdline based partition table.\n");

	add_mtd_partitions(imap_nand.mtd, parts, nr_part);

	pr_debug("Initialized OK.\n");
	imap_nand_inited = 1;
	return 0;

exit_error:
	kfree(imap_nand.mtd);

	return ret;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_suspend(struct platform_device *pdev, pm_message_t pm)
 *    imap_nand_resume(struct platform_device *pdev)
 *
 * -Description:
 *    PM functions
 *
 * -Input Param
 *                
 * -Return
 *    0
 ***********************************************************************
 */
#ifdef CONFIG_PM
static int imap_nand_suspend(struct platform_device *pdev, pm_message_t pm)
{
#ifndef CONFIG_IMAPX800_FPGA_PLATFORM
	/* Stop clock */
	clk_disable(imap_nand.clk);
#endif
	
	return 0;
}

static int imap_nand_resume(struct platform_device *pdev)
{
	int ret = 0;
        cpumask_var_t mask;
        mask->bits[0] = 0x2;

        ret = irq_set_affinity(imap_nand.irqno, mask);
        if(ret){
            dev_err(&pdev->dev, "acq irq set affinity cpu1 error \n");
            return ret;
        }
	/* Set nfconf & nfcont */
	ret = imap_nand_hwinit();

	return ret;
}
#endif

static int imap_nand_remove(struct platform_device *pdev)
{
	kfree(imap_nand.mtd);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver imap_nand_driver = {
	.probe		= imap_nand_probe,
	.remove		= imap_nand_remove,
#ifdef CONFIG_PM
	.suspend	= imap_nand_suspend,
	.resume		= imap_nand_resume,
#endif
	.driver		= {
		.name		= "imap-nand",
		.owner		= THIS_MODULE,
	},
};

static int __init imap_nand_init(void)
{
	printk(KERN_INFO "iMAPx800/900 NAND MTD Driver (c) 2009,2014 InfoTM ( V5 2CTR)\n");
	return platform_driver_register(&imap_nand_driver);
}

static void __exit imap_nand_exit(void)
{
	platform_driver_unregister(&imap_nand_driver);
}

module_init(imap_nand_init);
module_exit(imap_nand_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("jay <jay.hu@infotmic.com.cn>");
MODULE_DESCRIPTION("iMAPx800/900 NAND MTD Driver");

