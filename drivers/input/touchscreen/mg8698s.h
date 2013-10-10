#ifndef _LINUX_MG_I2C_MTOUCH_H
#define _LINUX_MG_I2C_MTOUCH_H

//Set Capacity Range
#define CAP_X_CORD 	        1471//0x0380
#define CAP_Y_CORD 		959//0x0240

#define CAP_MAX_X		CAP_X_CORD
#define CAP_MAX_Y		CAP_Y_CORD

#define DIG_MAX_X		0x37ff
#define DIG_MAX_Y		0x1fff

//<-- LH_SWRD_Richard@2011.05.23 add the pin on configure
#define TOUCH_INT	14
#define TOUCH_RESET	157
#define TOUCH_LED	62
#define LED_ON		1
#define LED_OFF		0
//#define RESET_PIN_HIGH  1
//#define RESER_PIN_LOW	0
#define	light_touch_LED  gpio_direction_output
#define CMD_BUF_SIZE  	5
#define CMD_NUM		8
#define RCV_BUF_SIZE	8
typedef enum
{
	MG_CALIBRATION = 0X0,
	MG_TOUCH_ENABLE,
	MG_TOUCH_DISABLE,
	MG_DEVICE_ID,
	MG_DEVICE_VERSION,
	MG_SLEEP,
	MG_IDLE,
	MG_RESET,
};

#define	COMMAND_COUSE		15
#define	COMMAND_BIT		30

static u_int8_t command_list[COMMAND_COUSE][COMMAND_BIT] = 
{
	{0x81, 0x01, 0x00, 0x00, 0x00},//calibrate
	{0xDE, 0xEA, 0x00, 0x00, 0x00},//enable i2c interface
	{0xDE, 0xDA, 0x00, 0x00, 0x00},//disable i2c interface
	{0xDE, 0xDD, 0x00, 0x00, 0x00},//read device id
	{0xDE, 0xEE, 0x00, 0x00, 0x00},//read device version
	{0xDE, 0x55, 0x00, 0x00, 0x00},//sleep
	{0xDE, 0xAA, 0x00, 0x00, 0x00},//idle
	{0xDE, 0x5A, 0x00, 0x00, 0x00},// 7 wake up


	{0x43, 0x41, 0x4C, 0x95, 0x55},// 8 calibrate success
	{0xAB, 0x55, 0x66, 0x00, 0x00},// 9 Jump to Bootloader OK
	{0xbd, 0xaa, 0x00, 0x00, 0x00},// 10 Jump to Bootloader / Action 1
	{0xab, 0x01, 0x55, 0x00, 0x00},// 11 Action1 OK/ Action2 OK/ Action3 OK/
	{0xbd, 0x10, 0x00, 0x00, 0x00},// 12 Action 2
	{0xbd, 0x70, 0x00, 0x00, 0x00},	// 13 Ending 
	{0xbd, 0x51, 0x00, 0x00, 0x00}	// 14 Get Check Sum
};
		


//add the pin on configure -->

enum mg_capac_report {

	MG_MODE = 0x0,
	TOUCH_KEY_CODE,
	ACTUAL_TOUCH_POINTS,
	
	MG_CONTACT_ID,
	MG_STATUS,
	MG_POS_X_LOW,
	MG_POS_X_HI,
	MG_POS_Y_LOW,
	MG_POS_Y_HI,

	MG_CONTACT_ID2,
	MG_STATUS2,
	MG_POS_X_LOW2,
	MG_POS_X_HI2,
	MG_POS_Y_LOW2,
	MG_POS_Y_HI2,
	
	MG_CONTACT_IDS3,
	MG_POS_X_LOW3,
	MG_POS_X_HI3,
	MG_POS_Y_LOW3,
	MG_POS_Y_HI3,	
	
	MG_CONTACT_IDS4,
	MG_POS_X_LOW4,
	MG_POS_X_HI4,
	MG_POS_Y_LOW4,
	MG_POS_Y_HI4,	

	MG_CONTACT_IDS5,
	MG_POS_X_LOW5,
	MG_POS_X_HI5,
	MG_POS_Y_LOW5,
	MG_POS_Y_HI5,	

	MG_CONTACT_IDS6,
	MG_POS_X_LOW6,
	MG_POS_X_HI6,
	MG_POS_Y_LOW6,
	MG_POS_Y_HI6,	

	MG_CONTACT_IDS7,
	MG_POS_X_LOW7,
	MG_POS_X_HI7,
	MG_POS_Y_LOW7,
	MG_POS_Y_HI7,	

	MG_CONTACT_IDS8,
	MG_POS_X_LOW8,
	MG_POS_X_HI8,
	MG_POS_Y_LOW8,
	MG_POS_Y_HI8,	

	MG_CONTACT_IDS9,
	MG_POS_X_LOW9,
	MG_POS_X_HI9,
	MG_POS_Y_LOW9,
	MG_POS_Y_HI9,	

	MG_CONTACT_IDS10,
	MG_POS_X_LOW10,
	MG_POS_X_HI10,
	MG_POS_Y_LOW10,
	MG_POS_Y_HI10,	
};

enum mg_int_mode {

	MG_INT_PERIOD = 0x0,
	MG_INT_FMOV,
	MG_INT_FTOUCH,
};

enum mg_int_trig {

	MG_INT_TRIG_LOW = 0x0,
	MG_INT_TRIG_HI,
};


#endif 	/* _LINUX_MG_I2C_MTOUCH_H */



