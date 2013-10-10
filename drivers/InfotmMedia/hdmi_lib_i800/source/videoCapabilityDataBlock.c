/*
 * videoCapabilityDataBlock.c
 *
 *  Created on: Jul 23, 2010
 *      Author: klabadi & dlopo
 */
#include "videoCapabilityDataBlock.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void videoCapabilityDataBlock_Reset(videoCapabilityDataBlock_t * vcdb)
{
	vcdb->mQuantizationRangeSelectable = FALSE;
	vcdb->mPreferredTimingScanInfo = 0;
	vcdb->mItScanInfo = 0;
	vcdb->mCeScanInfo = 0;
	vcdb->mValid = FALSE;
}

IM_INT32 videoCapabilityDataBlock_Parse(videoCapabilityDataBlock_t * vcdb, IM_UINT8 * data)
{
	LOG_TRACE();
	videoCapabilityDataBlock_Reset(vcdb);
	/* check tag code and extended tag */
	if (data != 0 && bitOperation_BitField(data[0], 5, 3) == 0x7
			&& bitOperation_BitField(data[1], 0, 8) == 0x0
			&& bitOperation_BitField(data[0], 0, 5) == 0x2)
	{ /* so far VCDB is 2 bytes long */
		vcdb->mCeScanInfo = bitOperation_BitField(data[2], 0, 2);
		vcdb->mItScanInfo = bitOperation_BitField(data[2], 2, 2);
		vcdb->mPreferredTimingScanInfo = bitOperation_BitField(data[2], 4, 2);
		vcdb->mQuantizationRangeSelectable = (bitOperation_BitField(data[2], 6,
				1) == 1) ? TRUE : FALSE;
		vcdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

IM_UINT8 videoCapabilityDataBlock_GetCeScanInfo(
		videoCapabilityDataBlock_t * vcdb)
{
	return vcdb->mCeScanInfo;
}

IM_UINT8 videoCapabilityDataBlock_GetItScanInfo(
		videoCapabilityDataBlock_t * vcdb)
{
	return vcdb->mItScanInfo;
}

IM_UINT8 videoCapabilityDataBlock_GetPreferredTimingScanInfo(
		videoCapabilityDataBlock_t * vcdb)
{
	return vcdb->mPreferredTimingScanInfo;
}

IM_INT32 videoCapabilityDataBlock_GetQuantizationRangeSelectable(videoCapabilityDataBlock_t * vcdb)
{
	return vcdb->mQuantizationRangeSelectable;
}
