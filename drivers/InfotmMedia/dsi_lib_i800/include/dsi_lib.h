/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of dsi library
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.1.0     Sam@2012/10/22 :  DSI first stable version, support the first product.
**
*****************************************************************************/

#ifndef _DSI_LIB_H_
#define _DSI_LIB_H_

enum MIPIDSI_CONFIG_ID{
	MIPIDSI_CONFIG_NONE = 0,
	MIPIDSI_DPI_16BIT_CONFIG_1 = 0x01,
	MIPIDSI_DPI_16BIT_CONFIG_2,
	MIPIDSI_DPI_16BIT_CONFIG_3,
	MIPIDSI_DPI_18BIT_CONFIG_1,
	MIPIDSI_DPI_18BIT_CONFIG_2,
	MIPIDSI_DPI_24BIT_CONFIG_1,

	MIPIDSI_DBI_8BIT_8BPP = 0x100,
	MIPIDSI_DBI_8BIT_12BPP,
	MIPIDSI_DBI_8BIT_16BPP,
	MIPIDSI_DBI_8BIT_18BPP,
	MIPIDSI_DBI_8BIT_24BPP,
	MIPIDSI_DBI_9BIT_18BPP,
	MIPIDSI_DBI_16BIT_8BPP,
	MIPIDSI_DBI_16BIT_12BPP,
	MIPIDSI_DBI_16BIT_16BPP,
	MIPIDSI_DBI_16BIT_18BPP_OPTION_1,
	MIPIDSI_DBI_16BIT_18BPP_OPTION_2,
	MIPIDSI_DBI_16BIT_24BPP_OPTION_1,
	MIPIDSI_DBI_16BIT_24BPP_OPTION_2,

};

typedef struct {
	IM_UINT32 width;
	IM_UINT32 height;
	IM_UINT32 vspw;
	IM_UINT32 vbpd;
	IM_UINT32 vfpd;
	IM_UINT32 hspw;
	IM_UINT32 hbpd;
	IM_UINT32 hfpd;
	IM_UINT32 vden_pol;
	IM_UINT32 hsync_pol;
	IM_UINT32 vsync_pol;
    IM_UINT32 data_en_pol;
	IM_UINT32 fpsx1000;
    IM_UINT32 virtual_channel;

    IM_UINT32 phyWidth; // mm
    IM_UINT32 phyHeight; // mm
}mipidsi_dpi_t;

typedef struct {
	IM_UINT8 out_dbi_conf;
	IM_INT32 partitioning_en;
	IM_UINT8 lut_size_conf;
	IM_UINT8 in_dbi_conf;
	IM_UINT16 allowed_cmd_size;
	IM_UINT16 wr_cmd_size;
}mipidsi_dbi_t;

IM_RET dsilib_init(void **inst);
IM_RET dsilib_deinit(void *inst);
IM_RET dsilib_set_dpi_config(void *inst,  mipidsi_dpi_t *dpicfg);
IM_RET dsilib_set_dbi_config(void *inst, mipidsi_dbi_t *dbicfg);
IM_RET dsilib_open(void *inst);
IM_RET dsilib_close(void *inst);

void dsilib_enter_cmd_mode(void);
void dsilib_enter_video_mode(void);
IM_RET dsilib_send_short_packet(IM_UINT32 vc, IM_UINT8 *cmd_buffer, IM_UINT32 length);
IM_RET dsilib_send_long_packet(IM_UINT32 vc, IM_UINT8 *cmd_buffer, IM_UINT32 length);

#endif //_DSI_LIB_H_ 

