/*
 * 
 * Copyright (C) 2011 Goodix, Inc.
 * 
 * Author: Scott
 * Date: 2011.11.08
 */

#ifndef _LINUX_GOODIX_TOUCH_H
#define _LINUX_GOODIX_TOUCH_H

#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/fs.h>

//***************************PART1:ON/OFF define*******************************
#define GTP_DEBUG_ON          1
#define GTP_DEBUG_ARRAY_ON    0
#define GTP_DEBUG_FUNC_ON     0
#define GTP_CUSTOM_CFG        0
#define GTP_DRIVER_SEND_CFG   1 
#define GTP_HAVE_TOUCH_KEY    0
#define GTP_CHANGE_X2Y        0

//***************************PART2:TODO define**********************************
//STEP_1(REQUIRED):Change config table.
/*TODO: puts the config info corresponded to your TP here, the following is just 
a sample config, send this config should cause the chip cannot work normally*/
#define CTP_CFG_GROUP1 {\
    0x65,0x03,0x04,0x00,0x03,0x00,0x0A,0x00,0x1E,0xE7,0x32,0x02,0x08,0x10,0x4C,0x4F,\
    0x4F,0x20,0x07,0x00,0x80,0x80,0x3C,0x64,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,0x07,\
    0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x1D,0x1C,0x1B,0x1A,0x19,0x18,0x17,0x16,0x15,\
    0x14,0x13,0x12,0x11,0x10,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20\
  }

/*
#define CTP_CFG_GROUP1 {\
    0x65,0x02,0x04,0x00,0x03,0x00,0x0A,0x22,0x1E,0xE7,0x32,0x05,0x08,0x10,0x4C,0x41,\
    0x42,0x20,0x00,0x09,0x60,0x60,0x32,0x46,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,0x07,\
    0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x1D,0x1C,0x1B,0x1A,0x19,0x18,0x17,0x16,0x15,\
    0x14,0x13,0x12,0x11,0x10,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00\
  }
*/
/*
#define CTP_CFG_GROUP1 {\
    0x65,0x00,0x04,0x00,0x03,0x00,0x05,0x61,0x01,0x00,0x0F,0x20,0x02,0x08,0x10,0x00,\
    0x00,0x23,0x00,0x00,0x10,0x10,0x10,0x11,0x37,0x00,0x00,0x0B,0x0A,0x09,0x08,0x07,\
    0x06,0x05,0x04,0x03,0x02,0x01,0x00,0xFF,0xFF,0xFF,0xFF,0x0B,0x0A,0x09,0x08,0x07,\
    0x06,0x05,0x04,0x03,0x02,0x01,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x3C,0x64,0x50,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20\
  }
*/
//TODO puts your group2 config info here,if need.
#define CTP_CFG_GROUP2 {\
  }
//TODO puts your group3 config info here,if need.
#define CTP_CFG_GROUP3 {\
  }
//TODO puts your group4 config info here,if need.
#define CTP_CFG_GROUP4 {\
  }

//STEP_2(REQUIRED):Change I/O define & I/O operation mode.
#define GTP_RST_PORT  S3C64XX_GPL(10)
#define GTP_INT_PORT  S3C64XX_GPN(15)
#define GTP_INT_IRQ   gpio_to_irq(GTP_INT_PORT)
#define GTP_INT_CFG   S3C_GPIO_SFN(2)

#define GTP_GPIO_AS_INPUT(pin)          do{\
                                            gpio_direction_input(pin);\
                                            s3c_gpio_setpull(pin, S3C_GPIO_PULL_NONE);\
                                        }while(0)
#define GTP_GPIO_AS_INT(pin)            do{\
                                            gpio_direction_input(pin);\
                                            s3c_gpio_setpull(pin, S3C_GPIO_PULL_NONE);\
                                            s3c_gpio_cfgpin(pin, GTP_INT_CFG);\
                                        }while(0)
#define GTP_GPIO_GET_VALUE(pin)         gpio_get_value(pin)
#define GTP_GPIO_OUTPUT(pin,level)      gpio_direction_output(pin,level)
#define GTP_GPIO_REQUEST(pin, label)    gpio_request(pin, label)
#define GTP_GPIO_FREE(pin)              gpio_free(pin)
#define GTP_GPIO_READ(pin)              gpio_get_value(pin)
#define GTP_IRQ_TAB                     {IRQ_TYPE_EDGE_RISING,IRQ_TYPE_EDGE_FALLING,\
                                        IRQ_TYPE_LEVEL_LOW,IRQ_TYPE_LEVEL_HIGH}

//STEP_3(optional):Custom set some config by themself,if need.
#if GTP_CUSTOM_CFG
  #define GTP_MAX_WIDTH     800			
  #define GTP_MAX_HEIGHT    480
  #define GTP_MAX_TOUCH     5
  #define GTP_INT_TRIGGER   1
