/* Morgan capacivite multi-touch device driver.
 *
 * Copyright(c) 2010 MorganTouch Inc.
 *
 *************************************
 ***	 Driver For Morgan Touch 
 ***	 Digitizer or Dual Mode Devices
 ***	 Data	 :	 2012.09.25
 ***	 Author  :	 suixing
 **************************************/


#include <linux/i2c.h>
#include <linux/input.h>
#include "mg8696s.h"
#include "mg8696s_update.h"
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
// for linux 2.6.36.3
#include <linux/cdev.h>
#include <linux/uaccess.h>
//for test thread
#include <linux/kthread.h>
#include <mach/hardware.h>
#include <mach/items.h>
#include <mach/pad.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/pm.h>
#include <linux/earlysuspend.h>
#endif

#define IOCTL_ENDING    		0xD0
#define IOCTL_I2C_SLAVE			0xD1
#define IOCTL_READ_ID		  	0xD2
#define IOCTL_READ_VERSION  	0xD3
#define IOCTL_RESET  			0xD4
#define IOCTL_IAP_MODE			0xD5
#define IOCTL_CALIBRATION		0xD6
#define IOCTL_ACTION2			0xD7
#define IOCTL_DEFAULT			0x88
static int command_flag= 0;
 
#define MG_I2C_NAME	"mg8696s"
#define BUF_SIZE 		30


//#define TP_UPDATE_FW	

#define BABBAGE_TOUCH_RESET		(3*32 + 4) /* GPIO4_4*/
//+++++++open debug message
//#define RANDY_DEBUG

static int ver_flag = 0;
static int  id_flag = 0;
static int CS_flag = 0;
#define COORD_INTERPRET(MSB_BYTE, LSB_BYTE) \
		(MSB_BYTE << 8 | LSB_BYTE)

/************ randy  add for return 816***********/
static int ack_flag = 0;
static u8 read_buf[BUF_SIZE]={0};
//static u8 ver_buf[COMMAND_BIT]={0};
static u8 ver_buf[5]={0};
/************ randy  add for return 816***********/


static void mg_early_suspend(struct early_suspend *h);
static void mg_late_resume(struct early_suspend *h);

static struct mg_data *private_ts;
static struct i2c_client *touch_i2c_client;
static int tsp_irq_index;
static unsigned int _sui_irq_num;
static void mg_i2c_work(struct work_struct *work);
static void ctp_reset(void);
static int FWPoll_2();

// --------------------------------------------------------------
struct mg_data{
	u16 x, y, w, p, id;
	struct i2c_client *client;
	/* capacivite device*/
	struct input_dev *dev;
	/* digitizer */
	struct timer_list timer;
	struct input_dev *dig_dev;
	struct mutex lock;
	int irq;
	struct work_struct work;
	struct early_suspend early_suspend;
	int (*power)(int on);
	int fw_ver;
   	struct miscdevice firmware;
	int iap_mode;		/* Firmware update mode or 0 for normal */
	struct ctp_platform_data *pdata;
};

//specific tp related macro: need be configured for specific tp

#define CTP_NAME			MG_I2C_NAME
#define SCREEN_MAX_HEIGHT		(screen_max_x)
#define SCREEN_MAX_WIDTH		(screen_max_y)

static char downloader_name[ITEM_MAX_LEN] = MG_I2C_NAME;
static int screen_max_x = 0;
static int screen_max_y = 0;
static int revert_x_flag = 1;
static int revert_y_flag = 0;
static int exchange_x_y_flag = 0;

static void ctp_reset(void)
{
	int index;
	index = item_integer("ts.reset", 1);
//	index = imapx_pad_index("cond9");
	printk("*****reset_mg8696***\n");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(1,1,index);
	msleep(20);
	imapx_pad_set_outdat(0,1,index);
	msleep(100);
	imapx_pad_set_outdat(1,1,index);
    	return;
}

/*
static void ctp_wakeup(void)
{
	int index;
	index = item_integer("ts.reset", 1);
//	index = imapx_pad_index("cond9");
	printk("*****wakeup_mg8696***\n");
	imapx_pad_set_mode(1, 1, index);
	imapx_pad_set_dir(0,1,index);
	imapx_pad_set_outdat(1,1,index);
	msleep(20);
	imapx_pad_set_outdat(0,1,index);
	msleep(50);
	imapx_pad_set_outdat(1,1,index);
    	msleep(30);
	return;
}
*/

static unsigned int kbc_i2c_read_reg(unsigned int reg) 
{
	return i2c_smbus_read_byte_data(touch_i2c_client, reg);
}

static unsigned int kbc_i2c_write_reg(unsigned int reg,unsigned int value) 
{
	return i2c_smbus_write_byte_data(touch_i2c_client, reg, value);
}

/*****************  ac dc *******************/
static u_int8_t touch_list[3][5] =
{
	{ 0x81, 0x02, 0x00, 0x00, 0x00},
	{ 0x81, 0x02, 0x01, 0x00, 0x00},
	{ 0x97, 0x00, 0x00, 0x00, 0x00},
};
/*****************  ac dc *******************/

