/*
 * halColorSpaceConverter.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALCOLORSPACECONVERTER_H_
#define HALCOLORSPACECONVERTER_H_

#include "types.h"

void halColorSpaceConverter_Interpolation(IM_UINT16 baseAddr, IM_UINT8 value);

void halColorSpaceConverter_Decimation(IM_UINT16 baseAddr, IM_UINT8 value);

void halColorSpaceConverter_ColorDepth(IM_UINT16 baseAddr, IM_UINT8 value);

void halColorSpaceConverter_ScaleFactor(IM_UINT16 baseAddr, IM_UINT8 value);

void halColorSpaceConverter_CoefficientA1(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientA2(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientA3(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientA4(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientB1(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientB2(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientB3(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientB4(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientC1(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientC2(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientC3(IM_UINT16 baseAddr, IM_UINT16 value);

void halColorSpaceConverter_CoefficientC4(IM_UINT16 baseAddr, IM_UINT16 value);

#endif /* HALCOLORSPACECONVERTER_H_ */
