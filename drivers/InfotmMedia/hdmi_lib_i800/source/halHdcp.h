/*
 * halHdcp.h
 *
 *  Created on: Jul 19, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */
 /** @file
 *   HAL (hardware abstraction layer) of HDCP engine register block
 */

#ifndef HALHDCP_H_
#define HALHDCP_H_

#include "types.h"

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_DeviceMode(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_EnableFeature11(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_RxDetected(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_EnableAvmute(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_RiCheck(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_BypassEncryption(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_EnableI2cFastMode(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_EnhancedLinkVerification(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
void halHdcp_SwReset(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_DisableEncryption(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_EncodingPacketHeader(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_DisableKsvListCheck(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_HdcpEngaged(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_AuthenticationState(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_CipherState(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_RevocationState(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_OessState(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_EessState(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_DebugInfo(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param value
 */
void halHdcp_InterruptClear(IM_UINT16 baseAddr, IM_UINT8 value);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_InterruptStatus(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param value
 */
void halHdcp_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 value);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_HSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_VSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param value
 */
void halHdcp_UnencryptedVideoColor(IM_UINT16 baseAddr, IM_UINT8 value);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param value
 */
void halHdcp_OessWindowSize(IM_UINT16 baseAddr, IM_UINT8 value);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT16 halHdcp_CoreVersion(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_MemoryAccessRequest(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_UINT8 halHdcp_MemoryAccessGranted(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param bit
 */
void halHdcp_UpdateKsvListState(IM_UINT16 baseAddr, IM_UINT8 bit);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param addr in list
 */
IM_UINT8 halHdcp_KsvListRead(IM_UINT16 baseAddr, IM_UINT16 addr);

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param addr in list
 * @param data
 */
void halHdcp_RevocListWrite(IM_UINT16 baseAddr, IM_UINT16 addr, IM_UINT8 data);

#endif /* HALHDCP_H_ */