static u_int8_t nomal_mode[5]=
{0x97, 0x00, 0x00, 0x00, 0x00};

//++++++++add for touch calibration
static u_int8_t cal_ack[5]=
{0x00,0x00,0x00,0x00,0x00};
static int calibration_flag=0;
static int IsCalibration=0;

static int Act_FWPoll = 0;
static int Rtn_FWPoll = 0;

static int FWRtnDecode()
{
	if( (read_buf[0]==0xab)&&
		(read_buf[1]==0x01)&&
		(read_buf[2]==0x55)		
		){
			return 1;
		}
		
    if( (read_buf[0]==0xab)&&
		(read_buf[1]==0x55)&&
		(read_buf[2]==0x66)		
		){
			return 1;
		}
		
	if( (read_buf[0]==0xab)&&
		(read_buf[1]==0x01)&&
		(read_buf[2]==0xCC)		
		){
			return -1;
		}		

	return 0;
}

static int FWPoll_1()
{
	Act_FWPoll = 1;
	command_flag = 1;
	Rtn_FWPoll = 0;

	return 1;
}

static int FWPoll_2()
{
	int PollCnt = 0;
	
	while(Act_FWPoll)
	{
		//Delay
		msleep(10);
		
		//
		PollCnt++;
		
		if( PollCnt>1000 )
		{
			Act_FWPoll = 0;
			command_flag = 0;
			return 0;
		}
	}	
	
	if( Rtn_FWPoll==-1 )  
	{
		printk("FW update return fail\n");
		return 0;	
	}
	
	return 1;
}

static int ReadFWCheckSum()
{
	int ret = 0;
	u_int8_t commandBuf[5];
		
	//unsigned char *p_data ;
	unsigned char *p_data = mg8696s_firmware;
	#ifdef RANDY_DEBUG    
	printk("================ Get Check Sum =================\n");	
	#endif
	//Reset 
	ctp_reset();
	
	//Action 1 CMD( I/O )
	#ifdef RANDY_DEBUG	
	printk("[Driver ] IOCTL_IAP_MODE\n");
	#endif

	#ifndef RANDY_DEBUG
	msleep(100);
	#endif

	FWPoll_1();
	
	#ifdef RANDY_DEBUG
	printk("IOCTL_IAP_MODE\n");	
	#endif
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[10], 5);	
	
	if(ret < 0)
	{
		printk("[Driver ]IOCTL_IAP_MODE error!!!!!\n");
		return -1;
	}	
	
	if( !FWPoll_2() )
		return -1;
	
	//Action 2 Command
	#ifdef RANDY_DEBUG	
	printk("[Driver ] Read Check Sum\n");
	#endif

	CS_flag	 = 1;
	command_flag = 1;
	
	commandBuf[0] = command_list[14][0];
	commandBuf[1] = command_list[14][1];
	commandBuf[2] = command_list[14][2];
	commandBuf[3] = p_data[8];
	commandBuf[4] = p_data[9];
	
	ret = i2c_master_send(touch_i2c_client, 
						commandBuf, 5);	

	#ifdef RANDY_DEBUG						
	printk("[Driver] Cmd Check Sum ************** : %4x,%4x,%4x,%4x,%4x\n", commandBuf[0], commandBuf[1],commandBuf[2],commandBuf[3],commandBuf[4]);
	#endif
	
	if(ret < 0)
	{
		printk("[Driver ]Get Check Sum error!!!!!\n");
		return -1;
	}	
	
	msleep(1000);
	
	ack_flag = 0;
	
	/**********  randy delete for return 816 **************/
	#ifdef RANDY_DEBUG						
	printk("[Driver] Read Check Sum : %4x,%4x,%4x,%4x,%4x\n", ver_buf[0], ver_buf[1],ver_buf[2],ver_buf[3],ver_buf[4]);
	#endif
		
	return 0;
}

static int UpdateFW()
{
	int i, j;
	int ret = 0;
	int PkgCnt = 0;
	
	int DataLen ;
	unsigned char *p_data ;
	
	printk("================start update=================\n");	
	//Reset 
	ctp_reset();
	
	//Action 1 CMD( I/O )
	#ifdef RANDY_DEBUG	
	printk("[Driver ] IOCTL_IAP_MODE\n");
	#endif

	FWPoll_1();
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[10], 5);	
	
	if(ret < 0)
	{
		printk("[Driver ]IOCTL_IAP_MODE error!!!!!\n");
		return -1;
	}	
	
	if( !FWPoll_2() )
		return -1;	

	//Action 2 CMD( I/O )
	#ifdef RANDY_DEBUG	
	printk("[Driver ] IOCTL_ACTION2\n");
	#endif
		
	FWPoll_1();
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[12], 5);	

	if(ret < 0)
	{
		printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
		return -1;
	}	

	if( !FWPoll_2() )
		return -1;
	
	//Loop To Read File & Load to CHip
	#ifdef RANDY_DEBUG
	printk("================update firmware=================\n");		
	#endif
	
	p_data += 16;
	PkgCnt = (DataLen-16)/13;
	for( i=0; i<PkgCnt; i++ )
	{
		FWPoll_1();
		
		#ifdef RANDY_DEBUG
		for( j=0; j<13; j++ )
			printk("0x%.2x ", *(p_data+j));
		#endif
				
		ret = i2c_master_send(touch_i2c_client, 
							p_data, 13);			
		
		if(ret < 0)
		{
			printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
			return -1;
		}	

		if( !FWPoll_2() )
		{
			printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
			return -1;	
		}
					
		p_data += 13;
	}
	
	printk("================end update=================\n");	
	
	ctp_reset();
	return 0;
}

