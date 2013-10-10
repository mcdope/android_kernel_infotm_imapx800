
 #define HY252_I2C_WADDR    0x20
 #define HY252_I2C_RADDR    0x20

struct hy252_regval_list {
	unsigned char  reg;
	unsigned char  value;
};
uint8_t HY252_04  = 0x04;

struct hy252_regval_list  hy252_init_regs[] = {
{0x01, 0xf9}, //sleep on
{0x08, 0x0f}, //Hi-Z on
{0x01, 0xf8}, //sleep off
{0x03, 0x00}, // Dummy 750us START
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00}, // Dummy 750us END
{0x0e, 0x03}, //PLL On
{0x0e, 0x74}, //PLLx2
{0x03, 0x00}, // Dummy 750us START
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00}, // Dummy 750us END

{0x0e, 0x00}, //PLL off
{0x01, 0xf1}, //sleep on
{0x08, 0x00}, //Hi-Z off

{0x01, 0xf3},
{0x01, 0xf1},

// PAGE 20
{0x03, 0x20}, //page 20
{0x10, 0x1c}, //ae off

// PAGE 22
{0x03, 0x22}, //page 22
{0x10, 0x69}, //awb off


//Initial Start
/////// PAGE 0 START ///////
{0x03, 0x00},
{0x10, 0x11}, // Sub1/2_Preview2 Mode_H binning
#if defined(CONFIG_IG_CAMIF0_MIRR) && defined(CONFIG_IG_CAMIF0_FLIP)
{0x11, 0x93},
#elif defined(CONFIG_IG_CAMIF0_FLIP)
{0x11, 0x92},
#elif defined(CONFIG_IG_CAMIF0_MIRR)
{0x11, 0x91},
#else
{0x11, 0x90},
#endif
{0x12, 0x00}, //00

{0x0b, 0xaa}, // ESD Check Register
{0x0c, 0xaa}, // ESD Check Register
{0x0d, 0xaa}, // ESD Check Register

{0x20, 0x00}, // Windowing start point Y
{0x21, 0x04},
{0x22, 0x00}, // Windowing start point X
{0x23, 0x07},

{0x24, 0x04},
{0x25, 0xb0},
{0x26, 0x06},
{0x27, 0x40}, // WINROW END

{0x40, 0x01}, //Hblank 408 428
{0x41, 0x68}, 
// 98
{0x42, 0x00}, //Vblank 20
{0x43, 0x14},

{0x45, 0x04},
{0x46, 0x18},
{0x47, 0xd8},

//BLC
{0x80, 0x2e},
{0x81, 0x7e},
{0x82, 0x90},
{0x83, 0x00},
{0x84, 0x0c},
{0x85, 0x00},
{0x90, 0x0a}, //BLC_TIME_TH_ON
{0x91, 0x0a}, //BLC_TIME_TH_OFF 
{0x92, 0xd8}, //BLC_AG_TH_ON
{0x93, 0xd0}, //BLC_AG_TH_OFF
{0x94, 0x75},
{0x95, 0x70},
{0x96, 0xdc},
{0x97, 0xfe},
{0x98, 0x38},

//OutDoor  BLC
{0x99, 0x43},
{0x9a, 0x43},
{0x9b, 0x43},
{0x9c, 0x43},

//Dark BLC
{0xa0, 0x43},// 00
{0xa2, 0x43},
{0xa4, 0x43},
{0xa6, 0x43},

//Normal BLC
{0xa8, 0x43},
{0xaa, 0x43},
{0xac, 0x43},
{0xae, 0x43},

