/*
 * @file:halFrameComposerVsd.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerVsd.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_VSDIEEEID0 = 0x29;
static const IM_UINT8 FC_VSDIEEEID1 = 0x30;
static const IM_UINT8 FC_VSDIEEEID2 = 0x31;
static const IM_UINT8 FC_VSDPAYLOAD0 = 0x32;

void halFrameComposerVsd_VendorOUI(IM_UINT16 baseAddr, IM_UINT32 id)
{
	LOG_TRACE1(id);
	access_CoreWriteByte(id, (baseAddr + FC_VSDIEEEID0));
	access_CoreWriteByte(id >> 8, (baseAddr + FC_VSDIEEEID1));
	access_CoreWriteByte(id >> 16, (baseAddr + FC_VSDIEEEID2));
}

IM_UINT8 halFrameComposerVsd_VendorPayload(IM_UINT16 baseAddr, const IM_UINT8 * data,
		IM_UINT16 length)
{
	const IM_UINT16 size = 24;
	IM_UINT32 i = 0;
	LOG_TRACE();
	if (data == 0)
	{
		LOG_WARNING("invalid parameter");
		return 1;
	}
	if (length > size)
	{
		length = size;
		LOG_WARNING("vendor payload truncated");
	}
	for (i = 0; i < length; i++)
	{
		access_CoreWriteByte(data[i], (baseAddr + FC_VSDPAYLOAD0 + i));
	}
	return 0;
}
