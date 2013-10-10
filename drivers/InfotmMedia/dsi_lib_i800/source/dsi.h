/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of dsi library internal configuration
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __MIPIDSI_H_
#define __MIPIDSI_H_

#define MIPI_DSI_BASE_ADDR (0x22040000)
#define MIPIDSI_INT_ID	   (81)

#define DSI_HOST_BASE_ADDRESS       MIPI_DSI_BASE_ADDR
// sam : could be ingored. DSI_HOST_BASE_ADDRESS + 0x54UL
#define DPHY_BASE_ADDRESS           DSI_HOST_BASE_ADDRESS

#define MIPIDSI_VERSION				0x00
#define MIPIDSI_PWR_UP				0x04
#define MIPIDSI_CLKMGR_CFG			0x08
#define MIPIDSI_DPI_CFG				0x0C
#define MIPIDSI_DBI_CFG				0x10
#define MIPIDSI_DBI_CMDSIZE			0x14
#define MIPIDSI_PCKHDL_CFG			0x18
#define MIPIDSI_VID_MODE_CFG		0x1C
#define MIPIDSI_VID_PKT_CFG			0x20
#define MIPIDSI_CMD_MODE_CFG		0x24
#define MIPIDSI_TMR_LINE_CFG		0x28
#define MIPIDSI_VTIMING_CFG			0x2C
#define MIPIDSI_PHY_TMR_CFG			0x30
#define MIPIDSI_GEN_HDR				0x34
#define MIPIDSI_GEN_PLD_DATA		0x38
#define MIPIDSI_CMD_PKT_STATUS		0x3C
#define MIPIDSI_TO_CNT_CFG			0x40
#define MIPIDSI_ERROR_ST0			0x44
#define MIPIDSI_ERROR_ST1			0x48
#define MIPIDSI_ERROR_MSK0			0x4C
#define MIPIDSI_ERROR_MSK1			0x50
#define MIPIDSI_PHY_RSTZ			0x54
#define MIPIDSI_PHY_IF_CFG			0x58
#define MIPIDSI_PHY_IF_CTRL			0x5C
#define MIPIDSI_PHY_STATUS			0x60
#define MIPIDSI_PHY_TST_CRTL0		0x64
#define MIPIDSI_PHY_TST_CRTL1		0x68

#endif // __MIPIDSI_H_


