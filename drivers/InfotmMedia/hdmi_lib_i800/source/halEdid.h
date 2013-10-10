/*
 * halEdid.h
 *
 *  Created on: Jul 5, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef HALEDID_H_
#define HALEDID_H_

#include "types.h"

/**
 *  Set the slave address in the I2CM registers.
 * @param baseAddr base address of the EDID module registers
 * @param addr the I2C slave address
 */
void halEdid_SlaveAddress(IM_UINT16 baseAddr, IM_UINT8 addr);

/** Set the address of the byte accessed at the slave device.
 * @param baseAddr base address of the EDID module registers
 * @param addr the address of the accessed byte
 */
void halEdid_RequestAddr(IM_UINT16 baseAddr, IM_UINT8 addr);

/** Write data that is to be sent to slave device
 * @param baseAddr base address of the EDID module registers
 * @param data a data byte to be sent to the slave device
 */
void halEdid_WriteData(IM_UINT16 baseAddr, IM_UINT8 data);

/** Read data received and stored in HW buffer.
 * @param baseAddr base address of the EDID module registers
 * @returns a read byte from the slave device
 */
IM_UINT8 halEdid_ReadData(IM_UINT16 baseAddr);

/** Send a read request to the slave device with the already requested address.
 */
void halEdid_RequestRead(IM_UINT16 baseAddr);

/** Send an extended read request to the slave device with the already requested address (IM_UINT16 baseAddr, E-DDC).
 * @param baseAddr base address of the EDID module registers
 */
void halEdid_RequestExtRead(IM_UINT16 baseAddr);

/** Send a write request to the slave device with the already requested address and data to be written
 * @param baseAddr base address of the EDID module registers
 */
void halEdid_RequestWrite(IM_UINT16 baseAddr);

/** Set the clock division from the SFR clock
 * @param baseAddr base address of the EDID module registers
 * @param value of clockl division (refer to HDMITXCTRL datasheet)
 */
void halEdid_MasterClockDivision(IM_UINT16 baseAddr, IM_UINT8 value);

/** Set the segment address. [to 0x60 for E-DDC (IM_UINT16 baseAddr, EDID)].
 * @param baseAddr base address of the EDID module registers
 * @param addr the segment address [set to 0x30 because HW inserts a 0 to the right].
 */
void halEdid_SegmentAddr(IM_UINT16 baseAddr, IM_UINT8 addr);

/** Set the segment pointer. (IM_UINT16 baseAddr, should follow the formula N/2 where N is the block number).
 * @param baseAddr base address of the EDID module registers
 * @param pointer the pointer to the segment to be accessed.
 */
void halEdid_SegmentPointer(IM_UINT16 baseAddr, IM_UINT8 pointer);

/** Modify the error interrupt mask.
 * @param baseAddr base address of the EDID module registers
 * @param mask to enable or disable the masking (IM_UINT16 baseAddr, true to mask, ie true to stop seeing the intrrupt).
 */
void halEdid_MaskInterrupts(IM_UINT16 baseAddr, IM_UINT8 mask);

/** Set the clock division from the SFR clock
 * @param baseAddr base address of the EDID module registers
 * @param value of fast speed counter (refer to HDMITXCTRL datasheet)
 */
void halEdid_FastSpeedCounter(IM_UINT16 baseAddr, IM_UINT32 value);

/** Set the clock division from the SFR clock
 * @param baseAddr base address of the EDID module registers
 * @param value of standard speed counter (refer to HDMITXCTRL datasheet)
 */
void halEdid_StandardSpeedCounter(IM_UINT16 baseAddr, IM_UINT32 value);

#endif /* HALEDID_H_ */
