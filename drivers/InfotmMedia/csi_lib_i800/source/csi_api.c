#include <InfotmMedia.h>
#include "csi_api.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CSI_API_I:"
#define WARNHEAD	"CSI_API_W:"
#define ERRHEAD		"CSI_API_E:"
#define TIPHEAD		"CSI_API_T:"

static handler_t csi_api_event_registry[MAX_EVENT] = {NULL};

IM_UINT8 csi_api_start()
{
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));
	csi_error_t e = SUCCESS;
	do
	{
		e = csi_init();
		if(e != SUCCESS)
		{
            IM_ERRMSG((IM_STR("Unable to initialise driver!")));
			break;
		}
		/* set only one lane (lane 0) as active (ON) */
		e = csi_set_on_lanes(1);
		if(e != SUCCESS)
		{
            IM_ERRMSG((IM_STR("Unable to set lanes!")));
			csi_close();
			break;
		}
        IM_INFOMSG((IM_STR("Lane set OK!")));
		e = csi_shut_down_phy(0);
		if(e != SUCCESS)
		{
            IM_ERRMSG((IM_STR("Unable to bring up PHY!")));
			csi_close();
			break;
		}
        IM_INFOMSG((IM_STR("PHY power up OK!")));

		e = csi_reset_phy();
		if(e != SUCCESS)
		{
            IM_ERRMSG((IM_STR("Unable to reset PHY!")));
			csi_close();
			break;
		}
        IM_INFOMSG((IM_STR("PHY reset OK!")));

		e = csi_reset_controller();
		if(e != SUCCESS)
		{
            IM_ERRMSG((IM_STR("Unable to reset controller!")));
			csi_close();
			break;
		}
		/* MASK all interrupts */
		csi_event_disable(0xffffffff, 1);
		csi_event_disable(0xffffffff, 2);
	} while(0);
	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return e;
}

IM_UINT8 csi_api_close()
{
	IM_INFOMSG((IM_STR("%s()++)"), IM_STR(_IM_FUNC_)));
	csi_api_unregister_all_events();
	csi_shut_down_phy(1);
	IM_INFOMSG((IM_STR("%s()--)"), IM_STR(_IM_FUNC_)));
	return csi_close();
}

IM_UINT8 csi_api_set_on_lanes(IM_UINT8 no_of_lanes)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_set_on_lanes(no_of_lanes);
}

IM_UINT8 csi_api_get_on_lanes()
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_get_on_lanes();
}

csi_lane_state_t csi_api_get_clk_state()
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_clk_state();
}

csi_lane_state_t csi_api_get_lane_state(IM_UINT8 lane)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_lane_module_state(lane);
}

static IM_UINT32 csi_api_event_map (IM_UINT8 event, IM_UINT8 vc_lane)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	switch((csi_event_t)(event))
	{
		case ERR_PHY_TX_START:
			return (0x1 << (event + vc_lane));
		case ERR_FRAME_BOUNDARY_MATCH:
			return (0x1 << (event + vc_lane));
		case ERR_FRAME_SEQUENCE:
			return (0x1 << (event + vc_lane));
		case ERR_CRC_DURING_FRAME:
			return (0x1 << (event + vc_lane));
		case ERR_LINE_CRC:
			return (0x1 << (24 + vc_lane));
		case ERR_DOUBLE_ECC:
			return (0x1 << 28);
		case ERR_PHY_ESCAPE_ENTRY:
			return (0x1 << (0 + vc_lane));
		case ERR_ECC_SINGLE:
			return (0x1 << (8 + vc_lane));
		case ERR_UNSUPPORTED_DATA_TYPE:
			return (0x1 << (12 + vc_lane));
		case MAX_EVENT:
			break;
		default:
			break;
	}
	switch((csi_line_event_t)(event))
	{
		case ERR_LINE_BOUNDARY_MATCH:
			return (0x1 << (16 + (vc_lane % 4)));
		case ERR_LINE_SEQUENCE:
			return (0x1 << (20 + (vc_lane % 4)));
		default:
			break;
	}
	/* writing the following value affects nothing */
	return 0x80000000;
}

