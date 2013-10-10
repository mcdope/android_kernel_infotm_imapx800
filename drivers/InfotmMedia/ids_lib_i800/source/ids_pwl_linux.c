/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of ids pwl, linux environment 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.2	 Sam@2012/4/09 :  Test stable except for mmu
** 1.0.5	 Sam@2012/6/08 :  hdmi test OK
** 1.0.6	 Sam@2012/6/15 :  module enable & disable location modify 
** 2.0.1     Sam@2013/2/25 :  new version : support double display
**
*****************************************************************************/

#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <asm/irq.h>

#include <mach/pad.h>
#include <mach/io.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <IM_buffallocapi.h>
#include <IM_devmmuapi.h>
#include <pmm_lib.h>
#include <ids_lib.h>
#include <ids_pwl.h>
#include <emif.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IDSPWL_I:"
#define WARNHEAD	"IDSPWL_W:"
#define ERRHEAD		"IDSPWL_E:"
#define TIPHEAD		"IDSPWL_T:"

//
//
//
typedef struct{
	IM_INT32		intrId;
	IM_CHAR			desc[32];
	fcbk_intr_handler_t	intrHandler;
}module_irq_obj_t;

typedef struct{
	IM_UINT32		regBasePhy;
	IM_UINT32		*regBaseVir;
	IM_INT32		regSize;

	IM_INT32		irqNum;
	module_irq_obj_t	irqObj[MODULE_IRQS_MAX];
}module_desc_t;

typedef struct{
	pmm_handle_t		pmm[2][3];
	IM_BOOL			cbcr[2][3];
	pmm_handle_t		pmmcbcr[2];
}dmmu_t;

typedef struct{
	void 			*pmmBufHandle;
	im_list_handle_t    abfList;   
	im_mempool_handle_t mpl;       
}bufalc_t;

typedef struct{
	module_desc_t		module[MODULE_NUMS];

	dmmu_t			dmmu;
	
	bufalc_t 		bufalc;
}pwl_context_t;

static pwl_context_t *gPwl = IM_NULL;


#define IDSINTPND			0x054	//LCD Interrupt pending
#define IDSSRCPND			0x058	//LCD Interrupt source
#define IDSINTMSK			0x05c	//LCD Interrupt mask

#define IDSINTPND_LCDINT 		0
#define IDSINTPND_VCLKINT 		1
#define IDSINTPND_OSDW0 		2
#define IDSINTPND_OSDW1 		3
#define IDSINTPND_OSDERR 		4
#define IDSINTMSK_I80INT 		5
#define IDSINTPND_OSDW2 		6
#define IDSINTPND_OSDW3 		7


typedef struct{
	IM_BOOL			intr_en;
	IM_INT32  		irq_mark;
	wait_queue_head_t 	wait_frame;
}pwl_ids_frame_intr_t;
static pwl_ids_frame_intr_t 	gIdsFrameIntr[2];

extern IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner);
extern IM_RET pmmdrv_release(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid);
extern IM_RET pmmdrv_dmmu_deinit(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_enable(IN pmm_handle_t handle);
extern IM_RET pmmdrv_dmmu_disable(IN pmm_handle_t handle);
extern IM_RET pmmdrv_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer);
extern IM_RET pmmdrv_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer);


IM_RET idspwl_wait_frame_intr(IM_INT32 idsx,IM_UINT32 num)
{
//#define TIME_CHECK	
	IM_UINT32 cnt;
	//IM_INFOMSG((IM_STR("%s(idsx=%d,num=%d)"),IM_STR(_IM_FUNC_),idsx,num));
    //printk("%s(idsx=%d,num=%d)\n",IM_STR(_IM_FUNC_),idsx,num);

#ifdef TIME_CHECK	
	struct timeval _s, _e;
	IM_UINT32 time;
	do_gettimeofday(&_s);
#endif

	if(!gIdsFrameIntr[idsx].intr_en) return IM_RET_OK;
	for(cnt=0;cnt<num;cnt++){
		gIdsFrameIntr[idsx].irq_mark = 0;
		wait_event_interruptible_timeout(gIdsFrameIntr[idsx].wait_frame, gIdsFrameIntr[idsx].irq_mark == 1, HZ/20); 
	}

#ifdef TIME_CHECK	
	do_gettimeofday(&_e);
	time = (_e.tv_sec - _s.tv_sec)*1000000 + _e.tv_usec - _s.tv_usec;
	IM_INFOMSG((IM_STR("ids %d wait %d frame, cost time=%dus"), idsx, num, time));
	//printk("ids %d wait %d frame, cost time=%dms\n", idsx, num, time/1000);
#endif

	return IM_RET_OK;
#undef TIME_CHECK
}

//#define IDS0_CLK_DOWN
#ifdef IDS0_CLK_DOWN
extern IM_INT32 gHdmiStateOn;
IM_INT32 gIds0clkdiv=0;  // TBD : when modify this param, need to have lock 
static IM_UINT32 val_0 =0;
static IM_UINT32 val_1 =0;
#endif

