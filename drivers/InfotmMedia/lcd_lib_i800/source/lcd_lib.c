/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of lcd library 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.2	 Sam@2012/6/08 :  clock calc and generate the division 
** 1.0.6 	 Sam@2012/6/15 :  module enable before register set
**
*****************************************************************************/

#include <InfotmMedia.h>

#include <ids_pwl.h>
#include <lcd_lib.h>
#include <lcd.h>

#define DBGINFO		0
#define DBGWARN 	1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"LCDLIB_I:"
#define WARNHEAD	"LCDLIB_W:"
#define ERRHEAD		"LCDLIB_E:"
#define TIPHEAD		"LCDLIB_T:"

IM_RET lcdlib_init(IM_INT32 idsx, lcdc_config_t *cfg)
{
	IM_UINT32 val;
	IM_UINT32 clkfreq, resClk, clkdiv=1;
	IM_UINT32 module = (idsx == 0)?MODULE_IDS0 : MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("idsx=%d width =%d, height=%d, fpsx1000=%d "),idsx, cfg->width,cfg->height,cfg->fpsx1000));
	IM_INFOMSG((IM_STR("VSPW=%d, VBPD=%d, VFPD=%d, HSPW=%d,HBPD=%d, HFPD=%d, -"),cfg->VSPW,cfg->VBPD,cfg->VFPD,cfg->HSPW,cfg->HBPD,cfg->HFPD));

	if(cfg != NULL){

		val = ((cfg->VBPD-1) << LCDCON2_VBPD) | ((cfg->VFPD-1) << LCDCON2_VFPD);
		idspwl_write_reg(module, LCDCON2, val);

		val = ((cfg->VSPW-1) << LCDCON3_VSPW) | ((cfg->HSPW-1) << LCDCON3_HSPW);
		idspwl_write_reg(module, LCDCON3, val);

		val = ((cfg->HBPD-1) << LCDCON4_HBPD) | ((cfg->HFPD-1) << LCDCON4_HFPD);
		idspwl_write_reg(module , LCDCON4, val);

		val = (cfg->SignalDataMapping << LCDCON5_RGBORDER) | (0 << LCDCON5_DSPTYPE) |
			(cfg->signalVclkInverse << LCDCON5_INVVCLK) | (cfg->signalHsyncInverse << LCDCON5_INVVLINE) |
			(cfg->signalVsyncInverse << LCDCON5_INVVFRAME) | (cfg->signalDataInverse << LCDCON5_INVVD) |
			(cfg->signalVdenInverse << LCDCON5_INVVDEN) | (cfg->signalPwrenInverse << LCDCON5_INVPWREN) |
			(cfg->signalPwrenEnable << LCDCON5_PWREN);
		idspwl_write_reg(module , LCDCON5, val);

		val = ((cfg->height -1) << LCDCON6_LINEVAL) | ((cfg->width -1) << LCDCON6_HOZVAL);
		idspwl_write_reg(module, LCDCON6, val);

        clkdiv = lcdlib_get_real_clk_fpsx1000(idsx, cfg, &clkfreq, &cfg->fpsx1000);

		val = ((clkdiv-1) << LCDCON1_CLKVAL) | (0 << LCDCON1_VMMODE) | (3 << LCDCON1_PNRMODE) |
			(12<<LCDCON1_STNBPP) | (0<<LCDCON1_ENVID);
		idspwl_write_reg(module, LCDCON1, val);

		// request idsx-ods clock : must be larger than idsx-eitf. But in case four windows opened 
		// at the same time, error here !!!!!
		val = clkfreq * 2;
		idspwl_module_request_frequency(idsx ? MODULE_IDS1: MODULE_IDS0, val, &resClk);
		IM_TIPMSG((IM_STR(" ids%d-ods request clk = %d , actual clk = %d - "),idsx, val, resClk));

        // disable ids freq auto-change
		idspwl_write_regbit(module, LCDVCLKFSR, 24, 4, 0x0F);
	}
	else{
		// make cfg has its contents in the lcd_api .
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET lcdlib_open(IM_INT32 idsx)
{
	IM_UINT32 val;
	IM_UINT32 module = (idsx == 0)?MODULE_IDS0 : MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s(),idsx=%d"), IM_STR(_IM_FUNC_),idsx));

	val = 1;
	idspwl_write_regbit(module , LCDCON1 ,0 , 1, val);

	return IM_RET_OK;
}

IM_RET lcdlib_close(IM_INT32 idsx)
{
	IM_UINT32 val;
	IM_UINT32 module = (idsx == 0)?MODULE_IDS0 : MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	val = 0;
	idspwl_write_regbit(module , LCDCON1 ,0 , 1, val);
	return IM_RET_OK;
}

IM_RET lcdlib_deinit(IM_INT32 idsx)
{
	return IM_RET_OK;
}

IM_UINT32 lcdlib_get_real_clk_fpsx1000(IM_UINT32 idsx, lcdc_config_t *cfg, IM_UINT32 *rClk, IM_UINT32 *rfpsx1000)
{
	IM_UINT32 pixTotal, h_total, v_total;
	IM_UINT32 clkfreq, resClk, cnt, clkdiv=1;
	IM_UINT32 val, rqstclk, clkdiff;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	IM_INFOMSG((IM_STR("idsx=%d width =%d, height=%d\n"),idsx, cfg->width,cfg->height));

	h_total = cfg->width + cfg->HSPW + cfg->HBPD + cfg->HFPD;
	v_total = cfg->height + cfg->VSPW + cfg->VBPD + cfg->VFPD;
    pixTotal = h_total * v_total;
	clkfreq = pixTotal / 1000 * cfg->fpsx1000;
	IM_INFOMSG((IM_STR("h_total=%d,v_total=%d,clkfreq=%d()"), h_total,v_total,clkfreq));

    // For IDS0_CLK_DOWN use.
#if 0
    if (clkfreq < 40000000){
        cnt = 3;
    }else {
        cnt = 2;
    }
#else
    cnt = 1;
#endif
    clkdiff = 20000000; // 20M clk
    for(; cnt <= 7; cnt++)
    {
        rqstclk = cnt * clkfreq;
        if (rqstclk > 350000000) // restriction : 350M 
            break;
        if(idspwl_module_request_frequency(idsx ? MODULE_LCD1 : MODULE_LCD0, cnt * clkfreq, &resClk) != IM_RET_OK){
            IM_ERRMSG((IM_STR("idspwl_module_request_frequency failed : %d - "),cnt*clkfreq));
            return IM_RET_FAILED;
        }
        resClk /= cnt;
        val = (resClk > clkfreq) ? (resClk - clkfreq) : (clkfreq - resClk);
        if (val < clkdiff) {
            clkdiff = val;
            clkdiv = cnt;
        }
    }
    idspwl_module_request_frequency(idsx ? MODULE_LCD1 : MODULE_LCD0, clkdiv * clkfreq, &resClk);

    if (rClk != IM_NULL)
        *rClk = resClk / clkdiv;
    if (rfpsx1000 != IM_NULL)
        *rfpsx1000 = (resClk/clkdiv + pixTotal/2) / pixTotal * 1000; // round value

    return clkdiv;
}


