/*
 * @file:control.c
 *
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#include "control.h"
#include "halMainController.h"
#include "halInterrupt.h"
#include "halIdentification.h"
#include "hdmi_log.h"
#include "error.h"

/* block offsets */
const IM_UINT16 ID_BASE_ADDR = 0x0000;
const IM_UINT16 IH_BASE_ADDR = 0x0100;
static const IM_UINT16 MC_BASE_ADDR = 0x4000;

IM_INT32 control_Initialize(IM_UINT16 baseAddr, IM_UINT8 dataEnablePolarity, IM_UINT16 pixelClock)
{
	/*  clock gate == 1 => turn off modules */
	halMainController_PixelClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_TmdsClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_PixelRepetitionClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_ColorSpaceConverterClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_AudioSamplerClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_CecClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_HdcpClockGate(baseAddr + MC_BASE_ADDR, 1);
	return TRUE;
}

IM_INT32 control_Configure(IM_UINT16 baseAddr, IM_UINT16 pClk, IM_UINT8 pRep, IM_UINT8 cRes, IM_INT32 cscOn, IM_INT32 audioOn,
		IM_INT32 cecOn, IM_INT32 hdcpOn)
{
	//sam : 1 --> csc enabled; 0 --> csc bypassed.
	halMainController_VideoFeedThroughOff(baseAddr + MC_BASE_ADDR,
				cscOn ? 1 : 0);
/*	halMainController_VideoFeedThroughOff(baseAddr + MC_BASE_ADDR,
			cscOn ? 0 : 1);*/
	/*  clock gate == 0 => turn on modules */
	halMainController_PixelClockGate(baseAddr + MC_BASE_ADDR, 0);
	halMainController_TmdsClockGate(baseAddr + MC_BASE_ADDR, 0);
	halMainController_PixelRepetitionClockGate(baseAddr
			+ MC_BASE_ADDR, (pRep > 0) ? 0 : 1);
	halMainController_ColorSpaceConverterClockGate(baseAddr
			+ MC_BASE_ADDR, cscOn ? 0 : 1);
	halMainController_AudioSamplerClockGate(baseAddr + MC_BASE_ADDR,
			audioOn ? 0 : 1);
	halMainController_CecClockGate(baseAddr + MC_BASE_ADDR,
			cecOn ? 0 : 1);
	halMainController_HdcpClockGate(baseAddr + MC_BASE_ADDR,
			hdcpOn ? 0 : 1);
	return TRUE;
}

IM_INT32 control_Standby(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	/*  clock gate == 1 => turn off modules */
	halMainController_HdcpClockGate(baseAddr + MC_BASE_ADDR, 1);
	/*  CEC is not turned off because it has to answer PINGs even in standby mode */
	halMainController_AudioSamplerClockGate(baseAddr + MC_BASE_ADDR,
			1);
	halMainController_ColorSpaceConverterClockGate(baseAddr
			+ MC_BASE_ADDR, 1);
	halMainController_PixelRepetitionClockGate(baseAddr
			+ MC_BASE_ADDR, 1);
	halMainController_PixelClockGate(baseAddr + MC_BASE_ADDR, 1);
	halMainController_TmdsClockGate(baseAddr + MC_BASE_ADDR, 1);
	return TRUE;
}

IM_INT32 control_exitStandby(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	/*  clock gate == 1 => turn off modules */
	halMainController_HdcpClockGate(baseAddr + MC_BASE_ADDR, 0);
	/*  CEC is not turned off because it has to answer PINGs even in standby mode */
	halMainController_AudioSamplerClockGate(baseAddr + MC_BASE_ADDR,
			0);
	halMainController_ColorSpaceConverterClockGate(baseAddr
			+ MC_BASE_ADDR, 0);
	halMainController_PixelRepetitionClockGate(baseAddr
			+ MC_BASE_ADDR, 0);
	halMainController_PixelClockGate(baseAddr + MC_BASE_ADDR, 0);
	halMainController_TmdsClockGate(baseAddr + MC_BASE_ADDR, 0);
	return TRUE;
}

