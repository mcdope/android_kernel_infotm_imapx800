/*
 * packets.h
 *
 *  Created on: Jul 7, 2010
  * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef PACKETS_H_
#define PACKETS_H_

#include "types.h"
#include "productParams.h"
#include "videoParams.h"
#include "audioParams.h"

/**
 * Initialize the packets package. Reset local variables.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @return TRUE when successful
 */
IM_INT32 packets_Initialize(IM_UINT16 baseAddr);

/**
 * Configure Source Product Description, Vendor Specific and Auxiliary
 * Video InfoFrames.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param video  Video Parameters to set up AVI InfoFrame (and all
 * other video parameters)
 * @param prod Description of Vendor and Product to set up Vendor
 * Specific InfoFrame and Source Product Description InfoFrame
 * @return TRUE when successful
 */
IM_INT32 packets_Configure(IM_UINT16 baseAddr, videoParams_t * video,
		productParams_t * prod);

/**
 * Configure Audio Content Protection packets.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param type Content protection type (see HDMI1.3a Section 9.3)
 * @param fields  ACP Type Dependent Fields
 * @param length of the ACP fields
 * @param autoSend Send Packets Automatically
 */
void packets_AudioContentProtection(IM_UINT16 baseAddr, IM_UINT8 type, const IM_UINT8 * fields,
		IM_UINT8 length, IM_UINT8 autoSend);

/**
 * Configure ISRC 1 & 2 Packets
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param initStatus Initial status which the packets are sent with (usually starting position)
 * @param codes ISRC codes array
 * @param length of the ISRC codes array
 * @param autoSend Send ISRC Automatically
 * @note Automatic sending does not change status automatically, it does the insertion of the packets in the data
 * islands.
 */
void packets_IsrcPackets(IM_UINT16 baseAddr, IM_UINT8 initStatus, const IM_UINT8 * codes,
		IM_UINT8 length, IM_UINT8 autoSend);

/**
 * Configure Audio InfoFrame Packets
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param params audio parameters
 */
void packets_AudioInfoFrame(IM_UINT16 baseAddr, audioParams_t *params);

/**
 * Send/stop sending AV Mute in the General Control Packet
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param enable (TRUE) /disable (FALSE) the AV Mute
 */
void packets_AvMute(IM_UINT16 baseAddr, IM_UINT8 enable);

/**
 * Set ISRC status that is changing during play back depending on position (see HDMI 1.3a Section 8.8)
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param status the ISRC status code according to position of track
 */
void packets_IsrcStatus(IM_UINT16 baseAddr, IM_UINT8 status);

/**
 * @param baseAddr Base Address of the HDMICTRL registers.
 * Stop sending ACP packets when in auto send mode
 */
void packets_StopSendAcp(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base Address of the HDMICTRL registers.
 * Stop sending ISRC 1 & 2 packets when in auto send mode (ISRC 2 packets cannot be send without ISRC 1)
 */
void packets_StopSendIsrc1(IM_UINT16 baseAddr);

/**
 * @param baseAddr Base Address of the HDMICTRL registers.
 * Stop sending ISRC 2 packets when in auto send mode
 */
void packets_StopSendIsrc2(IM_UINT16 baseAddr);

/**
 * Stop sending Source Product Description InfoFrame packets when in auto send mode
 * @param baseAddr Base Address of the HDMICTRL registers.
 */
void packets_StopSendSpd(IM_UINT16 baseAddr);

/**
 * Stop sending Vendor Specific InfoFrame packets when in auto send mode
 * @param baseAddr Base Address of the HDMICTRL registers.
 */
void packets_StopSendVsd(IM_UINT16 baseAddr);

/**
 * Configure and start sending Gamut Metadata Packets.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param gbdContent Gamut Boundary Description Content array
 * @param length of the content array
 */
void packets_GamutMetadataPackets(IM_UINT16 baseAddr, const IM_UINT8 * gbdContent,
		IM_UINT8 length);

/**
 * Disable all metadata packets from being sent automatically. (ISRC 1& 2, ACP, VSD and SPD)
 * @param baseAddr Base Address of the HDMICTRL registers.
 */
void packets_DisableAllPackets(IM_UINT16 baseAddr);

/**
 * Configure Source Product Description InfoFrames.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param vName Vendor Name
 * @param vLength length of the vendor name string
 * @param pName ProductName
 * @param pLength length of the product name string
 * @param code Source Device Information, Table 15 CEA-861-D
 * @param autoSend Start send SPD InfoFrame automatically
 */
IM_INT32 packets_SourceProductInfoFrame(IM_UINT16 baseAddr, const IM_UINT8 * vName, IM_UINT8 vLength,
		const IM_UINT8 * pName, IM_UINT8 pLength, IM_UINT8 code, IM_UINT8 autoSend);

/**
 * Configure Vendor Specific InfoFrames.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param oui Vendor Organisational Unique Identifier 24 bit IEEE
 * Registration Identifier
 * @param payload Vendor Specific Info Payload
 * @param length of the payload array
 * @param autoSend Start send Vendor Specific InfoFrame automatically
 */
IM_INT32 packets_VendorSpecificInfoFrame(IM_UINT16 baseAddr, IM_UINT32 oui, const IM_UINT8 * payload,
		IM_UINT8 length, IM_UINT8 autoSend);

/**
 * Configure Auxiliary Video InfoFrames.
 * @param baseAddr Base Address of the HDMICTRL registers.
 * @param videoParams Video Parameters to set up AVI InfoFrame (and all
 * other video parameters)
 */
void packets_AuxiliaryVideoInfoFrame(IM_UINT16 baseAddr, videoParams_t *videoParams);

#endif /* PACKETS_H_ */
