/*
 * productParams.h
 *
 *  Created on: Jul 6, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef PRODUCTPARAMS_H_
#define PRODUCTPARAMS_H_

#include "types.h"

/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct
{
	/* Vendor Name of eight 7-bit ASCII characters */
	IM_UINT8 mVendorName[8];

	IM_UINT8 mVendorNameLength;

	/* Product name or description, consists of sixteen 7-bit ASCII characters */
	IM_UINT8 mProductName[16];

	IM_UINT8 mProductNameLength;

	/* Code that classifies the source device (CEA Table 15) */
	IM_UINT8 mSourceType;

	/* oui 24 bit IEEE Registration Identifier */
	IM_UINT32 mOUI;

	IM_UINT8 mVendorPayload[24];

	IM_UINT8 mVendorPayloadLength;

} productParams_t;

void productParams_Reset(productParams_t *params);

IM_UINT8 productParams_SetProductName(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length);

IM_UINT8 * productParams_GetProductName(productParams_t *params);

IM_UINT8 productParams_GetProductNameLength(productParams_t *params);

IM_UINT8 productParams_SetVendorName(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length);

IM_UINT8 * productParams_GetVendorName(productParams_t *params);

IM_UINT8 productParams_GetVendorNameLength(productParams_t *params);

IM_UINT8 productParams_SetSourceType(productParams_t *params, IM_UINT8 value);

IM_UINT8 productParams_GetSourceType(productParams_t *params);

IM_UINT8 productParams_SetOUI(productParams_t *params, IM_UINT32 value);

IM_UINT32 productParams_GetOUI(productParams_t *params);

IM_UINT8 productParams_SetVendorPayload(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length);

IM_UINT8 * productParams_GetVendorPayload(productParams_t *params);

IM_UINT8 productParams_GetVendorPayloadLength(productParams_t *params);

IM_UINT8 productParams_IsSourceProductValid(productParams_t *params);

IM_UINT8 productParams_IsVendorSpecificValid(productParams_t *params);

#endif /* PRODUCTPARAMS_H_ */
