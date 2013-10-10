/**
 * Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 * 
 * Description: Header file of Camera APIs
 *	
 * Author:
 *     leo zhang<@leo.zhang@infotmic.com.cn>
 *
 * Revision History: 
 * ----------------- 
 * v1.0.1	leo@2011/4/23: first commit
 * v1.0.2	leo@2011/4/25:
 * v1.0.3	leo@2011/4/26:
 * v1.0.4	leo@2011/4/28: change some interface
 * v1.0.5	leo@2011/5/30: add interface for set source
 * v1.0.6	rane@2011/7/26: support whitebalance & special effect & more resotultion
 * v1.0.7	rane@2012/3/08: camif support 2048 * 1536 
 * v1.1.0	rane@2012/3/21: a stable version.
 * v2.0.1	leo@2012/04/06: rewrite interface, it's not compatible with previous version.
 */


#ifndef __IM_CAMERAAPI_H__
#define __IM_CAMERAAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if (TARGET_SYSTEM == FS_WINCE)
#ifdef CAMERA_EXPORTS
	#define CAMERA_API		__declspec(dllexport)	/* For dll lib */
#else
	#define CAMERA_API		__declspec(dllimport)	/* For application */
#endif
#else
	#define CAMERA_API
#endif

//#############################################################################
//
// camera context type.
//
typedef void * CAMCTX;

// path.
#define CAM_PATH_NONE			(0)
#define CAM_PATH_PREVIEW		(1)
#define CAM_PATH_CODEC			(2)
#define CAM_PATH_PICTURE		(4)

// camerelib class.
#define CAM_LIBCLASS_STUB		(0)	// cameralib_stub.
#define CAM_LIBCLASS_USB		(1)	// cameralib_usb(v4l2).
#define CAM_LIBCLASS_CAMDRV		(2)	// cameralib_camdrv. (for imapx200 camif).
#define CAM_LIBCLASS_ISP		(3)	// cameralib_isp.
#define CAM_LIBCLASS_CAMIF		(4)	// cameralib_camif.

#define CAM_LIBCLASS_ENUM_MAX	(5)	// total number of libclass.

// camere interface.
#define CAM_INTERFACE_UNKNOWN	    (0)
#define CAM_INTERFACE_ISP_DVP	    (1)	// isp dvp.
#define CAM_INTERFACE_ISP_MIPI	    (2)	// isp mipi
#define CAM_INTERFACE_CAMIF_DVP		(3)	// camif dvp.

#define CAM_INTERFACE_ENUM_MAX	    (4)	// total number of camera interface.

// camera data type
#define CAM_DATATYPE_LOW8_ALIGN     (0)	// low8 bit data input.
#define CAM_DATATYPE_MID8_ALIGN     (1)	// mid8 bit data input.
#define CAM_DATATYPE_LOW10_ALIGN    (2)	// low10 bit data input.
#define CAM_DATATYPE_HIGH_ALIGN     (3)	// high8/10/12 bit data input.


//#############################################################################
// module tree structure:
// ==================================
// |--libclass1
// 		|--moduleBox1
// 			|--module1
// 			|--module2
// 			|--...
// 		|--moduleBox2
// 			|--module1
// 			|--...
// 		...
// 	|--...
// ==================================
//
// The modules in same moduleBox can only one working in same time. the moduleBox is independent
// between each other.
//
// per module has a unique id in one moduleBox, so define the uid of cam_open() is:
// [31:8]reserved for other flag, [11:8] lib class index, [7:4]moduleBox index, [3:0]module index.
//
#define CAM_MODULE_MAKE_UID(libclass, box, module)	((((libclass & 0xf) << 8)) | (((box) & 0xf) << 4) | ((module) & 0xf))
#define CAM_MODULE_GET_UID_LIBCLASS(uid)			(((uid) >> 8) & 0xf)
#define CAM_MODULE_GET_UID_BOX(uid)					(((uid) >> 4) & 0xf)
#define CAM_MODULE_GET_UID_MODULE(uid)				((uid) & 0xf)

