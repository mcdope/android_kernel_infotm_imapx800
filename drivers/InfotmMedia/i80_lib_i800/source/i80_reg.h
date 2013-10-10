/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of i80 reigster definitions 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __I80_REG_H__
#define __I80_REG_H__

//########################################
//
#define I80IF_BASE_ADDR     (0x3000)
//#define I80IF_BASE_ADDR     (IDS1_BASE_ADDR+0x3000)

//########################################
//
#define I80TRIGCON         (I80IF_BASE_ADDR + 0x00)
#define I80IFCON0          (I80IF_BASE_ADDR + 0x04)
#define I80IFCON1          (I80IF_BASE_ADDR + 0x08)
#define I80CMDCON0         (I80IF_BASE_ADDR + 0x0C)
#define I80CMDCON1         (I80IF_BASE_ADDR + 0x10)
#define I80CMD15           (I80IF_BASE_ADDR + 0x14)
#define I80CMD14           (I80IF_BASE_ADDR + 0x18)
#define I80CMD13           (I80IF_BASE_ADDR + 0x1C)
#define I80CMD12           (I80IF_BASE_ADDR + 0x20)
#define I80CMD11           (I80IF_BASE_ADDR + 0x24)
#define I80CMD10           (I80IF_BASE_ADDR + 0x28)
#define I80CMD09           (I80IF_BASE_ADDR + 0x2C)
#define I80CMD08           (I80IF_BASE_ADDR + 0x30)
#define I80CMD07           (I80IF_BASE_ADDR + 0x34)
#define I80CMD06           (I80IF_BASE_ADDR + 0x38)
#define I80CMD05           (I80IF_BASE_ADDR + 0x3C)
#define I80CMD04           (I80IF_BASE_ADDR + 0x40)
#define I80CMD03           (I80IF_BASE_ADDR + 0x44)
#define I80CMD02           (I80IF_BASE_ADDR + 0x48)
#define I80CMD01           (I80IF_BASE_ADDR + 0x4C)
#define I80CMD00           (I80IF_BASE_ADDR + 0x50)
#define I80MANCON          (I80IF_BASE_ADDR + 0x54)
#define I80MANWDAT         (I80IF_BASE_ADDR + 0x58)
#define I80MANRDAT         (I80IF_BASE_ADDR + 0x5C)

//########################################
//
#define I80IF_TRIGCON_TR_VIDEO_OVER		(4)
#define I80IF_TRIGCON_TR_NORMALCMD		(3)
#define I80IF_TRIGCON_TR_AUTOCMD		(2)
#define I80IF_TRIGCON_TR_DATA			(1)
#define I80IF_TRIGCON_NORMAL_CMD_ST		(0)

//
#define I80IF_IFCON0_RDPOL         		(31)
#define I80IF_IFCON0_WRPOL             	(30)
#define I80IF_IFCON0_RSPOL             	(29)
#define I80IF_IFCON0_CS1POL           	(28)
#define I80IF_IFCON0_CS0POL           	(27)
#define I80IF_IFCON0_PARAM_CS_SETUP    	(22)
#define I80IF_IFCON0_PARAM_WR_SETUP    	(16)
#define I80IF_IFCON0_PARAM_WR_ACTIVE  	(10)
#define I80IF_IFCON0_PARAM_WR_HOLD    	(4)
#define I80IF_IFCON0_NORMAL_CMD_INTR_EN	(3)
#define I80IF_IFCON0_FRAME_END_INTR_EN	(2)
#define I80IF_IFCON0_MAINLCD_CFG      	(1)
#define I80IF_IFCON0_EN               	(0)

//
#define I80IF_DBI_COMPATIBLE_EN		(17)
#define I80IF_IFCON1_PORTTYPE          	(15)	// [16:15]
#define I80IF_IFCON1_PORTDISTRIBUTED   	(13)	// [14:13]
#define I80IF_IFCON1_DATASTYLE         	(10)	// [12:10]
#define I80IF_IFCON1_DATALSBFIRST     	(9)	// [9:9]
#define I80IF_IFCON1_DATAUSELOWPORT   	(8)	// [8:8]
#define I80IF_IFCON1_DISAUTOCMD       	(4)
#define I80IF_IFCON1_AUTOCMDRATE       	(0)

#define I80IF_IFCON1_DATA_FORMAT		(8)		//组合了DATAUSELOWPORT，DATALSBFIRST，DATASTYLE，PORTDISTRIBUTED，PORTTYPE

