/*
 * @file:halInterrupt.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALINTERRUPT_H_
#define HALINTERRUPT_H_
#include "types.h"

IM_UINT8 halInterrupt_AudioPacketsState(IM_UINT16 baseAddr);

void halInterrupt_AudioPacketsClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_OtherPacketsState(IM_UINT16 baseAddr);

void halInterrupt_OtherPacketsClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_PacketsOverflowState(IM_UINT16 baseAddr);

void halInterrupt_PacketsOverflowClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_AudioSamplerState(IM_UINT16 baseAddr);

void halInterrupt_AudioSamplerClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_PhyState(IM_UINT16 baseAddr);

void halInterrupt_PhyClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_I2cDdcState(IM_UINT16 baseAddr);

void halInterrupt_I2cDdcClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_CecState(IM_UINT16 baseAddr);

void halInterrupt_CecClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_VideoPacketizerState(IM_UINT16 baseAddr);

void halInterrupt_VideoPacketizerClear(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halInterrupt_I2cPhyState(IM_UINT16 baseAddr);

void halInterrupt_I2cPhyClear(IM_UINT16 baseAddr, IM_UINT8 value);

void halInterrupt_Mute(IM_UINT16 baseAddr, IM_UINT8 value);

#endif /* HALINTERRUPT_H_ */