static IM_RET ids_frame_intr_0(void)
{
	IM_UINT32 tmp, vfsr;
	IM_INT32 status;

	idspwl_read_reg(MODULE_IDS0, IDSINTPND, &status);
	if((status & (1 << IDSINTPND_LCDINT)) || (status & ( 1 << IDSINTMSK_I80INT))){
		if(gIdsFrameIntr[0].irq_mark == 0){
			gIdsFrameIntr[0].irq_mark = 1;
			wake_up_interruptible(&gIdsFrameIntr[0].wait_frame);
		}
	}

	idspwl_read_reg(MODULE_IDS0, 0x30, &vfsr);
#ifdef IDS0_CLK_DOWN
	if(vfsr != val_0 && val_0 != 0){
		printk("ids0 clk auto change 0x22000030 = 0x%x - \n",vfsr);
		if(gHdmiStateOn == IM_TRUE && gIds0clkdiv == 0){
			idspwl_read_reg(MODULE_IDS0, 0x00, &gIds0clkdiv);
			gIds0clkdiv = (gIds0clkdiv >> 8) & 0x3FF;
			IM_INFOMSG((IM_STR("gIds0clkdiv = %d - add ids0 clk division \n"),gIds0clkdiv));
            if (gIds0clkdiv > 1){ // to make sure fps > 45hz.
                gIds0clkdiv ++;
                idspwl_write_regbit(MODULE_IDS0,0x0, 8 , 10, gIds0clkdiv); 
            }else{
                gIds0clkdiv = 0;
            }
		}
	}
	val_0 = vfsr & (~0x01);

	if(gHdmiStateOn == IM_FALSE && gIds0clkdiv != 0){
		IM_INFOMSG((IM_STR("minus ids0 clk division -- \n")));
		gIds0clkdiv --;
		idspwl_write_regbit(MODULE_IDS0,0x0, 8 , 10, gIds0clkdiv); 
		gIds0clkdiv = 0;
	}
#endif

	idspwl_write_reg(MODULE_IDS0, IDSSRCPND, status);
	idspwl_write_reg(MODULE_IDS0, IDSINTPND, status);

	if (vfsr & 0x01){
		// If DSI is enabled, we check the auto change flag, if IDS-CLK has been auto changed,
		// it means some data been missed, so reset the MIPI module inorder to avoid the image 
		// displacement. But this action may cause a visual flash.
		idspwl_read_reg(MODULE_DSI, 0x04, &tmp);
		if (tmp & 0x01){
			idspwl_write_regbit(MODULE_DSI, 0x04, 0, 1, 0);
			idspwl_write_regbit(MODULE_DSI, 0x04, 0, 1, 1);
		}
		idspwl_write_regbit(MODULE_IDS0, 0x30, 0, 1, 1);
	}

	return IM_RET_OK;
}

static IM_RET ids_frame_intr_1(void)
{
#ifdef IDS0_CLK_DOWN
	IM_UINT32 vfsr;
#endif
	IM_INT32 status;

	idspwl_read_reg(MODULE_IDS1, IDSINTPND, &status);
	if((status & (1 << IDSINTPND_LCDINT)) || (status & ( 1 << IDSINTMSK_I80INT))){
		if(gIdsFrameIntr[1].irq_mark == 0){
			gIdsFrameIntr[1].irq_mark = 1;
			wake_up_interruptible(&gIdsFrameIntr[1].wait_frame);
		}
	}

#ifdef IDS0_CLK_DOWN
	idspwl_read_reg(MODULE_IDS1, 0x30, &vfsr);
	if(vfsr != val_1 && val_1 != 0){
		printk("ids1 clk auto change 0x23000030 = 0x%x - \n",vfsr);
		if(gHdmiStateOn == IM_TRUE && gIds0clkdiv == 0){
			idspwl_read_reg(MODULE_IDS0, 0x00, &gIds0clkdiv);
			if (gIds0clkdiv & 0x01){
				gIds0clkdiv = (gIds0clkdiv >> 8) & 0x3FF;
				IM_INFOMSG((IM_STR("gIds0clkdiv = %d - add ids0 clk division \n"),gIds0clkdiv));
                if (gIds0clkdiv > 1){ // to make sure fps > 45hz.
                    gIds0clkdiv ++;
                    idspwl_write_regbit(MODULE_IDS0,0x0, 8 , 10, gIds0clkdiv); 
                }else{
                    gIds0clkdiv = 0;
                }
			}
		}
	}
	val_1 = vfsr & ~(0x01);
	if (vfsr & 0x01){
		idspwl_write_regbit(MODULE_IDS0, 0x30, 0, 1, 1);
	}
#endif

	idspwl_write_reg(MODULE_IDS1, IDSSRCPND, status);
	idspwl_write_reg(MODULE_IDS1, IDSINTPND, status);
	return IM_RET_OK;
}

