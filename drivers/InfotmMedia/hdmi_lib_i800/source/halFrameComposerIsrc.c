/*
 * @file:halFrameComposerIsrc.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerIsrc.h"
#include "access.h"
#include "hdmi_log.h"

/* register offset */
static const IM_UINT8 FC_ISRC1_0 = 0x92;
static const IM_UINT8 FC_ISRC1_1 = 0xA2;
static const IM_UINT8 FC_ISRC1_16 = 0x93;
static const IM_UINT8 FC_ISRC2_0 = 0xB2;
static const IM_UINT8 FC_ISRC2_15 = 0xA3;
/* bit shifts */
static const IM_UINT8 ISRC_CONT = 0;
static const IM_UINT8 ISRC_VALID = 1;
static const IM_UINT8 ISRC_STATUS = 2;

void halFrameComposerIsrc_Status(IM_UINT16 baseAddr, IM_UINT8 code)
{
	LOG_TRACE1(code);
	access_CoreWrite(code, (baseAddr + FC_ISRC1_0), ISRC_STATUS, 3);
}

void halFrameComposerIsrc_Valid(IM_UINT16 baseAddr, IM_UINT8 validity)
{
	LOG_TRACE1(validity);
	access_CoreWrite((validity ? 1 : 0), (baseAddr + FC_ISRC1_0), ISRC_VALID, 1);
}

void halFrameComposerIsrc_Cont(IM_UINT16 baseAddr, IM_UINT8 isContinued)
{
	LOG_TRACE1(isContinued);
	access_CoreWrite((isContinued ? 1 : 0), (baseAddr + FC_ISRC1_0), ISRC_CONT,
			1);
}

void halFrameComposerIsrc_Isrc1Codes(IM_UINT16 baseAddr, IM_UINT8 * codes, IM_UINT8 length)
{
	IM_UINT8 c = 0;
	LOG_TRACE1(codes[0]);
	if (length > (FC_ISRC1_1 - FC_ISRC1_16 + 1))
	{
		length = (FC_ISRC1_1 - FC_ISRC1_16 + 1);
		LOG_WARNING("ISRC1 Codes Truncated");
	}

	for (c = 0; c < length; c++)
		access_CoreWriteByte(codes[c], (baseAddr + FC_ISRC1_1 - c));
}

void halFrameComposerIsrc_Isrc2Codes(IM_UINT16 baseAddr, IM_UINT8 * codes, IM_UINT8 length)
{
	IM_UINT8 c = 0;
	LOG_TRACE1(codes[0]);
	if (length > (FC_ISRC2_0 - FC_ISRC2_15 + 1))
	{
		length = (FC_ISRC2_0 - FC_ISRC2_15 + 1);
		LOG_WARNING("ISRC2 Codes Truncated");
	}

	for (c = 0; c < length; c++)
		access_CoreWriteByte(codes[c], (baseAddr + FC_ISRC2_0 - c));
}