/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of i80 library register manipulation
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/

#include <InfotmMedia.h>
#include <ids_pwl.h>
#include <i80.h>
#include <i80_reg.h>

static IM_UINT32 gModule;
/*===============================================
func: 延时"num"个ns
log:
-----------------------------------------------*/
void
i80reg_delay_ns(
		IM_UINT32 num
		)
{
	volatile IM_UINT32 register i;

	for(i=0; i<num; i++);
}

void i80reg_set_idsxNo(IM_UINT32 idsx)
{
	gModule = (idsx == 0) ?  MODULE_IDS0: MODULE_IDS1;
}

void i80if_manual_sig_init(void)
{
	idspwl_write_reg(gModule, I80MANCON, 0x3FF);
}
void i80if_manual_sig_deinit(void)
{
	idspwl_write_reg(gModule, I80MANCON, 0x00);
}
void i80if_manual_sig_doe_output(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= 1<<I80IF_MANCON_SIG_DOE;
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_doe_input(void )
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val &= ~(1<<I80IF_MANCON_SIG_DOE);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_enable(void )
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= 1<<I80IF_MANCON_EN;
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_disbale(void )
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val &= ~(1<<I80IF_MANCON_EN);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_rs_low(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON,&val);
	val &= ~(1<<I80IF_MANCON_SIG_RS);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_rs_high(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= (1<<I80IF_MANCON_SIG_RS);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_cs0_low(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON,&val);
	val &= ~(1<<I80IF_MANCON_SIG_CS0);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_cs0_high(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= (1<<I80IF_MANCON_SIG_CS0);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_cs1_low(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val &= ~(1<<I80IF_MANCON_SIG_CS1);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_cs1_high(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= (1<<I80IF_MANCON_SIG_CS1);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_rd_low(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val &= ~(1<<I80IF_MANCON_SIG_RD);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_rd_high(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= (1<<I80IF_MANCON_SIG_RD);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_wr_low(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val &= ~(1<<I80IF_MANCON_SIG_WR);
	idspwl_write_reg(gModule, I80MANCON, val);
}
void i80if_manual_sig_wr_high(void)
{
	IM_UINT32 val;
	idspwl_read_reg(gModule, I80MANCON, &val);
	val |= (1<<I80IF_MANCON_SIG_WR);
	idspwl_write_reg(gModule, I80MANCON, val);
}


/*===============================================
func: 初始化i80寄存器，并不使能i80模块
log:
-----------------------------------------------*/
void i80reg_init(
		struct_i80if *pI80if
		)
{
	IM_UINT32 tmp, offset;
	IM_INT32 i;

	offset = 0;
	//--------------------------------------
	tmp = 0;

	tmp |= ( pI80if->eData_format << I80IF_IFCON1_DATA_FORMAT );
	if( pI80if->stAuto_cmd_property.fAuto_cmd_disable )	tmp |= ( 1 << I80IF_IFCON1_DISAUTOCMD );
	tmp |= ( pI80if->stAuto_cmd_property.eAuto_cmd_rate << I80IF_IFCON1_AUTOCMDRATE );

	idspwl_write_reg(gModule, I80IFCON1, tmp);

	//--------------------------------------
	for( i=0; i<I80IF_CMD_FIFO_NUM; i++ ){
		i80reg_set_cmd_list(&pI80if->stCmd[i], i);
	}

	//--------------------------------------
	idspwl_write_reg(gModule, I80MANCON , 0);

	//--------------------------------------
	tmp = 0;

	if( pI80if->stPort_property.eSignal_rd == I80IF_SIGNAL_ACTIVE_HIGH )		tmp |= ( 1 << I80IF_IFCON0_RDPOL );
	if( pI80if->stPort_property.eSignal_wr == I80IF_SIGNAL_ACTIVE_HIGH )		tmp |= ( 1 << I80IF_IFCON0_WRPOL );
	if( pI80if->stPort_property.eSignal_rs == I80IF_SIGNAL_ACTIVE_HIGH )		tmp |= ( 1 << I80IF_IFCON0_RSPOL );
	if( pI80if->stPort_property.eSignal_cs1 == I80IF_SIGNAL_ACTIVE_HIGH )	tmp |= ( 1 << I80IF_IFCON0_CS1POL );
	if( pI80if->stPort_property.eSignal_cs0 == I80IF_SIGNAL_ACTIVE_HIGH )	tmp |= ( 1 << I80IF_IFCON0_CS0POL );
	tmp |= ( pI80if->stPort_property.nCycles_cs_setup << I80IF_IFCON0_PARAM_CS_SETUP );
	tmp |= ( pI80if->stPort_property.nCycles_wr_setup << I80IF_IFCON0_PARAM_WR_SETUP );
	tmp |= ( pI80if->stPort_property.nCycles_wr_active << I80IF_IFCON0_PARAM_WR_ACTIVE );
	tmp |= ( pI80if->stPort_property.nCycles_wr_hold << I80IF_IFCON0_PARAM_WR_HOLD );
	if( pI80if->fNormal_cmd_over_intr_en )	tmp |= ( 1 << I80IF_IFCON0_NORMAL_CMD_INTR_EN );
	if( pI80if->fFrame_over_intr_en )		tmp |= ( 1 << I80IF_IFCON0_FRAME_END_INTR_EN );
	if( pI80if->eMain_lcd == I80IF_LCD_1 )	tmp |= ( 1 << I80IF_IFCON0_MAINLCD_CFG );

	idspwl_write_reg(gModule, I80IFCON0, tmp);

	//--------------------------------------
	idspwl_write_reg(gModule, LCDCON1, (5 << LCDCON1_CLKVAL) | (0<<LCDCON1_VMMODE) | (3<<LCDCON1_PNRMODE) |
			(12<<LCDCON1_STNBPP) | (0<<LCDCON1_ENVID));
	idspwl_write_reg(gModule, LCDCON2, (100 <<LCDCON2_VBPD) | (2<<LCDCON2_VFPD));
	idspwl_write_reg(gModule, LCDCON3, (2<<LCDCON3_VSPW) | (96<<LCDCON3_HSPW));
	idspwl_write_reg(gModule, LCDCON4 , (48<<LCDCON4_HBPD) | (16<<LCDCON4_HFPD));
	idspwl_write_reg(gModule, LCDCON5, (6<<LCDCON5_RGBORDER) | (1<<LCDCON5_DSPTYPE) |
			(0<<LCDCON5_INVVCLK) | (0<<LCDCON5_INVVLINE) |
			(0<<LCDCON5_INVVFRAME) | (0<<LCDCON5_INVVD) |
			(0<<LCDCON5_INVVDEN) | (0<< LCDCON5_INVPWREN) |
			(0<<LCDCON5_PWREN));
	idspwl_write_reg(gModule, LCDCON6, ((pI80if->screen_height - 1) << LCDCON6_LINEVAL) | ((pI80if->screen_width - 1) << LCDCON6_HOZVAL));

}

/*===============================================
func: 使能或禁止i80模块
log:
-----------------------------------------------*/
void i80reg_if_enable(
		IM_UINT32 fEn	// true--使能, 0--禁止
		)
{
	IM_UINT32 tmp, offset;
	offset = 0;
	if(fEn){
		idspwl_read_reg(gModule, I80IFCON0,&tmp);
		tmp |= (1<<I80IF_IFCON0_EN);
		idspwl_write_reg(gModule, I80IFCON0, tmp);
	}else{
		idspwl_read_reg(gModule, I80IFCON0, &tmp);
		tmp &= ~(1<<I80IF_IFCON0_EN);
		idspwl_write_reg(gModule, I80IFCON0, tmp);
	}
}

/*===============================================
func: 使能或禁止lcd
log:
-----------------------------------------------*/
void i80reg_lcd_enable(IM_UINT32 fEn)
{
	IM_UINT32 tmp, offset;
	offset = 0;
	if( fEn ){
		idspwl_read_reg(gModule, LCDCON1, &tmp);
		tmp |= 0x01;
		idspwl_write_reg(gModule, LCDCON1, tmp);
	}else{
		idspwl_read_reg(gModule, LCDCON1,&tmp);
		tmp &= ~(0x01);
		idspwl_write_reg(gModule, LCDCON1, tmp);
	}
}

/*===============================================
func: 返回当前i80模块的状态，返回值在i80_api.h里定义的
log:
-----------------------------------------------*/
IM_UINT32 i80reg_get_status(void)
{
	IM_UINT32 val, uStat;
	IM_UINT32 offset;
	offset = 0;

	uStat = 0;
	idspwl_read_reg(gModule, I80TRIGCON, &val);

	if( val & (1<<I80IF_TRIGCON_TR_VIDEO_OVER) ){
		uStat |= I80IF_STAT_TRANSMIT_FRAME_END;
	}
	if( val & (1<<I80IF_TRIGCON_TR_NORMALCMD) ){
		uStat |= I80IF_STAT_TRANSMITTING_NORMAL_CMD;
	}
	if( val & (1<<I80IF_TRIGCON_TR_AUTOCMD) ){
		uStat |= I80IF_STAT_TRANSMITTING_AUTO_CMD;
	}
	if( val & (1<<I80IF_TRIGCON_TR_DATA) ){
		uStat |= I80IF_STAT_TRANSMITTING_FRAME;
	}
	if( val & (1<<I80IF_TRIGCON_NORMAL_CMD_ST) ){
		uStat |= I80IF_STAT_NORMAL_CMD_STARTED;
	}

	return uStat;
}

/*===============================================
func: 配置cmd列表
log:
-----------------------------------------------*/
void i80reg_set_cmd_list(
		struct_i80if_cmd_property *pCmd,
		IM_INT32 nIndex 	// 在16个cmd fifo中的索引,0-15
		)
{
	IM_UINT32 tmp, offset;
	offset = 0;

	idspwl_read_reg(gModule, I80CMDCON0, &tmp);
	tmp &= ~(0x3 << (nIndex << 1));
	tmp |= (pCmd->eCmd_type << (nIndex << 1));
	idspwl_write_reg(gModule, I80CMDCON0, tmp);

	idspwl_read_reg(gModule, I80CMDCON1, &tmp);
	tmp &= ~(0x3 << (nIndex << 1));
	tmp |= (pCmd->eRS_level << (nIndex << 1));
	idspwl_write_reg(gModule, I80CMDCON1, tmp);

	switch( nIndex )
	{
	case 0:		idspwl_write_reg(gModule, I80CMD00, pCmd->uCmd);	break;
	case 1:		idspwl_write_reg(gModule, I80CMD01, pCmd->uCmd);	break;
	case 2:		idspwl_write_reg(gModule, I80CMD02, pCmd->uCmd);	break;
	case 3:		idspwl_write_reg(gModule, I80CMD03, pCmd->uCmd);	break;
	case 4:		idspwl_write_reg(gModule, I80CMD04, pCmd->uCmd);	break;
	case 5:		idspwl_write_reg(gModule, I80CMD05, pCmd->uCmd);	break;
	case 6:		idspwl_write_reg(gModule, I80CMD06, pCmd->uCmd);	break;
	case 7:		idspwl_write_reg(gModule, I80CMD07, pCmd->uCmd);	break;
	case 8:		idspwl_write_reg(gModule, I80CMD08, pCmd->uCmd);	break;
	case 9:		idspwl_write_reg(gModule, I80CMD09, pCmd->uCmd);	break;
	case 10:	idspwl_write_reg(gModule, I80CMD10, pCmd->uCmd);	break;
	case 11:	idspwl_write_reg(gModule, I80CMD11, pCmd->uCmd);	break;
	case 12:	idspwl_write_reg(gModule, I80CMD12, pCmd->uCmd);	break;
	case 13:	idspwl_write_reg(gModule, I80CMD13, pCmd->uCmd);	break;
	case 14:	idspwl_write_reg(gModule, I80CMD14, pCmd->uCmd);	break;
	case 15:	idspwl_write_reg(gModule, I80CMD15, pCmd->uCmd);	break;

	default:
		break;
	}
}

/*===============================================
func: 清除所有cmd列表
log:
-----------------------------------------------*/
void i80reg_clear_cmd_list(void)
{
	IM_UINT32 offset;
	offset = 0;
	idspwl_write_reg(gModule, I80CMDCON0, 0x00);
	idspwl_write_reg(gModule, I80CMDCON1, 0x00);
	idspwl_write_reg(gModule, I80CMD00, 0x00);
	idspwl_write_reg(gModule, I80CMD01, 0x00);
	idspwl_write_reg(gModule, I80CMD02, 0x00);
	idspwl_write_reg(gModule, I80CMD03, 0x00);
	idspwl_write_reg(gModule, I80CMD04, 0x00);
	idspwl_write_reg(gModule, I80CMD05, 0x00);
	idspwl_write_reg(gModule, I80CMD06, 0x00);
	idspwl_write_reg(gModule, I80CMD07, 0x00);
	idspwl_write_reg(gModule, I80CMD08, 0x00);
	idspwl_write_reg(gModule, I80CMD09, 0x00);
	idspwl_write_reg(gModule, I80CMD10, 0x00);
	idspwl_write_reg(gModule, I80CMD11, 0x00);
	idspwl_write_reg(gModule, I80CMD12, 0x00);
	idspwl_write_reg(gModule, I80CMD13, 0x00);
	idspwl_write_reg(gModule, I80CMD14, 0x00);
	idspwl_write_reg(gModule, I80CMD15, 0x00);
}

/*===============================================
func: 使能/禁止normal cmd传送，normal cmd是指只发送cmd list里的命令，不发送数据
log:
-----------------------------------------------*/
void i80reg_transmit_normal_cmd(
		IM_UINT32 fEn	// true--使能, 0--禁止
		)
{
	IM_UINT32 offset,tmp;
	offset = 0;
	if( fEn ){
		idspwl_read_reg(gModule, I80TRIGCON, &tmp);
		tmp |= (1<<I80IF_TRIGCON_NORMAL_CMD_ST);
		idspwl_write_reg(gModule, I80TRIGCON, tmp);
	}else{
		idspwl_read_reg(gModule, I80TRIGCON, &tmp);
		tmp &= ~(1<<I80IF_TRIGCON_NORMAL_CMD_ST);
		idspwl_write_reg(gModule, I80TRIGCON, tmp);
	}
}

/*===============================================
func: 设置CS0或CS1为当前的有效控制CS
log:
-----------------------------------------------*/
void i80reg_set_current_lcd(
		enum_i80if_supports_lcd eLcd
		)
{
	IM_UINT32 offset,tmp;
	offset = 0;
	if( eLcd == I80IF_LCD_0 ){
		idspwl_read_reg(gModule, I80IFCON0, &tmp );
		tmp &= ~(1<<I80IF_IFCON0_MAINLCD_CFG);
		idspwl_write_reg(gModule, I80IFCON0 , tmp);
	}else{
		idspwl_read_reg(gModule, I80IFCON0, &tmp );
		tmp |= (1<<I80IF_IFCON0_MAINLCD_CFG);
		idspwl_write_reg(gModule, I80IFCON0 , tmp);
	}
}

/*===============================================
func:
log:
-----------------------------------------------*/
void i80reg_manual_write_once(
		IM_UINT32 reg_addr
		)
{
	IM_UINT32 offset;
	offset = 0;
	idspwl_write_reg(gModule, I80MANWDAT , reg_addr);
	i80if_manual_sig_doe_output();
	i80reg_delay_ns(5);

	i80if_manual_sig_wr_low();
	i80reg_delay_ns(50);
	i80if_manual_sig_wr_high();
	i80reg_delay_ns(50);
}

/*===============================================
func:
log:
-----------------------------------------------*/
void i80reg_manual_read_once(
		IM_UINT32 *pReg_val
		)
{
	IM_UINT32 offset;
	offset = 0;
	i80if_manual_sig_doe_input();
	i80reg_delay_ns(5);

	i80if_manual_sig_rd_low();
	i80reg_delay_ns(150);
	idspwl_read_reg(gModule, I80MANRDAT, pReg_val);
	i80if_manual_sig_rd_high();
	i80reg_delay_ns(150);
}

/*===============================================
func:
log:
-----------------------------------------------*/
void i80reg_set_data_format(
		enum_i80if_data_format fmt
		)
{
	IM_UINT32 offset,tmp;
	offset = 0;
	idspwl_read_reg(gModule, I80IFCON1, &tmp);
	tmp &= ~(0xff << I80IF_IFCON1_DATA_FORMAT);
	tmp |= (fmt << I80IF_IFCON1_DATA_FORMAT);
	idspwl_write_reg(gModule, I80IFCON1,tmp);
}


