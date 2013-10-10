/*
 * @file:halFrameComposerPackets.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerPackets.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_CTRLQHIGH = 0x73;
static const IM_UINT8 FC_CTRLQLOW = 0x74;
static const IM_UINT8 FC_DATAUTO1 = 0xB4;
static const IM_UINT8 FC_DATAUTO2 = 0xB5;
static const IM_UINT8 FC_DATAUTO3 = 0xB7;
static const IM_UINT8 FC_DATMAN = 0xB6;
static const IM_UINT8 FC_DATAUTO0 = 0xB3;
/* bit shifts */
static const IM_UINT8 FRAME_INTERPOLATION = 0;
static const IM_UINT8 FRAME_PER_PACKETS = 4;
static const IM_UINT8 LINE_SPACING = 0;

void halFrameComposerPackets_QueuePriorityHigh(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_CTRLQHIGH), 0, 5);
}

void halFrameComposerPackets_QueuePriorityLow(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_CTRLQLOW), 0, 5);
}

void halFrameComposerPackets_MetadataFrameInterpolation(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_DATAUTO1), FRAME_INTERPOLATION, 4);
}

void halFrameComposerPackets_MetadataFramesPerPacket(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_DATAUTO2), FRAME_PER_PACKETS, 4);
}

void halFrameComposerPackets_MetadataLineSpacing(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + FC_DATAUTO2), LINE_SPACING, 4);
}

void halFrameComposerPackets_AutoSend(IM_UINT16 baseAddr, IM_UINT8 enable, IM_UINT8 mask)
{
	LOG_TRACE2(enable, mask);
	access_CoreWrite((enable ? 1 : 0), (baseAddr + FC_DATAUTO0), mask, 1);
}

void halFrameComposerPackets_ManualSend(IM_UINT16 baseAddr, IM_UINT8 mask)
{
	LOG_TRACE1(mask);
	access_CoreWrite(1, (baseAddr + FC_DATMAN), mask, 1);
}

void halFrameComposerPackets_DisableAll(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWriteByte((~(BIT(ACP_TX) | BIT(ISRC1_TX) | BIT(ISRC2_TX)
			| BIT(SPD_TX) | BIT(VSD_TX))) & access_CoreReadByte
			(baseAddr + FC_DATAUTO0), (baseAddr + FC_DATAUTO0));
}
