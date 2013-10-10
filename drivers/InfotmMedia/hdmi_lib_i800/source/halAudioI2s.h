/*
 * hal_audio_i2s.h
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALAUDIOI2S_H_
#define HALAUDIOI2S_H_

#include "types.h"

void halAudioI2s_ResetFifo(IM_UINT16 baseAddress);

void halAudioI2s_Select(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioI2s_DataEnable(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioI2s_DataMode(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioI2s_DataWidth(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioI2s_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioI2s_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value);

#endif /* HALAUDIOI2S_H_ */
