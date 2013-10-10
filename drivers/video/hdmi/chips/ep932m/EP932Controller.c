/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005 
                           ALL RIGHTS RESERVED 
 
--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932Controller.c 

  Description :  EP932Controller program 
                 Control SFR directory and use HCI functions 

  Codeing     :  Shihken

\******************************************************************************/


// standard Lib
#include <linux/delay.h>
#include "EP932api.h"
#include "Edid.h"
#include "DDC_If.h"
#include "EP932Controller.h"
#include "EP932SettingsData.h"


//
// Defines
//


#if Enable_HDCP
// HDCP Key  
unsigned char HDCP_Key[64][8];
#endif

//
// Global State and Flags
//

// System flags
unsigned char is_Cap_HDMI;
unsigned char is_Cap_YCC444;
unsigned char is_Cap_YCC422;

unsigned char is_Hot_Plug;
unsigned char is_Connected;
unsigned char is_ReceiverSense;
unsigned char ChkSum, ConnectionState;

unsigned char is_EP932_Reset = 0;

#if Enable_HDCP
unsigned char is_HDCP_Info_BKSV_Rdy;
#endif


// System Data
unsigned char HP_ChangeCount;
TX_STATE TX_State;
VDO_PARAMS Video_Params;
ADO_PARAMS Audio_Params;
PEP932C_REGISTER_MAP pEP932C_Registers;

//
// Private Functions
//
void EP932_HDCP_Reset(void);
void TXS_RollBack_Stream(void);

#if Enable_HDCP
void TXS_RollBack_HDCP(void);
void ReadRiInterruptFlags(void);
#endif

SMBUS_STATUS IIC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);


//--------------------------------------------------------------------------------------------------------------------

void EP932Controller_Initial(PEP932C_REGISTER_MAP pEP932C_RegMap)
{	
	is_EP932_Reset = 1;
	
	// Save the Logical Hardware Assignment
	pEP932C_Registers = pEP932C_RegMap;

	// Hardware Reset 
	EP_EP932M_Reset();
//	EP932_EnableHPInit();

	// Initial IIC address	   
	EP932_IIC_Initial();

	// Initial Variables
	EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__OSCSEL);

	// Default Audio Mute and Video Mute
	HDMI_Tx_Mute_Enable();	

	// Software power down
	HDMI_Tx_Power_Down();

	// Reset Variables
	is_Cap_HDMI = 0;
	is_Cap_YCC444 = is_Cap_YCC422 = 0;
	is_Connected = 1;
	TX_State = TXS_Search_EDID;
	HP_ChangeCount = 0;
	is_ReceiverSense = 0;

	// Reset all EP932 parameters
	pEP932C_Registers->Version_Major = VERSION_MAJOR;
	pEP932C_Registers->Version_Minor = VERSION_MINOR;
	DBG_printf(("EP932 Code Version : %d.%d\r\n", (int)VERSION_MAJOR, (int)VERSION_MINOR ));

	pEP932C_Registers->System_Status = 0;
	pEP932C_Registers->EDID_ASFreq = 0;
	pEP932C_Registers->EDID_AChannel = 0;
	pEP932C_Registers->Audio_change = 0; 
	pEP932C_Registers->System_Configuration = 0;
	pEP932C_Registers->Video_Input_Format[1]= 0;
	pEP932C_Registers->Video_Output_Format = 0;			// Auto select output 
	//pEP932C_Registers->Video_Output_Format = 0x03;	// Force set RGB out

#if Enable_HDCP

	// Initial HDCP Info
	pEP932C_Registers->HDCP_Status = 0;			
	pEP932C_Registers->HDCP_State = 0;
	memset(pEP932C_Registers->HDCP_AKSV, 0x00, sizeof(pEP932C_Registers->HDCP_AKSV));
	memset(pEP932C_Registers->HDCP_BKSV, 0x00, sizeof(pEP932C_Registers->HDCP_BKSV));
	memset(pEP932C_Registers->HDCP_BCAPS3, 0x00, sizeof(pEP932C_Registers->HDCP_BCAPS3));
	memset(pEP932C_Registers->HDCP_KSV_FIFO, 0x00, sizeof(pEP932C_Registers->HDCP_KSV_FIFO));
	memset(pEP932C_Registers->HDCP_SHA, 0x00, sizeof(pEP932C_Registers->HDCP_SHA));
	memset(pEP932C_Registers->HDCP_M0, 0x00, sizeof(pEP932C_Registers->HDCP_M0));

	// Set Revocation List address
	HDCP_Extract_BKSV_BCAPS3(pEP932C_Registers->HDCP_BKSV);
	HDCP_Extract_FIFO((unsigned char*)pEP932C_Registers->HDCP_KSV_FIFO, sizeof(pEP932C_Registers->HDCP_KSV_FIFO));
	HDCP_Stop();

