/*
 * hal_audio_i2s.c
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#include "halAudioI2s.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 AUD_CONF0 = 0x00;
static const IM_UINT8 AUD_CONF1 = 0x01;
static const IM_UINT8 AUD_INT = 0x02;

void halAudioI2s_ResetFifo(IM_UINT16 baseAddress)
{
	LOG_TRACE();
	access_CoreWrite(1, baseAddress + AUD_CONF0, 7, 1);
}

void halAudioI2s_Select(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AUD_CONF0, 5, 1);
}

void halAudioI2s_DataEnable(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_CONF0, 0, 4);
}

void halAudioI2s_DataMode(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_CONF1, 5, 3);
}

void halAudioI2s_DataWidth(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_CONF1, 0, 5);
}

void halAudioI2s_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_INT, 2, 2);
}

void halAudioI2s_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_INT, 0, 2);
}

