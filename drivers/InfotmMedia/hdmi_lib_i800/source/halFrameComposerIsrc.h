/*
 * @file:halFrameComposerIsrc.h
 *
 *  Created on: Jul 1, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HALFRAMECOMPOSERISRC_H_
#define HALFRAMECOMPOSERISRC_H_

#include "types.h"
/*
 * Configure the ISRC packet status
 * @param code
 * 001 - Starting Position
 * 010 - Intermediate Position
 * 100 - Ending Position
 * @param baseAddr block base address
 */
void halFrameComposerIsrc_Status(IM_UINT16 baseAddr, IM_UINT8 code);
/*
 * Configure the validity bit in the ISRC packets
 * @param validity: 1 if valid
 * @param baseAddr block base address
 */
void halFrameComposerIsrc_Valid(IM_UINT16 baseAddr, IM_UINT8 validity);
/*
 * Configure the cont bit in the ISRC1 packets
 * When a subsequent ISRC2 Packet is transmitted, the ISRC_Cont field shall be set and shall be clear otherwise.
 * @param isContinued 1 when set
 * @param baseAddr block base address
 */
void halFrameComposerIsrc_Cont(IM_UINT16 baseAddr, IM_UINT8 isContinued);
/*
 * Configure the ISRC 1 Codes
 * @param codes
 * @param length
 * @param baseAddr block base address
 */
void halFrameComposerIsrc_Isrc1Codes(IM_UINT16 baseAddr, IM_UINT8 * codes, IM_UINT8 length);
/*
 * Configure the ISRC 2 Codes
 * @param codes
 * @param length
 * @param baseAddr block base address
 */
void halFrameComposerIsrc_Isrc2Codes(IM_UINT16 baseAddr, IM_UINT8 * codes, IM_UINT8 length);

#endif /* HALFRAMECOMPOSERISRC_H_ */
