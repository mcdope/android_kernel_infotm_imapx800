/*
 * access.h
 *
 *  Created on: Jun 25, 2010
 *
 * Synopsys Inc.
 * SG DWC PT02
 */
/**
 * @file 
 *      Access methods mask the the register access
 *      it accesses register using 16-bit addresses
 *      and 8-bit data
 *      @note: at least access_Write and access_Read should be re-implemented
 *      @note functions in this module are critical sections and should not be
 *      	interrupted
 *      @note: this implementation protects critical sections using muteces.
 *     		make sure critical section are protected from race conditions,
 *  	   	particularly when interrupts are enabled.
 */

#ifndef ACCESS_H_
#define ACCESS_H_

#include "types.h"
/**
 *Initialize communications with development board
 *@param baseAddr pointer to the address of the core on the bus
 *@return TRUE  if successful.
 */
IM_INT32 access_Initialize(IM_UINT8 * baseAddr);
/**
 *Read the contents of a register
 *@param addr of the register
 *@return 8bit byte containing the contents
 */
IM_UINT8 access_CoreReadByte(IM_UINT16 addr);
/**
 *Read several bits from a register
 *@param addr of the register
 *@param shift of the bit from the beginning
 *@param width or number of bits to read
 *@return the contents of the specified bits
 */
IM_UINT8 access_CoreRead(IM_UINT16 addr, IM_UINT8 shift, IM_UINT8 width);
/**
 *Write a byte to a register
 *@param data to be written to the register
 *@param addr of the register
 */
void access_CoreWriteByte(IM_UINT8 data, IM_UINT16 addr);
/**
 *Write to several bits in a register
 *@param data to be written to the required part
 *@param addr of the register
 *@param shift of the bits from the beginning
 *@param width or number of bits to written to
 */
void access_CoreWrite(IM_UINT8 data, IM_UINT16 addr, IM_UINT8 shift, IM_UINT8 width);

IM_UINT8 access_Read(IM_UINT16 addr);

void access_Write(IM_UINT8 data, IM_UINT16 addr);

#endif /* ACCESS_H_ */
