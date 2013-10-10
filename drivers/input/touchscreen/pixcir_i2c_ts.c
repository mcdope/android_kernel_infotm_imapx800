/*
 * Driver for Pixcir I2C touchscreen controllers.
 *
 * Copyright (C) 2010-2012 Pixcir, Inc.
 *
 * pixcir_i2c_ts.c V3.0	from v3.0 support TangoC solution and remove the previous soltutions
 *
 * pixcir_i2c_ts.c V3.1	Add bootloader function	7
 *			Add RESET_TP		9
 * 			Add ENABLE_IRQ		10
 *			Add DISABLE_IRQ		11
 * 			Add BOOTLOADER_STU	16
 *			Add ATTB_VALUE		17
 *			Add Write/Read Interface for APP software
 *
 * pixcir_i2c_ts.c V3.2.09	for INT_MODE 0x09
 *				change to workqueue for ISR
 *				adaptable report rate self
 *
 * pixcir_i2c_ts.c V3.3.09	Add Android early power management
 *				Add irq_flag for pixcir debug tool
 *				Add CRC attb check when bootloader
 *
 * This code is proprietary and is subject to license terms.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/utsname.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/gpio.h>
#include <mach/pad.h>
#include <mach/power-gate.h>
#include <mach/imap-iomap.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <mach/items.h>
#include "pixcir_i2c_ts.h"

//#define PIXCIR_DBG_FUCTION
#ifdef 	PIXCIR_DBG_FUCTION
#define DBG_FUNC(x...) printk(x)
#else
#define DBG_FUNC(x...)
#endif


/*********************************Bee-0928-TOP****************************************/
static int PIXCIR_DEBUG = 0;
static int tsp_irq_index;
static unsigned int _sui_irq_num;
static int _si_touch_num = 0;

extern int get_lcd_width(void);
extern int get_lcd_height(void);

static int X_MAX=1024;
static int Y_MAX=600;


//#define	PIXCIR_STATIC
//---------------------------------
#define TOUCHSCREEN_X_MAX 1024
#define TOUCHSCREEN_Y_MAX 600

//---------------------------------

#define SLAVE_ADDR		0x5c
#define	BOOTLOADER_ADDR		0x5d

#ifndef I2C_MAJOR
#define I2C_MAJOR 		976
#endif

#define I2C_MINORS 		256

#define	CALIBRATION_FLAG	1
#define	BOOTLOADER		7
#define RESET_TP		9

#define	ENABLE_IRQ		10
#define	DISABLE_IRQ		11
#define	BOOTLOADER_STU		16
#define ATTB_VALUE		17

#define	MAX_FINGER_NUM		5
static char cali_flag = -1;
static char *pix_driver_version="V-Tangoc-3.4-bate";
static unsigned char status_reg = 0;
static struct i2c_client *pixcir_i2c_client;

volatile int global_irq, irq_flag;


struct i2c_dev {
	struct list_head list;
	struct i2c_adapter *adap;
	struct device *dev;
};

static struct i2c_driver pixcir_i2c_ts_driver;
static struct class *i2c_dev_class;
static LIST_HEAD( i2c_dev_list);
static DEFINE_SPINLOCK( i2c_dev_list_lock);

static int pixcir_init(void)
{
	int index;

	index = item_integer("ts.int", 1);
	//index = imapx_pad_index("GPIO6");
	DBG_FUNC("CTP INT pad index %d\n", index);
	tsp_irq_index = index;
	_sui_irq_num = imapx_pad_irq_number(index);/* get irq */
	if(!_sui_irq_num)
		return -EINVAL;
	
	DBG_FUNC("CTP irq %d\n", _sui_irq_num);

	imapx_pad_set_mode(0, 1, index);/* func mode */
	imapx_pad_irq_config(index, INTTYPE_BOTH/* INTTYPE_FALLING*/, FILTER_MAX);/* set trigger mode and filter */
	return _sui_irq_num;
}

static void pixcir_reset(void)
{
	int index;
	index = item_integer("ts.reset", 1);

	printk("----jason--- ts.reset=%d\n",index);
	//index = imapx_pad_index("UART1_RTS");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0, 1, index);/*output*/
	imapx_pad_set_outdat(0, 1, index);/*output 0*/
	msleep(10);
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	msleep(10);
	imapx_pad_set_outdat(0, 1, index);/*output 0*/
	msleep(100);
}

static void return_i2c_dev(struct i2c_dev *i2c_dev)
{
	spin_lock(&i2c_dev_list_lock);
	list_del(&i2c_dev->list);
	spin_unlock(&i2c_dev_list_lock);
	kfree(i2c_dev);
}

