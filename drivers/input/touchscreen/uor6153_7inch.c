#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/earlysuspend.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <mach/pad.h>
#include <asm/gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <mach/items.h>

//#define UOR6153_DBG_FUCTION 1
#ifdef 	UOR6153_DBG_FUCTION
#define DBG_FUNC(x...) printk(x)
#else
#define DBG_FUNC(x...)
#endif

#define UORxL_AddW 0x90          //UOR AddresS for Writing"1001 1010"
#define UORxL_AddR 0x91          //UOR Address for Reading"1001 1011"
#define InitX			0x00
#define InitY			0x10
#define MSRX_2T                0x40
#define MSRY_2T                0x50
#define MSRX_1T                0xC0
#define MSRY_1T                0xD0
#define MSRZ1_1T               0xE0
#define MSRZ2_1T               0xF0

unsigned int GPIOPortE_Pin3;
unsigned int uor6153_int; 
#define UOR6153_I2C_NAME   "uor6153_ts"
#define FALSE 0
#define TRUE 1

#define R_xplate	 1024
#define R_Threshold 	 10000
#define R_Threshold2     1500//500 //mmmmmm
#define XMax 3500
#define XMin	300
#define YMax 3000
#define YMin	300

#define ZERO_TOUCH	0	
#define ONE_TOUCH	1
#define TWO_TOUCH	2

#define DX_T			64
#define DY_T			58

#define NumberFilter		4
#define NumberDrop		2	//This value must not bigger than (NumberFilter-1)
#define WMT_FILTER_NUM     4
#define FIRSTTOUCHCOUNT		1//��������ĸ���
#define ONETOUCHCountAfter2	 50// ���������ĵ������
#define JITTER_THRESHOLD	1000//ÂoÂI: «e«ášâ­Ó³æÂI­Y¶W¹LŠ¹­È
#define FIRST_TWO_TOUCH_FILTER	2//��������ĸ���
#define JITTER_THRESHOLD_DXDY	48//ÂoÂI: «e«ášâ­Óšâ«ü­Y¶W¹LŠ¹­È
#define	DROP_POINT_DELAY_J	  10		
#define	READ_DATA_DELAY_J	  2    // 3
#define FILTER_FUNC
#define NFilt NumberFilter
#define NDrop NumberDrop

typedef signed char VINT8;
typedef unsigned char VUINT8;
typedef signed short VINT16;
typedef unsigned short VUINT16;
typedef unsigned long VUINT32;
typedef signed long VINT32;

struct uor_touch_screen_struct {
	struct i2c_client *client;
	struct input_dev *dev;
	long xp;
	long yp;
	long pX;
	long pY;
	long pDX;
	long pDY;
	int count;
	int shift;
	int irq_num;
	int index;
	unsigned char n_touch;
	wait_queue_head_t wq;
	spinlock_t lock;
	struct timer_list	ts_timer;
	unsigned char ges_status;
	unsigned char GesNo;
	struct delayed_work work;
	struct early_suspend early_suspend;
};

static struct uor_touch_screen_struct * ts;
static int uor_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id);
static int uor6153_init_irq(struct uor_touch_screen_struct * tsdata);

static int uor_detach_adapter(struct i2c_client *client);
//static int uor_i2c_detect(struct i2c_adapter *adapter, int address, int kind);

VINT8 Init_UOR_HW(void);

void SendGestureKey(VUINT8);
VUINT8 gesture_decision(VUINT8,VUINT16,VUINT16,VUINT16,VUINT16);

//volatile struct adc_point gADPoint;

static int UOR_IICRead(char command, char *rxdata, int length)
{
	int retry;

	struct i2c_msg msgs[] = {
		{
			.addr = ts->client->addr,
			.flags = 0,
			.len = 1,
			.buf = &command,
		},
		{
			.addr = ts->client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxdata,
		},
	};

	for (retry = 0; retry < 3; retry++) {
		if (i2c_transfer(ts->client->adapter, msgs, 2) > 0)
			break;
		else
			udelay(100);
	}
	if (retry >= 3) {
		printk(KERN_ERR"%s: retry over 10!\n", __func__);
		return -EIO;
	}
	return 0;
}

