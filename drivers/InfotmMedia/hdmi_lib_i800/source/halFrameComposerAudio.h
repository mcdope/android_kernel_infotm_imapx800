/*
 * @file:halFrameComposerAudio.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERAUDIO_H_
#define HALFRAMECOMPOSERAUDIO_H_

#include "types.h"

void halFrameComposerAudio_PacketSampleFlat(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_PacketLayout(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerAudio_ValidityRight(IM_UINT16 baseAddr, IM_UINT8 bit,
				IM_UINT32 channel);

void halFrameComposerAudio_ValidityLeft(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel);

void halFrameComposerAudio_UserRight(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel);

void halFrameComposerAudio_UserLeft(IM_UINT16 baseAddr, IM_UINT8 bit, IM_UINT32 channel);

void halFrameComposerAudio_IecCgmsA(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecCopyright(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerAudio_IecCategoryCode(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecPcmMode(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecSource(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecChannelRight(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel);

void halFrameComposerAudio_IecChannelLeft(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel);

void halFrameComposerAudio_IecClockAccuracy(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecSamplingFreq(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecOriginalSamplingFreq(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerAudio_IecWordLength(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HALFRAMECOMPOSERAUDIO_H_ */
