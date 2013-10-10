/*
 * halAudioGpa.c
 *
 * Created on Oct. 30th 2010
 * Synopsys Inc.
 */
#include "halAudioGpa.h"
#include "access.h"
#include "hdmi_log.h"

static const IM_UINT16 GP_CONF0 = 0x00;
static const IM_UINT16 GP_CONF1 = 0x01;
static const IM_UINT16 GP_CONF2 = 0x02;
static const IM_UINT16 GP_STAT  = 0x03;

static const IM_UINT16 GP_MASK  = 0x06;
static const IM_UINT16 GP_POL   = 0x05;

void halAudioGpa_ResetFifo(IM_UINT16 baseAddress)
{
 	LOG_TRACE();
	access_CoreWrite(1, baseAddress + GP_CONF0, 0, 1);
}

void halAudioGpa_ChannelEnable(IM_UINT16 baseAddress, IM_UINT8 enable, IM_UINT8 channel)
{
	LOG_TRACE();
	access_CoreWrite(enable, baseAddress + GP_CONF1, channel, 1);
}

void halAudioGpa_HbrEnable(IM_UINT16 baseAddress, IM_UINT8 enable)
{
	IM_INT32 i = 0;
 	LOG_TRACE();
	access_CoreWrite(enable, baseAddress + GP_CONF2, 0, 1);
	/* 8 channels must be enabled */
	if (enable == 1)
	{
		for (i = 0; i < 8; i++)
			halAudioGpa_ChannelEnable(baseAddress, i, 1);
	}
}

IM_UINT8 halAudioGpa_InterruptStatus(IM_UINT16 baseAddress)
{
	LOG_TRACE();
	return access_CoreRead(baseAddress + GP_STAT, 0, 2);
}

void halAudioGpa_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE();
	access_CoreWrite(value, baseAddress + GP_MASK, 0, 2);
}

void halAudioGpa_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE();
	access_CoreWrite(value, baseAddress + GP_POL, 0, 2);
}
