/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of i80 library
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.2     liu@2012/12/20:  adjust the code structure
*****************************************************************************/
#include <InfotmMedia.h>
#include <ids_pwl.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <i80_api.h>
#include <i80.h>
#include <i80_lib.h>

#define DBGINFO		1
#define DBGWARN 	1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"I80LIB_I:"
#define WARNHEAD	"I80LIB_W:"
#define ERRHEAD		"I80LIB_E:"
#define TIPHEAD		"I80LIB_T:"

static void i80lib_set_cmd_list(i80c_config_t *cfg,i80if_cmd_property *pCmd,IM_INT32 nIndex,IM_INT32 idsx,i80if_supports_lcd Current_lcd); 
static void i80if_delay_ns( IM_UINT32 num);
static void i80if_manual_sig_doe_output(void);
static void i80if_manual_sig_doe_input(void);
static void i80if_manual_sig_rd_low(void);
static void i80if_manual_sig_rd_high(void);
static void i80if_manual_sig_wr_low(void);
static void i80if_manual_sig_wr_high(void);
static void i80lib_wait_idle(void);
void i80lib_transmit_normal_cmd(IM_UINT32 fEn,IM_UINT32 fWaitComplete);
static IM_UINT32 module;

void i80lib_set_idsxNo(IM_UINT32 idsx)
{
	module = (idsx == 0) ?  MODULE_IDS0: MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
}

IM_RET i80lib_init(IM_INT32 idsx, i80c_config_t *cfg)
{
	IM_UINT32 val;
	IM_UINT32 clkdiv=1;
	IM_UINT32 i;
	IM_UINT32 module = (idsx == 0)?MODULE_IDS0 : MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	i80lib_set_idsxNo(idsx);
	if(cfg != NULL){
	    i80lib_transmit_normal_cmd(1,1);         //   I80/RGB Triggle模式（使用DMA代替CPU来搬运图像数据）
		
		val = ((cfg->Port_property.Signal_rd) << I80IFCON0_RDPOL) | ((cfg->Port_property.Signal_wr) << I80IFCON0_WRPOL)|
			((cfg->Port_property.Signal_rs) << I80IFCON0_RSPOL) | ((cfg->Port_property.Signal_cs1) << I80IFCON0_CS1POL) |
			((cfg->Port_property.Signal_cs0) << I80IFCON0_CS0POL) | ((cfg->Port_property.nCycles_cs_setup) << I80IFCON0_LCD_CS_SETUP)
			| ((cfg->Port_property.nCycles_wr_setup) << I80IFCON0_LCD_WR_SETUP) | ((cfg->Port_property.nCycles_wr_active) << I80IFCON0_LCD_WR_ACTIVE)
			| ((cfg->Port_property.nCycles_wr_hold) << I80IFCON0_LCD_WR_HOLD) |((cfg->INTMASK) << I80IFCON0_INTMASK) 
			| ((cfg->Main_lcd) << I80IFCON0_MAINCONFIG)| (0 << I80IFCON0_I80IFEN) ;
		idspwl_write_reg(module, I80IFCON0, val);

		val = ((cfg->Data_format) << I80IFCON1_DATA_FORMAT) | ((cfg->stAuto_cmd_property.DISAUTOCMD) << I80IFCON1_DISAUTOCMD)
			| ((cfg->stAuto_cmd_property.AUTO_CMD_RATE) << I80IFCON1_AUTO_CMD_RATE);
		idspwl_write_reg(module, I80IFCON1, val);

		for(i=0;i<I80IF_CMD_FIFO_NUM;i++)
		{
			i80lib_set_cmd_list(cfg,&cfg->stCmd[i],i,idsx,0);
		}

		idspwl_write_reg(module ,I80MANCON, 0);

		val = 	(5 << LCDCON1_CLKVAL) | (0<<LCDCON1_VMMODE) | (3<<LCDCON1_PNRMODE) |
			(12<<LCDCON1_STNBPP) | (0<<LCDCON1_ENVID);
		idspwl_write_reg(module, LCDCON1, val);

		val = (100 <<LCDCON2_VBPD) | (2<<LCDCON2_VFPD);
		idspwl_write_reg(module, LCDCON2, val);

		val = (2<<LCDCON3_VSPW) | (96<<LCDCON3_HSPW);
		idspwl_write_reg(module, LCDCON3,val);

		val =  (48<<LCDCON4_HBPD) | (16<<LCDCON4_HFPD);
		idspwl_write_reg(module, LCDCON4 ,val);

		val =  (6<<LCDCON5_RGBORDER) | (1<<LCDCON5_DSPTYPE) | (0<<LCDCON5_INVVCLK) | (0<<LCDCON5_INVVLINE) |
			(0<<LCDCON5_INVVFRAME) | (0<<LCDCON5_INVVD) |(0<<LCDCON5_INVVDEN) | (0<< LCDCON5_INVPWREN) | (0<<LCDCON5_PWREN);
		idspwl_write_reg(module, LCDCON5,val);

		val = ((cfg->screen_height -1) << LCDCON6_LINEVAL) | ((cfg->screen_width -1) << LCDCON6_HOZVAL);
		idspwl_write_reg(module, LCDCON6, val);


		val = ((clkdiv-1) << LCDCON1_CLKVAL) | (0 << LCDCON1_VMMODE) | (3 << LCDCON1_PNRMODE) |
			(12<<LCDCON1_STNBPP) | (0<<LCDCON1_ENVID);
		idspwl_write_reg(module, LCDCON1, val);

	
		// disable ids freq auto-change
		idspwl_write_regbit(module, LCDVCLKFSR, 24, 4, 0x0F);
	}
	else{
		// make cfg has its contents in the i80_api .
		return IM_RET_FAILED;
	}
	return IM_RET_OK;
}

