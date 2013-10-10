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

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <mach/pad.h>
#include <mach/items.h>
#define PAD_SYSM_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000))
static int _si_touch_num = 0;
static unsigned int _sui_irq_num;
 static int tsp_irq_index;

//#define FT5X0X_DEBUG
#ifdef FT5X0X_DEBUG
#define DBG(fmt, args...)	printk("*** " fmt, ## args)
#else
#define DBG(fmt, args...)	do{}while(0)
#endif

#define EV_MENU					KEY_F1

#define FT5X0X_SPEED 100000
#define MAX_POINT  5

#if defined 	(CONFIG_IG_LCDRES_800x600)
#define SCREEN_MAX_X 800
#define SCREEN_MAX_Y 600
#elif defined 	(CONFIG_IG_LCDRES_800x480)
#define SCREEN_MAX_X 800
#define SCREEN_MAX_Y 480
#endif
#define SCREEN_MAX_X 800
#define SCREEN_MAX_Y 480

#define PRESS_MAX 200
#define MULTI_TOUCH 1
static const int button[4] = {KEY_MENU,KEY_BACK,KEY_HOME,KEY_MENU};

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend ft5x0x_early_suspend;
#endif


static unsigned char fu_sir_flag=1;

//#define TOUCHKEY_ON_SCREEN
extern struct completion Menu_Button;

static int  ft5x0x_probe(struct i2c_client *client, const struct i2c_device_id *id);
static void ft5x0x_tpscan_timer(unsigned long data);

struct ts_event {
	u16	x1;
	u16	y1;
	u16	x2;
	u16	y2;
	u16	x3;
	u16	y3;
	u16	x4;
	u16	y4;
	u16	x5;
	u16	y5;
	u16	pressure;
	u16	w;
    u8  touch_point;
};

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x39

typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;


static E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(struct i2c_client *client,FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth);
static int ft5x0x_write_reg(struct i2c_client *client,int addr,int value);
static void delay_qt_ms(unsigned long  w_ms);

static void delay_qt_ms(unsigned long  w_ms)
{
    unsigned long i;
    unsigned long j;

    for (i = 0; i < w_ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            udelay(1);
        }
    }
}

struct ft5x0x_data
{
	struct i2c_client *client;
	struct input_dev	*input_dev;
	spinlock_t 	lock;
	int			irq;
	int		reset_gpio;
	int		touch_en_gpio;
	int		last_points;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
	struct timer_list timer;
	u8  key_status; 
	u8  old_status;

};

struct i2c_client *g_client;

//int ft5x0x_rx_data(struct i2c_client *client, char *rxData, int dataLength)
int ft5x0x_rx_data(struct i2c_client *client, char *dataBuffer, int dataLength, char bufferIndex)
{
	int ret = 0;
	//char reg = rxData[0];
	//ret = i2c_master_reg8_recv(client, reg, rxData, length, FT5X0X_SPEED);
	//unsigned char bufferIndex = dataBuffer[0];
	struct i2c_msg msgs[2] = { 
		{
		       	.addr	= client->addr,
		       	.flags	= 0,
		       	//.flags	= I2C_M_NOSTART,
			.len	= 1,
		       	.buf	= &bufferIndex
		},
		{ 
			.addr	= client->addr,
		       	.flags	= I2C_M_RD,
		       	.len	= dataLength,
		       	.buf	= dataBuffer
	       	}
       	};

	memset(dataBuffer, 0xFF, dataLength);
	ret = i2c_transfer(client->adapter, msgs, 2);

	return (ret > 0)? 0 : ret;	//if read sucess,ret=2
}

//static int ft5x0x_tx_data(struct i2c_client *client, char *txData, int length)
static int ft5x0x_tx_data(struct i2c_client *client, char *dataBuffer, int dataLength)
{
	int ret = 0;
	char bufferIndex = dataBuffer[0];
//	char reg = txData[0];
//	ret = i2c_master_reg8_send(client, reg, &txData[1], length-1, FT5X0X_SPEED);
	unsigned char buffer4Write[256];
	struct i2c_msg msgs[1] = { { 
		.addr = client->addr, 
		.flags = 0, 
		.len = dataLength + 1, 
		.buf = buffer4Write 
		} };

	buffer4Write[0] = bufferIndex;
	memcpy(&(buffer4Write[1]), dataBuffer+1, dataLength);	//
	ret = i2c_transfer(client->adapter, msgs, 1);

	return (ret > 0)? 0 : ret;
}

char ft5x0x_read_reg(struct i2c_client *client, int addr)
{
	char tmp;
	int ret = 0;

	//printk("----%s----\n",__func__);
	tmp = addr;
	ret = ft5x0x_rx_data(client, &tmp, 1, addr);
	if (ret < 0) {
		return ret;
	}
	return tmp;
}

static int ft5x0x_write_reg(struct i2c_client *client,int addr,int value)
{
	char buffer[3];
	int ret = 0;
	//printk("----%s----\n",__func__);

	buffer[0] = addr;
	buffer[1] = value;
	//ret = ft5x0x_tx_data(client, &buffer[0], 2);
	ret = ft5x0x_tx_data(client, &buffer[0], 1);
	return ret;
}

