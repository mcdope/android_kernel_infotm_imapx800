/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of i80 APIs
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.2     liu@2012/12/20:  adjust the code structure
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <mach/items.h>
#include <linux/regulator/consumer.h>

#include <ids_lib.h>
#include <ids_pwl.h>
#include <i80_lib.h>
#include <i80_api.h>
#include <i80_def.h>
#include "i80_cfg.h"

#define DBGINFO		1
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"I80API_I:"
#define WARNHEAD	"I80API_W:"
#define ERRHEAD		"I80API_E:"
#define TIPHEAD		"I80API_T:"


#define I80_NUM_0 (0x50)
#define I80_NUM_1 (0x60)

enum{
	NONE  = 0,
	PAD_TRUE = 1,
	PMU_TRUE
};

typedef struct i80_suspend_struct{
	ids_display_device_t i80_0_config;
	ids_display_device_t i80_1_config;

	void *ieHandle;
}i80_suspend_struct;

static IM_UINT32 gI80Num[2] = {0};
static IM_INT32 gI80StateOn[2] = {IM_FALSE};

static i80_suspend_struct gI80Config;

static I80_Priv_conf *gI80Priv = IM_NULL;

extern idsdrv_resource_t *gIdsdrvResource;
static struct regulator *gIDSRegulator = (struct regulator*)-1;
static IM_INT32 gPadNum;
static IM_INT32 i80Power = NONE;

extern void imapbl_power_on(void);
extern void imapbl_shut_down(void);

static void i80_msleep(IM_INT32 ms)
{
    if (ms){
        msleep(ms);
    }
}

static IM_RET get_pad(IM_TCHAR *str)
{
	gPadNum = item_integer(str,1);//get dev num	
	return IM_RET_OK;
}

static IM_INT32 get_pmu_power(IM_TCHAR *pmustr)
{
	IM_INT32 err;
	IM_TCHAR str[ITEM_MAX_LEN];

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
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
static IM_RET get_config_format(i80c_config_t *cfg, IM_TCHAR *str);

IM_RET i80_init(dispdev_handle_t *handle , IM_INT32 idsx)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	val = (idsx == 0)? I80_NUM_0 : I80_NUM_1;
	if(gI80Num[idsx] == val){
		return IM_RET_FAILED;
	}
	memset((void *)&gI80Config, 0, sizeof(i80_suspend_struct));
	gI80Num[idsx] = val;
	*handle = &(gI80Num[idsx]);
	
	return IM_RET_OK;
}

IM_RET i80_deinit(dispdev_handle_t handle)
{
	IM_UINT32 idsx, i80No;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0) {
		idsx = 0;
		gI80Num[0] = 0;
	}
	else if (i80No == I80_NUM_1) {
		idsx = 1;
		gI80Num[1] = 0;
	}
	else {
		return IM_RET_FAILED;
	}

    if (gIDSRegulator != (struct regulator*)-1){     
        regulator_put(gIDSRegulator);                
        gIDSRegulator = (struct regulator*)-1;       
    }

	handle = IM_NULL;
	return i80lib_deinit(idsx);
}

