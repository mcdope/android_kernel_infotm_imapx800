/***************************************************************************** 
 ** gt2005.c 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 **     
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **             
 ** Description: sensor config for sensor of ov2655 production 
 **             
 ** Author:     
 **     neville <haixu_fu@infotm.com>
 **      
 ** Revision History: 
 ** ----------------- 
 ** 2.0  11/08/2010 neville   
 *******************************************************************************/

#include "sensor.h"
#include "gt2005_XYC.h"

static struct sensor_ops *ops;
#define cam_gt2005_read(b, c, d) ops->i2c_read(c, b, 2, d)
#define cam_gt2005_write(b) ops->i2c_write(b, 3)

static char gt2005_read_cmos(uint16_t addr)
{
	char buf = 0;
	cam_gt2005_read((uint8_t *)(&addr), &buf, 1);
	return buf;
}

int gt2005_xyc_reset(void __iomem *reg)
{
	uint32_t tmp = 0;

	printk(KERN_INFO "gt2005_xyc_reset\n");
	writel(0x0, reg + IMAP_CIGCTRL);
/*
	tmp = readl(reg+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);
	mdelay(30);
*/
	tmp = readl(reg+IMAP_CIGCTRL);
	tmp &= ~(0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);
	mdelay(30);

	tmp = readl(reg+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);

	return 0;
}


