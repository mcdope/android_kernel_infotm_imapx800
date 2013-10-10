/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of IDS APIs . It is used for user space use.
**
** 关于显示模式，作如下约定：
** 1. 模式间的切换都必须以本地模式为起点或终点；
** 2. 任何显示设备的设置都必须是在本地模式进行；
** 3. canvas的约束，宽是16的倍数，高是4的倍数，这样一般的g2d/pp硬件都可以作为目标size.
**    这样设置是为了简单化扩展模式时的处理；
**
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** v1.0.1	Sam@2012/03/20: first commit.
** v1.0.7	leo@2012/07/03: add region and rotation function.
** v2.0.0	leo@2012/12/27: move all the core resource management to kernel level
**
*****************************************************************************/
 
#ifndef __IM_IDSAPI_H__
#define __IM_IDSAPI_H__


#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINCE_)
#ifdef IDSAPI_EXPORTS
	#define IDS_API		__declspec(dllexport)	/* For dll lib */
#else
	#define IDS_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define IDS_API
#endif	
//#############################################################################


//
// instance handler.
//
typedef void *	IDSWLINST;	// wlayer instance.
typedef void *	IDSVMINST;	// view mode instance.
typedef void *	IDSIEINST;	// IE instance.
typedef void *	IDSEXINST;	// IE instance.


//
// view mode = IDS_VMODE_MODE_xxx | IDS_VMODE_BIT_xxx.
// [7:0] view mode.
//
#define IDS_VMODE_MODE_LOCAL		1
#define IDS_VMODE_MODE_PROJECTION	2

#define IDS_VMODE_BIT_SHOW_LOCAL	(1<<8)	// combined with IDS_VMODE_MODE_PROJECTION and IDS_VMODE_MODE_THEATRE.

#define IDS_VMODE_MODE(mode)        (mode & 0xff)


//
// IE.
//
#define IDS_IE_VAL_MIN              0
#define IDS_IE_VAL_MAX              100


//
// display device type list.
//
#define IDS_DISPDEV_TYPE_LCD		0
#define IDS_DISPDEV_TYPE_TV		1
#define IDS_DISPDEV_TYPE_I80		2
#define IDS_DISPDEV_TYPE_DSI		3
#define IDS_DISPDEV_TYPE_HDMI		4

#define IDS_DISPDEV_TYPE_MAX		5


#define IDS_DISPDEV_ID_DYNAMIC		0x12345678
typedef struct{
	IM_INT32	type;	// IDS_DISPDEV_TYPE_xxx.
	IM_INT32	id;		// static ID or IDS_DISPDEV_ID_DYNAMIC.
	IM_INT32	width;	// when id==IDS_DISPDEV_ID_DYNAMIC, it's the default width/height/fps.
	IM_INT32	height;
	IM_INT32	fpsx1000;	// the actual fps multiplied by 1000
}ids_display_device_t;

//
// coordinate is related to display device(screen coordinate).
//
typedef struct{
	IM_INT32	pixfmt;		// IM_PIC_FMT_xxx.
	IM_INT32	rotation;	// IDS_ROTATION_xxx.
	IM_INT32	width;
	IM_INT32	height;
	IM_INT32	xOffset;
	IM_INT32	yOffset;
}ids_wlayer_canvas_t;

typedef struct{
	IM_UINT32		mode;
	ids_display_device_t	locDev;
	ids_wlayer_canvas_t	locCanvas;
	ids_display_device_t	extDev;
	ids_wlayer_canvas_t	extCanvas;
}ids_vmode_config_t;

//
// notification from vmode&wlayer in IDS to vmode manager in ids_lib.
//
#define IDS_VMODE_NOTIFY_SETMODE	0x1000	// param is the status, IDS_VMODE_SETMODE_xxx.
#define IDS_VMODE_NOTIFY_EXCEPTION	0x2000	// param is the exception code, IDS_VMODE_EXCEPTION_xxx.

#define IDS_VMODE_SETMODE_OK		0
#define IDS_VMODE_SETMODE_FAILED	-1

#define IDS_VMODE_SETCANVAS_OK		0
#define IDS_VMODE_SETCANVAS_FAILED	-1

