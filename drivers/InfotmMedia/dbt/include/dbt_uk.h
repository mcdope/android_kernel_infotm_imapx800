/*
 *
 * Copyright (c) 2011~2112 ShangHai Infotm Ltd all rights reserved. 
 * 
 * Use of Infotm's code is governed by terms and conditions 
 * stated in the accompanying licensing statement. 
 *
 * Revision History: 
 * ----------------- 
 * v1.0.1	leo@2011/08/22: first commit.
 *
 */

#ifndef __DBT_UK_H__
#define __DBT_UK_H__

#define DBT_IOCTL_REG_READ	0x1000
#define DBT_IOCTL_REG_WRITE	0x1001

#define DBT_IOCTL_MEM_READ	0x2000
#define DBT_IOCTL_MEM_WRITE	0x2001

#define MAX_DATA_LENGTH     (1 * 1024)

typedef struct {
	IM_UINT32	phyAddr;
	IM_INT32	num;
	IM_UINT32	vals[MAX_DATA_LENGTH];
}dbt_ioctl_rw_t;


#endif	//__DBT_UK_H__
