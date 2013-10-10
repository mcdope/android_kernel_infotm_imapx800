/***************************************************************************** 
 * general.h
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: common head file of imapx200 media sensor driver
 *
 * Author:
 *     David
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.1  12/10/2009 
 * Author:
 *     Allen
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 2.0  01/07/2013 
 *******************************************************************************/

#define MISC_DEV           // choose one for your device
//#define CHAR_DEV

#ifdef MISC_DEV
//#define MISC_DYNAMIC_MINOR 10
#endif

#ifdef CHAR_DEV
#define GENERAL_DEFAULT_MAJOR		114
#define GENERAL_DEFAULT_MINOR		0
#endif

/*
 * in arch/arm/mach-imapx200/devices.c there reserved 4KB for
 * Decode register, but actually only first 404 bytes used, 
 * 4KB is a page size, so reserve 4KB much larger than sensor
 * needs does make sense
#define IMAPX200_SENSOR_ACT_REG_SIZE	(101 * 4)
 */
/*
 * ioctl commands
 */

#define BT_CMD_MAGIC           'T'
#define BT_CMD_MAX_NUM          3
#define IOCTL_BT_DEV_ID        _IOR(BT_CMD_MAGIC, 1, int)
#define IOCTL_BT_POWER_ON      _IOR(BT_CMD_MAGIC, 2, int)
#define IOCTL_BT_POWER_OFF     _IOR(BT_CMD_MAGIC, 3, int)
#define RDABT_I2C_NAME         "rdabt"               
#define RDABT_I2C_RF_NAME      "rdabt_iic_rf"
#define RDABT_I2C_CORE_NAME    "rdabt_iic_core"   

/* select the I2C channel and address*/            
#define RDABT_I2C_CHANNEL       1           
                                                  
/* BT I2C address*/                                
#define RDABT_I2C_RF_ADDR       0x16   //(0x2C>>1)
#define RDABT_I2C_CORE_ADDR     0x15   //(0x2A>>1) 
#define RDABT_I2C_ADDR          0x30                  

/*
 * debug macros include debug alert error
 */
#define CONFIG_IMAP_GENERAL_DEBUG
#ifdef CONFIG_IMAP_GENERAL_DEBUG

#define GENERAL_DEBUG(debug, ...)         \
	                printk(KERN_INFO "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)

#define GENERAL_ALERT(alert, ...)         \
	                printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)

#define GENERAL_ERROR(error, ...)         \
	                printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#else

#define GENERAL_DEBUG(debug, ...)	do{}while(0)
#define GENERAL_ALERT(alert, ...)	do{}while(0)
#define GENERAL_ERROR(error, ...)	do{}while(0)

#endif /* CONFIG_IMAP_MEDIA_SENSOR_DEBUG */

#define general_debug(debug, ...)         GENERAL_DEBUG(debug, ##__VA_ARGS__)
#define general_alert(alert, ...)         GENERAL_ALERT(alert, ##__VA_ARGS__)
#define general_error(error, ...)         GENERAL_ERROR(error, ##__VA_ARGS__)

/*
 * global variables
 */
typedef struct 
{
	struct resource         *resource_mem;
	void __iomem            *reg_base_virt_addr;
	unsigned int            reg_base_phys_addr;
	unsigned int            reg_reserved_size;
#ifdef CONFIG_IMAP_SENSOR_SIGNAL_MODE
	struct fasync_struct    *async_queue_dec;
	struct fasync_struct    *async_queue_pp;
#endif	/* CONFIG_IMAP_SENSOR_SIGNAL_MODE */
	unsigned int            dec_instance;
	unsigned int            pp_instance;
#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE
	struct timeval		end_time;
#endif
}general_param_t;

typedef enum              
{                         
    RDA_NONE_CHIP_ID=0,   
    RDA5876_ID=1,         
    RDA5876P_VERB_ID=2,   
    RDA5876P_VERC_ID=3    
}RDA_CHIP_ENUM;           

