/*
 * @file:halSourcePhy.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALSOURCEPHY_H_
#define HALSOURCEPHY_H_

#include "types.h"

void halSourcePhy_PowerDown(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_EnableTMDS(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_Gen2PDDQ(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_Gen2TxPowerOn(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_Gen2EnHpdRxSense(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_DataEnablePolarity(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_InterfaceControl(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_TestClear(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_TestEnable(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_TestClock(IM_UINT16 baseAddr, IM_UINT8 bit);

void halSourcePhy_TestDataIn(IM_UINT16 baseAddr, IM_UINT8 data);

IM_UINT8 halSourcePhy_TestDataOut(IM_UINT16 baseAddr);

IM_UINT8 halSourcePhy_PhaseLockLoopState(IM_UINT16 baseAddr);

IM_UINT8 halSourcePhy_HotPlugState(IM_UINT16 baseAddr);

void halSourcePhy_InterruptMask(IM_UINT16 baseAddr, IM_UINT8 value);

IM_UINT8 halSourcePhy_InterruptMaskStatus(IM_UINT16 baseAddr, IM_UINT8 mask);

void halSourcePhy_InterruptPolarity(IM_UINT16 baseAddr, IM_UINT8 bitShift, IM_UINT8 value);

IM_UINT8 halSourcePhy_InterruptPolarityStatus(IM_UINT16 baseAddr, IM_UINT8 mask);


#endif /* HALSOURCEPHY_H_ */
