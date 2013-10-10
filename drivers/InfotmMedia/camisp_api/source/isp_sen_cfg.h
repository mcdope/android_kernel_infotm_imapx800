/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file isp_sensor_config.h
--
--  Description :
--		config file to isp for each sensor	
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/05/04: first commit.
-- v1.0.2	arsor@2012/07/16: add default config of gt2005_pc0am0008a 
--			and gt2005_pc0am0004b.
--
------------------------------------------------------------------------------*/

#define	PRO_SPE_EFT_NONE				0
#define	PRO_SPE_EFT_MONO				1
#define	PRO_SPE_EFT_NEGATIVE			2
#define	PRO_SPE_EFT_SOLARIZE			3
#define	PRO_SPE_EFT_PASTEL				4
#define	PRO_SPE_EFT_MOSAIC				5
#define	PRO_SPE_EFT_RESIZE				6
#define	PRO_SPE_EFT_SEPIA				7
#define	PRO_SPE_EFT_POSTERIZE			8
#define	PRO_SPE_EFT_WHITEBOARD			9
#define	PRO_SPE_EFT_BLACKBOARD			10
#define	PRO_SPE_EFT_AQUA				11
//#define	PRO_SPE_EFT_XXX				12
//...
//#define	PRO_SPE_EFT_LAST			19	//8 type reserve for other special effect maybe support in the future
#define	PRO_SPE_EFT_OFFSET				PRO_SPE_EFT_NONE

#define PRO_SCE_MODE_AUTO				20
#define PRO_SCE_MODE_ACTION				21
#define PRO_SCE_MODE_PORTRAIT			22
#define PRO_SCE_MODE_LANDSCAPE			23
#define PRO_SCE_MODE_NIGHT				24
#define PRO_SCE_MODE_NIGHT_PORTRAIT		25
#define PRO_SCE_MODE_THEATRE			26
#define PRO_SCE_MODE_BEACH				27
#define PRO_SCE_MODE_SNOW				28
#define PRO_SCE_MODE_SUNSET				29
#define PRO_SCE_MODE_STEADYPHOTO		30
#define PRO_SCE_MODE_FIREWORKS			31
#define PRO_SCE_MODE_SPORTS				32
#define PRO_SCE_MODE_PARTY				33
#define PRO_SCE_MODE_CANDLELIGHT		34
#define PRO_SCE_MODE_BARCODE 			35
//#define	PRO_SCE_MODE_XXX			36
//...
//#define	PRO_SCE_MODE_LAST			39	//4 type reserve for other scene mode maybe support in the future
#define PRO_SCE_MODE_OFFSET				PRO_SCE_MODE_AUTO

#define MAX_PRO (50)


typedef struct{
	IM_UINT32 request;
	IM_UINT32 supply;
}isp_resolution_caps_t;

typedef struct{
	IM_CHAR					senName[256];
	IM_INT32				flag;
	isp_resolution_caps_t	resCaps[CAM_RES_ENUM_MAX];
	cam_preview_config_t 	preCfg;
	isp_config_t			ispCfg;
}isp_sensor_init_t;


