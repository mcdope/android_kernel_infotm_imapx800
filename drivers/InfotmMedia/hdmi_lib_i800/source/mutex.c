/*
 * mutex.c
 *
 *  Created on: Jun 25, 2010
 *      Author: klabadi & dlopo
 */

#include "mutex.h"
#include "hdmi_log.h"
#include "error.h"
#ifdef __XMK__
#include <sys/process.h>
#include "errno.h"
#endif

IM_INT32 mutex_Initialize(void* pHandle)
{
	return TRUE;
}

IM_INT32 mutex_Destruct(void* pHandle)
{
	return TRUE;
}

IM_INT32 mutex_Lock(void* pHandle)
{
	return TRUE;
}

IM_INT32 mutex_Unlock(void* pHandle)
{
	return TRUE;
}