static unsigned char BL_Ver_M = 0;
static unsigned char BL_Ver_S = 0;

static int UpdateFW_2()
{
	int i, j;
	int ret = 0;
	int PkgCnt = 0;
	int PkgIdx = 0;
	int FWBuf_Idx = 0;
	int bLoadEnd = 0;
	int BigIdx = 0;
	
	int DataLen = sizeof(mg8696s_firmware);
	unsigned char *p_data = mg8696s_firmware;
	unsigned char FWBuffer[1031];
	
	printk("================start update=================\n");	
	//Reset 
	ctp_reset();	
	//ctp_wakeup();	
	//Action 1 CMD( I/O )
	#ifdef RANDY_DEBUG	
	printk("[Driver ] IOCTL_IAP_MODE\n");
	#endif

	msleep(100);
	FWPoll_1();
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[10], 5);	
	
	if(ret < 0)
	{
		printk("[Driver ]IOCTL_IAP_MODE error!!!!!\n");
		return -1;
	}	
	
	if( !FWPoll_2() )
		return -1;	
		
	//Record Bootloader Version
	BL_Ver_M = read_buf[3];
	BL_Ver_S = read_buf[4];
	
	printk( "BL Version : %X.%X\n", BL_Ver_M, BL_Ver_S );

	//Action 2 CMD( I/O )
	#ifdef RANDY_DEBUG	
	printk("[Driver ] IOCTL_ACTION2\n");
	#endif
		
	FWPoll_1();
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[12], 5);	

	if(ret < 0)
	{
		printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
		return -1;
	}	

	if( !FWPoll_2() )
		return -1;
		
		
	
	//Loop To Read File & Load to CHip
	#ifdef RANDY_DEBUG
	printk("================update firmware=================\n");		
	#endif
	
if( (BL_Ver_M>3) || ((BL_Ver_M==3)&&(BL_Ver_S>=3)) )
{
	printk( "Start Mass Update Process\n" );
	
	//Prepeare Buffer Parameters
	p_data += 16;
	PkgCnt = (DataLen-16)/13;
	PkgIdx = 0;		//Index for 13 byte package in fw array
	FWBuf_Idx = 0;	//Index for 1k fw buffer( byte )
	bLoadEnd = 0;	//Boolean to flag if the loading process ended
	BigIdx = 0;
	
	if( PkgCnt<0 )
	{
		printk( "FW Data Size Error(%d)\n", PkgCnt );
		return -1;
	}
	
	printk( "Total Package Count in FW Array : %d\n", PkgCnt );
	
	//Loop to Load FW
	while( !bLoadEnd )
	{	
			FWPoll_1();
			
			//Prepare Data
			memset( FWBuffer, 0xff, sizeof(FWBuffer) );
			
			//Prepare Header
			FWBuffer[0] = 0xAD;
			FWBuffer[1] = 0x15;
			
			FWBuffer[2] = p_data[2];
			FWBuffer[3] = p_data[3];

			FWBuffer[4] = 0x04;
			FWBuffer[5] = 0x00;			
			
			FWBuffer[6] = 0x00;
			
			printk( "Data(%d), AddH=%x, AddL=%x\n", BigIdx, FWBuffer[2], FWBuffer[3]  );
			BigIdx++;
			
			FWBuf_Idx = 7;
			
			
			while(  (FWBuf_Idx<1031) )
			{
				if( PkgIdx<PkgCnt )
				{
					for( j=5; j<13; j++ )
					{
						FWBuffer[FWBuf_Idx] = p_data[j];
						FWBuf_Idx++;
					}	
					
					//Increase Small Package Index & Pointer					
					p_data += 13;
					PkgIdx++;						
				}
				else
				{
					break;
				}
			}
			
			if( PkgIdx>=PkgCnt )
				bLoadEnd = 1;
			
			//Send I2C Data
			ret = i2c_master_send(touch_i2c_client, 
								FWBuffer, 1031);			
			
			if(ret < 0)
			{
				printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
				return -1;
			}	

			//Wait Data Result
			if( !FWPoll_2() )
			{
				printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
				return -1;	
			}	

			if( bLoadEnd )
				break;
	}
	
	printk( "Total Package Count Processed : %d\n", PkgIdx );
}
else
{	
	printk( "Start 8-Bytes Update Process\n" );
	
	p_data += 16;
	PkgCnt = (DataLen-16)/13;
	for( i=0; i<PkgCnt; i++ )
	{
		FWPoll_1();
		
		#ifdef RANDY_DEBUG
		for( j=0; j<13; j++ )
			printk("0x%.2x ", *(p_data+j));
		#endif
				
		ret = i2c_master_send(touch_i2c_client, 
							p_data, 13);			
		
		if(ret < 0)
		{
			printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
			return -1;
		}	

		if( !FWPoll_2() )
		{
			printk("[Driver ]IOCTL_ACTION2 error!!!!!\n");
			return -1;	
		}
					
		p_data += 13;
	}
}
	
	printk("================end update=================\n");	
	
	ctp_reset();
