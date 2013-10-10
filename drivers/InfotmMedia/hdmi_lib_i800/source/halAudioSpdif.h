/*
 * halAudioSpdif.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALAUDIOSPDIF_H_
#define HALAUDIOSPDIF_H_

#include "types.h"

void halAudioSpdif_ResetFifo(IM_UINT16 baseAddr);

void halAudioSpdif_NonLinearPcm(IM_UINT16 baseAddr, IM_UINT8 bit);

void halAudioSpdif_DataWidth(IM_UINT16 baseAddr, IM_UINT8 value);

void halAudioSpdif_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 value);

void halAudioSpdif_InterruptPolarity(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HALAUDIOSPDIF_H_ */
