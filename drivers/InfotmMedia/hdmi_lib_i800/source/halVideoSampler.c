/*
 * @file:halVideoSampler.c
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */
#include "halVideoSampler.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 TX_INVID0 = 0x00;
static const IM_UINT8 TX_INSTUFFING = 0x01;
static const IM_UINT8 TX_GYDATA0 = 0x02;
static const IM_UINT8 TX_GYDATA1 = 0x03;
static const IM_UINT8 TX_RCRDATA0 = 0x04;
static const IM_UINT8 TX_RCRDATA1 = 0x05;
static const IM_UINT8 TX_BCBDATA0 = 0x06;
static const IM_UINT8 TX_BCBDATA1 = 0x07;

void halVideoSampler_InternalDataEnableGenerator(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + TX_INVID0), 7, 1);
}

void halVideoSampler_VideoMapping(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + TX_INVID0), 0, 5);
}

void halVideoSampler_StuffingGy(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + TX_GYDATA0));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + TX_GYDATA1));
	access_CoreWrite(1, (baseAddr + TX_INSTUFFING), 0, 1);
}

void halVideoSampler_StuffingRcr(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + TX_RCRDATA0));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + TX_RCRDATA1));
	access_CoreWrite(1, (baseAddr + TX_INSTUFFING), 1, 1);
}

void halVideoSampler_StuffingBcb(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value >> 0), (baseAddr + TX_BCBDATA0));
	access_CoreWriteByte((u8) (value >> 8), (baseAddr + TX_BCBDATA1));
	access_CoreWrite(1, (baseAddr + TX_INSTUFFING), 2, 1);
}
