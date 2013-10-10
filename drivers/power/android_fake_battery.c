/*
 * * Fake Battery driver for android
 * *
 * * Copyright © 2009 Rockie Cheng <aokikyon@gmail.com>
 * *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 as
 * * published by the Free Software Foundation.
 * */
/*
 * the condition that low level voltage warning message can display are:
 * 1, the voltage lower than 15%
 * 2, there is a debounce between two voltage and condition 1 is satisfied.
 **/

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
//#include <mach/imapx_base_reg.h>
#include <linux/io.h>
#include <asm/delay.h>
#include <linux/interrupt.h>
#include <asm/io.h>
//#include <mach/imapx_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <mach/pad.h>
#include <mach/items.h>

#if (defined(CONFIG_TP_TI2046) || defined(CONFIG_BATT_TI2046))
#include <linux/spi/ads7846.h>
#endif
#if defined(CONFIG_BATT_TLV0832)
#include "tlv0832.h"
#endif

#define BAT_STAT_PRESENT 0x01
#define BAT_STAT_FULL   0x02
#define BAT_STAT_LOW   0x04
#define BAT_STAT_DESTROY 0x08
#define BAT_STAT_AC   0x10
#define BAT_STAT_CHARGING 0x20
#define BAT_STAT_DISCHARGING 0x40

#define BAT_ERR_INFOFAIL 0x02
#define BAT_ERR_OVERVOLTAGE 0x04
#define BAT_ERR_OVERTEMP 0x05
#define BAT_ERR_GAUGESTOP 0x06
#define BAT_ERR_OUT_OF_CONTROL 0x07
#define BAT_ERR_ID_FAIL   0x09
#define BAT_ERR_ACR_FAIL 0x10

#define BAT_ADDR_MFR_TYPE 0x5F
#define IMAP_ADAPTER 615
#define IMAP_BAT_FULL 1500
#define IMAP_BAT_CRITICAL 1000
#define BATT_INITIAL 0
#define BATT_ON 1
#define BATT_AGAIN 2

//#define CONFIG_BATT_DEBUG
/* config the level of charg and uncharg base on the data of battery test*/
#ifndef CONFIG_BATT_REGION_SET
#define CONFIG_BATT_REGION_SET
#define CONFIG_FULL_CHARGE_TIME 20160
//#define CONFIG_BATT_CHG_LEV0 752
//#define CONFIG_BATT_CHG_LEV10 788
//#define CONFIG_BATT_CHG_LEV20 798
//#define CONFIG_BATT_CHG_LEV30 802
//#define CONFIG_BATT_CHG_LEV40 806
//#define CONFIG_BATT_CHG_LEV50 815
//#define CONFIG_BATT_CHG_LEV60 826
//#define CONFIG_BATT_CHG_LEV70 839
//#define CONFIG_BATT_CHG_LEV80 853
//#define CONFIG_BATT_CHG_LEV90 855
//#define CONFIG_BATT_CHG_LEV100 856
#define CONFIG_FULL_UNCHARGE_TIME 22370
//#define CONFIG_BATT_LEV0    702
//#define CONFIG_BATT_LEV10    733
//#define CONFIG_BATT_LEV20    740
//#define CONFIG_BATT_LEV30    747
//#define CONFIG_BATT_LEV40    751
//#define CONFIG_BATT_LEV50    757
//#define CONFIG_BATT_LEV60    767
//#define CONFIG_BATT_LEV70    782
//#define CONFIG_BATT_LEV80    795 
//#define CONFIG_BATT_LEV90    812
//#define CONFIG_BATT_LEV100   834
#endif

#define BATT_VALUE_SAMPLE_TIME 2000
#ifdef CONFIG_CHARGER_BQ
extern atomic_t chargingfull ;
#endif
extern int read_rtc_gpx(int io_num);

#ifdef CONFIG_FULL_CHARGE_TIME
int charging_count = CONFIG_FULL_CHARGE_TIME / 100 * 1000 / BATT_VALUE_SAMPLE_TIME;
#else
int charging_count = 101;
#endif
#ifdef CONFIG_FULL_UNCHARGE_TIME
int uncharging_count = CONFIG_FULL_UNCHARGE_TIME / 100 * 1000 / BATT_VALUE_SAMPLE_TIME;
#else
int uncharging_count = 112;
#endif

