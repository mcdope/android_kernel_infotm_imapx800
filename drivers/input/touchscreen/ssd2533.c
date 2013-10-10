#include <asm/irq.h>                                                                                        
#include <asm/io.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <asm/gpio.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/earlysuspend.h>
#include <linux/device.h>
#include <linux/i2c-dev.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/cpufreq.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/rtc.h>
#include <linux/time.h>
//#include <mach/imapx_gpio.h>     
#include <mach/items.h>                                                                           
#include <mach/irqs.h>                                                                                      
//#include <mach/imapx_intr.h>                                                                                
//#include <mach/imapx_sysmgr.h>
#include <mach/hardware.h>
#include <mach/pad.h>

//#include "ssd253x-ts_7inch.h"
#include "ssd253x.h"

//#define SSD_DBG_FUCTION 1
#ifdef 	SSD_DBG_FUCTION
#define DBG_FUNC(x...) printk(x)
#else
#define DBG_FUNC(x...)
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#define DEVICE_ID_REG                  2
#define VERSION_ID_REG                 3
#define AUTO_INIT_RST_REG          		 68
#define EVENT_STATUS                   121
#define EVENT_MSK_REG                  122
#define IRQ_MSK_REG                    123
#define FINGER01_REG                   124
#define EVENT_STACK                    128
#define EVENT_FIFO_SCLR                135
#define TIMESTAMP_REG                  136
#define SELFCAP_STATUS_REG             185		

//static int intpin = 0;
//static void* __iomem gpio_addr = NULL;  

static int tsp_irq_index;
static int SSDS53X_SCREEN_MAX_X;
static int SSDS53X_SCREEN_MAX_Y;

#define SSD253X_I2C_NAME	"ssd2533"
#define SSD2533_SLAVE_ADDR 0x48
/*
struct ChipSetting {
	char No;
	char Reg;
	char Data1;
	char Data2;
};*/
extern int get_lcd_width(void);
extern int get_lcd_height(void);


void deviceReset(struct i2c_client *client);
void deviceResume(struct i2c_client *client);
void deviceSuspend(struct i2c_client *client);
void SSD253xdeviceInit1(struct i2c_client *client);
void SSD253xdeviceInit(struct i2c_client *client); 

//static int ssd253x_ts_open(struct input_dev *dev);
//static void ssd253x_ts_close(struct input_dev *dev);
static int ssd253x_ts_suspend(struct i2c_client *client, pm_message_t mesg);
static int ssd253x_ts_resume(struct i2c_client *client);
#ifdef CONFIG_HAS_EARLYSUSPEND
static void ssd253x_ts_early_suspend(struct early_suspend *h);
static void ssd253x_ts_late_resume(struct early_suspend *h);
#endif /* CONFIG_HAS_EARLYSUSPEND */

static enum hrtimer_restart ssd253x_ts_timer(struct hrtimer *timer);
static irqreturn_t ssd253x_ts_isr(int irq, void *dev_id);
static struct workqueue_struct *ssd253x_wq;

struct ssl_ts_priv {
	struct i2c_client *client;
	struct input_dev *input;
	struct hrtimer timer;
	struct work_struct  ssl_work;
#ifdef	CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif 

	int irq;
	int use_irq;
	int FingerNo;
	int FingerX[FINGERNO];
	int FingerY[FINGERNO];
	int FingerP[FINGERNO];

	int Resolution;
	int EventStatus;
	int FingerDetect;

	int sFingerX[FINGERNO];
	int sFingerY[FINGERNO];
	int pFingerX[FINGERNO];
	int pFingerY[FINGERNO];
};

int ssd253x_record,ssd253x_current,ssd253x_timer_flag; //add by hjc

static void ssd253x_reset(void)
{
	int index;
	index = item_integer("ts.reset", 1);

	DBG_FUNC("----jason--- ts.reset=%d\n",index);
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0, 1, index);/*output*/
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	mdelay(5);
	imapx_pad_set_outdat(0, 1, index);/*output 0*/
	mdelay(10);
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	mdelay(10);
}

static int ssd253x_init_irq(void)
{
	int index, irq_num;

	index = item_integer("ts.int", 1);
	DBG_FUNC("CTP INT pad index %d\n", index);
	tsp_irq_index = index;
	irq_num = imapx_pad_irq_number(index);/* get irq */
	if(!irq_num)
		return -EINVAL;
	
	DBG_FUNC("CTP irq %d\n", irq_num);

	imapx_pad_set_mode(0, 1, index);/* func mode */
	imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);/* set trigger mode and filter */
	return irq_num;
}

