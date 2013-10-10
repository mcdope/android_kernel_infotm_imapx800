/*
 * monitorRangeLimits.h
 *
 *  Created on: Jul 22, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef MONITORRANGELIMITS_H_
#define MONITORRANGELIMITS_H_

#include "types.h"
/**
 * @file
 * Second Monitor Descriptor
 * Parse and hold Monitor Range Limits information read from EDID
 */

typedef struct
{
	IM_UINT8 mMinVerticalRate;

	IM_UINT8 mMaxVerticalRate;

	IM_UINT8 mMinHorizontalRate;

	IM_UINT8 mMaxHorizontalRate;

	IM_UINT8 mMaxPixelClock;

	IM_INT32 mValid;
} monitorRangeLimits_t;

void monitorRangeLimits_Reset(monitorRangeLimits_t * mrl);

IM_INT32 monitorRangeLimits_Parse(monitorRangeLimits_t * mrl, IM_UINT8 * data);

/**
 * @return the maximum parameter for horizontal frequencies
 */
IM_UINT8 monitorRangeLimits_GetMaxHorizontalRate(monitorRangeLimits_t * mrl);
/**
 * @return the maximum parameter for pixel clock rate
 */
IM_UINT8 monitorRangeLimits_GetMaxPixelClock(monitorRangeLimits_t * mrl);
/**
 * @return the maximum parameter for vertical frequencies
 */
IM_UINT8 monitorRangeLimits_GetMaxVerticalRate(monitorRangeLimits_t * mrl);
/**
 * @return the minimum parameter for horizontal frequencies
 */
IM_UINT8 monitorRangeLimits_GetMinHorizontalRate(monitorRangeLimits_t * mrl);
/**
 * @return the minimum parameter for vertical frequencies
 */
IM_UINT8 monitorRangeLimits_GetMinVerticalRate(monitorRangeLimits_t * mrl);

#endif /* MONITORRANGELIMITS_H_ */
