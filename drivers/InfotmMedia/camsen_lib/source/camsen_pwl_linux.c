/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camsen_pwl_linux.c
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/04/05: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/16: changed sensor pmu manager to this layer.
-- v1.0.4	arsor@2012/07/20: do erro handling when regulator_get() failed.
--
------------------------------------------------------------------------------*/

#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <mach/pad.h>
#include <mach/items.h>
#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMSEN_PWL_I:"
#define WARNHEAD	"CAMSEN_PWL_W:"
#define ERRHEAD		"CAMSEN_PWL_E:"
#define TIPHEAD		"CAMSEN_PWL_T:"

#define ISP_CLOCKGATE_ADDRESS	(0x21e34004)
#define CLKGATE_BUS				0
#define CLKGATE_OSD				1
#define CLKGATE_CSI				2
#define CLKGATE_CAMPIX			3
#define CLKGATE_CAMOUT			4


#define PMU_CTR_INTER		0
//#define PMU_CTR_INTER		1

static struct device *gDev = IM_NULL;

typedef struct {
	IM_CHAR name[256];
	IM_UINT32 facing;
	IM_UINT32 i2c_nr;
	IM_UINT32 pwdn;
	IM_UINT32 reset;
	IM_UINT32 flashLight;
	camsenpwl_pmu_info_t pmuInfo;
	camsen_ops *ops;
	struct i2c_adapter *adapter;
	struct clk *oclk;
	IM_INT32 oclkfrq;
}camsen_pwl_context_t;

#define REGULATOR_MAX	(6)

//camera sensor pmu regulator
typedef struct{
	char name[64];
	struct regulator *handle;
	int refcnt;
	int openRefcnt;
	int volt;
}camsenpwl_regulator_t;

typedef struct{
  int activeNum;
  camsenpwl_regulator_t rglt[REGULATOR_MAX];
}camsenpwl_regulator_manager_t;

static camsenpwl_regulator_manager_t gRgltMgr;

static int camsenpwl_rglt_init(IM_CHAR *rgltName)
{	
	int i;

	IM_INFOMSG((IM_STR("%s(rgltName=%s) "), IM_STR(_IM_FUNC_), rgltName));
	
	IM_ASSERT(gDev != IM_NULL);

	//
	for(i=0; i<REGULATOR_MAX; i++){
		if(gRgltMgr.rglt[i].refcnt != 0){
			if(strcmp(rgltName, gRgltMgr.rglt[i].name) == 0){
				gRgltMgr.rglt[i].refcnt++;
				IM_INFOMSG((IM_STR("%s(found same regulator(refcnt=%d)!) "), IM_STR(_IM_FUNC_), gRgltMgr.rglt[i].refcnt));
				break;
			}
		}
	}
	if(i == REGULATOR_MAX){	// no found same name.
		IM_INFOMSG((IM_STR("%s(not found same regulator!) "), IM_STR(_IM_FUNC_)));
		for(i=0; i<REGULATOR_MAX; i++){	// find a slot.
			if(gRgltMgr.rglt[i].refcnt == 0){
				break;
			}
		}
		IM_INFOMSG((IM_STR("(the %d rglt is empty!) "), i));
		IM_ASSERT(i<REGULATOR_MAX);
		gRgltMgr.rglt[i].handle = regulator_get(gDev, rgltName);
		if(IS_ERR(gRgltMgr.rglt[i].handle)){
			IM_ERRMSG((IM_STR("%s(get regulator failed: rglt=%s) "), IM_STR(_IM_FUNC_), rgltName));
			gRgltMgr.rglt[i].handle = IM_NULL;
			return -1;
		}
		IM_INFOMSG((IM_STR("%s(get regulator success: rgltName=%s) "), IM_STR(_IM_FUNC_), rgltName));
		strcpy(gRgltMgr.rglt[i].name, rgltName);
		gRgltMgr.rglt[i].refcnt = 1;
		gRgltMgr.rglt[i].openRefcnt = 0;

		gRgltMgr.activeNum++;
		IM_INFOMSG((IM_STR("(active regulator number is %d.) "), gRgltMgr.activeNum));
	}

	return 0;
}