static irqreturn_t idspwl_irq_handle(int irq, void *p)
{
	module_irq_obj_t *irqobj = (module_irq_obj_t *)p;
	//IM_INFOMSG((IM_STR("%s(irq=%d, intrId=%d, desc=%s)"), IM_STR(_IM_FUNC_), irq, irqobj->intrId, irqobj->desc));

	IM_ASSERT(irqobj->intrHandler != IM_NULL);
	if(irqobj->intrHandler() == IM_RET_OK){
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}


IM_RET idspwl_init(void)
{
	IM_INT32 i;
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	//
	IM_ASSERT(gPwl == IM_NULL);
	gPwl = idspwl_malloc(sizeof(pwl_context_t));
	if(gPwl == IM_NULL){
		IM_ERRMSG((IM_STR("idspwl_malloc(gPwl) failed")));
		goto Fail;
	}
	idspwl_memset((void *)gPwl, 0, sizeof(pwl_context_t));

	//
	gPwl->module[MODULE_IDS0].regBasePhy = MODULE_IDS0_REG_BASE;
	gPwl->module[MODULE_IDS0].regSize = MODULE_IDS0_REG_SIZE;
	gPwl->module[MODULE_IDS0].regBaseVir = (IM_UINT32 *)ioremap_nocache(MODULE_IDS0_REG_BASE, MODULE_IDS0_REG_SIZE);
	if(gPwl->module[MODULE_IDS0].regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("ioremap(IDS0) regBaseVir failed")));
		goto Fail;
	}

	gPwl->module[MODULE_IDS1].regBasePhy = MODULE_IDS1_REG_BASE;
	gPwl->module[MODULE_IDS1].regSize = MODULE_IDS1_REG_SIZE;
	gPwl->module[MODULE_IDS1].regBaseVir = (IM_UINT32 *)ioremap_nocache(MODULE_IDS1_REG_BASE, MODULE_IDS1_REG_SIZE);
	if(gPwl->module[MODULE_IDS1].regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("ioremap(IDS1) regBaseVir failed")));
		goto Fail;
	}

	gPwl->module[MODULE_DSI].regBasePhy = MODULE_DSI_REG_BASE;
	gPwl->module[MODULE_DSI].regSize = MODULE_DSI_REG_SIZE;
	gPwl->module[MODULE_DSI].regBaseVir = (IM_UINT32 *)ioremap_nocache(MODULE_DSI_REG_BASE, MODULE_DSI_REG_SIZE);
	if(gPwl->module[MODULE_DSI].regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("ioremap(DSI) regBaseVir failed")));
		goto Fail;
	}

	gPwl->module[MODULE_HDMI].regBasePhy = MODULE_HDMI_REG_BASE;
	gPwl->module[MODULE_HDMI].regSize = MODULE_HDMI_REG_SIZE;
	gPwl->module[MODULE_HDMI].regBaseVir = (IM_UINT32 *)ioremap_nocache(MODULE_HDMI_REG_BASE, MODULE_HDMI_REG_SIZE);
	if(gPwl->module[MODULE_HDMI].regBaseVir == IM_NULL){
		IM_ERRMSG((IM_STR("ioremap(HDMI) regBaseVir failed")));
		goto Fail;
	}

	// irq.
	gPwl->module[MODULE_IDS0].irqNum = 1;
	gPwl->module[MODULE_IDS0].irqObj[0].intrId = MODULE_IDS0_IRQID;
	strcpy(gPwl->module[MODULE_IDS0].irqObj[0].desc, "irq-ids0");
	
	gPwl->module[MODULE_IDS1].irqNum = 1;
	gPwl->module[MODULE_IDS1].irqObj[0].intrId = MODULE_IDS1_IRQID;
	strcpy(gPwl->module[MODULE_IDS1].irqObj[0].desc, "irq-ids1");

	gPwl->module[MODULE_DSI].irqNum = 1;
	gPwl->module[MODULE_DSI].irqObj[0].intrId = MODULE_DSI_IRQID;
	strcpy(gPwl->module[MODULE_DSI].irqObj[0].desc, "irq-dsi");

	gPwl->module[MODULE_HDMI].irqNum = 2;
	gPwl->module[MODULE_HDMI].irqObj[0].intrId = MODULE_HDMI_TX_IRQID;
	strcpy(gPwl->module[MODULE_HDMI].irqObj[0].desc, "irq-hdmi-tx");
	gPwl->module[MODULE_HDMI].irqObj[1].intrId = MODULE_HDMI_WK_IRQID;
	strcpy(gPwl->module[MODULE_HDMI].irqObj[1].desc, "irq-hdmi-wk");

	//
	gIdsFrameIntr[0].intr_en = IM_FALSE;
	gIdsFrameIntr[1].intr_en = IM_FALSE;

	ret = pmmdrv_open(&gPwl->bufalc.pmmBufHandle, "ids-pwl");  
	if(ret != IM_RET_OK){
		printk(" ids-pwl pmmdrv_open failed ");
		goto Fail;
	}

	gPwl->bufalc.mpl = im_mpool_init((func_mempool_malloc_t)idspwl_malloc, (func_mempool_free_t)idspwl_free);
	if(gPwl->bufalc.mpl == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed")));
		goto Fail;
	}

	gPwl->bufalc.abfList = im_list_init(sizeof(alc_buffer_t), gPwl->bufalc.mpl);
	if(gPwl->bufalc.abfList == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init() failed")));
		goto Fail;
	}

	return IM_RET_OK;
Fail:
	for(i=0; i<MODULE_NUMS; i++){
		if(gPwl->module[i].regBaseVir != IM_NULL){
			iounmap((void *)gPwl->module[i].regBaseVir);
		}
	}

	idspwl_free((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_FAILED;
}

IM_RET idspwl_deinit(void)
{
	IM_INT32 i;
	alc_buffer_t *abf;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_ASSERT(gPwl != IM_NULL);

	for(i=0; i<MODULE_NUMS; i++){
		if(gPwl->module[i].regBaseVir != IM_NULL){
			iounmap((void *)gPwl->module[i].regBaseVir);
		}
	}
	if(gPwl->bufalc.abfList != IM_NULL){
		abf = (alc_buffer_t *)im_list_begin(gPwl->bufalc.abfList);
		while(abf != IM_NULL){
			if(abf->attri & ALC_BUFFER_ATTRI_ALLOCATED){
				pmmdrv_mm_free(gPwl->bufalc.pmmBufHandle, abf);
			}
			else{
				IM_ASSERT(IM_FALSE);
			}
			abf = (alc_buffer_t *)im_list_erase(gPwl->bufalc.abfList, abf);
		}
		im_list_deinit(gPwl->bufalc.abfList);
	}
	if(gPwl->bufalc.mpl != IM_NULL){
		im_mpool_deinit(gPwl->bufalc.mpl);
	}
	if(gPwl->bufalc.pmmBufHandle != IM_NULL){
		pmmdrv_release(gPwl->bufalc.pmmBufHandle);
	}

	idspwl_free((void *)gPwl);
	gPwl = IM_NULL;

	return IM_RET_OK;
}

IM_RET idspwl_module_enable(IM_INT32 mid)
{
	IM_INT32 res;
	IM_UINT8 *addr;
	struct clk *clksrc;

	IM_INFOMSG((IM_STR("%s(mid=%d)"), IM_STR(_IM_FUNC_), mid));
	switch (mid){
	case MODULE_LCD0:
	case MODULE_I800:
		if(imapx_pad_cfg(IMAPX_RGB0,1) == -1 ){
			IM_ERRMSG((IM_STR("imapx_pad_cfg(IMAPX_RGB0, 1) failed")));
			return IM_RET_FAILED;
		}
		break;
	case MODULE_LCD1:
	case MODULE_I801:
		break;
	case MODULE_HDMI:
		clksrc = clk_get(NULL, "hdmi");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(hdmi) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("hdmi clk-rate is %lu"), clk_get_rate(clksrc)));

		addr = (IM_UINT8*) ioremap_nocache(SYS_HDMI_IIS_SPDIF_CLK_BASE, 0x100);
		addr[0x00] = 0x03;
		iounmap(addr);

		if (imapx_pad_cfg(IMAPX_HDMI, 1) == -1) {
			IM_ERRMSG((IM_STR("imapx_pad_cfg(IMAPX_HDMI, 1) failed")));
			return IM_RET_FAILED;
		}
		break;
	case MODULE_IDS0:
		module_power_on(SYSMGR_IDS0_BASE);

		clksrc = clk_get(NULL, "ids0");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids0) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids0 clk-rate is %lu"), clk_get_rate(clksrc)));

		clksrc = clk_get(NULL, "ids0-ods");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids0-ods) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids0-ods clk-rate is %lu"), clk_get_rate(clksrc)));

		clksrc = clk_get(NULL, "ids0-eitf");
		if(clksrc == NULL){
			IM_ERRMSG((IM_STR("clk_get(ids0-eitf) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids0-eitf clk-rate is %lu"), clk_get_rate(clksrc)));

		break;
	case MODULE_IDS1:
		module_power_on(SYSMGR_IDS1_BASE);

		clksrc = clk_get(NULL, "ids1");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids1) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids1 clk-rate is %lu"), clk_get_rate(clksrc)));

		clksrc = clk_get(NULL, "ids1-ods");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids1-ods) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids1-ods clk-rate is %lu"), clk_get_rate(clksrc)));

		clksrc = clk_get(NULL, "ids1-eitf");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids1-eitf) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
		//IM_TIPMSG((IM_STR("ids1-eitf clk-rate is %lu"), clk_get_rate(clksrc)));
		break;
	case MODULE_DSI:
		addr = (IM_UINT8*) ioremap_nocache(SYS_DSI_CONFIGURATION_CLK_BASE , 0x100);
		addr[0x00] = 0x07;
		iounmap(addr);

		clksrc = clk_get(NULL, "mipi-dphy-con");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-con) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
        res = clk_set_rate(clksrc,27000); 
        if (res < 0){
            IM_ERRMSG((IM_STR(" clk_set_rate(mipi-dphy-con) failed - ")));
            return IM_RET_FAILED;
        }

		clksrc = clk_get(NULL, "mipi-dphy-ref");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-ref) failed")));
			return IM_RET_FAILED;
		}
		clk_enable(clksrc);
        res = clk_set_rate(clksrc,27000); 
        if (res < 0){
            IM_ERRMSG((IM_STR(" clk_set_rate(mipi-dphy-ref) failed - ")));
            return IM_RET_FAILED;
        }

        break;
	case MODULE_TVIF0:
	case MODULE_TVIF1:
	default :
		break;
	}

	return IM_RET_OK;
}

