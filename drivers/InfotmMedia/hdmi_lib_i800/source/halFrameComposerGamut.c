/*
 * @file:halFrameComposerGamut.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerGamut.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_GMD_STAT = 0x00;
static const IM_UINT8 FC_GMD_EN = 0x01;
static const IM_UINT8 FC_GMD_UP = 0x02;
static const IM_UINT8 FC_GMD_CONF = 0x03;
static const IM_UINT8 FC_GMD_HB = 0x04;
static const IM_UINT8 FC_GMD_PB0 = 0x05;
static const IM_UINT8 FC_GMD_PB27 = 0x20;
/* bit shifts */
static const IM_UINT8 GMD_ENABLE_TX = 0;
static const IM_UINT8 GMD_ENABLE_TX_WIDTH = 1;
static const IM_UINT8 GMD_UPDATE = 0;
static const IM_UINT8 GMD_UPDATE_WIDTH = 1;
static const IM_UINT8 PACKET_LINE_SPACING = 0;
static const IM_UINT8 PACKET_LINE_SPACING_WIDTH = 4;
static const IM_UINT8 PACKETS_PER_FRAME = 4;
static const IM_UINT8 PACKETS_PER_FRAME_WIDTH = 4;
static const IM_UINT8 AFFECTED_SEQ_NO = 0;
static const IM_UINT8 AFFECTED_SEQ_NO_WIDTH = 4;
static const IM_UINT8 GMD_PROFILE = 4;
static const IM_UINT8 GMD_PROFILE_WIDTH = 3;
static const IM_UINT8 GMD_CURR_SEQ = 0;
static const IM_UINT8 GMD_PACK_SEQ = 4;
static const IM_UINT8 GMD_NO_CURR_GBD = 7;

void halFrameComposerGamut_Profile(IM_UINT16 baseAddr, IM_UINT8 profile)
{
	LOG_TRACE1(profile);
	access_CoreWrite(profile, baseAddr + FC_GMD_HB, GMD_PROFILE,
			GMD_PROFILE_WIDTH);
}

void halFrameComposerGamut_AffectedSeqNo(IM_UINT16 baseAddr, IM_UINT8 no)
{
	LOG_TRACE1(no);
	access_CoreWrite(no, baseAddr + FC_GMD_HB, AFFECTED_SEQ_NO,
			AFFECTED_SEQ_NO_WIDTH);
}

void halFrameComposerGamut_PacketsPerFrame(IM_UINT16 baseAddr, IM_UINT8 packets)
{
	LOG_TRACE1(packets);
	access_CoreWrite(packets, baseAddr + FC_GMD_CONF, PACKETS_PER_FRAME,
			PACKETS_PER_FRAME_WIDTH);
}

void halFrameComposerGamut_PacketLineSpacing(IM_UINT16 baseAddr, IM_UINT8 lineSpacing)
{
	LOG_TRACE1(lineSpacing);
	access_CoreWrite(lineSpacing, baseAddr + FC_GMD_CONF, PACKET_LINE_SPACING,
			PACKET_LINE_SPACING_WIDTH);
}

void halFrameComposerGamut_Content(IM_UINT16 baseAddr, const IM_UINT8 * content, IM_UINT8 length)
{
	IM_UINT8 c = 0;
	LOG_TRACE1(content[0]);
	if (length > (FC_GMD_PB27 - FC_GMD_PB0 + 1))
	{
		length = (FC_GMD_PB27 - FC_GMD_PB0 + 1);
		LOG_WARNING("Gamut Content Truncated");
	}

	for (c = 0; c < length; c++)
		access_CoreWriteByte(content[c], baseAddr + c + FC_GMD_PB0);
}

void halFrameComposerGamut_EnableTx(IM_UINT16 baseAddr, IM_UINT8 enable)
{
	LOG_TRACE1(enable);
	access_CoreWrite(enable, baseAddr + FC_GMD_EN, GMD_ENABLE_TX, 1);
}

void halFrameComposerGamut_UpdatePacket(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWrite(1, baseAddr + FC_GMD_UP, GMD_UPDATE, 1);
}

IM_UINT8 halFrameComposerGamut_CurrentSeqNo(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead(baseAddr + FC_GMD_STAT, GMD_CURR_SEQ, 4);
}

IM_UINT8 halFrameComposerGamut_PacketSeq(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead(baseAddr + FC_GMD_STAT, GMD_PACK_SEQ, 2);
}

IM_UINT8 halFrameComposerGamut_NoCurrentGbd(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead(baseAddr + FC_GMD_STAT, GMD_NO_CURR_GBD, 1);
}
