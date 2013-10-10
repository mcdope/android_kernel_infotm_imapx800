/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of ids lib, in Kernel call
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** v1.0.1	Sam@2012/3/20: first commit.
** v1.0.3	Sam@2012/4/25: maintian async-flush for safe-release the wlayer.
** v1.0.7	leo@2012/08/11: readjust.
**
*****************************************************************************/


#ifndef __IDS_INTERNAL_H__
#define __IDS_INTERNAL_H__


//
// version number.
//
#define VER_MAJOR	2
#define VER_MINOR	0
#define VER_PATCH	1

#define VER_STRING	"2.0.1"

//
//
//
enum{
	OVCDCR = 0x1000,
	OVCPCR = 0x1004,
	OVCBKCOLOR = 0x1008,
	OVCW0CR = 0x1080,
	OVCW0PCAR = 0x1084,
	OVCW0PCBR = 0x1088,	
	OVCW0VSSR = 0x108c,
	OVCW0CMR = 0x1090,
	OVCW0B0SAR = 0x1094,
	OVCW0B1SAR = 0x1098,
	OVCW0B2SAR = 0x109c,
	OVCW0B3SAR = 0x10a0,
	OVCW1CR = 0x1100,
	OVCW1PCAR = 0x1104,
	OVCW1PCBR = 0x1108,
	OVCW1PCCR = 0x110c,
	OVCW1VSSR = 0x1110,
	OVCW1CKCR = 0x1114,
	OVCW1CKR = 0x1118,
	OVCW1CMR = 0x111c,
	OVCW1B0SAR = 0x1120,
	OVCW1B1SAR = 0x1124,
	OVCW1B2SAR = 0x1128,
	OVCW1B3SAR = 0x112c,	
	OVCW2CR = 0x1180,
	OVCW2PCAR = 0x1184,
	OVCW2PCBR = 0x1188,
	OVCW2PCCR = 0x118c,
	OVCW2VSSR = 0x1190,
	OVCW2CKCR = 0x1194,
	OVCW2CKR = 0x1198,
	OVCW2CMR = 0x119c,
	OVCW2B0SAR = 0x11a0,
	OVCW2B1SAR = 0x11a4,
	OVCW2B2SAR = 0x11a8,
	OVCW2B3SAR = 0x11ac,	
	OVCW3CR = 0x1200,
	OVCW3PCAR = 0x1204,
	OVCW3PCBR = 0x1208,
	OVCW3PCCR = 0x120c,
	OVCW3VSSR = 0x1210,
	OVCW3CKCR = 0x1214,
	OVCW3CKR = 0x1218,
	OVCW3CMR = 0x121c,
	OVCW3B0SAR = 0x1220,
	OVCW3SABSAR = 0x1224,	
	OVCBRB0SAR = 0x1300,
	OVCBRB1SAR = 0x1304,
	OVCBRB2SAR = 0x1308,
	OVCBRB3SAR = 0x130c,
	OVCOMC = 0x1310,
	OVCOEF11 = 0x1314,
	OVCOEF12 = 0x1318,
	OVCOEF13 = 0x131c,
	OVCOEF21 = 0x1320,
	OVCOEF22 = 0x1324,
	OVCOEF23 = 0x1328,
	OVCOEF31 = 0x132c,
	OVCOEF32 = 0x1330,
	OVCOEF33 = 0x1334
};
#define OSDREGNUM		(OVCOEF33 + 1)

// OVCDCR
#define OVCDCR_W2_AUTOBUFFER_SOURCE	(20)
#define OVCDCR_W1_AUTOBUFFER_SOURCE	(18)
#define OVCDCR_W0_AUTOBUFFER_SOURCE	(16)
#define OVCDCR_UpdateReg	(11)
#define OVCDCR_ScalerMode	(10)
#define OVCDCR_Enrefetch	(9)
#define OVCDCR_Enrelax		(8)
#define OVCDCR_WaitTime		(4)
#define OVCDCR_SafeBand		(3)
#define OVCDCR_AllFetch		(2)
#define OVCDCR_IfType		(1)
#define OVCDCR_Interlace	(0)

// OVCPCR
#define OVCPCR_UPDATE_PAL	(15)
#define OVCPCR_W3PALFM		(9)
#define OVCPCR_W2PALFM		(6)
#define OVCPCR_W1PALFM		(3)
#define OVCPCR_W0PALFM		(0)