enum imap_charge_reader_state {
	IMAP_CHARGE_READER_INIT,
	IMAP_CHARGE_READER_RUN,
	IMAP_CHARGE_READER_RESUME,
	IMAP_CHARGE_READER_SUSPEND,
};

struct imap_batt_data_list {
	unsigned int	capacity;
	unsigned int	charging_val;
	unsigned int	uncharging_val;
};

struct im_batt_val {
	unsigned int capacity;
	unsigned int adc_val;
};

struct imap_charge_reader_info {
	struct im_batt_val	show_val; // values showing on the pad 
	struct im_batt_val	batt_val; // real_time values	
	enum imap_charge_reader_state	flag;  // indicate if the battery value is stable that we can start
	int		show_counter; // counter for show value to change 
	int		diff_lev; // the difference level between show_val and batt_val
	unsigned long	batt_state; // CHARGE_FULL, CHARGE, UNCHARGE
	unsigned long	charging_state;
};

static struct imap_batt_data_list imap_batt_datas[] = {
			{0,  752, 702},
			{10, 788, 733},
			{20, 798, 740},
			{30, 802, 747},
			{40, 806, 751},
			{50, 815, 757},
			{60, 826, 767},
			{70, 839, 782},
			{80, 853, 795},
			{90, 855, 812},
			{100,856, 834},
		};

void item_config_batt (void)
{
	if(item_exist("batt.chg")){
		int i=0;
		for (;i < 11;i++)
		{
			imap_batt_datas[i].charging_val = item_integer("batt.chg",i);
			imap_batt_datas[i].uncharging_val = item_integer("batt.dischg",i);
		}

	}
}

/*
#ifdef CONFIG_BATT_REGION_SET
struct imap_batt_data_list imap_batt_datas[] = {	// battery datas must be alligned in increase order of capacitty values, 
	{0,  CONFIG_BATT_CHG_LEV0,   CONFIG_BATT_LEV0  },			// and the first must be zero, the last must be 100;
	{10, CONFIG_BATT_CHG_LEV10,  CONFIG_BATT_LEV10 },
	{20, CONFIG_BATT_CHG_LEV20,  CONFIG_BATT_LEV20 },
	{30, CONFIG_BATT_CHG_LEV30,  CONFIG_BATT_LEV30 },
	{40, CONFIG_BATT_CHG_LEV40,  CONFIG_BATT_LEV40 },
	{50, CONFIG_BATT_CHG_LEV50,  CONFIG_BATT_LEV50 },
	{60, CONFIG_BATT_CHG_LEV60,  CONFIG_BATT_LEV60 },
	{70, CONFIG_BATT_CHG_LEV70,  CONFIG_BATT_LEV70 },
	{80, CONFIG_BATT_CHG_LEV80,  CONFIG_BATT_LEV80 },
	{90, CONFIG_BATT_CHG_LEV90,  CONFIG_BATT_LEV90 },
	{100,CONFIG_BATT_CHG_LEV100, CONFIG_BATT_LEV100},
};
#else
struct imap_batt_data_list imap_batt_datas[] = {	// battery datas must be alligned in increase order of capacitty values, 
	{0,  752, 702},			// and the first must be zero, the last must be 100;
	{10, 788, 733},
	{20, 798, 740},
	{30, 802, 747},
	{40, 806, 751},
	{50, 815, 757},
	{60, 826, 767},
	{70, 839, 782},
	{80, 853, 795},
	{90, 855, 812},
	{100,856, 834},
};
#endif
*/

static struct imap_charge_reader_info im_batt;
static int count = 0;
static int bat_level = 200;
//static int sum_val= 0;
static bool big_counter_flag = 0;
static struct timer_list supply_timer;
volatile static int adaptor_disconnect_num=0;
volatile static int charging_status = 0;

extern int rd_get_time(void);
static int resume_time;
static int suspend_time;

unsigned int chg_ful;
unsigned int adp_in;

int get_adc_default_val(void)
{
	return 0;
}
int (*get_adc_val)(void) = get_adc_default_val;

void __imapx_register_batt(int (*func)(void))
{
	 get_adc_val = func;
}

EXPORT_SYMBOL(__imapx_register_batt);

int battery_level(int level)
{
    printk("enter func %s bat_level is %d \n", __func__, level);
    bat_level = level;
    return 0;
}
EXPORT_SYMBOL(battery_level);