static isp_sensor_init_t gIspSenInit[] = {
	/*****************
	* for gc0308_xyc *
	*****************/
	{
		"gc0308_xyc",
		0x0000,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_QVGA}, 
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_QVGA | CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	* for gc0308_130w*
	*****************/
	{
		"gc0308_130w",
		0x0001,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_QVGA}, 
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_130W, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_QVGA | CAM_RES_VGA | CAM_RES_130W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{CAM_FPS_30},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	* for gt2005_xyc *
	*****************/
	{
		"gt2005_xyc",
		0x0002,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/************************
	* for gt2005_pc0am0008a *
	************************/
	{
		"gt2005_pc0am0008a",
		0x0003,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/************************
	* for gt2005_pc0am0004b *
	************************/
	{
		"gt2005_pc0am0004b",
		0x0004,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},
	/***************
	 * for hi704_yuv
	 **************/ 
	{
		"hi704_yuv",
		0x0005,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},
	/***************
	 * for hi704_hy
	 **************/ 
	{
		"hi704_hy",
		0x0006,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},
	/**************
	 * for hi253_yuv
	**************/ 
	{
		"hi253_yuv",
		0x0007,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599,						//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
			},
		},
	},
	/**************
	 * for hi253_hy
	**************/ 
	{
		"hi253_hy",
		0x0008,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599,						//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
			},
		},
	},

	/*******************
	 * for ov5640_zyhb
	*******************/ 
	{
		"ov5640_zyhb",
		0x0009,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_XVGA, CAM_RES_XVGA}, 
	        {CAM_RES_720P, CAM_RES_720P}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
			{CAM_RES_1080P, CAM_RES_1080P}, 
			{CAM_RES_320W, CAM_RES_320W}, 
			{CAM_RES_500W, CAM_RES_500W}, 
            {CAM_RES_WQXGA_U, CAM_RES_500W}, 
            {CAM_RES_12M, CAM_RES_500W}, 
		},
		//preview config
		{
            CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_XVGA | CAM_RES_720P | CAM_RES_UXGA | CAM_RES_1080P | CAM_RES_320W | CAM_RES_500W | CAM_RES_WQXGA_U | CAM_RES_12M, 
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{CAM_FPS_5},//XVGA
			    {CAM_FPS_30},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_5},//UXGA
			    {CAM_FPS_30},//1080P
                {CAM_FPS_5},//320W
				{0},//WQXGA
				{CAM_FPS_5},//500W
				{0},//QUXGA
				{CAM_FPS_5},//WQXGA_U
				{CAM_FPS_5},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479,						//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
			},
		},
	},

	/*****************
	* for gc0308_hy_130w*
	*****************/
	{
		"gc0308_hy_130w",
		0x000a,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_QVGA}, 
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_130W, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_QVGA | CAM_RES_VGA | CAM_RES_130W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{CAM_FPS_30},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/************************
	* for gt2005_500w *
	************************/
	{
		"gt2005_500w",
		0x000b,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
			{CAM_RES_500W, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA | CAM_RES_500W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{CAM_FPS_15},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	* for sp0838_130w*
	*****************/
	{
		"sp0838_130w",
		0x000c,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_130W, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA | CAM_RES_130W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{CAM_FPS_30},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/************************
	* for sp2518_500w *
	************************/
	{
		"sp2518_500w",
		0x000d,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
			//{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
			{CAM_RES_500W, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA /*| CAM_RES_SVGA*/ | CAM_RES_UXGA | CAM_RES_500W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{CAM_FPS_15},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/***********************
	* for gc0308_demo_130w *
	***********************/
	{
		"gc0308_demo_130w",
		0x000e,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_QVGA}, 
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_130W, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_QVGA | CAM_RES_VGA | CAM_RES_130W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{CAM_FPS_30},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/************************
	* for gc2035_demo_500w *
	************************/
	{
		"gc2035_demo_500w",
		0x000f,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
			{CAM_RES_500W, CAM_RES_UXGA}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA | CAM_RES_500W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{CAM_FPS_15},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*******************
	 * for 5642_demo
	*******************/ 
	{
		"ov5642_demo",
		0x0010,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_SVGA},//CAM_RES_SVGA}, 
		//	{CAM_RES_VGA, CAM_RES_VGA}, 
		//	{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_XVGA, CAM_RES_XVGA}, 
	        //	{CAM_RES_720P, CAM_RES_720P}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		//	{CAM_RES_1080P, CAM_RES_1080P}, 
		//	{CAM_RES_320W, CAM_RES_320W}, 
			{CAM_RES_500W, CAM_RES_500W}, 
            	//	{CAM_RES_WQXGA_U, CAM_RES_500W}, 
            	//	{CAM_RES_12M, CAM_RES_500W}, 
		},
		//preview config
		{
            		//CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_XVGA | CAM_RES_720P | CAM_RES_UXGA | CAM_RES_1080P | CAM_RES_320W | CAM_RES_500W | CAM_RES_WQXGA_U | CAM_RES_12M, 
			CAM_RES_VGA | CAM_RES_XVGA | CAM_RES_UXGA | CAM_RES_500W, 
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//{CAM_FPS_30},//SVGA
				{CAM_FPS_5},//XVGA
			    	{0},//{CAM_FPS_30},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				//{0},//UXGA
				{CAM_FPS_5},//UXGA
			    	{0},//{CAM_FPS_30},//1080P
                		{0},//{CAM_FPS_5},//320W
				{0},//WQXGA
				{CAM_FPS_5},//500W
				{0},//QUXGA
				{0},//{CAM_FPS_5},//WQXGA_U
				{0},//{CAM_FPS_5},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599,						//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
			},
		},
	},

	/*****************
	* for mt9m114_demo *
	*****************/
	{
		"mt9m114_demo",
		0x0011,
		//resolution caps
		{
			//{CAM_RES_QVGA, CAM_RES_130W}, 
			//{CAM_RES_VGA, CAM_RES_130W}, 
			{CAM_RES_QVGA, CAM_RES_VGA}, 
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_720P, CAM_RES_720P}, 
			{CAM_RES_130W, CAM_RES_130W}, 
		},
		//preview config
		{
			//CAM_RES_VGA | CAM_RES_UXGA,
			CAM_RES_QVGA | CAM_RES_VGA | CAM_RES_720P | CAM_RES_130W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{CAM_FPS_15},//{0},//720P
				{CAM_FPS_15},//{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640/*1280*/,								//inWidth
			480/*960*/,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639/*1279*/,						//x1
					479/*959*/							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640/*1280*/,							//sclInWidth
				480/*960*/,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	* for mt9d115_demo *
	*****************/
	{
		"mt9d115_demo",
		0x0012,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_SVGA}, 
			{CAM_RES_VGA, CAM_RES_SVGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_720P, CAM_RES_720P}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			CAM_RES_QVGA | CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_720P | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{CAM_FPS_30},//SVGA
				{0},//XVGA
				{CAM_FPS_30},//{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			800,								//inWidth
			600,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292	,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					799,						//x1
					599							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				800,							//sclInWidth
				600,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	*for ov5650_p48a *
	*****************/
	{
		"ov5650_p48a",
		0x1000,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_720P, CAM_RES_720P}, 
			//{CAM_RES_SXGA, CAM_RES_SXGA}, 
			{CAM_RES_1080P, CAM_RES_1080P}, 
	        {CAM_RES_500W, CAM_RES_500W}, 
            {CAM_RES_WQXGA_U, CAM_RES_500W}, 
            {CAM_RES_12M, CAM_RES_500W}, 
		},
		//preview config
		{
			CAM_RES_VGA | CAM_RES_720P | /*CAM_RES_SXGA |*/ CAM_RES_1080P | CAM_RES_500W| CAM_RES_WQXGA_U | CAM_RES_12M,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{CAM_FPS_30},//720P
				{0},//130W
				{0/*CAM_FPS_22*/},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{CAM_FPS_30},//1080P
				{0},//320W
				{0},//WQXGA
				{CAM_FPS_15},//500W
				{0},//QUXGA
				{CAM_FPS_15},//WQXGA_U
				{CAM_FPS_15},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_IO_RAWRGB,			    //intfMode
			ISP_INPUT_LOW_BITS_10,				//inputBitsNum
			ISP_RAWDATAMODE_BGBG,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				1,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_FALSE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
				    639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	*for ov5650_mipi *
	*****************/
	{
		"ov5650_mipi",
		0x2000,
		//resolution caps
		{
			{CAM_RES_QVGA, CAM_RES_QVGA}, 
			//{CAM_RES_VGA, CAM_RES_VGA}, 
			//{CAM_RES_720P, CAM_RES_720P}, 
			//{CAM_RES_SXGA, CAM_RES_SXGA}, 
			//{CAM_RES_1080P, CAM_RES_1080P}, 
	        //{CAM_RES_500W, CAM_RES_500W}, 
		},
		//preview config
		{
			CAM_RES_QVGA,
			//CAM_RES_VGA | CAM_RES_720P | CAM_RES_SXGA | CAM_RES_1080P | CAM_RES_500W,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{CAM_FPS_30},//QVGA
				{0},//CIF
				{0},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_QVGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			320,								//inWidth
			240,								//inHeight
			ISP_INTFMODE_MIPI_RAWRGB,			//intfMode
			ISP_INPUT_LOW_BITS_10,				//inputBitsNum
			ISP_RAWDATAMODE_BGBG,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			320,								//outWidth
			240,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_TRUE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_FALSE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
				    639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				320,							//sclInWidth
				240,							//sclInHeight
				320,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				240,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	*for gc2035_gsg *
	*****************/
	{
		"gc2035_gsg",
		0x2001,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
			{CAM_RES_SVGA, CAM_RES_SVGA}, 
			{CAM_RES_UXGA, CAM_RES_UXGA}, 
		},
		//preview config
		{
			CAM_RES_VGA | CAM_RES_SVGA | CAM_RES_UXGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
                {CAM_FPS_30},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{CAM_FPS_15},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_MIPI_YUV,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_BGBG,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
		        IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
				    639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/*****************
	*for ov5640_fxn *
	*****************/
	{
		"ov5640_fxn",
		0x2002,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
                {0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_MIPI_YUV,			//intfMode
			ISP_INPUT_LOW_BITS_10,				//inputBitsNum
			ISP_RAWDATAMODE_BGBG,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			/*IM_IMAGE_RGB0888*/IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
		        IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
				    639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/**************************
	* for general mipi sensor *
	**************************/
	{
		"general_mipi_sensor",
		0x10000,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_MIPI_YUV,				//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},

	/**************************
	* for general dvp  sensor *
	**************************/
	{
		"general_dvp_sensor",
		0x20000,
		//resolution caps
		{
			{CAM_RES_VGA, CAM_RES_VGA}, 
		},
		//preview config
		{
			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP | CAM_PIXFMT_YUV420P | CAM_PIXFMT_32BPP_RGB0888,
			{
				{0},//QQCIF
				{0},//SUB_QCIF
				{0},//QQVGA
				{0},//QCIF
				{0},//QVGA
				{0},//CIF
				{CAM_FPS_30},//VGA
				{0},//480P
				{0},//PAL
				{0},//SVGA
				{0},//XVGA
				{0},//720P
				{0},//130W
				{0},//SXGA
				{0},//SXGAPlus
				{0},//UXGA
				{0},//1080P
				{0},//320W
				{0},//WQXGA
				{0},//500W
				{0},//QUXGA
				{0},//WQXGA_U
				{0},//12M
			},

			CAM_RES_VGA,
			CAM_PIXFMT_YUV420SP,
			CAM_FPS_30
		},
		//isp default config
		{
			640,								//inWidth
			480,								//inHeight
			ISP_INTFMODE_ITU_YUV422,			//intfMode
			ISP_INPUT_LOW_BITS_8,				//inputBitsNum
			ISP_RAWDATAMODE_GRGR,				//rawMode
			//ispsen_itu_type_t	ituType
			{
				ISP_ITU_SCAN_PROGRESSIVE,		//scanMode
				ISP_ITU_FORMAT_ITU601,			//format
				ISP_ITU_ORDER_YUV422_YUYV,		//order
			},		
			//ispsen_signal_polarity sigPol
			{
				0,								//hsync
				0,								//href
				0,								//vsync
				0								//pclk
			},
			640,								//outWidth
			480,								//outHeight
			IM_IMAGE_YUV420SP,					//outImgType
			IM_FALSE,							//demEnable
			//ispsen_bccb_config_t
			{
				IM_FALSE,						//bcEnable
				IM_FALSE,						//cbEnable
				ISP_BCCB_MODE_BCCB,				//bccbMode
				0,								//blkTh
				256,							//rGain
				256,							//gGain
				256								//bGain			
			},
			//ispsen_bdc_config_t
			{
				IM_FALSE,						//enable
				ISP_BDC_MODE_CORRECT,			//bdcMode
				ISP_BDC_DETECT_TYPE_0,			//detType
				0x7/*0*/,						//crtType
				3968,							//hiTh, use at detect mode
				128,							//loTh, use at detect mode
				4095,							//sltPepTh, use at correct mode
				ISP_BDC_CORRECT_NOISE_LEVEL_BYPASS,	//nosLvl,use at correct mode
				//dmaBuf
				{
					IM_NULL, 					//vir_addr
					0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
					0,							//size
					0,							//flag, [15:0] reserved, user cannot write to.
				}
			},
			//ispsen_lens_config_t
			{
				IM_FALSE,						//enable
				ISP_LENS_LUTB_MODE_1			//lutbMode 
			},
			//ispsen_gma_config_t
			{
				IM_FALSE,						//enable
				ISP_GMA_MODE_0,					//mode
				ISP_ROUND_MINUS					//rdMode
			},
			//ispsen_ee_config_t
			{
				IM_FALSE,						//enable
				0x33,							//coefw
				0x40,							//coefa
				ISP_ROUND_MINUS,				//rdMode
				IM_FALSE,						//gasEn
				ISP_EE_GAUSS_MODE_0,			//gasMode
				180,							//errTh
				//thrMat
				{
					40,							//hTh
					40,							//vTh
					40,							//d0Th
					40,							//d1Th
				}
			},
			//ispsen_fcc_config_t
			{
				IM_FALSE,						//enable
				16								//threshold
			},
			//ispsen_af_config_t
			{
				IM_FALSE,						//enable
				ISP_AF_TYPE_CENTRE,				//type
				//coordinate(for 640*480)
				{
					//pos0
					304,						//x00
					224,						//y00
					336,						//x01
					256,						//y01
					//pos1
					268,						//x10
					224,						//y10
					300,						//x11
					256,						//y11
					//pos2 
					340,						//x20
					224,						//y20
					372,						//x21
					256,						//y21 
					//pos3 
					304,						//x30
					188,						//y30
					336,						//x31
					220,						//y31 
					//pos4 
					304,						//x40
					260,						//y40
					336,						//x41
					292,						//y41 
					//pos5 
					236,						//x50
					180,						//y50
					268,						//x51
					212,						//y51 
					//pos6	 
					372,						//x60
					180,						//y60
					404,						//x61
					212,						//y61 
					//pos7 
					236,						//x70
					268,						//y70
					268,						//x71
					300,						//y71 
					//pos8 
					372,						//x80
					268,						//y80
					404,						//x81
					300							//y81 
				},
			},
			//ispsen_awb_config_t
			{
				IM_FALSE,						//enable
				IM_FALSE,						//anaEnable
				ISP_AWB_ANALYZE_MODE_IMP,		//anaMode
				//roiPos
				{
					0,							//x00
					639,						//x01
					0,							//y00
					479							//y01
				},
				//thrMat
				{
					10,							//thy1
					240,						//thy2
					60,							//costh
					102,						//errth
					307,						//coef1
					2,							//cosbth
					12,							//cbAmp
					12,							//crAmp	
					5							//meanth
				},
				//parMat
				{
					115,						//rpDown
					140,						//rpUp
					115,						//bpDown
					140							//bpUp
				}	
			},
			//ispsen_cmncsc_config_t
			{
				IM_FALSE,						//enable
				ISP_CMNCSC_RGB2YUV				//mode
			},
			//ispsen_ae_config_t
			{
				IM_FALSE,						//enable
				//blokSelect
				{
					25,							//blokNum
					0x1ffffff					//blokEn
				},
				//coordinate, for SIZE_640_480
				{
					13,							//log2_blokPixNum
					{
						//pos0
						0,							//x0
						16,							//y0
						127,						//x1
						79,							//y1
						//pos1
						128,						//x0
						16,							//y0
						255,						//x1
						79,							//y1
						//pos2
						256,						//x0
						16,							//y0
						383,						//x1
						79,							//y1 
						//pos3
						384,						//x0
						16,							//y0
						511,						//x1
						79,							//y1 
						//pos4
						512,						//x0
						16,							//y0
						639,						//x1
						79,							//y1
						//pos5
						0,							//x0
						112,						//y0
						127,						//x1
						175,						//y1
						//pos6
						128,						//x0
						112,						//y0
						255,						//x1
						175,						//y1
						//pos7
						256,						//x0
						112,						//y0
						383,						//x1
						175,						//y1 
						//pos8
						384,						//x0
						112,						//y0
						511,						//x1
						175,						//y1 
						//pos9
						512,						//x0
						112,						//y0
						639,						//x1
						175,						//y1
						//pos10
						0,							//x0
						208,						//y0
						127,						//x1
						271,						//y1
						//pos11
						128,						//x0
						208,						//y0
						255,						//x1
						271,						//y1
						//pos12
						256,						//x0
						208,						//y0
						383,						//x1
						271,						//y1 
						//pos13
						384,						//x0
						208,						//y0
						511,						//x1
						271,						//y1 
						//pos14
						512,						//x0
						208,						//y0
						639,						//x1
						271,						//y1
						//pos15
						0,							//x0
						304,						//y0
						127,						//x1
						367,						//y1
						//pos16
						128,						//x0
						304,						//y0
						255,						//x1
						367,						//y1
						//pos17
						256,						//x0
						304,						//y0
						383,						//x1
						367,						//y1 
						//pos18
						384,						//x0
						304,						//y0
						511,						//x1
						367,						//y1 
						//pos19
						512,						//x0
						304,						//y0
						639,						//x1
						367,						//y1
						//pos20
						0,							//x0
						400,						//y0
						127,						//x1
						463,						//y1
						//pos21
						128,						//x0
						400,						//y0
						255,						//x1
						463,						//y1
						//pos22
						256,						//x0
						400,						//y0
						383,						//x1
						463,						//y1 
						//pos23
						384,						//x0
						400,						//y0
						511,						//x1
						463,						//y1 
						//pos24
						512,						//x0
						400,						//y0
						639,						//x1
						463,						//y1
					}
				}
			},
			//ispsen_hist_config_t
			{
				IM_FALSE,						//enable
				20,								//blitTh1Num
				32,								//blitTh1Den
				//thrMat
				{
					64,							//th1
					192							//th2
				}
			},
			//ispsen_acc_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				4096*2,							//coefe
				//lutbType
				{
					ISP_ACC_LUTB_CONTROL_MODE_NORMAL, //lutbCtrMode
					ISP_ACC_LUTB_MODE_0			//lutbMode
				},
				//hist
				{
					0,							//histFrames
					ISP_ROUND_MINUS				//histRdMode
				},
				//coMat
				{
					3,							//coefa
					4,							//coefb	
					5,							//coefc
					7							//coefd
				},
				//roMat
				{
					101,						//roaHi
					0,							//roaLo
					255,						//robHi
					154,						//robLo
					179,						//rocHi
					51							//rocLo
				}
			},
			//ispsen_ief_config_t
			{
				IM_FALSE,						//enable
				IM_TRUE,						//cscEn
				ISP_IEF_TYPE_NORMAL,			//type
				//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
				{
					296,						//coefCr
					196							//coefCb
				},
				//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
				{
					ISP_IEF_COLOR_SELECT_MODE_R,	//mode
					96,							//tha
					128							//thb

				},
				//cscMat
				{
					ISP_IEF_CSC_YUV2RGB,		//cscMode
					1192,						//coef11
					0,							//coef12
					1634,						//coef13
					1192,						//coef21
					-402,						//coef22
					-833,						//coef23
					1192,						//coef31
					2065,						//coef32
					0,							//coef33
					0,							//oft_a
					32							//oft_b
				}
			},
			//ispsen_acm_config_t
			{
				IM_FALSE,						//enable
				ISP_ROUND_MINUS,				//rdMode
				2048*3,							//ths
				//thrMat
				{
					200,						//tha
					150,						//thb
					100							//thc
				},
				//coMat
				{
					2048,						//coefp
					2048,						//coefr	
					2048,						//coefg
					2048,						//coefb
					0x052,						//m0r
					0x1ec,						//m1r
					0x2e1,						//m2r
					0x333,						//m3r
					0x19a,						//m4r
					0x052,						//m0g
					0x1ec,						//m1g
					0x2e1,						//m2g
					0x333,						//m3g
					0x19a,						//m4g
					0x052,						//m0b
					0x1ec,						//m1b
					0x2e1,						//m2b
					0x333,						//m3b
					0x19a						//m4b
				}
			},

			//ispsen_osd_config_t
			{
				IM_FALSE,						//enable
				0x0000ff,						//bgColor
				640,							//outWidth
				480,							//outHeight
				//wnd0
				{
					IM_FALSE,						//enable
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					}
				},
				//wnd1
				{
					IM_FALSE,						//enable
					ISP_OSD_IMAGE_RGB_BPP24_888,	//imgFormat
					//palette
					{
						ISP_OSD_PALETTE_FORMAT_888,	//palFormat
						2,
						IM_NULL
					},
					//swap
					{
						IM_FALSE,		 		//bitSwap
						IM_FALSE,			 	//bits2Swap
						IM_FALSE,				//bits4Swap
						IM_FALSE,				//byteSwap
						IM_FALSE,				//halfwordSwap
					},
					//alpha
					{
						ISP_OSD_ALPHA_PATH_0,	//path:			ISP_OSD_ALPHA_PATH_x.
						ISP_OSD_BLEND_PER_PLANE,//blendMode:	ISP_OSD_BLEND_PER_xxx.
						0xf,					//alpha0_r:		range 0--15 
						0xf,					//alpha0_g:		range 0--15 
						0xf,					//alpha0_b:		range 0--15 
						0x0,					//alpha1_r:		range 0--15 
						0x0,					//alpha1_g:		range 0--15 
						0x0,					//alpha1_b:		range 0--15 
					},
					//mapclr
					{
						IM_FALSE,				//enable
						0x00ff00,				//color
					},
					//clrkey
					{
						IM_FALSE,				//enable
						IM_FALSE,				//enableBlend
						ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND, //matchMode
						0x0,					//mask
						0x0,					//color
					},
					//vm
					{
						640,					//width
						480,					//height
						0,						//xOffset
						0,						//yOffset
					},
					//coordinate
					{
						0,						//x0
						0,						//y0
						640,					//w
						480						//h
					},
					//buf
					{
						IM_NULL, 					//vir_addr
						0x0,						//phy_addr, if flag has IM_BUFFER_FLAG_PHY_NONLINEAR, phy_addr is the first block physical memory address.
						0,							//size
						0,							//flag, [15:0] reserved, user cannot write to.
					}
				}

			},
			//ispsen_crop_config_t
			{
				IM_TRUE,						//enable
				//coordinate
				{
					0,							//x0
					0,							//y0
					639,						//x1
					479							//y1
				}
			},
			//ispsen_scl_config_t
			{
				IM_TRUE,						//verEnable
				IM_TRUE,						//horEnable
				640,							//sclInWidth
				480,							//sclInHeight
				640,							//sclOutWidth;		(it is ISP last(real) outWidth if scl enable)
				480,							//sclOutHeight;		(it is ISP last(real) outHeight if scl enable)
				ISP_ROUND_MINUS,				//vrdMode
				ISP_ROUND_MINUS,				//hrdMode
			}
		},
	},
};


static isp_sub_config_t gIspSubCfg[][MAX_PRO] = {
	/*****************
	* for gc0308_xyc *
	*****************/
	{
		#include "gc0308_xyc_isp_config_table.h"
	},
	/*****************
	* for gc0308_130w*
	*****************/
	{
		#include "gc0308_130w_isp_config_table.h"
	},
	/*****************
	* for gt2005_xyc *
	*****************/
	{
		#include "gt2005_xyc_isp_config_table.h"
	},
	/************************
	* for gt2005_pc0am0008a *
	************************/
	{
		#include "gt2005_pc0am0008a_isp_config_table.h"
	},
	/************************
	* for gt2005_pc0am0004b *
	************************/
	{
		#include "gt2005_pc0am0004b_isp_config_table.h"
	},
	/************************
	* for hi704_yuv  *
	************************/
	{
		#include "hi704_yuv_isp_config_table.h"
	},
	/************************
	* for hi704_hy  *
	************************/
	{
		#include "hi704_hy_isp_config_table.h"
	},
	/************************
	* for hi253_yuv  *
	************************/
	{
		#include "hi253_yuv_isp_config_table.h"
	},
	/************************
	* for hi253_hy  *
	************************/
	{
		#include "hi253_hy_isp_config_table.h"
	},
	/************************
	* for ov5640_zyhb  *
	************************/
	{
		#include "ov5640_zyhb_isp_config_table.h"
	},
	/*****************
	* for gc0308_hy_130w*
	*****************/
	{
		#include "gc0308_hy_130w_isp_config_table.h"
	},
	/************************
	* for gt2005_500w *
	************************/
	{
		#include "gt2005_500w_isp_config_table.h"
	},
	/*****************
	* for sp0838_130w*
	*****************/
	{
		#include "sp0838_130w_isp_config_table.h"
	},
	/************************
	* for sp2518_500w *
	************************/
	{
		#include "sp2518_500w_isp_config_table.h"
	},
	/***********************
	* for gc0308_demo_130w *
	***********************/
	{
		#include "gc0308_demo_130w_isp_config_table.h"
	},
	/************************
	* for gc2035_demo_500w *
	************************/
	{
		#include "gc2035_demo_500w_isp_config_table.h"
	},
	/************************
	* for ov5642_demo  *
	************************/
	{
		#include "ov5642_demo_isp_config_table.h"
	},
	/************************
	* for mt9m114_demo  *
	************************/
	{
		#include "mt9m114_demo_isp_config_table.h"
	},
	/************************
	* for mt9d115_demo  *
	************************/
	{
		#include "mt9d115_demo_isp_config_table.h"
	},
	/************************
	* for ov5650_p48a  *
	************************/
	{
		#include "ov5650_p48a_isp_config_table.h"
	},
	/************************
	* for ov5650_mipi  *
	************************/
	{
		#include "ov5650_mipi_isp_config_table.h"
	},
	/************************
	* for gc2035_gsg  *
	************************/
	{
		#include "gc2035_gsg_isp_config_table.h"
	},
	/************************
	* for ov5640_fxn  *
	************************/
	{
		#include "ov5640_fxn_isp_config_table.h"
	},
	/*************************
	* for general mipi sensor *
	**************************/
	{
		#include "general_mipi_sensor_isp_config_table.h"
	},
	/*************************
	* for general dvp sensor *
	**************************/
	{
		#include "general_dvp_sensor_isp_config_table.h"
	},
};

