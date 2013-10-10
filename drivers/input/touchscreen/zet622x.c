/*   drivers/input/touchscreen/zet6221_i2c.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * ZEITEC Semiconductor Co., Ltd
 * Tel: +886-3-579-0045
 * Fax: +886-3-579-9960
 * http://www.zeitecsemi.com
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/async.h>
//#include <mach/gpio.h>
#include <linux/irq.h>
//#include <mach/board.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <mach/pad.h>
#include <mach/items.h>

//#include "zet6221_fw.h"

/* -------------- global variable definition -----------*/
#define _MACH_MSM_TOUCH_H_

#define ZET_TS_ID_NAME "zet6221-ts"

#define MJ5_TS_NAME "zet6221_touchscreen"

#define TPINFO	0
#define X_MAX	1088//896//800//960//800
#define Y_MAX	640//480//640//480
#define FINGER_NUMBER 10
#define KEY_NUMBER 3
#define P_MAX	20
#define DEBOUNCE_NUMBER	    1
#define D_POLLING_TIME	25000
#define U_POLLING_TIME	25000
#define S_POLLING_TIME  100
#define REPORT_POLLING_TIME  5
#define PRESS_MAX 255

#define MAX_KEY_NUMBER      	8
#define MAX_FINGER_NUMBER	16
#define TRUE 		1
#define FALSE 		0

#define debug_mode  0 
#define DPRINTK(fmt,args...)	do { if (debug_mode) printk(KERN_EMERG "[%s][%d] "fmt"\n", __FUNCTION__, __LINE__, ##args);} while(0)

//#define TRANSLATE_ENABLE 1
#define TOPRIGHT 	0
#define TOPLEFT  	1
#define BOTTOMRIGHT	2
#define BOTTOMLEFT	3
#define ORIGIN		BOTTOMLEFT

/**************Opens the charging mode 2012-09-07 suixing**************************/
#define CHARGER_MODE_RUN
int probe_finished = 0;
static int ChargeChange = 0;
extern int isacon(void);
/**********************end********************************************************/

struct msm_ts_platform_data {
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
};

struct zet6221_tsdrv {
	struct i2c_client *i2c_ts;
	struct work_struct work1;
	struct delayed_work work2;//suixing 2012-09-07 write_cmd
	struct input_dev *input;
	struct workqueue_struct *ts_workqueue;
	struct timer_list polling_timer;
	struct early_suspend early_suspend;
	unsigned int gpio; /* GPIO used for interrupt of TS1*/
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int pressure_max;
};

struct zet6221_tsdrv *zet6221_ts;
static u16 polling_time = S_POLLING_TIME;

static int __devinit zet6221_ts_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit zet6221_ts_remove(struct i2c_client *dev);

static unsigned char zeitec_zet622x_page[131];
static unsigned char zeitec_zet622x_page_in[131];

s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length);
s32 zet6221_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length);

static u8  key_menu_pressed = 0x2;
static u8  key_back_pressed = 0x1;
static u8  key_home_pressed = 0x4;
static u8  key_search_pressed = 0x8;

static u16 ResolutionX = 1088;//X_MAX;
static u16 ResolutionY = 640;//Y_MAX;
static u16 FingerNum=0;
static u16 KeyNum=0;
static int bufLength=0;
static u8 inChargerMode=0;
static u8 xyExchange=0;
static int f_up_cnt=0;
static int orient_num = 1 ;

/********************downloader suixing 2012-09-12************/
#include "zet6221_fw.h"
#include "zet6223_fw.h"
static char downloader_name[ITEM_MAX_LEN] = "zet6221";
static u8 pc[8];
static u16 fb[8] = {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; 
static u8 ic_model = 0;// 0:6221 / 1:6223
extern u8  zet622x_ts_version();
static u16 fb21[8] = {0x3DF1,0x3DF4,0x3DF7,0x3DFA,0x3EF6,0x3EF9,0x3EFC,0x3EFF}; 
static u16 fb23[8] = {0x7BFC,0x7BFD,0x7BFE,0x7BFF,0x7C04,0x7C05,0x7C06,0x7C07};
//20120927 make firmwares into a header file
unsigned char *zeitec_zet6221_firmware;
int firmware_len;
/***************************end*****************************/

static struct i2c_client *this_client;
static int tsp_irq_index = 74;
static unsigned int _sui_irq_num;


void ctp_reset(void);
// {IC Model, FW Version, FW version,Codebase Type=0x08, Customer ID, Project ID, Config Board No, Config Serial No}

//Touch Screen
static const struct i2c_device_id zet6221_ts_idtable[] = {
       { ZET_TS_ID_NAME, 0 },
       { }
};

static int ts_early_suspend(struct i2c_client *client)
{
//	disable_irq(_sui_irq_num);
    	return 0;
}

static int ts_late_resume(struct i2c_client *client)
{
    imapx_pad_set_mode(0, 1, tsp_irq_index);
    imapx_pad_irq_config(tsp_irq_index, INTTYPE_FALLING, FILTER_MAX);
    ctp_reset();
    ChargeChange = 0;
    return 0;
}

/*************************zet622x downloader suixing 2012-09-12*******************************/

u8 zet622x_ts_sndpwd(struct i2c_client *client)
{
	u8 ts_sndpwd_cmd[3] = {0x20,0xC5,0x9D};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_sndpwd_cmd, 3);

	return ret;
}

