/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of lcd APIs
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.5	 Sam@2012/6/14 :  move rgb order item judge from lcd_lib to lcd_api
** v2.0.1	 Sam@2013/02/25:  disable asynchronous power on
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <mach/items.h>
#include <linux/regulator/consumer.h>

#include <ids_lib.h>
#include <ids_pwl.h>
#include <lcd_lib.h>
#include <lcd_api.h>
#include <lcd_def.h>
#include "lcd_cfg.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"LCDAPI_I:"
#define WARNHEAD	"LCDAPI_W:"
#define ERRHEAD		"LCDAPI_E:"
#define TIPHEAD		"LCDAPI_T:"


#define LCD_NUM_0 (0x10)
#define LCD_NUM_1 (0x20)

enum{
	NONE  = 0,
	PAD_TRUE = 1,
	PMU_TRUE
};

typedef struct lcd_suspend_struct{
	ids_display_device_t lcd_0_config;
	ids_display_device_t lcd_1_config;

	void *ieHandle;
}lcd_suspend_struct;

static IM_UINT32 gLcdNum[2] = {0};
static IM_INT32 gLCDStateOn[2] = {IM_FALSE};

static lcd_suspend_struct gLcdConfig;

static LCD_Priv_conf *gLcdPriv = IM_NULL;

extern idsdrv_resource_t *gIdsdrvResource;
static struct regulator *gLcdRegulator = (struct regulator*)-1;
static struct regulator *gLvdsRegulator = (struct regulator*)-1;
static IM_INT32 gLcdPadNum, gLvdsPadNum;
static IM_INT32 lcdPower = NONE;
static IM_INT32 lvdsPower = NONE;

extern void imapbl_power_on(void);
extern void imapbl_shut_down(void);

static void lcd_msleep(IM_INT32 ms)
{
    if (ms){
        msleep(ms);
    }
}

static IM_INT32 get_pad(IM_TCHAR *str)
{
	return item_integer(str,1);	
}

// if lcd IM_TRUE, means it pmu power for lcd, or it means pmu power for lvds
static IM_INT32 get_pmu_power(IM_TCHAR *pmustr, IM_BOOL lcd)
{
	IM_INT32 err;
	IM_TCHAR str[ITEM_MAX_LEN];

    if (lcd){
        if (gLcdRegulator != (struct regulator*)-1){
            regulator_put(gLcdRegulator);
            gLcdRegulator = (struct regulator*)-1;
        }
        item_string(str, pmustr, 1);
        gLcdRegulator = regulator_get(&gIdsdrvResource->pdev->dev, str);
        if(IS_ERR(gLcdRegulator))
        {
            IM_ERRMSG((IM_STR("%s: lcd get regulator fail"), __func__));
            return IM_RET_FAILED;
        }
        err = regulator_set_voltage(gLcdRegulator, 3300000, 3300000);
        if(err)
        {
            IM_ERRMSG((IM_STR("%s: lcd set regulator fail"), __func__));
            return IM_RET_FAILED;
        }
    }
    else {
        if (gLvdsRegulator != (struct regulator*)-1){
            regulator_put(gLvdsRegulator);
            gLvdsRegulator = (struct regulator*)-1;
        }
        item_string(str, pmustr, 1);
        gLvdsRegulator = regulator_get(&gIdsdrvResource->pdev->dev, str);
        if(IS_ERR(gLvdsRegulator))
        {
            IM_ERRMSG((IM_STR("%s: lvds get regulator fail"), __func__));
            return IM_RET_FAILED;
        }
        err = regulator_set_voltage(gLvdsRegulator, 3300000, 3300000);
        if(err)
        {
            IM_ERRMSG((IM_STR("%s: lvds set regulator fail"), __func__));
            return IM_RET_FAILED;
        }
    }
    return IM_RET_OK;
}

IM_RET lcd_init(dispdev_handle_t *handle , IM_INT32 idsx)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	val = (idsx == 0)? LCD_NUM_0 : LCD_NUM_1;
	if(gLcdNum[idsx] == val){
		return IM_RET_FAILED;
	}
	memset((void *)&gLcdConfig, 0, sizeof(lcd_suspend_struct));
	gLcdNum[idsx] = val;
	*handle = &(gLcdNum[idsx]);
	
	return IM_RET_OK;
}