IM_RET i80lib_deinit(IM_INT32 idsx)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return IM_RET_OK;
}

IM_RET i80lib_open(IM_INT32 idsx)
{   
	IM_UINT32 val=1;
	IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	idspwl_write_regbit(module,I80IFCON0,0,1,val);
	idspwl_write_regbit(module,LCDCON1,0,1,val);
	idspwl_write_regbit(module,LCDCON5,11,2,val);
	return IM_RET_OK;
}

IM_RET i80lib_close(IM_INT32 idsx)
{
	IM_UINT32 val;
	IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	val =0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	idspwl_write_regbit(module,LCDCON5,11,2,val);
	idspwl_write_regbit(module,LCDCON1,0,1,val);
	idspwl_write_regbit(module,I80IFCON0,0,1,val);
	return IM_RET_OK;
}
/*===============================================
func: 配置cmd列表
log:
-----------------------------------------------*/
void i80lib_set_cmd_list(i80c_config_t *cfg,i80if_cmd_property *pCmd,IM_INT32 nIndex,IM_INT32 idsx,i80if_supports_lcd Current_lcd) 	// 在16个cmd fifo中的索引,0-15
{
	IM_UINT32 tmp;
    IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	IM_UINT32 high_byte, low_byte;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if( (nIndex >= I80IF_CMD_FIFO_NUM) || (nIndex < 0) ){
		return;
	}

	cfg->stCmd[nIndex].uCmd = pCmd->uCmd;
	cfg->stCmd[nIndex].eCmd_type = pCmd->eCmd_type;
	cfg->stCmd[nIndex].eRS_level = pCmd->eRS_level;

	if( cfg->Main_lcd == Current_lcd )
	{
		i80lib_wait_idle();
		switch( cfg->Data_format )
		{
			case I80IF_BUS16_00_11_x_x:
			case I80IF_BUS8_0x_10_0_0:
				break;

			case I80IF_BUS18_xx_xx_x_x:
			case I80IF_BUS16_11_11_x_x:
			case I80IF_BUS16_11_10_0_0:
			case I80IF_BUS16_11_01_0_1:
				high_byte = (cfg->stCmd[nIndex].uCmd & 0xff00) << 2;	//高字节在DB10上开始
				low_byte = (cfg->stCmd[nIndex].uCmd & 0x00ff) << 1;	//低字节在DB1上开始
				cfg->stCmd[nIndex].uCmd = high_byte | low_byte;
				break;

			default:
				break;
		}

		idspwl_read_reg(module, I80CMDCON0, &tmp);                         //配置CMDX_EN
		tmp &= ~(0x3 << (nIndex << 1));
		tmp |= (pCmd->eCmd_type << (nIndex << 1));
		idspwl_write_reg(module, I80CMDCON0, tmp);

		idspwl_read_reg(module, I80CMDCON1, &tmp);                         //配置CMDX_RS
		tmp &= ~(0x3 << (nIndex << 1));
		tmp |= (pCmd->eRS_level << (nIndex << 1));
		idspwl_write_reg(module, I80CMDCON1, tmp);

		switch( nIndex )                                                   //I80CMDX ,写入数据脚VD[17-0]
		{
			case 0:		idspwl_write_reg(module, I80CMD00, pCmd->uCmd);	break;
			case 1:		idspwl_write_reg(module, I80CMD01, pCmd->uCmd);	break;
			case 2:		idspwl_write_reg(module, I80CMD02, pCmd->uCmd);	break;
			case 3:		idspwl_write_reg(module, I80CMD03, pCmd->uCmd);	break;
			case 4:		idspwl_write_reg(module, I80CMD04, pCmd->uCmd);	break;
			case 5:		idspwl_write_reg(module, I80CMD05, pCmd->uCmd);	break;
			case 6:		idspwl_write_reg(module, I80CMD06, pCmd->uCmd);	break;
			case 7:		idspwl_write_reg(module, I80CMD07, pCmd->uCmd);	break;
			case 8:		idspwl_write_reg(module, I80CMD08, pCmd->uCmd);	break;
			case 9:		idspwl_write_reg(module, I80CMD09, pCmd->uCmd);	break;
			case 10:	idspwl_write_reg(module, I80CMD10, pCmd->uCmd);	break;
			case 11:	idspwl_write_reg(module, I80CMD11, pCmd->uCmd);	break;
			case 12:	idspwl_write_reg(module, I80CMD12, pCmd->uCmd);	break;
			case 13:	idspwl_write_reg(module, I80CMD13, pCmd->uCmd);	break;
			case 14:	idspwl_write_reg(module, I80CMD14, pCmd->uCmd);	break;
			case 15:	idspwl_write_reg(module, I80CMD15, pCmd->uCmd);	break;

			default:
						break;
		}
	}
}