//
#define CAM_MODULE_FACING_UNKNOWN	(0)
#define CAM_MODULE_FACING_FRONT		(1)
#define CAM_MODULE_FACING_BACK		(2)

#define CAM_MODULE_DESC_LENGTH_MAX	(64)
typedef struct{
	IM_TCHAR 	desc[CAM_MODULE_DESC_LENGTH_MAX];
	IM_UINT32 	id;
	IM_UINT32 	facing;		// CAM_MODULE_INFO_FACING_xxx.
	IM_INT32 	orientation;
	IM_INT32	resolution;	// max sample resolution.
}cam_module_info_t;

// For simple implement, assume the moduleBox contain limited module, and camlib contain limited moduleBox.
#define CAM_MODULE_BOX_NUM_MAX		4
#define CAM_MODULE_NUM_MAX			4
typedef struct{
	IM_INT32			moduleNum;	// the valid number.
	cam_module_info_t	modules[CAM_MODULE_NUM_MAX];
}cam_module_box_t;

typedef struct{
	IM_INT32			boxNum;	// the valid number.
	cam_module_box_t	boxes[CAM_MODULE_BOX_NUM_MAX];
}cam_module_class_t;

typedef struct{
	cam_module_class_t	libcls[CAM_LIBCLASS_ENUM_MAX];
}cam_module_tree_t;

//#############################################################################
//
// property key
//
#define CAM_KEY_R_CAPS				0x0		// value type: IM_UINT32, see CAM_CAP_xxx.

#define CAM_KEY_R_PICTURE_BUFFER_REQUIREMENT	0x10000	// value type: cam_buffer_property_t.
#define CAM_KEY_R_CODEC_BUFFER_REQUIREMENT	0x10001	// value type: cam_buffer_property_t.
#define CAM_KEY_R_PREVIEW_BUFFER_REQUIREMENT	0x10002	// value type: cam_buffer_property_t.

#define CAM_KEY_R_SUPPORT_WB_MODE 			0x20000	// value type: IM_INT32, see CAM_WB_MODE_xxx.
#define CAM_KEY_RW_WB_MODE 			0x20001	// value type: IM_INT32, see CAM_WB_MODE_xxx.

#define CAM_KEY_R_SUPPORT_SPECIAL_EFFECT 		0x30000	// value type: IM_INT32, see CAM_SPECIAL_EFFECT_xxx.
#define CAM_KEY_RW_SPECIAL_EFFECT 		0x30001	// value type: IM_INT32, see CAM_SPECIAL_EFFECT_xxx.

#define CAM_KEY_R_SUPPORT_SCENE_MODE 	0x40000	// value type: IM_INT32, see CAM_SCENE_MODE_xxx.
#define CAM_KEY_RW_SCENE_MODE 			0x40001	// value type: IM_INT32, see CAM_SCENE_MODE_xxx.

#define CAM_KEY_R_SUPPORT_FLASH_MODE 			0x50000	// value type: IM_INT32, see CAM_FLASH_MODE_xxx.
#define CAM_KEY_RW_FLASH_MODE 			0x50001	// value type: IM_INT32, see CAM_FLASH_MODE_xxx.
#define CAM_KEY_RW_LIGHT_TURN_ON 			0x50002	// value type: IM_INT32 0: off; others: on.

#define CAM_KEY_R_SUPPORT_FOCUS_MODE 			0x60000	// value type: IM_INT32, see CAM_FOCUS_MODE_xxx.
#define CAM_KEY_RW_FOCUS_MODE 			0x60001	// value type: IM_INT32, see CAM_FOCUS_MODE_xxx.
#define CAM_KEY_R_MAX_NUM_FOCUS_AREAS 		0x60002	// value type: IM_INT32.
#define CAM_KEY_RW_FOCUS_AREA 			0x60003	// value type: IM_INT32.
#define CAM_KEY_RW_FOCAL_LENGTH 		0x60004	// value type: IM_INT32.
#define CAM_KEY_R_HORIZONTAL_VIEW_ANGLE		0x60005	// value type: IM_INT32.
#define CAM_KEY_R_VERTICAL_VIEW_ANGLE		0x60006	// value type: IM_INT32.

