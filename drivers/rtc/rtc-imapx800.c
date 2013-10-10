/***************************************************************************** 
** drivers/rtc/rtc-imapx800.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: RTC driver for iMAPx800
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 09/18/2009 XXX	Initialized
** 1.2  XXX 10/21/2010 XXX	Add PM shutdown function, set default if invalid
*****************************************************************************/


#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/time.h>
#include <mach/imap-rtc.h>
#include <mach/belt.h>

#define	RTCMSG(x...) printk(KERN_ERR "iMAP_RTC: " x)

//#define RTC_DEBUG
#ifdef RTC_DEBUG
#define rtc_dbg(x...) printk(KERN_ERR "RTC_info:" x)
#else
#define rtc_dbg(x...) 
#endif

/* I have yet to find an iMAP implementation with more than one
 * of these rtc blocks in */

#define IMAP_RTC_ACCESS_TM	0x01
#define IMAP_RTC_ACCESS_ALM	0x02
#define IMAP_RTC_ACCESS_READ	0x10
#define IMAP_RTC_ACCESS_WRITE	0x20

static struct resource *imap_rtc_mem;

static void __iomem *imap_rtc_base;
static int imap_rtc_alarmno		= NO_IRQ;
static struct rtc_device *imap_rtc;
static int week;


/* FIXME: Workaround for RTC writing delay */
static unsigned long set_jiffies = 0;
static struct rtc_time rtc_buffer;
void rtc_get(void __iomem *base,int *week, int *day, int *hour, int *min, int *sec, int *msec);
int  read_rtc_gpx(int io_num)
{
    int err;
    err = readb(IO_ADDRESS(SYSMGR_RTC_BASE) + SYS_RTC_IO_ENABLE);
    err |= (0x1<<io_num);
    writel(err,IO_ADDRESS(SYSMGR_RTC_BASE) + SYS_RTC_IO_ENABLE);
    err = readb(IO_ADDRESS(SYSMGR_RTC_BASE) + SYS_RTC_IO_IN);
    err &= (0x1<<io_num);
    return err;
}
EXPORT_SYMBOL(read_rtc_gpx);

int rtc_pwrwarning_mask(int en)
{
	int err;
	if(en)
	{
		/* mask */
		err = readb(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60);
		err |= 0x01;
		writel(err, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60);
	}
	else
	{
		/* unmask */
		err = readb(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60);
		err &= 0xfe;
		writel(err, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60);
	}
}
EXPORT_SYMBOL(rtc_pwrwarning_mask);

#ifdef CONFIG_PM_SHUT_DOWN
static void RTCTIME(struct rtc_time *rtc_tm)
{
	printk(KERN_ERR "%04d-%02d-%02d %02d:%02d:%02d\n",
	   rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday,
	   rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);
	return ;
}
#endif

#if 1//defined(CONFIG_RESUME_BLOCK_DEBUG)
void __imap_rtc_access_debug(struct rtc_time *tm)
{
	void __iomem *base = imap_rtc_base;
	int msec;
	rtc_get(base, &week, &(tm->tm_wday), &(tm->tm_hour), &(tm->tm_min), &(tm->tm_sec), &msec);

}
EXPORT_SYMBOL(__imap_rtc_access_debug);
#endif

u32 rd_get_time_sec(void)
{
	u32 tmp0, tmp1, tmp2, total_sec;

	void __iomem *base = imap_rtc_base;

	tmp0 = readl(base + SYS_RTC_SECR0);
	tmp1 = readl(base + SYS_RTC_SECR1);
	tmp2 = readl(base + SYS_RTC_SECR2) & 0xf;
	total_sec = (tmp2 << 16) | (tmp1 << 8) | tmp0;

	return total_sec;
}
EXPORT_SYMBOL(rd_get_time_sec);


