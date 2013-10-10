/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of i80 library 
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#ifndef __I80_LIB_H__
#define __I80_LIB_H__

//// i80if status indication.
#define I80IF_STAT_TRANSMIT_FRAME_END			(1<<4)
#define I80IF_STAT_TRANSMITTING_NORMAL_CMD		(1<<3)
#define I80IF_STAT_TRANSMITTING_AUTO_CMD		(1<<2)
#define I80IF_STAT_TRANSMITTING_FRAME			(1<<1)
#define I80IF_STAT_NORMAL_CMD_STARTED			(1<<0)

//########################################
//
#define I80_ROTATE_IMAGE_0				(0)
#define I80_ROTATE_IMAGE_90				(1)
#define I80_ROTATE_IMAGE_180			(2)
#define I80_ROTATE_IMAGE_270			(3)
#define I80_ROTATE_IMAGE_LEFT_RIGHT		(4)
#define I80_ROTATE_IMAGE_UP_DOWN		(5)
#define I80_ROTATE_IMAGE_90_LEFT_RIGHT	(6)
#define I80_ROTATE_IMAGE_270_LEFT_RIGHT	(7)

//########################################
//
#define I80IF_VALIDATE_DATA(n)		((n) &= 0x3FFFF)

//########################################
//
// i80 interface data format.
// format = [dbi_compatible_en]_[port/bus type]_[port distributed]_[data style]_[data LSB first]_[data use low port]
typedef enum{
	I80IF_BUS18_xx_xx_x_x	=	0x180,	// 0_11_xx_xxx_x_x

	I80IF_BUS9_1x_xx_0_x	=	0x0c0,	// 0_01_1x_xxx_0_x
	I80IF_BUS9_1x_xx_1_x	=	0x0c2,	// 0_01_1x_xxx_1_x
	I80IF_BUS9_0x_xx_0_x	=	0x080,	// 0_01_0x_xxx_0_x
	I80IF_BUS9_0x_xx_1_x	=	0x082,	// 0_01_0x_xxx_1_x

	I80IF_BUS16_10_11_x_x	=	0x14C,	// 0_10_10_x11_x_x
	I80IF_BUS16_00_11_x_x	=	0x10C,	// 0_10_00_x11_x_x
	I80IF_BUS16_11_11_x_x	=	0x16C,	// 0_10_11_x11_x_x
	I80IF_BUS16_01_11_x_x	=	0x12C,	// 0_10_01_x11_x_x
	I80IF_BUS16_10_10_0_0	=	0x148,	// 0_10_10_x10_0_0
	I80IF_BUS16_10_10_0_1	=	0x149,	// 0_10_10_x10_0_1
	I80IF_BUS16_10_10_1_0	=	0x14A,	// 0_10_10_x10_1_0
	I80IF_BUS16_10_10_1_1	=	0x14B,	// 0_10_10_x10_1_1
	I80IF_BUS16_10_01_1_0	=	0x146,	// 0_10_10_x01_1_0
	I80IF_BUS16_10_01_1_1	=	0x147,	// 0_10_10_x01_1_1
	I80IF_BUS16_10_01_0_0	=	0x144,	// 0_10_10_x01_0_0
	I80IF_BUS16_10_01_0_1	=	0x145,	// 0_10_10_x01_0_1
	I80IF_BUS16_00_10_0_0	=	0x108,	// 0_10_00_x10_0_0
	I80IF_BUS16_00_10_0_1	=	0x109,	// 0_10_00_x10_0_1
	I80IF_BUS16_00_10_1_0	=	0x10A,	// 0_10_00_x10_1_0
	I80IF_BUS16_00_10_1_1	=	0x10B,	// 0_10_00_x10_1_1
	I80IF_BUS16_00_01_1_0	=	0x106,	// 0_10_00_x01_1_0
	I80IF_BUS16_00_01_1_1	=	0x107,	// 0_10_00_x01_1_1
	I80IF_BUS16_00_01_0_0	=	0x104,	// 0_10_00_x01_0_0
	I80IF_BUS16_00_01_0_1	=	0x105,	// 0_10_00_x01_0_1
	I80IF_BUS16_11_10_0_0	=	0x168,	// 0_10_11_x10_0_0
	I80IF_BUS16_11_10_0_1	=	0x169,	// 0_10_11_x10_0_1
	I80IF_BUS16_11_10_1_0	=	0x16A,	// 0_10_11_x10_1_0
	I80IF_BUS16_11_10_1_1	=	0x16B,	// 0_10_11_x10_1_1
	I80IF_BUS16_11_01_1_1	=	0x167,	// 0_10_11_x01_1_1
	I80IF_BUS16_11_01_0_1	=	0x165,	// 0_10_11_x01_0_1
	I80IF_BUS16_11_01_1_0	=	0x166,	// 0_10_11_x01_1_0
	I80IF_BUS16_01_10_0_0	=	0x128,	// 0_10_01_x10_0_0
	I80IF_BUS16_01_10_0_1	=	0x129,	// 0_10_01_x10_0_1
	I80IF_BUS16_01_10_1_0	=	0x12A,	// 0_10_01_x10_1_0
	I80IF_BUS16_01_10_1_1	=	0x12B,	// 0_10_01_x10_1_1
	I80IF_BUS16_01_01_1_0	=	0x126,	// 0_10_01_x01_1_0
	I80IF_BUS16_01_01_1_1	=	0x127,	// 0_10_01_x01_1_1
	I80IF_BUS16_01_01_0_0	=	0x124,	// 0_10_01_x01_0_0
	I80IF_BUS16_01_01_0_1	=	0x125,	// 0_10_01_x01_0_1

	I80IF_BUS8_0x_11_0_x	=	0x00C,	// 0_00_0x_x11_0_x
	I80IF_BUS8_0x_11_1_x	=	0x00D,	// 0_00_0x_x11_1_x
	I80IF_BUS8_1x_11_0_x	=	0x04C,	// 0_00_1x_x11_0_x
	I80IF_BUS8_1x_11_1_x	=	0x04E,	// 0_00_1x_x11_1_x
	I80IF_BUS8_0x_10_0_1	=	0x009,	// 0_00_0x_x10_0_1
	I80IF_BUS8_0x_10_0_0	=	0x008,	// 0_00_0x_x10_0_0
	I80IF_BUS8_0x_10_1_1	=	0x00B,	// 0_00_0x_x10_1_1
	I80IF_BUS8_0x_10_1_0	=	0x00A,	// 0_00_0x_x10_1_0
	I80IF_BUS8_1x_10_0_1	=	0x049,	// 0_00_1x_x10_0_1
	I80IF_BUS8_1x_10_0_0	=	0x048,	// 0_00_1x_x10_0_0
	I80IF_BUS8_1x_10_1_1	=	0x04B,	// 0_00_1x_x10_1_1
	I80IF_BUS8_1x_10_1_0	=	0x04A,	// 0_00_1x_x1x_1_0
	I80IF_BUS8_0x_01_0_1	=	0x005,	// 0_00_0x_x01_0_1
	I80IF_BUS8_0x_01_0_0	=	0x004,	// 0_00_0x_x01_0_0
	I80IF_BUS8_0x_01_1_1	=	0x007,	// 0_00_0x_x01_1_1
	I80IF_BUS8_0x_01_1_0	=	0x006,	// 0_00_0x_x01_1_0
	I80IF_BUS8_0x_00_1_1	=	0x003,	// 0_00_0x_x00_1_1
	I80IF_BUS8_0x_00_1_0	=	0x002,	// 0_00_0x_x00_1_0
	I80IF_BUS8_0x_00_0_1	=	0x001,	// 0_00_0x_x00_0_1
	I80IF_BUS8_0x_00_0_0	=	0x000,	// 0_00_0x_x00_0_0
	I80IF_BUS8_1x_01_0_1	=	0x045,	// 0_00_1x_x01_0_1
	I80IF_BUS8_1x_01_0_0	=	0x044,	// 0_00_1x_x01_0_0
	I80IF_BUS8_1x_01_1_1	=	0x047,	// 0_00_1x_x01_1_1
	I80IF_BUS8_1x_01_1_0	=	0x046,	// 0_00_1x_x01_1_0
	I80IF_BUS8_1x_00_1_1	=	0x043,	// 0_00_1x_x00_1_1
	I80IF_BUS8_1x_00_1_0	=	0x042,	// 0_00_1x_x00_1_0
	I80IF_BUS8_1x_00_0_1	=	0x041,	// 0_00_1x_x00_0_1
	I80IF_BUS8_1x_00_0_0	=	0x040,	// 0_00_1x_x00_0_0
	
	I80IF_DBI_BUS16_xx_111_x_x	=	0x31c,	// 1_10_xx_111_x_x   16-wire, 8+16,  24bpp
	I80IF_DBI_BUS16_xx_010_x_x	=	0x308,	// 1_10_xx_010_x_x   16-wire, 6+12,  18bpp
	I80IF_DBI_BUS16_xx_011_x_x	=	0x30c,	// 1_10_xx_011_x_x   16-wire, 16,    16bpp
	I80IF_DBI_BUS16_xx_110_x_x	=	0x318,	// 1_10_xx_110_x_x   16-wire, 12,    12bpp
	I80IF_DBI_BUS9_xx_010_x_x	=	0x288,	// 1_01_xx_010_x_x   9-wire,  9+9,   18bpp
	I80IF_DBI_BUS8_xx_110_1_x	=	0x21c,	// 1_00_xx_111_x_x   8-wire,  8+8+8, 24bpp
	I80IF_DBI_BUS8_xx_010_x_x	=	0x208,	// 1_00_xx_010_x_x   8-wire,  6+6+6, 18bpp
	I80IF_DBI_BUS8_xx_010_1_x	=	0x20c,	// 1_00_xx_011_x_x   8-wire,  8+8,   16bpp
	I80IF_DBI_BUS8_xx_001_x_x	=	0x214	// 1_00_xx_101_x_x   8-wire,  8,     8bpp
}i80if_data_format;