IM_RET idspwl_module_disable(IM_INT32 mid)
{
	IM_UINT8 *addr;
	struct clk *clksrc;
	IM_INFOMSG((IM_STR("%s(mid=%d)"), IM_STR(_IM_FUNC_), mid));
	switch (mid){
	case MODULE_IDS0:
		clksrc = clk_get(NULL, "ids0-eitf");
		if(clksrc == NULL){
			IM_ERRMSG((IM_STR("clk_get(ids0-eitf) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		clksrc = clk_get(NULL, "ids0-ods");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids0-ods) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		clksrc = clk_get(NULL, "ids0");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids0) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		module_power_down(SYSMGR_IDS0_BASE);
		break;
	case MODULE_IDS1:
		clksrc = clk_get(NULL, "ids1-eitf");
		if(clksrc == NULL){
			IM_ERRMSG((IM_STR("clk_get(ids1-eitf) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		clksrc = clk_get(NULL, "ids1-ods");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids1-ods) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		clksrc = clk_get(NULL, "ids1");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(ids1) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		module_power_down(SYSMGR_IDS1_BASE);
		break;
	case MODULE_HDMI:
		clksrc = clk_get(NULL, "hdmi");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(hdmi) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		addr = (IM_UINT8*) ioremap_nocache(SYS_HDMI_IIS_SPDIF_CLK_BASE, 0x100);
		addr[0x00] = 0x03;
		iounmap(addr);
		break;
	case MODULE_LCD0:
	case MODULE_I800:
		if(imapx_pad_cfg(IMAPX_RGB0,0) == -1 ){
			IM_ERRMSG((IM_STR("imapx_pad_cfg(IMAPX_RGB0, 0) failed")));
			return IM_RET_FAILED;
		}
		break;
	case MODULE_DSI:
		clksrc = clk_get(NULL, "mipi-dphy-con");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-con) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		clksrc = clk_get(NULL, "mipi-dphy-ref");
		if (clksrc == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-ref) failed")));
			return IM_RET_FAILED;
		}
		clk_disable(clksrc);

		module_power_down(SYSMGR_MIPI_BASE);

		addr = (IM_UINT8*) ioremap_nocache(SYS_DSI_CONFIGURATION_CLK_BASE , 0x100);
		addr[0x00] = 0x00;
		iounmap(addr);
        break;
	case MODULE_LCD1:
	case MODULE_I801:
	case MODULE_TVIF0:
	case MODULE_TVIF1:
	default:
		break;
	}

	return IM_RET_OK;
}

IM_RET idspwl_module_reset(IM_INT32 mid)
{
	IM_INFOMSG((IM_STR("%s(mid=%d)"), IM_STR(_IM_FUNC_), mid));
	switch (mid){
	case MODULE_IDS0:
		module_reset(SYSMGR_IDS0_BASE, 1);
		break;
	case MODULE_IDS1:
		module_reset(SYSMGR_IDS1_BASE, 1);
		break;
	case MODULE_HDMI:
		module_reset(SYSMGR_IDS1_BASE, 6);
		break;
	case MODULE_LCD0:
	case MODULE_I800:
	case MODULE_TVIF0:
	case MODULE_LCD1:
	case MODULE_I801:
	case MODULE_TVIF1:
	case MODULE_DSI:
	default:
		break;
	}
	return IM_RET_OK;
}

IM_RET idspwl_module_request_frequency(IM_INT32 mid, IM_UINT32 freq, IM_UINT32 *resClk)
{
	struct clk *clksrc;
	IM_INT32 res;
	IM_INFOMSG((IM_STR("%s(mid=%d, freq=%d)"), IM_STR(_IM_FUNC_), mid, freq));

	switch (mid){
		case MODULE_LCD0:
		case MODULE_I800:
			clksrc = clk_get(NULL, "ids0-eitf");
			if (clksrc == NULL) {
				IM_ERRMSG((IM_STR("clk_get(ids0-eitf) failed")));
				return IM_RET_FAILED;
			}
			res = clk_set_rate(clksrc,freq/1000); 
			if (res < 0){
				IM_ERRMSG((IM_STR(" clk_set_rate(ids0-eitf) failed - ")));
				return IM_RET_FAILED;
			}
			*resClk = (IM_UINT32)clk_get_rate(clksrc);
			break;
		case MODULE_LCD1:
		case MODULE_I801:
			clksrc = clk_get(NULL, "ids1-eitf");
			if (clksrc == NULL) {
				IM_ERRMSG((IM_STR("clk_get(ids1-eitf) failed")));
				return IM_RET_FAILED;
			}
			res = clk_set_rate(clksrc,freq/1000); 
			if (res < 0){
				IM_ERRMSG((IM_STR(" clk_set_rate(ids1-eitf) failed - ")));
				return IM_RET_FAILED;
			}
			*resClk = (IM_UINT32)clk_get_rate(clksrc);
			break;
		case MODULE_IDS0:
			clksrc = clk_get(NULL, "ids0-ods");
			if (clksrc == NULL) {
				IM_ERRMSG((IM_STR("clk_get(ids0-ods) failed")));
				return IM_RET_FAILED;
			}
			res = clk_set_rate(clksrc,freq/1000); 
			if (res < 0){
				IM_ERRMSG((IM_STR(" clk_set_rate(ids0-ods) failed - ")));
				return IM_RET_FAILED;
			}
			*resClk = (IM_UINT32)clk_get_rate(clksrc);
			break;
		case MODULE_IDS1:
			clksrc = clk_get(NULL, "ids1-ods");
			if (clksrc == NULL) {
				IM_ERRMSG((IM_STR("clk_get(ids1-ods) failed")));
				return IM_RET_FAILED;
			}
			res = clk_set_rate(clksrc,freq/1000); 
			if (res < 0){
				IM_ERRMSG((IM_STR(" clk_set_rate(ids1-ods) failed - ")));
				return IM_RET_FAILED;
			}
			*resClk = (IM_UINT32)clk_get_rate(clksrc);
			break;
		case MODULE_HDMI:
			clksrc = clk_get(NULL, "hdmi");
			if (clksrc == NULL) {
				IM_ERRMSG((IM_STR("clk_get(hdmi) failed")));
				return IM_RET_FAILED;
			}
			res = clk_set_rate(clksrc,freq/1000); 
			if (res < 0){
				IM_ERRMSG((IM_STR(" clk_set_rate(hdmi) failed - ")));
				return IM_RET_FAILED;
			}
			*resClk = (IM_UINT32)clk_get_rate(clksrc);
			break;
		case MODULE_DSI:
		case MODULE_TVIF0:
		case MODULE_TVIF1:
		default :
			break;
	}

	return IM_RET_OK;
}

void *idspwl_malloc(IM_INT32 size)
{
	return kmalloc(size, GFP_KERNEL);
}

void idspwl_free(void *p)
{
	if(p != IM_NULL){
		kfree(p);
	}
}

void idspwl_memcpy(void *dst, void *src, IM_INT32 size)
{
	memcpy(dst, src, size);
}

void idspwl_memset(void *dst, IM_INT32 c, IM_INT32 size)
{
	memset(dst, c, size);
}

IM_RET idspwl_alloc_memory_block(OUT IM_Buffer *buffer, IN IM_INT32 size, IN IM_BOOL linear)
{
	IM_RET ret;
	alc_buffer_t abf;
	IM_INT32 flag = linear ? ALC_FLAG_PHY_MUST : (ALC_FLAG_PHY_LINEAR_PREFER | ALC_FLAG_DEVADDR);
	IM_ASSERT(buffer != IM_NULL);
	
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ret = pmmdrv_mm_alloc(gPwl->bufalc.pmmBufHandle, size, flag, &abf);
	if (ret == IM_RET_FAILED){
		printk("pmmdrv_mm_alloc failed -- \n");
		return IM_RET_FAILED;
	}
	IM_ASSERT(abf.attri & ALC_BUFFER_ATTRI_ALLOCATED);

	if(im_list_put_back(gPwl->bufalc.abfList, &abf) != IM_RET_OK){
		IM_ERRMSG((IM_STR("im_list_put_back() failed")));
		pmmdrv_mm_free(gPwl->bufalc.pmmBufHandle, &abf);
		return IM_RET_FAILED;
	}
	idspwl_memcpy((void*)buffer, (void*)&abf.buffer, sizeof(IM_Buffer));
	return IM_RET_OK;
}

IM_RET idspwl_free_memory_block(IN IM_Buffer *buffer)
{
	alc_buffer_t *abf;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	abf = (alc_buffer_t *)im_list_begin(gPwl->bufalc.abfList);
	while(abf != IM_NULL){
		if((abf->buffer.vir_addr == buffer->vir_addr) && (abf->attri & ALC_BUFFER_ATTRI_ALLOCATED)){
			break;
		}
		abf = (alc_buffer_t *)im_list_next(gPwl->bufalc.abfList);
	}

	if(abf == IM_NULL){
		IM_ERRMSG((IM_STR("not found the allocated buffer, vir_addr=0x%x"), (IM_INT32)buffer->vir_addr));
		return IM_RET_FAILED;
	}
	
	pmmdrv_mm_free(gPwl->bufalc.pmmBufHandle, abf);
	im_list_erase(gPwl->bufalc.abfList, abf);
	
	return IM_RET_OK;
}


void *idspwl_lock_init(void)
{
	struct mutex *mtx = (struct mutex *)idspwl_malloc(sizeof(struct mutex));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(mtx == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(mtx) failed")));
		return IM_NULL;
	}
	mutex_init(mtx);
	return (void *)mtx;
}

IM_RET idspwl_lock_deinit(void *lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_destroy((struct mutex *)lck);
	idspwl_free(lck);
	return IM_RET_OK;
}

IM_RET idspwl_lock_lock(void *lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_lock((struct mutex *)lck);
	return IM_RET_OK;
}

IM_RET idspwl_lock_unlock(void *lck)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(lck != IM_NULL);
	mutex_unlock((struct mutex *)lck);
	return IM_RET_OK;
}

typedef struct{
	IM_BOOL manualReset;
	struct semaphore sem;
}signal_t;

void * idspwl_sig_init(IM_BOOL manualReset)
{
	signal_t *sig = (signal_t *)idspwl_malloc(sizeof(signal_t));
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(sig == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(sig) failed")));
		return IM_NULL;
	}
	sig->manualReset = manualReset;
	sema_init(&sig->sem, 0);
	return (void *)sig;
}

IM_RET idspwl_sig_deinit(void *sig)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);
	idspwl_free(sig);
	return IM_RET_OK;
}

IM_RET idspwl_sig_set(void *sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(signal != IM_NULL);
	up(&signal->sem);
	return IM_RET_OK;
}

IM_RET idspwl_sig_reset(void *sig)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(signal != IM_NULL);
	while(down_trylock(&signal->sem) == 0);
	return IM_RET_OK;
}

IM_RET idspwl_sig_wait(void *sig, void *lck, IM_INT32 timeout)
{
	signal_t *signal = (signal_t *)sig;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(sig != IM_NULL);

	if(lck != IM_NULL){		
		idspwl_lock_unlock(lck);
	}

	if(timeout == -1){
		if(down_interruptible(&signal->sem) != 0){
			if(lck != IM_NULL){
				idspwl_lock_lock(lck);
			}
			return IM_RET_FAILED;
		}
	}else{
		if(down_timeout(&signal->sem, (long)timeout * HZ / 1000) == -ETIME){
			if(lck != IM_NULL){
				idspwl_lock_lock(lck);
			}
			return IM_RET_TIMEOUT;
		}
	}

	if(signal->manualReset == IM_TRUE){
		up(&signal->sem);
	}
	if(lck != IM_NULL){
		idspwl_lock_lock(lck);
	}

	return IM_RET_OK;
}

typedef struct {
	struct work_struct work;
	struct workqueue_struct *wq;
	void *data;
	idspwl_func_thread_entry_t func;
}thread_t;

static void work_handle(struct work_struct *p)
{
	thread_t *thrd = container_of(p, thread_t, work);
	thrd->func(thrd->data);
}

void * idspwl_thread_init(idspwl_func_thread_entry_t func, void *data)
{
	int ret = 1;
	thread_t *thrd = IM_NULL;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	thrd = (thread_t *)idspwl_malloc(sizeof(thread_t));
	if(thrd == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(thread_t) failed!")));
		return IM_NULL;
	}

	thrd->wq = create_singlethread_workqueue("idspwl_wq");
	if(thrd->wq == NULL){
		IM_ERRMSG((IM_STR("create_singlethread_workqueue() failed!")));
		idspwl_free((void *)thrd);
		return IM_NULL;
	}

	thrd->data = data;
	thrd->func = func;
	INIT_WORK(&thrd->work, work_handle);

	//INIT_WORK(&thrd->work, func);
	ret = queue_work(thrd->wq, &thrd->work);
	if(ret != 1){
		destroy_workqueue(thrd->wq);
		idspwl_free((void *)thrd);
		return IM_NULL;
	}
	
	return (void *)thrd;
}

IM_RET idspwl_thread_deinit(void * thread)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(thread != IM_NULL);
	destroy_workqueue(((thread_t *)thread)->wq);
	idspwl_free((void *)thread);
	return IM_RET_OK;
}

IM_RET idspwl_vsync(IM_INT32 idsx)
{
	idspwl_wait_frame_intr(idsx, 1);
	return IM_RET_OK;
}

void idspwl_msleep(IM_INT32 ms)
{
	msleep(ms);
}

IM_RET idspwl_write_reg(IM_INT32 mid, IM_UINT32 addr, IM_UINT32 val)
{
	writel(val, (IM_UINT32)gPwl->module[mid].regBaseVir + addr);
	return IM_RET_OK;
}

IM_RET idspwl_write_regbit(IM_INT32 mid, IM_UINT32 addr, IM_INT32 bit, IM_INT32 width, IM_UINT32 val)
{
	IM_UINT32 tmp;
	idspwl_read_reg(mid, addr, &tmp);
	tmp &= ~(((0xFFFFFFFF << (32 - width)) >> (32 - width)) << bit);
	tmp |= val << bit;
	idspwl_write_reg(mid, addr, tmp);
	return IM_RET_OK;
}

IM_RET idspwl_read_reg(IM_INT32 mid, IM_UINT32 addr, IM_UINT32 *val)
{
	*val = readl((IM_UINT32)gPwl->module[mid].regBaseVir + addr);
	return IM_RET_OK;
}

IM_RET idspwl_register_isr(IM_INT32 mid, IM_INT32 index, fcbk_intr_handler_t handler)
{
	IM_INFOMSG((IM_STR("%s(mid=%d, index=%d)"), IM_STR(_IM_FUNC_), mid, index));

	gPwl->module[mid].irqObj[index].intrHandler = handler;
	if(request_irq(gPwl->module[mid].irqObj[index].intrId, idspwl_irq_handle, IRQF_DISABLED, 
		gPwl->module[mid].irqObj[index].desc, (void *)&gPwl->module[mid].irqObj[index]) != 0){
		IM_ERRMSG((IM_STR("request_irq() failed")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

IM_RET idspwl_unregister_isr(IM_INT32 mid, IM_INT32 index)
{
	IM_INFOMSG((IM_STR("%s(mid=%d, index=%d)"), IM_STR(_IM_FUNC_), mid, index));
	free_irq(gPwl->module[mid].irqObj[index].intrId, (void *)&gPwl->module[mid].irqObj[index]);
	return IM_RET_OK;
}

IM_RET idspwl_enable_intr(IM_INT32 mid, IM_INT32 index)
{
	IM_INFOMSG((IM_STR("%s(mid=%d, index=%d)"), IM_STR(_IM_FUNC_), mid, index));
	enable_irq(gPwl->module[mid].irqObj[index].intrId);
	return IM_RET_OK;
}

IM_RET idspwl_disable_intr(IM_INT32 mid, IM_INT32 index)
{
	IM_INFOMSG((IM_STR("%s(mid=%d, index=%d)"), IM_STR(_IM_FUNC_), mid, index));
	disable_irq(gPwl->module[mid].irqObj[index].intrId);
	return IM_RET_OK;
}

IM_RET idspwl_enable_frame_intr(IM_INT32 idsx)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_), idsx));

	if(gIdsFrameIntr[idsx].intr_en){
	       	return IM_RET_OK;
	}
	gIdsFrameIntr[idsx].intr_en = IM_TRUE;

	init_waitqueue_head(&gIdsFrameIntr[idsx].wait_frame);
	gIdsFrameIntr[idsx].irq_mark = 1;

	if(idsx == 0){
		ret = idspwl_register_isr(MODULE_IDS0, 0, (fcbk_intr_handler_t) ids_frame_intr_0);
		idspwl_write_reg(MODULE_IDS0,IDSINTMSK, 0);
	}else{
		ret = idspwl_register_isr(MODULE_IDS1, 0, (fcbk_intr_handler_t) ids_frame_intr_1);
		idspwl_write_reg(MODULE_IDS1,IDSINTMSK, 0);
	}
	return ret;
}

IM_RET idspwl_disable_frame_intr(IM_INT32 idsx)
{
	IM_INFOMSG((IM_STR("%s(idsx=%d)"), IM_STR(_IM_FUNC_),idsx));
	if (gIdsFrameIntr[idsx].intr_en == IM_TRUE){
		idspwl_unregister_isr(idsx ? MODULE_IDS1 : MODULE_IDS0, 0);
		idspwl_write_reg(idsx ? MODULE_IDS1: MODULE_IDS0, IDSINTMSK, 0xFF);
		gIdsFrameIntr[idsx].intr_en = IM_FALSE;
	}
	return IM_RET_OK;
}

IM_RET idspwl_init_mmu(IM_INT32 idsx, IM_INT32 wx, IM_BOOL cbcr)
{
	char owner[ALC_OWNER_LEN_MAX];
	IM_INT32 devid;
	IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d, cbcr=%d)"), IM_STR(_IM_FUNC_), idsx, wx, cbcr));
	IM_ASSERT(wx <= 2);

	IM_ASSERT(gPwl->dmmu.pmm[idsx][wx] == IM_NULL);
	
	sprintf(owner, "ids%dw%d", idsx, wx);
	if(pmmdrv_open(&gPwl->dmmu.pmm[idsx][wx], owner) != IM_RET_OK){
		IM_ERRMSG((IM_STR("pmmdrv_open(%s) failed"), owner));
		return IM_RET_FAILED;
	}

	//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ENABLE_W0, 1, 1);
	//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ACCEL_W0, 1, 1);

	devid = (idsx==0) ? DMMU_DEV_IDS0_W0 : DMMU_DEV_IDS1_W0;
	devid += wx;
	if(pmmdrv_dmmu_init(gPwl->dmmu.pmm[idsx][wx], devid) != IM_RET_OK){
		IM_ERRMSG((IM_STR("pmmdrv_dmmu_init(devid=%d) failed"), devid));
		goto Fail;
	}

	if(cbcr == IM_TRUE){
		IM_ASSERT(gPwl->dmmu.pmmcbcr[idsx] == IM_NULL);

		sprintf(owner, "ids%dw%dcbcr", idsx, wx);
		if(pmmdrv_open(&gPwl->dmmu.pmmcbcr[idsx], owner) != IM_RET_OK){
			IM_ERRMSG((IM_STR("pmmdrv_open(%s) failed"), owner));
			goto Fail;
		}
		
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ENABLE_CBCR, 1, 1);
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ACCEL_CBCR, 1, 1);

		devid = (idsx==0) ? DMMU_DEV_IDS0_CBCR : DMMU_DEV_IDS1_CBCR;
		if(pmmdrv_dmmu_init(gPwl->dmmu.pmmcbcr[idsx], devid) != IM_RET_OK){
			IM_ERRMSG((IM_STR("pmmdrv_dmmu_init(devid=%d) failed"), devid));
			goto Fail;
		}
	}

	gPwl->dmmu.cbcr[idsx][wx] = cbcr;
	return IM_RET_OK;
