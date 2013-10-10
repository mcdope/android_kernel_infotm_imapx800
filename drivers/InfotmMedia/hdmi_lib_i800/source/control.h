/*
 * control.h
 *
 *  Created on: Jul 2, 2010
 *
 * Synopsys Inc.
 * SG DWC PT02
 */
/**
 * @file
 * Core control module:
 * Product information
 * Power management
 * Interrupt handling
 *
 */
#ifndef CONTROL_H_
#define CONTROL_H_

#include "types.h"
/**
 * Initializes PHY and core clocks
 * @param baseAddr base address of controller
 * @param dataEnablePolarity data enable polarity
 * @param pixelClock pixel clock [10KHz]
 * @return TRUE if successful
 */
IM_INT32 control_Initialize(IM_UINT16 baseAddr, IM_UINT8 dataEnablePolarity, IM_UINT16 pixelClock);
/**
 * Configures PHY and core clocks
 * @param baseAddr base address of controller
 * @param pClk pixel clock [10KHz]
 * @param pRep pixel repetition factor
 * @param cRes color resolution
 * @param cscOn 1 if colour space converter active
 * @param audioOn 1 if Audio active
 * @param cecOn 1 if Cec module active
 * @param hdcpOn 1 if Hdcp active
 * @return TRUE if successful
 */
IM_INT32 control_Configure(IM_UINT16 baseAddr, IM_UINT16 pClk, IM_UINT8 pRep, IM_UINT8 cRes, IM_INT32 cscOn,
		IM_INT32 audioOn, IM_INT32 cecOn, IM_INT32 hdcpOn);
/**
 * Go into standby mode: stop all clocks from all modules except for the CEC (refer to CEC for more detail)
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
IM_INT32 control_Standby(IM_UINT16 baseAddr);
//sam : 
/**
 * Go into standby mode: stop all clocks from all modules except for the CEC (refer to CEC for more detail)
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
IM_INT32 control_exitStandby(IM_UINT16 baseAddr);

/**
 * Read product design information
 * @param baseAddr base address of controller
 * @return the design number stored in the hardware
 */
IM_UINT8 control_Design(IM_UINT16 baseAddr);
/**
 * Read product revision information
 * @param baseAddr base address of controller
 * @return the revision number stored in the hardware
 */
IM_UINT8 control_Revision(IM_UINT16 baseAddr);
/**
 * Read product line information
 * @param baseAddr base address of controller
 * @return the product line stored in the hardware
 */
IM_UINT8 control_ProductLine(IM_UINT16 baseAddr);
/**
 * Read product type information
 * @param baseAddr base address of controller
 * @return the product type stored in the hardware
 */
IM_UINT8 control_ProductType(IM_UINT16 baseAddr);
/**
 * Check if hardware is compatible with the API
 * @param baseAddr base address of controller
 * @return TRUE if the HDMICTRL API supports the hardware (HDMI TX)
 */
IM_INT32 control_SupportsCore(IM_UINT16 baseAddr);
/**
 * Check if HDCP is instantiated in hardware
 * @param baseAddr base address of controller
 * @return TRUE if hardware supports HDCP encryption
 */
IM_INT32 control_SupportsHdcp(IM_UINT16 baseAddr);

/** 
 * Mute controller interrupts
 * @param baseAddr base address of controller
 * @param value mask of the register
 * @return TRUE when successful
 */
IM_INT32 control_InterruptMute(IM_UINT16 baseAddr, IM_UINT8 value);
/** 
 * Clear CEC interrupts
 * @param baseAddr base address of controller
 * @param value
 * @return TRUE if successful
 */
IM_INT32 control_InterruptCecClear(IM_UINT16 baseAddr, IM_UINT8 value);
/**
 * @param baseAddr base address of controller
 * @return CEC interrupts state 
 */
IM_UINT8 control_InterruptCecState(IM_UINT16 baseAddr);
/** 
 * Clear EDID reader (I2C master) interrupts
 * @param baseAddr base address of controller
 * @param value
 * @return TRUE if successful
 */
IM_INT32 control_InterruptEdidClear(IM_UINT16 baseAddr, IM_UINT8 value);

/** 
 * @param baseAddr base address of controller
 * @return EDID reader interrupts state (I2C master controller)
 */
IM_UINT8 control_InterruptEdidState(IM_UINT16 baseAddr);
/** 
 * Clear phy interrupts
 * @param baseAddr base address of controller
 * @param value
 * @return TRUE if successful
 */
IM_INT32 control_InterruptPhyClear(IM_UINT16 baseAddr, IM_UINT8 value);

/**
 * @param baseAddr base address of controller
 * @return PHY interrupts state 
 */
IM_UINT8 control_InterruptPhyState(IM_UINT16 baseAddr);
/** 
 * Clear all controller interrputs (except for hdcp)
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
IM_INT32 control_InterruptClearAll(IM_UINT16 baseAddr);


#endif /* CONTROL_H_ */
