/*
 * @file:productParams.c
 *
 *  Created on: Jul 6, 2010
 *      Author: klabadi & dlopo
 */

#include "productParams.h"
#include "hdmi_log.h"

void productParams_Reset(productParams_t *params)
{
	IM_UINT32 i = 0;
	params->mVendorNameLength = 0;
	params->mProductNameLength = 0;
	params->mSourceType = (u8) (-1);
	params->mOUI = (u8) (-1);
	params->mVendorPayloadLength = 0;
	for (i = 0; i < sizeof(params->mVendorName); i++)
	{
		params->mVendorName[i] = 0;
	}
	for (i = 0; i < sizeof(params->mProductName); i++)
	{
		params->mVendorName[i] = 0;
	}
	for (i = 0; i < sizeof(params->mVendorPayload); i++)
	{
		params->mVendorName[i] = 0;
	}
}

IM_UINT8 productParams_SetProductName(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length)
{
	IM_UINT16 i = 0;
	if ((data == 0) || length > sizeof(params->mProductName))
	{
		LOG_ERROR("invalid parameter");
		return 1;
	}
	params->mProductNameLength = 0;
	for (i = 0; i < sizeof(params->mProductName); i++)
	{
		params->mProductName[i] = (i < length) ? data[i] : 0;
	}
	params->mProductNameLength = length;
	return 0;
}

IM_UINT8 * productParams_GetProductName(productParams_t *params)
{
	return params->mProductName;
}

IM_UINT8 productParams_GetProductNameLength(productParams_t *params)
{
	return params->mProductNameLength;
}

IM_UINT8 productParams_SetVendorName(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length)
{
	IM_UINT16 i = 0;
	if (data == 0 || length > sizeof(params->mVendorName))
	{
		LOG_ERROR("invalid parameter");
		return 1;
	}
	params->mVendorNameLength = 0;
	for (i = 0; i < sizeof(params->mVendorName); i++)
	{
		params->mVendorName[i] = (i < length) ? data[i] : 0;
	}
	params->mVendorNameLength = length;
	return 0;
}

IM_UINT8 * productParams_GetVendorName(productParams_t *params)
{
	return params->mVendorName;
}

IM_UINT8 productParams_GetVendorNameLength(productParams_t *params)
{
	return params->mVendorNameLength;
}

IM_UINT8 productParams_SetSourceType(productParams_t *params, IM_UINT8 value)
{
	params->mSourceType = value;
	return 0;
}

IM_UINT8 productParams_GetSourceType(productParams_t *params)
{
	return params->mSourceType;
}

IM_UINT8 productParams_SetOUI(productParams_t *params, IM_UINT32 value)
{
	params->mOUI = value;
	return 0;
}

IM_UINT32 productParams_GetOUI(productParams_t *params)
{
	return params->mOUI;
}

IM_UINT8 productParams_SetVendorPayload(productParams_t *params, const IM_UINT8 * data,
		IM_UINT8 length)
{
	IM_UINT16 i = 0;
	if (data == 0 || length > sizeof(params->mVendorPayload))
	{
		LOG_ERROR("invalid parameter");
		return 1;
	}
	params->mVendorPayloadLength = 0;
	for (i = 0; i < sizeof(params->mVendorPayload); i++)
	{
		params->mVendorPayload[i] = (i < length) ? data[i] : 0;
	}
	params->mVendorPayloadLength = length;
	return 0;
}

IM_UINT8 * productParams_GetVendorPayload(productParams_t *params)
{
	return params->mVendorPayload;
}

IM_UINT8 productParams_GetVendorPayloadLength(productParams_t *params)
{
	return params->mVendorPayloadLength;
}

IM_UINT8 productParams_IsSourceProductValid(productParams_t *params)
{
	return productParams_GetSourceType(params) != (u8) (-1)
			&& productParams_GetVendorNameLength(params) != 0
			&& productParams_GetProductNameLength(params) != 0;
}

IM_UINT8 productParams_IsVendorSpecificValid(productParams_t *params)
{
	return productParams_GetOUI(params) != (u32) (-1)
			&& productParams_GetVendorPayloadLength(params) != 0;
}
