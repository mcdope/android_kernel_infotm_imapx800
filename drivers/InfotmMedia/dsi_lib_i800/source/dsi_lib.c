/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of dsi library 
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

#include <InfotmMedia.h>
#include <ids_pwl.h>

#include <mipi_dsih_local.h>
#include <mipi_dsih_api.h>
#include <mipi_dsih_dphy.h>

#include <dsi_lib.h>
#include <dsi.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"DSILIB_I:"
#define WARNHEAD	"DSILIB_W:"
#define ERRHEAD		"DSILIB_E:"
#define TIPHEAD		"DSILIB_T:"

enum {
	MIPIDSI_TYPE_DPI = 0x01,
	MIPIDSI_TYPE_DBI = 0x02
};

static IM_UINT32 gDsiType = 0;
static dsih_dpi_video_t video_dpi = {0};
static dsih_dbi_video_t video_dbi = {0};
static dphy_t phy;
static dsih_ctrl_t gInstance;

static IM_RET dsilib_configure_params(void);
static IM_UINT32 io_bus_read32(IM_UINT32 base_address, IM_UINT32  offset);
static void io_bus_write32(IM_UINT32 base_address, IM_UINT32 offset, IM_UINT32  data);
static IM_RET event_register(void);

static void ms_sleep(IM_UINT32 time)
{
	if (time){
		msleep(time);
	}
}

IM_RET dsilib_init(void **inst)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(*inst != IM_NULL){
		return IM_RET_FAILED;
	}
	memset((void*)&phy, 0, sizeof(dphy_t));
    phy.address = DPHY_BASE_ADDRESS;
    phy.reference_freq = 27000; /*KHz*/
    phy.status = NOT_INITIALIZED;
    phy.core_read_function = io_bus_read32;
    phy.core_write_function = io_bus_write32;
    phy.log_error = 0;
    phy.log_info = 0;

	gInstance.address = DSI_HOST_BASE_ADDRESS;
	gInstance.phy_instance = phy;
	gInstance.max_lanes = 4; /* DWC MIPI D-PHY Bidir TSMC40LP has only 2 lanes */
    // The following two polarity must be coordinate to the the MIPI dsi configuration register 0x21E24020
	gInstance.color_mode_polarity = 1;
	gInstance.shut_down_polarity = 1;
	gInstance.max_hs_to_lp_cycles = 100;
	gInstance.max_lp_to_hs_cycles = 40;
	gInstance.max_bta_cycles = 2; // 4095; rtl
	gInstance.status = NOT_INITIALIZED;
	gInstance.core_read_function = io_bus_read32;
	gInstance.core_write_function = io_bus_write32;
	gInstance.log_error = 0;
	gInstance.log_info = 0;

	gDsiType = 0;
	*inst = (void *)&gInstance;
	return IM_RET_OK;
}

IM_RET dsilib_deinit(void *inst)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	inst = IM_NULL;
	gDsiType = 0;
	return IM_RET_OK;
}