/* RTC common Functions for iMAP API */

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_aie(uint on)
 *
 * -Description:
 *    Turn on/off alarm enable bit.
 *
 * -Input Param
 *    on		Wether enable alarm
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
static void __imap_rtc_aie(uint on)
{
//	uint tmp;

//	pr_debug("%s: aie=%d\n", __func__, on);

//	tmp = 0;

//	if (on)
//	{
//		tmp |= 0x1;
//	}
	
	rtc_dbg("%s: aie=%d\n",__func__, on);
	writeb(!!on, imap_rtc_base + SYS_ALARM_WEN);
}

void rtc_info_set(int num, int val)
{
	int reg_addr;
	void __iomem *base = imap_rtc_base;
	rtc_dbg("%s, num = %d, val = %d(0x%x)\n", __func__, num , val, val);
	if((num >= 0)&&(num < 8))
	{
		reg_addr = SYS_INFO_0 + (num * 4);
		writel(val, base+reg_addr);
	}
}
EXPORT_SYMBOL(rtc_info_set);

int rtc_info_get(int num)
{
	int reg_addr, reg_data = 0;
	void __iomem *base = imap_rtc_base;
	
	if((num >= 0)&&(num < 8))
	{
		reg_addr = SYS_INFO_0 + (num * 4);
		reg_data = readl(base+reg_addr);
	}
	rtc_dbg("%s, num = %d, val = %d(0x%x)\n", __func__, num , reg_data, reg_data);

	return (int)reg_data;
}
EXPORT_SYMBOL(rtc_info_get);

void rtc_set(void __iomem *base,int week, int day, int hour, int min, int sec, int msec)
{
	int total_sec = 0;

	week = week % 4096; //week max is 4096
	total_sec = (((day*24 + hour)*60 + min)*60) + sec;
	
	writel((total_sec & 0xff), base + SYS_RTC_SEC0);
	writel(((total_sec & 0xff00)>>8), base + SYS_RTC_SEC1);
	writel(((total_sec & 0xf0000)>>16), base + SYS_RTC_SEC2);
	
	writel((week & 0xff), base + SYS_RTC_WEEK0);
	writel(((week & 0xf00)>>8), base + SYS_RTC_WEEK1);

	writel((msec & 0xff), base + SYS_RTC_MSEC0);
	writel(((msec & 0x7f00)>>8), base + SYS_RTC_MSEC1);

	writel(0x1, base + SYS_RTC_SECWEN);
	writel(0x1, base + SYS_RTC_WEEKWEN);
	writel(0x1, base + SYS_RTC_MSECWEN);
}

void rtc_get(void __iomem *base,int *week, int *day, int *hour, int *min, int *sec, int *msec)
{

	int total_sec = 0;
	unsigned int tmp0,tmp1,tmp2;

	tmp0 = readl(base + SYS_RTC_WEEKR0);
	tmp1 = readl(base + SYS_RTC_WEEKR1) & 0xf;
	*week = (tmp1 << 8) | tmp0;
	
	tmp0 = readl(base + SYS_RTC_SECR0);
	tmp1 = readl(base + SYS_RTC_SECR1);
	tmp2 = readl(base + SYS_RTC_SECR2) & 0xf;
	total_sec = (tmp2 << 16) | (tmp1 << 8) | tmp0;
	
	tmp0 = readl(base + SYS_RTC_MSECR0);
	tmp1 = readl(base + SYS_RTC_MSECR1) & 0x7f;
	*msec = (tmp1 << 8) | tmp0;

	*sec = total_sec % 60;
	*min = (total_sec % (60*60))/60;
	*hour = (total_sec % (60*60*24))/(60*60);
	*day = (total_sec % (60*60*24*7))/(60*60*24);
		
}


