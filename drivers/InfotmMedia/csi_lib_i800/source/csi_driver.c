#include <InfotmMedia.h>
#include "csi_driver.h"
#include "csi_pwl.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CSI_DRV_I:"
#define WARNHEAD	"CSI_DRV_W:"
#define ERRHEAD		"CSI_DRV_E:"
#define TIPHEAD		"CSI_DRV_T:"

static IM_BOOL csi_core_initialized = IM_FALSE;

IM_UINT8 csi_init()
{
	csi_error_t e = SUCCESS;
    IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));
	do
	{
		if (csi_core_initialized == IM_FALSE)
		{
            ret = csipwl_init();
            if(ret != IM_RET_OK)
            {
                IM_ERRMSG((IM_STR("csipwl_init() failed!")));
                return ERR_UNDEFINED;
            }

			if (csi_core_read(VERSION) == (IM_UINT32)(CURRENT_VERSION))
			{
                csi_core_initialized = IM_TRUE;
				break;
			}
			else
			{
                IM_ERRMSG((IM_STR("Driver not compatible with core!")));
                csipwl_deinit();
				e = ERR_NOT_COMPATIBLE;
				break;
			}
		}
		else
		{
            IM_ERRMSG((IM_STR("Driver already initialised!")));
			e = ERR_ALREADY_INIT;
			break;
		}

	} while(0);
	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return e;
}

IM_UINT8 csi_close()
{
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));
	csi_shut_down_phy(1);

    csipwl_deinit();

	csi_core_initialized = IM_FALSE;
	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return SUCCESS;
}

IM_UINT8 csi_get_on_lanes()
{
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));
	return (csi_core_read_part(N_LANES, 0, 2) + 1);
}

IM_UINT8 csi_set_on_lanes(IM_UINT8 lanes)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_core_write_part(N_LANES, (lanes - 1), 0, 2);
}

IM_UINT8 csi_shut_down_phy(IM_UINT8 shutdown)
{
    IM_INFOMSG((IM_STR("shutdown=%d!"), shutdown));
	/*  active low - bit 0 */
	return csi_core_write_part(PHY_SHUTDOWNZ, shutdown? 0: 1, 0, 1);
}

IM_UINT8 csi_reset_phy()
{
	/*  active low - bit 0 */
	int retVal = 0xffff;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	retVal = csi_core_write_part(DPHY_RSTZ, 0, 0, 1);
	switch (retVal)
	{
	case SUCCESS:
		return csi_core_write_part(DPHY_RSTZ, 1, 0, 1);
		break;
	case ERR_NOT_INIT:
        IM_ERRMSG((IM_STR("Driver not initialized!")));
		return retVal;
		break;
	default:
        IM_ERRMSG((IM_STR("Undefined error!")));
		return ERR_UNDEFINED;
		break;
	}
}

IM_UINT8 csi_reset_controller()
{
	/*  active low - bit 0 */
	int retVal = 0xffff;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	retVal = csi_core_write_part(CSI2_RESETN, 0, 0, 1);
	switch (retVal)
	{
	case SUCCESS:
		return csi_core_write_part(CSI2_RESETN, 1, 0, 1);
		break;
	case ERR_NOT_INIT:
        IM_ERRMSG((IM_STR("Driver not initialized!")));
		return retVal;
		break;
	default:
        IM_ERRMSG((IM_STR("Undefined error!")));
		return ERR_UNDEFINED;
		break;
	}
}

csi_lane_state_t csi_lane_module_state(IM_UINT8 lane)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (lane < csi_core_read_part(N_LANES, 0, 2) + 1)
	{
		if (csi_core_read_part(PHY_STATE, lane, 1))
		{
			return CSI_LANE_ULTRA_LOW_POWER;
		}
		else if (csi_core_read_part(PHY_STATE, (lane + 4), 1))
		{
			return CSI_LANE_STOP;
		}
		return CSI_LANE_ON;
	}
	else
	{
        IM_WARNMSG((IM_STR("Lane switched off!")));
		return CSI_LANE_OFF;
	}
}

