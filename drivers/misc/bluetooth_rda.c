/***************************************************************************** 
 * general.c 
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: main file of imapx820 rda bt driver
 *
 * Author:
 *     David
 *      
 * Revision History: 
 * 1.1  12/9/2009  
 * Author:
 *     Allen
 *      
 * Revision History: 
 * 2.0  01/07/2013  
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>  
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/types.h>    
#include <linux/interrupt.h>
#include <linux/init.h>      
#include <linux/string.h>
#include <linux/mm.h>             
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
//#include <mach/imapx_gpio.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>     
#include <asm/uaccess.h>
#include<mach/items.h>
/*
 * XXX:hardware directly connect to system bus, there is 
 * no use to clock set
 */ 
//#include <plat/clock.h>


#include <mach/pad.h>
#include "bluetooth_rda.h"

#ifdef MISC_DEV
#include <linux/miscdevice.h>
#endif

/*
 * functions delare
 */
#ifdef CHAR_DEV
static int general_driver_register(void);	/* register driver as an char device */
static int general_driver_unregister(void);
#endif


int _index; /*pads/rtc of bt power on*/
char mode_string[ITEM_MAX_LEN];  /*pads or rtc ?*/
int _pin_mode;  /*pads or rtc ?*/

/*
 * this structure include global varaibles
 */
general_param_t	general_param;		/* global variables group */
struct general_global{
	char buff[3];
	int buffersize;
};

struct general_global *global_dev; 

#define I2C_BOARD_INFO_REGISTER_STATIC
#ifdef I2C_BOARD_INFO_REGISTER_STATIC
static struct class *rdabt_dev_class;                                                   
static struct i2c_client *rda_bt_rf_client=NULL;                                        
static struct i2c_board_info RDABT_rf_i2c_boardinfo =                                   
{                                                                                       
        I2C_BOARD_INFO(RDABT_I2C_RF_NAME, RDABT_I2C_RF_ADDR),                               
};                                                                                      
                                                                                        
static struct i2c_board_info RDABT_core_i2c_boardinfo =                                 
{                                                                                       
        I2C_BOARD_INFO(RDABT_I2C_CORE_NAME, RDABT_I2C_CORE_ADDR),                           
};                                                                                      
                                                                                        
static const struct i2c_device_id RDABT_rf_i2c_id[] =                                   
{                                                                                       
    { RDABT_I2C_RF_NAME, 0 }, { },                                                      
};                                                                                      

static int rda_bt_rf_probe(struct i2c_client *client, const struct i2c_device_id *id)   
{                                                                                       
    rda_bt_rf_client=client;                                                            
    return 0;                                                                           
}                                                                                       
static int rda_bt_rf_remove(struct i2c_client *client)                                         
{                                                                                              
    return 0;                                                                                  
}                                                                                              

static int rda_bt_rf_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)  
{                                                                                              
    strcpy(info->type, RDABT_I2C_RF_NAME);                                                     
    return 0;                                                                                  
}                                                                                              

MODULE_DEVICE_TABLE(i2c, RDABT_rf_i2c_id);                                                     

static struct i2c_driver RDABT_rf_i2c_driver =                                                 
{                                                                                              
    .probe = rda_bt_rf_probe,                                                                 
    .remove = rda_bt_rf_remove,                                                               
    .detect = rda_bt_rf_detect,                                                               
    .driver.name = RDABT_I2C_RF_NAME,                                                         
    .id_table =  RDABT_rf_i2c_id,                                                             
};                                                                                             

static int rda_bt_i2c_5400_write(struct i2c_client* client, const unsigned char addr, const unsigned short data)  
{                                                                                                                 
    unsigned char  temp[3];                                                                                       
    int ret = 0;                                                                                                  

    temp[0] = addr;                                                                                               
    temp[1] = data >> 8;                                                                                          
    temp[2] = data >> 0;                                                                                          
    ret = i2c_master_send(client, (char*)temp, 3);                                                                
    if (ret < 0)                                                                                                  
    {                                                                                                             
        printk(KERN_ERR"I2c write 5400 fail\n");                                                                
        return -1;                                                                                                
    }                                                                                                             
    return 0;                                                                                                     
}                                                                                                                 

