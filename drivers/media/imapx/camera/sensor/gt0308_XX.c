/***************************************************************************** 
 ** gt0308.c 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 **     
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **             
 ** Description: sensor config for sensor of gt0308 production 
 **             
 ** Author:     
 **     ice <hupei@szchallenge.com>
 **      
 ** Revision History: 
 ** ­­­­­­­­­­­­­­­­­ 
 ** 1.0  13/05/2011 ice
 *******************************************************************************/

#include "sensor.h"
#include "gt0308_XX.h"

static struct sensor_ops *ops;
#define cam_gt0308_read(b, c, d) ops->i2c_read(c, b, 1, d)
#define cam_gt0308_write(b) ops->i2c_write(b, 2)

static char gt0308_read_cmos(uint16_t addr)
{
	char buf = 0;
	cam_gt0308_read((unsigned char *)(&addr), &buf, 1);
	return buf;
}
int gt0308_xx_reset(void __iomem *reg)
{
	uint32_t tmp = 0;

	printk(KERN_INFO "gt0308_xx_reset\n");
	writel(0x0, reg + IMAP_CIGCTRL);
/*
	tmp = readl(param->ioaddr+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, param->ioaddr+IMAP_CIGCTRL);
	mdelay(30);
*/
	tmp = readl(reg+IMAP_CIGCTRL);
	tmp &= ~(0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);
	msleep(30);

	tmp = readl(reg+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);

	return 0;
}


int gt0308_xx_start(void){
	int i, ret;
	char buf = 0;

	printk(KERN_INFO "gt0308_xx_start++\n");

	cam_gt0308_read((unsigned char *)(&GT0308_VENID0), &buf, 1);
	printk(KERN_ERR "ID1:%x\n",buf);
	buf = 0;

	for(i = 0; i < ((sizeof(gt0308_init_regs) / 2) -1); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_init_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
		SENSOR_DELAY();
	}

	printk(KERN_INFO "gt0308 gt0308_xx_start--\n");
	return 0;
}

uint32_t gt0308_xx_get_id(void)
{
	uint32_t cmos_id;

	cmos_id = (gt0308_read_cmos(GT0308_VENID0));

	return cmos_id;
}

int gt0308_xx_close(void){
	printk(KERN_INFO "gt0308 gt0308_xx_close\n");
	return -1;
}

int gt0308_xx_switch_low_svga(void) {
	printk(KERN_INFO "gt0308 gt0308_xx_switch_low_svga\n");
	return -1;
}

int gt0308_xx_switch_high_svga(void) {
	printk(KERN_INFO "gt0308 impx200_cami_high_svga\n");
	return -1;
}

