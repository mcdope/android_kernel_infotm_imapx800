/*
 * @file:halVideoGenerator.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALVIDEOGENERATOR_H_
#define HALVIDEOGENERATOR_H_

#include "types.h"

void halVideoGenerator_SwReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_Ycc(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_Ycc422(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_VBlankOsc(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_ColorIncrement(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_Interlaced(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_VSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_HSyncPolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_ColorResolution(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoGenerator_PixelRepetitionInput(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoGenerator_HActive(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_HBlank(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_HSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_HSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_VActive(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_VBlank(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_VSyncEdgeDelay(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_VSyncPulseWidth(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_Enable3D(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoGenerator_Structure3D(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoGenerator_WriteStart(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoGenerator_WriteData(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT16 halVideoGenerator_WriteCurrentAddr(IM_UINT16 baseAddr);

IM_UINT8 halVideoGenerator_WriteCurrentByte(IM_UINT16 baseAddr);

#endif /* HALVIDEOGENERATOR_H_ */