{0x03, 0x02}, //Page 02
{0x10, 0x00}, //Mode_test
{0x11, 0x00}, //Mode_dead_test
{0x12, 0x03}, //pwr_ctl_ctl1
{0x13, 0x03}, //Mode_ana_test
{0x14, 0x00}, //mode_memory
{0x16, 0x00}, //dcdc_ctl1
{0x17, 0x8c}, //dcdc_ctl2
{0x18, 0x4C}, //analog_func1
{0x19, 0x00}, //analog_func2
{0x1a, 0x39}, //analog_func3
{0x1b, 0x00}, //analog_func4
{0x1c, 0x09}, //dcdc_ctl3
{0x1d, 0x40}, //dcdc_ctl4
{0x1e, 0x30}, //analog_func7
{0x1f, 0x10}, //analog_func8
{0x20, 0x77}, //pixel bias
{0x21, 0xde}, //adc},asp bias
{0x22, 0xa7}, //main},bus bias
{0x23, 0x30}, //clamp
{0x24, 0x4a},		
{0x25, 0x10},		
{0x27, 0x3c},		
{0x28, 0x00},		
{0x29, 0x0c},		
{0x2a, 0x80},		
{0x2b, 0x80},		
{0x2c, 0x02},		
{0x2d, 0xa0},		
{0x2e, 0x11},		
{0x2f, 0xa1},		
{0x30, 0x05}, //swap_ctl
{0x31, 0x99},		
{0x32, 0x00},		
{0x33, 0x00},		
{0x34, 0x22},		
{0x38, 0x88},		
{0x39, 0x88},		
{0x50, 0x20},		
{0x51, 0x00},		
{0x52, 0x01},		
{0x53, 0xc1},		
{0x54, 0x10},		
{0x55, 0x1c},		
{0x56, 0x11},		
{0x58, 0x10},		
{0x59, 0x0e},		
{0x5d, 0xa2},		
{0x5e, 0x5a},		
{0x60, 0x87},		
{0x61, 0x99},		
{0x62, 0x88},		
{0x63, 0x97},		
{0x64, 0x88},		
{0x65, 0x97},		
{0x67, 0x0c},		
{0x68, 0x0c},		
{0x69, 0x0c},		
{0x6a, 0xb4},		
{0x6b, 0xc4},		
{0x6c, 0xb5},		
{0x6d, 0xc2},		
{0x6e, 0xb5},		
{0x6f, 0xc0},		
{0x70, 0xb6},		
{0x71, 0xb8},		
{0x72, 0x89},		
{0x73, 0x96},		
{0x74, 0x89},		
{0x75, 0x96},		
{0x76, 0x89},		
{0x77, 0x96},		
{0x7c, 0x85},		
{0x7d, 0xaf},		
{0x80, 0x01},		
{0x81, 0x7f},		
{0x82, 0x13}, //rx_on1_read
{0x83, 0x24},		
{0x84, 0x7D},		
{0x85, 0x81},		
{0x86, 0x7D},		
{0x87, 0x81},		
{0x88, 0xab},		
{0x89, 0xbc},		
{0x8a, 0xac},		
{0x8b, 0xba},		
{0x8c, 0xad},		
{0x8d, 0xb8},		
{0x8e, 0xae},		
{0x8f, 0xb2},		
{0x90, 0xb3},		
{0x91, 0xb7},		
{0x92, 0x48},		
{0x93, 0x54},		
{0x94, 0x7D},		
{0x95, 0x81},		
{0x96, 0x7D},		
{0x97, 0x81},		
{0xa0, 0x02},		
{0xa1, 0x7B},		
{0xa2, 0x02},		
{0xa3, 0x7B},		
{0xa4, 0x7B},		
{0xa5, 0x02},		
{0xa6, 0x7B},		
{0xa7, 0x02},		
{0xa8, 0x85},		
{0xa9, 0x8C},		
{0xaa, 0x85},		
{0xab, 0x8C},		
{0xac, 0x10}, //Rx_pwr_off1_read
{0xad, 0x16}, //Rx_pwr_on1_read
{0xae, 0x10}, //Rx_pwr_off2_read
{0xaf, 0x16}, //Rx_pwr_on1_read
{0xb0, 0x99},		
{0xb1, 0xA3},		
{0xb2, 0xA4},		
{0xb3, 0xAE},		
{0xb4, 0x9B},		
{0xb5, 0xA2},		
{0xb6, 0xA6},		
{0xb7, 0xAC},		
{0xb8, 0x9B},		
{0xb9, 0x9F},		
{0xba, 0xA6},		
{0xbb, 0xAA},		
{0xbc, 0x9B},		
{0xbd, 0x9F},		
{0xbe, 0xA6},		
{0xbf, 0xaa},		
{0xc4, 0x2c},		
{0xc5, 0x43},		
{0xc6, 0x63},		
{0xc7, 0x79},		
{0xc8, 0x2d},		
{0xc9, 0x42},		
{0xca, 0x2d},		
{0xcb, 0x42},		
{0xcc, 0x64},		
{0xcd, 0x78},		
{0xce, 0x64},		
{0xcf, 0x78},		
{0xd0, 0x0a},		
{0xd1, 0x09},		
{0xd2, 0x20},		
{0xd3, 0x00},	
	
{0xd4, 0x0a},		
{0xd5, 0x0a},		
{0xd6, 0xb8},		
{0xd7, 0xb0},
		
{0xe0, 0xc4},		
{0xe1, 0xc4},		
{0xe2, 0xc4},		
{0xe3, 0xc4},		
{0xe4, 0x00},		
{0xe8, 0x80},		
{0xe9, 0x40},		
{0xea, 0x7f},		
{0xf0, 0x01}, //sram1_cfg
{0xf1, 0x01}, //sram2_cfg
{0xf2, 0x01}, //sram3_cfg
{0xf3, 0x01}, //sram4_cfg
{0xf4, 0x01}, //sram5_cfg

/////// PAGE 3 ///////
{0x03, 0x03},
{0x10, 0x10},

/////// PAGE 10 START ///////
{0x03, 0x10},
{0x10, 0x03}, //03
{0x12, 0x30},
{0x20, 0x00},

{0x30, 0x00},
{0x31, 0x00},
{0x32, 0x00},
{0x33, 0x00},

{0x34, 0x30},
{0x35, 0x00},
{0x36, 0x00},
{0x38, 0x00},
{0x3e, 0x58},
{0x3f, 0x00},
{0x48, 0x94},
{0X4A, 0X84},
{0x40, 0x10}, // YOFS 80
{0x41, 0x00}, // DYOFS

{0x60, 0x67},
{0x61, 0x7c}, //7e //8e //88 //80
{0x62, 0x7c}, //7e //8e //88 //80
{0x63, 0x50}, //Double_AG 50->30
{0x64, 0x41},

{0x66, 0x42},
{0x67, 0x20},

{0x6a, 0x80}, //8a
{0x6b, 0x84}, //74
{0x6c, 0x80}, //7e //7a
{0x6d, 0x80}, //8e