int isacon(void)
{
	unsigned long tmp;
    tmp = read_rtc_gpx(2);
    
//  	printk("isacon is %x\n",tmp);
	return !!tmp; 
}
EXPORT_SYMBOL(isacon); //for TP: zet6221  suixing 2012-09-07

static int ischargingfull(void)
{

	unsigned long tmp;
    int index;
	int abs_index;
#ifdef CONFIG_CHARGER_BQ
    return atomic_read(&chargingfull);
#else
    if(isacon())
    {
        if(item_exist("chargerfull.detect")){
            index = item_integer("chargerfull.detect", 1);
		abs_index = (index>0)? index : (-1*index);
            //imapx_pad_set_mode(1,1,abs_index);
            //imapx_pad_set_dir(1,1,abs_index);
            return (index>0)? imapx_pad_get_indat(abs_index) : (!imapx_pad_get_indat(abs_index));
        }else{
            tmp = (read_rtc_gpx(1));
            tmp = !tmp;
        }
    }
    else 
        tmp = 0;
    return tmp;
#endif

}

static int capacity2batt(int capacity_value)
{
	unsigned int batt_value = 0;
	int i;
	int n = sizeof(imap_batt_datas) / sizeof(unsigned int) / 3;
	for (i=1; i<n; i++)
	{
		if (capacity_value <= imap_batt_datas[i].capacity)
		{
			break;
		}
	}
	if (isacon())
	{
		batt_value = (capacity_value - imap_batt_datas[i-1].capacity)
		       	* (imap_batt_datas[i].charging_val - imap_batt_datas[i-1].charging_val)
			/ (imap_batt_datas[i].capacity - imap_batt_datas[i-1].capacity)
			+  imap_batt_datas[i-1].charging_val;
	}
	else
	{
		batt_value = (capacity_value - imap_batt_datas[i-1].capacity)
		       	* (imap_batt_datas[i].uncharging_val - imap_batt_datas[i-1].uncharging_val)
			/ (imap_batt_datas[i].capacity - imap_batt_datas[i-1].capacity)
			+  imap_batt_datas[i-1].uncharging_val;
	}

	return batt_value;
}

void init_batt_info(void)
{
	im_batt.flag	 = IMAP_CHARGE_READER_INIT;
	im_batt.batt_state	 = POWER_SUPPLY_STATUS_UNKNOWN;
	im_batt.diff_lev = 0;
	im_batt.charging_state = POWER_SUPPLY_STATUS_UNKNOWN;
	im_batt.show_val.adc_val = 800;
	im_batt.show_val.capacity = 50;
}
static int get_show_counter(int batt_value)
{
    int show_counter,i;
	int n = sizeof(imap_batt_datas) / sizeof(unsigned int) / 3;
    if(isacon())
    {
        for(i=1; i<n;i++)
        {
            if(batt_value <=imap_batt_datas[i].charging_val)
            {
                show_counter =charging_count * (imap_batt_datas[10].charging_val-imap_batt_datas[0].charging_val) / 
                    ((imap_batt_datas[i].charging_val-imap_batt_datas[i-1].charging_val) * 10);
//printk("show_counter:%d,batt_value:%d\n",show_counter,batt_value);
                return show_counter;
            }
        }
        show_counter =charging_count ;
    }
    else 
    {
        for(i=1; i<n;i++)
        {
            if(batt_value <=imap_batt_datas[i].uncharging_val)
            {
                show_counter = uncharging_count * (imap_batt_datas[10].uncharging_val - imap_batt_datas[0].uncharging_val) /
                    ((imap_batt_datas[i].uncharging_val-imap_batt_datas[i-1].uncharging_val) * 10);
                //printk("show_counter:%d,batt_value:%d\n",show_counter,batt_value);
                return show_counter;
            }
        }
                show_counter =uncharging_count ;
    }
//printk("show_counter:%d,batt_value:%d\n",show_counter,batt_value);
    return show_counter;
}

