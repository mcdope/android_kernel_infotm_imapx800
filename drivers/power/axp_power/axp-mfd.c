/*
 * Base driver for X-Powers AXP
 *
 * Copyright (C) 2011 X-Powers, Ltd.
 *  Zhang Donglu <zhangdonglu@x-powers.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <mach/pad.h>
#include <mach/items.h>
#include <mach/irqs.h>
#include <mach/imap-iomap.h>
#include <asm/io.h>

#include "axp-cfg.h"
#include "axp-rw.h"

//#define AXP_DBG 
#ifdef  AXP_DBG
#define AXP_MSG(format,args...)   printk("[AXP202]"format,##args)
#else
#define AXP_MSG(format,args...)   do {} while (0)
#endif

struct axp_mfd_chip *pchip;
 
extern void __imapx_register_pcbtest_batt_v(int (*func1)(void), int (*func2)(void));
extern void __imapx_register_pcbtest_chg_state(int (*func1)(void));

void vbus_en_pull(int val)
{
    if(item_equal("charger.pattern", "mode0", 0) ||
       item_equal("charger.pattern", "mode1", 0) ||
       item_equal("charger.pattern", "mode4", 0)) {
        imapx_pad_set_mode(1, 1, 61);
        imapx_pad_set_dir(0, 1, 61);
        imapx_pad_set_outdat(!!val, 1, 61);
    }else if(item_equal("charger.pattern", "mode2", 0) ||
	     item_equal("charger.pattern", "mode5", 0)) {
        imapx_pad_set_mode(1, 1, 54);
        imapx_pad_set_dir(0, 1, 54);
        imapx_pad_set_outdat(!!val, 1, 54);
    }
}
EXPORT_SYMBOL(vbus_en_pull);

void drvbus_pull(int val)
{
    if(item_equal("charger.pattern", "mode2", 0) ||
       item_equal("charger.pattern", "mode5", 0)) {
        imapx_pad_set_mode(1, 1, 61);
        imapx_pad_set_dir(0, 1, 61);
        imapx_pad_set_outdat(!!val, 1, 61);
    }else if(item_equal("charger.pattern", "mode1", 0)) {
        imapx_pad_set_mode(1, 1, 109);
        imapx_pad_set_dir(0, 1, 109);
        imapx_pad_set_outdat(!!val, 1, 109);
    }
}
EXPORT_SYMBOL(drvbus_pull);

int axp_get_batt_voltage(struct device *dev)
{
    int vol;
    uint8_t high, low;

    axp_read(dev, 0x78, &high);
    axp_read(dev, 0x79, &low);

    vol = (high<<4)|low;
    vol = vol*11/10;
    AXP_MSG("ADC get batt voltage is %d\n", vol);

    return vol;
}
EXPORT_SYMBOL(axp_get_batt_voltage);

#define AXP_PCBTEST_ABS(X) ((X)>0?(X):-(X))
 
int axp_pcbtest_get_batt_voltage(void)
{
     int vol;
     uint8_t high, low;

     printk("axp_pcbtest_get_batt_voltage()\n");

     __axp_read(pchip->client, 0x78, &high);
     __axp_read(pchip->client, 0x79, &low);
 
     vol = (high<<4)|low;
     vol = vol *11/10;
     printk("axp202 get batt voltage is %d\n", vol);

     return vol;
}

int axp_pcbtest_get_batt_cap(void)
{
     uint8_t temp[8], value1, value2;
     int64_t rValue1, rValue2, rValue;
     int m, Cur_coulombCounter_tmp;
 
     printk("axp_pcbtest_get_batt_cap() \n");
     __axp_read(pchip->client, 0xB0, &temp[0]);
     __axp_read(pchip->client, 0xB1, &temp[1]);
     __axp_read(pchip->client, 0xB2, &temp[2]);
     __axp_read(pchip->client, 0xB3, &temp[3]);
     __axp_read(pchip->client, 0xB4, &temp[4]);
     __axp_read(pchip->client, 0xB5, &temp[5]);
     __axp_read(pchip->client, 0xB6, &temp[6]);
     __axp_read(pchip->client, 0xB7, &temp[7]);
     rValue1 = ((temp[0]<<24) + (temp[1]<<16) + (temp[2] << 8) + temp[3]);
     rValue2 = ((temp[4]<<24) + (temp[5]<<16) + (temp[6] << 8) + temp[7]);
 
     rValue = (AXP_PCBTEST_ABS(rValue1 - rValue2))*4369;

     __axp_read(pchip->client, 0x84, &m);
     m &= 0xc0;
     switch(m>>6)
     {
         case 0: m = 25;break;
         case 1: m = 50;break;
         case 2: m = 100;break;
         case 3: m = 200;break;
         default: return 0xff;break;
     }
     //m = axp_get_freq() * 480;
     do_div(rValue, m);
     if(rValue1 >= rValue2)
         Cur_coulombCounter_tmp = (int)rValue;
     else
         Cur_coulombCounter_tmp = (int)(0-rValue);
 
     Cur_coulombCounter_tmp = AXP_PCBTEST_ABS(Cur_coulombCounter_tmp);
 
     __axp_read(pchip->client, 0x05, &value1);
     __axp_read(pchip->client, 0xB9, &value2);
     if(AXP_PCBTEST_ABS((value1 & 0x7F)-(value2 & 0x7F))>= 10 || Cur_coulombCounter_tmp > 50)
         return (value2 & 0x7F);
     else
         return (value1 & 0x7F);
}

int axp_pcbtest_get_ac_st(void)
{
     int st;
     printk("axp_pcbtest_get_ac_st()\n"); 
     //axp_read(&pchip->dev, 0x00, &st);
     __axp_read(pchip->client, 0x00, &st);
     st &= 0x3f;
     if((st & 0x40) || (st & 0x80))
         return 0x01;
     else
         return 0x00;
}

static int axp_init_chip(struct axp_mfd_chip *chip)
{
	uint8_t chip_id;
	int err;
#if defined (CONFIG_AXP_DEBUG)
	uint8_t val[AXP_DATA_NUM];
	int i;
#endif
	/*read chip id*/
	err =  __axp_read(chip->client, AXP_IC_TYPE, &chip_id);
	if (err) {
	    AXP_MSG("[AXP-MFD] try to read chip id failed!\n");
		return err;
	}

	dev_info(chip->dev, "AXP (CHIP ID: 0x%02x) detected\n", chip_id);

	/* mask and clear all IRQs */
