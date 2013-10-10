/*
 * @file:halFrameComposerVsd.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERVSD_H_
#define HALFRAMECOMPOSERVSD_H_

#include "types.h"
/*
 * Configure the 24 bit IEEE Registration Identifier
 * @param baseAddr Block base address
 * @param id vendor unique identifier
 */
void halFrameComposerVsd_VendorOUI(IM_UINT16 baseAddr, IM_UINT32 id);
/*
 * Configure the Vendor Payload to be carried by the InfoFrame
 * @param info array
 * @param length of the array
 * @return 0 when successful and 1 on error
 */
IM_UINT8 halFrameComposerVsd_VendorPayload(IM_UINT16 baseAddr, const IM_UINT8 * data,
		IM_UINT16 length);

#endif /* HALFRAMECOMPOSERVSD_H_ */