IM_UINT8 csi_api_register_event(csi_event_t event, IM_UINT8 vc_lane, handler_t handler)
{
	/* the VC_LANE value is the lane number ONLY in PHY ERRORS
	 * the rest are all virtual channels number except for
	 * double ECC, where VC is unknown */
	IM_UINT8 e = SUCCESS;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (vc_lane < 4) /* the maximum */
	{
		switch (event)
		{
		case ERR_PHY_TX_START:
                IM_WARNMSG((IM_STR("the lane value depends on the synthesis defines!")));
                IM_WARNMSG((IM_STR("make sure this value does not exceed these defines!")));
			case ERR_FRAME_BOUNDARY_MATCH:
			case ERR_FRAME_SEQUENCE:
			case ERR_CRC_DURING_FRAME:
			case ERR_LINE_CRC:
			case ERR_DOUBLE_ECC:
				e = csi_event_enable(csi_api_event_map(event, vc_lane), 1);
				break;
			case ERR_PHY_ESCAPE_ENTRY:
                IM_WARNMSG((IM_STR("the lane value depends on the synthesis defines!")));
                IM_WARNMSG((IM_STR("make sure this value does not exceed these defines!")));
			case ERR_ECC_SINGLE:
			case ERR_UNSUPPORTED_DATA_TYPE:
				e = csi_event_enable(csi_api_event_map(event, vc_lane), 2);
				break;
			default:
				e = ERROR_EVENT_TYPE_INVALID;
				break;
		}
	}
	else
	{
		e = ERROR_VC_LANE_OUT_OF_BOUND;
	}
	if (e == SUCCESS)
	{
		csi_api_event_registry[event + vc_lane] = handler;
	}
	return e;
}

IM_UINT8 csi_api_unregister_event(csi_event_t event, IM_UINT8 vc_lane)
{
	csi_error_t e = SUCCESS;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if (vc_lane < 4) /* the maximum */
	{
		switch (event)
		{
			case ERR_PHY_TX_START:
                IM_WARNMSG((IM_STR("the lane value depends on the synthesis defines!")));
                IM_WARNMSG((IM_STR("make sure this value does not exceed these defines!")));
			case ERR_FRAME_BOUNDARY_MATCH:
			case ERR_FRAME_SEQUENCE:
			case ERR_CRC_DURING_FRAME:
			case ERR_LINE_CRC:
			case ERR_DOUBLE_ECC:
				e = csi_event_disable(csi_api_event_map(event, vc_lane), 1);
				break;
			case ERR_PHY_ESCAPE_ENTRY:
                IM_WARNMSG((IM_STR("the lane value depends on the synthesis defines!")));
                IM_WARNMSG((IM_STR("make sure this value does not exceed these defines!")));
			case ERR_ECC_SINGLE:
			case ERR_UNSUPPORTED_DATA_TYPE:
				e = csi_event_disable(csi_api_event_map(event, vc_lane), 2);
				break;
			default:
				e = ERROR_EVENT_TYPE_INVALID;
				break;
		}
	}
	else
	{
		e = ERROR_VC_LANE_OUT_OF_BOUND;
	}
	if (e == SUCCESS)
	{
		csi_api_event_registry[event + vc_lane] = NULL;
	}
	return e;
}

IM_UINT8 csi_api_register_line_event(IM_UINT8 vc, csi_data_type_t data_type, csi_line_event_t line_event, handler_t handler)
{
	IM_UINT8 id = 0;
	int counter = 0;
	int first_slot = -1;
	int already_registered = 0;
	IM_UINT8 e = SUCCESS;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if ((data_type < NULL_PACKET)
	|| ((data_type > EMBEDDED_8BIT_NON_IMAGE_DATA) && (data_type < YUV420_8BIT))
	|| ((data_type > RGB888) && (data_type < RAW6))
	|| ((data_type > RAW14) && (data_type < USER_DEFINED_8BIT_DATA_TYPE_1 ))
	|| (data_type > USER_DEFINED_8BIT_DATA_TYPE_8))
	{
		/* wrong data type - not long packet */
		return ERROR_DATA_TYPE_INVALID;
	}
	if (vc > 3) /* the maximum */
	{
		return ERROR_VC_LANE_OUT_OF_BOUND;
	}
	id = (IM_UINT8)((vc << 6)) | (IM_UINT8)(data_type);

	for (counter = 0; counter < 8; counter++)
	{
		if (csi_get_registered_line_event(counter) == 0)
		{
			/* first empty slot to register event */
			first_slot = counter;
			break;
		}
		if (id == csi_get_registered_line_event(counter))
		{
            IM_WARNMSG((IM_STR("already registered line/datatype")));
			already_registered = 1;
			first_slot = counter; /* no need to register it again */
								/* but NOT that there are no slots!*/
			break;
		}
	}
	if (first_slot != -1) /* if not all slots are taken */
	{
		/* un-mask (whether registered or not) */
		e = csi_event_enable(csi_api_event_map(line_event, first_slot), (first_slot / 4 == 1)? 2: 1);
		if (e == SUCCESS)
		{
			if (already_registered != 1) /* if not already registered */
			{
				e = csi_register_line_event(vc, data_type, first_slot);
				if (csi_api_event_registry[line_event + first_slot] != NULL)
				{
                    IM_WARNMSG((IM_STR("already registered event and callback (overwriting!)")));
				}
			}
			/* register callback */
			csi_api_event_registry[line_event + first_slot] = handler;
		}
	}
	else
	{
        IM_WARNMSG((IM_STR("all slots are taken!)")));
		e = ERROR_SLOTS_FULL;
	}
	return e;
}