static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{
	struct i2c_dev *i2c_dev;
	i2c_dev = NULL;

	spin_lock(&i2c_dev_list_lock);
	list_for_each_entry(i2c_dev, &i2c_dev_list, list)
	{
		if (i2c_dev->adap->nr == index)
			goto found;
	}
	i2c_dev = NULL;
	found: spin_unlock(&i2c_dev_list_lock);
	return i2c_dev;
}

/*********************************Bee-0928-bottom**************************************/

static struct workqueue_struct *pixcir_wq;

struct pixcir_i2c_ts_data {
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
	struct early_suspend early_suspend;
	int irq;
};

struct point_node_t {
	unsigned char 	active ;
	unsigned char	finger_id;
	unsigned int	posx;
	unsigned int	posy;
};

static struct point_node_t point_slot[MAX_FINGER_NUM*2];

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pixcir_i2c_ts_early_suspend(struct early_suspend *h);
static void pixcir_i2c_ts_late_resume(struct early_suspend *h);
#endif

#define PIX_DRV_ATTR

#ifdef PIX_DRV_ATTR
/****************************************************************************************/
/*-----------------------------------add the sysfs attr during the driver *drv---------*/
static ssize_t pix_store_debug_drv(struct device_driver *drv,
							const char *buf,size_t count)
{
	unsigned char cali_buf[2]={0x3a,0x03};
	char val;
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(pixcir_i2c_client);
	printk("enter %s:buf = %s\n",__func__,(char)(*buf));
	if(sscanf(buf,"%s",&val)<1)
		return -EINVAL;
	//printk("val = %s \n",val);
	switch(val){
	case 'c':
	case 'C':
		if(i2c_master_send(pixcir_i2c_client,cali_buf,sizeof(cali_buf))==sizeof(cali_buf)){
			printk("%s : calibration successful from attr!\n",__func__);
			cali_flag = 1;
		}else {
			printk("%s : calibration fail from attr!\n",__func__);
			cali_flag = 0;
			}
	break;
	case 'p':
	case 'P':
		 PIXCIR_DEBUG = 1;
	break;
	case 's':
	case 'S':
		 if(irq_flag == 1) {
			irq_flag = 0;
			disable_irq_nosync(global_irq);
		}
		//hrtimer_cancel(&tsdata->timer);		
	break;
	case 'n':
	case 'N':
	 PIXCIR_DEBUG =0;
		 enable_irq(tsdata->irq);
		 if(irq_flag == 0) {
			irq_flag = 1;
			enable_irq(global_irq);
		}
		//hrtimer_start(&tsdata->timer, ktime_set(0,10000000), HRTIMER_MODE_REL);
	break;
	default:
		printk("this conmand is unknow!!\n");
	break;		
	}
	return count;
}
static ssize_t pix_show_debug_drv(struct device_driver *drv,
	                                        const char *buf )
{

	return sprintf(buf, "%d\n", cali_flag);

}
static ssize_t pix_show_version_drv(struct device_driver *drv,
						  const char *buf)
{	
        //ssize_t len;
       return snprintf(buf,PAGE_SIZE,"  %s   ::  %s  ::  %s  ::  %s  ::  %s\n"
			"PIXCIR-DRIVER-VERSION:::%s\n",
			utsname()->sysname,
			utsname()->nodename,
			utsname()->release,
			utsname()->version,
                	utsname()->machine,
                          pix_driver_version);
	//return len;
}
static DRIVER_ATTR(pix_debug, S_IRUGO|S_IWUGO,pix_show_debug_drv,pix_store_debug_drv);
static DRIVER_ATTR(pix_version, S_IRUGO,pix_show_version_drv,NULL);
static struct attribute *pixcir_drv_attr[] = {
	//&driver_attr_pix_version.attr,
	&driver_attr_pix_debug.attr,
	&driver_attr_pix_version.attr,
	NULL
};
static struct attribute_group pixcir_drv_attr_grp = {
	.attrs =pixcir_drv_attr,
};
static const struct attribute_group *pixcir_drv_grp[] = {
	&pixcir_drv_attr_grp,
	NULL
};

#endif



enum eORIT
{
eORIT_xy='x'*256+'y',
eORIT_Xy='X'*256+'y',	
eORIT_xY='x'*256+'Y',
eORIT_XY='X'*256+'Y',

eORIT_yx='y'*256+'x',
eORIT_Yx='Y'*256+'x',
eORIT_yX='y'*256+'X',
eORIT_YX='Y'*256+'X',
};


