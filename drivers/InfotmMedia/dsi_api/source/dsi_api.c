/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of dsi APIs
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
#include <IM_idsapi.h>
#include <mach/items.h>
#include <linux/regulator/consumer.h>

#include <ids_lib.h>
#include <ids_pwl.h>
#include <lcd_lib.h>
#include <i80_api.h>
#include <i80_lib.h>
#include <dsi_lib.h>
#include <dsi_api.h>
#include <dsi_def.h>
#include <dsi_cfg.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"DSIAPI_I:"
#define WARNHEAD	"DSIAPI_W:"
#define ERRHEAD		"DSIAPI_E:"
#define TIPHEAD		"DSIAPI_T:"

enum{
	NONE  = 0,
	PAD_TRUE = 1,
	PMU_TRUE
};

typedef struct{
	IM_INT32 idsx;
	IM_INT32 mipidsi_type;
	void *DsiInst;
}mipidsi_handle;

static IM_UINT32 gDSIStateOn = IM_FALSE;
static DSI_Priv_conf *gDsiPriv = IM_NULL;

extern idsdrv_resource_t *gIdsdrvResource;
static struct regulator *gIDSRegulator = (struct regulator*)-1;
static IM_INT32 gPadNum;
static IM_INT32 dsiPower = NONE;

static mipidsi_handle gDsiHandle = {0};

extern void imapbl_power_on(void);
extern void imapbl_shut_down(void);

static void dsi_msleep(IM_INT32 ms)
{
    if (ms){
        msleep(ms);
    }
}
static IM_RET get_pad(IM_TCHAR *str)
{
	gPadNum = item_integer(str,1);	
	return IM_RET_OK;
}

