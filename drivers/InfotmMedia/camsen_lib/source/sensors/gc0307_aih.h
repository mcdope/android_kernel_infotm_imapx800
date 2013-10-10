#define GC0307_I2C_ADDR    (0x42>>1)

static struct gc0307_regval_list{
	IM_UINT8 reg;
	IM_UINT8 value;
	};
	
static struct gc0307_regval_list gc0307_qvga_regs[] = {
	
	//320*240
	
	{0xf0,0x00},
	{0x0e,0x0a},  //row even skip
	{0x43,0xc0},  //more boundary mode opclk output enable
			//{0x44 , 0xe2),  // mtk is e0
			//{0x45,  0x2b),  //col subsample  
	{0x45,0x28},  //col subsample
	{0x4e,0x33},  //  32 opclk gate in subsample  // mtk is 33
	
	{0x01,0xd1},
	{0x02,0x82},
	{0x10,0x00},
	{0xd6,0xce},
	
	{0x28,0x02},  //AEC_exp_level_1bit11to8   // 33.3fps
	{0x29,0x6a},  //AEC_exp_level_1bit7to0
	{0x2a,0x04},  //AEC_exp_level_2bit11to8   // 20fps
	{0x2b,0x06},  //AEC_exp_level_2bit7to0 
	{0x2c,0x06},  //AEC_exp_level_3bit11to8    // 12.5fps
	{0x2d,0x70},  //AEC_exp_level_3bit7to0 
	{0x2e,0x0c},  //AEC_exp_level_4bit11to8
	{0x2f,0xe0},  //AEC_exp_level_4bit7to0 
	
	{0xe1,0x01},  //big_win_y0 
	{0xe3,0x0f},  //432, big_win_y1    , height
	{0xea,0x16},  //small_win_height1 
	{0xeb,0x28},  //small_win_height2
	{0xec,0x39},  //small_win_heigh3 //only for AWB
	{0xae,0x0c},  //black pixel target number
	{0xc3,0x20},  //number limit
	{0x74,0x1e},  //lsc_row_center , 0x3c
	{0x75,0x52},  //lsc_col_center , 0x52
	
	{0xff,0xff},
	{0xff,0xff},
	};
static struct gc0307_regval_list gc0307_init_regs[] = {
	// Initail Sequence Write In.
	//========= close output
	{0x43,0x00},
	{0x44,0xa2},
	
	//========= close some functions
	// open them after configure their parmameters
	{0x40,0x10},
	{0x41,0x00},
	{0x42,0x10},
	{0x47,0x00},  //mode1 
	{0x48,0xc3},  //mode2
	{0x49,0x00},  //dither_mode
	{0x4a,0x00},  //clock_gating_en
	{0x4b,0x00},  //mode_reg3
	{0x4E,0x23},  //sync mode
	{0x4F,0x01},  //AWB, AEC, every N frame
	
	//========= frame timing
	{0x01,0x32},  //normal preview 50hz 24M
	{0x02,0x70},
	{0x10,0x01},
	{0xd6,0x78},
	{0x28,0x02},
	{0x29,0x58},
	{0x2a,0x02},
	{0x2b,0x58},
	{0x2c,0x02},
	{0x2d,0x58},
	{0x2e,0x04},
	{0x2f,0xb0},
	
	
	{0x1c,0x00},  //Vs_st
	{0x1D,0x00},  //Vs_et
	{0x10,0x00},  //high 4 bits of VB, HB
	{0x11,0x05},  //row_tail,  AD_pipe_number
	
	//========= windowing
	{0x05,0x00},  //row_start
	{0x06,0x00},  
	{0x07,0x00},  //col start
	{0x08,0x00},
	{0x09,0x01},  //win height
	{0x0A,0xE8},
	{0x0B,0x02},  //win width, pixel array only 640
	{0x0C,0x80},
	
	//========= analog
	{0x0D,0x22},  //rsh_width
	{0x0E,0x02},  //CISCTL mode2,
	{0x12,0x70},  //7 hrst, 6_4 darsg,
	{0x13,0x00},  //7 CISCTL_restart, 0 apwd
	{0x14,0x00},  //NA
	{0x15,0xba},  //7_4 vref
	{0x16,0x13},  //5to4 _coln_r,  __1to0__da18
	{0x17,0x52},  //opa_r, ref_r, sRef_r
	//cmos_sensor(0x18, 0xc0), //analog_mode, best case for left band.
	
