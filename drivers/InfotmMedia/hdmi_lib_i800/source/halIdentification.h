/*
 * @file:halIdentification.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALIDENTIFICATION_H_
#define HALIDENTIFICATION_H_

#include "types.h"

IM_UINT8 halIdentification_Design(IM_UINT16 baseAddr);

IM_UINT8 halIdentification_Revision(IM_UINT16 baseAddr);

IM_UINT8 halIdentification_ProductLine(IM_UINT16 baseAddr);

IM_UINT8 halIdentification_ProductType(IM_UINT16 baseAddr);

#endif /* HALIDENTIFICATION_H_ */