#define CAM_KEY_RW_EXPOSURE_COMPENSATION 	0x70000	// value type: IM_INT32. set 0 as default
#define CAM_KEY_R_MAX_EXPOSURE_COMPENSATION 	0x70001	// value type: IM_INT32.
#define CAM_KEY_R_MIN_EXPOSURE_COMPENSATION 	0x70002	// value type: IM_INT32.
#define CAM_KEY_R_EXPOSURE_COMPENSATION_STEP 	0x70003	// value type: IM_FLOAT.
#define CAM_KEY_R_SUPPORT_EXPOSURE_LOCK 	0x70004	// value type: IM_BOOL.
#define CAM_KEY_RW_EXPOSURE_LOCK			0x70005	// value type: IM_INT32.

#define CAM_KEY_R_ZOOM_RATIOS 			0x80001	// value type: IM_TCHAR *  (such as "100,120,140").
#define CAM_KEY_R_MAX_ZOOM				0x80002	// value type: IM_INT32.
#define CAM_KEY_RW_ZOOM					0x80003	// value type: IM_INT32, these values multiply by 100 to avoid use float.

#define CAM_KEY_R_SUPPORT_ANTIBANDING_MODE 	0x90000	// value type: IM_INT32, see CAM_ANTIBANDING_MODE_xxx.
#define CAM_KEY_RW_ANTIBANDING_MODE			0x90001	// value type: IM_INT32, see CAM_ANTIBANDING_MODE_xxx.

#define CAM_KEY_RW_DEINTERLACED             0X100000 //value type: IM_INT32, 0:DO NOTHING    1: DE_INTERLACED

// capabilities.
#define CAM_CAP_PREVIEW_SUPPORT				(1<<0)
#define CAM_CAP_CODEC_SUPPORT				(1<<1)
#define CAM_CAP_PICTURE_SUPPORT				(1<<2)
#define CAM_CAP_WB_MODE_SUPPORT				(1<<3)
#define CAM_CAP_SPECIAL_EFFECT_SUPPORT		(1<<4)
#define CAM_CAP_SCENE_MODE_SUPPORT			(1<<5)
#define CAM_CAP_FLASH_MODE_SUPPORT			(1<<6)
#define CAM_CAP_FOCUS_MODE_SUPPORT			(1<<7)
#define CAM_CAP_ZOOM						(1<<8)
#define CAM_CAP_SMOOTH_ZOOM					(1<<9)
#define CAM_CAP_FLASH						(1<<10)
#define CAM_CAP_FOCUS						(1<<11)
#define CAM_CAP_ANTIBANDING					(1<<12)
#define CAM_CAP_EXPOSURE					(1<<13)
#define CAM_CAP_INTERLACE                   (1<<14)

// white balance mode.
#define CAM_WB_MODE_AUTO 		 		(1<<0)
#define CAM_WB_MODE_INCANDESCENT  		(1<<1)
#define CAM_WB_MODE_FLUORESCENT   		(1<<2)
#define CAM_WB_MODE_DAYLIGHT  	 		(1<<3)
#define CAM_WB_MODE_WARM_FLUORECENT  	(1<<4)
#define CAM_WB_MODE_CLOUDY_DAYLIGHT  	(1<<5)
#define CAM_WB_MODE_TWILIGHT 			(1<<6)
#define CAM_WB_MODE_SHADE  				(1<<7)

