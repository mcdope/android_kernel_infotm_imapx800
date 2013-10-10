/*
 * speakerAllocationDataBlock.c
 *
 *  Created on: Jul 22, 2010
 *      Author: klabadi & dlopo
 */

#include "speakerAllocationDataBlock.h"
#include "bitOperation.h"
#include "hdmi_log.h"

void speakerAllocationDataBlock_Reset(speakerAllocationDataBlock_t * sadb)
{
	sadb->mByte1 = 0;
	sadb->mValid = FALSE;
}

IM_INT32 speakerAllocationDataBlock_Parse(speakerAllocationDataBlock_t * sadb,
		IM_UINT8 * data)
{
	LOG_TRACE();
	speakerAllocationDataBlock_Reset(sadb);
	if (data != 0 && bitOperation_BitField(data[0], 0, 5) == 0x03
			&& bitOperation_BitField(data[0], 5, 3) == 0x04)
	{
		sadb->mByte1 = data[1];
		sadb->mValid = TRUE;
		return TRUE;
	}
	return FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsFlFr(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 0, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsLfe(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 1, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsFc(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 2, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsRlRr(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 3, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsRc(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 4, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsFlcFrc(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 5, 1) == 1) ? TRUE : FALSE;
}

IM_INT32 speakerAllocationDataBlock_SupportsRlcRrc(
		speakerAllocationDataBlock_t * sadb)
{
	return (bitOperation_BitField(sadb->mByte1, 6, 1) == 1) ? TRUE : FALSE;
}

IM_UINT8 speakerAllocationDataBlock_GetChannelAllocationCode(speakerAllocationDataBlock_t * sadb)
{
	/* TODO use an array instead */
	switch (sadb->mByte1)
	{
		case 1:
			return 0;
		case 3:
			return 1;
		case 5:
			return 2;
		case 7:
			return 3;
		case 17:
			return 4;
		case 19:
			return 5;
		case 21:
			return 6;
		case 23:
			return 7;
		case 9:
			return 8;
		case 11:
			return 9;
		case 13:
			return 10;
		case 15:
			return 11;
		case 25:
			return 12;
		case 27:
			return 13;
		case 29:
			return 14;
		case 31:
			return 15;
		case 73:
			return 16;
		case 75:
			return 17;
		case 77:
			return 18;
		case 79:
			return 19;
		case 33:
			return 20;
		case 35:
			return 21;
		case 37:
			return 22;
		case 39:
			return 23;
		case 49:
			return 24;
		case 51:
			return 25;
		case 53:
			return 26;
		case 55:
			return 27;
		case 41:
			return 28;
		case 43:
			return 29;
		case 45:
			return 30;
		case 47:
			return 31;
		default:
			return (u8) (-1);
	}
}

IM_UINT8 speakerAllocationDataBlock_GetSpeakerAllocationByte(speakerAllocationDataBlock_t * sadb)
{
	return sadb->mByte1;
}