// OVCWxCR
#define OVCWxCR_OPSEL       (22)
#define OVCWxCR_PALSEL      (21)
#define OVCWxCR_RAMPAL      (20)
#define OVCWxCR_CURAND      (19)
#define OVCWxCR_BUFSEL      (17)
#define OVCWxCR_BUFAUTOEN   (16)
#define OVCWxCR_BUFAUTOMODE (14)
#define OVCWxCR_BITSWP      (13)
#define OVCWxCR_Bits2SWP    (12)
#define OVCWxCR_Bits4SWP    (11)
#define OVCWxCR_BYTSWP      (10)
#define OVCWxCR_HAWSWP      (9)
#define OVCWxCR_ALPHA_SEL   (8)
#define OVCWxCR_BLD_PIX     (7)
#define OVCWxCR_RBEXG       (6)
#define OVCWxCR_BPPMODE     (1)
#define OVCWxCR_ENWIN       (0)

// OVCWxPCAR
#define OVCWxPCAR_FIELDSTATUS	(31)
#define OVCWxPCAR_BUFSTATUS	(29)
#define OVCWxPCAR_LeftTopY	(16)
#define OVCWxPCAR_LeftTopX	(0)

// OVCWxPCBR
#define OVCWxPCBR_RightBotY	(16)
#define OVCWxPCBR_RightBotX	(0)

// OVCWxPCCR
#define OVCWxPCCR_ALPHA0_R	(20)
#define OVCWxPCCR_ALPHA0_G	(16)
#define OVCWxPCCR_ALPHA0_B	(12)
#define OVCWxPCCR_ALPHA1_R	(8)
#define OVCWxPCCR_ALPHA1_G	(4)
#define OVCWxPCCR_ALPHA1_B	(0)

// OVCWxCKCR
#define OVCWxCKCR_KEYBLEN	(26)
#define OVCWxCKCR_KEYEN		(25)
#define OVCWxCKCR_DIRCON	(24)
#define OVCWxCKCR_COMPKEY	(0)

// OVCWxCMR
#define OVCWxCMR_MAPCOLEN	(24)
#define OVCWxCMR_MAPCOLOR	(0)


// OVCOMC
#define OVCOMC_ToRGB		(31)
#define OVCOMC_OddEven		(24)
#define OVCOMC_BitSwap		(20)
#define OVCOMC_Bits2Swap	(19)
#define OVCOMC_Bits4Swap	(18)
#define OVCOMC_ByteSwap		(17)
#define OVCOMC_HawSwap		(16)
#define OVCOMC_oft_b		(8)
#define OVCOMC_oft_a		(0)

//
// resource.
//
#define RES_WLAYER_0		(1<<0)
#define RES_WLAYER_1		(1<<1)
#define RES_WLAYER_2		(1<<2)
#define RES_WLAYER_3		(1<<3)
#define RES_RAMPAL_W0		(1<<4)
#define RES_RAMPAL_W1		(1<<5)
#define RES_YUVtoRGB_MATRIX	(1<<8)
#define RES_DEVMMU_W0		(1<<12)
#define RES_DEVMMU_W1		(1<<13)
#define RES_DEVMMU_W2		(1<<14)
#define RES_DEVMMU_W3		(1<<15)

#define RES_ALL			(0x233f)	// default no RES_DEVMMU_W0/2/3
#define RES_WLAYER_ALL		(RES_WLAYER_0 | RES_WLAYER_1 | RES_WLAYER_2 | RES_WLAYER_3)

//
// wlayer.
//
#define WLAYER_FLAG_INITED		(1<<0)
#define WLAYER_FLAG_OPENED		(1<<1)
#define WLAYER_FLAG_MMU_INITED		(1<<2)
#define WLAYER_FLAG_MMU_ENABLED		(1<<3)
#define WLAYER_FLAG_ASYNC_CONFIG	(1<<4)
typedef struct{
	IM_INT32		idsx;
	IM_INT32		wx;
	IM_INT32 		pixfmt;
	IM_UINT32		flag;	// WLAYER_FLGA_xxx.
	IM_UINT32		rsvdRes; // RES_xxx.
	
	IM_BOOL         setup;  // if set panorama.
	IM_BOOL         opened;

	IM_BOOL         mapclrEnable;
	IM_UINT32       mapclr;
	IM_BOOL         clrkeyEnable;
	IM_BOOL         clrkeyMatchForeground;
	IM_BOOL         clrkey;
	IM_INT32        imgWidth;
	IM_INT32        imgHeight;

	ids_wlayer_panorama_t	pano;
	// record the current show buffer.
	IM_Buffer       showBuffer;
	IM_Buffer       showBufferUv;
}wlayer_t;

