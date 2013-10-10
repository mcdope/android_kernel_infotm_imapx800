#include <linux/fb.h>
#include <plat/imapx.h>
#include <asm/io.h>
#include "imapx200_ids.h"
#include <linux/delay.h>
#include "../infotm/imapfb.h"


lcd_timing_param_t ids_timing[10] = 
{
	{0}, /* this row is removed, lcd timing will be provied by lcd driver, apr.20.2011 */

	{1,0,3,12,0,36,1080,4,5,1,127,1920,109,44,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,3,12,0,20,720,5,5,0,127,1280,203,40,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,30,480,9,6,0,60,720,16,62,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,30,480,9,6,0,60,720,16,62,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,39,576,5,5,0,68,720,12,64,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,39,576,5,5,0,68,720,12,64,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1}, 
	{2,0,3,12,0,33,480,10,2,0,48,640,16,96,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},    
	{1,0,3,12,0,23,600,1,4,0,88,800,40,128,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,29,768,3,6,0,160,1024,24,136,0x06,0,0,0,1,1,0,0,0,0,0,0,0,1},
};

tvif_timing_param_t tvif_timing[10] = 
{
	{1,0,0,1,0,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/18,/**/600,/**/10,/**/18,/**/600,/**/10,/**/408,/**/1024,/**/0,134,32,/**/134,4667,/**/134,4667,/**/1023,/**/599},
	{1,0,0,1,1,/**/0,1,0,1,0,0,1,1,0,0,0,/**/0,1,0,0,0,0,
	 /**/41,/**/1080,/**/4,/**/41,/**/1080,/**/4,/**/272,/**/1920,/**/0,84,44,/**/84,8191,/**/84,8191,/**/1919,/**/1079},
	{1,0,0,1,1,/**/0,1,0,1,0,0,1,1,0,0,0,/**/0,1,0,0,0,0,
	 /**/25,/**/720,/**/5,/**/25,/**/720,/**/5,/**/362,/**/1280,/**/0,106,40,/**/106,8191,/**/106,8191,/**/1279,/**/719},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/36,/**/480,/**/9,/**/36,/**/480,/**/9,/**/130,/**/720,/**/0,12,62,/**/12,5147,/**/12,5147,/**/719,/**/479},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/36,/**/480,/**/9,/**/36,/**/480,/**/9,/**/130,/**/720,/**/0,12,62,/**/12,5147,/**/12,5147,/**/719,/**/479},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/44,/**/576,/**/5,/**/44,/**/576,/**/5,/**/136,/**/720,/**/0,8,64,/**/8,4319,/**/8,4319,/**/719,/**/575},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/44,/**/576,/**/5,/**/44,/**/576,/**/5,/**/136,/**/720,/**/0,8,64,/**/8,4319,/**/8,4319,/**/719,/**/575},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/35,/**/480,/**/10,/**/35,/**/480,/**/10,/**/152,/**/640,/**/0,12,96,/**/12,1599,/**/12,1599,/**/639,/**/479},
	{1,0,0,1,1,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/27,/**/600,/**/1,/**/27,/**/600,/**/1,/**/248,/**/800,/**/0,40,128,/**/40,3168,/**/40,3168,/**/799,/**/599},
	{1,0,0,1,2,/**/0,1,0,1,0,0,0,0,0,0,0,/**/0,1,0,0,0,0,
	 /**/35,/**/768,/**/3,/**/35,/**/768,/**/3,/**/312,/**/1024,/**/0,24,136,/**/24,6720,/**/24,6720,/**/1023,/**/767},
};

lds_clk_param_t ids_clk[10] = 
{
	{0x13,0 /* this value is removed, invoke lcd driver interface instead, apr.20.2011 */},
	{0x24,0x0909},
	{0x24,0x1515},
	{0x1A,0x1D1D},
	{0x1A,0x1D1D},
	{0x1A,0x1D1D},
	{0x1A,0x1D1D},
	{0x18,0x1D1D},
	{0x13,0x1515},
	{0x40,0x1d1d},
};

static unsigned int LCDCON_DataBuf[6];
static unsigned int LCDINT_DataBuf[3];
static unsigned int OVLCON0_DataBuf[12];
static unsigned int OVLCON1_DataBuf[12];
static unsigned int OVLCON2_DataBuf[12];
static unsigned int OVLCON3_DataBuf[10];
static unsigned int OVLCOLCON_DataBuf[14];
static unsigned int OVLPAL0_DataBuf[256];
static unsigned int OVLPAL1_DataBuf[256];
static unsigned int OVLPAL2_DataBuf[128];
static unsigned int OVLPAL3_DataBuf[128];