u8 zet622x_ts_option(struct i2c_client *client)
{

	u8 ts_cmd[1] = {0x27};
	u8 ts_in_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int ret;
	u16 model;
	int i;
	
	printk("\noption write : "); 

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(1);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\nread : "); 

	ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);

	msleep(1);

	for(i=0;i<16;i++)
	{
		printk("%02x ",ts_in_data[i]); 
	}
	printk("\n"); 

	model = 0x0;
	model = ts_in_data[7];
	model = (model << 8) | ts_in_data[6];
	switch(model) { 
        case 0xFFFF: 
        	ret = 1;
           	ic_model = 0;
	    	printk("ic_model = %d******* zet6221 start downloader\n", ic_model);
	   	for(i=0;i<8;i++)
			{
				fb[i]=fb21[i];
			}
            		printk("\n");
			break; 
			
        case 0x6223:
        	ret = 1;
			ic_model = 1;
			printk("ic_model = %d******* zet6223 start downloader\n", ic_model);
			for(i=0;i<8;i++)
			{
				fb[i]=fb23[i];
			}
            break; 
        default: 
            ret=0; 
    } 

	return ret;
}

u8 zet622x_ts_sfr(struct i2c_client *client)
{

	u8 ts_cmd[1] = {0x2C};
	u8 ts_in_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 ts_cmd17[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	u8 ts_sfr_data[16] = {0x18,0x76,0x27,0x27,0xFF,0x03,0x8E,0x14,0x00,0x38,0x82,0xEC,0x00,0x00,0x7d,0x03};
	int ret;
	int i;
	
	printk("\nwrite : "); 

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	msleep(1);
	
	printk("%02x ",ts_cmd[0]); 
	
	printk("\nread : "); 

	ret=zet6221_i2c_read_tsdata(client, ts_in_data, 16);

	msleep(1);

	for(i=0;i<16;i++)
	{
		ts_cmd17[i+1]=ts_in_data[i];
		printk("%02x ",ts_in_data[i]); 
	}
//	printk("\n"); 

#if 1
	if(ts_in_data[14]!=0x3D && ts_in_data[14]!=0x7D)
	{
		return 0;
	}
#endif

/*
#if 1
		if (i >1 && i<8)
		{
			if(ts_in_data[i]!=ts_sfr_data[i]){
				printk("***** ts_in_data[i] = %x\n", ts_in_data[i]);
				return 0;
			}
		}
#endif
*///	}
	if(ts_in_data[14]!=0x3D)
	{
		ts_cmd17[15]=0x3D;
		
		ts_cmd17[0]=0x2B;	
		
		ret=zet6221_i2c_write_tsdata(client, ts_cmd17, 17);
	}
	
	return 1;
}

u8 zet622x_ts_masserase(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x24};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_pageerase(struct i2c_client *client,int npage)
{
	u8 ts_cmd[3] = {0x23,0x00,0x00};
	u8 len=0;
	int ret;
	switch(ic_model)
	{
		case 0: //6221
			ts_cmd[1]=npage;
			len=2;
			break;
		case 1: //6223
			ts_cmd[1]=npage & 0xff;
			ts_cmd[2]=npage >> 8;
			len=3;
			break;
	}

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, len);

	return 1;
}

u8 zet622x_ts_resetmcu(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0x29};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_hwcmd(struct i2c_client *client)
{
	u8 ts_cmd[1] = {0xB9};
	
	int ret;

	ret=zet6221_i2c_write_tsdata(client, ts_cmd, 1);

	return 1;
}

u8 zet622x_ts_version()
{	
	int i;
		
	printk("pc: ");
	for(i=0;i<8;i++)
		printk("%02x ",pc[i]);
	printk("\n");
	
	printk("src: ");
	for(i=0;i<8;i++)
	{
		switch(ic_model)
		{
			case 0://6221
				printk("%02x ",zeitec_zet6221_firmware[fb[i]]);
				break;
			case 1://6223
				printk("%02x ",zeitec_zet6223_firmware[fb[i]]);
				break;
		}
	}
	printk("\n");
	
	for(i=0;i<8;i++)
	{
		switch(ic_model)
		{
			case 0://6221
				if(pc[i]!=zeitec_zet6221_firmware[fb[i]])
					return 0;
				else break;
			case 1://6223
				if(pc[i]!=zeitec_zet6223_firmware[fb[i]])
					return 0;
				else break;
		}
	
	}	
	return 1;
}

//support compatible
int __init zet622x_downloader( struct i2c_client *client )
{
	int BufLen=0;
	int BufPage=0;
	int BufIndex=0;
	int ret;
	int i;
	int y = 0;	
	int nowBufLen=0;
	int nowBufPage=0;
	int nowBufIndex=0;
	int retryCount=0;
	
	int i2cLength=0;
	int bufOffset=0;
	int index;	
begin_download:
	
	//reset mcu
	index = item_integer("ts.reset", 1); 
	printk("*****reset_zet622x_downloader***\n");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(0,1,index);
	msleep(20);

	msleep(200);
	//send password
	ret=zet622x_ts_sndpwd(client);
	msleep(100);
	
	if(ret<=0)
		return ret;
		
	ret=zet622x_ts_option(client);
	msleep(200);
	
	if(ret<0)
		return ret;

/*****compare version*******/

	//0~3
	memset(zeitec_zet622x_page_in,0x00,131);
	switch(ic_model)
	{
		case 0: //6221
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[0] >> 7);//(fb[0]/128);
			
			i2cLength=2;
			break;
		case 1: //6223
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[0] >> 7) & 0xff; //(fb[0]/128);
			zeitec_zet622x_page_in[2]=(fb[0] >> 7) >> 8; //(fb[0]/128);
			
			i2cLength=3;
			break;
	}
	
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

	if(ret<=0)
		return ret;
	
	zeitec_zet622x_page_in[0]=0x0;
	zeitec_zet622x_page_in[1]=0x0;
	zeitec_zet622x_page_in[2]=0x0;

	ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);


	if(ret<=0)
		return ret;
	
	for(i=0;i<4;i++)
	{
		pc[i]=zeitec_zet622x_page_in[(fb[i] & 0x7f)];//[(fb[i]%128)];
//		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));//(fb[i]%128));
	}
