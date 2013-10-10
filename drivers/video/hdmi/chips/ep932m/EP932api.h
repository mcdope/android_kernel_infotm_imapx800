#ifndef EP932_API_H
#define EP932_API_H

#include <asm/io.h>
#include <linux/i2c.h>
#include <plat/imapx.h>
#include "../../hdmi.h"

// for EP MCU =====
//#include "smbus.h"
//#include "DBG.h"
//#include "GPIO.h"
//#include "DELAY.H"
// ================

// HDCP	enable define ////////////////

#define Enable_HDCP 0

//////////////////////////////////////


// Debug define	//////////////////////

#define	DEBUG	0

#if DEBUG
#define DBG_printf(x) printk x		// enable DBG message
#else
#define DBG_printf(x)					// disable DBG message
#endif

//////////////////////////////////////


#define IIC_EP932_Addr  0x38

typedef enum {
	// Master
	SMBUS_STATUS_Success = 0x00,
	SMBUS_STATUS_Pending,//	SMBUS_STATUS_Abort,
	SMBUS_STATUS_NoAct = 0x02,
	SMBUS_STATUS_TimeOut,
	SMBUS_STATUS_ArbitrationLoss = 0x04
} SMBUS_STATUS;


typedef enum {
	AUD_I2S = 0,
	AUD_SPDIF	
}HDMI_AudFmt_t;

typedef enum {
	//AUD_None = 0,
	AUD_SF_32000Hz = 1,
	AUD_SF_44100Hz,
	AUD_SF_48000Hz,	
	//AUD_SF_88200Hz,
	//AUD_SF_96000Hz,
	//AUD_SF_176400Hz,
	//AUD_SF_192000Hz

}HDMI_AudFreq;


#ifndef min
#define min(a,b) (((a)<(b))? (a):(b))
#endif

#ifndef max
#define max(a,b) (((a)>(b))? (a):(b))
#endif


void EP_EP932M_Reset(void);
void EP_HDMI_SetAudFmt(HDMI_AudFmt_t Audfmt, HDMI_AudFreq Audfreq);
void  EP_HDMI_Set_Video_Timing(video_timing Timing);
void EP_HDMI_Init(void); 
void hdmi_main (video_timing timing);
void EP932_EnableHPInit(void);

#endif