IM_UINT8 csi_api_unregister_line_event(IM_UINT8 vc, csi_data_type_t data_type, csi_line_event_t line_event)
{
	IM_UINT8 id = 0;
	int counter = 0;
	int replace_slot = -1;
	int last_slot = -1;
	IM_UINT8 vc_new = 0;
	csi_data_type_t dt_new;
	csi_line_event_t other_line_event;

	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	if ((data_type < NULL_PACKET)
	|| ((data_type > EMBEDDED_8BIT_NON_IMAGE_DATA) && (data_type < YUV420_8BIT))
	|| ((data_type > RGB888) && (data_type < RAW6))
	|| ((data_type > RAW14) && (data_type < USER_DEFINED_8BIT_DATA_TYPE_1 ))
	|| (data_type > USER_DEFINED_8BIT_DATA_TYPE_8))
	{
		/* wrong data type - not long packet */
		return ERROR_DATA_TYPE_INVALID;
	}
	if (vc > 3) /* the maximum */
	{
		return ERROR_VC_LANE_OUT_OF_BOUND;
	}
	if (line_event == ERR_LINE_BOUNDARY_MATCH)
	{
		other_line_event = ERR_LINE_SEQUENCE;
	}
	else if (line_event == ERR_LINE_SEQUENCE)
	{

		other_line_event = ERR_LINE_BOUNDARY_MATCH;
	}
	else
	{
		return ERROR_DATA_TYPE_INVALID;
	}
	id = (vc << 6) | data_type;
	for (counter = 0; counter < 8; counter++)
	{
		if (id == csi_get_registered_line_event(counter))
		{
			replace_slot = counter;
		}
		if (csi_get_registered_line_event(counter) == 0)
		{
			last_slot = counter - 1;
			break;
		}
	}
	if (replace_slot == -1)
	{
		return ERROR_NOT_REGISTERED;
	}
	if (last_slot == -1)
	{
		last_slot = 7; /* the very last entry */
	}
	vc_new = csi_get_registered_line_event(last_slot) >> 6;
	dt_new = (csi_get_registered_line_event(last_slot) << 2) >> 2;

	if ((csi_api_event_registry[other_line_event + replace_slot] == NULL) && (last_slot != replace_slot))
	{
		/* swap IDI with last registered if its NOT the last entry*/
		/* copy the last to (over) the one to be deleted*/
		csi_register_line_event(vc_new, dt_new, replace_slot);
		/* now that last registered is copied to a new location, unregister old location */
		csi_event_disable(csi_api_event_map(line_event, last_slot), (last_slot / 4 == 1)? 2: 1);
		/* swap event registry */
		csi_api_event_registry[line_event + replace_slot] = csi_api_event_registry[line_event + last_slot];
		/* mask last slot */
		if (csi_api_event_registry[other_line_event + last_slot] != NULL)
		{
			/* swap event registry - the other possible line event of the last slot */
			csi_api_event_registry[other_line_event + replace_slot] = csi_api_event_registry[other_line_event + last_slot];
			/* mask last slot, other line event!*/
			csi_event_disable(csi_api_event_map(other_line_event, last_slot), (last_slot / 4 == 1)? 2: 1);
			/* un mask other line event */
			csi_event_enable(csi_api_event_map(other_line_event, replace_slot), (replace_slot / 4 == 1)? 2: 1);
		}
		csi_unregister_line_event(last_slot);
	}
	else
	{
		/* remove from event registry */
		csi_api_event_registry[line_event + replace_slot] = NULL;
		/* mask */
		csi_event_disable(csi_api_event_map(line_event, replace_slot), (replace_slot / 4 == 1)? 2: 1);

		if ((csi_api_event_registry[other_line_event + replace_slot] == NULL) && (last_slot == replace_slot))
		{  /* if it is last entry */
			csi_unregister_line_event(last_slot);
		}
	}
	return SUCCESS;
}