static int batt2capacity(int batt_value)
{
	unsigned int capacity_value = 0;
	int i;
	int n = sizeof(imap_batt_datas) / sizeof(unsigned int) / 3;

	if (ischargingfull())
		capacity_value = 100;
	else if (isacon())
	{
		if (batt_value <= imap_batt_datas[0].charging_val)
			capacity_value = 0;
		else
		{
			for (i=1; i<n; i++)
			{
				//prink("now batt_value is:%d");
				if (batt_value <= imap_batt_datas[i].charging_val)
				{
					capacity_value = 
						( batt_value - imap_batt_datas[i-1].charging_val )
						* ( imap_batt_datas[i].capacity - imap_batt_datas[i-1].capacity )
						 /( imap_batt_datas[i].charging_val - imap_batt_datas[i-1].charging_val)
						 + imap_batt_datas[i-1].capacity;
					break;
				}
			}
			if (batt_value > imap_batt_datas[n-1].charging_val)
				capacity_value = 100;
		}
	//	printk("now is charging, batt_value: %d, imap_batt_datas: %d,%d,%d, capacity_value: %d\n",
	//		       batt_value, imap_batt_datas[i].capacity,imap_batt_datas[i].charging_val,imap_batt_datas[i].uncharging_val,capacity_value);	
	}
	else
	{
		if (batt_value <= imap_batt_datas[0].uncharging_val)
			capacity_value = 0;
		else
		{
			for (i=1; i<n; i++)
			{
				if (batt_value <= imap_batt_datas[i].uncharging_val)
				{
					capacity_value = 
						( batt_value - imap_batt_datas[i-1].uncharging_val )
						* ( imap_batt_datas[i].capacity - imap_batt_datas[i-1].capacity )
						 /( imap_batt_datas[i].uncharging_val - imap_batt_datas[i-1].uncharging_val)
						 + imap_batt_datas[i-1].capacity;
					break;
				}
			}
			if (batt_value > imap_batt_datas[n-1].uncharging_val)
				capacity_value = 100;
		}
	//	printk("now is uncharging, batt_value: %d, imap_batt_datas: %d,%d,%d, capacity_value: %d\n",
	//		       batt_value, imap_batt_datas[i].capacity,imap_batt_datas[i].charging_val,imap_batt_datas[i].uncharging_val,capacity_value);	
	}
//capacity_value=100;
	return capacity_value;
}

