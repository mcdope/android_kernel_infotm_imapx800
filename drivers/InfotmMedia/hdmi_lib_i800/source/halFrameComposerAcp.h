/*
 * halFrameComposerAcp.h
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERACP_H_
#define HALFRAMECOMPOSERACP_H_

#include "types.h"

void halFrameComposerAcp_Type(IM_UINT16 baseAddr, IM_UINT8 type);

void halFrameComposerAcp_TypeDependentFields(IM_UINT16 baseAddr, IM_UINT8 * fields,
		IM_UINT8 fieldsLength);

#endif /* HALFRAMECOMPOSERACP_H_ */
