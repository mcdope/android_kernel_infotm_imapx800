/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai InfoTM Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Header file of DSI APIs
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.1.0     Sam@2012/10/22 :  DSI first stable version, support the first product.
**
*****************************************************************************/

#ifndef _DSI_API_H_
#define _DSI_API_H_

// DI stardard interface.
IM_RET dsi_init(dispdev_handle_t *handle, IM_INT32 idsx);
IM_RET dsi_deinit(dispdev_handle_t handle);
IM_RET dsi_set_config(dispdev_handle_t handle, ids_display_device_t *config, IM_INT32 suffix);
IM_RET dsi_get_config(dispdev_handle_t handle, ids_display_device_t *config, dispdev_info_t *info, IM_INT32 suffix);
IM_RET dsi_open(dispdev_handle_t handle);
IM_RET dsi_close(dispdev_handle_t handle);
IM_RET dsi_register_listener(dispdev_handle_t handle, fcbk_dispdev_listener_t listener);
IM_RET dsi_suspend(dispdev_handle_t handle);
IM_RET dsi_resume(dispdev_handle_t handle);

#endif // _DSI_API_H_