/*only for system UI*/
void osd_excursion_bottom(video_timing timing)
{
	unsigned int lefttopx;
	unsigned int lefttopy;
	unsigned int rightbottomx;
	unsigned int rightbottomy;
	unsigned int excursionx = 0;
	unsigned int excursiony = 0;

	switch(timing)
	{
		case HDMI_1080P:                                 
			excursionx = (1920 - imapfb_fimd.xres)/2;
			excursiony = (1080 - imapfb_fimd.yres) - 20;
			break;                                   

		case HDMI_720P:                                  
			excursionx = (1280- imapfb_fimd.xres)/2; 
			excursiony = (720- imapfb_fimd.yres) - 20;  
			break;                                   

		case HDMI_480P_16_9:                             
		case HDMI_480P_4_3:                              
			excursionx = (720 - imapfb_fimd.xres)/2; 
			excursiony = (480 - imapfb_fimd.yres); 
			break;                                   

		case HDMI_576P_16_9:                             
		case HDMI_576P_4_3:                              
			excursionx = (720 - imapfb_fimd.xres)/2; 
			excursiony = (576 - imapfb_fimd.yres); 
			break;                                   

		case HDMI_640_480:                               
			excursionx = (640- imapfb_fimd.xres)/2;  
			excursiony = (480- imapfb_fimd.yres);  
			break;                                   

		case HDMI_800_600:
			excursionx = (800 - imapfb_fimd.xres)/2;
			excursiony = (600 - imapfb_fimd.yres);
			break;

		case HDMI_1024_768:
			excursionx = (1024 - imapfb_fimd.xres)/2;
			excursiony = (768 - imapfb_fimd.yres);
			break;
		default:
			printk("Not supported by now Maybe later\n");
			break;
	}

	lefttopx = 0;//((__raw_readl(IMAP_OVCW0PCAR) >> 16) & (0x7ff));
	lefttopy = 0;//((__raw_readl(IMAP_OVCW0PCAR) >> 0) & (0x7ff));
	rightbottomx = imapfb_fimd.xres;//((__raw_readl(IMAP_OVCW0PCBR) >> 16) & (0x7ff));
	rightbottomy = imapfb_fimd.yres;//((__raw_readl(IMAP_OVCW0PCBR) >> 0) & (0x7ff));
	__raw_writel(IMAP_OVCWxPCAR_LEFTTOPX(lefttopx + excursionx) | IMAP_OVCWxPCAR_LEFTTOPY(lefttopy + excursiony), IMAP_OVCW0PCAR);
	__raw_writel(IMAP_OVCWxPCBR_RIGHTBOTX(rightbottomx + excursionx - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(rightbottomy + excursiony - 1), IMAP_OVCW0PCBR);

}

void osd_excursion(video_timing timing)
{
	unsigned int lefttopx;
	unsigned int lefttopy;
	unsigned int rightbottomx;
	unsigned int rightbottomy;
	unsigned int excursionx = 0;
	unsigned int excursiony = 0;

	switch(timing)
	{
		case HDMI_1080P:                                 
			excursionx = (1920 - imapfb_fimd.xres)/2;
			excursiony = (1080 - imapfb_fimd.yres)/2;
			break;                                   

		case HDMI_720P:                                  
			excursionx = (1280- imapfb_fimd.xres)/2; 
			excursiony = (720- imapfb_fimd.yres)/2;  
			break;                                   

		case HDMI_480P_16_9:                             
		case HDMI_480P_4_3:                              
			excursionx = (720 - imapfb_fimd.xres)/2; 
			excursiony = (480 - imapfb_fimd.yres)/2; 
			break;                                   

		case HDMI_576P_16_9:                             
		case HDMI_576P_4_3:                              
			excursionx = (720 - imapfb_fimd.xres)/2; 
			excursiony = (576 - imapfb_fimd.yres)/2; 
			break;                                   

		case HDMI_640_480:                               
			excursionx = (640- imapfb_fimd.xres)/2;  
			excursiony = (480- imapfb_fimd.yres)/2;  
			break;                                   

		case HDMI_800_600:
			excursionx = (800 - imapfb_fimd.xres)/2;
			excursiony = (600 - imapfb_fimd.yres)/2;
			break;

		case HDMI_1024_768:
			excursionx = (1024 - imapfb_fimd.xres)/2;
			excursiony = (768 - imapfb_fimd.yres)/2;
			break;
		default:
			printk("Not supported by now Maybe later\n");
			break;
	}

	lefttopx = 0;//((__raw_readl(IMAP_OVCW0PCAR) >> 16) & (0x7ff));
	lefttopy = 0;//((__raw_readl(IMAP_OVCW0PCAR) >> 0) & (0x7ff));
	rightbottomx = imapfb_fimd.xres;//((__raw_readl(IMAP_OVCW0PCBR) >> 16) & (0x7ff));
	rightbottomy = imapfb_fimd.yres;//((__raw_readl(IMAP_OVCW0PCBR) >> 0) & (0x7ff));
	__raw_writel(IMAP_OVCWxPCAR_LEFTTOPX(lefttopx + excursionx) | IMAP_OVCWxPCAR_LEFTTOPY(lefttopy + excursiony), IMAP_OVCW0PCAR);
	__raw_writel(IMAP_OVCWxPCBR_RIGHTBOTX(rightbottomx + excursionx - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(rightbottomy + excursiony - 1), IMAP_OVCW0PCBR);
}



static void lcd_config_clk(video_timing timing)
{
	unsigned int temp;

	/* if lcd, use imapfb_set_clk interface, apr.20.2011 */
	if(timing == LCD)
	{
		imapfb_set_clk();
		return ;
	}

	temp = readl(rDPLL_CFG); 
	temp &=~(1<<31);
	writel(temp,rDPLL_CFG);

	temp = readl(rDPLL_CFG); 
	temp = ids_clk[timing].DPLLCFG;
	writel(temp,rDPLL_CFG);

	//enable dpll	
	temp = readl(rDPLL_CFG); 
	temp |=(1<<31);
	writel(temp,rDPLL_CFG);

	/*wait untill dpll is locked*/
	while(!(readl(rPLL_LOCKED) & 0x2));

	temp = readl(rDIV_CFG4);
	temp = ids_clk[timing].DIVCFG4;
	writel(temp,rDIV_CFG4);
}

static void tvif_config_controller(video_timing timing)
{
	unsigned int reg_temp[16];

	reg_temp[0] = (tvif_timing[timing].Clock_enable << 31) |
			(tvif_timing[timing].TV_PCLK_mode << 11) |
			(tvif_timing[timing].Inv_clock << 9) |
			(tvif_timing[timing].clock_sel << 8) |
			(tvif_timing[timing].Clock_div << 0 );

	reg_temp[1] = (tvif_timing[timing].tvif_enable << 31) |
			(tvif_timing[timing].ITU601_656n << 30) |
			(tvif_timing[timing].Bit16ofITU60 << 29) |
			(tvif_timing[timing].Direct_data << 28 ) |
			(tvif_timing[timing].Bitswap << 18 ) |
			(tvif_timing[timing].Data_order << 16) |
			(tvif_timing[timing].Inv_vsync << 13 ) |
			(tvif_timing[timing].Inv_hsync << 12 ) |
			(tvif_timing[timing].Inv_href << 11 ) |
			(tvif_timing[timing].Inv_field << 10) |
			(tvif_timing[timing].Begin_with_EAV << 0);

	reg_temp[2] = (tvif_timing[timing].Matrix_mode << 31 ) |
			(tvif_timing[timing].Passby << 30) |
			(tvif_timing[timing].Inv_MSB_in << 29)|
			(tvif_timing[timing].Inv_MSB_out << 28) |
			(tvif_timing[timing].Matrix_oft_b << 8 ) |
			(tvif_timing[timing].Matrix_oft_a << 0);

	reg_temp[3] = tvif_timing[timing].UBA1_LEN;

	reg_temp[4] = tvif_timing[timing].UNBA_LEN;

	reg_temp[5] = tvif_timing[timing].UNBA2_LEN;

	reg_temp[6] = tvif_timing[timing].LBA1_LEN;

	reg_temp[7] = tvif_timing[timing].LNBA_LEN;

	reg_temp[8] = tvif_timing[timing].LBA2_LEN;

	reg_temp[9] = tvif_timing[timing].BLANK_LEN;

	reg_temp[10] = tvif_timing[timing].VIDEO_LEN;

	reg_temp[11] = (tvif_timing[timing].Hsync_VB1_ctrl << 30)|
			(tvif_timing[timing].Hsync_delay << 16) |
			(tvif_timing[timing].Hsync_extend);

	reg_temp[12] = (tvif_timing[timing].Vsync_delay_upper << 16) |
			(tvif_timing[timing].Vsync_extend_upper);

	reg_temp[13] = (tvif_timing[timing].Vsync_delay_lower << 16) |
			(tvif_timing[timing].Vsync_extend_lower);

	reg_temp[14] = tvif_timing[timing].DISP_XSIZE;

	reg_temp[15] = tvif_timing[timing].DISP_YSIZE;	

	__raw_writel(reg_temp[0], IMAP_TVCCR);
	__raw_writel(reg_temp[1], IMAP_TVICR);
	__raw_writel(reg_temp[2], IMAP_TVCMCR);
	__raw_writel(reg_temp[3], IMAP_TVUBA1);
	__raw_writel(reg_temp[4], IMAP_TVUNBA);
	__raw_writel(reg_temp[5], IMAP_TVUBA2);
	__raw_writel(reg_temp[6], IMAP_TVLBA1);
	__raw_writel(reg_temp[7], IMAP_TVLNBA);
	__raw_writel(reg_temp[8], IMAP_TVLBA2);
	__raw_writel(reg_temp[9], IMAP_TVBLEN);
	__raw_writel(reg_temp[10], IMAP_TVVLEN);
	__raw_writel(reg_temp[11], IMAP_TVHSCR);
	__raw_writel(reg_temp[12], IMAP_TVVSHCR);
	__raw_writel(reg_temp[13], IMAP_TVVSLCR);
	__raw_writel(reg_temp[14], IMAP_TVXSIZE);
	__raw_writel(reg_temp[15], IMAP_TVYSIZE);
}

static void lcd_config_controller(video_timing timing)
{	
	unsigned int reg_temp[5];

	if(timing == LCD)
	{
		/* if LCD , use imapfb_fimd configurations, apr.20.2011 */
		writel(imapfb_fimd.lcdcon1, IMAP_LCDCON1); 
		writel(imapfb_fimd.lcdcon2, IMAP_LCDCON2); 
		writel(imapfb_fimd.lcdcon3, IMAP_LCDCON3); 
		writel(imapfb_fimd.lcdcon4, IMAP_LCDCON4); 
		writel(imapfb_fimd.lcdcon5, IMAP_LCDCON5); 

		return ;
	}

	reg_temp[0] = (ids_timing[timing].VCLK <<	8) |		
				(ids_timing[timing].EACH_FRAME << 7) |		
				(ids_timing[timing].LCD_PANNEL << 5) |		
				(ids_timing[timing].BPP_MODE << 1) |
				(ids_timing[timing].LCD_OUTPUT);

	reg_temp[1] = ((ids_timing[timing].VBPD -1) << 24) |
				(((ids_timing[timing].VACTIVE -1) & ~0x400) <<14) |
				((ids_timing[timing].VFPD -1 ) << 6) |
				((ids_timing[timing].VSPW -1));

	reg_temp[2] = (ids_timing[timing].VACTIVE_HIGHBIT <<31) |
				((ids_timing[timing].HBPD -1) << 19) |
				((ids_timing[timing].HACTIVE -1) << 8) |
				((ids_timing[timing].HFPD -1));

	reg_temp[3] = ((ids_timing[timing].HSPW -1));

	reg_temp[4] = ((ids_timing[timing].COLOR_MODE) << 24)|
				((ids_timing[timing].BPP24BL) << 12)|
				((ids_timing[timing].FRM565) << 11) |
				((ids_timing[timing].INVVCLK) << 10)|
				((ids_timing[timing].INVVLINE) <<9 )|
				((ids_timing[timing].INVVFRAME) <<8)|
				((ids_timing[timing].INVVD) <<7) |
				((ids_timing[timing].INVVDEN) << 6) |
				((ids_timing[timing].INVPWREN) << 5)|
				((ids_timing[timing].INVENDLINE) << 4)|
				((ids_timing[timing].PWREN) << 3) |
				((ids_timing[timing].ENLEND) <<2 ) |
				((ids_timing[timing].BSWP) << 1) |
				((ids_timing[timing].HWSWP));
				
	writel(reg_temp[0],IMAP_LCDCON1);
	writel(reg_temp[1],IMAP_LCDCON2);
	writel(reg_temp[2],IMAP_LCDCON3);
	writel(reg_temp[3],IMAP_LCDCON4);
	writel(reg_temp[4],IMAP_LCDCON5);
}

void lcd_change_timing(video_timing timing, uint8_t tv_IF, uint8_t open_tvif)
{
	unsigned int temp;
	unsigned char temp_reg[4];
	int i;

/*close lcd screen*/
//	if(timing != LCD)
//	  imapbl_lowlevel_blctrl(0);

	/*Save IDS register value before software reset*/
	/*LCD controller*/
	LCDCON_DataBuf[0] = __raw_readl(IMAP_LCDCON1);
	LCDCON_DataBuf[1] = __raw_readl(IMAP_LCDCON2);
	LCDCON_DataBuf[2] = __raw_readl(IMAP_LCDCON3);
	LCDCON_DataBuf[3] = __raw_readl(IMAP_LCDCON4);
	LCDCON_DataBuf[4] = __raw_readl(IMAP_LCDCON5);
	LCDCON_DataBuf[5] = __raw_readl(IMAP_LCDVCLKFSR);

/*Interrupt */	
	LCDINT_DataBuf[0] = __raw_readl(IMAP_IDSINTPND);
	LCDINT_DataBuf[1] = __raw_readl(IMAP_IDSSRCPND);
	LCDINT_DataBuf[2] = __raw_readl(IMAP_IDSINTMSK);

/*Overlay controller 0*/	
	OVLCON0_DataBuf[0] = __raw_readl(IMAP_OVCDCR);
	OVLCON0_DataBuf[1] = __raw_readl(IMAP_OVCPCR);	
	OVLCON0_DataBuf[2] = __raw_readl(IMAP_OVCBKCOLOR);
	OVLCON0_DataBuf[3] = __raw_readl(IMAP_OVCW0CR);
	OVLCON0_DataBuf[4] = __raw_readl(IMAP_OVCW0PCAR);
	OVLCON0_DataBuf[5] = __raw_readl(IMAP_OVCW0PCBR);
	OVLCON0_DataBuf[6] = __raw_readl(IMAP_OVCW0B0SAR);
	OVLCON0_DataBuf[7] = __raw_readl(IMAP_OVCW0B1SAR);
	OVLCON0_DataBuf[8] = __raw_readl(IMAP_OVCW0VSSR);
	OVLCON0_DataBuf[9] = __raw_readl(IMAP_OVCW0CMR);
	OVLCON0_DataBuf[10] = __raw_readl(IMAP_OVCW0B2SAR);
	OVLCON0_DataBuf[11] = __raw_readl(IMAP_OVCW0B3SAR);

/*Overlay controller 1*/	
	OVLCON1_DataBuf[0] = __raw_readl(IMAP_OVCW1CR);
	OVLCON1_DataBuf[1] = __raw_readl(IMAP_OVCW1PCAR);
	OVLCON1_DataBuf[2] = __raw_readl(IMAP_OVCW1PCBR);
	OVLCON1_DataBuf[3] = __raw_readl(IMAP_OVCW1PCCR);
	OVLCON1_DataBuf[4] = __raw_readl(IMAP_OVCW1B0SAR);
	OVLCON1_DataBuf[5] = __raw_readl(IMAP_OVCW1B1SAR);
	OVLCON1_DataBuf[6] = __raw_readl(IMAP_OVCW1VSSR);
	OVLCON1_DataBuf[7] = __raw_readl(IMAP_OVCW1CKCR);
	OVLCON1_DataBuf[8] = __raw_readl(IMAP_OVCW1CKR);
	OVLCON1_DataBuf[9] = __raw_readl(IMAP_OVCW1CMR);
	OVLCON1_DataBuf[10] = __raw_readl(IMAP_OVCW1B2SAR);
	OVLCON1_DataBuf[11] = __raw_readl(IMAP_OVCW1B3SAR);

/*Overlay controller 2*/	
	OVLCON2_DataBuf[0] = __raw_readl(IMAP_OVCW2CR);
	OVLCON2_DataBuf[1] = __raw_readl(IMAP_OVCW2PCAR);
	OVLCON2_DataBuf[2] = __raw_readl(IMAP_OVCW2PCBR);
	OVLCON2_DataBuf[3] = __raw_readl(IMAP_OVCW2PCCR);
	OVLCON2_DataBuf[4] = __raw_readl(IMAP_OVCW2B0SAR);
	OVLCON2_DataBuf[5] = __raw_readl(IMAP_OVCW2B1SAR);
	OVLCON2_DataBuf[6] = __raw_readl(IMAP_OVCW2VSSR);
	OVLCON2_DataBuf[7] = __raw_readl(IMAP_OVCW2CKCR);
	OVLCON2_DataBuf[8] = __raw_readl(IMAP_OVCW2CKR);
	OVLCON2_DataBuf[9] = __raw_readl(IMAP_OVCW2CMR);
	OVLCON2_DataBuf[10] = __raw_readl(IMAP_OVCW2B2SAR);
	OVLCON2_DataBuf[11] = __raw_readl(IMAP_OVCW2B3SAR);

/*Overlay controller 3*/	
	OVLCON3_DataBuf[0] = __raw_readl(IMAP_OVCW3CR);
	OVLCON3_DataBuf[1] = __raw_readl(IMAP_OVCW3PCAR);
	OVLCON3_DataBuf[2] = __raw_readl(IMAP_OVCW3PCBR);
	OVLCON3_DataBuf[3] = __raw_readl(IMAP_OVCW3PCCR);
	OVLCON3_DataBuf[4] = __raw_readl(IMAP_OVCW3BSAR);
	OVLCON3_DataBuf[5] = __raw_readl(IMAP_OVCW3VSSR);
	OVLCON3_DataBuf[6] = __raw_readl(IMAP_OVCW3CKCR);
	OVLCON3_DataBuf[7] = __raw_readl(IMAP_OVCW3CKR);
	OVLCON3_DataBuf[8] = __raw_readl(IMAP_OVCW3CMR);
	OVLCON3_DataBuf[9] = __raw_readl(IMAP_OVCW3SABSAR);

/*Overlay color controller*/	
	OVLCOLCON_DataBuf[0] = __raw_readl(IMAP_OVCBRB0SAR);
	OVLCOLCON_DataBuf[1] = __raw_readl(IMAP_OVCBRB1SAR);
	OVLCOLCON_DataBuf[2] = __raw_readl(IMAP_OVCOEF11);
	OVLCOLCON_DataBuf[3] = __raw_readl(IMAP_OVCOEF12);
	OVLCOLCON_DataBuf[4] = __raw_readl(IMAP_OVCOEF13);
	OVLCOLCON_DataBuf[5] = __raw_readl(IMAP_OVCOEF21);
	OVLCOLCON_DataBuf[6] = __raw_readl(IMAP_OVCOEF22);
	OVLCOLCON_DataBuf[7] = __raw_readl(IMAP_OVCOEF23);
	OVLCOLCON_DataBuf[8] = __raw_readl(IMAP_OVCOEF31);
	OVLCOLCON_DataBuf[9] = __raw_readl(IMAP_OVCOEF32);
	OVLCOLCON_DataBuf[10] = __raw_readl(IMAP_OVCOEF33);
	OVLCOLCON_DataBuf[11] = __raw_readl(IMAP_OVCOMC);
	OVLCOLCON_DataBuf[12] = __raw_readl(IMAP_OVCBRB2SAR);
	OVLCOLCON_DataBuf[13] = __raw_readl(IMAP_OVCBRB3SAR);

/*Overlay PAL0 */
	for(i=0;i<256;i++)	
		OVLPAL0_DataBuf[i] = __raw_readl(IMAP_OVCW0PAL+i*0x4);

/*Overlay PAL1 */
	for(i=0;i<256;i++)	
		OVLPAL1_DataBuf[i] = __raw_readl(IMAP_OVCW1PAL+i*0x4);

/*Overlay PAL2 */
	for(i=0;i<128;i++)	
		OVLPAL2_DataBuf[i] = __raw_readl(IMAP_OVCW2PAL+i*0x4);

/*Overlay PAL3 */
	for(i=0;i<256;i++)	
		OVLPAL3_DataBuf[i] = __raw_readl(IMAP_OVCW3PAL+i*0x4);

/*Save lcd and osd open status*/
	temp_reg[0] = (OVLCON0_DataBuf[3] & 0x1);
	temp_reg[1]= (OVLCON1_DataBuf[0] & 0x1);
	temp_reg[2]= (OVLCON2_DataBuf[0] & 0x1);
	temp_reg[3]= (OVLCON3_DataBuf[0] & 0x1);

/*LDS module software reset*/
	temp = __raw_readl(rAHBP_RST);
	temp |= 0x1<<6;
	__raw_writel(temp, rAHBP_RST);

	msleep(10);

	temp = __raw_readl(rAHBP_RST);
	temp &= ~(0x1<<6);
	__raw_writel(temp, rAHBP_RST);
	
/*Config LCD clock*/
	lcd_config_clk(timing);

/*write value back to register*/
	__raw_writel(LCDINT_DataBuf[2], IMAP_IDSINTMSK);

	__raw_writel(OVLCON0_DataBuf[0], IMAP_OVCDCR);
	__raw_writel(OVLCON0_DataBuf[1], IMAP_OVCPCR);
	__raw_writel(OVLCON0_DataBuf[2], IMAP_OVCBKCOLOR);
	__raw_writel((OVLCON0_DataBuf[3] & ~0x1), IMAP_OVCW0CR);
	__raw_writel(OVLCON0_DataBuf[4], IMAP_OVCW0PCAR);
	__raw_writel(OVLCON0_DataBuf[5], IMAP_OVCW0PCBR);
	__raw_writel(OVLCON0_DataBuf[6], IMAP_OVCW0B0SAR);
	__raw_writel(OVLCON0_DataBuf[7], IMAP_OVCW0B1SAR);
	__raw_writel(OVLCON0_DataBuf[8], IMAP_OVCW0VSSR);
	__raw_writel(OVLCON0_DataBuf[9], IMAP_OVCW0CMR);
	__raw_writel(OVLCON0_DataBuf[10], IMAP_OVCW0B2SAR);
	__raw_writel(OVLCON0_DataBuf[11], IMAP_OVCW0B3SAR);
	
	__raw_writel((OVLCON1_DataBuf[0] & ~0x1), IMAP_OVCW1CR);
	__raw_writel(OVLCON1_DataBuf[1], IMAP_OVCW1PCAR);
	__raw_writel(OVLCON1_DataBuf[2], IMAP_OVCW1PCBR);
	__raw_writel(OVLCON1_DataBuf[3], IMAP_OVCW1PCCR);
	__raw_writel(OVLCON1_DataBuf[4], IMAP_OVCW1B0SAR);
	__raw_writel(OVLCON1_DataBuf[5], IMAP_OVCW1B1SAR);
	__raw_writel(OVLCON1_DataBuf[6], IMAP_OVCW1VSSR);
	__raw_writel(OVLCON1_DataBuf[7], IMAP_OVCW1CKCR);
	__raw_writel(OVLCON1_DataBuf[8], IMAP_OVCW1CKR);
	__raw_writel(OVLCON1_DataBuf[9], IMAP_OVCW1CMR);
	__raw_writel(OVLCON1_DataBuf[10], IMAP_OVCW1B2SAR);
	__raw_writel(OVLCON1_DataBuf[11], IMAP_OVCW1B3SAR);
	
	__raw_writel((OVLCON2_DataBuf[0] & ~0x1), IMAP_OVCW2CR);
	__raw_writel(OVLCON2_DataBuf[1], IMAP_OVCW2PCAR);
	__raw_writel(OVLCON2_DataBuf[2], IMAP_OVCW2PCBR);
	__raw_writel(OVLCON2_DataBuf[3], IMAP_OVCW2PCCR);
	__raw_writel(OVLCON2_DataBuf[4], IMAP_OVCW2B0SAR);
	__raw_writel(OVLCON2_DataBuf[5], IMAP_OVCW2B1SAR);
	__raw_writel(OVLCON2_DataBuf[6], IMAP_OVCW2VSSR);
	__raw_writel(OVLCON2_DataBuf[7], IMAP_OVCW2CKCR);
	__raw_writel(OVLCON2_DataBuf[8], IMAP_OVCW2CKR);
	__raw_writel(OVLCON2_DataBuf[9], IMAP_OVCW2CMR);
	__raw_writel(OVLCON2_DataBuf[10], IMAP_OVCW2B2SAR);
	__raw_writel(OVLCON2_DataBuf[11], IMAP_OVCW2B3SAR);

	__raw_writel((OVLCON3_DataBuf[0] & ~0x1), IMAP_OVCW3CR);
	__raw_writel(OVLCON3_DataBuf[1], IMAP_OVCW3PCAR);
	__raw_writel(OVLCON3_DataBuf[2], IMAP_OVCW3PCBR);
	__raw_writel(OVLCON3_DataBuf[3], IMAP_OVCW3PCCR);
	__raw_writel(OVLCON3_DataBuf[4], IMAP_OVCW3BSAR);
	__raw_writel(OVLCON3_DataBuf[5], IMAP_OVCW3VSSR);
	__raw_writel(OVLCON3_DataBuf[6], IMAP_OVCW3CKCR);
	__raw_writel(OVLCON3_DataBuf[7], IMAP_OVCW3CKR);
	__raw_writel(OVLCON3_DataBuf[8], IMAP_OVCW3CMR);
	__raw_writel(OVLCON3_DataBuf[9], IMAP_OVCW3SABSAR);

	__raw_writel(OVLCOLCON_DataBuf[0], IMAP_OVCBRB0SAR);
	__raw_writel(OVLCOLCON_DataBuf[1], IMAP_OVCBRB1SAR);
	__raw_writel(OVLCOLCON_DataBuf[2], IMAP_OVCOEF11);
	__raw_writel(OVLCOLCON_DataBuf[3], IMAP_OVCOEF12);
	__raw_writel(OVLCOLCON_DataBuf[4], IMAP_OVCOEF13);
	__raw_writel(OVLCOLCON_DataBuf[5], IMAP_OVCOEF21);
	__raw_writel(OVLCOLCON_DataBuf[6], IMAP_OVCOEF22);
	__raw_writel(OVLCOLCON_DataBuf[7], IMAP_OVCOEF23);
	__raw_writel(OVLCOLCON_DataBuf[8], IMAP_OVCOEF31);
	__raw_writel(OVLCOLCON_DataBuf[9], IMAP_OVCOEF32);
	__raw_writel(OVLCOLCON_DataBuf[10], IMAP_OVCOEF33);
	__raw_writel(OVLCOLCON_DataBuf[11], IMAP_OVCOMC);
	__raw_writel(OVLCOLCON_DataBuf[12], IMAP_OVCBRB2SAR);
	__raw_writel(OVLCOLCON_DataBuf[13], IMAP_OVCBRB3SAR);

	for(i=0;i<256;i++)
		__raw_writel(OVLPAL0_DataBuf[i], IMAP_OVCW0PAL + 0x4*i);
	
	for(i=0;i<256;i++)
		__raw_writel(OVLPAL1_DataBuf[i], IMAP_OVCW1PAL + 0x4*i);
	
	for(i=0;i<128;i++)
		__raw_writel(OVLPAL2_DataBuf[i], IMAP_OVCW2PAL + 0x4*i);
	
	for(i=0;i<128;i++)
		__raw_writel(OVLPAL3_DataBuf[i], IMAP_OVCW3PAL + 0x4*i);

/*change osd window smaller than Hactive and Vactive back to system resolution if timing 12*/

	if(timing == LCD || (imapfb_fimd.xres == 800 && timing == HDMI_800_600) || (imapfb_fimd.xres == 1024 && timing == HDMI_1024_768))
	{
		/* if LCD use imapfb_fimd configurations, apr.20.2011 */
		__raw_writel(IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0), IMAP_OVCW0PCAR);
		__raw_writel(IMAP_OVCWxPCBR_RIGHTBOTX(imapfb_fimd.xres - 1) |
		   IMAP_OVCWxPCBR_RIGHTBOTY(imapfb_fimd.yres - 1), IMAP_OVCW0PCBR);

	}
	else
	{
		if(ids_timing[timing].HACTIVE < imapfb_fimd.xres)
		{	
			__raw_writel(IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0), IMAP_OVCW0PCAR);
			__raw_writel(IMAP_OVCWxPCBR_RIGHTBOTX(ids_timing[timing].HACTIVE -1) | IMAP_OVCWxPCBR_RIGHTBOTY((__raw_readl(IMAP_OVCW0PCBR) & 0x7ff ) -1), IMAP_OVCW0PCBR);
		}

		if(ids_timing[timing].VACTIVE < imapfb_fimd.yres)
		{	
			__raw_writel(IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0), IMAP_OVCW0PCAR);
			__raw_writel(IMAP_OVCWxPCBR_RIGHTBOTX(((__raw_readl(IMAP_OVCW0PCBR) & (0x7ff<<16)) >> 16) -1) | IMAP_OVCWxPCBR_RIGHTBOTY(ids_timing[timing].VACTIVE -1), IMAP_OVCW0PCBR);
		}
	}

