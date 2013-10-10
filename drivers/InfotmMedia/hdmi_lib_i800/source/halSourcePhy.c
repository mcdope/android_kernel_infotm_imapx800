/*
 * @file:halSourcePhy.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halSourcePhy.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 PHY_CONF0 = 0x00;
static const IM_UINT8 PHY_TST0 = 0x01;
static const IM_UINT8 PHY_TST1 = 0x02;
static const IM_UINT8 PHY_TST2 = 0x03;
static const IM_UINT8 PHY_STAT0 = 0x04;
static const IM_UINT8 PHY_MASK0 = 0x06;
static const IM_UINT8 PHY_POL0 = 0x07;

void halSourcePhy_PowerDown(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 7, 1);
}

void halSourcePhy_EnableTMDS(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 6, 1);
}

void halSourcePhy_Gen2PDDQ(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 4, 1);
}

void halSourcePhy_Gen2TxPowerOn(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 3, 1);
}

void halSourcePhy_Gen2EnHpdRxSense(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 2, 1);
}

void halSourcePhy_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 1, 1);
}

void halSourcePhy_InterfaceControl(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_CONF0), 0, 1);
}

void halSourcePhy_TestClear(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_TST0), 5, 1);
}

void halSourcePhy_TestEnable(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_TST0), 4, 1);
}

void halSourcePhy_TestClock(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + PHY_TST0), 0, 1);
}

void halSourcePhy_TestDataIn(IM_UINT16 baseAddr, IM_UINT8 data)
{
	LOG_TRACE1(data);
	access_CoreWriteByte(data, (baseAddr + PHY_TST1));
}

IM_UINT8 halSourcePhy_TestDataOut(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + PHY_TST2);
}

IM_UINT8 halSourcePhy_PhaseLockLoopState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + PHY_STAT0), 0, 1);
}

IM_UINT8 halSourcePhy_HotPlugState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + PHY_STAT0), 1, 1);
}

void halSourcePhy_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 mask)
{
	LOG_TRACE1(mask);
	access_CoreWriteByte(mask, (baseAddr + PHY_MASK0));
}

IM_UINT8 halSourcePhy_InterruptMaskStatus(IM_UINT16 baseAddr, IM_UINT8 mask)
{
	LOG_TRACE1(mask);
	return access_CoreReadByte(baseAddr + PHY_MASK0) & mask;
}

void halSourcePhy_InterruptPolarity(IM_UINT16 baseAddr, IM_UINT8 bitShift, IM_UINT8 value)
{
	LOG_TRACE2(bitShift, value);
	access_CoreWrite(value, (baseAddr + PHY_POL0), bitShift, 1);
}

IM_UINT8 halSourcePhy_InterruptPolarityStatus(IM_UINT16 baseAddr, IM_UINT8 mask)
{
	LOG_TRACE1(mask);
	return access_CoreReadByte(baseAddr + PHY_POL0) & mask;
}
