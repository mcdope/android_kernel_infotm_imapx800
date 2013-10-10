/*
 * csi_driver
 * 12/05/2010
 * Synopsys Inc. PT02
 */

#ifndef CSI_DRIVER_H_
#define CSI_DRIVER_H_
/**
 * @file
 * MIPI CSI-2 Host driver
 * Low level driver interface to the driver core
 * It masks the access to the core's registers
 * @note This version of the driver only allows for ONE instance.
 */

#include <InfotmMedia.h>

/** define current version that this driver works with,
 * currently: 1.00* */
#define CURRENT_VERSION 0x3130322A

/** csi hardware register addresses */
typedef enum
{
	VERSION =		0x00,
	N_LANES =		0x04,
	PHY_SHUTDOWNZ = 0x08,
	DPHY_RSTZ =		0x0C,
	CSI2_RESETN =	0x10,
	PHY_STATE =		0x14,
	DATA_IDS_1 =	0x18,
	DATA_IDS_2 =	0x1C,
	ERR1 =			0x20,
	ERR2 =			0x24,
	MASK1 =			0x28,
	MASK2 =			0x2C,
	PHY_TST_CTR0 =	0x30,
	PHY_TST_CTR1 =	0x34
} csi_registers_t;

/** csi lanes power mode */
typedef enum
{
	CSI_LANE_OFF = 0,
	CSI_LANE_ON,
	CSI_LANE_ULTRA_LOW_POWER,
	CSI_LANE_STOP,
	CSI_LANE_HIGH_SPEED
} csi_lane_state_t;

typedef enum
{
	ERR_NOT_INIT = 0xFE,
	ERR_ALREADY_INIT = 0xFD,
	ERR_NOT_COMPATIBLE = 0xFC,
	ERR_UNDEFINED = 0xFB,
	ERR_OUT_OF_BOUND = 0xFA,
	SUCCESS = 0
} csi_error_t;

typedef enum
{
	NULL_PACKET = 0x10,
	BLANKING_DATA = 0x11,
	EMBEDDED_8BIT_NON_IMAGE_DATA = 0x12,
	YUV420_8BIT = 0x18,
	YUV420_10BIT = 0x19,
	LEGACY_YUV420_8BIT = 0x1A,
	YUV420_8BIT_CHROMA_SHIFTED = 0x1C,
	YUV420_10BIT_CHROMA_SHIFTED = 0x1D,
	YUV422_8BIT = 0x1E,
	YUV422_10BIT = 0x1F,
	RGB444 = 0x20,
	RGB555 = 0x21,
	RGB565 = 0x22,
	RGB666 = 0x23,
	RGB888 = 0x24,
	RAW6 = 0x28,
	RAW7 = 0x29,
	RAW8 = 0x2A,
	RAW10 = 0x2B,
	RAW12 = 0x2C,
	RAW14 = 0x2D,
	USER_DEFINED_8BIT_DATA_TYPE_1 = 0x30,
	USER_DEFINED_8BIT_DATA_TYPE_2 = 0x31,
	USER_DEFINED_8BIT_DATA_TYPE_3 = 0x32,
	USER_DEFINED_8BIT_DATA_TYPE_4 = 0x33,
	USER_DEFINED_8BIT_DATA_TYPE_5 = 0x34,
	USER_DEFINED_8BIT_DATA_TYPE_6 = 0x35,
	USER_DEFINED_8BIT_DATA_TYPE_7 = 0x36,
	USER_DEFINED_8BIT_DATA_TYPE_8 = 0x37
} csi_data_type_t;

/**
 * Initialize the CSI software
 * - check if compatible with core
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_init(void);
/**
 * close the CSI software module
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_close(void);
/**
 * get the number of enabled lanes
 * @return number of enabled lanes
 */
IM_UINT8 csi_get_on_lanes(void);
/**
 * set the number of lanes that are to switch On
 * @param lanes number to be enabled
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_set_on_lanes(IM_UINT8 lanes);
/**
 * shut down phy module
 * @param shutdown 1 for shutting it down, 0 for bringing it up
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_shut_down_phy(IM_UINT8 shutdown);
/**
 * reset phy module
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_reset_phy(void);
/**
 * reset CSI-2 Host controller module
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_reset_controller(void);
/**
 * Get lane state
 * @param lane number to be queried
 * @return csi_lane_state_t
 */
csi_lane_state_t csi_lane_module_state(IM_UINT8 lane);
/**
 * Get clock lane state
 * @return csi_lane_state_t
 */
csi_lane_state_t csi_clk_state(void);
/**
 * Payload Bypass test mode for double ECC errors
 * @param on (1)or off (0)
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_payload_bypass(IM_UINT8 on);
/**
 * Register DATA IDs (long packet data type) for which a virtual channel should
 *  report an error
 * @param virtual_channel_no
 * @param data_type
 * @param offset of event in registers (up to 8)
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_register_line_event(IM_UINT8 virtual_channel_no, csi_data_type_t data_type, IM_UINT8 offset);
/**
 * Register DATA IDs (long packet data type) for which a virtual channel should
 *  report an error
 * @param offset of event in registers (up to 8)
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_unregister_line_event(IM_UINT8 offset);
/**
 * Get the event registered at a certain offset
 * @param offset of event in registers (up to 8)
 * @return the event and vc it is associated to (5:0 - data type, 7:6 - VC)
 */
IM_UINT8 csi_get_registered_line_event(IM_UINT8 offset);
/**
 * Disable error from triggering an event
 * @param mask 32-bit of the error register
 * @param err_reg_no 1 or 2
 * @note (refer to DesignWare Cores MIPI CSI-2 Host Controller Databook)
 */
IM_UINT8 csi_event_disable(IM_UINT32 mask, IM_UINT8 err_reg_no);
/**
 * Enable error to trigger an event
 * @param mask 32-bit of the error register
 * @param err_reg_no 1 or 2
 * @return SUCCESS or errorcode
 * @note (refer to DesignWare Cores MIPI CSI-2 Host Controller Databook)
 */
IM_UINT8 csi_event_enable(IM_UINT32 mask, IM_UINT8 err_reg_no);
/**
 * Read the contents of either ERR1 or ERR2
 * @param err_reg_no 1 or 2
 * @return the contents of the register
 * @note (refer to DesignWare Cores MIPI CSI-2 Host Controller Databook)
 */
IM_UINT32 csi_event_get_source(IM_UINT8 err_reg_no);

/******************************************************************************
 * register access methods
******************************************************************************/
/**
 * read a register from CSI-2 host core
 * @param address register address/enumeration
 * @return 32-bit contents of the register
 */
IM_UINT32 csi_core_read(csi_registers_t address);
/**
 * write to a register in CSI-2 host core
 * @param address register address/enumeration
 * @param data 32-bit word
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_core_write(csi_registers_t address, IM_UINT32 data);
/**
 * write to a part of a register in CSI-2 host core
 * @param address register address/enumeration
 * @param data 32-bit word
 * @param shift from the beginning (0)
 * @param width of the data to be read
 * @return SUCCESS or errorcode
 */
IM_UINT8 csi_core_write_part(csi_registers_t address, IM_UINT32 data, IM_UINT8 shift, IM_UINT8 width);
/**
 * read a part of a register from CSI-2 host core
 * @param address register address/enumeration
 * @param shift from the beginning (0)
 * @param width of the data to be read
 * @return specified part of contents of the register
 */
IM_UINT32 csi_core_read_part(csi_registers_t address, IM_UINT8 shift, IM_UINT8 width);

#endif /* CSI_DRIVER_H_ */