static int rda_bt_i2c_5400_read(struct i2c_client* client, const unsigned char addr, unsigned short* data)
{                                                                                                         
    unsigned char temp[2];                                                                                
    int ret = 0;                                                                                          

    ret = i2c_master_send(client, (char*)&addr, 1);                                                       
    if (ret < 0)                                                                                          
    {                                                                                                     
        printk(KERN_ERR"I2c read 5400 1 fail\n");                                                       
        return -1;                                                                                        
    }                                                                                                     

    ret = i2c_master_recv(client, temp, 2);                                                               
    if (ret < 0)                                                                                          
    {                                                                                                     
        printk(KERN_ERR"I2c read 5400 2 fail\n");                                                       
        return -1;                                                                                        
    }                                                                                                     
    *data = (temp[0] << 8) | temp[1];                                                                     

    return 0;                                                                                             
}                                                                                                         

static void RDABT_5400_WriteData(unsigned char addr, unsigned short val)    
{                                                                           
        rda_bt_i2c_5400_write(rda_bt_rf_client,addr,val);                       
}                                                                           
                                                                            
static void RDABT_5400_ReadData(unsigned char addr,unsigned short* val)     
{                                                                           
        rda_bt_i2c_5400_read(rda_bt_rf_client,addr,val);                        
}                                                                           

int RDABT_get_chipid(void)                             
{                                                             
    unsigned char pageaddr = 0x3f;                            
    unsigned char chipidaddr = 0x20;                          
    unsigned char revidaddr =0x21;                            
    unsigned short pagevalue = 0x0001;                        
    unsigned short chipid=0xffff;                             
    unsigned short revid=0;                                   

    RDABT_5400_WriteData(pageaddr,pagevalue);                 
    RDABT_5400_ReadData(chipidaddr,&chipid);                  
    RDABT_5400_ReadData(revidaddr,&revid);                    
    
    if(chipid == 0x5876)                                      
    {                                                         
        return RDA5876_ID;                                    
    }                                                         
    else if(chipid == 0x587f)                                 
    {                                                         
        if(revid==1)                                          
        {                                                     
            return RDA5876P_VERB_ID;                          
        }                                                     
        else if (revid==3)                                    
        {                                                     
            return RDA5876P_VERC_ID;                          
        }                                                     
    }                                                         
    else                                                      
    {                                                         
        return RDA_NONE_CHIP_ID;                              
    }                                                         
}                                                              
EXPORT_SYMBOL(RDABT_get_chipid);
#endif  //i2c if 0
/*
 * open system call just mark file private data as a sensor 
 * instance by default, and you can change it by a ioctl call
 */