struct ST_UOR_BUF {
		unsigned short pXbuf[WMT_FILTER_NUM];
		unsigned short pYbuf[WMT_FILTER_NUM];
		unsigned short pDXbuf[WMT_FILTER_NUM];
		unsigned short pDYbuf[WMT_FILTER_NUM];
};

static unsigned short uor_avg_XY(unsigned short *pData )
{
	int i,j,k=1;
	unsigned int avg;
	
	for(i=WMT_FILTER_NUM-1; i>0;i--){
		for(j=i-1;j>=0;j--){
			if(pData[j] > pData[i]){
				k =pData[j];
				pData[j] = pData[i];
				pData[i] = k;
			}			
		}
	}
	i = WMT_FILTER_NUM/2;
	avg = pData[i]+pData[i-1];

        if(abs(pData[i]-pData[i-1])>350)
           avg=4095;
        else
           avg/=2;
	return avg;
}

static unsigned short uor_avg_DXY(unsigned short *pData )
{
	int i,j,k;//swap = 1;
	unsigned int avg;
	
	for(i=WMT_FILTER_NUM-1; i>0;i--){
		for(j=i-1;j>=0;j--){
			if(pData[j] > pData[i]){
				k =pData[j];
				pData[j] = pData[i];
				pData[i] = k;
			}			
		}
	}
	i = WMT_FILTER_NUM/2;
	avg =pData[i]+pData[i-1];
        if(abs(pData[i]-pData[i-1])>16)
           avg=256;
        else
           avg /=2;

	return avg;
}

static void uor_read_XY(unsigned short *X, unsigned short *Y)
{
	int i = 0;
	char  EpBuf[2];
	struct ST_UOR_BUF uor_point;
	
	for(i=0; i<WMT_FILTER_NUM; i++){
		memset(EpBuf, 0, sizeof(EpBuf));
		UOR_IICRead(MSRX_1T, EpBuf, 2);
		uor_point.pXbuf[i] = (EpBuf[0]<<4)|(EpBuf[1]>>4);
	
		UOR_IICRead(MSRY_1T,  EpBuf, 2);
		uor_point.pYbuf[i] = (EpBuf[0]<<4)|(EpBuf[1]>>4);
	}	

	*X = uor_avg_XY(uor_point.pXbuf);
	*Y = uor_avg_XY(uor_point.pYbuf);

	return;
}

static void uor_read_DXY(unsigned short *DX, unsigned short *DY)
{
	int i = 0;
	char EpBuf[4];
	struct ST_UOR_BUF uor_point;
	
	for(i=0; i<WMT_FILTER_NUM;i++){
		memset(EpBuf, 0, sizeof(EpBuf));
		UOR_IICRead(MSRX_2T,  (EpBuf), 3);
		uor_point.pDXbuf[i] = EpBuf[2];
	
		UOR_IICRead(MSRY_2T,  (EpBuf), 3);
		uor_point.pDYbuf[i] = EpBuf[2];
	}	

	*DX = uor_avg_DXY(uor_point.pDXbuf);// 2 µãÖ®ŒäX ÖáŸàÀë
	*DY = uor_avg_DXY(uor_point.pDYbuf);// 2 µãÖ®ŒäY ÖáŸàÀë

	return;
}

//--------------------------

int uor_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//disable your interrupt !!!

//	disable_irq(TOUCH_INT_IOPIN);

//	gpio_irq_disable(GPIOPortE_Pin3);
	DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
	disable_irq_nosync(ts->irq_num);
	return 0;
}

int uor_resume(struct i2c_client *client)
{
	//enable your interrupt !!!
	//enable_irq(TOUCH_INT_IOPIN);
//	gpio_irq_enable(GPIOPortE_Pin3);
	DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
	uor6153_init_irq(ts);
	enable_irq(ts->irq_num);
	return 0;
}