//Don't touch//////////////////////////
//{0x72, 0x84},
//{0x76, 0x19},
//{0x73, 0x70},
//{0x74, 0x68},
//{0x75, 0x60}, // white protection ON
//{0x77, 0x0e}, //08 //0a
//{0x78, 0x2a}, //20
//{0x79, 0x08},
////////////////////////////////////////

/////// PAGE 11 START ///////
{0x03, 0x11},
{0x10, 0x7f},
{0x11, 0x40},
{0x12, 0x0a}, // Blue Max-Filter Delete
{0x13, 0xbb},

{0x26, 0x31}, // Double_AG 31->20
{0x27, 0x34}, // Double_AG 34->22
{0x28, 0x0f},
{0x29, 0x10},
{0x2b, 0x30},
{0x2c, 0x32},

//Out2 D-LPF th
{0x30, 0x70},
{0x31, 0x10},
{0x32, 0x58},
{0x33, 0x09},
{0x34, 0x06},
{0x35, 0x03},

//Out1 D-LPF th
{0x36, 0x70},
{0x37, 0x18},
{0x38, 0x58},
{0x39, 0x09},
{0x3a, 0x06},
{0x3b, 0x03},

//Indoor D-LPF th
{0x3c, 0x80},
{0x3d, 0x18},
{0x3e, 0xa0}, //80
{0x3f, 0x0c},
{0x40, 0x09},
{0x41, 0x06},

{0x42, 0x80},
{0x43, 0x18},
{0x44, 0xa0}, //80
{0x45, 0x12},
{0x46, 0x10},
{0x47, 0x10},

{0x48, 0x90},
{0x49, 0x40},
{0x4a, 0x80},
{0x4b, 0x13},
{0x4c, 0x10},
{0x4d, 0x11},

{0x4e, 0x80},
{0x4f, 0x30},
{0x50, 0x80},
{0x51, 0x13},
{0x52, 0x10},
{0x53, 0x13},

{0x54, 0x11},
{0x55, 0x17},
{0x56, 0x20},
{0x57, 0x01},
{0x58, 0x00},
{0x59, 0x00},

{0x5a, 0x1f}, //18
{0x5b, 0x00},
{0x5c, 0x00},

{0x60, 0x3f},
{0x62, 0x60},
{0x70, 0x06},

/////// PAGE 12 START ///////
{0x03, 0x12},
{0x20, 0x0f},
{0x21, 0x0f},

{0x25, 0x00}, //{0x30

{0x28, 0x00},
{0x29, 0x00},
{0x2a, 0x00},

{0x30, 0x50},
{0x31, 0x18},
{0x32, 0x32},
{0x33, 0x40},
{0x34, 0x50},
{0x35, 0x70},
{0x36, 0xa0},

//Out2 th
{0x40, 0xa0},
{0x41, 0x40},
{0x42, 0xa0},
{0x43, 0x90},
{0x44, 0x90},
{0x45, 0x80},

//Out1 th
{0x46, 0xb0},
{0x47, 0x55},
{0x48, 0xa0},
{0x49, 0x90},
{0x4a, 0x90},
{0x4b, 0x80},

//Indoor th
{0x4c, 0xb0},
{0x4d, 0x40},
{0x4e, 0x90},
{0x4f, 0x90},
{0x50, 0xa0},
{0x51, 0x80},

//Dark1 th
{0x52, 0xb0},
{0x53, 0x60},
{0x54, 0xc0},
{0x55, 0xc0},
{0x56, 0xc0},
{0x57, 0x80},

//Dark2 th
{0x58, 0x90},
{0x59, 0x40},
{0x5a, 0xd0},
{0x5b, 0xd0},
{0x5c, 0xe0},
{0x5d, 0x80},

//Dark3 th
{0x5e, 0x88},
{0x5f, 0x40},
{0x60, 0xe0},
{0x61, 0xe0},
{0x62, 0xe0},
{0x63, 0x80},

{0x70, 0x15},
{0x71, 0x01}, //Don't Touch register

{0x72, 0x18},
{0x73, 0x01}, //Don't Touch register

{0x74, 0x25},
{0x75, 0x15},

{0x80, 0x30},
{0x81, 0x50},
{0x82, 0x80},
{0x85, 0x1a},
{0x88, 0x00},
{0x89, 0x00},
{0x90, 0x5d}, //For Preview

{0xc5, 0x30},
{0xc6, 0x2a},

//Dont Touch register
{0xD0, 0x0c},
{0xD1, 0x80},
{0xD2, 0x67},
{0xD3, 0x00},
{0xD4, 0x00},
{0xD5, 0x02},
{0xD6, 0xff},
{0xD7, 0x18},
//End
{0x3b, 0x06},
{0x3c, 0x06},

{0xc5, 0x30},//55->48
{0xc6, 0x2a},//48->40

/////// PAGE 13 START ///////
{0x03, 0x13},
//Edge
{0x10, 0xcb},
{0x11, 0x7b},
{0x12, 0x07},// 
{0x14, 0x00},

{0x20, 0x15},
{0x21, 0x13},
{0x22, 0x33},
{0x23, 0x05},
{0x24, 0x09},

{0x25, 0x0a},

{0x26, 0x18},
{0x27, 0x30},
{0x29, 0x12},
{0x2a, 0x50},

//Low clip th
{0x2b, 0x01},// 02
{0x2c, 0x01},// 02
{0x25, 0x06},
{0x2d, 0x0c},
{0x2e, 0x12},
{0x2f, 0x12},

