/*
 * @file:halMainController.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halMainController.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 MC_SFRDIV = 0x00;
static const IM_UINT8 MC_CLKDIS = 0x01;
static const IM_UINT8 MC_SWRSTZREQ = 0x02;
static const IM_UINT8 MC_FLOWCTRL = 0x04;
static const IM_UINT8 MC_PHYRSTZ = 0x05;
static const IM_UINT8 MC_LOCKONCLOCK = 0x06;
static const IM_UINT8 MC_HEACPHY_RST = 0x07;

void halMainController_SfrClockDivision(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + MC_SFRDIV), 0, 4);
}

void halMainController_HdcpClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 6, 1);
}

void halMainController_CecClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 5, 1);
}

void halMainController_ColorSpaceConverterClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 4, 1);
}

void halMainController_AudioSamplerClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 3, 1);
}

void halMainController_PixelRepetitionClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 2, 1);
}

void halMainController_TmdsClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 1, 1);
}

void halMainController_PixelClockGate(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_CLKDIS), 0, 1);
}

void halMainController_CecClockReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 6, 1);
}

void halMainController_AudioGpaReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 7, 1);
}

void halMainController_AudioHbrReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 5, 1);
}

void halMainController_AudioSpdifReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 4, 1);
}

void halMainController_AudioI2sReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 3, 1);
}

void halMainController_PixelRepetitionClockReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 2, 1);
}

void halMainController_TmdsClockReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 1, 1);
}

void halMainController_PixelClockReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_SWRSTZREQ), 0, 1);
}

void halMainController_VideoFeedThroughOff(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + MC_FLOWCTRL), 0, 1);
}

void halMainController_PhyReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	/* active low */
	access_CoreWrite(bit ? 0 : 1, (baseAddr + MC_PHYRSTZ), 0, 1);
}

void halMainController_HeacPhyReset(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	/* active high */
	access_CoreWrite(bit ? 1 : 0, (baseAddr + MC_HEACPHY_RST), 0, 1);
}

IM_UINT8 halMainController_LockOnClockStatus(IM_UINT16 baseAddr, IM_UINT8 clockDomain)
{
	LOG_TRACE1(clockDomain);
	return access_CoreRead((baseAddr + MC_LOCKONCLOCK), clockDomain, 1);
}

void halMainController_LockOnClockClear(IM_UINT16 baseAddr, IM_UINT8 clockDomain)
{
	LOG_TRACE1(clockDomain);
	access_CoreWrite(1, (baseAddr + MC_LOCKONCLOCK), clockDomain, 1);
}
