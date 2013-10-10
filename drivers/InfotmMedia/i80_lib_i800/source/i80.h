/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of I80 library internal definitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/
//lcd
#define LCDCON1					0x000	//LCD control 1
#define LCDCON2					0x004	//LCD control 2
#define LCDCON3					0x008	//LCD control 3
#define LCDCON4					0x00c	//LCD control 4
#define LCDCON5					0x010	//LCD control 5
#define LCDCON6					0x018	
#define LCDVCLKFSR				0x030	
#define IDSINTPND				0x054	//LCD Interrupt pending中断等待
#define IDSSRCPND				0x058	//LCD Interrupt source
#define IDSINTMSK				0x05c	//LCD Interrupt mask   中断屏蔽
//i80
#define I80TRIGCON              0x3000  //I80 Trigger Control
#define I80IFCON0               0x3004  //I80 Interface Control0
#define I80IFCON1               0x3008  //I80 Interface Control1
#define I80CMDCON0              0x300C  //I80 Command Control0
#define I80CMDCON1              0x3010  //I80 Command Control1
#define I80CMD15                0x3014  //I80 Command Instruct 15--0
#define I80CMD14                0x3018
#define I80CMD13                0x301C
#define I80CMD12                0x3020
#define I80CMD11                0x3024
#define I80CMD10                0x3028
#define I80CMD09                0x302C
#define I80CMD08                0x3030
#define I80CMD07                0x3034
#define I80CMD06                0x3038
#define I80CMD05                0x303C
#define I80CMD04                0x3040
#define I80CMD03                0x3044
#define I80CMD02                0x3048
#define I80CMD01                0x304C
#define I80CMD00                0x3050
#define I80MANCON               0x3054  //I80 Manual Control
#define I80MANWDAT              0x3058  //I80 Manual Write Data
#define I80MANRDAT              0x305C  //I80 Manual Read Data

//lcd
#define LCDCON1_LINECNT			         18	// [29:18]
#define LCDCON1_CLKVAL			         8	// [17:8]
#define LCDCON1_VMMODE			         7	// [7:7]
#define LCDCON1_PNRMODE			         5  // [6:5]
#define LCDCON1_STNBPP			         1  // [4:1]
#define LCDCON1_ENVID			         0	// [0:0]

#define LCDCON2_VBPD			         16	// [26:16]
#define LCDCON2_VFPD			         0	// [10:0]

#define LCDCON3_VSPW			         16	// [26:16]
#define LCDCON3_HSPW			         0	// [10:0]

#define LCDCON4_HBPD 			         16	// [26:16]
#define LCDCON4_HFPD			         0	// [10:0]

#define LCDCON5_RGBORDER		         24	// [29:24]
#define LCDCON5_CONFIGORDER		         20	// [22:20] 0->dsi24bpp, 1->dsi16bpp1, 2->dsi16bpp2,3->dsi16bpp3,4->dsi18bpp1,5->dsi18bpp2
#define LCDCON5_VSTATUS			         15	// [16:15]
#define LCDCON5_HSTATUS 		         13	// [14:13]
#define LCDCON5_DSPTYPE			         11	// [12:11]
#define LCDCON5_INVVCLK			         10	// [10:10]
#define LCDCON5_INVVLINE		         9	// [9:9]
#define LCDCON5_INVVFRAME		         8	// [8:8]
#define LCDCON5_INVVD			         7	// [7:7]
#define LCDCON5_INVVDEN			         6	// [6:6]
#define LCDCON5_INVPWREN		         5	// [5:5]
#define LCDCON5_PWREN			         3	// [3:3]

#define LCDCON6_LINEVAL 		         16	// [26:16]
#define LCDCON6_HOZVAL			         0	// [10:0]

#define LCDVCLKFSR_CDOWN 		         24	// [27:24]  If set this field 0xf, The auto frequency conversion will be disabled
#define LCDVCLKFSR_RFRM_NUM 	         16	// [21:16]  Frame number for restore clock divider
#define LCDVCLKFSR_CLKVAL 		         4	// [13:4]   Current VCLK divisor
#define LCDVCLKFSR_VCLKFAC 	             0	// [0:0]    VCLK frequency auto change flag

#define IDSINTPND_OSDW2INT 	             6	// [6:6]    
#define IDSINTPND_I80INT 		         5	// [5:5]   i80 transmission end or control commond sequence end source pending bit
#define IDSINTPND_OSDERR 		         4	// [4:4]
#define IDSINTPND_OSDW1INT 	             3	// [3:3]
#define IDSINTPND_OSDW0INT 	             2	// [2:2]
#define IDSINTPND_VCLKINT 		         1	// [1:1]
#define IDSINTPND_LCDINT 		         0	// [0:0]

