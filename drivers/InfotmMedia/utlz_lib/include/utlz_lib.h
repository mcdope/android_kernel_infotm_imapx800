/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Revision History: 
** ----------------- 
** 1.0.1	leo@2012/05/04: first commit.
**
*****************************************************************************/
 
#ifndef __IM_UTLZLIB_H__
#define __IM_UTLZLIB_H__

//#############################################################################
//
//
//
typedef void *	utlz_handle_t;

typedef struct{
    IM_TCHAR    *name;
	IM_INT32	period;	// feedback period, second unit.
	void (*callback)(IM_INT32 coeffi);
}utlz_module_t;

//
//
//
#define UTLZ_NOTIFY_CODE_RUN		(1)
#define UTLZ_NOTIFY_CODE_STOP		(2)

IM_RET utlzlib_init(void);
IM_RET utlzlib_deinit(void);

utlz_handle_t utlzlib_register_module(utlz_module_t *um);
IM_RET utlzlib_unregister_module(utlz_handle_t hdl);
IM_RET utlzlib_notify(utlz_handle_t hdl, IM_INT32 code);

IM_UINT32 utlzlib_get_coefficient(IM_CHAR *name);

//
//
//
#define UTLZ_IOCTL_GET_COEFFICIENT	(0x1000)	// ds=utlz_ioctl_ds_get_coefficient_t *.

typedef struct{
	IM_CHAR	*module;	// IN
	IM_UINT32	coeffi;	// OUT
}utlz_ioctl_ds_get_coefficient_t;


//#############################################################################

#endif	// __IM_UTLZLIB_H__