IM_UINT8 control_Design(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halIdentification_Design(baseAddr + ID_BASE_ADDR);
}

IM_UINT8 control_Revision(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halIdentification_Revision(baseAddr + ID_BASE_ADDR);
}

IM_UINT8 control_ProductLine(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halIdentification_ProductLine(baseAddr + ID_BASE_ADDR);
}

IM_UINT8 control_ProductType(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halIdentification_ProductType(baseAddr + ID_BASE_ADDR);
}

IM_INT32 control_SupportsCore(IM_UINT16 baseAddr)
{
	/*  Line 0xA0 - HDMICTRL */
	/*  Type 0x01 - TX */
	/*  Type 0xC1 - TX with HDCP */
	return control_ProductLine(baseAddr) == 0xA0 && (control_ProductType(baseAddr) == 0x01
			|| control_ProductType(baseAddr) == 0xC1);
}

IM_INT32 control_SupportsHdcp(IM_UINT16 baseAddr)
{
	/*  Line 0xA0 - HDMICTRL */
	/*  Type 0xC1 - TX with HDCP */
	return control_ProductLine(baseAddr) == 0xA0 && control_ProductType(baseAddr) == 0xC1;
}

IM_INT32 control_InterruptMute(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	halInterrupt_Mute(baseAddr + IH_BASE_ADDR, value);
	return TRUE;
}
IM_INT32 control_InterruptCecClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	halInterrupt_CecClear(baseAddr + IH_BASE_ADDR, value);
	return TRUE;
}

IM_UINT8 control_InterruptCecState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halInterrupt_CecState(baseAddr + IH_BASE_ADDR);
}

IM_INT32 control_InterruptEdidClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	halInterrupt_I2cDdcClear(baseAddr + IH_BASE_ADDR, value);
	return TRUE;
}

IM_UINT8 control_InterruptEdidState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return halInterrupt_I2cDdcState(baseAddr + IH_BASE_ADDR);
}

IM_INT32 control_InterruptPhyClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	halInterrupt_PhyClear(baseAddr + IH_BASE_ADDR, value);
	return TRUE;
}

IM_UINT8 control_InterruptPhyState(IM_UINT16 baseAddr)
{
#if 0
	static IM_INT32 i = 0;
	if ((i % 100) == 0)
	{
		LOG_NOTICE("---------");
		LOG_NOTICE2("audio packets   ", halInterrupt_AudioPacketsState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("other packets   ", halInterrupt_OtherPacketsState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("packets overflow", halInterrupt_PacketsOverflowState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("audio sampler   ", halInterrupt_AudioSamplerState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("phy state       ", halInterrupt_PhyState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("i2c ddc state   ", halInterrupt_I2cDdcState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("cec state       ", halInterrupt_CecState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("video packetizer", halInterrupt_VideoPacketizerState(baseAddr + IH_BASE_ADDR));
		LOG_NOTICE2("i2c phy state   ", halInterrupt_I2cPhyState(baseAddr + IH_BASE_ADDR));
	}
	i += 1;
#endif
	LOG_TRACE();
	return halInterrupt_PhyState(baseAddr + IH_BASE_ADDR);
}

IM_INT32 control_InterruptClearAll(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	halInterrupt_AudioPacketsClear(baseAddr + IH_BASE_ADDR,	(u8)(-1));
	halInterrupt_OtherPacketsClear(baseAddr + IH_BASE_ADDR,	(u8)(-1));
	halInterrupt_PacketsOverflowClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	halInterrupt_AudioSamplerClear(baseAddr + IH_BASE_ADDR,	(u8)(-1));
	halInterrupt_PhyClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	halInterrupt_I2cDdcClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	halInterrupt_CecClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	halInterrupt_VideoPacketizerClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	halInterrupt_I2cPhyClear(baseAddr + IH_BASE_ADDR, (u8)(-1));
	return TRUE;
}