//	ctp_wakeup();
	return 0;
}

static int ReadVersion()
{
	u_int8_t ret = 0;

	#ifdef RANDY_DEBUG    
	printk("[Driver ] IOCTL_READ_VERSION\n");
	#endif
	
	memset( ver_buf, 0, sizeof(ver_buf) );
	
	ctp_reset();
	#ifdef RANDY_DEBUG						
	printk("[Driver ]DISABLE !!!!!!!\n");
	#endif
	msleep(2000);	
	ret = i2c_master_send(touch_i2c_client, command_list[2], 5);

	if(ret < 0)
	{
		printk("[Driver ]DISABLE  Error !!!!!!!\n");
		return -1;
	}
	
	#ifdef RANDY_DEBUG						
	printk("[Driver ]ENABLE !!!!!!!\n");
	#endif
	
	ret = i2c_master_send(touch_i2c_client, command_list[1], 5);

	if(ret < 0)
	{
		printk("[Driver ]ENABLE  Error !!!!!!!\n");
		return -1;
	}
	ver_flag = 1;
	command_flag = 1;

	ret = i2c_master_send(touch_i2c_client,command_list[4], 5);

	if(ret < 0)
	{
		printk("[Driver ]IOCTL_READ_VERSION error!!!!!\n");
		return -1;
	}
	ver_flag = 1;
	command_flag = 1;
	msleep(300);
//	msleep(800);
	/**********  randy delete for return 816 **************/
	ack_flag = 0;
	/**********  randy delete for return 816 **************/
	#ifdef RANDY_DEBUG						
	printk("[Driver] Read Fw Version : %4x,%4x,%4x,%4x,%4x\n", ver_buf[0], ver_buf[1],ver_buf[2],ver_buf[3],ver_buf[4]);
	#endif
	return 0;
}

static int ReadID()
{
	int ret = 0;
#ifdef RANDY_DEBUG
	printk("[Driver ] IOCTL_READ_ID\n");
#endif
	
	memset( ver_buf, 0, sizeof(ver_buf) );

	#ifdef RANDY_DEBUG						
	printk("[Driver ]DISABLE !!!!!!!\n");
	#endif
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[2], COMMAND_BIT);
	if(ret < 0)
	{
		printk("[Driver ]DISABLE  Error !!!!!!!\n");
		return -1;
	}
	#ifdef RANDY_DEBUG						
	printk("[Driver ]ENABLE !!!!!!!\n");
	#endif
	
	ret = i2c_master_send(touch_i2c_client, 
						command_list[1], COMMAND_BIT);
	if(ret < 0)
	{
		printk("[Driver ]ENABLE  Error !!!!!!!\n");
		return -1;
	}
	msleep(500);
	ver_flag = 1;
	command_flag = 1;

	ret = i2c_master_send(touch_i2c_client, 
						command_list[3], COMMAND_BIT);
	if(ret < 0)
	{
		printk("[Driver ]IOCTL_READ_VERSION error!!!!!\n");
		return -1;
	}
	ver_flag = 1;
	command_flag = 1;
	
	msleep(300);
	msleep(800);
	/**********  randy delete for return 816 **************/
	ack_flag = 0;
	/**********  randy delete for return 816 **************/
	#ifdef RANDY_DEBUG						
	printk("[Driver] Read ID Version : %4x,%4x,%4x,%4x,%4x\n", ver_buf[0], ver_buf[1],ver_buf[2],ver_buf[3],ver_buf[4]);
	#endif
}



static int IC_ProjID = 0;
static int IC_IC_ID  = 0;

static int IC_FW_Ver_1 = 0;
static int IC_FW_Ver_2 = 0;
static int IC_FW_Ver_3 = 0;

static int UF_ProjID = 0;
static int UF_IC_ID  = 0;

static int UF_FW_Ver_1 = 0;
static int UF_FW_Ver_2 = 0;
static int UF_FW_Ver_3 = 0;

static unsigned char IC_CS_L = 0;
static unsigned char IC_CS_H = 0;

static unsigned char UF_CS_L = 0;
static unsigned char UF_CS_H = 0;