//Out2 Edge
{0x50, 0x10},
{0x51, 0x14},
{0x52, 0x12},
{0x53, 0x0c},
{0x54, 0x0f},
{0x55, 0x0c},

//Out1 Edge
{0x56, 0x10},
{0x57, 0x13},
{0x58, 0x12},
{0x59, 0x0c},
{0x5a, 0x0f},
{0x5b, 0x0c},

//Indoor Edge
{0x5c, 0x0a},
{0x5d, 0x0b},
{0x5e, 0x0a},
{0x5f, 0x08},
{0x60, 0x09},
{0x61, 0x08},

//Dark1 Edge
{0x62, 0x08},
{0x63, 0x08},
{0x64, 0x08},
{0x65, 0x06},
{0x66, 0x06},
{0x67, 0x06},

//Dark2 Edge
{0x68, 0x07},
{0x69, 0x07},
{0x6a, 0x07},
{0x6b, 0x05},
{0x6c, 0x05},
{0x6d, 0x05},

//Dark3 Edge
{0x6e, 0x07},
{0x6f, 0x07},
{0x70, 0x07},
{0x71, 0x05},
{0x72, 0x05},
{0x73, 0x05},

//2DY
{0x80, 0xfd},
{0x81, 0x1f},
{0x82, 0x05},
{0x83, 0x31},

{0x90, 0x05},
{0x91, 0x05},
{0x92, 0x33},
{0x93, 0x30},
{0x94, 0x03},
{0x95, 0x14},
{0x97, 0x20},
{0x99, 0x20},

{0xa0, 0x01},
{0xa1, 0x02},
{0xa2, 0x01},
{0xa3, 0x02},
{0xa4, 0x05},
{0xa5, 0x05},
{0xa6, 0x07},
{0xa7, 0x08},
{0xa8, 0x07},
{0xa9, 0x08},
{0xaa, 0x07},
{0xab, 0x08},

//Out2 
{0xb0, 0x22},
{0xb1, 0x2a},
{0xb2, 0x28},
{0xb3, 0x22},
{0xb4, 0x2a},
{0xb5, 0x28},

//Out1 
{0xb6, 0x22},
{0xb7, 0x2a},
{0xb8, 0x28},
{0xb9, 0x22},
{0xba, 0x2a},
{0xbb, 0x28},

//Indoor 
{0xbc, 0x25},
{0xbd, 0x2a},
{0xbe, 0x27},
{0xbf, 0x25},
{0xc0, 0x2a},
{0xc1, 0x27},

//Dark1
{0xc2, 0x1e},
{0xc3, 0x24},
{0xc4, 0x20},
{0xc5, 0x1e},
{0xc6, 0x24},
{0xc7, 0x20},

//Dark2
{0xc8, 0x18},
{0xc9, 0x20},
{0xca, 0x1e},
{0xcb, 0x18},
{0xcc, 0x20},
{0xcd, 0x1e},

//Dark3 
{0xce, 0x18},
{0xcf, 0x20},
{0xd0, 0x1e},
{0xd1, 0x18},
{0xd2, 0x20},
{0xd3, 0x1e},

/////// PAGE 14 START ///////
{0x03, 0x14},
{0x10, 0x11},

{0x14, 0x80}, // GX
{0x15, 0x80}, // GY
{0x16, 0x80}, // RX
{0x17, 0x80}, // RY
{0x18, 0x80}, // BX
{0x19, 0x80}, // BY

{0x20, 0x60}, //X 60 //a0
{0x21, 0x80}, //Y

{0x22, 0x80},
{0x23, 0x80},
{0x24, 0x80},

{0x30, 0xc8},
{0x31, 0x2b},
{0x32, 0x00},
{0x33, 0x00},
{0x34, 0x90},

{0x40, 0x48}, //31
{0x50, 0x34}, //23 //32
{0x60, 0x29}, //1a //27
{0x70, 0x34}, //23 //32

/////// PAGE 15 START ///////
{0x03, 0x15},
{0x10, 0x0f},

//Rstep H 16
//Rstep L 14
{0x14, 0x42}, //CMCOFSGH_Day //4c
{0x15, 0x32}, //CMCOFSGM_CWF //3c
{0x16, 0x24}, //CMCOFSGL_A //2e
{0x17, 0x2f}, //CMC SIGN

//CMC_Default_CWF
{0x30, 0x8f},
{0x31, 0x59},
{0x32, 0x0a},
{0x33, 0x15},
{0x34, 0x5b},
{0x35, 0x06},
{0x36, 0x07},
{0x37, 0x40},
{0x38, 0x87}, //86

//CMC OFS L_A
{0x40, 0x92},
{0x41, 0x1b},
{0x42, 0x89},
{0x43, 0x81},
{0x44, 0x00},
{0x45, 0x01},
{0x46, 0x89},
{0x47, 0x9e},
{0x48, 0x28},

//{0x40, 0x93},
//{0x41, 0x1c},
//{0x42, 0x89},
//{0x43, 0x82},
//{0x44, 0x01},
//{0x45, 0x01},
//{0x46, 0x8a},
//{0x47, 0x9d},
//{0x48, 0x28},