int ReadRegister(struct i2c_client *client,uint8_t reg,int ByteNo)
{
	unsigned char buf[4];
	struct i2c_msg msg[2];
	int ret;
	u16 flags = 0;

	memset(buf, 0xFF, sizeof(buf));
	msg[0].addr = client->addr;
	msg[0].flags = 0|flags|I2C_M_NOSTART;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD|flags;
	msg[1].len = ByteNo;
	msg[1].buf = buf;

	ret = i2c_transfer(client->adapter, msg, 2);

	if(ret<0)	DBG_FUNC("		ReadRegister: i2c_transfer Error !\n");
	else		DBG_FUNC("		ReadRegister: i2c_transfer OK !\n");

	if(ByteNo==1) return (int)((unsigned int)buf[0]<<0);
	if(ByteNo==2) return (int)((unsigned int)buf[1]<<0)|((unsigned int)buf[0]<<8);
	if(ByteNo==3) return (int)((unsigned int)buf[2]<<0)|((unsigned int)buf[1]<<8)|((unsigned int)buf[0]<<16);
	if(ByteNo==4) return (int)((unsigned int)buf[3]<<0)|((unsigned int)buf[2]<<8)|((unsigned int)buf[1]<<16)|(buf[0]<<24);
	return 0;
}

void WriteRegister(struct i2c_client *client,uint8_t Reg,unsigned char Data1,unsigned char Data2,int ByteNo)
{	
	struct i2c_msg msg;
	unsigned char buf[4];
	int ret;

	buf[0]=Reg;
	buf[1]=Data1;
	buf[2]=Data2;
	buf[3]=0;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = ByteNo+1;
	msg.buf = (char *)buf;
	ret = i2c_transfer(client->adapter, &msg, 1);

	if(ret<0)	DBG_FUNC("		WriteRegister: i2c_master_send Error !\n");
	else		DBG_FUNC("		WriteRegister: i2c_master_send OK !\n");
}

void SSD253xdeviceInit1(struct i2c_client *client) 
{	
#ifdef	SSD2533FIXEDCODE
	int i;
	mdelay(600); //SSD2533 ESD2 EEPROM VERSION
	for(i=0;i<sizeof(ssd253xcfgTable1)/sizeof(ssd253xcfgTable1[0]);i++)
	{
		WriteRegister(	client,ssd253xcfgTable1[i].Reg,
				ssd253xcfgTable1[i].Data1,ssd253xcfgTable1[i].Data2,
				ssd253xcfgTable1[i].No);
	}
#endif
}

void SSD253xdeviceInit(struct i2c_client *client)
{	
	int i;
	if( item_equal("board.name", "HY971", 0)== 1) {
		for(i=0;i<sizeof(ssd253xcfgTable_hy971)/sizeof(ssd253xcfgTable_hy971[0]);i++)
		{
			WriteRegister(client,ssd253xcfgTable_hy971[i].Reg,
					ssd253xcfgTable_hy971[i].Data1,ssd253xcfgTable_hy971[i].Data2,
					ssd253xcfgTable_hy971[i].No);
		}
		mdelay(150);
	}
	else {
		for(i=0;i<sizeof(ssd253xcfgTable)/sizeof(ssd253xcfgTable[0]);i++)
		{
			WriteRegister(client,ssd253xcfgTable[i].Reg,
					ssd253xcfgTable[i].Data1,ssd253xcfgTable[i].Data2,
					ssd253xcfgTable[i].No);
		}
		mdelay(150);
	}
}

void deviceReset(struct i2c_client *client)
{	
	int i;
	for(i=0;i<sizeof(Reset)/sizeof(Reset[0]);i++)
	{
		WriteRegister(	client,Reset[i].Reg,
				Reset[i].Data1,Reset[i].Data2,
				Reset[i].No);
	}
	mdelay(200);
	SSD253xdeviceInit1(client);
}

void deviceResume(struct i2c_client *client)    
{	
	#if 1
	/*
	//RST pin pull down
	imapx_gpio_pull(intpin,0,IG_NMSL);
	imapx_gpio_setpin(intpin, 0,IG_NMSL);
	//gpio_direction_output(SHUTDOWN_PORT, 0);
	mdelay(5);
	imapx_gpio_pull(intpin,1,IG_NMSL);
	imapx_gpio_setpin(intpin, 1 ,IG_NMSL);
	//gpio_direction_output(SHUTDOWN_PORT, 1);
	mdelay(2);
	*/
	ssd253x_reset();
	ssd253x_init_irq();
	deviceReset(client);
	SSD253xdeviceInit(client);
	#else
	int i;
	//int timeout=10;
	//int status;
	for(i=0;i<sizeof(Resume)/sizeof(Resume[0]);i++)
	{
		WriteRegister(	client,Resume[i].Reg,
				Resume[i].Data1,Resume[i].Data2,
				Resume[i].No);
		mdelay(150);
	}
	/*
	do {
		status=ReadRegister(client,0x26,1);
		DBG_FUNC("		deviceResume: status : %d !\n",status);
		if(status==Resume[2].Data1) break;
		mdelay(1);
	}while(timeout--); // Check the status
	*/
	#endif
	
}