Fail:
	if(gPwl->dmmu.pmmcbcr[idsx]){
		pmmdrv_dmmu_deinit(gPwl->dmmu.pmmcbcr[idsx]);
		pmmdrv_release(gPwl->dmmu.pmmcbcr[idsx]);
		gPwl->dmmu.pmmcbcr[idsx] = IM_NULL;
		
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ENABLE_CBCR, 1, 0);
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ACCEL_CBCR, 1, 0);
	}
	if(gPwl->dmmu.pmm[idsx][wx]){
		pmmdrv_dmmu_deinit(gPwl->dmmu.pmm[idsx][wx]);
		pmmdrv_release(gPwl->dmmu.pmm[idsx][wx]);
		gPwl->dmmu.pmm[idsx][wx] = IM_NULL;
		
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ENABLE_W0, 1, 0);
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ACCEL_W0, 1, 0);
	}

	return IM_RET_FAILED;
}

IM_RET idspwl_deinit_mmu(IM_INT32 idsx, IM_INT32 wx)
{
	IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));
	IM_ASSERT(wx <= 2);

	if(gPwl->dmmu.pmm[idsx][wx]){
		pmmdrv_dmmu_deinit(gPwl->dmmu.pmm[idsx][wx]);
		pmmdrv_release(gPwl->dmmu.pmm[idsx][wx]);
		gPwl->dmmu.pmm[idsx][wx] = IM_NULL;
	}

	if(gPwl->dmmu.cbcr[idsx][wx]){
		IM_ASSERT(gPwl->dmmu.pmmcbcr[idsx]);
		pmmdrv_dmmu_deinit(gPwl->dmmu.pmmcbcr[idsx]);
		pmmdrv_release(gPwl->dmmu.pmmcbcr[idsx]);
		gPwl->dmmu.pmmcbcr[idsx] = IM_NULL;
	}

	return IM_RET_OK;
}

