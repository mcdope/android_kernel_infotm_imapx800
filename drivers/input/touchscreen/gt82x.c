/* drivers/input/touchscreen/goodix_touch.c
 *
 * Copyright (C) 2011 Goodix, Inc.
 * 
 * Author: Scott
 * Date: 2011.11.08
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/proc_fs.h>
#include "gt82x.h"
#include <linux/gpio.h>

#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h> 
#include <linux/mm.h> 
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <mach/imap-iomap.h> 
#include <mach/pad.h>
#include <mach/items.h>

#include <linux/clk.h>

static struct workqueue_struct *goodix_wq;
static const char *s3c_ts_name = "Goodix Capacitive TouchScreen";
static struct i2c_client * i2c_connect_client = NULL;
//static short  goodix_read_version(struct goodix_ts_data *ts);    
static void guitar_reset(s32);

#ifdef CREATE_WR_NODE
static struct proc_dir_entry *goodix_proc_entry;
#endif 

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h);
static void goodix_ts_late_resume(struct early_suspend *h);
#endif

#ifdef AUTO_UPDATE_GUITAR
static int guitar_update_proc(void*);
static u8 get_ic_fw_msg(struct goodix_ts_data *);
struct goodix_ts_data g_ts;
#endif
static int tsp_irq_index;

static unsigned int _sui_irq_num;
static unsigned int g_pad_index;

static int SCREEN_MAX_X=1024;
static int SCREEN_MAX_Y=768;

extern int get_lcd_width(void);
extern int get_lcd_height(void);


/*******************************************************	
功能：	
	读取从机数据
	每个读操作用两条i2c_msg组成，第1条消息用于发送从机地址，
	第2条用于发送读取地址和取回数据；每条消息前发送起始信号
参数：
	client:	i2c设备，包含设备地址
	buf[0]~buf[1]：	 首字节为读取地址
	buf[2]~buf[len]：数据缓冲区
	len：	读取数据长度
return：
	执行消息数
*********************************************************/
/*Function as i2c_master_send */
static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, int len)
{
#if 0
	int ret;
	ret=i2c_master_recv(client, buf, len);
	if(ret<=0)                                          
	{                                                   
		printk("[TSP]i2c_read_interface error\n");  
		return -1;                           
	}                                                   

	return 0;                                    
#endif

#if 1
    struct i2c_msg msgs[2];
    int ret=-1;

    //发送写地址
    msgs[0].flags=!I2C_M_RD; //写消息
    msgs[0].addr=client->addr;
    msgs[0].len=2;
    msgs[0].buf=&buf[0];
    //接收数据
    msgs[1].flags=I2C_M_RD;//读消息
    msgs[1].addr=client->addr;
    msgs[1].len=len-2;
    msgs[1].buf=&buf[2];

    ret=i2c_transfer(client->adapter,msgs, 2);
    return ret;
#endif
}

/*******************************************************	
功能：
	向从机写数据
参数：
	client:	i2c设备，包含设备地址
	buf[0]~buf[1]：	 首字节为写地址
	buf[2]~buf[len]：数据缓冲区
	len：	数据长度	
return：
	执行消息数
*******************************************************/
/*Function as i2c_master_send */
static int i2c_write_bytes(struct i2c_client *client,uint8_t *data,int len)
{
/*
	ret=i2c_master_send(client, data, len);

	if(ret<=0)                                          
	{                                                   
		printk("[TSP]i2c_read_interface error\n");  
		return -1;                           
	}                                                   

	return 0;                                    
*/
#if 1
    struct i2c_msg msg;
    int ret=-1;
    
    //发送设备地址
    msg.flags=!I2C_M_RD;//写消息
    msg.addr=client->addr;
    msg.len=len;
    msg.buf=data;        

    ret=i2c_transfer(client->adapter,&msg, 1);
    return ret;
#endif
}

/*******************************************************
功能：
	发送前缀命令
	
	ts:	client私有数据结构体
return：

	执行结果码，0表示正常执行
*******************************************************/
static int i2c_pre_cmd(struct goodix_ts_data *ts)
{
#if 1
    int ret;
    uint8_t pre_cmd_data[2]={0};
    pre_cmd_data[0]=0x0f;
    pre_cmd_data[1]=0xff;
    ret=i2c_write_bytes(ts->client,pre_cmd_data,2);
    return ret;//*/
#else
    return 1;
#endif
}

/*******************************************************
功能：
	发送后缀命令
	
	ts:	client私有数据结构体
return：

	执行结果码，0表示正常执行
*******************************************************/
static int i2c_end_cmd(struct goodix_ts_data *ts)
{
    int ret;
    uint8_t end_cmd_data[2]={0};    
    end_cmd_data[0]=0x80;
    end_cmd_data[1]=0x00;
    ret=i2c_write_bytes(ts->client,end_cmd_data,2);
    return ret;//*/
}

/*******************************************************
功能：
	Guitar初始化函数，用于发送配置信息，获取版本信息
参数：
	ts:	client私有数据结构体
return：
	执行结果码，0表示正常执行
*******************************************************/

static const uint8_t config_info_M810I[]=      //Do not change the array name
    {
        0x0F,0x80,/*config address*/

	0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,
	0x07,0x16,0x08,0x17,0x09,0x18,0xFF,0x11,0x12,0x13,
	0xFF,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,
	0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,0x0E,0x04,
	0x0D,0x03,0xFF,0x0D,0xFF,0x0F,0x10,0x11,0x12,0x13,
	0x0F,0x03,0x60,0x00,0x00,0x13,0x00,0x00,0x07,0x00,
	0x00,0x02,0x45,0x30,0x14,0x03,0x00,0x05,0x00,0x03,
	0x00,0x04,0x00,0x5F,0x53,0x63,0x57,0x08,0x00,0x25,
	0x22,0x05,0x14,0x10,0x00,0x03,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x30,0x30,0x20,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01

    };

static const uint8_t config_info_def[]=      //Do not change the array name
    {
        0x0F,0x80,/*config address*/

    0x1D,0x0E,0x1C,0x0D,0x1B,0x0C,0x1A,0x0B,0x19,0x0A,
    0x18,0x09,0x17,0x08,0x16,0x07,0x15,0x06,0x14,0x05,
    0x13,0x04,0x12,0x03,0x11,0x02,0x10,0x01,0x0F,0x00,
    0x00,0x0A,0x01,0x0B,0x02,0x0C,0x03,0x0D,0x04,0x0E,
    0x05,0x0F,0x06,0x10,0x07,0x11,0x08,0x12,0x09,0x13,
    0x0F,0x03,0x08,0x00,0x00,0x20,0x00,0x00,0x00,0x00,
    0x00,0x02,0x40,0x30,0x34,0x03,0x00,0x05,0x00,0x03,
    0x00,0x04,0x00,0x43,0x58,0x3F,0x52,0x09,0x00,0x25,
    0x19,0x05,0x14,0x10,0x03,0x70,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01



    };

static const uint8_t config_info_w2[]=      //Do not change the array name
    {
	0x0F,0x80,/*config address*/
		
	0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,
		0x07,0x16,0x08,0x17,0x09,0x18,0x0A,0x19,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,
		0x10,0x07,0x11,0x08,0x12,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0x1B,0x03,0x30,0x10,0x10,
		0x10,0x00,0x00,0x05,0x00,0x00,0x02,0x42,0x32,
		0x35,0x03,0x00,0x05,0x00,0x02,0x58,0x04,0x00,
		0x43,0x3C,0x3F,0x38,0x08,0x00,0x27,0x19,0x05,
		0x14,0x10,0x02,0x9E,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x01
    };

static const uint8_t config_info_COBY7058[]=      //Do not change the array name
    {
	0x0F,0x80,/*config address*/
		
	0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,
	0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,
	0x0A,0x19,0x0B,0x1A,0xFF,0x15,0x16,0x17,
	0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x03,0x0D,
	0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,
	0x08,0x12,0xFF,0x13,0xFF,0x0F,0x10,0x11,
	0x12,0x13,0x0F,0x03,0x88,0x00,0x00,0x30,
	0x00,0x00,0x08,0x00,0x00,0x02,0x48,0x30,
	0x34,0x03,0x00,0x05,0x00,0x02,0x58,0x04,
	0x00,0x66,0x66,0x60,0x60,0x2A,0x00,0x28,
	0x20,0x02,0x14,0x10,0x00,0x07,0x01,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };
static const uint8_t config_info_R810I[]=      //Do not change the array name
    {
        0x0F,0x80,/*config address*/
        0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,
        0x07,0x16,0x08,0x17,0x09,0x18,0x0A,0x19,0x0B,0x1A,
        0xFF,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,
        0x13,0x09,0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,
        0x0E,0x04,0x0D,0x03,0xFF,0x0E,0x0F,0x10,0x11,0x12,
        0x1F,0x03,0x48,0x10,0x10,0x1D,0x00,0x00,0x09,0x00,
        0x00,0x02,0x45,0x30,0x1C,0x03,0x00,0x05,0x00,0x03,
        0x00,0x04,0x00,0x33,0x38,0x30,0x35,0x25,0x00,0x25,
        0x19,0x02,0x14,0x10,0x02,0x30,0x01,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };

static const uint8_t config_info_R100I[]=      //Do not change the array name
    {
        0x0F,0x80,/*config address*/
        0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,0x04,0x13,
        0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,
        0x0A,0x19,0x0B,0x1A,0x0C,0x1B,0x0D,0x1C,0x0E,0x1D,
        0x13,0x09,0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,
        0x0E,0x04,0x0D,0x03,0x0C,0x02,0x0B,0x01,0x0A,0x00,
        0x0F,0x03,0xE0,0x10,0x10,0x2D,0x00,0x00,0x0A,0x00,
        0x00,0x0A,0x48,0x30,0x3D,0x03,0x00,0x05,0x00,0x03,
        0x00,0x04,0x00,0x3A,0x3C,0x3D,0x3E,0x28,0x00,0x06,
        0x19,0x02,0x14,0x10,0x00,0x04,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x60,0x20,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };
static const uint8_t config_info_R101I[]=      //Do not change the array name
    {
        0x0F,0x80,/*config address*/
        0x1D,0x0E,0x1C,0x0D,0x1B,0x0C,0x1A,0x0B,0x19,0x0A,
        0x18,0x09,0x17,0x08,0x16,0x07,0x15,0x06,0x14,0x05,
        0x13,0x04,0x12,0x03,0x11,0x02,0x10,0x01,0x0F,0x00,
        0x00,0x0A,0x01,0x0B,0x02,0x0C,0x03,0x0D,0x04,0x0E,
        0x05,0x0F,0x06,0x10,0x07,0x11,0x08,0x12,0x09,0x13,
        0x1F,0x03,0x48,0x10,0x10,0x27,0x00,0x00,0x0A,0x00,
        0x00,0x0A,0x48,0x30,0x3D,0x03,0x00,0x05,0x00,0x03,
        0x00,0x04,0x00,0x49,0x49,0x44,0x44,0x28,0x00,0x25,
        0x19,0x02,0x14,0x10,0x02,0xDA,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x60,0x20,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    }; 


static const uint8_t config_info_M7030[]=      //Do not change the array name
{
	0x0F,0x80,  /*config address*/

	0x1A,0x0B,0x19,0x0A,0x18,0x09,0x17,0x08,0x16,0x07,
	0x15,0x06,0x14,0x05,0x13,0x04,0x12,0x03,0x11,0x02,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x03,0x0D,0x04,0x0E,0x05,0x0F,0x06,0x10,0x07,0x11,
	0x08,0x12,0x09,0x13,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x0F,0x03,0x88,0x88,0x88,0x2A,0x00,0x00,0x08,0x00,
	0x00,0x0E,0x45,0x30,0x18,0x03,0x00,0x05,0x00,0x02,
	0x58,0x04,0x00,0x5C,0x52,0x60,0x56,0x25,0x00,0x05,
	0x19,0x05,0x14,0x10,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01
};

static const uint8_t config_info_HY971[]=      //Do not change the array name
{
	0x0F,0x80,  /*config address*/

	0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,0x04,0x13,
	0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0x09,0x18,
	0x0A,0x19,0x0B,0x1A,0x0C,0x1B,0x0D,0x1C,0x0E,0x1D,
	0x13,0x09,0x12,0x08,0x11,0x07,0x10,0x06,0x0F,0x05,
	0x0E,0x04,0x0D,0x03,0x0C,0x02,0x0B,0x01,0x0A,0x00,
	0x1F,0x03,0x48,0x10,0x10,0x25,0x00,0x00,0x09,0x00,
	0x00,0x0A,0x5A,0x30,0x1C,0x03,0x00,0x05,0x00,0x03,
	0x00,0x04,0x00,0x49,0x49,0x44,0x44,0x26,0x00,0x25,
	0x19,0x02,0x14,0x10,0x02,0xDA,0x01,0x00,0x00,0x00,
	0x00,0x00,0x38,0x28,0x30,0x20,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01
};
    
static int goodix_init_panel(struct goodix_ts_data *ts)
{
    int ret = -1;
    const uint8_t * config_info;

		if(item_exist("board.name"))
		{
			if( item_equal("board.name", "M801I", 0)== 1)
			{
				printk("%s:,M801I\n",__func__);
				config_info = config_info_M810I;
			}
			else if( item_equal("board.name", "w2", 0)== 1)
			{
				printk("%s:,W2\n",__func__);
				config_info = config_info_w2;
			}
			else if( item_equal("board.name", "COBY7058", 0)== 1)
			{
				printk("%s:,W2\n",__func__);
				config_info = config_info_COBY7058;
			}
			else if( item_equal("board.name", "M7030", 0)== 1)
			{
				printk("%s:,W2\n",__func__);
				config_info = config_info_M7030;
			}
			else if( item_equal("board.name", "ruiyida8inch", 0)== 1)
			{
				printk("%s:,ruiyida8inch\n",__func__);
				config_info = config_info_R810I;
			}
			else if( item_equal("board.name", "ruiyida100inch", 0)== 1)
			{
			     printk("%s:,ruiyida100inch\n",__func__);
			     config_info = config_info_R100I;
			}
			else if( item_equal("board.name", "ruiyida101inch", 0)== 1)
			{
			     printk("%s:,ruiyida101inch\n",__func__);
			     config_info = config_info_R101I;
			}
			else if( item_equal("board.name", "HY971", 0)== 1)
			{
				printk("%s:,HY971\n",__func__);
				config_info = config_info_HY971;
			}
			else
			{
				printk("%s:,default\n",__func__);
				config_info = config_info_def;
			}
			
		}
		else
		{
				printk("%s:,default",__func__);
				config_info = config_info_def;
		}

#ifdef RESOLUTION_LOC
    TOUCH_MAX_HEIGHT  = ((config_info[RESOLUTION_LOC] << 8)\
                        |config_info[RESOLUTION_LOC + 1]);
    TOUCH_MAX_WIDTH = ((config_info[RESOLUTION_LOC + 2] << 8)\
                        |config_info[RESOLUTION_LOC + 3]);

    DEBUG_MSG("TOUCH_MAX_WIDTH  : %d\n", (int)TOUCH_MAX_WIDTH);
    DEBUG_MSG("TOUCH_MAX_HEIGHT : %d\n", (int)TOUCH_MAX_HEIGHT);
#endif
#if 0
    ret=i2c_write_bytes(ts->client,config_info, (sizeof(config_info)/sizeof(config_info[0])));
    if (ret < 0)
    {
        return fail;
    }
#endif
    {
	    int i;
	    char buf[3];
	    for(i = 0; i < 112 ;i++)
	    {
		    buf[0]= 0x0f;
		    buf[1]= 0x80+i;
		    buf[2]= config_info[i+2];
		    ret=i2c_write_bytes(ts->client,buf, 3);
	    }
    }

    msleep(10);
    return success;
}

#if 0
static short get_chip_version( unsigned int sw_ver )
{
    return 0;
}

/*******************************************************
功能：
	获取版本信息
参数：
	ts:	client私有数据结构体
return：
	执行结果码，0表示正常执行
*******************************************************/
static short goodix_read_version(struct goodix_ts_data *ts)
{
    int ret = -1;
    uint8_t version_data[25+2]={0};
    version_data[0] = 0x0F;
    version_data[0] = 0x68;

    ret = i2c_read_bytes(ts->client, version_data, 26);
    if (ret < 0)
    {
        return ret;
    }
    
    return 1;
}
#endif

static int touch_num(uint8_t value, int max)
{
    int tmp = 0;

    while((tmp < max) && value)
    {
        if ((value & 0x01) == 1)
        {
            tmp++;
        }
        value = value >> 1;
    }

    return tmp;
}
#if 0
static int i2c_read_coor(struct i2c_client *client, uint8_t *buf, int len)
{
    struct i2c_msg msgs[4];
    u8 end_buf[2] = {0x08, 0x00};
    int ret=-1;

    //发送写地址
    msgs[0].flags=!I2C_M_RD; //写消息
    msgs[0].addr=client->addr;
    msgs[0].len=2;
    msgs[0].buf=&buf[0];
    //接收数据
    msgs[1].flags=I2C_M_RD;//读消息
    msgs[1].addr=client->addr;
    msgs[1].len=len-2;
    msgs[1].buf=&buf[2];

    //发送结束命令
    msgs[2].flags=!I2C_M_RD; //写消息
    msgs[2].addr=client->addr;
    msgs[2].len=2;
    msgs[2].buf=&end_buf[0];

    ret=i2c_transfer(client->adapter,msgs, 2);
    ret=i2c_transfer(client->adapter,&msgs[2], 1);
    return ret;
}
#endif