#define IDSSRCPND_OSDW2INT 	             6	// [6:6]
#define IDSSRCPND_I80INT 		         5	// [5:5]   i80 transmission end or control commond sequence end source pending bit
#define IDSSRCPND_OSDERR 		         4	// [4:4]
#define IDSSRCPND_OSDW1INT 	             3	// [3:3]
#define IDSSRCPND_OSDW0INT    	         2	// [2:2]
#define IDSSRCPND_VCLKINT 		         1	// [1:1]
#define IDSSRCPND_LCDINT 		         0	// [0:0]

#define IDSINTMSK_OSDW2INTMSK 	         6	// [6:6]
#define IDSINTMSK_I80INTMSK 	         5	// [5:5]
#define IDSINTMSK_OSDERRMSK 	         4	// [4:4]
#define IDSINTMSK_OSDW1INTMSK 	         3	// [3:3]
#define IDSINTMSK_OSDW0INTMSK 	         2	// [2:2]
#define IDSINTMSK_VCLKINTMSK 		     1	// [1:1]
#define IDSINTMSK_LCDINTMSK 		     0	// [0:0]

//i80
#define I80TRIGCON_XFER_VIDEO_OVER		 4  //[4:4]      Video Data transfer over status
#define I80TRIGCON_TNORMALCMD		     3  //[3:3]      Transmitting Normal Command, Status Indicator
#define I80TRIGCON_TAUTOCMD		         2  //[2:2]      Transmitting Auto Command, Status Indicator 
#define I80TRIGCON_TVIDEO			     1  //[1:1]      Transmitting Video Data, Status indicator    4--1为状态寄存器，只读
#define I80TRIGCON_NORMAL_CMD_ST		 0  //[0:0]      Trigger the Normal Command: 1 只发送cmd list里的命令，不发送数据            状态判断
//自动控制
#define I80IFCON0_RDPOL         		 31 //[31:31]    0: RD_n signal is active low  Inverse the polarity of the RD_n (Read) Signal in non-manual mode
#define I80IFCON0_WRPOL             	 30 //[30:30]    0: WR_n signal is active low                              WR_n (Write)
#define I80IFCON0_RSPOL             	 29 //[29:29]    0: RS_n signal is active low                              RS_n (Register Select)
#define I80IFCON0_CS1POL           	     28 //[28:28]    0: CS1_n signal is active low                             CS1_n(Chip Select)
#define I80IFCON0_CS0POL           	     27 //[27:27]    0: CS2_n signal is active low
#define I80IFCON0_LCD_CS_SETUP    	     22 //[26:22]    
#define I80IFCON0_LCD_WR_SETUP    	     16 //[21:16]
#define I80IFCON0_LCD_WR_ACTIVE  	     10 //[15:10]    //[31:10] i80 interface属性，包括信号极性和周期数
#define I80IFCON0_LCD_WR_HOLD    	     4  //[9:4]
#define I80IFCON0_INTMASK	             2  //[3:2]     Interrupt Mask 00: No interrupt 01: Frame over 10: Normal CMD over 11: Frame over and Normal CMD over
#define I80IFCON0_MAINCONFIG      	     1  //[1:1]     Main LCD configuration 0: use CS0_n as main LCD (controlled by I80IF) 设置cs0或cs1为当前有效控制cs
#define I80IFCON0_I80IFEN                0  //[0:0]     LCD I80 Interface Enable
//
#define I80IFCON1_PORTTYPE          	 14	// [15:14]  Data Port Connect Type, 18/16/9/8-Wire
#define I80IFCON1_PORTDISTRIBUTED   	 12	// [13:12]  Video Data Distribute on Data Port
#define I80IFCON1_DATASTYLE         	 10	// [11:10]  Video Data transfer Style on the Data Port. 16/16+2/2+16 in 16-Wire. 8+8/6+6+6/8+8+2/2+8+8 in 8-Wire
#define I80IFCON1_DATALSBFIRST     	     9	// [9:9]    Video Data LSB data is Transferred First
#define I80IFCON1_DATAUSELOWPORT   	     8	// [8:8]    Video Data Use Low Select Data Port
#define I80IFCON1_DISAUTOCMD       	     4  // [4:4]    1：Send video data only in every frame
#define I80IFCON1_AUTO_CMD_RATE          0  // [0:0]    1111 : per 30 Frames

