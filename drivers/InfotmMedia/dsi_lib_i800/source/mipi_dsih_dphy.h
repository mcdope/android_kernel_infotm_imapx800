/*
 * @file mipi_dsih_dphy.h
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#ifndef MIPI_DSIH_DPHY_H_
#define MIPI_DSIH_DPHY_H_

#include "mipi_dsih_local.h"

#define R_DSI_HOST_PHY_RSTZ     	0x54UL
#define R_DSI_HOST_PHY_IF_CFG   	0x58UL
#define R_DSI_HOST_PHY_IF_CTRL  	0x5CUL
#define R_DSI_HOST_PHY_STATUS  	   	0x60UL
#define R_DSI_HOST_PHY_TST_CRTL0 	0x64UL
#define R_DSI_HOST_PHY_TST_CRTL1  	0x68UL

/* obligatory functions - code can be changed for different phys*/
dsih_error_t mipi_dsih_dphy_open(dphy_t * phy);
dsih_error_t mipi_dsih_dphy_configure(dphy_t * phy, IM_UINT8 no_of_lanes, IM_UINT32 output_freq);

void mipi_dsih_dphy_clock_en(dphy_t * instance, IM_INT32 en);
void mipi_dsih_dphy_reset(dphy_t * instance, IM_INT32 reset);
void mipi_dsih_dphy_shutdown(dphy_t * instance, IM_INT32 powerup);

void mipi_dsih_dphy_stop_wait_time(dphy_t * instance, IM_UINT8 no_of_byte_cycles);
void mipi_dsih_dphy_no_of_lanes(dphy_t * instance, IM_UINT8 no_of_lanes);

IM_UINT8 mipi_dsih_dphy_get_no_of_lanes(dphy_t * instance);

void mipi_dsih_dphy_if_control(dphy_t * instance, IM_UINT8 mask);
IM_UINT32 mipi_dsih_dphy_get_if_control(dphy_t * instance, IM_UINT8 mask);
IM_UINT32 mipi_dsih_dphy_status(dphy_t * instance, IM_UINT16 mask);
/* end of obligatory functions*/
void mipi_dsih_dphy_test_clock(dphy_t * instance, IM_INT32 value);
void mipi_dsih_dphy_test_clear(dphy_t * instance, IM_INT32 value);
void mipi_dsih_dphy_test_en(dphy_t * instance, IM_UINT8 on_falling_edge);
IM_UINT8 mipi_dsih_dphy_test_data_out(dphy_t * instance);
void mipi_dsih_dphy_test_data_in(dphy_t * instance, IM_UINT8 test_data);

void mipi_dsih_dphy_write(dphy_t * instance, IM_UINT8 address, IM_UINT8 * data, IM_UINT8 data_length);

void mipi_dsih_dphy_write_word(dphy_t * instance, IM_UINT32 reg_address, IM_UINT32 data);
void mipi_dsih_dphy_write_part(dphy_t * instance, IM_UINT32 reg_address, IM_UINT32 data, IM_UINT8 shift, IM_UINT8 width);
IM_UINT32 mipi_dsih_dphy_read_word(dphy_t * instance, IM_UINT32 reg_address);
IM_UINT32 mipi_dsih_dphy_read_part(dphy_t * instance, IM_UINT32 reg_address, IM_UINT8 shift, IM_UINT8 width);
#endif /* MIPI_DSIH_DPHY_H_ */