	{0x1E,0x0d},  //tsp_width
	{0x1F,0x32},	//sh_delay
	
	//========= offset
	{0x47,0x00},  //7__test_image, __6__fixed_pga, __5__auto_DN, __4__CbCr_fix, 
				//__3to2__dark_sequence, __1__allow_pclk_vcync, __0__LSC_test_image
	
	
	{0x19,0x06},  //pga_o
	{0x1a,0x06},  //pga_e
	
	
	{0x31,0x00},  //pga_oFFset ,	 high 8bits of 11bits
	{0x3B,0x00},  //global_oFFset, low 8bits of 11bits
	
	{0x59,0x0f},  //offset_mode
	{0x58,0x88}, //DARK_VALUE_RATIO_G,  DARK_VALUE_RATIO_RB
	{0x57,0x08}, //DARK_CURRENT_RATE 
	{0x56,0x77},  //PGA_OFFSET_EVEN_RATIO, PGA_OFFSET_ODD_RATIO
	
	//========= blk
	{0x35,0xd8},  //blk_mode
	{0x36,0x40},
	{0x3C,0x00},
	{0x3D,0x00},
	{0x3E,0x00},
	{0x3F,0x00},
	{0xb5,0x70},
	{0xb6,0x40},
	{0xb7,0x00},
	{0xb8,0x38},
	{0xb9,0xc3},
	{0xba,0x0f},
	
	
	{0x7e,0x35},
	{0x7f,0x86},
	{0x5c,0x68},  //78
	{0x5d,0x78},  //88
	
	//========= manual_gain
	{0x61,0x80},  //manual_gain_g1
	{0x63,0x80},  //manual_gain_r
	{0x65,0x98},  //manual_gai_b, 0xa0=1.25, 0x98=1.1875
	{0x67,0x80},  //manual_gain_g2
	{0x68,0x18},  //global_manual_gain	 2.4bits
	
	//===============CC  _R
	{0x69,0x58},  //54
	{0x6A,0xf6},  //ff
	{0x6B,0xfb},  //fe
	{0x6C,0xf4},  //ff
	{0x6D,0x5a},  //5f
	{0x6E,0xe6},  //e1
	{0x6f,0x00},
	
	
	//=========lsc
	{0x70,0x14},
	{0x71,0x1c},
	{0x72,0x20},
	{0x73,0x10},
	{0x74,0x3c},
	{0x75,0x52},
	
	//=========dn
	{0x7d,0x2f},  //dn_mode
	{0x80,0x0c},  //when auto_dn, check 7e,7f
	{0x81,0x0c},
	{0x82,0x44},
	
	//dd
	{0x83,0x18},  //DD_TH1
	{0x84,0x18},  //DD_TH2
	{0x85,0x04},  //DD_TH3 
	{0x87,0x34},  //32 b DNDD_low_range X16,  DNDD_low_range_C_weight_center	
	
	//=========intp-ee
	{0x88,0x04},
	{0x89,0x01},
	{0x8a,0x50},  //60
	{0x8b,0x50},  //60
	{0x8c,0x07},
	
	
	{0x50,0x0c},
	{0x5f,0x3c},
	
	
	{0x8e,0x02},
	{0x86,0x02},
	{0x51,0x20},
	{0x52,0x08},
	{0x53,0x00},
	
	//========= YCP 
	//contrast_center	
	{0x77,0x80},  //contrast_center
	{0x78,0x00},  //fixed_Cb
	{0x79,0x00},  //fixed_Cr
	{0x7a,0x00},  //luma_offset
	{0x7b,0x40},  //hue_cos
	{0x7c,0x00},  //hue_sin
	
	//saturation
	{0xa0,0x40},  //global_saturation
	{0xa1,0x40},  //luma_contrast
	{0xa2,0x34},  //saturation_Cb
	{0xa3,0x34},  //saturation_Cr
	
	{0xa4,0xc8},
	{0xa5,0x02},
	{0xa6,0x28},
	{0xa7,0x02},
	
	//skin
	{0xa8,0xee},
	{0xa9,0x12},
	{0xaa,0x01},
	{0xab,0x20},
	{0xac,0xf0},
	{0xad,0x10},
	
	//========= ABS
	{0xae,0x18},
	{0xaf,0x74},
	{0xb0,0xe0},
	{0xb1,0x20},
	{0xb2,0x6c},
	{0xb3,0x40},
	{0xb4,0x04},
	