#define IDS_VMODE_FBL_SWPBF_OK		0
#define IDS_VMODE_FBL_SWPBF_FAILED	 -1

#define IDS_VMODE_EXCEPTION_UNKNOWN 	 -1

//
// rotation in clockwise
//
#define IDS_ROTATION_0       0
#define IDS_ROTATION_90      90
#define IDS_ROTATION_180     180
#define IDS_ROTATION_270     270


//
// window layer.
//
typedef struct{
	IM_INT32	left;
	IM_INT32	top;
	IM_INT32	width;
	IM_INT32	height;
}ids_wlayer_position_t;

typedef struct{
	IM_BOOL         enable;
	IM_UINT32       clr;
}ids_wlayer_mapclr_t;

typedef struct{
	IM_BOOL 	pixelBlend; 	// If false, blend with plane, else blend with pixel.
	IM_UINT32	alpha_r;	// range 0--15.
	IM_UINT32	alpha_g;	// range 0--15.
	IM_UINT32	alpha_b;	// range 0--15.
}ids_wlayer_alpha_t;

typedef struct{	// region is related to the whole-image.
	IM_BOOL     enable; // if false, is whole-image.
	IM_INT32    xOffset;
	IM_INT32    yOffset;
	IM_INT32    width;
	IM_INT32    height;
}ids_wlayer_region_t;

typedef struct{
	IM_INT32	imgWidth;
	IM_INT32	imgHeight;

	// region define as a valid image area will draw to the wlayer, you can
	// consider it as cropping function.
	ids_wlayer_region_t     region; // related to imgWidth&imgHeight.

	// pos is related to canvas.
	ids_wlayer_position_t	pos;

	ids_wlayer_alpha_t 		alpha;

	IM_Buffer	buffer;
	IM_Buffer	bufferUv;
}ids_wlayer_panorama_t;

typedef struct{
	IM_UINT32			pixFormat;	// IM_PIC_FMT_xxx, see IM_picformat.h

	// if or not use devmmu, if true, the IM_Buffer's phy_addr is the devaddr, else is physical address.
	IM_BOOL				useDevMMU;

	// special(dedicated) for extend display. if it's true, request extend idsx for use
	IM_BOOL				specialForExt;
}ids_wlayer_config_t;

//   vmode & extlayer callback
typedef void (*fcbk_vmode_listener_t)(IM_UINT32 evt, void *p);
typedef void (*extlayer_use_listener_t)(IM_UINT32 evt, void *p);

//
// vmode event
//
#define IDS_VMODE_EVENT_EXTDEV_UPDATE			0x1	// p = IM_NULL, user can call ids_vmode_get_ext_devs().
#define IDS_VMODE_EVENT_EXTDEV_BREAK			0x2	// p = IM_NULL.
#define IDS_VMODE_EVENT_BACK_TO_LOCAL			0x3	// p = IM_NULL.

#define IDS_EXTLAYER_EVENT_EXTDEV_BREAK			0x4		// p = IM_NULL.
#define IDS_EXTLAYER_EVENT_BACK_TO_LOCAL		0x5		// p = IM_NULL.

#define IDS_VMODE_EVENT_FBLAYER_MODE_CHANGE 	0x10
#define IDS_VMODE_EVENT_SETMODE_STATUS			0x11 // p = IM_INT32 *, IDS_VMODE_SETMODE_xxx.
#define IDS_VMODE_EVENT_EXTDEV_CHANGE_CANVAS	0x12 //
#define IDS_VMODE_EVENT_EXTDEV_SETCANVAS_STATUS	0x13 // p = IM_INT32 *, IDS_VMODE_SETCANVAS_xxx.
#define IDS_VMODE_EVENT_FBLAYER_SWAP_BUFFER		0x14 //
#define IDS_VMODE_NOTIFY_FBL_SWAPBF				0x15

#define IDS_VMODE_EVENT_EXCEPTION				0x8000	// p = IM_INT32 *, IDS_VMODE_EXCEPTION_xxx.

//
// interface list.
//
IDS_API IM_UINT32 ids_version(IM_TCHAR *ver_string);