/*===============================================
func: 清除所有cmd列表
log:
-----------------------------------------------*/
void i80lib_clear_cmd_list(i80c_config_t *cfg,IM_INT32 idsx,i80if_supports_lcd Current_lcd)
{
	IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	IM_INT32 i;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	for(i=0; i<I80IF_CMD_FIFO_NUM; i++){
		cfg->stCmd[i].uCmd = 0;
		cfg->stCmd[i].eCmd_type = I80IF_CMD_TYPE_DISABLE;
		cfg->stCmd[i].eRS_level = I80IF_RS_LEVEL_LOW;
	}

	// 当更改当前Lcd命令列表配置时才去修改寄存器
	if( cfg->Main_lcd == Current_lcd){
		i80lib_wait_idle();
		idspwl_write_reg(module, I80CMDCON0, 0x00);
		idspwl_write_reg(module, I80CMDCON1, 0x00);
		idspwl_write_reg(module, I80CMD00, 0x00);
		idspwl_write_reg(module, I80CMD01, 0x00);
		idspwl_write_reg(module, I80CMD02, 0x00);
		idspwl_write_reg(module, I80CMD03, 0x00);
		idspwl_write_reg(module, I80CMD04, 0x00);
		idspwl_write_reg(module, I80CMD05, 0x00);
		idspwl_write_reg(module, I80CMD06, 0x00);
		idspwl_write_reg(module, I80CMD07, 0x00);
		idspwl_write_reg(module, I80CMD08, 0x00);
		idspwl_write_reg(module, I80CMD09, 0x00);
		idspwl_write_reg(module, I80CMD10, 0x00);
		idspwl_write_reg(module, I80CMD11, 0x00);
		idspwl_write_reg(module, I80CMD12, 0x00);
		idspwl_write_reg(module, I80CMD13, 0x00);
		idspwl_write_reg(module, I80CMD14, 0x00);
		idspwl_write_reg(module, I80CMD15, 0x00);
	}
}
/*===============================================
func: 返回当前i80模块的状态，返回值在i80_api.h里定义的
log:
-----------------------------------------------*/
IM_UINT32 i80lib_get_status(void)
{
	IM_UINT32 val, uStat;
	IM_UINT32 offset;
	offset = 0;
	uStat = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80TRIGCON, &val);

	if( val & (1<<I80TRIGCON_XFER_VIDEO_OVER) ){
		uStat |= I80IF_STAT_TRANSMIT_FRAME_END;
	}
	if( val & (1<<I80TRIGCON_TNORMALCMD) ){
		uStat |= I80IF_STAT_TRANSMITTING_NORMAL_CMD;
	}
	if( val & (1<<I80TRIGCON_TAUTOCMD) ){
		uStat |= I80IF_STAT_TRANSMITTING_AUTO_CMD;
	}
	if( val & (1<<I80TRIGCON_TVIDEO) ){
		uStat |= I80IF_STAT_TRANSMITTING_FRAME;
	}
	if( val & (1<<I80TRIGCON_NORMAL_CMD_ST) ){
		uStat |= I80IF_STAT_NORMAL_CMD_STARTED;
	}

	return uStat;
}