static void ft5x0x_power_en(struct ft5x0x_data *ft5x0x, int on)
{
#if defined (GPIO_TOUCH_EN)
	if (on) {
		gpio_direction_output(ft5x0x->touch_en_gpio, 1);
		gpio_set_value(ft5x0x->touch_en_gpio, 1);
		mdelay(10);
	} else {
		gpio_direction_output(ft5x0x->touch_en_gpio, 0);
		gpio_set_value(ft5x0x->touch_en_gpio, 0);
		mdelay(10);
	}
#endif
}

static void ft5x0x_chip_reset(struct ft5x0x_data *ft5x0x)
{
    gpio_direction_output(ft5x0x->reset_gpio, 0);
    gpio_set_value(ft5x0x->reset_gpio, 1);
	mdelay(10);
    gpio_set_value(ft5x0x->reset_gpio, 0);
	mdelay(10);
    gpio_set_value(ft5x0x->reset_gpio, 1);
}

FTS_DWRD i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    ret=i2c_master_send(g_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[FTS]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

FTS_DWRD i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
    ret=i2c_master_recv(g_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[FTS]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

FTS_DWRD ft_cmd_write(unsigned char btcmd, unsigned char btPara1, unsigned char btPara2,
		unsigned char btPara3, int num)
{
    unsigned char write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

FTS_DWRD byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

FTS_DWRD byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}

static int ft5x0x_chip_init(struct i2c_client * client)
{
	int ret = 0;
	int w_value;
	char r_value;
	int reg;
	int i = 0, flag = 1;
	struct ft5x0x_data *ft5x0x = i2c_get_clientdata(client);

	//ft5x0x_chip_reset(ft5x0x);
	//ft5x0x_power_en(ft5x0x, 0);
   	//gpio_direction_output(ft5x0x->reset_gpio, 1);
    	//gpio_set_value(ft5x0x->reset_gpio, 1);
 	//-----------guang
    	uint32_t reset_pin;
#if 0
   	reset_pin = __imapx_name_to_gpio("GPC6");
    	imapx_gpio_setcfg(reset_pin, IG_OUTPUT, IG_NORMAL);
    	imapx_gpio_setpin(reset_pin, 0 , IG_NORMAL);
	mdelay(100);
#endif
	int tmp;
#if 0
	tmp = readl(rGPCPUD);
	tmp |= (0x01<<7);
	writel(tmp , rGPCPUD);

    	imapx_gpio_setpin(reset_pin, 1 , IG_NORMAL);
#endif
	//printk("reset_pin value= %x\n",imapx_gpio_getpin(reset_pin,IG_NORMAL));
    	//-----------guang
	//mdelay(10);
	//ft5x0x_power_en(ft5x0x, 1);
    	//ft_cmd_write(0x07,0x00,0x00,0x00,1);
	//mdelay(100);

#if 0
	reg = 0x80;
	w_value = 15; 
	ret = ft5x0x_write_reg(client, reg, w_value);    /* adjust sensitivity */
	if (ret < 0) {
		printk(KERN_ERR "ft5x0x i2c txdata failed\n");
	}
#endif
#if 0
	while (1) {
		reg = 0x88;

		//w_value = 5; 
		w_value = 6; 

		ret = ft5x0x_write_reg(client, reg, w_value);    /* adjust frequency 60Hz */
		if (ret < 0) {
			printk(KERN_ERR "ft5x0x i2c txdata failed\n");
			//goto out;
		}

		r_value = ft5x0x_read_reg(client, reg);
		if (ret < 0) {
			printk(KERN_ERR "ft5x0x i2c rxdata failed\n");
			//goto out;
		}
		printk("ft5x0x: r_value = %d, i = %d, flag = %d", r_value, i, flag);
		i++;

		if (w_value != r_value) {
			ret = -1;
			flag = 0;
			if (i > 5) { /* test 3 count */
				break;
			}
		} else {
			ret = 0;
			break;
		}
	}
#endif

	return ret;
}


static int g_screen_key=0;
#ifdef TOUCHKEY_ON_SCREEN

static unsigned char initkey_code[] =
{
    KEY_BACK, KEY_MENU, KEY_HOME, KEY_SEARCH
};

static int get_screen_key(int x, int y)
{
	typedef struct
	{
		int x;
		int y;
		int keycode;
	}rect;
		const int span = 10;
		int idx;
		const rect rt[]=
#if defined (TOUCH_KEY_MAP)
		TOUCH_KEY_MAP;
#else
		{
				{855,	130, 	KEY_SEARCH},      /* search */
				{855,	90,	KEY_HOME},        /* home */
				{855,	55,	KEY_MENU},      /* menu */
				{855,	0,	KEY_BACK},    /* back */

				{0,0,0}
		};
#endif
		DBG("***x=%d, y=%d\n", x, y);
		for(idx=0; rt[idx].keycode; idx++)
		{
			if(x >= rt[idx].x-span && x<= rt[idx].x+span)
				if(y >= rt[idx].y-span && y<= rt[idx].y+span)
					return rt[idx].keycode;
		}
		return 0;
}

static int key_led_ctrl(int on)
{
	#ifdef TOUCH_KEY_LED
		gpio_set_value(TOUCH_KEY_LED, on);
	#endif
}

struct input_dev *tp_key_input;

static int report_screen_key(int down_up)
{
#if 1  //by peter
	struct input_dev * keydev=(struct input_dev *)tp_key_input;

	input_report_key(keydev, g_screen_key, down_up);
#if 0
#ifdef CONFIG_IG_DEVICE_HDMI
	if(g_screen_key == KEY_MENU)
		complete(&Menu_Button);
#endif
#endif
	input_sync(keydev);
	key_led_ctrl(down_up);
	if(!down_up) {
		g_screen_key=0;
	}
#endif
	return 0;
}
#endif

static void ft5x_ts_release(void)
{                       
        struct ft5x0x_data *data = i2c_get_clientdata(g_client);                       
#ifdef CONFIG_FT5X0X_MULTITOUCH         
        input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);                          
#else                                   
        input_report_abs(data->input_dev, ABS_PRESSURE, 0);                                
        input_report_key(data->input_dev, BTN_TOUCH, 0);                                   
#endif                                  
                                                

	data->key_status = 0;   
	if(data->key_status != data->old_status)                                   
	{                       
		input_report_key(data->input_dev, button[data->old_status - 1], 0);
		data->old_status = 0;                                              
	}                       


	input_sync(data->input_dev);            
                                        
}                                       


static int ft5x0x_process_points(struct ft5x0x_data *data)
{
	struct ts_event *event = &data->event;
	struct i2c_client *client = data->client;
	u8 start_reg=0x0;
	u8 buf[32] = {0};
	int ret = -1;
	int status = 0;
	int last_points;
	int id;
	int back_press = 0, search_press=0, menu_press=0, home_press=0;
	

	start_reg = 0;
	buf[0] = start_reg;

#if MULTI_TOUCH
	if (MAX_POINT == 5) {
#if 1
		ret = ft5x0x_rx_data(client, &buf[0],31, start_reg);
	//	ret = ft5x0x_rx_data(client, &buf[13],10,0x13);

	//	ret = ft5x0x_rx_data(client, &buf[23],8, 0x23);
#endif
#if 0
		{
			int i = 0;
			for(i; i < 31; i++)
			{
				ret = ft5x0x_rx_data(client, &buf[i],1,i);
			}
		}
#endif
	} else {
		ret = ft5x0x_rx_data(client, buf, 13, start_reg);
	}
#else
	ret = ft5x0x_rx_data(client, buf, 7);
#endif
#if 0
	{
		int i = 0;
		while(i < 31){
			printk(KERN_INFO "buf[%d]-%x  ", i,buf[i]);
			i++;
		}

			printk(KERN_INFO "\n");
	}
#endif
 
    if (ret < 0) {
		printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}

	memset(event, 0, sizeof(struct ts_event));

#if MULTI_TOUCH
	if (MAX_POINT == 5) {
		event->touch_point = buf[2] & 0x07;
	} else {
		event->touch_point = buf[2] & 0x03;
	}

	last_points = event->touch_point;
	if (data->last_points > event->touch_point) {
		event->touch_point = data->last_points;
	}
	data->last_points = last_points;

#else
	event->touch_point = buf[2] & 0x01;
#endif
//add by peter
	if (event->touch_point == 0) {
		if(buf[1]==0)                                    
		{       //ÊÇ0x00£¬ÉÏ±¨¶ÔÓ¦´¥Ãþ°´¼üÌ§Æð    
			ft5x_ts_release();                
			return 1;                         
		}                                         
		else                                      
		{                                         
			//·Ç0x00,ÉÏ±¨¶ÔÓ¦´¥Ãþ°´¼ü°´ÏÂ     
			int key_id=0;                     
			switch(buf[1])                    
			{                                 
				case 0x01:                
					{                 
						key_id=1; 
					}                 
					break;            
				case 0x02:                
					{                 
						key_id=2; 
					}                 
					break;            
				case 0x04:                
					{                 
						key_id=3; 
					}                 
					break;            
				case 0x08:                
					{                 
						key_id=4; 
					}                 
					break;            
				default:                  
					{                 
						key_id=0; 
					}                 
					break;            
			}
			data->key_status=key_id;                                                            
			if((data->key_status != data->old_status) && data->key_status)                      
			{                                                                           
				input_report_key(data->input_dev, button[data->key_status - 1], 1); 
				data->old_status = data->key_status;                                
			}                                                                           





                      return 1;                                                                           
              }                                                                                           
  }                                                                                                       



#if 0
#if MULTI_TOUCH
	if (event->touch_point == 0) {
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 0);
		input_mt_sync(data->input_dev);
		input_sync(data->input_dev);

		DBG("release all points!!!!!!\n");
		return 0;
	}
#else
	if (event->touch_point == 0) {
		event->pressure = 0;
		event->w = 0;
	} else {
		event->pressure = 200;
		event->w = 50;
	}
#endif
#endif

#if MULTI_TOUCH
    switch (event->touch_point) {
		if (MAX_POINT == 5){
			case 5:
				event->x5 = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
				event->y5 = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
				status = (s16)((buf[0x1b] & 0xc0) >> 6);
				id = (buf[0x1d] & 0xf0) >> 4;
				if (status == 1) {
					event->pressure = 0;
					event->w = 0;
				} else {
					event->pressure = 200;
					event->w = 50;
				}
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);			
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, event->w);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x5);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y5);
				input_mt_sync(data->input_dev);
				//printk("TOUCH_NO=%d: id=%d,(%d,%d), pressure=%d, w=%d\n",5,id,event->x5, event->y5, event->pressure, event->w);
		case 4:
				event->x4 = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
				event->y4 = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
				status = (s16)((buf[0x15] & 0xc0) >> 6);
				id = (buf[0x17] & 0xf0) >> 4;
				if (status == 1) {
					event->pressure = 0;
					event->w = 0;
				} else {
					event->pressure = 200;
					event->w = 50;
				}
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);			
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, event->w);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x4);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y4);
				input_mt_sync(data->input_dev);
				//printk("TOUCH_NO=%d: id=%d,(%d,%d), pressure=%d, w=%d\n",4,id,event->x4, event->y4, event->pressure, event->w);
			case 3:
				event->x3 = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
				event->y3 = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
				status = (s16)((buf[0x0f] & 0xc0) >> 6);
				id = (buf[0x11] & 0xf0) >> 4;
				if (status == 1) {
					event->pressure = 0;
					event->w = 0;
				} else {
					event->pressure = 200;
					event->w = 50;
				}
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);			
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, event->w);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x3);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y3);
				input_mt_sync(data->input_dev);
				//printk("TOUCH_NO=%d: id=%d,(%d,%d), pressure=%d, w=%d\n",3,id,event->x3, event->y3, event->pressure, event->w);
		}
		case 2:
			event->x2 = (s16)(buf[9] & 0x0F)<<8 | (s16)buf[10];
			event->y2 = (s16)(buf[11] & 0x0F)<<8 | (s16)buf[12];
			status = (s16)((buf[0x9] & 0xc0) >> 6);
			id = (buf[11] & 0xf0) >> 4;
			if (status == 1) {
				event->pressure = 0;
				event->w = 0;
			} else {
				event->pressure = 200;
				event->w = 50;
			}
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id); 
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, event->w);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x2);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y2);
			input_mt_sync(data->input_dev);
			//printk("TOUCH_NO=%d;id=%d,(%d,%d), pressure=%d, w=%d\n",2,id,event->x2, event->y2, event->pressure, event->w);
		case 1:
			event->x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
			event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
			status = (s16)((buf[0x3] & 0xc0) >> 6);
			id = (buf[5] & 0xf0) >> 4;
			if (status == 1) {
				event->pressure = 0;
				event->w = 0;
			} else {
				event->pressure = 200;
				event->w = 50;
			}
			
			if (event->x1 < 810) {
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id); 
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, event->w);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->x1);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->y1);
				input_mt_sync(data->input_dev);			
				//printk("TOUCH_NO=%d: id=%d,(%d,%d), pressure=%d, w=%d\n",1,id,event->x1, event->y1, event->pressure, event->w);
			}
