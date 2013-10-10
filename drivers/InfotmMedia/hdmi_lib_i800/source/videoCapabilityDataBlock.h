/*
 * videoCapabilityDataBlock.h
 *
 *  Created on: Jul 23, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef VIDEOCAPABILITYDATABLOCK_H_
#define VIDEOCAPABILITYDATABLOCK_H_


#include "types.h"

/**
 * @file
 * Video Capability Data Block.
 * (videoCapabilityDataBlock_t * vcdbCEA Data Block Tag Code 0).
 * Parse and hold information from EDID data structure.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct
{
	IM_INT32 mQuantizationRangeSelectable;

	IM_UINT8 mPreferredTimingScanInfo;

	IM_UINT8 mItScanInfo;

	IM_UINT8 mCeScanInfo;

	IM_INT32 mValid;
} videoCapabilityDataBlock_t;

void videoCapabilityDataBlock_Reset(videoCapabilityDataBlock_t * vcdb);

int
		videoCapabilityDataBlock_Parse(videoCapabilityDataBlock_t * vcdb,
				IM_UINT8 * data);
/**
 * @return CE Overscan/Underscan behaviour
 * 0 0  CE video formats not supported
 * 0 1  Always Overscanned
 * 1 0  Always Underscanned
 * 1 1  Supports both over- and underscan
 */
IM_UINT8 videoCapabilityDataBlock_GetCeScanInfo(
		videoCapabilityDataBlock_t * vcdb);
/**
 * @return IT Overscan/Underscan behaviour
 * 0 0  IT video formats not supported
 * 0 1  Always Overscanned
 * 1 0  Always Underscanned
 * 1 1  Supports both over- and underscan
 */
IM_UINT8 videoCapabilityDataBlock_GetItScanInfo(
		videoCapabilityDataBlock_t * vcdb);
/**
 * @return Preferred Time Overscan/Underscan behaviour
 * 0 0 No Data (videoCapabilityDataBlock_t * vcdbrefer to CE or IT fields)
 * 0 1  Always Overscanned
 * 1 0  Always Underscanned
 * 1 1  Supports both over- and underscan
 */
IM_UINT8 videoCapabilityDataBlock_GetPreferredTimingScanInfo(
		videoCapabilityDataBlock_t * vcdb);

/**
 * @return TRUE if Quantization Range is Selectable
 */
IM_INT32 videoCapabilityDataBlock_GetQuantizationRangeSelectable(videoCapabilityDataBlock_t * vcdb);

#endif /* VIDEOCAPABILITYDATABLOCK_H_ */