#else

	// Disable HDCP
	HDMI_Tx_HDCP_Disable();

#endif 

	// HDCP KEY reset
	EP932_HDCP_Reset();

	// Info Frame Reset
	EP932_Info_Reset();
	
	is_EP932_Reset = 0;
}

void EP932_HDCP_Reset(void)
{
#if Enable_HDCP
	int i;

	//////////////////////////////////////////////////////////////////
	// Read HDCP Key for EEPROM
	
	status = HDMI_Tx_Get_Key((unsigned char *)HDCP_Key);
	//DBG_printf(("Read HDCP Key = 0x%02X\r\n",(int)status));
	pEP932C_Registers->System_Status &= ~EP932E_System_Status__KEY_FAIL;

	// Check HDCP key and up load the key
	if(status) {
		// Do not upload the default Key!
		pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
		pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
		DBG_printf(("No HDCP Key - Disable HDCP function\r\n"));
	}
	else {
		// Check HDCP key and up load the key
		ChkSum = 0;
		for(i=0; i<328; ++i) {
			ChkSum += *((unsigned char *)HDCP_Key+i);
		}	
		DBG_printf(("HDCP Key Check Sum 0x%02X\r\n", (int)ChkSum ));
		
		if(HDCP_Key[3][7] != 0x50 || HDCP_Key[12][7] != 0x01 || ChkSum != 0x00) {
			pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
			pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
			DBG_printf(("HDCP Key Check failed! - Disable HDCP function\r\n"));
		}
		else {
			// Upload the key 0-39
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
				memcpy(DDC_Data,&HDCP_Key[i][0],7);
				status |= EP932_Reg_Write(EP932_Key_Data, DDC_Data, 7);
			}
			// Read and check	
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
				status |= EP932_Reg_Read(EP932_Key_Data, DDC_Data, 7);
				if((memcmp(DDC_Data,&HDCP_Key[i][0],7) != 0) || status) {
					// Test failed
					pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
					pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
					DBG_printf(("HDCP Key Check failed! - Disable HDCP function\r\n"));
					break;
				}
			}
			// Upload final KSV 40
			DDC_Data[0] = 40;
			status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
			memcpy(DDC_Data,&HDCP_Key[40][0],7);
			status |= EP932_Reg_Write(EP932_Key_Data, DDC_Data, 7);
			// Read back and check
	    	if(!HDMI_Tx_read_AKSV(pEP932C_Registers->HDCP_AKSV)) {
				// Test failed
				pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
				pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
				DBG_printf(("HDCP Check KSV failed! - Disable HDCP function\r\n"));
			}	
		}
	}

#else

	pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
	pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
	DBG_printf(("User define - Disable HDCP function\r\n"));

#endif 

}

void EP932Controller_Timer(void)
{

#if Enable_HDCP
	if(TX_State == TXS_HDCP) HDCP_Timer();
#endif

}

unsigned int EP932_HotPlugMonitorInt(void)
{               
	unsigned char temp;
	is_Hot_Plug = HDMI_Tx_HTPLG();

//	EP932_Reg_Read(EP932_General_Control_2, &temp, 1);

//	if(temp & 0x1)
		return is_Hot_Plug;     
//	else
//		return -1;
}               

unsigned int EP932_HotPlugMonitor(void)
{       
	unsigned char temp;
	is_Hot_Plug = HDMI_Tx_HTPLG();
	return is_Hot_Plug;

}       