#ifdef TOUCHKEY_ON_SCREEN
			else {
				if (event->w) {
					if (g_screen_key = get_screen_key(event->x1, event->y1)) {
						DBG("touch key = %d down\n", g_screen_key);
						report_screen_key(1);
					}
				} else {
					DBG("touch key up!\n");
					report_screen_key(0);
				}
			}
#endif
		default:
			break;
	}
#else
    if (event->touch_point == 1) {
    	event->x1 = (s16)(buf[3] & 0x0F)<<8 | (s16)buf[4];
		event->y1 = (s16)(buf[5] & 0x0F)<<8 | (s16)buf[6];
    }
	input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);
	input_report_key(data->input_dev, BTN_TOUCH, (event->pressure!=0?1:0));
	if (event->touch_point == 1) {
		input_report_abs(data->input_dev, ABS_X, event->x1);
		input_report_abs(data->input_dev, ABS_Y, event->y1);
		//printk("report x = %d, y=%d, pressure = %d\n", event->x1, event->y1, event->pressure);
	} else {
		DBG("report pressure = %d\n",  event->pressure);
	}

#endif
	input_sync(data->input_dev);
	///mod_timer(&data->timer,jiffies + msecs_to_jiffies(35) );	//delete by guang
	return 0;
}

static void  ft5x0x_delaywork_func(struct work_struct *work)
{
	struct ft5x0x_data *ft5x0x = container_of(work, struct ft5x0x_data, pen_event_work);
	struct i2c_client *client = ft5x0x->client;
	ft5x0x_process_points(ft5x0x);
	///__raw_writel((1 << client->irq), rSRCPND);	//clear interrupt src
	///enable_irq(client->irq);		
}

