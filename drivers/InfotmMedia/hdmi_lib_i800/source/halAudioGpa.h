/* 
 * halAudioGpa.h
 * 
 * Created on Oct. 30th 2010
 * Synopsys Inc.
 */

#ifndef HALAUDIOGPA_H_
#define HALAUDIOGPA_H_

#include "types.h"
 
void halAudioGpa_ResetFifo(IM_UINT16 baseAddress);
void halAudioGpa_ChannelEnable(IM_UINT16 baseAddress, IM_UINT8 enable, IM_UINT8 channel);
void halAudioGpa_HbrEnable(IM_UINT16 baseAddress, IM_UINT8 enable);

IM_UINT8 halAudioGpa_InterruptStatus(IM_UINT16 baseAddress);
void halAudioGpa_InterruptMask(IM_UINT16 baseAddress, IM_UINT8 value);
void halAudioGpa_InterruptPolarity(IM_UINT16 baseAddress, IM_UINT8 value);

#endif /* HALAUDIOGPA_H_ */