//
#define I80IF_CMDCON0_CMD15EN			(30)
#define I80IF_CMDCON0_CMD14EN  			(28)
#define I80IF_CMDCON0_CMD13EN          	(26)
#define I80IF_CMDCON0_CMD12EN          	(24)
#define I80IF_CMDCON0_CMD11EN          	(22)
#define I80IF_CMDCON0_CMD10EN          	(20)
#define I80IF_CMDCON0_CMD09EN          	(18)
#define I80IF_CMDCON0_CMD08EN          	(16)
#define I80IF_CMDCON0_CMD07EN          	(14)
#define I80IF_CMDCON0_CMD06EN          	(12)
#define I80IF_CMDCON0_CMD05EN          	(10)
#define I80IF_CMDCON0_CMD04EN          	(8)
#define I80IF_CMDCON0_CMD03EN          	(6)
#define I80IF_CMDCON0_CMD02EN          	(4)
#define I80IF_CMDCON0_CMD01EN          	(2)
#define I80IF_CMDCON0_CMD00EN          	(0)

//
#define I80IF_CMDCON1_CMD15RS			(30)
#define I80IF_CMDCON1_CMD14RS  			(28)
#define I80IF_CMDCON1_CMD13RS          	(26)
#define I80IF_CMDCON1_CMD12RS          	(24)
#define I80IF_CMDCON1_CMD11RS          	(22)
#define I80IF_CMDCON1_CMD10RS          	(20)
#define I80IF_CMDCON1_CMD09RS          	(18)
#define I80IF_CMDCON1_CMD08RS          	(16)
#define I80IF_CMDCON1_CMD07RS          	(14)
#define I80IF_CMDCON1_CMD06RS          	(12)
#define I80IF_CMDCON1_CMD05RS          	(10)
#define I80IF_CMDCON1_CMD04RS          	(8)
#define I80IF_CMDCON1_CMD03RS          	(6)
#define I80IF_CMDCON1_CMD02RS          	(4)
#define I80IF_CMDCON1_CMD01RS          	(2)
#define I80IF_CMDCON1_CMD00RS          	(0)

//
#define I80IF_MANCON_SIG_DOE           	(6)
#define I80IF_MANCON_SIG_RS            	(5)
#define I80IF_MANCON_SIG_CS0           	(4)
#define I80IF_MANCON_SIG_CS1           	(3)
#define I80IF_MANCON_SIG_RD            	(2)
#define I80IF_MANCON_SIG_WR            	(1)
#define I80IF_MANCON_EN            		(0)

//########################################
//


//#############################################################################        
// rIDS_LCDCON1
#define LCDCON1_LINECNT		18	// [29:18]
#define LCDCON1_CLKVAL			8	// [17:8]
#define LCDCON1_VMMODE		7	// [7:7]
#define LCDCON1_PNRMODE		5
#define LCDCON1_STNBPP			1
#define LCDCON1_ENVID			0	// [0:0]

// rIDS_LCDCON2
#define LCDCON2_VBPD			16	// [26:16]
#define LCDCON2_VFPD			0	// [10:0]

// rIDS_LCDCON3
#define LCDCON3_VSPW			16	// [26:16]
#define LCDCON3_HSPW			0	// [10:0]

// rIDS_LCDCON4
#define LCDCON4_HBPD 			16	// [26:16]
#define LCDCON4_HFPD			0	// [10:0]

// rIDS_LCDCON5
#define LCDCON5_RGBORDER		24	// [29:24]
#define LCDCON5_CONFIGORDER		20	// [22:20] 0->dsi24bpp, 1->dsi16bpp1, 2->dsi16bpp2,3->dsi16bpp3,4->dsi18bpp1,5->dsi18bpp2
#define LCDCON5_VSTATUS		15	// [16:15]
#define LCDCON5_HSTATUS 		13	// [14:13]
#define LCDCON5_DSPTYPE		11	// [12:11]
#define LCDCON5_INVVCLK		10	// [10:10]
#define LCDCON5_INVVLINE		9	// [9:9]
#define LCDCON5_INVVFRAME		8	// [8:8]
#define LCDCON5_INVVD			7	// [7:7]
#define LCDCON5_INVVDEN		6	// [6:6]
#define LCDCON5_INVPWREN		5	// [5:5]
#define LCDCON5_PWREN			3	// [3:3]

// rIDS_LCDCON6
#define LCDCON6_LINEVAL 		16	// [26:16]
#define LCDCON6_HOZVAL			0	// [10:0]

// rIDS_LCDVCLKFSR
#define LCDVCLKFSR_CDOWN 		24	// [27:24]
#define LCDVCLKFSR_RFRM_NUM 	16	// [21:16]
#define LCDVCLKFSR_CLKVAL 		4	// [13:4]
#define LCDVCLKFSR_VCLKFAC 	0	// [0:0]

// rIDS_IDSINTPND
#define IDSINTPND_OSDW2INT 	6	// [6:6]
#define IDSINTPND_I80INT 		5	// [5:5]
#define IDSINTPND_OSDERR 		4	// [4:4]
#define IDSINTPND_OSDW1INT 	3	// [3:3]
#define IDSINTPND_OSDW0INT 	2	// [2:2]
#define IDSINTPND_VCLKINT 		1	// [1:1]
#define IDSINTPND_LCDINT 		0	// [0:0]