static irqreturn_t ft5x0x_interrupt(int irq, void *handle)
{
	struct ft5x0x_data *ft5x0x_ts = handle;

//	printk("---irq=%d, Enter:%s %d\n",irq,__FUNCTION__,__LINE__);
	if(imapx_pad_irq_pending(tsp_irq_index)) 
		imapx_pad_irq_clear(tsp_irq_index);
	else
		return IRQ_HANDLED;

	///disable_irq_nosync(irq);
#if 0
	int tmp;
	tmp = readl(rINTPND);	//clear interrupt
	tmp &= ~(0x1 << 5);
	writel(tmp, rINTPND);

	tmp = readl(rINTPND);	//clear interrupt
	printk("--------tmp=%x\n",tmp);
#endif


	if (!work_pending(&ft5x0x_ts->pen_event_work)) {
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}
	//mod_timer(&ft5x0x_ts->timer,jiffies + msecs_to_jiffies(35) );
	return IRQ_HANDLED;
}

static void ft5x0x_tpscan_timer(unsigned long data)
{
	struct ft5x0x_data *ft5x0x_ts=data; 
	if( !work_pending(&ft5x0x_ts->pen_event_work) ) 
	{
	  	DBG("ft5x0x_tpscan_timer read data\n"); 
		disable_irq_nosync(ft5x0x_ts->client->irq);
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}		
}


