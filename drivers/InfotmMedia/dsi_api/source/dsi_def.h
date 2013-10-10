/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of DSI screen parameter defitions
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1     Sam@2012/10/10 :  first commit
** 1.1.0     Sam@2012/10/22 :  DSI first stable version, support the first product.
**
************************************************************************************/

#ifndef _DSI_DEF_H_
#define _DSI_DEF_H_

#define DSI_PRIV_BLOCK_NUM 7

typedef void (*priv_func)(void);

enum DSI_MANU_ID {
	DSI_MANU_POWER 	 = 1, // dsi screen power supply
	DSI_MANU_RGB 	 = 2, // rgb data render
	DSI_MANU_CMD_ON  = 3, // CMD sent to power on the mipi device 
	DSI_MANU_CMD_OFF = 4, // CMD sent to power off the mipi device 
	DSI_MANU_BL	 	 = 5, // backlight
	DSI_MANU_PROC1 	 = 11, // reserved function 1
	DSI_MANU_PROC2 	 = 12, // reserved function 2
};
	
typedef struct {
	enum DSI_MANU_ID		id; // manupulation id 
	IM_UINT32 		ms_time;  // sleep time in ms
	void 			*priv; // private data
	priv_func 		proc; // private function
}dsi_ob;

typedef struct {
	dsi_ob   block[DSI_PRIV_BLOCK_NUM];
}DSI_Priv_conf;


/*********************/
/**    REFERENCE *****/
/********************/

/*
  NO1.
 	 DSI_MANU_PROC1 & DSI_MANU_PROC2 & DSI_MANU_PROC3 : are for reserved use.
   	 		For special cases, DSI screen may need some particular operations.
	 		we have three reserved functions for these needs.
 
  NO2.
     DSI_MANU_POWER : for dsi screen own power supply, through pad IO set.
	 				its configuration is read from item : standby_pin 

  NO3.
 	 DSI_MANU_RGB : rgb data render to dsi screen enable, through configure 
	 				the dsi controller, and configure the pads 

  NO4.
 	 DSI_MANU_CMD_ON :  Command to be sent to mipi screen during power on sequence.
                For some mipi screens needs to be configure its internal register to support 
                a certain working mode, eg. exit_sleep command

  NO5.
 	 DSI_MANU_CMD_OFF :  Command to be sent to mipi screen during power off sequence.
                eg. to sent the enter_sleep command to the mipi screen 

  NO6.
    normal power-on processes:
			DSI-POWER --> delay 10ms --> RGB data render --> delay 250ms  --> backlight on 
	
 */

#endif// _DSI_DEF_H_



