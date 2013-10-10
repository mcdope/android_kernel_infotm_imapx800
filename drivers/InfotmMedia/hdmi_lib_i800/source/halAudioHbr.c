/*
 * hal_audio_hbr.c
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#include "halAudioHbr.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 AUD_CONF0_HBR = 0x00;
static const IM_UINT8 AUD_HBR_STATUS = 0x01;
static const IM_UINT8 AUD_HBRINT = 0x02;
static const IM_UINT8 AUD_HBRPOL = 0x03;
static const IM_UINT8 AUD_HBRMASK = 0x04;

void halAudioHbr_ResetFifo(IM_UINT16 baseAddress)
{
	LOG_TRACE();
	access_CoreWrite(1, baseAddress + AUD_CONF0_HBR, 7, 1);
}

void halAudioHbr_Select(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AUD_CONF0_HBR, 2, 1);
}

void halAudioHbr_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_HBRMASK, 0, 3);
}

void halAudioHbr_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AUD_HBRPOL, 0, 3);
}