IM_RET idspwl_enable_mmu(IM_INT32 idsx, IM_INT32 wx)
{
	IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));
	IM_ASSERT(wx <= 2);

	IM_ASSERT(gPwl->dmmu.pmm[idsx][wx]);
	if(pmmdrv_dmmu_enable(gPwl->dmmu.pmm[idsx][wx]) != IM_RET_OK){
		IM_ERRMSG((IM_STR("pmmdrv_dmmu_enable() failed")));
		return IM_RET_FAILED;
	}

	if(gPwl->dmmu.cbcr[idsx][wx]){
		IM_ASSERT(gPwl->dmmu.pmmcbcr[idsx]);
		if(pmmdrv_dmmu_enable(gPwl->dmmu.pmmcbcr[idsx]) != IM_RET_OK){
			IM_ERRMSG((IM_STR("pmmdrv_dmmu_enable() failed")));
			pmmdrv_dmmu_disable(gPwl->dmmu.pmm[idsx][wx]);
			return IM_RET_FAILED;
		}
	}

	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ENABLE_W0, 1, 1);
    //sam : mmu accelarator may have some bugs. it may cause ids dead.
	//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ACCEL_W0, 1, 1);
	if(gPwl->dmmu.cbcr[idsx][wx]){
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ENABLE_CBCR, 1, 1);
		//idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ACCEL_CBCR, 1, 1);
	}

	return IM_RET_OK;
}