void deviceSuspend(struct i2c_client *client)
{	
	#if 1
	ssd253x_reset();

	#else
	//int i;
	//int timeout=10;
	//int status;
	
	/*
	WriteRegister(	client,Suspend[0].Reg,
			Suspend[0].Data1,Suspend[0].Data2,
			Suspend[0].No);
	do {
		status=ReadRegister(client,0x26,1);
		if(status==Suspend[0].Data1) break;
		mdelay(1);				
	}while(timeout--);
	*/
	
	for(i=0;i<sizeof(Suspend)/sizeof(Suspend[0]);i++)
	{
		WriteRegister(	client,Suspend[i].Reg,
				Suspend[i].Data1,Suspend[i].Data2,
				Suspend[i].No);
		mdelay(200);
	}
	#endif
}

#define Mode RunningAverageMode
#define Dist RunningAverageDist
void RunningAverage(unsigned short *xpos,unsigned short *ypos,int No,struct ssl_ts_priv *ssl_priv)
{	
	int FilterMode[4][2]={{0,8},{5,3},{6,2},{7,1}};
	int dx,dy;
	int X,Y;

	X=*xpos;
	Y=*ypos;
	if((ssl_priv->pFingerX[No]!=0x0FFF)&&(X!=0x0FFF))
	{
		dx=abs(ssl_priv->pFingerX[No]-X);
		dy=abs(ssl_priv->pFingerY[No]-Y);
		if(dx+dy<Dist*64)
		{
			ssl_priv->pFingerX[No]=(FilterMode[Mode][0]*ssl_priv->pFingerX[No]+FilterMode[Mode][1]*X)/8;
			ssl_priv->pFingerY[No]=(FilterMode[Mode][0]*ssl_priv->pFingerY[No]+FilterMode[Mode][1]*Y)/8;
		}
		else
		{
			ssl_priv->pFingerX[No]=X;
			ssl_priv->pFingerY[No]=Y;
		}
	}
	else
	{
		ssl_priv->pFingerX[No]=X;
		ssl_priv->pFingerY[No]=Y;
	}
	*xpos=ssl_priv->pFingerX[No];
	*ypos=ssl_priv->pFingerY[No];
}

void FingerCheckSwap(int *FingerX,int *FingerY,int *FingerP,int FingerNo,int *sFingerX,int *sFingerY)
{
  	int i,j;
  	int index1,index2;
  	int Vx,Vy;
  	int Ux,Uy;
  	int R1x,R1y;
  	int R2x,R2y;
	for(i=0;i<FingerNo;i++)
  	{
 		index1=i;
	    	if( FingerX[index1]!=0xFFF)
		if(sFingerX[index1]!=0xFFF) 
		{
			for(j=i+1;j<FingerNo+3;j++)
			{
				index2=j%FingerNo;
	    			if( FingerX[index2]!=0xFFF)
				if(sFingerX[index2]!=0xFFF) 
		    		{
					Ux=sFingerX[index1]-sFingerX[index2];
					Uy=sFingerY[index1]-sFingerY[index2];      
					Vx= FingerX[index1]- FingerX[index2];
					Vy= FingerY[index1]- FingerY[index2];					

					R1x=Ux-Vx;
					R1y=Uy-Vy;
					R2x=Ux+Vx;
					R2y=Uy+Vy;
							
					R1x=R1x*R1x;
					R1y=R1y*R1y; 
					R2x=R2x*R2x;
					R2y=R2y*R2y;

					if(R1x+R1y>R2x+R2y)
				    	{
				    		Ux=FingerX[index1];
						Uy=FingerY[index1];
						Vx=FingerP[index1];
							          
						FingerX[index1]=FingerX[index2];
						FingerY[index1]=FingerY[index2];
						FingerP[index1]=FingerP[index2];
							
						FingerX[index2]=Ux;
						FingerY[index2]=Uy;
						FingerP[index2]=Vx;
					}
					break;
			    	}
			}
		}
  	}        
  	for(i=0;i<FingerNo;i++)
  	{
    		sFingerX[i]=FingerX[i];
    		sFingerY[i]=FingerY[i];
  	}
}