#else
  #define GTP_MAX_WIDTH     4096			
  #define GTP_MAX_HEIGHT    4096
  #define GTP_MAX_TOUCH     10
  #define GTP_INT_TRIGGER   1
#endif

//STEP_4(optional):if this project have touch key,Set touch key config.                                    
#if GTP_HAVE_TOUCH_KEY
    #define GTP_READ_COOR_ADDR 0x00
    #define GTP_KEY_TAB {KEY_MENU, KEY_HOME, KEY_SEND}
#else
    #define GTP_READ_COOR_ADDR 0x01
#endif


//***************************PART3:OTHER define*********************************
#define GTP_DRIVER_VERSION    "V1.0<2012/05/01>"
#define GTP_I2C_NAME          "Goodix-TS"
#define GTP_POLL_TIME		      10
#define GTP_ADDR_LENGTH       1
#define FAIL                  0
#define SUCCESS               1
#define GT801PLUS             1
#define GT81XPLUS             0
//Register define
#define GTP_REG_SLEEP         0x50
#define GTP_REG_SENSOR_ID     0x52
#define GTP_REG_XMAX_H        0x66
#define GTP_REG_I2CDELAY      0x6E
#define GTP_REG_VERSION       0xF0
//Log define
#define GTP_INFO(fmt,arg...)           printk("<<-GTP-INFO->> "fmt"\n",##arg)
#define GTP_ERROR(fmt,arg...)          printk("<<-GTP-ERROR->> "fmt"\n",##arg)
#define GTP_DEBUG(fmt,arg...)          do{\
                                         if(GTP_DEBUG_ON)\
                                         printk("<<-GTP-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define GTP_DEBUG_ARRAY(array, num)    do{\
                                         s32 i;\
                                         u8* a = array;\
                                         if(GTP_DEBUG_ARRAY_ON)\
                                         {\
                                            printk("<<-GTP-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printk("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printk("\n");\
                                                }\
                                            }\
                                            printk("\n");\
                                        }\
                                       }while(0)
#define GTP_DEBUG_FUNC()               do{\
                                         if(GTP_DEBUG_FUNC_ON)\
                                         printk("<<-GTP-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)
#define GTP_SWAP(x, y)                 do{\
                                         typeof(x) z = x;\
                                         x = y;\
                                         y = z;\
                                       }while (0)


#define fail    0
#define success 1

#define false   0
#define true    1

#if 1
#define DEBUG_MSG(fmt, arg...) printk("<--GT msg-->"fmt, ##arg)
#else
#define DEBUG_MSG(fmt, arg...)
#endif

#if 1
#define DEBUG_UPDATE(fmt, arg...) printk("<--GT update-->"fmt, ##arg)
#else
#define DEBUG_UPDATE(fmt, arg...)
#endif 

#if 1
#define DEBUG_COOR(fmt, arg...) printk(fmt, ##arg)
#else
#define DEBUG_COOR(fmt, arg...)
#define DEBUG_COORD
#endif

#if 0
#define DEBUG_ARRAY(array, num)   do{\
                                   int i; \
                                   for (i = 0; i < (num); i++)\
                                   {\
                                       printk("%02x   ", (array)[i]);\
                                       if ((i + 1 ) %10 == 0)\
                                       {\
                                           printk("\n");\
                                       }\
                                   }\
                                   printk("\n");\
                                  }while(0)
#else
#define DEBUG_ARRAY(array, num) 
#endif 


//-------------------------GPIO REDEFINE START----------------------------//
#define GPIO_DIRECTION_INPUT(port)          gpio_direction_input(port)
#define GPIO_DIRECTION_OUTPUT(port, val)    gpio_direction_output(port, val)
#define GPIO_SET_VALUE(port, val)           gpio_set_value(port, val)
#define GPIO_FREE(port)                     gpio_free(port)
#define GPIO_REQUEST(port, name)            gpio_request(port, name)
#define GPIO_PULL_UPDOWN(port, val)         s3c_gpio_setpull(port, val)
#define GPIO_CFG_PIN(port, cfg)             s3c_gpio_cfgpin(port, cfg)
//-------------------------GPIO REDEFINE END------------------------------//


#define FW_HEAD_LENGTH   30
#define FILE_HEAD_LENGTH 100
#define IGNORE_LENGTH    100
#define FW_MSG_LENGTH    7
#define ADDR_LENGTH      2
#define UPDATE_DATA_LENGTH  5000

#define READ_TOUCH_ADDR_H   0x0F
#define READ_TOUCH_ADDR_L   0x40
#define READ_KEY_ADDR_H     0x0F
#define READ_KEY_ADDR_L     0x41
#define READ_COOR_ADDR_H    0x0F
#define READ_COOR_ADDR_L    0x42
#define READ_ID_ADDR_H      0x00
#define READ_ID_ADDR_L      0xff
#define READ_FW_MSG_ADDR_H    0x0F
#define READ_FW_MSG_ADDR_L    0x7C
#define UPDATE_FW_MSG_ADDR_H  0x40
#define UPDATE_FW_MSG_ADDR_L  0x50
#define READ_MSK_VER_ADDR_H   0xC0
#define READ_MSK_VER_ADDR_L   0x09

//*************************TouchScreen Work Part*****************************
#define GOODIX_I2C_NAME "Goodix-TS"

#if 0
#define TOUCH_MAX_HEIGHT_GT801     7680
#define TOUCH_MAX_WIDTH_GT801      5120
#else
#define RESOLUTION_LOC      71
u16 TOUCH_MAX_HEIGHT_GT801;
u16 TOUCH_MAX_WIDTH_GT801;
#endif

#define RESET_PORT      S3C64XX_GPF(3)         //RESET管脚号
#define INT_PORT        S3C64XX_GPL(10)         //Int IO port
#ifdef INT_PORT
    #define TS_INT         gpio_to_irq(INT_PORT)      //Interrupt Number,EINT18(119)
    #define INT_CFG        S3C_GPIO_SFN(3)            //IO configer as EINT
#else
    #define TS_INT    0
#endif

#define INT_TRIGGER     IRQ_TYPE_EDGE_FALLING       // 1=rising 0=falling
#define POLL_TIME       10        //actual query spacing interval:POLL_TIME+6

#define GOODIX_MULTI_TOUCH
#ifdef GOODIX_MULTI_TOUCH
    #define MAX_FINGER_NUM    5
#else
    #define MAX_FINGER_NUM    1
#endif

#define swap(x, y) do { typeof(x) z = x; x = y; y = z; } while (0)

//****************************升级模块参数******************************************
//#define AUTO_UPDATE_GUITAR             //如果定义了则上电会自动判断是否需要升级
#ifdef AUTO_UPDATE_GUITAR
#define SEARCH_FILE_TIMES    100
#define UPDATE_FILE_PATH_2   "/data/goodix/_goodix_update_.bin"
#define UPDATE_FILE_PATH_1   "/sdcard/goodix/_goodix_update_.bin"
#endif

//#define CREATE_WR_NODE
#ifdef CREATE_WR_NODE
static int goodix_update_write(struct file *filp, const char __user *buff, unsigned long len, void *data);
static int goodix_update_read( char *page, char **start, off_t off, int count, int *eof, void *data );
#endif 

#define  PACK_SIZE              512                    //update file package size

#define  BIT_NVRAM_STROE        0
#define  BIT_NVRAM_RECALL       1
#define  BIT_NVRAM_LOCK         2
#define  REG_NVRCS_H            0X12
#define  REG_NVRCS_L            0X01

#pragma pack(1)
typedef struct 
{
    u8  type;          //产品类型//
    u16 version;       //FW版本号//
    u8  msk_ver[4];    //MASK版本//
    u8  st_addr[2];    //烧录的起始地址//
    u16 lenth;         //FW长度//
    u8  chk_sum[3];
    u8  force_update[6];//强制升级标志,为"GOODIX"则强制升级//
}st_fw_head;
#pragma pack()

//******************************************************************************
struct goodix_ts_data {
    uint8_t bad_data;
    uint8_t irq_flag;
    uint16_t addr;
    int use_reset;        //use RESET flag
    int use_irq;          //use EINT flag
    unsigned int version;
    int (*power)(struct goodix_ts_data * ts, int on);
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct hrtimer timer;
    struct work_struct work;
    struct early_suspend early_suspend;
    char phys[32];
  u16 abs_x_max;
  u16 abs_y_max;
  u8  max_touch_num;
  u8  int_trigger_type;
  u8  green_wake_mode;
  u8  chip_type;
  u8  enter_update;
 // u8  version[17];    
#ifdef AUTO_UPDATE_GUITAR
    uint8_t force_update;
    s32 gt_loc;
    st_fw_head  ic_fw_msg;
    struct file *file; 
    mm_segment_t old_fs;
#endif
};

//*************************Touchkey Surpport Part*****************************
#define HAVE_TOUCH_KEY
#ifdef HAVE_TOUCH_KEY
    const uint16_t touch_key_array_gt801[]={
                                      KEY_MENU,             //MENU
                                      KEY_BACK,             //BACK
                                      KEY_HOME,             //HOME
                                      KEY_SEARCH,           //SEARCH
                                     }; 
    #define MAX_KEY_NUM     (sizeof(touch_key_array_gt801)/sizeof(touch_key_array_gt801[0]))
#endif

struct goodix_i2c_rmi_platform_data {
    uint32_t version;    /* Use this entry for panels with */
    //reservation
};

#endif /* _LINUX_GOODIX_TOUCH_H */
