/*
 * halColorSpaceConverter.c
 *
 *  Created on: Jun 30, 2010
 *      Author: klabadi & dlopo
 */

#include "halColorSpaceConverter.h"
#include "access.h"
#include "hdmi_log.h"
/* Color Space Converter register offsets */
static const IM_UINT8 CSC_CFG = 0x00;
static const IM_UINT8 CSC_SCALE = 0x01;
static const IM_UINT8 CSC_COEF_A1_MSB = 0x02;
static const IM_UINT8 CSC_COEF_A1_LSB = 0x03;
static const IM_UINT8 CSC_COEF_A2_MSB = 0x04;
static const IM_UINT8 CSC_COEF_A2_LSB = 0x05;
static const IM_UINT8 CSC_COEF_A3_MSB = 0x06;
static const IM_UINT8 CSC_COEF_A3_LSB = 0x07;
static const IM_UINT8 CSC_COEF_A4_MSB = 0x08;
static const IM_UINT8 CSC_COEF_A4_LSB = 0x09;
static const IM_UINT8 CSC_COEF_B1_MSB = 0x0A;
static const IM_UINT8 CSC_COEF_B1_LSB = 0x0B;
static const IM_UINT8 CSC_COEF_B2_MSB = 0x0C;
static const IM_UINT8 CSC_COEF_B2_LSB = 0x0D;
static const IM_UINT8 CSC_COEF_B3_MSB = 0x0E;
static const IM_UINT8 CSC_COEF_B3_LSB = 0x0F;
static const IM_UINT8 CSC_COEF_B4_MSB = 0x10;
static const IM_UINT8 CSC_COEF_B4_LSB = 0x11;
static const IM_UINT8 CSC_COEF_C1_MSB = 0x12;
static const IM_UINT8 CSC_COEF_C1_LSB = 0x13;
static const IM_UINT8 CSC_COEF_C2_MSB = 0x14;
static const IM_UINT8 CSC_COEF_C2_LSB = 0x15;
static const IM_UINT8 CSC_COEF_C3_MSB = 0x16;
static const IM_UINT8 CSC_COEF_C3_LSB = 0x17;
static const IM_UINT8 CSC_COEF_C4_MSB = 0x18;
static const IM_UINT8 CSC_COEF_C4_LSB = 0x19;

void halColorSpaceConverter_Interpolation(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	access_CoreWrite(value, baseAddr + CSC_CFG, 4, 2);
}

void halColorSpaceConverter_Decimation(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	access_CoreWrite(value, baseAddr + CSC_CFG, 0, 2);
}

void halColorSpaceConverter_ColorDepth(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	/* 4-bit width */
	access_CoreWrite(value, baseAddr + CSC_SCALE, 4, 4);
}

void halColorSpaceConverter_ScaleFactor(IM_UINT16 baseAddr, IM_UINT8 value)
{
	LOG_TRACE1(value);
	/* 2-bit width */
	access_CoreWrite(value, baseAddr + CSC_SCALE, 0, 2);
}

void halColorSpaceConverter_CoefficientA1(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_A1_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_A1_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientA2(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_A2_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_A2_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientA3(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_A3_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_A3_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientA4(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_A4_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_A4_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientB1(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_B1_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_B1_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientB2(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_B2_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_B2_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientB3(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_B3_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_B3_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientB4(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_B4_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_B4_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientC1(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_C1_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_C1_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientC2(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_C2_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_C2_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientC3(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	/* 15-bit width */
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_C3_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_C3_MSB, 0, 7);
}

void halColorSpaceConverter_CoefficientC4(IM_UINT16 baseAddr, IM_UINT16 value)
{
	LOG_TRACE1(value);
	access_CoreWriteByte((u8) (value), baseAddr + CSC_COEF_C4_LSB);
	access_CoreWrite((u8) (value >> 8), baseAddr + CSC_COEF_C4_MSB, 0, 7);
}