#if defined (CONFIG_KP_AXP20)
	chip->ops->disable_irqs(chip,0xffffffffff);
	chip->ops->clear_irqs(chip,0xffffffffff);
#endif

#if defined (CONFIG_KP_AXP19)
	chip->ops->disable_irqs(chip,0xffffffff);
	chip->ops->clear_irqs(chip,0xffffffff);
#endif

#if defined (CONFIG_AXP_DEBUG)
	axp_reads(&axp->dev,AXP_DATA_BUFFER0,AXP_DATA_NUM,val);
	for( i = 0; i < AXP_DATA_NUM; i++){
		AXP_MSG("REG[0x%x] = 0x%x\n",i+AXP_DATA_BUFFER0,val[i]);	
	}
#endif
	return 0;
}

static int axp_disable_irqs(struct axp_mfd_chip *chip, uint64_t irqs)
{
#if defined (CONFIG_KP_AXP20)
	uint8_t v[9];
#endif

#if defined (CONFIG_KP_AXP19)
	uint8_t v[7];
#endif
	int ret;

	chip->irqs_enabled &= ~irqs;

	v[0] = ((chip->irqs_enabled) & 0xff);
	v[1] = AXP_INTEN2;
	v[2] = ((chip->irqs_enabled) >> 8) & 0xff;
	v[3] = AXP_INTEN3;
	v[4] = ((chip->irqs_enabled) >> 16) & 0xff;
	v[5] = AXP_INTEN4;
	v[6] = ((chip->irqs_enabled) >> 24) & 0xff;
#if defined (CONFIG_KP_AXP20)
	v[7] = AXP_INTEN5;
	v[8] = ((chip->irqs_enabled) >> 32) & 0xff;	
	ret =  __axp_writes(chip->client, AXP_INTEN1, 9, v);
#endif
#if defined (CONFIG_KP_AXP19)
	ret =  __axp_writes(chip->client, AXP_INTEN1, 7, v);
#endif
	return ret;

}