//
// fblayer.
//

// window mixing.
typedef struct{
    IM_INT32    num;    // 0 is disabled, else is the number of mix window.
    IM_INT32    wxs; // window index, only for mix window, exclude the win0 
    ids_wlayer_position_t coords[3];
    IM_INT32    vmoffset[3]; // bytes offset
    IM_INT32    imgWidth;
    IM_INT32    pixfmt;
}wmix_t;

#define FBLAYER_FLAG_INITED		(1<<0)
#define FBLAYER_FLAG_OPENED		(1<<1)
#define FBLAYER_BUFFER_SET		(1<<4)
typedef struct{
	IM_INT32	flag; // FBLAYER_FLAG_xxx.
	wlayer_t	*wlayer;	// pointer link to the idsMgr[x].wlayer.
    IM_Buffer   showBuffer; // used for PM, record the newest show buffer.

    wmix_t      wmix;
}fblayer_t;

//
// vmode state-machine has 2 states, stable and unstable.
// meanwhile, there has some flag, INITED, SETUP, OK.
//
// stable   setup       meanning
// 0        0           local setup(exit extend) building...
// 0        1           extend setup building...
// 1        0           local stable
// 1        1           extend stable
//
#define VMODE_FLAG_INITED		(1<<0)
#define VMODE_FLAG_STABLE		(1<<1)
#define VMODE_FLAG_EXT_SETUP	(1<<2)  // extend setup flag
#define VMODE_FLAG_HIDELOC      (1<<3)

#define VMODE_FLAG_CHECK_STABLE(flag)    \
    (((flag & VMODE_FLAG_INITED) && (flag & VMODE_FLAG_STABLE)) ? IM_TRUE : IM_FALSE)
#define VMODE_FLAG_CHECK_EXT_SETUP(flag)    \
    (((flag & VMODE_FLAG_INITED) && (flag & VMODE_FLAG_EXT_SETUP)) ? IM_TRUE : IM_FALSE)
#define VMODE_FLAG_CHECK_EXT_VALID(flag)  \
    (((flag & VMODE_FLAG_INITED) && (flag & VMODE_FLAG_EXT_SETUP) && (flag & VMODE_FLAG_STABLE)) ? IM_TRUE : IM_FALSE)


#define VM_THREAD_STAT_UNKNOWN		0
#define VM_THREAD_STAT_RUN		1
#define VM_THREAD_ACTION_START		1
#define VM_THREAD_ACTION_STOP		2

typedef struct{
	IM_INT32		flag; // VMODE_FLAG_xxx.
	idslib_func_idsdrv_listener_t	listener;
	ids_vmode_config_t	config;

	// vmode thread.
	void *			thread;
	void *			signal;

	IM_INT32		stat;
	IM_INT32		action;
	im_list_handle_t	evtlst;		// list of vmode_evt_internal_t.

	IM_BOOL			extDevSel;	// mark if ext device has set, it's precondition of set mode.
	IM_BOOL 		shareBuffer;

	IM_Buffer 		buffA;
	IM_Buffer 		buffB;
	IM_BOOL			buffAIsBack;

	IM_Buffer	*srcBuffer;
	IM_Buffer	*dstBuffer;

	IM_INT32	rotate;	// IDS_ROTATION_xxx.
    IM_INT32    eeval;

	IM_BOOL			waitSwapFbBuffer;	// when the first swap buffer, needs to open the window.

	fblayer_t       *extfblayer;
}vmode_t;

typedef struct{
	IM_INT32		evt;
	IM_INT32		idsx;
	IM_INT32		wx;
	IM_UINT32		p[32];
}vmode_evt_internal_t;


typedef struct {
	idslib_func_idsdrv_listener_t	listener;
}extlayer_t;

