/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: csi_lib.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/09/05: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include "csi_pwl.h"
#include "csi_driver.h"
#include "csi_api.h"
#include "csi_lib.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CSI_LIB_I:"
#define WARNHEAD	"CSI_LIB_W:"
#define ERRHEAD		"CSI_LIB_E:"
#define TIPHEAD		"CSI_LIB_T:"

static inline void csi_delay(int num)
{
    mdelay(num);
}


static void calibration(void)
{

    int tmp;
    int val, cmpout, aux_tripu, aux_tripd, aux, aux_a = 0x0, aux_b = 0x7;

    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
    csipwl_write_reg(0x30, 2);
    csipwl_write_reg(0x34, 0x21);
    csipwl_read_reg(0x34, &tmp);
	csipwl_write_reg(0x34, tmp | (1<<16));//set testen high
    csipwl_write_reg(0x30, 0);
    csipwl_read_reg(0x34, &tmp);
	csipwl_write_reg(0x34, tmp&(~(1<<16)));//set testen low

    val=0x0;
    csipwl_write_reg(0x34, (val<<2) | 0x3);
    csipwl_write_reg(0x30, 2);
    csipwl_read_reg(0x34, &cmpout);
    IM_INFOMSG((IM_STR("testdout=0x%x, val=0x%x, testdin=0x%x"), cmpout, val, (val<<2) | 0x3));
    cmpout = cmpout >> 8;
    cmpout >>= 7;
    aux_tripu = cmpout;
    IM_INFOMSG((IM_STR("cmpout=0x%x"), cmpout));

    for(val=0x1; val <= 0x7; val++){
        csi_delay(1);
        csipwl_write_reg(0x30, 0);
        csipwl_write_reg(0x34, (val<<2) | 0x3);
        csipwl_write_reg(0x30, 2);
        csipwl_read_reg(0x34, &cmpout);
        IM_INFOMSG((IM_STR("testdout1=0x%x, val=0x%x, testdin1=0x%x"), cmpout, val, (val<<2) | 0x3));
        cmpout = cmpout >> 8;
        cmpout >>= 7;
        IM_INFOMSG((IM_STR("cmpout1=0x%x"), cmpout));
        if(cmpout != aux_tripu){
            aux_a = val;
            aux_tripu = cmpout;
            break;
        }
    }

    if(aux_a == 0x0){
        if(cmpout == 0){
            aux = 0x0;
        }else{
            aux = 0x7;
        }
    }
    else
    {
        val=0x7;
        csi_delay(1);
        csipwl_write_reg(0x30, 0);
        csipwl_write_reg(0x34, (val<<2) | 0x3);
        csipwl_write_reg(0x30, 2);
        csipwl_read_reg(0x34, &cmpout);
        IM_INFOMSG((IM_STR("testdout2=0x%x, val=0x%x, testdin2=0x%x"), cmpout, val, (val<<2) | 0x3));
        cmpout = cmpout >> 8;
        cmpout >>= 7;
        aux_tripd = cmpout;
        IM_INFOMSG((IM_STR("cmpout2=0x%x"), cmpout));

        if(aux_tripd != aux_tripu){
            aux = 0x3;    // 3b'011.
            IM_ERRMSG((IM_STR("calibration failed!")));
        }
        else
        {

            for(val=0x6; val >= 0x0; val--){
                csi_delay(1);
                csipwl_write_reg(0x30, 0);
                csipwl_write_reg(0x34, (val<<2) | 0x3);
                csipwl_write_reg(0x30, 2);
                csipwl_read_reg(0x34, &cmpout);
                IM_INFOMSG((IM_STR("testdout3=0x%x, val=0x%x, testdin3=0x%x"), cmpout, val, (val<<2) | 0x3));
                cmpout = cmpout >> 8;
                cmpout >>= 7;
                IM_INFOMSG((IM_STR("cmpout3=0x%x"), cmpout));
                if(cmpout != aux_tripd){
                    aux_b = val;
                    aux_tripd = cmpout;
                    break;
                }
            }

            aux = (aux_a + aux_b + 1) >> 1; // round_max
        }
    }

    IM_INFOMSG((IM_STR("last aux=0x%x"), aux));

    csi_delay(1);
    csipwl_write_reg(0x30, 0);
    csipwl_write_reg(0x34, (aux<<2) | 0x3);
    csipwl_write_reg(0x30, 2);
    csipwl_read_reg(0x34, &cmpout);

    IM_INFOMSG((IM_STR("last testdout=0x%x"), cmpout));
}

static IM_UINT8 testCode(IM_UINT16 code, IM_UINT8 data)
{
    IM_UINT16 tmp;
    IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

    csipwl_write_reg(0x30, 2);//set testclk high
    csipwl_write_reg(0x34, code);//set testcode
    csipwl_read_reg(0x34, &tmp);
	csipwl_write_reg(0x34, tmp | (1<<16));//set testen high
    csipwl_write_reg(0x30, 0);//set testclk low
    csipwl_read_reg(0x34, &tmp);
	csipwl_write_reg(0x34, tmp&(~(1<<16)));//set testen low

    //csipwl_write_reg(0x30, 0);//set testclk is alwready low
    csipwl_write_reg(0x34, data);
    csipwl_write_reg(0x30, 2);//set testclk high
    csipwl_read_reg(0x34, &tmp);//read testout data
    IM_INFOMSG((IM_STR("testdata=0x%x"), tmp));

    return (tmp >> 8) & 0xff;
}