//CMC POFS H_DAY
{0x50, 0x02},
{0x51, 0x82},
{0x52, 0x00},
{0x53, 0x07},
{0x54, 0x11},
{0x55, 0x98},
{0x56, 0x00},
{0x57, 0x0b},
{0x58, 0x8b},

{0x80, 0x03},
{0x85, 0x40},
{0x87, 0x02},
{0x88, 0x00},
{0x89, 0x00},
{0x8a, 0x00},

/////// PAGE 16 START ///////
{0x03, 0x16},
{0x10, 0x31},
{0x18, 0x5e},// Double_AG 5e->37
{0x19, 0x5d},// Double_AG 5e->36
{0x1a, 0x0e},
{0x1b, 0x01},
{0x1c, 0xdc},
{0x1d, 0xfe},

//GMA Default
{0x30, 0x00},
{0x31, 0x0a},
{0x32, 0x1f},
{0x33, 0x33},
{0x34, 0x53},
{0x35, 0x6c},
{0x36, 0x81},
{0x37, 0x94},
{0x38, 0xa4},
{0x39, 0xb3},
{0x3a, 0xc0},
{0x3b, 0xcb},
{0x3c, 0xd5},
{0x3d, 0xde},
{0x3e, 0xe6},
{0x3f, 0xee},
{0x40, 0xf5},
{0x41, 0xfc},
{0x42, 0xff},

{0x50, 0x00},
{0x51, 0x09},
{0x52, 0x1f},
{0x53, 0x37},
{0x54, 0x5b},
{0x55, 0x76},
{0x56, 0x8d},
{0x57, 0xa1},
{0x58, 0xb2},
{0x59, 0xbe},
{0x5a, 0xc9},
{0x5b, 0xd2},
{0x5c, 0xdb},
{0x5d, 0xe3},
{0x5e, 0xeb},
{0x5f, 0xf0},
{0x60, 0xf5},
{0x61, 0xf7},
{0x62, 0xf8},

{0x70, 0x00},
{0x71, 0x08},
{0x72, 0x17},
{0x73, 0x2f},
{0x74, 0x53},
{0x75, 0x6c},
{0x76, 0x81},
{0x77, 0x94},
{0x78, 0xa4},
{0x79, 0xb3},
{0x7a, 0xc0},
{0x7b, 0xcb},
{0x7c, 0xd5},
{0x7d, 0xde},
{0x7e, 0xe6},
{0x7f, 0xee},
{0x80, 0xf4},
{0x81, 0xfa},
{0x82, 0xff},

/////// PAGE 17 START ///////
{0x03, 0x17},
{0x10, 0xf7},

/////// PAGE 20 START ///////
{0x03, 0x20},
{0x11, 0x1c},
{0x18, 0x30},
{0x1a, 0x08},
{0x20, 0x01}, //05_lowtemp Y Mean off
{0x21, 0x30},
{0x22, 0x10},
{0x23, 0x00},
{0x24, 0x00}, //Uniform Scene Off

{0x28, 0xe7},
{0x29, 0x0d}, //20100305 ad->0d
{0x2a, 0xff},
{0x2b, 0x04}, //f4->Adaptive off

{0x2c, 0xc2},
{0x2d, 0xcf},  //fe->AE Speed option
{0x2e, 0x33},
{0x30, 0x78}, //f8
{0x32, 0x03},
{0x33, 0x2e},
{0x34, 0x30},
{0x35, 0xd4},
{0x36, 0xfe},
{0x37, 0x32},
{0x38, 0x04},

{0x39, 0x22}, //AE_escapeC10
{0x3a, 0xde}, //AE_escapeC11

{0x3b, 0x22}, //AE_escapeC1
{0x3c, 0xde}, //AE_escapeC2

{0x50, 0x45},
{0x51, 0x88},

{0x56, 0x03},
{0x57, 0xf7},
{0x58, 0x14},
{0x59, 0x88},
{0x5a, 0x04},

//New Weight For Samsung
//{0x60, 0xaa},
//{0x61, 0xaa},
//{0x62, 0xaa},
//{0x63, 0xaa},
//{0x64, 0xaa},
//{0x65, 0xaa},
//{0x66, 0xab},
//{0x67, 0xEa},
//{0x68, 0xab},
//{0x69, 0xEa},
//{0x6a, 0xaa},
//{0x6b, 0xaa},
//{0x6c, 0xaa},
//{0x6d, 0xaa},
//{0x6e, 0xaa},
//{0x6f, 0xaa},

{0x60, 0x55}, // AEWGT1
{0x61, 0x55}, // AEWGT2
{0x62, 0x6a}, // AEWGT3
{0x63, 0xa9}, // AEWGT4
{0x64, 0x6a}, // AEWGT5
{0x65, 0xa9}, // AEWGT6
{0x66, 0x6a}, // AEWGT7
{0x67, 0xa9}, // AEWGT8
{0x68, 0x6b}, // AEWGT9
{0x69, 0xe9}, // AEWGT10
{0x6a, 0x6a}, // AEWGT11
{0x6b, 0xa9}, // AEWGT12
{0x6c, 0x6a}, // AEWGT13
{0x6d, 0xa9}, // AEWGT14
{0x6e, 0x55}, // AEWGT15
{0x6f, 0x55}, // AEWGT16

{0x70, 0x76}, //6e
{0x71, 0x00}, //82(+8)->+0