IM_RET i80_set_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix)
{
	IM_INT32 ret;
	IM_UINT32 idsx, i80No, cnt, locIdsx;
	i80c_config_t *cfg;
	IM_UINT8 configstr[ITEM_MAX_LEN];
	IM_UINT8 powerstr[ITEM_MAX_LEN];
	IM_TCHAR str[ITEM_MAX_LEN];
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0){
		idsx = 0;
	}else if(i80No == I80_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

    cnt = 0;
    while(strcasecmp(gI80Conf[cnt].idstr, "none")){
        if(config->id == gI80Conf[cnt].devid){
            gI80Priv = (I80_Priv_conf*) gI80Conf[cnt].priv;
            break;
        }
        cnt ++;
    }
	if (gI80Priv == IM_NULL){
		IM_ERRMSG((IM_STR("i80 config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(cnt = 0; cnt < I80_PRIV_BLOCK_NUM; cnt++){
		if(gI80Priv->block[cnt].id == I80_MANU_RGB)
			break;
	}
	if(cnt==I80_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" i80 screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}
	//memcpy((void *)&cfg, (void *)gI80Priv->block[cnt].priv, sizeof(i80c_config_t));
	cfg = (i80c_config_t*)gI80Priv->block[cnt].priv;
	
	if (i80No == I80_NUM_0){
		memcpy((void*)&gI80Config.i80_0_config, config, sizeof(ids_display_device_t));
	}else{
		memcpy((void*)&gI80Config.i80_1_config, config, sizeof(ids_display_device_t));
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
	    sprintf(configstr, "ids.loc.dev%d.data.format",suffix);
		sprintf(powerstr,"ids.loc.dev%d.i80.power",suffix);
	}else{
        sprintf(configstr, "ids.loc.dev%d.data.format",suffix);
		sprintf(powerstr,"ids.ext.dev%d.i80.power",suffix);
	}

    if(item_exist(configstr)){
		item_string(str, configstr, 0);
		get_config_format(cfg, str);
	}

	if(item_exist(powerstr)){
		item_string(str, powerstr, 0);
		if(strncmp(str, "pads",4)==0){
			get_pad(powerstr);
			i80Power = PAD_TRUE;
		}
		else if (strncmp(str, "pmu", 3) == 0){
			ret = get_pmu_power(powerstr);
			if (ret != IM_RET_OK){
				return IM_RET_FAILED;
			}
			i80Power = PMU_TRUE;
		}
		else {
			i80Power = NONE;
		}
	}
	return i80lib_init(idsx, cfg);
}

IM_RET i80_get_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix)
{
	IM_UINT32 cnt, locIdsx;
	IM_UINT32 idsx, i80No, fpsx1000;
	IM_UINT8 dp[ITEM_MAX_LEN];
	IM_UINT8 acmstr[ITEM_MAX_LEN];
	IM_TCHAR str[ITEM_MAX_LEN];
	I80_Priv_conf *i80_priv= IM_NULL;
    i80c_config_t *cfg;
	IM_INFOMSG((IM_STR("%s() config->type=%d, config->id=%d"), IM_STR(_IM_FUNC_), config->type, config->id));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0){
		idsx = 0;
	}else if(i80No == I80_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

    cnt = 0;
    while(strcasecmp(gI80Conf[cnt].idstr, "none")){
        if(config->id == gI80Conf[cnt].devid){
            i80_priv = (I80_Priv_conf*) gI80Conf[cnt].priv;
            break;
        }
        cnt ++;
    }
	if (i80_priv == IM_NULL){
		IM_ERRMSG((IM_STR("i80 config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(cnt = 0; cnt < I80_PRIV_BLOCK_NUM; cnt++){
		if(i80_priv->block[cnt].id == I80_MANU_RGB)
			break;
	}
	if(cnt==I80_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" i80 screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}

    cfg = (i80c_config_t*)i80_priv->block[cnt].priv;

	config->width = ((i80c_config_t*)(i80_priv->block[cnt].priv))->screen_width;
	config->height = ((i80c_config_t*)(i80_priv->block[cnt].priv))->screen_height;
	config->fpsx1000 = 60000;

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
		if (locIdsx == idsx){
			sprintf(acmstr ,"ids.loc.dev%d.acm_level",suffix);
		}else{
			sprintf(acmstr ,"ids.ext.dev%d.acm_level",suffix);
		}
		info->dftacm = 0;
		if (item_exist(acmstr)){
			info->dftacm = item_integer(acmstr, 0);
		}

		info->width = ((i80c_config_t*)(i80_priv->block[cnt].priv))->screen_width;
		info->height = ((i80c_config_t*)(i80_priv->block[cnt].priv))->screen_height;
		info->hspw = 0;
		info->hbpd = 1;
		info->hfpd = 0;
		info->vspw = 0;
		info->vbpd = 1;
		info->vfpd = 0;
		info->fpsx1000 = 60000;
	}
	return IM_RET_OK;
}

IM_RET i80_open(dispdev_handle_t handle)
{
	IM_UINT32 idsx, i80No, cnt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0) {
		idsx = 0;
	}else if(i80No == I80_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

	for (cnt = 0; cnt < I80_PRIV_BLOCK_NUM; cnt ++){
		switch (gI80Priv->block[cnt].id){
			case I80_MANU_POWER:
				if (i80Power == PAD_TRUE){
					idspwl_pad_set(gPadNum, IM_TRUE);
				}
				else if(i80Power == PMU_TRUE)
				{
					if(!IS_ERR(gIDSRegulator)){
						if(regulator_enable(gIDSRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: regulator_enable failed "), __func__));
							return -1;
						}
					}
				}
				i80_msleep(gI80Priv->block[cnt].ms_time);
				break;
			case I80_MANU_RGB:                                         //手动模式一般只用于上电下电
				i80lib_open(idsx);
				if(idsx)
					idspwl_module_enable(MODULE_I801);
				else 
					idspwl_module_enable(MODULE_I800);
				i80_msleep(gI80Priv->block[cnt].ms_time);
				break;
			case I80_MANU_BL:
				imapbl_power_on();
				i80_msleep(gI80Priv->block[cnt].ms_time);
				break;
			case I80_MANU_PROC1:
			case I80_MANU_PROC2:
			case I80_MANU_PROC3:
				if (gI80Priv->block[cnt].proc){
					gI80Priv->block[cnt].proc();
					i80_msleep(gI80Priv->block[cnt].ms_time);
				}
				break;
			default : 
				break;
		}
	}
	gI80StateOn[idsx] = IM_TRUE;
	return IM_RET_OK;
}

IM_RET i80_close(dispdev_handle_t handle)
{
	IM_UINT32 idsx, i80No;
	IM_INT32 cnt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0){
		idsx = 0;
	}else if (i80No == I80_NUM_1) {
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

	for (cnt = I80_PRIV_BLOCK_NUM-1; cnt >= 0; cnt--)
	{
		switch (gI80Priv->block[cnt].id)
		{
			case I80_MANU_POWER:
				i80_msleep(gI80Priv->block[cnt].ms_time);
				if (i80Power == PAD_TRUE){
					idspwl_pad_set(gPadNum, IM_FALSE);
				}
				else if(i80Power == PMU_TRUE)
				{
					if(!IS_ERR(gIDSRegulator)){
						if(regulator_disable(gIDSRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: regulator_enable failed "), __func__));
							return -1;
						}
					}
				}
				break;
			case I80_MANU_RGB:
				i80_msleep(gI80Priv->block[cnt].ms_time);
				if(idsx)
					idspwl_module_disable(MODULE_I801);
				else 
					idspwl_module_disable(MODULE_I800);
				i80lib_close(idsx);
				break;
			case I80_MANU_BL:
				i80_msleep(gI80Priv->block[cnt].ms_time);
				imapbl_shut_down();
				break;
			case I80_MANU_PROC1:
			case I80_MANU_PROC2:
			case I80_MANU_PROC3:
				if (gI80Priv->block[cnt].proc){
					i80_msleep(gI80Priv->block[cnt].ms_time);
					gI80Priv->block[cnt].proc();
				}
				break;
			default : 
				break;
		}
	}
	gI80StateOn[idsx] = IM_FALSE;
	return IM_RET_OK;
}

IM_RET i80_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET i80_suspend(dispdev_handle_t handle)
{
	IM_UINT32 idsx, i80No;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80No = *(IM_UINT32 *)handle;
	if(i80No == I80_NUM_0){
		idsx = 0;
	}else if (i80No == I80_NUM_1) {
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

	//I80 no need to record the state before suspend.
	if(gI80StateOn[idsx] == IM_TRUE){
		i80_close(handle);
		gI80StateOn[idsx] = IM_FALSE;
	}

	return IM_RET_OK;
}

IM_RET i80_resume(dispdev_handle_t handle)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET get_config_format(i80c_config_t *cfg, IM_TCHAR *str)
{
	IM_INFOMSG((IM_STR("%s() config str(%s)"), IM_STR(_IM_FUNC_),str));

	cfg->Data_format = I80IF_BUS18_xx_xx_x_x; // default config : 0x180
	if (strcmp(str, "BUS9_1x_xx_0_x") == 0){
		cfg->Data_format = I80IF_BUS9_1x_xx_0_x;
	}
	else if (strcmp(str, "BUS9_1x_xx_1_x") == 0){
		cfg->Data_format = I80IF_BUS9_1x_xx_1_x;
	}
	else if (strcmp(str, "BUS9_0x_xx_0_x") == 0){
		cfg->Data_format = I80IF_BUS9_0x_xx_0_x;
	}
	else if (strcmp(str, "BUS9_0x_xx_1_x") == 0){
		cfg->Data_format = I80IF_BUS9_0x_xx_1_x;
	}
	else if (strcmp(str, "BUS16_10_11_x_x") == 0){
		cfg->Data_format = I80IF_BUS16_10_11_x_x;
	}
	else if (strcmp(str, "BUS16_00_11_x_x") == 0){
		cfg->Data_format = I80IF_BUS16_00_11_x_x;
	}
	else if (strcmp(str, "BUS16_11_11_x_x") == 0){
		cfg->Data_format = I80IF_BUS16_11_11_x_x;
	}
	else if (strcmp(str, "BUS16_01_11_x_x") == 0){
		cfg->Data_format = I80IF_BUS16_01_11_x_x;
	}
	else if (strcmp(str, "BUS16_10_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_10_10_0_0;
	}
	else if (strcmp(str, "BUS16_10_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_10_10_0_1;
	}
	else if (strcmp(str, "BUS16_10_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_10_10_1_0;
	}
	else if (strcmp(str, "BUS16_10_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_10_10_1_1;
	}
	else if (strcmp(str, "BUS16_10_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_10_01_1_0;
	}
	else if (strcmp(str, "BUS16_10_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_10_01_1_1;
	}
	else if (strcmp(str, "BUS16_10_01_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_10_01_0_0;
	}
	else if (strcmp(str, "BUS16_10_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_10_01_0_1;
	}
	else if (strcmp(str, "BUS16_00_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_0_0;
	}
	else if (strcmp(str, "BUS16_00_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_0_1;
	}
	else if (strcmp(str, "BUS16_00_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_1_0;
	}
	else if (strcmp(str, "BUS16_00_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_0_1;
	}
	else if (strcmp(str, "BUS16_00_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_1_0;
	}
	else if (strcmp(str, "BUS16_00_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_00_10_1_1;
	}
	else if (strcmp(str, "BUS16_00_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_00_01_1_0;
	}
	else if (strcmp(str, "BUS16_00_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_00_01_1_1;
	}
	else if (strcmp(str, "BUS16_00_01_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_00_01_0_0;
	}
	else if (strcmp(str, "BUS16_00_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_00_01_0_1;
	}
	else if (strcmp(str, "BUS16_11_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_11_10_0_0;
	}
	else if (strcmp(str, "BUS16_11_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_11_10_0_1;
	}
	else if (strcmp(str, "BUS16_11_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_11_10_1_0;
	}
	else if (strcmp(str, "BUS16_11_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_11_10_1_1;
	}
	else if (strcmp(str, "BUS16_11_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_11_01_1_1;
	}
	else if (strcmp(str, "BUS16_11_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_11_01_0_1;
	}
	else if (strcmp(str, "BUS16_11_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_11_01_1_0;
	}
	else if (strcmp(str, "BUS16_01_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_01_10_0_0;
	}
	else if (strcmp(str, "BUS16_01_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_01_10_0_1;
	}
	else if (strcmp(str, "BUS16_01_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_01_10_1_0;
	}
	else if (strcmp(str, "BUS16_01_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_01_10_1_1;
	}
	else if (strcmp(str, "BUS16_01_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS16_01_01_1_0;
	}
	else if (strcmp(str, "BUS16_01_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS16_01_01_1_1;
	}
	else if (strcmp(str, "BUS16_01_01_0_0") == 0){
		cfg->Data_format = I80IF_BUS16_01_01_0_0;
	}
	else if (strcmp(str, "BUS16_01_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS16_01_01_0_1;
	}
	else if (strcmp(str, "BUS8_0x_11_0_x") == 0){
		cfg->Data_format = I80IF_BUS8_0x_11_0_x;
	}
	else if (strcmp(str, "BUS8_0x_11_1_x") == 0){
		cfg->Data_format = I80IF_BUS8_0x_11_1_x;
	}
	else if (strcmp(str, "BUS8_1x_11_0_x") == 0){
		cfg->Data_format = I80IF_BUS8_1x_11_0_x;
	}
	else if (strcmp(str, "BUS8_1x_11_1_x") == 0){
		cfg->Data_format = I80IF_BUS8_1x_11_1_x;
	}
	else if (strcmp(str, "BUS8_0x_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_10_0_1;
	}
	else if (strcmp(str, "BUS8_0x_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_10_0_0;
	}
	else if (strcmp(str, "BUS8_0x_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_10_1_1;
	}
	else if (strcmp(str, "BUS8_0x_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_10_1_0;
	}
	else if (strcmp(str, "BUS8_1x_10_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_10_0_1;
	}
	else if (strcmp(str, "BUS8_1x_10_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_10_0_0;
	}
	else if (strcmp(str, "BUS8_1x_10_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_10_1_1;
	}
	else if (strcmp(str, "BUS8_1x_10_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_10_1_0;
	}
	else if (strcmp(str, "BUS8_0x_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_01_0_1;
	}
	else if (strcmp(str, "BUS8_0x_01_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_01_0_0;
	}
	else if (strcmp(str, "BUS8_0x_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_01_1_1;
	}
	else if (strcmp(str, "BUS8_0x_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_01_1_0;
	}
	else if (strcmp(str, "BUS8_0x_00_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_01_1_1;
	}
	else if (strcmp(str, "BUS8_0x_00_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_00_1_0;
	}
	else if (strcmp(str, "BUS8_0x_00_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_0x_00_0_1;
	}
	else if (strcmp(str, "BUS8_0x_00_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_0x_00_0_0;
	}
	else if (strcmp(str, "BUS8_1x_01_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_01_0_1;
	}
	else if (strcmp(str, "BUS8_1x_01_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_01_0_0;
	}
	else if (strcmp(str, "BUS8_1x_01_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_01_1_1;
	}
	else if (strcmp(str, "BUS8_1x_01_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_01_1_0;
	}
	else if (strcmp(str, "BUS8_1x_00_1_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_00_1_1;
	}
	else if (strcmp(str, "BUS8_1x_00_1_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_00_1_0;
	}
	else if (strcmp(str, "BUS8_1x_00_0_1") == 0){
		cfg->Data_format = I80IF_BUS8_1x_00_0_1;
	}
	else if (strcmp(str, "BUS8_1x_00_0_0") == 0){
		cfg->Data_format = I80IF_BUS8_1x_00_0_0;
	}
	else if (strcmp(str, "DBI_BUS16_xx_111_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS16_xx_111_x_x;
	}
	else if (strcmp(str, "DBI_BUS16_xx_010_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS16_xx_010_x_x;
	}
	else if (strcmp(str, "DBI_BUS16_xx_011_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS16_xx_011_x_x;
	}
	else if (strcmp(str, "DBI_BUS16_xx_110_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS16_xx_110_x_x;
	}
	else if (strcmp(str, "DBI_BUS9_xx_010_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS9_xx_010_x_x;
	}
	else if (strcmp(str, "DBI_BUS8_xx_110_1_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS8_xx_110_1_x;
	}
	else if (strcmp(str, "DBI_BUS8_xx_010_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS8_xx_010_x_x;
	}
	else if (strcmp(str, "DBI_BUS8_xx_010_1_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS8_xx_010_1_x;
	}
	else if (strcmp(str, "DBI_BUS8_xx_001_x_x") == 0){
		cfg->Data_format = I80IF_DBI_BUS8_xx_001_x_x;
	}else{
		IM_ERRMSG((IM_STR(" i80 config data format error - check it !!!")));
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}


