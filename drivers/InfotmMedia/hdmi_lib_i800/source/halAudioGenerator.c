/*
 * hal_audio_generator.c
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#include "halAudioGenerator.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 AG_SWRSTZ = 0x00;
static const IM_UINT8 AG_MODE = 0x01;
static const IM_UINT8 AG_INCLEFT0 = 0x02;
static const IM_UINT8 AG_INCLEFT1 = 0x03;
static const IM_UINT8 AG_INCRIGHT0 = 0x04;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS2 = 0x12;
static const IM_UINT8 AG_INCRIGHT1 = 0x05;
static const IM_UINT8 AG_SPDIF_CONF = 0x16;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS1 = 0x11;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS0 = 0x10;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS3 = 0x13;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS5 = 0x15;
static const IM_UINT8 AG_SPDIF_AUDSCHNLS4 = 0x14;
static const IM_UINT8 AG_HBR_CONF = 0x17;
static const IM_UINT8 AG_HBR_CLKDIV0 = 0x18;
static const IM_UINT8 AG_HBR_CLKDIV1 = 0x19;
static const IM_UINT8 AG_GPA_CONF0 = 0x1A;
static const IM_UINT8 AG_GPA_CONF1 = 0x1B;
static const IM_UINT8 AG_GPA_SAMPLEVALID = 0x1C;
static const IM_UINT8 AG_GPA_CHNUM1 = 0x1D;
static const IM_UINT8 AG_GPA_CHNUM2 = 0x1E;
static const IM_UINT8 AG_GPA_CHNUM3 = 0x1F;
static const IM_UINT8 AG_GPA_USERBIT = 0x20;
static const IM_UINT8 AG_GPA_SAMPLE_LSB = 0x21;
static const IM_UINT8 AG_GPA_SAMPLE_MSB = 0x22;
static const IM_UINT8 AG_GPA_SAMPLE_DIFF = 0x23;
static const IM_UINT8 AG_GPA_INT = 0x24;

void halAudioGenerator_SwReset(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	/* active low */
	access_CoreWrite(bit ? 0 : 1, baseAddress + AG_SWRSTZ, 0, 1);
}

void halAudioGenerator_I2sMode(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_MODE, 0, 3);
}

void halAudioGenerator_FreqIncrementLeft(IM_UINT16 baseAddress, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value), baseAddress + AG_INCLEFT0);
	access_CoreWriteByte(value >> 8, baseAddress + AG_INCLEFT1);
}

void halAudioGenerator_FreqIncrementRight(IM_UINT16 baseAddress, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value), baseAddress + AG_INCRIGHT0);
	access_CoreWriteByte(value >> 8, baseAddress + AG_INCRIGHT1);
}

void halAudioGenerator_IecCgmsA(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS0, 1, 2);
}

void halAudioGenerator_IecCopyright(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_SPDIF_AUDSCHNLS0, 0, 1);
}

void halAudioGenerator_IecCategoryCode(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, baseAddress + AG_SPDIF_AUDSCHNLS1);
}

void halAudioGenerator_IecPcmMode(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS2, 4, 3);
}

void halAudioGenerator_IecSource(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS2, 0, 4);
}

void halAudioGenerator_IecChannelRight(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channelNo)
{
	LOG_TRACE1(value);
	switch (channelNo)
	{
		case 0:
			access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS3, 4, 4);
			break;
		case 1:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM1, 4, 4);
			break;
		case 2:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM2, 4, 4);
			break;
		case 3:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM3, 4, 4);
			break;
		default:
			LOG_ERROR("wrong channel number");
			break;
	}
	
}

void halAudioGenerator_IecChannelLeft(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channelNo)
{
	LOG_TRACE1(value);
	switch (channelNo)
	{
		case 0:
			access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS3, 0, 4);
			break;
		case 1:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM1, 0, 4);
			break;
		case 2:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM2, 0, 4);
			break;
		case 3:
			access_CoreWrite(value, baseAddress + AG_GPA_CHNUM3, 0, 4);
			break;
		default:
			LOG_ERROR("wrong channel number");
			break;
	}	
}

void halAudioGenerator_IecClockAccuracy(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS4, 4, 2);
}

void halAudioGenerator_IecSamplingFreq(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS4, 0, 4);
}

void halAudioGenerator_IecOriginalSamplingFreq(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS5, 4, 4);
}

void halAudioGenerator_IecWordLength(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_SPDIF_AUDSCHNLS5, 0, 4);
}

void halAudioGenerator_UserRight(IM_UINT16 baseAddress, IM_UINT8 bit, IM_UINT8 channelNo)
{
	LOG_TRACE1(bit);
	switch (channelNo)
	{
		case 0:
			access_CoreWrite(bit, baseAddress + AG_SPDIF_CONF, 3, 1);
			break;
		case 1:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 1, 1);
			break;
		case 2:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 2, 1);
			break;
		case 3:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 5, 1);
			break;
		default:
			LOG_ERROR("wrong channel number");
			break;
	}
}

void halAudioGenerator_UserLeft(IM_UINT16 baseAddress, IM_UINT8 bit, IM_UINT8 channelNo)
{
	LOG_TRACE1(bit);
	switch (channelNo)
	{
		case 0:
			access_CoreWrite(bit, baseAddress + AG_SPDIF_CONF, 2, 1);
			break;
		case 1:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 0, 1);
			break;
		case 2:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 2, 1);
			break;
		case 3:
			access_CoreWrite(bit, baseAddress + AG_GPA_USERBIT, 4, 1);
			break;
		default:
			LOG_ERROR("wrong channel number");
			break;
	}
}

void halAudioGenerator_SpdifValidity(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_SPDIF_CONF, 1, 1);
}

void halAudioGenerator_SpdifEnable(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_SPDIF_CONF, 0, 1);
}

void halAudioGenerator_HbrEnable(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_HBR_CONF, 0, 1);
}

void halAudioGenerator_HbrDdrEnable(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_HBR_CONF, 1, 1);
}

void halAudioGenerator_HbrDdrChannel(IM_UINT16 baseAddress, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddress + AG_HBR_CONF, 2, 1);
}

void halAudioGenerator_HbrBurstLength(IM_UINT16 baseAddress, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddress + AG_HBR_CONF, 3, 4);
}

void halAudioGenerator_HbrClockDivider(IM_UINT16 baseAddress, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 9-bit width */
	access_CoreWriteByte((u8) (value), baseAddress + AG_HBR_CLKDIV0);
	access_CoreWrite(value >> 8, baseAddress + AG_HBR_CLKDIV1, 0, 1);
}

void halAudioGenerator_UseLookUpTable(IM_UINT16 baseAddress, IM_UINT8 value)
{
	access_CoreWrite(value, baseAddress + AG_GPA_CONF0, 5, 1);
}

void halAudioGenerator_GpaReplyLatency(IM_UINT16 baseAddress, IM_UINT8 value)
{
	access_CoreWrite(value, baseAddress + AG_GPA_CONF0, 0, 2);
}

void halAudioGenerator_ChannelSelect(IM_UINT16 baseAddress, IM_UINT8 enable, IM_UINT8 channel)
{
	LOG_TRACE2(channel, enable);
	access_CoreWrite(enable, baseAddress + AG_GPA_CONF1, channel, 1);
}

void halAudioGenerator_GpaSampleValid(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channelNo)
{
		access_CoreWrite(value, baseAddress + AG_GPA_SAMPLEVALID, channelNo, 1);	
}
