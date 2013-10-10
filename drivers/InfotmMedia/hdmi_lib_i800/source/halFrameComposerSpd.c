/*
 * @file:halFrameComposerSpd.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerSpd.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_SPDVENDORNAME0 = 0x4A;
static const IM_UINT8 FC_SPDPRODUCTNAME0 = 0x52;
static const IM_UINT8 FC_SPDDEVICEINF = 0x62;

void halFrameComposerSpd_VendorName(IM_UINT16 baseAddr, const IM_UINT8 * data,
		IM_UINT16 length)
{
	IM_UINT16 i = 0;
	LOG_TRACE();
	for (i = 0; i < length; i++)
	{
		access_CoreWriteByte(data[i], (baseAddr + FC_SPDVENDORNAME0 + i));
	}
}
void halFrameComposerSpd_ProductName(IM_UINT16 baseAddr, const IM_UINT8 * data,
		IM_UINT16 length)
{
	IM_UINT16 i = 0;
	LOG_TRACE();
	for (i = 0; i < length; i++)
	{
		access_CoreWriteByte(data[i], (baseAddr + FC_SPDPRODUCTNAME0 + i));
	}
}

void halFrameComposerSpd_SourceDeviceInfo(IM_UINT16 baseAddr, IM_UINT8 code)
{
	LOG_TRACE1(code);
	access_CoreWriteByte(code, (baseAddr + FC_SPDDEVICEINF));
}