static const struct i2c_device_id uor6153_id[] = {
	{ UOR6153_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver uor_i2c_driver = {
	.probe = uor_i2c_probe,
	.remove = uor_detach_adapter,
	.id_table = uor6153_id,
	.driver = {
		.name = UOR6153_I2C_NAME,
	},
#ifndef CONFIG_HAS_EARLYSUSPEND	
	.suspend = uor_suspend,	
	.resume	= uor_resume,
#endif
};

//static unsigned short normal_i2c[]      = {UORxL_AddW>>1, I2C_CLIENT_END};
//static unsigned short probe[2]          = {I2C_CLIENT_END, I2C_CLIENT_END};
//static unsigned short ignore[2]         = {I2C_CLIENT_END, I2C_CLIENT_END};
/*
static struct i2c_client_address_data addr_data = {

        normal_i2c,

        probe,

        ignore,

};
*/

static int uor_detach_adapter(struct i2c_client *client){

//	printk("uor.c:uor_detach_adapter() \n");
	//i2c_detach_client(client);
	//kfree(client);
	return 0;
}



int xFilter[NFilt], yFilter[NFilt],DxFilter[NFilt], DyFilter[NFilt];

unsigned int XYIndex = 0;

int  XYFilter(int *xFilter, int *yFilter, int Num,int Drop)
{
	unsigned int i,SumTempX=0,SumTempY=0;
	int Dp,checkSmx,checkSmy,checkBgx,checkBgy;
	int SmX =0, SmY = 0;
	int LaX = 0, LaY = 0;
	int SmInX = 0, SmInY = 0;
	int LaInX = 0, LaInY =0;

// printk("in filter 0 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);

	if( (Num <=2) && (Drop > (Num-1)) )
		return FALSE; // not enough to sample

	for(i=0;i<Num;i++){
		SumTempX += xFilter[i];
		SumTempY += yFilter[i];
	}
	Dp = Drop;
	checkSmx = 0;
	checkSmy = 0;
	checkBgx = 0;
	checkBgy = 0;

	while(Dp>0){
		SmX = 0x0FFF;SmY = 0x0FFF;
		LaX = 0x0;LaY = 0x0;
		SmInX = 0;SmInY = 0;
		LaInX = 0;LaInY =0;

		for(i =  0; i < Num; i++){
			if(checkSmx&(1<<i)) {
			}else if(SmX > xFilter[i]){
				SmX = xFilter[i];
				SmInX= i;
			}

			if(checkSmy&(1<<i)) {
			}else if(SmY > yFilter[i]){
				SmY = yFilter[i];
				SmInY = i;
			}

			if(checkBgx&(1<<i)){

			}else if(LaX < xFilter[i]){
				LaX = xFilter[i];
				LaInX = i;
			}

			if(checkBgy&(1<<i)){

			}else if(LaY < yFilter[i]){
				LaY = yFilter[i];
				LaInY = i;
			}
		}

		if(Dp){
			SumTempX-= xFilter[SmInX];
			SumTempX-= xFilter[LaInX];
			SumTempY-= yFilter[SmInY];
			SumTempY-= yFilter[LaInY];
			Dp -= 2;
		//	printk("in filter 1 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);
		}

		checkSmx |= 1<<SmInX;
		checkSmy |= 1<<SmInY;
		checkBgx |= 1<<LaInX;
		checkBgy |= 1<<LaInY;
	}
	xFilter[0] = SumTempX/(Num-Drop);
	yFilter[0] = SumTempY/(Num-Drop);
//	printk("in filter 2 :SmInX %d,LaInX %d, SmInY %d , LaInY %d!!!\n", SmInX,LaInX, SmInY, LaInY);
	return TRUE;
}

VINT8 Init_UOR_HW(void){
	VUINT8   i,icdata[2];
	VUINT32   Dx_REF,Dy_REF,Dx_Check,Dy_Check;
	int		  TempDx[NumberFilter],TempDy[NumberFilter];

	for(i=0;i<NumberFilter;i++){
		UOR_IICRead(InitX,icdata,2);
		TempDx[i] = (icdata[0]<<4 | icdata[1]>>4);
        UOR_IICRead(InitY,icdata,2);
		TempDy[i] = (icdata[0]<<4 | icdata[1]>>4);
		//printk(KERN_ERR "filter test:#%d (x,y)=(%d,%d) !!!\n", i,TempDx[i], TempDy[i]);
	}

	XYFilter(TempDx,TempDy,NumberFilter,2);
    Dx_REF = TempDx[0];
    Dy_REF = TempDy[0];

	//printk(KERN_ERR "filter result:(x,y)=(%d,%d) !!!\n", Dx_REF, Dy_REF);
	i = 0;
	do{
		UOR_IICRead(InitX,icdata,2);
		Dx_Check = abs((icdata[0]<<4 | icdata[1]>>4) - Dx_REF);
		UOR_IICRead(InitY,icdata,2);
		Dy_Check = abs((icdata[0]<<4 | icdata[1]>>4) - Dy_REF);
		i++;
		if(i>NumberFilter)
			return -1;
//	printk("%s:%d,%d,%d\n",__FUNCTION__,__LINE__,Dx_Check,Dy_Check);
	}while(Dx_Check > 4 || Dy_Check > 4);
	return 0;
}

static struct workqueue_struct *queue = NULL;
//static struct work_struct work;

//static struct delayed_work work;
static int FirstTC = 0,OneTCountAfter2 = 0,TWOTouchFlag = 0;
static int two_touch_count = 0, pre_dx = 0, pre_dy = 0;

#if 0
static void uor_read_data(unsigned short *X, unsigned short *Y, 
	unsigned short *DX, unsigned short *DY)
{
	VUINT8 EpBuf[16];
	unsigned short	x, y;
	unsigned short	Dx, Dy;
	memset(EpBuf, 0, sizeof(EpBuf));

	UOR_IICRead(MSRX_1T, EpBuf, 2);
	
	x= EpBuf[0]; 
	x <<=4;
	x |= (EpBuf[1]>>4);

	UOR_IICRead(MSRY_1T,  EpBuf, 2);
	y = EpBuf[0]; 
	y <<=4;
	y |= (EpBuf[1]>>4);
	UOR_IICRead(MSRX_2T,  (EpBuf), 3);
	Dx = EpBuf[2];
	UOR_IICRead(MSRY_2T,  (EpBuf), 3);
	Dy = EpBuf[2];	
	*X = x;
	*Y = y;
	*DX = Dx;
	*DY = Dy;
}
#endif
static int uor_get_int_state(void)
{
    imapx_pad_set_mode(1,1,ts->index);//gpio
    imapx_pad_set_dir(1,1,ts->index);//input
    return imapx_pad_get_indat(ts->index);
}

static void uor_read_loop(struct work_struct *data)
{
	unsigned short  x, y;
//	unsigned short x1,x2,y1,y2,x3,y3,x4,y4;
	unsigned short  Dx, Dy, z1, z2;
//	unsigned short Dx1,Dx2,Dy1,Dy2;
	unsigned int	R_touch;
	unsigned int Rt;
	unsigned int nTouch = 0;	
    VUINT8 EpBuf[3];
	int	dx_coord = 0 ;
	int	dy_coord = 0 ;
	int x1,y1,x2,y2;
//	int xp,yp;

//	int mask;
	DBG_FUNC("uor_read_loop %s:%d\n",__FUNCTION__,__LINE__);	
	while(1){
		#ifdef FILTER_FUNC
			uor_read_XY(&x, &y);
			ts->xp = x;
			ts->yp = y;
		#else // no filter
		    memset(EpBuf, 0, sizeof(EpBuf));
		
		    UOR_IICRead(MSRX_1T, EpBuf, 2);
		    x1= EpBuf[0]; 
		    x1<<=4;
		    x1|= (EpBuf[1]>>4);
			
		    UOR_IICRead(MSRX_1T, EpBuf, 2);
		    x2= EpBuf[0]; 
		    x2<<=4;
		    x2|= (EpBuf[1]>>4);
		
		    UOR_IICRead(MSRY_1T,  EpBuf, 2);
		    y1= EpBuf[0]; 
		    y1<<=4;
		    y1 |= (EpBuf[1]>>4);
		
		    UOR_IICRead(MSRY_1T,  EpBuf, 2);
		    y2= EpBuf[0]; 
		    y2<<=4;
		    y2 |= (EpBuf[1]>>4);
		
			if ((x1-x2)>350 ||(x2-x1)>350)
				ts->xp = 4095;
			else
		    	ts->xp = (x1+x2)/2;
			if ((y1-y2)>350 ||(y2-y1)>350)
			    ts->yp = 4095;
			else
		        ts->yp = (y1+y2)/2;
		#endif
			memset(EpBuf, 0, sizeof(EpBuf));				
			//check nTouch again after AVG
		
		    UOR_IICRead(MSRZ1_1T, EpBuf, 2);
			z1 = EpBuf[0]; 
			z1 <<=4;
			z1 |= (EpBuf[1]>>4);
			UOR_IICRead(MSRZ2_1T, EpBuf, 2);
			z2 = EpBuf[0]; 
			z2 <<=4;
			z2 |= (EpBuf[1]>>4);
		
			if(z1 ==0) 
				z1 =1;//avoid divde by zero 
			if (ts->xp < 4095) {
				R_touch =(abs(((z2*ts->xp)/z1-ts->xp)))/4;
				Rt =R_touch;
			}
			else {
				Rt = R_Threshold2+1;
			}
			DBG_FUNC("Rt = %d\n", Rt);
			if(uor_get_int_state()){
				DBG_FUNC(KERN_ERR "GPIO zero\n");
				nTouch =  ZERO_TOUCH;
			}
			else if((Rt < R_Threshold2)){
				nTouch =  TWO_TOUCH;
				DBG_FUNC(KERN_ERR "GPIO two\n");
			}
			else {
				nTouch = ONE_TOUCH;
				DBG_FUNC(KERN_ERR "GPIO one\n");
			}
			ts->yp=4095-ts->yp;
	//		DBG_FUNC(KERN_ERR "%s:after Avg Filter (x,y)=(%d,%d)  n_touch %d, R_touch %d , !!!\n",__FUNCTION__, ts->xp, ts->yp,  nTouch, Rt);
		
				
			if(nTouch == TWO_TOUCH) {
		#ifdef FILTER_FUNC
				uor_read_DXY(&Dx, &Dy);
				Dx = Dx;
				Dy = Dy;
		#else
				memset(EpBuf, 0, sizeof(EpBuf));
				UOR_IICRead(MSRX_2T,  (EpBuf), 3);
		        Dx1 = EpBuf[2];
		       	UOR_IICRead(MSRX_2T,  (EpBuf), 3);
		        Dx2 = EpBuf[2];
		        UOR_IICRead(MSRY_2T,  (EpBuf), 3);
		        Dy1 = EpBuf[2];	
			    UOR_IICRead(MSRY_2T,  (EpBuf), 3);
		        Dy2 = EpBuf[2];	
		
				if ((Dx1-Dx2)>16 || (Dx2-Dx1)>16)
				   Dx = 256;
				else
			       Dx = (Dx1+Dx2)>>1;
				if ((Dy1-Dy2)>16 ||(Dy2-Dy1)>16)
				   Dy = 256;
				else
			        Dy = (Dy1+Dy2)>>1;
				#endif
		
				DBG_FUNC("%s:after Avg Filter (Dx,Dy)=(%d,%d) !!!\n",__FUNCTION__,Dx,  Dy);
				
				if((Dx < DX_T && Dy < DY_T) || (Dx>255 || Dy>255)){
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
				}
				else if(two_touch_count < FIRST_TWO_TOUCH_FILTER){
		
			        //printk(KERN_ERR "%s:(dx,dy)=(%d,%d),count = %d, FIRST_TWO_TOUCH_FILTER = %d  !!!\n",__FUNCTION__,Dx, Dy,two_touch_count, FIRST_TWO_TOUCH_FILTER);
					two_touch_count++;
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
		
				}
				else if(ts->xp>4000||ts->yp>4000){
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
				}
				else if( (pre_dx!=0) && (pre_dy!=0) && (Dx - pre_dx > JITTER_THRESHOLD_DXDY || pre_dx - Dx > JITTER_THRESHOLD_DXDY || pre_dy - Dy > JITTER_THRESHOLD_DXDY || Dy - pre_dy > JITTER_THRESHOLD_DXDY)){
						//printk(KERN_ERR "%s:filter for jitter(dual) --(pre_dx,pre_dy)=(%d,%d) ,(dx,dy)=(%d,%d) , JITTER_THRESHOLD_DXDY = %d !!!\n",__FUNCTION__, pre_dx, pre_dy , Dx, Dy, JITTER_THRESHOLD_DXDY);
						queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
						goto READ_LOOP_OUT;
		
				}
				else{
						//report x,y,pressure,dx,dy to Linux/Android
						//printk(KERN_ERR "%s:raw data for dual touch-- (x,y)=(%d,%d) (dx,dy)=(%d,%d)  !!!\n",__FUNCTION__, x, y, Dx, Dy);
					if ( (ts->pX!=0) && (ts->pY!=0) && ((ts->xp - ts->pX) <3000 && (ts->pX-ts->xp) <3000 && (ts->yp - ts->pY )<3000 && (ts->pY-ts->yp )<3000)){
					    ts->xp = ts->pX;
					    ts->yp = ts->pY;
					}
					if( (pre_dx!=0) && (pre_dy!=0) && ((Dx>120) || (Dy>120)) && ((Dx - pre_dx) <24 && (pre_dx-Dx) <24 && (Dy - pre_dy )<24 && (pre_dy-Dy )<24)){
						Dx=pre_dx;
						Dy=pre_dy;
					}														
					if ( (pre_dx!=0) && (pre_dy!=0) && ((Dx - pre_dx) <12 && (pre_dx-Dx) <12 && (Dy - pre_dy )<12 && (pre_dy-Dy )<12)){
						Dx = pre_dx;
						Dy = pre_dy;
					}

					if((Dx - DX_T) < 0)
						dx_coord = 0 ;
					else
						dx_coord = ((Dx - 40) & 0xff8) * 2+2;
					if((Dy - DY_T) < 0)
						dy_coord = 0 ;
					else
						dy_coord = ((Dy - 40) & 0xff8) * 2+2;
					//compute for two touch coordinate p1=(x1,y1) p2=(x2,y2)
					#if 1
					 x1 = ts->xp - dx_coord;
					 y1 = ts->yp - dy_coord;
					 x2 = ts->xp + dx_coord;
					 y2 = ts->yp + dy_coord;
					#else
					 x1 = 2048 - dx_coord;
					 y1 = 2048 - dy_coord;
					 x2 = 2048 + dx_coord;
					 y2 = 2048 + dy_coord;
					#endif
					//report dual touch p1 ,p2 to Linux/Android
					DBG_FUNC(KERN_ERR "%s:report dual touch for android (x1,y1)=(%d,%d) (x2,y2)=(%d,%d)  !!!\n",__FUNCTION__, x1, y1, x2, y2);
					//input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));
					//input_report_abs(ts->dev, ABS_MT_WIDTH_MAJOR, 500+press);
					input_report_key(ts->dev, BTN_TOUCH, 1);		
					input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 15);
					
					input_report_abs(ts->dev, ABS_MT_POSITION_X, x1 );
					input_report_abs(ts->dev, ABS_MT_POSITION_Y, y1 );
					input_mt_sync(ts->dev);
					//input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200));
					//input_report_abs(ts->dev, ABS_MT_WIDTH_MAJOR, 600+press);
					input_report_key(ts->dev, BTN_TOUCH, 1);		
					input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 15);
					input_report_abs(ts->dev, ABS_MT_POSITION_X, x2 );
					input_report_abs(ts->dev, ABS_MT_POSITION_Y, y2 );
					input_mt_sync(ts->dev);
					input_sync(ts->dev);
	
					TWOTouchFlag = 1;
					OneTCountAfter2 = 0;
					pre_dx = Dx;
					pre_dy = Dy;
					ts->pX = ts->xp;
					ts->pY = ts->yp;
					queue_delayed_work(queue, &ts->work, READ_DATA_DELAY_J);
					goto READ_LOOP_OUT;
				}
			}
			else if(nTouch == ONE_TOUCH) {
				if((TWOTouchFlag == 1) && (OneTCountAfter2 < ONETOUCHCountAfter2)){
					DBG_FUNC(KERN_ERR "%s:filter after two touch -- (x,y)=(%d,%d) ,OneTCountAfter2 = %d, ONETOUCHCountAfter2 = %d !!!\n",__FUNCTION__, x, y, OneTCountAfter2, ONETOUCHCountAfter2);
					OneTCountAfter2++;
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
				}
				else if((ts->xp>4000||ts->yp>4000||TWOTouchFlag == 0) && (FirstTC < FIRSTTOUCHCOUNT)){//¥á±Œ«eFIRSTTOUCHCOUNT(X)µ§³æÂI
					DBG_FUNC(KERN_ERR "%s:filter before single touch -- (x,y)=(%d,%d) ,FirstTC = %d, FIRSTTOUCHCOUNT = %d !!!\n",__FUNCTION__, x, y, FirstTC, FIRSTTOUCHCOUNT);
					FirstTC++;
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
				}
				else if( (ts->pX!=0) && (ts->pY!=0) && (ts->xp - ts->pX > JITTER_THRESHOLD || ts->pX - ts->xp > JITTER_THRESHOLD || ts->pY - ts->yp > JITTER_THRESHOLD || ts->yp - ts->pY > JITTER_THRESHOLD)){//single touch point «e«á®t¶ZJITTER_THRESHOLD «hÂoÂI 
					DBG_FUNC(KERN_ERR "%s:filter for jitter -- (px,py)=(%d,%d) ,(x,y)=(%d,%d) , JITTER_THRESHOLD = %d !!!\n",__FUNCTION__, ts->pX, ts->pY ,x, y, JITTER_THRESHOLD);
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
		
				}
				else{
					if ((ts->pX!=0) && (ts->pY!=0) 
					&& ((ts->xp-ts->pX) < 20 && (ts->pX-ts->xp) < 20 
					&& (ts->yp - ts->pY) < 20 && (ts->pY-ts->yp) < 20)) {
			 			ts->yp = ts->pY;
					}
					input_report_key(ts->dev, BTN_TOUCH, 1);		
					input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 15);
					//input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 800 + (Rt%200) );
					input_report_abs(ts->dev, ABS_MT_POSITION_X, ts->xp);
					input_report_abs(ts->dev, ABS_MT_POSITION_Y, ts->yp);
					input_mt_sync(ts->dev);
					input_sync(ts->dev);
					DBG_FUNC("x=%d, y=%d\n", ts->xp, ts->yp);
					//save previous single touch point
					ts->pX = ts->xp; 
					ts->pY = ts->yp;
					queue_delayed_work(queue, &ts->work, READ_DATA_DELAY_J);														
					goto READ_LOOP_OUT;
				}
			}
			else if(nTouch == ZERO_TOUCH){
//		ZERO_TOUCH_PROCESS:
				//DBG_FUNC(KERN_ERR "%s:zero touch (x,y)=(%d,%d) (dx,dy)=(%d,%d) n_touch %d, R_touch %d, (z1,z2)=(%d,%d) !!!\n",__FUNCTION__, x, y, Dx, Dy, nTouch, R_touch, z1, z2);
				DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
				mdelay(2);
			    if(!(uor_get_int_state())) {
					DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
					queue_delayed_work(queue, &ts->work, DROP_POINT_DELAY_J);
					goto READ_LOOP_OUT;
			    }
				input_report_key(ts->dev, BTN_TOUCH, 0);
				input_report_abs(ts->dev, ABS_MT_TOUCH_MAJOR, 0 );
				input_mt_sync(ts->dev);
				input_sync(ts->dev);

				//reset filter parameters
				FirstTC = 0;
				OneTCountAfter2 = 0;
				TWOTouchFlag = 0;
				two_touch_count = 0;
				ts->xp= 0;
				ts->yp = 0;
				ts->pX = 0;
				ts->pY = 0;
				pre_dx = 0;
				pre_dy = 0;
			    Init_UOR_HW();
				imapx_pad_set_mode(0, 1, ts->index);/* func mode */
				imapx_pad_irq_config(ts->index, INTTYPE_FALLING, FILTER_MAX);/* set trigger mode and filter */
				enable_irq(ts->irq_num);
				break;
			}
			else{
				printk(KERN_ERR "uor_read_loop(): n_touch state error !!!\n");
			}
	}	