// special effect.
#define CAM_SPECIAL_EFFECT_NONE 	  		(1<<0) 
#define CAM_SPECIAL_EFFECT_MONO 	  		(1<<1)
#define CAM_SPECIAL_EFFECT_NEGATIVE   		(1<<2)
#define CAM_SPECIAL_EFFECT_SOLARIZE	  		(1<<3)
#define CAM_SPECIAL_EFFECT_PASTEL 	  		(1<<4)
#define CAM_SPECIAL_EFFECT_MOSAIC 	  		(1<<5)
#define CAM_SPECIAL_EFFECT_RESIZE	  		(1<<6)
#define CAM_SPECIAL_EFFECT_SEPIA 	  		(1<<7)
#define CAM_SPECIAL_EFFECT_POSTERIZE  		(1<<8)
#define CAM_SPECIAL_EFFECT_WHITEBOARD 		(1<<9)
#define CAM_SPECIAL_EFFECT_BLACKBOARD 		(1<<10)
#define CAM_SPECIAL_EFFECT_AQUA 	  		(1<<11)

// scene mode.
#define CAM_SCENE_MODE_AUTO					(1<<0)
#define CAM_SCENE_MODE_ACTION				(1<<1)
#define CAM_SCENE_MODE_PORTRAIT				(1<<2)
#define CAM_SCENE_MODE_LANDSCAPE			(1<<3)
#define CAM_SCENE_MODE_NIGHT				(1<<4)
#define CAM_SCENE_MODE_NIGHT_PORTRAIT		(1<<5)
#define CAM_SCENE_MODE_THEATRE				(1<<6)
#define CAM_SCENE_MODE_BEACH				(1<<7)
#define CAM_SCENE_MODE_SNOW					(1<<8)
#define CAM_SCENE_MODE_SUNSET				(1<<9)
#define CAM_SCENE_MODE_STEADYPHOTO			(1<<10)
#define CAM_SCENE_MODE_FIREWORKS			(1<<11)
#define CAM_SCENE_MODE_SPORTS				(1<<12)
#define CAM_SCENE_MODE_PARTY				(1<<13)
#define CAM_SCENE_MODE_CANDLELIGHT			(1<<14)
#define CAM_SCENE_MODE_BARCODE				(1<<15)

// flash mode.
#define CAM_FLASH_MODE_OFF					(1<<0)
#define CAM_FLASH_MODE_AUTO					(1<<1)
#define CAM_FLASH_MODE_ON					(1<<2)
#define CAM_FLASH_MODE_RED_EYE				(1<<3)
#define CAM_FLASH_MODE_TORCH				(1<<4)

// focus mode.
#define CAM_FOCUS_MODE_AUTO					(1<<0)
#define CAM_FOCUS_MODE_INFINITY				(1<<1)
#define CAM_FOCUS_MODE_MACRO				(1<<2)
#define CAM_FOCUS_MODE_FIXED				(1<<3)
#define CAM_FOCUS_MODE_EDOF					(1<<4)
#define CAM_FOCUS_MODE_CONTINUOUS_VIDEO		(1<<5)
#define CAM_FOCUS_MODE_CONTINUOUS_PICTURE	(1<<6)

// antibanding.
#define CAM_ANTIBANDING_MODE_OFF					(1<<0)
#define CAM_ANTIBANDING_MODE_50HZ					(1<<1)
#define CAM_ANTIBANDING_MODE_60HZ					(1<<2)
#define CAM_ANTIBANDING_MODE_AUTO					(1<<3)

