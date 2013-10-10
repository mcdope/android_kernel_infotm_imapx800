/*
 * hdcp.h
 *
 *  Created on: Jul 21, 2010
 * 
 *  Synopsys Inc.
 *  SG DWC PT02 
 */

#ifndef HDCP_H_
#define HDCP_H_

#include "types.h"
#include "hdcpParams.h"

/**
 * @param baseAddr Base address of the HDCP module registers
 * @param dataEnablePolarity
 * @return TRUE if successful
 */
IM_INT32 hdcp_Initialize(IM_UINT16 baseAddr, IM_INT32 dataEnablePolarity);
/**
 * @param baseAddr Base address of the HDCP module registers
 * @param params HDCP parameters
 * @param hdmi TRUE if HDMI, FALSE if DVI
 * @param hsPol HSYNC polarity
 * @param vsPol VSYNC polarity
 * @return TRUE if successful
 */
IM_INT32 hdcp_Configure(IM_UINT16 baseAddr, hdcpParams_t *params, IM_INT32 hdmi, IM_INT32 hsPol,
		IM_INT32 vsPol);
/**
 * The method handles DONE and ERROR events.
 * A DONE event will trigger the retrieving the read byte, and sending a request to read the following byte. The EDID is read until the block is done and then the reading moves to the next block.
 *  When the block is successfully read, it is sent to be parsed.
 * @param baseAddr Base address of the HDCP module registers
 * @param state of the HDCP engine interrupts
 * @param hpd on or off
 */
IM_UINT8 hdcp_EventHandler(IM_UINT16 baseAddr, IM_INT32 hpd, IM_UINT8 state);
/**
 * @param baseAddr Base address of the HDCP module registers
 * @param detected if RX is detected (TRUE or FALSE)
 */
void hdcp_RxDetected(IM_UINT16 baseAddr, IM_INT32 detected);
/**
 * Enter or exit AV mute mode
 * @param baseAddr Base address of the HDCP module registers
 * @param enable the HDCP AV mute
 */
void hdcp_AvMute(IM_UINT16 baseAddr, IM_INT32 enable);
/**
 * Bypass data encryption stage
 * @param baseAddr Base address of the HDCP module registers
 * @param bypass the HDCP AV mute
 */
void hdcp_BypassEncryption(IM_UINT16 baseAddr, IM_INT32 bypass);
/**
 * @param baseAddr Base address of the HDCP module registers
 * @param disable the HDCP encrption
 */
void hdcp_DisableEncryption(IM_UINT16 baseAddr, IM_INT32 disable);
/**
 * @param baseAddr Base address of the HDCP module registers
 */
IM_INT32 hdcp_Engaged(IM_UINT16 baseAddr);
/**
 * The engine goes through the authentication
 * statesAs defined in the HDCP spec
 * @param baseAddr Base address of the HDCP module registers
 * @return a the state of the authentication machine
 * @note Used for debug purposes
 */
IM_UINT8 hdcp_AuthenticationState(IM_UINT16 baseAddr);
/**
 * The engine goes through several cipher states
 * @param baseAddr Base address of the HDCP module registers
 * @return a the state of the cipher machine
 * @note Used for debug purposes
 */
IM_UINT8 hdcp_CipherState(IM_UINT16 baseAddr);
/**
 * The engine goes through revocation states
 * @param baseAddr Base address of the HDCP module registers
 * @return a the state of the revocation the engine is going through
 * @note Used for debug purposes
 */
IM_UINT8 hdcp_RevocationState(IM_UINT16 baseAddr);
/**
 * @param baseAddr Base address of the HDCP module registers
 * @param data pointer to memory where SRM info is located
 * @return TRUE if successful
 */
IM_INT32 hdcp_SrmUpdate(IM_UINT16 baseAddr, const IM_UINT8 * data);
/**
 * @param baseAddr base address of HDCP module registers
 * @return HDCP interrupts state  
 */
IM_UINT8 hdcp_InterruptStatus(IM_UINT16 baseAddr);
/** 
 * Clear HDCP interrupts
 * @param baseAddr base address of controller
 * @param value mask of interrupts to clear
 * @return TRUE if successful
 */
IM_INT32 hdcp_InterruptClear(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HDCP_H_ */