void rtc_alarm_set(void __iomem *base,int week, int day, int hour, int min, int sec)
{
	int total_sec = 0;

	week = week % 4096; //week max is 4096
	total_sec = (((day*24 + hour)*60 + min)*60) + sec;
	
	writel((total_sec & 0xff), base + SYS_ALARM_SEC0);
	writel(((total_sec & 0xff00)>>8), base + SYS_ALARM_SEC1);
	writel(((total_sec & 0xf0000)>>16), base + SYS_ALARM_SEC2);
	
	writel((week & 0xff), base + SYS_ALARM_WEEK0);
	writel(((week & 0xf00)>>8), base + SYS_ALARM_WEEK1);
	
	writel(0x1, base + SYS_ALARM_WEN);
	
}

void rtc_alarm_set_ext(u32 type, u32 data)
{
	void __iomem *base = imap_rtc_base;

	if(belt_scene_get() & SCENE_RTC_WAKEUP)
		// refuse set alarm_ext if rtc wakeup
		return ;

	if(type == 1)
	{
		writel((data & 0xff), base + SYS_ALARM_SEC0);
		writel(((data & 0xff00)>>8), base + SYS_ALARM_SEC1);
		writel(((data & 0xff0000)>>16), base + SYS_ALARM_SEC2);	
	}

	if(type == 2)
	{
		writel((data & 0xff), base + SYS_ALARM_WEEK0);
		writel(((data & 0xff00)>>8), base + SYS_ALARM_WEEK1);
	}
}
EXPORT_SYMBOL(rtc_alarm_set_ext);


void rtc_alarm_get(void __iomem *base,int *week, int *day, int *hour, int *min, int *sec)
{
	int total_sec = 0;
	unsigned int tmp0,tmp1,tmp2;
	
	*week = (*week) % 4096; //week max is 4096
	total_sec = ((((*day)*24 + *hour)*60 + *min)*60) + *sec;
	
	tmp0 = readl(base + SYS_ALARM_SEC0);
	tmp1 = readl(base + SYS_ALARM_SEC1);
	tmp2 = readl(base + SYS_ALARM_SEC2);
	total_sec = (tmp2<<16) | (tmp1<<8) | tmp0;
	
	tmp0 = readl(base + SYS_ALARM_WEEK0);
	tmp1 = readl(base + SYS_ALARM_WEEK1);
	*week = (tmp1<<8) | tmp0;
	
	*sec = total_sec % 60;
	*min = (total_sec % (60*60))/60;
	*hour = (total_sec % (60*60*24))/(60*60);
	*day = (total_sec % (60*60*24*7))/(60*60*24);
	
}

u32 rtc_alarm_get_ext(u32 type)
{
	u32 total_sec = 0;
	unsigned int tmp0,tmp1,tmp2;
	void __iomem *base = imap_rtc_base;

	if(type == 1)
	{
		tmp0 = readl(base + SYS_ALARM_SEC0);
		tmp1 = readl(base + SYS_ALARM_SEC1);
		tmp2 = readl(base + SYS_ALARM_SEC2);
	}
	
	if(type == 2)
	{
		tmp0 = readl(base + SYS_ALARM_WEEK0);
		tmp1 = readl(base + SYS_ALARM_WEEK1);
		tmp2 = 0;
	}

	total_sec = (tmp2<<16) | (tmp1<<8) | tmp0;
	
	return total_sec;
}
EXPORT_SYMBOL(rtc_alarm_get_ext);
/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_access(struct rtc_time *tm, uint acc)
 *
 * -Description:
 *	  Access RTC registers to set or read time/alarm.
 *
 * -Input Param
 *	  tm, the structure to store time values.
 *	  acc,	access descriptor,	0x00 write time
 *				   	0x01 write alarm
 *					0x10 read time
 *					0x11 read alarm
 *
 * -Output Param
 *	tm, the structure to store time values.
 *                
 * -Return
 *	None
 *
 * -Others
 *	None
 ***********************************************************************
 */