int gt0308_xx_switch_high_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt0308_xuga_high_regs) / 3); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_xuga_high_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
}
int gt0308_xx_switch_upmid_xuga(void){
#if 0
	int i, ret;

	for(i = 0; i < (sizeof(gt0308_xuga_upmid_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_xuga_upmid_regs[i]));
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
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
int gt0308_xx_switch_mid_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt0308_xuga_mid_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_xuga_mid_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
}
int gt0308_xx_switch_low_xuga(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt0308_xuga_low_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_xuga_low_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
}

int gt0308_xx_svga_to_xuga(void){
	return -1;
}

int gt0308_xx_to_preview_320_240(void){ 
	int i, ret;

	for(i = 0; i < (sizeof(gt0308_to_preview_320_240_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_to_preview_320_240_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0;
}
int gt0308_xx_to_preview_640_480(void){ 
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_to_preview_640_480_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_to_preview_640_480_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0;
}	
int gt0308_xx_sepia(void){ 
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_sepia_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_sepia_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
}
int gt0308_xx_bluish(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_bluish_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_bluish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_greenish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_greenish_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_greenish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_reddish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_reddish_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_reddish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_yellowish(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_yellowish_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_yellowish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_bandw(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_bandw_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_bandw_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_negative(void){
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_negative_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_negative_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_normal(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_normal_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_normal_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_auto(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_auto_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_auto_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_sunny(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_sunny_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_sunny_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_cloudy(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_cloudy_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_cloudy_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_office(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_office_regs) / 2); i++)
	{
		ret = cam_gt0308_write( (unsigned char *)(&gt0308_office_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}

int gt0308_xx_home(void){ 
#if 1
	int i, ret;
	for(i = 0; i < (sizeof(gt0308_home_regs) / 2); i++)
	{
		ret = cam_gt0308_write((unsigned char *)(&gt0308_home_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else
		{
		}
		SENSOR_DELAY();
	}
	return 0; 
#else
	return -1;
#endif
}
int gt0308_xx_saturation_0(void){ return -1; }
int gt0308_xx_saturation_1(void){ return -1; }
int gt0308_xx_saturation_2(void){ return -1; }
int gt0308_xx_saturation_3(void){ return -1; }
int gt0308_xx_saturation_4(void){ return -1; }
int gt0308_xx_brightness_0(void){ return -1; }
int gt0308_xx_brightness_1(void){ return -1; }
int gt0308_xx_brightness_2(void){ return -1; }
int gt0308_xx_brightness_3(void){ return -1; }
int gt0308_xx_brightness_4(void){ return -1; }
int gt0308_xx_brightness_5(void){ return -1; }
int gt0308_xx_brightness_6(void){ return -1; }
int gt0308_xx_contrast_0(void){ return -1; }
int gt0308_xx_contrast_1(void){ return -1; }
int gt0308_xx_contrast_2(void){ return -1; }
int gt0308_xx_contrast_3(void){ return -1; }
int gt0308_xx_contrast_4(void){ return -1; }
int gt0308_xx_contrast_5(void){ return -1; }
int gt0308_xx_contrast_6(void){ return -1; }
int gt0308_xx_sharpness_0(void){ return -1; }
int gt0308_xx_sharpness_1(void){ return -1; }
int gt0308_xx_sharpness_2(void){ return -1; }
int gt0308_xx_sharpness_3(void){ return -1; }
int gt0308_xx_sharpness_4(void){ return -1; }
int gt0308_xx_sharpness_auto(void){ return -1; }
int gt0308_xx_night_mode_on(void){ return -1; }
int gt0308_xx_night_mode_off(void){ return -1; }



int gt0308_xx_set_mode(int value,void __iomem *reg){
	int ret;
	uint32_t tmp = 0;	

	printk(KERN_INFO "gt0308 set mode %d\n", value);
	switch(value)
	{
		case INIT_SENSOR:
			printk(KERN_INFO "INIT_SENSOR %d\n", value);
			tmp  = readl(reg+IMAP_CIGCTRL);		
			tmp |= (0x1 << 4);
			writel(tmp, reg+IMAP_CIGCTRL); 
			msleep(50);
			ret = gt0308_xx_start();
			break;
		case SWITCH_SENSOR_TO_HIGH_SVGA:   
			printk(KERN_INFO "SWITCH_SENSOR_TO_HIGH_SVGA %d\n", value);           
			ret = gt0308_xx_switch_high_svga();
			break;                                
		case SWITCH_SENSOR_TO_LOW_SVGA:  
			printk(KERN_INFO "SWITCH_SENSOR_TO_LOW_SVGA %d\n", value);             
			ret = gt0308_xx_switch_low_svga();
			break;                               
		case SWITCH_SENSOR_TO_HIGH_XUGA: 
			printk(KERN_INFO "SWITCH_SENSOR_TO_HIGH_XUGA %d\n", value);              
			ret = gt0308_xx_svga_to_xuga();    
			ret = gt0308_xx_switch_high_xuga();
			break;                                
		case SWITCH_SENSOR_TO_MID_XUGA:   
			printk(KERN_INFO "SWITCH_SENSOR_TO_MID_XUGA %d\n", value);                
			//ret = gt0308_xx_svga_to_xuga();       
			ret = gt0308_xx_switch_mid_xuga();    
			break;                                   

		case SWITCH_SENSOR_TO_LOW_XUGA:  
			printk(KERN_INFO "SWITCH_SENSOR_TO_LOW_XUGA %d\n", value);                 
			ret = gt0308_xx_switch_low_xuga();    
			break;                                   

		case SENSOR_TO_HIGH_PREVIEW:      
			printk(KERN_INFO "SENSOR_TO_HIGH_PREVIEW %d\n", value);                
			ret = gt0308_xx_to_preview_640_480(); 
			break;                                   

		case SENSOR_TO_LOW_PREVIEW:  
			printk(KERN_INFO "SENSOR_TO_LOW_PREVIEW %d\n", value);                     
			ret = gt0308_xx_to_preview_320_240(); 
			break;                                   
		case CLOSE_SENSOR:     
			printk(KERN_INFO "SENSOR_TO_LOW_PREVIEW %d\n", value);                           
			ret = gt0308_xx_close();              
			break;                                   
		default:                                         
			ret = -1;                                

	}

	return ret;
}

int gt0308_xx_set_effect(int value)
{
		printk(KERN_INFO "gt0308_xx_set_effect %d\n", value); 
	switch(value)                            
	{                                        
		case SENSOR_EFFECT_OFF:    
			printk(KERN_INFO "SENSOR_EFFECT_OFF %d\n", value);       
			gt0308_xx_normal();   
			break;                   
		case SENSOR_EFFECT_MONO:   
			printk(KERN_INFO "SENSOR_EFFECT_MONO %d\n", value);       
			gt0308_xx_bandw();    
			break;                   
		case SENSOR_EFFECT_NEGATIVE:  
			printk(KERN_INFO "SENSOR_EFFECT_NEGATIVE %d\n", value);    
			gt0308_xx_negative(); 
			break;                   
		case SENSOR_EFFECT_SOLARIZE:     
			printk(KERN_INFO "SENSOR_EFFECT_SOLARIZE %d\n", value); 
			gt0308_xx_yellowish();
			break;                   
		case SENSOR_EFFECT_PASTEL: 
			printk(KERN_INFO "SENSOR_EFFECT_PASTEL %d\n", value);       
			gt0308_xx_reddish();  
			break;                   
		case SENSOR_EFFECT_MOSAIC:   
			printk(KERN_INFO "SENSOR_EFFECT_MOSAIC %d\n", value);     
			break;                   
		case SENSOR_EFFECT_RESIZE: 
			printk(KERN_INFO "SENSOR_EFFECT_RESIZE %d\n", value);      
			break;                   
		case SENSOR_EFFECT_SEPIA:     
			printk(KERN_INFO "SENSOR_EFFECT_SEPIA %d\n", value);    
			gt0308_xx_sepia();    
			break;                   
		case SENSOR_EFFECT_POSTERIZE:   
			printk(KERN_INFO "SENSOR_EFFECT_POSTERIZE %d\n", value);  
			gt0308_xx_bluish();   
			break;                   
		case SENSOR_EFFECT_WHITEBOARD:  
			printk(KERN_INFO "SENSOR_EFFECT_WHITEBOARD %d\n", value);  
			break;                   
		case SENSOR_EFFECT_BLACKBOARD:   
			printk(KERN_INFO "SENSOR_EFFECT_BLACKBOARD %d\n", value); 
			break;                   
		case SNESOR_EFFECT_AQUA:         
			printk(KERN_INFO "NESOR_EFFECT_AQUA %d\n", value); 
			gt0308_xx_greenish(); 
			break;                   
		default:                         
			break;                   
	}                                        

	return 0;
}


int gt0308_xx_set_wb(int value)
{
	
		printk(KERN_INFO "gt0308_xx_set_wb %d\n", value); 
	switch(value)
	{
		case SENSOR_WB_AUTO:
			printk(KERN_INFO "SENSOR_WB_AUTO %d\n", value); 
			gt0308_xx_auto();
			break;
	//	case SENSOR_WB_CUSTOM:
	//		printk(KERN_INFO "SENSOR_WB_CUSTOM %d\n", value); 
	//		break;
		case SENSOR_WB_INCANDESCENT:
			printk(KERN_INFO "SENSOR_WB_INCANDESCENT %d\n", value); 
			gt0308_xx_home();
			break;
		case SENSOR_WB_FLUORESCENT:
			printk(KERN_INFO "SENSOR_WB_FLUORESCENT %d\n", value); 
			gt0308_xx_office();
			break;
		case SENSOR_WB_DAYLIGHT:
			printk(KERN_INFO "SENSOR_WB_DAYLIGHT %d\n", value); 
			gt0308_xx_sunny();
			break;
		case SENSOR_WB_CLOUDY:
			printk(KERN_INFO "SENSOR_WB_CLOUDY %d\n", value); 
			gt0308_xx_cloudy();
			break;
		case SENSOR_WB_TWILIGHT:
			printk(KERN_INFO "SENSOR_WB_TWILIGHT %d\n", value); 
			break;
		case SENSOR_WB_SHADE:
			printk(KERN_INFO "SENSOR_WB_SHADE %d\n", value); 
			break;
		default:
			break;
	}

	return 0;
}



int gt0308_xx_set_antibanding(void){
	return -1;
} 

int gt0308_xx_set_brightness(void) {
	return -1;
}

int gt0308_xx_set_nightmode(void) {
	return -1;
}

static uint32_t cmos_id_list[] = { 0x9b, 0};

static struct res_to_fps_map gt0308_xx_rf_maps[2] = {
	{
		.res = DWL_CAM_RES_VGA,
		.fps = DWL_CAM_FPS_25,
	}, {
		.res = DWL_CAM_RES_QVGA,
		.fps = DWL_CAM_FPS_30,
	}
};
static uint32_t gt0308_xx_get_fps(uint32_t res)
{
	int i = 0;
	uint32_t fps = 0;

	if (res == 0) {
		return DWL_CAM_FPS_22;
	}

	for ( ; i < (sizeof(gt0308_xx_rf_maps) / sizeof(gt0308_xx_rf_maps[0])); 
			i++) {
		if (gt0308_xx_rf_maps[i].res == res) {
			fps = gt0308_xx_rf_maps[i].fps;
			break;
		}
	}

	return fps;
}

struct sensor_ops gt0308_xx_ops = {
		DWL_CAM_RES_VGA | DWL_CAM_RES_QVGA,
	.get_fps		= gt0308_xx_get_fps,

	.s_lmode		= DWL_CAM_LIGHT_MODE_AUTO |
		DWL_CAM_LIGHT_MODE_INCANDESCENT |
		DWL_CAM_LIGHT_MODE_FLUORESCENT |
		DWL_CAM_LIGHT_MODE_DAYLIGHT |
		DWL_CAM_LIGHT_MODE_CLOUDY_DAYLIGHT,

	.s_seffect		= DWL_CAM_SPECIAL_EFFECT_NONE |
		DWL_CAM_SPECIAL_EFFECT_MONO |
		DWL_CAM_SPECIAL_EFFECT_NEGATIVE |
		DWL_CAM_SPECIAL_EFFECT_SOLARIZE |
		DWL_CAM_SPECIAL_EFFECT_PASTEL |
		DWL_CAM_SPECIAL_EFFECT_SEPIA |
		DWL_CAM_SPECIAL_EFFECT_POSTERIZE |
		DWL_CAM_SPECIAL_EFFECT_AQUA,

	.reset                  = gt0308_xx_reset,
	.set_mode               = gt0308_xx_set_mode,
	.set_effect             = gt0308_xx_set_effect,
	.set_wb                 = gt0308_xx_set_wb,
	.get_id			= gt0308_xx_get_id,
	.idlist                 = cmos_id_list,
	.addr                   = GT0308_I2C_WADDR,
	.name                   = "xhb:gt0308",
	.pwdn                   = 1,
	.hwid                   = 4,
};

static int __init gt0308_xx_init(void)
{
	printk("gt0308-ops-register");
	ops = &gt0308_xx_ops;
	imapx200_cam_sensor_register(ops);

	return 0;
}

static void __exit gt0308_xx_exit(void)
{
	printk(KERN_INFO "gt0308-ops-unregister");
	imapx200_cam_sensor_unregister(&gt0308_xx_ops);
}

module_init(gt0308_xx_init);
module_exit(gt0308_xx_exit);