IDS_API IM_RET ids_wlayer_register_extend_use(OUT IDSEXINST *extinst, IN extlayer_use_listener_t listener);
IDS_API IM_RET ids_wlayer_unregister_extend_use(OUT IDSEXINST extinst);
IDS_API IM_RET ids_wlayer_init(OUT IDSWLINST *wlinst, IN ids_wlayer_config_t *config);
IDS_API IM_RET ids_wlayer_deinit(IN IDSWLINST wlinst);
IDS_API IM_RET ids_wlayer_open(IN IDSWLINST wlinst);
IDS_API IM_RET ids_wlayer_close(IN IDSWLINST wlinst);
IDS_API IM_RET ids_wlayer_get_devs(IN IDSWLINST wlinst, IN ids_display_device_t *dev);
IDS_API IM_RET ids_wlayer_set_panorama(IN IDSWLINST wlinst, IN ids_wlayer_panorama_t *panorama);
IDS_API IM_RET ids_wlayer_set_region(IN IDSWLINST wlinst, IN ids_wlayer_region_t *region);
IDS_API IM_RET ids_wlayer_set_buffer(IN IDSWLINST wlinst, IN IM_Buffer *buffer, IN IM_Buffer *bufferUv);
IDS_API IM_RET ids_wlayer_set_position(IN IDSWLINST wlinst, IN ids_wlayer_position_t *pos);
IDS_API IM_RET ids_wlayer_set_mapclr(IN IDSWLINST wlinst, IN IM_BOOL enable, IN IM_UINT32 clr);	// [31:24]nouse, [23:16]Red, [15:8]green, [7:0]blue.
IDS_API IM_RET ids_wlayer_set_clrkey(IN IDSWLINST wlinst, IN IM_BOOL enable, IN IM_BOOL matchForeground, IN IM_UINT32 clr);
IDS_API IM_RET ids_wlayer_set_alpha(IN IDSWLINST wlinst, IN ids_wlayer_alpha_t *alpha);

IDS_API IM_RET ids_vmode_init(OUT IDSVMINST *vminst, IN fcbk_vmode_listener_t listener);
IDS_API IM_RET ids_vmode_deinit(IN IDSVMINST vminst);
IDS_API IM_RET ids_vmode_get_mode(IN IDSVMINST vminst, OUT ids_vmode_config_t *vmcfg);
IDS_API IM_RET ids_vmode_set_mode(IN IDSVMINST vminst, IN IM_UINT32 mode);
IDS_API IM_RET ids_vmode_get_ext_devs(IN IDSVMINST vminst, INOUT im_list_handle_t devlist);
IDS_API IM_RET ids_vmode_set_ext_dev(IN IDSVMINST vminst, IN IM_INT32 devtype, IN IM_INT32 devid);
IDS_API IM_RET ids_vmode_set_ext_canvas(IN IDSVMINST vminst, IN ids_wlayer_canvas_t *canvas);
IDS_API IM_RET ids_vmode_ie_init(IN IDSVMINST vminst, OUT IDSIEINST *ieinst);

IDS_API IM_RET ids_ie_init(OUT IDSIEINST *ieinst, IN IM_BOOL ext);
IDS_API IM_RET ids_ie_deinit(IN IDSIEINST ieinst);
IDS_API IM_RET ids_ie_set_acc(IN IDSIEINST ieinst, IN IM_INT32 acc);
IDS_API IM_RET ids_ie_get_acc(IN IDSIEINST ieinst, OUT IM_INT32 *acc);
IDS_API IM_RET ids_ie_set_acm(IN IDSIEINST ieinst, IN IM_INT32 acm);
IDS_API IM_RET ids_ie_get_acm(IN IDSIEINST ieinst, OUT IM_INT32 *acm);
IDS_API IM_RET ids_ie_set_gamma(IN IDSIEINST ieinst, IN IM_INT32 gamma);
IDS_API IM_RET ids_ie_get_gamma(IN IDSIEINST ieinst, OUT IM_INT32 *gamma);
IDS_API IM_RET ids_ie_set_ee(IN IDSIEINST ieinst, IN IM_INT32 ee);
IDS_API IM_RET ids_ie_get_ee(IN IDSIEINST ieinst, OUT IM_INT32 *ee);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_IDSAPI_H__