static int camsenpwl_rglt_deinit(IM_CHAR *rgltName)
{
	int i;
	IM_INFOMSG((IM_STR("%s(rgltName=%s) "), IM_STR(_IM_FUNC_), rgltName));

	//
	for(i=0; i<REGULATOR_MAX; i++){
		if(strcmp(rgltName, gRgltMgr.rglt[i].name) == 0){
			break;
		}
	}
	if((i == REGULATOR_MAX) || (gRgltMgr.rglt[i].refcnt == 0) || (gRgltMgr.rglt[i].handle == IM_NULL))
	{
		IM_INFOMSG((IM_STR("%s( regulator %s has deinit !!!) "), IM_STR(_IM_FUNC_), rgltName));
		return 0;
	}

	//
	gRgltMgr.rglt[i].refcnt--;
	if(gRgltMgr.rglt[i].refcnt == 0){
		regulator_put(gRgltMgr.rglt[i].handle);
		gRgltMgr.rglt[i].handle = IM_NULL;
		gRgltMgr.activeNum--;
	}
	
	return 0;
}

static int camsenpwl_rglt_set(IM_CHAR *rgltName, int volt)	// volt unit is mv, if volt is 0, then disble it's output.
{
	int i;
	IM_INFOMSG((IM_STR("%s(rgltName=%s, volt=%d) "), IM_STR(_IM_FUNC_), rgltName, volt));

	for(i=0; i<REGULATOR_MAX; i++){
		if(strcmp(rgltName, gRgltMgr.rglt[i].name) == 0){
			IM_INFOMSG((IM_STR("%s(found this regulator:openRefcnt=%d!) "), IM_STR(_IM_FUNC_), gRgltMgr.rglt[i].openRefcnt));
			break;
		}
	}
	if((i == REGULATOR_MAX) || (gRgltMgr.rglt[i].refcnt == 0) || (gRgltMgr.rglt[i].handle == IM_NULL))
	{
		IM_ERRMSG((IM_STR("%s(this channel regulator has not init!) "), IM_STR(_IM_FUNC_)));
		return 0;
	}

	if(volt == 0)
	{
		IM_INFOMSG((IM_STR("%s(channel=%s disable!) "), IM_STR(_IM_FUNC_), rgltName));
		gRgltMgr.rglt[i].openRefcnt--;
		if(gRgltMgr.rglt[i].openRefcnt == 0){
			regulator_disable(gRgltMgr.rglt[i].handle);
		}
	}
	else
	{
		IM_INFOMSG((IM_STR("%s(channel=%s enable, openRefcnt=%d!) "), IM_STR(_IM_FUNC_), rgltName, gRgltMgr.rglt[i].openRefcnt));
		if(gRgltMgr.rglt[i].openRefcnt == 0){
			regulator_set_voltage(gRgltMgr.rglt[i].handle, volt, volt);
			regulator_enable(gRgltMgr.rglt[i].handle);
			gRgltMgr.rglt[i].volt = volt;
			gRgltMgr.rglt[i].openRefcnt = 1;
		}else{
			if(volt != gRgltMgr.rglt[i].volt){
				//error.
				IM_ERRMSG((IM_STR("%s(volt value is not the same with other channel!) "), IM_STR(_IM_FUNC_)));
				return -1;
			}else{
				gRgltMgr.rglt[i].openRefcnt++;
			}
		}
	}

	return 0;
}


void camsenpwl_set_pmu_handle(void *pmuHandle)
{
	gDev = (struct device *)pmuHandle;
}

