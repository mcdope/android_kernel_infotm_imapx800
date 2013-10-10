/*
 * @file:halFrameComposerVideo.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerVideo.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_INVIDCONF = 0x00;
static const IM_UINT8 FC_INHACTV0 = 0x01;
static const IM_UINT8 FC_INHACTV1 = 0x02;
static const IM_UINT8 FC_INHBLANK0 = 0x03;
static const IM_UINT8 FC_INHBLANK1 = 0x04;
static const IM_UINT8 FC_INVACTV0 = 0x05;
static const IM_UINT8 FC_INVACTV1 = 0x06;
static const IM_UINT8 FC_INVBLANK = 0x07;
static const IM_UINT8 FC_HSYNCINDELAY0 = 0x08;
static const IM_UINT8 FC_HSYNCINDELAY1 = 0x09;
static const IM_UINT8 FC_HSYNCINWIDTH0 = 0x0A;
static const IM_UINT8 FC_HSYNCINWIDTH1 = 0x0B;
static const IM_UINT8 FC_VSYNCINDELAY = 0x0C;
static const IM_UINT8 FC_VSYNCINWIDTH = 0x0D;
static const IM_UINT8 FC_INFREQ0 = 0x0E;
static const IM_UINT8 FC_INFREQ1 = 0x0F;
static const IM_UINT8 FC_INFREQ2 = 0x10;
static const IM_UINT8 FC_CTRLDUR = 0x11;
static const IM_UINT8 FC_EXCTRLDUR = 0x12;
static const IM_UINT8 FC_EXCTRLSPAC = 0x13;
static const IM_UINT8 FC_CH0PREAM = 0x14;
static const IM_UINT8 FC_CH1PREAM = 0x15;
static const IM_UINT8 FC_CH2PREAM = 0x16;
static const IM_UINT8 FC_PRCONF = 0xE0;

void halFrameComposerVideo_HdcpKeepout(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 7, 1);
}

void halFrameComposerVideo_VSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 6, 1);
}

void halFrameComposerVideo_HSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 5, 1);
}

void halFrameComposerVideo_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 4, 1);
}

void halFrameComposerVideo_DviOrHdmi(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	/* 1: HDMI; 0: DVI */
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 3, 1);
}

void halFrameComposerVideo_VBlankOsc(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 1, 1);
}

void halFrameComposerVideo_Interlaced(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + FC_INVIDCONF), 0, 1);
}

void halFrameComposerVideo_HActive(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 12-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_INHACTV0));
	access_CoreWrite((u8) (value >> 8), (baseAddr + FC_INHACTV1), 0, 4);
}

void halFrameComposerVideo_HBlank(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 10-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_INHBLANK0));
	access_CoreWrite((u8) (value >> 8), (baseAddr + FC_INHBLANK1), 0, 2);
}

void halFrameComposerVideo_VActive(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 11-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_INVACTV0));
	access_CoreWrite((u8) (value >> 8), (baseAddr + FC_INVACTV1), 0, 3);
}

void halFrameComposerVideo_VBlank(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 8-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_INVBLANK));
}

void halFrameComposerVideo_HSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 11-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_HSYNCINDELAY0));
	access_CoreWrite((u8) (value >> 8), (baseAddr + FC_HSYNCINDELAY1), 0, 3);
}

void halFrameComposerVideo_HSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 9-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_HSYNCINWIDTH0));
	access_CoreWrite((u8) (value >> 8), (baseAddr + FC_HSYNCINWIDTH1), 0, 1);
}

void halFrameComposerVideo_VSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 8-bit width */
	access_CoreWriteByte((u8) (value), (baseAddr + FC_VSYNCINDELAY));
}

void halFrameComposerVideo_VSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWrite((u8) (value), (baseAddr + FC_VSYNCINWIDTH), 0, 6);
}

void halFrameComposerVideo_RefreshRate(IM_UINT16 baseAddr, IM_UINT32 value)
{
	LOG_TRACE1(value);
	/* 20-bit width */
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + FC_INFREQ0));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + FC_INFREQ1));
	access_CoreWrite((u8) (value >> 16), (baseAddr + FC_INFREQ2), 0, 4);
}

void halFrameComposerVideo_ControlPeriodMinDuration(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + FC_CTRLDUR));
}

void halFrameComposerVideo_ExtendedControlPeriodMinDuration(IM_UINT16 baseAddr,
		IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + FC_EXCTRLDUR));
}

void halFrameComposerVideo_ExtendedControlPeriodMaxSpacing(IM_UINT16 baseAddr,
		IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + FC_EXCTRLSPAC));
}

void halFrameComposerVideo_PreambleFilter(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel)
{
	LOG_TRACE1(value);
	if (channel == 0)
		access_CoreWriteByte(value, (baseAddr + FC_CH0PREAM));
	else if (channel == 1)
		access_CoreWrite(value, (baseAddr + FC_CH1PREAM), 0, 6);
	else if (channel == 2)
		access_CoreWrite(value, (baseAddr + FC_CH2PREAM), 0, 6);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerVideo_PixelRepetitionInput(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_PRCONF), 4, 4);
}
