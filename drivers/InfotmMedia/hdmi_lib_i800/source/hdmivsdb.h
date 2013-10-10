/*
 * hdmivsdb.h
 *
 *  Created on: Jul 21, 2010
 *      Author: klabadi & dlopo
 */

#ifndef HDMIVSDB_H_
#define HDMIVSDB_H_

#include "types.h"

/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct
{
	IM_UINT16 mPhysicalAddress;

	IM_INT32 mSupportsAi;

	IM_INT32 mDeepColor30;

	IM_INT32 mDeepColor36;

	IM_INT32 mDeepColor48;

	IM_INT32 mDeepColorY444;

	IM_INT32 mDviDual;

	IM_UINT8 mMaxTmdsClk;

	IM_UINT16 mVideoLatency;

	IM_UINT16 mAudioLatency;

	IM_UINT16 mInterlacedVideoLatency;

	IM_UINT16 mInterlacedAudioLatency;

	IM_UINT32 mId;

	IM_UINT8 mLength;

	IM_INT32 mValid;

} hdmivsdb_t;

void hdmivsdb_Reset(hdmivsdb_t *vsdb);

/**
 * Parse an array of data to fill the hdmivsdb_t data strucutre
 * @param *vsdb pointer to the structure to be filled
 * @param *data pointer to the 8-bit data type array to be parsed
 * @return Success, or error code:
 * @return 1 - array pointer invalid
 * @return 2 - Invalid datablock tag
 * @return 3 - Invalid minimum length
 * @return 4 - HDMI IEEE registration identifier not valid
 * @return 5 - Invalid length - latencies are not valid
 * @return 6 - Invalid length - Interlaced latencies are not valid
 */
IM_INT32 hdmivsdb_Parse(hdmivsdb_t *vsdb, IM_UINT8 * data);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if sink supports 10bit/pixel deep color mode
 */
IM_INT32 hdmivsdb_GetDeepColor30(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if sink supports 12bit/pixel deep color mode
 */
IM_INT32 hdmivsdb_GetDeepColor36(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if sink supports 16bit/pixel deep color mode
 */
IM_INT32 hdmivsdb_GetDeepColor48(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if sink supports YCC 4:4:4 in deep color modes
 */
IM_INT32 hdmivsdb_GetDeepColorY444(hdmivsdb_t *vsdb);

/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if sink supports at least one function that uses
 * information carried by the ACP, ISRC1, or ISRC2 packets
 */
IM_INT32 hdmivsdb_GetSupportsAi(hdmivsdb_t *vsdb);

/**
 * @param *vsdb pointer to the structure holding the information
 * @return TRUE if Sink supports DVI dual-link operation
 */
IM_INT32 hdmivsdb_GetDviDual(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return maximum TMDS clock rate supported [divided by 5MHz]
 */
IM_UINT8 hdmivsdb_GetMaxTmdsClk(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the amount of video latency when receiving progressive video formats
 */
IM_UINT16 hdmivsdb_GetPVideoLatency(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the amount of audio latency when receiving progressive video formats
 */
IM_UINT16 hdmivsdb_GetPAudioLatency(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the amount of audio latency when receiving interlaced video formats
 */
IM_UINT16 hdmivsdb_GetIAudioLatency(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the amount of video latency when receiving interlaced video formats
 */
IM_UINT16 hdmivsdb_GetIVideoLatency(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return Physical Address extracted from the VSDB
 */
IM_UINT16 hdmivsdb_GetPhysicalAddress(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the 24-bit IEEE Registration Identifier of 0x000C03, a value belonging to HDMI Licensing
 */
IM_UINT32 hdmivsdb_GetId(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure holding the information
 * @return the length of the HDMI VSDB (hdmivsdb_t *vsdb, without counting the first byte in, where the TAG is)
 */
IM_UINT8 hdmivsdb_GetLength(hdmivsdb_t *vsdb);
/**
 * @param *vsdb pointer to the structure to be edited
 * @param id the 24-bit IEEE Registration Identifier of  the VSDB.
 * where 0x000C03 belonging to HDMI Licensing.
 */
void hdmivsdb_SetId(hdmivsdb_t *vsdb, IM_UINT32 id);
/**
 * @param *vsdb pointer to the structure to be edited
 * @param length of the HDMI VSDB datablock
 */
void hdmivsdb_SetLength(hdmivsdb_t *vsdb, IM_UINT8 length);

#endif /* HDMIVSDB_H_ */