// haunting control
{0x76, 0x43},
{0x77, 0xe2}, //04 //f2
{0x78, 0x23}, //Yth1
{0x79, 0x42}, //Yth2 //46
{0x7a, 0x23}, //23
{0x7b, 0x22}, //22
{0x7d, 0x23},

//{0x83, 0x01}, //EXP Normal 33.33 fps 
//{0x84, 0x5f}, 
//{0x85, 0x00}, 

//{0x86, 0x02}, //EXPMin 5859.38 fps
//{0x87, 0x00}, 

//{0x88, 0x04}, //EXP Max 10.00 fps 
//{0x89, 0x92}, 
//{0x8a, 0x00}, 

//{0x8B, 0x75}, //EXP100 
//{0x8C, 0x00}, 
//{0x8D, 0x61}, //EXP120 
//{0x8E, 0x00}, 

//{0x9c, 0x18}, //EXP Limit 488.28 fps 
//{0x9d, 0x00}, 
//{0x9e, 0x02}, //EXP Unit 
//{0x9f, 0x00}, 
{0x03, 0x20}, //Page 20
{0x83, 0x01}, //EXP Normal 33.33 fps 
{0x84, 0x8b}, 
{0x85, 0x82}, 
{0x86, 0x01}, //EXPMin 5859.38 fps
{0x87, 0xf4}, 
{0x88, 0x02}, //EXP Max 6.25 fps 
{0x89, 0x93}, 
{0x8a, 0x2E}, 
{0x8B, 0x83}, //EXP100 
{0x8C, 0xD6}, 
{0x8D, 0x6D}, //EXP120 
{0x8E, 0x60}, 
{0x9c, 0x17}, //EXP Limit 488.28 fps 
{0x9d, 0x70}, 
{0x9e, 0x01}, //EXP Unit 
{0x9f, 0xf4}, 


//AE_Middle Time option
//{0xa0, 0x03},
//{0xa1, 0xa9},
//{0xa2, 0x80},

{0xb0, 0x18},
{0xb1, 0x14}, //ADC 400->560
{0xb2, 0xe0}, //d0
{0xb3, 0x18},
{0xb4, 0x1a},
{0xb5, 0x44},
{0xb6, 0x2f},
{0xb7, 0x28},
{0xb8, 0x25},
{0xb9, 0x22},
{0xba, 0x21},
{0xbb, 0x20},
{0xbc, 0x1f},
{0xbd, 0x1f},
{0xc8, 0x80},
{0xc9, 0x40},

/////// PAGE 22 START ///////
{0x03, 0x22},
{0x10, 0xfd},
{0x11, 0x2e},
{0x19, 0x01}, // Low On //
{0x20, 0x30},
{0x21, 0x80},
{0x24, 0x01},
//{0x25, 0x00}, //7f New Lock Cond & New light stable

{0x30, 0x80},
{0x31, 0x80},
{0x38, 0x11},
{0x39, 0x34},

{0x40, 0xf4},
{0x41, 0x55}, //44
{0x42, 0x33}, //43

{0x43, 0xf6},
{0x44, 0x55}, //44
{0x45, 0x44}, //33

{0x46, 0x00},
{0x50, 0xb2},
{0x51, 0x81},
{0x52, 0x98},

{0x80, 0x40}, //3e
{0x81, 0x20},
{0x82, 0x3e},

{0x83, 0x5e}, //5e
{0x84, 0x1e}, //24
{0x85, 0x5e}, //54 //56 //5a
{0x86, 0x22}, //24 //22

{0x87, 0x49},
{0x88, 0x39},
{0x89, 0x37}, //38
{0x8a, 0x28}, //2a

{0x8b, 0x41}, //47
{0x8c, 0x39}, 
{0x8d, 0x34}, 
{0x8e, 0x28}, //2c

{0x8f, 0x53}, //4e
{0x90, 0x52}, //4d
{0x91, 0x51}, //4c
{0x92, 0x4e}, //4a
{0x93, 0x4a}, //46
{0x94, 0x45},
{0x95, 0x3d},
{0x96, 0x31},
{0x97, 0x28},
{0x98, 0x24},
{0x99, 0x20},
{0x9a, 0x20},

{0x9b, 0x77},
{0x9c, 0x77},
{0x9d, 0x48},
{0x9e, 0x38},
{0x9f, 0x30},

{0xa0, 0x60},
{0xa1, 0x34},
{0xa2, 0x6f},
{0xa3, 0xff},

{0xa4, 0x14}, //1500fps
{0xa5, 0x2c}, // 700fps
{0xa6, 0xcf},

{0xad, 0x40},
{0xae, 0x4a},

{0xaf, 0x28},  // low temp Rgain
{0xb0, 0x26},  // low temp Rgain

{0xb1, 0x00}, //{0x20 -> {0x00 0405 modify
{0xb4, 0xea},
{0xb8, 0xa0}, //a2: b-2}, R+2  //b4 B-3}, R+4 lowtemp
{0xb9, 0x00},

/////// PAGE 20 ///////
{0x03, 0x20},
{0x10, 0x8c},

// PAGE 20
{0x03, 0x20}, //page 20
{0x10, 0x9c}, //ae off

// PAGE 22
{0x03, 0x22}, //page 22
{0x10, 0xe9}, //awb off