csi_lane_state_t csi_clk_state()
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (!csi_core_read_part(PHY_STATE, 9, 1))
	{
		return CSI_LANE_ULTRA_LOW_POWER;
	}
	else if (csi_core_read_part(PHY_STATE, 10, 1))
	{
		return CSI_LANE_STOP;
	}
	else if (csi_core_read_part(PHY_STATE, 8, 1))
	{
		return CSI_LANE_HIGH_SPEED;
	}
	return CSI_LANE_ON;
}

IM_UINT8 csi_payload_bypass(IM_UINT8 on)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_core_write_part(PHY_STATE, on? 1: 0, 11, 1);
}

IM_UINT8 csi_register_line_event(IM_UINT8 virtual_channel_no, csi_data_type_t data_type, IM_UINT8 offset)
{
	IM_UINT8 id = 0;
	csi_registers_t reg_offset = 0;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if ((virtual_channel_no > 4) || (offset > 8))
	{
		return ERR_OUT_OF_BOUND;
	}
	id = (virtual_channel_no << 6) | data_type;

	reg_offset = ((offset / 4) == 1) ? DATA_IDS_2: DATA_IDS_1;

	return csi_core_write_part(reg_offset, id, (offset * 8), 8);
}
IM_UINT8 csi_unregister_line_event(IM_UINT8 offset)
{
	csi_registers_t reg_offset = 0;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (offset > 8)
	{
		return ERR_OUT_OF_BOUND;
	}
	reg_offset = ((offset / 4) == 1) ? DATA_IDS_2: DATA_IDS_1;
	return csi_core_write_part(reg_offset, 0x00, (offset * 8), 8);
}

IM_UINT8 csi_get_registered_line_event(IM_UINT8 offset)
{
	csi_registers_t reg_offset = 0;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (offset > 8)
	{
		return ERR_OUT_OF_BOUND;
	}
	reg_offset = ((offset / 4) == 1) ? DATA_IDS_2: DATA_IDS_1;
	return (IM_UINT8)csi_core_read_part(reg_offset, (offset * 8), 8);
}

IM_UINT8 csi_event_disable(IM_UINT32 mask, IM_UINT8 err_reg_no)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	switch (err_reg_no)
	{
		case 1:
			return csi_core_write(MASK1, mask | csi_core_read(MASK1));
		case 2:
			return csi_core_write(MASK2, mask | csi_core_read(MASK2));
		default:
			return ERR_OUT_OF_BOUND;
	}
}

IM_UINT8 csi_event_enable(IM_UINT32 mask, IM_UINT8 err_reg_no)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	switch (err_reg_no)
	{
		case 1:
			return csi_core_write(MASK1, (~mask) & csi_core_read(MASK1));
		case 2:
			return csi_core_write(MASK2, (~mask) & csi_core_read(MASK2));
		default:
			return ERR_OUT_OF_BOUND;
	}
}
IM_UINT32 csi_event_get_source(IM_UINT8 err_reg_no)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	switch (err_reg_no)
	{
		case 1:
			return csi_core_read(ERR1);
		case 2:
			return csi_core_read(ERR2);
		default:
			return ERR_OUT_OF_BOUND;
	}
}

/*  register methods */
IM_UINT32 csi_core_read(csi_registers_t address)
{
    IM_UINT32 value = 0;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
    csipwl_read_reg(address, &value);
	return value;
}

IM_UINT8 csi_core_write(csi_registers_t address, IM_UINT32 data)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (csi_core_initialized == IM_FALSE)
	{
        IM_ERRMSG((IM_STR("Driver not initialized!")));
		return ERR_NOT_INIT;
	}
    IM_INFOMSG((IM_STR("write data=0x%x, address=0x%x"), data, address));
    csipwl_write_reg(address, data);
	return SUCCESS;
}

IM_UINT8 csi_core_write_part(csi_registers_t address, IM_UINT32 data, IM_UINT8 shift, IM_UINT8 width)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	IM_UINT32 mask = (1 << width) - 1;
	IM_UINT32 temp = csi_core_read(address);
	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	return csi_core_write(address, temp);
}

IM_UINT32 csi_core_read_part(csi_registers_t address, IM_UINT8 shift, IM_UINT8 width)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return (csi_core_read(address) >> shift) & ((1 << width) - 1);
}
