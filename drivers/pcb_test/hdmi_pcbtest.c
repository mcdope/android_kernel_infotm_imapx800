#include <linux/module.h>   
#include <linux/ioport.h>   
#include <mach/imap-iomap.h>
#include <mach/imap-mac.h>  
#include <asm/io.h>         
#include <linux/gpio.h>     
#include <mach/power-gate.h>
#include <mach/pad.h>       
#include <linux/init.h>     
#include <linux/dma-mapping.h>
#include <asm/delay.h>      
#include <linux/delay.h>    
#include <mach/items.h>    
#include <mach/imap-iis.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/cache.h>
#include <linux/io.h>
#include <linux/poll.h>

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <hdmi_api.h>


#define OVCDCR     0x1000
#define OVCOMC     0x1310
#define OVCOEF11   0x1314
#define OVCOEF12   0x1318
#define OVCOEF13   0x131c
#define OVCOEF21   0x1320
#define OVCOEF22   0x1324
#define OVCOEF23   0x1328
#define OVCOEF31   0x132c
#define OVCOEF32   0x1330
#define OVCOEF33   0x1334
#define OVCW0CR    0x1080
#define OVCW0VSSR  0x108c
#define OVCW0PCAR  0x1084
#define OVCW0PCBR  0x1088
#define OVCW0B0SAR 0x1094

#define OVCDCR_UpdateReg	(11)
#define OVCOMC_ToRGB		(31)
#define OVCOMC_oft_b		(8)
#define OVCOMC_oft_a		(0)
#define OVCWxCR_BPPMODE		(1)
#define OVCWxPCAR_LeftTopX	(0)
#define OVCWxPCAR_LeftTopY	(16)
#define OVCWxPCBR_RightBotX	(0)
#define OVCWxPCBR_RightBotY	(16)
#define OVCWxCR_ENWIN		(0)

#define WIDTH  640
#define HEIGHT 480 
static unsigned int *regIDSVir;
static unsigned int  *memVir;
static unsigned int   memPhy;
static unsigned int   memSize;

static void *gHdmiHandle;
static bool gHDMIConfig = false;
extern IM_INT32 gHdmiLastState;
extern unsigned long hpdConnected;
extern int edidDone;
extern hdmi_video_t gVid_A[3];

int gHdmiPcbTestOn = 0;

static void ids_reg_read(unsigned int addr, unsigned int *val)
{
	*val = readl((IM_UINT32)regIDSVir + addr);
}
static void ids_write_reg(unsigned int addr, unsigned int val)
{
	writel(val, (IM_UINT32)regIDSVir + addr);
}
static void ids_write_regbit(unsigned int addr, unsigned int bit, unsigned int width, unsigned int val)
{
	unsigned int tmp;
	ids_reg_read(addr, &tmp);
	tmp &= ~(((0xFFFFFFFF << (32 - width)) >> (32 - width)) << bit);
	tmp |= val << bit;
	ids_write_reg(addr, tmp);
}

static int device_listener(void *handle, unsigned int evt, void *p)
{
	int i, num;
	ids_display_device_t *devs;
	printk(" hdmi pcb device_listener evt(0x%x) -- \n",evt);
	if (evt == DISPDEV_EVT_CONFIG_UPDATE){
		num = ((dispdev_evt_config_update_t *)p)->num;
		devs = ((dispdev_evt_config_update_t *)p)->devs;
		for (i = 0; i < num ; i++){
			printk("sam : HDMI Video Info(%d) : width(%d), height(%d), fps(%d) \n",i,devs[i].width,devs[i].height,devs[i].fpsx1000/1000);
		}
		gHDMIConfig = true;
	}
	else if (evt == DISPDEV_EVT_BREAK){
		gHDMIConfig = false;
	}
    // change gVid_A[0] to mCode 1, VGA
    gVid_A[0].mCode = 0;                                                                                        
    gVid_A[0].mPixelRepetitionInput = 0;
    gVid_A[0].mPixelClock = 2520;
    gVid_A[0].mInterlaced = 0;
    gVid_A[0].mHActive = 640;
    gVid_A[0].mHBlanking = 160;
    gVid_A[0].mHBorder = 0;
    gVid_A[0].mHImageSize = 4;
    gVid_A[0].mHSyncOffset = 16;
    gVid_A[0].mHSyncPulseWidth = 96;
    gVid_A[0].mHSyncPolarity = 0;
    gVid_A[0].mVActive = 480;
    gVid_A[0].mVBlanking = 45;
    gVid_A[0].mVBorder = 0;
    gVid_A[0].mVImageSize = 3;
    gVid_A[0].mVSyncOffset = 10;
    gVid_A[0].mVSyncPulseWidth = 2;
    gVid_A[0].mVSyncPolarity = 0;
    gVid_A[0].mFpsx1000 = 60000;
	return 0;
}

static int hdmi_hpd_confirm(void)
{
	int ret, cnt;

	gHDMIConfig = false;

	ret = hdmi_init(&gHdmiHandle, 1);	
	if (ret){
		printk("sam : hdmi_init failed -- \n");
		return -1;
	}
	ret = hdmi_register_listener(gHdmiHandle, device_listener);
	if (ret != 0){
		printk("sam : hdmi_register_listener failed - \n");
		return -1;
	}

	for (cnt = 0; cnt < 50 ; cnt ++){
		if(gHDMIConfig == true)
			break;
//		printk(" HDMI delay waiting for Hot Plug Detect.....(%ds) \n",cnt);
		msleep(100);
	}

	if (gHDMIConfig == false){
		printk(" HDMI hpd confirm failed, check it -- \n");
		printk(" hpdConnected = %d - edidDone=%d \n",hpdConnected,edidDone);
		return -1;
	}
	return 0;
}

