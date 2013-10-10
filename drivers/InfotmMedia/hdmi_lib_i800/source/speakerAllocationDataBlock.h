/*
 * speakerAllocationDataBlock.h
 *
 *  Created on: Jul 22, 2010
  * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef SPEAKERALLOCATIONDATABLOCK_H_
#define SPEAKERALLOCATIONDATABLOCK_H_

#include "types.h"

/** 
 * @file
 * SpeakerAllocation Data Block.
 * Holds and parse the Speaker Allocation datablock information.
 * For detailed handling of this structure, refer to documentation of the functions
 */

typedef struct
{
	IM_UINT8 mByte1;

	IM_INT32 mValid;
} speakerAllocationDataBlock_t;

void speakerAllocationDataBlock_Reset(speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_Parse(speakerAllocationDataBlock_t * sadb,
		IM_UINT8 * data);

IM_INT32 speakerAllocationDataBlock_SupportsFlFr(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsLfe(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsFc(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsRlRr(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsRc(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsFlcFrc(
		speakerAllocationDataBlock_t * sadb);

IM_INT32 speakerAllocationDataBlock_SupportsRlcRrc(
		speakerAllocationDataBlock_t * sadb);
/**
 * @return the Channel Allocation code used in the Audio Infoframe to ease the translation process
 */
IM_UINT8 speakerAllocationDataBlock_GetChannelAllocationCode(speakerAllocationDataBlock_t * sadb);
/**
 * @return the whole byte of Speaker Allocation DataBlock where speaker allocation is indicated. User wishing to access and interpret it must know specifically how to parse it.
 */
IM_UINT8 speakerAllocationDataBlock_GetSpeakerAllocationByte(speakerAllocationDataBlock_t * sadb);

#endif /* SPEAKERALLOCATIONDATABLOCK_H_ */
