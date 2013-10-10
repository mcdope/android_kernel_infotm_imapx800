/*
 * @file:halFrameComposerGcp.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerGcp.h"
#include "access.h"
#include "hdmi_log.h"

/* register offset */
static const IM_UINT8 FC_GCP = 0x18;
/* bit shifts */
static const IM_UINT8 DEFAULT_PHASE = 2;
static const IM_UINT8 CLEAR_AVMUTE = 0;
static const IM_UINT8 SET_AVMUTE = 1;

void halFrameComposerGcp_DefaultPhase(IM_UINT16 baseAddr, IM_UINT8 uniform)
{
	LOG_TRACE1(uniform);
	access_CoreWrite((uniform ? 1 : 0), (baseAddr + FC_GCP), DEFAULT_PHASE, 1);
}

void halFrameComposerGcp_AvMute(IM_UINT16 baseAddr, IM_UINT8 enable)
{
	LOG_TRACE1(enable);
	access_CoreWrite((enable ? 1 : 0), (baseAddr + FC_GCP), SET_AVMUTE, 1);
	access_CoreWrite((enable ? 0 : 1), (baseAddr + FC_GCP), CLEAR_AVMUTE, 1);
}
