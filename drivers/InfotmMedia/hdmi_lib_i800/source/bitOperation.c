/*
 * @file:bitOperation.c
 *
 *  Created on: Jul 5, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#include "bitOperation.h"

	IM_UINT16 bitOperation_ConcatBits(IM_UINT8 bHi, IM_UINT8 oHi, IM_UINT8 nHi, IM_UINT8 bLo, IM_UINT8 oLo, IM_UINT8 nLo)
	{
		return (bitOperation_BitField(bHi, oHi, nHi) << nLo) | bitOperation_BitField(bLo, oLo, nLo);
	}

	IM_UINT16 bitOperation_Bytes2Word(const IM_UINT8 hi, const IM_UINT8 lo)
	{
		return bitOperation_ConcatBits(hi, 0, 8, lo, 0, 8);
	}

	IM_UINT8 bitOperation_BitField(const IM_UINT16 data, IM_UINT8 shift, IM_UINT8 width)
	{
		return ((data >> shift) & ((((u16)1) << width) - 1));
	}

	IM_UINT32 bitOperation_Bytes2Dword(IM_UINT8 b3, IM_UINT8 b2, IM_UINT8 b1, IM_UINT8 b0)
	{
		IM_UINT32 retval = 0;
		retval |= b0 << (0 * 8);
		retval |= b1 << (1 * 8);
		retval |= b2 << (2 * 8);
		retval |= b3 << (3 * 8);
		return retval;
	}