static int ft5x0x_remove(struct i2c_client *client)
{
	struct ft5x0x_data *ft5x0x = i2c_get_clientdata(client);
	
    input_unregister_device(ft5x0x->input_dev);
    input_free_device(ft5x0x->input_dev);
    free_irq(client->irq, ft5x0x);
    kfree(ft5x0x); 
#ifdef CONFIG_HAS_EARLYSUSPEND
    unregister_early_suspend(&ft5x0x_early_suspend);
#endif      
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x0x_suspend(struct early_suspend *h)
{
	int err;
	int w_value;
	int reg;

	//printk("==ft5x0x_ts_suspend=\n");
#if defined(TP_USE_WAKEUP_PIN)
	w_value = 3;
	reg = 0xa5;
	err = ft5x0x_write_reg(g_client, reg, w_value);   /* enter sleep mode */
	if (err < 0) {
		printk("ft5x0x enter sleep mode failed\n");
	}
#endif
	disable_irq(g_client->irq);		
}

static void ft5x0x_resume(struct early_suspend *h)
{
	struct ft5x0x_data *ft5x0x = i2c_get_clientdata(g_client);

#if defined(TP_USE_WAKEUP_PIN)
	ft5x0x_chip_reset(ft5x0x);
#endif
	enable_irq(g_client->irq);		
}
#else
static int ft5x0x_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return 0;
}
static int ft5x0x_resume(struct i2c_client *client)
{
	return 0;
}
#endif

static const struct i2c_device_id ft5x0x_id[] = {
		{"ft5x0x", 0},
		{ }
};

static struct i2c_driver ft5x0x_driver = {
	.driver = {
		.name = "ft5x0x",
	    },
	.id_table 	= ft5x0x_id,
	.probe		= ft5x0x_probe,
	.remove		= __devexit_p(ft5x0x_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND	
	.suspend = &ft5x0x_suspend,
	.resume = &ft5x0x_resume,
#endif	
};

static int ft5x0x_init_client(struct i2c_client *client)
{
	struct ft5x0x_data *ft5x0x;
	int ret;
	int index;
	ft5x0x = i2c_get_clientdata(client);

#if 0
	DBG("gpio_to_irq(%d) is %d\n",client->irq,gpio_to_irq(client->irq));
	if ( !gpio_is_valid(client->irq)) {
		DBG("+++++++++++gpio_is_invalid\n");
		return -EINVAL;
	}

	gpio_free(client->irq);
	ret = gpio_request(client->irq, "ft5x0x_int");
	if (ret) {
		DBG( "failed to request ft5x0x GPIO%d\n",gpio_to_irq(client->irq));
		return ret;
	}

    ret = gpio_direction_input(client->irq);
    if (ret) {
        DBG("failed to set ft5x0x  gpio input\n");
		return ret;
    }

	gpio_pull_updown(client->irq, GPIOPullUp);
#endif

	//client->irq = gpio_to_irq(client->irq);
	 //index = imapx_pad_index("cond20");
     index = item_integer("ts.int", 1); 
	  if(index == -1)
		  return -EINVAL;
	   tsp_irq_index = index;  
	   int reg = 0;  
	    reg = readl(PAD_SYSM_VA);
	     writel(0,PAD_SYSM_VA);
	      _sui_irq_num = imapx_pad_irq_number(index);
	       if(!_sui_irq_num)
		       return -EINVAL; 
	       imapx_pad_set_mode(0, 1, index);
	       imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);
	client->irq = _sui_irq_num;
	ft5x0x->irq = client->irq;
	ret = request_irq(client->irq, ft5x0x_interrupt, /*IRQF_TRIGGER_FALLING*/IRQF_DISABLED, client->dev.driver->name, ft5x0x);
	DBG("request irq is %d,ret is  0x%x\n",client->irq,ret);
	if (ret ) {
		DBG(KERN_ERR "ft5x0x_init_client: request irq failed,ret is %d\n",ret);
        return ret;
	}
	//disable_irq(client->irq);
 
	return 0;
}



#define    FTS_PACKET_LENGTH        2