/*******************************************************	
功能：
	触摸屏工作函数
	由中断触发，接受1组坐标数据，校验后再分析输出
参数：
	ts:	client私有数据结构体
return：
	执行结果码，0表示正常执行
********************************************************/
static void goodix_ts_work_func(struct work_struct *work)
{
    uint8_t finger = 0;
    uint8_t key = 0;
    uint8_t chk_sum = 0;
    static uint8_t last_key = 0;
    uint16_t X_value;
    uint16_t Y_value;
    unsigned int count = 0;
    unsigned int position = 0;
    int ret = -1;
    int tmp = 0;
    uint8_t *coor_point;
    int i;
    uint8_t touch_data[2 + 2 + 5*MAX_FINGER_NUM + 1] = {READ_TOUCH_ADDR_H,READ_TOUCH_ADDR_L,0, 0};
  	//uint8_t buf[12];
	uint8_t buf[2 + 2 + 5*MAX_FINGER_NUM + 1];
    static uint8_t finger_last[MAX_FINGER_NUM+1]={0};        //上次触摸按键的手指索引
    uint8_t finger_current[MAX_FINGER_NUM+1] = {0};        //当前触摸按键的手指索引

    struct goodix_ts_data *ts = container_of(work, struct goodix_ts_data, work);
    ts->irq_flag = 0;
    
#ifndef INT_PORT
COORDINATE_POLL:
#endif
    if( tmp > 9)
    {
        dev_info(&(ts->client->dev), "Because of transfer error,touchscreen stop working.\n");
        goto XFER_ERROR ;
    }

    //建议将数据一次性读取完,i2c error occur when read/write continuesly//
	if(MAX_FINGER_NUM == 1)
	{
		ret=i2c_read_bytes(ts->client, touch_data,sizeof(touch_data)/sizeof(touch_data[0])); 
	} else {
		buf[0] = READ_TOUCH_ADDR_H;
		buf[1] = READ_TOUCH_ADDR_L;
		ret = i2c_read_bytes(ts->client, buf, 30);
		memcpy(&touch_data[2], &buf[2],28);

//		buf[0] = READ_TOUCH_ADDR_H;
  //      buf[1] = READ_TOUCH_ADDR_L;
//	    ret = i2c_read_bytes(ts->client, buf, 12); 
  //      memcpy(&touch_data[2], &buf[2],10);
		
//		buf[0] = READ_TOUCH_ADDR_H;
  //      buf[1] = READ_TOUCH_ADDR_L + 10;
//	    ret = i2c_read_bytes(ts->client, buf, 11); 
  //      memcpy(&touch_data[12], &buf[2],9);

//		buf[0] = READ_TOUCH_ADDR_H;
  //      buf[1] = READ_TOUCH_ADDR_L + 10 + 9;
//	    ret = i2c_read_bytes(ts->client, buf, 11); 
  //      memcpy(&touch_data[21], &buf[2],9);
	}
    i2c_end_cmd(ts);
    if(ret <= 0) 
    {
        dev_err(&(ts->client->dev),"I2C transfer error. Number:%d\n ", ret);
        ts->bad_data = 1;
        tmp ++;
#ifndef INT_PORT
        goto COORDINATE_POLL;
#else
        goto XFER_ERROR;
#endif
    }

    if(ts->bad_data)
    {
        //TODO:Is sending config once again (to reset the chip) useful?    
        ts->bad_data = 0;
        msleep(20);
    }

    key = touch_data[3]&0x0f; // 1, 2, 4, 8
    if (key == 0x0f)
    {
        if (fail ==goodix_init_panel(ts))
        {
/**/        DEBUG_COOR("Reload config failed!\n");
        }
        else
        {   
            DEBUG_COOR("Reload config successfully!\n");
        }
        goto XFER_ERROR;
    }

    if((touch_data[2]&0xC0)!=0x80)
    {
        goto DATA_NO_READY;        
    }

    finger = (uint8_t)touch_num(touch_data[2]&0x1f, MAX_FINGER_NUM);

/**///DEBUG_COOR("touch num:%x\n", finger);

    for (i = 1;i < MAX_FINGER_NUM + 1; i++)        
    {
        finger_current[i] = !!(touch_data[2] & (0x01<<(i-1)));
    }

#ifndef DEBUG_COORD
/**/for (i = 0; i < finger*5+4; i++)
/**/{  
/**/  //  DEBUG_COOR("%5x", touch_data[i]);
/**/}
/**///DEBUG_COOR("\n");
#endif 

    //检验校验和    
    coor_point = &touch_data[4];
    chk_sum = 0;
    for ( i = 0; i < 5*finger; i++)
    {
        chk_sum += coor_point[i];
/**/    //DEBUG_COOR("%5x", coor_point[i]);
    }
/**///DEBUG_COOR("\ncheck sum:%x\n", chk_sum);
/**///DEBUG_COOR("check sum byte:%x\n", coor_point[5*finger]);
    if (chk_sum != coor_point[5*finger])
    {
        goto XFER_ERROR;
    }

    //发送坐标//
    for(i = 0, position=1;position < MAX_FINGER_NUM+1; position++)
    {
        if((finger_current[position] == 0)&&(finger_last[position] != 0))
        {
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X, 0);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, 0);
            input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
            input_mt_sync(ts->input_dev);
        }
        else if(finger_current[position])
        {     
            X_value = coor_point[i] << 8;
            X_value = X_value | coor_point[i + 1];
			      //X_value = X_value+270;

            Y_value = coor_point[i + 2] << 8;
            Y_value = Y_value | coor_point[i + 3];
			  

	    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, position-1);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X, Y_value);  //can change x-y!!!
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, X_value);
            input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR,1);
	    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 50);
            input_mt_sync(ts->input_dev);
            i += 5;
	   // printk("X=%d>>>Y=%d\n",X_value,Y_value);
		//  printk("TOUCH_MAX_WIDTH = %d<><><><>TOUCH_MAX_HEIGHT = %d\n",TOUCH_MAX_WIDTH,TOUCH_MAX_HEIGHT);

/**///        DEBUG_COOR("Y:%d\n", (int)X_value);
/**/  //      DEBUG_COOR("X:%d\n", (int)Y_value);
        }
    }
    input_sync(ts->input_dev);    

    for(position=1;position<MAX_FINGER_NUM+1; position++)
    {
        finger_last[position] = finger_current[position];
    }

#ifdef HAVE_TOUCH_KEY
#ifndef DEBUG_COORD
/**/for (i = 0; i < 4; i++)
/**/{
/**/ //   DEBUG_COOR("key:%4x   ", !!(key&(0x01<<i)));
/**/}
/**///DEBUG_COOR("\n");
#endif

    if((last_key == 0)&&(key == 0))
        ;
    else
    {
        for(count = 0; count < 4; count++)
        {
            input_report_key(ts->input_dev, touch_key_array[count], !!(key&(0x01<<count)));    
        }
    }
    last_key = key;
#endif
    
DATA_NO_READY:
XFER_ERROR:
    ts->irq_flag = 1;
    if(ts->use_irq)
	    enable_irq(_sui_irq_num);
}

/*******************************************************	
功能：
	计时器响应函数
	由计时器触发，调度触摸屏工作函数运行；之后重新计时
参数：
	timer：函数关联的计时器	
return：
	计时器工作模式，HRTIMER_NORESTART表示不需要自动重启
********************************************************/
static enum hrtimer_restart goodix_ts_timer_func(struct hrtimer *timer)
{
    struct goodix_ts_data *ts = container_of(timer, struct goodix_ts_data, timer);

