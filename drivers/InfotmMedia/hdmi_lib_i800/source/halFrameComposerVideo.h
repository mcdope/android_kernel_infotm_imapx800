/*
 * @file:halFrameComposerVideo.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERVIDEO_H_
#define HALFRAMECOMPOSERVIDEO_H_

#include "types.h"

void halFrameComposerVideo_HdcpKeepout(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_VSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_HSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_DviOrHdmi(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_VBlankOsc(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_Interlaced(IM_UINT16 baseAddr, IM_UINT8 bit);

void halFrameComposerVideo_HActive(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_HBlank(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_VActive(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_VBlank(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_HSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_HSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_VSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_VSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value);

void halFrameComposerVideo_RefreshRate(IM_UINT16 baseAddr, IM_UINT32 value);

void halFrameComposerVideo_ControlPeriodMinDuration(IM_UINT16 baseAddr, IM_UINT8 value);

void halFrameComposerVideo_ExtendedControlPeriodMinDuration(IM_UINT16 baseAddr,
		IM_UINT8 value);

void halFrameComposerVideo_ExtendedControlPeriodMaxSpacing(IM_UINT16 baseAddr,
		IM_UINT8 value);

void halFrameComposerVideo_PreambleFilter(IM_UINT16 baseAddr, IM_UINT8 value,
		IM_UINT32 channel);

void halFrameComposerVideo_PixelRepetitionInput(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HALFRAMECOMPOSERVIDEO_H_ */