#define ITEM_CFG_ORI_SIZE  ITEM_MAX_LEN //sync to item
typedef struct _ItemCfgOri_T
{
	int orient; 
	int debug; //for debug enable flags.
	char str[ITEM_CFG_ORI_SIZE];
	int ScrX;//for screen X size
	int ScrY;// for screen Y size
}ItemCfgOri_T;
	
static ItemCfgOri_T pixcItemCfgOri;	

/*
this func only use to get cfg
*/
void DoItemCfgOri(ItemCfgOri_T *p)
{
	if(NULL==p)	 
	{
		printk("%s, input NULL pointer!!!\n",__FUNCTION__);
		return;
	}
	
    if(item_exist("ts.orientation"))
    {
        item_string(p->str, "ts.orientation", 0);
		
		if(p->debug) 
		{
			printk("str:%s\n",p->str);
			printk("A:%x,B:%x\n",p->str[0],p->str[1]);
		}
		
		p->orient=((int)(p->str[0]))*256+ p->str[1];//get the value
		if('D'==(p->str[2]))
			p->debug=1; //open the debug 
		else 
			p->debug=0;

		if(p->debug) 
		{
			printk("str:%s\n",p->str);
			printk("orient:%x\n",p->orient);
			printk("ScrX:%d\n",p->ScrX);
			printk("ScrY:%d\n",p->ScrY);			
			printk("--%x,%x,%x,%x\n",eORIT_xy,eORIT_Xy,eORIT_xY,eORIT_XY);
		}				
	
    }
    else
	{
        p->orient = eORIT_xy;
    }

}


/*
this func only use cfg to exchange XY value.
*/
void DoItemHALOriexchange(ItemCfgOri_T *p ,struct point_node_t *pnode )
{
int orgx;
int orgy;
int ScreenMaxX,ScreenMaxY;
	if(NULL==p)	 
	{
		printk("%s, input NULL pointer!!!\n",__FUNCTION__);
		return;
	}

	orgx=pnode->posx;
	orgy=pnode->posy;
	ScreenMaxX=p->ScrX;
	ScreenMaxY=p->ScrY;

	if(p->debug) 
	{
		printk("iX:%d\n",pnode->posx);
		printk("iY:%d\n",pnode->posy);
	}

	
	switch(p->orient)
	{
		case eORIT_xy:
			break;//do nothing 
		case eORIT_xY:
			pnode->posy=ScreenMaxY-orgy;			
			break;
		case eORIT_Xy:
			pnode->posx=ScreenMaxX-orgx;
			break;
		case eORIT_XY:
			pnode->posx=ScreenMaxX-orgx;
			pnode->posy=ScreenMaxY-orgy;			
			break;
		case eORIT_yx:
			pnode->posx=orgy;
			pnode->posy=orgx;				
			break;
		case eORIT_yX:
			pnode->posx=orgy;
			pnode->posy=ScreenMaxX-orgx;
			break;
		case eORIT_Yx:
			pnode->posx=ScreenMaxY-orgy;
			pnode->posy=orgx;
			break;
		case eORIT_YX:
			pnode->posx=ScreenMaxY-orgy;
			pnode->posy=ScreenMaxX-orgx;
			break;
		default:
			break;
			
	}
	if(p->debug) 
	{
		printk("oX:%d\n",pnode->posx);
		printk("oY:%d\n",pnode->posy);
		
	}	


}

static void pixcir_ts_poscheck(struct work_struct *work)
{
	struct pixcir_i2c_ts_data *tsdata = container_of(work,
			struct pixcir_i2c_ts_data,
			work.work);
	
	unsigned char *p;
	unsigned char touch, button, pix_id,slot_id;
	unsigned char rdbuf[27], wrbuf[1] = { 0 };
	int ret, i;

	ret = i2c_master_send(tsdata->client, wrbuf, sizeof(wrbuf));
	if (ret != sizeof(wrbuf)) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}

	ret = i2c_master_recv(tsdata->client, rdbuf, sizeof(rdbuf));
	if (ret != sizeof(rdbuf)) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}

	touch = rdbuf[0]&0x07;
	button = rdbuf[1];
	DBG_FUNC("touch=%d,button=%d\n",touch,button);
#ifdef BUTTON
	if(button) {
		switch(button) {
			case 1:
				input_report_key(tsdata->input, KEY_HOME, 1);
			case 2:
				//add other key down report
			case 4:
			case 8:
			//add case for other more key 
			default:
				break;
		}
	} else {
		input_report_key(tsdata->input, KEY_HOME, 0);
		//add other key up report
	}
