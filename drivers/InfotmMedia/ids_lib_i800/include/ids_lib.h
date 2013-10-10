/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:  Header of ids lib. in kernel call 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** v1.0.1	Sam@2012/3/20: first commit.
** v1.0.6	leo@2012/6/18: add suspend&resume.
** v1.0.7	leo@2012/08/11: readjust.
**
*****************************************************************************/


#ifndef __IDS_LIB_H__
#define __IDS_LIB_H__


//
//
//
typedef void * wlayer_handle_t;
typedef void * extlayer_handle_t;
typedef void * vmode_handle_t;
typedef void * ie_handle_t;
typedef void * fblayer_handle_t;

#define IDSLIB_INST_WLAYER		1
#define IDSLIB_INST_EXTLAYER	2
#define IDSLIB_INST_VMODE		3
#define IDSLIB_INST_IE			4


//
//
//
#define IDSLIB_PIXFMT_16BPP_RGB565		5
#define IDSLIB_PIXFMT_32BPP_RGB0888		11
#define IDSLIB_PIXFMT_32BPP_ARGBA888	16
#define IDSLIB_PIXFMT_12BPP_YUV420SP	17

typedef struct{
	IM_INT32	pixfmt;
	IM_INT32	width;
	IM_INT32	height;
}idslib_fb_info_t;

//
// 
//
#define WLAYER_RES_RAMPAL		0x1	// param: [0:0]rampal, range 0--1.
#define WLAYER_RES_YUVtoRGB_MATRIX	0x2	// param: no use.
#define WLAYER_RES_MMU			0x4	// param: no use.


//
// vmode event, caution, these combined with parts of event defined in IM_idsapi.h.
//
#define IDS_VMODE_EVENT_LOCDEV_UPDATE	0x2001
#define IDS_VMODE_EVENT_LOCDEV_BREAK	0x2002

typedef struct{
	IM_UINT32	evt;
	IM_UINT32	param[64];	// cast to appropriate evt "p" pointer.
}idslib_event_t;

typedef void (*idslib_func_idsdrv_listener_t)(IM_INT32 mask, IM_INT32 idsx, IM_INT32 wx, idslib_event_t *vmevt);

#define IDSLIB_VMEVT_MASK_EXTLAYER	0x1
#define IDSLIB_VMEVT_MASK_VMODE		0x2
#define IDSLIB_VMEVT_MASK_IE		0x4

//
// display device.
//
#define DISPDEV_EVT_CONFIG_UPDATE	0x1	// p=dispdev_evt_config_update_t *, only use to dynamic connect device, for example, HDMI.
#define DISPDEV_EVT_BREAK		0x2	// p=IM_NULL.

typedef struct{
    const IM_TCHAR *idstr;
    IM_UINT32 devid;
    void *priv;
}dispdev_cfg_t;

typedef struct{
	IM_INT32 num;
	ids_display_device_t *devs;
}dispdev_evt_config_update_t;

#define DISPDEV_DATAPORT_RGB888		0x0
#define DISPDEV_DATAPORT_RGB565		0x1
#define DISPDEV_DATAPORT_RGB666		0x2

#define DISPDEV_DATAPORT_ITU601_24	0x3  //RGB888
#define DISPDEV_DATAPORT_ITU601_16	0x4
#define DISPDEV_DATAPORT_ITU601_8	0x5
#define DISPDEV_DATAPORT_ITU656_8	0x6
typedef struct{
	IM_INT32	width;
	IM_INT32 	height;
	IM_INT32    dataport; // DISPDEV_DATAPORT_xxx.
    IM_INT32    dftacm;

	IM_INT32	hspw;
	IM_INT32	hbpd;
	IM_INT32	hfpd;
	IM_INT32	vspw;
	IM_INT32	vbpd;
	IM_INT32	vfpd;
	IM_INT32	fpsx1000;

	// plates size , in millimeter unit
	IM_UINT32 	phyWidth;
	IM_UINT32 	phyHeight;
}dispdev_info_t;

typedef void * dispdev_handle_t;
typedef IM_RET (*fcbk_dispdev_listener_t)(dispdev_handle_t handle, IM_UINT32 evt, void *p);

