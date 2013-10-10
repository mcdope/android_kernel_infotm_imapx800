/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file  of ids pwl  
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** v1.0.1	Sam@2012/3/20: first commit.
** v1.0.7	leo@2012/08/11: readjust.
**
*****************************************************************************/


#ifndef __IDS_PWL_H__
#define __IDS_PWL_H__

//
// keep the order from 0 to (MODULE_NUMS-1).
//
#define MODULE_IDS0		0
#define MODULE_IDS1		1
#define MODULE_DSI		2
#define MODULE_HDMI		3
#define MODULE_LCD0 		5
#define MODULE_LCD1 		6
#define MODULE_I800 		7
#define MODULE_I801 		8
#define MODULE_TVIF0		9
#define MODULE_TVIF1		10
#define MODULE_NUMS		11

#define MODULE_IRQS_MAX		2	// the max irqs for per module.

//
#define SYS_HDMI_IIS_SPDIF_CLK_BASE 0x21E2801C
#define SYS_DSI_CONFIGURATION_CLK_BASE 0x21E2401C

//
#define MODULE_IDS0_REG_BASE	0x22000000
#define MODULE_IDS0_REG_SIZE	0x7000
#define MODULE_IDS0_IRQID	80

#define MODULE_IDS1_REG_BASE	0x23000000
#define MODULE_IDS1_REG_SIZE	0x7000
#define MODULE_IDS1_IRQID	96

#define MODULE_DSI_REG_BASE	0x22040000
#define MODULE_DSI_REG_SIZE	0x1000
#define MODULE_DSI_IRQID	81

#define MODULE_HDMI_REG_BASE	0x23040000
#define MODULE_HDMI_REG_SIZE	0x1000
#define MODULE_HDMI_TX_IRQID	97	
#define MODULE_HDMI_WK_IRQID	98	


//
// freq flags, [31:24] percent of tolerance, [7:0] freq requirement.
//
#define FREQ_FLAG_PRECISE	0x1
#define FREQ_FLAG_ATLEAST	0x2
#define FREQ_FLAG_ATMOST	0x3
#define FREQ_FLAG_AROUND	0x4

//
// mmu register.
//
#define OVCMMU			0x6000

#define OVCMMU_ACCEL_CBCR	(7)
#define OVCMMU_ACCEL_W2		(6)
#define OVCMMU_ACCEL_W1		(5)
#define OVCMMU_ACCEL_W0		(4)
#define OVCMMU_ENABLE_CBCR	(3)
#define OVCMMU_ENABLE_W2	(2)
#define OVCMMU_ENABLE_W1	(1)
#define OVCMMU_ENABLE_W0	(0)

//
//
//
typedef struct{
#if (TARGET_SYSTEM == KERNEL_LINUX)
	struct platform_device *	pdev;
#else
	IM_INT32	rsvd;
#endif
}idsdrv_resource_t;

//
//
//
typedef IM_RET (*fcbk_intr_handler_t)(void);

IM_RET idspwl_init(void);
IM_RET idspwl_deinit(void);

IM_RET idspwl_wait_frame_intr(IM_INT32 idsx, IM_UINT32 num);
IM_RET idspwl_module_enable(IM_INT32 mid);
IM_RET idspwl_module_disable(IM_INT32 mid);
IM_RET idspwl_module_reset(IM_INT32 mid);
IM_RET idspwl_module_request_frequency(IM_INT32 mid, IM_UINT32 freq, IM_UINT32 *resClk);

void *idspwl_malloc(IM_INT32 size);
void idspwl_free(void *p);
void idspwl_memcpy(void *dst, void *src, IM_INT32 size);
void idspwl_memset(void *dst, IM_INT32 c, IM_INT32 size);
IM_RET idspwl_alloc_memory_block(OUT IM_Buffer *buffer, IN IM_INT32 size, IN IM_BOOL linear);
IM_RET idspwl_free_memory_block(IN IM_Buffer *buffer);

void *idspwl_lock_init(void);
IM_RET idspwl_lock_deinit(void *lck);
IM_RET idspwl_lock_lock(void *lck);
IM_RET idspwl_lock_unlock(void *lck);

void *idspwl_sig_init(IM_BOOL manualReset);
IM_RET idspwl_sig_deinit(void *sig);
IM_RET idspwl_sig_set(void *sig);
IM_RET idspwl_sig_reset(void *sig);
IM_RET idspwl_sig_wait(void *sig, void *lck, IM_INT32 timeout);

typedef void (*idspwl_func_thread_entry_t)(void *p);
void *idspwl_thread_init(idspwl_func_thread_entry_t func, void *p);
IM_RET idspwl_thread_deinit(void *thread);

IM_RET idspwl_vsync(IM_INT32 idsx);
void idspwl_msleep(IM_INT32 ms);

IM_RET idspwl_write_reg(IM_INT32 mid, IM_UINT32 addr, IM_UINT32 val);
IM_RET idspwl_write_regbit(IM_INT32 mid, IM_UINT32 addr, IM_INT32 bit, IM_INT32 width, IM_UINT32 val);
IM_RET idspwl_read_reg(IM_INT32 mid, IM_UINT32 addr, IM_UINT32 *val);

IM_RET idspwl_register_isr(IM_INT32 mid, IM_INT32 index, fcbk_intr_handler_t handler);
IM_RET idspwl_unregister_isr(IM_INT32 mid, IM_INT32 index);
IM_RET idspwl_enable_intr(IM_INT32 mid, IM_INT32 index);
IM_RET idspwl_disable_intr(IM_INT32 mid, IM_INT32 index);

IM_RET idspwl_enable_frame_intr(IM_INT32 idsx);
IM_RET idspwl_disable_frame_intr(IM_INT32 idsx);

IM_RET idspwl_init_mmu(IM_INT32 idsx, IM_INT32 wx, IM_BOOL cbcr);
IM_RET idspwl_deinit_mmu(IM_INT32 idsx, IM_INT32 wx);
IM_RET idspwl_enable_mmu(IM_INT32 idsx, IM_INT32 wx);
IM_RET idspwl_disable_mmu(IM_INT32 idsx, IM_INT32 wx);

IM_RET idspwl_pad_set(IM_INT32 index, IM_BOOL high);

IM_RET idspwl_set_bus_config(IM_INT32 busEffic, IM_INT32 ids0DevType, IM_INT32 ids0WorkMode,
            IM_INT32 ids1DevType, IM_INT32 ids1WorkMode);

IM_RET idspwl_suspend(void);
IM_RET idspwl_resume(void);

#endif	// __IDS_PWL_H__