// rIDS_IDSSRCPND
#define IDSSRCPND_OSDW2INT 	6	// [6:6]
#define IDSSRCPND_I80INT 		5	// [5:5]
#define IDSSRCPND_OSDERR 		4	// [4:4]
#define IDSSRCPND_OSDW1INT 	3	// [3:3]
#define IDSSRCPND_OSDW0INT 	2	// [2:2]
#define IDSSRCPND_VCLKINT 		1	// [1:1]
#define IDSSRCPND_LCDINT 		0	// [0:0]

// rIDS_IDSINTMSK
#define IDSINTMSK_OSDW2INT 	6	// [6:6]
#define IDSINTMSK_I80INT 		5	// [5:5]
#define IDSINTMSK_OSDERR 		4	// [4:4]
#define IDSINTMSK_OSDW1INT 	3	// [3:3]
#define IDSINTMSK_OSDW0INT 	2	// [2:2]
#define IDSINTMSK_VCLKINT 		1	// [1:1]
#define IDSINTMSK_LCDINT 		0	// [0:0]
//========================================================================
// IDS0
//========================================================================
#define LCDCON1					0x000	//LCD control 1
#define LCDCON2					0x004	//LCD control 2
#define LCDCON3					0x008	//LCD control 3
#define LCDCON4					0x00c	//LCD control 4
#define LCDCON5					0x010	//LCD control 5
#define LCDCON6					0x018	

// interrupt offset, "which" parameter.
#define IDS_INTR_WIN2		6
#define IDS_INTR_I80		5
#define IDS_INTR_OSDERR	4
#define IDS_INTR_WIN1		3
#define IDS_INTR_WIN0		2
#define IDS_INTR_VCLK		1
#define IDS_INTR_LCD		0

//########################################
//
void 
i80reg_set_idsxNo(IM_UINT32 idsx);

void i80if_manual_sig_init(void);
void i80if_manual_sig_deinit(void);
void i80if_manual_sig_doe_output(void);
void i80if_manual_sig_doe_input(void );
void i80if_manual_enable(void );
void i80if_manual_disbale(void );
void i80if_manual_sig_rs_low(void);
void i80if_manual_sig_rs_high(void);
void i80if_manual_sig_cs0_low(void);
void i80if_manual_sig_cs0_high(void);
void i80if_manual_sig_cs1_low(void);
void i80if_manual_sig_cs1_high(void);
void i80if_manual_sig_rd_low(void);
void i80if_manual_sig_rd_high(void);
void i80if_manual_sig_wr_low(void);
void i80if_manual_sig_wr_high(void);

// ---------------------------------------------
/* 初始化i80寄存器，并不使能i80模块 */
void
i80reg_init(
		struct_i80if *pI80if    // [IN] 根据该参数配置寄存器
		);

// ---------------------------------------------
/* 使能或禁止i80模块 */
void
i80reg_if_enable(
		IM_UINT32 fEn	// [IN] true--使能, false--禁止
		);

// ---------------------------------------------
/* 返回当前i80模块的状态 */
unsigned int		// 返回值在i80_api.h里定义的
i80reg_get_status(void);

// ---------------------------------------------
/* 配置cmd列表 */
void
i80reg_set_cmd_list(
		struct_i80if_cmd_property *pCmd, // [IN] 命令结构
		IM_INT32 nIndex 	// [IN] 在16个cmd fifo中的索引,0-15
		);

// ---------------------------------------------
/* 清除所有cmd列表 */
void
i80reg_clear_cmd_list(void);

// ---------------------------------------------
/* 使能/禁止normal cmd传送，normal cmd是指只发送cmd list里的命令，不发送数据 */
void
i80reg_transmit_normal_cmd(
		IM_UINT32 fEn	// [IN] true--使能, false--禁止
		);

// ---------------------------------------------
/* 设置CS0或CS1为当前的有效控制CS */
void
i80reg_set_current_lcd(
		enum_i80if_supports_lcd eLcd    // [IN] 当前要使能的lcd
		);

// ---------------------------------------------
/* i80总线manual方式写一个数据，18位的 */
void
i80reg_manual_write_once(
		IM_UINT32 data   // [IN] 要写的数据
		);

// ---------------------------------------------
/* i80总线manual方式读一个数据，18位的 */
void
i80reg_manual_read_once(
		IM_UINT32 *pData // [OUT] 读出数据存放的地址
		);

// ---------------------------------------------
/* 使能或禁止LCD */
void
i80reg_lcd_enable(IM_UINT32 fEn);    // [IN] true--使能, false--禁止

// ---------------------------------------------
/* 设置data format */
void
i80reg_set_data_format(
		enum_i80if_data_format fmt
		);

//########################################

#endif	// __I80_REG_H__