//#############################################################################
// pre-defined resolutions.
#define CAM_RES_QQCIF		(1<<0)	// 88*72
#define CAM_RES_SUB_QCIF	(1<<1)	// 128*96
#define CAM_RES_QQVGA		(1<<2)	// 160*120
#define CAM_RES_QCIF		(1<<3)	// 176*144
#define CAM_RES_QVGA		(1<<4)	// 320*240
#define CAM_RES_CIF			(1<<5)	// 352*288
#define CAM_RES_VGA			(1<<6)	// 640*480
#define CAM_RES_480P		(1<<7)	// 720*480
#define CAM_RES_PAL			(1<<8)	// 768*576
#define CAM_RES_SVGA		(1<<9)	// 800*600
#define CAM_RES_XVGA		(1<<10)	// 1024*768
#define CAM_RES_720P		(1<<11)	// 1280*720
#define CAM_RES_130W		(1<<12)	// 1280*960
#define CAM_RES_SXGA		(1<<13)	// 1280*1024
#define CAM_RES_SXGAPlus	(1<<14)	// 1400*1050
#define CAM_RES_UXGA		(1<<15)	// 1600*1200
#define CAM_RES_1080P		(1<<16)	// 1920*1080
#define CAM_RES_320W		(1<<17)	// 2048*1536
#define CAM_RES_WQXGA		(1<<18)	// 2560*1600
#define CAM_RES_500W		(1<<19) // 2592*1936
#define CAM_RES_QUXGA		(1<<20)	// 3200*2400
#define CAM_RES_WQXGA_U		(1<<21)	// 3840*2400
#define CAM_RES_12M	    	(1<<22)	// 4000*3000

#define CAM_RES_ENUM_MAX	(23)

// pre-defined fps. [7:0] 0~19fps, [15:8] 20~29fps, [23:16] 30~120, [31:24]reserved.
#define CAM_FPS_5			(1<<0)
#define CAM_FPS_7			(1<<1)
#define CAM_FPS_10			(1<<2)
#define CAM_FPS_12			(1<<3)
#define CAM_FPS_15			(1<<4)
#define CAM_FPS_18			(1<<5)

#define CAM_FPS_20			(1<<8)
#define CAM_FPS_22			(1<<9)
#define CAM_FPS_24			(1<<10)
#define CAM_FPS_25			(1<<11)
#define CAM_FPS_26			(1<<12)
#define CAM_FPS_29			(1<<13)

#define CAM_FPS_30			(1<<16)

// pre-defined pixel format.
#define CAM_PIXFMT_YUV420P			(1<<0)	
#define CAM_PIXFMT_YUV420SP			(1<<1)
#define CAM_PIXFMT_YUV422P			(1<<2)
#define CAM_PIXFMT_YUV422SP			(1<<3)
#define CAM_PIXFMT_YUV444P			(1<<4)
#define CAM_PIXFMT_YUV422I			(1<<5)
#define CAM_PIXFMT_16BPP_RGB565		(1<<6)
#define CAM_PIXFMT_32BPP_RGB0888	(1<<7)
#define CAM_PIXFMT_32BPP_RGB8880	(1<<8)
#define CAM_PIXFMT_32BPP_BGR0888	(1<<9)
#define CAM_PIXFMT_32BPP_BGR8880	(1<<10)

//
typedef struct{
	IM_UINT32	resBits;	// CAM_RES_xxx bit or.
	IM_UINT32	fmtBits;	// CAM_PIXFMT_xxx bit or.
	struct{
		IM_UINT32	fpsBits;	// CAM_FPS_xxx bit or.
	}attri[CAM_RES_ENUM_MAX];

	// current setting.
	IM_UINT32	res;	// CAM_RES_xxx.
	IM_UINT32	fmt;	// CAM_PIXFMT_xxx.
	IM_UINT32	fps;	// CAM_FPS_xxx.
}cam_preview_config_t;
typedef cam_preview_config_t	cam_codec_config_t;
typedef cam_preview_config_t	cam_picture_config_t;

//#############################################################################
//
// re-organize path.
//
typedef struct{
	IM_INT32	preRedirTo;	// CAM_PATH_xxx.
	IM_INT32	coRedirTo;	// CAM_PATH_xxx.
	IM_INT32	picRedirTo;	// CAM_PATH_xxx.
}cam_reorg_path_t;


//
// cam frame.
//
#define CAM_FRAME_FLAG_INFO				(1<<0)
#define CAM_FRAME_FLAG_INFO_ABORT		(1<<4)
typedef struct{
	IM_UINT32 		flag;	// CAM_FRAME_FLAG_xxx.
	IM_INT64 		ts;		// ms.
	IM_Buffer 		buffer;
	IM_UINT32 		dataSize;
}cam_frame_t;