#ifdef SSD253x_CUT_EDGE
static int ssd253x_ts_cut_edge(unsigned short pos,unsigned short x_y)
{
	u8 cut_value = 10; //cut_value < 32
	if(pos == 0xfff)
	{
		return pos;
	}
	if(x_y) //xpos
	{
		if(pos < 16)
			pos = cut_value + pos*(48 - cut_value) / 16;
		else if(pos > (XPOS_MAX - 16) )
			pos = XPOS_MAX + 16 + (pos - (XPOS_MAX -16))*(48 - cut_value) / 16;
		else
			pos = pos + 32;

		pos = SSDS53X_SCREEN_MAX_X * pos / (DRIVENO * 64);
		return pos;
	}
	else    //ypos
	{
		if(pos < 16)
			pos = cut_value + pos*(48 - cut_value) / 16;
		else if(pos > (YPOS_MAX - 16) )
			pos = YPOS_MAX + 16 + (pos - (YPOS_MAX -16))*(48 - cut_value) / 16;
		else
			pos = pos + 32;
		
		pos = SSDS53X_SCREEN_MAX_Y* pos /(SENSENO * 64); 
		return pos;		
	}
	
	
}
#endif

static void ssd253x_ts_work(struct work_struct *work)
{
	int i;
	unsigned short xpos=0, ypos=0;
	int FingerInfo;
	int EventStatus;
	int FingerX[FINGERNO];
	int FingerY[FINGERNO];
	int FingerP[FINGERNO];
	int clrFlag=0;
	int timer_status;
#if 0
	#ifdef SSD253x_TOUCH_KEY
	u8 btn_status;
	static bool key[4] = { 0, 0, 0, 0 }; 
	#endif
#endif	

	struct ssl_ts_priv *ssl_priv = container_of(work,struct ssl_ts_priv,ssl_work);
	
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_work!                  |\n");
	DBG_FUNC("+-----------------------------------------+\n");

	if(!ssd253x_timer_flag)
	{
		timer_status = ReadRegister(ssl_priv->client,TIMESTAMP_REG,2);
		if(!ssd253x_record)                                      
		{
				ssd253x_record = timer_status/1000;   			
		}
		
		ssd253x_current = timer_status/1000;               
		
		if((ssd253x_current - ssd253x_record) > 10)
		{
		WriteRegister(ssl_priv->client,AUTO_INIT_RST_REG,0x00,0x00,1);
		ssd253x_record = 0;
		ssd253x_timer_flag = 1;
		}
	 }
#if 0
	#ifdef SSD253x_TOUCH_KEY
		KeyInfo = ReadRegister(ssl_priv->client,0xB9,1);
		if(KeyInfo<0)	DBG_FUNC("		ssd253x_ts_work: i2c_transfer Error !\n");

		DBG_FUNC("ssd253x_ts_work read 0xB9,KeyInfo is %x\n",KeyInfo);
		if(KeyInfo & 0x0f){
			switch(KeyInfo & 0x0f){
			case 1:
			key[0] = 1;
			input_event(ssl_priv->input,EV_KEY, key_code[0], 1);
			break;
			case 2:
			key[1] = 1;
			input_event(ssl_priv->input,EV_KEY, key_code[1], 1);
			break;
			case 4:
			key[2] = 1;
			input_event(ssl_priv->input,EV_KEY, key_code[2], 1);
			break;
			case 8:
			key[3] = 1;
			input_event(ssl_priv->input,EV_KEY, key_code[3], 1);
			break;
			default:
			break;
			}
			hrtimer_start(&ssl_priv->timer, ktime_set(0, MicroTimeTInterupt), HRTIMER_MODE_REL);
		goto work_touch;
		}
		for(i = 0; i < 4; i++)
		{
			if(key[i])
			{
				key[i] = 0;
				input_event(ssl_priv->input, EV_KEY, key_code[i], 0);
			}
		}
	work_touch:
	#endif
#endif
	EventStatus = ReadRegister(ssl_priv->client,EVENT_STATUS,2)>>4;
	ssl_priv->FingerDetect=0;
	for(i=0;i<ssl_priv->FingerNo;i++)
	{
		if((EventStatus>>i)&0x1)
		{
			FingerInfo=ReadRegister(ssl_priv->client,FINGER01_REG+i,4);
			xpos = ((FingerInfo>>4)&0xF00)|((FingerInfo>>24)&0xFF);
			ypos = ((FingerInfo>>0)&0xF00)|((FingerInfo>>16)&0xFF);
			//width= ((FingerInfo>>4)&0x00F);	

			if(xpos!=0xFFF)
			{
				ssl_priv->FingerDetect++;

				#ifdef SSD253x_CUT_EDGE
				xpos = ssd253x_ts_cut_edge(xpos, 1);
				ypos = ssd253x_ts_cut_edge(ypos, 0);
				#endif
			}
			else 
			{
				// This part is to avoid asyn problem when the finger leaves
				//DBG_FUNC("		ssd253x_ts_work: Correct %x\n",EventStatus);
				EventStatus=EventStatus&~(1<<i);
				clrFlag=1;
			}
		}
		else
		{
			xpos=ypos=0xFFF;
			//width=0;
			clrFlag=1;
		}
		FingerX[i]=xpos;
		FingerY[i]=ypos;
		//FingerP[i]=width;
	}
	if(ssl_priv->use_irq==1) ;//enable_irq(ssl_priv->irq);
	if(ssl_priv->use_irq==2)
	{
		if(ssl_priv->FingerDetect==0) ;//enable_irq(ssl_priv->irq);
		else hrtimer_start(&ssl_priv->timer, ktime_set(0, MicroTimeTInterupt), HRTIMER_MODE_REL);
	}
	if(clrFlag) WriteRegister(ssl_priv->client,EVENT_FIFO_SCLR,0x01,0x00,1);

	if(ssl_priv->input->id.product==0x2533)
	if(ssl_priv->input->id.version==0x0101) 
		FingerCheckSwap(FingerX,FingerY,FingerP,ssl_priv->FingerNo,ssl_priv->sFingerX,ssl_priv->sFingerY);

	for(i=0;i<ssl_priv->FingerNo;i++)
	{
		xpos=FingerX[i];
		ypos=FingerY[i];
		//width=FingerP[i];
		if(ssl_priv->input->id.product==0x2533)
		{
			if(ssl_priv->input->id.version==0x0101) RunningAverage(&xpos,&ypos,i,ssl_priv);
			if(ssl_priv->input->id.version==0x0102) RunningAverage(&xpos,&ypos,i,ssl_priv);
		}

		if(xpos!=0xFFF)
		{
			input_report_abs(ssl_priv->input, ABS_MT_TRACKING_ID, i);  
			input_report_abs(ssl_priv->input, ABS_MT_TOUCH_MAJOR, 1);
			input_report_abs(ssl_priv->input, ABS_MT_PRESSURE, 1);
			input_report_key(ssl_priv->input, BTN_TOUCH, 1);
			input_report_abs(ssl_priv->input, ABS_MT_POSITION_X, xpos);
			input_report_abs(ssl_priv->input, ABS_MT_POSITION_Y, ypos);
			input_report_abs(ssl_priv->input, ABS_MT_WIDTH_MAJOR, 200);//width);
			input_mt_sync(ssl_priv->input);
			
			if(i==0) DBG_FUNC("		ssd253x_ts_work: X = %d , Y = %d, W = 0x%d\n",xpos,ypos,width);
		}
		else if(ssl_priv->FingerX[i]!=0xFFF)
		{
			input_report_abs(ssl_priv->input, ABS_MT_TRACKING_ID, i);
			input_report_abs(ssl_priv->input, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(ssl_priv->input, ABS_MT_PRESSURE, 0);
			input_report_key(ssl_priv->input, BTN_TOUCH, 0);
			input_mt_sync(ssl_priv->input);
			if(i==0) DBG_FUNC("	release	ssd253x_ts_work: X = %d , Y = %d, W = %d\n",xpos,ypos,width);
		}
		ssl_priv->FingerX[i]=FingerX[i];
		ssl_priv->FingerY[i]=FingerY[i];
		//ssl_priv->FingerP[i]=width;
	}		
	ssl_priv->EventStatus=EventStatus;	
	input_sync(ssl_priv->input);
}

static int ssd253x_ts_probe(struct i2c_client *client,const struct i2c_device_id *idp)
{
	struct ssl_ts_priv *ssl_priv;
	struct input_dev *ssl_input;
	int error;
	int i;
  	int err;
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_probe!                 |\n");
	DBG_FUNC("+-----------------------------------------+\n");

	SSDS53X_SCREEN_MAX_X = get_lcd_width();
	SSDS53X_SCREEN_MAX_Y = get_lcd_height();
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		DBG_FUNC("ssd253x_ts_probe: need I2C_FUNC_I2C\n");
		return -ENODEV;
	}
	else
	{
		DBG_FUNC("ssd253x_ts_probe: i2c Check OK!\n");
		DBG_FUNC("ssd253x_ts_probe: i2c_client name : %s\n",client->name);
	}

	ssl_priv = kzalloc(sizeof(*ssl_priv), GFP_KERNEL);
	if (!ssl_priv)
	{
		DBG_FUNC("ssd253x_ts_probe: kzalloc Error!\n");
		error=-ENODEV;
		goto	err0;
	}
	else
	{
		DBG_FUNC("ssd253x_ts_probe: kzalloc OK!\n");
	}

	ssd253x_reset();
	
	dev_set_drvdata(&client->dev, ssl_priv);
	
	ssl_input = input_allocate_device();
	if (!ssl_input)
	{
		DBG_FUNC("ssd253x_ts_probe: input_allocate_device Error\n");
		error=-ENODEV;
		goto	err1;
	}
	else
	{
		DBG_FUNC("ssd253x_ts_probe: input_allocate_device OK\n");
	}
#if 0
	ssl_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN) ;
	ssl_input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) | BIT_MASK(BTN_2);
