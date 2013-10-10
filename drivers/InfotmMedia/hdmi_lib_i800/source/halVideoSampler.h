/*
 * @file:halVideoSampler.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALVIDEOSAMPLER_H_
#define HALVIDEOSAMPLER_H_

#include "types.h"

void halVideoSampler_InternalDataEnableGenerator(IM_UINT16 baseAddr, IM_UINT8 bit);

void halVideoSampler_VideoMapping(IM_UINT16 baseAddr, IM_UINT8 value);

void halVideoSampler_StuffingGy(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoSampler_StuffingRcr(IM_UINT16 baseAddr, IM_UINT16 value);

void halVideoSampler_StuffingBcb(IM_UINT16 baseAddr, IM_UINT16 value);

#endif /* HALVIDEOSAMPLER_H_ */