void __imap_rtc_access(struct rtc_time *tm, uint acc)
{
	void __iomem *base = imap_rtc_base;
	int msec;

	if (likely(acc & IMAP_RTC_ACCESS_READ))
	{
		if (likely(acc & IMAP_RTC_ACCESS_TM))
		{
			rtc_get(base, &week, &(tm->tm_wday), &(tm->tm_hour), &(tm->tm_min), &(tm->tm_sec), &msec);
		} else {
			rtc_alarm_get(base, &week, &(tm->tm_wday), &(tm->tm_hour), &(tm->tm_min), &(tm->tm_sec));
		}
	} else { /* Write process */
		if (likely(acc & IMAP_RTC_ACCESS_TM))
		{
			rtc_set(base, week, tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec, 0);
		} else {
			rtc_alarm_set(base, week, tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		}
	}
	return ;
}

/* IRQ Handlers */
/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_alarmirq(int irq, void *id)
 *
 * -Description:
 *    Interrupt handler when RTCALARM INT is generated.
 *
 * -Input Param
 *    irq		The irq number
 *    id		The private device pointer
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    IRQ_HANDLED
 *
 * -Others
 *    None
 ***********************************************************************
 */
static irqreturn_t imap_rtc_alarmirq(int irq, void *id)
{
	struct rtc_device *rdev = id;
	void __iomem *base = imap_rtc_base;
	rtc_dbg("---%s invoked---\n",__func__);

	writel(0x80,base + SYS_INT_CLR);
	rtc_update_irq(rdev, 1, RTC_AF | RTC_IRQF);

	return IRQ_HANDLED;
}

static int imap_rtc_aie(struct device *dev, unsigned int enabled)
{
	__imap_rtc_aie(enabled);
	return 0;
}

/* Time read/write */
/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
 *
 * -Description:
 *	  Read the current time from RTC registers.
 *	  The date and time value is store in BCD format in BCDSEC~BCDYEAR
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *
 * -Output Param
 *	  rtc_tm	This is a pointer to a time values container structure,
 *				which carries the read values.
 *                
 * -Return
 *	  0 on success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
int imap_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	unsigned long sum_sec;
	struct rtc_time tm;
	unsigned long sec_delta;
	rtc_dbg("---%s invoked, %p---\n",__func__,dev);

	if (set_jiffies && time_before(jiffies, set_jiffies+(1*HZ)))
	{
		rtc_tm->tm_sec  = rtc_buffer.tm_sec;
		rtc_tm->tm_min  = rtc_buffer.tm_min;
		rtc_tm->tm_hour = rtc_buffer.tm_hour;
		rtc_tm->tm_mday = rtc_buffer.tm_mday;
		rtc_tm->tm_mon  = rtc_buffer.tm_mon;
		rtc_tm->tm_year = rtc_buffer.tm_year;
	}
	else
	{
		week = 0;

		__imap_rtc_access(rtc_tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_TM);

		/* the only way to work out wether the system war mid-update
		* when we read it is to check the second counter, and if it
		* is zero, then we re-try the entire read
		*/
		if(!rtc_tm->tm_sec)
			__imap_rtc_access(rtc_tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_TM);

		sec_delta = mktime(2007,1,1,0,0,0);
		sum_sec = ((((week*7 + rtc_tm->tm_wday)*24 + rtc_tm->tm_hour)*60 + rtc_tm->tm_min)*60 + rtc_tm->tm_sec) + sec_delta;
		rtc_time_to_tm(sum_sec,&tm);

		rtc_tm->tm_sec  = tm.tm_sec;
		rtc_tm->tm_min  = tm.tm_min;
		rtc_tm->tm_hour = tm.tm_hour;
		rtc_tm->tm_mday = tm.tm_mday;
		rtc_tm->tm_mon  = tm.tm_mon;
		rtc_tm->tm_year = tm.tm_year;
	}

	rtc_dbg("rtc get time %04d-%02d-%02d  %02d:%02d:%02d\n",
			rtc_tm->tm_year+1900, rtc_tm->tm_mon+1, rtc_tm->tm_mday,
			rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	return 0;
}
EXPORT_SYMBOL(imap_rtc_gettime);


/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_settime(struct device *dev, struct rtc_time *tm)
 *
 * -Description:
 *	  Store the values in tm structure into RTC registers.
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *	  rtc_tm	This is a pointer to a time values container structure,
 *				values in this structure will be wrote into RTC regs.
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0		    on success
 *	  -EINVAL	on failure
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	struct rtc_time ttm;
	unsigned long sum_sec;
	unsigned long sec_delta;
	rtc_dbg("---%s invoked, %p---\n",__func__,dev);

	memcpy(&ttm, tm, sizeof(struct rtc_time));

	if((ttm.tm_year < 107) || (ttm.tm_year > 184))
	{
		rtc_dbg("RTC settime out of range.\n");
		return -EINVAL;
	}

	rtc_dbg("rtc set time %04d-%02d-%02d  %02d:%02d:%02d\n",
	   		ttm.tm_year+1900, ttm.tm_mon+1, ttm.tm_mday,
	   		ttm.tm_hour, ttm.tm_min, ttm.tm_sec);

	week = 0;
	sec_delta = mktime(2007,1,1,0,0,0);

	sum_sec = mktime(ttm.tm_year+1900,ttm.tm_mon+1,ttm.tm_mday,ttm.tm_hour,ttm.tm_min,ttm.tm_sec) - sec_delta;
	week = sum_sec/(60*60*24*7);
	sum_sec -= week*60*60*24*7;
	ttm.tm_sec = sum_sec % 60;
	ttm.tm_min = (sum_sec % (60*60))/60;
	ttm.tm_hour = (sum_sec % (60*60*24))/(60*60);
	ttm.tm_wday = sum_sec/(60*60*24);

	__imap_rtc_access(&ttm, IMAP_RTC_ACCESS_WRITE | IMAP_RTC_ACCESS_TM);

	set_jiffies = jiffies;
	rtc_buffer.tm_sec	= tm->tm_sec;
	rtc_buffer.tm_min	= tm->tm_min;
	rtc_buffer.tm_hour	= tm->tm_hour;
	rtc_buffer.tm_mday	= tm->tm_mday;
	rtc_buffer.tm_wday	= tm->tm_wday;
	rtc_buffer.tm_yday	= tm->tm_yday;
	rtc_buffer.tm_mon	= tm->tm_mon;
	rtc_buffer.tm_year	= tm->tm_year;

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
 *
 * -Description:
 *	  Read the current alarm from RTC registers.
 *	  The date and time value is store in BCD format in ALMSEC~ALMYEAR
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *
 * -Output Param
 *	  alrm		Alarm structure, containning:
 *					enabled;	0 = alarm disabled, 1 = alarm enabled 
 *					pending;	0 = alarm not pending, 1 = alarm pending 
 *					time;		time the alarm is set to 
 *				The read values is stored in alrm.time, if the relative
 *				alarm bit is not enabled, 0xff will be read.
 *                
 * -Return
 *	  0 on success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *alm_tm = &alrm->time,tm,ttm;
	void __iomem *base = imap_rtc_base;
	uint alm_en;
	unsigned long sum_sec;
	unsigned long sec_delta = mktime(2007,1,1,0,0,0);
	rtc_dbg("---%s invoked, %p---\n",__func__,dev);

	week = 0;

	__imap_rtc_access(&tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_ALM);
	
	sum_sec = ((((week*7 + tm.tm_wday)*24 + tm.tm_hour)*60 + tm.tm_min)*60 + tm.tm_sec) + sec_delta;
	rtc_time_to_tm(sum_sec,&ttm);

	alm_tm->tm_sec  = ttm.tm_sec;
	alm_tm->tm_min  = ttm.tm_min;
	alm_tm->tm_hour = ttm.tm_hour;
	alm_tm->tm_mday = ttm.tm_mday;
	alm_tm->tm_mon  = ttm.tm_mon;
	alm_tm->tm_year = ttm.tm_year;
	alm_tm->tm_wday = ttm.tm_wday;
	alm_tm->tm_yday = ttm.tm_yday;

	alm_en = readb(base + SYS_ALARM_WEN);
	alrm->enabled = (alm_en) ? 1: 0;

	rtc_dbg("rtc get alarm %02d, %04d-%02d-%02d  %02d:%02d:%02d\n",
	   alm_en,
	   alm_tm->tm_year+1900, alm_tm->tm_mon+1, alm_tm->tm_mday,
	   alm_tm->tm_hour, alm_tm->tm_min, alm_tm->tm_sec);

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
 *
 * -Description:
 *	  Store the values in tm structure into RTC registers.
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *	  alrm		values in alrm.time will be stored into RTC ALM regs,
 *				and alarm will be enabled.
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0		    on success
 *	  -EINVAL	on failure
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time ttm;
	unsigned long sum_sec;
	unsigned long sec_delta;
	rtc_dbg("---%s invoked, %p---\n",__func__,dev);

	memcpy(&ttm, &alrm->time, sizeof(struct rtc_time));

	if((ttm.tm_year < 107) || (ttm.tm_year > 184))
	{
		rtc_dbg("RTC setalarm out of range.\n");
		return -EINVAL;
	}

	rtc_dbg("rtc set alarm: %d, %04d-%02d-%02d  %02d:%02d:%02d\n",
	   alrm->enabled,
	   ttm.tm_year+1900, ttm.tm_mon+1, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec);

	week = 0;
	sec_delta = mktime(2007,1,1,0,0,0);

	sum_sec = mktime(ttm.tm_year+1900,ttm.tm_mon+1,ttm.tm_mday,ttm.tm_hour,ttm.tm_min,ttm.tm_sec) - sec_delta;
	week = sum_sec/(60*60*24*7);
	sum_sec -= week*60*60*24*7;
	ttm.tm_sec = sum_sec % 60;
	ttm.tm_min = (sum_sec % (60*60))/60;
	ttm.tm_hour = (sum_sec % (60*60*24))/(60*60);
	ttm.tm_wday = sum_sec/(60*60*24);

	__imap_rtc_access(&ttm, IMAP_RTC_ACCESS_WRITE | IMAP_RTC_ACCESS_ALM);

	__imap_rtc_aie(alrm->enabled);

	return 0;
}