IM_RET camsenpwl_pwdn(IM_CHAR *moduleName, IM_UINT32 facing)
{
	camsen_ops *ops;
	IM_UINT32 pwdn;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s(name=%s, facing=%d)++"), IM_STR(_IM_FUNC_), moduleName, facing));

	if(facing == CAM_MODULE_FACING_FRONT)
	{
		if(item_equal("camera.front.model", moduleName, 0))
		{
			pwdn = item_integer("camera.front.power_down", 1);
		}
		else
		{
			IM_ERRMSG((IM_STR("camera sensor info error!")));
			return IM_RET_FAILED;
		}
	}
	else if(facing == CAM_MODULE_FACING_BACK)
	{
		if(item_equal("camera.rear.model", moduleName, 0))
		{
			pwdn = item_integer("camera.rear.power_down", 1);
		}
		else
		{
			IM_ERRMSG((IM_STR("camera sensor info error!")));
			return IM_RET_FAILED;
		}
	}
	else
	{
		IM_ERRMSG((IM_STR("camera sensor info error!")));
		return IM_RET_FAILED;
	}

	ops = im_find_camsen(moduleName);
	if(ops == NULL)
	{
		IM_ERRMSG((IM_STR("im_find_camsen failed!")));
		return IM_RET_FAILED;
	}

	ret = ops->sen_pwdn(pwdn);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_pwdn failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_init(pwl_handle_t *pwl, IM_CHAR *name, IM_UINT32 facing, IM_BOOL checkOnly)
{
	IM_INT32 i;
	IM_CHAR str[128];
	IM_CHAR rgltName[128];
	IM_BOOL rgltInit = IM_FALSE;
	camsen_pwl_context_t *lpwl = IM_NULL;
    IM_UINT32 flashLed = 0;
	IM_RET ret = IM_RET_OK;
	IM_INT32 res = 0;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	lpwl = camsenpwl_malloc(sizeof(camsen_pwl_context_t));
	if(lpwl == IM_NULL) {
		IM_ERRMSG((IM_STR("camsenpwl_malloc(camsen_pwl_context_t) failed!")));
		return IM_RET_FAILED;
	}
	camsenpwl_memset(lpwl, 0, sizeof(camsen_pwl_context_t));
	strcpy(lpwl->name, name);
	lpwl->facing = facing;

    lpwl->ops = im_find_camsen(name);
    if(lpwl->ops == IM_NULL) {
        IM_ERRMSG((IM_STR("im_find_camsen() failed, this camera(%s) not support, please check your boarditem!"), name));
        goto Fail;
    }

    //get pmu info
    ret = lpwl->ops->sen_get_pmu_info(&lpwl->pmuInfo);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_init failed!")));
		goto Fail;
	}

	if(facing == CAM_MODULE_FACING_FRONT)
	{
#if (PMU_CTR_INTER == 1)
		//pmu regulator init&enable
		for(i=0; i<lpwl->pmuInfo.useChs; i++)
		{
			strcpy(str, "camera.front.power_");
			strcat(str, lpwl->pmuInfo.channel[i].pwName);

			IM_INFOMSG((IM_STR("%s(str=%s)"), IM_STR(_IM_FUNC_), str));
			if(item_exist(str)){
				item_string(rgltName,str,1);
				IM_INFOMSG((IM_STR("%s(rgltName=%s)"), IM_STR(_IM_FUNC_), rgltName));
				//regulator init
				res = camsenpwl_rglt_init(rgltName);
				if(res != 0)
				{
					IM_ERRMSG((IM_STR("regulator init failed!")));
					goto Fail;
				}
				//regulator enable
				camsenpwl_rglt_set(rgltName, lpwl->pmuInfo.channel[i].volt);
			}
		}
#endif

		if(item_equal("camera.front.model", name, 0))
		{
			lpwl->i2c_nr = item_integer("camera.front.ctrl", 1);
			lpwl->pwdn = item_integer("camera.front.power_down", 1);
            if(item_exist("camera.front.reset")){
                lpwl->reset = item_integer("camera.front.reset", 1);
            }else{
                //default imapx820 reset is pad 142
                lpwl->reset = 142;
            }
			IM_INFOMSG((IM_STR("camera front(%s:i2c=%d, pwdn=%d, reset=%d)"), name, lpwl->i2c_nr, lpwl->pwdn, lpwl->reset));
		}

        //flash light
        if(item_exist("camera.front.flash_led")){
            flashLed = item_integer("camera.front.flash_led", 1);
            //flash led set high
            imapx_pad_set_mode(1, 1, flashLed);
            imapx_pad_set_dir(0, 1, flashLed);
            imapx_pad_set_outdat(1, 1, flashLed);
            if(item_exist("camera.front.flash_en")){
                lpwl->flashLight = item_integer("camera.front.flash_en", 1);
                //flash light set disable
                imapx_pad_set_mode(1, 1, lpwl->flashLight);
                imapx_pad_set_dir(0, 1, lpwl->flashLight);
                imapx_pad_set_outdat(0, 1, lpwl->flashLight);
            }
        }
	}
	else if(facing == CAM_MODULE_FACING_BACK)
	{
#if (PMU_CTR_INTER == 1)
		//pmu regulator init&enable 
		for(i=0; i<lpwl->pmuInfo.useChs; i++)
		{
			strcpy(str, "camera.rear.power_");
			strcat(str, lpwl->pmuInfo.channel[i].pwName);

			if(item_exist(str)){
				item_string(rgltName,str,1);
				//regulator init
				res = camsenpwl_rglt_init(rgltName);
				if(res != 0)
				{
					IM_ERRMSG((IM_STR("regulator init failed!")));
					goto Fail;
				}
				//regulator enable
				camsenpwl_rglt_set(rgltName, lpwl->pmuInfo.channel[i].volt);
			}
		}
#endif

		if(item_equal("camera.rear.model", name, 0))
		{
			lpwl->i2c_nr = item_integer("camera.rear.ctrl", 1);
			lpwl->pwdn = item_integer("camera.rear.power_down", 1);
            if(item_exist("camera.rear.reset")){
                lpwl->reset = item_integer("camera.rear.reset", 1);
            }else{
                //default imapx820 reset is pad 142
                lpwl->reset = 142;
            }
			IM_INFOMSG((IM_STR("camera rear(%s:i2c=%d, pwdn=%d, reset=%d)"), name, lpwl->i2c_nr, lpwl->pwdn, lpwl->reset));
		}

        //flash light
        if(item_exist("camera.rear.flash_led")){
            flashLed = item_integer("camera.rear.flash_led", 1);
            //flash led set high
            imapx_pad_set_mode(1, 1, flashLed);
            imapx_pad_set_dir(0, 1, flashLed);
            imapx_pad_set_outdat(1, 1, flashLed);
            if(item_exist("camera.rear.flash_en")){
                lpwl->flashLight = item_integer("camera.rear.flash_en", 1);
                //flash light set disable
                imapx_pad_set_mode(1, 1, lpwl->flashLight);
                imapx_pad_set_dir(0, 1, lpwl->flashLight);
                imapx_pad_set_outdat(0, 1, lpwl->flashLight);
            }
        }
    }
    else
    {
        IM_ERRMSG((IM_STR("camera sensor info of board item error!")));
        goto Fail;
	}
	rgltInit = IM_TRUE;
	IM_INFOMSG((IM_STR("%s(i2c_get_adapter before: number=%d"), IM_STR(_IM_FUNC_), lpwl->i2c_nr));
	lpwl->adapter = i2c_get_adapter(lpwl->i2c_nr);
	if (!lpwl->adapter) {
		IM_ERRMSG((IM_STR("i2c get adapter failed!")));
	}
	IM_INFOMSG((IM_STR("%s(i2c_get_adapter after: adapter=0x%x"), IM_STR(_IM_FUNC_), (IM_UINT32)lpwl->adapter));

	ret = lpwl->ops->sen_init((pwl_handle_t)lpwl, checkOnly);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_init failed!")));
		goto Fail;
	}
	*pwl = (pwl_handle_t)lpwl;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;