static IM_INT32 get_pmu_power(IM_TCHAR *pmustr)
{
	IM_INT32 err;
	IM_TCHAR str[ITEM_MAX_LEN];

	if (gIDSRegulator != (struct regulator*)-1){
		regulator_put(gIDSRegulator);
		gIDSRegulator = (struct regulator*)-1;
	}
	item_string(str, pmustr, 1);
	gIDSRegulator = regulator_get(&gIdsdrvResource->pdev->dev, str);
	if(IS_ERR(gIDSRegulator))
	{
		IM_ERRMSG((IM_STR("%s: get regulator fail"), __func__));
		return IM_RET_FAILED;
	}
	err = regulator_set_voltage(gIDSRegulator, 3300000, 3300000);
	if(err)
	{
		IM_ERRMSG((IM_STR("%s: set regulator fail"), __func__));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

static IM_RET get_config(mipidsi_handle *dsi_handle, IM_TCHAR *str)
{
	IM_INFOMSG((IM_STR("%s() config str(%s)"), IM_STR(_IM_FUNC_),str));

	if (dsi_handle->mipidsi_type != MIPIDSI_CONFIG_NONE){
		IM_INFOMSG((IM_STR(" dsi config has already configured.")));
		return IM_RET_OK;
	}

	dsi_handle->mipidsi_type = MIPIDSI_DPI_24BIT_CONFIG_1; // default config : 24bit
	if (strcmp(str, "DPI_16BIT_CONFIG_1") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_16BIT_CONFIG_1;
	}
	else if (strcmp(str, "DPI_16BIT_CONFIG_2") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_16BIT_CONFIG_2;
	}
	else if (strcmp(str, "DPI_16BIT_CONFIG_3") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_16BIT_CONFIG_3;
	}
	else if (strcmp(str, "DPI_18BIT_CONFIG_1") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_18BIT_CONFIG_1;
	}
	else if (strcmp(str, "DPI_18BIT_CONFIG_2") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_18BIT_CONFIG_2;
	}
	else if (strcmp(str, "DPI_24BIT_CONFIG_1") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DPI_24BIT_CONFIG_1;
	}
	else if (strcmp(str, "DBI_8BIT_8BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_8BIT_8BPP;
	}
	else if (strcmp(str, "DBI_8BIT_12BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_8BIT_12BPP;
	}
	else if (strcmp(str, "DBI_8BIT_16BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_8BIT_16BPP;
	}
	else if (strcmp(str, "DBI_8BIT_18BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_8BIT_18BPP;
	}
	else if (strcmp(str, "DBI_8BIT_24BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_8BIT_24BPP;
	}
	else if (strcmp(str, "DBI_9BIT_18BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_9BIT_18BPP;
	}
	else if (strcmp(str, "DBI_16BIT_8BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_8BPP;
	}
	else if (strcmp(str, "DBI_16BIT_12BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_12BPP;
	}
	else if (strcmp(str, "DBI_16BIT_16BPP") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_16BPP;
	}
	else if (strcmp(str, "DBI_16BIT_18BPP_OPTION_1") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_18BPP_OPTION_1;
	}
	else if (strcmp(str, "DBI_16BIT_18BPP_OPTION_2") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_18BPP_OPTION_2;
	}
	else if (strcmp(str, "DBI_16BIT_24BPP_OPTION_1") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_24BPP_OPTION_1;
	}
	else if (strcmp(str, "DBI_16BIT_24BPP_OPTION_2") == 0){
		dsi_handle->mipidsi_type = MIPIDSI_DBI_16BIT_24BPP_OPTION_2;
	}else{
		IM_ERRMSG((IM_STR(" mipi dsi config error - check it !!!")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET dsi_init(dispdev_handle_t *handle, IM_INT32 idsx)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(idsx != 0){ // DSI is fixed to IDS0.
		IM_ERRMSG((IM_STR(" dsi refered idsx error - check it !!!!")));
		return IM_RET_FAILED;
	}

	ret = dsilib_init(&gDsiHandle.DsiInst);	
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("dsilib_init error")));
		return IM_RET_FAILED;
	}
	gDsiHandle.idsx = 0;
	gDsiHandle.mipidsi_type = MIPIDSI_CONFIG_NONE;

    idspwl_module_enable(MODULE_DSI);
    idspwl_module_reset(MODULE_DSI);

	*handle = (dispdev_handle_t)&gDsiHandle;
	return IM_RET_OK;
}

IM_RET dsi_deinit(dispdev_handle_t handle)
{
	IM_RET ret;
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	ret = dsilib_deinit(dsi_handle->DsiInst);
	if(ret != IM_RET_OK){
		IM_ERRMSG((IM_STR(" dsilib_deinit error")));
		return IM_RET_FAILED;
	}
	if(dsi_handle->mipidsi_type <= MIPIDSI_DPI_24BIT_CONFIG_1){
		lcdlib_deinit(dsi_handle->idsx);
	}else{//i80 switch
		i80lib_deinit(dsi_handle->idsx);
	}

	if (gIDSRegulator != (struct regulator*)-1){
		regulator_put(gIDSRegulator);
		gIDSRegulator = (struct regulator*)-1;
	}

	return IM_RET_OK;
}

IM_RET dsi_set_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix)
{
	IM_RET ret;
	IM_INT32 cnt, index, idsx, locIdsx ;
    mipidsi_dpi_t *dpicfg;
    mipidsi_dbi_t *dbicfg;
	lcdc_config_t lcdcfg;
	i80c_config_t i80cfg;
	IM_UINT8 str[ITEM_MAX_LEN];
	IM_UINT8 config_str[ITEM_MAX_LEN];
	IM_UINT8 power_str[ITEM_MAX_LEN];
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idsx = dsi_handle->idsx; 

    cnt = 0;
    while(strcasecmp(gDSIConf[cnt].idstr, "none")){
        if(config->id == gDSIConf[cnt].devid){
            gDsiPriv = (DSI_Priv_conf*) gDSIConf[cnt].priv;
            break;
        }
        cnt ++;
    }
	if (gDsiPriv == IM_NULL){
		IM_ERRMSG((IM_STR("dsi config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(index = 0; index < DSI_PRIV_BLOCK_NUM; index++){
		if(gDsiPriv->block[index].id == DSI_MANU_RGB)
			break;
	}
	if(index == DSI_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" dsi screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}

	locIdsx = 0;
	if (item_exist("ids.locidsx")){
		locIdsx = item_integer("ids.locidsx", 0);
		IM_ASSERT(locIdsx <= 1);
	}else{
		IM_ERRMSG((IM_STR(" local ids not defined in item, error !! ")));
		return IM_RET_FAILED;
	}

	if (locIdsx == idsx){
		sprintf(config_str, "ids.loc.dev%d.config_str",suffix);
		sprintf(power_str, "ids.loc.dev%d.dsi.power",suffix);
	}else{
		sprintf(config_str, "ids.ext.dev%d.config_str",suffix);
		sprintf(power_str, "ids.ext.dev%d.dsi.power",suffix);
	}

	if (item_exist(config_str)){
		item_string(str, config_str, 0);
		get_config(dsi_handle, str);
	}

	if (item_exist(power_str)){
		item_string(str, power_str, 0);
		if(strncmp(str, "pads",4)==0){
			get_pad(power_str);
			dsiPower = PAD_TRUE;
		}
		else if (strncmp(str, "pmu", 3) == 0){
			ret = get_pmu_power(power_str);
			if (ret != IM_RET_OK){
				return IM_RET_FAILED;
			}
			dsiPower = PMU_TRUE;
		}
		else {
			dsiPower = NONE;
		}
	}

	if(dsi_handle->mipidsi_type <= MIPIDSI_DPI_24BIT_CONFIG_1 ){ // DPI
		dpicfg = (mipidsi_dpi_t *)gDsiPriv->block[index].priv;
		ret = dsilib_set_dpi_config(dsi_handle->DsiInst, dpicfg); 
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("  dsilib set dpi config error -- ")));
			return IM_RET_FAILED;
		}

        lcdlib_close(dsi_handle->idsx);
		memset((void*)&lcdcfg, 0, sizeof(lcdc_config_t));
		lcdcfg.width = dpicfg->width; 
		lcdcfg.height = dpicfg->height;
		lcdcfg.VSPW = dpicfg->vspw;
		lcdcfg.VBPD = dpicfg->vbpd;
		lcdcfg.VFPD = dpicfg->vfpd;
		lcdcfg.HSPW = dpicfg->hspw;
		lcdcfg.HBPD = dpicfg->hbpd;
		lcdcfg.HFPD = dpicfg->hfpd;
		lcdcfg.signalPwrenEnable = 1;
		lcdcfg.signalPwrenInverse = 0;
		lcdcfg.signalVclkInverse = 1;
		lcdcfg.signalHsyncInverse = dpicfg->hsync_pol? 0: 1;
		lcdcfg.signalVsyncInverse = dpicfg->vsync_pol? 0 : 1;
		lcdcfg.signalDataInverse = 0 ;
		lcdcfg.signalVdenInverse = dpicfg->vden_pol? 0 : 1;
		lcdcfg.SignalDataMapping = DATASIGMAPPING_RGB;
		lcdcfg.fpsx1000 = dpicfg->fpsx1000;
		ret = lcdlib_init(dsi_handle->idsx, &lcdcfg);
		if(ret != IM_RET_OK){
			return IM_RET_FAILED;
		}
	}
	else{ // DBI
		dbicfg = (mipidsi_dbi_t *)gDsiPriv->block[index].priv;
		ret = dsilib_set_dbi_config(dsi_handle->DsiInst,dbicfg); 
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("  dsilib set dbi config error -- ")));
			return IM_RET_FAILED;
		}
		// should think over it .
		memset((void*)&i80cfg, 0, sizeof(i80c_config_t));
		i80lib_init(dsi_handle->idsx,&i80cfg);
	}

	return IM_RET_OK;
}

IM_RET dsi_get_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix)
{
	IM_UINT32 cnt, index;
	IM_UINT32 idsx, fpsx1000, locIdsx;
	IM_UINT8 dp[ITEM_MAX_LEN];
	IM_UINT8 acmstr[ITEM_MAX_LEN];
	IM_TCHAR str[ITEM_MAX_LEN];
	IM_UINT8 config_str[ITEM_MAX_LEN];
	DSI_Priv_conf *dsi_priv = IM_NULL;
    mipidsi_dpi_t *dpicfg;
    mipidsi_dbi_t *dbicfg;
	lcdc_config_t lcdcfg;
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s() config->id=%d"), IM_STR(_IM_FUNC_), config->id));

	idsx = dsi_handle->idsx; 

	cnt = 0;
	while(strcasecmp(gDSIConf[cnt].idstr, "none")){
		if(config->id == gDSIConf[cnt].devid){
			dsi_priv = (DSI_Priv_conf*) gDSIConf[cnt].priv;
			break;
		}
		cnt ++;
	}
	if (dsi_priv == IM_NULL){
		IM_ERRMSG((IM_STR("dsi config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(index = 0; index < DSI_PRIV_BLOCK_NUM; index++){
		if(dsi_priv->block[index].id == DSI_MANU_RGB)
			break;
	}
	if(index==DSI_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" dsi screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}

	locIdsx = 0;
	if (item_exist("ids.locidsx")){
		locIdsx = item_integer("ids.locidsx", 0);
		IM_ASSERT(locIdsx <= 1);
	}else{
		IM_ERRMSG((IM_STR(" local ids not defined in item, error !! ")));
		return IM_RET_FAILED;
	}

	if (locIdsx == idsx){
		sprintf(config_str, "ids.loc.dev%d.config_str",suffix);
	}else{
		sprintf(config_str, "ids.ext.dev%d.config_str",suffix);
	}

	if (item_exist(config_str)){
		item_string(str, config_str, 0);
		get_config(dsi_handle, str);
	}

	if(dsi_handle->mipidsi_type <= MIPIDSI_DPI_24BIT_CONFIG_1 ){ // DPI

		dpicfg = (mipidsi_dpi_t*)dsi_priv->block[index].priv;
		memset((void*)&lcdcfg, 0, sizeof(lcdc_config_t));
		lcdcfg.width = dpicfg->width; 
		lcdcfg.height = dpicfg->height;
		lcdcfg.VSPW = dpicfg->vspw;
		lcdcfg.VBPD = dpicfg->vbpd;
		lcdcfg.VFPD = dpicfg->vfpd;
		lcdcfg.HSPW = dpicfg->hspw;
		lcdcfg.HBPD = dpicfg->hbpd;
		lcdcfg.HFPD = dpicfg->hfpd;
		lcdcfg.signalPwrenEnable = 1;
		lcdcfg.signalPwrenInverse = 0;
		lcdcfg.signalVclkInverse = 1;
		lcdcfg.signalHsyncInverse = dpicfg->hsync_pol;
		lcdcfg.signalVsyncInverse = dpicfg->vsync_pol;
		lcdcfg.signalDataInverse = 0 ;
		lcdcfg.signalVdenInverse = dpicfg->vden_pol;
		lcdcfg.SignalDataMapping = DATASIGMAPPING_RGB;
		lcdcfg.fpsx1000 = dpicfg->fpsx1000 ;
		lcdlib_get_real_clk_fpsx1000(dsi_handle->idsx, &lcdcfg, IM_NULL, &fpsx1000);

		config->width = dpicfg->width;
		config->height = dpicfg->height;
		config->fpsx1000 = fpsx1000;

		if (info != IM_NULL){
			locIdsx = 0;
			if (item_exist("ids.locidsx")){
				locIdsx = item_integer("ids.locidsx", 0);
				IM_ASSERT(locIdsx <= 1);
			}else{
				IM_ERRMSG((IM_STR(" local ids not defined in item, error !! ")));
				return IM_RET_FAILED;
			}
			// data port
			if (locIdsx == idsx){
				sprintf(dp,"ids.loc.dev%d.data_port",suffix);
			}else{
				sprintf(dp,"ids.ext.dev%d.data_port",suffix);
			}
			info->dataport = DISPDEV_DATAPORT_RGB888;
			if (item_exist(dp)){
				item_string(str, dp, 0);
				if (strcmp(str,"888") == 0){
					info->dataport = DISPDEV_DATAPORT_RGB888;
				}
				else if (strcmp(str,"565") == 0){
					info->dataport = DISPDEV_DATAPORT_RGB565;
				}
				else if (strcmp(str,"666") == 0){
					info->dataport = DISPDEV_DATAPORT_RGB666;
				}
			}
			// default acm level
			if (locIdsx == idsx){
				sprintf(acmstr ,"ids.loc.dev%d.acm_level",suffix);
			}else{
				sprintf(acmstr ,"ids.ext.dev%d.acm_level",suffix);
			}
			info->dftacm = 0;
			if (item_exist(acmstr)){
				info->dftacm = item_integer(acmstr, 0);
			}

			info->width = dpicfg->width;
			info->height = dpicfg->height;
			info->hspw = dpicfg->hspw;
			info->hbpd = dpicfg->hbpd;
			info->hfpd = dpicfg->hfpd;
			info->vspw = dpicfg->vspw;
			info->vbpd = dpicfg->vbpd;
			info->vfpd = dpicfg->vfpd;
			info->fpsx1000 = fpsx1000;
            info->phyWidth = dpicfg->phyWidth;
            info->phyHeight = dpicfg->phyHeight;
		}
	}else {
		// DBI
		// TODO
		dbicfg = (mipidsi_dbi_t*)dsi_priv->block[index].priv;
	}

	return IM_RET_OK;
}

IM_RET dsi_open(dispdev_handle_t handle)
{
	IM_RET ret;
	IM_UINT32 cnt;
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	for (cnt=0; cnt < DSI_PRIV_BLOCK_NUM; cnt ++){
		switch (gDsiPriv->block[cnt].id){
			case DSI_MANU_POWER:
				if (dsiPower == PAD_TRUE){
					idspwl_pad_set(gPadNum, IM_TRUE);
				}
				else if(dsiPower == PMU_TRUE)
				{
					if(!IS_ERR(gIDSRegulator)){
						if(regulator_enable(gIDSRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: regulator_enable failed "),IM_STR(_IM_FUNC_)));
							return IM_RET_FAILED;
						}
					}
				}
				idspwl_module_enable(MODULE_DSI);
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				break;
			case DSI_MANU_CMD_ON:
				if (gDsiPriv->block[cnt].proc){
					gDsiPriv->block[cnt].proc();
					dsi_msleep(gDsiPriv->block[cnt].ms_time);
				}
				break;
			case DSI_MANU_RGB:
				ret = dsilib_open(dsi_handle->DsiInst);
				if(ret != IM_RET_OK){
					IM_ERRMSG((IM_STR("  dsilib_open error -- ")));
					return IM_RET_FAILED;
				}
				if(dsi_handle->mipidsi_type <= MIPIDSI_DPI_24BIT_CONFIG_1)
				{
					lcdlib_open(dsi_handle->idsx);
				}
				else
				{
					i80lib_open(dsi_handle->idsx);	
				}
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				break;
			case DSI_MANU_BL:
				imapbl_power_on();
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				break;
			case DSI_MANU_PROC1:
			case DSI_MANU_PROC2:
				if (gDsiPriv->block[cnt].proc){
					gDsiPriv->block[cnt].proc();
					dsi_msleep(gDsiPriv->block[cnt].ms_time);
				}
				break;
			default : 
				break;

		}
	}
	gDSIStateOn = IM_TRUE;

	return IM_RET_OK;
}

IM_RET dsi_close(dispdev_handle_t handle)
{
	IM_RET ret;
    IM_INT32 cnt;
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	for (cnt = DSI_PRIV_BLOCK_NUM - 1; cnt >= 0 ; cnt--){ 
		switch (gDsiPriv->block[cnt].id){
			case DSI_MANU_POWER:
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				if (dsiPower == PAD_TRUE){
					idspwl_pad_set(gPadNum, IM_FALSE);
				}
				else if(dsiPower == PMU_TRUE)
				{
					if(!IS_ERR(gIDSRegulator)){
						if(regulator_disable(gIDSRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: regulator_disable failed "), IM_STR(_IM_FUNC_)));
							return IM_RET_FAILED;
						}
					}
				}
				break;
			case DSI_MANU_CMD_OFF:
				if (gDsiPriv->block[cnt].proc){
					dsi_msleep(gDsiPriv->block[cnt].ms_time);
					gDsiPriv->block[cnt].proc();
				}
				break;
			case DSI_MANU_RGB:
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				ret = dsilib_close(dsi_handle->DsiInst);
				if(ret != IM_RET_OK){
					IM_ERRMSG((IM_STR("  dsilib_close error -- ")));
					return IM_RET_FAILED;
				}
				if(dsi_handle->mipidsi_type <= MIPIDSI_DPI_24BIT_CONFIG_1)
				{
					lcdlib_close(dsi_handle->idsx);
				}
				else
				{//i80 switch
					i80lib_close(dsi_handle->idsx);	
				}
				break;
			case DSI_MANU_BL:
				dsi_msleep(gDsiPriv->block[cnt].ms_time);
				imapbl_shut_down();
				break;
			case DSI_MANU_PROC1:
			case DSI_MANU_PROC2:
				if (gDsiPriv->block[cnt].proc){
					dsi_msleep(gDsiPriv->block[cnt].ms_time);
					gDsiPriv->block[cnt].proc();
				}
				break;
			default : 
				break;

		}
	}

	gDSIStateOn = IM_FALSE;
	return IM_RET_OK;
}

IM_RET dsi_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener)
{
	//mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;
}

IM_RET dsi_suspend(dispdev_handle_t handle)
{
	IM_RET ret;
	mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if (gDSIStateOn == IM_TRUE){
		dsi_close(handle);
		gDSIStateOn = IM_FALSE;
	}

	return IM_RET_OK;
}

IM_RET dsi_resume(dispdev_handle_t handle)
{
	//mipidsi_handle *dsi_handle = (mipidsi_handle*)handle;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;
}