//	printk("\n");
	
	/*
	printk("page=%d ",(fb[0] >> 7));
	for(i=0;i<4;i++)
	{
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));
	}
	printk("\n");
	*/
	
	//4~7
	memset(zeitec_zet622x_page_in,0x00,131);
	switch(ic_model)
	{
		case 0: //6221
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[4] >> 7);//(fb[4]/128);
			
			i2cLength=2;
			break;
		case 1: //6223
			zeitec_zet622x_page_in[0]=0x25;
			zeitec_zet622x_page_in[1]=(fb[4] >> 7) & 0xff; //(fb[4]/128);
			zeitec_zet622x_page_in[2]=(fb[4] >> 7) >> 8; //(fb[4]/128);
			
			i2cLength=3;
			break;
	}
	
	ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

	if(ret<=0)
		return ret;
	
	zeitec_zet622x_page_in[0]=0x0;
	zeitec_zet622x_page_in[1]=0x0;
	zeitec_zet622x_page_in[2]=0x0;

	ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);


	for(i=4;i<8;i++)
	{
		pc[i]=zeitec_zet622x_page_in[(fb[i] & 0x7f)]; //[(fb[i]%128)];
		printk("offset[%d]=%d ",i,(fb[i] & 0x7f));  //(fb[i]%128));
	}
	printk("\n");

	if (ret <= 0)
		return 0;

	if(zet622x_ts_version()!=0)
		goto exit_download;
		
/*****compare version*******/

proc_sfr:
	//sfr
	if(zet622x_ts_sfr(client)==0)
	{
//		imapx_gpio_setpin(gpio, 0, GPIO_HIGH);
		imapx_pad_set_outdat(1,1,index);
		msleep(20);

//		imapx_gpio_setpin(gpio, 0, GPIO_LOW);
		imapx_pad_set_outdat(0,1,index);
		msleep(20);
		
//		imapx_gpio_setpin(gpio, 0, GPIO_HIGH);
		imapx_pad_set_outdat(1,1,index);
		
		msleep(20);
		goto begin_download;		
	}
	msleep(20);
	
	//comment out to enable download procedure
	//return 1;
	
	//erase
	if(BufLen==0)
	{
		//mass erase
		//DPRINTK( "mass erase\n");
		zet622x_ts_masserase(client);
		msleep(200);

		switch(ic_model)
		{
			case 0://6221
                //20120927 make firmwares into a header file
				//BufLen=sizeof(zeitec_zet6221_firmware)/sizeof(char);
				BufLen = firmware_len;
				break;
			case 1://6223
				BufLen=sizeof(zeitec_zet6223_firmware)/sizeof(char);
				break;
		}
	}else
	{
		zet622x_ts_pageerase(client,BufPage);
		msleep(200);
	}
	
	
	while(BufLen>0)
	{
download_page:

		memset(zeitec_zet622x_page,0x00,131);
		
		DPRINTK( "Start: write page%d\n",BufPage);
		nowBufIndex=BufIndex;
		nowBufLen=BufLen;
		nowBufPage=BufPage;
		switch(ic_model)
		{
			case 0: //6221
				bufOffset = 2;
				i2cLength=130;
				
				zeitec_zet622x_page[0]=0x22;
				zeitec_zet622x_page[1]=BufPage;				
				break;
			case 1: //6223
				bufOffset = 3;
				i2cLength=131;
				
				zeitec_zet622x_page[0]=0x22;
				zeitec_zet622x_page[1]=BufPage & 0xff;
				zeitec_zet622x_page[2]=BufPage >> 8;
				break;
		}
		
		if(BufLen>128)
		{
			for(i=0;i<128;i++)
			{
				switch(ic_model)
				{
					case 0://6221
						zeitec_zet622x_page[i+bufOffset]=zeitec_zet6221_firmware[BufIndex];
						break;
					case 1://6223
						zeitec_zet622x_page[i+bufOffset]=zeitec_zet6223_firmware[BufIndex];
						break;
				}
				BufIndex+=1;
			}
			BufLen-=128;
		}
		else
		{
			for(i=0;i<BufLen;i++)
			{
				switch(ic_model)
				{
					case 0://6221
						zeitec_zet622x_page[i+bufOffset]=zeitec_zet6221_firmware[BufIndex];
						break;
					case 1://6223
						zeitec_zet622x_page[i+bufOffset]=zeitec_zet6223_firmware[BufIndex];
						break;
				}
				BufIndex+=1;
			}
			BufLen=0;
		}
		printk( "End: write page%d\n",BufPage);

		ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page, i2cLength);


		msleep(200);
		
#if 1

		memset(zeitec_zet622x_page_in,0x00,131);
		switch(ic_model)
		{
			case 0: //6221
				zeitec_zet622x_page_in[0]=0x25;
				zeitec_zet622x_page_in[1]=BufPage;
				
				i2cLength=2;
				break;
			case 1: //6223
				zeitec_zet622x_page_in[0]=0x25;
				zeitec_zet622x_page_in[1]=BufPage & 0xff;
				zeitec_zet622x_page_in[2]=BufPage >> 8;

				i2cLength=3;
				break;
		}		
		
		ret=zet6221_i2c_write_tsdata(client, zeitec_zet622x_page_in, i2cLength);

		zeitec_zet622x_page_in[0]=0x0;
		zeitec_zet622x_page_in[1]=0x0;
		zeitec_zet622x_page_in[2]=0x0;
		
		ret=zet6221_i2c_read_tsdata(client, zeitec_zet622x_page_in, 128);

		
		for(i=0;i<128;i++)
		{
//			printk("********i=%d zeitec_zet622x_page_in=%2x \n",i,zeitec_zet622x_page_in[i]);
			if(i < nowBufLen)
			{
				if(zeitec_zet622x_page[i+bufOffset]!=zeitec_zet622x_page_in[i])
				{
					printk("*****Downloader failure\n");
					BufIndex=nowBufIndex;
					BufLen=nowBufLen;
					BufPage=nowBufPage;
					
					if(retryCount < 5)
					{
						retryCount++;
						goto download_page;
					}else
					{
						//BufIndex=0;
						//BufLen=0;
						//BufPage=0;
						retryCount=0;
						
						imapx_pad_set_outdat(1,1,index);
						msleep(20);
		
						imapx_pad_set_outdat(0,1,index);
						msleep(20);
		
						imapx_pad_set_outdat(1,1,index);
						msleep(20);
						goto begin_download;

					}

				}
			}
		}
		
