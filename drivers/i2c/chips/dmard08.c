/*
 * @file drivers/i2c/dmard08.c
 * @brief DMARD08 g-sensor Linux device driver
 * @author Domintech Technology Co., Ltd (http://www.domintech.com.tw)
 * @version 1.22
 * @date 2011/12/01
 *
 * @section LICENSE
 *
 *  Copyright 2011 Domintech Technology Co., Ltd
 *
 * 	This software is licensed under the terms of the GNU General Public
 * 	License version 2, as published by the Free Software Foundation, and
 * 	may be copied, distributed, and modified under those terms.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 *
 */
#include "dmard08.h"

#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/earlysuspend.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/serio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
//#include <linux/gpio.h>
#include <mach/items.h>
#include <linux/platform_device.h>
// ****Add by Steve Huang*********2011-11-18********
#include <linux/syscalls.h>
void gsensor_write_offset_to_file(void);
void gsensor_read_offset_from_file(void);
char OffsetFileName[] = "/data/misc/dmt/offset.txt";
static int Device_First_Time_Opened_flag=1;
//**************************************************

static char const *const ACCELEMETER_CLASS_NAME = "accelemeter";
static char const *const GSENSOR_DEVICE_NAME = "dmard08";

static int device_init(void);
static void device_exit(void);