/*===============================================
func: 等待i80空闲
log:
-----------------------------------------------*/
void i80lib_wait_idle(void)
{
	// wait idle
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	while( i80lib_get_status() &
			(I80IF_STAT_TRANSMITTING_NORMAL_CMD |
			 I80IF_STAT_TRANSMITTING_AUTO_CMD |
			 I80IF_STAT_TRANSMITTING_FRAME |
			 I80IF_STAT_NORMAL_CMD_STARTED) );
}


/*===============================================
func: 使能/禁止normal cmd,normal cmd是指只发送cmd list里的命令，不发送数据
log:
-----------------------------------------------*/
void i80lib_transmit_normal_cmd(IM_UINT32 fEn,IM_UINT32 fWaitComplete)	// fEn,是否使能，fWaitComplete 是否等待normal cmd传送结束
{
	IM_UINT32 tmp;	
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if( fEn )
	{
		i80lib_wait_idle();
		idspwl_read_reg(module, I80TRIGCON, &tmp);
		tmp |= (1<<I80TRIGCON_NORMAL_CMD_ST);
		idspwl_write_reg(module, I80TRIGCON, tmp);
		if( fWaitComplete ){
			i80lib_wait_idle();
		}
	}else{
		idspwl_read_reg(module, I80TRIGCON, &tmp);
		tmp &= ~(1<<I80TRIGCON_NORMAL_CMD_ST);
		idspwl_write_reg(module, I80TRIGCON, tmp);
	}
}
/*===============================================
func:
log:
-----------------------------------------------*/
void i80lib_manual_write_once(IM_UINT32 reg_addr,IM_INT32 idsx)
{
	IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	IM_UINT32 offset;
	offset = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_write_reg(module, I80MANWDAT , reg_addr);
	i80if_manual_sig_doe_output();
	i80if_delay_ns(5);

	i80if_manual_sig_wr_low();
	i80if_delay_ns(50);
	i80if_manual_sig_wr_high();
	i80if_delay_ns(50);
}

/*===============================================
func:
log:
-----------------------------------------------*/
void i80lib_manual_read_once(IM_UINT32 *pReg_val,IM_INT32 idsx)
{
	IM_UINT32 module =(idsx == 0)? MODULE_IDS0 : MODULE_IDS1;
	IM_UINT32 offset;
	offset = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	i80if_manual_sig_doe_input();
	i80if_delay_ns(5);

	i80if_manual_sig_rd_low();
	i80if_delay_ns(150);
	idspwl_read_reg(module, I80MANRDAT, pReg_val);
	i80if_manual_sig_rd_high();
	i80if_delay_ns(150);
}

/*===============================================
func: 延时"num"个ns
log:
-----------------------------------------------*/
void i80if_delay_ns( IM_UINT32 num)
{
	volatile IM_UINT32 register i;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	for(i=0; i<num; i++);
}
void i80if_manual_enable(void )
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= 1<<I80MANCON_MAN_EN;
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_disbale(void )
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val &= ~(1<<I80MANCON_MAN_EN);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_rs_low(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON,&val);
	val &= ~(1<<I80MANCON_MAN_RS);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_rs_high(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= (1<<I80MANCON_MAN_RS);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_cs0_low(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON,&val);
	val &= ~(1<<I80MANCON_MAN_CS0);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_cs0_high(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= (1<<I80MANCON_MAN_CS0);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_cs1_low(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val &= ~(1<<I80MANCON_MAN_CS1);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_cs1_high(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= (1<<I80MANCON_MAN_CS1);
	idspwl_write_reg(module, I80MANCON, val);
}

void i80if_manual_sig_doe_output(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= 1<<I80MANCON_MAN_DOE;
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_doe_input(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val &= ~(1<<I80MANCON_MAN_DOE);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_rd_low(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val &= ~(1<<I80MANCON_MAN_RD);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_rd_high(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= (1<<I80MANCON_MAN_RD);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_wr_low(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val &= ~(1<<I80MANCON_MAN_WR);
	idspwl_write_reg(module, I80MANCON, val);
}
void i80if_manual_sig_wr_high(void)
{
	IM_UINT32 val;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	idspwl_read_reg(module, I80MANCON, &val);
	val |= (1<<I80MANCON_MAN_WR);
	idspwl_write_reg(module, I80MANCON, val);
}

