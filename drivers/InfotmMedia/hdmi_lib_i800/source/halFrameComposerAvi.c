/*
 * @file:halFrameComposerAvi.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */
#include "halFrameComposerAvi.h"
#include "access.h"
#include "hdmi_log.h"

/* register offsets */
static const IM_UINT8 FC_AVICONF0 = 0x19;
static const IM_UINT8 FC_AVICONF1 = 0x1A;
static const IM_UINT8 FC_AVICONF2 = 0x1B;
static const IM_UINT8 FC_AVIVID = 0x1C;
static const IM_UINT8 FC_AVIETB0 = 0x1D;
static const IM_UINT8 FC_AVIETB1 = 0x1E;
static const IM_UINT8 FC_AVISBB0 = 0x1F;
static const IM_UINT8 FC_AVISBB1 = 0x20;
static const IM_UINT8 FC_AVIELB0 = 0x21;
static const IM_UINT8 FC_AVIELB1 = 0x22;
static const IM_UINT8 FC_AVISRB0 = 0x23;
static const IM_UINT8 FC_AVISRB1 = 0x24;
static const IM_UINT8 FC_PRCONF = 0xE0;
/* bit shifts */
static const IM_UINT8 RGBYCC = 0; // sam : 5
static const IM_UINT8 SCAN_INFO = 4; // sam : 0
static const IM_UINT8 COLORIMETRY = 6;
static const IM_UINT8 PIC_ASPECT_RATIO = 4;
static const IM_UINT8 ACTIVE_FORMAT_ASPECT_RATIO = 0;
static const IM_UINT8 ACTIVE_FORMAT_AR_VALID = 6; // sam : 4
static const IM_UINT8 IT_CONTENT = 7;
static const IM_UINT8 EXT_COLORIMETRY = 4;
static const IM_UINT8 QUANTIZATION_RANGE = 2;
static const IM_UINT8 NON_UNIFORM_PIC_SCALING = 0;
static const IM_UINT8 H_BAR_INFO = 3;
static const IM_UINT8 V_BAR_INFO = 2;
static const IM_UINT8 PIXEL_REP_OUT = 0;

void halFrameComposerAvi_RgbYcc(IM_UINT16 baseAddr, IM_UINT8 type)
{
	LOG_TRACE1(type);
	access_CoreWrite(type, baseAddr + FC_AVICONF0, RGBYCC, 2);
}

void halFrameComposerAvi_ScanInfo(IM_UINT16 baseAddr, IM_UINT8 left)
{
	LOG_TRACE1(left);
	access_CoreWrite(left, baseAddr + FC_AVICONF0, SCAN_INFO, 2);
}

void halFrameComposerAvi_Colorimetry(IM_UINT16 baseAddr, IM_UINT32 cscITU)
{
	LOG_TRACE1(cscITU);
	access_CoreWrite(cscITU, baseAddr + FC_AVICONF1, COLORIMETRY, 2);
}

void halFrameComposerAvi_PicAspectRatio(IM_UINT16 baseAddr, IM_UINT8 ar)
{
	LOG_TRACE1(ar);
	access_CoreWrite(ar, baseAddr + FC_AVICONF1, PIC_ASPECT_RATIO, 2);
}

void halFrameComposerAvi_ActiveAspectRatioValid(IM_UINT16 baseAddr, IM_UINT8 valid)
{
	LOG_TRACE1(valid);
	access_CoreWrite(valid, baseAddr + FC_AVICONF0, ACTIVE_FORMAT_AR_VALID, 1); /* data valid flag */
}

void halFrameComposerAvi_ActiveFormatAspectRatio(IM_UINT16 baseAddr, IM_UINT8 left)
{
	LOG_TRACE1(left);
	access_CoreWrite(left, baseAddr + FC_AVICONF1, ACTIVE_FORMAT_ASPECT_RATIO,
			4);
}

void halFrameComposerAvi_IsItContent(IM_UINT16 baseAddr, IM_UINT8 it)
{
	LOG_TRACE1(it);
	access_CoreWrite((it ? 1 : 0), baseAddr + FC_AVICONF2, IT_CONTENT, 1);
}

void halFrameComposerAvi_ExtendedColorimetry(IM_UINT16 baseAddr, IM_UINT8 extColor)
{
	LOG_TRACE1(extColor);
	access_CoreWrite(extColor, baseAddr + FC_AVICONF2, EXT_COLORIMETRY, 3);
	access_CoreWrite(0x3, baseAddr + FC_AVICONF1, COLORIMETRY, 2); /*data valid flag */
}

void halFrameComposerAvi_QuantizationRange(IM_UINT16 baseAddr, IM_UINT8 range)
{
	LOG_TRACE1(range);
	access_CoreWrite(range, baseAddr + FC_AVICONF2, QUANTIZATION_RANGE, 2);
}

void halFrameComposerAvi_NonUniformPicScaling(IM_UINT16 baseAddr, IM_UINT8 scale)
{
	LOG_TRACE1(scale);
	access_CoreWrite(scale, baseAddr + FC_AVICONF2, NON_UNIFORM_PIC_SCALING, 2);
}

void halFrameComposerAvi_VideoCode(IM_UINT16 baseAddr, IM_UINT8 code)
{
	LOG_TRACE1(code);
	access_CoreWriteByte(code, baseAddr + FC_AVIVID);
}

void halFrameComposerAvi_HorizontalBarsValid(IM_UINT16 baseAddr, IM_UINT8 validity)
{
	access_CoreWrite((validity ? 1 : 0), baseAddr + FC_AVICONF0, H_BAR_INFO, 1); /*data valid flag */
}

void halFrameComposerAvi_HorizontalBars(IM_UINT16 baseAddr, IM_UINT16 endTop,
		IM_UINT16 startBottom)
{
	LOG_TRACE2(endTop, startBottom);
	access_CoreWriteByte((u8) (endTop), baseAddr + FC_AVIETB0);
	access_CoreWriteByte((u8) (endTop >> 8), baseAddr + FC_AVIETB1);
	access_CoreWriteByte((u8) (startBottom), baseAddr + FC_AVISBB0);
	access_CoreWriteByte((u8) (startBottom >> 8), baseAddr + FC_AVISBB1);
}

void halFrameComposerAvi_VerticalBarsValid(IM_UINT16 baseAddr, IM_UINT8 validity)
{
	access_CoreWrite((validity ? 1 : 0), baseAddr + FC_AVICONF0, V_BAR_INFO, 1); /*data valid flag */
}

void halFrameComposerAvi_VerticalBars(IM_UINT16 baseAddr, IM_UINT16 endLeft, IM_UINT16 startRight)
{
	LOG_TRACE2(endLeft, startRight);
	access_CoreWriteByte((u8) (endLeft), baseAddr + FC_AVIELB0);
	access_CoreWriteByte((u8) (endLeft >> 8), baseAddr + FC_AVIELB1);
	access_CoreWriteByte((u8) (startRight), baseAddr + FC_AVISRB0);
	access_CoreWriteByte((u8) (startRight >> 8), baseAddr + FC_AVISRB1);
}

void halFrameComposerAvi_OutPixelRepetition(IM_UINT16 baseAddr, IM_UINT8 pr)
{
	LOG_TRACE1(pr);
	access_CoreWrite(pr, baseAddr + FC_PRCONF, PIXEL_REP_OUT, 4);
}