    if (ts->irq_flag)
    {
        queue_work(goodix_wq, &ts->work);
        hrtimer_start(&ts->timer, ktime_set(0, (POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
    }

    return HRTIMER_NORESTART;
}

/*******************************************************	
功能：
	中断响应函数
	由中断触发，调度触摸屏处理函数运行
参数：
	timer：函数关联的计时器	
return：
	计时器工作模式，HRTIMER_NORESTART表示不需要自动重启
********************************************************/
static irqreturn_t goodix_ts_irq_handler(int irq, void *dev_id)
{
    struct goodix_ts_data *ts = dev_id;

    if(imapx_pad_irq_pending(tsp_irq_index))
	    imapx_pad_irq_clear(tsp_irq_index);
    else
	    return IRQ_HANDLED;

    disable_irq_nosync(_sui_irq_num);
    if (ts->irq_flag)
    {
        queue_work(goodix_wq, &ts->work);
        return IRQ_HANDLED;
    }
    
    return IRQ_NONE;
}

/*******************************************************	
功能：
	管理GT801的电源，允许GT801 PLUS进入睡眠或将其唤醒
参数：
	on:	0表示使能睡眠，1为唤醒
return：
	是否设置成功，0为成功
	错误码：-1为i2c错误，-2为GPIO错误；-EINVAL为参数on错误
********************************************************/
//#if defined(INT_PORT)
static int goodix_ts_power(struct goodix_ts_data * ts, int on)
{
    int ret = -1;

    unsigned char i2c_control_buf[3] = {0x0f,0xf2,0xc0};        //suspend cmd

    if(ts != NULL && !ts->use_irq)
        return -2;

    switch(on)
    {
    case 0:
        ret = i2c_write_bytes(ts->client, i2c_control_buf, 3);
        return ret;

    case 1:
#if 0
        GPIO_DIRECTION_OUTPUT(INT_PORT, 0);
        GPIO_SET_VALUE(INT_PORT, 0);
        msleep(1);
        GPIO_SET_VALUE(INT_PORT, 1);
        msleep(1);
        GPIO_DIRECTION_INPUT(INT_PORT);
        GPIO_PULL_UPDOWN(INT_PORT, 0);

        msleep(10);
        ret = 0;
        return success;
#endif
	return 0;
    default:
        DEBUG_MSG(KERN_DEBUG "%s: Cant't support this command.", s3c_ts_name);
        return -EINVAL;
    }

}

/*******************************************************	
功能：
	触摸屏探测函数
	在注册驱动时调用(要求存在对应的client)；
	用于IO,中断等资源申请；设备注册；触摸屏初始化等工作
参数：
	client：待驱动的设备结构体
	id：设备ID
return：
	执行结果码，0表示正常执行
********************************************************/
static int goodix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    int retry=0;
    struct goodix_ts_data *ts = NULL;
    struct task_struct *thread = NULL;
    struct goodix_i2c_rmi_platform_data *pdata = NULL;
    int index;

    dev_dbg(&client->dev,"Install touch driver.\n");
    printk(KERN_INFO "[CTP-GT828]-goodix_ts_probe\n");

    guitar_reset(30);
//    index = imapx_pad_index("cond20");
//    if(index == -1)                                
//	    return -EINVAL;
	index = item_integer("ts.int", 1);
    	g_pad_index = index;
	printk("CTP INT pad index %d\n", index);       
    tsp_irq_index = index;                         
//#define PAD_SYSM_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000))
//    int reg = 0;
//    reg = readl(PAD_SYSM_VA);
//    writel(0,PAD_SYSM_VA);


    _sui_irq_num = imapx_pad_irq_number(index);
     if(!_sui_irq_num)                     
	              return -EINVAL;               
     // printk("CTP irq %d\n", _sui_irq_num); 

      imapx_pad_set_mode(0, 1, index);
      imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);



    //Check I2C function
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
    {
        dev_err(&client->dev, "Must have I2C_FUNC_I2C.\n");
        ret = -ENODEV;
        goto err_check_functionality_failed;
    }

    ts = kzalloc(sizeof(*ts), GFP_KERNEL);
    if (ts == NULL)
    {
        ret = -ENOMEM;
        goto err_alloc_data_failed;
    }

    INIT_WORK(&ts->work, goodix_ts_work_func);        //init work_struct
    ts->client = client;
    ts->power = goodix_ts_power;
    ts->bad_data = 0;
    ts->irq_flag = 1;
    i2c_set_clientdata(client, ts);
    pdata = client->dev.platform_data;

#if 0
    //Test I2C connection.    
    for(retry = 0;retry < 3; retry++)
    //while(1)
    {
        ret =i2c_pre_cmd(ts);
        if (ret > 0)
            break;
        msleep(20);
    }
    if(ret <= 0)
    {
        dev_err(&client->dev, "Warnning: I2C communication might be ERROR!\n");
        DEBUG_MSG("I2C test failed. I2C addr:%x\n", client->addr);
        goto err_i2c_failed;
    }
    
#ifdef AUTO_UPDATE_GUITAR
    DEBUG_MSG("Ready to run update thread.\n");
    thread = kthread_run(guitar_update_proc, (void*)ts, "guitar_update");
    if (IS_ERR(thread))
    {
        dev_err(&client->dev, " failed to create update thread\n");
    }
#endif

    for (retry=0; retry < 3; retry++)
    {
        if (success == goodix_init_panel(ts))
        {
            DEBUG_MSG("Initialize successfully!\n");
            break;
        }
    }
    if (retry >= 3)
    {
        ts->bad_data=1;
        DEBUG_MSG("Initialize failed!\n");
        goto err_i2c_failed;
    }
#endif
#if 0
    ts->input_dev = input_allocate_device();
    if (ts->input_dev == NULL)
    {
        ret = -ENOMEM;
        dev_dbg(&client->dev,"goodix_ts_probe: Failed to allocate input device\n");
        goto err_input_dev_alloc_failed;
    }

    ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
    ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);// absolute coor (x,y)

#ifdef HAVE_TOUCH_KEY
    for(retry = 0; retry < MAX_KEY_NUM; retry++)
    {
        input_set_capability(ts->input_dev,EV_KEY,touch_key_array[retry]);
    }
#endif

#ifdef GOODIX_MULTI_TOUCH
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TOUCH_MAX_HEIGHT, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TOUCH_MAX_WIDTH, 0, 0);
#else
    input_set_abs_params(ts->input_dev, ABS_X, 0, TOUCH_MAX_HEIGHT, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_Y, 0, TOUCH_MAX_WIDTH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
#endif    

    memcpy(ts->phys, "input/ts", 8);
    ts->input_dev->name = s3c_ts_name;
    ts->input_dev->phys = ts->phys;
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0xDEAD;
    ts->input_dev->id.product = 0xBEEF;
    ts->input_dev->id.version = 10427;    //screen firmware version

    ret = input_register_device(ts->input_dev);
    if (ret) 
    {
        dev_err(&client->dev,"Probe: Unable to register %s input device\n", ts->input_dev->name);
        goto err_input_register_device_failed;
    }
    DEBUG_MSG("Register input device successfully!\n");
#endif    
#if 0   
    client->irq=TS_INT;        //If not defined in client
    if (client->irq)
    {
        ret = GPIO_REQUEST(INT_PORT, "TS_INT");    //Request IO
        if (ret < 0) 
        {
            dev_err(&client->dev, "Failed to request GPIO:%d, ERRNO:%d\n",(int)INT_PORT,ret);
            goto err_gpio_request_failed;
        }
        DEBUG_MSG("Request int port successfully!\n");
        
        GPIO_DIRECTION_INPUT(INT_PORT);
        GPIO_PULL_UPDOWN(INT_PORT, 0);
        GPIO_CFG_PIN(INT_PORT, INT_CFG);        //Set IO port function    

        ret = request_irq(client->irq, goodix_ts_irq_handler, INT_TRIGGER,
                          client->name, ts);
        if (ret != 0) 
        {
            dev_err(&client->dev,"Cannot allocate ts INT!ERRNO:%d\n", ret);
            GPIO_DIRECTION_INPUT(INT_PORT);
            GPIO_FREE(INT_PORT);
            goto err_gpio_request_failed;
        }
        else 
        {
            disable_irq(client->irq);
            ts->use_irq = 1;
            dev_dbg(&client->dev,"Reques EIRQ %d succesd on GPIO:%d\n",TS_INT,INT_PORT);
        }
    }
#endif    

    ret = request_irq(_sui_irq_num, goodix_ts_irq_handler, 0, "gt828_ts",ts); 
    if (ret < 0) {                                                            
	            dev_err(&client->dev, "gt828_probe: request irq failed\n");      
		    goto err_gpio_request_failed;
    } else {                                                                   
	    disable_irq(_sui_irq_num);                                                
	    ts->use_irq = 1;
    }

err_gpio_request_failed:
    if (!ts->use_irq) 
    {
        hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        ts->timer.function = goodix_ts_timer_func;
        hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
        DEBUG_MSG("Use timer!\n");
    }
//FIXME  PETER
#if 0
    ret = GPIO_REQUEST(RESET_PORT, "TS_RESET");    //Request IO
    if (ret < 0) 
    {
        dev_err(&client->dev, "Failed to request GPIO:%d, ERRNO:%d\n",(int)RESET_PORT,ret);
    }
    else
    {
        ts->use_reset = 1;
        GPIO_DIRECTION_INPUT(RESET_PORT);
        GPIO_PULL_UPDOWN(RESET_PORT, 0);
    }
#endif  
//FIXME peter

#if 1
  //  printk(KERN_INFO "step 0\n");
    msleep(5);
    
    //Test I2C connection.    
    for(retry = 0;retry < 3; retry++)
    {
        ret =i2c_pre_cmd(ts);
        if (!ret)
            break;
        msleep(20);
	retry++;
    }
    if(ret <= 0)
    {
        dev_err(&client->dev, "Warnning: I2C communication might be ERROR!\n");
        DEBUG_MSG("I2C test failed. I2C addr:%x\n", client->addr);
        goto err_init_godix_ts;
    }

   // printk(KERN_INFO "step 1\n");
#ifdef AUTO_UPDATE_GUITAR
    DEBUG_MSG("Ready to run update thread.\n");
    
    get_ic_fw_msg(ts);
    ts->gt_loc = -1;
    thread = kthread_run(guitar_update_proc, (void*)ts, "guitar_update");
    if (IS_ERR(thread))
    {
        dev_err(&client->dev, " failed to create update thread\n");
    }
#endif

    for (retry=0; retry < 3; retry++)
    {
        if (success == goodix_init_panel(ts))
        {
            DEBUG_MSG("Initialize successfully!\n");
            break;
        }
    }
    i2c_end_cmd(ts);
   // printk(KERN_INFO "step 2\n");
    if (retry >= 3)
    {
        ts->bad_data=1;
        DEBUG_MSG("Initialize failed!\n");
        goto err_init_godix_ts;
    }
#endif
#if 1
    ts->input_dev = input_allocate_device();
    if (ts->input_dev == NULL)
    {
        ret = -ENOMEM;
        dev_dbg(&client->dev,"goodix_ts_probe: Failed to allocate input device\n");
        goto err_init_godix_ts;
    }

    ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    //ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
    ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);// absolute coor (x,y)

#ifdef HAVE_TOUCH_KEY
    for(retry = 0; retry < MAX_KEY_NUM; retry++)
    {
        input_set_capability(ts->input_dev,EV_KEY,touch_key_array[retry]);
    }
#endif

    set_bit(EV_ABS, ts->input_dev->evbit);
    set_bit(ABS_MT_TRACKING_ID, ts->input_dev->absbit);
    set_bit(ABS_MT_TOUCH_MAJOR, ts->input_dev->absbit);
    set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, ts->input_dev->absbit);
    set_bit(EV_SYN, ts->input_dev->evbit); 


#ifdef GOODIX_MULTI_TOUCH
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, 30, 0, 0);

    //input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TOUCH_MAX_HEIGHT, 0, 0);
    //input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TOUCH_MAX_WIDTH, 0, 0);
		
    if( item_equal("board.name", "M7030", 0)== 1)
    {
    	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, 1024, 0, 0);
    	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, 600, 0, 0);
		}
		else
		{
	    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
    	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);		
		}