#endif
		retryCount=0;
		BufPage+=1;
	}

exit_download:

	imapx_pad_set_outdat(1,1,index);
	msleep(200);

	zet622x_ts_resetmcu(client);
	msleep(100);
	
	//return 1;

}

/*********************************************end***********************************************/

static struct i2c_driver zet6221_ts_driver = {
	.probe	  = zet6221_ts_probe,
	.remove		= __devexit_p(zet6221_ts_remove),
	.suspend  = ts_early_suspend,
        .resume   = ts_late_resume,	
	.id_table = zet6221_ts_idtable,
	.driver = {
		.owner = THIS_MODULE,
		.name  = ZET_TS_ID_NAME,
	},
};


/***********************************************************************
    [function]: 
		        callback: Timer Function if there is no interrupt fuction;
    [parameters]:
			    arg[in]:  arguments;
    [return]:
			    NULL;
************************************************************************/

static void polling_timer_func(unsigned long arg)
{
	struct zet6221_tsdrv *ts = (struct zet6221_tsdrv *)arg;
	schedule_work(&ts->work1);
	mod_timer(&ts->polling_timer,jiffies + msecs_to_jiffies(polling_time));
}

/***********************************************************************
    [function]: 
		        callback: read data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client — represent an I2C slave device;
			    data [out]:  data buffer to read;
			    length[in]:  data length to read;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_read_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: write data by i2c interface;
    [parameters]:
			    client[in]:  struct i2c_client — represent an I2C slave device;
			    data [out]:  data buffer to write;
			    length[in]:  data length to write;
    [return]:
			    Returns negative errno, else the number of messages executed;
************************************************************************/
s32 zet6221_i2c_write_tsdata(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter,&msg, 1);
}

/***********************************************************************
    [function]: 
		        callback: coordinate traslating;
    [parameters]:
			    px[out]:  value of X axis;
			    py[out]:  value of Y axis;
				p [in]:   pressed of released status of fingers;
    [return]:
			    NULL;
************************************************************************/
void touch_coordinate_traslating(u32 *px, u32 *py, u8 p)
{
	int i;
	u8 pressure;

	#if ORIGIN == TOPRIGHT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			px[i] = X_MAX - px[i];
		}
	}
	#elif ORIGIN == BOTTOMRIGHT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			px[i] = X_MAX - px[i];
			py[i] = Y_MAX - py[i];
		}
	}
	#elif ORIGIN == BOTTOMLEFT
	for(i=0;i<MAX_FINGER_NUMBER;i++){
		pressure = (p >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
		if(pressure)
		{
			py[i] = Y_MAX - py[i];
		}
	}
	#endif
}

/***********************************************************************
    [function]: 
		        callback: reset function;
    [parameters]:
			    void;
    [return]:
			    void;
************************************************************************/
void ctp_reset(void)
{
	uint32_t index;
	index = item_integer("ts.reset", 1);
//	index = imapx_pad_index("cond9");
	printk("*****reset_zet6221***\n");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(1,1,index);
	msleep(1);
	imapx_pad_set_outdat(0,1,index);
	msleep(10);
	imapx_pad_set_outdat(1,1,index);
	msleep(20);
    return;
}
/********************charger mode 2012-09-07 suixing********************/
/***********************************************************************
    [function]: 
		        callback: charger mode enable;
    [parameters]:
    			void

    [return]:
			    void
************************************************************************/

void zet6221_ts_charger_mode()
{
	printk("****************** %s is run************\n", __func__);
	struct zet6221_tsdrv *zet6221_ts_mode;
	u8 ts_write_charge_cmd[1] = {0xb5}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_write_charge_cmd, 1);
}
//EXPORT_SYMBOL_GPL(zet6221_ts_charger_mode);

/***********************************************************************
    [function]: 
		        callback: charger mode disable;
    [parameters]:
    			void

    [return]:
			    void
************************************************************************/
void zet6221_ts_charger_mode_disable()
{
	printk("****************** %s is run ************\n", __func__);
	struct zet6221_tsdrv *zet6221_ts_mode;
	u8 ts_write_cmd[1] = {0xb6}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_write_cmd, 1);
}
//EXPORT_SYMBOL_GPL(zet6221_ts_charger_mode_disable);

void write_cmd_work()
{
//	printk("**************probe_finished = %d, isacon() = %d, ChargeChange = %d************\n",probe_finished, isacon(), ChargeChange);
	if (probe_finished ==1){
		if(isacon() != ChargeChange)
		{	
			if(isacon() == 1) {
				zet6221_ts_charger_mode();
			}else if(isacon() == 0)
			{
				zet6221_ts_charger_mode_disable();
			}
			ChargeChange = isacon();
		}
	}
	schedule_delayed_work(&zet6221_ts->work2, 800);
}
/***************************end*******************************************************/

