/*
 * halAudioSpdif.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#include "halAudioSpdif.h"
#include "access.h"
#include "hdmi_log.h"
/* register offsets */
static const IM_UINT8 AUD_SPDIF0 = 0x00;
static const IM_UINT8 AUD_SPDIF1 = 0x01;
static const IM_UINT8 AUD_SPDIFINT = 0x02;

void halAudioSpdif_ResetFifo(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWrite(1, baseAddr + AUD_SPDIF0, 7, 1);
}

void halAudioSpdif_NonLinearPcm(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddr + AUD_SPDIF1, 7, 1);
}

void halAudioSpdif_DataWidth(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + AUD_SPDIF1, 0, 5);
}

void halAudioSpdif_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + AUD_SPDIFINT, 2, 2);
}

void halAudioSpdif_InterruptPolarity(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + AUD_SPDIFINT, 0, 2);
}
