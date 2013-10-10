/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932Controller.h

  Description :  Head file of EP932Controller.

\******************************************************************************/

#ifndef EP932CONTROLLER_H
#define EP932CONTROLLER_H

#include "EP932ERegDef.h"
#include "EP932_If.h"
#include "EP932api.h"

#if Enable_HDCP
#include "HDCP.h"
#endif

// for EP MCU =========
//#include "SMBUS.h"
//#include "GPIO.h"
//=====================

#define VERSION_MAJOR             1   // 
#define VERSION_MINOR             42  //   

//#define EP932C_TIMER_PERIOD       10  //The EP932Controller.c must be re-compiled if user want to change this value.
//#define EP932C_TIMER_PERIOD       100  //100ms

typedef enum {
	TXS_Search_EDID,
	TXS_Wait_Upstream,
	TXS_Stream,
	TXS_HDCP
} TX_STATE;

typedef struct _EP932C_REGISTER_MAP {

	unsigned char		Version_Major;
	unsigned char		Version_Minor;

	unsigned char		System_Status;		

#if Enable_HDCP
	unsigned char		HDCP_Status;			
	unsigned char		HDCP_State;
	unsigned char		HDCP_AKSV[5];
	unsigned char		HDCP_BKSV[5];
	unsigned char		HDCP_BCAPS3[3];
	unsigned char		HDCP_KSV_FIFO[5*16];
	unsigned char		HDCP_SHA[20];
	unsigned char		HDCP_M0[8];
#endif

	unsigned char		Readed_EDID[256];		
	unsigned char		EDID_ASFreq;
	unsigned char		EDID_AChannel;
	//unsigned char		EDID_VideoDataAddr;
	//unsigned char		EDID_AudioDataAddr;
	//unsigned char		EDID_SpeakerDataAddr;
	//unsigned char		EDID_VendorDataAddr;

	unsigned char		System_Configuration;

	unsigned char		Video_Interface[2];		//		
	unsigned char		Video_Input_Format[2];	//	
	unsigned char 		Video_Output_Format;	//

	unsigned char		Audio_Interface;		//		
	unsigned char		Audio_Input_Format;		//
	unsigned char 		Audio_change;		

} EP932C_REGISTER_MAP, *PEP932C_REGISTER_MAP;

// -----------------------------------------------------------------------------

extern TX_STATE TX_State;

// -----------------------------------------------------------------------------

typedef void (*EP932C_CALLBACK)(void);

void EP932Controller_Initial(PEP932C_REGISTER_MAP pEP932C_RegMap);

void EP932Controller_Task(void);

void EP932Controller_Timer(void);

void EP932_Audio_reg_config(void);
void EP932_Video_reg_config(void);

void EP_HDMI_DumpMessage(unsigned char Dump_Parameter);
unsigned int EP932_HotPlugMonitor(void);
unsigned int EP932_HotPlugMonitorInt(void);

// -----------------------------------------------------------------------------
#endif