/***********************************************************************
    [function]: 
		        callback: read finger information from TP;
    [parameters]:
    			client[in]:  struct i2c_client — represent an I2C slave device;
			    x[out]:  values of X axis;
			    y[out]:  values of Y axis;
			    z[out]:  values of Z axis;
				pr[out]:  pressed of released status of fingers;
				ky[out]:  pressed of released status of keys;
    [return]:
			    Packet ID;
************************************************************************/
u8 zet6221_ts_get_xy_from_panel(struct i2c_client *client, u32 *x, u32 *y, u32 *z, u32 *pr, u32 *ky)
{
	u8  ts_data[70];
//	u8 ts_report_cmd[1] = {0xb2};
	int ret;
	int i;
//	int j;
	memset(ts_data,0,70);

	ret=zet6221_i2c_read_tsdata(client, ts_data, bufLength);
//	ret=zet6221_i2c_read_tsdata(client, ts_data, 8);
//	ret=zet6221_i2c_read_tsdata(client, ts_data+8, 8);
//	ret=zet6221_i2c_read_tsdata(client, ts_data+16, 8);
//	for (j = 0; j <bufLength; j++)
//	{
//		printk("ts_data[%d] = %d",j, ts_data[j]);
//		i = j%10;
//		if(i==0 && j !=0)
//			printk("\n");	
//	}
	*pr = ts_data[1];
	*pr = (*pr << 8) | ts_data[2];
	for(i=0;i<FingerNum;i++)
	{
		
		x[i]=(u8)((ts_data[3+4*i])>>4)*256 + (u8)ts_data[(3+4*i)+1];
		y[i]=(u8)((ts_data[3+4*i]) & 0x0f)*256 + (u8)ts_data[(3+4*i)+2];
		z[i]=(u8)((ts_data[(3+4*i)+3]) & 0x0f);
//		printk("******************i = %d, ts_data[3+4*i] = %d, x[i] = %d, y[i] = %d, z[i] = %d********\n", i, ts_data[3+4*i],x[i],y[i],z[i]);
	}		
	//if key enable
	if(KeyNum > 0)
		*ky = ts_data[3+4*FingerNum];
	return ts_data[0];
}
/***********************************************************************
    [function]: 
		        callback: get dynamic report information;
    [parameters]:
    			client[in]:  struct i2c_client — represent an I2C slave device;

    [return]:
			    1;
************************************************************************/
u8 zet6221_ts_get_report_mode(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {0xb2};
//	u8 ts_reset_cmd[1] = {0xb0};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	int ret;
	int i;
	int count=0;

	ret=zet6221_i2c_write_tsdata(client, ts_report_cmd, 1);

	if (ret > 0)
	{
		while(1)
		{
			msleep(1);


			if (imapx_pad_get_indat(tsp_irq_index) == 0)
			{
				DPRINTK( "int low\n");
				ret=zet6221_i2c_read_tsdata(client, ts_in_data, 17);
				
				if(ret > 0)
				{
				
					for(i=0;i<8;i++)
					{
						pc[i]=ts_in_data[i] & 0xff;
					}				
					
					if(pc[3] != 0x08)
					{
						printk ("=============== zet6221_ts_get_report_mode report error ===============\n");
						return 0;
					}

					xyExchange = (ts_in_data[16] & 0x8) >> 3;
					if(xyExchange == 1)
					{
						ResolutionY= ts_in_data[9] & 0xff;
						ResolutionY= (ResolutionY << 8)|(ts_in_data[8] & 0xff);
						ResolutionX= ts_in_data[11] & 0xff;
						ResolutionX= (ResolutionX << 8) | (ts_in_data[10] & 0xff);
					}
					else
					{
						ResolutionX = ts_in_data[9] & 0xff;
						ResolutionX = (ResolutionX << 8)|(ts_in_data[8] & 0xff);
						ResolutionY = ts_in_data[11] & 0xff;
						ResolutionY = (ResolutionY << 8) | (ts_in_data[10] & 0xff);
					}
					
					FingerNum = (ts_in_data[15] & 0x7f);
					KeyNum = (ts_in_data[15] & 0x80);

					if(KeyNum==0)
						bufLength  = 3+4*FingerNum;
					else
						bufLength  = 3+4*FingerNum+1;

					//DPRINTK( "bufLength=%d\n",bufLength);
				
					break;
				
				}else
				{
					printk ("=============== zet6221_ts_get_report_mode read error ===============\n");
					return 0;
				}
				
			}else
			{
				//DPRINTK( "int high\n");
				if(count++ > 2000)
				{
					printk ("=============== zet6221_ts_get_report_mode time out ===============\n");
					return 0;
				}
				
			}
		}

	}else
	{
		printk ("=============== zet6221_ts_get_report_mode WRITE ERROR ===============\n");
		return 0;
	}
	return 1;
}

/***********************************************************************
    [function]: 
		        callback: get dynamic report information with timer delay;
    [parameters]:
    			client[in]:  struct i2c_client represent an I2C slave device;

    [return]:
			    1;
************************************************************************/