static int axp_enable_irqs(struct axp_mfd_chip *chip, uint64_t irqs)
{
#if defined (CONFIG_KP_AXP20)
	uint8_t v[9];
#endif

#if defined (CONFIG_KP_AXP19)
	uint8_t v[7];
#endif
	int ret;

	chip->irqs_enabled |=  irqs;

	v[0] = ((chip->irqs_enabled) & 0xff);
	v[1] = AXP_INTEN2;
	v[2] = ((chip->irqs_enabled) >> 8) & 0xff;
	v[3] = AXP_INTEN3;
	v[4] = ((chip->irqs_enabled) >> 16) & 0xff;
	v[5] = AXP_INTEN4;
	v[6] = ((chip->irqs_enabled) >> 24) & 0xff;
#if defined (CONFIG_KP_AXP20)
	v[7] = AXP_INTEN5;
	v[8] = ((chip->irqs_enabled) >> 32) & 0xff;
	ret =  __axp_writes(chip->client, AXP_INTEN1, 9, v);
#endif
#if defined (CONFIG_KP_AXP19)
	ret =  __axp_writes(chip->client, AXP_INTEN1, 7, v);
#endif
	return ret;
}

#if defined (CONFIG_KP_AXP20)
static int axp_read_irqs(struct axp_mfd_chip *chip, uint64_t *irqs)
{
	uint8_t v[5];
	int ret;

	ret =  __axp_reads(chip->client, AXP_INTSTS1, 5, v);

	*irqs =(((uint64_t) v[4]) << 32) |(((uint64_t) v[3]) << 24) | (((uint64_t) v[2])<< 16) | (((uint64_t)v[1]) << 8) | ((uint64_t) v[0]);
	return ret;
}
#endif

#if defined (CONFIG_KP_AXP19)
static int axp_read_irqs(struct axp_mfd_chip *chip, uint64_t *irqs)
{
	uint8_t v[4];
	int ret;
	ret =  __axp_reads(chip->client, AXP_INTSTS1, 4, v);
	
	*irqs =((((uint64_t) v[3]) << 24) | (((uint64_t) v[2])<< 16) | (((uint64_t)v[1]) << 8) | ((uint64_t) v[0]));
	return ret;
}
#endif

static int axp_clear_irqs(struct axp_mfd_chip *chip, uint64_t irqs)
{
#if defined (CONFIG_KP_AXP20)
	uint8_t v[9];
#endif

#if defined (CONFIG_KP_AXP19)
	uint8_t v[7];
#endif
	int ret;
	v[0] = (irqs >>  0)& 0xFF;
	v[1] = AXP_INTSTS2;
	v[2] = (irqs >>  8) & 0xFF;
	v[3] = AXP_INTSTS3;
	v[4] = (irqs >> 16) & 0xFF;
	v[5] = AXP_INTSTS4;
	v[6] = (irqs >> 24) & 0xFF;
#if defined (CONFIG_KP_AXP20)
	v[7] = AXP_INTSTS5;
	v[8] = (irqs >> 32) & 0xFF;	
	ret =  __axp_writes(chip->client, AXP_INTSTS1, 9, v);
#endif
#if defined (CONFIG_KP_AXP19)
	ret =  __axp_writes(chip->client, AXP_INTSTS1, 7, v);
#endif
	return ret;
}

