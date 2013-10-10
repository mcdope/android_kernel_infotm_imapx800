/*
 * shortAudioDesc.c
 *
 *  Created on: Jul 22, 2010
 *      Author: klabadi & dlopo
 */
#include "shortAudioDesc.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void shortAudioDesc_Reset(shortAudioDesc_t *sad)
{
	sad->mFormat = 0;
	sad->mMaxChannels = 0;
	sad->mSampleRates = 0;
	sad->mByte3 = 0;
}

IM_INT32 shortAudioDesc_Parse(shortAudioDesc_t *sad, IM_UINT8 * data)
{
	LOG_TRACE();
	shortAudioDesc_Reset(sad);
	if (data != 0)
	{
		sad->mFormat = bitOperation_BitField(data[0], 3, 4);
		sad->mMaxChannels = bitOperation_BitField(data[0], 0, 3) + 1;
		sad->mSampleRates = bitOperation_BitField(data[1], 0, 7);
		sad->mByte3 = data[2];
		return TRUE;
	}
	return FALSE;
}

IM_UINT8 shortAudioDesc_GetByte3(shortAudioDesc_t *sad)
{
	return sad->mByte3;
}

IM_UINT8 shortAudioDesc_GetFormatCode(shortAudioDesc_t *sad)
{
	return sad->mFormat;
}

IM_UINT8 shortAudioDesc_GetMaxChannels(shortAudioDesc_t *sad)
{
	return sad->mMaxChannels;
}

IM_UINT8 shortAudioDesc_GetSampleRates(shortAudioDesc_t *sad)
{
	return sad->mSampleRates;
}

IM_INT32 shortAudioDesc_Support32k(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 0, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support44k1(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 1, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support48k(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 2, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support88k2(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 3, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support96k(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 4, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support176k4(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 5, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 shortAudioDesc_Support192k(shortAudioDesc_t *sad)
{
	return (bitOperation_BitField(sad->mSampleRates, 6, 1) == 1) ? TRUE : FALSE;
}

IM_UINT8 shortAudioDesc_GetMaxBitRate(shortAudioDesc_t *sad)
{
	if (sad->mFormat > 1 && sad->mFormat < 9)
	{
		return sad->mByte3;
	}
	LOG_WARNING("Information is not valid for this format");
	return 0;
}

IM_INT32 shortAudioDesc_Support16bit(shortAudioDesc_t *sad)
{
	if (sad->mFormat == 1)
	{
		return (bitOperation_BitField(sad->mByte3, 0, 1) == 1) ? TRUE : FALSE;
	}
	LOG_WARNING("Information is not valid for this format");
	return FALSE;
}

IM_INT32 shortAudioDesc_Support20bit(shortAudioDesc_t *sad)
{
	if (sad->mFormat == 1)
	{
		return (bitOperation_BitField(sad->mByte3, 1, 1) == 1) ? TRUE : FALSE;
	}
	LOG_WARNING("Information is not valid for this format");
	return FALSE;
}

IM_INT32 shortAudioDesc_Support24bit(shortAudioDesc_t *sad)
{
	if (sad->mFormat == 1)
	{
		return (bitOperation_BitField(sad->mByte3, 2, 1) == 1) ? TRUE : FALSE;
	}
	LOG_WARNING("Information is not valid for this format");
	return FALSE;
}