static int general_open(struct inode *inode, struct file *file)
{  
	if(_pin_mode)
	{
		imapx_pad_set_mode(MODE_GPIO, 1, _index);
		imapx_pad_set_dir(DIRECTION_OUTPUT, 1, _index);

		imapx_pad_set_outdat(OUTPUT_0, 1, _index);
	}
	else
	{
		rtc_gpio_dir(_index, DIRECTION_OUTPUT);
		rtc_gpio_outdat(_index , OUTPUT_0);
	}
	
	//file->private_data = &(general_param.dec_instance);
	return 0;
}
static ssize_t general_write(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	
	/* dec instance by default, you can change it by ioctl pp instance */

	return count;
}
static ssize_t general_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	
	/* dec instance by default, you can change it by ioctl pp instance */

	if (copy_to_user(buf, global_dev->buff, count)) {
	    printk(KERN_ERR"Device Read Failed\n");
		return -EFAULT;
	}
        /* TO-DO */
	return count;
}
static int  general_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
        int rda_bt_chip_id = 0; //bt i2c
        
		if (_IOC_TYPE(cmd) != BT_CMD_MAGIC) {
			    printk(KERN_ERR"Unknow IO control command magic.\n");
				    return -EINVAL;
		} else if (_IOC_NR(cmd) > BT_CMD_MAX_NUM){
			   printk(KERN_ERR"Overflow IO control index.\n");
			      return -EINVAL;
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			if (!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd))) {
				printk(KERN_ERR"IO control request read but buffer unwritable.\n");
				return -EINVAL;
			}
		} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
			if (!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd))) {
				printk(KERN_ERR"IO control request write but buffer unreadable.\n");
				return -EINVAL;
			}
		}

        switch(cmd)                                                             
        {
			case IOCTL_BT_POWER_OFF:                                            
				//printk(KERN_INFO" *** PULL DOWN ***\n");	
				if(_pin_mode)
					imapx_pad_set_outdat(OUTPUT_0, 1, _index);
				else
					rtc_gpio_outdat(_index, OUTPUT_0);
				break;                                                          
			case IOCTL_BT_POWER_ON:                                         
				//printk(KERN_INFO" *** PULL UP ***\n");	
				if(_pin_mode)
					imapx_pad_set_outdat(OUTPUT_1, 1, _index);
				else
					rtc_gpio_outdat(_index, OUTPUT_1);
				break;                                                             
			case IOCTL_BT_DEV_ID:                                               
				rda_bt_chip_id = RDABT_get_chipid();                              
                copy_to_user((unsigned char*)arg,&rda_bt_chip_id,sizeof(int));
                break;                                                          
            default :                                                           
                break;                                                          
        }                                                                       
        return 0;
}
/*
 * fasync system call be called here
 */
static int general_release(struct inode *inode, struct file *file)
{

	return 0;
}

static struct file_operations general_fops = 
{
	owner:		THIS_MODULE,
	open:		general_open,
    //read:       general_read,
	//write:		general_write,
	release:	general_release,
    unlocked_ioctl:      general_ioctl,
};

#ifdef MISC_DEV
static struct miscdevice general_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bluetooth_rda",
	.fops = &general_fops,
};

#endif

/*
 * platform operation relate functions
 */
static int general_probe(struct platform_device *pdev)
{
	int ret = -1;


        global_dev = kmalloc(sizeof(struct general_global),GFP_KERNEL);
#ifdef CHAR_DEV
	  ret = general_driver_register();
#endif
#ifdef MISC_DEV
        ret = misc_register(&general_device);
#endif

	if(ret)
	{
		printk(KERN_ERR"Fail to register  device \n");
		return ret;
	} 

	printk(KERN_INFO"rda BT Device Init OK\n");
	return ret;
}

static int general_remove(struct platform_device *pdev)
{
	/* release irq */
	//general_driver_unregister();
	return 0;
}

#ifdef CONFIG_PM
static int general_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int general_resume(struct platform_device *pdev)
{
           return 0;
}
#endif

static struct platform_driver general_driver = 
{
	.probe		= general_probe,
	.remove		= general_remove,
#ifdef CONFIG_PM
	.suspend	= general_suspend,
	.resume		= general_resume,
#endif
	.driver		=
	{
		.owner		= THIS_MODULE,
		.name		= "bluetooth_rda",
	},
};

/*
 * init and exit
 */
static struct platform_device device_general = {
	.name		= "bluetooth_rda",
};

#ifdef I2C_BOARD_INFO_REGISTER_STATIC                               

int i2c_static_add_device(struct i2c_board_info *info)              
{                                                                   
    struct i2c_adapter *adapter;                                    
    struct i2c_client  *client;                                     
    int    ret;                                                     
    adapter = i2c_get_adapter(RDABT_I2C_CHANNEL);                  
    if (!adapter) {                                                 
        pr_err("%s: can't get i2c adapter\n", __func__);            
        ret = -ENODEV;                                              
        goto i2c_err;                                               
    }                                                               

    client = i2c_new_device(adapter, info);                         
    if (!client) {                                                  
        pr_err("%s:  can't add i2c device at 0x%x\n",               
                __FUNCTION__, (unsigned int)info->addr);            
        ret = -ENODEV;                                              
        goto i2c_err;                                               
    }                                                               

    i2c_put_adapter(adapter);                                       
    return 0;                                                       

i2c_err:                                                            
    return ret;                                                     
}                                                                   

