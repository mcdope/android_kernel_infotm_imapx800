/*
 * @file:bitOperation.h
 *
 *  Created on: Jul 5, 2010
  * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef BITOPERATION_H_
#define BITOPERATION_H_

#include "types.h"
/**
 * Concatenate two parts of two 8-bit bytes into a new 16-bit word
 * @param bHi first byte
 * @param oHi shift part of first byte (to be place as most significant
 * bits)
 * @param nHi width part of first byte (to be place as most significant
 * bits)
 * @param bLo second byte
 * @param oLo shift part of second byte (to be place as least
 * significant bits)
 * @param nLo width part of second byte (to be place as least
 * significant bits)
 * @returns 16-bit concatinated word as part of the first byte and part of
 * the second byte
 */
IM_UINT16 bitOperation_ConcatBits(IM_UINT8 bHi, IM_UINT8 oHi, IM_UINT8 nHi, IM_UINT8 bLo, IM_UINT8 oLo, IM_UINT8 nLo);

/** Concatinate two full bytes into a new 16-bit word
 * @param hi
 * @param lo
 * @returns hi as most segnificant bytes concatenated with lo as least
 * significant bits.
 */
IM_UINT16 bitOperation_Bytes2Word(const IM_UINT8 hi, const IM_UINT8 lo);

/** Extract the content of a certain part of a byte
 * @param data 8bit byte
 * @param shift shift from the start of the bit (0)
 * @param width width of the desired part starting from shift
 * @returns an 8bit byte holding only the desired part of the passed on
 * data byte
 */
IM_UINT8 bitOperation_BitField(const IM_UINT16 data, IM_UINT8 shift, IM_UINT8 width);

/** Concatenate four 8-bit bytes into a new 32-bit word
 * @param b3 assumed as most significant 8-bit byte
 * @param b2
 * @param b1
 * @param b0 assumed as least significant 8bit byte
 * @returns a 2D word, 32bits, composed of the 4 passed on parameters
 */
IM_UINT32 bitOperation_Bytes2Dword(IM_UINT8 b3, IM_UINT8 b2, IM_UINT8 b1, IM_UINT8 b0);

#endif /* BITOPERATION_H_ */