	//========= AWB
	{0xbb,0x42},
	{0xbc,0x60},
	{0xbd,0x50},
	{0xbe,0x50},
	
	
	{0xbf,0x0c},
	{0xc0,0x06},
	{0xc1,0x06},
	{0xc2,0xf1},  //f1
	{0xc3,0x40},
	{0xc4,0x1c},  //18//20
	{0xc5,0x56},  //33
	{0xc6,0x1d},
	{0xca,0x70},
	{0xcb,0x70},
	{0xcc,0x78},
	{0xcd,0x80},  //R_ratio
	{0xce,0x80},  //G_ratio,  cold_white white
	{0xcf,0x80},  //B_ratio
	
	//=========  aecT
	{0x20,0x06},  //0x02
	{0x21,0xc0},
	{0x22,0x60},
	{0x23,0x88},
	{0x24,0x96},
	{0x25,0x30},
	{0x26,0xd0},
	{0x27,0x00},
	{0x28,0x01},  //AEC_exp_level_1bit11to8
	{0x29,0xf4},  //AEC_exp_level_1bit7to0
	{0x2a,0x02},  //AEC_exp_level_2bit11to8
	{0x2b,0xbc},  //AEC_exp_level_2bit7to0
	{0x2c,0x03},  //AEC_exp_level_3bit11to8   659 - 8FPS,  8ca - 6FPS  //
	{0x2d,0xe8},  //AEC_exp_level_3bit7to0
	{0x2e,0x09},  //AEC_exp_level_4bit11to8   4FPS
	{0x2f,0xc4},  //AEC_exp_level_4bit7to0	
	{0x30,0x20},
	{0x31,0x00},
	{0x32,0x1c},
	{0x33,0x90},
	{0x34,0x10},
	
	
	{0xd0,0x34},
	{0xd1,0x50},  //AEC_target_Y
	{0xd2,0x61},  //0xf2 
	{0xd4,0x96},
	{0xd5,0x01},  // william 0318
	{0xd6,0x96},  //antiflicker_step
	{0xd7,0x03},  //AEC_exp_time_min ,william 20090312
	{0xd8,0x02},
	{0xdd,0x22},  //0x12 
	
	//========= measure window
	{0xe0,0x03},
	{0xe1,0x02},
	{0xe2,0x27},
	{0xe3,0x1e},
	{0xe8,0x3b},
	{0xe9,0x6e},
	{0xea,0x2c},
	{0xeb,0x50},
	{0xec,0x73},
	
	//========= close_frame
	{0xed,0x00},  //close_frame_num1 ,can be use to reduce FPS
	{0xee,0x00},  //close_frame_num2
	{0xef,0x00},  //close_frame_num
	
	// page1
	{0xf0,0x01},
	{0x00,0x20},
	{0x01,0x20},
	{0x02,0x20},
	{0x03,0x20},
	{0x04,0x78},
	{0x05,0x78},
	{0x06,0x78},
	{0x07,0x78},
	
	{0x10,0x04},
	{0x11,0x04},
	{0x12,0x04},
	{0x13,0x04},
	{0x14,0x01},
	{0x15,0x01},
	{0x16,0x01},
	{0x17,0x01},
	
	{0x20,0x00},
	{0x21,0x00},
	{0x22,0x00},
	{0x23,0x00},
	{0x24,0x00},
	{0x25,0x00},
	{0x26,0x00},
	{0x27,0x00},
	{0x40,0x01},
	
	//=============================lscP
	{0x45,0x06},
	{0x46,0x06},
	{0x47,0x05},
	{0x48,0x04},
	{0x49,0x03},
	{0x4a,0x03},
	
	{0x62,0xd8},
	{0x63,0x24},
	{0x64,0x24},
	{0x65,0x24},
	{0x66,0xd8},
	{0x67,0x24},
	
	{0x5a,0x00},
	{0x5b,0x00},
	{0x5c,0x00},
	{0x5d,0x00},
	{0x5e,0x00},
	{0x5f,0x00},
	
	//============================= ccP
	{0x69,0x03},  //cc_mode
	
	//CC_G
	{0x70,0x5d},
	{0x71,0xed},
	{0x72,0xff},
	{0x73,0xe5},
	{0x74,0x5f},
	{0x75,0xe6},
	
