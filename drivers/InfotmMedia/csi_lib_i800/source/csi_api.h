/*
 * csi_driver
 * 12/05/2010
 * Synopsys Inc. PT02
 */

#ifndef CSI_API_H_
#define CSI_API_H_
/**
 * @file
 * MIPI CSI-2 Host driver API
 * Top level driver interface to the driver core
 * It masks core's registers and structure
 * and presents an easy to use software API
 * @note This version of the driver only allows for ONE instance.
 */
 
#include <InfotmMedia.h>
#include "csi_driver.h"

typedef void (*handler_t)(void *);

/* enumerators index the handler array rather than masks */
/**
 * Events that could be enabled to call a specified callback
 * when they happen. (they enable interrupts)
 * @note MAX_EVENT is a dummy enumeration
 */
typedef enum
{
	ERR_PHY_TX_START = 0,
	ERR_FRAME_BOUNDARY_MATCH = 4,
	ERR_FRAME_SEQUENCE = 8,
	ERR_CRC_DURING_FRAME = 12,
	ERR_LINE_CRC = 16,
	ERR_DOUBLE_ECC = 20,
	ERR_PHY_ESCAPE_ENTRY = 21,
	ERR_ECC_SINGLE = 25,
	ERR_UNSUPPORTED_DATA_TYPE = 29,
	MAX_EVENT = 49
} csi_event_t;
/**
 * Line events that could be enabled to call a specified callback
 * when they happen. (they enable interrupts)
 */
typedef enum
{
	ERR_LINE_BOUNDARY_MATCH = 33,
	ERR_LINE_SEQUENCE = 41
} csi_line_event_t;
/**
 * Error types that could be returned. These build up on top of the
 * low level driver errors
 */
typedef enum
{
	ERROR_VC_LANE_OUT_OF_BOUND = 	0xCA,
	ERROR_EVENT_TYPE_INVALID = 0xC9,
	ERROR_DATA_TYPE_INVALID = 0xC8,
	ERROR_SLOTS_FULL = 0xC7,
	ERROR_ALREADY_REGISTERED = 0xC6,
	ERROR_NOT_REGISTERED = 0xC5
} csi_api_error_t;

/**
 * Initialize the CSI software
 * initialize core and phy
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_start(void);
/**
 * close the CSI software module
 * unregister all events
 * shut down phy
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_close(void);
/**
 * set the number of lanes that are to switch On
 * @param no_of_lanes to be enabled
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_set_on_lanes(IM_UINT8 no_of_lanes);
/**
 * get the number of enabled lanes
 * @return number of enabled lanes
 */
IM_UINT8 csi_api_get_on_lanes(void);
/**
 * Get clock lane state
 * @return csi_lane_state_t
 */
csi_lane_state_t csi_api_get_clk_state(void);
/**
 * Get lane state
 * @param lane number to be queried
 * @return csi_lane_state_t
 */
csi_lane_state_t csi_api_get_lane_state(IM_UINT8 lane);
/**
 * Register line events to call the specified handler when it occures
 * Boundary match or sequence of a certain long packet type
 * @param vc virtual channel to which long packets are destined
 * @param data_type long packets data types, one enum of csi_data_type_t
 * @param line_event the line event (boundary match or line sequence)
 * @param handler pointer to the callback function to be called when the
 * specified event occurs
 * @return SUCCESS or error code
 */
IM_UINT8 csi_api_register_line_event(IM_UINT8 vc, csi_data_type_t data_type, csi_line_event_t line_event, handler_t handler);
/**
 * clear an already registered line event of a certain virtual channel
 * and long packet date type
 * @param vc virtual channel to which long packets are destined
 * @param data_type long packets data types, one enum of csi_data_type_t
 * @param line_event the line event (boundary match or line sequence)
 * @return SUCCESS or error code
 */
IM_UINT8 csi_api_unregister_line_event(IM_UINT8 vc, csi_data_type_t data_type, csi_line_event_t line_event);
/**
 * register a callback for an event
 * @param event of type csi_event_t to be detected
 * @param vc_lane virtual channel or lane number, depending on event.
 * (PHY events will have vc_lane as LANE, and virtual channel otherwise)
 * @param handler pointer to the callback function to be called when the
 * specified line event occurs
 * @return SUCCESS or error code
 * @note Double ECC error has no virtual channel
 * (vc_lane value is not validated)
 */
IM_UINT8 csi_api_register_event(csi_event_t event, IM_UINT8 vc_lane, handler_t handler);
/**
 * clear a registered event
 * @param event of type csi_event_t to be stop being detected
 * @param vc_lane virtual channel or lane number, depending on event.
 * (PHY events will have vc_lane as LANE, and virtual channel otherwise)
 * @return SUCCESS or error code
 * @note Double ECC error has no virtual channel
 * (vc_lane value is not validated)
 */
IM_UINT8 csi_api_unregister_event(csi_event_t event, IM_UINT8 vc_lane);
/**
 * clear and unregister all events
 * @return SUCCESS or error code
 */
IM_UINT8 csi_api_unregister_all_events();
/**
 * function to be called upon receiving an interrupt
 * it invokes registered callbacks depending on registered event
 * @note must be invoked by user - to accomodate all possible scenarios
 * that user may implement (message queues, polling, or even
 * as a direct IRQ callback)
 * @param param pointer to void in case a parameter is to be passed on to
 * a callback
 */
void csi_api_event_handler(void *param);
/**
 * shut down phy module
 * @param shutdown 1 for shutting it down, 0 for bringing it up
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_shut_down_phy(IM_UINT8 shutdown);
/**
 * reset phy module
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_reset_phy();
/**
 * reset CSI-2 Host controller module
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_reset_controller();
/**
 * write to a register in CSI-2 host core
 * @param address register address/enumeration
 * @param data 32-bit word
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_api_core_write(csi_registers_t address, IM_UINT32 data);
/**
 * read a register from CSI-2 host core
 * @param address register address/enumeration
 * @return 32-bit contents of the register
 */
IM_UINT32 csi_api_core_read(csi_registers_t address);

#endif /* CSI_API_H_ */
