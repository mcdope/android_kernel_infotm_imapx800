/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *    note: only support mulititouch    Wenfs 2010-10-01
 */


//#define CONFIG_FTS_CUSTOME_ENV
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/pad.h>
#include <mach/power-gate.h>
#include <mach/imap-iomap.h>
#include <mach/items.h>
#include <linux/io.h>

#include "ft5x0x.h"

//#define CONFIG_CTP_FT5X0X_I2C   (2)
//#define CONFIG_CTP_FT5X0X_INT   "cond20"

/* -------------- global variable definition -----------*/
static struct i2c_client *this_client;
static REPORT_FINGER_INFO_T _st_finger_infos[CFG_MAX_POINT_NUM];
static unsigned int _sui_irq_num;
static int _si_touch_num = 0;
static int tsp_irq_index;
static int SCREEN_MAX_X=1024;
static int SCREEN_MAX_Y=600;
static int orient_num = 1 ;
static int key_num = 0;
static int key_span = 0;
static struct fts_key keys_data[CFG_NUMOFKEYS];
static atomic_t ping_ok;

extern int get_lcd_width(void);
extern int get_lcd_height(void);

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
static void ft5x0x_early_suspend(struct early_suspend *s);
static void ft5x0x_early_resume(struct early_suspend *s);
#endif



//static bool tsp_keystatus[CFG_NUMOFKEYS];

/***********************************************************************
  [function]: 
callback:              read data from ctpm by i2c interface;
[parameters]:
buffer[in]:            data buffer;
length[in]:           the length of the data buffer;
[return]:
FTS_TRUE:            success;
FTS_FALSE:           fail;
 ************************************************************************/