void rtc_set_delayed_alarm(u32 delay)
{
	struct rtc_time tm;
	struct timespec time;
	struct rtc_wkalrm alm_tm;

	imap_rtc_gettime(NULL, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);

	rtc_time_to_tm(time.tv_sec + delay, &tm);	
	//printk("rtc1: %d, %d, %d, %d, %d, %d, %d, %d\n", tm.tm_year, tm.tm_yday, tm.tm_wday, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	memcpy(&alm_tm.time, &tm, sizeof(struct rtc_time));

	alm_tm.enabled = 1;
	imap_rtc_setalarm(NULL, &alm_tm);
	//printk("rtc-alm: %d, %d, %d, %d, %d, %d, %d, %d\n", alm_tm.time.tm_year, alm_tm.time.tm_yday, alm_tm.time.tm_wday, 
	//		alm_tm.time.tm_mon, alm_tm.time.tm_mday, alm_tm.time.tm_hour, alm_tm.time.tm_min, alm_tm.time.tm_sec);

}
EXPORT_SYMBOL(rtc_set_delayed_alarm);

static int imap_rtc_proc(struct device *dev, struct seq_file *seq)
{
	if(!dev || !dev->driver)
		return 0;

	return seq_printf(seq, "name\t\t: %s\n", dev_name(dev));
}