READ_LOOP_OUT:
	DBG_FUNC("%s: exit ts while loop !!!\n",__FUNCTION__ );		
	return;
}

static irqreturn_t uor_isr(int irq,void *dev_id)
{
	struct uor_touch_screen_struct *tsdata = dev_id;
	
	if(imapx_pad_irq_pending(tsdata->index))
		imapx_pad_irq_clear(tsdata->index);
	else
		return IRQ_HANDLED;
	DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
	if (!work_pending(&tsdata->work.work)) {
		disable_irq_nosync(tsdata->irq_num);
		queue_work(queue, &tsdata->work.work);
	}
	return IRQ_HANDLED;
}

static int uor6153_init_irq(struct uor_touch_screen_struct * tsdata)
{
	int index, irq_num;

	index = tsdata->index = item_integer("ts.int", 1);
	DBG_FUNC("uor6153_init_irq INT pad index %d\n", index);
	irq_num = imapx_pad_irq_number(index);/* get irq */
	if(!irq_num)
		return -EINVAL;
	
	DBG_FUNC("uor6153_init_irq irq %d\n", irq_num);
	imapx_pad_set_mode(0, 1, index);/* func mode */
	imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);/* set trigger mode and filter */
	return irq_num;
}

static int uor_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int	ret;
	struct uor_touch_screen_struct * tsdata;
	struct input_dev *	input_device;

	ts = tsdata = kzalloc(sizeof(struct uor_touch_screen_struct), GFP_KERNEL);
	uor6153_int = tsdata->irq_num = uor6153_init_irq(tsdata);
	
	i2c_set_clientdata(client, tsdata);
	tsdata->client = client;
	input_device = input_allocate_device();
	if (!input_device) {
			printk(KERN_ERR "Unable to allocate the input device !!\n");
			return -ENOMEM;
	}
	input_device->name = "UOR-touch";
	tsdata->dev = input_device;
	
	__set_bit(EV_KEY, input_device->evbit);
	__set_bit(EV_ABS, input_device->evbit);
	__set_bit(EV_SYN, input_device->evbit);
	__set_bit(BTN_TOUCH, input_device->keybit);
	__set_bit(ABS_MT_TOUCH_MAJOR, input_device->absbit);
	__set_bit(ABS_MT_TRACKING_ID, input_device->absbit);
	__set_bit(ABS_MT_POSITION_X, input_device->absbit);
	__set_bit(ABS_MT_POSITION_Y, input_device->absbit);

	input_set_abs_params(input_device, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_device, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
	input_set_abs_params(tsdata->dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);
	input_set_abs_params(tsdata->dev, ABS_MT_POSITION_Y, 128, 3950, 0, 0);
	ret = input_register_device(tsdata->dev);
	if (ret) {
		printk(KERN_ERR "%s: unabled to register input device, ret = %d\n",
		__FUNCTION__, ret);
		return ret;
	}
	DBG_FUNC(KERN_ERR "uor.c: before UOR init !\n");
	queue = create_singlethread_workqueue("uor-touch-screen-read-loop");
	//queue = create_rt_workqueue("uor-touch-screen-read-loop");
	INIT_DELAYED_WORK(&tsdata->work, uor_read_loop);
	//printk(KERN_ERR "uor.c: before UOR workqueue !\n");
	if(request_irq(uor6153_int, uor_isr, IRQF_DISABLED, "uor6153-ts", tsdata)){
		printk(KERN_ERR "uor.c: Could not allocate GPIO intrrupt for touch screen !!!\n");
		free_irq(uor6153_int,NULL);
		return -EIO;	
	}
#ifdef CONFIG_HAS_EARLYSUSPEND	
	tsdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;	
	tsdata->early_suspend.suspend = uor_suspend;	
	tsdata->early_suspend.resume = uor_resume;	
	register_early_suspend(&tsdata->early_suspend);
#endif
	DBG_FUNC("%s:==================== %d ================\n", __func__, __LINE__);
	return 0;
}

