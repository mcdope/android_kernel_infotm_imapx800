#include <linux/delay.h>
#include <linux/gpio.h>
#include "EP932Controller.h"  // HDMI Transmiter
#include "EP932api.h"

EP932C_REGISTER_MAP EP932C_Registers;


/*
============================================================================
	need use customer main chip function 
		
	1. EP932 reset - use customer's GPIO function 

		EP_EP932M_Reset()
					
	2. EP932 IIC/DDC - use customer's IIC function

		DDC_If.c  	=> 	DDC_Write(......) , DDC_Read(.......)
		EP932_If.c	=> 	IIC_Write(......) , IIC_Read(.......)

		### customer' IIC function must can check IIC error (no ack, write error, read error) ###
	
   ============================================================================
	EP932 code process 

	1. set video interface and timing, timing need modify to fit with customer's require

		EP_HDMI_Set_Video_Timing( 4 ); //720P60Hz 

	2. set audio interface

		EP_HDMI_Set_Audio_Fmt(AUD_I2S, AUD_SF_48000Hz); // IIS input , 48KHz 

	3. initial EP932 variable and hardware reset EP932

		EP_HDMI_Init();
		
	4. 
		A. Need HDCP Function
			need use [timer] or [thread] to run EP932Controller_Task(); and EP932Controller_Timer(); every " 10ms "

			while(1)
			{
				EP932Controller_Timer();
				EP932Controller_Task();
			}

		B. Not need HDCP Function
			only need use [timer] or [thread] to run EP932Controller_Task(); every " 100ms " or less than 100ms
		
			while(1)
			{
				EP932Controller_Task();
			}


============================================================================
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EP932_EnableHPInit(void)
{
	EP932_Reg_Set_Bit(EP932_General_Control_1,EP932_General_Control_1__TSEL_HTP);
	EP932_Reg_Set_Bit(EP932_General_Control_1,EP932_General_Control_1__INT_OD);
	EP932_Reg_Clear_Bit(EP932_General_Control_1,EP932_General_Control_1__INT_POL);
	EP932_Reg_Clear_Bit(EP932_General_Control_2, EP932_General_Control_2__RIE);
	EP932_Reg_Clear_Bit(EP932_General_Control_2, EP932_General_Control_2__VIE);
	EP932_Reg_Set_Bit(EP932_General_Control_2, EP932_General_Control_2__MIE);
}

void EP_EP932M_Reset(void)
{
	///////////////////////////////////////////////////////////////
	// need use customer's GPIO to reset EP932 after video timing change
	//
	// 1. GPIO set to low level
	// 2. delay 5ms
	// 3. GPIO set to high level
	//
	///////////////////////////////////////////////////////////////
	unsigned long reg_temp;
	unsigned int i;

	reg_temp = __imapx_name_to_gpio(CONFIG_HDMI_EP932_RESET);

	if(reg_temp == IMAPX_GPIO_ERROR)
	{
		printk(KERN_ERR "EP932 get reset pin failed.\n");
		return ;
	}

	imapx_gpio_setcfg(reg_temp, IG_OUTPUT, IG_NORMAL);
	imapx_gpio_setpin(reg_temp, 0, IG_NORMAL);
	msleep(200);
	imapx_gpio_setpin(reg_temp, 1, IG_NORMAL);
}

void hdmi_main (video_timing timing)
{
	unsigned int i=0;

	/////////////////////////////////////////////////////
	//
	// set video interface and timing, timing need modify to fit with customer's require
	//
	/////////////////////////////////////////////////////
	EP_HDMI_Set_Video_Timing(timing);

	////////////////////////////////////////////////////
	//
	// set audio interface
	//
	///////////////////////////////////////////////////
	EP_HDMI_SetAudFmt(AUD_I2S, AUD_SF_44100Hz);

	//////////////////////////////////////////////////////
	//
	// initial EP932 variable and customer's GPIO + I2C (if need).
	//
	/////////////////////////////////////////////////////
	EP_HDMI_Init();
	
	//////////////////////////////////////////////////////////////////
	//
	// need to run EP932Controller_Task and EP932Controller_Timer every 10ms
	//
	/////////////////////////////////////////////////////////////////
	while(1)
	{
		i++;
		if(i>100)
			break;

		if(TX_State >= TXS_Stream)		// modify - Eric.Lu 
		{
			break;
		}

		EP932Controller_Task();
	}
}

void  EP_HDMI_SetAudFmt(HDMI_AudFmt_t  Audfmt, HDMI_AudFreq  Audfreq)
{
	if(Audfmt == AUD_I2S)
	{
		EP932C_Registers.Audio_Interface = 0x18;		// 2 channel IIS
	}
	else
	{
		EP932C_Registers.Audio_Interface = 0x10;		// SPDIF
	}
	EP932C_Registers.Audio_Input_Format = Audfreq;		// set Audio frequency
	EP932C_Registers.Audio_change = 1;					// Audio setting change flag
	
	/*
	// for Debug
	if(Audfmt == AUD_I2S)	DBG_printf(("EP932 Audio input interface is IIS - 2.0 CH, "));
	else					DBG_printf(("EP932 Audio input interface is SPDIF, "));
	
	DBG_printf(("Audio sample rate = "));
	switch(Audfreq)
	{
		case AUD_SF_32000Hz: DBG_printf(("32K Hz\r\n")); break;
		case AUD_SF_44100Hz: DBG_printf(("44.1K Hz\r\n")); break;
		case AUD_SF_48000Hz: DBG_printf(("48K Hz\r\n")); break;
		default: DBG_printf(("Unknown %d\r\n",Audfreq)); break;
	}
*/
}



