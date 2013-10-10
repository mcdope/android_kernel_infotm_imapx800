/*
 * @file:halFrameComposerAudioInfo.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerAudioInfo.h"
#include "access.h"
#include "hdmi_log.h"

static const IM_UINT8 FC_AUDICONF0 = 0X25;
static const IM_UINT8 FC_AUDICONF1 = 0X26;
static const IM_UINT8 FC_AUDICONF2 = 0X27;
static const IM_UINT8 FC_AUDICONF3 = 0X28;
static const IM_UINT8 CHANNEL_COUNT = 4; //sam : 0 ? Differences between hdmi controller data book & CEA-861-D spec
static const IM_UINT8 CODING_TYPE = 0; //sam : 4 ? 
static const IM_UINT8 SAMPLING_SIZE = 4; // sam : 0 ?
static const IM_UINT8 SAMPLING_FREQ = 0; // sam : 2 ? 
static const IM_UINT8 CHANNEL_ALLOCATION = 0;
static const IM_UINT8 LEVEL_SHIFT_VALUE = 0;
static const IM_UINT8 DOWN_MIX_INHIBIT = 4;

void halFrameComposerAudioInfo_ChannelCount(IM_UINT16 baseAddr, IM_UINT8 noOfChannels)
{
	LOG_TRACE1(noOfChannels);
	access_CoreWrite(noOfChannels, baseAddr + FC_AUDICONF0, CHANNEL_COUNT, 3);
}

void halFrameComposerAudioInfo_SampleFreq(IM_UINT16 baseAddr, IM_UINT8 sf)
{
	LOG_TRACE1(sf);
	access_CoreWrite(sf, baseAddr + FC_AUDICONF1, SAMPLING_FREQ, 3);
}

void halFrameComposerAudioInfo_AllocateChannels(IM_UINT16 baseAddr, IM_UINT8 ca)
{
	LOG_TRACE1(ca);
	access_CoreWriteByte(ca, baseAddr + FC_AUDICONF2);
}

void halFrameComposerAudioInfo_LevelShiftValue(IM_UINT16 baseAddr, IM_UINT8 lsv)
{
	LOG_TRACE1(lsv);
	access_CoreWrite(lsv, baseAddr + FC_AUDICONF3, LEVEL_SHIFT_VALUE, 4);
}

void halFrameComposerAudioInfo_DownMixInhibit(IM_UINT16 baseAddr, IM_UINT8 prohibited)
{
	LOG_TRACE1(prohibited);
	access_CoreWrite((prohibited ? 1 : 0), baseAddr + FC_AUDICONF3,
			DOWN_MIX_INHIBIT, 1);
}

void halFrameComposerAudioInfo_CodingType(IM_UINT16 baseAddr, IM_UINT8 codingType)
{
	LOG_TRACE1(codingType);
	access_CoreWrite(codingType, baseAddr + FC_AUDICONF0, CODING_TYPE, 4);
}

void halFrameComposerAudioInfo_SamplingSize(IM_UINT16 baseAddr, IM_UINT8 ss)
{
	LOG_TRACE1(ss);
	access_CoreWrite(ss, baseAddr + FC_AUDICONF1, SAMPLING_SIZE, 2);
}
