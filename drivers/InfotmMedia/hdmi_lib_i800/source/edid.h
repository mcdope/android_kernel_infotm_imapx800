/*
 * edid.h
 *
 *  Created on: Jul 23, 2010
 *
 * Synopsys Inc.
 * SG DWC PT02
 */
/**
 * @file
 * E-EDID reader and parser
 * Initiates and handles I2C communications to read E-EDID then
 * parses read blocks and retains the information in corresponding
 * data structures
 */

#ifndef EDID_H_
#define EDID_H_

#include "dtd.h"
#include "shortVideoDesc.h"
#include "shortAudioDesc.h"
#include "halEdid.h"
#include "hdmivsdb.h"
#include "monitorRangeLimits.h"
#include "videoCapabilityDataBlock.h"
#include "colorimetryDataBlock.h"
#include "speakerAllocationDataBlock.h"
#include "types.h"

typedef enum
{
	EDID_ERROR = 0, EDID_IDLE, EDID_READING, EDID_DONE
} edid_status_t;

/**
 * Initialise the E-EDID reader
 * reset all internal variables
 * reset reader state
 * prepare I2C master
 * commence reading E-EDID memory from sink
 * @param baseAddr base address of controller
 * @param sfrClock external clock supplied to controller
 * @note: this version only works with 25MHz
 */
IM_INT32 edid_Initialize(IM_UINT16 baseAddr, IM_UINT16 sfrClock);
/**
 * Reset local variables and clear information from memory to avoid confusion with any newly read EDID structure
 * @param baseAddr base address of controller
 */
IM_INT32 edid_Standby(IM_UINT16 baseAddr);
/**
 * The method handles DONE and ERROR events.
 * A DONE event will trigger the retrieving the read byte, and sending a request to read the following byte. 
 * The EDID is read until the block is done and then the reading moves to the next block.
 *  When the block is successfully read, it is sent to be parsed.
 * @param baseAddr base address of controller
 * @param hpd state of hot plug (whether virtual or physical)
 * @param state of the EDID reader interrupts
 * @return edid_status_t status of the reading statemachine 
 */
IM_UINT8 edid_EventHandler(IM_UINT16 baseAddr, IM_INT32 hpd, IM_UINT8 state);
/**
 * Get the number of DTDs read and parsed from EDID structure
 * @param baseAddr base address of controller
 * @return the number of DTDs read and parsed from EDID structure
 */
IM_UINT32 edid_GetDtdCount(IM_UINT16 baseAddr);
/**
 * Get a Detailed Timing Descriptors data type read and parsed from EDID
 * structure.
 * @param baseAddr base address of controller
 * @param n index of the DTD in the sinks EDID
 * @param dtd pointer to a dtd_t structure to hold the information
 * @return TRUE if a DTD is found at specified index
 */
IM_INT32 edid_GetDtd(IM_UINT16 baseAddr, IM_UINT32 n, dtd_t * dtd);
/**
 * Get a HDMI VSDB data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param vsdb pointer to a hdmivsdb_t structure to hold the information
 * @return TRUE if an HDMI VSDB is found (ie. if sink is HDMI)
 */
IM_INT32 edid_GetHdmivsdb(IM_UINT16 baseAddr, hdmivsdb_t * vsdb);
/**
 * Get the monitor name parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param name pointer to the allocated memory of IM_INT8 type to hold the name
 * @param length of the memory (no of charachters the memory can hold)
 * @return number of copied characters
 */
IM_INT32 edid_GetMonitorName(IM_UINT16 baseAddr, IM_INT8 * name, IM_UINT32 length);
/**
 * Get the monitor range limits parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param limits pointer to structure of type monitorRangeLimits_t to hold 
 * information 
 * @return TRUE if a monitor range limit is found
 */
IM_INT32 edid_GetMonitorRangeLimits(IM_UINT16 baseAddr, monitorRangeLimits_t * limits);
/**
 * @param baseAddr base address of controller
 * @return the number of all Short Video Descriptors read and parsed from EDID structure
 */