IM_RET idspwl_disable_mmu(IM_INT32 idsx, IM_INT32 wx)
{
	IM_INFOMSG((IM_STR("%s(idsx=%d, wx=%d)"), IM_STR(_IM_FUNC_), idsx, wx));
	IM_ASSERT(wx <= 2);
	IM_ASSERT(gPwl->dmmu.pmm[idsx][wx]);

	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ENABLE_W0, 1, 0);
	idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, wx << OVCMMU_ACCEL_W0, 1, 0);
	if(gPwl->dmmu.cbcr[idsx][wx]){
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ENABLE_CBCR, 1, 0);
		idspwl_write_regbit((idsx==0)?MODULE_IDS0:MODULE_IDS1, OVCMMU, OVCMMU_ACCEL_CBCR, 1, 0);
	}

	if(pmmdrv_dmmu_disable(gPwl->dmmu.pmm[idsx][wx]) != IM_RET_OK){
		IM_ERRMSG((IM_STR("pmmdrv_dmmu_disable() failed")));
		return IM_RET_FAILED;
	}

	if(gPwl->dmmu.cbcr[idsx][wx]){
		IM_ASSERT(gPwl->dmmu.pmmcbcr[idsx]);
		if(pmmdrv_dmmu_disable(gPwl->dmmu.pmmcbcr[idsx]) != IM_RET_OK){
			IM_ERRMSG((IM_STR("pmmdrv_dmmu_disable() failed")));
			return IM_RET_FAILED;
		}
	}

	return IM_RET_OK;
}