// PAGE 0
{0x03, 0x00},
{0x0e, 0x04}, //PLL On
{0x0e, 0x74}, //PLLx2
{0x03, 0x00}, // Dummy 750us
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00},
{0x03, 0x00}, // Page 0
{0x01, 0xc0}, // Sleep Off f8

};




struct hy252_regval_list  hy252_before[] = {
};
struct hy252_regval_list  hy252_after[] = { 
};                                            

struct hy252_regval_list  hy252_svga_low_regs[] = {   
};

struct hy252_regval_list  hy252_to_preview_320_240_regs[] = {
	
	
	
	
	
	{0x03, 0x00},
	{0x10, 0x21},
	{0x03, 0x18},// scaling
	{0x12, 0x20},
	{0x10, 0x05},
	{0x11, 0x00},
	{0x20, 0x05},
	{0x21, 0x00},
	{0x22, 0x03},
	{0x23, 0xc0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x05},
	{0x29, 0x00},
	{0x2a, 0x03},
	{0x2b, 0xc0},
	{0x2c, 0x0a},
	{0x2d, 0x00},
	{0x2e, 0x0a},
	{0x2f, 0x00},
	{0x30, 0x44},// 1280*960

};

struct hy252_regval_list  hy252_to_preview_640_480_regs[] = {
	{0x03, 0x00},
	{0x10, 0x11},
	{0x12, 0x00},
	{0x40, 0x01},
	{0x41, 0x68},

	{0x03, 0x18},// scaling
	{0x12, 0x20},
	{0x10, 0x05},
	{0x11, 0x00},
	{0x20, 0x05},
	{0x21, 0x00},
	{0x22, 0x03},
	{0x23, 0xc0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x05},
	{0x29, 0x00},
	{0x2a, 0x03},
	{0x2b, 0xc0},
	{0x2c, 0x0a},
	{0x2d, 0x00},
	{0x2e, 0x0a},
	{0x2f, 0x00},
	{0x30, 0x44},// 1280*960

	{0x03, 0x20}, //Page 20 Mclk=21.6Mhz
	{0x10, 0x9c},
	{0x83, 0x01}, //EXP Normal 33.33 fps 
	{0x84, 0x8b}, 
	{0x85, 0x82}, 
	{0x86, 0x01}, //EXPMin 5859.38 fps
	{0x87, 0xf4}, 
	{0x88, 0x02}, //EXP Max 6.25 fps 
	{0x89, 0x93}, 
	{0x8a, 0x2E}, 
	{0x8B, 0x83}, //EXP100 
	{0x8C, 0xD6}, 
	{0x8D, 0x6D}, //EXP120 
	{0x8E, 0x60}, 
	{0x9c, 0x17}, //EXP Limit 488.28 fps 
	{0x9d, 0x70}, 
	{0x9e, 0x01}, //EXP Unit 
	{0x9f, 0xf4},

        {0x03, 0x22},
	{0x10, 0xea},
};

struct hy252_regval_list  hy252_svga_high_regs[] = {  
};

struct hy252_regval_list  hy252_xuga_regs[] = { 
};
struct hy252_regval_list  hy252_xuga_high_regs[] = {
	{0x03, 0x00},
	{0x10, 0x00},
	{0x03, 0x18},
	{0x10, 0x00},
};
struct hy252_regval_list  hy252_xuga_upmid_regs[] = {
};
struct hy252_regval_list  hy252_xuga_mid_regs[] = { 

	{0x03, 0x00},
	{0x10, 0x11},
	{0x03, 0x18},
	{0x12, 0x20},
	{0x10, 0x05},
	{0x11, 0x00},
	{0x20, 0x05},
	{0x21, 0x00},
	{0x22, 0x03},
	{0x23, 0xc0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x05},
	{0x29, 0x00},
	{0x2a, 0x03},
	{0x2b, 0xc0},
	{0x2c, 0x0a},
	{0x2d, 0x00},
	{0x2e, 0x0a},
	{0x2f, 0x00},
	{0x30, 0x44},



#if 0
	{0x03, 0x00},
	{0x10, 0x00},
	{0x03, 0x18},
	{0x12, 0x20},
	{0x10, 0x07},
	{0x11, 0x00},
	{0x20, 0x02},
	{0x21, 0x80},
	{0x22, 0x01},
	{0x23, 0xe0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x02},
	{0x29, 0x80},
	{0x2a, 0x01},
	{0x2b, 0xe0},
	{0x2c, 0x14},
	{0x2d, 0x00},
	{0x2e, 0x14},
	{0x2f, 0x00},
	{0x30, 0x64},// 640*480*/
#endif
};
struct hy252_regval_list  hy252_xuga_low_regs[] = { 
	{0x03, 0x00},
	{0x10, 0x21},
	{0x03, 0x18},
	{0x12, 0x20},
	{0x10, 0x05},
	{0x11, 0x00},
	{0x20, 0x05},
	{0x21, 0x00},
	{0x22, 0x03},
	{0x23, 0xc0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x05},
	{0x29, 0x00},
	{0x2a, 0x03},
	{0x2b, 0xc0},
	{0x2c, 0x0a},
	{0x2d, 0x00},
	{0x2e, 0x0a},
	{0x2f, 0x00},
	{0x30, 0x44},



#if 0
	{0x03, 0x00},
	{0x10, 0x00},
	{0x03, 0x18},
	{0x12, 0x20},
	{0x10, 0x07},
	{0x11, 0x00},
	{0x20, 0x01},
	{0x21, 0x40},
	{0x22, 0x00},
	{0x23, 0xf0},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x01},
	{0x29, 0x40},
	{0x2a, 0x00},
	{0x2b, 0xf0},
	{0x2c, 0x28},
	{0x2d, 0x00},
	{0x2e, 0x28},
	{0x2f, 0x00},
	{0x30, 0x44},// 320*240  
