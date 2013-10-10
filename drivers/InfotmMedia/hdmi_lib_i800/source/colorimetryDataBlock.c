/*
 * colorimetryDataBlock.c
 *
 *  Created on: Jul 22, 2010
 *      Author: klabadi & dlopo
 */

#include "colorimetryDataBlock.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void colorimetryDataBlock_Reset(colorimetryDataBlock_t * cdb)
{
	cdb->mByte3 = 0;
	cdb->mByte4 = 0;
	cdb->mValid = FALSE;
}

IM_INT32 colorimetryDataBlock_Parse(colorimetryDataBlock_t * cdb, IM_UINT8 * data)
{
	LOG_TRACE();
	colorimetryDataBlock_Reset(cdb);
	if (data != 0 && bitOperation_BitField(data[0], 0, 5) == 0x03
			&& bitOperation_BitField(data[0], 5, 3) == 0x07
			&& bitOperation_BitField(data[1], 0, 7) == 0x05)
	{
		cdb->mByte3 = data[2];
		cdb->mByte4 = data[3];
		cdb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 colorimetryDataBlock_SupportsXvYcc709(colorimetryDataBlock_t * cdb)
{
	return (bitOperation_BitField(cdb->mByte3, 1, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 colorimetryDataBlock_SupportsXvYcc601(colorimetryDataBlock_t * cdb)
{
	return (bitOperation_BitField(cdb->mByte3, 0, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 colorimetryDataBlock_SupportsMetadata0(colorimetryDataBlock_t * cdb)
{
	return (bitOperation_BitField(cdb->mByte4, 0, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 colorimetryDataBlock_SupportsMetadata1(colorimetryDataBlock_t * cdb)
{
	return (bitOperation_BitField(cdb->mByte4, 1, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 colorimetryDataBlock_SupportsMetadata2(colorimetryDataBlock_t * cdb)
{
	return (bitOperation_BitField(cdb->mByte4, 2, 1) == 1) ? TRUE : FALSE;
}