static int imap_rtc_open(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rtc_device *rtc_dev = platform_get_drvdata(pdev);
	int ret;
	rtc_dbg("---%s invoked by %p---\n",__func__,dev);

	ret = request_irq(imap_rtc_alarmno, imap_rtc_alarmirq,
	   IRQF_DISABLED, "imap-rtc alarm", rtc_dev);

	if (ret) {
		dev_err(dev, "IRQ%d error %d\n", imap_rtc_alarmno, ret);
	}

	return ret;
}

static void imap_rtc_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rtc_device *rtc_dev = platform_get_drvdata(pdev);

	/* do not clear AIE here, it may be needed for wake */

	free_irq(imap_rtc_alarmno, rtc_dev);
}

static const struct rtc_class_ops imap_rtcops = {
	.open				=	imap_rtc_open,
	.release			=	imap_rtc_release,
	.ioctl				=	NULL,
	.read_time			=	imap_rtc_gettime,
	.set_time			=	imap_rtc_settime,
	.read_alarm			=	imap_rtc_getalarm,
	.set_alarm			=	imap_rtc_setalarm,
	.proc				=	imap_rtc_proc,
	.alarm_irq_enable		=	imap_rtc_aie,
};

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_probe(struct platform_device *pdev)
 *
 * -Description:
 *	  Platform device probe.
 *
 * -Input Param
 *	  pdev		platform device pointer provided by system
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0			On success
 *	  -ENOENT	If resource is not availiable
 *	  -EINVAL	If resource is invalid
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int __devinit imap_rtc_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct rtc_time tm;
	int ret;
	RTCMSG("+++%s: probe=%p+++\n", __func__, pdev);

	/* find the IRQs */
	
	/*tckint has been removed in imapx800*/

	imap_rtc_alarmno = platform_get_irq(pdev, 0);
	if (imap_rtc_alarmno < 0) {
		RTCMSG("no irq for alarm\n");
		return -ENOENT;
	}

	rtc_dbg("Alarm IRQ = %d\n", imap_rtc_alarmno);

	/* get the memory region */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		RTCMSG("failed to get memory region resource\n");
		return -ENOENT;
	}

	imap_rtc_mem = request_mem_region(res->start,
	   res->end - res->start + 1, pdev->name);
	
	if (imap_rtc_mem == NULL) {
		RTCMSG("failed to reserve memory region\n");
		ret = -ENOENT;
		goto err_nores;
	}

	imap_rtc_base = ioremap(res->start, res->end - res->start + 1);
	if (imap_rtc_base == NULL) {
		RTCMSG("failed ioremap()\n");
		ret = -EINVAL;
		goto err_nomap;
	}

	/* check to see if everything is setup correctly */

	/* register RTC and exit */

	imap_rtc = rtc_device_register("imap-rtc", &pdev->dev, &imap_rtcops, THIS_MODULE);

	if (IS_ERR(imap_rtc)) {
		RTCMSG("cannot attach rtc\n");
		ret = PTR_ERR(imap_rtc);
		goto err_nortc;
	}

	imap_rtc_gettime(&pdev->dev, &tm);
	if(rtc_valid_tm(&tm))
	{
		/* invalid RTC time, set RTC to 2010/09/22 */
		RTCMSG("Invalid RTC time detected, setting to default.\n");
		rtc_time_to_tm(mktime(2010, 9, 22, 0, 0, 0), &tm);
		imap_rtc_settime(&pdev->dev, &tm);
	}

	/* open int mask */
	writel(readb(imap_rtc_base + SYS_INT_MASK)&(~0x8),imap_rtc_base + SYS_INT_MASK);
	platform_set_drvdata(pdev, imap_rtc);
	return 0;