typedef IM_RET (*func_dintf_init)(dispdev_handle_t *handle, IM_INT32 idsx);
typedef IM_RET (*func_dintf_deinit)(dispdev_handle_t handle);
typedef IM_RET (*func_dintf_set_config)(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix);
typedef IM_RET (*func_dintf_get_config)(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix);
typedef IM_RET (*func_dintf_open)(dispdev_handle_t handle);
typedef IM_RET (*func_dintf_close)(dispdev_handle_t handle);
typedef IM_RET (*func_dintf_register_listener)(dispdev_handle_t handle, fcbk_dispdev_listener_t listener);
typedef IM_RET (*func_dintf_suspend)(dispdev_handle_t handle);
typedef IM_RET (*func_dintf_resume)(dispdev_handle_t handle);

typedef struct{
	func_dintf_init			device_init;
	func_dintf_deinit		device_deinit;
	func_dintf_set_config		device_set_config;
	func_dintf_get_config		device_get_config;
	func_dintf_open			device_open;
	func_dintf_close		device_close;
	func_dintf_register_listener	device_register_listener;
	func_dintf_suspend		device_suspend;
	func_dintf_resume		device_resume;
}dispdev_interface_t;


//
//
//
#define IDSLIB_BUS_EFFIC_LEVEL_1       1
#define IDSLIB_BUS_EFFIC_LEVEL_2       2
#define IDSLIB_BUS_EFFIC_LEVEL_3       3
#define IDSLIB_BUS_EFFIC_LEVEL_4       4
#define IDSLIB_BUS_EFFIC_LEVEL_5       5

#define IDSLIB_WORKMODE_IDLE           0
#define IDSLIB_WORKMODE_1080P_32BIT    0x1     // 1080p >= ? > 720p
#define IDSLIB_WORKMODE_720P_32BIT     0x2     // 720p >= ? > 576p
#define IDSLIB_WORKMODE_576P_32BIT     0x3     // 576p >= ? > VGA
#define IDSLIB_WORKMODE_VGA_32BIT     0x4     // VGA >= ?
#define IDSLIB_WORKMODE_1080P_16BIT    0x11    // 1080p >= ? > 720p
#define IDSLIB_WORKMODE_720P_16BIT     0x12    // 720p >= ? > 576p
#define IDSLIB_WORKMODE_576P_16BIT     0x13    // 576p >= ? > VGA
#define IDSLIB_WORKMODE_VGA_16BIT     0x14    // VGA >= ?


//
//
//
#define IDSLIB_DEV_CONFIG_MAX		8

typedef struct{
	// global.
	IM_INT32		locidsx;	// 0, 1.

	// framebuffer.
	IM_BOOL			resvFblayer;	// if reserved a wlayer for fblayer.
	IM_INT32		pixfmt;			// IDSLIB_PIXFMT_xxx
	IM_INT32		fbWidth;
	IM_INT32		fbHeight;

	// devices.
	IM_INT32		locDevNum;
	IM_INT32		locDevPrefer;	// 0 to locDevNum-1.
	ids_display_device_t	locDevs[IDSLIB_DEV_CONFIG_MAX];
	IM_INT32		extDevNum;
	IM_INT32		extDevPrefer;	// 0 to extDevPrefer-1.
	ids_display_device_t	extDevs[IDSLIB_DEV_CONFIG_MAX];

    // bus efficiency.
    IM_INT32        busEffic;   // IDSLIB_BUS_EFFIC_xxx.
}idslib_init_config_t;

//
//
//
#define IDSLIB_SUPPORT_DEVS_MAX		16	// supported max devices in same time.


//
//
//
IM_UINT32 idslib_version(void);
IM_RET idslib_init(idslib_init_config_t *config);
IM_RET idslib_deinit(void);
IM_RET idslib_get_local_ids(IM_INT32 *idsx);
IM_RET idslib_vsync(IM_INT32 idsx);
IM_RET idslib_suspend(void);
IM_RET idslib_resume(void);

IM_RET idslib_extlayer_init(extlayer_handle_t *handle, idslib_func_idsdrv_listener_t listener);
IM_RET idslib_extlayer_deinit(extlayer_handle_t handle);