Fail:
	if(rgltInit == IM_TRUE)
	{
		if(facing == CAM_MODULE_FACING_FRONT)
		{
#if (PMU_CTR_INTER == 1)
			//pmu regulator deinit&disable
			for(i=0; i<lpwl->pmuInfo.useChs; i++)
			{
				strcpy(str, "camera.front.power_");
				strcat(str, lpwl->pmuInfo.channel[i].pwName);

				if(item_exist(str)){
					item_string(rgltName,str,1);
					IM_INFOMSG((IM_STR("%s(rgltName=%s)"), IM_STR(_IM_FUNC_), rgltName));
					//regulator disable
					camsenpwl_rglt_set(rgltName, 0);
					//regulator deinit
					camsenpwl_rglt_deinit(rgltName);
				}
			}
#endif
            //flash light close
            if(item_exist("camera.front.flash_led")){
                flashLed = item_integer("camera.front.flash_led", 1);
                //flash led set low
                imapx_pad_set_mode(1, 1, flashLed);
                imapx_pad_set_dir(0, 1, flashLed);
                imapx_pad_set_outdat(0, 1, flashLed);
                if(item_exist("camera.front.flash_en")){
                    lpwl->flashLight = item_integer("camera.front.flash_en", 1);
                    //flash light set disable
                    imapx_pad_set_mode(1, 1, lpwl->flashLight);
                    imapx_pad_set_dir(0, 1, lpwl->flashLight);
                    imapx_pad_set_outdat(0, 1, lpwl->flashLight);
                }
            }
		}
		else if(facing == CAM_MODULE_FACING_BACK)
		{
#if (PMU_CTR_INTER == 1)
			//pmu regulator deinit&disable
			for(i=0; i<lpwl->pmuInfo.useChs; i++)
			{
				strcpy(str, "camera.rear.power_");
				strcat(str, lpwl->pmuInfo.channel[i].pwName);

				if(item_exist(str)){
					item_string(rgltName,str,1);
					//regulator disable
					camsenpwl_rglt_set(rgltName, 0);
					//regulator deinit
					camsenpwl_rglt_deinit(rgltName);
				}
			}
#endif
            //flash light close
            if(item_exist("camera.rear.flash_led")){
                flashLed = item_integer("camera.rear.flash_led", 1);
                //flash led set high
                imapx_pad_set_mode(1, 1, flashLed);
                imapx_pad_set_dir(0, 1, flashLed);
                imapx_pad_set_outdat(0, 1, flashLed);
                if(item_exist("camera.rear.flash_en")){
                    lpwl->flashLight = item_integer("camera.rear.flash_en", 1);
                    //flash light set disable
                    imapx_pad_set_mode(1, 1, lpwl->flashLight);
                    imapx_pad_set_dir(0, 1, lpwl->flashLight);
                    imapx_pad_set_outdat(0, 1, lpwl->flashLight);
                }
            }
		}
	}

	if(lpwl != IM_NULL)
	{
		camsenpwl_free((void*)lpwl);
	}

	return IM_RET_FAILED;
}

