/*
 * dtd.h
 *
 *  Created on: Jul 5, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */

#ifndef DTD_H_
#define DTD_H_

#include "types.h"

/**
 * @file
 * For detailed handling of this structure, refer to documentation of the functions
 */
typedef struct
{
	/** VIC code */
	IM_UINT8 mCode;

	IM_UINT16 mPixelRepetitionInput;

	IM_UINT16 mPixelClock;

	IM_UINT8 mInterlaced;

	IM_UINT16 mHActive;

	IM_UINT16 mHBlanking;

	IM_UINT16 mHBorder;

	IM_UINT16 mHImageSize;

	IM_UINT16 mHSyncOffset;

	IM_UINT16 mHSyncPulseWidth;

	IM_UINT8 mHSyncPolarity;

	IM_UINT16 mVActive;

	IM_UINT16 mVBlanking;

	IM_UINT16 mVBorder;

	IM_UINT16 mVImageSize;

	IM_UINT16 mVSyncOffset;

	IM_UINT16 mVSyncPulseWidth;

	IM_UINT8 mVSyncPolarity;

} dtd_t;
/**
 * Parses the Detailed Timing Descriptor.
 * Encapsulating the parsing process
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param data a pointer to the 18-byte structure to be parsed.
 * @return TRUE if success
 */
IM_INT32 dtd_Parse(dtd_t *dtd, IM_UINT8 data[18]);

/**
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param code the CEA 861-D video code.
 * @param refreshRate the specified vertical refresh rate.
 * @return TRUE if success
 */
IM_INT32 dtd_Fill(dtd_t *dtd, IM_UINT8 code, IM_UINT32 refreshRate);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the CEA861-D code if the DTD is listed in the spec
 * @return < 0 when the code has an undefined DTD
 */
IM_UINT8 dtd_GetCode(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the input pixel reptition
 */
IM_UINT16 dtd_GetPixelRepetitionInput(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the pixel clock rate of the DTD
 */
IM_UINT16 dtd_GetPixelClock(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return 1 if the DTD is of an interlaced viedo format
 */
IM_UINT8 dtd_GetInterlaced(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal active pixels (addressable video)
 */
IM_UINT16 dtd_GetHActive(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal blanking pixels
 */
IM_UINT16 dtd_GetHBlanking(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal border in pixels
 */
IM_UINT16 dtd_GetHBorder(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal image size in mm
 */
IM_UINT16 dtd_GetHImageSize(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync offset (front porch) from blanking start to start of sync in pixels
 */
IM_UINT16 dtd_GetHSyncOffset(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync polarity
 */
IM_UINT8 dtd_GetHSyncPolarity(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync pulse width in pixels
 */
IM_UINT16 dtd_GetHSyncPulseWidth(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical active pixels (addressable video)
 */
IM_UINT16 dtd_GetVActive(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical border in pixels
 */
IM_UINT16 dtd_GetVBlanking(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal image size
 */
IM_UINT16 dtd_GetVBorder(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical image size in mm
 */
IM_UINT16 dtd_GetVImageSize(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync offset (front porch) from blanking start to start
 *  of sync in lines
 */
IM_UINT16 dtd_GetVSyncOffset(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync polarity
 */
IM_UINT8 dtd_GetVSyncPolarity(const dtd_t *dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync pulse width in line
 */
IM_UINT16 dtd_GetVSyncPulseWidth(const dtd_t *dtd);
/**
 * @param dtd1 pointer to dtd_t structure to be compared
 * @param dtd2 pointer to dtd_t structure to be compared to 
 * @return TRUE if the two DTDs are identical 
 * @note: although DTDs may have different refresh rates, and hence 
 * pixel clocks, they can still be identical
 */
IM_INT32 dtd_IsEqual(const dtd_t *dtd1, const dtd_t *dtd2);
/**
 * Set the desired pixel repetition
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @param value of pixel repetitions
 * @return TRUE if successful
 * @note for CEA video modes, the value has to fall within the
 * defined range, otherwise the method will fail.
 */
IM_INT32 dtd_SetPixelRepetitionInput(dtd_t *dtd, IM_UINT16 value);

#endif /* DTD_H_ */
