/*
 * @file:halInterrupt.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halInterrupt.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 IH_FC_STAT0 = 0x00;
static const IM_UINT8 IH_FC_STAT1 = 0x01;
static const IM_UINT8 IH_FC_STAT2 = 0x02;
static const IM_UINT8 IH_AS_STAT0 = 0x03;
static const IM_UINT8 IH_PHY_STAT0 = 0x04;
static const IM_UINT8 IH_I2CM_STAT0 = 0x05;
static const IM_UINT8 IH_CEC_STAT0 = 0x06;
static const IM_UINT8 IH_VP_STAT0 = 0x07;
static const IM_UINT8 IH_I2CMPHY_STAT0 = 0x08;
static const IM_UINT8 IH_MUTE = 0xFF;

IM_UINT8 halInterrupt_AudioPacketsState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_FC_STAT0);
}

void halInterrupt_AudioPacketsClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_FC_STAT0));
}

IM_UINT8 halInterrupt_OtherPacketsState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_FC_STAT1);
}

void halInterrupt_OtherPacketsClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_FC_STAT1));
}

IM_UINT8 halInterrupt_PacketsOverflowState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_FC_STAT2);
}

void halInterrupt_PacketsOverflowClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_FC_STAT2));
}

IM_UINT8 halInterrupt_AudioSamplerState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_AS_STAT0);
}

void halInterrupt_AudioSamplerClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_AS_STAT0));
}

IM_UINT8 halInterrupt_PhyState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_PHY_STAT0);
}

void halInterrupt_PhyClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_PHY_STAT0));
}

IM_UINT8 halInterrupt_I2cDdcState(IM_UINT16 baseAddr)

{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_I2CM_STAT0);
}
void halInterrupt_I2cDdcClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_I2CM_STAT0));
}

IM_UINT8 halInterrupt_CecState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_CEC_STAT0);
}

void halInterrupt_CecClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_CEC_STAT0));
}

IM_UINT8 halInterrupt_VideoPacketizerState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_VP_STAT0);
}

void halInterrupt_VideoPacketizerClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_VP_STAT0));
}

IM_UINT8 halInterrupt_I2cPhyState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + IH_I2CMPHY_STAT0);
}

void halInterrupt_I2cPhyClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_I2CMPHY_STAT0));
}

void halInterrupt_Mute(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + IH_MUTE));
}
