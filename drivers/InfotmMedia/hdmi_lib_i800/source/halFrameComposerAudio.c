/*
 * @file:halFrameComposerAudio.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerAudio.h"
#include "access.h"
#include "hdmi_log.h"

/* frame composer audio register offsets */
static const IM_UINT8 FC_AUDSCONF = 0x63;
static const IM_UINT8 FC_AUDSSTAT = 0x64;
static const IM_UINT8 FC_AUDSV = 0x65;
static const IM_UINT8 FC_AUDSU = 0x66;
static const IM_UINT8 FC_AUDSCHNLS0 = 0x67;
static const IM_UINT8 FC_AUDSCHNLS1 = 0x68;
static const IM_UINT8 FC_AUDSCHNLS2 = 0x69;
static const IM_UINT8 FC_AUDSCHNLS3 = 0x6A;
static const IM_UINT8 FC_AUDSCHNLS4 = 0x6B;
static const IM_UINT8 FC_AUDSCHNLS5 = 0x6C;
static const IM_UINT8 FC_AUDSCHNLS6 = 0x6D;
static const IM_UINT8 FC_AUDSCHNLS7 = 0x6E;
static const IM_UINT8 FC_AUDSCHNLS8 = 0x6F;

void halFrameComposerAudio_PacketSampleFlat(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCONF, 4, 4);
}

void halFrameComposerAudio_PacketLayout(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddr + FC_AUDSCONF, 0, 1);
}

void halFrameComposerAudio_ValidityRight(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel)
{
	LOG_TRACE2(bit, channel);
	if (channel < 4)
		access_CoreWrite(bit, baseAddr + FC_AUDSV, 4 + channel, 1);
	else
		LOG_ERROR("invalid channel number");
}

void halFrameComposerAudio_ValidityLeft(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel)
{
	LOG_TRACE2(bit, channel);
	if (channel < 4)
		access_CoreWrite(bit, baseAddr + FC_AUDSV, channel, 1);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerAudio_UserRight(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel)
{
	LOG_TRACE2(bit, channel);
	if (channel < 4)
		access_CoreWrite(bit, baseAddr + FC_AUDSU, 4 + channel, 1);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerAudio_UserLeft(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel)
{
	LOG_TRACE2(bit, channel);
	if (channel < 4)
		access_CoreWrite(bit, baseAddr + FC_AUDSU, channel, 1);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerAudio_IecCgmsA(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS0, 4, 2);
}

void halFrameComposerAudio_IecCopyright(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, baseAddr + FC_AUDSCHNLS0, 0, 1);
}

void halFrameComposerAudio_IecCategoryCode(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, baseAddr + FC_AUDSCHNLS1);
}

void halFrameComposerAudio_IecPcmMode(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS2, 4, 3);
}

void halFrameComposerAudio_IecSource(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS2, 0, 4);
}

void halFrameComposerAudio_IecChannelRight(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel)
{
	LOG_TRACE2(value, channel);
	if (channel == 0)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS3, 0, 4);
	else if (channel == 1)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS3, 4, 4);
	else if (channel == 2)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS4, 0, 4);
	else if (channel == 3)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS4, 4, 4);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerAudio_IecChannelLeft(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel)
{
	LOG_TRACE2(value, channel);
	if (channel == 0)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS5, 0, 4);
	else if (channel == 1)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS5, 4, 4);
	else if (channel == 2)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS6, 0, 4);
	else if (channel == 3)
		access_CoreWrite(value, baseAddr + FC_AUDSCHNLS6, 4, 4);
	else
		LOG_ERROR2("invalid channel number: ", channel);
}

void halFrameComposerAudio_IecClockAccuracy(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS7, 4, 2);
}

void halFrameComposerAudio_IecSamplingFreq(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS7, 0, 4);
}

void halFrameComposerAudio_IecOriginalSamplingFreq(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS8, 4, 4);
}

void halFrameComposerAudio_IecWordLength(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, baseAddr + FC_AUDSCHNLS8, 0, 4);
}