#define I80IFCON1_DATA_FORMAT            8  //[15:8]   including PORTTYPE, DATADISTRIBUTED,DATASTYLE,DATALSBFIRST,DATAUSELOWPORT  i80接口数据格式
//
#define I80CMDCON0_CMD15_EN			     30 //[31:30]   00 : Disable 01 : Normal Command Enable 10 : Auto Command Enable 11 : Normal and Auto Command Enable
#define I80CMDCON0_CMD14_EN  			 28 //[29:28]   命令控制方式 
#define I80CMDCON0_CMD13_EN          	 26 //[27:26]
#define I80CMDCON0_CMD12_EN          	 24 //[25:24]
#define I80CMDCON0_CMD11_EN          	 22 //[23:22]
#define I80CMDCON0_CMD10_EN          	 20 //[21:20]
#define I80CMDCON0_CMD09_EN          	 18 //[19:18]
#define I80CMDCON0_CMD08_EN          	 16 //[17:16]
#define I80CMDCON0_CMD07_EN          	 14 //[15:14]
#define I80CMDCON0_CMD06_EN          	 12 //[13:12]
#define I80CMDCON0_CMD05_EN          	 10 //[11:10]
#define I80CMDCON0_CMD04_EN          	 8  //[9:8]
#define I80CMDCON0_CMD03_EN          	 6  //[7:6]
#define I80CMDCON0_CMD02_EN          	 4  //[5:4]
#define I80CMDCON0_CMD01_EN          	 2  //[3:2]
#define I80CMDCON0_CMD00_EN          	 0  //[1:0]
//
#define I80CMDCON1_CMD15_RS			     30 //[30:30] Command 15 RS control 1:data 0:commmand   控制脚RS（置1为写数据，置0为写命令）
#define I80CMDCON1_CMD14_RS  			 28 //[28:28]
#define I80CMDCON1_CMD13_RS          	 26 //[26:26]
#define I80CMDCON1_CMD12_RS          	 24 //[24:24]
#define I80CMDCON1_CMD11_RS          	 22 //[22:22]
#define I80CMDCON1_CMD10_RS          	 20 //[20:20]
#define I80CMDCON1_CMD09_RS          	 18 //[18:18]
#define I80CMDCON1_CMD08_RS          	 16 //[16:16]
#define I80CMDCON1_CMD07_RS          	 14 //[14:14]
#define I80CMDCON1_CMD06_RS          	 12 //[12:12]
#define I80CMDCON1_CMD05_RS          	 10 //[10:10]
#define I80CMDCON1_CMD04_RS          	 8  //[8:8]
#define I80CMDCON1_CMD03_RS          	 6  //[6:6]
#define I80CMDCON1_CMD02_RS          	 4  //[4:4]
#define I80CMDCON1_CMD01_RS          	 2  //[2:2]
#define I80CMDCON1_CMD00_RS          	 0  //[0:0]
//
#define I80CMD15_LDI_CMD15               0  //[17:0]  LDI command activate with CMDx_RS       数据脚DB【17-0】
#define I80CMD14_LDI_CMD14               0  //[17:0]              controlled by CMDx_en in Normal Command Mode and Auto Command Mode, LDI_CMDx and CMDx_RS are mapped to PAD of D[17: 0] and RS
#define I80CMD13_LDI_CMD13               0  //[17:0]
#define I80CMD12_LDI_CMD12               0  //[17:0]
#define I80CMD11_LDI_CMD11               0  //[17:0]
#define I80CMD10_LDI_CMD10               0  //[17:0]
#define I80CMD9_LDI_CMD9                 0  //[17:0]
#define I80CMD8_LDI_CMD8                 0  //[17:0]
#define I80CMD7_LDI_CMD7                 0  //[17:0]
#define I80CMD6_LDI_CMD6                 0  //[17:0]
#define I80CMD5_LDI_CMD5                 0  //[17:0]
#define I80CMD4_LDI_CMD4                 0  //[17:0]
#define I80CMD3_LDI_CMD3                 0  //[17:0]
#define I80CMD2_LDI_CMD2                 0  //[17:0]
#define I80CMD1_LDI_CMD1                 0  //[17:0]
#define I80CMD0_LDI_CMD0                 0  //[17:0]
//手动控制
#define I80MANCON_MAN_DOE           	 6  //[6:6]  LCD i80 System Interface Data Enable Signal control 0: Disable (input) 1: Enable (output) 控制输入输出 
#define I80MANCON_MAN_RS            	 5  //[5:5]  LCD I80 System Interface nRS Signal control    0: level low 1: level high  信号控制
#define I80MANCON_MAN_CS0           	 4  //[4:4]  LCD I80 System Interface nCS0 (main) Signal control
#define I80MANCON_MAN_CS1           	 3  //[3:3]  LCD I80 System Interface nCS1 (sub) Signal control
#define I80MANCON_MAN_RD            	 2  //[2:2]  LCD I80 System Interface nOE Signal control
#define I80MANCON_MAN_WR            	 1  //[1:1]  LCD I80 System Interface nWE Signal control
#define I80MANCON_MAN_EN            	 0  //[0:0]  LCD I80 System Interface Command Mode Enable  0: Disable 1: Enable (Manual Command Mode)
//
#define I80MANWDAT_SYS_WDAT              0  //[17:0] LCD I80 System Interface Write Data Buffer
//
#define I80MANRDAT_SYS_WDAT              0  //[17:0] LCD I80 System Interface Read Data Buffer