#endif

	p=&rdbuf[2];
	for (i=0; i<touch; i++)	{
		pix_id = (*(p+4));
		slot_id = ((pix_id & 7)<<1) | ((pix_id & 8)>>3);
		point_slot[slot_id].active = 1;
		point_slot[slot_id].finger_id = pix_id;	
		point_slot[slot_id].posx = (*(p+1)<<8)+(*(p));
		point_slot[slot_id].posy = (*(p+3)<<8)+(*(p+2));
		p+=5;
	}

	if(touch) {
		input_report_key(tsdata->input, BTN_TOUCH, 1);
		input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 15);
		for (i=0; i<MAX_FINGER_NUM*2; i++) {
			if (point_slot[i].active == 1) {
				point_slot[i].active = 0;
				input_report_key(tsdata->input, ABS_MT_TRACKING_ID, i);
				DoItemHALOriexchange(&pixcItemCfgOri,&(point_slot[i]));
				input_report_abs(tsdata->input, ABS_MT_POSITION_X,  (point_slot[i].posx * X_MAX) / TOUCHSCREEN_X_MAX);
				input_report_abs(tsdata->input, ABS_MT_POSITION_Y,  (point_slot[i].posy * Y_MAX) / TOUCHSCREEN_Y_MAX);
				input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 15);
				input_mt_sync(tsdata->input);
				DBG_FUNC("slot=%d,x%d=%d,y%d=%d  ",i, i/2,point_slot[i].posx, i/2, point_slot[i].posy);
				
			}
		}	
		DBG_FUNC("\n");
	} else {
		input_report_key(tsdata->input, BTN_TOUCH, 0);
		input_report_abs(tsdata->input, ABS_MT_TOUCH_MAJOR, 0);
	}
	input_sync(tsdata->input);
//	msleep(5);

}

static irqreturn_t pixcir_ts_isr(int irq, void *dev_id)
{
	struct pixcir_i2c_ts_data *tsdata = dev_id;
	//printk("--pixcir_ts_isr--\n");
	if(imapx_pad_irq_pending(tsp_irq_index)){
		imapx_pad_irq_clear(tsp_irq_index);
		if (imapx_pad_get_indat(tsp_irq_index) == 0)
		{
			/* IRQ is triggered by FALLING code here */
			//struct mg_data *mg = _mg;
			//schedule_work(&mg->work);
			if (!work_pending(&tsdata->work.work))
					queue_work(pixcir_wq, &tsdata->work.work);			
		//	printk("TS1_INT_GPIO falling\n");
		}else{
		//	printk("TS1_INT_GPIO raising\n");
		}
	}			
	else
		return IRQ_HANDLED;

	return IRQ_HANDLED;
}

static struct i2c_dev *get_free_i2c_dev(struct i2c_adapter *adap)
{	
	struct i2c_dev *i2c_dev;	
	if (adap->nr >= I2C_MINORS) 
	{		
		printk(KERN_ERR "i2c-dev: Out of device minors (%d)\n",				adap->nr);
		return ERR_PTR(-ENODEV);	
	}	
	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);	
	if (!i2c_dev)		
		return ERR_PTR(-ENOMEM);	
	i2c_dev->adap = adap;	
	spin_lock(&i2c_dev_list_lock);	
	list_add_tail(&i2c_dev->list, &i2c_dev_list);	
	spin_unlock(&i2c_dev_list_lock);	
	return i2c_dev;
}