static int android_ac_get_prop(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{

	switch (psp)
	{
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = im_batt.batt_state;
			if(val->intval > POWER_SUPPLY_STATUS_NOT_CHARGING)
			  val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		default:
			break;
	}
	return 0;
}

static enum power_supply_property android_ac_props[] =
{
	POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply android_ac =
{
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = android_ac_props,
	.num_properties = ARRAY_SIZE(android_ac_props),
	.get_property = android_ac_get_prop,
};

static int android_bat_get_status(union power_supply_propval *val)
{
	val->intval = im_batt.batt_state;
	return 0;
}

static int android_bat_get_capacity(union power_supply_propval *val)
{
//#ifdef CONFIG_BATT_DEBUG
	if(item_equal("batt.debug", "1", 0))
	{
		/* do not report zero to prevent soft shutdown */
		val->intval = (im_batt.show_val.capacity? im_batt.show_val.capacity: 1);
	}else{
		val->intval = im_batt.show_val.capacity;
	}
//#else
//	val->intval = im_batt.show_val.capacity;
//#endif

	return 0;
}


static int android_bat_get_health(union power_supply_propval *val)
{

	val->intval = POWER_SUPPLY_HEALTH_GOOD;
	return 0;
}

static int android_bat_get_mfr(union power_supply_propval *val)
{

	val->strval = "Rockie";
	return 0;
}

static int android_bat_get_tech(union power_supply_propval *val)
{
	val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
	return 0;
}

static int android_bat_get_property(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{
	int ret = 0;

	switch (psp)
	{
		case POWER_SUPPLY_PROP_STATUS:

			ret = android_bat_get_status(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = BAT_STAT_PRESENT;
			break;

		case POWER_SUPPLY_PROP_HEALTH:
			ret = android_bat_get_health(val);
			break;

        case POWER_SUPPLY_PROP_ONLINE:
			val->intval = im_batt.batt_state;
            break;

		case POWER_SUPPLY_PROP_TECHNOLOGY:
			ret = android_bat_get_tech(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			//val->intval = 100;
			android_bat_get_capacity(val);
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

static enum power_supply_property android_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
};

/*********************************************************************
 * *   Initialisation
 * *********************************************************************/

//static struct platform_device *bat_pdev;

static struct power_supply android_bat =
{
	.properties = android_bat_props,
	.num_properties = ARRAY_SIZE(android_bat_props),
	.get_property = android_bat_get_property,
	.use_for_apm = 1,
};

void adjust_show_counter_and_value(int batt_value)
{
    int show_counter = im_batt.show_counter;
    int get_counter = get_show_counter(batt_value);
	if (!isacon())
	{
		im_batt.diff_lev = -im_batt.diff_lev;
        //sum_val = -sum_val;
	}
    if(big_counter_flag == 1)
        im_batt.show_counter = im_batt.show_counter/2;	
	if (im_batt.diff_lev > 2*im_batt.show_counter/3)
	{
		im_batt.show_counter = show_counter/3;
	}
	else if (im_batt.diff_lev > im_batt.show_counter/2)
	{
		im_batt.show_counter = show_counter/2;
	}
	else if (im_batt.diff_lev < -2*im_batt.show_counter/3)
	{
		im_batt.show_counter = show_counter*3;
	}
	else if (im_batt.diff_lev < -1*im_batt.show_counter/2)
	{
		im_batt.show_counter = show_counter*2;
	}
	if (im_batt.show_counter > 2*get_counter)
		im_batt.show_counter = 2*get_counter;
	if (im_batt.show_counter < 10)
		im_batt.show_counter = 10;

    if (isacon())
    {
        if(im_batt.show_val.capacity==99&&im_batt.batt_val.adc_val>im_batt.show_val.adc_val&&im_batt.batt_val.adc_val >= imap_batt_datas[10].charging_val)
        {
            im_batt.show_val.capacity = 100;
        }
        else{
            im_batt.show_val.capacity = (im_batt.show_val.capacity==100) ? 100 : 
                ((!ischargingfull()) ? (im_batt.show_val.capacity==99) ? 99 : im_batt.show_val.capacity+1
                 : im_batt.show_val.capacity+1);
        }
        if(im_batt.show_counter > charging_count)
            big_counter_flag = 1;
        else
            big_counter_flag = 0;
    }
	else
    {
        if(im_batt.show_val.capacity > 1||im_batt.batt_val.adc_val <= im_batt.show_val.adc_val)
        {
            im_batt.show_val.capacity = (im_batt.show_val.capacity==0) ? 0 : im_batt.show_val.capacity-1;
        }
        if(im_batt.show_counter > uncharging_count)
            big_counter_flag = 1;
        else
            big_counter_flag = 0;
    }

	im_batt.show_val.adc_val = capacity2batt(im_batt.show_val.capacity);
}

static void supply_timer_func(unsigned long unused)
{
    int bat_val_corr;
	int adp_flag = isacon();
	int chgful_flag = ischargingfull();
	unsigned long charging_state = adp_flag ? POWER_SUPPLY_STATUS_CHARGING : POWER_SUPPLY_STATUS_NOT_CHARGING;
	unsigned long battery_state;
	int capacity_show = im_batt.show_val.capacity;
	int s2r_time;
	int up_limit;
	int down_limit;
	im_batt.batt_val.adc_val = get_adc_val();
	int capacity_val = batt2capacity(im_batt.batt_val.adc_val);

//#ifdef CONFIG_BATT_DEBUG
//	printk(KERN_ERR "[%d] [%d] [%d] => real:[%3d%%],show:[%d][%3d%%]\n",
//			adp_flag, im_batt.batt_val.adc_val, chgful_flag, capacity_val,im_batt.show_val.adc_val,im_batt.show_val.capacity);
	if(item_equal("batt.debug", "1", 0))
	{
	printk(KERN_ERR "[%d] [%d] [%d] => show:[%d][%3d%%]\n",
			adp_flag, im_batt.batt_val.adc_val, chgful_flag, im_batt.show_val.adc_val,im_batt.show_val.capacity);
	}
//#endif

	switch (im_batt.flag)
	{
		case IMAP_CHARGE_READER_INIT:
            if (item_equal("batt.corr.boot", "1", 0))
            {
                //printk("enter func %s at line %d \n", __func__, __LINE__);
                if (count >= 8)
                {
                    //printk("enter func %s at line %d \n", __func__, __LINE__);
                    im_batt.batt_val.capacity = batt2capacity(im_batt.batt_val.adc_val);
                    im_batt.show_val.capacity = im_batt.batt_val.capacity;
                    im_batt.show_val.adc_val = im_batt.batt_val.adc_val;

                    if(bat_level != 200)
                    {
                        printk("enter func %s im_batt.batt_val.adc_val is %d, capacity2batt(bat_level) is %d\n", __func__, im_batt.batt_val.adc_val, capacity2batt(bat_level));
                        if (!(capacity2batt(bat_level)-20 > im_batt.batt_val.adc_val || capacity2batt(bat_level)+20 < im_batt.batt_val.adc_val))
                        {
                            printk("enter func %s, bat_level is %d\n", __func__, bat_level);
                            im_batt.batt_val.adc_val = capacity2batt(bat_level);
                            im_batt.batt_val.capacity = bat_level;
                            im_batt.show_val.capacity = im_batt.batt_val.capacity;
                            im_batt.show_val.adc_val = im_batt.batt_val.adc_val;
                        }
                    }
                    count = 0;
                    im_batt.flag = IMAP_CHARGE_READER_RUN;
                    power_supply_changed(&android_bat);
                }
                else
                    count++;
                break;
            }
            else 
            {
                // wait for battery stable
                if (count ==5)
                {
                    count++;
                    if(!adp_flag && item_exist("batt.corr.val")==1)
                        bat_val_corr = item_integer("batt.corr.val", 0);
                    else
                        bat_val_corr = 0;
                    im_batt.batt_val.adc_val += bat_val_corr;
                    im_batt.batt_val.capacity = batt2capacity(im_batt.batt_val.adc_val);
                    im_batt.show_val.capacity = im_batt.batt_val.capacity;
                    im_batt.show_val.adc_val = im_batt.batt_val.adc_val;
                    power_supply_changed(&android_bat);
                }    
                else if (count > 15)
                {
                    count = 0;
                    im_batt.flag = IMAP_CHARGE_READER_RUN;
                    power_supply_changed(&android_bat);
                }
                else
                    count++;
                break;
            }

		case IMAP_CHARGE_READER_RUN:
			if (im_batt.charging_state == charging_state)
			{
				battery_state = (adp_flag?(chgful_flag?                                           
							POWER_SUPPLY_STATUS_FULL: POWER_SUPPLY_STATUS_CHARGING)
						: ((im_batt.show_val.capacity == 100)? POWER_SUPPLY_STATUS_FULL:           
							POWER_SUPPLY_STATUS_NOT_CHARGING));                    

				if(battery_state != im_batt.batt_state)
				{
					count = 0;
					im_batt.diff_lev = 0;
					im_batt.show_val.adc_val = capacity2batt(im_batt.show_val.capacity);
				}


				if (count < im_batt.show_counter)
				{
                    //printk("im_batt.show_counter:%d,count:%d,im_batt.diff_lev:%d\n",im_batt.show_counter,count,im_batt.diff_lev);
					if(chgful_flag)
					{
						count = im_batt.show_counter-1;
						im_batt.diff_lev = im_batt.show_counter;
					}
					else 
					{
                        if(big_counter_flag == 0)
                        {
                        //sum_val += im_batt.batt_val.adc_val;
						if (im_batt.batt_val.adc_val > im_batt.show_val.adc_val)
							im_batt.diff_lev++;
						else if (im_batt.batt_val.adc_val < im_batt.show_val.adc_val)
							im_batt.diff_lev--;
                        }
                        else if(count > im_batt.show_counter/2)
                        {
                            if (im_batt.batt_val.adc_val > im_batt.show_val.adc_val)
                                    im_batt.diff_lev++;
                            else if (im_batt.batt_val.adc_val < im_batt.show_val.adc_val)
                                    im_batt.diff_lev--;

                        }
					}

					count++;
				}
				else
				{
//                    sum_val /= count;
 //                   sum_val  = sum_val - im_batt.show_val.adc_val;
                    //printk("im_batt.show_counter:%d,count:%d,im_batt.diff_lev:%d, average_val:%d\n",im_batt.show_counter,count,im_batt.diff_lev,sum_val);
					adjust_show_counter_and_value(im_batt.batt_val.adc_val);
					im_batt.diff_lev = 0;
                    //sum_val = 0;
					count = 0;
				}

				if((im_batt.batt_state != battery_state) ||
						(im_batt.show_val.capacity != capacity_show)) {
					im_batt.batt_state = battery_state;
					power_supply_changed(&android_bat);

				}
				im_batt.charging_state = charging_state;
			}
			else
			{
			//	im_batt.show_counter = adp_flag ? charging_count : uncharging_count;
                im_batt.show_counter = get_show_counter(im_batt.batt_val.adc_val);
				count = 0;
				im_batt.diff_lev = 0;
				im_batt.show_val.adc_val = capacity2batt(im_batt.show_val.capacity);
				im_batt.charging_state = charging_state;
				if(POWER_SUPPLY_STATUS_NOT_CHARGING==charging_state)
				{
					printk(KERN_DEBUG"-notify discharge mode\n");
					power_supply_changed(&android_bat);//notify charge mode change.
					
				}
			}
			break;
		case IMAP_CHARGE_READER_RESUME:

			if (suspend_time>resume_time)
			{// something must be wrong
				printk(KERN_DEBUG"get suspend/resume time error(%d > %d)\n",suspend_time, resume_time);
				init_batt_info();
			}
			else if(count > 3)
			{
                count = 0;
				s2r_time = resume_time - suspend_time;
				up_limit = s2r_time / 64;
				down_limit = s2r_time / 256;

				printk(KERN_DEBUG "r-s_time=%d-%d=%d, limit: %d ~ %d, record:%d, now_read:%d\n",
						resume_time,
						suspend_time,
						s2r_time,
						down_limit,
						up_limit,
						im_batt.show_val.capacity,
						capacity_val);
				if (im_batt.show_val.capacity + up_limit < capacity_val)
				{
					im_batt.show_val.capacity = im_batt.show_val.capacity + up_limit;
					im_batt.show_val.capacity = (im_batt.show_val.capacity<100)?im_batt.show_val.capacity:(chgful_flag)?100:99;
				}
				else if (im_batt.show_val.capacity > capacity_val + down_limit)
				{
					im_batt.show_val.capacity = im_batt.show_val.capacity - down_limit;
					im_batt.show_val.capacity = (im_batt.show_val.capacity<0)?0:im_batt.show_val.capacity;
				}
				else
				{
					im_batt.show_val.capacity = capacity_val;
				}

				printk(KERN_DEBUG "now should show:%d\n", im_batt.show_val.capacity);
				im_batt.show_val.adc_val = capacity2batt(im_batt.show_val.capacity);
				im_batt.flag = IMAP_CHARGE_READER_RUN;
				//im_batt.show_counter = (adp_flag) ? charging_count : uncharging_count;
                im_batt.show_counter = get_show_counter(im_batt.batt_val.adc_val);
				im_batt.batt_state = (adp_flag?(chgful_flag?                                           
							POWER_SUPPLY_STATUS_FULL: POWER_SUPPLY_STATUS_CHARGING)
						: ((im_batt.show_val.capacity == 100)? POWER_SUPPLY_STATUS_FULL:           
							POWER_SUPPLY_STATUS_NOT_CHARGING));                    
				im_batt.charging_state = charging_state;
				im_batt.diff_lev = 0;
				count = 0;
				power_supply_changed(&android_bat);
			}
            else
                count ++;
			break;
		case IMAP_CHARGE_READER_SUSPEND:
		default:
			break;
	}

	//	time_work:
	mod_timer(&supply_timer,\
			jiffies + msecs_to_jiffies(BATT_VALUE_SAMPLE_TIME));
}

void android_bat_sta_pin_cfg(void)
{
	int index;
	int abs_index;

	if(item_exist("chargerfull.detect")){
		index = item_integer("chargerfull.detect", 1);
		abs_index = (index>0)? index : (-1*index);
		imapx_pad_set_mode(1,1,abs_index);
            	imapx_pad_set_dir(1,1,abs_index);
		imapx_pad_set_pull(abs_index, 0, 1);
	}
}

static int __devinit android_bat_probe(struct platform_device *pdev)
{


	int ret = 0;
/*	
	int i;
	for(i=0;i<11;i++)
	{
	printk(KERN_EMERG"batt_val:%d....chg_val:%d\n",
			imap_batt_datas[i].charging_val, imap_batt_datas[i].uncharging_val);
	}
*/
	/**********************************************/
	//here, we need to know the power type, AC or Battery and detect the voltage each 5 second.
	//need to register a irq for pluging in or out the AC.
	//at last, need to monitor the voltage each 5 second.
	//now, we have a question, cannot display the AC icon.
	/***************************************************/
/*******************************************************/
#if 0
	chg_ful = __imapx_name_to_gpio(CONFIG_BATT_CHARGE_FULL);
	if(chg_ful == IMAPX_GPIO_ERROR) {
		printk(KERN_ERR "failed to get chg_ful pin.\n");
		return -1;
	}
	adp_in = __imapx_name_to_gpio(CONFIG_BATT_ADAPTER_IN);
	if(adp_in == IMAPX_GPIO_ERROR) {
		printk(KERN_ERR "failed to get adp_in pin.\n");
		return -1;
	}
	imapx_gpio_setcfg(chg_ful, IG_INPUT, IG_NORMAL);
	imapx_gpio_pull(chg_ful,1,IG_NORMAL);
	imapx_gpio_setcfg(adp_in, IG_INPUT, IG_NORMAL);
	imapx_gpio_pull(adp_in,1,IG_NORMAL);
#endif
	android_bat_sta_pin_cfg();

    /* adapter_detection */
	printk("***************entering %s\n",__func__);
	item_config_batt();

	init_batt_info();
	setup_timer(&supply_timer, supply_timer_func, 0);
	mod_timer(&supply_timer,\
			jiffies + msecs_to_jiffies(3000));
	/*******************************/
	im_batt.batt_state = (isacon()?(ischargingfull()?                                           
				POWER_SUPPLY_STATUS_FULL: POWER_SUPPLY_STATUS_CHARGING)
			: ((0)? POWER_SUPPLY_STATUS_FULL:           
				POWER_SUPPLY_STATUS_NOT_CHARGING));                    

	ret = power_supply_register(&pdev->dev, &android_ac);
	if (ret)
		goto ac_failed;

	android_bat.name = pdev->name;
	ret = power_supply_register(&pdev->dev, &android_bat);
	if (ret)
		goto battery_failed;

	//	else 
	//	{batt_init_ok = 1;
	goto success;
	//	}

	power_supply_unregister(&android_bat);
battery_failed:
	power_supply_unregister(&android_ac);
ac_failed:
	platform_device_unregister(pdev);
success:
	return ret;
}

static int __devexit android_bat_remove(struct platform_device *pdev)
{
	power_supply_unregister(&android_bat);
	power_supply_unregister(&android_ac);
	platform_device_unregister(pdev);
    return 0;
}

#ifdef CONFIG_PM
static int
android_bat_suspend(struct platform_device *pdev, pm_message_t state) 
{
	int ret;
	printk(KERN_INFO"Enter %s\n",__func__);

	ret = del_timer(&supply_timer);
	while (!ret) {
		printk(KERN_ERR"timer is inactive..\n");
		msleep(1);
		ret = del_timer(&supply_timer);
	}

	im_batt.flag = IMAP_CHARGE_READER_SUSPEND;
	suspend_time = rd_get_time();
	count = 0;
	return 0;
}

static int
android_bat_resume(struct platform_device *pdev) 
{
	printk(KERN_INFO"Enter %s\n",__func__);
	android_bat_sta_pin_cfg();
	im_batt.flag = IMAP_CHARGE_READER_RESUME;
	count = 0;
	resume_time = rd_get_time();
	mod_timer(&supply_timer,\
			jiffies + msecs_to_jiffies(1000));        
	return 0;

}
#else
#define android_bat_suspend NULL
#define android_bat_resume  NULL
#endif

static struct platform_driver android_bat_driver = {
	.probe          = android_bat_probe,
	.remove         = __devexit_p(android_bat_remove),
	.suspend        = android_bat_suspend,
	.resume         = android_bat_resume,
	.driver = {
		.name    = "battery",
		.owner   = THIS_MODULE,
	}
};

static int __init android_bat_init(void)
{
	return platform_driver_register(&android_bat_driver);
}

static void __exit android_bat_exit(void)
{
	platform_driver_unregister(&android_bat_driver);
}
module_init(android_bat_init);
module_exit(android_bat_exit);

MODULE_AUTHOR("Rockie Cheng <aokikyon@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fake Battery driver for android");