IM_RET idslib_wlayer_init(wlayer_handle_t *handle, IM_INT32 idsx, IM_INT32 wx);
IM_RET idslib_wlayer_deinit(wlayer_handle_t handle);
IM_RET idslib_wlayer_query_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param);
IM_RET idslib_wlayer_reserve_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param);
IM_RET idslib_wlayer_release_resource(wlayer_handle_t handle, IM_INT32 res, IM_INT32 param);
IM_RET idslib_wlayer_open(wlayer_handle_t handle);
IM_RET idslib_wlayer_close(wlayer_handle_t handle);
IM_RET idslib_wlayer_vsync(wlayer_handle_t handle);
IM_RET idslib_wlayer_get_device(wlayer_handle_t handle, ids_display_device_t *dev, dispdev_info_t *info, ids_wlayer_canvas_t *canvas);
IM_RET idslib_wlayer_set_panorama(wlayer_handle_t handle, IM_INT32 pixfmt, ids_wlayer_panorama_t *pano);
IM_RET idslib_wlayer_set_buffer(wlayer_handle_t handle, IM_Buffer *buffer, IM_Buffer *bufferUv);
IM_RET idslib_wlayer_set_region(wlayer_handle_t handle, ids_wlayer_region_t *region);
IM_RET idslib_wlayer_set_position(wlayer_handle_t handle, ids_wlayer_position_t *pos);
IM_RET idslib_wlayer_set_mapclr(wlayer_handle_t handle, IM_BOOL enable, IM_INT32 clr);
IM_RET idslib_wlayer_set_alpha(wlayer_handle_t handle, ids_wlayer_alpha_t *alpha);
IM_RET idslib_wlayer_set_clrkey(wlayer_handle_t handle, IM_BOOL enable, IM_BOOL matchForeground, IM_INT32 clr);

IM_RET idslib_fblayer_init(fblayer_handle_t *handle);
IM_RET idslib_fblayer_deinit(fblayer_handle_t handle);
IM_RET idslib_fblayer_open(fblayer_handle_t handle);
IM_RET idslib_fblayer_close(fblayer_handle_t handle);
IM_RET idslib_fblayer_swap_buffer(fblayer_handle_t handle, IM_Buffer *buffer);
IM_RET idslib_fblayer_vsync(fblayer_handle_t handle);
IM_RET idslib_fblayer_get_device(fblayer_handle_t handle, ids_display_device_t *dev, dispdev_info_t *info, ids_wlayer_canvas_t *canvas);

IM_RET idslib_vmode_init(vmode_handle_t *handle, idslib_func_idsdrv_listener_t listener);
IM_RET idslib_vmode_deinit(vmode_handle_t handle);
IM_RET idslib_vmode_get_mode(vmode_handle_t handle, ids_vmode_config_t *config);
IM_RET idslib_vmode_set_mode(vmode_handle_t handle, IM_UINT32 mode);
IM_RET idslib_vmode_get_devs(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 *num, ids_display_device_t *devs);
IM_RET idslib_vmode_set_dev(vmode_handle_t handle, IM_INT32 idsx, IM_INT32 devtype, IM_INT32 devid);
IM_RET idslib_vmode_set_canvas(vmode_handle_t handle, IM_INT32 idsx, ids_wlayer_canvas_t *canvas);

IM_RET idslib_ie_init(ie_handle_t *handle, IM_INT32 idsx);
IM_RET idslib_ie_deinit(ie_handle_t handle);
IM_RET idslib_ie_acc_set(ie_handle_t handle, IM_INT32 val);
IM_RET idslib_ie_acc_get(ie_handle_t handle, IM_INT32 *val);
IM_RET idslib_ie_acm_set(ie_handle_t handle, IM_INT32 val);
IM_RET idslib_ie_acm_get(ie_handle_t handle, IM_INT32 *val);
IM_RET idslib_ie_gamma_set(ie_handle_t handle, IM_INT32 val);
IM_RET idslib_ie_gamma_get(ie_handle_t handle, IM_INT32 *val);
IM_RET idslib_ie_ee_set(ie_handle_t handle, IM_INT32 val);
IM_RET idslib_ie_ee_get(ie_handle_t handle, IM_INT32 *val);

