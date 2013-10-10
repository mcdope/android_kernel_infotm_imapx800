/*
 * @file:halFrameComposerAvi.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERAVI_H_
#define HALFRAMECOMPOSERAVI_H_

#include "types.h"

void halFrameComposerAvi_RgbYcc(IM_UINT16 baseAddr, IM_UINT8 type);

void halFrameComposerAvi_ScanInfo(IM_UINT16 baseAddr, IM_UINT8 left);

void halFrameComposerAvi_Colorimetry(IM_UINT16 baseAddr, IM_UINT32 cscITU);

void halFrameComposerAvi_PicAspectRatio(IM_UINT16 baseAddr, IM_UINT8 ar);

void halFrameComposerAvi_ActiveAspectRatioValid(IM_UINT16 baseAddr, IM_UINT8 valid);

void halFrameComposerAvi_ActiveFormatAspectRatio(IM_UINT16 baseAddr, IM_UINT8 left);

void halFrameComposerAvi_IsItContent(IM_UINT16 baseAddr, IM_UINT8 it);

void halFrameComposerAvi_ExtendedColorimetry(IM_UINT16 baseAddr, IM_UINT8 extColor);

void halFrameComposerAvi_QuantizationRange(IM_UINT16 baseAddr, IM_UINT8 range);

void halFrameComposerAvi_NonUniformPicScaling(IM_UINT16 baseAddr, IM_UINT8 scale);

void halFrameComposerAvi_VideoCode(IM_UINT16 baseAddr, IM_UINT8 code);

void halFrameComposerAvi_HorizontalBarsValid(IM_UINT16 baseAddr, IM_UINT8 validity);

void halFrameComposerAvi_HorizontalBars(IM_UINT16 baseAddr, IM_UINT16 endTop, IM_UINT16 startBottom);

void halFrameComposerAvi_VerticalBarsValid(IM_UINT16 baseAddr, IM_UINT8 validity);

void halFrameComposerAvi_VerticalBars(IM_UINT16 baseAddr, IM_UINT16 endLeft, IM_UINT16 startRight);

void halFrameComposerAvi_OutPixelRepetition(IM_UINT16 baseAddr, IM_UINT8 pr);

#endif /* HALFRAMECOMPOSERAVI_H_ */