#endif /*  I2C_BOARD_INFO_REGISTER_STATIC */                         

static int __init general_init(void)
{
    int ret = -1;
    if(item_exist("bluetooth.exist")&&(item_integer("bluetooth.exist",0) == 1)&&item_exist("bt.int")){
		item_string(mode_string, "bt.int", 0);
        _index = item_integer("bt.int",1);    

		if(strncmp(mode_string, "pads", 4) == 0 || strncmp(mode_string, "pad",3) == 0)
			_pin_mode = 1;		
		else if(strncmp(mode_string ,"rtc" ,3)==0)
			_pin_mode = 0;
		else
		{
			printk(KERN_ERR"Sorry,an error in items of bt.int\n");
			return -EPERM;
		}
        if(ret = platform_device_register(&device_general)){
            general_error("[bt]Fail to add platform device for BT Decode Driver\n");
            return -EPERM;

        }
        if(ret = platform_driver_register(&general_driver))
        {
            general_error("[bt]Fail to register platform driver for BT Decode Driver\n");
            return -EPERM;
        }

#ifdef I2C_BOARD_INFO_REGISTER_STATIC                                   
        ret = i2c_static_add_device(&RDABT_rf_i2c_boardinfo);               
        if (ret < 0) {                                                      
            printk(KERN_ERR"[bt]add i2c device error\n");         
            return ret;
        }
#endif                                                                  
        if (i2c_add_driver(&RDABT_rf_i2c_driver))             
        {                                                     
            printk(KERN_ERR"[bt]rdabt_rf_i2c_driver failed!\n");          
            ret = -ENODEV;                                    
            return ret;                                       
        }                                                     
        printk(KERN_INFO"[bt]i2c driver add ok!!\n");              
    }else{
        printk(KERN_INFO"[bt]no bt or an error in items\n");
    }
    return ret;
}

static void __exit general_exit(void)
{
    /* call remove */
    printk(KERN_INFO"[RDABT] rdabt_exit module\n");       

#ifdef I2C_BOARD_INFO_REGISTER_STATIC             
    i2c_unregister_device(rda_bt_rf_client);      
#endif                                            

    i2c_del_driver(&RDABT_rf_i2c_driver);         

	platform_driver_unregister(&general_driver);

	general_debug("rda BT Decode Driver exit OK\n");
}

module_init(general_init);
module_exit(general_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALLEN");
MODULE_DESCRIPTION("General Device Driver");

/*
 * just write 0 to all registers to reset harware
 * TODO: we have check whether it's needed
int reset_hw_reg_sensor(void)
{
	return 0;
}
*/
/*
 * this function do driver register to regist a node under /dev
 */
static struct class *general_class;
#ifdef CHAR_DEV
int general_driver_register(void)
{
	int ret = 0;
	ret = register_chrdev(GENERAL_DEFAULT_MAJOR, "general", &generalfops);
	if(ret < 0)
	{
		printk(KERN_ERR"register char deivce error\n");
		return -1;
	}

	general_class = class_create(THIS_MODULE, "general");
	device_create(general_class, NULL, MKDEV(GENERAL_DEFAULT_MAJOR, GENERAL_DEFAULT_MINOR), NULL, "general");

	return ret;
}

int general_driver_unregister(void)
{
	device_destroy(general_class, MKDEV(GENERAL_DEFAULT_MAJOR, GENERAL_DEFAULT_MINOR));
	class_destroy(general_class);
	unregister_chrdev(GENERAL_DEFAULT_MAJOR, "general");

	return 0;
}
#endif