static int device_open(struct inode*, struct file*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int device_close(struct inode*, struct file*);

static int device_i2c_suspend(struct i2c_client *client, pm_message_t mesg);
static int device_i2c_resume(struct i2c_client *client);
static int __devinit device_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit device_i2c_remove(struct i2c_client *client);
static inline void device_i2c_correct_accel_sign(s16 *val);
void device_i2c_read_xyz(struct i2c_client *client, s16 *xyz);
void device_i2c_merge_register_values(struct i2c_client *client, s16 *val, u8 msb, u8 lsb);

typedef union {
	struct {
		s16	x;
		s16	y;
		s16	z;
	} u;
	s16	v[SENSOR_DATA_SIZE];
} raw_data;
static raw_data offset;

struct dev_data 
{
	dev_t devno;
	struct cdev cdev;
  	struct class *class;
	struct i2c_client *client;
};
static struct dev_data dev;

#define CONFIG_GSEN_LAYOUT_PAT_7 1

s16 sensorlayout[3][3] = {
#if defined(CONFIG_GSEN_LAYOUT_PAT_1)
    { 1, 0, 0},	{ 0, 1,	0}, { 0, 0, 1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_2)
    { 0, 1, 0}, {-1, 0,	0}, { 0, 0, 1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_3)
    {-1, 0, 0},	{ 0,-1,	0}, { 0, 0, 1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_4)
    { 0,-1, 0},	{ 1, 0,	0}, { 0, 0, 1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_5)
    {-1, 0, 0},	{ 0, 1,	0}, { 0, 0,-1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_6)
    { 0,-1, 0}, {-1, 0,	0}, { 0, 0,-1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_7)
    { 1, 0, 0},	{ 0,-1,	0}, { 0, 0,-1},
#elif defined(CONFIG_GSEN_LAYOUT_PAT_8)
    { 0, 1, 0},	{ 1, 0,	0}, { 0, 0,-1},
#endif
};

void gsensor_read_accel_avg(int num_avg, raw_data *avg_p )
{
   	long xyz_acc[SENSOR_DATA_SIZE];   
  	s16 xyz[SENSOR_DATA_SIZE];
  	int i, j;
	
	//initialize the accumulation buffer
  	for(i = 0; i < SENSOR_DATA_SIZE; ++i) 
		xyz_acc[i] = 0;

	for(i = 0; i < num_avg; i++) 
	{      
		device_i2c_read_xyz(dev.client, (s16 *)&xyz);
		for(j = 0; j < SENSOR_DATA_SIZE; j++) 
			xyz_acc[j] += xyz[j];
  	}

	// calculate averages
  	for(i = 0; i < SENSOR_DATA_SIZE; i++) 
		avg_p->v[i] = (s16) (xyz_acc[i] / num_avg);
}
/* calc delta offset */
int gsensor_calculate_offset(int gAxis,raw_data avg)
{
	char calibrate[ITEM_MAX_LEN];
	item_string(calibrate, "sensor.grivaty.orientation", 0);
	switch(gAxis)
	{
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_Z_NEGATIVE:  
	   		offset.u.x =  avg.u.x ;    
	   		offset.u.y =  avg.u.y ;
		if (calibrate[2] == 'Z'){
	   		offset.u.z =  avg.u.z + DEFAULT_SENSITIVITY;
		}else if (calibrate[2] == 'z'){
			offset.u.z =  avg.u.z - DEFAULT_SENSITIVITY;
		}
	    		break;
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_X_POSITIVE:  
			offset.u.x =  avg.u.x + DEFAULT_SENSITIVITY;    
			offset.u.y =  avg.u.y ;
			offset.u.z =  avg.u.z ;
		 	break;
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_Z_POSITIVE:  
			offset.u.x =  avg.u.x ;    
			offset.u.y =  avg.u.y ;
		if (calibrate[2] == 'Z'){
			offset.u.z =  avg.u.z - DEFAULT_SENSITIVITY;
		}else if (calibrate[2] == 'z'){
			offset.u.z =  avg.u.z + DEFAULT_SENSITIVITY;
		}
			break;
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_X_NEGATIVE:  
			offset.u.x =  avg.u.x - DEFAULT_SENSITIVITY;    
			offset.u.y =  avg.u.y ;
			offset.u.z =  avg.u.z ;
		 	break;
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_Y_NEGATIVE:
			offset.u.x =  avg.u.x ;    
			offset.u.y =  avg.u.y + DEFAULT_SENSITIVITY;
			offset.u.z =  avg.u.z ;
		 	break;
		case CONFIG_GSEN_CALIBRATION_GRAVITY_ON_Y_POSITIVE: 
			offset.u.x =  avg.u.x ;    
			offset.u.y =  avg.u.y - DEFAULT_SENSITIVITY;
			offset.u.z =  avg.u.z ;
		 	break;
		default:  
			return -ENOTTY;
	}
	return 0;
}

void gsensor_calibrate(int side)
{	
	raw_data avg;
	int avg_num = 16;
	
	//IN_FUNC_MSG;
	// get acceleration average reading
	gsensor_read_accel_avg(avg_num, &avg);
	// calculate and set the offset
	gsensor_calculate_offset(side, avg); 
}

void ce_on(void)
{
#if 0
	int gppdat;
	gppdat = __raw_readl(S3C64XX_GPPDAT);
	gppdat |= (1 << 0);

	__raw_writel(gppdat,S3C64XX_GPPDAT);
#endif
}

void ce_off(void)
{
#if 0
	int gppdat;
	gppdat = __raw_readl(S3C64XX_GPPDAT);
	gppdat &= ~(1 << 0);

	__raw_writel(gppdat,S3C64XX_GPPDAT);
#endif
}
 
void config_ce_pin(void)
{
#if 0
	unsigned int value;
	//D08's CE (pin#12) is connected to S3C64XX AP processor's port P0
	//Below codes set port P0 as digital output 
	value = readl(S3C64XX_GPPCON);
	value &= ~ (0x3);
	value |= 1 ;  //Output =01 , Input = 00 , Ext. Interrupt = 10
	writel(value, S3C64XX_GPPCON);  //save S3C64XX_GPPCON change
#endif
}

void gsensor_reset(void)
{
	ce_off();
	msleep(300); 
	ce_on();
}

void gsensor_set_offset(int val[3])
{
	int i;
	IN_FUNC_MSG;
	for(i = 0; i < SENSOR_DATA_SIZE; ++i)
		offset.v[i] = (s16) val[i];
}

struct file_operations dmt_g_sensor_fops = 
{
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_close,
};

static const struct i2c_device_id device_i2c_ids[] = 
{
	{DEVICE_I2C_NAME, 0},
	{}   
};

MODULE_DEVICE_TABLE(i2c, device_i2c_ids);

static struct i2c_driver device_i2c_driver = 
{
	.driver	= {
		.owner = THIS_MODULE,
		.name = DEVICE_I2C_NAME,
		},
	.class = I2C_CLASS_HWMON,
	.probe = device_i2c_probe,
	.remove	= __devexit_p(device_i2c_remove),
	.suspend = device_i2c_suspend,
	.resume	= device_i2c_resume,
	.id_table = device_i2c_ids,
};



/*static struct miscdevice dmard08_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "dmard08",
	.fops = &dmt_g_sensor_fops,
};*/

static int device_open(struct inode *inode, struct file *filp)
{
	IN_FUNC_MSG;
	//Add by Steve Huang 2011-11-30
	if(Device_First_Time_Opened_flag)
	{
		Device_First_Time_Opened_flag=0;
		gsensor_read_offset_from_file();
	}
	return 0; 
}

static ssize_t device_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	IN_FUNC_MSG;
	
	return 0;
}

