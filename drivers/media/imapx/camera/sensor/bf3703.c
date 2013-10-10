/***************************************************************************** 
 ** bf3703.c 
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
 ** 2.0  04/20/2011 neville   
 *******************************************************************************/

#include "sensor.h"
#include "bf3703.h"

static struct sensor_ops *ops;
#define cam_bf3703_read(b, c, d) ops->i2c_read(c, b, 1, d)
#define cam_bf3703_write(b) ops->i2c_write(b, 2)          
                                                                                          
/*
static int HI253_write_cmos_sensor(unsigned char addr, unsigned value)
{
	struct bf3703_regval_list  regval;
	regval.reg = addr;
	regval.value  = value;
	cam_bf3703_write((unsigned char *)(&regval));
	return 0;
}

static  char HI253_read_cmos_sensor(unsigned char addr)
{
	char buf;
	cam_bf3703_read((unsigned char *)(&addr), &buf, 1);
	return buf;
}
*/

int bf3703_xx_reset(void __iomem *reg)
{

	uint32_t tmp = 0;
	printk(KERN_INFO "bf3703 cam reset\n");
	writel(0x0, reg + IMAP_CIGCTRL);

	tmp = readl(reg+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);
	mdelay(100);

	tmp = readl(reg+IMAP_CIGCTRL);
	tmp &= ~(0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);
	mdelay(100);

	tmp = readl(reg+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, reg+IMAP_CIGCTRL);

	return 0;
}
/*
 *   init & close sensor 
 */