err_nortc:
	iounmap(imap_rtc_base);

err_nomap:
	release_resource(imap_rtc_mem);

err_nores:
	return ret;
}

static int __devexit imap_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	rtc_device_unregister(rtc);

	imap_rtc_aie(&pdev->dev, 0);

	iounmap(imap_rtc_base);
	release_resource(imap_rtc_mem);
	kfree(imap_rtc_mem);

	return 0;
}

#ifdef CONFIG_PM
/* RTC Power management control */

//static struct timespec imap_rtc_delta;
//static int ticnt_save;

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_suspend
 *	  imap_rtc_resume
 *
 * -Description:
 *	  RTC PM functions, RTC should be disabled before suspend, and re-enabled
 *	  after resuem. Alarm state should be leave alone.
 *
 * -Input Param
 *	  pdev		platform device pointer provided by system
 *    state		Standard function interface required, but not used here
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0			On success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct rtc_time tm;
	struct timespec time;
#ifdef CONFIG_PM_SHUT_DOWN
	struct rtc_wkalrm alrm;

	void __iomem *base = imap_rtc_base;
#endif

	time.tv_nsec = 0;

	imap_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);

#ifdef CONFIG_PM_SHUT_DOWN
	if(rtc_valid_tm(&tm))
	{
		/* The time in RTC is invalid */
		RTCMSG("Time in RTC is invalid!\n");
	} else
	{
		RTCMSG("Current time is:\n");
		RTCTIME(&tm);

		rtc_time_to_tm(time.tv_sec + CONFIG_PM_SHUT_DOWN_SEC, &alrm.time);
		alrm.enabled = 1;

		RTCMSG("Setting alarm to shut down:\n");
		RTCTIME(&alrm.time);
		imap_rtc_setalarm(&pdev->dev, &alrm);

		writel(0x1e, base + SYS_WAKEUP_MASK);
	}