static int CheckUpdate()
{	
	//int DataLen ;
  // unsigned char *p_data ;
   int DataLen = sizeof(mg8696s_firmware);
	unsigned char *p_data = mg8696s_firmware;

	if ((DataLen==0) || (p_data == NULL))
	{
		printk("No firmware for update\n");
		return 0;
	}
	
	//Get Version & Platform Info. from Update File Package
	ReadVersion();  // need check return
	#ifdef RANDY_DEBUG						
	printk("[Driver] Read Check Sum : %4x,%4x,%4x,%4x,%4x\n", ver_buf[0], ver_buf[1],ver_buf[2],ver_buf[3],ver_buf[4]);
	#endif
	if( (ver_buf[0]==0xde)&&(ver_buf[1]==0xee) )
	{	
		//Get Version Info
		IC_FW_Ver_1 	= ver_buf[3];
		IC_FW_Ver_2 	= ver_buf[4];
		IC_FW_Ver_3 	= ver_buf[2];
		
		#ifdef RANDY_DEBUG
		printk( "IC Version = %x.%x(%x)\n", IC_FW_Ver_1, IC_FW_Ver_2, IC_FW_Ver_3 );		
		#endif
	}
	else
	{
		printk( "[Drv:FU] Can't Get FW Version\n" );
		return 1;		//Condition 1 : Can Not Read F/W Version from IC -- Update			
	}
		
	ReadID();
	if( (ver_buf[0]==0xde)&&(ver_buf[1]==0xdd) )
	{	
		//Get ID Info
		IC_IC_ID 	= ver_buf[2];
		IC_ProjID 	= ver_buf[4];
		
		#ifdef RANDY_DEBUG
		printk( "IC Project ID=%X, IC ID=%X\n", IC_ProjID, IC_IC_ID );		
		#endif
	}
	else 
	{
		printk( "[Drv:FU] Can't Get Project ID\n" );
		return 1;		//Condition 1 : Can Not Read F/W Version from IC -- Update			
	}
	
	//Read Check Sum
   ReadFWCheckSum();
   
   IC_CS_L = ver_buf[3];
   IC_CS_H = ver_buf[4];
   
   #ifdef RANDY_DEBUG
   printk( "IC CheckSum=%X,%X\n", IC_CS_L, IC_CS_H );		
   #endif
	
	//Get Version & Platform Info. from IC
   UF_ProjID = p_data[2];
   UF_IC_ID  = p_data[0];

   UF_FW_Ver_1 = p_data[4];
   UF_FW_Ver_2 = p_data[5];
   UF_FW_Ver_3 = p_data[3];

   UF_CS_L = p_data[6];
   UF_CS_H = p_data[7];      
   
   #ifdef RANDY_DEBUG
   printk( "UF Version = %x.%x(%x)\n", UF_FW_Ver_1, UF_FW_Ver_2, UF_FW_Ver_3 );
   printk( "UF Project ID=%X, IC ID=%X\n", UF_ProjID, UF_IC_ID );		   
   printk( "UF CheckSum=%X,%X\n", UF_CS_L, UF_CS_H );		
   #endif
   
 
   
   //Compare Info. between IC & Update File
   if( (IC_IC_ID!=UF_IC_ID)||(IC_ProjID!=UF_ProjID) )
   {
		printk( "[Drv:FU] IC/Project ID Not Match\n" );
		return 1;		//Condition 2 : Platform Info. different from IC & Update File Package -- Update
   }

   //Comparing of forst Layer
   if( UF_FW_Ver_1>IC_FW_Ver_1 )
   {
		printk( "[Drv:FU] Older version in IC\n" );
		return 1;		//Condition 3 : Version number of Update Package larger than Version Number of IC -- Update
   }
   else if( UF_FW_Ver_1==IC_FW_Ver_1 )
   {
		//Comparing of Second Layer
		if( UF_FW_Ver_2>IC_FW_Ver_2 )
		{
			printk( "[Drv:FU] Older version in IC\n" );
			return 1;		//Condition 3 : Version number of Update Package larger than Version Number of IC -- Update
		}		
		else if( UF_FW_Ver_2==IC_FW_Ver_2 )
		{
			//Comparing of Third Layer
		//	if( UF_FW_Ver_3>IC_FW_Ver_3 )
			if( UF_FW_Ver_3 != IC_FW_Ver_3)
			{
				printk( "[Drv:FU] Older version in IC\n" );
				return 1;		//Condition 3 : Version number of Update Package larger than Version Number of IC -- Update
			}		
			else if( UF_FW_Ver_3==IC_FW_Ver_3 )
			{
				// Check Check Sum
				if( (UF_CS_L==IC_CS_L)&&(UF_CS_H==IC_CS_H) )
				{
					//Check Sum Successed
					printk( "Check Sum Successed\n" );
				}
				else
				{
					printk( "[Drv:FU] Check Sum Error\n" );
					return 1;
				}
			}
		}
   }  		
	
	return 0;
}

 static struct i2c_driver mg_driver;

static irqreturn_t mg_irq(int irq, void *_mg)
{
	if(imapx_pad_irq_pending(tsp_irq_index))
	    imapx_pad_irq_clear(tsp_irq_index);

		if (imapx_pad_get_indat(tsp_irq_index) == 0)
		{
			/* IRQ is triggered by FALLING code here */
			struct mg_data *mg = _mg;
			schedule_work(&mg->work);
		}else
		{
		    return IRQ_NONE;
		}
	return IRQ_HANDLED;
}