static bool i2c_read_interface(u8* pbt_buf, int dw_lenth)
{
	int ret;

	ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

	if(ret<=0)
	{
		printk("[TSP]i2c_read_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}



/***********************************************************************
  [function]: 
callback:               write data to ctpm by i2c interface;
[parameters]:
buffer[in]:             data buffer;
length[in]:            the length of the data buffer;
[return]:
FTS_TRUE:            success;
FTS_FALSE:           fail;
 ************************************************************************/
static bool  i2c_write_interface(u8* pbt_buf, int dw_lenth)
{
	int ret;
	ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
	if(ret<=0)
	{
		printk("[TSP]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
		return FTS_FALSE;
	}

	return FTS_TRUE;
}



/***********************************************************************
  [function]: 
callback:                 read register value ftom ctpm by i2c interface;
[parameters]:
reg_name[in]:         the register which you want to read;
rx_buf[in]:              data buffer which is used to store register value;
rx_length[in]:          the length of the data buffer;
[return]:
FTS_TRUE:              success;
FTS_FALSE:             fail;
 ************************************************************************/
static bool fts_register_read(u8 reg_name, u8* rx_buf, int rx_length)
{
	u8 read_cmd[2]= {0};
	u8 cmd_len 	= 0;

	read_cmd[0] = reg_name;
	cmd_len = 1;	

	/*send register addr*/
	if(!i2c_write_interface(&read_cmd[0], cmd_len))
	{
		return FTS_FALSE;
	}

	/*call the read callback function to get the register value*/		
	if(!i2c_read_interface(rx_buf, rx_length))
	{
		return FTS_FALSE;
	}
	return FTS_TRUE;
}




/***********************************************************************
  [function]: 
callback:                read register value ftom ctpm by i2c interface;
[parameters]:
reg_name[in]:         the register which you want to write;
tx_buf[in]:              buffer which is contained of the writing value;
[return]:
FTS_TRUE:              success;
FTS_FALSE:             fail;
 ************************************************************************/
static bool fts_register_write(u8 reg_name, u8* tx_buf)
{
	u8 write_cmd[2] = {0};

	write_cmd[0] = reg_name;
	write_cmd[1] = *tx_buf;

	/*call the write callback function*/
	return i2c_write_interface(write_cmd, 2);
}

/***********************************************************************
  [function]: 
callback:                 read touch  data ftom ctpm by i2c interface;
[parameters]:
rxdata[in]:              data buffer which is used to store touch data;
length[in]:              the length of the data buffer;
[return]:
FTS_TRUE:              success;
FTS_FALSE:             fail;
 ************************************************************************/
static int fts_i2c_rxdata(u8 *rxdata, int length)
{
	int ret;
	struct i2c_msg msg;


	msg.addr = this_client->addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = rxdata;
	ret = i2c_transfer(this_client->adapter, &msg, 1);

	if (ret < 0)
		pr_err("msg %s i2c write error: %d\n", __func__, ret);

	msg.addr = this_client->addr;
	msg.flags = I2C_M_RD;
	msg.len = length;
	msg.buf = rxdata;
	ret = i2c_transfer(this_client->adapter, &msg, 1);
	if (ret < 0)
		pr_err("msg %s i2c write error: %d\n", __func__, ret);

	return ret;
}





/***********************************************************************
  [function]: 
callback:                send data to ctpm by i2c interface;
[parameters]:
txdata[in]:              data buffer which is used to send data;
length[in]:              the length of the data buffer;
[return]:
FTS_TRUE:              success;
FTS_FALSE:             fail;
 ************************************************************************/
static int fts_i2c_txdata(u8 *txdata, int length)
{
	int ret;

	struct i2c_msg msg;

	msg.addr = this_client->addr;
	msg.flags = 0;
	msg.len = length;
	msg.buf = txdata;
	ret = i2c_transfer(this_client->adapter, &msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}

/***********************************************************************
  [function]: 
callback:            gather the finger information and calculate the X,Y
coordinate then report them to the input system;
[parameters]:
null;
[return]:
null;
 ************************************************************************/
int fts_read_data(void)
{
	struct FTS_TS_DATA_T *data = i2c_get_clientdata(this_client);
	u8 buf[6*CFG_MAX_POINT_NUM+3] = {0};
	u8 * pt_tmp;
	static int key_id=0x80;
	int i,id,temp,temp1,i_count,ret = -1,k;
	int touch_event, x, y;
	i_count = 0;

	DBG_FUNC(KERN_INFO"Enter %s ,and ts orientation\n", __func__);
	ret=fts_i2c_rxdata(buf, 6*CFG_MAX_POINT_NUM+3);
	DBG_FUNC("ret = %d\n", ret);
	if (ret > 0) {
		for (k = 0; k < CFG_MAX_POINT_NUM; ++k) {
			pt_tmp = buf+3+k*6;
			touch_event = pt_tmp[0]>>6;
			id = pt_tmp[2] >> 4;
			if (id >= 0 && id< CFG_MAX_POINT_NUM) {
				temp = ((pt_tmp[0] & 0x0F) << 8) | pt_tmp[1];
				temp1 = ((pt_tmp[2] & 0x0F) << 8) | pt_tmp[3];
			}
			if (touch_event == 0) {//press down
				x = temp;
				y = temp1;
				for(i=0; i<key_num; i++) {							
					if((x >= keys_data[i].x-key_span) && (x <= keys_data[i].x+key_span) && (y >= keys_data[i].y-key_span) && (y <= keys_data[i].y+key_span)) {
						input_report_key(data->input_dev, keys_data[i].value, 1);
						keys_data[i].press = 1;
						key_id = i;
						break;
					}
				}
					//tsp_keystatus[key_id] = KEY_PRESS;
			}
			if(touch_event == 1) {
				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				//input_report_abs(data->input_dev, ABS_MT_POSITION_X, _st_finger_infos[i].i2_x);
				//input_report_abs(data->input_dev, ABS_MT_POSITION_Y, _st_finger_infos[i].i2_y);
				//input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, _st_finger_infos[i].u2_pressure);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(data->input_dev, BTN_TOUCH, 0);
				//input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, _st_finger_infos[i].ui2_id);
				input_mt_sync(data->input_dev);
				DBG_FUNC("up\n");
			}
			else if(touch_event == 2) {
				switch(orient_num){
					case 1:
						x = temp;
						y = temp1;
						break;
					case 2:
						x = temp;
						y = SCREEN_MAX_Y - temp1;
						break;
					case 3:
						x = SCREEN_MAX_X - temp;
						y = temp1;
						break;
					case 4: 
						x = SCREEN_MAX_X - temp;
						y = SCREEN_MAX_Y - temp1;
						break;
					case 5:
						x = temp1;
						y = temp;
						break;
					case 6:
						x = temp1;
						y = SCREEN_MAX_Y - temp;
						break;
					case 7:
						x = SCREEN_MAX_X - temp1;
						y = temp;
						break;
					case 8:
						x = SCREEN_MAX_X - temp1;
						y = SCREEN_MAX_Y - temp;
						break;
				}

				input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, id);
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 1);
				input_report_abs(data->input_dev, ABS_MT_POSITION_X,  x);
				input_report_abs(data->input_dev, ABS_MT_POSITION_Y,  y);
				input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 50);
				input_report_key(data->input_dev, BTN_TOUCH, 1);
				input_mt_sync(data->input_dev);
				DBG_FUNC("x,y = %d, %d\n", x, y);
			}
			else {
			}
		}
		input_sync(data->input_dev);
	}	
	return 0;
}

static void fts_work_func(struct work_struct *work)
{
	DBG_FUNC(KERN_INFO"Enter %s\n", __func__);
	fts_read_data();    
}

static irqreturn_t fts_ts_irq(int irq, void *dev_id)
{
	struct FTS_TS_DATA_T *ft5x0x_ts = dev_id;

	DBG_FUNC(KERN_INFO"Enter %s\n", __func__);

	if(imapx_pad_irq_pending(tsp_irq_index))
		imapx_pad_irq_clear(tsp_irq_index);
	else
		return IRQ_HANDLED;

	if (!work_pending(&ft5x0x_ts->pen_event_work)) {
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}

	return IRQ_HANDLED;
}

/***********************************************************************
  [function]: 
callback:         send a command to ctpm.
[parameters]:
btcmd[in]:       command code;
btPara1[in]:     parameter 1;    
btPara2[in]:     parameter 2;    
btPara3[in]:     parameter 3;    
num[in]:         the valid input parameter numbers, 
if only command code needed and no 
parameters followed,then the num is 1;    
[return]:
FTS_TRUE:      success;
FTS_FALSE:     io fail;
 ************************************************************************/
static bool cmd_write(u8 btcmd,u8 btPara1,u8 btPara2,u8 btPara3,u8 num)
{
	u8 write_cmd[4] = {0};

	write_cmd[0] = btcmd;
	write_cmd[1] = btPara1;
	write_cmd[2] = btPara2;
	write_cmd[3] = btPara3;
	return i2c_write_interface(write_cmd, num);
}

/***********************************************************************
  [function]: 
callback:         write a byte data  to ctpm;
[parameters]:
buffer[in]:       write buffer;
length[in]:      the size of write data;    
[return]:
FTS_TRUE:      success;
FTS_FALSE:     io fail;
 ************************************************************************/
static bool byte_write(u8* buffer, int length)
{

	return i2c_write_interface(buffer, length);
}

/***********************************************************************
  [function]: 
callback:         read a byte data  from ctpm;
[parameters]:
buffer[in]:       read buffer;
length[in]:      the size of read data;    
[return]:
FTS_TRUE:      success;
FTS_FALSE:     io fail;
 ************************************************************************/
static bool byte_read(u8* buffer, int length)
{
	return i2c_read_interface(buffer, length);
}

#define    FTS_PACKET_LENGTH        128

const unsigned char CTPM_FW[]=
{
	#include "ft_app.i"
	//#include "QH-YFW-COB-OLD_V07_120328.i"
};

/***********************************************************************
  [function]: 
callback:          burn the FW to ctpm.
[parameters]:
pbt_buf[in]:     point to Head+FW ;
dw_lenth[in]:   the length of the FW + 6(the Head length);    
[return]:
ERR_OK:          no error;
ERR_MODE:      fail to switch to UPDATE mode;
ERR_READID:   read id fail;
ERR_ERASE:     erase chip fail;
ERR_STATUS:   status error;
ERR_ECC:        ecc error.
 ************************************************************************/
E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(u8* pbt_buf, int dw_lenth)
{
	u8  cmd,reg_val[2] = {0};
	u8  packet_buf[FTS_PACKET_LENGTH + 6];
	u8  auc_i2c_write_buf[10];
	u8  bt_ecc;

	int  j,temp,lenght,i_ret,packet_number, i = 0;
	int  i_is_new_protocol = 0;


	/******write 0xaa to register 0xfc******/
	cmd=0xaa;
	fts_register_write(0xfc,&cmd);
	mdelay(50);

	/******write 0x55 to register 0xfc******/
	cmd=0x55;
	fts_register_write(0xfc,&cmd);
	printk("[TSP] Step 1: Reset CTPM test\n");

	mdelay(10);   


	/*******Step 2:Enter upgrade mode ****/
	printk("\n[TSP] Step 2:enter new update mode\n");
	auc_i2c_write_buf[0] = 0x55;
	auc_i2c_write_buf[1] = 0xaa;
	do
	{
		i ++;
		i_ret = fts_i2c_txdata(auc_i2c_write_buf, 2);
		mdelay(5);
	}while(i_ret <= 0 && i < 10 );

	if (i > 1)
	{
		i_is_new_protocol = 1;
	}

	/********Step 3:check READ-ID********/        
	cmd_write(0x90,0x00,0x00,0x00,4);
	byte_read(reg_val,2);
	if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
	{
		printk("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	}
	else
	{
		return ERR_READID;
		//i_is_new_protocol = 1;
	}


	/*********Step 4:erase app**********/
	if (i_is_new_protocol)
	{
		cmd_write(0x61,0x00,0x00,0x00,1);
	}
	else
	{
		cmd_write(0x60,0x00,0x00,0x00,1);
	}
	mdelay(1500);
	printk("[TSP] Step 4: erase. \n");



	/*Step 5:write firmware(FW) to ctpm flash*/
	bt_ecc = 0;
	printk("[TSP] Step 5: start upgrade. \n");
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
		mdelay(FTS_PACKET_LENGTH/6 + 1);
		if ((j * FTS_PACKET_LENGTH % 1024) == 0)
		{
			printk("[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
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
		mdelay(20);
	}

	/***********send the last six byte**********/
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
		mdelay(20);
	}

	/********send the opration head************/
	cmd_write(0xcc,0x00,0x00,0x00,1);
	byte_read(reg_val,1);
	printk("[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
	if(reg_val[0] != bt_ecc)
	{
		return ERR_ECC;
	}

	/*******Step 7: reset the new FW**********/
	cmd_write(0x07,0x00,0x00,0x00,1);

	return ERR_OK;
}

int fts_ctpm_fw_upgrade_with_i_file(void)
{
	u8* pbt_buf = FTS_NULL;
	int i_ret;

	pbt_buf = (u8*) CTPM_FW;
	i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW));

	return i_ret;
}

unsigned char fts_ctpm_get_upg_ver(void)
{
	unsigned int ui_sz;

	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2)
	{
		return CTPM_FW[ui_sz - 2];
	}
	else
		return 0xff; 

}

static void ft_reset(void)
{
	int index;
	index = item_integer("ts.reset", 1);

	//index = imapx_pad_index("UART1_RTS");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0, 1, index);/*output*/
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	msleep(10);
	imapx_pad_set_outdat(0, 1, index);/*output 0*/
	msleep(5);
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	msleep(10);

    cmd_write(0x07,0x00,0x00,0x00,1);
    msleep(200);
}

static ssize_t ping_i2c_ok_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char str[2][16]={"Ping Error","Ping Ok", "Already Turn Off the Item Switch"};
	int flag;
	flag=atomic_read(&ping_ok);
	return sprintf(buf, "%s\n", str[flag]);
}

static struct device_attribute ping_i2c_attributes[] = {
	__ATTR(ping_i2c, /*0660*/0666, ping_i2c_ok_show, NULL),
	__ATTR_NULL,
};
static int fts_create_file()
{
	dev_t devno;
	struct class *class;struct device * device;
	int err = -1;
	err = alloc_chrdev_region(&devno, 0, 1, "PING_I2C_NAME");
  	if(err)
	{
		printk("%s, can't allocate chrdev\n", __func__);
		return err;
	}
	class = class_create(THIS_MODULE, "ping_i2c_name");
	if(IS_ERR(class))
	{
   		printk("%s, create class, error\n", __func__);
		return err;
  	}
	device=device_create(class, NULL, devno, NULL, "ping_i2c_name");
	//err = create_device_attributes(device, ping_i2c_attributes);
	if(device_create_file(device, &ping_i2c_attributes[0]) < 0)	return -1;
	return 0;
}

static int fts_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct FTS_TS_DATA_T *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char reg_value;
	unsigned char reg_version;
	int i;
	int index;
	char orient_x, orient_y;
    char orientation[ITEM_MAX_LEN];
    int r_index;
	r_index = item_integer("ts.reset", 1);
	imapx_pad_set_mode(1, 1, r_index);/*gpio*/
	imapx_pad_set_dir(0, 1, r_index);
	DBG_FUNC("Enter %s\n", __func__);
	index = item_integer("ts.int", 1);
	tsp_irq_index = index;
	_sui_irq_num = imapx_pad_irq_number(index);/* get irq */
	if(!_sui_irq_num)
		return -EINVAL;
	/*printk("CTP irq %d\n", _sui_irq_num);*/

	imapx_pad_set_mode(0, 1, index);/* func mode */
	//imapx_pad_set_dir(1, 1, index);/* input */
	imapx_pad_irq_config(index, INTTYPE_FALLING, FILTER_MAX);/* set trigger mode and filter */

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(*ft5x0x_ts), GFP_KERNEL);
	if (!ft5x0x_ts)    {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	this_client = client;
	i2c_set_clientdata(client, ft5x0x_ts);
	
	ft_reset();
	if(item_exist("ts.i2c_test")) {
		if(fts_create_file() < 0) {
			printk("%s, create file, error\n", __func__);
			return -1;
		}
		if(fts_start_ping_i2c_addr())
			atomic_set(&ping_ok, 1);
		else
			atomic_set(&ping_ok, 0);
	}
	else {
		atomic_set(&ping_ok, 2);
	}
	INIT_WORK(&ft5x0x_ts->pen_event_work, fts_work_func);

	ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ft5x0x_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

	/***wait CTP to bootup normally***/
	msleep(200); 
    
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

    fts_register_read(FT5X0X_REG_FIRMID, &reg_version,1);
	printk("[TSP]firmware version = 0x%2x\n", reg_version);
	fts_register_read(FT5X0X_REG_REPORT_RATE, &reg_value,1);
	printk("[TSP]firmware report rate = %dHz\n", reg_value*10);
	fts_register_read(FT5X0X_REG_THRES, &reg_value,1);
	printk("[TSP]firmware threshold = %d\n", reg_value * 4);
	fts_register_read(FT5X0X_REG_NOISE_MODE, &reg_value,1);
	printk("[TSP]nosie mode = 0x%2x\n", reg_value);

	//if(item_exist("ts.downloader"))
	if(0)
	{
		if (fts_ctpm_get_upg_ver() != reg_version)  //upgrade touch screen firmware if not the same with the firmware in host flashs
		{
			printk("[TSP] start upgrade new verison 0x%2x\n", fts_ctpm_get_upg_ver());
			msleep(200);
			err =  fts_ctpm_fw_upgrade_with_i_file();
			if (err == 0)
			{
				printk("[TSP] ugrade successfuly.\n");
				msleep(300);
				fts_register_read(FT5X0X_REG_FIRMID, &reg_value, 1);
				printk("FTS_DBG from old version 0x%2x to new version = 0x%2x\n", reg_version, reg_value);
			}
			else
			{
				printk("[TSP]  ugrade fail err=%d, line = %d.\n",
						err, __LINE__);
			}
			msleep(4000);
		}
	}

	err = request_irq(_sui_irq_num, fts_ts_irq, 0, "qt602240_ts", ft5x0x_ts);

	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	disable_irq(_sui_irq_num);

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	/***setup coordinate area******/
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(ABS_MT_TRACKING_ID, input_dev->absbit);
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	/****** for multi-touch *******/
	for (i=0; i<CFG_MAX_POINT_NUM; i++)   
		_st_finger_infos[i].u2_pressure = -1;

	input_set_abs_params(input_dev,
			ABS_MT_TRACKING_ID, 0, 30, 0, 0);
	input_set_abs_params(input_dev,
			ABS_MT_WIDTH_MAJOR, 0, 50, 0, 0);
	input_set_abs_params(input_dev,
			ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev,
			ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	DBG_DUMMY(KERN_INFO"Xmax:%d....Ymax:%d....PRESSmax:%d....ID:%d\n",
			SCREEN_MAX_X, SCREEN_MAX_Y, PRESS_MAX, ABS_MT_TRACKING_ID);
	/*****setup key code area******/
	set_bit(EV_SYN, input_dev->evbit);
	if(item_exist("ts.keynum") && item_exist("ts.key")) {
		//int num_key;
		key_num = item_integer("ts.keynum", 0);
		key_span = item_integer("ts.key", key_num*3);
			
		set_bit(EV_KEY, input_dev->evbit);
	//	set_bit(BTN_TOUCH, input_dev->keybit);
		input_dev->keycode = keys_data;
		for(i = 0; i < key_num; i++)
		{
			keys_data[i].value = item_integer("ts.key", i*3);
			keys_data[i].x = item_integer("ts.key", i*3+1);
			keys_data[i].y = item_integer("ts.key", i*3+2);
			keys_data[i].press = 0;
							
			input_set_capability(input_dev, EV_KEY, keys_data[i].value);
			//tsp_keystatus[i] = KEY_RELEASE;
		}
	}
	input_dev->name        = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
				"fts_ts_probe: failed to register input device: %s\n",
				dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

	enable_irq(_sui_irq_num);    
	printk("[TSP] file(%s), function (%s), -- end\n", __FILE__, __FUNCTION__);

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND
    ft5x0x_ts->eSuspnd.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
    ft5x0x_ts->eSuspnd.suspend = ft5x0x_early_suspend;             
    ft5x0x_ts->eSuspnd.resume = ft5x0x_early_resume;               
    register_early_suspend(&ft5x0x_ts->eSuspnd);                
#endif
    
	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
	free_irq(_sui_irq_num, ft5x0x_ts);
exit_irq_request_failed:
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
exit_create_singlethread:
	printk("[TSP] ==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	printk("%s err exit\n", __func__);
	return err;
}

static int __devexit fts_ts_remove(struct i2c_client *client)
{
	struct FTS_TS_DATA_T *ft5x0x_ts;

	DBG_FUNC("Enter %s\n", __func__);
	ft5x0x_ts = (struct FTS_TS_DATA_T *)i2c_get_clientdata(client);
#ifdef CONFIG_IDSDRV_EARLY_SUSPEND                     
    unregister_early_suspend(&ft5x0x_ts->eSuspnd);
#endif
	free_irq(_sui_irq_num, ft5x0x_ts);
	input_unregister_device(ft5x0x_ts->input_dev);
	kfree(ft5x0x_ts);
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
	i2c_set_clientdata(client, NULL);
	return 0;
}

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND                    
static void ft5x0x_early_suspend(struct early_suspend *s)
#else                                                 
static int fts_ts_suspend(struct i2c_client *client)
#endif
{
	disable_irq(_sui_irq_num);
#ifndef CONFIG_IDSDRV_EARLY_SUSPEND
    return 0;
#endif
}

#ifdef CONFIG_IDSDRV_EARLY_SUSPEND                   
static void ft5x0x_early_resume(struct early_suspend *s)
#else                                                
static int fts_ts_resume(struct i2c_client *client)
#endif
{
	imapx_pad_set_mode(0, 1, tsp_irq_index);
	imapx_pad_irq_config(tsp_irq_index, INTTYPE_FALLING, FILTER_MAX);
	ft_reset();
	enable_irq(_sui_irq_num);
	if(item_exist("ts.i2c_test")) {
		if(fts_start_ping_i2c_addr()) {
			atomic_set(&ping_ok, 1);
			printk("ping ok\n");
		}
		else {
			atomic_set(&ping_ok, 0);
			printk("ping error\n");
		}
	}
#ifndef CONFIG_IDSDRV_EARLY_SUSPEND
    return 0;
#endif
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FT5X0X_NAME, 0 },{ }
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver fts_ts_driver = {
	.probe        = fts_ts_probe,
	.remove        = __devexit_p(fts_ts_remove),
#ifndef CONFIG_IDSDRV_EARLY_SUSPEND
    .suspend        = fts_ts_suspend,
    .resume         = fts_ts_resume,
#endif
	.id_table    = ft5x0x_ts_id,
	.driver    = {
		.name = FT5X0X_NAME,
	},
};

/*
func:
this func only use for I2C device detect , if exit return 1, or return 0;
*/
extern int ctp_auto_detect;
#define LOG printk

static void ft_reset_detect(void)
{
	int index;
	index = item_integer("ts.reset", 1);

	//index = imapx_pad_index("UART1_RTS");
	imapx_pad_set_mode(1, 1, index);/*gpio*/
	imapx_pad_set_dir(0, 1, index);/*output*/
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	msleep(30);
	imapx_pad_set_outdat(0, 1, index);/*output 0*/
	msleep(20);
	imapx_pad_set_outdat(1, 1, index);/*output 1*/
	msleep(30);
}

static fts_start_ping_i2c_addr()
{	
	struct i2c_adapter *adapter=NULL;
	__u16 i2cAddr=0x38;//slave addr
	__u8 subByteAddr=0x0;//fix to read 0
	char databuf[8];
	int ret;

#define I2C_DETECT_TRY_SIZE  5
	int i,i2cDetecCnt=0;
	struct i2c_msg I2Cmsgs[2] = { 
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
//initDev
	ft_reset_detect();
	LOG("B%s\n",__FUNCTION__);
	//get i2c handle	
	adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
	if (!adapter) {
		printk("****** get_adapter error! ******\n");
		return 0;
	}
	LOG("C%s\n",__FUNCTION__);
	//scan i2c 
	for(i=0; i<I2C_DETECT_TRY_SIZE; i++) {
		ret = i2c_transfer(adapter,I2Cmsgs,2);
		LOG("D-%s,ret=%d\n",__FUNCTION__,ret);
		if(ret>0) break;			
	}
	printk("%s-try cnt = %d\n",  __FUNCTION__, i);
	if(i >= I2C_DETECT_TRY_SIZE) {
		return 0;
	}else {
		return 1;
	}
}

static int DoI2cScanDetectDev( void )
{
	LOG("A%s\n",__FUNCTION__);
	int ret = 0;
	if(item_exist("ts.model")) {
		if(item_equal("ts.model", "auto", 0)) {
			if(ctp_auto_detect ==1)	return 0;
			ret =  fts_start_ping_i2c_addr();
			if(ret == 1) 	ctp_auto_detect =1;
			return ret;
		}
	}
	return ret;
}

static int __init fts_ts_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	DBG_FUNC("Enter %s\n", __func__);
	//module_power_on(SYSMGR_GPIO_BASE);
	
	if(item_exist("ts.model"))
	{
		if(item_equal("ts.model", "ft5306", 0) || DoI2cScanDetectDev())
		{
			SCREEN_MAX_X = get_lcd_width();
			SCREEN_MAX_Y = get_lcd_height();
			
			memset(&info, 0, sizeof(struct i2c_board_info));
			info.addr = 0x38;
			strlcpy(info.type, "ft5x0x_ts", I2C_NAME_SIZE);

			adapter = i2c_get_adapter(item_integer("ts.ctrl", 1));
			if (!adapter) {
			printk("****** get_adapter error! ******\n");
			}
			client = i2c_new_device(adapter, &info);

			return i2c_add_driver(&fts_ts_driver);
		}
		else
			printk(KERN_ERR "%s: touchscreen is not ft5x0x\n", __func__);
	}
	else
		printk(KERN_ERR "%s: touchscreen is not exist\n", __func__);

	return -1;
}

static void __exit fts_ts_exit(void)
{
	DBG_FUNC("Enter %s\n", __func__);
	i2c_del_driver(&fts_ts_driver);
}

late_initcall(fts_ts_init);
module_exit(fts_ts_exit);

MODULE_AUTHOR("<duxx@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
