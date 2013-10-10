/**
 * Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 * 
 * Revision History: 
 * ----------------- 
 * 1.0.1	leo@2011/05/11: first commit.
 */


#ifndef __CAMIF_LIB_H__
#define __CAMIF_LIB_H__

//############################register offset##################################
enum{
	CISRCFMT = 0,
	CIWDOFST,
	CIGCTRL,	
	CICOTRGSIZE,
	CICOTRGFMT,
	CIPRTRGSIZE,
	CIPRTRGFMT,
	CIPRSCPRERATIO,
	CIIMGCPT,
	CICPTSTATUS,
	CIOBSSTATUS,
	CIDGBCH4CNT,
	CIDGBCH3CNT,
	CIDGBCH2CNT,
	CIDGBCH1CNT,
	CIDGBCH0CNT,
	CIPRFIFOSTATUS,
	CICOFIFOSTATUS,	
	CIMX_COEF11,
	CIMX_COEF12,
	CIMX_COEF13,
	CIMX_COEF21,
	CIMX_COEF22,
	CIMX_COEF23,
	CIMX_COEF31,
	CIMX_COEF32,
	CIMX_COEF33,
	CIMX_CFG,
	CICH0_ST_ADR,
	CICH0_CTRL,
	CICH0_ALT_ST_ADR_1,
	CICH0_ALT_CTRL_1,
	CICH0_ALT_ST_ADR_2,
	CICH0_ALT_CTRL_2,
	CICH0_ALT_ST_ADR_3,
	CICH0_ALT_CTRL_3,
	CICH1_ST_ADR,
	CICH1_CTRL,
	CICH1_ALT_ST_ADR_1,
	CICH1_ALT_CTRL_1,
	CICH1_ALT_ST_ADR_2,
	CICH1_ALT_CTRL_2,
	CICH1_ALT_ST_ADR_3,
	CICH1_ALT_CTRL_3,
	CICH2_ST_ADR,
	CICH2_CTRL,
	CICH2_ALT_ST_ADR_1,
	CICH2_ALT_CTRL_1,
	CICH2_ALT_ST_ADR_2,
	CICH2_ALT_CTRL_2,
	CICH2_ALT_ST_ADR_3,
	CICH2_ALT_CTRL_3,
	CICH3_ST_ADR,
	CICH3_CTRL,
	CICH3_ALT_ST_ADR_1,
	CICH3_ALT_CTRL_1,
	CICH3_ALT_ST_ADR_2,
	CICH3_ALT_CTRL_2,
	CICH3_ALT_ST_ADR_3,
	CICH3_ALT_CTRL_3,
	CICH4_ST_ADR,
	CICH4_CTRL,
	CICH4_ALT_ST_ADR_1,
	CICH4_ALT_CTRL_1,
	CICH4_ALT_ST_ADR_2,
	CICH4_ALT_CTRL_2,
	CICH4_ALT_ST_ADR_3,
	CICH4_ALT_CTRL_3
};

#define CIREGNUM	(CICH4_ALT_CTRL_3 + 1)

//############################bit offset#######################################
//
// CISRCFMT
#define	CISRCFMT_ITU601or656		(31)
#define	CISRCFMT_UVOffset		(30)
#define	CISRCFMT_ScanMode		(29)
#define	CISRCFMT_Order422		(14)

// CIWDOFST
#define	CIWDOFST_WinHorOfst		(16)
#define	CIWDOFST_WinVerOfst		(0)

// CIGCTRL
#define	CIGCTRL_IRQ_OvFiCH4_en		(31)
#define	CIGCTRL_IRQ_OvFiCH3_en		(30)
#define	CIGCTRL_IRQ_OvFiCH2_en		(29)
#define	CIGCTRL_IRQ_OvFiCH1_en		(28)
#define	CIGCTRL_IRQ_OvFiCH0_en	    	(27)
#define CIGCTRL_DEBUG_EN		(26)
#define	CIGCTRL_IRQ_en	    		(24)
#define	CIGCTRL_IRQ_Int_Mask_Pr		(22)
#define	CIGCTRL_IRQ_Int_Mask_Co		(20)
#define	CIGCTRL_IRQ_Bad_SYN_en		(19)
#define	CIGCTRL_InvPolCAMPCLK		(5)
#define	CIGCTRL_InvPolCAMVSYNC	    	(4)
#define	CIGCTRL_InvPolCAMHREF	    	(3)
#define	CIGCTRL_InvPolCAMFIELD		(2)
#define	CIGCTRL_CamRst			(1)
#define	CIGCTRL_SwRst			(0)

