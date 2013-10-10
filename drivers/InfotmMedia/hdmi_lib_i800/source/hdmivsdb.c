/*
 * hdmivsdb.c
 *
 *  Created on: Jul 21, 2010
 *      Author: klabadi & dlopo
 */

#include "hdmivsdb.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void hdmivsdb_Reset(hdmivsdb_t *vsdb)
{
	vsdb->mPhysicalAddress = 0;
	vsdb->mSupportsAi = FALSE;
	vsdb->mDeepColor30 = FALSE;
	vsdb->mDeepColor36 = FALSE;
	vsdb->mDeepColor48 = FALSE;
	vsdb->mDeepColorY444 = FALSE;
	vsdb->mDviDual = FALSE;
	vsdb->mMaxTmdsClk = 0;
	vsdb->mVideoLatency = 0;
	vsdb->mAudioLatency = 0;
	vsdb->mInterlacedVideoLatency = 0;
	vsdb->mInterlacedAudioLatency = 0;
	vsdb->mId = 0;
	vsdb->mLength = 0;
	vsdb->mValid = FALSE;
}
IM_INT32 hdmivsdb_Parse(hdmivsdb_t *vsdb, IM_UINT8 * data)
{
	IM_UINT8 blockLength = 0;
	LOG_TRACE();
	hdmivsdb_Reset(vsdb);
	if (data == 0)
	{
		return FALSE;
	}
	if (bitOperation_BitField(data[0], 5, 3) != 0x3)
	{
		LOG_WARNING("Invalid datablock tag");
		return FALSE;
	}
	blockLength = bitOperation_BitField(data[0], 0, 5);
	hdmivsdb_SetLength(vsdb, blockLength);
	if (blockLength < 5)
	{
		LOG_WARNING("Invalid minimum length");
		return FALSE;
	}
	if (bitOperation_Bytes2Dword(0x00, data[3], data[2], data[1]) != 0x000C03)
	{
		LOG_WARNING("HDMI IEEE registration identifier not valid");
		return FALSE;
	}
	hdmivsdb_Reset(vsdb);
	hdmivsdb_SetId(vsdb, 0x000C03);
	vsdb->mPhysicalAddress = bitOperation_Bytes2Word(data[4], data[5]);
	/* parse extension fields if they exist */
	if (blockLength > 5)
	{
		vsdb->mSupportsAi = bitOperation_BitField(data[6], 7, 1) == 1;
		vsdb->mDeepColor48 = bitOperation_BitField(data[6], 6, 1) == 1;
		vsdb->mDeepColor36 = bitOperation_BitField(data[6], 5, 1) == 1;
		vsdb->mDeepColor30 = bitOperation_BitField(data[6], 4, 1) == 1;
		vsdb->mDeepColorY444 = bitOperation_BitField(data[6], 3, 1) == 1;
		vsdb->mDviDual = bitOperation_BitField(data[6], 0, 1) == 1;
	}
	else
	{
		vsdb->mSupportsAi = FALSE;
		vsdb->mDeepColor48 = FALSE;
		vsdb->mDeepColor36 = FALSE;
		vsdb->mDeepColor30 = FALSE;
		vsdb->mDeepColorY444 = FALSE;
		vsdb->mDviDual = FALSE;
	}
	vsdb->mMaxTmdsClk = (blockLength > 6) ? data[7] : 0;
	vsdb->mVideoLatency = 0;
	vsdb->mAudioLatency = 0;
	vsdb->mInterlacedVideoLatency = 0;
	vsdb->mInterlacedAudioLatency = 0;
	if (blockLength > 7)
	{
		if (bitOperation_BitField(data[8], 7, 1) == 1)
		{
			if (blockLength < 10)
			{
				LOG_WARNING("Invalid length - latencies are not valid");
				return FALSE;
			}
			if (bitOperation_BitField(data[8], 6, 1) == 1)
			{
				if (blockLength < 12)
				{
					LOG_WARNING("Invalid length - Interlaced latencies are not valid");
					return FALSE;
				}
				else
				{
					vsdb->mVideoLatency = data[9];
					vsdb->mAudioLatency = data[10];
					vsdb->mInterlacedVideoLatency = data[11];
					vsdb->mInterlacedAudioLatency = data[12];
				}
			}
			else
			{
				vsdb->mVideoLatency = data[9];
				vsdb->mAudioLatency = data[10];
				vsdb->mInterlacedVideoLatency = data[9];
				vsdb->mInterlacedAudioLatency = data[10];
			}
		}
	}
	vsdb->mValid = TRUE;
	return TRUE;
}
IM_INT32 hdmivsdb_GetDeepColor30(hdmivsdb_t *vsdb)
{
	return vsdb->mDeepColor30;
}

IM_INT32 hdmivsdb_GetDeepColor36(hdmivsdb_t *vsdb)
{
	return vsdb->mDeepColor36;
}

IM_INT32 hdmivsdb_GetDeepColor48(hdmivsdb_t *vsdb)
{
	return vsdb->mDeepColor48;
}

IM_INT32 hdmivsdb_GetDeepColorY444(hdmivsdb_t *vsdb)
{
	return vsdb->mDeepColorY444;
}

IM_INT32 hdmivsdb_GetSupportsAi(hdmivsdb_t *vsdb)
{
	return vsdb->mSupportsAi;
}

IM_INT32 hdmivsdb_GetDviDual(hdmivsdb_t *vsdb)
{
	return vsdb->mDviDual;
}

IM_UINT8 hdmivsdb_GetMaxTmdsClk(hdmivsdb_t *vsdb)
{
	return vsdb->mMaxTmdsClk;
}

IM_UINT16 hdmivsdb_GetPVideoLatency(hdmivsdb_t *vsdb)
{
	return vsdb->mVideoLatency;
}

IM_UINT16 hdmivsdb_GetPAudioLatency(hdmivsdb_t *vsdb)
{
	return vsdb->mAudioLatency;
}

IM_UINT16 hdmivsdb_GetIAudioLatency(hdmivsdb_t *vsdb)
{
	return vsdb->mInterlacedAudioLatency;
}

IM_UINT16 hdmivsdb_GetIVideoLatency(hdmivsdb_t *vsdb)
{
	return vsdb->mInterlacedVideoLatency;
}

IM_UINT16 hdmivsdb_GetPhysicalAddress(hdmivsdb_t *vsdb)
{
	return vsdb->mPhysicalAddress;
}

IM_UINT32 hdmivsdb_GetId(hdmivsdb_t *vsdb)
{
	return vsdb->mId;
}

IM_UINT8 hdmivsdb_GetLength(hdmivsdb_t *vsdb)
{
	return vsdb->mLength;
}

void hdmivsdb_SetId(hdmivsdb_t *vsdb, IM_UINT32 id)
{
	vsdb->mId = id;
}

void hdmivsdb_SetLength(hdmivsdb_t *vsdb, IM_UINT8 length)
{
	vsdb->mLength = length;
}