IM_RET csilib_init(IM_INT32 lanes, IM_INT32 freq)
{
    IM_RET ret = IM_RET_OK;
    IM_UINT8 val;
    csi_error_t e = SUCCESS;
    int code;

    IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));

    /* initialise chip and phy */
    e = csi_api_start();
    csi_api_set_on_lanes(lanes);

	//
	csi_core_write_part(DPHY_RSTZ, 0, 0, 1);
	csi_core_write_part(CSI2_RESETN, 0, 0, 1);
	csi_api_shut_down_phy(1);

    //test clear
    csipwl_write_reg(0x30, 1);
    csi_delay(5);
    csipwl_write_reg(0x30, 0);

    //testCode(0x44, 0x28);   //for 288MHz
    csilib_set_frequency(freq);

    calibration();

	csipwl_write_reg(DPHY_RSTZ, 0xffffffff);
	csipwl_write_reg(CSI2_RESETN, 0xffffffff);

	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
    return ret;
}

IM_RET csilib_deinit()
{
    IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));

    csi_api_close();

	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET csilib_open()
{
    IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));

	csi_api_shut_down_phy(0);

	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET csilib_close()
{
    IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));

	csi_api_shut_down_phy(1);

	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET csilib_set_frequency(IM_INT32 freq)
{
    IM_RET ret = IM_RET_OK;
    IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));

    if((freq>=80) && (freq<=90))
    {
        testCode(0x44, 0x00);   //Range: 80-90MHz, default bit rate(90Mbps)
    }
    else if((freq>90) && (freq<=100))
    {
        testCode(0x44, 0x20);   //Range: 90-100MHz
    }
    else if((freq>101) && (freq<=110))
    {
        testCode(0x44, 0x40);   //Range: 100-110MHz
    }
    else if((freq>111) && (freq<=125))
    {
        testCode(0x44, 0x02);   //Range: 110-125MHz
    }
    else if((freq>125) && (freq<=140))
    {
        testCode(0x44, 0x22);   //Range: 125-140MHz
    }
    else if((freq>140) && (freq<=150))
    {
        testCode(0x44, 0x42);   //Range: 140-150MHz
    }
    else if((freq>150) && (freq<=160))
    {
        testCode(0x44, 0x04);   //Range: 150-160MHz
    }
    else if((freq>160) && (freq<=180))
    {
        testCode(0x44, 0x24);   //Range: 160-180MHz
    }
    else if((freq>180) && (freq<=200))
    {
        testCode(0x44, 0x44);   //Range: 180-200MHz
    }
    else if((freq>200) && (freq<=210))
    {
        testCode(0x44, 0x06);   //Range: 200-210MHz
    }
    else if((freq>210) && (freq<=240))
    {
        testCode(0x44, 0x26);   //Range: 210-240MHz
    }
    else if((freq>240) && (freq<=250))
    {
        testCode(0x44, 0x46);   //Range: 240-250MHz
    }
    else if((freq>250) && (freq<=270))
    {
        testCode(0x44, 0x08/*0x48*/);   //Range: 250-270MHz
    }
    else if((freq>270) && (freq<=300))
    {
        testCode(0x44, 0x28);   //Range: 270-300MHz
    }
    else if((freq>300) && (freq<=330))
    {
        testCode(0x44, 0x08);   //Range: 300-330MHz
    }
    else if((freq>330) && (freq<=360))
    {
        testCode(0x44, 0x2a);   //Range: 330-360MHz
    }
    else if((freq>360) && (freq<=400))
    {
        testCode(0x44, 0x4a);   //Range: 360-400MHz
    }
    else if((freq>400) && (freq<=450))
    {
        testCode(0x44, 0x0c);   //Range: 400-450MHz
    }
    else if((freq>450) && (freq<=500))
    {
        testCode(0x44, 0x2c);   //Range: 450-500MHz
    }
    else if((freq>500) && (freq<=550))
    {
        testCode(0x44, 0x0e);   //Range: 500-550MHz
    }
    else if((freq>550) && (freq<=600))
    {
        testCode(0x44, 0x2e);   //Range: 550-600MHz
    }
    else if((freq>600) && (freq<=650))
    {
        testCode(0x44, 0x10);   //Range: 600-650MHz
    }
    else if((freq>650) && (freq<=700))
    {
        testCode(0x44, 0x30);   //Range: 650-700MHz
    }
    else if((freq>700) && (freq<=750))
    {
        testCode(0x44, 0x12);   //Range: 700-750MHz
    }
    else if((freq>750) && (freq<=800))
    {
        testCode(0x44, 0x32);   //Range: 750-800MHz
    }
    else if((freq>800) && (freq<=850))
    {
        testCode(0x44, 0x14);   //Range: 800-850MHz
    }
    else if((freq>850) && (freq<=900))
    {
        testCode(0x44, 0x34);   //Range: 850-900MHz
    }
    else if((freq>900) && (freq<=950))
    {
        testCode(0x44, 0x54);   //Range: 900-950MHz
    }
    else if((freq>950) && (freq<=1000))
    {
        testCode(0x44, 0x74);   //Range: 950-1000MHz
    }
    else
    {
        IM_ERRMSG((IM_STR("%s(), this frequency not support!)"), IM_STR(_IM_FUNC_)));
        return IM_RET_FAILED;
    }

    IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
    return ret;
}

IM_UINT32 csilib_get_state()
{
    return  csi_api_core_read(PHY_STATE);
}