#endif

	return 0;
}

static int imap_rtc_resume(struct platform_device *pdev)
{
	struct rtc_time tm = {.tm_sec = 0,};
	struct timespec time;
#ifdef CONFIG_PM_SHUT_DOWN
	static int wp_count = 0;

	void __iomem *base = imap_rtc_base;
#endif

	time.tv_nsec = 0;

#ifdef CONFIG_PM_SHUT_DOWN
	/* Check if RTC wake up, if so close the Machine */
	if(readl(base + SYS_INT_ST) & (1<<7))
	{
		/* first clear this wp status */
		writel((1<<7), base + SYS_INT_CLR);

		if(CONFIG_PM_SHUT_DOWN_SEC == 13)
		  RTCMSG("Wakeup by RTC, wake up count %d\n", ++wp_count);
		else
		{
			RTCMSG("Wakeup by RTC, it is time to shutdown machine.\n");
			/* shut down */
			writel(0x1f, base + SYS_WAKEUP_MASK);
			writel(0x2, base + SYS_CFG_CMD);
		}
	}
#endif

	while(!tm.tm_sec)
		imap_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);

	if(!rtc_valid_tm(&tm))
	  do_settimeofday(&time);

	return 0;
}
#else
#define imap_rtc_suspend NULL
#define imap_rtc_resume	 NULL
#endif

static struct platform_driver imap_rtc_driver = {
	.probe		= imap_rtc_probe,
	.remove		= __devexit_p(imap_rtc_remove),
	.suspend	= imap_rtc_suspend,
	.resume		= imap_rtc_resume,
	.driver		=	{
		.name	= "imap-rtc",
		.owner	= THIS_MODULE,
	},
};


static int __init imap_rtc_init(void)
{
	printk(KERN_INFO "iMAPx800 RTCv2, (c) 2009, 2014 InfoTM Microelctronics Co., Ltd\n");
	return platform_driver_register(&imap_rtc_driver);
}

static void __exit imap_rtc_exit(void)
{
	platform_driver_unregister(&imap_rtc_driver);
}

module_init(imap_rtc_init);
module_exit(imap_rtc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("warits <warits.wang@infotm.com>");
MODULE_DESCRIPTION("InfoTM iMAPx800 RTC driver");
