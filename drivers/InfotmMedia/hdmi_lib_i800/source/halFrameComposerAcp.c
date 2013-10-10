/*
 * @file:halFrameComposerAcp.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#include "halFrameComposerAcp.h"
#include "access.h"
#include "hdmi_log.h"
/* Frame composer ACP register offsets*/
static const IM_UINT8 FC_ACP0 = 0x75;
static const IM_UINT8 FC_ACP1 = 0x91;
static const IM_UINT8 FC_ACP16 = 0x82;

void halFrameComposerAcp_Type(IM_UINT16 baseAddr, IM_UINT8 type)
{
	LOG_TRACE1(type);
	access_CoreWriteByte(type, baseAddr + FC_ACP0);
}

void halFrameComposerAcp_TypeDependentFields(IM_UINT16 baseAddr, IM_UINT8 * fields,
		IM_UINT8 fieldsLength)
{
	IM_UINT8 c = 0;
	LOG_TRACE1(fields[0]);
	if (fieldsLength > (FC_ACP1 - FC_ACP16 + 1))
	{
		fieldsLength = (FC_ACP1 - FC_ACP16 + 1);
		LOG_WARNING("ACP Fields Truncated");
	}

	for (c = 0; c < fieldsLength; c++)
		access_CoreWriteByte(fields[c], baseAddr + FC_ACP1 - c);
}