IM_RET dsilib_set_dpi_config(void *inst, mipidsi_dpi_t *dpicfg) 
{
	IM_RET ret;
    IM_UINT32 clkfreq;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    clkfreq = (dpicfg->width + dpicfg->hspw + dpicfg->hbpd + dpicfg->hfpd) 
              * (dpicfg->height + dpicfg->vspw + dpicfg->vbpd + dpicfg->vfpd) 
                / 1000 * dpicfg->fpsx1000;
	IM_INFOMSG((IM_STR("clkfreq=%d"), clkfreq));

	gDsiType = MIPIDSI_TYPE_DPI;
	memset((void*)&video_dpi, 0, sizeof(dsih_dpi_video_t));

    // lanes No prefer or clk prefer? 
    // Now make 4 lanes as default, so we could get the smallest byte_clock
    video_dpi.no_of_lanes = 4;
    video_dpi.virtual_channel = dpicfg->virtual_channel ;
    video_dpi.video_mode = VIDEO_BURST_WITH_SYNC_PULSES ;
    video_dpi.receive_ack_packets = 1 ;
    video_dpi.pixel_clock = clkfreq / 1000; // K
    video_dpi.byte_clock = video_dpi.pixel_clock; // byte_clock * 8 * 4(lanes) > pixel_clock * 24(bits);
    video_dpi.color_coding = COLOR_CODE_24BIT;
    video_dpi.is_18_loosely = 0;
    video_dpi.data_en_polarity = dpicfg->data_en_pol ;
    video_dpi.h_polarity = dpicfg->hsync_pol ;
    video_dpi.h_active_pixels = dpicfg->width ;
    video_dpi.h_sync_pixels = dpicfg->hspw ;
    video_dpi.h_back_porch_pixels = dpicfg->hbpd ;
    video_dpi.h_total_pixels = dpicfg->hspw + dpicfg->hbpd + dpicfg->hfpd + dpicfg->width ;
    video_dpi.v_polarity = dpicfg->vsync_pol ;
    video_dpi.v_active_lines = dpicfg->height ;
    video_dpi.v_sync_lines = dpicfg->vspw ;
    video_dpi.v_back_porch_lines = dpicfg->vbpd ;
    video_dpi.v_total_lines = dpicfg->vspw + dpicfg->vbpd + dpicfg->vfpd + dpicfg->height ;

	ret = dsilib_configure_params();
	if (ret != IM_RET_OK){
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET dsilib_set_dbi_config(void *inst, mipidsi_dbi_t *dbicfg)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	gDsiType = MIPIDSI_TYPE_DBI;
	memset((void*)&video_dbi, 0, sizeof(dsih_dbi_video_t ));
    //
    // TODO : configure the video_dbi params.
    // 

	ret = dsilib_configure_params();
	if (ret != IM_RET_OK){
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET dsilib_open(void *inst)
{
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) inst;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	mipi_dsih_dphy_if_control(&dsi_inst->phy_instance,1);
	dsilib_enter_video_mode();

	return IM_RET_OK;
}

IM_RET dsilib_close(void *inst)
{
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) inst;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	mipi_dsih_shutdown_controller(dsi_inst,1);
	// also OK
	mipi_dsih_close(dsi_inst);
	return IM_RET_OK;
}

IM_UINT32 io_bus_read32(IM_UINT32 base_address, IM_UINT32 offset)
{
	IM_UINT32 val;

	idspwl_read_reg(MODULE_DSI, offset, &val);

	return val;
}
void io_bus_write32(IM_UINT32 base_address, IM_UINT32 offset, IM_UINT32 data)
{
	idspwl_write_reg(MODULE_DSI, offset, data);
}

// sam : configure the video params.
static IM_RET dsilib_configure_params(void)
{
	IM_UINT32 cIdle, i;
	dsih_error_t ret = 0;
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) &gInstance;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    ret = mipi_dsih_open(dsi_inst);
	if (ret != DSI_OK){
		IM_ERRMSG((IM_STR(" mipi_dsih_open failed ret=%d "),ret));
		return IM_RET_FAILED;
	}

    /*
	if(event_register() != IM_RET_OK){
		IM_ERRMSG((IM_STR(" event_register failed")));
		return IM_RET_FAILED;
	}*/

	if(gDsiType == MIPIDSI_TYPE_DPI){
		ret = mipi_dsih_dpi_video(dsi_inst, &video_dpi);
		if (ret != DSI_OK){
            IM_ERRMSG((IM_STR(" mipi_dsih_dpi_video failed - ")));
			return IM_RET_FAILED;
	    }	
	}else {
		ret = mipi_dsih_dbi_video(dsi_inst, &video_dbi);
		if(ret != DSI_OK){
            IM_ERRMSG((IM_STR(" mipi_dsih_dbi_video failed - ")));
			return IM_RET_FAILED;
		}
	}

	mipi_dsih_shutdown_controller(dsi_inst,0);
	mipi_dsih_dphy_open(&(gInstance.phy_instance));

	if(gDsiType == MIPIDSI_TYPE_DPI)
	{
		mipi_dsih_dphy_configure(&dsi_inst->phy_instance, video_dpi.no_of_lanes, video_dpi.byte_clock * 8);
	}
	else if(gDsiType == MIPIDSI_TYPE_DBI)
	{
		mipi_dsih_dphy_configure(&dsi_inst->phy_instance, video_dbi.no_of_lanes, video_dbi.byte_clock * 8);
	}
	else
	{
		IM_ERRMSG((IM_STR(" dsi type error , check it !!")));
		return IM_RET_FAILED;
	}

	for(i = 0 ; i < 10000; i ++)
	{
		if ((i % 100) == 0)
		{
			idspwl_read_reg(MODULE_DSI, MIPIDSI_PHY_STATUS , &cIdle);// polling PHY LOCK
			if(cIdle==0x1)	// LOCK
			{
				break;
			}
			mdelay(1);
		}
	}
	idspwl_read_reg(MODULE_DSI, MIPIDSI_PHY_STATUS , &cIdle);
	if (cIdle == 0){
		IM_ERRMSG((IM_STR(" DSI PHY not locked - \n")));
		return IM_RET_FAILED;
	}

	for (i = 0; i < 10000; i ++)
	{
		if ((i % 100) == 0){
			idspwl_read_reg(MODULE_DSI, MIPIDSI_PHY_STATUS , &cIdle);   // polling STOP State of Clock Lane
			if(cIdle==0x4)	// Stop State
			{
				break;
			}
			mdelay(1);
		}
	}
	idspwl_read_reg(MODULE_DSI, MIPIDSI_PHY_STATUS , &cIdle);
	if (cIdle == 0){
		IM_ERRMSG((IM_STR(" DSI Clock Lane not in STOP State - \n")));
		return IM_RET_FAILED;
	}

	return IM_RET_OK;
}