	//CC_B
	{0x76,0x41},
	{0x77,0xef},
	{0x78,0xff},
	{0x79,0xff},
	{0x7a,0x5f},
	{0x7b,0xfa},
	
	//============================= AGP
	{0x7e,0x00},
	{0x7f,0x00},
	{0x80,0xc8},
	{0x81,0x06},
	{0x82,0x08},
	
	{0x83,0x23},
	{0x84,0x38},
	{0x85,0x4F},
	{0x86,0x61},
	{0x87,0x72},
	{0x88,0x80},
	{0x89,0x8D},
	{0x8a,0xA2},
	{0x8b,0xB2},
	{0x8c,0xC0},
	{0x8d,0xCA},
	{0x8e,0xD3},
	{0x8f,0xDB},
	{0x90,0xE2},
	{0x91,0xED},
	{0x92,0xF6},
	{0x93,0xFD},
	
	//about gamma1 is hex r oct
	{0x94,0x04},
	{0x95,0x0E},
	{0x96,0x1B},
	{0x97,0x28},
	{0x98,0x35},
	{0x99,0x41},
	{0x9a,0x4E},
	{0x9b,0x67},
	{0x9c,0x7E},
	{0x9d,0x94},
	{0x9e,0xA7},
	{0x9f,0xBA},
	{0xa0,0xC8},
	{0xa1,0xD4},
	{0xa2,0xE7},
	{0xa3,0xF4},
	{0xa4,0xFA},
	
	//========= open functions
	{0xf0,0x00},  //set back to page0
	{0x40,0x7e},
	{0x41,0x2f},
	
	//=========open output
	{0x43,0x40},
	{0x44,0xE2},
	
	{0x0f,0x82},  //b2
	{0x45,0x24},  //27
	{0x47,0x20},  //2c
	
	{0xff,0xff},
	{0xff,0xff},
	};
	
static struct gc0307_regval_list gc0307_default_regs[] = {
	{0xff,0xff},
	{0xff,0xff},
	};
//============================================
//================SENSOR_SET_ANTIBANDING===
static struct gc0307_regval_list gc0307_night_60hz_regs[]  = {
	/*60hz*/
	{0x01,0x32},
	{0x02,0x70},
	{0x10,0x01},
	{0xd6,0x64},
	{0x28,0x02},
	{0x29,0x58},
	{0x2a,0x02},
	{0x2b,0x58},
	{0x2c,0x02},
	{0x2d,0x58},
	{0x2e,0x03},
	{0x2f,0xe8},
	{0xff,0xff},
	{0xff,0xff},
	};
	
static struct gc0307_regval_list gc0307_night_50hz_regs[] = {
	/*50hz*/
	{0x01,0x32},  //normal preview 50hz 24M
	{0x02,0x70},
	{0x10,0x01},
	{0xd6,0x78},
	{0x28,0x02},
	{0x29,0x58},
	{0x2a,0x02},
	{0x2b,0x58},
	{0x2c,0x02},
	{0x2d,0x58},
	{0x2e,0x04},
	{0x2f,0xb0},
	{0xff,0xff},
	{0xff,0xff},
	};
//==================set effect==================
	
static struct gc0307_regval_list gc0307_greenish_regs[] = {
	{0x41,0x2f},  //SEPIAGREEN green
	{0x40,0x7e},
	{0x42,0x10},
	{0x47,0x30},
	{0x48,0xc3},
	{0x8a,0x60},
	{0x8b,0x60},
	{0x8c,0x07},
	{0x50,0x0c},
	{0x77,0x80},
	{0xa1,0x40},
	{0x7a,0x00},
	{0x78,0xc0},
	{0x79,0xc0},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};
static struct gc0307_regval_list gc0307_blackboard_regs[] = {
	{0x41,0x00},
	{0x40,0x3e},
	{0x42,0x14},
	{0x47,0x30},
	{0x48,0xc2},
	{0x8d,0xff},
	{0x8a,0xf0},
	{0x8b,0xf0},
	{0x8c,0x00},
	{0x50,0x08},
	{0xdb,0x46},
	{0xb0,0x46},
	{0x77,0xa8},
	{0xa1,0xff},
	{0x7a,0x98},
	{0x78,0x00},
	{0x79,0x00},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};
