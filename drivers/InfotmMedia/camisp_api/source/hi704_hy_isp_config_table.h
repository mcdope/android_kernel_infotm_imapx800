//===============
//special effect
//===============

//special effect 0: CAM_SPECIAL_EFFECT_NONE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_NORMAL,			//type
		//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
		{
			296,						//coefCr
			196							//coefCb
		},
		//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
		{
			ISP_IEF_COLOR_SELECT_MODE_R,//mode
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 1: CAM_SPECIAL_EFFECT_MONO
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_GRAY,				//type
		//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
		{
			296,						//coefCr
			196							//coefCb
		},
		//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
		{
			ISP_IEF_COLOR_SELECT_MODE_R,//mode
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 2: CAM_SPECIAL_EFFECT_NEGATIVE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_NEGATIVE,				//type
		//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
		{
			296,						//coefCr
			196							//coefCb
		},
		//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
		{
			ISP_IEF_COLOR_SELECT_MODE_R,//mode
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 3: CAM_SPECIAL_EFFECT_SOLARIZE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_SOLARIZATION,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 4: CAM_SPECIAL_EFFECT_PASTEL
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_COLOR_SELECT,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 5: CAM_SPECIAL_EFFECT_MOSAIC
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_COLOR_SELECT,			//type
		//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
		{
			296,						//coefCr
			196							//coefCb
		},
		//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
		{
			ISP_IEF_COLOR_SELECT_MODE_G,	//mode
			64,							//tha
			84							//thb

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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 6: CAM_SPECIAL_EFFECT_RESIZE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_COLOR_SELECT,			//type
		//rgcfMat						/*for ISP_IE_TYPE_SEPIA*/
		{
			296,						//coefCr
			196							//coefCb
		},
		//selMat						/*for ISP_IE_TYPE_COLOR_SELECT*/
		{
			ISP_IEF_COLOR_SELECT_MODE_B,	//mode
			72,							//tha
			52							//thb

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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 7: CAM_SPECIAL_EFFECT_SEPIA
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_SEPIA,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 8: CAM_SPECIAL_EFFECT_POSTERIZE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_EMBOSS,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 9: CAM_SPECIAL_EFFECT_WHITEBOARD
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_SKETCH_SOBEL,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 10: CAM_SPECIAL_EFFECT_BLACKBOARD
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_TRUE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_SKETCH_LP,			//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 11: CAM_SPECIAL_EFFECT_AQUA
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_FALSE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_FALSE,		//accNeedConfig;	
	IM_TRUE,		//iefNeedConfig;	
	IM_FALSE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		0
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		0
	},
	//ispsen_ief_config_t
	{
		IM_FALSE,						//enable
		IM_TRUE,						//cscEn
		ISP_IEF_TYPE_NORMAL,		//type
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
		0
	},
	//ispsen_osd_config_t
	{
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//special effect 12: reserved
{
	0
},
//special effect 13: reserved
{
	0
},
//special effect 14: reserved
{
	0
},
//special effect 15: reserved
{
	0
},
//special effect 16: reserved
{
	0
},
//special effect 17: reserved
{
	0
},
//special effect 18: reserved
{
	0
},
//special effect 19: reserved
{
	0
},

//=============
//scene mode
//=============
//scene mode 0:CAM_SCENE_MODE_AUTO
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_FALSE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_FALSE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_FALSE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 1:CAM_SCENE_MODE_ACTION
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_FALSE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_FALSE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_FALSE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 2:CAM_SCENE_MODE_PORTRAIT
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 3:CAM_SCENE_MODE_LANDSCAPE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1638,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1638,							//ths
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
			2130,						//coefg
			2130,						//coefb
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 4:CAM_SCENE_MODE_NIGHT
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1638,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
			2130,						//coefg
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 5:CAM_SCENE_MODE_NIGHT_PORTRAIT
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 6:CAM_SCENE_MODE_THEATRE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 7:CAM_SCENE_MODE_BEACH
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 8:CAM_SCENE_MODE_SNOW
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 9:CAM_SCENE_MODE_SUNSET
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1638,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1638,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2355,						//coefr	
			2300,						//coefg
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 10:CAM_SCENE_MODE_STEADYPHOTO
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 11:CAM_SCENE_MODE_FIREWORKS
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 12:CAM_SCENE_MODE_SPORTS
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 13:CAM_SCENE_MODE_PARTY
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 14:CAM_SCENE_MODE_CANDLELIGHT
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 15:CAM_SCENE_MODE_BARCODE
{
	IM_FALSE,		//demNeedConfig;	
	IM_FALSE,		//bccbNeedConfig;	
	IM_FALSE,		//bdcNeedConfig;	
	IM_FALSE,		//lensNeedConfig;	
	IM_FALSE,		//gmaNeedConfig;	
	IM_TRUE,		//eeNeedConfig;	
	IM_FALSE,		//fccNeedConfig;	
	IM_FALSE,		//afNeedConfig;	
	IM_FALSE,		//awbNeedConfig;	
	IM_FALSE,		//cmncscNeedConfig;	
	IM_FALSE,		//aeNeedConfig;	
	IM_FALSE,		//histNeedConfig;	
	IM_TRUE,		//accNeedConfig;	
	IM_FALSE,		//iefNeedConfig;	
	IM_TRUE,		//acmNeedConfig;	
	IM_FALSE,		//osdNeedConfig;	
	IM_FALSE,		//cropNeedConfig;	
	IM_FALSE,		//sclNeedConfig;	
	//demEnable
	IM_FALSE,
	//ispsen_bccb_config_t
	{
		0
	},
	//ispsen_bdc_config_t
	{
		0
	},
	//ispsen_lens_config_t
	{
		0
	},
	//ispsen_gma_config_t
	{
		0
	},
	//ispsen_ee_config_t
	{
		IM_TRUE,						//enable
		0x33,							//coefw
		0x40,							//coefa
		ISP_ROUND_MINUS,				//rdMode
		IM_TRUE,						//gasEn
		ISP_EE_GAUSS_MODE_0,			//gasMode
		0xfff,							//errTh
		//thrMat
		{
			255,						//hTh
			255,						//vTh
			255,						//d0Th
			255,						//d1Th
		}
	},
	//ispsen_fcc_config_t
	{
		0
	},
	//ispsen_af_config_t
	{
		0
	},
	//ispsen_awb_config_t
	{
		0
	},
	//ispsen_cmncsc_config_t
	{
		0
	},
	//ispsen_ae_config_t
	{
		0
	},
	//ispsen_hist_config_t
	{
		0
	},
	//ispsen_acc_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		1229,							//coefe
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
		0
	},
	//ispsen_acm_config_t
	{
		IM_TRUE,						//enable
		ISP_ROUND_MINUS,				//rdMode
		-819,							//ths
		//thrMat
		{
			200,						//tha
			150,						//thb
			100							//thc
		},
		//coMat
		{
			2048,						//coefp
			2130,						//coefr	
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
		0
	},
	//ispsen_crop_config_t
	{
		0
	},
	//ispsen_scl_config_t
	{
		0
	},
},
//scene mode 16: reserved
{
	0
},
//scene mode 17: reserved
{
	0
},
//scene mode 18: reserved
{
	0
},
//scene mode 19: reserved
{
	0
},