#endif
};
struct hy252_regval_list hy252_stop_regs[] = {

};
struct hy252_regval_list  hy252_sepia_regs[] = {
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x33},
	{0x13, 0x0A},
	{0x44, 0x70},
	{0x45, 0x98},
};
struct hy252_regval_list  hy252_bluish_regs[] = { 
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x03},
//	{0x40, 0x00},
	{0x13, 0x0A},
	{0x44, 0xb0},
	{0x45, 0x40},
};
struct hy252_regval_list  hy252_greenish_regs[] = {
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x03},
//	{0x40, 0x00},
	{0x13, 0x0A},
	{0x44, 0x30},
	{0x45, 0x50},
};
struct hy252_regval_list  hy252_reddish_regs[] = { 
};

struct hy252_regval_list  hy252_yellowish_regs[] = {
};
struct hy252_regval_list  hy252_bandw_regs[] = {
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x03},
	{0x13, 0x0A},
//	{0x40, 0x00},
	{0x44, 0x80},
	{0x45, 0x80},
};
struct hy252_regval_list  hy252_negative_regs[] = {
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x08},
	{0x13, 0x0A},
	{0x14, 0x00},
};
struct hy252_regval_list  hy252_normal_regs[] = { 
	{0x03, 0x10},
	{0x11, 0x03},
	{0x12, 0x30},
	{0x13, 0x0A},
	{0x44, 0x80},
	{0x45, 0x80},
};

struct hy252_regval_list  hy252_auto_regs[] = {
	{0x03, 0x22},
	{0x11, 0x2e},
	{0x83, 0x5e},
	{0x84, 0x1e},
	{0x85, 0x5e},
	{0x86, 0x22},
};

struct hy252_regval_list  hy252_sunny_regs[] = {

	{0x03, 0x22},
	{0x11, 0x28},		  
	{0x80, 0x59},
	{0x82, 0x29},
	{0x83, 0x60},
	{0x84, 0x50},
	{0x85, 0x2f},
	{0x86, 0x23},
};
struct hy252_regval_list  hy252_cloudy_regs[] = {
	{0x03, 0x22},
	{0x11, 0x28},
	{0x80, 0x71},
	{0x82, 0x2b},
	{0x83, 0x72},
	{0x84, 0x70},
	{0x85, 0x2b},
	{0x86, 0x28},
};

struct hy252_regval_list  hy252_office_regs[] = {
	{0x03, 0x22},
	{0x11, 0x28},
	{0x80, 0x29},
	{0x82, 0x54},
	{0x83, 0x2e},
	{0x84, 0x23},
	{0x85, 0x58},
	{0x86, 0x4f},

};
struct hy252_regval_list  hy252_home_regs[] = {
	{0x03, 0x22},
	{0x80, 0x24},
	{0x81, 0x20},
	{0x82, 0x58},
	{0x83, 0x27},
	{0x84, 0x22},
	{0x85, 0x58},
	{0x86, 0x52},
};
struct hy252_regval_list  hy252_saturation_0_regs[] = {};
struct hy252_regval_list  hy252_saturation_1_regs[] = {};
struct hy252_regval_list  hy252_saturation_2_regs[] = {};
struct hy252_regval_list  hy252_saturation_3_regs[] = {};
struct hy252_regval_list  hy252_saturation_4_regs[] = {};
struct hy252_regval_list  hy252_brightness_0_regs[] = {};
struct hy252_regval_list  hy252_brightness_1_regs[] = {};
struct hy252_regval_list  hy252_brightness_2_regs[] = {};
struct hy252_regval_list  hy252_brightness_3_regs[] = {};
struct hy252_regval_list  hy252_brightness_4_regs[] = {};
struct hy252_regval_list  hy252_brightness_5_regs[] = {};
struct hy252_regval_list  hy252_brightness_6_regs[] = {};

struct hy252_regval_list  hy252_contrast_0_regs[] = {};
struct hy252_regval_list  hy252_contrast_1_regs[] = {};
struct hy252_regval_list  hy252_contrast_2_regs[] = {};
struct hy252_regval_list  hy252_contrast_3_regs[] = {};
struct hy252_regval_list  hy252_contrast_4_regs[] = {};
struct hy252_regval_list  hy252_contrast_5_regs[] = {};
struct hy252_regval_list  hy252_contrast_6_regs[] = {};
struct hy252_regval_list  hy252_sharpness_0_regs[] = {};
struct hy252_regval_list  hy252_sharpness_1_regs[] = {};
struct hy252_regval_list  hy252_sharpness_2_regs[] = {};
struct hy252_regval_list  hy252_sharpness_3_regs[] = {};
struct hy252_regval_list  hy252_sharpness_4_regs[] = {};
struct hy252_regval_list  hy252_sharpness_auto_regs[] = {};

struct hy252_regval_list  hy252_night_mode_on_regs[] = {};
struct hy252_regval_list  hy252_night_mode_off_regs[] ={};
