/*
 * drivers/power/bq2415x_battery.c
 *
 * BQ24153 / BQ24156 battery charging driver
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Author: Texas Instruments, Inc.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c/twl.h>
#include <linux/i2c/bq2415x.h>
#include <mach/items.h>

//struct bq2415x_device_info *di_bat;
//#define DEBUG
#ifdef DEBUG
#define bq_debug(di,msg...) do\
{\
		dev_info(di->dev, msg);\
} while (0)
#else
#define bq_debug(di,msg...) do\
{\
}while(0)
#endif

int bat_check = 0;
char bat_present; 
atomic_t chargingfull ;
export_symbol(atomic_t chargingfull);

struct charge_params {
    int max_charger_currentmA;
    int max_charger_voltagemV;
    int charge_currentmA;
    int charge_voltagemV;
	unsigned long		currentmA;
	unsigned long		voltagemV;
	unsigned long		term_currentmA;
	unsigned long		enable_iterm;
	bool			enable;
};
struct charge_params bq_charge_param;

struct bq2415x_device_info {
	struct device		*dev;
	struct i2c_client	*client;
	struct charge_params	params;
	struct delayed_work	bq2415x_charger_work;
	struct notifier_block	nb;

	unsigned short		status_reg;
	unsigned short		control_reg;
	unsigned short		voltage_reg;
	unsigned short		bqchip_version;
	unsigned short		current_reg;
	unsigned short		special_charger_reg;

	unsigned int		cin_limit;
	unsigned int		currentmA;
	unsigned int		voltagemV;
	unsigned int		max_currentmA;
	unsigned int		max_voltagemV;
	unsigned int		term_currentmA;

	int 			timer_fault;

	bool			cfg_params;
	bool			enable_iterm;
	bool			active;
};





static int bq2415x_write_block(struct bq2415x_device_info *di, u8 *value,
						u8 reg, unsigned num_bytes)
{
	struct i2c_msg msg[1];
	int ret;

	*value		= reg;

	msg[0].addr	= di->client->addr;
	msg[0].flags	= 0;
	msg[0].buf	= value;
	msg[0].len	= num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		dev_err(di->dev,
			"i2c_write failed to transfer all messages\n");
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int bq2415x_read_block(struct bq2415x_device_info *di, u8 *value,
						u8 reg, unsigned num_bytes)
{
	struct i2c_msg msg[2];
	u8 buf;
	int ret;

	buf		= reg;

	msg[0].addr	= di->client->addr;
	msg[0].flags	= 0;
	msg[0].buf	= &buf;
	msg[0].len	= 1;

	msg[1].addr	= di->client->addr;
	msg[1].flags	= I2C_M_RD;
	msg[1].buf	= value;
	msg[1].len	= num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, 2);

	/* i2c_transfer returns number of messages transferred */
	if (ret != 2) {
		dev_err(di->dev,
			"i2c_write failed to transfer all messages\n");
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

static int bq2415x_write_byte(struct bq2415x_device_info *di, u8 value, u8 reg)
{
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[2] = { 0 };

	/* offset 1 contains the data */
	temp_buffer[1] = value;
	return bq2415x_write_block(di, temp_buffer, reg, 1);
}

static int bq2415x_read_byte(struct bq2415x_device_info *di, u8 *value, u8 reg)
{
	return bq2415x_read_block(di, value, reg, 1);
}
static ssize_t bq2415x_show_register(struct device *dev,
                              struct device_attribute *attr, char *buf)
{


        unsigned i, n, reg_count = 8;
        u8 read_reg[8] = {0};
        struct bq2415x_device_info *di = dev_get_drvdata(dev);

        //reg_count = sizeof(bma180_regs) / sizeof(bma180_regs[0]);

        for (i = 0, n = 0; i < reg_count; i++) {
                bq2415x_read_block(di, &read_reg[i], i, 7);
                n += scnprintf(buf + n, PAGE_SIZE - n,
                               "%x = 0x%02X\n",
                               i,
                               read_reg[i]);
        }

        return n;
}

static ssize_t bq2415x_set_register(struct device *dev,
                              struct device_attribute *attr, char *buf)
{

return 0;
}


//bq2416x
static void bq2415x_config_status_reg(struct bq2415x_device_info *di)
{
    u8 reg_ctl;
//	di->status_reg = (TIMER_RST | ENABLE_STAT_PIN); 
	bq2415x_read_byte(di, (u8 *)(&di->status_reg), REG_STATUS_CONTROL);
	di->status_reg |= TIMER_RST ;//vijay 
	bq2415x_write_byte(di, di->status_reg, REG_STATUS_CONTROL);

	bq2415x_read_byte(di, &reg_ctl, REG_CONTROL_REGISTER);
    bq_debug(di,"REG_CONTROL_REGISTER:%x\n",reg_ctl);
	reg_ctl &= ~ENABLE_STAT_PIN ; 
	bq2415x_write_byte(di, reg_ctl, REG_CONTROL_REGISTER);
	bq2415x_read_byte(di, &reg_ctl, REG_CONTROL_REGISTER);
    bq_debug(di,"REG_CONTROL_REGISTER:%x\n",reg_ctl);

//	di->voltage_reg = VOLTAGE_VALUE;// vijay added
 //       bq2415x_write_byte(di, di->voltage_reg, REG_BATTERY_VOLTAGE);

	return;
}

static int bq2415x_charger_event(struct notifier_block *nb, unsigned long event,
				void *_data)
{

	struct bq2415x_device_info *di;
	struct charge_params *data;
	u8 read_reg[7] = {0};
	int ret = 0;

	di = container_of(nb, struct bq2415x_device_info, nb);
	data = &di->params;
	di->cfg_params = 1;

	if (event & BQ2415x_CHARGER_FAULT) {
		bq2415x_read_block(di, &read_reg[0], 0, 7);
		ret = read_reg[0] & 0x3F;
		return ret;
	}

	if (data->enable == 0) {
		di->currentmA = data->currentmA;
		di->voltagemV = data->voltagemV;
		di->enable_iterm = data->enable_iterm;
	}

	if ((event & BQ2415x_START_CHARGING) && (di->active == 0)) {
		schedule_delayed_work(&di->bq2415x_charger_work,
						msecs_to_jiffies(0));
		di->active = 1;
	}

	if (event & BQ2415x_STOP_CHARGING) {
		cancel_delayed_work(&di->bq2415x_charger_work);
		di->active = 0;
	}

//	if (event & BQ2415x_RESET_TIMER) {
		/* reset 25 second timer */
//		bq2415x_config_status_reg(di);
//	}

	return ret;
}

static void bq2415x_config_control_reg(struct bq2415x_device_info *di)
{
	u8 Iin_limit;

	if (di->cin_limit <= 100)
		Iin_limit = 0;
	else if (di->cin_limit > 100 && di->cin_limit <= 500)
		Iin_limit = 1;
	else if (di->cin_limit > 500 && di->cin_limit <= 800)
		Iin_limit = 2;
	else
		Iin_limit = 3;

	bq2415x_read_byte(di, (u8 *)&di->control_reg, REG_CONTROL_REGISTER);
    bq_debug(di,"REG_CONTROL_REGISTER:%x\n",di->control_reg);
    di->control_reg &= 0x33;
	di->control_reg |= ((Iin_limit << 6)| (di->enable_iterm << 3));

	di->control_reg &= ~ENABLE_STAT_PIN ; 
    //di->control_reg &= ~(0x3<<4);//set the weak voltage threshold is 3.4
	bq2415x_write_byte(di, di->control_reg, REG_CONTROL_REGISTER);
	bq2415x_read_byte(di, (u8 *)&di->control_reg, REG_CONTROL_REGISTER);
    bq_debug(di,"REG_CONTROL_REGISTER:%x\n",di->control_reg);

	bq2415x_read_byte(di, (u8 *)&di->status_reg, REG_STATUS_CONTROL);
    bq_debug(di,"REG_STATUS_CONTROL:%x\n",di->status_reg);
	di->status_reg |= 1<<6; 
	bq2415x_write_byte(di, di->status_reg, REG_STATUS_CONTROL);
	bq2415x_read_byte(di, (u8 *)&di->status_reg, REG_STATUS_CONTROL);
    bq_debug(di,"REG_STATUS_CONTROL:%x\n",di->status_reg);
	return;
}
//bq2416x
static int bq2415x_config_voltage_reg(struct bq2415x_device_info *di)
{
	unsigned int voltagemV;
	u8 Voreg;
    int err =0;

	voltagemV = di->voltagemV;
	if (voltagemV < 3500)
    {
		voltagemV = 3500;
        err = -2;
    }
	else if (voltagemV > 4440)
    {
		voltagemV = 4440;
        err = -1;
    }

	Voreg = (voltagemV - 3500)/20;
    bq2415x_read_byte(di,(u8 *)&di->voltage_reg,REG_BATTERY_VOLTAGE);
    di->voltage_reg &= 0x03;
	di->voltage_reg |= (Voreg << 2);
//	di->voltage_reg = VOLTAGE_VALUE;// vijay added
	bq2415x_write_byte(di, di->voltage_reg, REG_BATTERY_VOLTAGE);
	return err;
}
//bq2416x
static void bq2415x_config_current_reg(struct bq2415x_device_info *di)
{
	unsigned int currentmA;
	unsigned int term_currentmA;
	u8 Vichrg;
	u8 shift = 0;
	u8 Viterm;
    int err;

	currentmA = di->currentmA;
	term_currentmA = di->term_currentmA;

	if (currentmA < 550)
    {
        err = -2;
		currentmA = 550;
        bq_debug(di,"the current is under rang\n");
    }

	if ((di->bqchip_version & (BQ24153 | BQ24158))) {
		shift = 4;
		if (currentmA > 1250)
        {
			currentmA = 1250;
            err = -1;
            bq_debug(di,"the current is over rang\n");
        }
	}
#if 0
	if ((di->bqchip_version & BQ24156)) {
		shift = BQ24156_CURRENT_SHIFT;
		if (currentmA > 1550)
			currentmA = 1550;
	}
#endif

	if (term_currentmA > 350)
		term_currentmA = 350;
    if(term_currentmA < 50)
        term_currentmA = 50;

	Vichrg = (currentmA - 550)/100;
	Viterm = term_currentmA/50;
    bq2415x_read_byte(di, (u8 *)&di->current_reg, REG_BATTERY_CURRENT);
    di->current_reg &= 0x8f;

	di->current_reg = (Vichrg << shift | Viterm);
//	 di->current_reg = CURRENT_VALUE;// vijay	
	bq2415x_write_byte(di, di->current_reg, REG_BATTERY_CURRENT);
	return;
}
#if 1
static void bq2415x_config_special_charger_reg(struct bq2415x_device_info *di)
{
	u8 Vsreg = 2;				/* 160/80 */

	di->special_charger_reg = Vsreg;
	bq2415x_write_byte(di, di->special_charger_reg,
					REG_SPECIAL_CHARGER_VOLTAGE);
	return;
}
#endif
static void bq2415x_config_safety_reg(struct bq2415x_device_info *di,
						unsigned int max_currentmA,
						unsigned int max_voltagemV)
{
	u8 Vmchrg;
	u8 Vmreg;
	u8 limit_reg;

	if (max_currentmA < 550)
		max_currentmA = 550;
	else if (max_currentmA > 1550)
		max_currentmA = 1550;


	if (max_voltagemV < 4200)
		max_voltagemV = 4200;
	else if (max_voltagemV > 4440)
		max_voltagemV = 4440;

	di->max_voltagemV = max_voltagemV;
	di->max_currentmA = max_currentmA;
//	di->voltagemV = max_voltagemV;
//	di->currentmA = max_currentmA;

	Vmchrg = (max_currentmA - 550)/100;
	Vmreg = (max_voltagemV - 4200)/20;
	limit_reg = ((Vmchrg << 0x4) | Vmreg);

	//if(bat_check){
	//	limit_reg |= ~(TS_DISABLE);
	//}
	bq2415x_write_byte(di, limit_reg, REG_SAFETY_LIMIT);
	return;
}
static void bq2415x_reset(struct bq2415x_device_info *di)
{
      u8 read_reg = 0;
     // int j;
    bq2415x_read_byte(di, &read_reg, REG_BATTERY_CURRENT);
	bq_debug(di,"******** REG_BATTERY_CURRENT register value %x\n\n",read_reg);
    read_reg |= RESET_BIT;
    bq2415x_write_byte(di, read_reg, REG_BATTERY_CURRENT);
	//udelay(1000*1000);
	//for(j=0;j<=1000;j++)
    bq2415x_read_byte(di, &read_reg, REG_BATTERY_CURRENT);
	bq_debug(di,"******** REG_BATTERY_CURRENT register value %x\n\n",read_reg);
    
}


static char check_bat_presence(struct bq2415x_device_info *di)
 {
      char bat_present;
      int ret;
//      int i,j;
      u8 read_reg = 0;

	/* Disabling the Temperature sensor bit (TS in safety timer register) */
	ret = bq2415x_read_byte(di, &read_reg, REG_SAFETY_LIMIT);
	bq_debug(di,"******** REG_SAFETY_LIMIT register value %x\n\n",read_reg);//0x40 if default
#if 0
    
	read_reg &= TS_DISABLE;

	//disable TS here	
	bq2415x_write_byte(di, read_reg, REG_SAFETY_LIMIT);

	//udelay(1000*1000);
	for(j=0;j<1000;j++)
	{
		for(i=0;i<(1000*1000);i++);
	}
	
	bq2415x_read_byte(di, &read_reg, REG_SAFETY_LIMIT);
	//bq_debug(di,"******** TS register value 2 %x\n\n",read_reg);
#endif
	/* Check the battery presence */
	ret = bq2415x_read_byte(di, &read_reg, REG_STATUS_CONTROL);
	bq_debug(di,"******** REG_STATUS_CONTROL register value %x\n\n",read_reg);
    if((read_reg & 0x7) == 0x3)
    {
        bq_debug(di,"bad adapter detection\n");
    }
	ret = bq2415x_read_byte(di, &bat_present,REG_CONTROL_REGISTER);
	bq_debug(di,"****** REG_CONTROL_REGISTER: %x\n\n", bat_present);

	if((bat_present & 0x6) != 0x4){ 

		bq_debug(di,"*** Battery  presence detected\n\n");
	    //    bq2415x_read_byte(di, &read_reg, REG_SAFETY_LIMIT);			
	//	read_reg |= ~(TS_DISABLE);
	//	ret = bq2415x_write_byte(di, read_reg, REG_SAFETY_LIMIT);
	}else{
		bq_debug(di,"Battery not detected \n");
	}	

         return bat_present;

 }
/* Disable charger termination */
int start_charger_termination(struct bq2415x_device_info *di)
{


    u8 reg=0;
	bq_debug(di,"+%s\n", __func__);
        bq2415x_read_byte(di, &reg, 0x1);
        reg |=1<<4;
        bq2415x_write_byte(di, reg, 0x1);


	return 0;

}
/* Enable charger termination */
int stop_charger_termination(struct bq2415x_device_info *di)
{

    u8 reg=0;
	bq_debug(di,"+%s\n", __func__);
        bq2415x_read_byte(di, &reg, 0x1);
        reg &= ~(1<<4);
        bq2415x_write_byte(di, reg, 0x1);
		
	return 0;
}


//bq2416x
static void
bq2415x_charger_update_status(struct bq2415x_device_info *di)
{
	u8 read_reg[7] = {0};

	bq_debug(di,"+%s\n", __func__);
	di->timer_fault = 0;


	bq2415x_read_block(di, &read_reg[0], REG_STATUS_CONTROL, 7);
    bq_debug(di,"REG_STATUS_CONTROL:%x\n",read_reg[0]);
//    bq_debug(di,"read_reg[0] & 0x30:%x\n",read_reg[0] & 0x30)
	if ((read_reg[0] & 0x30) == 0x20)
    {
		dev_err(di->dev, "CHARGE DONE\n");
        atomic_set(&chargingfull,1);
    }
    else
        atomic_set(&chargingfull,1);

	if ((read_reg[0] & 0x7) == 0x6)
		di->timer_fault = 1;

	if (read_reg[0] & 0x7) {
		di->cfg_params = 1;
		dev_err(di->dev, "CHARGER FAULT %x\n", read_reg[0]);
	}

	if ((di->timer_fault == 1) || (di->cfg_params == 1)) {
		bq2415x_write_byte(di, 0x38, REG_CONTROL_REGISTER);
//		di->voltage_reg = VOLTAGE_VALUE; //default
		bq2415x_write_byte(di, di->voltage_reg, REG_BATTERY_VOLTAGE);
       // di->current_reg = 0x01; 
		bq2415x_write_byte(di, di->current_reg, REG_BATTERY_CURRENT);
		//bq2415x_config_special_charger_reg(di);
		di->cfg_params = 0;
		di->timer_fault = 0;
	}

	/* reset 32 second timer */
	bq2415x_config_status_reg(di);

	return;
}

static void bq2415x_charger_work(struct work_struct *work)
{
	struct bq2415x_device_info *di = container_of(work,
		struct bq2415x_device_info, bq2415x_charger_work.work);

	bq_debug(di,"+%s\n", __func__);
	bq2415x_charger_update_status(di);
	schedule_delayed_work(&di->bq2415x_charger_work,
				msecs_to_jiffies(BQ2415x_WATCHDOG_TIMEOUT));
}


static ssize_t bq2415x_set_enable_itermination(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	bq_debug(di,"+%s\n", __func__);
	if ((strict_strtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->enable_iterm = val;
	bq2415x_config_control_reg(di);

	return count;
}

static ssize_t bq2415x_show_enable_itermination(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	unsigned long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	bq_debug(di,"+%s\n", __func__);
	val = di->enable_iterm;
	return sprintf(buf, "%lu\n", val);
}

static ssize_t bq2415x_set_cin_limit(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	bq_debug(di,"+%s\n", __func__);
	if ((strict_strtol(buf, 10, &val) < 0) || (val < 100)
					|| (val > di->max_currentmA))
		return -EINVAL;

	di->cin_limit = val;
	bq2415x_config_control_reg(di);

	return count;
}

static ssize_t bq2415x_show_cin_limit(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	unsigned long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	bq_debug(di,"+%s\n", __func__);
	val = di->cin_limit;
	return sprintf(buf, "%lu\n", val);
}
//bq2416x
static ssize_t bq2415x_set_regulation_voltage(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	bq_debug(di,"+%s\n", __func__);
	if ((strict_strtol(buf, 10, &val) < 0) || (val < 3500)
					|| (val > di->max_voltagemV))
		return -EINVAL;

	di->voltagemV = val;
	bq2415x_config_voltage_reg(di);

	return count;
}

static ssize_t bq2415x_show_regulation_voltage(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	unsigned long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	val = di->voltagemV;
	return sprintf(buf, "%lu\n", val);
}
//bq2416x
static ssize_t bq2415x_set_charge_current(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	if ((strict_strtol(buf, 10, &val) < 0) || (val < 550)
					|| (val > di->max_currentmA))
		return -EINVAL;

	di->currentmA = val;
	bq2415x_config_current_reg(di);

	return count;
}

static ssize_t bq2415x_show_charge_current(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	unsigned long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	val = di->currentmA;
	return sprintf(buf, "%lu\n", val);
}

static ssize_t bq2415x_set_termination_current(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	if ((strict_strtol(buf, 10, &val) < 0) || (val < 0) || (val > 350))
		return -EINVAL;

	di->term_currentmA = val;
	bq2415x_config_current_reg(di);

	return count;
}

static ssize_t bq2415x_show_termination_current(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	unsigned long val;
	struct bq2415x_device_info *di = dev_get_drvdata(dev);

	val = di->term_currentmA;
	return sprintf(buf, "%lu\n", val);
}

static DEVICE_ATTR(enable_itermination, S_IWUSR | S_IRUGO,
				bq2415x_show_enable_itermination,
				bq2415x_set_enable_itermination);
static DEVICE_ATTR(cin_limit, S_IWUSR | S_IRUGO,
				bq2415x_show_cin_limit,
				bq2415x_set_cin_limit);
static DEVICE_ATTR(regulation_voltage, S_IWUSR | S_IRUGO,
				bq2415x_show_regulation_voltage,
				bq2415x_set_regulation_voltage);
static DEVICE_ATTR(charge_current, S_IWUSR | S_IRUGO,
				bq2415x_show_charge_current,
				bq2415x_set_charge_current);
static DEVICE_ATTR(termination_current, S_IWUSR | S_IRUGO,
				bq2415x_show_termination_current,
				bq2415x_set_termination_current);

static DEVICE_ATTR(show_register,S_IWUSR | S_IRUGO,
                                bq2415x_show_register,
                                bq2415x_set_register);

static struct attribute *bq2415x_attributes[] = {
	&dev_attr_enable_itermination.attr,
	&dev_attr_cin_limit.attr,
	&dev_attr_regulation_voltage.attr,
	&dev_attr_charge_current.attr,
	&dev_attr_termination_current.attr,
	&dev_attr_show_register.attr,
	NULL,
};

static const struct attribute_group bq2415x_attr_group = {
	.attrs = bq2415x_attributes,
};

static int __devinit bq2415x_charger_probe(struct i2c_client *client,
				 const struct i2c_device_id *id)
{
	struct bq2415x_device_info *di;
	int ret;
    int i;
	u8 read_reg = 0;
	printk("\n \n bq24153 driver is probed ************* \n \n");

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	di->client = client;

	i2c_set_clientdata(client, di);

	ret = bq2415x_read_byte(di, &read_reg, REG_PART_REVISION);

	if (ret < 0) {
		dev_err(&client->dev, "chip not present at address %x\n",
								client->addr);
		ret = -EINVAL;
		goto err_kfree;
        

	}
	if ((read_reg & 0x18) == 0x00 && (client->addr == 0x6a))
		di->bqchip_version = BQ24156;
	if ((read_reg & 0x18) == 0x10 && (client->addr == 0x6a))
		di->bqchip_version = BQ24158;
    bq_debug(di,"di->bqchip_version:%x\n",di->bqchip_version);

	if (di->bqchip_version == 0) {
		dev_err(&client->dev, "unknown bq chip\n");
		dev_err(&client->dev, "Chip address %x", client->addr);
		dev_err(&client->dev, "bq chip version reg value %x", read_reg);
		ret = -EINVAL;
		goto err_kfree;
	}
   bq2415x_reset(di);

	check_bat_presence(di);
    /* Initialization of charger parameters*/
    bq_charge_param.max_charger_currentmA = 1250;
    bq_charge_param.max_charger_voltagemV = 4440;
    bq_charge_param.charge_currentmA = 950;
    bq_charge_param.charge_voltagemV = 4200;
    bq_charge_param.term_currentmA= 50;
//    bq_charge_param.enable = true;
    di->params = bq_charge_param;
    di->voltagemV =4200; 
    di->currentmA = 950;
	di->cin_limit = 100;
	di->term_currentmA = bq_charge_param.term_currentmA;
	di->enable_iterm = 1;
	di->active = 0;
	di->params.enable = 1;
	di->cfg_params = 1;
    /* *****************************************/

	//di->nb.notifier_call = bq2415x_charger_event;
	bq2415x_config_safety_reg(di, bq_charge_param.max_charger_currentmA,
            bq_charge_param.max_charger_voltagemV);
    
    
	bq2415x_config_control_reg(di);
    bq2415x_config_voltage_reg(di); //bq2416x
	bq2415x_config_current_reg(di);

	INIT_DELAYED_WORK_DEFERRABLE(&di->bq2415x_charger_work,
				bq2415x_charger_work);

    for(i=0;i<7;i++)
    {
        bq2415x_read_byte(di, &read_reg, i);
        bq_debug(di,"%x:%x\n",i,read_reg);

    }
#if 1
	ret = bq2415x_read_byte(di, &read_reg, REG_SPECIAL_CHARGER_VOLTAGE);
	if (!(read_reg & 0x08)) {
		di->active = 1;
		schedule_delayed_work(&di->bq2415x_charger_work, 0);
	}
#endif
	ret = sysfs_create_group(&client->dev.kobj, &bq2415x_attr_group);
	if (ret)
		dev_err(&client->dev, "could not create sysfs files\n");

	//twl6030_register_notifier(&di->nb, 1);

	return 0;

err_kfree:
	kfree(di);

	return ret;
}

static int __devexit bq2415x_charger_remove(struct i2c_client *client)
{
	struct bq2415x_device_info *di = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &bq2415x_attr_group);
	cancel_delayed_work(&di->bq2415x_charger_work);
	flush_scheduled_work();
	//twl6030_unregister_notifier(&di->nb, 1);
	kfree(di);

	return 0;
}

static const struct i2c_device_id bq2415x_id[] = {
	{ "bq2415x_charger", 1 },
	{},
};

static struct i2c_driver bq2415x_charger_driver = {
	.probe		= bq2415x_charger_probe,
	.remove		= __devexit_p(bq2415x_charger_remove),
	.id_table	= bq2415x_id,
	.driver		= {
	.name	= "bq2415x_charger",
	},
};

static int __init bq2415x_charger_init(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret;
        atomic_set(&chargingfull,0);

	if(item_exist("charger.model") && item_equal("charger.model", "bq24158", 0))
    {
    memset(&info,0,sizeof(struct i2c_board_info));
    info.addr = 0x6a;
    strlcpy(info.type,"bq2415x_charger",I2C_NAME_SIZE);
	adapter = i2c_get_adapter(item_integer("charger.ctrl", 1));
	if (!adapter) {
		printk("*******get_adapter error!\n");
	}
    client = i2c_new_device(adapter,&info);
	return i2c_add_driver(&bq2415x_charger_driver);
    }
    else
    {
        printk(KERN_ERR "%s:charger is not bq24158 or exist",__func__);
    }
}
module_init(bq2415x_charger_init);

static void __exit bq2415x_charger_exit(void)
{
	i2c_del_driver(&bq2415x_charger_driver);
}
module_exit(bq2415x_charger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Texas Instruments Inc");