IM_RET dsilib_send_short_packet(IM_UINT32 vc, IM_UINT8 *cmd_buffer, IM_UINT32 length)
{
	dsih_error_t error_code;
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) &gInstance;
	IM_INFOMSG((IM_STR("%s(), vc = %d , length=%d - "),IM_STR(_IM_FUNC_),vc, length));
	
	if (cmd_buffer == IM_NULL || length != 2){
		IM_ERRMSG((IM_STR(" dsi send short packet : params error  ")));
		return IM_RET_FAILED;
	}

	error_code = mipi_dsih_gen_wr_cmd(dsi_inst, vc , 0x05, cmd_buffer, 2);
	if(error_code != DSI_OK) {
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET dsilib_send_long_packet(IM_UINT32 vc, IM_UINT8 *cmd_buffer, IM_UINT32 length)
{
	IM_UINT32 i, j, cnt, cIdle,temp;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if (length <= 2){
		IM_ERRMSG((IM_STR(" dsi send long packet : params error -")));
		return IM_RET_FAILED;
	}

	for(i=0; i < length; i += j){
		temp = 0x00;
		for (j = 0; j < 4 && (j + i) < length ; j ++){
			temp |= cmd_buffer[i+j] << (j * 8);
		}
		for(cnt = 0; cnt < 10; cnt ++){ 
			idspwl_read_reg(MODULE_DSI, MIPIDSI_CMD_PKT_STATUS, &cIdle);
			cIdle &= 0x08;
			if(!cIdle) {
				idspwl_write_reg(MODULE_DSI,MIPIDSI_GEN_PLD_DATA , temp);
				break;
			}
		}
	}
	for(i = 0; i <= 10;i ++){
		idspwl_read_reg(MODULE_DSI, MIPIDSI_CMD_PKT_STATUS, &cIdle);
		cIdle &= 0x02;
		if(!cIdle){
			temp = (length << 8) | (vc << 6) | 0x39;
			idspwl_write_reg(MODULE_DSI, MIPIDSI_GEN_HDR, temp);
			break;
		}
	}
	return IM_RET_OK;
}

void dsilib_enter_video_mode(void)
{
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) &gInstance;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ms_sleep(1);
	mipi_dsih_video_mode(dsi_inst,1);
}
void dsilib_enter_cmd_mode(void)
{
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) &gInstance;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ms_sleep(1);
	mipi_dsih_cmd_mode(dsi_inst,1);	
}

void dsi_event_cb(void *param)
{
	//IM_UINT8 event = *(IM_UINT8 *)(param);
	IM_ERRMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	// 
	// TO_DO 
	// 
}

IM_RET event_register(void)
{
	dsih_ctrl_t *dsi_inst = (dsih_ctrl_t*) &gInstance;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* register events in the core to be notified */
	if (mipi_dsih_register_event(dsi_inst, DPHY_ESC_ENTRY_ERR, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, DPHY_SYNC_ESC_LP_ERR, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, DPHY_CONTROL_LANE0_ERR, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, DPHY_CONTENTION_LP1_ERR, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, DPHY_CONTENTION_LP0_ERR, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, HS_CONTENTION, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, LP_CONTENTION, dsi_event_cb) != DSI_OK)
	{
		return IM_RET_FAILED;
	}
	if (mipi_dsih_register_event(dsi_inst, RX_ECC_SINGLE_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, RX_ECC_MULTI_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, RX_CRC_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, RX_PKT_SIZE_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, RX_EOPT_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, DPI_PLD_FIFO_FULL_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, GEN_TX_CMD_FIFO_FULL_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, GEN_TX_PLD_FIFO_FULL_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, GEN_TX_PLD_FIFO_EMPTY_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, GEN_RX_PLD_FIFO_EMPTY_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	if (mipi_dsih_register_event(dsi_inst, GEN_RX_PLD_FIFO_FULL_ERR, dsi_event_cb) != DSI_OK)
    {
		return IM_RET_FAILED;
    }
	// 
	// TO_DO  : register the system interrupt for DSI.
	// 
	return IM_RET_OK;
}