static int resource_init(void)
{
	int i, j, cnt;
	struct clk *clksrc;

	// map the IDS register 
	regIDSVir = (unsigned int*)ioremap_nocache(0x23000000, 0x7000);
	if (regIDSVir == NULL){
		printk("sam : ioremap IDS1 failed - \n");
		return -1;
	}

	memSize = WIDTH * HEIGHT * 4;
	memVir = (unsigned int*)dma_alloc_writecombine(NULL, memSize, &memPhy, GFP_KERNEL);
	if (memVir == 0){
		printk("sam : alloc mem failed -- \n");
		return -1;
	}

	for (i=0 ; i < HEIGHT; i++){
		for (j=0; j < WIDTH ; j++){
			cnt = (j/10) % 5;
			switch (cnt){
				case 0:
					memVir[i*WIDTH + j] = 0x00FF0000; // red
					break;
				case 1:
					memVir[i*WIDTH + j] = 0x0000FF00; // green
					break;
				case 2:
					memVir[i*WIDTH + j] = 0x000000FF; // blue
					break;
				case 3:
					memVir[i*WIDTH + j] = 0x00000000; // black 
					break;
				case 4:
					memVir[i*WIDTH + j] = 0xFFFFFFFF; // white
					break;
				default:
					break;
			}
		}
	}

	// To enable IDS1 module
	module_power_on(SYSMGR_IDS1_BASE);
	clksrc = clk_get(NULL, "ids1");
	if (clksrc == NULL) {
		printk("clk_get(ids1) failed");
		return -1;
	}
	clk_enable(clksrc);
	clksrc = clk_get(NULL, "ids1-ods");
	if (clksrc == NULL) {
		printk("clk_get(ids1-ods) failed");
		return -1;
	}
	clk_enable(clksrc);
	clksrc = clk_get(NULL, "ids1-eitf");
	if (clksrc == NULL) {
		printk("clk_get(ids1-eitf) failed");
		return -1;
	}
	clk_enable(clksrc);
	module_reset(SYSMGR_IDS1_BASE, 1);

	// IDS1 basic register set.
	ids_write_regbit(OVCDCR, OVCDCR_UpdateReg, 1, 1);
	ids_write_reg(OVCOMC, (1<<OVCOMC_ToRGB) | (16<<OVCOMC_oft_b) | (0<<OVCOMC_oft_a));
	ids_write_reg(OVCOEF11, 298);
	ids_write_reg(OVCOEF12, 0);
	ids_write_reg(OVCOEF13, 409);
	ids_write_reg(OVCOEF21, 298);
	ids_write_reg(OVCOEF22, (IM_UINT32)-100);
	ids_write_reg(OVCOEF23, (IM_UINT32)-208);
	ids_write_reg(OVCOEF31, 298);
	ids_write_reg(OVCOEF32, 516);
	ids_write_reg(OVCOEF33, 0);

	ids_write_regbit(OVCW0CR, OVCWxCR_BPPMODE, 5, 11);
	ids_write_reg(OVCW0VSSR, WIDTH);
	ids_write_reg(OVCW0PCAR, (0 << OVCWxPCAR_LeftTopX) | (0 << OVCWxPCAR_LeftTopY));
	ids_write_reg(OVCW0PCBR, (WIDTH-1)<< OVCWxPCBR_RightBotX | (HEIGHT-1)<< OVCWxPCBR_RightBotY);	
	ids_write_reg(OVCW0B0SAR, memPhy);

	return 0;
}

static int resource_free(void)
{
	struct clk *clksrc;

	if (memVir != NULL){
		dma_free_writecombine(NULL, memSize, memVir, memPhy);
	}
	if (regIDSVir != NULL){
		iounmap(regIDSVir);
	}

	clksrc = clk_get(NULL, "ids1-eitf");
	clk_disable(clksrc);

	clksrc = clk_get(NULL, "ids1-ods");
	clk_disable(clksrc);

	clksrc = clk_get(NULL, "ids1");
	clk_disable(clksrc);

	module_power_down(SYSMGR_IDS1_BASE);
	return 0;
}

static int start_play(void)
{
	ids_display_device_t config;

	ids_write_regbit(OVCW0CR, OVCWxCR_ENWIN, 1, 1);

	memset((void*)&config, 0, sizeof(ids_display_device_t));
	config.type = IDS_DISPDEV_TYPE_HDMI;
	config.id = 0;
	config.width = WIDTH;
	config.height = HEIGHT;
	config.fpsx1000 = 60000;
	if (hdmi_set_video_basic_config(gHdmiHandle, &config, 0) != 0 ){
		printk("sam : hdmi_set_video_basic_config failed - \n");
		return -1;
	}

	if (hdmi_open(gHdmiHandle) != 0){
		printk("sam : hdmi_open failed --- \n");
		return -1;
	}
	return 0;
}

// Default VGA size RGB888 display
int hdmi_pcbtest(void)
{
	int ret = 0;
	printk("##########  sam : hdmi_pcbtest start ########### \n");

	gHdmiPcbTestOn = 1;
	if(resource_init() != 0){
		ret = -1;
		goto release;
	}
	if (hdmi_hpd_confirm() != 0){
		ret = -1;
		goto deinit;
	}
	if (start_play() != 0){
		ret = -1;
		goto out;
	}
	msleep(10000);

out:
	hdmi_close(gHdmiHandle);
	ids_write_regbit(OVCW0CR, OVCWxCR_ENWIN, 1, 1);
deinit:
    gHdmiLastState = 0;
	hdmi_deinit(gHdmiHandle);
release:
	resource_free();
	gHdmiPcbTestOn = 0;
	return ret;
}