u8 zet6221_ts_get_report_mode_t(struct i2c_client *client)
{
	u8 ts_report_cmd[1] = {0xb2};
	u8 ts_in_data[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	int ret;
	int i;
	ret=zet6221_i2c_write_tsdata(client, ts_report_cmd, 1);
	if (ret > 0)
	{
			//udelay(10);
			msleep(10);
			printk ("=============== zet6221_ts_get_report_mode_t ===============\n");
			ret=zet6221_i2c_read_tsdata(client, ts_in_data, 17);
			if(ret > 0)
			{
				
				for(i=0;i<8;i++)
				{
					pc[i]=ts_in_data[i] & 0xff;
				//	printk("**********pc[i] = %x\n", pc[i]);
				}
				
				if(pc[3] != 0x08)
				{
					printk ("=============== zet6221_ts_get_report_mode_t report error ===============\n");
					return 0;
				}

				xyExchange = (ts_in_data[16] & 0x8) >> 3;
				if(xyExchange == 1)
				{
					ResolutionY= ts_in_data[9] & 0xff;
					ResolutionY= (ResolutionY << 8)|(ts_in_data[8] & 0xff);
					ResolutionX= ts_in_data[11] & 0xff;
					ResolutionX= (ResolutionX << 8) | (ts_in_data[10] & 0xff);
				}
				else
				{
					ResolutionX = ts_in_data[9] & 0xff;
					ResolutionX = (ResolutionX << 8)|(ts_in_data[8] & 0xff);
					ResolutionY = ts_in_data[11] & 0xff;
					ResolutionY = (ResolutionY << 8) | (ts_in_data[10] & 0xff);
				}
					
				FingerNum = (ts_in_data[15] & 0x7f);
				KeyNum = (ts_in_data[15] & 0x80);
				inChargerMode = (ts_in_data[16] & 0x2) >> 1;

				if(KeyNum==0)
					bufLength  = 3+4*FingerNum;
				else
					bufLength  = 3+4*FingerNum+1;
				
			}else
			{
				printk ("=============== zet6221_ts_get_report_mode_t READ ERROR ===============\n");
				return 0;
			}
							
	}else
	{
		printk ("=============== zet6221_ts_get_report_mode_t WRITE ERROR ===============\n");
		return 0;
	}
	return 1;
}

/***********************************************************************
    [function]: 
		        callback: interrupt function;
    [parameters]:
    			irq[in]:  irq value;
    			dev_id[in]: dev_id;

    [return]:
			    NULL;
************************************************************************/
static irqreturn_t zet6221_ts_interrupt(int irq, void *dev_id)
{
//	printk("***************** %s *************\n", __func__);
	if(imapx_pad_irq_pending(tsp_irq_index))
	    imapx_pad_irq_clear(tsp_irq_index);
//	else
//	    return IRQ_HANDLED;


		if (imapx_pad_get_indat(tsp_irq_index) == 0)
		{
			/* IRQ is triggered by FALLING code here */
			struct zet6221_tsdrv *ts_drv = dev_id;
			schedule_work(&ts_drv->work1);
			DPRINTK("TS1_INT_GPIO falling\n");
		}else
		{
			DPRINTK("TS1_INT_GPIO raising\n");
		}
	//}

	return IRQ_HANDLED;
}

/***********************************************************************
    [function]: 
		        callback: touch information handler;
    [parameters]:
    			_work[in]:  struct work_struct;

    [return]:
			    NULL;
************************************************************************/
static void zet6221_ts_work(struct work_struct *_work)
{
	u32 x[MAX_FINGER_NUMBER], y[MAX_FINGER_NUMBER], z[MAX_FINGER_NUMBER], pr, ky, points;
	u32 px,py,pz;
	u8 ret;
	u8 pressure;
	int i;
	if (bufLength == 0)
	{
		return;
	}

	if (imapx_pad_get_indat(tsp_irq_index) != 0)
	{
		/* do not read when IRQ is triggered by RASING*/
		DPRINTK("INT HIGH\n");
		return;
	}

	struct zet6221_tsdrv *ts = container_of(_work, struct zet6221_tsdrv, work1);

	struct i2c_client *tsclient1 = ts->i2c_ts;

	ret = zet6221_ts_get_xy_from_panel(tsclient1, x, y, z, &pr, &ky);

	if(ret == 0x3C)
	{


		points = pr;
		
		#if defined(TRANSLATE_ENABLE)
		touch_coordinate_traslating(x, y, points);
		#endif

		if(points == 0)
		{
			f_up_cnt++;
			if( f_up_cnt >= DEBOUNCE_NUMBER )
			{
				f_up_cnt = 0;
				input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input);
			}
			goto no_finger;
		}

		f_up_cnt = 0;

		for(i=0;i<FingerNum;i++){
			pressure = (points >> (MAX_FINGER_NUMBER-i-1)) & 0x1;
			if(pressure)
			{
//				px = x[i];
//				py = y[i];
//				pz = z[i];
				switch(orient_num){
			       		case 1:
			    			px = x[i];
			    			py = y[i];
						pz = z[i];
			    			break;
					case 2:
			    			px = x[i];
			    			py = ResolutionX - y[i];
						pz = z[i];
			    			break;
					case 3:
						px = ResolutionX - x[i];
			    			py = y[i];
						pz = z[i];
			    			break;
					case 4: 
			    			px = ResolutionX - x[i];
			    			py = ResolutionY - y[i];
						pz = z[i];
			    			break;
					case 5:
			    			px = y[i];
			    			py = x[i];
						pz = z[i];
			    			break;
					case 6:
			    			px = y[i];
			    			py = ResolutionX - x[i];
						pz = z[i];
			    			break;
					case 7:
			    			px = ResolutionY - y[i];
			    			py = x[i];
						pz = z[i];
			    			break;
					case 8:
			    			px = ResolutionY - y[i];
			    			py = ResolutionX - x[i];
						pz = z[i];
			    			break;
				}		
		
			input_report_abs(ts->input, ABS_MT_TRACKING_ID, i);
	    		//input_report_abs(ts->input, ABS_MT_POSITION_X, x[i]);
	    		//input_report_abs(ts->input, ABS_MT_POSITION_Y, y[i]);
	    		input_report_abs(ts->input, ABS_MT_POSITION_X, px);
	    		input_report_abs(ts->input, ABS_MT_POSITION_Y, py);
	    		input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, pressure);
			input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR, 50);
		//	input_report_key(ts->input, BTN_TOUCH, 1);	//The collected datum will not be transfered without the code   2012-07-07 suixing
			//input_report_key(ts->input, ABS_PRESSURE, 1);
//	     		printk( "valid=%04X, pressure[%d]= %d, x[i] = %d, y[i] = %d,z[i] = %d \n",points , i, pressure,px,py);
	    		input_mt_sync(ts->input);
	
			}
		//	else
		//	{
		//		input_report_abs(ts->input, ABS_MT_TRACKING_ID, i);
		//		input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR, 0);//pressure);
		//		input_report_key(ts->input, ABS_PRESSURE, 0);
		//		input_mt_sync(ts->input);
		//	}
		}

