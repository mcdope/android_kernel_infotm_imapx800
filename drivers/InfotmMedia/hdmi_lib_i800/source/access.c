/*
 * access.c
 * 
 *  Synopsys Inc.
 *  SG DWC PT02
 */
#include <InfotmMedia.h>
#include "access.h"
#include "hdmi_log.h"

#include "error.h"
#include "mutex.h"

static void * access_mMutex = NULL;
static IM_UINT8 * access_mBaseAddr = 0;
IM_INT32 access_Initialize(IM_UINT8 * baseAddr)
{
	//access_mBaseAddr = baseAddr;
	access_mBaseAddr = (IM_UINT8 *)ioremap_nocache((IM_UINT32)baseAddr, 0x8000);
	mutex_Initialize(access_mMutex);
	return TRUE;
}

IM_UINT8 access_CoreReadByte(IM_UINT16 addr)
{
	IM_UINT8 data = 0;
	mutex_Lock(access_mMutex);
	data = access_Read(addr);
	mutex_Unlock(access_mMutex);
	return data;
}

IM_UINT8 access_CoreRead(IM_UINT16 addr, IM_UINT8 shift, IM_UINT8 width)
{
	if (width <= 0)
	{
		error_Set(ERR_DATA_WIDTH_INVALID);
		LOG_ERROR("Invalid parameter: width == 0");
		return 0;
	}
	return (access_CoreReadByte(addr) >> shift) & (BIT(width) - 1);
}

void access_CoreWriteByte(IM_UINT8 data, IM_UINT16 addr)
{
	mutex_Lock(access_mMutex);
	access_Write(data, addr);
	mutex_Unlock(access_mMutex);
}

void access_CoreWrite(IM_UINT8 data, IM_UINT16 addr, IM_UINT8 shift, IM_UINT8 width)
{
	IM_UINT8 temp = 0;
	IM_UINT8 mask = 0;
	if (width <= 0)
	{
		error_Set(ERR_DATA_WIDTH_INVALID);
		LOG_ERROR("Invalid parameter: width == 0");
		return;
	}
	mask = BIT(width) - 1;
	if (data > mask)
	{
		error_Set(ERR_DATA_GREATER_THAN_WIDTH);
		LOG_ERROR("Invalid parameter: data > width ");
		printk("data(0x%x),addr(0x%x),shift(%d),width(%d)\n",data,addr,shift,width);
		return;
	}
	mutex_Lock(access_mMutex);
	temp = access_Read(addr);
	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	access_Write(temp, addr);
	mutex_Unlock(access_mMutex);
}

IM_UINT8 access_Read(IM_UINT16 addr)
{
	IM_UINT8 val;
	val = *(volatile IM_UINT8 *)((IM_UINT32)access_mBaseAddr + (IM_UINT32)addr);
	return val;
}

void access_Write(IM_UINT8 data, IM_UINT16 addr)
{
	/* mutex is locked */
	LOG_WRITE(addr, data);
	//if(addr == 0x3006) printk("sam : phy mask write, addr=0x%x ,data=0x%x - \n",addr,data);
	//if(addr == 0x3007) printk("sam : phy pol write, addr=0x%x ,data=0x%x - \n",addr,data);
	*(volatile IM_UINT8 *)((IM_UINT32)access_mBaseAddr + (IM_UINT32)addr) = data;
}
