/*
 * edid.c
 *
 *  Created on: Jul 23, 2010
 *      Author: klabadi & dlopo
 */

#include "edid.h"
#include "hdmi_system.h"
#include "bitOperation.h"
#include "hdmi_log.h"
#include "error.h"

const IM_UINT16 I2CM_BASE_ADDR = 0x7E00;

IM_UINT8 edid_mBlocksNo;
/**
 * Local variable hold the number of the currently read byte in a certain block of the EDID structure
 */
IM_UINT8 edid_mCurrAddress;
/**
 * Local variable hold the number of the currently read block in the EDID structure.
 */
IM_UINT8 edid_mCurrBlockNo;
/**
 * The accumulator of the block check sum value.
 */
IM_UINT8 edid_mBlockSum;
/**
 * Local variable to hold the status when the reading of the whole EDID structure
 */
edid_status_t edid_mStatus = EDID_IDLE;
/**
 * Local variable holding the buffer of 128 read bytes.
 */
IM_UINT8 edid_mBuffer[0x80];
/**
 * Array to hold all the parsed Detailed Timing Descriptors.
 */
dtd_t edid_mDtd[32];

IM_UINT32 edid_mDtdIndex;
/**
 * array to hold all the parsed Short Video Descriptors.
 */
shortVideoDesc_t edid_mSvd[128];

IM_UINT32 edid_mSvdIndex;
/**
 * array to hold all the parsed Short Audio Descriptors.
 */
shortAudioDesc_t edid_mSad[128];

IM_UINT32 edid_mSadIndex;
/**
 * A string to hold the Monitor Name parsed from EDID.
 */
IM_INT8 edid_mMonitorName[13];

IM_INT32 edid_mYcc444Support;

IM_INT32 edid_mYcc422Support;

IM_INT32 edid_mBasicAudioSupport;

IM_INT32 edid_mUnderscanSupport;

hdmivsdb_t edid_mHdmivsdb;

monitorRangeLimits_t edid_mMonitorRangeLimits;

videoCapabilityDataBlock_t edid_mVideoCapabilityDataBlock;

colorimetryDataBlock_t edid_mColorimetryDataBlock;

speakerAllocationDataBlock_t edid_mSpeakerAllocationDataBlock;

IM_INT32 edid_Initialize(IM_UINT16 baseAddr, IM_UINT16 sfrClock)
{
	LOG_TRACE2(baseAddr, sfrClock);
	if (sfrClock != 2500)
	{
		error_Set(ERR_SFR_CLOCK_NOT_SUPPORTED);
		LOG_ERROR("SFR clock is not 25MHz");
		return FALSE;
	}

	halEdid_MaskInterrupts(baseAddr + I2CM_BASE_ADDR, TRUE);
	edid_Reset(baseAddr);
	/* set clock division - SFR is always 25MHz (TODO) */
	halEdid_MasterClockDivision(baseAddr + I2CM_BASE_ADDR, 0x05);
	halEdid_StandardSpeedCounter(baseAddr + I2CM_BASE_ADDR, 0x00790091);
	halEdid_FastSpeedCounter(baseAddr + I2CM_BASE_ADDR, 0x000F0021);

	/* set address to EDID address -see spec */
	halEdid_SlaveAddress(baseAddr + I2CM_BASE_ADDR, 0x50); /* HW deals with LSB (alternating the address between 0xA0 & 0xA1) */
	halEdid_SegmentAddr(baseAddr + I2CM_BASE_ADDR, 0x30); /* HW deals with LSB (making the segment address go to 60) */

	/* read EDID */
	edid_mStatus = EDID_READING;
	LOG_NOTICE("reading EDID");
	halEdid_MaskInterrupts(baseAddr + I2CM_BASE_ADDR, FALSE); /* enable interrupts */
	edid_ReadRequest(baseAddr, 0, 0);
	return TRUE;
}

IM_INT32 edid_Standby(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	halEdid_MaskInterrupts(baseAddr + I2CM_BASE_ADDR, TRUE); /* disable intterupts */
	edid_mStatus = EDID_IDLE;
	return TRUE;
}