static void mg_mtreport(struct mg_data *mg)
{
	//delete for angry birds
	u16 x,y;
	x = mg->x;
    y = mg->y;
	
	
    if(1 == revert_x_flag){
     x= CAP_MAX_X - x;
      }
   if(1 == revert_y_flag){
    y = CAP_MAX_Y -y;
     }
    if(1 == exchange_x_y_flag) {
    
      swap(x, y);
    }

//	input_report_key(mg->dev, BTN_TOUCH, 1);
	input_report_abs(mg->dev, ABS_MT_TRACKING_ID, mg->id);
	input_report_abs(mg->dev, ABS_MT_TOUCH_MAJOR, mg->w);
	input_report_abs(mg->dev, ABS_MT_WIDTH_MAJOR, 1);

//    printk("line:%d x:%d, y:%d\n",__LINE__, x, y);
	input_report_abs(mg->dev, ABS_MT_POSITION_X, x); 
	input_report_abs(mg->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mg->dev);
}

static void mg_i2c_work(struct work_struct *work)
{
	int i = 0;
	u_int8_t ret = 0;
	struct mg_data *mg = container_of(work, struct mg_data, work);

#ifdef  TP_UPDATE_FW	
	//printk("**************%s*******\n", __func__);
	for(i = 0; i < BUF_SIZE; i ++)	
			read_buf[i] = 0;

	if(command_flag == 1) 
	{
		if( Act_FWPoll )
		{
			ret = i2c_master_recv(mg->client, read_buf, 5);
			if(ret < 0) {
				printk("Read error!!!!!\n");
				return;
			}		
		
			Rtn_FWPoll = FWRtnDecode();
			
			if( Rtn_FWPoll )
			{
				Act_FWPoll = 0;
				command_flag = 0;	

				#ifdef RANDY_DEBUG	
				printk("FW Update Return : " );
				for(i = 0; i < 5; i ++) 	
					printk("%4x ", read_buf[i]);
					
				printk("\n");
				#endif		
				
				return ;			
			}
		}
		else
		{
//			ret = i2c_master_recv(mg->client, read_buf, COMMAND_BIT);
			ret = i2c_master_recv(mg->client, read_buf, 5);
			if(ret < 0) {
				printk("Read error!!!!!\n");
				return;
			}		
		}
		
		//++++++add for touch calibration --------copy reg data to global var
		if(calibration_flag==1) {
			for(i=0;i<5;i++)
				cal_ack[i]=read_buf[i];
			for(i = 0; i < 5; i ++) 	
				printk("%4x", cal_ack[i]);
			printk("\n");

			if(cal_ack[4]==0x55)
				IsCalibration=1;
			else
				IsCalibration=0;
			//--------by Jay 2011/11/14
			printk("[Bohai]======touch calibration!!\n");
			calibration_flag=0;
		}
		//-------by Jay 2011/11/14
		#ifdef RANDY_DEBUG	
		printk("\nRecieve Command ACK : "); 
		for(i = 0; i < 5; i ++) 	
			printk("%4x", read_buf[i]);
		printk("\n");
		#endif
			//++++add for touch F/W
		if( ver_flag == 1 || id_flag == 1 || CS_flag==1 ) {
			printk("read version id cs\n");
			for(i = 0; i < 5; i ++)
				ver_buf[i] = read_buf[i];
						//++++++add for nomal mode
			msleep(20);
			ret =i2c_smbus_write_i2c_block_data(mg->client, 0, COMMAND_BIT, nomal_mode);
			if(ret<0) {
				printk("Wistron==>show f/w!!\n");
				return;
			}
			//++++++add for nomal mode
			ver_flag = 0;
			id_flag = 0;
		}
		command_flag = 0;
		/**********  randy delete for return 816 **************/
		ack_flag = 1;
		/**********  randy delete for return 816 **************/
		return;
	}
#endif
	//ret = i2c_smbus_read_i2c_block_data(mg->client, 0x0, BUF_SIZE, read_buf);
	ret = i2c_master_recv(mg->client, read_buf, BUF_SIZE  );

	if(ret < 0)
	{
		printk("Read error!!!!!\n");
		return;
	}

#ifdef RANDY_DEBUG	
	printk("\nRead: "); 
	for(i = 0; i < BUF_SIZE; i ++) 
	{
//	    read_buf[i] = read_buf[i+1]; 	
		printk("%4x", read_buf[i]);
	}
	printk("\n");	
#endif
    
	if(read_buf[MG_MODE] == 2){

	}
	else if (read_buf[MG_MODE] ==1)
	{
		if (read_buf[ACTUAL_TOUCH_POINTS] > 0) 
		{
			if(read_buf[ACTUAL_TOUCH_POINTS] >= 1) {
				mg->x = COORD_INTERPRET(read_buf[MG_POS_X_HI], read_buf[MG_POS_X_LOW]);
				mg->y = COORD_INTERPRET(read_buf[MG_POS_Y_HI], read_buf[MG_POS_Y_LOW]);
				mg->w = read_buf[MG_STATUS];
				mg->id = read_buf[MG_CONTACT_ID];
				mg_mtreport(mg);
			}
			if(read_buf[ACTUAL_TOUCH_POINTS] >= 2) {
				mg->x =	COORD_INTERPRET(read_buf[MG_POS_X_HI2], read_buf[MG_POS_X_LOW2]);
				mg->y = COORD_INTERPRET(read_buf[MG_POS_Y_HI2], read_buf[MG_POS_Y_LOW2]);
				mg->w = read_buf[MG_STATUS2];
				mg->id = read_buf[MG_CONTACT_ID2];
				mg_mtreport(mg);
			}

			if(read_buf[ACTUAL_TOUCH_POINTS] >= 3) {
				mg->x =	COORD_INTERPRET(read_buf[MG_POS_X_HI3], read_buf[MG_POS_X_LOW3]);
				mg->y = COORD_INTERPRET(read_buf[MG_POS_Y_HI3], read_buf[MG_POS_Y_LOW3]);
				mg->w = read_buf[MG_CONTACT_IDS3] & 0x0F;
				mg->id = (read_buf[MG_CONTACT_IDS3]>>4)&0x0F;
				mg_mtreport(mg);
			}		

			if(read_buf[ACTUAL_TOUCH_POINTS] >= 4) {
				mg->x =	COORD_INTERPRET(read_buf[MG_POS_X_HI4], read_buf[MG_POS_X_LOW4]);
				mg->y = COORD_INTERPRET(read_buf[MG_POS_Y_HI4], read_buf[MG_POS_Y_LOW4]);
				mg->w = read_buf[MG_CONTACT_IDS4] & 0x0F;
				mg->id = (read_buf[MG_CONTACT_IDS4]>>4)&0x0F;
				mg_mtreport(mg);
			}				

			if(read_buf[ACTUAL_TOUCH_POINTS] >= 5) {
				mg->x =	COORD_INTERPRET(read_buf[MG_POS_X_HI5], read_buf[MG_POS_X_LOW5]);
				mg->y = COORD_INTERPRET(read_buf[MG_POS_Y_HI5], read_buf[MG_POS_Y_LOW5]);
				mg->w = read_buf[MG_CONTACT_IDS5] & 0x0F;
				mg->id = (read_buf[MG_CONTACT_IDS5]>>4)&0x0F;
				mg_mtreport(mg);
			}					
			
		} else if (read_buf [2] == 0)
		{
//			input_report_key(mg->dev, BTN_TOUCH, 0);
			input_report_abs(mg->dev, ABS_MT_TOUCH_MAJOR, 0);
	        input_report_abs(mg->dev, ABS_MT_WIDTH_MAJOR, 0);
			input_mt_sync(mg->dev);
		}
		else {
         printk("other!\n");		
		}
		input_sync(mg->dev);
	}
}


