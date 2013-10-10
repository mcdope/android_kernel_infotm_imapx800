/*
 * halAudioHbr.h
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALAUDIOHBR_H_
#define HALAUDIOHBR_H_

#include "types.h"

void halAudioHbr_ResetFifo(IM_UINT16 baseAddress);

void halAudioHbr_Select(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioHbr_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioHbr_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value);

#endif /* HALAUDIOHBR_H_ */