//
// buffer property.
//
typedef struct{
	IM_INT32	minNumber;	// minimum number requirement.
	IM_BOOL		needBusAddr;
	IM_UINT32	align;		// IM_BUFFER_ALIGN_xxx.
}cam_buffer_property_t;

//
// prepare take picture configure.
//
typedef struct{
	IM_BOOL		needBusAddr;
	IM_UINT32	align;	// IM_BUFFER_ALIGN_xxx.
}cam_takepic_config_t;

//
// About control.
//
#define	CAM_CTRL_FLAG_ASYNC			(1<<0)

#define CAM_CTRL_CODE_AUTOFOCUS		0x1
#define CAM_CTRL_CODE_NUMBER		1	// the number of control codes.

struct __cam_control_t;
typedef void(*func_cam_control_callback)(struct __cam_control_t *ctrl, IM_RET ret);
typedef struct __cam_control_t{
	IM_INT32	code;	// CAM_CTRL_CODE_XXX
	IM_BOOL		async;
	void *		dataStruct;
	IM_INT32	dsSize;

	// used in async mode;
	func_cam_control_callback	listener;
	IM_INT32	timeout;	// ms, -1 infinity.
	IM_INT32	cwid;	// control word id, returned by sendControl().
}cam_control_t;


/*============================Interface API==================================*/
/* 
 * FUNC: get cam version.
 * PARAMS: ver_string, save this version string.
 * RETURN: see IM_common.h about version defination. 
 */
CAMERA_API IM_UINT32 cam_version(OUT IM_TCHAR *ver_string);

/*
 * FUNC: get module tree.
 * PARAMS: mtree, save the module tree.
 * RETURN: IM_RET_OK is successful, IM_RET_NOITEM is no module(mtree is IM_NULL), else failed.
 */
CAMERA_API IM_RET cam_get_modules(OUT cam_module_tree_t *mtree);

/* 
 * FUNC: open camera.
 * PARAMS:
 * 	cam, save camera context.
 *	uid, camera module unique id.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_open(OUT CAMCTX *cam, IN IM_INT32 uid);

/* 
 * FUNC: close camera.
 * PARAMS: cam, cemera context created by cam_open().
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_close(IN CAMCTX cam);

/*
 * FUNC: re-organize path.
 * PARAMS: cam, instance.
 * 	org, orgnaize.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_reorganize_path(IN CAMCTX cam, IN cam_reorg_path_t *reorg);

/* 
 * FUNC: get preview/codec/picture configure.
 * PARAMS: cam, cemera context created by cam_open().
 * 	cfgs, save the preview configure.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_get_preview_configs(IN CAMCTX cam, OUT cam_preview_config_t *cfgs);
CAMERA_API IM_RET cam_get_picture_configs(IN CAMCTX cam, OUT cam_picture_config_t *cfgs);
CAMERA_API IM_RET cam_get_codec_configs(IN CAMCTX cam, OUT cam_codec_config_t *cfgs);

/* 
 * FUNC: set preview/codec/picture configure.
 * PARAMS: cam, cemera context created by cam_open().
 * 	resolution, CAM_RES_xxx.
 * 	pixfmt, CAM_PIXFMT_xxx.
 * 	fps, CAM_FPS_xxx.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_set_preview_config(IN CAMCTX cam, IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps);
CAMERA_API IM_RET cam_set_codec_config(IN CAMCTX cam, IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps);
CAMERA_API IM_RET cam_set_picture_config(IN CAMCTX cam, IN IM_UINT32 res, IN IM_UINT32 fmt, IN IM_UINT32 fps);

/*
 * FUNC: start camera, after this can capture preview and codec stream, picture don't need start first.
 * PARAMS: cam, cemera context created by cam_open().
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_start(IN CAMCTX cam);

/*
 * FUNC: stop camera, after this cannot capture preview and codec stream.
 * PARAMS: cam, cemera context created by cam_open().
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_stop(IN CAMCTX cam);

/*
 * FUNC: set camera property, property use key as it's id.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	key, property id, see CAM_KEY_XXX.
 * 	value, key&value pair.
 * 	size, size of value buffer.
 * RETURN: IM_RET_OK is successful, IM_RET_FALSE done set but has some limitation, else failed. 
 */