//-------------------------------------------
typedef enum{
	I80IF_SIGNAL_ACTIVE_LOW = 0,
	I80IF_SIGNAL_ACTIVE_HIGH
}i80if_signal_active_level;

typedef struct{
	i80if_signal_active_level Signal_rd;
	i80if_signal_active_level Signal_wr;
	i80if_signal_active_level Signal_rs;	// 不要设置，因为在具体配置cmd列表时会指定RS的电平，如果这里配置了，就表示是否再翻转一次
	i80if_signal_active_level Signal_cs1;
	i80if_signal_active_level Signal_cs0;

	IM_INT32 nCycles_cs_setup;
	IM_INT32 nCycles_wr_setup;
	IM_INT32 nCycles_wr_active;
	IM_INT32 nCycles_wr_hold;
}i80if_port_property;

//-------------------------------------------
#define I80IF_CMD_FIFO_NUM		(16)    // i80if command list, 严格意义上不能称之为fifo,这里先姑且叫之

typedef enum{
	I80IF_RS_LEVEL_LOW = 0,
	I80IF_RS_LEVEL_HIGH
}i80if_RS_level;

typedef enum{
	I80IF_CMD_TYPE_DISABLE = 0,
	I80IF_CMD_TYPE_NORMAL,
	I80IF_CMD_TYPE_AUTO,
	I80IF_CMD_TYPE_NORMAL_AUTO
}i80if_cmd_type;

