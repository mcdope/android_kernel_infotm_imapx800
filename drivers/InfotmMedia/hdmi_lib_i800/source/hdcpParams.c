/*
 * hdcpParams.c
 *
 *  Created on: Jul 21, 2010
 *      Author: klabadi
 */
#include <types.h>
#include "hdcpParams.h"

void hdcpParams_Reset(hdcpParams_t *params)
{
	params->mEnable11Feature = 0;
	params->mRiCheck = 0;
	params->mI2cFastMode = 0;
	params->mEnhancedLinkVerification = 0;
}

IM_INT32 hdcpParams_GetEnable11Feature(hdcpParams_t *params)
{
	return params->mEnable11Feature;
}

IM_INT32 hdcpParams_GetRiCheck(hdcpParams_t *params)
{
	return params->mRiCheck;
}

IM_INT32 hdcpParams_GetI2cFastMode(hdcpParams_t *params)
{
	return params->mI2cFastMode;
}

IM_INT32 hdcpParams_GetEnhancedLinkVerification(hdcpParams_t *params)
{
	return params->mEnhancedLinkVerification;
}

void hdcpParams_SetEnable11Feature(hdcpParams_t *params, IM_INT32 value)
{
	params->mEnable11Feature = value;
}

void hdcpParams_SetEnhancedLinkVerification(hdcpParams_t *params, IM_INT32 value)
{
	params->mEnhancedLinkVerification = value;
}

void hdcpParams_SetI2cFastMode(hdcpParams_t *params, IM_INT32 value)
{
	params->mI2cFastMode = value;
}

void hdcpParams_SetRiCheck(hdcpParams_t *params, IM_INT32 value)
{
	params->mRiCheck = value;
}