int gt2005_xyc_start(void){
	int i, ret;
	char buf = 0;

	printk(KERN_INFO "gt2005 gt2005_xyc_start++\n");

	cam_gt2005_read((unsigned char *)(&GT2005_VENID0), &buf, 1);
	printk(KERN_ERR "ID1:%x\n",buf);
	buf = 0;
	cam_gt2005_read((unsigned char *)(&GT2005_VENID1), &buf, 1);
	printk(KERN_ERR "ID2:%x\n",buf);
	buf = 0;

	for(i = 0; i < (sizeof(gt2005_init_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_init_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}

	printk(KERN_INFO "gt2005 gt2005_xyc_start--\n");
	return 0;
}

uint32_t gt2005_xyc_get_id(void)
{
	uint32_t cmos_id;

	cmos_id = (gt2005_read_cmos(GT2005_VENID1) << 8) |
		gt2005_read_cmos(GT2005_VENID0);
	return cmos_id;
}

int gt2005_xyc_close(void){
	printk(KERN_INFO "GT2005 gt2005_xyc_close\n");
	return -1;
}

int gt2005_xyc_switch_low_svga(void) {
	printk(KERN_INFO "GT2005 gt2005_xyc_switch_low_svga\n");
	return -1;
}

int gt2005_xyc_switch_high_svga(void) {
	printk(KERN_INFO "GT2005 impx200_cami_high_svga\n");
	return -1;
}

int gt2005_xyc_switch_high_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt2005_xuga_high_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_xuga_high_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
}
int gt2005_xyc_switch_upmid_xuga(void){
#if 0
	int i, ret;

	for(i = 0; i < (sizeof(gt2005_xuga_upmid_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_xuga_upmid_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
    	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_switch_mid_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt2005_xuga_mid_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_xuga_mid_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
}
int gt2005_xyc_switch_low_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt2005_xuga_low_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_xuga_low_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
}

int gt2005_xyc_svga_to_xuga(void){
#if 1
	uint16_t shutter = 0;		
	uint16_t AGain_shutter = 0;
	uint16_t DGain_shutter = 0;

	uint16_t reg0x0012 = 0;
	uint16_t reg0x0013 = 0;
	uint16_t reg0x0014 = 0;
	uint16_t reg0x0015 = 0;
	uint16_t reg0x0016 = 0;
	uint16_t reg0x0017 = 0;

	char reg0x0304 = 0;
	char reg0x0305 = 0;
	char reg0x0306 = 0;
	char reg0x0307 = 0;
	char reg0x0308 = 0;


	struct gt2005_regval_list gt2005_0300[] = {
		{0x03, 0x00, 0x00},
	};

	struct gt2005_regval_list gt2005_0304[] = {
		{0x03, 0x04, 0x00},
	};

	struct gt2005_regval_list gt2005_0305[] = {
		{0x03, 0x05, 0x00},
	};

	struct gt2005_regval_list gt2005_0306[] = {
		{0x03, 0x06, 0x00},
	};

	struct gt2005_regval_list gt2005_0307[] = {
		{0x03, 0x07, 0x00},
	};

	struct gt2005_regval_list gt2005_0308[] = {
		{0x03, 0x08, 0x00},
	};

	/*****/
	gt2005_0300->value = 0xc1;
	cam_gt2005_write((unsigned char *)(gt2005_0300));  

	cam_gt2005_read((unsigned char *)(&GT2005_0012), (uint8_t *)&reg0x0012, 1);   	
	cam_gt2005_read((unsigned char *)(&GT2005_0013), (uint8_t *)&reg0x0013, 1);

	shutter = ((reg0x0012 << 8) |reg0x0013);
	
	cam_gt2005_read((unsigned char *)(&GT2005_0014), (uint8_t *)&reg0x0014, 1);
	cam_gt2005_read((unsigned char *)(&GT2005_0015), (uint8_t *)&reg0x0015, 1);

	AGain_shutter = ((reg0x0014 << 8) | reg0x0015);
	
	cam_gt2005_read((unsigned char *)(&GT2005_0016), (uint8_t *)&reg0x0016, 1);
	cam_gt2005_read((unsigned char *)(&GT2005_0017), (uint8_t *)&reg0x0017, 1);

	DGain_shutter = ((reg0x0016 << 8) | reg0x0017);
/*
 *	Close ALC
 */
	gt2005_0300->value = 0x41;
	cam_gt2005_write((unsigned char *)(gt2005_0300));  

	shutter = shutter / 2;
	
	reg0x0305 = shutter & 0xff ;
	gt2005_0305->value = reg0x0305;
	cam_gt2005_write((unsigned char *)(gt2005_0305));  
		
	reg0x0304 = (shutter >> 8) & 0xff ;
	gt2005_0304->value = reg0x0304;
	cam_gt2005_write((unsigned char *)(gt2005_0304));  

	reg0x0307 = AGain_shutter & 0xff;
	gt2005_0307->value = reg0x0307;
	cam_gt2005_write((unsigned char *)(gt2005_0307));

	reg0x0306 = (AGain_shutter >> 8) & 0xff;
	gt2005_0306->value = reg0x0306;
	cam_gt2005_write((unsigned char *)(gt2005_0306));

	reg0x0308 = DGain_shutter & 0xff;
	gt2005_0308->value = reg0x0308;
	cam_gt2005_write((unsigned char *)(gt2005_0308));

       	return 0;
#else
	return -1;
#endif
}

int gt2005_xyc_to_preview_320_240(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt2005_to_preview_320_240_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_to_preview_320_240_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0;
}
int gt2005_xyc_to_preview_640_480(void){ 
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_to_preview_640_480_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_to_preview_640_480_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0;
}	
int gt2005_xyc_sepia(void){ 
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_sepia_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_sepia_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
}
int gt2005_xyc_bluish(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_bluish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_bluish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_greenish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_greenish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_greenish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_reddish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_reddish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_reddish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_yellowish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_yellowish_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_yellowish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_bandw(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_bandw_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_bandw_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_negative(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_negative_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_negative_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_normal(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_normal_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_normal_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_auto(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_auto_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_auto_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_sunny(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_sunny_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_sunny_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_cloudy(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_cloudy_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_cloudy_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_office(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_office_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_office_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}

int gt2005_xyc_home(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt2005_home_regs) / 3); i++)
	{
		ret = cam_gt2005_write((unsigned char *)(&gt2005_normal_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
	}
	return 0; 
#else
	return -1;
#endif
}
int gt2005_xyc_saturation_0(void){ return -1; }
int gt2005_xyc_saturation_1(void){ return -1; }
int gt2005_xyc_saturation_2(void){ return -1; }
int gt2005_xyc_saturation_3(void){ return -1; }
int gt2005_xyc_saturation_4(void){ return -1; }
int gt2005_xyc_brightness_0(void){ return -1; }
int gt2005_xyc_brightness_1(void){ return -1; }
int gt2005_xyc_brightness_2(void){ return -1; }
int gt2005_xyc_brightness_3(void){ return -1; }
int gt2005_xyc_brightness_4(void){ return -1; }
int gt2005_xyc_brightness_5(void){ return -1; }
int gt2005_xyc_brightness_6(void){ return -1; }
int gt2005_xyc_contrast_0(void){ return -1; }
int gt2005_xyc_contrast_1(void){ return -1; }
int gt2005_xyc_contrast_2(void){ return -1; }
int gt2005_xyc_contrast_3(void){ return -1; }
int gt2005_xyc_contrast_4(void){ return -1; }
int gt2005_xyc_contrast_5(void){ return -1; }
int gt2005_xyc_contrast_6(void){ return -1; }
int gt2005_xyc_sharpness_0(void){ return -1; }
int gt2005_xyc_sharpness_1(void){ return -1; }
int gt2005_xyc_sharpness_2(void){ return -1; }
int gt2005_xyc_sharpness_3(void){ return -1; }
int gt2005_xyc_sharpness_4(void){ return -1; }
int gt2005_xyc_sharpness_auto(void){ return -1; }
int gt2005_xyc_night_mode_on(void){ return -1; }
int gt2005_xyc_night_mode_off(void){ return -1; }



int gt2005_xyc_set_mode(int value, void __iomem *reg){
	int ret;
	uint32_t tmp = 0;	

	printk(KERN_INFO "gt2005 set mode %d\n", value);
	switch(value)
	{
		case INIT_SENSOR:
			printk(KERN_INFO "INIT %d\n", value);
			tmp  = readl(reg+IMAP_CIGCTRL);		
			tmp |= (0x1 << 4);
			writel(tmp, reg+IMAP_CIGCTRL); 
			mdelay(50);
			ret = gt2005_xyc_start();
			break;
		case SWITCH_SENSOR_TO_HIGH_SVGA:              
			ret = gt2005_xyc_switch_high_svga();
			break;                                
		case SWITCH_SENSOR_TO_LOW_SVGA:              
			ret = gt2005_xyc_switch_low_svga();
			break;                               
		case SWITCH_SENSOR_TO_HIGH_XUGA:              
			ret = gt2005_xyc_svga_to_xuga();    
			ret = gt2005_xyc_switch_high_xuga();
			break;                                
		case SWITCH_SENSOR_TO_MID_XUGA:                  
			//ret = gt2005_xyc_svga_to_xuga();       
			ret = gt2005_xyc_switch_mid_xuga();    
			break;                                   

		case SWITCH_SENSOR_TO_LOW_XUGA:                  
			ret = gt2005_xyc_switch_low_xuga();    
			break;                                   

		case SENSOR_TO_HIGH_PREVIEW:                     
			ret = gt2005_xyc_to_preview_640_480(); 
			break;                                   

		case SENSOR_TO_LOW_PREVIEW:                      
			ret = gt2005_xyc_to_preview_320_240(); 
			break;                                   
		case CLOSE_SENSOR:                               
			ret = gt2005_xyc_close();              
			break;                                   
		default:                                         
			ret = -1;                                

	}

	return ret;
}

int gt2005_xyc_set_effect(int value)
{

	switch(value)                            
	{                                        
		case SENSOR_EFFECT_OFF:          
			gt2005_xyc_normal();   
			break;                   
		case SENSOR_EFFECT_MONO:         
			gt2005_xyc_bandw();    
			break;                   
		case SENSOR_EFFECT_NEGATIVE:     
			gt2005_xyc_negative(); 
			break;                   
		case SENSOR_EFFECT_SOLARIZE:     
			gt2005_xyc_yellowish();
			break;                   
		case SENSOR_EFFECT_PASTEL:       
			gt2005_xyc_reddish();  
			break;                   
		case SENSOR_EFFECT_MOSAIC:       
			break;                   
		case SENSOR_EFFECT_RESIZE:       
			break;                   
		case SENSOR_EFFECT_SEPIA:        
			gt2005_xyc_sepia();    
			break;                   
		case SENSOR_EFFECT_POSTERIZE:    
			gt2005_xyc_bluish();   
			break;                   
		case SENSOR_EFFECT_WHITEBOARD:   
			break;                   
		case SENSOR_EFFECT_BLACKBOARD:   
			break;                   
		case SNESOR_EFFECT_AQUA:         
			gt2005_xyc_greenish(); 
			break;                   
		default:                         
			break;                   
	}                                        

	return 0;
}


int gt2005_xyc_set_wb(int value)
{
	switch(value)
	{
		case SENSOR_WB_AUTO:
			gt2005_xyc_auto();
			break;
		case SENSOR_WB_INCANDESCENT:
			gt2005_xyc_home();
			break;
		case SENSOR_WB_FLUORESCENT:
			gt2005_xyc_office();
			break;
		case SENSOR_WB_DAYLIGHT:
			gt2005_xyc_sunny();
			break;
		case SENSOR_WB_CLOUDY:
			gt2005_xyc_cloudy();
			break;
		case SENSOR_WB_TWILIGHT:
			break;
		case SENSOR_WB_SHADE:
			break;
		default:
			break;
	}

	return 0;
}



int gt2005_xyc_set_antibanding(void){
	return -1;
} 

int gt2005_xyc_set_brightness(void) {
	return -1;
}

int gt2005_xyc_set_nightmode(void) {
	return -1;
}

static uint32_t cmos_id_list[] = { 0x3851, 0};

/* FPS api added by Sololz. */
static struct res_to_fps_map gt2005_rf_maps[3] = {
	{
		.res = DWL_CAM_RES_UXGA,
		.fps = DWL_CAM_FPS_18,
	}, {
		.res = DWL_CAM_RES_VGA,
		.fps = DWL_CAM_FPS_25,
	}, {
		.res = DWL_CAM_RES_QVGA,
		.fps = DWL_CAM_FPS_30,
	}
};
static uint32_t gt2005_get_fps(uint32_t res)
{
	int i = 0;
	uint32_t fps = 0;

	if (res == 0) {
		return DWL_CAM_FPS_22;
	}

	for ( ; i < (sizeof(gt2005_rf_maps) / sizeof(gt2005_rf_maps[0])); i++) {
		if (gt2005_rf_maps[i].res == res) {
			fps = gt2005_rf_maps[i].fps;
			break;
		}
	}

	return fps;
}

struct sensor_ops gt2005_xyc_ops = {
	.s_res			= DWL_CAM_RES_UXGA | 
		DWL_CAM_RES_VGA | DWL_CAM_RES_QVGA,
	.get_fps		= gt2005_get_fps,

	/* Light mode. */
	.s_lmode		= DWL_CAM_LIGHT_MODE_AUTO |
		DWL_CAM_LIGHT_MODE_INCANDESCENT | 
		DWL_CAM_LIGHT_MODE_FLUORESCENT |
		DWL_CAM_LIGHT_MODE_DAYLIGHT |
		DWL_CAM_LIGHT_MODE_CLOUDY_DAYLIGHT,

	/* Special mode. */
	.s_seffect		= DWL_CAM_SPECIAL_EFFECT_NONE | 
		DWL_CAM_SPECIAL_EFFECT_MONO |
		DWL_CAM_SPECIAL_EFFECT_NEGATIVE |
		DWL_CAM_SPECIAL_EFFECT_SOLARIZE |
		DWL_CAM_SPECIAL_EFFECT_PASTEL |
		DWL_CAM_SPECIAL_EFFECT_SEPIA |
		DWL_CAM_SPECIAL_EFFECT_POSTERIZE |
		DWL_CAM_SPECIAL_EFFECT_AQUA,

	.reset			= gt2005_xyc_reset,
	.set_mode		= gt2005_xyc_set_mode,
	.set_effect		= gt2005_xyc_set_effect,
	.set_wb			= gt2005_xyc_set_wb,
	.get_id			= gt2005_xyc_get_id,
	.idlist			= cmos_id_list,
	.addr			= GT2005_I2C_WADDR,
	.name			= "xyc:gt2005",
	.hwid			= 0,
};

static int __init gt2005_xyc_init(void)
{
	printk(KERN_INFO "gt2005xyc-ops-register");
	ops = &gt2005_xyc_ops;
	return imapx200_cam_sensor_register(ops);
}
static void __exit gt2005_xyc_exit(void)
{
	printk(KERN_INFO "gt2005xyc-ops-unregister");
	imapx200_cam_sensor_unregister(&gt2005_xyc_ops);
}

module_init(gt2005_xyc_init);
module_exit(gt2005_xyc_exit);