CAMERA_API IM_RET cam_set_property(IN CAMCTX cam, IN IM_INT32 key, IN void *value, IN IM_INT32 size);

/*
 * FUNC: get camera property, property use key as it's id.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	key, property id, see CAM_KEY_XXX.
 * 	value, key&value pair.
 * 	size, size of value buffer.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_get_property(IN CAMCTX cam, IN IM_INT32 key, OUT void *value, IN IM_INT32 size);

/*
 * FUNC: enable path, if camera is started, and this path has been configured buffer, then
 * 	it will start automatically.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	path, see CAM_PATH_PREVIEW or CAM_PATH_CODEC.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_enable_stream(IN CAMCTX cam, IN IM_INT32 path);

/*
 * FUNC: disble path, if camera is started, this path will stop automatically.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	path, see CAM_PATH_PREVIEW or CAM_PATH_CODEC.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_disable_stream(IN CAMCTX cam, IN IM_INT32 path);

/*
 * FUNC: assign buffer to camera, buffer provided by caller.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	path, see CAM_PATH_PREVIEW or CAM_PATH_CODEC.
 * 	interAlloc, whether internal allocated.
 * 	buff, buffer, when request internal allocate, it save the allocated buffer.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_assign_buffer(IN CAMCTX cam, IN IM_INT32 path, IN IM_BOOL interAlloc, INOUT IM_Buffer *buffer);

/*
 * FUNC: prepare to take picture.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_prepare_take_picture(IN CAMCTX cam, IN cam_takepic_config_t *cfg);

/*
 * FUNC: de-prepare picture.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_cancel_take_picture(IN CAMCTX cam);

/*
 * FUNC: take a picture.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	picFrame, use to save captured picture.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_take_picture(IN CAMCTX cam, OUT cam_frame_t *picFrame);

/*
 * FUNC: release this picture that got by cam_take_picture().
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	picFrame, to release.
 * RETURN: IM_RET_OK is successful, else failed. 
 */
CAMERA_API IM_RET cam_release_picture(IN CAMCTX cam, IN cam_frame_t *picFrame);

/*
 * FUNC: get a preview frame, it's a block calling.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	frame, use to save captured frame.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_get_preview_frame(IN CAMCTX cam, OUT cam_frame_t *frame);

/*
 * FUNC: release a preview frame that captured by cam_get_preview_frame().
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	frame, to release.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_release_preview_frame(IN CAMCTX cam, IN cam_frame_t *frame);

/*
 * FUNC: get a codec frame, it's a block calling.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	frame, use to save captured frame.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_get_codec_frame(IN CAMCTX cam, OUT cam_frame_t *frame);

/*
 * FUNC: release a codec frame that captured by cam_get_codec_frame().
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	frame, to release.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_release_codec_frame(IN CAMCTX cam, IN cam_frame_t *frame);

/*
 * FUNC: send a control, it has block and non-block calling mode.
 * PARAMS: 
 * 	cam, cemera context created by cam_open().
 * 	ctrl, structure of control.
 * RETURN: IM_RET_OK is successful, else failed.
 */
CAMERA_API IM_RET cam_send_control(IN CAMCTX cam, INOUT cam_control_t *ctrl);
CAMERA_API IM_RET cam_cancel_control(IN CAMCTX cam, IN IM_INT32 cwid);

//#############################################################################
#ifdef __cplusplus
}
#endif

#endif	// __IM_CAMERAAPI_H__

