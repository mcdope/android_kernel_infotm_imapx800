/*
 * hal_audio_generator.h
 *
 *  Created on: Jun 29, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HAL_AUDIO_GENERATOR_H_
#define HAL_AUDIO_GENERATOR_H_

#include "types.h"

void halAudioGenerator_SwReset(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_I2sMode(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_FreqIncrementLeft(IM_UINT16 baseAddress, IM_UINT16 value);

void halAudioGenerator_FreqIncrementRight(IM_UINT16 baseAddress, IM_UINT16 value);

void halAudioGenerator_IecCgmsA(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecCopyright(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_IecCategoryCode(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecPcmMode(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecSource(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecChannelRight(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channelNo);

void halAudioGenerator_IecChannelLeft(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channelNo);

void halAudioGenerator_IecClockAccuracy(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecSamplingFreq(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecOriginalSamplingFreq(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_IecWordLength(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_UserRight(IM_UINT16 baseAddress, IM_UINT8 bit, IM_UINT8 channelNo);

void halAudioGenerator_UserLeft(IM_UINT16 baseAddress, IM_UINT8 bit, IM_UINT8 channelNo);

void halAudioGenerator_SpdifValidity(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_SpdifEnable(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_HbrEnable(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_HbrDdrEnable(IM_UINT16 baseAddress, IM_UINT8 bit);

/*
 * @param bit: right(1) or left(0)
 */
void halAudioGenerator_HbrDdrChannel(IM_UINT16 baseAddress, IM_UINT8 bit);

void halAudioGenerator_HbrBurstLength(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_HbrClockDivider(IM_UINT16 baseAddress, IM_UINT16 value);
/* 
 * (HW simulation only)
 */
void halAudioGenerator_GpaReplyLatency(IM_UINT16 baseAddress, IM_UINT8 value);
/* 
 * (HW simulation only)
 */
void halAudioGenerator_UseLookUpTable(IM_UINT16 baseAddress, IM_UINT8 value);

void halAudioGenerator_GpaSampleValid(IM_UINT16 baseAddress, IM_UINT8 value, IM_UINT8 channel);

void halAudioGenerator_ChannelSelect(IM_UINT16 baseAddress, IM_UINT8 enable, IM_UINT8 channel); 

#endif /* HAL_AUDIO_GENERATOR_H_ */