void EP932Controller_Task(void)
{
	if(is_EP932_Reset)
	{
		//DBG_printf(("x")); 	// for debug
		return;
	}
	else
	{
		//DBG_printf(("-"));	// for debug	
	}

	// Read Ri ready Flag 
#if Enable_HDCP
	ReadRiInterruptFlags();
#endif

	// Polling check Hot-Plug status 		
	ConnectionState = HDMI_Tx_HTPLG();

	is_Hot_Plug = (ConnectionState == 1)? 1:0;
	is_ReceiverSense = HDMI_Tx_RSEN(); 
		
	if(is_Connected != ((ConnectionState)?1:0) ) {

#if Enable_HDCP
		if(HP_ChangeCount++ >= 50) // Hotplug continuous low 500ms	(10ms x 50)
#else
		if(HP_ChangeCount++ >= 5)  // Hotplug continuous low 500ms	(100ms x 5)
#endif
		{	
			HP_ChangeCount = 0;

			is_Connected = ((ConnectionState)?1:0);
				
			if(!is_Connected){

				DBG_printf(("HDMI - Not connect \r\n"));
					
				// power down EP932 Tx
				HDMI_Tx_Power_Down();
				
				DBG_printf(("\r\nState Transist: Power Down -> [TXS_Search_EDID]\r\n"));							
				TX_State = TXS_Search_EDID;						
			}
		}
	}
	else {
		HP_ChangeCount = 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Update EP932 Registers according to the System Process
	//
	switch(TX_State) 
	{
		case TXS_Search_EDID:

			if(is_Hot_Plug) {
					
				unsigned char EDID_DDC_Status;

				// clear EDID buffer
				memset(pEP932C_Registers->Readed_EDID, 0xFF, 256);
					
				// Read EDID
				DBG_printf(("\r\n[Read EDID] :\r\n"));				
				EDID_DDC_Status = Downstream_Rx_read_EDID(pEP932C_Registers->Readed_EDID);
					
				if(EDID_DDC_Status) {
					DBG_printf(("WARNING: EDID read failed 0x%02X\r\n", (int)EDID_DDC_Status));
					break;
				}
 
				// check EDID
				is_Cap_HDMI = EDID_GetHDMICap(pEP932C_Registers->Readed_EDID);
					
				if(is_Cap_HDMI) {			
					DBG_printf(("EDID : Support HDMI"));

					// Default Capability
					is_Cap_YCC444 =	is_Cap_YCC422 = 0;
					pEP932C_Registers->EDID_ASFreq = 0x07;
					pEP932C_Registers->EDID_AChannel = 1;

					if(!EDID_DDC_Status) {

						if(pEP932C_Registers->Readed_EDID[131] & 0x20) {	// Support YCC444
							is_Cap_YCC444 = 1;
							DBG_printf((" YCC444"));
						}
						if(pEP932C_Registers->Readed_EDID[131] & 0x10) {	// Support YCC422
							is_Cap_YCC422 = 1;
							DBG_printf((" YCC422"));
						}
						DBG_printf(("\r\n"));
						pEP932C_Registers->EDID_ASFreq = EDID_GetPCMFreqCap(pEP932C_Registers->Readed_EDID);
						DBG_printf(("EDID : ASFreq = 0x%02X\r\n",(int)pEP932C_Registers->EDID_ASFreq));
						pEP932C_Registers->EDID_AChannel = EDID_GetPCMChannelCap(pEP932C_Registers->Readed_EDID);
						DBG_printf(("EDID : AChannel = 0x%02X\r\n",(int)pEP932C_Registers->EDID_AChannel));
						DBG_printf(("EDID : Support Max Audio Channel = %d\r\n", (int)pEP932C_Registers->EDID_AChannel+1));

						// Optional 
						//pEP932C_Registers->EDID_VideoDataAddr = 0x00;
						//pEP932C_Registers->EDID_AudioDataAddr = 0x00;
						//pEP932C_Registers->EDID_SpeakerDataAddr = 0x00;
						//pEP932C_Registers->EDID_VendorDataAddr = 0x00;
							
						//pEP932C_Registers->EDID_VideoDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x40);
						//pEP932C_Registers->EDID_AudioDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x20);
						//pEP932C_Registers->EDID_SpeakerDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x80);
						//pEP932C_Registers->EDID_VendorDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x60);
					}
				}
				else {
					DBG_printf(("EDID : Support DVI(RGB) only\r\n"));
					is_Cap_YCC444 =	is_Cap_YCC422 = 0;
					pEP932C_Registers->EDID_ASFreq = pEP932C_Registers->EDID_AChannel = 0;
				}

				is_Connected = 1; // HDMI is connected
					
				DBG_printf(("\r\nState Transit: Read EDID -> [TXS_Wait_Upstream]\r\n"));				
				TX_State = TXS_Wait_Upstream;
			}
			break;
			
		case TXS_Wait_Upstream:

			// update EP932 register setting
			EP932_Audio_reg_config();
			EP932_Video_reg_config();
				
			// Power Up Tx
			HDMI_Tx_Power_Up();

			DBG_printf(("\r\nState Transist: Power Up -> [TXS_Stream]\r\n"));							
			TX_State = TXS_Stream;
				
			// dump EP932 register value for debug ===================================
			
			//EP_HDMI_DumpMessage(1);	// dump EP932 parameter + register				
			EP_HDMI_DumpMessage(0);		// dump EP932 register only 		
			
			// ================================================================

			break;

		case TXS_Stream:

			if(pEP932C_Registers->Audio_change){	// Audio Sample Rate change

				EP932_Audio_reg_config();
				pEP932C_Registers->Audio_change = 0;	
			}
			
			if(!is_Connected) {						// HDMI	not connected

				DBG_printf(("\r\nTXS_Stream: HDMI is not Connected\r\n"));							
				TXS_RollBack_Stream();
				TX_State = TXS_Search_EDID;
			}

#if Enable_HDCP
			else if(!(pEP932C_Registers->System_Configuration & EP932E_System_Configuration__HDCP_DIS) && is_ReceiverSense) {
			
				if(!is_HDCP_Info_BKSV_Rdy) {
					// Get HDCP Info
			    	if(!Downstream_Rx_read_BKSV(pEP932C_Registers->HDCP_BKSV)) {
						pEP932C_Registers->HDCP_Status = EP932E_HDCP_Status__BKSV;
					}
					pEP932C_Registers->HDCP_BCAPS3[0] = Downstream_Rx_BCAPS();
					is_HDCP_Info_BKSV_Rdy = 1;
				}
				
				// Enable mute
				DBG_printf(("Mute first for start HDCP\r\n"));
				HDMI_Tx_Mute_Enable();
				
				DBG_printf(("\r\nState Transist: Start HDCP -> [TXS_HDCP]\r\n"));
				TX_State = TXS_HDCP;
			}
#endif			

			break;

#if Enable_HDCP

		case TXS_HDCP:
		
			if(pEP932C_Registers->Audio_change){	// Audio Sample Rate change
			
				EP932_Audio_reg_config();
				pEP932C_Registers->Audio_change = 0;	
			}

			if(!is_Connected) {						// HDMI	not connected

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TX_State = TXS_Search_EDID;
			}
			else {
				pEP932C_Registers->HDCP_State = HDCP_Authentication_Task(is_ReceiverSense && is_Connected/*is_Hot_Plug*/);
				pEP932C_Registers->HDCP_Status = HDCP_Get_Status();
				if(pEP932C_Registers->HDCP_Status != 0)
				{
					DBG_printf(("ERROR : HDCP_Status = 0x%02X\r\n",(int)pEP932C_Registers->HDCP_Status));
					
					TXS_RollBack_HDCP();
					TXS_RollBack_Stream();
					TX_State = TXS_Search_EDID;
				}
			}
			break;
#endif

	}
}

void EP932_Video_reg_config(void)
{
	DBG_printf(("\r\n ========== EP932 Video Parameter setting ==========\r\n"));

	// Mute Control
	HDMI_Tx_Mute_Enable();

	// HDMI Mode
	if(!is_Cap_HDMI ) {
		HDMI_Tx_DVI();	// Set to DVI output mode (The Info Frame and Audio Packets would not be send)
	}
	else {
		HDMI_Tx_HDMI();	// Set to HDMI output mode
	}

	///////////////////////////////////////////////////////////////////////
	// Update Video Params
	//
	DBG_printf(("EP932 Video_Interface[0] = 0x%02X\r\n",(int)pEP932C_Registers->Video_Interface[0]));
	DBG_printf(("EP932 Video_Interface[1] = 0x%02X\r\n",(int)pEP932C_Registers->Video_Interface[1]));
	DBG_printf(("EP932 Video_Output_Format = 0x%02X \r\n",(int)pEP932C_Registers->Video_Output_Format ));
	DBG_printf(("EP932 Video_Input_Format[0] = 0x%02X \r\n",(int)pEP932C_Registers->Video_Input_Format[0] ));
			
	// Video Interface
	Video_Params.Interface = pEP932C_Registers->Video_Interface[0];
		
	// Video Timing
	if(pEP932C_Registers->Video_Input_Format[0]) { 
		// Manul set the Video Timing
		if(pEP932C_Registers->Video_Input_Format[0] < 35) {
			Video_Params.VideoSettingIndex = pEP932C_Registers->Video_Input_Format[0];
		}
		else{
			DBG_printf(("ERROR: EP932 Video_Input_Format[0] = 0x%02X OVER CEA-861-B SPEC\r\n",(int)pEP932C_Registers->Video_Input_Format[0]));
		}
	} 
		
	// Select Sync Mode
	Video_Params.SyncMode = (pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__SYNC) >> 2;
		
	// Select Color Space
	switch(pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__COLOR) {
		default:
		case EP932E_Video_Interface_Setting_1__COLOR__Auto:
			switch(Video_Params.VideoSettingIndex) {
				case  4: case  5: case 16: case 19: case 20: case 31: case 32: 
				case 33: case 34: 													// HD Timing
					Video_Params.ColorSpace = COLORSPACE_709;
					break;
		
				default:
					if(Video_Params.VideoSettingIndex) { 							// SD Timing
						Video_Params.ColorSpace = COLORSPACE_601;
					}
					else {															// IT Timing
						Video_Params.ColorSpace = COLORSPACE_709;
					}
			}
			break;
	}
		
	// Set In and Output Color Format	
	switch(pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__VIN_FMT) {
		
		default:
		case EP932E_Video_Interface_Setting_1__VIN_FMT__RGB:	 	// input is RGB
			Video_Params.FormatIn = COLORFORMAT_RGB;
			Video_Params.FormatOut = COLORFORMAT_RGB;
			break;
		
		case EP932E_Video_Interface_Setting_1__VIN_FMT__YCC444: 	// input is YCC444
			Video_Params.FormatIn = COLORFORMAT_YCC444;
			if(is_Cap_YCC444) {
				Video_Params.FormatOut = COLORFORMAT_YCC444;
			}
			else {
				Video_Params.FormatOut = COLORFORMAT_RGB;
			}
			break;
	
		case EP932E_Video_Interface_Setting_1__VIN_FMT__YCC422: 	// inut is YCC422
			Video_Params.FormatIn = COLORFORMAT_YCC422;
			if(is_Cap_YCC422) {
				Video_Params.FormatOut = COLORFORMAT_YCC422;
			}
			else {
				Video_Params.FormatOut = COLORFORMAT_RGB;
			}
			break;
	}
			
	// Force Output Color Format overwrite
	switch(pEP932C_Registers->Video_Output_Format) {
		
		case 0: 	
			// Auto, don't need change setting.
			break;
												
		default:
		case 3: 	
			// Force to RGB444 output format
			Video_Params.FormatOut = COLORFORMAT_RGB;
			break;
	}
							
	// DVI mode settings overwrite
	if(!is_Cap_HDMI) {
		Video_Params.FormatOut = COLORFORMAT_RGB;
	}
					
	// AFAR
	//Video_Params.AFARate = ((pEP932C_Registers->Video_Input_Format[1] & EP932E_Video_Input_Format_1__AFAR) >> 4) | 0x08;
	Video_Params.AFARate = 0;

	// SCAN 		
	//Video_Params.SCAN = (pEP932C_Registers->Video_Input_Format[1] & EP932E_Video_Input_Format_1__SCAN);
	Video_Params.SCAN = 0;

	// Update EP932 Video Registers 
	HDMI_Tx_Video_Config(&Video_Params);

	// mute control
	HDMI_Tx_Mute_Disable();
}

void EP932_Audio_reg_config(void)
{
	DBG_printf(("\r\n ========== EP932 Audio Parameter setting ==========\r\n"));

	// Mute Control
	HDMI_Tx_AMute_Enable();

	///////////////////////////////////////////////////////////////////////
	// Update Audio Params
	//
	DBG_printf(("EP932 Audio_Interface = 0x%02X\r\n",(int)pEP932C_Registers->Audio_Interface));
	DBG_printf(("EP932 Audio_Input_Format = 0x%02X\r\n",(int)pEP932C_Registers->Audio_Input_Format));

	Audio_Params.Interface = pEP932C_Registers->Audio_Interface & 0x0F; // IIS, WS_M, WS_POL, SCK_POL
	Audio_Params.VideoSettingIndex = Video_Params.VideoSettingIndex;

	// Update Audio Channel Number
	if(EP932_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type <= PIX_FREQ_27027KHz) {
		Audio_Params.ChannelNumber = 1;
	}
	else {
		Audio_Params.ChannelNumber = min(((pEP932C_Registers->Audio_Interface & 0x70) >> 4), pEP932C_Registers->EDID_AChannel);
	}

	// Update VFS
	if(Audio_Params.VideoSettingIndex < EP932_VDO_Settings_IT_Start) {
		Audio_Params.VFS = 1;  
	}
	else {
		Audio_Params.VFS = 0;
	}

	Audio_Params.NoCopyRight = 0;
		
	// Write Frequency info (Use ADO_FREQ or Auto)
	switch( pEP932C_Registers->Audio_Input_Format & EP932E_Audio_Input_Format__ADO_FREQ ) {
		
		case EP932E_Audio_Input_Format__ADO_FREQ__32000Hz:
			Audio_Params.InputFrequency = ADSFREQ_32000Hz;
			Audio_Params.ADSRate = 0; // Disable Down Sample
			break;
		
		default:
		case EP932E_Audio_Input_Format__ADO_FREQ__44100Hz:
			Audio_Params.InputFrequency = ADSFREQ_44100Hz;
			Audio_Params.ADSRate = 0; // Disable Down Sample
			break;
		
		case EP932E_Audio_Input_Format__ADO_FREQ__48000Hz:
			Audio_Params.InputFrequency = ADSFREQ_48000Hz;
			Audio_Params.ADSRate = 0; // Disable Down Sample
			break;
		
		case EP932E_Audio_Input_Format__ADO_FREQ__88200Hz:
			Audio_Params.InputFrequency = ADSFREQ_88200Hz;
			if(pEP932C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
				Audio_Params.ADSRate = 0; // Disable Down Sample
			}
			else {
				Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
			}
			break;
		
		case EP932E_Audio_Input_Format__ADO_FREQ__96000Hz:
			Audio_Params.InputFrequency = ADSFREQ_96000Hz;
			if(pEP932C_Registers->EDID_ASFreq & 0x10) { // 96kHz
				Audio_Params.ADSRate = 0; // Disable Down Sample
			}
			else {
				if(pEP932C_Registers->EDID_ASFreq & 0x04) { // 48kHz
					Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
				}
				else {
					Audio_Params.ADSRate = 2; // Enable Down Sample 1/3
				}
			}
			break;
		
		case EP932E_Audio_Input_Format__ADO_FREQ__176400Hz:
			Audio_Params.InputFrequency = ADSFREQ_176400Hz;
			if(pEP932C_Registers->EDID_ASFreq & 0x20) { // 176kHz
				Audio_Params.ADSRate = 0; // Disable Down Sample
			}
			else {
				if(pEP932C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
					Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
				}
				else {
					Audio_Params.ADSRate = 3; // Enable Down Sample 1/4
				}
			}
			break;
	
		case EP932E_Audio_Input_Format__ADO_FREQ__192000Hz:
			Audio_Params.InputFrequency = ADSFREQ_192000Hz;
			if(pEP932C_Registers->EDID_ASFreq & 0x40) { // 192kHz
				Audio_Params.ADSRate = 0; // Disable Down Sample
			}
			else {
				if(pEP932C_Registers->EDID_ASFreq & 0x10) { // 96kHz
					Audio_Params.ADSRate = 1; // Enable Down Sample 1/2
			}
			else {
					Audio_Params.ADSRate = 3; // Enable Down Sample 1/4
				}
			}
			break;
	}
		
	// Update EP932 Audio Registers 
	HDMI_Tx_Audio_Config(&Audio_Params);

	// mute control
	HDMI_Tx_AMute_Disable();

	// clear flag
	pEP932C_Registers->Audio_change = 0;
}

void TXS_RollBack_Stream(void)
{
	DBG_printf(("\r\nState Rollback: Power Down -> [TXS_Search_EDID]\r\n"));

	// Power Down
	HDMI_Tx_Power_Down();

#if Enable_HDCP
	// Reset HDCP Info
	memset(pEP932C_Registers->HDCP_BKSV, 0x00, sizeof(pEP932C_Registers->HDCP_BKSV));
	is_HDCP_Info_BKSV_Rdy = 0;
#endif

}

#if Enable_HDCP
void TXS_RollBack_HDCP(void)
{
	DBG_printf(("\r\nState Rollback: Stop HDCP -> [TXS_Stream]\r\n"));
	
	HDCP_Stop();
	pEP932C_Registers->HDCP_Status = 0;
	pEP932C_Registers->HDCP_State = 0;
}

void ReadRiInterruptFlags(void) 
{
	EP932_Reg_Read(EP932_General_Control_2, DDC_Data, 1);	
	if(DDC_Data[0] & EP932_General_Control_2__RIF) {
		HDCP_Ext_Ri_Trigger();
		
		// Clear the interrupt flag
		DDC_Data[0] = EP932_General_Control_2__RIF;
		EP932_Reg_Write(EP932_General_Control_2, DDC_Data, 1);
	}
}
#endif

//----------------------------------------------------------------------------------------------------------------------

void  EP_HDMI_DumpMessage(unsigned char Dump_Parameter)
{
	unsigned short Temp_USHORT;
	unsigned char temp_R[2];
	unsigned char reg_addr;

	if(Dump_Parameter)
	{
		DBG_printf(("\r\n\r\n======= Dump EP932M information =======\r\n"));
	
		DBG_printf(("\r\n[EDID Data]"));
		for(Temp_USHORT = 0; Temp_USHORT < 256; ++Temp_USHORT) {
			if(Temp_USHORT%16 == 0) DBG_printf(("\r\n"));
			if(Temp_USHORT%8 == 0) DBG_printf((" "));
			DBG_printf(("0x%02X,", (int)pEP932C_Registers->Readed_EDID[Temp_USHORT] ));
		}
		DBG_printf(("\r\n"));
	
		DBG_printf(("[Revision]\r\n"));
		DBG_printf(("Version=%d.%d\r\n", (int)pEP932C_Registers->Version_Major, (int)pEP932C_Registers->Version_Minor ));

		DBG_printf(("[Video Interface 0]\r\n"));
		DBG_printf(("DK=%d, ", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__DK)?1:0) ));
		DBG_printf(("DKEN=%d, ", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__DKEN)?1:0) ));
		DBG_printf(("DSEL=%d, ", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__DSEL)?1:0) ));
		DBG_printf(("BSEL=%d, ", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__BSEL)?1:0) ));
		DBG_printf(("EDGE=%d, ", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__EDGE)?1:0) ));
		DBG_printf(("FMT12=%d\r\n", (int)((pEP932C_Registers->Video_Interface[0] & EP932E_Video_Interface_Setting_0__FMT12)?1:0) ));
	
		DBG_printf(("[Video Interface 1]\r\n"));
		DBG_printf(("COLOR=%d, ", (int)((pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__COLOR)>>4) ));
		DBG_printf(("SYNC=%d, ", (int)((pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__SYNC)>>2) ));
		DBG_printf(("VIN_FMT=%d\r\n", (int)((pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__VIN_FMT)>>0) ));
	
		DBG_printf(("[Audio Interface]\r\n"));
		DBG_printf(("CHANNEL=%d, ", (int) (pEP932C_Registers->Audio_Interface & EP932E_Audio_Interface_Setting__CHANNEL)>>4 ));
		DBG_printf(("IIS=%d, ", (int)((pEP932C_Registers->Audio_Interface & EP932E_Audio_Interface_Setting__IIS)?1:0) ));
		DBG_printf(("WS_M=%d, ", (int)((pEP932C_Registers->Audio_Interface & EP932E_Audio_Interface_Setting__WS_M)?1:0) ));
		DBG_printf(("WS_POL=%d, ", (int)((pEP932C_Registers->Audio_Interface & EP932E_Audio_Interface_Setting__WS_POL)?1:0) ));
		DBG_printf(("SCK_POL=%d\r\n", (int)((pEP932C_Registers->Audio_Interface & EP932E_Audio_Interface_Setting__SCK_POL)?1:0) ));	
	
		DBG_printf(("[Video Input Format 0]\r\n"));
		DBG_printf(("VIC=%d\r\n", (int)pEP932C_Registers->Video_Input_Format[0] ));	
	}

	DBG_printf(("\r\n[EP932 Register value]\r\n"));
	DBG_printf(("    -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F\r\n"));
	DBG_printf(("    -----------------------------------------------"));
	for(reg_addr = 0; reg_addr<=0x88; reg_addr++)
	{
		EP932_Reg_Read(reg_addr, temp_R, 1);

		if(reg_addr%16 == 0)
		{
			DBG_printf(("\r\n%02X| ",(int)((reg_addr/16)<<4)));
		}
		DBG_printf(("%02X ",(int)temp_R[0]));

	}
	DBG_printf(("\r\n"));
	DBG_printf(("    -----------------------------------------------\r\n\n"));
}