IM_UINT8 edid_EventHandler(IM_UINT16 baseAddr, IM_INT32 hpd, IM_UINT8 state)
{
	LOG_TRACE2(hpd, state);
	if (edid_mStatus != EDID_READING)
	{
		return EDID_IDLE;
	}
	else if (!hpd)
	{ /* hot plug detected without cable disconnection */
		error_Set(ERR_HPD_LOST);
		LOG_WARNING("hpd");
		edid_mStatus = EDID_ERROR;
	}
	else if ((state & BIT(0)) != 0) /* error */
	{ 
		LOG_WARNING("error");
		edid_mStatus = EDID_ERROR;
	}
	else if ((state & BIT(1)) != 0) /* done */
	{ 
		if (edid_mCurrAddress >= sizeof(edid_mBuffer))
		{
			error_Set(ERR_OVERFLOW);
			LOG_WARNING("overflow");
			edid_mStatus = EDID_ERROR;
		}
		else
		{
			edid_mBuffer[edid_mCurrAddress] = halEdid_ReadData(baseAddr
					+ I2CM_BASE_ADDR);
			edid_mBlockSum += edid_mBuffer[edid_mCurrAddress++];
			if (edid_mCurrAddress >= sizeof(edid_mBuffer))
			{
				/*check if checksum is correct (CEA-861 D Spec p108) */
				if (edid_mBlockSum % 0x100 == 0)
				{
					LOG_NOTICE("block checksum correct");
					edid_ParseBlock(baseAddr, edid_mBuffer);
					edid_mCurrAddress = 0;
					edid_mCurrBlockNo++;
					edid_mBlockSum = 0;
					if (edid_mCurrBlockNo >= edid_mBlocksNo)
					{
						edid_mStatus = EDID_DONE; 
					}
				}
				else
				{
					error_Set(ERR_BLOCK_CHECKSUM_INVALID);
					LOG_WARNING("block checksum invalid");
					edid_mStatus = EDID_ERROR; 
				}
			}
		}
	}
	if (edid_mStatus == EDID_READING)
	{
		edid_ReadRequest(baseAddr, edid_mCurrAddress, edid_mCurrBlockNo);
	}
	else if (edid_mStatus == EDID_DONE)
	{
		edid_mBlocksNo = 1;
		edid_mCurrBlockNo = 0;
		edid_mCurrAddress = 0;
		edid_mBlockSum = 0;
	}
	return edid_mStatus;
}

IM_UINT32 edid_GetDtdCount(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return edid_mDtdIndex;
}