static ssize_t device_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{

	s16 xyz[SENSOR_DATA_SIZE];
	int i;
	IN_FUNC_MSG;
	device_i2c_read_xyz(dev.client, (s16 *)&xyz);
	//offset compensation 
	for(i = 0; i < SENSOR_DATA_SIZE; ++i)
			xyz[i] -= offset.v[i];
	
	if(copy_to_user(buf, &xyz, count)) 
		return -EFAULT;
	PRINT_X_Y_Z(xyz[0], xyz[1], xyz[2]);
	return count;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0, ret = 0, i;
	int intBuf[SENSOR_DATA_SIZE];
	s16 xyz[SENSOR_DATA_SIZE];
		
	//check type and number
	if (_IOC_TYPE(cmd) != IOCTL_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > SENSOR_MAXNR) return -ENOTTY;

	//check user space pointer is valid
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
	
	switch(cmd) 
	{
		case SENSOR_RESET:
			gsensor_reset();
			printk("RUN RESET");
			return ret;

		case SENSOR_CALIBRATION:
			//get orientation info
			if(copy_from_user(&intBuf, (int*)arg, sizeof(int))) return -EFAULT;
			gsensor_calibrate(intBuf[0]);
			// save file 2011-11-30	
			gsensor_write_offset_to_file();
			
			//return the offset
			for(i = 0; i < SENSOR_DATA_SIZE; ++i)
				intBuf[i] = offset.v[i];

			ret = copy_to_user((int *)arg, &intBuf, sizeof(intBuf));
			return ret;
		
		case SENSOR_GET_OFFSET:
			// get data from file 2011-11-30
			gsensor_read_offset_from_file();
			
			for(i = 0; i < SENSOR_DATA_SIZE; ++i)
				intBuf[i] = offset.v[i];

			ret = copy_to_user((int *)arg, &intBuf, sizeof(intBuf));
			return ret;

		case SENSOR_SET_OFFSET:
			ret = copy_from_user(&intBuf, (int *)arg, sizeof(intBuf));
			gsensor_set_offset(intBuf);
			// write in to file 2011-11-30
			gsensor_write_offset_to_file();
			return ret;
		
		case SENSOR_READ_ACCEL_XYZ:
			device_i2c_read_xyz(dev.client, (s16 *)&xyz);
			for(i = 0; i < SENSOR_DATA_SIZE; ++i)
				intBuf[i] = xyz[i] - offset.v[i];
			
		  	ret = copy_to_user((int*)arg, &intBuf, sizeof(intBuf));
			return ret;
		
		default:  /* redundant, as cmd was checked against MAXNR */
			return -ENOTTY;
	}
	
	return 0;
}
	
static int device_close(struct inode *inode, struct file *filp)
{
	IN_FUNC_MSG;
	
	return 0;
}

static int device_i2c_xyz_read_reg(struct i2c_client *client,u8 *buffer, int length)
{
	
	struct i2c_msg msg[] = 
	{
		{.addr = client->addr, .flags = 0, .len = 1, .buf = buffer,}, 
		{.addr = client->addr, .flags = I2C_M_RD, .len = length, .buf = buffer,},
	};
	IN_FUNC_MSG;
	return i2c_transfer(client->adapter, msg, 2);
}

void device_i2c_read_xyz(struct i2c_client *client, s16 *xyz_p)
{
		
	u8 buffer[6];
	s16 xyzTmp[SENSOR_DATA_SIZE];
	int i, j;
	IN_FUNC_MSG;   

	//get xyz high/low bytes, 0x02~0x07
	buffer[0] = 2;
	device_i2c_xyz_read_reg(client, buffer, 6);
   
	//merge to 11-bits value
	for(i = 0; i < SENSOR_DATA_SIZE; ++i){
		device_i2c_merge_register_values(client, (xyzTmp + i), buffer[2*i], buffer[2*i + 1]);
	}
	//transfer to the default layout
	for(i = 0; i < SENSOR_DATA_SIZE; ++i)
	{
		xyz_p[i] = 0;
		for(j = 0; j < 3; j++){
		     xyz_p[i] += sensorlayout[i][j] * xyzTmp[j];
		}
	}
}

void device_i2c_merge_register_values(struct i2c_client *client, s16 *val, u8 msb, u8 lsb)
{
	IN_FUNC_MSG;
	
	*val = (((u16)msb) << 3) | (u16)lsb;
	device_i2c_correct_accel_sign(val);
}

