/*
 * @file:halFrameComposerPackets.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERPACKETS_H_
#define HALFRAMECOMPOSERPACKETS_H_

#include "types.h"

#define ACP_TX  	0
#define ISRC1_TX 	1
#define ISRC2_TX 	2
#define SPD_TX 		4
#define VSD_TX 		3

void halFrameComposerPackets_QueuePriorityHigh(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerPackets_QueuePriorityLow(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerPackets_MetadataFrameInterpolation(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerPackets_MetadataFramesPerPacket(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerPackets_MetadataLineSpacing(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerPackets_AutoSend(IM_UINT16 baseAddr, IM_UINT8 enable, IM_UINT8 mask);

void halFrameComposerPackets_ManualSend(IM_UINT16 baseAddr, IM_UINT8 mask);

void halFrameComposerPackets_DisableAll(IM_UINT16 baseAddr);

#endif /* HALFRAMECOMPOSERPACKETS_H_ */