typedef struct{
	IM_UINT32 uCmd;
	i80if_RS_level eRS_level;
	i80if_cmd_type eCmd_type;
}i80if_cmd_property;

//-------------------------------------------
typedef enum{
	I80IF_AUTOCMD_RATE_PER_1_FRAME = 0,
	I80IF_AUTOCMD_RATE_PER_2_FRAME,
	I80IF_AUTOCMD_RATE_PER_4_FRAME,
	I80IF_AUTOCMD_RATE_PER_6_FRAME,
	I80IF_AUTOCMD_RATE_PER_8_FRAME,
	I80IF_AUTOCMD_RATE_PER_10_FRAME,
	I80IF_AUTOCMD_RATE_PER_12_FRAME,
	I80IF_AUTOCMD_RATE_PER_14_FRAME,
	I80IF_AUTOCMD_RATE_PER_16_FRAME,
	I80IF_AUTOCMD_RATE_PER_18_FRAME,
	I80IF_AUTOCMD_RATE_PER_20_FRAME,
	I80IF_AUTOCMD_RATE_PER_22_FRAME,
	I80IF_AUTOCMD_RATE_PER_24_FRAME,
	I80IF_AUTOCMD_RATE_PER_26_FRAME,
	I80IF_AUTOCMD_RATE_PER_28_FRAME,
	I80IF_AUTOCMD_RATE_PER_30_FRAME
}i80if_autocmd_rate;


typedef struct{
	IM_UINT32 DISAUTOCMD;
	i80if_autocmd_rate AUTO_CMD_RATE;
}i80if_autocmd_porperty;

//-------------------------------------------
typedef enum{
	I80IF_LCD_0 = 0,
	I80IF_LCD_1
}i80if_supports_lcd;

//-------------------------------------------
typedef struct{

	i80if_port_property Port_property;				// i80 interface属性，包括信号极性和周期数
	IM_UINT32 INTMASK;								// i80 frame end 中断使能标志  i80 normal cmd over 中断使能标志
	i80if_supports_lcd Main_lcd;						// i80 main lcd, user doesn't need care.
	i80if_data_format Data_format;					// i80 interface接口数据格式
	i80if_cmd_property stCmd[I80IF_CMD_FIFO_NUM];	// i80 cmd fifo配置
	i80if_autocmd_porperty stAuto_cmd_property;		// i80 auto cmd的配置

	IM_UINT32 screen_width;      // 物理屏幕宽度
	IM_UINT32 screen_height;     // 物理屏幕高度
}i80c_config_t;





IM_RET i80lib_init(IM_INT32 idsx, i80c_config_t *cfg);
IM_RET i80lib_open(IM_INT32 idsx);
IM_RET i80lib_close(IM_INT32 idsx);
IM_RET i80lib_deinit(IM_INT32 idsx);
void i80lib_transmit_normal_cmd(IM_UINT32 fEn,IM_UINT32 fWaitComplete);	
void i80lib_manual_write_once(IM_UINT32 reg_addr,IM_INT32 idsx);
void i80lib_manual_read_once(IM_UINT32 *pReg_val,IM_INT32 idsx);
void i80if_manual_enable(void );
void i80if_manual_disbale(void);


#endif	// __I80_LIB_H__