// CICOTRGSIZE
#define	CICOTRGSIZE_TargetHsize		(16)
#define	CICOTRGSIZE_TargetVsize		(0)

// CICOTRGFMT
#define	CICOTRGFMT_ycc422		(31)
#define	CICOTRGFMT_HalfWordSwap		(30)
#define	CICOTRGFMT_ByteSwap		(29)
#define	CICOTRGFMT_StoredFormat		(15)

// CIPRTRGSIZE
#define	CIPRTRGSIZE_TargetHsize		(16)
#define	CIPRTRGSIZE_TargetVsize		(0)

// CIPRTRGFMT
#define	CIPRTRGFMT_HalfWordSwap		(30)
#define	CIPRTRGFMT_ByteSwap		(29)
#define	CIPRTRGFMT_BPP24BL		(15)
#define	CIPRTRGFMT_BPP16Format	    	(14)
#define	CIPRTRGFMT_StoreFormat	    	(0)

// CIPRSCPRERATIO
#define	CIPRSCPRERATIO_PreHorRatio	(16)
#define	CIPRSCPRERATIO_PreVerRatio	(0)

// CIIMGCPT
#define	CIIMGCPT_CAMIF_EN		(31)
#define	CIIMGCPT_ImgCptEn_CoSc		(30)
#define	CIIMGCPT_ImgCptEn_PrSc		(29)
#define	CIIMGCPT_OneShot_CoSc		(28)
#define	CIIMGCPT_OneShot_PrSc		(27)

// CICPTSTATUS
#define	CICPTSTATUS_OvFiCH4_Pr		(31)
#define	CICPTSTATUS_OvFiCH3_Pr		(30)
#define	CICPTSTATUS_OvFiCH2_Co		(29)
#define	CICPTSTATUS_OvFiCH1_Co		(28)
#define	CICPTSTATUS_OvFiCH0_Co		(27)
#define	CICPTSTATUS_UNFICH4STS		(26)
#define	CICPTSTATUS_UNFICH3STS		(25)
#define	CICPTSTATUS_UNFICH2STS		(24)
#define	CICPTSTATUS_UNFICH1STS		(23)
#define	CICPTSTATUS_UNFICH0STS		(22)
#define	CICPTSTATUS_P_PATH_FIFO_DIRTY_STATUS	(21)
#define	CICPTSTATUS_C_PATH_FIFO_DIRTY_STATUS	(20)
#define	CICPTSTATUS_BadSynFlag		(19)
#define	CICPTSTATUS_P_PATH_LEISURE	(18)
#define	CICPTSTATUS_C_PATH_LEISURE	(17)
#define	CICPTSTATUS_DMA_CH4_Once	(16)
#define	CICPTSTATUS_DMA_CH3_Once	(15)
#define	CICPTSTATUS_DMA_CH2_Once	(14)
#define	CICPTSTATUS_DMA_CH1_Once	(13)
#define	CICPTSTATUS_DMA_CH0_Once	(12)
#define	CICPTSTATUS_DMA_CH4_Twice	(11)
#define	CICPTSTATUS_DMA_CH3_Twice	(10)
#define	CICPTSTATUS_DMA_CH2_Twice	(9)
#define	CICPTSTATUS_DMA_CH1_Twice	(8)
#define	CICPTSTATUS_DMA_CH0_Twice	(7)
#define	CICPTSTATUS_Smart_Over_Status_Pr	(6)
#define	CICPTSTATUS_Smart_Over_Status_Co	(5)
#define	CICPTSTATUS_Frame_Over_Status_Pr	(4)
#define	CICPTSTATUS_Frame_Over_Status_Co	(3)
#define	CICPTSTATUS_P_PATH_DMA_SUCCESS	(2)
#define	CICPTSTATUS_C_PATH_DMA_SUCCESS	(1)
#define	CICPTSTATUS_DMA_HAS_BEGIN_POP	(0)

// CIPRFIFOSTATUS
#define	CIPRFIFOSTATUS_CH4_fifo_wordcnt	(16)
#define	CIPRFIFOSTATUS_CH3_fifo_wordcnt	(0)

// CICOFIFOSTATUS
#define	CICOFIFOSTATUS_CH2_fifo_wordcnt	(16)
#define	CICOFIFOSTATUS_CH1_fifo_wordcnt	(8)
#define	CICOFIFOSTATUS_CH0_fifo_wordcnt	(0)

// CIMATRIX
#define	CIMATRIX_CFG_toRGB		(31)
#define	CIMATRIX_CFG_PassBy		(30)
#define	CIMATRIX_CFG_InvMsbIn		(29)
#define	CIMATRIX_CFG_InvMsbOut		(28)
#define	CIMATRIX_CFG_OftB		(8)
#define	CIMATRIX_CFG_OftA		(0)