IM_RET camsenpwl_deinit(pwl_handle_t pwl)
{
	IM_INT32 i;
	IM_CHAR str[128];
	IM_CHAR rgltName[128];
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
    IM_UINT32 flashLed;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
	ret = lpwl->ops->sen_deinit();
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_deinit failed!")));
		return IM_RET_FAILED;
	}

	//pmu disable&deinit
	if(lpwl->facing == CAM_MODULE_FACING_FRONT)
	{
#if (PMU_CTR_INTER == 1)
		//pmu regulator init&enable
		for(i=0; i<lpwl->pmuInfo.useChs; i++)
		{
			strcpy(str, "camera.front.power_");;
			strcat(str, lpwl->pmuInfo.channel[i].pwName);

			if(item_exist(str)){
				item_string(rgltName,str,1);
				//regulator disable
				camsenpwl_rglt_set(rgltName, 0);
				//regulator deinit
				camsenpwl_rglt_deinit(rgltName);
			}

		}
#endif
        //flash light close
        if(item_exist("camera.front.flash_led")){
            flashLed = item_integer("camera.front.flash_led", 1);
            //flash led set low
            imapx_pad_set_mode(1, 1, flashLed);
            imapx_pad_set_dir(0, 1, flashLed);
            imapx_pad_set_outdat(0, 1, flashLed);
            if(item_exist("camera.front.flash_en")){
                //lpwl->flashLight = item_integer("camera.front.flash_en", 1);
                //flash light set disable
                imapx_pad_set_mode(1, 1, lpwl->flashLight);
                imapx_pad_set_dir(0, 1, lpwl->flashLight);
                imapx_pad_set_outdat(0, 1, lpwl->flashLight);
            }
        }
	}
	else if(lpwl->facing == CAM_MODULE_FACING_BACK)
	{
#if (PMU_CTR_INTER == 1)
		//pmu regulator init&enable 
		for(i=0; i<lpwl->pmuInfo.useChs; i++)
		{
			strcpy(str, "camera.rear.power_");;
			strcat(str, lpwl->pmuInfo.channel[i].pwName);

			if(item_exist(str)){
				item_string(rgltName,str,1);
				//regulator disable
				camsenpwl_rglt_set(rgltName, 0);
				//regulator deinit
				camsenpwl_rglt_deinit(rgltName);
			}
		}
#endif
        //flash light close
        if(item_exist("camera.rear.flash_led")){
            flashLed = item_integer("camera.rear.flash_led", 1);
            //flash led set high
            imapx_pad_set_mode(1, 1, flashLed);
            imapx_pad_set_dir(0, 1, flashLed);
            imapx_pad_set_outdat(0, 1, flashLed);
            if(item_exist("camera.rear.flash_en")){
                //lpwl->flashLight = item_integer("camera.rear.flash_en", 1);
                //flash light set disable
                imapx_pad_set_mode(1, 1, lpwl->flashLight);
                imapx_pad_set_dir(0, 1, lpwl->flashLight);
                imapx_pad_set_outdat(0, 1, lpwl->flashLight);
            }
        }
	}

	camsenpwl_free(lpwl);

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_start(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_start();
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_start failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_stop(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_stop();
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_stop failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_get_caps(pwl_handle_t pwl, camsen_caps_t *caps)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_get_caps(caps);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_get_caps failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_get_out_mode(pwl_handle_t pwl, camsen_out_mode_t *outMode)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_get_out_mode(outMode);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_get_out_mode failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_set_out_mode(pwl_handle_t pwl, camsen_out_mode_t *outMode)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_set_out_mode(outMode);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_set_out_mode failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}


IM_RET camsenpwl_get_property(pwl_handle_t pwl, IM_UINT32 property, void *p)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_get_property(property, p);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_get_property failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_set_property(pwl_handle_t pwl, IM_UINT32 property, void *p)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	ret = lpwl->ops->sen_set_property(property, p);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("sen_set_property failed!")));
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_clock_enable(pwl_handle_t pwl, IM_UINT32 clkfrq)
{
	IM_UINT32 temp;
	IM_UINT32 res;
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	
#if 1
	//camo clock gate enable
	temp = __raw_readl(IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));
	temp |= 1<<CLKGATE_CAMOUT;
	__raw_writel(temp, IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));

	lpwl->oclk = clk_get(NULL, "camo");
