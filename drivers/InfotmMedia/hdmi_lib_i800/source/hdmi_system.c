/*
 * system.c
 *
 *  Created on: Jun 25, 2010
 *      Author: klabadi & dlopo
 * 
 * 
 * 	@note: this file should be re-written to match the environment the 
 * 	API is running on
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include <InfotmMedia.h>
#include "hdmi_system.h"
#include "hdmi_log.h"
#include "error.h"

void system_SleepMs(IM_UINT32 ms)
{
	msleep(ms);
}

IM_INT32 system_ThreadCreate(void* handle, thread_t pFunc, void* param)
{
	return TRUE;
}
IM_INT32 system_ThreadDestruct(void* handle)
{
	return TRUE;
}

IM_INT32 system_Start(thread_t thread)
{
	return 0;
}

IM_INT32 system_InterruptDisable(interrupt_id_t id)
{
	return TRUE;	
}

IM_INT32 system_InterruptEnable(interrupt_id_t id)
{
	return TRUE;	
}

IM_INT32 system_InterruptAcknowledge(interrupt_id_t id)
{
	return TRUE;
}
IM_INT32 system_InterruptHandlerRegister(interrupt_id_t id, handler_t handler,
		void * param)
{
	if(idspwl_register_isr(MODULE_HDMI, 0,(fcbk_intr_handler_t) handler) != IM_RET_OK)
	{
		printk(KERN_ERR " register hdmi tx interrupt error \n");
		return FALSE;
	}
	//idspwl_enable_intr(MODULE_HDMI,0);
	return TRUE;
}

IM_INT32 system_InterruptHandlerUnregister(interrupt_id_t id)
{
	//idspwl_disable_intr(MODULE_HDMI,0);
	idspwl_unregister_isr(MODULE_HDMI,0);
	return TRUE;
}

#ifdef __cplusplus
}
#endif
