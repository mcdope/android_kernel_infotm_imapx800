/*
 * @file:halFrameComposerGamut.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERGAMUT_H_
#define HALFRAMECOMPOSERGAMUT_H_

#include "types.h"

void halFrameComposerGamut_Profile(IM_UINT16 baseAddr, IM_UINT8 profile);

void halFrameComposerGamut_AffectedSeqNo(IM_UINT16 baseAddr, IM_UINT8 no);

void halFrameComposerGamut_PacketsPerFrame(IM_UINT16 baseAddr, IM_UINT8 packets);

void halFrameComposerGamut_PacketLineSpacing(IM_UINT16 baseAddr, IM_UINT8 lineSpacing);

void halFrameComposerGamut_Content(IM_UINT16 baseAddr, const IM_UINT8 * content, IM_UINT8 length);

void halFrameComposerGamut_EnableTx(IM_UINT16 baseAddr, IM_UINT8 enable);

/* Only when this function is called is the packet updated with
 * the new information
 * @param baseAddr of the frame composer-gamut block
 * @return nothing
 */
void halFrameComposerGamut_UpdatePacket(IM_UINT16 baseAddr);

IM_UINT8 halFrameComposerGamut_CurrentSeqNo(IM_UINT16 baseAddr);

IM_UINT8 halFrameComposerGamut_PacketSeq(IM_UINT16 baseAddr);

IM_UINT8 halFrameComposerGamut_NoCurrentGbd(IM_UINT16 baseAddr);

#endif /* HALFRAMECOMPOSERGAMUT_H_ */
