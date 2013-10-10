/*
 * @file:halVideoPacketizer.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALVIDEOPACKETIZER_H_
#define HALVIDEOPACKETIZER_H_

#include "types.h"

IM_UINT8 halVideoPacketizer_PixelPackingPhase(IM_UINT16 baseAddr);

void halVideoPacketizer_ColorDepth(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoPacketizer_PixelPackingDefaultPhase(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoPacketizer_PixelRepetitionFactor(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoPacketizer_Ycc422RemapSize(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoPacketizer_OutputSelector(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HALVIDEOPACKETIZER_H_ */