#else
	__set_bit(EV_KEY, ssl_input->evbit);
	__set_bit(EV_ABS, ssl_input->evbit);
	__set_bit(EV_SYN, ssl_input->evbit);
	__set_bit(BTN_TOUCH, ssl_input->keybit);
	__set_bit(ABS_MT_TOUCH_MAJOR, ssl_input->absbit);
	__set_bit(ABS_MT_TRACKING_ID, ssl_input->absbit);
	__set_bit(ABS_MT_POSITION_X, ssl_input->absbit);
	__set_bit(ABS_MT_POSITION_Y, ssl_input->absbit);
#endif

	
	ssl_input->name = client->name;
	ssl_input->id.bustype = BUS_I2C;
	ssl_input->id.vendor  = 0x2878; // Modify for Vendor ID
	ssl_input->dev.parent = &client->dev;
	//ssl_input->open = ssd253x_ts_open;
	//ssl_input->close = ssd253x_ts_close;

	input_set_drvdata(ssl_input, ssl_priv);
	ssl_priv->client = client;
	ssl_priv->input = ssl_input;
	ssl_priv->use_irq = ENABLE_INT;
//	ssl_priv->irq = TOUCH_INT_NO;
	ssl_priv->FingerNo=FINGERNO;
	ssl_priv->Resolution=64;

	for(i=0;i<ssl_priv->FingerNo;i++)
	{
		//ssl_priv->FingerP[i]=0;
		// For Finger Check Swap
		ssl_priv->sFingerX[i]=0xFFF;
		ssl_priv->sFingerY[i]=0xFFF;

		// For Adaptive Running Average
		ssl_priv->pFingerX[i]=0xFFF;
		ssl_priv->pFingerY[i]=0xFFF;
	}

	deviceReset(client);
	//DBG_FUNC("SSL Touchscreen I2C Address: 0x%02X\n",client->addr);
	ssl_input->id.product = ReadRegister(client, DEVICE_ID_REG,2);
	ssl_input->id.version = ReadRegister(client,VERSION_ID_REG,2);
	ssl_input->id.product = ReadRegister(client, DEVICE_ID_REG,2);
	ssl_input->id.version = ReadRegister(client,VERSION_ID_REG,2);
	DBG_FUNC("SSL Touchscreen Device ID  : 0x%04X\n",ssl_input->id.product);
	DBG_FUNC("SSL Touchscreen Version ID : 0x%04X\n",ssl_input->id.version);

	SSD253xdeviceInit(client);
	WriteRegister(client,EVENT_FIFO_SCLR,0x01,0x00,1); // clear Event FiFo
	DBG_FUNC("ssd253X_ts_probe: %04XdeviceInit OK!\n",ssl_input->id.product);

	if(ssl_priv->input->id.product==0x2531)		
		ssl_priv->Resolution=32;
	else if(ssl_priv->input->id.product==0x2533)	
		ssl_priv->Resolution=64;
	else
	{
		DBG_FUNC("ssd253x_ts_probe: ssl_input->id.product Error\n");
		error=-ENODEV;
		goto	err1;
	}
