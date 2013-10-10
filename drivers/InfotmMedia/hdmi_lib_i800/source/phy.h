/*
 * phy.h
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */
/**
 * @file
 * Physical line interface configuration
 */
#ifndef PHY_H_
#define PHY_H_

#include "types.h"

/* #define PHY_THIRD_PARTY */
//sam : should be this type of phy ?
#define PHY_GEN2_TSMC_40LP_2_5V /* TQL */
/* #define PHY_GEN2_TSMC_40G_1_8V */ /* E102 */
//#define PHY_GEN2_TSMC_65LP_2_5V   /* E104 */
/* #define PHY_TNP */ /* HDMI 1.3 Tx PHY 2.25Gbps in TSMC 65nm GP 2.5V */
/* #define PHY_CNP */ /* HDMI 1.3 Tx PHY 2.25Gbps in Chartered 65nm LPe 2.5V */

/* phy pll lock timeout */
#define PHY_TIMEOUT 1000
/** Initialise phy and put into a known state
 * @param baseAddr of controller
 * @param dataEnablePolarity data enable polarity
 */
IM_INT32 phy_Initialize(IM_UINT16 baseAddr, IM_UINT8 dataEnablePolarity);
/** Bring up PHY and start sending media for a specified pixel clock, pixel
 * repetition and colour resolution (to calculate TMDS)
 * @param baseAddr of controller
 * @param pClk pixel clock
 * @param cRes colour depth or colour resolution (bits/component/pixel)
 * @param pRep pixel repetition
 */
IM_INT32 phy_Configure (IM_UINT16 baseAddr, IM_UINT16 pClk, IM_UINT8 cRes, IM_UINT8 pRep);
/** Set PHY to standby mode - turn off all interrupts
 * @param baseAddr of controller
 */
IM_INT32 phy_Standby(IM_UINT16 baseAddr);
/** Set PHY to exit standby mode - turn on all interrupts
 * @param baseAddr of controller
 */
IM_INT32 phy_exitStandby(IM_UINT16 baseAddr);
/** Enable HPD sensing ciruitry
 * @param baseAddr of controller
 */
IM_INT32 phy_EnableHpdSense(IM_UINT16 baseAddr);
/** Disable HPD sensing ciruitry
 * @param baseAddr of controller
 */
IM_INT32 phy_DisableHpdSense(IM_UINT16 baseAddr);
/**
 * Detects the signal on the HPD line and 
 * upon change, it inverts polarity of the interrupt
 * bit so that interrupt raises only on change
 * @param baseAddr of controller
 * @return TRUE the HPD line is asserted
 */
IM_INT32 phy_HotPlugDetected(IM_UINT16 baseAddr);
/**
 * @param baseAddr of controller
 * @param value of mask of interrupt register
 */
IM_INT32 phy_InterruptEnable(IM_UINT16 baseAddr, IM_UINT8 value);
/**
 * @param baseAddr of controller
 * @param value
 */
IM_INT32 phy_TestControl(IM_UINT16 baseAddr, IM_UINT8 value);
/**
 * @param baseAddr of controller
 * @param value
 */
IM_INT32 phy_TestData(IM_UINT16 baseAddr, IM_UINT8 value);
/**
 * @param baseAddr of controller
 * @param data 
 * @param addr of register offset in PHY register bank (through I2C)
 */
IM_INT32 phy_I2cWrite(IM_UINT16 baseAddr, IM_UINT16 data, IM_UINT8 addr);

//sam : 
IM_UINT16 phy_I2cRead(IM_UINT16 baseAddr, IM_UINT8 addr);


#endif /* PHY_H_ */