IM_RET idspwl_pad_set(IM_INT32 index, IM_BOOL high)
{
	IM_INFOMSG((IM_STR("%s(index=%d, high=%d)"), IM_STR(_IM_FUNC_), index, high));
	imapx_pad_set_mode(1, 1, index);
	imapx_pad_set_dir(0, 1, index);
	imapx_pad_set_outdat(high?1:0, 1, index);

	return IM_RET_OK;
}

IM_RET idspwl_suspend(void)
{
    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
#ifdef IDS0_CLK_DOWN
    gIds0clkdiv = 0;
    val_0 = 0;
    val_1 = 0;
#endif
    return IM_RET_OK;
}

IM_RET idspwl_resume(void)
{
    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
    return IM_RET_OK;
}


static int gHDMILastState = IM_FALSE;
void (*maliCallback) (int) = IM_NULL;                    
void hdmi_register_mali_callback(void *callback)         
{                                                        
    IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_))); 
    maliCallback = callback;                     
}
EXPORT_SYMBOL(hdmi_register_mali_callback);              

IM_RET idspwl_set_bus_config(IM_INT32 busEffic, IM_INT32 ids0DevType, IM_INT32 ids0WorkMode, 
    IM_INT32 ids1DevType, IM_INT32 ids1WorkMode)
{
    IM_UINT32 emif;
    IM_INFOMSG((IM_STR("%s() busEffic=(%d),ids0DevType(%d),ids0WorkMode(%d),ids1DevType(%d),ids1WorkMode(%d)))")
                ,IM_STR(_IM_FUNC_),busEffic,ids0DevType,ids0WorkMode,ids1DevType,ids1WorkMode));

    // IDS0 port set
    if (ids0WorkMode == IDSLIB_WORKMODE_IDLE){
        emif = IDS0_EXIT;
    }
    else {
        emif = IDS0_ENTRY;
    }
    emif_control(emif);
    
    // ids1 port set
    if (ids1WorkMode == IDSLIB_WORKMODE_IDLE){
        emif = IDS1_EXIT;
    }
    else {
        emif = IDS1_ENTRY;
    }
    emif_control(emif);

    if ((ids1WorkMode!=IDSLIB_WORKMODE_IDLE) && (ids1DevType==IDS_DISPDEV_TYPE_HDMI)){
        // limit gpu freq
        if (maliCallback != IM_NULL && gHDMILastState == IM_FALSE){
            maliCallback(IM_TRUE);
			gHDMILastState = IM_TRUE;
        }   
    }
    else{
        if (maliCallback != IM_NULL && gHDMILastState == IM_TRUE){
			maliCallback(IM_FALSE);
			gHDMILastState = IM_FALSE;
        }
    }
    return IM_RET_OK;
}