static struct axp_mfd_chip_ops axp_mfd_ops[] = {
	[0] = {
		.init_chip		= axp_init_chip,
		.enable_irqs	= axp_enable_irqs,
		.disable_irqs	= axp_disable_irqs,
		.read_irqs		= axp_read_irqs,
		.clear_irqs		= axp_clear_irqs,
	},
};
#if defined (CONFIG_KP_USEIRQ)

#define base	IO_ADDRESS(SYSMGR_RTC_BASE)

void axp_rtcgp_irq_config(int io)
{
	uint8_t val;
	//AXP_MSG("axp_rtcgp1_irq_config\n");

	/*set RTCGP1 to input mode*/
	val = readl(base + 0x4c);
	val |= 1<<io;
	writel(val, base + 0x4c);
	//AXP_MSG("input mode reg=0x%x\n", readl(base + 0x4c));

	/*enable RTCGP1 pulldown*/
	val = readl(base + 0x54);
	val |= 1<<io;
	writel(val, base + 0x54);
	//AXP_MSG("pulldown enable reg=0x%x\n", readl(base + 0x54));

	/*set intr level*/
	val = readl(base + 0x5c);
	val |= 1<<(4+io);/*low*/
	writel(val, base + 0x5c);
	//AXP_MSG("intr type reg=0x%x\n", readl(base + 0x5c));
}
EXPORT_SYMBOL(axp_rtcgp_irq_config);

void axp_rtcgp_irq_mask(int io)
{
	uint8_t val;

	val = readl(base + 0xC);
	val |= 1<<3;
	writel(val, base + 0xC);
}
EXPORT_SYMBOL(axp_rtcgp_irq_mask);

void axp_rtcgp_irq_unmask(int io)
{
	uint8_t val;

	val = readl(base + 0xC);
	val &= ~(1<<3);
	writel(val, base + 0xC);
}
EXPORT_SYMBOL(axp_rtcgp_irq_unmask);

void axp_rtcgp_irq_clr(int io)
{
	uint8_t val;

	val = readl(base + 0x8);
	if(val & (1<<(3+io))) {
		writel((1<<(3+io)), base + 0x4);
	}
}
EXPORT_SYMBOL(axp_rtcgp_irq_clr);

void axp_rtcgp_irq_enable(int io)
{
	uint8_t val;

	val = readl(base + 0x5c);
	val |= 1 << io;
	writel(val, base + 0x5c);
	//AXP_MSG("rtcgp1 irq enable: reg=0x%x\n", readl(base + 0x5c));
}
EXPORT_SYMBOL(axp_rtcgp_irq_enable);

void axp_rtcgp_irq_disable(int io)
{
	uint8_t val;

	val = readl(base + 0x5c);
	val &= ~(1<<io);
	writel(val, base + 0x5c);
	//AXP_MSG("rtcgp1 irq disable: reg=0x%x\n", readl(base + 0x5c));
}
EXPORT_SYMBOL(axp_rtcgp_irq_disable);

static void axp_mfd_irq_work(struct work_struct *work)
{
	struct axp_mfd_chip *chip =
		container_of(work, struct axp_mfd_chip, irq_work);
	uint64_t irqs = 0;
	//AXP_MSG("axp_mfd_irq_work run!\n");

	while (1) {
		if (chip->ops->read_irqs(chip, &irqs))
			break;

		irqs &= chip->irqs_enabled;
		if (irqs == 0)
			break;
		blocking_notifier_call_chain(
				&chip->notifier_list, irqs, NULL);
	}
	//enable_irq(chip->client->irq);

	//if(chip->ops->read_irqs(chip, &irqs)) {
	//	AXP_MSG("axp202 read irqs error\n");
	//	goto exit;
	//}

	//irqs &= chip->irqs_enabled;
	//if(irqs == 0) {
	//	AXP_MSG("no irq or irq not enable\n");
	//	goto exit;
	//}else {
	//	AXP_MSG("irqs == 0x%llx\n", irqs);
	//	chip->ops->clear_irqs(chip, irqs);
	//}

//exit:
	axp_rtcgp_irq_enable(1);
	axp_rtcgp_irq_unmask(1);
}