static inline void device_i2c_correct_accel_sign(s16 *val)
{
	IN_FUNC_MSG;
	
	*val<<= (sizeof(s16) * BITS_PER_BYTE - 11);  
	*val>>= (sizeof(s16) * BITS_PER_BYTE - 11);  
}

static int device_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	IN_FUNC_MSG;
	
	return 0;
}

static int __devinit device_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	char cAddress = 0 , cData = 0;
	int i,j;
	//int err = 0;
	int ret = 0;

	for(i = 0; i < SENSOR_DATA_SIZE; ++i)
		offset.v[i] = 0;

	IN_FUNC_MSG;
	
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
  		printk("%s, I2C_FUNC_I2C not support\n", __func__);
    		return -1;
  	}
	config_ce_pin();
	gsensor_reset();
//------------------------------ Before register, test if it is DMARD08 is needed ---------------------
	cAddress = 0x08;
	i2c_master_send( client, (char*)&cAddress, 1);
	i2c_master_recv( client, (char*)&cData, 1);
	if( cData == 0x00)
	{
		cAddress = 0x09;
		i2c_master_send( client, (char*)&cAddress, 1);
		i2c_master_recv( client, (char*)&cData, 1);	
		if( cData == 0x00){
			cAddress = 0x0a;
			i2c_master_send( client, (char*)&cAddress, 1);
			i2c_master_recv( client, (char*)&cData, 1);	
			if( cData == 0x88){
				cAddress = 0x0b;
				i2c_master_send( client, (char*)&cAddress, 1);
				i2c_master_recv( client, (char*)&cData, 1);	
				if( cData == 0x08){
					printk(KERN_INFO "@@@ %s DMT_DEVICE_NAME registered I2C driver!\n",__FUNCTION__);
					dev.client = client;
				}else{
					printk(KERN_INFO "err : i2c Read 0x0B = %d!\n",cData);
					dev.client = NULL;
					return -1;
				}
			}else{
				dev.client = NULL;
				return -1;
			}
		}else{
			printk(KERN_INFO "err : i2c Read 0x09 = %d!\n",cData);
			dev.client = NULL;
			return -1;
		}
	}else{
		printk(KERN_INFO "err : i2c Read 0x08 = %d!\n",cData);
		dev.client = NULL;
		return -1;
	}
	//check sensorlayout[i][j]
	for(i = 0; i < 3; ++i)
	{
		for(j = 0; j < 3; j++)
			printk("%d",sensorlayout[i][j]);
		printk("\n");
	}
//------------------------------------------------------------------------------------------
	ret = alloc_chrdev_region(&dev.devno, 0, 1, GSENSOR_DEVICE_NAME);
  	if(ret)
	{
    		printk("%s, can't allocate chrdev\n", __func__);
		return ret;
	}

	
	cdev_init(&dev.cdev, &dmt_g_sensor_fops);  
	dev.cdev.owner = THIS_MODULE;
  	ret = cdev_add(&dev.cdev, dev.devno, 1);
  	if(ret < 0)
	{
    		printk("%s, add character device error, ret %d\n", __func__, ret);
		return ret;
	}
	dev.class = class_create(THIS_MODULE, ACCELEMETER_CLASS_NAME);
	if(IS_ERR(dev.class))
	{
   		printk("%s, create class, error\n", __func__);
		return ret;
  	}
  	device_create(dev.class, NULL, dev.devno, NULL, GSENSOR_DEVICE_NAME);
//---------------end---------------------

	printk("DMARD08 probe  end************\n");
	return 0;
}

static int __devexit device_i2c_remove(struct i2c_client *client)
{
	IN_FUNC_MSG;
	
	return 0;
}

static int device_i2c_resume(struct i2c_client *client)
{
	IN_FUNC_MSG;
	
	return 0;
}


static int __init device_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;

	IN_FUNC_MSG;
	printk("Enter dmard08 init \n");
	if(item_exist("sensor.grivaty") && item_equal("sensor.grivaty.model", "dmard08", 0))
	{
		memset(&info, 0, sizeof(struct i2c_board_info));
		info.addr = 0x1c;  
		strlcpy(info.type, "dmard08", I2C_NAME_SIZE);
		//adapter = i2c_get_adapter(CONFIG_ACC_DMARD08_I2C + 1);
		adapter = i2c_get_adapter(item_integer("sensor.grivaty.ctrl", 1));
		if (!adapter) {
			printk("*******get_adapter error!\n");
		}
		dev.client = i2c_new_device(adapter, &info);
	
		return i2c_add_driver(&device_i2c_driver);

	}
	else
	{
		printk("%s: G-sensor is not dmard08 or exist.\n", __func__);
		return -1;
	}
}