static int __devinit pixcir_i2c_ts_probe(struct i2c_client *client,
					 const struct i2c_device_id *id)
{
	//const struct pixcir_ts_platform_data *pdata = client->dev.platform_data;
	struct i2c_adapter *adapter;
	struct i2c_board_info info;
	struct pixcir_i2c_ts_data *tsdata;
	struct input_dev *input;
	struct device *dev;
	struct i2c_dev *i2c_dev;
	int i, error, irq_num;

	DBG_FUNC("At %s %d now----------------------\n\n\n", __FUNCTION__, __LINE__);

	X_MAX = get_lcd_width();			
	Y_MAX = get_lcd_height();
	pixcItemCfgOri.ScrX=X_MAX;
	pixcItemCfgOri.ScrY=Y_MAX;
	DoItemCfgOri(&pixcItemCfgOri);// cfg orientation from item
	//initlize the interrupt pin
	global_irq = client->irq = pixcir_init(); 
//	pixcir_reset();
	
	for(i=0; i<MAX_FINGER_NUM*2; i++) 
	{
		point_slot[i].active = 0;
	}

	tsdata = kzalloc(sizeof(*tsdata), GFP_KERNEL);
	input = input_allocate_device();
	if (!tsdata || !input) 
	{
		dev_err(&client->dev, "Failed to allocate driver data!\n");
		error = -ENOMEM;
		goto err_free_mem;
	}

	pixcir_i2c_client = tsdata->client = client;
	tsdata->input = input;
	global_irq = tsdata->irq = client->irq;

	INIT_WORK(&tsdata->work.work, pixcir_ts_poscheck);

	input->name = client->name;
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;

	__set_bit(EV_KEY, input->evbit);
	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_SYN, input->evbit);
	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(ABS_MT_TOUCH_MAJOR, input->absbit);
	__set_bit(ABS_MT_TRACKING_ID, input->absbit);
	__set_bit(ABS_MT_POSITION_X, input->absbit);
	__set_bit(ABS_MT_POSITION_Y, input->absbit);

	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, X_MAX, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, Y_MAX, 0, 0);

	input_set_drvdata(input, tsdata);

	DBG_FUNC("At %s %d now----------------------\n\n\n", __FUNCTION__, __LINE__);
	
	
	
	
	

	error = request_irq(client->irq, pixcir_ts_isr,	
			    IRQF_SHARED,//IRQF_TRIGGER_FALLING,
			    //0,
			    client->name, tsdata);
	if (error) {
		dev_err(&client->dev, "Unable to request touchscreen IRQ.\n");
		goto err_free_mem;
	}
	disable_irq_nosync(client->irq);

	error = input_register_device(input);
	if (error)
		goto err_free_irq;

	i2c_set_clientdata(client, tsdata);
	//device_init_wakeup(&client->dev, 1);
	/*********************************Bee-0928-TOP****************************************/
	i2c_dev = get_free_i2c_dev(client->adapter);
	if (IS_ERR(i2c_dev)) {
		error = PTR_ERR(i2c_dev);
		return error;
	}

	dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,
			client->adapter->nr), NULL, "pixcir_i2c_ts%d", 0);
	if (IS_ERR(dev)) {
		error = PTR_ERR(dev);
		return error;
	}
	

	
	
	/*********************************Bee-0928-BOTTOM****************************************/
#ifdef CONFIG_HAS_EARLYSUSPEND
	tsdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	tsdata->early_suspend.suspend = pixcir_i2c_ts_early_suspend;
	tsdata->early_suspend.resume = pixcir_i2c_ts_late_resume;
	register_early_suspend(&tsdata->early_suspend);
#endif

	dev_err(&tsdata->client->dev, "insmod successfully!\n");
	
	irq_flag = 1;
	enable_irq(client->irq);
	DBG_FUNC("At %s %d now----------------------\n\n\n", __FUNCTION__, __LINE__);
	return 0;

err_free_irq:
	free_irq(client->irq, tsdata);
err_free_mem:
	input_free_device(input);
	kfree(tsdata);
	return error;
}

static int __devexit pixcir_i2c_ts_remove(struct i2c_client *client)
{
	int error;
	struct i2c_dev *i2c_dev;
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&tsdata->early_suspend);
#endif

	device_init_wakeup(&client->dev, 0);

	mb();
	free_irq(client->irq, tsdata);
#if 1
	/*********************************Bee-0928-TOP****************************************/
	i2c_dev = get_free_i2c_dev(client->adapter);
	if (IS_ERR(i2c_dev)) {
		error = PTR_ERR(i2c_dev);
		return error;
	}

	return_i2c_dev(i2c_dev);
	device_destroy(i2c_dev_class, MKDEV(I2C_MAJOR, client->adapter->nr));
	/*********************************Bee-0928-BOTTOM****************************************/
#endif
	input_unregister_device(tsdata->input);
	kfree(tsdata);

	return 0;
}