static irqreturn_t axp_mfd_irq_handler(int irq, void *data)
{
	struct axp_mfd_chip *chip = data;
	//disable_irq_nosync(irq);
	//(void)schedule_work(&chip->irq_work);

	//AXP_MSG("axp_mfd_irq_handler run! pin-statu=0x%x\n", readl(base + 0x58));
	axp_rtcgp_irq_mask(1);
	axp_rtcgp_irq_clr(1);
	axp_rtcgp_irq_disable(1);
	(void)schedule_work(&chip->irq_work);

	return IRQ_HANDLED;
}
#endif

static const struct i2c_device_id axp_mfd_id_table[] = {
	{ "axp_mfd", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, axp_mfd_id_table);

static int __remove_subdev(struct device *dev, void *unused)
{
	platform_device_unregister(to_platform_device(dev));
	return 0;
}

static int axp_mfd_remove_subdevs(struct axp_mfd_chip *chip)
{
	return device_for_each_child(chip->dev, NULL, __remove_subdev);
}

static int __devinit axp_mfd_add_subdevs(struct axp_mfd_chip *chip,
					struct axp_platform_data *pdata)
{
	struct axp_funcdev_info *regl_dev;
	struct axp_funcdev_info *sply_dev;
	struct platform_device *pdev;
	int i, ret = 0;
	/* register for regultors */
	for (i = 0; i < pdata->num_regl_devs; i++) {
		regl_dev = &pdata->regl_devs[i];
		pdev = platform_device_alloc(regl_dev->name, regl_dev->id);
		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = regl_dev->platform_data;
		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}

	/* register for power supply */
	for (i = 0; i < pdata->num_sply_devs; i++) {
		sply_dev = &pdata->sply_devs[i];
		pdev = platform_device_alloc(sply_dev->name, sply_dev->id);
		pdev->dev.parent = chip->dev;
		pdev->dev.platform_data = sply_dev->platform_data;
		ret = platform_device_add(pdev);
		if (ret)
			goto failed;
	}

	return 0;

failed:
	axp_mfd_remove_subdevs(chip);
	return ret;
}
/* 系统软件关机函数 */
static void axp_power_off(void)
{
	uint8_t val;
	/* 清库仑计 */
#if defined (CONFIG_USE_OCV)
	axp_read(&axp->dev, AXP_COULOMB_CTL, &val);
	val &= 0x3f;
	axp_write(&axp->dev, AXP_COULOMB_CTL, val);
	val |= 0x80;
	val &= 0xbf;
	axp_write(&axp->dev, AXP_COULOMB_CTL, val);
#endif

	AXP_MSG("[axp] send power-off command!\n");
	mdelay(20);
	/* 需要做插入火牛、usb关机重启进boot时加入下面一段代码，不需要就注释掉 */
//	axp_read(&axp->dev, AXP_STATUS, &val);
//	if(val & 0xF0){
//		axp_read(&axp->dev, AXP_MODE_CHGSTATUS, &val);
//		if(val & 0x20){
//			AXP_MSG("[axp] set flag!\n");
//			mdelay(20);
//			AXP_MSG("[axp] reboot!\n");
//			arch_reset(0,NULL);
//			AXP_MSG("[axp] warning!!! arch can't ,reboot, maybe some error happend!\n");
//		}
//	}
	axp_set_bits(&axp->dev, AXP_OFF_CTL, 0x80);
	mdelay(20);
	AXP_MSG("[axp] warning!!! axp can't power-off, maybe some error happend!\n");
}

static int __devinit axp_mfd_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct axp_platform_data *pdata = client->dev.platform_data;
	struct axp_mfd_chip *chip;
	int ret;
	chip = kzalloc(sizeof(struct axp_mfd_chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;
	pchip = chip;
	AXP_MSG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ axp_mfd_probe ! @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    if(item_equal("charger.pattern", "mode2", 0)) {
        imapx_pad_set_pull(54, 0, 0);
    }
		
	axp = client;

	chip->client = client;
	chip->dev = &client->dev;
	chip->ops = &axp_mfd_ops[id->driver_data];

	mutex_init(&chip->lock);

	i2c_set_clientdata(client, chip);

	ret = chip->ops->init_chip(chip);
	if (ret)
		goto out_free_chip;

#if defined (CONFIG_KP_USEIRQ)
    client->irq = GIC_RTCGP1_ID;
	axp_rtcgp_irq_config(1);
	INIT_WORK(&chip->irq_work, axp_mfd_irq_work);
	BLOCKING_INIT_NOTIFIER_HEAD(&chip->notifier_list);
	ret = request_irq(client->irq, axp_mfd_irq_handler,
						0, "axp_mfd", chip);
	if(ret) {
		dev_err(&client->dev, "failed to request irq %d\n",
				client->irq);
		goto out_free_chip;
	}

	//INIT_WORK(&chip->irq_work, axp_mfd_irq_work);
	//BLOCKING_INIT_NOTIFIER_HEAD(&chip->notifier_list);
	//ret = request_irq(client->irq, axp_mfd_irq_handler,
	//	IRQF_DISABLED, "axp_mfd", chip);
  	//if (ret) {
  	//	dev_err(&client->dev, "failed to request irq %d\n",
  	//			client->irq);
  	//	goto out_free_chip;
  	//}
#endif

#if 1
    int val;
    uint8_t tmp;

    val = axp_get_batt_voltage(&client->dev);

    if(val < 1000) {
        AXP_MSG("battery_voltage < 1000, input no limit\n");
        udelay(1000);
        axp_write(&client->dev, 0x30, 0x83);
    }else {
        if(item_equal("charger.pattern", "mode0", 0) ||
                item_equal("charger.pattern", "mode4", 0)) {
            printk("battery exist, set PMU_VBUS_EN high\n");
            vbus_en_pull(1);
        }
	if(item_equal("charger.pattern", "mode5", 0)) {
	    vbus_en_pull(0);
	}
    }

	if(item_exist("board.cpu") && item_equal("board.cpu", "i15", 0)){
		axp_write(&client->dev, 0x23, 0x16);/*1.25V*/
	}else {
		axp_write(&client->dev, 0x23, 0x12);/*1.15V*/
	}

	/*close UVP*/
	axp_read(&client->dev, 0x81, &tmp);
	tmp &= ~(0x1c);
	axp_write(&client->dev, 0x81, tmp);

    /*N_OE set to 128ms*/
    axp_clr_bits(&client->dev, 0x32, 0x3);

	/*set Voff*/
    if(item_equal("charger.pattern", "mode4", 0)) {//3.2V
        axp_read(&client->dev, 0x31, &val);
        val &= ~0x7;
        val |= 0x6;
        axp_write(&client->dev, 0x31, val);
    }else {//2.9V
        axp_read(&client->dev, 0x31, &val);
        val &= ~0x7;
        val |= 0x3;
        axp_write(&client->dev, 0x31, val);
    }

    /*set charging target voltage*/
    //axp_set_bits(&client->dev, 0x33, 0x60);
    if(item_equal("charger.pattern", "mode0", 0)) {//4.36V
        axp_read(&client->dev, 0x33, &val);
        val &= ~0x60;
        val |= 0x60;
        axp_write(&client->dev, 0x33, val);
    }else {//4.2V
        axp_read(&client->dev, 0x33, &val);
        val &= ~0x60;
        val |= 0x40;
        axp_write(&client->dev, 0x33, val);
    }

	/*disable batt monitor*/
	axp_clr_bits(&client->dev, 0x32, (1<<6));

	/*set charging current*/
    if(item_equal("charger.pattern", "mode4", 0)) {
        axp_write(&client->dev,0x33,0xc2); // set charger battery 500mA default
    }else if(item_equal("charger.pattern", "mode3", 0)){
        axp_set_bits(&client->dev, 0x30, 0x3);//vbus current no limit
	    axp_set_bits(&client->dev, 0x33, 0xf);//1800mA
    }else if(item_equal("charger.pattern", "mode5", 0)){
	axp_update(&client->dev, 0x30, 0x1, 0x3);//vbus current limit 500mA
    }

	/*set timeout*/
	axp_write(&client->dev, 0x34, 0xC3);

	/*ADC setting*/
	axp_write(&client->dev, 0x82, 0xfe);
	axp_write(&client->dev, 0x83, 0x80);
	axp_write(&client->dev, 0x84, 0xf4);

	/*OT shutdown*/
	axp_set_bits(&client->dev, 0x8f, (1<<2));

#endif

#if 1
	uint8_t value;
	axp_set_bits(&client->dev, 0x90, 0x3);
	axp_clr_bits(&client->dev, 0x90, 0x4);
#endif

	ret = axp_mfd_add_subdevs(chip, pdata);
	if (ret)
		goto out_free_irq;

	/* PM hookup */
	//if(!pm_power_off)
	//	pm_power_off = axp_power_off;

	//axp_rtcgp_irq_enable(1);
	
	AXP_MSG("axp202 register function for pcbtest!\n");

	__imapx_register_pcbtest_batt_v(axp_pcbtest_get_batt_voltage, axp_pcbtest_get_batt_cap);
        __imapx_register_pcbtest_chg_state(axp_pcbtest_get_ac_st);

	AXP_MSG("axp202 register function for pcbtest success!!!");
	return ret;

out_free_irq:
	free_irq(client->irq, chip);

out_free_chip:
	AXP_MSG("@@@@@@@@@@@@@@@@@@@@@@@@@@ error exit: out_free_chip @@@@@@@@@@@@@@@@@@@@@@@@\n");
	i2c_set_clientdata(client, NULL);
	kfree(chip);

	return ret;
}

static int __devexit axp_mfd_remove(struct i2c_client *client)
{
	struct axp_mfd_chip *chip = i2c_get_clientdata(client);

	//pm_power_off = NULL;
	axp = NULL;

	axp_mfd_remove_subdevs(chip);
	kfree(chip);
	return 0;
}
/*
static const unsigned short axp_i2c[] = {
	AXP_DEVICES_ADDR, I2C_CLIENT_END };
*/
static struct i2c_driver axp_mfd_driver = {
	//.class = I2C_CLASS_HWMON,
	.driver	= {
		.name	= "axp_mfd",
		.owner	= THIS_MODULE,
	},
	.probe		= axp_mfd_probe,
	.remove		= __devexit_p(axp_mfd_remove),
	.id_table	= axp_mfd_id_table,
	//.detect         = axp_detect,
	//.address_list   = axp_i2c,
};

static int __init axp_mfd_init(void)
{
	char buf[ITEM_MAX_LEN];

	if(item_equal("pmu.model", "axp202", 0)) {
		return i2c_add_driver(&axp_mfd_driver);
	}else {
		AXP_MSG("pmu is not axp202!\n");
	}

	return -1;
}
subsys_initcall(axp_mfd_init);

static void __exit axp_mfd_exit(void)
{
	i2c_del_driver(&axp_mfd_driver);
}
module_exit(axp_mfd_exit);

MODULE_DESCRIPTION("MFD Driver for X-Powers AXP PMIC");
MODULE_AUTHOR("Donglu Zhang, <zhangdonglu@x-powers.com>");
MODULE_LICENSE("GPL");