#else
    input_set_abs_params(ts->input_dev, ABS_X, 0, TOUCH_MAX_HEIGHT, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_Y, 0, TOUCH_MAX_WIDTH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
#endif    

    memcpy(ts->phys, "input/ts", 8);
    ts->input_dev->name = "Goodix-TS";
    ts->input_dev->phys = ts->phys;
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0xDEAD;
    ts->input_dev->id.product = 0xBEEF;
    ts->input_dev->id.version = 10427;    //screen firmware version

    ret = input_register_device(ts->input_dev);
    if (ret) 
    {
        dev_err(&client->dev,"Probe: Unable to register %s input device\n", ts->input_dev->name);
        goto err_init_godix_ts;
    }
    DEBUG_MSG("Register input device successfully!\n");

#endif 

#ifdef CONFIG_HAS_EARLYSUSPEND
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    ts->early_suspend.suspend = goodix_ts_early_suspend;
    ts->early_suspend.resume = goodix_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

#ifdef CREATE_WR_NODE
    goodix_proc_entry = create_proc_entry("goodix-update", 0666, NULL);
    if (goodix_proc_entry == NULL)
    {
        dev_info(&client->dev, "Couldn't create proc entry!\n");
        ret = -ENOMEM;
        goto err_create_proc_entry;
    }
    else
    {
        dev_info(&client->dev, "Create proc entry success!\n");
        goodix_proc_entry->write_proc = goodix_update_write;
        goodix_proc_entry->read_proc = goodix_update_read;
        goodix_proc_entry->owner =THIS_MODULE;
    }
#endif

    dev_info(&client->dev,"Start %s in %s mode\n", 
              ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");
#if 0
    {
	    char buf[3];
	    int i;
	    i2c_pre_cmd(ts);
	    i2c_end_cmd(ts);
	    for(i = 0; i < 112; i++)
	    {
		    buf[0] = 0x0f;
		    buf[1] = 0x80 + i;
		    i2c_read_bytes(ts->client, buf, 3);
		    if(!(i%10))
			    printk("\n");
		    printk("0x%02x  ",buf[2]);
	    }
    }
#endif

    if(ts->use_irq)
    {
        enable_irq(client->irq);
    }
    enable_irq(_sui_irq_num);                                                
    //printk(KERN_INFO "kjflahfklahfkjashfkahsnvonoiewnvonebo\n");
    return 0;

err_init_godix_ts:
    if(ts->use_irq)
    {
        ts->use_irq = 0;
        free_irq(_sui_irq_num,ts);
    }
    else 
        hrtimer_cancel(&ts->timer);

err_input_register_device_failed:
    input_free_device(ts->input_dev);
    
err_input_dev_alloc_failed:
err_i2c_failed:

    i2c_set_clientdata(client, NULL);
    kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
#ifdef CREATE_WR_NODE
err_create_proc_entry:
#endif
    DEBUG_MSG("\nprobe failed\n");
    return ret;
}


/*******************************************************	
功能：
	驱动资源释放
参数：
	client：设备结构体
return：
	执行结果码，0表示正常执行
********************************************************/
static int goodix_ts_remove(struct i2c_client *client)
{
    struct goodix_ts_data *ts = i2c_get_clientdata(client);
    
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&ts->early_suspend);
#endif

#ifdef CREATE_WR_NODE
    remove_proc_entry("goodix-update", NULL);
#endif

    if (ts && ts->use_irq) 
    {
        free_irq(client->irq, ts);
    }    
    else if(ts)
        hrtimer_cancel(&ts->timer);
    
    dev_notice(&client->dev,"The driver is removing...\n");
    i2c_set_clientdata(client, NULL);
    input_unregister_device(ts->input_dev);
    input_free_device(ts->input_dev);
    kfree(ts);
    return 0;
}

//停用设备
static int goodix_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret;
    struct goodix_ts_data *ts = i2c_get_clientdata(client);
    disable_irq(_sui_irq_num);                                                
    printk("goodix_ts_suspend\n");
    return 0;

    if (ts->use_irq)
    {
        disable_irq(client->irq);
        ts->irq_flag = 0;
    }
    else
        hrtimer_cancel(&ts->timer);

    if (ts->power) 
    {
        ret = ts->power(ts, 0);
        if (ret < 0)
            DEBUG_MSG(KERN_ERR "goodix_ts_resume power off failed\n");
    }
    return 0;
}

