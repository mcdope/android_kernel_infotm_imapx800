/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of LCD screen parameter defitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** 
**/

#ifndef _LCD_DEF_H_
#define _LCD_DEF_H_

#define LCD_PRIV_BLOCK_NUM 7 

typedef void (*priv_func)(void);

enum LCD_MANU_ID {
	LCD_MANU_POWER 	= 1, // lcd screen power supply
	LCD_MANU_RGB 	= 2, // rgb data render
	LCD_MANU_LVDS 	= 3, // lvds power supply
	LCD_MANU_BL	 	= 4, // backlight
	LCD_MANU_PROC1 	= 11, // reserved function 1
	LCD_MANU_PROC2 	= 12, // reserved function 2
	LCD_MANU_PROC3 	= 13  // reserved function 3
};
	
typedef struct {
	enum LCD_MANU_ID		id; // manupulation id 
	IM_UINT32 		ms_time;  // sleep time in ms
	void 			*priv; // private data
	priv_func 		proc; // private function
}lcd_ob;

typedef struct {
	lcd_ob   block[LCD_PRIV_BLOCK_NUM];
}LCD_Priv_conf;


/*********************/
/**    REFERENCE *****/
/********************/

/*
  NO1.
 	 LCD_MANU_PROC1 & LCD_MANU_PROC2 & LCD_MANU_PROC3 : are for reserved use.
   	 		For special cases, LCD screen may need some particular operations.
	 		we have three reserved functions for these needs.
 
  NO2.
     LCD_MANU_POWER : for lcd screen own power supply, through pad IO set.
	 				its configuration is read from item : standby_pin 

  NO3.
	 LCD_MANU_LVDS : lvds lcd screen needs this configuration, to power on 
	 				 the lvds ic through regulator. It is read from : : power_pin

  NO4.
 	 LCD_MANU_RGB : rgb data render to lcd screen enable, through configure 
	 				the lcd controller, and configure the pads 

  NO5.
    normal power-on processes: (eg. BF097XN LCD screen )
			LCD-POWER --> delay 10ms --> RGB data render --> delay 10ms --> LVDS-POWER
			--> delay 250ms  --> backlight on 
	
 */

#endif// _LCD_DEF_H_