#if 0
static int __init device_init(void)
{

	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	//struct i2c_client *client;

	printk(KERN_INFO "---------------------------------------@@@ %s In\n",__FUNCTION__);

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = 0x1c;
	strlcpy(info.type, "dmard08", I2C_NAME_SIZE);

	adapter = i2c_get_adapter(2);
	if (!adapter) 
	{
		printk(KERN_INFO "-------------------------------------@@@ %s Get Adapter Error\n",__FUNCTION__);
	}
	
	dev.client = i2c_new_device(adapter, &info);

	if (!(dev.client))
	{
		printk(KERN_INFO "@@@ %s can't add i2c device at 0x%x\n",__FUNCTION__,(unsigned int)info.addr);
	}
	
	i2c_put_adapter(adapter);
    
	i2c_add_driver(&device_i2c_driver);

	printk(KERN_INFO "---------------------------------------@@@ %s Out\n",__FUNCTION__);
	return 0;
}
#endif


static void __exit device_exit(void)
{
	IN_FUNC_MSG;
	
	cdev_del(&dev.cdev);
	unregister_chrdev_region(dev.devno, 1);
	device_destroy(dev.class, dev.devno);
	class_destroy(dev.class);
	i2c_del_driver(&device_i2c_driver);
}

//*********************************************************************************************************
// 2011-11-30
// Add by Steve Huang 
// function definition
void gsensor_write_offset_to_file(void)
{
	char data[18];
	//int orgfsuid, orgfsgid;
	unsigned int orgfs;
	long lfile=-1;

	sprintf(data,"%5d %5d %5d",offset.u.x,offset.u.y,offset.u.z);

	//orgfsuid = current->fsuid;
	//orgfsgid = current->fsgid;
	//current->fsuid = current->fsgid = 0;
	orgfs = get_fs();
// Set segment descriptor associated to kernel space
	set_fs(KERNEL_DS);
	//lock_kernel();
	
        lfile=sys_open(OffsetFileName,O_WRONLY|O_CREAT, 0777);
	if (lfile < 0)
	{
 	 printk("sys_open %s error!!. %ld\n",OffsetFileName,lfile);
	}
	else
	{
   	 sys_write(lfile, data,18);
	 sys_close(lfile);
	}
	//unlock_kernel();
	set_fs(orgfs);
	//current->fsuid = orgfsuid;
	//current->fsgid = orgfsgid;

 return;
}

void gsensor_read_offset_from_file(void)
{
	//int orgfsuid, orgfsgid;
	unsigned int orgfs;
	char data[18];
	long lfile=-1;
	//orgfsuid = current->fsuid;
	//orgfsgid = current->fsgid;
	//current->fsuid = current->fsgid = 0;
	orgfs = get_fs();
// Set segment descriptor associated to kernel space
	set_fs(KERNEL_DS);
	//lock_kernel();

	lfile=sys_open(OffsetFileName, O_RDONLY, 0);
	if (lfile < 0)
	{
 	 printk("sys_open %s error!!. %ld\n",OffsetFileName,lfile);
	/* if(lfile == -2)
	 {
           lfile=sys_open(OffsetFileName,O_WRONLY|O_CREAT, 0777);
	   if(lfile >=0)
	   {
	    strcpy(data,"00000 00000 00000");
 	    printk("sys_open %s OK!!. %ld\n",OffsetFileName,lfile);
	    sys_write(lfile,data,18);
	    sys_read(lfile, data, 18);
	    sys_close(lfile);
	   }
	  else
 	   printk("sys_open %s error!!. %ld\n",OffsetFileName,lfile);
	 }  
	 */
	}
	else
	{
	 sys_read(lfile, data, 18);
	 sys_close(lfile);
	}
	sscanf(data,"%hd %hd %hd",&offset.u.x,&offset.u.y,&offset.u.z);
	//unlock_kernel();
	set_fs(orgfs);
	//current->fsuid = orgfsuid;
	//current->fsgid = orgfsgid;

}
//*********************************************************************************************************
MODULE_AUTHOR("DMT_RD");
MODULE_DESCRIPTION("DMARD08 g-sensor Driver");
MODULE_LICENSE("GPL");

module_init(device_init);
//late_initcall(device_init);
module_exit(device_exit);