#if 0
	input_set_abs_params(ssl_input, ABS_MT_TRACKING_ID, 0,16, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_TOUCH_MAJOR, 0, 1, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_WIDTH_MAJOR, 0, 8, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_POSITION_X,  0,SSDS53X_SCREEN_MAX_X+1, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_POSITION_Y,  0,SSDS53X_SCREEN_MAX_Y+1, 0, 0);
#else
	input_set_abs_params(ssl_input, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_POSITION_X, 0, SSDS53X_SCREEN_MAX_X, 0, 0);
	input_set_abs_params(ssl_input, ABS_MT_POSITION_Y, 0, SSDS53X_SCREEN_MAX_Y, 0, 0);
#endif
	if(item_exist("ts.keynum") && item_exist("ts.key")) {
		set_bit(KEY_MENU, ssl_input->keybit);
		set_bit(KEY_HOME, ssl_input->keybit);
		set_bit(KEY_BACK, ssl_input->keybit);
		set_bit(KEY_SEARCH, ssl_input->keybit);
	}
	
	INIT_WORK(&ssl_priv->ssl_work, ssd253x_ts_work);
	error = input_register_device(ssl_input);
	if(error)
	{
		DBG_FUNC("		ssd253x_ts_probe: input_register_device input Error!\n");
		error=-ENODEV;
		goto	err1;
	}
	else
	{
		DBG_FUNC("		ssd253x_ts_probe: input_register_device input OK!\n");
	}
	
	ssl_priv->irq = ssd253x_init_irq();
	disable_irq_nosync(ssl_priv->irq);
	if (request_irq(ssl_priv->irq, ssd253x_ts_isr, /*IRQF_DISABLED*/0, client->name,  ssl_priv)) 
	{
		err = -ENODEV;
		goto err2;
	}
	
	enable_irq(ssl_priv->irq);
	if((ssl_priv->use_irq==0)||(ssl_priv->use_irq==2))
	{
		hrtimer_init(&ssl_priv->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ssl_priv->timer.function = ssd253x_ts_timer;
		DBG_FUNC("		ssd253x_ts_probe: timer_init OK!\n");
	}
        hrtimer_start(&ssl_priv->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

#ifdef	CONFIG_HAS_EARLYSUSPEND
	ssl_priv->early_suspend.suspend = ssd253x_ts_early_suspend;
	ssl_priv->early_suspend.resume  = ssd253x_ts_late_resume;
	ssl_priv->early_suspend.level   = EARLY_SUSPEND_LEVEL_BLANK_SCREEN+1;
	register_early_suspend(&ssl_priv->early_suspend);
#endif 
	return 0;

err2:	input_unregister_device(ssl_input);
err1:	input_free_device(ssl_input);
/*
err_wakeup_request:
err_int_request:
err_ioremap_failed:
    if(gpio_addr){
        iounmap(gpio_addr);
    }
*/
	kfree(ssl_priv);
err0:	dev_set_drvdata(&client->dev, NULL);
	return error;
}
#if 0
static int ssd253x_ts_open(struct input_dev *dev)
{
	struct ssl_ts_priv *ssl_priv = input_get_drvdata(dev);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_open!                  |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	deviceResume(ssl_priv->client);
	if(ssl_priv->use_irq) 
		enable_irq(ssl_priv->irq);
	else 
		hrtimer_start(&ssl_priv->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	return 0;
}

static void ssd253x_ts_close(struct input_dev *dev)
{
	struct ssl_ts_priv *ssl_priv = input_get_drvdata(dev);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_close!                 |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	deviceSuspend(ssl_priv->client);
	ssd253x_timer_flag = 0;
	if((ssl_priv->use_irq==0)||(ssl_priv->use_irq==2)) hrtimer_cancel(&ssl_priv->timer);
	if((ssl_priv->use_irq==1)||(ssl_priv->use_irq==2)) 
		disable_irq(ssl_priv->irq);// free_irq()\B0\B4ƽ̨ʵ\BC\CA\C7\E9\BF\F6ʹ\D3û\F2disable
}
#endif
static int ssd253x_ts_resume(struct i2c_client *client)
{
	struct ssl_ts_priv *ssl_priv = dev_get_drvdata(&client->dev);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_resume!                |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	deviceResume(client);
	ssd253x_timer_flag = 0;
	if(ssl_priv->use_irq)
		enable_irq(ssl_priv->irq);
	else 
		hrtimer_start(&ssl_priv->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	return 0;
}

static int ssd253x_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct ssl_ts_priv *ssl_priv = dev_get_drvdata(&client->dev);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_suspend!               |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	deviceSuspend(client);		
	if((ssl_priv->use_irq==0)||(ssl_priv->use_irq==2)) hrtimer_cancel(&ssl_priv->timer);
	if((ssl_priv->use_irq==1)||(ssl_priv->use_irq==2)) 
	disable_irq(ssl_priv->irq);	// free_irq()\B0\B4ƽ̨ʵ\BC\CA\C7\E9\BF\F6ʹ\D3û\F2disable
	return 0;
}

#ifdef	CONFIG_HAS_EARLYSUSPEND
static void ssd253x_ts_late_resume(struct early_suspend *h)
{
	struct ssl_ts_priv *ssl_priv = container_of(h, struct ssl_ts_priv, early_suspend);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_late_resume!           |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	ssd253x_ts_resume(ssl_priv->client);
}
static void ssd253x_ts_early_suspend(struct early_suspend *h)
{
	struct ssl_ts_priv *ssl_priv = container_of(h, struct ssl_ts_priv, early_suspend);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_early_suspend!         |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	ssd253x_ts_suspend(ssl_priv->client, PMSG_SUSPEND);
}
#endif

static int ssd253x_ts_remove(struct i2c_client *client)
{
	struct ssl_ts_priv *ssl_priv = dev_get_drvdata(&client->dev);	
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_remove !               |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	if((ssl_priv->use_irq==0)||(ssl_priv->use_irq==2)) hrtimer_cancel(&ssl_priv->timer);
	if((ssl_priv->use_irq==1)||(ssl_priv->use_irq==2)) 
		disable_irq(ssl_priv->irq);// free_irq()\B0\B4ƽ̨ʵ\BC\CA\C7\E9\BF\F6ʹ\D3û\F2disable
	input_unregister_device(ssl_priv->input);
	input_free_device(ssl_priv->input);
	kfree(ssl_priv);
	dev_set_drvdata(&client->dev, NULL);
	return 0;
}

static irqreturn_t ssd253x_ts_isr(int irq, void *dev_id)
{
	//int reg_val;
	struct ssl_ts_priv *ssl_priv = dev_id;
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_isr!                   |\n");
	DBG_FUNC("+-----------------------------------------+\n");

	if(imapx_pad_irq_pending(tsp_irq_index))
		imapx_pad_irq_clear(tsp_irq_index);
	else
		return IRQ_HANDLED;
	
	if( !work_pending(&ssl_priv->ssl_work)) 
	{
		disable_irq_nosync(ssl_priv->irq);
	
		queue_work(ssd253x_wq, &ssl_priv->ssl_work);
		enable_irq(ssl_priv->irq);
	}
	return IRQ_HANDLED;	
}

static enum hrtimer_restart ssd253x_ts_timer(struct hrtimer *timer)
{
	struct ssl_ts_priv *ssl_priv = container_of(timer, struct ssl_ts_priv, timer);
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_timer!                 |\n");
	DBG_FUNC("+-----------------------------------------+\n");

	queue_work(ssd253x_wq, &ssl_priv->ssl_work);
	if(ssl_priv->use_irq==0) hrtimer_start(&ssl_priv->timer, ktime_set(0, MicroTimeTInterupt), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static const struct i2c_device_id ssd253x_ts_id[] = {
	{ SSD253X_I2C_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, ssd253x_ts_id);

static struct i2c_driver ssd253x_ts_driver = {
	.driver = {
		.name = SSD253X_I2C_NAME,
	},
	.probe = ssd253x_ts_probe,
	.remove = ssd253x_ts_remove,
#ifndef	CONFIG_HAS_EARLYSUSPEND
	.suspend = ssd253x_ts_suspend,
	.resume = ssd253x_ts_resume,
#endif
	.id_table = ssd253x_ts_id,
};

//static char banner[] __initdata = KERN_INFO "SSL Touchscreen driver, (c) 2011 Solomon Systech Ltd.\n";
static int __init ssd253x_ts_init(void)
{
	int ret = -1;
	struct i2c_board_info info;                                                                               
	struct i2c_adapter *adapter;                                                                              
	struct i2c_client *client;

	if(item_exist("ts.model"))
	{
		if(item_equal("ts.model", "ssd2533", 0))
		{
			
			DBG_FUNC("+-----------------------------------------+\n");
			DBG_FUNC("|	SSL_ts_init!                      |\n");
			DBG_FUNC("+-----------------------------------------+\n");
			
			ssd253x_wq = create_singlethread_workqueue("ssd253x_wq");
			if (!ssd253x_wq)
			{
				DBG_FUNC("		ssd253x_ts_init: create_singlethread_workqueue Error!\n");
				return -ENOMEM;
			}
			else
			{
				DBG_FUNC("		ssd253x_ts_init: create_singlethread_workqueue OK!\n");
			}	
			
			memset(&info, 0, sizeof(struct i2c_board_info));                                                          
			info.addr = SSD2533_SLAVE_ADDR;                
			strlcpy(info.type, SSD253X_I2C_NAME, I2C_NAME_SIZE);
			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
		    if (!adapter) {                        
				DBG_FUNC(KERN_ERR"*******get_adapter error!\n");              
			}
			client = i2c_new_device(adapter, &info);
			if(!client) 
				DBG_FUNC(KERN_ERR"failed to create new i2c device ssd2533\n");
		    
		  	ret=i2c_add_driver(&ssd253x_ts_driver);
			
			if(ret) 
				DBG_FUNC("ssd253x_ts_init: i2c_add_driver Error! \n");
			else    
				DBG_FUNC("ssd253x_ts_init: i2c_add_driver OK! \n");

			return ret;
		}
		else
			printk("%s: touchscreen is not ssd2533\n", __func__);
	}
	else
		printk("%s: touchscreen is not exist\n", __func__);

	return -1;
}

static void __exit ssd253x_ts_exit(void)
{
	DBG_FUNC("+-----------------------------------------+\n");
	DBG_FUNC("|	ssd253x_ts_exit!                  |\n");
	DBG_FUNC("+-----------------------------------------+\n");
	i2c_del_driver(&ssd253x_ts_driver);
	if (ssd253x_wq) destroy_workqueue(ssd253x_wq);
}

module_init(ssd253x_ts_init);
module_exit(ssd253x_ts_exit);

MODULE_AUTHOR("Solomon Systech Ltd - Design Technology, Icarus Choi");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ssd253x Touchscreen Driver 1.3");
