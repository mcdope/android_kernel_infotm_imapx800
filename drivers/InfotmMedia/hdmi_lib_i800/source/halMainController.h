/*
 * @file:halMainController.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALMAINCONTROLLER_H_
#define HALMAINCONTROLLER_H_

#include "types.h"

void halMainController_SfrClockDivision(IM_UINT16 baseAddr, IM_UINT8 value);

void halMainController_HdcpClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_CecClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_ColorSpaceConverterClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_AudioSamplerClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_PixelRepetitionClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_TmdsClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_PixelClockGate(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_CecClockReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_AudioGpaReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_AudioHbrReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_AudioSpdifReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_AudioI2sReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_PixelRepetitionClockReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_TmdsClockReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_PixelClockReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_VideoFeedThroughOff(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_PhyReset(IM_UINT16 baseAddr, IM_UINT8 bit);

void halMainController_HeacPhyReset(IM_UINT16 baseAddr, IM_UINT8 bit);

IM_UINT8 halMainController_LockOnClockStatus(IM_UINT16 baseAddr, IM_UINT8 clockDomain);

void halMainController_LockOnClockClear(IM_UINT16 baseAddr, IM_UINT8 clockDomain);

#endif /* HALMAINCONTROLLER_H_ */
