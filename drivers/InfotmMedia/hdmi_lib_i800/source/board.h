/*
 * board.h
 *
 *  Created on: Jun 28, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */
/**
 * @file
 * board specific functions
 * configure onboard PLLs and multiplexers
 * 	@note: this file should be re-written to match the environment the 
 * 	API is running on
 */

#ifndef BOARD_H_
#define BOARD_H_

#include "types.h"

/** Initialize board
 * @param baseAddr base address of controller
 * @param pixelClock pixel clock [10KHz]
 * @param cd color depth (8, 10, 12 or 16)
 * @return TRUE if successful
 */
IM_INT32 board_Initialize(IM_UINT16 baseAddr, IM_UINT16 pixelClock, IM_UINT8 cd);

/** Set up oscillator for audio
 * @param baseAddr base address of controller
 * @param value: audio clock [Hz]
 * @return TRUE if successful
 */
IM_INT32 board_AudioClock(IM_UINT16 baseAddr, IM_UINT32 value);

/** Set up oscillator for video
 * @param baseAddr base address of controller
 * @param value: pixel clock [10KHz]
 * @param cd color depth
 * @return TRUE if successful
 */
IM_INT32 board_PixelClock(IM_UINT16 baseAddr, IM_UINT16 value, IM_UINT8 cd);

/**  Write to DRP controller which allows to change the PLL parameters
 *  dynamically - controlling the input signals of the controller
 *  @note: this is not included in package but rather board specific
 *  @param pllBaseAddress 16-bit base address of a DRP PLL controller
 *  @param regAddr 8-bit register address in the DRP PLL controller
 *  @param data 8-bit data to be written to the refereced register
 */
void board_PllWrite(IM_UINT16 pllBaseAddress, IM_UINT8 regAddr, IM_UINT16 data);

/** Bypass internal static frame video generator
 * @param baseAddr base address of controller
 * @param enable 0 off, 1 on (ie external video through)
 */
void board_VideoGeneratorBypass(IM_UINT16 baseAddr, IM_UINT8 enable);

/** Read value in a specific register in a DRP PLL controller
 *  @param pllBaseAddress 16-bit base address of a DRP PLL controller
 *  @param regAddr 8-bit register address in the DRP PLL controller
 */
IM_UINT16 board_PllRead(IM_UINT16 pllBaseAddress, IM_UINT8 regAddr);

/** Reset the DRP PLL controller
 * Usually called before and after setting of controller
 * @param pllBaseAddress 16-bit base address of a DRP PLL controller
 * @param value
 */
void board_PllReset(IM_UINT16 pllBaseAddress, IM_UINT8 value);

/** Bypass internal audio generator
 * @param baseAddr base address of controller
 * @param enable 0 off, 1 on (ie external audio through)
 */
void board_AudioGeneratorBypass(IM_UINT16 baseAddr, IM_UINT8 enable);

/** Return the refresh rates supported by the board for the CEA codes
 * @return IM_UINT32 refresh rate  in ([Hz] x 100) for supported modes, -1 if not
 * @note: refresh rates affect pixel clocks, which not all are
 * supported by the demo board 
 * (eg. for 25.18MHz and 25.2Mhz, only  25.2MHz is supported)
 */
IM_UINT32 board_SupportedRefreshRate(IM_UINT8 code);

#endif /** BOARD_H_ */