void  EP_HDMI_Set_Video_Timing(video_timing Timing)
{
		DBG_printf(("\r\n\r\n"));
		DBG_printf(("##############################################\r\n"));
	
		// no skew, Dual edge - falling edge first, 12 bit, FMT12 = 0, 
		EP932C_Registers.Video_Interface[0] = 0x04 /*| BSEL_24bit*/ /*| EDGE_rising */ /*| FMT_12*/;
		DBG_printf(("Video_Interface_0 = 0x%02X \r\n",(int)EP932C_Registers.Video_Interface[0] ));
		
		// mode: DE + Hsync + Vsync , input: YUV422
		EP932C_Registers.Video_Interface[1] = 0x0; 	// DE,HS,VS, YUV422
		DBG_printf(("Video_Interface_1 = 0x%02X \r\n",(int)EP932C_Registers.Video_Interface[1] ));
	
		switch (Timing)
		{
			case HDMI_1080P:/*1920x1080p 60Hz[16:9]*/	
				DBG_printf(("TVOUT_MODE_1080P60\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x10;
				break;
				
//			case HDMI_1080I: /*1920x1080I 60Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_1080I60\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x05;
//				break;
				
			case HDMI_720P: /*1280x720p 60Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_720p\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x04;
				break;
				
			case HDMI_480P_16_9: /*720x480p 60Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_480p[16:9]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x03;
				break;
				
			case HDMI_480P_4_3: /*720x480p 60Hz[4:3]*/
				DBG_printf(("TVOUT_MODE_480p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x02;
				break;
				
//			case HDMI_480I_16_9: /*720x480I 60Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_480I[16:9]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x07;
//				break;
				
//			case HDMI_480I_4_3: /*720x480I 60Hz[4:3]*/
//				DBG_printf(("TVOUT_MODE_480I[4:3]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x06;
//				break;
				
			case HDMI_576P_16_9: /*720x576p 50Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_576p[16:9]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x12;
				break;
				
			case HDMI_576P_4_3: /*720x576p 50Hz[4:3]*/
				DBG_printf(("TVOUT_MODE_576p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x11;
				break;
				
//			case HDMI_576I_16_9: /*720x576I 50Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_576I[16:9]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x16;
//				break;
				
//			case HDMI_576I_4_3: /*720x576I 50Hz[4:3]*/
//				DBG_printf(("TVOUT_MODE_576I[4:3]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x15;
//				break;

			case HDMI_640_480: //640x480p 60Hz[4:3]:
				DBG_printf(("TVOUT_MODE_640x480p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x01;
				break;
			case HDMI_800_600: //800x600p 60Hz[4:3]:
				DBG_printf(("TVOUT_MODE_800x600p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 144;
				break;

			case HDMI_1024_768: //1024x768p 60Hz[4:3]:
				DBG_printf(("TVOUT_MODE_1024x768p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 152;
				break;
			default:
				DBG_printf(("TVOUT_MODE_Unknown : %d\r\n",Timing));
				EP932C_Registers.Video_Input_Format[0] = 0x00;
				break;
		}
	
	//===================================================================
	
		DBG_printf(("##############################################\r\n"));
	
}

void EP_HDMI_Init(void)
{
	EP932Controller_Initial(&EP932C_Registers);
}