static int __init uor_init(void)
{
	int	ret;
//	struct input_dev *	input_device;
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	if(item_exist("ts.model"))	{		
		if(item_equal("ts.model", "uor6153", 0)) {
			DBG_FUNC(KERN_INFO"%s: UOR6153 TouchScreen driver: init\n", __func__);
			memset(&info, 0, sizeof(struct i2c_board_info));
			info.addr = 0x48;  
			strlcpy(info.type, UOR6153_I2C_NAME, I2C_NAME_SIZE);
			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
			if (!adapter) {
				printk(KERN_ERR "%s: get_adapter error!\n", __func__);
			}
			client = i2c_new_device(adapter, &info);
			ret = i2c_add_driver(&uor_i2c_driver);
			if(ret < 0)
				printk(KERN_ERR "uor.c: i2c_add_driver() fail in uor_init()!\n");	
			return ret;
		}
		else
			return -1;
	}
	else
		return -1;
}

static void __exit uor_exit(void)
{
	free_irq(GPIOPortE_Pin3,NULL);
	i2c_del_driver(&uor_i2c_driver);  
}

module_init(uor_init);
module_exit(uor_exit);
MODULE_DESCRIPTION("UOR Touchscreen driver");
MODULE_AUTHOR("Ming-Wei Chang <mingwei@uutek.com.tw>");
MODULE_LICENSE("GPL");