void csi_api_event_handler(void *param)
{
	IM_UINT32 source = 0;
	IM_UINT32 tmp = 0x80000000;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	source = csi_event_get_source(1);
	if (source > 0)
	{	/* map to enumerator csi_event_t or csi_line_event_t */
		if ((source & (0xf << 0)) > 0)
		{
			tmp = ERR_PHY_TX_START + ((source & 0xf) >> 1);
		}
		if ((source & (0xf << 4)) > 0)
		{
			tmp = ERR_FRAME_BOUNDARY_MATCH + ((source & 0xf0) >> 5);
		}
		if ((source & (0xf << 8)) > 0)
		{
			tmp = ERR_FRAME_SEQUENCE + ((source & (0xf << 8)) >> 9);
		}
		if ((source & (0xf << 12)) > 0)
		{
			tmp = ERR_CRC_DURING_FRAME + ((source & (0xf << 12)) >> 13);
		}
		if ((source & (0xf << 16)) > 0)
		{
			tmp = ERR_LINE_BOUNDARY_MATCH + ((source & (0xf << 16)) >> 17);
		}
		if ((source & (0xf << 20)) > 0)
		{
			tmp = ERR_LINE_SEQUENCE + ((source & (0xf << 20)) >> 21);
		}
		if ((source & (0xf << 24)) > 0)
		{
			tmp = ERR_LINE_CRC + ((source & (0xf << 24)) >> 25);
		}
		if ((source & (0xf << 28)) > 0)
		{
			tmp = ERR_DOUBLE_ECC;
		}
		/* call callback if valid */
		if (tmp != 0x80000000)
		{
			if (csi_api_event_registry[tmp] != NULL)
			{
				csi_api_event_registry[tmp](param);
			}
		}
	}
	else
	{
		source = csi_event_get_source(2);
		if (source > 0)
		{	/* map to enumerator csi_event_t or csi_line_event_t */
			if ((source & (0xf << 0)) > 0)
			{
				tmp = ERR_PHY_ESCAPE_ENTRY + ((source & 0xf) >> 1);
			}
			if ((source & (0xf << 4)) > 0)
			{
				; /* error discarded */
			}
			if ((source & (0xf << 8)) > 0)
			{
				tmp = ERR_ECC_SINGLE + ((source & (0xf << 8)) >> 9);
			}
			if ((source & (0xf << 12)) > 0)
			{
				tmp = ERR_UNSUPPORTED_DATA_TYPE + ((source & (0xf << 12)) >> 13);
			}
			if ((source & (0xf << 16)) > 0)
			{
				tmp = ERR_LINE_BOUNDARY_MATCH + 4 + ((source & (0xf << 16)) >> 17);
				/* 4 added because its in ERR2. ERR1 already has 4 */
			}
			if ((source & (0xf << 20)) > 0)
			{
				tmp = ERR_LINE_SEQUENCE + 4 + ((source & (0xf << 20)) >> 21);
			}
			/* call callback if valid */
			if (tmp != 0x80000000)
			{
				if (csi_api_event_registry[tmp] != NULL)
				{
					csi_api_event_registry[tmp](param);
				}
			}
		}
	}
}

IM_UINT8 csi_api_unregister_all_events()
{
	IM_UINT8 e = SUCCESS;
	IM_UINT8 counter = 0;
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	e = csi_event_disable(0xffffffff, 1);
	e = csi_event_disable(0xffffffff, 2);
	if (e == SUCCESS)
	{
		for (counter = 0; counter < 8; counter++)
		{
			e = csi_unregister_line_event(counter);
		}
		for (counter = (IM_UINT8)(ERR_PHY_TX_START); counter < (IM_UINT8)(MAX_EVENT); counter++)
		{
			csi_api_event_registry[counter] = NULL;
		}
	}
	return e;
}

IM_UINT8 csi_api_shut_down_phy(IM_UINT8 shutdown)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_shut_down_phy(shutdown);
}

IM_UINT8 csi_api_reset_phy()
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_reset_phy();
}

IM_UINT8 csi_api_reset_controller()
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_reset_controller();
}

IM_UINT8 csi_api_core_write(csi_registers_t address, IM_UINT32 data)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_core_write(address, data);
}

IM_UINT32 csi_api_core_read(csi_registers_t address)
{
	IM_INFOMSG((IM_STR("%s())"), IM_STR(_IM_FUNC_)));
	return csi_core_read(address);
}
