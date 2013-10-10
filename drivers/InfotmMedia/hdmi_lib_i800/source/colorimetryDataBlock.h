/*
 * colorimetryDataBlock.h
 *
 *  Created on: Jul 22, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef COLORIMETRYDATABLOCK_H_
#define COLORIMETRYDATABLOCK_H_

#include "types.h"

/**
 * @file
 * Colorimetry Data Block class.
 * Holds and parses the Colorimetry datablock information.
 */

typedef struct
{
	IM_UINT8 mByte3;

	IM_UINT8 mByte4;

	IM_INT32 mValid;

} colorimetryDataBlock_t;

void colorimetryDataBlock_Reset(colorimetryDataBlock_t * cdb);

IM_INT32 colorimetryDataBlock_Parse(colorimetryDataBlock_t * cdb, IM_UINT8 * data);

IM_INT32 colorimetryDataBlock_SupportsXvYcc709(colorimetryDataBlock_t * cdb);

IM_INT32 colorimetryDataBlock_SupportsXvYcc601(colorimetryDataBlock_t * cdb);

IM_INT32 colorimetryDataBlock_SupportsMetadata0(colorimetryDataBlock_t * cdb);

IM_INT32 colorimetryDataBlock_SupportsMetadata1(colorimetryDataBlock_t * cdb);

IM_INT32 colorimetryDataBlock_SupportsMetadata2(colorimetryDataBlock_t * cdb);

#endif /* COLORIMETRYDATABLOCK_H_ */