/*************************************Bee-0928****************************************/
/*                        	     pixcir_open                                     */
/*************************************Bee-0928****************************************/
static int pixcir_open(struct inode *inode, struct file *file)
{
	int subminor;
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	struct i2c_dev *i2c_dev;
	int ret = 0;
if(PIXCIR_DEBUG)
	printk("enter pixcir_open function\n");
	subminor = iminor(inode);

	//lock_kernel();
	i2c_dev = i2c_dev_get_by_minor(subminor);
	if (!i2c_dev) {
		printk("error i2c_dev\n");
		return -ENODEV;
	}

	adapter = i2c_get_adapter(i2c_dev->adap->nr);
	if (!adapter) {
		return -ENODEV;
	}
	
	client = kzalloc(sizeof(*client), GFP_KERNEL);

	if (!client) {
		i2c_put_adapter(adapter);
		ret = -ENOMEM;
	}

	snprintf(client->name, I2C_NAME_SIZE, "pixcir_i2c_ts%d", adapter->nr);
	client->driver = &pixcir_i2c_ts_driver;
	client->adapter = adapter;
	
	file->private_data = client;

	return 0;
}

/*************************************Bee-0928****************************************/
/*                        	     pixcir_ioctl                                    */
/*************************************Bee-0928****************************************/
static long pixcir_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client *) file->private_data;
if(PIXCIR_DEBUG)
	printk("pixcir_ioctl(),cmd = %d, arg = %ld\n", cmd, arg);
	switch (cmd)
	{
	case CALIBRATION_FLAG:	//CALIBRATION_FLAG = 1
		client->addr = SLAVE_ADDR;
		status_reg = CALIBRATION_FLAG;
		break;

	case BOOTLOADER:	//BOOTLOADER = 7
		client->addr = BOOTLOADER_ADDR;
		status_reg = BOOTLOADER;

		pixcir_reset();
		mdelay(5);
		break;

	case RESET_TP:		//RESET_TP = 9
		pixcir_reset();
		break;
		
	case ENABLE_IRQ:	//ENABLE_IRQ = 10
		status_reg = 0;
		if(irq_flag == 0) {
			irq_flag = 1;
			enable_irq(global_irq);
		}
		break;
		
	case DISABLE_IRQ:	//DISABLE_IRQ = 11
		if(irq_flag == 1) {
			irq_flag = 0;
			disable_irq_nosync(global_irq);
		}
		break;

	case BOOTLOADER_STU:	//BOOTLOADER_STU = 16
		client->addr = BOOTLOADER_ADDR;
		status_reg = BOOTLOADER_STU;

		pixcir_reset();
		mdelay(5);
		break;

	case ATTB_VALUE:	//ATTB_VALUE = 17
		client->addr = SLAVE_ADDR;
		status_reg = ATTB_VALUE;
		break;

	default:
		client->addr = SLAVE_ADDR;
		status_reg = 0;
		break;
	}
	return 0;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_read                                      */