#else //only for devboard mipi camera test
    camsenpwl_io_set_mode(73, 0); //set function mode
	lpwl->oclk = clk_get(NULL, "clk-out0"); 
#endif
	if(lpwl->oclk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get cam oclk failed!")));
		return IM_RET_FAILED;
	}

	res = clk_set_rate(lpwl->oclk,clkfrq/1000); 
	if (res < 0){
		IM_ERRMSG((IM_STR("cam oclk set rate failed! ")));
		return IM_RET_FAILED;
	}
	lpwl->oclkfrq = (IM_UINT32)clk_get_rate(lpwl->oclk);

	IM_INFOMSG((IM_STR("%s(request clock frq=%d, provided clock frq=%d)"), IM_STR(_IM_FUNC_), clkfrq, lpwl->oclkfrq));

	if(clk_enable(lpwl->oclk) != 0)
	{
		IM_ERRMSG((IM_STR("cam oclk enable failed!")));
		clk_put(lpwl->oclk);
		return IM_RET_FAILED;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsenpwl_clock_disable(pwl_handle_t pwl)
{
	IM_UINT32 temp;
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	if(lpwl->oclk != IM_NULL)
	{
		clk_disable(lpwl->oclk);

		clk_put(lpwl->oclk);

		lpwl->oclk = IM_NULL;
	}

	//camo clock gate disable
	temp = __raw_readl(IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));
	temp &= ~(1<<CLKGATE_CAMOUT);
	__raw_writel(temp, IO_ADDRESS(ISP_CLOCKGATE_ADDRESS));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_UINT32 camsenpwl_clock_get_freq(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));
	return lpwl->oclkfrq;
}

IM_UINT32 camsenpwl_get_pwdn_padnum(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return lpwl->pwdn;
}

IM_UINT32 camsenpwl_get_reset_padnum(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return lpwl->reset;
}

IM_UINT32 camsenpwl_get_flash_light_padnum(pwl_handle_t pwl)
{
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return lpwl->flashLight;
}