IM_RET idslib_check_hdmi_on(IM_UINT32 *hdmiOn);

//
// ds=dataStruct
//
#define IDSLIB_IOCTL_VERSION			0x1000	// idslib_version(), ds=IM_UINT32 *
#define IDSLIB_IOCTL_GET_LOCAL_IDS		0x1001	// idslib_get_local_ids(), ds=IM_INT32 *
#define IDSLIB_IOCTL_VSYNC				0x1002	// idslib_get_local_ids(), ds=IM_INT32 *

#define IDSLIB_IOCTL_EXTLAYER_INIT		0x1100
#define IDSLIB_IOCTL_EXTLAYER_DEINIT	0x1101

#define IDSLIB_IOCTL_WLAYER_INIT		0x2000	// idslib_wlayer_init(), ds=ioctl_ds_wlayer_init_t *
#define IDSLIB_IOCTL_WLAYER_DEINIT		0x2001	// idslib_wlayer_deinit()
#define IDSLIB_IOCTL_WLAYER_OPEN		0x2002	// idslib_wlayer_open()
#define IDSLIB_IOCTL_WLAYER_CLOSE		0x2003	// idslib_wlayer_close()
#define IDSLIB_IOCTL_WLAYER_GET_DEVICE		0x2004	// idslib_wlayer_get_device(), ds=ioctl_ds_wlayer_get_device_t *
#define IDSLIB_IOCTL_WLAYER_RESERVE_RESOURCE	0x2005	// idslib_wlayer_reserve_resource(), ds=ioctl_ds_resource_t *
#define IDSLIB_IOCTL_WLAYER_RELEASE_RESOURCE	0x2006	// idslib_wlayer_release_resource(), ds=ioctl_ds_resource_t *
#define IDSLIB_IOCTL_WLAYER_SET_PANORAMA		0x2007  // idslib_wlayer_set_panorama(), ds=ioctl_ds_panorama_t*
#define IDSLIB_IOCTL_WLAYER_SET_REGION			0x2008  // idslib_wlayer_set_region(), ds=ioctl_ds_region_t*
#define IDSLIB_IOCTL_WLAYER_SET_BUFFER			0x2009  // idslib_wlayer_set_buffer(), ds=ioctl_ds_buffer_t*
#define IDSLIB_IOCTL_WLAYER_SET_POSITION		0x2010  // idslib_wlayer_set_position(), ds=ioctl_ds_position_t*
#define IDSLIB_IOCTL_WLAYER_SET_MAPCLR			0x2011  // idslib_wlayer_set_mapclr(), ds=ioctl_ds_mapclr_t*
#define IDSLIB_IOCTL_WLAYER_SET_ALPHA			0x2012  // idslib_wlayer_set_alpha(), ds=ioctl_ds_alpha_t*
#define IDSLIB_IOCTL_WLAYER_SET_CLRKEY			0x2013  // idslib_wlayer_set_clrkey(), ds=ioctl_ds_clrkey_t*
#define IDSLIB_IOCTL_WLAYER_QUERY_RESOURCE		0x2014	// idslib_wlayer_query_resource(), ds=ioctl_ds_resource_t *


#define IDSLIB_IOCTL_VMODE_INIT			0x3000	// idslib_vmode_init()
#define IDSLIB_IOCTL_VMODE_DEINIT		0x3001	// idslib_vmode_deinit()
#define IDSLIB_IOCTL_VMODE_GET_MODE		0x3002	// idslib_vmode_get_mode(), ds=ids_vmode_config_t * 
#define IDSLIB_IOCTL_VMODE_SET_MODE		0x3003	// idslib_vmode_set_mode(), ds=ioctl_ds_vmode_set_mode_t *
#define IDSLIB_IOCTL_VMODE_GET_DEVS		0x3004	// idslib_vmode_get_devs(), ds=ioctl_ds_vmode_get_devs_t *
#define IDSLIB_IOCTL_VMODE_SET_DEV		0x3005	// idslib_vmode_set_dev(), ds=ioctl_ds_vmode_set_dev_t *
#define IDSLIB_IOCTL_VMODE_SET_CANVAS		0x3006	// idslib_vmode_set_canvas(), ds=ioctl_ds_vmode_set_canvas_t *