static int mg_probe(struct i2c_client *client, const struct i2c_device_id *ids)
{
	int err = 0;
	struct mg_data *mg;
	struct input_dev *input_dev;
	int index;
	printk("====%s begin=====.  \n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_kfree;
	}

	mg = kzalloc(sizeof(struct mg_data), GFP_KERNEL);
	if (!mg)
		return -ENOMEM;

	mg->client = client;
	mg->pdata = client->dev.platform_data;
	touch_i2c_client=client;

	dev_info(&mg->client->dev, "device probing\n");
	i2c_set_clientdata(client, mg);
	mutex_init(&mg->lock);

	//Open Device Node for APK Update
    mg->firmware.minor = MISC_DYNAMIC_MINOR;
    mg->firmware.name = "MG_Update";
    mg->firmware.mode = S_IRWXUGO; 
    if (misc_register(&mg->firmware) < 0)
        printk("[mg]misc_register failed!!");
    else
        printk("[mg]misc_register finished!!"); 
		
	/* allocate input device for capacitive */
	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&mg->client->dev, "failed to allocate input device \n");
		goto exit_kfree;
	}
	
	//Initial Device Parameters
	input_dev->name = "mg-capacitive";
	input_dev->phys = "I2C";
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.vendor = 0x0EAF;
	input_dev->id.product = 0x1020;
	mg->dev = input_dev;
	
	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
//	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	
	set_bit(KEY_BACK, input_dev->keybit);
        set_bit(KEY_MENU, input_dev->keybit);
        set_bit(KEY_HOME, input_dev->keybit);
		
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 8, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 8, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 256, 0, 0);

//	if (mg->pdata->swap_xy) {
//		input_set_abs_params(input_dev, ABS_MT_POSITION_X,
//						0, mg->pdata->ymax, 0, 0);  
//		input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
//						0, mg->pdata->xmax, 0, 0);  
//	//} else {
		input_set_abs_params(input_dev, ABS_MT_POSITION_X,
						0, CAP_X_CORD, 0, 0);  
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
						0, CAP_Y_CORD, 0, 0);  
	//}

	//Register Device Object
	err = input_register_device(input_dev);
	if (err)
		goto exit_input;

//	ctp_ops.ts_wakeup();
	ctp_reset();
	mg->dev = input_dev;
	input_set_drvdata(mg->dev, private_ts);

