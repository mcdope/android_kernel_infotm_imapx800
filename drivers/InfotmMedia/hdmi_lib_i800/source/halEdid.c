/*
 * @file:halEdid.c
 *
 *  Created on: Jul 5, 2010
 *      Author: klabadi & dlopo
 */

#include "halEdid.h"
#include "hdmi_log.h"
#include "access.h"

static const IM_UINT8 SLAVE_ADDR = 0x00;
static const IM_UINT8 REQUEST_ADDR = 0x01;
static const IM_UINT8 DATA_OUT = 0x02;
static const IM_UINT8 DATA_IN = 0x03;
static const IM_UINT8 OPERATION = 0x04;
static const IM_UINT8 DONE_INT = 0x05;
static const IM_UINT8 ERROR_INT = 0x06;
static const IM_UINT8 CLOCK_DIV = 0x07;
static const IM_UINT8 SEG_ADDR = 0x08;
static const IM_UINT8 SEG_POINTER = 0x0A;
static const IM_UINT8 SS_SCL_CNT = 0x0B;
static const IM_UINT8 FS_SCL_CNT = 0x0F;

void halEdid_SlaveAddress(IM_UINT16 baseAddr, IM_UINT8 addr)
{
	LOG_TRACE1(addr);
	access_CoreWrite(addr, (baseAddr + SLAVE_ADDR), 0, 7);
}

void halEdid_RequestAddr(IM_UINT16 baseAddr, IM_UINT8 addr)
{
	LOG_TRACE1(addr);
	access_CoreWriteByte(addr, (baseAddr + REQUEST_ADDR));
}

void halEdid_WriteData(IM_UINT16 baseAddr, IM_UINT8 data)
{
	LOG_TRACE1(data);
	access_CoreWriteByte(data, (baseAddr + DATA_OUT));
}

IM_UINT8 halEdid_ReadData(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte((baseAddr + DATA_IN));
}

void halEdid_RequestRead(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWrite(1, (baseAddr + OPERATION), 0, 1);
}

void halEdid_RequestExtRead(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWrite(1, (baseAddr + OPERATION), 1, 1);
}

void halEdid_RequestWrite(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	access_CoreWrite(1, (baseAddr + OPERATION), 4, 1);
}

void halEdid_MasterClockDivision(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	/* bit 4 selects between high and standard speed operation */
	access_CoreWrite(value, (baseAddr + CLOCK_DIV), 0, 4);
}

void halEdid_SegmentAddr(IM_UINT16 baseAddr, IM_UINT8 addr)
{
	LOG_TRACE1(addr);
	access_CoreWriteByte(addr, (baseAddr + SEG_ADDR));
}

void halEdid_SegmentPointer(IM_UINT16 baseAddr, IM_UINT8 pointer)
{
	LOG_TRACE1(pointer);
	access_CoreWriteByte(pointer, (baseAddr + SEG_POINTER));
}

void halEdid_MaskInterrupts(IM_UINT16 baseAddr, IM_UINT8 mask)
{
	LOG_TRACE1(mask);
	access_CoreWrite(mask ? 1 : 0, (baseAddr + DONE_INT), 2, 1);
	access_CoreWrite(mask ? 1 : 0, (baseAddr + ERROR_INT), 2, 1);
	access_CoreWrite(mask ? 1 : 0, (baseAddr + ERROR_INT), 6, 1);
}

void halEdid_FastSpeedCounter(IM_UINT16 baseAddr, IM_UINT32 value)
{
	LOG_TRACE2((baseAddr + FS_SCL_CNT), value);
	access_CoreWriteByte((u8) (value >> 24), (baseAddr + FS_SCL_CNT + 0));
	access_CoreWriteByte((u8) (value >> 16), (baseAddr + FS_SCL_CNT + 1));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + FS_SCL_CNT + 2));
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + FS_SCL_CNT + 3));
}

void halEdid_StandardSpeedCounter(IM_UINT16 baseAddr, IM_UINT32 value)
{
	LOG_TRACE2((baseAddr + SS_SCL_CNT), value);
	access_CoreWriteByte((u8) (value >> 24), (baseAddr + SS_SCL_CNT + 0));
	access_CoreWriteByte((u8) (value >> 16), (baseAddr + SS_SCL_CNT + 1));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + SS_SCL_CNT + 2));
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + SS_SCL_CNT + 3));
}