static struct gc0307_regval_list gc0307_whiteboard_regs[] = {
	{0x41,0x00},
	{0x40,0x3e},
	{0x42,0x14},
	{0x47,0x30},
	{0x48,0xc2},
	{0x8d,0xff},
	{0x8a,0xf0},
	{0x8b,0xf0},
	{0x8c,0x00},
	{0x50,0x08},
	{0xdb,0x50},
	{0xb0,0xff},
	{0x77,0xab},
	{0xa1,0xff},
	{0x7a,0xab},
	{0x78,0x00},
	{0x79,0x00},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};	
	
static struct gc0307_regval_list gc0307_sepia_regs[] = {
	{0x41,0x2f},  //SEPIA
	
	{0x40,0x7e},
	{0x42,0x10},
	{0x47,0x30},
	{0x48,0xc3},
	{0x8a,0x60},
	{0x8b,0x60},
	{0x8c,0x07},
	{0x50,0x0c},
	{0x77,0x80},
	{0xa1,0x40},
	{0x7a,0x00},
	{0x78,0xc0},
	{0x79,0x20},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};
static struct gc0307_regval_list gc0307_reddish_regs[] = {
	{0xff,0xff},
	{0xff,0xff},
	};	
	
	
static struct gc0307_regval_list gc0307_yellowish_regs[] = {
	{0xff,0xff},
	{0xff,0xff},
	};
	
static struct gc0307_regval_list gc0307_negative_regs[] = {
	{0x41,0x6f},
	
	{0x40,0x7e},
	{0x42,0x10},
	{0x47,0x30},
	{0x48,0xc3},
	{0x8a,0x60},
	{0x8b,0x60},
	{0x8c,0x07},
	{0x50,0x0c},
	{0x77,0x80},
	{0xa1,0x40},
	{0x7a,0x00},
	{0x78,0x00},
	{0x79,0x00},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};
static struct gc0307_regval_list gc0307_bandw_regs[] = {
	{0x41,0x2f},  //danse w&b
	
	{0x40,0x7e},
	{0x42,0x10},
	{0x47,0x30},
	{0x48,0xc3},
	{0x8a,0x60},
	{0x8b,0x60},
	{0x8c,0x07},
	{0x50,0x0c},
	{0x77,0x80},
	{0xa1,0x40},
	{0x7a,0x00},
	{0x78,0x00},
	{0x79,0x00},
	{0x7b,0x40},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};
	
static struct gc0307_regval_list gc0307_normal_regs[] = {
	{0x41,0x2f},  //normal
	
	{0x40,0x7e},
	{0x42,0x10},
	
	{0x47,0x20},  //change
	{0x48,0xc3},
	{0x8a,0x50},  //60
	{0x8b,0x50},
	{0x8c,0x07},
	{0x50,0x0c},
	{0x77,0x80},
	{0xa1,0x40},
	{0x7a,0x00},
	{0x78,0x00},
	{0x79,0x00},
	{0x7b,0x00},
	{0x7c,0x00},
	{0xff,0xff},
	{0xff,0xff},
	};

//==================set wb=====================	
static struct gc0307_regval_list gc0307_auto_regs[] = {
	{0xc7,0x4c},  //for AWB can adjust back 
	{0xc8,0x40},
	{0xc9,0x4a},
	{0x41,0x2f},  //Enable AWB
	{0xff,0xff},
	{0xff,0xff},
	
	};

static struct gc0307_regval_list gc0307_home_regs[] = {
	{0x41,0x2b},  //disable awb
	{0xc7,0x48},
	{0xc8,0x40},
	{0xc9,0x5c},
	{0xff,0xff},
	{0xff,0xff},
	};	

static struct gc0307_regval_list gc0307_office_regs[] = {
	{0x41,0x2b},
	{0xc7,0x40},
	{0xc8,0x42},
	{0xc9,0x50},
	{0xff,0xff},
	{0xff,0xff},
	};

static struct gc0307_regval_list gc0307_sunny_regs[] = {
	{0x41,0x2b},
	{0xc7,0x50},
	{0xc8,0x45},
	{0xc9,0x40},
	{0xff,0xff},
	{0xff,0xff},
	};
	
static struct gc0307_regval_list gc0307_cloudy_regs[] = {
	{0x41,0x2b},
	{0xc7,0x5a},
	{0xc8,0x42},
	{0xc9,0x40},
	{0xff,0xff},
	{0xff,0xff},
	};
//====================================================	
	