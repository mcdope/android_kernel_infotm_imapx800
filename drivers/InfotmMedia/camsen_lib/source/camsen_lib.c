/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: camsen_lib.c
--
--  Description :
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/05: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/16: add api of pmu set handle and power down.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMSEN_LIB_I:"
#define WARNHEAD	"CAMSEN_LIB_W:"
#define ERRHEAD		"CAMSEN_LIB_E:"
#define TIPHEAD		"CAMSEN_LIB_T:"


IM_UINT32 camsen_version(void)
{
	return IM_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_PATCH);
}

void camsen_set_pmu_handle(void *pmuHandle)
{
	camsenpwl_set_pmu_handle(pmuHandle);
}

IM_RET camsen_pwdn(IM_CHAR *moduleName, IM_UINT32 facing)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s(name=%s, facing=%d)"), IM_STR(_IM_FUNC_), moduleName, facing));
	ret = camsenpwl_pwdn(moduleName, facing);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_pwdn failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_init(camsen_handle_t *handle, IM_CHAR *moduleName, IM_UINT32 facing, IM_BOOL checkOnly)
{
	camsen_context_t *sen = IM_NULL;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s(moduleName=%s, facing=%d)++"), IM_STR(_IM_FUNC_), moduleName, facing));

	sen = (camsen_context_t *)camsenpwl_malloc(sizeof(camsen_context_t));
	if(sen == IM_NULL) {
		IM_ERRMSG((IM_STR("camsenpwl_malloc(camsen_context_t) failed!")));
		return IM_RET_FAILED;
	}
	camsenpwl_memset(sen, 0, sizeof(camsen_context_t));
	strcpy(sen->name, moduleName);
	
	ret = camsenpwl_init(&sen->pwl, moduleName, facing, checkOnly);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_init failed!")));
		camsenpwl_free((void*)sen);
		return IM_RET_FAILED;
	}

	*handle = (camsen_handle_t)sen;
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));
	return ret;
}

IM_RET camsen_deinit(camsen_handle_t handle)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_deinit(sen->pwl);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_deinit failed!")));
		return IM_RET_FAILED;
	}

	camsenpwl_free(sen);

	return ret;
}

IM_RET camsen_start(camsen_handle_t handle)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_start(sen->pwl);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_start failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_stop(camsen_handle_t handle)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_stop(sen->pwl);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_stop failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_get_caps(camsen_handle_t handle, camsen_caps_t *caps)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_get_caps(sen->pwl, caps);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_get_caps failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_get_out_mode(camsen_handle_t handle, camsen_out_mode_t *outMode)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_get_out_mode(sen->pwl, outMode);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_get_out_mode failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_set_out_mode(camsen_handle_t handle, camsen_out_mode_t *outMode)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_set_out_mode(sen->pwl, outMode);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_set_out_mode failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}



IM_RET camsen_get_property(camsen_handle_t handle, IM_UINT32 property, void *p)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_get_property(sen->pwl, property, p);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_get_caps failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}

IM_RET camsen_set_property(camsen_handle_t handle, IM_UINT32 property, void *p)
{
	camsen_context_t *sen = (camsen_context_t *)handle;
	IM_RET ret = IM_RET_OK;
	IM_ASSERT(sen != IM_NULL);
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));

	ret = camsenpwl_set_property(sen->pwl, property, p);
	if(ret != IM_RET_OK) {
		IM_ERRMSG((IM_STR("camsenpwl_set_property failed!")));
		return IM_RET_FAILED;
	}

	return ret;
}