/***********************************Bee-0928****************************************/
static ssize_t pixcir_read (struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	struct i2c_client *client = (struct i2c_client *)file->private_data;
	unsigned char *tmp, bootloader_stu[4], attb_value[1];
	int ret = 0;

	switch(status_reg)
	{
	case BOOTLOADER_STU:
		i2c_master_recv(client, bootloader_stu, sizeof(bootloader_stu));
		if (ret!=sizeof(bootloader_stu)) {
			dev_err(&client->dev,
				"%s: BOOTLOADER_STU: i2c_master_recv() failed, ret=%d\n",
				__func__, ret);
			return -EFAULT;
		}

		if (copy_to_user(buf, bootloader_stu, sizeof(bootloader_stu))) {
			dev_err(&client->dev,
				"%s: BOOTLOADER_STU: copy_to_user() failed.\n",	__func__);
			return -EFAULT;
		} else {
			ret = 4;
		}
		break;

	case ATTB_VALUE:
		attb_value[0] = attb_read_val();
		if(copy_to_user(buf, attb_value, sizeof(attb_value))) {
			dev_err(&client->dev,
				"%s: ATTB_VALUE: copy_to_user() failed.\n", __func__);
			return -EFAULT;
		} else {
			ret = 1;
		}
		break;

	default:
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp==NULL)
			return -ENOMEM;

		ret = i2c_master_recv(client, tmp, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: default: i2c_master_recv() failed, ret=%d\n",
				__func__, ret);
			return -EFAULT;
		}

		if(copy_to_user(buf, tmp, count)) {
			dev_err(&client->dev,
				"%s: default: copy_to_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}
		kfree(tmp);
		break;
	}
	return ret;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_write                                     */
/***********************************Bee-0928****************************************/
static ssize_t pixcir_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct i2c_client *client;
	unsigned char *tmp, bootload_data[143];
	int ret = 0, time_out = 0;

	client = file->private_data;

		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;

		if (copy_from_user(tmp, buf, count)) { 	
			dev_err(&client->dev,
				"%s: CALIBRATION_FLAG: copy_from_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}

		printk("----jason--- tmp=%s\n",tmp);
		if(strncmp(tmp,"reset",5)==0){
			pixcir_reset();
		}
		kfree(tmp);
		

	switch(status_reg)
	{
	case CALIBRATION_FLAG:	//CALIBRATION_FLAG=1
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;

		if (copy_from_user(tmp, buf, count)) { 	
			dev_err(&client->dev,
				"%s: CALIBRATION_FLAG: copy_from_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}

		ret = i2c_master_send(client, tmp, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: CALIBRATION: i2c_master_send() failed, ret=%d\n",
				__func__, ret);
			kfree(tmp);
			return -EFAULT;
		}

		kfree(tmp);
		break;

	case BOOTLOADER:
		memset(bootload_data, 0, sizeof(bootload_data));

		if (copy_from_user(bootload_data, buf, count)) {
			dev_err(&client->dev,
				"%s: BOOTLOADER: copy_from_user() failed.\n", __func__);
			return -EFAULT;
		}
		time_out = 0;
		while (attb_read_val()) {
			if(time_out > 100)			
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}
		ret = i2c_master_send(client, bootload_data, count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: BOOTLOADER: i2c_master_send() failed, ret = %d\n",
				__func__, ret);
			return -EFAULT;
		}
		time_out = 0;
		while (!attb_read_val()) {
			if(time_out > 100)			
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}

		time_out = 0;
		while(attb_read_val()) {
			if(time_out > 100)
				break;
			else {
				time_out++;
				mdelay(1);
			}
		}

		break;
	default:
		tmp = kmalloc(count, GFP_KERNEL);
		if (tmp == NULL)
			return -ENOMEM;

		if (copy_from_user(tmp, buf, count)) { 	
			dev_err(&client->dev,
				"%s: default: copy_from_user() failed.\n", __func__);
			kfree(tmp);
			return -EFAULT;
		}
		
		ret = i2c_master_send(client,tmp,count);
		if (ret != count) {
			dev_err(&client->dev,
				"%s: default: i2c_master_send() failed, ret=%d\n",
				__func__, ret);
			kfree(tmp);
			return -EFAULT;
		}
		kfree(tmp);
		break;
	}
	return ret;
}

/***********************************Bee-0928****************************************/
/*                        	  pixcir_release                                   */
/***********************************Bee-0928****************************************/
static int pixcir_release(struct inode *inode, struct file *file)
{
	struct i2c_client *client = file->private_data;
if(PIXCIR_DEBUG)
	printk("enter pixcir_release funtion\n");
	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;

	return 0;
}

/*********************************Bee-0928-TOP****************************************/
static const struct file_operations pixcir_i2c_ts_fops =
{	.owner		= THIS_MODULE,
	.open		= pixcir_open,
	.unlocked_ioctl = pixcir_ioctl,
	.read		= pixcir_read,
	.write		= pixcir_write,
	.release	= pixcir_release,
};
/*********************************Bee-0928-BOTTOM****************************************/

static int pixcir_i2c_ts_suspend(struct i2c_client *client)
{
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);
	unsigned char wrbuf[2] = { 0 };
	int ret;
	printk("------------------pixcir TP early suspend--------------------\n");
	wrbuf[0] = 0x33;
	wrbuf[1] = 0x03;	//enter into Sleep mode;
	/**************************************************************
	wrbuf[1]:	0x00: Active mode

			0x01: Sleep mode
			0xA4: Sleep mode automatically switch
			0x03: Freeze mode
	More details see application note 710 power manangement section
	****************************************************************/

	/*
	ret = i2c_master_send(tsdata->client, wrbuf, 2);
	if (ret != 2) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}
	*/

	//if (device_may_wakeup(&tsdata->client->dev))
	//	enable_irq_wake(tsdata->irq);
//	disable_irq(global_irq);      /**add by lou***/
	return 0;
}

static int pixcir_i2c_ts_resume(struct i2c_client *client)
{
	struct pixcir_i2c_ts_data *tsdata = i2c_get_clientdata(client);
	printk("-----------------pixcir TP late resume---------------------\n");
///if suspend enter into freeze mode please reset TP
	disable_irq(global_irq);      /**add by lou to edit  tp and earphone  conflict***/
#if 1
	pixcir_init();
	pixcir_reset();
#else
	unsigned char wrbuf[2] = { 0 };
	int ret;

	wrbuf[0] = 0x33;
	wrbuf[1] = 0;
	ret = i2c_master_send(tsdata->client, wrbuf, 2);
	if(ret != 2) {
		dev_err(&tsdata->client->dev,
			"%s: i2c_master_send failed(), ret=%d\n",
			__func__, ret);
	}
#endif
	//if (device_may_wakeup(&tsdata->client->dev))
	//	disable_irq_wake(tsdata->irq);
	enable_irq(global_irq);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void pixcir_i2c_ts_early_suspend(struct early_suspend *h)
{
	struct pixcir_i2c_ts_data *tsdata;
	tsdata = container_of(h, struct pixcir_i2c_ts_data, early_suspend);
	pixcir_i2c_ts_suspend(tsdata->client);
}

static void pixcir_i2c_ts_late_resume(struct early_suspend *h)
{
	struct pixcir_i2c_ts_data *tsdata;
	tsdata = container_of(h, struct pixcir_i2c_ts_data, early_suspend);
	pixcir_i2c_ts_resume(tsdata->client);
}
#endif

static const struct i2c_device_id pixcir_i2c_ts_id[] = {
	{ "pixcir_ts", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pixcir_i2c_ts_id);

static struct i2c_driver pixcir_i2c_ts_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		//.name	= "pixcir_i2c_ts_v3.3.09",
		.name	= "pixcir_ts",
		#ifdef PIX_DRV_ATTR
		.groups = pixcir_drv_grp,
		#endif
	},
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= pixcir_i2c_ts_suspend,
	.resume		= pixcir_i2c_ts_resume,
#endif
	.probe		= pixcir_i2c_ts_probe,
	.remove		= __devexit_p(pixcir_i2c_ts_remove),
	.id_table	= pixcir_i2c_ts_id,
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
__u16 i2cAddr=SLAVE_ADDR;//slave addr
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
			pixcir_reset();
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

static int __init pixcir_i2c_ts_init(void)
{
	int ret;
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	if(item_exist("ts.model"))
	{
		if(item_equal("ts.model", "pixcir", 0) || DoI2cScanDetectDev() )
		{
			
			DBG_FUNC("At %s %d now----------------------\n\n\n", __FUNCTION__, __LINE__);
			pixcir_wq = create_singlethread_workqueue("pixcir_wq");
			if (!pixcir_wq)
				return -ENOMEM;
			/*********************************Bee-0928-TOP****************************************/
			ret = register_chrdev(I2C_MAJOR, "pixcir_i2c_ts", &pixcir_i2c_ts_fops);
			if (ret) {
				printk(KERN_ERR "%s:register chrdev failed\n", __FILE__);
				return ret;
			}

			//i2c_dev_class = class_create(THIS_MODULE, "pixcir_i2c_dev");
			i2c_dev_class = class_create(THIS_MODULE, "pixcir_i2c_ts");
			if (IS_ERR(i2c_dev_class)) {
				ret = PTR_ERR(i2c_dev_class);
				class_destroy(i2c_dev_class);
			}
			/********************************Bee-0928-BOTTOM******************************************/
			DBG_FUNC("At %s %d now----------------------\n\n\n", __FUNCTION__, __LINE__);

			memset(&info, 0, sizeof(struct i2c_board_info));
			info.addr = SLAVE_ADDR;
			strlcpy(info.type, "pixcir_ts", I2C_NAME_SIZE);

			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
			//adapter = i2c_get_adapter(2);
			if (!adapter) {
				printk("****** get_adapter error! ******\n");
				return -1;
			}
			//------------------------- snake add -------------------
			pixcir_reset();
			//------------------------------------------------------
			client = i2c_new_device(adapter, &info);
			return i2c_add_driver(&pixcir_i2c_ts_driver);	
		}
		else
			printk("%s: touchscreen is not pixcir\n", __func__);
	}
	else
		printk("%s: touchscreen is not exist\n", __func__);

	return -1;
}
late_initcall(pixcir_i2c_ts_init);

static void __exit pixcir_i2c_ts_exit(void)
{
	i2c_del_driver(&pixcir_i2c_ts_driver);
	/********************************Bee-0928-TOP******************************************/
	class_destroy(i2c_dev_class);
	unregister_chrdev(I2C_MAJOR,"pixcir_i2c_ts");
	/********************************Bee-0928-BOTTOM******************************************/
	if(pixcir_wq)
		destroy_workqueue(pixcir_wq);
}
module_exit(pixcir_i2c_ts_exit);

MODULE_AUTHOR("Jianchun Bian <jcbian@pixcir.com.cn>");
MODULE_DESCRIPTION("Pixcir I2C Touchscreen Driver");
MODULE_LICENSE("GPL");