// CIOBSSTATUS
#define	CIOBSSTATUS_VSYNC		(23)
#define	CICPTSTATUS_ImgCptEn_CoSc	(21)
#define	CICPTSTATUS_ImgCptEn_PrSc	(20)

// CICHxCTRL
#define	CICHxCTRL_DMAEn			(31)
#define	CICHxCTRL_RST			(30)
#define	CICHxCTRL_WorkRegInd		(28)
#define	CICHxCTRL_AltEn			(27)
#define	CICHxCTRL_Autoload		(26)
#define	CICHxCTRL_Dir			(25)
#define	CICHxCTRL_DMALen		(0)

// CICHxALTCTRL
#define	CICHxALTCTRL_AltEn		(27)
#define	CICHxALTCTRL_Autoload		(26)
#define	CICHxALTCTRL_Dir		(25)
#define	CICHxALTCTRL_DMALen		(0)


//#############################################################################
//
// ITU输入方式，[ITU601]/[ITU656]
//
#define CAMIF_ITU_MODE_601	1
#define CAMIF_ITU_MODE_656	0

//
// 输入扫描模式，[逐行]/[隔行]
//
#define CAMIF_SCAN_MODE_PROGRESSIVE	0
#define CAMIF_SCAN_MODE_INTERLACED	1	

//
// Y,cb,cr输入顺序， [YcbYcr]/[YcrYcb]/[cbYcrY]/[crYcbY]
//
#define CAMIF_INPUT_YUV_ORDER_YCbYCr	0
#define CAMIF_INPUT_YUV_ORDER_YCrYCb	1
#define CAMIF_INPUT_YUV_ORDER_CbYCrY	2
#define CAMIF_INPUT_YUV_ORDER_CrYCbY	3

//
//the polarity of CAMPCLK
//
#define CAMIF_POLARITY_CAMPCLK_NORMAL	0
#define CAMIF_POLARITY_CAMPCLK_INVERSE	1

//
//the polarity of CAMVSYNC
//
#define CAMIF_POLARITY_CAMVSYNC_NORMAL	0
#define CAMIF_POLARITY_CAMVSYNC_INVERSE	1

//
//the polarity of CAMHREF
//
#define CAMIF_POLARITY_CAMHREF_NORMAL	0
#define CAMIF_POLARITY_CAMHREF_INVERSE	1

//
//the polarity of CAMFIELD
//
#define CAMIF_POLARITY_CAMFIELD_NORMAL	0
#define CAMIF_POLARITY_CAMFIELD_INVERSE	1

//
//
// p-path和c-path的中断源
//
#define CAMIF_INTR_SOURCE_FRAME_END	1
#define CAMIF_INTR_SOURCE_DMA_DONE	2
#define CAMIF_INTR_SOURCE_SMART		3

//
// 采样模式和存储模式, [7:4] sample mode, [3:0]store mode.
// sample mode: 0--rgb16, 1--rgb24, 2--yuv420, 3--yuv422
// store mode: 0--565, 1--555I, 2--0888, 3--8880, 4--planar, 5--semiplanar, 6--interleaved
//
#define CAMIF_SAMPLE_MODE_RGB16BPP_565		0x0	// preview path
#define CAMIF_SAMPLE_MODE_RGB16BPP_555I		0x01	// preview path
#define CAMIF_SAMPLE_MODE_RGB24BPP_0888		0x12	// preview path
#define CAMIF_SAMPLE_MODE_RGB24BPP_8880		0x13	// preview path
#define CAMIF_SAMPLE_MODE_YUV420_PLANAR		0x24	// codec path
#define CAMIF_SAMPLE_MODE_YUV420_SEMIPLANAR	0x25	// preview&codec path
#define CAMIF_SAMPLE_MODE_YUV422_PLANAR		0x34	// codec path
#define CAMIF_SAMPLE_MODE_YUV422_SEMIPLANAR	0x35	// preview&codec path
#define CAMIF_SAMPLE_MODE_YUV422_INTERLEAVED	0x36	// codec path

//
// 转换矩阵的方向，[RGB-->YUV]/[YUV-->RGB]
//
#define CAMIF_MATRIX_DIR_RGBtoYUV	0
#define CAMIF_MATRIX_DIR_YUVtoRGB	1		

//
// camif path.
//
#define CAMIF_PATH_PREVIEW	0
#define CAMIF_PATH_CODEC	1