#define IDSLIB_IOCTL_IE_INIT			0x4000	// idslib_ie_init(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_DEINIT			0x4001	// idslib_ie_deinit()
#define IDSLIB_IOCTL_IE_ACC_SET 		0x4002	// idslib_ie_acc_set(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_ACC_GET 		0x4003	// idslib_ie_acc_get(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_ACM_SET 		0x4004	// idslib_ie_acm_set(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_ACM_GET 		0x4005	// idslib_ie_acm_get(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_GAMMA_SET 		0x4006	// idslib_ie_gamma_set(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_GAMMA_GET		0x4007	// idslib_ie_gamma_get(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_EE_SET 			0x4008	// idslib_ie_ee_set(), ds=IM_INT32 *
#define IDSLIB_IOCTL_IE_EE_GET 			0x4009	// idslib_ie_ee_get(), ds=IM_INT32 *

#define IDSLIB_IOCTL_BUFFER_REALLOC_CHECK 	0x5000
#define IDSLIB_IOCTL_BANK_CHECK			0x5001
#define IDSLIB_IOCTL_SHOW_LOCAL			0x5002

#define IDSLIB_IOCTL_REGISTER_INSTANCE		0x8000	// ds=IM_INT32 *, IDSLIB_INST_xxx.
#define IDSLIB_IOCTL_GET_EVENT			0x8001	// ds=idslib_cb_event_t *

typedef struct{
	ids_display_device_t	dev; // [OUT]
	dispdev_info_t	info; // [OUT]
	ids_wlayer_canvas_t	canvas; // [OUT]
}ioctl_ds_wlayer_get_device_t;

typedef struct{
	IM_INT32 	idsx; // [IN]
	IM_INT32	wx; // [IN]
	IM_INT32 	pixFormat;
	IM_RET		ret; // [OUT]
}ioctl_ds_wlayer_init_t;

typedef struct{
	IM_INT32	res; // [IN]
	IM_INT32	param; // [IN]
	IM_RET		ret; // [OUT]
}ioctl_ds_resource_t;

typedef struct{
	IM_UINT32	mode; // [IN]
	IM_RET		ret; // [OUT] IM_RET_OK, IM_RET_FALSE needs to async wait.
}ioctl_ds_vmode_set_mode_t;

typedef struct{
	IM_INT32	idsx; // [IN]
	IM_INT32	num; // [OUT]
	ids_display_device_t	devs[IDSLIB_SUPPORT_DEVS_MAX]; // [OUT]
}ioctl_ds_vmode_get_devs_t;

typedef struct{
	IM_INT32	idsx; // [IN]
	IM_INT32	devtype; // [IN]
	IM_INT32	devid; // [IN]
}ioctl_ds_vmode_set_dev_t;

typedef struct{
	IM_INT32 idsx; // [IN]
	ids_wlayer_canvas_t	canvas; // [IN]
	IM_RET ret; // [OUT]
}ioctl_ds_vmode_set_canvas_t;

typedef struct{
	ids_wlayer_panorama_t panorama;
	IM_RET ret;
}ioctl_ds_panorama_t;

typedef struct{
	ids_wlayer_region_t region;
	IM_RET ret;
}ioctl_ds_region_t;

typedef struct{
	IM_Buffer buffer;
	IM_Buffer bufferUv;
	IM_RET ret;
}ioctl_ds_buffer_t;

typedef struct{
	ids_wlayer_position_t position;
	IM_RET ret;
}ioctl_ds_position_t;

typedef struct{
	ids_wlayer_mapclr_t mapclr;
	IM_RET ret;
}ioctl_ds_mapclr_t;

typedef struct{
	ids_wlayer_alpha_t alpha;
	IM_RET ret;
}ioctl_ds_alpha_t;

typedef struct{
	IM_BOOL enable;
	IM_BOOL matchForeground;
	IM_UINT32 clr;
	IM_RET ret;
}ioctl_ds_clrkey_t;

typedef struct{
	IM_BOOL bankNeeds;
	IM_UINT32 size;
	IM_Buffer bufA;
	IM_Buffer bufB;
}bank_check;

#endif	// __IDS_LIB_H__