no_finger:
		if(KeyNum > 0)
		{
			//for(i=0;i<MAX_KEY_NUMBER;i++)
			for(i=0;i<4;i++)
			{			
				pressure = ky & ( 0x01 << i );
				//printk("*********ky = %d, pressure = %d*********\n", ky, pressure);
				switch(i)
				{
					case 0:
						if(pressure)
						{
							if(key_back_pressed)
							{
								input_report_key(ts->input, KEY_BACK, 1);
								input_report_key(ts->input, KEY_BACK, 0);
								key_back_pressed = 0x0;
							}
						}else
						{
							key_back_pressed = 0x1;
						}
						break;
					case 1:
						if(pressure)
						{
							if(key_menu_pressed)
							{
								input_report_key(ts->input, KEY_MENU, 1);
								input_report_key(ts->input, KEY_MENU, 0);
								key_menu_pressed = 0x0;
							}
						}else
						{
							key_menu_pressed = 0x1;
						}
						break;
					case 2:
						if(pressure)
							if(key_home_pressed)
							{
								input_report_key(ts->input, KEY_HOME, 1);
								input_report_key(ts->input, KEY_HOME, 0);
							}
						break;

					case 3:
						if(pressure)
						{
							if(key_search_pressed)
							{
								input_report_key(ts->input, KEY_SEARCH, 1);
								input_report_key(ts->input, KEY_SEARCH, 0);
								key_search_pressed = 0x0;
							}
						}else
						{
							key_search_pressed = 0x1;
						}
						break;
					/*case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;*/
				}

			}
		}

		input_sync(ts->input);		
		
		msleep(10);
	}

}

# if 0
static void ts_early_suspend(struct early_suspend *handler)
{
	//Sleep Mode
	u8 ts_sleep_cmd[1] = {0xb1}; 
	int ret=0;
	ret=zet6221_i2c_write_tsdata(this_client, ts_sleep_cmd, 1);
        return;

}

static void ts_late_resume(struct early_suspend *handler)
{
#if 0
        gpio_direction_output(TS_RST_GPIO, 0);
		msleep(10);
		gpio_direction_output(TS_RST_GPIO, 1);
		msleep(20);
#else
		//Wake Up Mode
		u8 ts_wakeup_cmd[1] = {0xb4};
		zet6221_i2c_write_tsdata(this_client, ts_wakeup_cmd, 1);
#endif
      
return;

}
#endif

static int zet6221_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int result;
	struct input_dev *input_dev;
//	struct zet6221_tsdrv *zet6221_ts;
   	int index;
	int err = 0;
	int count=0;
	char orient_x, orient_y;
        char orientation[ITEM_MAX_LEN];
	
	printk( "[TS] zet6221_ts_probe \n");
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	zet6221_ts = kzalloc(sizeof(struct zet6221_tsdrv), GFP_KERNEL);
	zet6221_ts->i2c_ts = client;
//	zet6221_ts->gpio = TS_INT_GPIO; /*s3c6410*/
	//zet6221_ts->gpio = TS1_INT_GPIO;
	
	this_client = client;
	i2c_set_clientdata(client, zet6221_ts);

	client->driver = &zet6221_ts_driver;
	INIT_WORK(&zet6221_ts->work1, zet6221_ts_work);

    	if(item_exist("ts.orientation"))
   	 {
       		item_string(orientation, "ts.orientation", 0);
        	orient_x = orientation[0];
        	orient_y = orientation[1];

	        if(orient_x == 'x')
       		{
            		if(orient_y == 'y') orient_num = 1;
            		if(orient_y == 'Y') orient_num = 2;
       		}
        	else if(orient_x == 'X')
        	{
			if(orient_y == 'y') orient_num = 3;
			if(orient_y == 'Y') orient_num = 4;
        	}
        	else if(orient_x == 'y')
        	{
			if(orient_y == 'x') orient_num = 5;
			if(orient_y == 'X') orient_num = 6;
	        }
        	else if(orient_x == 'Y')
        	{
			if(orient_y == 'x') orient_num = 7;
         		if(orient_y == 'X') orient_num = 8;
        	}
    	}
    	else{
        	orient_num = 1;
    	}


#ifdef CHARGER_MODE_RUN	
	INIT_DELAYED_WORK(&zet6221_ts->work2, write_cmd_work);
	schedule_delayed_work(&zet6221_ts->work2, 800);
#endif
	input_dev = input_allocate_device();
	if (!input_dev || !zet6221_ts) {
		result = -ENOMEM;
		goto fail_alloc_mem;
	}
	
	i2c_set_clientdata(client, zet6221_ts);

	input_dev->name = ZET_TS_ID_NAME;
	input_dev->phys = "zet6221_touch/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0002;
	input_dev->id.version = 0x0100;
//	KEY_NUMBER = item_integer("ts.keynum", 1);

/***********************open downloader suixing 2012-09-12**********************/
zet_download:
	if(item_exist("ts.downloader")){
		//20120927 make firmwares into a header file
        char section[ITEM_MAX_LEN] = "";
        int firmware_no;
        item_string(downloader_name, "ts.downloader", 0);
		get_string_sec(section, downloader_name, 0);
		if (strcmp(section, "zet622x") == 0){
		    get_string_sec(section, downloader_name, 1);
            if (strcmp(section, "zet6221") == 0) {
                firmware_no = item_integer("ts.downloader", 2);
                switch (firmware_no) {
                        case 0 : 
				                firmware_len = sizeof(zeitec_zet6221_firmware_0)/sizeof(char);
                                break;
                        case 1 : 
				                firmware_len = sizeof(zeitec_zet6221_firmware_1)/sizeof(char);
                                break;
                        case 2 : 
				                firmware_len = sizeof(zeitec_zet6221_firmware_2)/sizeof(char);
                                break;
                      
                        default :
				                firmware_len = sizeof(zeitec_zet6221_firmware_0)/sizeof(char);
                                break;
                }
                printk("++++++++++++++firmware_no: %d++++++++++++firmware_len: %d++++++\n", firmware_no, firmware_len);
                zeitec_zet6221_firmware = (unsigned char*) zeitec_zet6221_firmware_collection[firmware_no];
		        if(zet622x_ts_version() == 0){
                    zet622x_downloader(client);
                }
			}
		}
	}

