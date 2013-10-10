/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of I80 screen parameter defitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** 
**/

#ifndef _I80_DEF_H_
#define _I80_DEF_H_

#define I80_PRIV_BLOCK_NUM 6 

typedef void (*priv_func)(void);

enum I80_MANU_ID {
	I80_MANU_POWER 	= 1, // i80 screen power supply
	I80_MANU_RGB 	= 2, // rgb data render
	I80_MANU_BL	 	= 4, // backlight
	I80_MANU_PROC1 	= 11, // reserved function 1
	I80_MANU_PROC2 	= 12, // reserved function 2
	I80_MANU_PROC3 	= 13  // reserved function 3
};
	
typedef struct {
	enum I80_MANU_ID		id; // manupulation id 
	IM_UINT32 		ms_time;  // sleep time in ms
	void 			*priv; // private data
	priv_func 		proc; // private function
}i80_ob;

typedef struct {
	i80_ob   block[I80_PRIV_BLOCK_NUM];
}I80_Priv_conf;


/*********************/
/**    REFERENCE *****/
/********************/

/*
  NO1.
 	 I80_MANU_PROC1 & I80_MANU_PROC2 & I80_MANU_PROC3 : are for reserved use.
   	 		For special cases, I80 screen may need some particular operations.
	 		we have three reserved functions for these needs.
 
  NO2.
     I80_MANU_POWER : for i80 screen own power supply, through pad IO set.
	 				its configuration is read from item : standby_pin 

  NO3.
 	 I80_MANU_RGB : rgb data render to i80 screen enable, through configure 
	 				the i80 controller, and configure the pads 

  NO4.
    normal power-on processes: (eg. BF097XN I80 screen )
			I80-POWER --> delay 10ms --> RGB data render --> delay 10ms --> backlight on 
	
 */

#endif// _I80_DEF_H_