IM_INT32 edid_GetDtd(IM_UINT16 baseAddr, IM_UINT32 n, dtd_t * dtd)
{
	LOG_TRACE1(n);
	if (n < edid_GetDtdCount(baseAddr))
	{
		*dtd = edid_mDtd[n];
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_GetHdmivsdb(IM_UINT16 baseAddr, hdmivsdb_t * vsdb)
{
	LOG_TRACE();
	if (edid_mHdmivsdb.mValid)
	{
		*vsdb = edid_mHdmivsdb;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_GetMonitorName(IM_UINT16 baseAddr, IM_INT8 * name, IM_UINT32 length)
{
	IM_UINT32 i = 0;
	LOG_TRACE();
	for (i = 0; i < length && i < sizeof(edid_mMonitorName); i++)
	{
		name[i] = edid_mMonitorName[i];
	}
	return i;
}

IM_INT32 edid_GetMonitorRangeLimits(IM_UINT16 baseAddr, monitorRangeLimits_t * limits)
{
	LOG_TRACE();
	if (edid_mMonitorRangeLimits.mValid)
	{
		*limits = edid_mMonitorRangeLimits;
		return TRUE;
	}
	return FALSE;
}

IM_UINT32 edid_GetSvdCount(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return edid_mSvdIndex;
}

IM_INT32 edid_GetSvd(IM_UINT16 baseAddr, IM_UINT32 n, shortVideoDesc_t * svd)
{
	LOG_TRACE1(n);
	if (n < edid_GetSvdCount(baseAddr))
	{
		*svd = edid_mSvd[n];
		return TRUE;
	}
	return FALSE;
}

IM_UINT32 edid_GetSadCount(IM_UINT16 baseAddr)
{
	LOG_TRACE();
	return edid_mSadIndex;
}

IM_INT32 edid_GetSad(IM_UINT16 baseAddr, IM_UINT32 n, shortAudioDesc_t * sad)
{
	LOG_TRACE1(n);
	if (n < edid_GetSadCount(baseAddr))
	{
		*sad = edid_mSad[n];
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_GetVideoCapabilityDataBlock(IM_UINT16 baseAddr,
		videoCapabilityDataBlock_t * capability)
{
	LOG_TRACE();
	if (edid_mVideoCapabilityDataBlock.mValid)
	{
		*capability = edid_mVideoCapabilityDataBlock;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_GetSpeakerAllocationDataBlock(IM_UINT16 baseAddr,
		speakerAllocationDataBlock_t * allocation)
{
	LOG_TRACE();
	if (edid_mSpeakerAllocationDataBlock.mValid)
	{
		*allocation = edid_mSpeakerAllocationDataBlock;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_GetColorimetryDataBlock(IM_UINT16 baseAddr,
		colorimetryDataBlock_t * colorimetry)
{
	LOG_TRACE();
	if (edid_mColorimetryDataBlock.mValid)
	{
		*colorimetry = edid_mColorimetryDataBlock;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 edid_SupportsBasicAudio(IM_UINT16 baseAddr)
{
	return edid_mBasicAudioSupport;
}

IM_INT32 edid_SupportsUnderscan(IM_UINT16 baseAddr)
{
	return edid_mUnderscanSupport;
}

IM_INT32 edid_SupportsYcc422(IM_UINT16 baseAddr)
{
	return edid_mYcc422Support;
}

IM_INT32 edid_SupportsYcc444(IM_UINT16 baseAddr)
{
	return edid_mYcc444Support;
}

IM_INT32 edid_Reset(IM_UINT16 baseAddr)
{
	IM_UINT32 i = 0;
	LOG_TRACE();
	edid_mBlocksNo = 1;
	edid_mCurrBlockNo = 0;
	edid_mCurrAddress = 0;
	edid_mBlockSum = 0;
	edid_mStatus = EDID_READING; 
	for (i = 0; i < sizeof(edid_mBuffer); i++)
	{
		edid_mBuffer[i] = 0;
	}
	for (i = 0; i < sizeof(edid_mMonitorName); i++)
	{
		edid_mMonitorName[i] = 0;
	}
	edid_mBasicAudioSupport = FALSE;
	edid_mUnderscanSupport = FALSE;
	edid_mYcc422Support = FALSE;
	edid_mYcc444Support = FALSE;
	edid_mDtdIndex = 0;
	edid_mSadIndex = 0;
	edid_mSvdIndex = 0;
	hdmivsdb_Reset(&edid_mHdmivsdb);
	monitorRangeLimits_Reset(&edid_mMonitorRangeLimits);
	videoCapabilityDataBlock_Reset(&edid_mVideoCapabilityDataBlock);
	colorimetryDataBlock_Reset(&edid_mColorimetryDataBlock);
	speakerAllocationDataBlock_Reset(&edid_mSpeakerAllocationDataBlock);
	return TRUE;
}

IM_INT32 edid_ReadRequest(IM_UINT16 baseAddr, IM_UINT8 address, IM_UINT8 blockNo)
{
	/*to incorporate extensions we have to include the following - see VESA E-DDC spec. P 11 */
	IM_UINT8 sPointer = blockNo / 2;
	IM_UINT8 edidAddress = ((blockNo % 2) * 0x80) + address;

	LOG_TRACE2(sPointer, edidAddress);
	halEdid_RequestAddr(baseAddr + I2CM_BASE_ADDR, edidAddress);
	halEdid_SegmentPointer(baseAddr + I2CM_BASE_ADDR, sPointer);
	if (sPointer == 0)
	{
		halEdid_RequestRead(baseAddr + I2CM_BASE_ADDR);
	}
	else
	{
		halEdid_RequestExtRead(baseAddr + I2CM_BASE_ADDR);
	}
	return TRUE;
}

IM_INT32 edid_ParseBlock(IM_UINT16 baseAddr, IM_UINT8 * buffer)
{
	const IM_UINT32 DTD_SIZE = 0x12;
	const IM_UINT32 TIMING_MODES = 0x36;
	dtd_t tmpDtd;
	IM_UINT32 c = 0;
	IM_UINT32 i = 0;
	LOG_TRACE();
	if (edid_mCurrBlockNo == 0)
	{
		if (buffer[0] == 0x00)
		{ /* parse block zero */
			edid_mBlocksNo = buffer[126] + 1;
			/* parse DTD's */
			for (i = TIMING_MODES; i < (TIMING_MODES + (DTD_SIZE * 4)); i
					+= DTD_SIZE)
			{
				if (bitOperation_Bytes2Word(buffer[i + 1], buffer[i + 0]) > 0)
				{
					if (dtd_Parse(&tmpDtd, buffer + i) == TRUE)
					{
						if (edid_mDtdIndex < (sizeof(edid_mDtd)
								/ sizeof(dtd_t)))
						{
							edid_mDtd[edid_mDtdIndex++] = tmpDtd;
						}
						else
						{
							error_Set(ERR_DTD_BUFFER_FULL);
							LOG_WARNING("buffer full - DTD ignored");
						}
					}
					else
					{
						LOG_WARNING("DTD corrupt");
					}
				}
				else if ((buffer[i + 2] == 0) && (buffer[i + 4] == 0))
				{
					/* it is a Display-monitor Descriptor */
					if (buffer[i + 3] == 0xFC)
					{ /* Monitor Name */
						for (c = 0; c < 13; c++)
						{
							edid_mMonitorName[c] = buffer[i + c + 5];
						}
					}
					else if (buffer[i + 3] == 0xFD)
					{ /* Monitor Range Limits */
						if (monitorRangeLimits_Parse(&edid_mMonitorRangeLimits,
								buffer + i) != TRUE)
						{
							LOG_WARNING2("Monitor Range Limits corrupt", i);
						}
					}
				}
			}
		}
	}
	else if (buffer[0] == 0xF0)
	{ /* Block Map Extension */
		/* last is checksum */
		for (i = 1; i < (sizeof(edid_mBuffer) - 1); i++)
		{
			if (buffer[i] == 0x00)
			{
				break;
			}
		}
		if (edid_mBlocksNo < 128)
		{
			if (i > edid_mBlocksNo)
			{ /* N (no of extensions) does NOT include Block maps */
				edid_mBlocksNo += 2;
			}
			else if (i == edid_mBlocksNo)
			{
				edid_mBlocksNo += 1;
			}
		}
		else
		{
			i += 127;
			if (i > edid_mBlocksNo)
			{ /* N (no of extensions) does NOT include Block maps */
				edid_mBlocksNo += 3;
			}
			else if (i == edid_mBlocksNo)
			{
				edid_mBlocksNo += 1;
			}
		}
	}
	else if (buffer[0] == 0x02)
	{ /* CEA Extension block */
		if (buffer[1] == 0x03)
		{ /* revision number (only rev3 is allowed by HDMI spec) */
			IM_UINT8 offset = buffer[2];
			edid_mYcc422Support = bitOperation_BitField(buffer[3], 4, 1) == 1;
			edid_mYcc444Support = bitOperation_BitField(buffer[3], 5, 1) == 1;
			edid_mBasicAudioSupport = bitOperation_BitField(buffer[3], 6, 1)
					== 1;
			edid_mUnderscanSupport = bitOperation_BitField(buffer[3], 7, 1)
					== 1;
			if (offset != 4)
			{
				for (i = 4; i < offset; i += edid_ParseDataBlock(baseAddr,
						buffer + i))
					;
			}
			/* last is checksum */
			for (i = offset, c = 0; i < (sizeof(edid_mBuffer) - 1) && c < 6; i
					+= DTD_SIZE, c++)
			{
				if (dtd_Parse(&tmpDtd, buffer + i) == TRUE)
				{
					if (edid_mDtdIndex < (sizeof(edid_mDtd) / sizeof(dtd_t)))
					{
						edid_mDtd[edid_mDtdIndex++] = tmpDtd;
					}
					else
					{
						error_Set(ERR_DTD_BUFFER_FULL);
						LOG_WARNING("buffer full - DTD ignored");
					}
				}
			}
		}
	}
	return TRUE;
}

IM_UINT8 edid_ParseDataBlock(IM_UINT16 baseAddr, IM_UINT8 * data)
{
	IM_UINT8 tag = bitOperation_BitField(data[0], 5, 3);
	IM_UINT8 length = bitOperation_BitField(data[0], 0, 5);
	IM_UINT8 c = 0;
	shortAudioDesc_t tmpSad;
	shortVideoDesc_t tmpSvd;
	LOG_TRACE3("TAG", tag, length);
	switch (tag)
	{
	case 0x1: /* Audio Data Block */
		for (c = 1; c < (length + 1); c += 3)
		{
			shortAudioDesc_Parse(&tmpSad, data + c);
			if (edid_mSadIndex < (sizeof(edid_mSad) / sizeof(shortAudioDesc_t)))
			{
				edid_mSad[edid_mSadIndex++] = tmpSad;
			}
			else
			{
				error_Set(ERR_SHORT_AUDIO_DESC_BUFFER_FULL);
				LOG_WARNING("buffer full - SAD ignored");
			}
		}
		break;
	case 0x2: /* Video Data Block */
		for (c = 1; c < (length + 1); c++)
		{
			shortVideoDesc_Parse(&tmpSvd, data[c]);
			if (edid_mSvdIndex < (sizeof(edid_mSvd) / sizeof(shortVideoDesc_t)))
			{
				edid_mSvd[edid_mSvdIndex++] = tmpSvd;
			}
			else
			{
				error_Set(ERR_SHORT_VIDEO_DESC_BUFFER_FULL);
				LOG_WARNING("buffer full - SVD ignored");
			}
		}
		break;
	case 0x3: /* Vendor Specific Data Block */
	{
		IM_UINT32 ieeeId = bitOperation_Bytes2Dword(0x00, data[3], data[2], data[1]);
		if (ieeeId == 0x000C03)
		{ /* HDMI */
			if (hdmivsdb_Parse(&edid_mHdmivsdb, data) != TRUE)
			{
				LOG_WARNING("HDMI Vendor Specific Data Block corrupt");
			}
		}
		else
		{
			LOG_WARNING2("Vendor Specific Data Block not parsed", ieeeId);
		}
		break;
	}
	case 0x4: /* Speaker Allocation Data Block */
		if (speakerAllocationDataBlock_Parse(&edid_mSpeakerAllocationDataBlock,
				data) != TRUE)
		{
			LOG_WARNING("Speaker Allocation Data Block corrupt");
		}
		break;
	case 0x7:
	{
		IM_UINT8 extendedTag = data[1];
		switch (extendedTag)
		{
		case 0x00: /* Video Capability Data Block */
			if (videoCapabilityDataBlock_Parse(&edid_mVideoCapabilityDataBlock,
					data) != TRUE)
			{
				LOG_WARNING("Video Capability Data Block corrupt");
			}
			break;
		case 0x05: /* Colorimetry Data Block */
			if (colorimetryDataBlock_Parse(&edid_mColorimetryDataBlock, data)
					!= TRUE)
			{
				LOG_WARNING("Colorimetry Data Block corrupt");
			}
			break;
		case 0x04: /* HDMI Video Data Block */
		case 0x12: /* HDMI Audio Data Block */
			break;
		default:
			LOG_WARNING2("Extended Data Block not parsed", extendedTag);
			break;
		}
		break;
	}
	default:
		LOG_WARNING2("Data Block not parsed", tag);
		break;
	}
	return length + 1;
}