IM_UINT32 edid_GetSvdCount(IM_UINT16 baseAddr);
/**
 * Get a Short Video Descriptor data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param n index of the SVD in the sinks EDID
 * @param svd pointer to a Short Video Descriptor structure to hold the information
 * @return TRUE if a Short Video Descriptor is found at specified index
 */
IM_INT32 edid_GetSvd(IM_UINT16 baseAddr, IM_UINT32 n, shortVideoDesc_t * svd);
/**
 * @param baseAddr base address of controller
 * @return the number of all Short Audio Descriptors read and parsed from EDID structure
 */
IM_UINT32 edid_GetSadCount(IM_UINT16 baseAddr);
/**
 * Get a Short Audio Descriptor data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param n index of the SAD in the sinks EDID
 * @param sad pointer to a Short Audio Descriptor structure to hold the information
 * @return TRUE if a Short Audio Descriptor is found at specified index
 */
IM_INT32 edid_GetSad(IM_UINT16 baseAddr, IM_UINT32 n, shortAudioDesc_t * sad);
/**
 * Get the video capability data block parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param capability pointer to structure of type videoCapabilityDataBlock_t to 
 * hold information 
 * @return TRUE if a video capability data block is found
 */
IM_INT32 edid_GetVideoCapabilityDataBlock(IM_UINT16 baseAddr,
		videoCapabilityDataBlock_t * capability);
/**
 * Get the speaker allocation data block parsed from the E-EDID strucutre at 
 * sink
 * @param baseAddr base address of controller
 * @param allocation pointer to structure of type speakerAllocationDataBlock_t
 *  to hold information 
 * @return TRUE if a speaker allocation data block is found
 */
IM_INT32 edid_GetSpeakerAllocationDataBlock(IM_UINT16 baseAddr,
		speakerAllocationDataBlock_t * allocation);
/**
 * Get the colorimetry data block parsed from the E-EDID strucutre at 
 * sink
 * @param baseAddr base address of controller
 * @param colorimetry pointer to structure of type colorimetryDataBlock_t
 *  to hold information 
 * @return TRUE if a colorimetry data block is found
 */
IM_INT32 edid_GetColorimetryDataBlock(IM_UINT16 baseAddr,
		colorimetryDataBlock_t * colorimetry);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports basic audio
 */
IM_INT32 edid_SupportsBasicAudio(IM_UINT16 baseAddr);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports underscan
 */
IM_INT32 edid_SupportsUnderscan(IM_UINT16 baseAddr);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports YCC:4:2:2
 */
IM_INT32 edid_SupportsYcc422(IM_UINT16 baseAddr);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports YCC:4:4:4
 */
IM_INT32 edid_SupportsYcc444(IM_UINT16 baseAddr);
/**
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
IM_INT32 edid_Reset(IM_UINT16 baseAddr);
/**
 * Sends a read request (of indicated address and block) to the EDID structure in the SINK CE.
 * @param baseAddr base address of controller
 * @param address the address of the byte to be read within the block
 * @param blockNo the number of the block of which the data is to be read from
 * @return TRUE if successful
 */
IM_INT32 edid_ReadRequest(IM_UINT16 baseAddr, IM_UINT8 address, IM_UINT8 blockNo);
/**
 * Parses an EDID block of 128 bytes.
 * Each known group (based on location or CEA tag) is parsed using the constructor of its own datatype (class), called in this function accordingly, this function then adds the new valid data structures into the EDID library.
 * @param baseAddr base address of controller
 * @param buffer a pointer to buffer (of 128 bytes)
 * @return TRUE if successful
 */
IM_INT32 edid_ParseBlock(IM_UINT16 baseAddr, IM_UINT8 * buffer);
/**
 * Parse Data Block data structures listed in the CEA Data Block Collection.
 * It identifies the block type and calls the appropriate class constructor to parse it.
 * It adds the new parsed block (object) to the respective vector arrays in the edid object.
 * @param baseAddr base address of controller
 * @param data a buffer array of bytes (pointer to the start of the block).
 * @return the length of the parsed Data Block in the Collection
 */
IM_UINT8 edid_ParseDataBlock(IM_UINT16 baseAddr, IM_UINT8 * data);

#endif /* EDID_H_ */
