/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of i80 APIs
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
**
*****************************************************************************/
 
#ifndef __I80_API_H__
#define __I80_API_H__

// DI stardard interface.
IM_RET i80_init(dispdev_handle_t *handle , IM_INT32 idsx);
IM_RET i80_deinit(dispdev_handle_t handle);
IM_RET i80_set_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix);
IM_RET i80_get_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix);
IM_RET i80_open(dispdev_handle_t handle);
IM_RET i80_close(dispdev_handle_t handle);
IM_RET i80_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener);
IM_RET i80_suspend(dispdev_handle_t handle);
IM_RET i80_resume(dispdev_handle_t handle);

// Extended interface.


#endif	// __I80_API_H__