//	init_completion(&mg_touch_event);
	INIT_WORK(&mg->work, mg_i2c_work);

	index = item_integer("ts.int", 1);
        tsp_irq_index = index;
       	imapx_pad_set_mode(0, 1, index);
        imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);
	_sui_irq_num = imapx_pad_irq_number(index);
	err =  request_irq(_sui_irq_num, mg_irq, 0, client->name, mg);
	if (err < 0) {
		pr_info( "goodix_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	
	mg->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	mg->early_suspend.suspend = mg_early_suspend;
	mg->early_suspend.resume = mg_late_resume;
	register_early_suspend(&mg->early_suspend);

#ifdef  TP_UPDATE_FW	
	//add for update FW
	if(item_exist("ts.downloader")){
		if(!CheckUpdate()){
			printk("FW is the newest version!\n");
		}
		else
		{
		       item_string(downloader_name, "ts.downloader", 0);
		       if(strcmp(downloader_name, "mg8696") == 0){	       
	    			printk("need update FW!\n");
				msleep(100);
				UpdateFW_2();
				msleep(50);

			}
	        }
	}
#endif

#ifdef RANDY_DEBUG	
	printk("\nregister_early_suspend  OK !!!!\n"); 
#endif

	return 0;
exit_set_irq_mode:
exit_irq_request_failed:
exit_input:
	input_unregister_device(mg->dev);
exit_kfree:
	kfree(mg);
	return err;
}

static int __devexit mg_remove(struct i2c_client *client)
{
	struct mg_data *mg = i2c_get_clientdata(client);
	
	free_irq(_sui_irq_num, mg);
	input_unregister_device(mg->dev);
	del_timer_sync(&mg->timer);
	kfree(mg);
	
	return 0;
}


static int mg_suspend(struct i2c_client *client, pm_message_t state)
{
	int ret;
//	struct mg_data *ts = i2c_get_clientdata(touch_i2c_client);
	printk("suspend!\n");
	
	ret = i2c_smbus_write_i2c_block_data(touch_i2c_client, 0, 5, command_list[5]);
	if (ret < 0)
		printk(KERN_ERR "mg_suspend: i2c_smbus_write_i2c_block_data failed\n");
	
	return 0;
}



static int mg_resume(struct i2c_client *client)
{
	//struct mg_data *ts = i2c_get_clientdata(touch_i2c_client);
   int ret;
   printk("resume\n");
    imapx_pad_set_mode(0, 1, tsp_irq_index);
    imapx_pad_irq_config(tsp_irq_index, INTTYPE_FALLING, FILTER_MAX);
   ret = i2c_smbus_write_i2c_block_data(touch_i2c_client, 0, 5, command_list[7]);
	if (ret < 0)
		printk(KERN_ERR "mg_resume: i2c_smbus_write_i2c_block_data   failed\n");
	else 
		printk("mg_resume: i2c_smbus_write_i2c_block_data  success\n");		
	return 0;
}



static void mg_early_suspend(struct early_suspend *h)
{
   // struct mg_data *ts;
#ifdef RANDY_DEBUG	
	printk("\nmg_early_suspend\n"); 
#endif
    printk("\nmg_early_suspend\n"); 
	mg_suspend(touch_i2c_client, PMSG_SUSPEND);

}

static void mg_late_resume(struct early_suspend *h)
{
#ifdef RANDY_DEBUG	
	printk("\nmg_late_resume\n"); 
#endif
	//struct mg_data *ts = i2c_get_clientdata(touch_i2c_client);
	mg_resume(touch_i2c_client);
}


static struct i2c_device_id mg_id_table[] = {
    /* the slave address is passed by i2c_boardinfo */
    {CTP_NAME,0 },
    {/* end of list */}
};

static struct i2c_driver mg_driver = {
        .probe 		= mg_probe,
	.remove 	= mg_remove,
	.driver = {
		.name	 = CTP_NAME,
       		 .owner = THIS_MODULE,
	},
	.id_table 	= mg_id_table,
};

static int __init mg_init(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret;

    printk("------mg8696 module init------\n");
    if(item_exist("ts.model"))
    {
        if(item_equal("ts.model", "mg8696", 0))
        {
	    memset(&info, 0, sizeof(struct i2c_board_info));
            info.addr = 0x44;  
            strlcpy(info.type, CTP_NAME, I2C_NAME_SIZE);
            adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
            
            if (!adapter) {
                printk("*******get_adapter error!********\n");
            }
	    client = i2c_new_device(adapter, &info);
	    ret = i2c_add_driver(&mg_driver);
	return ret;
	}
	else
		printk("%s: touchscreen is not mg8696\n", __func__);
    }
	else
		printk("%s: touchscreen is not exist\n", __func__);
    return -1;
}

static void mg_exit(void)
{
	i2c_del_driver(&mg_driver);
}

module_init(mg_init);
module_exit(mg_exit);

MODULE_DESCRIPTION("MG Touchscreen Driver");
MODULE_LICENSE("GPL v2");

