/*
 * hdcpParams.h
 *
 *  Created on: Jul 20, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */

#ifndef HDCPPARAMS_H_
#define HDCPPARAMS_H_

typedef struct
{
	IM_INT32 mEnable11Feature;
	IM_INT32 mRiCheck;
	IM_INT32 mI2cFastMode;
	IM_INT32 mEnhancedLinkVerification;
} hdcpParams_t;

void hdcpParams_Reset(hdcpParams_t *params);

IM_INT32 hdcpParams_GetEnable11Feature(hdcpParams_t *params);

IM_INT32 hdcpParams_GetRiCheck(hdcpParams_t *params);

IM_INT32 hdcpParams_GetI2cFastMode(hdcpParams_t *params);

IM_INT32 hdcpParams_GetEnhancedLinkVerification(hdcpParams_t *params);

void hdcpParams_SetEnable11Feature(hdcpParams_t *params, IM_INT32 value);

void hdcpParams_SetEnhancedLinkVerification(hdcpParams_t *params, IM_INT32 value);

void hdcpParams_SetI2cFastMode(hdcpParams_t *params, IM_INT32 value);

void hdcpParams_SetRiCheck(hdcpParams_t *params, IM_INT32 value);

#endif /* HDCPPARAMS_H_ */