/*change timing*/

	lcd_config_controller(timing);
	if(tv_IF == 1)
	{
		if(timing == LCD)
		{
			__raw_writel((__raw_readl(IMAP_LCDCON5) & ~(0x3<<11)), IMAP_LCDCON5);
		}
		else
		{
			__raw_writel((__raw_readl(IMAP_LCDCON5) | (0x2<<11)), IMAP_LCDCON5);
			tvif_config_controller(timing);
		}
	}

/*enable lcd data output*/	
	if(temp_reg[0])
		__raw_writel((__raw_readl(IMAP_OVCW0CR ) |IMAP_OVCWxCR_ENWIN_ENABLE), IMAP_OVCW0CR);

	if(temp_reg[1])
		__raw_writel((__raw_readl(IMAP_OVCW1CR ) |IMAP_OVCWxCR_ENWIN_ENABLE), IMAP_OVCW1CR);

	if(temp_reg[2])
		__raw_writel((__raw_readl(IMAP_OVCW2CR ) |IMAP_OVCWxCR_ENWIN_ENABLE), IMAP_OVCW2CR);

	if(temp_reg[3])
		__raw_writel((__raw_readl(IMAP_OVCW3CR ) |IMAP_OVCWxCR_ENWIN_ENABLE), IMAP_OVCW3CR);

	if(tv_IF == 1)
	{
		if(timing == LCD)
		{
			__raw_writel((__raw_readl(IMAP_TVICR) & ~(0x1<<31)), IMAP_TVICR);
			__raw_writel((__raw_readl(IMAP_OVCDCR) & ~(0x3)), IMAP_OVCDCR);
			__raw_writel((__raw_readl(IMAP_LCDCON1) | IMAP_LCDCON1_ENVID_ENABLE), IMAP_LCDCON1);
		}
		else
		{
			temp = __raw_readl(IMAP_OVCW3CMR);
			__raw_writel(~(0x1<<24), IMAP_OVCW3CMR);
			__raw_writel((__raw_readl(IMAP_OVCDCR) | (0x1<<1)), IMAP_OVCDCR);
			msleep(10);
			if(open_tvif == 1)
				__raw_writel((__raw_readl(IMAP_TVICR) | (0x1<<31)), IMAP_TVICR);

			__raw_writel(temp, IMAP_OVCW3CMR);
			/*
			if((temp & 0x1000000))
			{
				printk("cursor osd color mapped\n");
				__raw_writel(~0x1<<24, IMAP_OVCW3CMR);
				mdelay(10);
				temp = __raw_readl(IMAP_TVICR);
				__raw_writel(0, IMAP_TVICR);
				mdelay(10);
				__raw_writel(temp, IMAP_TVICR);
				mdelay(10);
				__raw_writel(0x1<<24 | ((0x0 & 0xffffff)<<0), IMAP_OVCW3CMR);
			}
			*/
		}
	}
	else
	{
		__raw_writel((__raw_readl(IMAP_LCDCON1) | IMAP_LCDCON1_ENVID_ENABLE), IMAP_LCDCON1);
	}