//
//
#define IE_FLAG_INITED          (1<<0)
#define IE_FLAG_EMPLOYEED       (1<<1)
#define IE_FLAG_DITHER_OPENED   (1<<4)
typedef struct{
	ie_handle_t  	handle;
    IM_INT32        idsx;
	IM_INT32		flag;   // IE_FLAG_xxx;
}ie_t;

//
//
//
#define DISPDEV_FLAG_INITED	    (1<<0)
#define DISPDEV_FLAG_OPENED	    (1<<1)
#define DISPDEV_FLAG_CONFIGURED (1<<2)
#define DISPDEV_FLAG_BREAKED    (1<<3)
#define DISPDEV_FLAG_LISTENING	(1<<4)
#define DISPDEV_FLAG_SUSPEND	(1<<8)
typedef struct{
	IM_INT32		type;
	IM_INT32		idsx;
	IM_BOOL			dynamic;
	IM_INT32		suffixStart; // the start suffix of "x" of ids.loc/ext.devx in initcfg.
	dispdev_interface_t	*intf;
	im_list_handle_t	cfglst;	// list of ids_display_device_t.
	IM_UINT32			flag;	// DISPDEV_FLAG_xxx.
	dispdev_handle_t	handle;

	// current
	IM_INT32		suffix;
	ids_display_device_t	config;
	dispdev_info_t		info;
	ids_wlayer_canvas_t	canvas;
}dispdev_t;

//
//
//
#define IDSMGR_FLAG_HW_INITED	(1<<0)
typedef struct{
	IM_INT32	flag; // IDSMGR_FLAG_xxx.
	IM_UINT32	resource;	// RES_xxx bit or.
	wlayer_t	wlayer[4];
	IM_INT32	asyncConfigRefcnt;

	dispdev_t	*dev;	// link to the selected dev of the gIdsInst->devs.
}idsmgr_t;

//
// suspend & resume
//
#define PM_WLFLAG_OPEN		(1<<0)
#define PM_DEVFLAG_OPEN		(1<<0)
#define PM_IDSHWFLAG_INIT	(1<<0)
#define PM_OTHERFLAG_FB_SWAP_BUFFER (1<<0)
typedef struct{
	// wlayer.
	IM_INT32	wlFlagStore[2][4];

	// device.
	IM_INT32	devFlagStore[2];

	// idshw.
	IM_INT32	idshwFlagStore[2];

    // other flag.
    IM_INT32    otherFlag;

	// ids register.
	IM_UINT32	osdRegStore[2][OSDREGNUM];
}pm_t;


//
//
//
typedef struct{
    IM_INT32        busEffic;       // IDSLIB_BUS_EFFIC_xxx.
    IM_INT32        workMode[2];    // IDSLIB_WORKMODE_xxx.
}effic_t;

//
//
//
#define IDS_MODE_FB_L_to_E			0	// fb is loc, to extend
#define IDS_MODE_FB_to_L_E			1	// fb is standalone, to local and extend.
#define IDS_MODE_EXTLAYER_W0_USE	(0x01 << 8)
#define IDS_MODE_EXTLAYER_W1_USE	(0x01 << 9)
#define IDS_MODE_EXTLAYER_W2_USE	(0x01 << 10)
#define IDS_MODE_EXTLAYER_W3_USE	(0x01 << 11)
#define IDS_MODE_EXTLAYER_WALL_USE	(0x0F << 8)

#define IDS_MODE_EXTLAYER_FLAG		(0x01 << 12)
#define IDS_MODE_VMODE_USE			(0x01 << 13)

#define IDS_MODE_FB_MODE(flag)  (flag & 0xFF)

typedef struct{
	idslib_init_config_t	config;
	IM_INT32		mode;	// IDS_MODE_xxx.
	IM_BOOL			G2DSuspendInit;

    IM_BOOL         suspendFlag;
	IM_INT32		locidsx;
	IM_INT32		extidsx;
	im_mempool_handle_t	mpl;
	idsmgr_t		idsMgr[2];
	dispdev_t		devs[2][IDS_DISPDEV_TYPE_MAX];
	fblayer_t		fblayer;

	extlayer_t 		extlayer;

	vmode_t			vmode;
	void *			vmlock; // CAUTION: only allow got vmlock then get vmode->devlock.
	pm_t			pm;
	ie_t			ie[2];

    effic_t         effic;
}ids_instance_t;


#endif	// __IDS_INTERNAL_H__
 