static unsigned char CTPM_FW[]=
	{
#ifdef CONFIG_HLT_YFW
		#include "HLT_YFW_COB_V07_120328.i"
#endif
#ifdef CONFIG_QH_YFW
		#include "QH-YFW-COB-OLD_V07_120328.i"
#endif
#ifdef CONFIG_FT_YFW
		#include "FT-YFW-COB-OLD_0409.i"
#endif
#ifdef CONFIG_ZLCG_YFW
		#include "ZHILIN-YFW-0426.i"
#endif
	};

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(struct i2c_client *client,FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    unsigned char fusir_8u=0;
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc; int      i_ret;


    //printk(KERN_INFO "dw_lenth = %d\n",dw_lenth);
    if(dw_lenth == 0)
    {
	    printk(KERN_INFO "Nothing should be built in\n");
	    return -1;
    }

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(client, 0xfc, 0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(client,0xfc,0x55);
    //printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   


    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_tx_data(client,auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    ft_cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        //printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
        //i_is_new_protocol = 1;
    }

    ft_cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    //printk("[FTS] bootloader version = 0x%x\n", reg_val[0]);

     /*********Step 4:erase app and panel paramenter area ********************/
    ft_cmd_write(0x61,0x00,0x00,0x00,1);  //erase app area
    delay_qt_ms(1500); 
    ft_cmd_write(0x63,0x00,0x00,0x00,1);  //erase panel parameter area
    delay_qt_ms(100);
    //printk("[FTS] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    //printk("[FTS] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              //printk("[FTS] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    ft_cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    //printk("[FTS] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    ft_cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(300);  //make sure CTP startup normally
 //   fusir_8u=ft5x0x_read_reg(client,0xA6);
  //  printk(".............................................050402.....read_reg = %d\n", fusir_8u);
    
    return ERR_OK;
}





#if  0
static E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(struct i2c_client *client,FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{

    unsigned char fusir_8u=0;
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;
    int fusir_gpio;
    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
//    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_DWRD  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;
    fusir_8u=ft5x0x_read_reg(client,0xA6);
    printk(".............................................0504.....read_reg = %d\n", fusir_8u); 
/* while(1)
{
	ft5x0x_read_reg(client,0xA6);
	delay_qt_ms(13);
	
} 
 
//    int ms[7]={21,24,27,30,33,36,39,};
    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    //ft5x0x_write_reg(0xfc,0xaa);


   ft5x0x_write_reg(client, 0xfc, 0xaa);
    delay_qt_ms(50);
     //write 0x55 to register 0xfc
    //ft5x0x_write_reg(0xfc,0x55);
    ft5x0x_write_reg(client, 0xfc, 0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");


/*
/////////////////////by fusir
fusir_gpio =__imapx_name_to_gpio("GPP9");
if(fusir_gpio == IMAPX_GPIO_ERROR) {
		printk(KERN_ERR "failed to get fusir_gpio pin.\n");
		return -1;
	}
imapx_gpio_setcfg(fusir_gpio, IG_OUTPUT, IG_NORMAL);
imapx_gpio_setpin(fusir_gpio, 1, IG_NORMAL);
delay_qt_ms(10);
imapx_gpio_setpin(fusir_gpio, 0, IG_NORMAL);
delay_qt_ms(2);
printk("........................................");
imapx_gpio_setpin(fusir_gpio, 1, IG_NORMAL);
/////////////////////////
*/

//    fusir_8u=ft5x0x_read_reg(client,0xA6);
//    printk(".............................................0504010000000.....read_reg = %d\n", fusir_8u);

    delay_qt_ms(25);

//  for(i=0;i<7;i++)
 //   delay_qt_ms(ms[i]);

//    fusir_8u=ft5x0x_read_reg(client,0xA6);
//    printk(".............................................050401.....read_reg = %d\n", fusir_8u);

    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
       // i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
	i_ret = ft5x0x_tx_data(client, auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );
	printk("[FTS] Step 2: Enter upgrade mode\n");

//    fusir_8u=ft5x0x_read_reg(client,0xA6);
//    printk(".............................................050402.....read_reg = %d\n", fusir_8u);

    /*********Step 3:check READ-ID***********************/        
    ft_cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
        printk("[FTS] Step 301: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
      printk("...................................................................................................");
        return ERR_READID;
        //i_is_new_protocol = 1;
    }

    ft_cmd_write(0xcd,0x0,0x00,0x00,1);	
    byte_read(reg_val,1);
    printk("[FTS] bootloader version = 0x%x\n", reg_val[0]);

//    fusir_8u=ft5x0x_read_reg(client,0xA6);
//    printk(".............................................050403.....read_reg = %d\n", fusir_8u);

     /*********Step 4:erase app and panel paramenter area ********************/
    ft_cmd_write(0x61,0x00,0x00,0x00,1);  //erase app area
    delay_qt_ms(1500); 
    ft_cmd_write(0x63,0x00,0x00,0x00,1);  //erase panel parameter area
    delay_qt_ms(100);
    printk("[FTS] Step 4: erase. \n");

//    fusir_8u=ft5x0x_read_reg(client,0xA6);
//    printk(".............................................050404.....read_reg = %d\n", fusir_8u);

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
//    printk("[FTS] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
//	printk("............................................01\n");
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
//	printk("...........................................02.......packet_number = %d\n", packet_number);
    packet_buf[0] = 0xbf;
//	printk("............................................03\n");
    packet_buf[1] = 0x00;
//	printk("............................................04\n");
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
//printk("............................................0501\n");
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;
//printk("............................................0502.............packet_buf[5] = %d\n", packet_buf[5]);

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
         //printk("............................................0503.....FTS_PACKET_LENGTH = %d\n", i);
	 //printk("............................................0504.....packet_buf = %d\n", packet_buf[6+i]);
	 //printk("............................................0505.....pbt_buf[j*FTS_PACKET_LENGTH + i] = %d\n", pbt_buf[j*FTS_PACKET_LENGTH + i]);
//	fusir_8u=ft5x0x_read_reg(client,0xA6);
//	printk(".............................................050405.....read_reg = %d\n", fusir_8u);  
	 packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
	//printk("............................................05\n");
            bt_ecc ^= packet_buf[6+i];
        }
//        printk("............................................06\n");
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[FTS] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    ft_cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    ft_cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(300);  //make sure CTP startup normally
    
    return ERR_OK;
}
#endif 


int fts_ctpm_auto_clb(struct i2c_client *client)
{
    unsigned char uc_temp;
    unsigned char i ;

    //printk("[FTS] start auto CLB.\n");
    msleep(200);
    ft5x0x_write_reg(client,0, 0x40);  
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(client,2, 0x4);  //write command to start calibration
    delay_qt_ms(300);
    for(i=0;i<100;i++)
    {
        ft5x0x_read_reg(client,0);
        if ( ((uc_temp&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
        {
            break;
        }
        delay_qt_ms(200);
        //printk("[FTS] waiting calibration %d\n",i);
        
    }
    //printk("[FTS] calibration OK.\n");
    
    msleep(300);
    ft5x0x_write_reg(client,0, 0x40);  //goto factory mode
    delay_qt_ms(100);   //make sure already enter factory mode
    ft5x0x_write_reg(client,2, 0x5);  //store CLB result
    delay_qt_ms(300);
    ft5x0x_write_reg(client,0, 0x0); //return to normal mode 
    msleep(300);
    //printk("[FTS] store CLB result OK.\n");
//    return 0;
}


static int  ft5x0x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	FTS_BYTE*    pbt_buf=FTS_NULL;
	pbt_buf = CTPM_FW;
        int i_ret;
	int index;

	struct ft5x0x_data *ft5x0x_ts;
//	struct ts_hw_data *pdata = client->dev.platform_data;
	int err = 0;
	int i;
	///struct i2c_adapter *adapter;

	printk("%s enter\n",__FUNCTION__);

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_data), GFP_KERNEL);
	if (!ft5x0x_ts) {
		DBG("[ft5x0x]:alloc data failed.\n");
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
    	//client->addr = 0x30;	//?
	///adapter = i2c_get_adapter(1);
	//client->adapter = adapter;

	g_client = client;
	ft5x0x_ts->client = client;
	ft5x0x_ts->last_points = 0;
	//ft5x0x_ts->reset_gpio = pdata->reset_gpio;
	//ft5x0x_ts->touch_en_gpio = pdata->touch_en_gpio;
	i2c_set_clientdata(client, ft5x0x_ts);

	//gpio_free(ft5x0x_ts->reset_gpio);
	//err = gpio_request(ft5x0x_ts->reset_gpio, "ft5x0x rst");
	//if (err) {
	//	DBG( "failed to request ft5x0x reset GPIO%d\n",gpio_to_irq(client->irq));
	//	goto exit_alloc_gpio_rst_failed;
	//}
	err = ft5x0x_chip_init(client);
/*
 * TP firmware Flash
 */
	uint32_t ver;
	ver = ft5x0x_read_reg(client,0xA6);
	printk(KERN_INFO "ver = 0x%x\n",ver);
	if(ver != 0x7)
	{
		//fts_ctpm_fw_upgrade(client,pbt_buf, sizeof(CTPM_FW));
		//printk("[FTS] Begin to update FT fiware\n");

		i_ret =  fts_ctpm_fw_upgrade(client,pbt_buf,sizeof(CTPM_FW));

		if (i_ret != 0)
		{
			printk("[FTS] upgrade failed i_ret = %d.\n", i_ret);
			//error handling ...
			//TBD
		}
		else
		{
			printk("[FTS] upgrade successfully.\n");
			fts_ctpm_auto_clb(client);  //start auto CLB
			fts_ctpm_auto_clb(client);  //start auto CLB
		}
	}

#if defined (GPIO_TOUCH_EN)
	gpio_free(ft5x0x_ts->touch_en_gpio);
	err = gpio_request(ft5x0x_ts->touch_en_gpio, "ft5x0x power enable");
	if (err) {
		DBG( "failed to request ft5x0x power enable GPIO%d\n",gpio_to_irq(client->irq));
		goto exit_alloc_gpio_power_failed;
	}
#endif


	if (err < 0) {
		printk(KERN_ERR
		       "ft5x0x_probe: ft5x0x chip init failed\n");
		goto exit_request_gpio_irq_failed;
	}

	ft5x0x_ts->timer.expires=jiffies + msecs_to_jiffies(1);
	///setup_timer(&ft5x0x_ts->timer,ft5x0x_tpscan_timer,(unsigned long)ft5x0x_ts);

	INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_delaywork_func);
	ft5x0x_ts->ts_workqueue = create_singlethread_workqueue("ft5x0x_ts");
	if (!ft5x0x_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_request_gpio_irq_failed;
	}
//printk(KERN_ERR"......................................................................\n");
//printk(KERN_ERR"tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt\n");



	ft5x0x_ts->input_dev = input_allocate_device();
	if (!ft5x0x_ts->input_dev) {
		err = -ENOMEM;
		printk(KERN_ERR
		       "ft5x0x_probe: Failed to allocate input device\n");
		goto exit_input_allocate_device_failed;
	}

	ft5x0x_ts->input_dev->name = "ft5x0x-ts";
	ft5x0x_ts->input_dev->dev.parent = &client->dev;

	err = input_register_device(ft5x0x_ts->input_dev);
	if (err < 0) {
		printk(KERN_ERR
		       "ft5x0x_probe: Unable to register input device: %s\n",
		       ft5x0x_ts->input_dev->name);
		goto exit_input_register_device_failed;
	}

#ifdef TOUCHKEY_ON_SCREEN
	#ifdef TOUCH_KEY_LED
		err = gpio_request(TOUCH_KEY_LED, "key led");
		if (err < 0) {
			printk(KERN_ERR
			       "ft5x0x_probe: Unable to request gpio: %d\n",
			       TOUCH_KEY_LED);
			goto exit_input_register_device_failed;
		}
		gpio_direction_output(TOUCH_KEY_LED, GPIO_LOW);
		gpio_set_value(TOUCH_KEY_LED, GPIO_LOW);
	#endif
	
	tp_key_input = ft5x0x_ts->input_dev;
	for (i = 0; i < ARRAY_SIZE(initkey_code); i++)
		set_bit(initkey_code[i], ft5x0x_ts->input_dev->keybit);
#endif
	for (i = 0; i < ARRAY_SIZE(button); i++)
	{                               
		set_bit(button[i] & KEY_MAX, ft5x0x_ts->input_dev->keybit);
	}               

	ft5x0x_ts->key_status = ft5x0x_ts->old_status = 0;  


#if MULTI_TOUCH
	set_bit(EV_SYN, ft5x0x_ts->input_dev->evbit);
	set_bit(EV_KEY, ft5x0x_ts->input_dev->evbit);
	set_bit(EV_ABS, ft5x0x_ts->input_dev->evbit);
	//set_bit(BTN_TOUCH, ft5x0x_ts->input_dev->keybit);
	//set_bit(BTN_2, ft5x0x_ts->input_dev->keybit);
	//set_bit(BTN_3, ft5x0x_ts->input_dev->keybit);
	//set_bit(BTN_4, ft5x0x_ts->input_dev->keybit);
	//set_bit(BTN_5, ft5x0x_ts->input_dev->keybit);

	//input_set_abs_params(ft5x0x_ts->input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	//input_set_abs_params(ft5x0x_ts->input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	//input_set_abs_params(ft5x0x_ts->input_dev, ABS_HAT0X, 0, SCREEN_MAX_X, 0, 0);
	//input_set_abs_params(ft5x0x_ts->input_dev, ABS_HAT0Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 50, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
#else
	set_bit(EV_SYN, ft5x0x_ts->input_dev->evbit);
	set_bit(EV_KEY, ft5x0x_ts->input_dev->evbit);
	set_bit(EV_ABS, ft5x0x_ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ft5x0x_ts->input_dev->keybit);

	input_set_abs_params(ft5x0x_ts->input_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(ft5x0x_ts->input_dev, ABS_PRESSURE, 0, 0, 0, 0);
#endif
	
#ifdef CONFIG_HAS_EARLYSUSPEND
    ft5x0x_early_suspend.suspend = ft5x0x_suspend;
    ft5x0x_early_suspend.resume = ft5x0x_resume;
    ft5x0x_early_suspend.level = 0x2;
    register_early_suspend(&ft5x0x_early_suspend);
#endif

	err = ft5x0x_init_client(client);
	if (err < 0) {
		printk(KERN_ERR
				"ft5x0x_probe: ft5x0x_init_client failed\n");
		goto exit_input_register_device_failed;
	}
	return 0;

exit_input_register_device_failed:
	input_free_device(ft5x0x_ts->input_dev);
exit_input_allocate_device_failed:
    free_irq(client->irq, ft5x0x_ts);
exit_request_gpio_irq_failed:
	kfree(ft5x0x_ts);	
exit_alloc_gpio_power_failed:
#if defined (GPIO_TOUCH_EN)
	gpio_free(ft5x0x_ts->touch_en_gpio);
#endif
exit_alloc_gpio_rst_failed:
    gpio_free(ft5x0x_ts->reset_gpio);
exit_alloc_data_failed:
	printk("%s error\n",__FUNCTION__);
	return err;
}


static int __init ft5x0x_mod_init(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret;
    int gpio,irq;

    printk("------ft5x0x module init------\n");
    if(item_exist("ts.model"))
    {
        if(item_equal("ts.model", "ft5304", 0))
        {
            memset(&info, 0, sizeof(struct i2c_board_info));
            info.addr = 0x38;  
            // the i2c addr 0x38 is suit for ft5406,and 0x39 is ft5206.
            info.irq = irq,
                //info.irq = IRQ_GPIO,
                //info.addr = EKT2201_I2C_ADDR;  
                strlcpy(info.type, "ft5x0x", I2C_NAME_SIZE);

            adapter = i2c_get_adapter(2);
            //adapter = i2c_get_adapter(CONFIG_ACC_EKT2201_I2C + 1);
            if (!adapter) {
                printk("*******get_adapter error!********\n");
            }
            client = i2c_new_device(adapter, &info);
            //printk("++++++ft5x0x module init++++++\n");

            return i2c_add_driver(&ft5x0x_driver);
        }else
            printk(KERN_ERR "%s: touchscreen is not ft5x0x\n", __func__); 

    }else
        printk(KERN_ERR "%s: touchscreen is not exist\n", __func__);

}

static void __exit ft5x0x_mod_exit(void)
{
	i2c_del_driver(&ft5x0x_driver);
}


late_initcall(ft5x0x_mod_init);
module_exit(ft5x0x_mod_exit);

MODULE_DESCRIPTION("SO381010 Camera sensor driver");
MODULE_AUTHOR("lbt <kernel@rock-chips>");
MODULE_LICENSE("GPL");