/***********************************end**************************************/

#if defined(TPINFO)

		//wakeup pin for reset 	
	ctp_reset(); 
	if(zet6221_ts_get_report_mode_t(client)<=0)  //get IC info by delay 
	{
		goto exit_check_functionality_failed;
	}
#else
	ResolutionX = X_MAX;
	ResolutionY = Y_MAX;
	FingerNum = FINGER_NUMBER;
	KeyNum = KEY_NUMBER;
	if(KeyNum==0)
		bufLength  = 3+4*FingerNum;
	else
		bufLength  = 3+4*FingerNum+1;
#endif

    printk( "ResolutionX=%d ResolutionY=%d FingerNum=%d KeyNum=%d\n",ResolutionX,ResolutionY,FingerNum,KeyNum);

    //  set_bit(BTN_TOUCH, input_dev->evbit);
    set_bit(EV_ABS, input_dev->evbit);	
    set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);	
    set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit); 
    set_bit(ABS_MT_POSITION_X, input_dev->absbit); 
    set_bit(ABS_MT_POSITION_Y, input_dev->absbit); 
    set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit); 
    set_bit(EV_SYN, input_dev->evbit);
    set_bit(EV_KEY, input_dev->evbit);

	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 30, 0, 0);

	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, P_MAX, 0, 0);
	
	//if(ResolutionX==0 && ResolutionY==0)
	//{		
		input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ResolutionX, 0, 0);
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ResolutionY, 0, 0);
		input_set_abs_params(input_dev,ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	//}else
	//{
	//	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ResolutionX, 0, 0);
	//	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ResolutionY, 0, 0);
	//}	
	//input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 10, 0, 0);
	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_HOME, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_SEARCH, input_dev->keybit);

	input_set_capability(input_dev, EV_KEY,KEY_BACK);
	input_set_capability(input_dev, EV_KEY,KEY_HOME);
	input_set_capability(input_dev, EV_KEY,KEY_MENU);
	input_set_capability(input_dev, EV_KEY,KEY_SEARCH);

	input_dev->evbit[0] = BIT(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
//	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	result = input_register_device(input_dev);
	if (result)
		goto fail_ip_reg;
	zet6221_ts->input = input_dev;
	input_set_drvdata(zet6221_ts->input, zet6221_ts);

	index = item_integer("ts.int", 1);
        imapx_pad_set_mode(0, 1, index);
        imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);
	_sui_irq_num = imapx_pad_irq_number(index);
	result = request_irq(_sui_irq_num, zet6221_ts_interrupt,0, ZET_TS_ID_NAME, zet6221_ts);
	if (result ) {
		printk(KERN_ERR "zet6221_init_client: request irq failed,ret is %d\n",result);
        	goto request_irq_fail;
	}
//	enable_irq(_sui_irq_num);
//	zet6221_ts->early_suspend.suspend = ts_early_suspend,
//	zet6221_ts->early_suspend.resume = ts_late_resume,
//	zet6221_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 2,
//	register_early_suspend(&zet6221_ts->early_suspend);
	//disable_irq(zet6221_ts->irq);

	probe_finished = 1;  //To prevent the bootloader and charge_mode conflict 2012-09-07 suixing
	return 0;

request_irq_fail:
	kfree(zet6221_ts);
fail_ip_reg:
exit_check_functionality_failed:
	printk("%s err exit\n", __func__);
fail_alloc_mem:
	input_free_device(input_dev);
	kfree(zet6221_ts);
	return result;
}

static int __devexit zet6221_ts_remove(struct i2c_client *dev)
{
	struct zet6221_tsdrv *zet6221_ts_remove = i2c_get_clientdata(dev);

	free_irq(zet6221_ts_remove->irq, zet6221_ts_remove);
	//gpio_free(zet6221_ts_remove->gpio);
	del_timer_sync(&zet6221_ts_remove->polling_timer);
	input_unregister_device(zet6221_ts_remove->input);
	kfree(zet6221_ts_remove);

	return 0;
}
static int __init zet6221_ts_init(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret;

    printk("------zet622x module init------\n");
    if(item_exist("ts.model"))
    {
        if(item_equal("ts.model", "zet622x", 0))
        {
//	    if (item_exist("ts.xmax") && item_exist("ts.ymax")){
//		X_MAX = item_integer("ts.xmax", 0);
//	    	Y_MAX = item_integer("ts.ymax", 0);	    
//          	ResolutionX = X_MAX;
//	    	ResolutionY = Y_MAX;
//	    }
	    memset(&info, 0, sizeof(struct i2c_board_info));
            info.addr = 0x76;  
            strlcpy(info.type, ZET_TS_ID_NAME, I2C_NAME_SIZE);
            adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
            
            if (!adapter) {
                printk("*******get_adapter error!********\n");
            }
            client = i2c_new_device(adapter, &info);
	    ret = i2c_add_driver(&zet6221_ts_driver);
	return ret;
	}
	else
		printk("%s: touchscreen is not zet6221\n", __func__);
    }
	else
		printk("%s: touchscreen is not exist\n", __func__);
    return -1;
}

static void __exit zet6221_ts_exit(void)
{
    i2c_del_driver(&zet6221_ts_driver);
}

module_init(zet6221_ts_init);


module_exit(zet6221_ts_exit);

void zet6221_set_ts_mode(u8 mode)
{
	DPRINTK( "[Touch Screen]ts mode = %d \n", mode);
}
EXPORT_SYMBOL_GPL(zet6221_set_ts_mode);


MODULE_AUTHOR("<duxx@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech zet6221 TouchScreen driver");
MODULE_LICENSE("GPL");





