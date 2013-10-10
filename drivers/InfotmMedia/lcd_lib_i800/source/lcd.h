/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of LCD library internal definitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#define LCDCON1					0x000	//LCD control 1
#define LCDCON2					0x004	//LCD control 2
#define LCDCON3					0x008	//LCD control 3
#define LCDCON4					0x00c	//LCD control 4
#define LCDCON5					0x010	//LCD control 5
#define LCDCON6					0x018	
#define LCDVCLKFSR				0x030	
#define IDSINTPND				0x054	//LCD Interrupt pending
#define IDSSRCPND				0x058	//LCD Interrupt source
#define IDSINTMSK				0x05c	//LCD Interrupt mask

#define LCDCON1_LINECNT			18	// [29:18]
#define LCDCON1_CLKVAL			8	// [17:8]
#define LCDCON1_VMMODE			7	// [7:7]
#define LCDCON1_PNRMODE			5
#define LCDCON1_STNBPP			1
#define LCDCON1_ENVID			0	// [0:0]

#define LCDCON2_VBPD			16	// [26:16]
#define LCDCON2_VFPD			0	// [10:0]

#define LCDCON3_VSPW			16	// [26:16]
#define LCDCON3_HSPW			0	// [10:0]

#define LCDCON4_HBPD 			16	// [26:16]
#define LCDCON4_HFPD			0	// [10:0]

#define LCDCON5_RGBORDER		24	// [29:24]
#define LCDCON5_CONFIGORDER		20	// [22:20] 0->dsi24bpp, 1->dsi16bpp1, 2->dsi16bpp2,3->dsi16bpp3,4->dsi18bpp1,5->dsi18bpp2
#define LCDCON5_VSTATUS			15	// [16:15]
#define LCDCON5_HSTATUS 		13	// [14:13]
#define LCDCON5_DSPTYPE			11	// [12:11]
#define LCDCON5_INVVCLK			10	// [10:10]
#define LCDCON5_INVVLINE		9	// [9:9]
#define LCDCON5_INVVFRAME		8	// [8:8]
#define LCDCON5_INVVD			7	// [7:7]
#define LCDCON5_INVVDEN			6	// [6:6]
#define LCDCON5_INVPWREN		5	// [5:5]
#define LCDCON5_PWREN			3	// [3:3]

#define LCDCON6_LINEVAL 		16	// [26:16]
#define LCDCON6_HOZVAL			0	// [10:0]