//
// ping-ping buffer mode
//
#define CAMIF_PPMODE_DISABLE	0	// user provide buff[0], internal only use buff[0]
#define CAMIF_PPMODE_1_BUFFER	1	// user provide buff[0], internal buff[1--3] = buff[0]
#define CAMIF_PPMODE_2_BUFFER	2	// user provide buff[0] and buff[1], internal buff[2] = buff[0], buff[3] = buff[1]
#define CAMIF_PPMODE_4_BUFFER	4	// user provide buff[0], buff[1], buff[2], buff[3]

//
// codec窗口限制
//
#define	CAMIF_CO_XSIZE_MIN		(4)
#define	CAMIF_CO_YSIZE_MIN		(2)
#define	CAMIF_CO_XSIZE_MAX		(0x1FFF)
#define	CAMIF_CO_YSIZE_MAX		(0x1FFF)

//
// preview窗口限制
//
#define	CAMIF_PR_XSIZE_MIN		(4)
#define	CAMIF_PR_YSIZE_MIN		(2)
#define	CAMIF_PR_XSIZE_MAX		(0x1FFF)
#define	CAMIF_PR_YSIZE_MAX		(0x1FFF)

//
// camif中断原因
//
#define CAMIF_INTR_PREVIEW	(1<<0)
#define CAMIF_INTR_CODEC	(1<<1)
#define CAMIF_INTR_ERROR	(1<<2)

//
// camif数据源描述
//
typedef struct{
	IM_UINT32	xSize;
	IM_UINT32	ySize;
	IM_UINT32	xOffset;
	IM_UINT32	yOffset;

	IM_INT32	ituMode;	// CAMIF_ITU_MODE_xxx.
	IM_INT32	scanMode;	// CAMIF_SCAN_MODE_xxx.
	IM_INT32	yuvOrder;	// CAMIF_INPUT_YUV_ORDER_xxx.
	IM_BOOL		uvOffset;
	IM_BOOL		invPinPclk;
	IM_BOOL		invPinVsync;
	IM_BOOL		invPinHref;
	IM_BOOL		invPinField;
}CAMIF_SOURCE_CONFIG;

//
// yuv to rgb matrix
//
#define CAMIF_MATRIX_MODE_DEFAULT	0
#define CAMIF_MATRIX_MODE_CUSTOM	100
typedef struct{
	IM_UINT32	mode;		// CAMIF_MATRIX_MODE_xxx 

	IM_BOOL		yuv2rgb;	// must be IM_TRUE
	IM_UINT32	oft_a;	// range: 0--31
	IM_UINT32	oft_b;	// range: 0--31
	IM_INT32	coef11;	// range: -1024 -- 1023
	IM_INT32	coef12;	// range: -1024 -- 1023
	IM_INT32	coef13;	// range: -1024 -- 1023
	IM_INT32	coef21;	// range: -1024 -- 1023
	IM_INT32	coef22;	// range: -1024 -- 1023
	IM_INT32	coef23;	// range: -1024 -- 1023
	IM_INT32	coef31;	// range: -1024 -- 1023
	IM_INT32	coef32;	// range: -1024 -- 1023
	IM_INT32	coef33;	// range: -1024 -- 1023
}CAMIF_CSC_MATRIX;

//
// camif通道描述
//
typedef struct{
	IM_UINT32	sampleMode;	// CAMIF_SAMPLE_MODE_xxx
	IM_BOOL		halfWordSwap;
	IM_BOOL		byteSwap;
	IM_BOOL		interlaced;
	IM_UINT32	width;		// range: CAMIF_xx_XSIZE_MIN--CAMIF_xx_XSIZE_MAX, multiple of 4
	IM_UINT32	height;		// range: CAMIF_xx_YSIZE_MIN--CAMIF_xx_YSIZE_MAX, multiple of 2
	IM_UINT32	intrMode;	// CAMIF_INTR_SOURCE_xxx

	IM_UINT32	ppMode;		// CAMIF_PPMODE_xxx
	IM_Buffer	buffY[4];	// y, rgb, ycbycr
	IM_Buffer	buffCb[4];	// cb, cbcr
	IM_Buffer	buffCr[4];	// cr
	CAMIF_CSC_MATRIX	matrix;	// Only for preview
}CAMIF_PATH_CONFIG;

typedef struct{
	IM_BOOL		enabled;
	IM_BOOL		configured;
	IM_BOOL		oneshot;

	IM_UINT32	lengthY;	// 0 indicate not in use
	IM_UINT32	lengthCb;	// 0 indicate not in use
	IM_UINT32	lengthCr;	// 0 indicate not in use

	IM_UINT32	ratioX;
	IM_UINT32	ratioY;
}CAMIF_PATH_STATUS;