int bf3703_start(void){
	int i,ret; 
/*	
	char reg0x04 = 0;
	int mod8;

	mod8 = sizeof(bf3703_init_regs);
	printk("mod8 = %d,data addr %x \n",mod8,bf3703_init_regs);
	ret = cam_bf3703_write_big(BF3703_I2C_WADDR, bf3703_init_regs, mod8);
	printk(KERN_INFO "ret = %d\n",ret);
*/	
	printk(KERN_INFO "bf3703_start++ \n");
#if 1
	for(i = 0; i < (sizeof(bf3703_init_regs) / 2); i++)
	{
		ret = cam_bf3703_write((unsigned char *)(&bf3703_init_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}

int bf3703_close(void)
{
	int i, ret;
	printk(KERN_INFO "bf3703_close\n");	

	for(i = 0; i < ((sizeof(bf3703_stop_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char *)(&bf3703_stop_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
	return 0; 
} 
/*
 *  set capture mode
 */
	/* 
 	 * used to change fps to 22
	 */
int  bf3703_switch_low_svga(void)
{

	/* int i, ret; */

	printk(KERN_INFO "bf3703_switch_low_svga\n");	
#if 0
	for(i = 0; i < ((sizeof(bf3703_svga_low_regs) / 3)); i++)
	{
		ret = cam_bf3703_write(BF3703_I2C_WADDR, (unsigned char *)(&bf3703_svga_low_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}

	/* 
 	 * used to change fps to 18
	 */
int  bf3703_switch_high_svga(void)
{
	/* int i, ret; */

	printk(KERN_INFO "bf3703_switch_high_svga\n");	
#if 0
	for(i = 0; i < ((sizeof(bf3703_svga_high_regs) / 3)); i++)
	{
		ret = cam_bf3703_write(BF3703_I2C_WADDR, (unsigned char *)(&bf3703_svga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}

 /*
  * 1600x1200
  */
int  bf3703_switch_high_xuga(void)
{
#if 0
	int i, ret;
	printk(KERN_INFO "HI253 to high xuga\n");

	for(i = 0; i < ((sizeof(bf3703_xuga_high_regs) / 2)); i++)
	{
		ret = cam_bf3703_write(BF3703_I2C_WADDR, (unsigned char *)(&bf3703_xuga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}

/*
 * 1280x960
 */
int bf3703_switch_upmid_xuga(void)
{
#if 0
	int i, ret;

	for(i = 0; i < ((sizeof(bf3703_xuga_upmid_regs) / 2)); i++)
	{
		ret = cam_bf3703_write(BF3703_I2C_WADDR, (unsigned char *)(&bf3703_xuga_upmid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}

/*
 * 640 * 480
 */
int  bf3703_switch_mid_xuga(void)
{
#if 0
	int i, ret;
	printk(KERN_INFO "HI253 to mid xuga\n");
	for(i = 0; i < ((sizeof(bf3703_xuga_mid_regs) / 2)); i++)
	{
		ret = cam_bf3703_write(BF3703_I2C_WADDR, (unsigned char *)(&bf3703_xuga_mid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif
	return 0;
}
/*
 * 320 * 240
 */

int  bf3703_switch_low_xuga(void)
{
	int i, ret;
	for(i = 0; i < ((sizeof(bf3703_xuga_low_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char *)(&bf3703_xuga_low_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
	return 0;
}

/*
 * svga --> xuga
 */

int bf3703_svga_to_xuga(int width, int height)
{
#if 0
	uint32_t HI253_pv_HI253_exposure_lines = 0x0249f0;
	uint32_t HI253_cp_HI253_exposure_lines = 0;
	uint8_t  HI253_Sleep_Mode;
	uint8_t  CLK_DIV_REG;
	printk(KERN_INFO "Taking Picture!\n");
	if( width <=  640 && height <= 480){
		
		HI253_write_cmos_sensor(0x03, 0x00);
/*
		HI253_write_cmos_sensor(0x20, 0x00);
		HI253_write_cmos_sensor(0x21, 0x04);
		HI253_write_cmos_sensor(0x22, 0x00);
		HI253_write_cmos_sensor(0x23, 0x07);
*/
		HI253_write_cmos_sensor(0x40, 0x01);
		HI253_write_cmos_sensor(0x41, 0x68);
		HI253_write_cmos_sensor(0x42, 0x00);
		HI253_write_cmos_sensor(0x43, 0x14);

#if 0
		HI253_write_cmos_sensor(0x03, 0x10);
		HI253_write_cmos_sensor(0x3f, 0x00);
		//page 12
		HI253_write_cmos_sensor(0x03, 0x12);
		HI253_write_cmos_sensor(0x20, 0x0f);
		HI253_write_cmos_sensor(0x21, 0x0f);
		HI253_write_cmos_sensor(0x90, 0x5d);
		//page 13
		HI253_write_cmos_sensor(0x03, 0x13);
		HI253_write_cmos_sensor(0x80, 0xfd);
#endif
		// 800 * 600
	//	HI253_write_cmos_sensor(0x03, 0x00);
	//	HI253_write_cmos_sensor(0x10, 0x11);

		HI253_write_cmos_sensor(0x03, 0x20);
		HI253_write_cmos_sensor(0x86, 0x01);
		HI253_write_cmos_sensor(0x87, 0xf4);

		HI253_write_cmos_sensor(0x8b, 0x83);
		HI253_write_cmos_sensor(0x8c, 0xd6);
		HI253_write_cmos_sensor(0x8d, 0x6d);
		HI253_write_cmos_sensor(0x8e, 0x60);

		HI253_write_cmos_sensor(0x9c, 0x17);
		HI253_write_cmos_sensor(0x9d, 0x70);
		HI253_write_cmos_sensor(0x9e, 0x01);
		HI253_write_cmos_sensor(0x9f, 0xf4);

		HI253_write_cmos_sensor(0x03, 0x20);
		HI253_pv_HI253_exposure_lines = (HI253_read_cmos_sensor(0x80) << 16)|(HI253_read_cmos_sensor(0x81) << 8)|HI253_read_cmos_sensor(0x82);

		HI253_cp_HI253_exposure_lines=HI253_pv_HI253_exposure_lines;

		if(HI253_cp_HI253_exposure_lines<1)
			HI253_cp_HI253_exposure_lines=1;

		HI253_write_cmos_sensor(0x03, 0x20);
		HI253_write_cmos_sensor(0x83, HI253_cp_HI253_exposure_lines >> 16);
		HI253_write_cmos_sensor(0x84, (HI253_cp_HI253_exposure_lines >> 8) & 0x000000FF);
		HI253_write_cmos_sensor(0x85, HI253_cp_HI253_exposure_lines & 0x000000FF);
	}
	else {

		HI253_write_cmos_sensor(0x03,0x00);
//		HI253_Sleep_Mode = (HI253_read_cmos_sensor(0x01) & 0xfe);
//		HI253_Sleep_Mode |= 0x01;
//		HI253_write_cmos_sensor(0x01, HI253_Sleep_Mode);

		CLK_DIV_REG=(HI253_read_cmos_sensor(0x12)&0xFc);

		//read the shutter (manual exptime)

		HI253_write_cmos_sensor(0x03, 0x20);
		HI253_pv_HI253_exposure_lines = (HI253_read_cmos_sensor(0x80) << 16)|(HI253_read_cmos_sensor(0x81) << 8)|HI253_read_cmos_sensor(0x82);

		HI253_cp_HI253_exposure_lines = HI253_pv_HI253_exposure_lines/2;

		HI253_write_cmos_sensor(0x03, 0x00);

//		HI253_write_cmos_sensor(0x20, 0x00);
//		HI253_write_cmos_sensor(0x21, 0x0a);
//		HI253_write_cmos_sensor(0x22, 0x00);
//		HI253_write_cmos_sensor(0x23, 0x0a);

		HI253_write_cmos_sensor(0x40, 0x01);
		HI253_write_cmos_sensor(0x41, 0x58);// 0168
		HI253_write_cmos_sensor(0x42, 0x00);
		HI253_write_cmos_sensor(0x43, 0x14);

//		HI253_write_cmos_sensor(0x03, 0x10);
//		HI253_write_cmos_sensor(0x3f, 0x00);
		//PAGE 12

//		HI253_write_cmos_sensor(0x03, 0x12);
//		HI253_write_cmos_sensor(0x20, 0x0f);
//		HI253_write_cmos_sensor(0x21, 0x0f);
//		HI253_write_cmos_sensor(0x90, 0x5d);
		//PAGE 13
//		HI253_write_cmos_sensor(0x03, 0x13);
//		HI253_write_cmos_sensor(0x80, 0xfd);
		//1600 *1200
	//	HI253_write_cmos_sensor(0x03,0x00);
	//	HI253_write_cmos_sensor(0x10,0x00);

		CLK_DIV_REG=CLK_DIV_REG|0x1;
		HI253_write_cmos_sensor(0x03, 0x00);
		HI253_write_cmos_sensor(0x12,CLK_DIV_REG);

		HI253_write_cmos_sensor(0x03, 0x20);
        	HI253_write_cmos_sensor(0x86, 0x01);
		HI253_write_cmos_sensor(0x87, 0xf0);

		HI253_write_cmos_sensor(0x8b, 0x41);
		HI253_write_cmos_sensor(0x8c, 0xe0);
		HI253_write_cmos_sensor(0x8d, 0x36);
		HI253_write_cmos_sensor(0x8e, 0x40);

		HI253_write_cmos_sensor(0x9c, 0x0d);
		HI253_write_cmos_sensor(0x9d, 0x90);
		HI253_write_cmos_sensor(0x9e, 0x01);
		HI253_write_cmos_sensor(0x9f, 0xf0);

		if(HI253_cp_HI253_exposure_lines<1)
			HI253_cp_HI253_exposure_lines=1;

		HI253_write_cmos_sensor(0x03, 0x20);
		HI253_write_cmos_sensor(0x83, HI253_cp_HI253_exposure_lines >> 16);
		HI253_write_cmos_sensor(0x84, (HI253_cp_HI253_exposure_lines >> 8) & 0x000000FF);
		HI253_write_cmos_sensor(0x85, HI253_cp_HI253_exposure_lines & 0x000000FF);

//		HI253_write_cmos_sensor(0x03,0x00);
//		HI253_Sleep_Mode = (HI253_read_cmos_sensor(0x01) & 0xfe);
//		HI253_Sleep_Mode |= 0x00;
//		HI253_write_cmos_sensor(0x01, HI253_Sleep_Mode);

	}
#endif
	return 0;
}

int bf3703_to_preview_320_240(void)
{
#if 1
	int i, ret; 
	printk(KERN_INFO "[bf3703]-to preview 320 * 240\n");	
	for(i = 0; i < (sizeof(bf3703_to_preview_320_240_regs) / 2); i++)
	{
		ret = cam_bf3703_write((unsigned char *)(&bf3703_to_preview_320_240_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif

	return 0;
}

int bf3703_to_preview_640_480(void)
{
	int i, ret; 
	printk(KERN_INFO "[hy253]-to preview 640 * 480\n");	
	for(i = 0; i < (sizeof(bf3703_to_preview_640_480_regs) / 2); i++)
	{
		ret = cam_bf3703_write((unsigned char *)(&bf3703_to_preview_640_480_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
	return 0;
}



/*
 *   set the color effect
 */

int bf3703_sepia(void)
{ 
#if 1
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sepia_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sepia_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 
#endif
	return 0;
} 


int bf3703_bluish(void)
{ 
#if 1
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_bluish_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_bluish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	}
#endif	
	return 0;
} 


int bf3703_greenish(void)
{ 
#if 1
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_greenish_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_greenish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 
#endif
	return 0;
}

int bf3703_reddish(void)
{ 
#if 1
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_reddish_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_reddish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 
#endif
	return 0;
}


int bf3703_yellowish(void)
{ 
#if 1
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_yellowish_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_yellowish_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 
#endif
	return 0;
}


int bf3703_bandw(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_bandw_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_bandw_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}


int bf3703_negative(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_negative_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_negative_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

int bf3703_normal(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_normal_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_normal_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

/*
 * Light Mode
 */


int bf3703_auto(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_auto_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_auto_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

int bf3703_sunny(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sunny_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sunny_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

int bf3703_cloudy(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_cloudy_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_cloudy_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

int bf3703_office(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_office_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_office_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}

int bf3703_home(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_home_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_home_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0;
}


/*
 * Color saturation
 */

int bf3703_saturation_0(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_saturation_0_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_saturation_0_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
} 

int bf3703_saturation_1(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_saturation_1_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_saturation_1_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_saturation_2(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_saturation_2_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_saturation_2_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_saturation_3(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_saturation_3_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_saturation_3_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_saturation_4(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_saturation_4_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_saturation_4_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}


/*
 *  Brightness
 */

int bf3703_brightness_0(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_0_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_0_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
} 

int bf3703_brightness_1(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_1_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_1_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_brightness_2(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_2_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_2_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_brightness_3(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_3_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_3_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_brightness_4(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_4_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_4_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_brightness_5(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_5_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_5_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_brightness_6(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_brightness_6_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_brightness_6_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}


/*
 * Constrat
 */

int bf3703_contrast_0(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_0_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_0_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
} 

int bf3703_contrast_1(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_1_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_1_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_contrast_2(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_2_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_2_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_contrast_3(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_3_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_3_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_contrast_4(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_4_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_4_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_contrast_5(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_5_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_5_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_contrast_6(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_contrast_6_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_contrast_6_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}


/*
 * Sharpness
 */

int bf3703_sharpness_0(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_0_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_0_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
} 

int bf3703_sharpness_1(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_1_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_1_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_sharpness_2(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_2_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_2_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_sharpness_3(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_3_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_3_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_sharpness_4(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_4_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_4_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_sharpness_auto(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_sharpness_auto_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_sharpness_auto_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}


/*
 * Night Mode
 */

int bf3703_night_mode_on(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_night_mode_on_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_night_mode_on_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}

int bf3703_night_mode_off(void)
{ 
	int i, ret; 

	for(i = 0; i< ((sizeof(bf3703_night_mode_off_regs) / 2)); i++)
	{
		ret = cam_bf3703_write((unsigned char*)(&bf3703_night_mode_off_regs[i]));
		if(ret)
		{
			IC_ERR("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		}
	} 

	return 0 ;
}
int bf3703_xx_set_mode(int value, void __iomem *reg)
{
	int ret;
	printk( KERN_INFO "bf3703 set mode %d \n",value);
	switch(value)
	{
		case INIT_SENSOR:
			ret = bf3703_start();
			break;

		case SWITCH_SENSOR_TO_HIGH_SVGA:
			ret = bf3703_switch_high_svga();
			break;

		case SWITCH_SENSOR_TO_LOW_SVGA:
			ret = bf3703_switch_low_svga();
			break;

		case SWITCH_SENSOR_TO_HIGH_XUGA:
			ret = bf3703_svga_to_xuga(1600,1200);
			ret = bf3703_switch_high_xuga();
			break;

		case SWITCH_SENSOR_TO_UPMID_XUGA:
			ret = bf3703_svga_to_xuga(1280, 960);
			ret = bf3703_switch_upmid_xuga();
			break;

		case SWITCH_SENSOR_TO_MID_XUGA:
			ret = bf3703_svga_to_xuga(640,480);
			ret = bf3703_switch_mid_xuga();
			break;

		case SWITCH_SENSOR_TO_LOW_XUGA:
			ret = bf3703_svga_to_xuga(320,240);
			ret = bf3703_switch_low_xuga();
			break;

		case SENSOR_TO_HIGH_PREVIEW:
			ret = bf3703_to_preview_640_480();
			break;

		case SENSOR_TO_LOW_PREVIEW:
			ret = bf3703_start();
			ret = bf3703_to_preview_320_240();
			break;
		case CLOSE_SENSOR:
			ret = bf3703_close();
			break;
		default: 
			ret = -1;
			break;

	}
	return ret;

}

int bf3703_xx_set_effect(int value)
{

	switch(value)                            
	{                                        
		case SENSOR_EFFECT_OFF:          
			bf3703_normal();   
			break;                   
		case SENSOR_EFFECT_MONO:         
			bf3703_bandw();    
			break;                   
		case SENSOR_EFFECT_NEGATIVE:     
			bf3703_negative(); 
			break;                   
		case SENSOR_EFFECT_SOLARIZE:     
			bf3703_yellowish();
			break;                   
		case SENSOR_EFFECT_PASTEL:       
			bf3703_reddish();  
			break;                   
		case SENSOR_EFFECT_MOSAIC:       
			break;                   
		case SENSOR_EFFECT_RESIZE:       
			break;                   
		case SENSOR_EFFECT_SEPIA:        
			bf3703_sepia();    
			break;                   
		case SENSOR_EFFECT_POSTERIZE:    
			bf3703_bluish();   
			break;                   
		case SENSOR_EFFECT_WHITEBOARD:   
			break;                   
		case SENSOR_EFFECT_BLACKBOARD:   
			break;                   
		case SNESOR_EFFECT_AQUA:         
			bf3703_greenish(); 
			break;                   
		default:                         
			break;                   
	}                                        

	return -1;
}

int bf3703_xx_set_wb(int value)
{
	switch(value)
	{
		case SENSOR_WB_AUTO:
			bf3703_auto();
			break;
		case SENSOR_WB_INCANDESCENT:
			bf3703_home();
			break;
		case SENSOR_WB_FLUORESCENT:
			bf3703_office();
			break;
		case SENSOR_WB_DAYLIGHT:
			bf3703_sunny();
			break;
		case SENSOR_WB_CLOUDY:
			bf3703_cloudy();
			break;
		case SENSOR_WB_TWILIGHT:
			break;
		case SENSOR_WB_SHADE:
			break;
		default:
			break;
	}

	return -1;
}

int bf3703_xx_set_antibanding(int value)
{
	return -1;
}

int bf3703_xx_set_brightness(int value)
{
	return -1;
}

int bf3703_xx_set_nightmode(int value)
{
	return -1;
}

#ifdef CONFIG_SENSOR_TEST 
int bf3703_xx_sensor_dbg(uint32_t value)
{
	unsigned char add = 0,val = 0,buf = 0;
	struct bf3703_regval_list  regval;

	printk(KERN_ERR  "[IIC-DBUG]- Get %05x",value);
	switch(value & 0x00001){

		case 1:
			add = (value & 0xFF000) >> 12;
			val =  (value & 0x00FF0) >> 4;
			cam_bf3703_read((unsigned char *)(&add), &buf, 1);
			printk(KERN_ERR "    [IIC_READ]- reg-%x,val-%x\n",add,buf);
			break;
		case 0:
			add = (value & 0xFF000) >> 12;
			val = (value & 0x00FF0) >> 4;
			regval.reg= add;
			regval.value = val;
			printk(KERN_ERR "    [IIC_WRTE]- reg-%x,val-%x\n",add,val);
			cam_bf3703_write((unsigned char *)(&regval));
			break;
		default:
			break;
	}
	return 0;
}
#endif

/* FPS api added by Sololz. */
static struct res_to_fps_map bf3703_rf_maps[3] = {
	{
		.res = DWL_CAM_RES_VGA,
		.fps = DWL_CAM_FPS_22,
	}, {
		.res = DWL_CAM_RES_QVGA,
		.fps = DWL_CAM_FPS_30,
	}
};
static uint32_t bf3703_get_fps(uint32_t res)
{
	int i = 0;
	uint32_t fps = 0;

	if (res == 0) {
		return DWL_CAM_FPS_22;
	}

	for ( ; i < (sizeof(bf3703_rf_maps) / sizeof(bf3703_rf_maps[0])); 
			i++) {
		if (bf3703_rf_maps[i].res == res) {
			fps = bf3703_rf_maps[i].fps;
			break;
		}
	}

	return fps;
}

struct sensor_ops bf3703_ops = {
	.s_res			= DWL_CAM_RES_VGA | DWL_CAM_RES_QVGA,
	.get_fps		= bf3703_get_fps,

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

	.reset			= bf3703_xx_reset,
	.set_mode		= bf3703_xx_set_mode,
	.set_effect		= bf3703_xx_set_effect,
	.set_wb			= bf3703_xx_set_wb,
	.addr			= BF3703_I2C_WADDR,
	.name			= "bf3703",
	.hwid			= 1,

	/* 1 indicates the power on state    
	 *  * requires pwdn pin to be set to low
	 *   */                                  
	.pwdn                   = 1,         
#ifdef CONFIG_SENSOR_TEST
	.sensor_dbg		= bf3703_xx_sensor_dbg,
#endif
	
};

static int __init bf3703_init(void)                 
{                                                       
	printk(KERN_INFO "bf3703-ops-register");     
	ops = &bf3703_ops;                          
	imapx200_cam_sensor_register(ops);              

	return 0;                                       
}                                                       

static void __exit bf3703_exit(void)                
{                                                       
	printk(KERN_INFO "bf3703-ops-unregister");   
	imapx200_cam_sensor_unregister(&bf3703_ops);
}                                                       

module_init(bf3703_init);
module_exit(bf3703_exit);
