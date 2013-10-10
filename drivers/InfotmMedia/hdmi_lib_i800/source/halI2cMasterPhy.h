/*
 * @file:halI2cMasterPhy.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 *      NOTE: only write operations are implemented in this module
 *   	This module is used only from PHY GEN2 on
 */

#ifndef HALI2CMASTERPHY_H_
#define HALI2CMASTERPHY_H_

#include "types.h"

void halI2cMasterPhy_SlaveAddress(IM_UINT16 baseAddr, IM_UINT8 value);

void halI2cMasterPhy_RegisterAddress(IM_UINT16 baseAddr, IM_UINT8 value);

void halI2cMasterPhy_WriteData(IM_UINT16 baseAddr, IM_UINT16 value);

void halI2cMasterPhy_WriteRequest(IM_UINT16 baseAddr);

void halI2cMasterPhy_FastMode(IM_UINT16 baseAddr, IM_UINT8 bit);

void halI2cMasterPhy_Reset(IM_UINT16 baseAddr);

//sam : 
void halI2cMasterPhy_ReadRequest(IM_UINT16 baseAddr);
IM_UINT16 halI2cMasterPhy_ReadData(IM_UINT16 baseAddr);


#endif /* HALI2CMASTERPHY_H_ */