//
// camif 中断控制结构
//
typedef struct{
	IM_BOOL	ch0OverFlow;
	IM_BOOL	ch1OverFlow;
	IM_BOOL	ch2OverFlow;
	IM_BOOL	ch3OverFlow;
	IM_BOOL	ch4OverFlow;
	IM_BOOL	badSync;
}CAMIF_ERR_INTR_CONFIG;

//
// camif
//
typedef struct{
	CAMIF_SOURCE_CONFIG	source;
	CAMIF_ERR_INTR_CONFIG	errintr;
	CAMIF_PATH_CONFIG	path[2];
	CAMIF_PATH_STATUS	pstat[2];
	
	struct{
		IM_UINT32	offset;
		IM_UINT32	value;
	}regs[CIREGNUM];
}CAMIF;

//====================================interface===============================
/**
 * call sequence:
 *	1. camiflib_init()
 *		initialize camif;
 *	2. camiflib_set_source(), camiflib_set_err_intr(), 
 *		configure camif. 
 *	3. camiflib_set_preview(), camiflib_set_codec()
 *		configure preview and codec.
 *	4. camiflib_start_preview(), camiflib_start_codec()
 *		start capture. when P path started, then set C path width and height, 
 *		before start C path, must stop P path first, then re-start P path.
 *	5. camiflib_set_source_offset(), camiflib_set_matrix()
 *	   camiflib_wait_hw_ready(), camiflib_get_ready_buffer()
 *	   	capture data.
 *	6. camiflib_stop_preview(), camiflib_stop_codec()
 *		stop capture path.
 *	7. camiflib_deinit()
 *		relese camiflib.
 */

/*
 * FUNC: init/deinit camiflib.
 * PARAM: none.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_init(void);
IM_RET camiflib_deinit(void);

/*
 * FUNC: check register default value, write 0/1 test.
 * PARAM: none.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_check_register(void);

/*
 * FUNC: set source configuration.
 * PARAM: srccfg, source configure.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_source(CAMIF_SOURCE_CONFIG *srccfg);

/*
 * FUNC: set camif error interrupt.
 * PARAM: intrcfg, interrupt cofigure.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_err_intr(CAMIF_ERR_INTR_CONFIG *intrcfg);

/*
 * FUNC: set preview configure.
 * PARAM: precfg, preview configure.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_preview(CAMIF_PATH_CONFIG *precfg);

/*
 * FUNC: set codec configure.
 * PARAM: cocfg, codec configure.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_codec(CAMIF_PATH_CONFIG *cocfg);

/*
 * FUNC: start preview path.
 * PARAM: oneshot, just oneshot?
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_start_preview(IM_BOOL oneshot);

/*
 * FUNC: stop preview capture.
 * PARAM: none.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_stop_preview(void);

/*
 * FUNC: start codec path.
 * PARAM: oneshot, just oneshot?
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_start_codec(IM_BOOL oneshot);

/*
 * FUNC: stop codec capture.
 * PARAM: none.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_stop_codec(void);

/*
 * FUNC: wait hw ready.
 * PARAM: intr, which intr happend, CAMIF_INTR_xxx.
 * 	timeout, timeout of this wait.
 * RETURN: IM_RET_OK succeed, IM_RET_TIMEOUT and others failed.
 */
IM_RET camiflib_wait_hw_ready(IM_UINT32 *intr, IM_INT32 timeout);

/*
 * FUNC: get the path's ready buffer.
 * PARAM: path, CAMIF_PATH_PREVIEW or CAMIF_PATH_CODEC.
 * 	readyBuffs, ready buffers, include y, cb, cr element, depent on sampleMode.
 * 	replaceBuffs, replace buffers, include y, cb, cr element, depent on sampleMode.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_get_ready_buffer(IM_UINT32 path, IM_Buffer *readyBuffs, IM_Buffer *replaceBuffs);

/*
 * FUNC: set source window offset.
 * PARAM: xOffset, x offset.
 * 	yOffset, y offset.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_source_offset(IM_UINT32 xOffset, IM_UINT32 yOffset);

/*
 * FUNC: set yuv2rgb matrix.
 * PARAM: matrix, matrix configure.
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_matrix(CAMIF_CSC_MATRIX *matrix);

/*
 * FUNC: camif reset camif ip module.
 * PARAM: 
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_hw_reset(void);

/*
 * FUNC: camif set sensor-reset pin to control sensor reset.
 * PARAM: 
 * RETURN: IM_RET_OK succeed, else failed.
 */
IM_RET camiflib_set_reset_pin(IM_BOOL elecLevel);

//#############################################################################
#endif	// __CAMIF_LIB_H__