IM_RET lcd_deinit(dispdev_handle_t handle)
{
	IM_UINT32 idsx, lcdNo;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0) {
		idsx = 0;
		gLcdNum[0] = 0;
	}
	else if (lcdNo == LCD_NUM_1) {
		idsx = 1;
		gLcdNum[1] = 0;
	}
	else {
		return IM_RET_FAILED;
	}

	if (gLcdRegulator != (struct regulator*)-1){     
		regulator_put(gLcdRegulator);                
		gLcdRegulator = (struct regulator*)-1;       
	}
	if (gLvdsRegulator != (struct regulator*)-1){     
		regulator_put(gLvdsRegulator);                
		gLvdsRegulator = (struct regulator*)-1;       
	}

	handle = IM_NULL;
	return lcdlib_deinit(idsx);
}

IM_RET lcd_set_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix)
{
	IM_INT32 ret;
	IM_UINT32 idsx, lcdNo, cnt, locIdsx;
	lcdc_config_t *cfg;
	IM_UINT8 rgbstr[ITEM_MAX_LEN];
	IM_UINT8 lcdstr[ITEM_MAX_LEN];
	IM_UINT8 lvdsstr[ITEM_MAX_LEN];
	IM_TCHAR str[ITEM_MAX_LEN];
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_INFOMSG((IM_STR("id=0x%x, width=%d,height=%d"), config->id,config->width,config->height));

	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0){
		idsx = 0;
	}else if(lcdNo == LCD_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

    cnt = 0;
    while(strcasecmp(gLCDConf[cnt].idstr, "none")){
        if(config->id == gLCDConf[cnt].devid){
            gLcdPriv = (LCD_Priv_conf*) gLCDConf[cnt].priv;
            break;
        }
        cnt ++;
    }
	if (gLcdPriv == IM_NULL){
		IM_ERRMSG((IM_STR("lcd config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(cnt = 0; cnt < LCD_PRIV_BLOCK_NUM; cnt++){
		if(gLcdPriv->block[cnt].id == LCD_MANU_RGB)
			break;
	}
	if(cnt==LCD_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" lcd screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}
	//memcpy((void *)&cfg, (void *)gLcdPriv->block[cnt].priv, sizeof(lcdc_config_t));
	cfg = (lcdc_config_t*)gLcdPriv->block[cnt].priv;
	
	if (lcdNo == LCD_NUM_0){
		memcpy((void*)&gLcdConfig.lcd_0_config, config, sizeof(ids_display_device_t));
	}else{
		memcpy((void*)&gLcdConfig.lcd_1_config, config, sizeof(ids_display_device_t));
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
		sprintf(rgbstr,"ids.loc.dev%d.rgb_order",suffix);
		sprintf(lcdstr,"ids.loc.dev%d.lcd.power",suffix);
		sprintf(lvdsstr,"ids.loc.dev%d.lvds.power",suffix);
	}else{
		sprintf(rgbstr,"ids.ext.dev%d.rgb_order",suffix);
		sprintf(lcdstr,"ids.ext.dev%d.lcd.power",suffix);
		sprintf(lvdsstr,"ids.ext.dev%d.lvds.power",suffix);
	}

	if(item_exist(lcdstr)){
		item_string(str, lcdstr, 0);
		if(strncmp(str, "pads",4)==0){
			gLcdPadNum = get_pad(lcdstr);
			lcdPower = PAD_TRUE;
		}
		else if (strncmp(str, "pmu", 3) == 0){
			ret = get_pmu_power(lcdstr, IM_TRUE);
			if (ret != IM_RET_OK){
				return IM_RET_FAILED;
			}
			lcdPower = PMU_TRUE;
		}
		else {
			lcdPower = NONE;
		}
	}
	if(item_exist(lvdsstr)){
		item_string(str, lvdsstr, 0);
		if(strncmp(str, "pads",4)==0){
			gLvdsPadNum = get_pad(lvdsstr);
			lvdsPower = PAD_TRUE;
		}
		else if (strncmp(str, "pmu", 3) == 0){
			ret = get_pmu_power(lvdsstr, IM_FALSE);
			if (ret != IM_RET_OK){
				return IM_RET_FAILED;
			}
			lvdsPower = PMU_TRUE;
		}
		else {
			lvdsPower = NONE;
		}
	}

	if(item_exist(rgbstr)){
		item_string(str,rgbstr,0);
		IM_INFOMSG((IM_STR("lcd rgb order = %s - "),str));
		if(strcmp(str,"rgb") == 0){
			cfg->SignalDataMapping = DATASIGMAPPING_RGB;
		}
		else if(strcmp(str,"rbg") ==0){
			cfg->SignalDataMapping = DATASIGMAPPING_RBG;
		}
		else if(strcmp(str,"grb") ==0){
			cfg->SignalDataMapping = DATASIGMAPPING_GRB;
		}
		else if(strcmp(str,"gbr") ==0){
			cfg->SignalDataMapping = DATASIGMAPPING_GBR;
		}
		else if(strcmp(str,"brg") ==0){
			cfg->SignalDataMapping = DATASIGMAPPING_BRG;
		}
		else if(strcmp(str,"bgr") ==0){
			cfg->SignalDataMapping = DATASIGMAPPING_BGR;
		}
		else{
			IM_ERRMSG((IM_STR(" rgb order not supported -- ")));
		}
	}

	return lcdlib_init(idsx, cfg);
}

IM_RET lcd_get_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix)
{
	IM_UINT32 cnt, locIdsx;
	IM_UINT32 idsx, lcdNo, fpsx1000;
	IM_UINT8 dp[ITEM_MAX_LEN];
	IM_UINT8 acmstr[ITEM_MAX_LEN];
	IM_TCHAR str[ITEM_MAX_LEN];
	LCD_Priv_conf *lcd_priv= IM_NULL;
    lcdc_config_t *cfg;
	IM_INFOMSG((IM_STR("%s() config->id=%d"), IM_STR(_IM_FUNC_), config->id));

	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0){
		idsx = 0;
	}else if(lcdNo == LCD_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

    cnt = 0;
    while(strcasecmp(gLCDConf[cnt].idstr, "none")){
        if(config->id == gLCDConf[cnt].devid){
            lcd_priv = (LCD_Priv_conf*) gLCDConf[cnt].priv;
            break;
        }
        cnt ++;
    }

	if (lcd_priv == IM_NULL){
		IM_ERRMSG((IM_STR("lcd config id not support, error ! ")));
		return IM_RET_FAILED;
	}
	for(cnt = 0; cnt < LCD_PRIV_BLOCK_NUM; cnt++){
		if(lcd_priv->block[cnt].id == LCD_MANU_RGB)
			break;
	}
	if(cnt==LCD_PRIV_BLOCK_NUM){
		IM_ERRMSG((IM_STR(" lcd screen internal definition error , check it ")));
		return IM_RET_FAILED;
	}

    cfg = (lcdc_config_t*)lcd_priv->block[cnt].priv;
    lcdlib_get_real_clk_fpsx1000(idsx, cfg, IM_NULL, &fpsx1000);

	config->width = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->width;
	config->height = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->height;
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

		info->width = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->width;
		info->height = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->height;
		info->hspw = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->HSPW;
		info->hbpd = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->HBPD;
		info->hfpd = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->HFPD;
		info->vspw = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->VSPW;
		info->vbpd = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->VBPD;
		info->vfpd = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->VFPD;
		info->fpsx1000 = fpsx1000;
		info->phyWidth = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->phyWidth;;
		info->phyHeight = ((lcdc_config_t*)(lcd_priv->block[cnt].priv))->phyHeight;
	}
	return IM_RET_OK;
}

IM_RET lcd_open(dispdev_handle_t handle)
{
	IM_UINT32 lcdNo, idsx, cnt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0) {
		idsx = 0;
	}
	else if(lcdNo == LCD_NUM_1){
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

	for (cnt = 0; cnt < LCD_PRIV_BLOCK_NUM; cnt ++){
		switch (gLcdPriv->block[cnt].id){
			case LCD_MANU_POWER:
				if (lcdPower == PAD_TRUE){
					idspwl_pad_set(gLcdPadNum, IM_TRUE);
				}
				else if(lcdPower == PMU_TRUE)
				{
					if(!IS_ERR(gLcdRegulator)){
						if(regulator_enable(gLcdRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: lcd regulator_enable failed "), __func__));
							return IM_RET_FAILED;
						}
					}
				}
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				break;
			case LCD_MANU_RGB:
				lcdlib_open(idsx);
				if(idsx)
					idspwl_module_enable(MODULE_LCD1);
				else 
					idspwl_module_enable(MODULE_LCD0);
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				break;
			case LCD_MANU_LVDS:
				if (lvdsPower == PAD_TRUE){
					idspwl_pad_set(gLvdsPadNum, IM_TRUE);
				}
				else if(lvdsPower == PMU_TRUE)
				{
					if(!IS_ERR(gLvdsRegulator)){
						if(regulator_enable(gLvdsRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: lvds regulator_enable failed "), __func__));
							return IM_RET_FAILED;
						}
					}
				}
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				break;
			case LCD_MANU_BL:
				imapbl_power_on();
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				break;
			case LCD_MANU_PROC1:
			case LCD_MANU_PROC2:
			case LCD_MANU_PROC3:
				if (gLcdPriv->block[cnt].proc){
					gLcdPriv->block[cnt].proc();
					lcd_msleep(gLcdPriv->block[cnt].ms_time);
				}
				break;
			default : 
				break;
		}
	}
	gLCDStateOn[idsx] = IM_TRUE;
	return IM_RET_OK;
}


IM_RET lcd_close(dispdev_handle_t handle)
{
	IM_UINT32 idsx, lcdNo;
	IM_INT32 cnt;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0){
	   	idsx = 0;
	}else if (lcdNo == LCD_NUM_1) {
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

	for (cnt = LCD_PRIV_BLOCK_NUM-1; cnt >= 0; cnt--)
	{
		switch (gLcdPriv->block[cnt].id)
		{
			case LCD_MANU_POWER:
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				if (lcdPower == PAD_TRUE){
					idspwl_pad_set(gLcdPadNum, IM_FALSE);
				}
				else if(lcdPower == PMU_TRUE)
				{
					if(!IS_ERR(gLcdRegulator)){
						if(regulator_disable(gLcdRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: lcd regulator_disbale failed "), __func__));
							return -1;
						}
					}
				}
				break;
			case LCD_MANU_RGB:
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				if(idsx)
					idspwl_module_disable(MODULE_LCD1);
				else 
					idspwl_module_disable(MODULE_LCD0);
				lcdlib_close(idsx);
				break;
			case LCD_MANU_LVDS:
				//lcd_msleep(gLcdPriv->block[cnt].ms_time);
				if (lvdsPower == PAD_TRUE){
					idspwl_pad_set(gLvdsPadNum, IM_FALSE);
				}
				else if(lvdsPower == PMU_TRUE)
				{
					if(!IS_ERR(gLvdsRegulator)){
						if(regulator_disable(gLvdsRegulator) != 0){
							IM_ERRMSG((IM_STR("%s: lvds regulator_disable failed "), __func__));
							return -1;
						}
					}
				}
				break;
			case LCD_MANU_BL:
				lcd_msleep(gLcdPriv->block[cnt].ms_time);
				imapbl_shut_down();
				break;
			case LCD_MANU_PROC1:
			case LCD_MANU_PROC2:
			case LCD_MANU_PROC3:
				if (gLcdPriv->block[cnt].proc){
					lcd_msleep(gLcdPriv->block[cnt].ms_time);
					gLcdPriv->block[cnt].proc();
				}
				break;
			default : 
				break;
		}
	}
    gLCDStateOn[idsx] = IM_FALSE;
	return IM_RET_OK;
}

IM_RET lcd_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET lcd_suspend(dispdev_handle_t handle)
{
	IM_UINT32 idsx, lcdNo;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	lcdNo = *(IM_UINT32 *)handle;
	if(lcdNo == LCD_NUM_0){
	   	idsx = 0;
	}else if (lcdNo == LCD_NUM_1) {
		idsx = 1;
	}else {
		return IM_RET_FAILED;
	}

    // LCD no need to record the state before suspend.
    if(gLCDStateOn[idsx] == IM_TRUE){
        lcd_close(handle);
        gLCDStateOn[idsx] = IM_FALSE;
    }

	return IM_RET_OK;
}

IM_RET lcd_resume(dispdev_handle_t handle)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}