#define I2C_FAULT_TOLERANCE_TIMES 5

IM_INT32 camsenpwl_i2c_read(pwl_handle_t pwl, IM_UINT8 *buf,
		IM_UINT8 *addr, IM_UINT32 size, IM_UINT32 len)
{
    IM_INT32 i2c_num = 0;
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	IM_INFOMSG((IM_STR("%s(i2c_device_addr=0x%x, adapter=0x%x"), IM_STR(_IM_FUNC_), lpwl->ops->i2c_dev_addr, (IM_INT32)lpwl->adapter));
	struct i2c_msg msgs[] = {{.addr = lpwl->ops->i2c_dev_addr, .flags = I2C_M_NOSTART,
		.len = size, .buf = addr}, {.addr = lpwl->ops->i2c_dev_addr,
			.flags = I2C_M_RD, .len = len, .buf = buf}};

	if(lpwl->adapter)
    {
		if(2 == i2c_transfer(lpwl->adapter, msgs, 2))
        {
			return 0;
        }
        else
        {
            do{
                if(2 == i2c_transfer(lpwl->adapter, msgs, 2))
                    return 0;
                i2c_num++;
                IM_TIPMSG((IM_STR("%s(i2c read error!!)"), IM_STR(_IM_FUNC_)));
            }while(i2c_num < I2C_FAULT_TOLERANCE_TIMES);
        }
    }
	return -1;
}

IM_INT32 camsenpwl_i2c_write(pwl_handle_t pwl, IM_UINT8 *buf, IM_UINT32 len)
{
    IM_INT32 i2c_num = 0;
	camsen_pwl_context_t *lpwl = (camsen_pwl_context_t*)pwl;
	struct i2c_msg msgs[] = {{.addr = lpwl->ops->i2c_dev_addr, .flags= 0,
		.len = len, .buf = buf} };

	if(lpwl->adapter)
    {
		if(1 == i2c_transfer(lpwl->adapter, msgs, 1))
        {
			return 0;
        }
        else
        {
            do{
                if(2 == i2c_transfer(lpwl->adapter, msgs, 2))
                    return 0;
                i2c_num++;
                IM_TIPMSG((IM_STR("%s(i2c write error!!)"), IM_STR(_IM_FUNC_)));
            }while(i2c_num < I2C_FAULT_TOLERANCE_TIMES);
        }
    }
	return -1;
}

IM_INT32 camsenpwl_io_set_mode(IM_INT32 index, IM_UINT32 mode)
{
	IM_INFOMSG((IM_STR("%s(index=%d, mode=%d)"), IM_STR(_IM_FUNC_), index, mode));
	return imapx_pad_set_mode(mode, 1, index);
}

IM_INT32 camsenpwl_io_set_dir(IM_INT32 index, IM_UINT32 dir)
{
	IM_INFOMSG((IM_STR("%s(index=%d, dir=%d)"), IM_STR(_IM_FUNC_), index, dir));
	return imapx_pad_set_dir(dir, 1, index);
}

IM_INT32 camsenpwl_io_set_outdat(IM_INT32 index, IM_UINT32 data)
{
	IM_INFOMSG((IM_STR("%s(index=%d, outdat=%d)"), IM_STR(_IM_FUNC_), index, data));
	return imapx_pad_set_outdat(data, 1, index);
}



void *camsenpwl_malloc(IM_UINT32 size)
{
	return kmalloc(size,GFP_KERNEL);
}

void camsenpwl_free(void *p)
{
	if(p != IM_NULL) kfree(p);
}

void camsenpwl_memcpy(void *dst, void *src, IM_UINT32 size)
{
	memcpy(dst, src, size);
}

void camsenpwl_memset(void *dst, IM_INT8 c, IM_UINT32 size)
{
	memset(dst, c, size);
}

static int __init imapx_camera_sensor_init(void)
{
	printk("####imapx-camera-sensors-init####\n");
	return 0;
}

static void __exit imapx_camera_sensor_exit(void)
{
	printk("####imapx-camera-sensors-exit####\n");
}

//module_init(imapx_camera_sensor_init);
late_initcall(imapx_camera_sensor_init);
module_exit(imapx_camera_sensor_exit);
