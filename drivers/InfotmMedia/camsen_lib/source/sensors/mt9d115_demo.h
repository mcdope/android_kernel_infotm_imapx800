/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: mt9d115_demo.h
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/08/23: first commit.
--
------------------------------------------------------------------------------*/

#define MT9D115_I2C_ADDR               0x3C


#define REG_ADDR_STEP 2
#define REG_DATA_STEP 2
#define REG_STEP 			(REG_ADDR_STEP+REG_DATA_STEP)

static struct mt9d115_regval_list {
	IM_UINT8 reg_num[REG_ADDR_STEP]; //0:high_8bits, 1:low_8bits
	IM_UINT8 value[REG_DATA_STEP]; //0:high_8bits, 1:low_8bits
};

static struct mt9d115_regval_list  mt9d115_uxga_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_svga_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_vga_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_qvga_regs[] = {
};


static struct mt9d115_regval_list  mt9d115_softreset_regs[] = {
	

};

static struct mt9d115_regval_list  mt9d115_init_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_sepia_regs[] = {
};

static struct mt9d115_regval_list  mt9d115_bluish_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_greenish_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_reddish_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_yellowish_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_bandw_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_negative_regs[] = {

};

static struct mt9d115_regval_list  mt9d115_normal_regs[] = {

};


static struct mt9d115_regval_list  mt9d115_auto_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_sunny_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_cloudy_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_office_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_home_regs[] = {
	
};

static struct mt9d115_regval_list  mt9d115_sunset_regs[] = {
};

static struct mt9d115_regval_list  mt9d115_night_regs[] = {
};

static	struct mt9d115_regval_list  mt9d115_50hz_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_60hz_regs[] = {};


static	struct mt9d115_regval_list  mt9d115_ev_neg4_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_neg3_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_neg2_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_neg1_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_zero_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_pos1_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_pos2_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_pos3_regs[] = {};

static	struct mt9d115_regval_list  mt9d115_ev_pos4_regs[] = {};

