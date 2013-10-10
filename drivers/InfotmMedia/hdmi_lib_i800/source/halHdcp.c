/*
 * halHdcp.c
 *
 *  Created on: Jul 19, 2010
 *      Author: klabadi & dlopo
 */
#include "halHdcp.h"
#include "access.h"
#include "hdmi_log.h"

const IM_UINT8 A_HDCPCFG0 = 0x00;
const IM_UINT8 A_HDCPCFG1 = 0x01;
const IM_UINT8 A_HDCPOBS0 = 0x02;
const IM_UINT8 A_HDCPOBS1 = 0x03;
const IM_UINT8 A_HDCPOBS2 = 0x04;
const IM_UINT8 A_HDCPOBS3 = 0x05;
const IM_UINT8 A_APIINTCLR = 0x06;
const IM_UINT8 A_APIINTSTA = 0x07;
const IM_UINT8 A_APIINTMSK = 0x08;
const IM_UINT8 A_VIDPOLCFG = 0x09;
const IM_UINT8 A_OESSWCFG = 0x0A;
const IM_UINT8 A_COREVERLSB = 0x14;
const IM_UINT8 A_COREVERMSB = 0x15;
const IM_UINT8 A_KSVMEMCTRL = 0x16;
const IM_UINT8 A_MEMORY = 0x20;
const IM_UINT16 REVOCATION_OFFSET = 0x299;

void halHdcp_DeviceMode(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 0, 1);
}

void halHdcp_EnableFeature11(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 1, 1);
}

void halHdcp_RxDetected(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 2, 1);
}

void halHdcp_EnableAvmute(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 3, 1);
}

void halHdcp_RiCheck(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 4, 1);
}

void halHdcp_BypassEncryption(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 5, 1);
}

void halHdcp_EnableI2cFastMode(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 6, 1);
}

void halHdcp_EnhancedLinkVerification(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG0), 7, 1);
}

void halHdcp_SwReset(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	/* active low */
	access_CoreWrite(0, (baseAddr + A_HDCPCFG1), 0, 1);
}

void halHdcp_DisableEncryption(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG1), 1, 1);
}

void halHdcp_EncodingPacketHeader(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG1), 2, 1);
}

void halHdcp_DisableKsvListCheck(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_HDCPCFG1), 3, 1);
}

IM_UINT8 halHdcp_HdcpEngaged(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_HDCPOBS0), 0, 1);
}

IM_UINT8 halHdcp_AuthenticationState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_HDCPOBS0), 1, 7);
}

IM_UINT8 halHdcp_CipherState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_HDCPOBS2), 3, 3);
}

IM_UINT8 halHdcp_RevocationState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_HDCPOBS1), 0, 3);
}

IM_UINT8 halHdcp_OessState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_HDCPOBS1), 3, 3);
}

IM_UINT8 halHdcp_EessState(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	/* TODO width = 3 or 4? */
	return access_CoreRead((baseAddr + A_HDCPOBS2), 0, 3);
}

IM_UINT8 halHdcp_DebugInfo(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + A_HDCPOBS3);
}

void halHdcp_InterruptClear(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + A_APIINTCLR));
}

IM_UINT8 halHdcp_InterruptStatus(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreReadByte(baseAddr + A_APIINTSTA);
}

void halHdcp_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + A_APIINTMSK));
}

void halHdcp_HSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_VIDPOLCFG), 1, 1);
}

void halHdcp_VSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_VIDPOLCFG), 3, 1);
}

void halHdcp_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_VIDPOLCFG), 4, 1);
}

void halHdcp_UnencryptedVideoColor(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWrite(value, (baseAddr + A_VIDPOLCFG), 5, 2);
}

void halHdcp_OessWindowSize(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte(value, (baseAddr + A_OESSWCFG));
}

IM_UINT16 halHdcp_CoreVersion(IM_UINT16 baseAddr)
{
	IM_UINT16 version = 0;
	LOG_TRACE();
	version = access_CoreReadByte(baseAddr + A_COREVERLSB);
	version |= access_CoreReadByte(baseAddr + A_COREVERMSB) << 8;
	return version;
}

void halHdcp_MemoryAccessRequest(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE();
	access_CoreWrite(bit, (baseAddr + A_KSVMEMCTRL), 0, 1);
}

IM_UINT8 halHdcp_MemoryAccessGranted(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return access_CoreRead((baseAddr + A_KSVMEMCTRL), 1, 1);
}

void halHdcp_UpdateKsvListState(IM_UINT16 baseAddr, IM_UINT8 bit)
{
	LOG_TRACE1(bit);
	access_CoreWrite(bit, (baseAddr + A_KSVMEMCTRL), 3, 1);
	access_CoreWrite(1, (baseAddr + A_KSVMEMCTRL), 2, 1);
	access_CoreWrite(0, (baseAddr + A_KSVMEMCTRL), 2, 1);
}

IM_UINT8 halHdcp_KsvListRead(IM_UINT16 baseAddr, IM_UINT16 addr)
{
	LOG_TRACE1(addr);
	if (addr >= REVOCATION_OFFSET)
	{
		LOG_WARNING("Invalid address");
	}
	return access_CoreReadByte((baseAddr + A_MEMORY) + addr);
}

void halHdcp_RevocListWrite(IM_UINT16 baseAddr, IM_UINT16 addr, IM_UINT8 data)
{
	LOG_TRACE2(addr, data);
	access_CoreWriteByte(data, (baseAddr + A_MEMORY) + REVOCATION_OFFSET + addr);
}