static int goodix_ts_resume(struct i2c_client *client)
{
    int ret;
    struct goodix_ts_data *ts;
    enable_irq(_sui_irq_num);                                                
    printk("goodix_ts_resume\n");
    ts = i2c_get_clientdata(client);
      imapx_pad_set_mode(0, 1, g_pad_index);
      imapx_pad_irq_config(g_pad_index, INTTYPE_FALLING, FILTER_MAX);
    return 0;

    if (ts->power) 
    {
        ret = ts->power(ts, 1);
        if (ret < 0)
            DEBUG_MSG(KERN_ERR "goodix_ts_resume power on failed\n");
    }

    if (ts->use_irq)
    {
        enable_irq(client->irq);
        ts->irq_flag = 1;
    }
    else
        hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

    return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void goodix_ts_early_suspend(struct early_suspend *h)
{
    struct goodix_ts_data *ts;
    ts = container_of(h, struct goodix_ts_data, early_suspend);
    goodix_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void goodix_ts_late_resume(struct early_suspend *h)
{
    struct goodix_ts_data *ts;
    guitar_reset(30);
    ts = container_of(h, struct goodix_ts_data, early_suspend);
    goodix_ts_resume(ts->client);
}
#endif
//******************************Begin of firmware update surpport*******************************

static void guitar_reset(s32 ms)
{
	uint32_t index;
	//index = imapx_pad_index("cond20");
	index = item_integer("ts.int", 1);
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(1,1,index);
	//index = imapx_pad_index("cond9");

	index = item_integer("ts.reset", 1);
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(1,1,index);
	msleep(ms);
	imapx_pad_set_outdat(0,1,index);
	msleep(ms);
	imapx_pad_set_outdat(1,1,index);
	msleep(ms);
//	imapx_pad_set_dir(1,1,index);
/*TODO
    GPIO_DIRECTION_OUTPUT(RESET_PORT, 0);
    GPIO_SET_VALUE(RESET_PORT, 0);
    msleep(ms);

    GPIO_DIRECTION_INPUT(RESET_PORT);
    GPIO_PULL_UPDOWN(RESET_PORT, 0);

    msleep(20);
   */

    return;
}

#ifdef AUTO_UPDATE_GUITAR
static u8 is_equal( u8 *src , u8 *dst , int len )
{
    int i;

    for( i = 0 ; i < len ; i++ )
    {
        if (src[i] != dst[i])
        {
            return false;
        }
    }

    return true;
}

static u8 get_ic_msg(struct goodix_ts_data *ts, u16 addr, u8* msg, s32 len)
{
    s32 i = 0;

    msg[0] = addr >> 8 & 0xff;
    msg[1] = addr & 0xff;

    for (i = 0; i < 5; i++)
    {
        if (i2c_read_bytes(ts->client, msg, ADDR_LENGTH + len) > 0)
        {
            break;
        }
    }

    if (i >= 5)
    {
        DEBUG_UPDATE("Read data from 0x%02x%02x failed!\n", msg[0], msg[1]);
        return fail;
    }

    return success;
}

static u8 clear_mix_flag(struct goodix_ts_data *ts)
{
    s32 i = 0;
    u8 buf[3];
    
    buf[0] = 0x14;
    buf[1] = 0x00;
    buf[2] = 0x80;

    for (i = 0; i < 5; i++)
    {
        if (i2c_write_bytes(ts->client, buf, 3) > 0)
        {
            break;
        }
    }

    if (i >= 5)
    {
        DEBUG_UPDATE("Clear mix flag failed!\n");
        return fail;
    }

    return success;
}

static u8 get_ic_fw_msg(struct goodix_ts_data *ts)
{
    s32 ret = 0;
    u8 buf[32];

    clear_mix_flag(ts);
    
    //Get the mask version in rom of IC
    if (fail == get_ic_msg(ts, READ_MSK_VER_ADDR_H << 8 | READ_MSK_VER_ADDR_L, buf, 4))
    {
        DEBUG_UPDATE("Read mask version failed!\n");
        return fail;
    }
    
    memcpy(ts->ic_fw_msg.msk_ver, &buf[ADDR_LENGTH], 4);
    DEBUG_UPDATE("IC The mask version in rom is %c%c%c%c.\n",
                 ts->ic_fw_msg.msk_ver[0],ts->ic_fw_msg.msk_ver[1],
                 ts->ic_fw_msg.msk_ver[2],ts->ic_fw_msg.msk_ver[3]);

    //Cuts the frequency
    buf[0] = 0x15;
    buf[1] = 0x22;
    buf[2] = 0x18;
    ret =  i2c_write_bytes(ts->client, buf, 3);
    if (ret <= 0)
    {
        return fail;
    }
    
    //Get the pid at 0x4011 in nvram
    if (fail == get_ic_msg(ts, 0x4011, buf, 1))
    {
        DEBUG_UPDATE("Read pid failed!\n");
        return fail;
    }
    ts->ic_fw_msg.type = buf[ADDR_LENGTH];

    guitar_reset(10);
    return success;
}

/*
* Steps of reset guitar
*1. INT脚输出低，延时5ms
*2. RESET脚拉低100ms，转输入悬浮态
*3. I2C寻址GUITAR
*4. 延时100ms读取0xff(3、4轮询80次，直至成功)
*5. Oxff等于0x55则返回成功，否则失败
*/
static int guitar_update_mode( struct goodix_ts_data *ts )
{
    int ret = 1;
    u8 retry;
    unsigned char inbuf[3] = {0,0xff,0};

    // step 1
    GPIO_DIRECTION_OUTPUT(INT_PORT, 0);
    GPIO_SET_VALUE(INT_PORT, 0);
    msleep(5);

    //step 2
    guitar_reset(100);

    for(retry=0;retry < 80; retry++)
    {
        //step 3
        ret =i2c_write_bytes(ts->client, inbuf, 0);    //Test I2C connection.
        if (ret > 0)
        {
            DEBUG_UPDATE("<Set update mode>I2C is OK!\n");
            //step 4
            msleep(100);
            ret =i2c_read_bytes(ts->client, inbuf, 3);
            if (ret > 0)
            {
                DEBUG_UPDATE("The value of 0x00ff is 0x%02x\n", inbuf[2]);
                //step 5
                if(inbuf[2] == 0x55)
                {
                    return success;
                }
            }
        }
    }
    DEBUG_UPDATE(KERN_INFO"Detect address %0X\n", ts->client->addr);

    return fail;
}

static void guitar_leave_update_mode(void)
{
    GPIO_DIRECTION_INPUT(INT_PORT);
    GPIO_PULL_UPDOWN(INT_PORT, 0);
}

u8 load_update_file(struct goodix_ts_data *ts, st_fw_head* fw_head, u8* data)
{
    u8 mask_num = 0;
    int ret = 0;
    int i = 0;
    u8 buf[FW_HEAD_LENGTH];
#if 0
    for (i = 0; i< 5; i++)
    {
        if (success== guitar_update_mode(ts))
        {
            DEBUG_UPDATE("Set update mode in load file successfully.\n");
            break;;
        }
    }
    if (i >= 5)
    {
        DEBUG_UPDATE("Set update mode in load file failed.\n");
        guitar_leave_update_mode();
        return fail;
    }

    //Get the mask version in rom of IC
    if (fail == get_ic_msg(ts, READ_MSK_VER_ADDR_H << 8 | READ_MSK_VER_ADDR_L, buf, 4))
    {
        guitar_leave_update_mode();

        DEBUG_UPDATE("Read mask version failed!\n");
        return fail;
    }
    
    memcpy(ic_fw_msg.msk_ver, &buf[ADDR_LENGTH], 4);
    DEBUG_UPDATE("IC The mask version in rom is %c%c%c%c.\n",
                 ic_fw_msg.msk_ver[0],ic_fw_msg.msk_ver[1],
                 ic_fw_msg.msk_ver[2],ic_fw_msg.msk_ver[3]);

    //Get the pid at 0x4011 in nvram
    if (fail == get_ic_msg(ts, 0x4011, buf, 4))
    {
        guitar_leave_update_mode();

        DEBUG_UPDATE("Read pid failed!\n");
        return fail;
    }
    ic_fw_msg.type = buf[ADDR_LENGTH];

    guitar_leave_update_mode();
#endif 
    //Begin to search update file
    for (i = 0; i < SEARCH_FILE_TIMES; i++)
    {
        ts->file = filp_open(UPDATE_FILE_PATH_1, O_RDWR, 0644);
        if (IS_ERR(ts->file))
        { 
            ts->file = filp_open(UPDATE_FILE_PATH_2, O_RDWR, 0644);//O_RDWR
            if (IS_ERR(ts->file))
            {
                DEBUG_UPDATE("%3d:Searching file...\n", i);
                msleep(3000);
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    if (i >= 100)
    {
        DEBUG_UPDATE("Can't find update file.\n");
        return fail;
    }
    DEBUG_UPDATE("Find the update file.\n");

    ts->old_fs = get_fs();
    set_fs(KERNEL_DS);

    ts->file->f_pos = IGNORE_LENGTH;

    //Make sure the file is the right file.(By compare the "Guitar" flag)
    ret = ts->file->f_op->read(ts->file, (char*)&buf, 6, &ts->file->f_pos);
    if (ret < 0)
    {
        DEBUG_UPDATE("Read \"Guitar\" flag error.\n");
        goto load_failed;
    }
    if (false == is_equal(buf, "Guitar", 6))
    {
        DEBUG_UPDATE("The flag is %s.Not equal!\n"
                     "The update file is incorrect!\n", buf);
        goto load_failed; 
    }
    DEBUG_UPDATE("The file flag is :%s.\n", buf);
    
    //Get the total number of masks
    ts->file->f_pos++; //ignore one byte.
    ret = ts->file->f_op->read(ts->file, &mask_num, 1, &ts->file->f_pos);
    if (ret < 0)
    {
        DEBUG_UPDATE("Didn't get the mask number from the file.\n");
        goto load_failed;
    }
    DEBUG_UPDATE("FILE The total number of masks is:%d.\n", mask_num);
    ts->file->f_pos = FILE_HEAD_LENGTH + IGNORE_LENGTH;

    //Get the firmware msg in IC, include firmware version and checksum flag
    for (i = 0; i < 2; i++)
    {
        if (fail == get_ic_msg(ts, READ_FW_MSG_ADDR_H<< 8 | READ_FW_MSG_ADDR_L, buf, 4))
        {
            DEBUG_UPDATE("Get firmware msg in IC error.\n");
            goto load_failed;
        }
        ts->force_update = buf[ADDR_LENGTH];
        if (i == 0 && ts->force_update == 0xAA)
        {
            DEBUG_UPDATE("The check sum in ic is error.\n");
            DEBUG_UPDATE("IC will be reset.\n");
            DEBUG_UPDATE("If the check sum is still error,\n ");
            DEBUG_UPDATE("The IC will be updated by force.\n");

            guitar_reset(10);
            msleep(100);
        }
    }
    //ic_fw_msg.type = buf[ADDR_LENGTH + 1];
    ts->ic_fw_msg.version = buf[ADDR_LENGTH + 2] << 8 | buf[ADDR_LENGTH + 3];
    DEBUG_UPDATE("IC PID:%x\n", ts->ic_fw_msg.type);
    DEBUG_UPDATE("IC VID:0x%x\n", (int)ts->ic_fw_msg.version);
    DEBUG_UPDATE("IC force update:%x\n", ts->force_update);
#if 0    
    //Get the mask version in rom of IC
    buf[0] = READ_MSK_VER_ADDR_H;
    buf[1] = READ_MSK_VER_ADDR_L;
    ret = i2c_read_bytes(ts->client, buf, ADDR_LENGTH + 4);
    memcpy(ic_fw_msg.msk_ver, &buf[ADDR_LENGTH], 4);
    DEBUG_UPDATE("IC The mask version in rom is %2x %2x %2x %2x.\n",
                 ic_fw_msg.msk_ver[0],ic_fw_msg.msk_ver[1],
                 ic_fw_msg.msk_ver[2],ic_fw_msg.msk_ver[3]);
#endif 
    //Get the correct nvram data
    //The correct conditions: 
    //1. the product id is the same
    //2. the mask id is the same
    //3. the nvram version in update file is greater than the nvram version in ic 
    //or force update flag is marked or the check sum in ic is wrong
    ts->gt_loc = -1;
    for ( i = 0; i < mask_num; i++)
    {        
        ret = ts->file->f_op->read(ts->file, (char*)buf, FW_HEAD_LENGTH, &ts->file->f_pos);
        if (ret < 0)
        {
            DEBUG_UPDATE("Read update file head error.\n");
            goto load_failed;
        }
        memcpy(fw_head, buf, sizeof(st_fw_head));
        fw_head->version = buf[1] << 8 | buf[2];
        fw_head->lenth = buf[9] << 8 | buf[10];
        DEBUG_UPDATE("No.%d firmware\n", i);
        DEBUG_UPDATE("FILE PID:%x\n", fw_head->type);
        DEBUG_UPDATE("FILE VID:0x%x\n", fw_head->version);
        DEBUG_UPDATE("FILE mask version:%c%c%c%c.\n", fw_head->msk_ver[0],
                     fw_head->msk_ver[1],fw_head->msk_ver[2],fw_head->msk_ver[3]);
        DEBUG_UPDATE("FILE start address:0x%02x%02x.\n", fw_head->st_addr[0], fw_head->st_addr[1]);
        DEBUG_UPDATE("FILE length:%d\n", (int)fw_head->lenth);
        DEBUG_UPDATE("FILE force update flag:%s\n", fw_head->force_update);
        DEBUG_UPDATE("FILE chksum:0x%02x%02x%02x\n", fw_head->chk_sum[0], 
                                 fw_head->chk_sum[1], fw_head->chk_sum[2]);

        //First two conditions
        if (is_equal(fw_head->msk_ver, ts->ic_fw_msg.msk_ver, sizeof(ts->ic_fw_msg.msk_ver))
            && ts->ic_fw_msg.type == fw_head->type)
        {
            DEBUG_UPDATE("Get the same mask version and same pid.\n");
            //The third condition
            if (fw_head->version > ts->ic_fw_msg.version
                || is_equal(fw_head->force_update, "GOODIX", 6) 
                || ts->force_update == 0xAA)
            {
               // DEBUG_UPDATE("FILE read position:%d\n", file->f_pos);
               // file->f_pos = FW_HEAD_LENGTH + FILE_HEAD_LENGTH + IGNORE_LENGTH;

                if (is_equal(fw_head->force_update, "GOODIX", 6))
                {
                    ts->gt_loc = ts->file->f_pos - FW_HEAD_LENGTH + sizeof(st_fw_head) - sizeof(fw_head->force_update);
                }
                
                ret = ts->file->f_op->read(ts->file, (char*)data, fw_head->lenth, &ts->file->f_pos);
                if (ret <= 0)
                {
                    DEBUG_UPDATE("Read firmware data in file error.\n");
                    goto load_failed;
                }
               // DEBUG_ARRAY(data, 512);
               // set_fs(ts->old_fs);
              //  filp_close(ts->file, NULL);
                DEBUG_UPDATE("Load data from file successfully.\n");
                return success;
            }
            DEBUG_UPDATE("Don't meet the third condition.\n");
            goto load_failed;
        }

        ts->file->f_pos += UPDATE_DATA_LENGTH;
    }

load_failed:    
    set_fs(ts->old_fs);
    filp_close(ts->file, NULL);
    return fail;
}

static u8 guitar_nvram_store( struct goodix_ts_data *ts )
{
    int ret;
    int i;
    u8 inbuf[3] = {REG_NVRCS_H,REG_NVRCS_L,0};

    ret = i2c_read_bytes(ts->client, inbuf, 3);
    if ( ret < 0 )
    {
        return fail;
    }

    if ((inbuf[2] & BIT_NVRAM_LOCK ) == BIT_NVRAM_LOCK)
    {
        return fail;
    }

    inbuf[2] = (1<<BIT_NVRAM_STROE);        //store command

    for ( i = 0 ; i < 300 ; i++ )
    {
        ret = i2c_write_bytes( ts->client, inbuf, 3 );
        if ( ret > 0 )
            return success;
    }

    return fail;
}

static u8 guitar_nvram_recall( struct goodix_ts_data *ts )
{
    int ret;
    u8 inbuf[3] = {REG_NVRCS_H,REG_NVRCS_L,0};

    ret = i2c_read_bytes( ts->client, inbuf, 3 );
    if ( ret < 0 )
    {
        return fail;
    }

    if ( ( inbuf[2]&BIT_NVRAM_LOCK) == BIT_NVRAM_LOCK )
    {
        return fail;
    }

    inbuf[2] = ( 1 << BIT_NVRAM_RECALL );        //recall command
    ret = i2c_write_bytes( ts->client , inbuf, 3);

    if (ret <= 0)
    {
        return fail;
    }
    return success;
}

static u8 guitar_update_nvram(struct goodix_ts_data *ts, st_fw_head* fw_head, u8 *nvram)
{
    int length = 0;
    int ret = 0;
    int write_bytes = 0;
    int retry = 0;
    int i = 0;
    int comp = 0;
    u16 st_addr = 0;
    u8 w_buf[PACK_SIZE + ADDR_LENGTH];
    u8 r_buf[PACK_SIZE + ADDR_LENGTH];

    if (fw_head->lenth > PACK_SIZE)
    {
        write_bytes = PACK_SIZE;
    }
    else
    {
        write_bytes = fw_head->lenth;
    }

    st_addr = (fw_head->st_addr[0] << 8) | (fw_head->st_addr[1]&0xff);
    memcpy(&w_buf[2], &nvram[length], write_bytes);
    DEBUG_UPDATE("Total length:%d\n", (int)fw_head->lenth);
    while(length < fw_head->lenth)
    {
        w_buf[0] = st_addr >> 8;
        w_buf[1] = st_addr & 0xff;
        DEBUG_UPDATE("Write address:0x%02x%02x\tlength:%d\n", w_buf[0], w_buf[1], write_bytes);
        ret =  i2c_write_bytes(ts->client, w_buf, ADDR_LENGTH + write_bytes);
        if (ret <= 0)
        {
            if (retry++ > 10)
            {
                DEBUG_UPDATE("Write the same address 10 times.Give up!\n");
                return fail;
            }
            continue;
        }
        else
        {
            DEBUG_UPDATE("w_buf:\n");
            DEBUG_ARRAY(w_buf, ADDR_LENGTH + write_bytes);
            r_buf[0] = 0x14;
            r_buf[1] = 0x00;
            r_buf[2] = 0x9e;
            i2c_write_bytes(ts->client, r_buf, 3);
            r_buf[0] = 0x14;
            r_buf[1] = 0x00;
            i2c_read_bytes(ts->client, r_buf, 3);
            DEBUG_UPDATE("I2CCS:0x%x\n", r_buf[2]);
            
            r_buf[0] = w_buf[0];
            r_buf[1] = w_buf[1];

            for (i = 0; i < 10; i++)
            {
                ret = i2c_read_bytes(ts->client, r_buf, ADDR_LENGTH + write_bytes);
                if (ret <= 0)
                {
                    continue;
                }
                break;
            }
            if (i >= 10)
            {
                DEBUG_UPDATE("Read error! Can't check the nvram data.\n");
                return fail;
            }
            DEBUG_UPDATE("r_buf:\n");
            DEBUG_ARRAY(r_buf, ADDR_LENGTH + write_bytes);
#if 0            
            if (fail == guitar_nvram_store(ts))
            {
                DEBUG_UPDATE("Store nvram failed.\n");
                //continue;
            }
            return fail;
#endif
            if (false == is_equal(r_buf, w_buf, ADDR_LENGTH + write_bytes))
            {   
                if (comp ++ > 10)
                {
                    DEBUG_UPDATE("Compare error!\n");
                    return fail;
                }
                DEBUG_UPDATE("Updating nvram: Not equal!\n");
                continue;
                //return fail;
            }
        }
        comp = 0;
        retry = 0;
        length += PACK_SIZE;
        st_addr += PACK_SIZE;
        if ((length + PACK_SIZE) > fw_head->lenth)
        {
            write_bytes = fw_head->lenth - length;
        }
        memcpy(&w_buf[2], &nvram[length], write_bytes);
    }

    return success;
}

static u8 guitar_update_firmware(struct goodix_ts_data *ts, st_fw_head* fw_head, u8 *nvram)
{
    int retry;
    int ret;
    u32 status = 0;
    u8 buf[32];

    //Cuts the frequency
    buf[0] = 0x15;
    buf[1] = 0x22;
    buf[2] = 0x18;
    ret =  i2c_write_bytes(ts->client, buf, 3);
    if (ret <= 0)
    {
        return fail;
    }

    for (retry = 0; retry < 30; retry++)
    {
        //Write the 1st part (pid and vid)
        if (!(status & 0x01))
        {
            buf[0] = UPDATE_FW_MSG_ADDR_H;
            buf[1] = UPDATE_FW_MSG_ADDR_L;
            buf[2] = fw_head->type;
            buf[3] = fw_head->version >> 8;
            buf[4] = fw_head->version & 0xff;
            ret = i2c_write_bytes(ts->client, buf, 5);
            if (ret <= 0)
            {
                continue;
            }
            else
            {
                DEBUG_UPDATE("Update pid and vid successfully!\n");
                status |= 0x01;
                msleep(1);
            }
        }

        //Write the 2nd part (nvram)
        if (!(status & 0x02))
        {
            if (fail == guitar_update_nvram(ts, fw_head, nvram))
            {
                continue;
            }
            else
            {
                DEBUG_UPDATE("Update nvram successfully!\n");
                status |= 0x02;
                msleep(1);
            }
        }

        //Write the 3rd part (check sum)
        if (1)
        {
            buf[0] = 0x4f;
            buf[1] = 0xf3;
            memcpy(&buf[2], fw_head->chk_sum, sizeof(fw_head->chk_sum));
            ret = i2c_write_bytes(ts->client, buf, 5);
            if (ret <= 0)
            {
                continue;
            }
            else
            {
                DEBUG_UPDATE("Update check sum successfully!\n");
                break;
            }
        }
    }

    if (retry >= 80)
    {
        return fail;
    }
    else
    {
        for (retry = 0; retry < 10; retry++)
        {
            buf[0] = 0x00;
            buf[1] = 0xff;
            buf[2] = 0x44;
            ret = i2c_write_bytes(ts->client, buf, 3);
            if (ret > 0)
            {
                break;
            }
        }

        if (retry >= 10)
        {
            return fail;
        }
        msleep(10);
    }

    ret =  i2c_read_bytes(ts->client, buf, 3);
    if (ret <= 0)
    {
        return fail;
    }

    if (0xcc == buf[2])
    {
        return success;
    }

    return fail;
}

static int guitar_update_proc(void *v_ts)
{
#if 0
    
    struct goodix_ts_data* ts = (struct goodix_ts_data*)v_ts;
    while (1)
    {
        
        DEBUG_UPDATE("Start\n");
        DEBUG_UPDATE("Disable int.\n");
        msleep(1000);
        gpio_direction_output(INT_PORT, 0);
        gpio_set_value(INT_PORT, 0);
        msleep(5);

        guitar_reset();
        msleep(10000);
        
        DEBUG_UPDATE("Enable int!\n");
        gpio_set_value(INT_PORT, 1);
        gpio_direction_input(INT_PORT);
        GPIO_PULL_UPDOWN(INT_PORT, 0);
       // GPIO_CFG_PIN(INT_PORT, INT_CFG);
        msleep(10);
       /* guitar_reset();
        msleep(100);
        goodix_init_panel(ts);
        msleep(1000);

        msleep(2000);//*/
        msleep(5000);
        DEBUG_UPDATE("Stop!\n");
    }
#else
    u8 ret;
    u32 retry = 100;
    u32 i = 0;
    struct goodix_ts_data* ts = (struct goodix_ts_data*)v_ts;
    u8* data = NULL;
    u8* ic_nvram = NULL;
    st_fw_head fw_head;
    u8 buf[32];

    data = kzalloc(UPDATE_DATA_LENGTH, GFP_KERNEL);
    if (NULL == data)
    {
        DEBUG_UPDATE("data failed apply for memory.\n");
        return fail;
    }
    
    ic_nvram = kzalloc(UPDATE_DATA_LENGTH, GFP_KERNEL);
    if (NULL == ic_nvram)
    {
        DEBUG_UPDATE("ic_nvram failed apply for memory.\n");
        goto app_mem_failed;
    }
    DEBUG_UPDATE("Apply for memory successfully.memory size: %d.\n", UPDATE_DATA_LENGTH);

    msleep(1000);
    DEBUG_UPDATE("Updating...\n");

    if (fail == load_update_file(ts, &fw_head, &data[2]))
    {
        DEBUG_UPDATE("Load file data failed!\n");
        goto load_failed;
    }
    DEBUG_UPDATE("Load file data successfully!\n");
    disable_irq(ts->client->irq);

    retry = 0;
    while(retry++ < 5)
    {
        if (fail == guitar_update_mode(ts))
        {
            DEBUG_UPDATE("Set update mode failed.\n");
            continue;
        }
        DEBUG_UPDATE("Set update mode successfully.\n");

        if (fail == guitar_update_firmware(ts, &fw_head, &data[2]))
        {
            DEBUG_UPDATE("Update firmware failed.\n");
            continue;
        }
        DEBUG_UPDATE("Update firmware successfully.\n");

        if (fail == guitar_nvram_store(ts))
        {
            DEBUG_UPDATE("Store nvram failed.\n");
            continue;
        }
        msleep(100);
#if 0        
        ic_nvram[0] = 0x12;
        ic_nvram[1] = 0x01;
        i2c_read_bytes(ts->client, ic_nvram, 3);
        DEBUG_UPDATE("NVRCS:0x%02x\n", ic_nvram[2]);
#else
        if (fail == get_ic_msg(ts, 0x1201, buf, 3))
        {
            DEBUG_UPDATE("Read NVRCS failed.(Store)\n");
            continue;
        }
        if (!(buf[ADDR_LENGTH] && 0x01))
        {
            DEBUG_UPDATE("Check NVRCS failed.(Store)\n");
            continue;
        }
#endif
        DEBUG_UPDATE("Store nvram successfully.\n");

        if (fail == guitar_nvram_recall(ts))
        {
            DEBUG_UPDATE("Recall nvram failed.\n");
            continue;
        }
        msleep(5);
#if 0        
        ic_nvram[0] = 0x12;
        ic_nvram[1] = 0x01;
        i2c_read_bytes(ts->client, ic_nvram, 3);
        DEBUG_UPDATE("NVRCS:0x%02x\n", ic_nvram[2]);
#else
        
        if (fail == get_ic_msg(ts, 0x1201, buf, 3))
        {
            DEBUG_UPDATE("Read NVRCS failed.(Recall)\n");
            continue;
        }
        if (!(buf[ADDR_LENGTH] && 0x02))
        {
            DEBUG_UPDATE("Check NVRCS failed.(Recall)\n");
            continue;
        }
#endif
        DEBUG_UPDATE("Recall nvram successfully.\n");

        ic_nvram[0] = fw_head.st_addr[0];
        ic_nvram[1] = fw_head.st_addr[1];

        for ( i = 0; i < 10; i++)
        {
            ret = i2c_read_bytes(ts->client, ic_nvram, ADDR_LENGTH + fw_head.lenth);
            if (ret <= 0)
            {
                continue;
            }
            break;
        }

        if (i >= 10)
        {
            DEBUG_UPDATE("Read nvram failed!\n");
            continue;
        }
        DEBUG_UPDATE("Read nvram successfully!\n");

        if (false == is_equal(&data[2], &ic_nvram[2], fw_head.lenth))
        {
            DEBUG_UPDATE("Nvram not equal!\n");
            continue;
        }
        if (ts->gt_loc > 0)
        {
            memset(buf, 0, sizeof(buf));
            ret = ts->file->f_op->write(ts->file, buf, 6, (loff_t*)&ts->gt_loc);
            if (ret < 0)
            {
                DEBUG_UPDATE("Didn't clear the focre update flag in file.\n");
            }
        }
        set_fs(ts->old_fs);
        filp_close(ts->file, NULL);
        DEBUG_UPDATE("Update successfully!\n");
        break;
    }

    guitar_leave_update_mode();
    
    //Reset guitar
    guitar_reset(10);
    msleep(100);
    for (i = 0; i < 3; i++)
    {
        if (fail == goodix_init_panel(ts))
        {
            msleep(10);
            continue;
        }
        break;
    }
    if (i >= 3)
    {
        DEBUG_UPDATE("Send config data failed.\n");
    }
    
    msleep(10);
    enable_irq(ts->client->irq);
load_failed:
    kfree(ic_nvram);
app_mem_failed:
    kfree(data);

    if (retry < 5)
    {
        return success;
    }
    return fail;
#endif
    
}
#endif   //endif AUTO_UPDATE_GUITAR
//******************************End of firmware update surpport*******************************

//可用于该驱动的 设备名—设备ID 列表
//only one client
static const struct i2c_device_id goodix_ts_id[] = {
    { GOODIX_I2C_NAME, 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, goodix_ts_id);

//设备驱动结构体
static struct i2c_driver goodix_ts_driver = {
    .probe      = goodix_ts_probe,
    .remove     = goodix_ts_remove,
//#ifndef CONFIG_HAS_EARLYSUSPEND
  //  .suspend    = goodix_ts_suspend,
  //  .resume     = goodix_ts_resume,
//#endif
    .id_table    = goodix_ts_id,
    .driver     = {
        .name  = GOODIX_I2C_NAME,
//        .owner = THIS_MODULE,
    },
};



/*
func:
this func only use for I2C device detect , if exit return 1, or return 0;
*/
extern int ctp_auto_detect;
#define LOG printk

static int DoI2cScanDetectDev( void )
{

	if(ctp_auto_detect ==1)
		return 0;
		
struct i2c_adapter *adapter=NULL;
__u16 i2cAddr=0x5d;//slave addr
__u8 subByteAddr=0x0;//fix to read 0
char databuf[8];
int ret;

#define I2C_DETECT_TRY_SIZE  5
int i,i2cDetecCnt=0;

struct i2c_msg I2Cmsgs[2] = 
	{ 
		{
			.addr	= i2cAddr,
			.flags	= 0,
			.len	= 1,
			.buf	= &subByteAddr,
		},
		{
			.addr	= i2cAddr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= databuf,
		}
	};

	LOG("A%s\n",__FUNCTION__);

	if(item_exist("ts.model"))
	{
		if(item_equal("ts.model", "auto", 0))
		{
			//initDev
			guitar_reset(10);
			LOG("B%s\n",__FUNCTION__);
			//get i2c handle	
			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
			if (!adapter) {
				printk("****** get_adapter error! ******\n");
				return 0;
			}
			LOG("C%s\n",__FUNCTION__);
			//scan i2c 
			i2cDetecCnt=0;
			for(i=0;i<I2C_DETECT_TRY_SIZE;i++)
			{
				ret=i2c_transfer(adapter,I2Cmsgs,2);
				LOG("D-%s,ret=%d\n",__FUNCTION__,ret);
				if(ret>0)
					i2cDetecCnt++;				
			}
			LOG("E-%s,thd=%d,%d\n",__FUNCTION__,(I2C_DETECT_TRY_SIZE/2),i2cDetecCnt);
			if(i2cDetecCnt<=(I2C_DETECT_TRY_SIZE/2)){
				return 0;
			}else {
				ctp_auto_detect =1;
				return 1;
			}
		}
	}
	return 0;
}




/*******************************************************	
功能：
	驱动加载函数
return：
	执行结果码，0表示正常执行
********************************************************/
static int __init goodix_ts_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;  
	int ret;

    printk(KERN_INFO "[CTP-GT828]-Enter %s\n", __func__);

   	if(item_exist("ts.model"))
	{
		if(item_equal("ts.model", "gt828", 0) || DoI2cScanDetectDev() )
		{
			if( item_equal("board.name", "ruiyida101inch", 0)== 1)
         		 {}
			else
			{
				SCREEN_MAX_X = get_lcd_width();
				SCREEN_MAX_Y = get_lcd_height();
			}
			
			
    			goodix_wq = create_workqueue("goodix_wq");        //create a work queue and worker thread
    			if (!goodix_wq)
    			{
        			printk(KERN_ERR "[CTP-GT828]-creat workqueue faiked\n");
        			return -ENOMEM;
    			}
    			memset(&info, 0, sizeof(struct i2c_board_info));      
    			info.addr = 0x5d;                                     
    			strlcpy(info.type, "Goodix-TS", I2C_NAME_SIZE);       

    			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));     
    			if (!adapter) {
	    			printk("****** get_adapter error! ******\n"); 
    			}
    			client = i2c_new_device(adapter, &info);

    			ret = i2c_add_driver(&goodix_ts_driver);   
    			printk(KERN_INFO "[CTP-GT828]-i2c_add_driver return %d\n",ret);
    			return ret;
		}
		else
			printk("%s: touchscreen is not gt828\n", __func__);
	}
	else
		printk("%s: touchscreen is not exist\n", __func__);

	return -1;
}

/*******************************************************	
功能：
	驱动卸载函数
参数：
	client：设备结构体
********************************************************/
static void __exit goodix_ts_exit(void)
{
    DEBUG_MSG(KERN_ALERT "Touchscreen driver of guitar exited.\n");
    i2c_del_driver(&goodix_ts_driver);
    if (goodix_wq)
        destroy_workqueue(goodix_wq);        //release our work queue
}

late_initcall(goodix_ts_init);                //最后初始化驱动
module_exit(goodix_ts_exit);

MODULE_DESCRIPTION("Goodix Touchscreen Driver");
MODULE_AUTHOR("<duxx@Focaltech-systems.com>");
MODULE_LICENSE("GPL");
               