//	if(timing == LCD || timing == HDMI_800_600 || timing == HDMI_1024_768)
//	{
//		imapbl_lowlevel_blctrl(1);	
//	}
}

void detect_overflow(void)
{
	unsigned int temp, temp1;

	/*detect osd data overflow do recovery*/
	/*RGB mode*/
	temp = __raw_readl(IMAP_LCDVCLKFSR);
	if(temp & 0x1)
	{
		printk("RGB mode data overflow\n");
		temp = __raw_readl(IMAP_OVCW3PCAR);
		temp1 = __raw_readl(IMAP_OVCW3PCBR);

		if(((temp & 0xffff) > imapfb_fimd.yres) || (((temp >> 16) & 0xffff)> imapfb_fimd.xres) || \
				((temp1 & 0xffff) > imapfb_fimd.yres) || (((temp1 >> 16) & 0xffff)> imapfb_fimd.xres))
		{
			printk("overflow cause cursor osd not match\n");
			__raw_writel((imapfb_fimd.yres/2) | ((imapfb_fimd.xres/2) << 16), IMAP_OVCW3PCAR);
			__raw_writel(((imapfb_fimd.yres/2) + 0x17) | ((((imapfb_fimd.xres/2) << 16) + 0x17)), IMAP_OVCW3PCBR);
		}

		temp = __raw_readl(IMAP_TVICR);
		if(temp & 0x80000000)
			        __raw_writel(0, IMAP_TVICR);

		__raw_writel((__raw_readl(IMAP_LCDCON1) & ~IMAP_LCDCON1_ENVID_ENABLE), IMAP_LCDCON1);
		msleep(10);
		__raw_writel(-1, IMAP_LCDVCLKFSR);
		msleep(10);
		__raw_writel((__raw_readl(IMAP_LCDCON1) | IMAP_LCDCON1_ENVID_ENABLE), IMAP_LCDCON1);
	}

	/*TVIF*/
	temp = __raw_readl(IMAP_TVSTSR);
	if(temp & 0x1)
	{
		printk("TVIF mode data overflow\n");
		temp = __raw_readl(IMAP_OVCW3PCAR);
		temp1 = __raw_readl(IMAP_OVCW3PCBR);

		if(((temp & 0xffff) > imapfb_fimd.yres) || (((temp >> 16) & 0xffff)> imapfb_fimd.xres) || \
				((temp1 & 0xffff) > imapfb_fimd.yres) || (((temp1 >> 16) & 0xffff)> imapfb_fimd.xres))
		{
			printk("overflow cause cursor osd not match\n");
			__raw_writel((imapfb_fimd.yres/2) | ((imapfb_fimd.xres/2) << 16), IMAP_OVCW3PCAR);
			__raw_writel(((imapfb_fimd.yres/2) + 0x17) | ((((imapfb_fimd.xres/2) << 16) + 0x17)), IMAP_OVCW3PCBR);
		}

		__raw_writel((__raw_readl(IMAP_TVICR) & ~(0x1<<31)), IMAP_TVICR);
		msleep(1);
		__raw_writel((__raw_readl(IMAP_TVICR) | (0x1<<31)), IMAP_TVICR);
		msleep(1);

		temp = __raw_readl(IMAP_OVCW3CMR);
		__raw_writel(~(0x1<<24), IMAP_OVCW3CMR);

		msleep(1);
		__raw_writel((__raw_readl(IMAP_TVICR) & ~(0x1<<31)), IMAP_TVICR);
		msleep(1);
		__raw_writel((__raw_readl(IMAP_TVICR) | (0x1<<31)), IMAP_TVICR);
		msleep(1);

		__raw_writel(temp, IMAP_OVCW3CMR);
	}
}
