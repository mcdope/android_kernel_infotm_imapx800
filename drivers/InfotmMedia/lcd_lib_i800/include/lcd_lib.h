/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of lcd library 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __LCD_LIB_H__
#define __LCD_LIB_H__

//
// signal data[23:0] mapping.
//
#define DATASIGMAPPING_RGB  0x6		// [5:4]2b'00, [3:2]2b'01, [1:0]2b'10. RGB
#define DATASIGMAPPING_RBG  0x9		// [5:4]2b'00, [3:2]2b'10, [1:0]2b'01. RBG
#define DATASIGMAPPING_GRB  0x12	// [5:4]2b'01, [3:2]2b'00, [1:0]2b'10. GRB
#define DATASIGMAPPING_GBR  0x18	// [5:4]2b'01, [3:2]2b'10, [1:0]2b'00. GBR
#define DATASIGMAPPING_BRG 	0x21	// [5:4]2b'10, [3:2]2b'00, [1:0]2b'01. BRG
#define DATASIGMAPPING_BGR  0x24	// [5:4]2b'10, [3:2]2b'01, [1:0]2b'00. BGR

//
//
//
typedef struct{
	IM_INT32 width;
	IM_INT32 height;
	
	IM_INT32 VSPW;
	IM_INT32 VBPD;
	IM_INT32 VFPD;
	IM_INT32 HSPW;
	IM_INT32 HBPD;
	IM_INT32 HFPD;
	
	IM_INT32 signalPwrenEnable;
	IM_INT32 signalPwrenInverse;
	IM_INT32 signalVclkInverse;
	IM_INT32 signalHsyncInverse;
	IM_INT32 signalVsyncInverse;
	IM_INT32 signalDataInverse;
	IM_INT32 signalVdenInverse;
	
	IM_INT32 SignalDataMapping;	// DATASIGMAPPING_xxx.
	
	IM_UINT32 fpsx1000;

	IM_UINT32 phyWidth;
	IM_UINT32 phyHeight;
}lcdc_config_t;

// params:
// 		idsx, 0--basic ids, 1--extended ids.
// return:
//		IM_RET_xxx.
IM_RET lcdlib_init(IM_INT32 idsx, lcdc_config_t *cfg);
IM_RET lcdlib_open(IM_INT32 idsx);
IM_RET lcdlib_close(IM_INT32 idsx);
IM_RET lcdlib_deinit(IM_INT32 idsx);
IM_UINT32 lcdlib_get_real_clk_fpsx1000(IM_UINT32 idsx, lcdc_config_t *cfg, IM_UINT32 *rClk, IM_UINT32 *rfpsx1000);


#endif	// __LCD_LIB_H__
