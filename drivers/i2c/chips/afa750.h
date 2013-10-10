/*
 * AFA750 Three-Axis Digital Accelerometers (I2C/SPI Interface)
 *
 *
 * Copyright (C) 2012 Frances Chu, Afa Micro Corp.
 * Licensed under the GPL-2 or later.
 */

#ifndef _AFA750_H_
#define _AFA750_H_

struct device;
struct afa750;

struct afa750_bus_ops {
	u16 bustype;
	int (*read)(struct device *, unsigned char);
	int (*read_block)(struct device *, unsigned char, int, void *);
	int (*write)(struct device *, unsigned char, unsigned char);
};

void afa750_suspend(struct afa750 *ac);
void afa750_resume(struct afa750 *ac);
struct afa750 *afa750_probe(struct device *dev, int irq,
			      bool fifo_delay_default,
			      const struct afa750_bus_ops *bops);
int afa750_remove(struct afa750 *ac);

#endif

#ifndef __LINUX_INPUT_AFA750_H__
#define __LINUX_INPUT_AFA750_H__

struct afa750_platform_data {
    /*
	* int_enable:
	* functions enable
	*/
#define AFA_CNT_DATA_RDY  1
#define AFA_FIFO_EMPTY   (1 << 1)
#define AFA_FIFO_OVER    (1 << 2)
#define AFA_FIFO_FULL    (1 << 3)
#define AFA_FF_EN        (1 << 4)
#define AFA_MOTION_EN    (1 << 5)
#define AFA_TAP_EN       (1 << 6)
#define AFA_ORN_EN       (1 << 7) 
    
	u8 int_enable;
	
	/*
	 * X,Y,Z Axis Offset:
	 * offer user offset adjustments in twoscompliment
	 * form with a scale factor of 15.6 mg/LSB (i.e. 0x7F = +2 g)
	 */

	s8 x_axis_offset;
	s8 y_axis_offset;
	s8 z_axis_offset;


	/*
	 * tap_threshold:
	 * holds the threshold value for tap detection/interrupts.
	 */

	u8 tap_threshold;

	/*
	 * tap_min:
	 * defines the minimum duration a valid tapping should be. If the interval
     * from the beginning of tapping (where TAP_THRES is satisfied) to tap end id shorter
	 * than TAP_MIN, the tapping is not recognized.
	 * tap_max:
	 * defines the maximum duration a valid tapping should be. If the interval from
	 * the beginning of tapping to tap end is longer than TAP_MAX, the tapping is
	 * not recognized.
	 */

	u8 tap_min;
	u8 tap_max;

	/*
	 * tap_latency:
	 * it is used to distinguish the end of single tapping and the 
	 * beginning of double tapping. 
	 */

	u8 tap_latency;

	/*
	 * tap_dlatency:
	 * is an unsigned time value representing the amount
	 * of time after the expiration of tap_latency during which a second
	 * tap can begin.
	 */

	u8 tap_dlatency;
	
	/*
	 * tap_hysteresis:
	 * TAP_HYST works with TAP_THRE. The interval that two thresholds defines 
	 * a hysteresis area (TAP_THRE >= hysteresis area >= (TAP_THRE - TAP_HYST))
     * which is used to filter irregular spurious data that might impede motion detection.
	 */

	u8 tap_hysteresis;

	/*
	 * motion_threshold:
	 * Motion-Detect Threshold (The MSB 8 bits of valid CNT)
	 * The increment of MDT internal counter is trigger by following equation:
	 * (|£GX| + |£GY| + |£GZ|) >= MDT_THRE
	 * Once it¡¦s triggered, the threshold to be compared is changed to hysteresis, as:
	 * (|£GX| + |£GY| + |£GZ|) >= (MDT_THRE - MDT_HYST)
	 * If above equation is not satisfied, the MDT internal counter stop incrementing.
	 */

	u8 motion_threshold;

	/*
	 * motion_latency:
	 * MDT_LATY is used to compare with Free-Fall internal counter. 
	 * If following equation is satisfied:MDT internal counter = MDT_LATY
	 * , MDT_FLG interrupt generates and the counter stop incrementing.
	 * Note: Unit = an ODR update
	 */

	u8 motion_latency;

	/*
	 * motion_hysteresis:
	 * Motion-Detect Hysteresis Interval (The MSB 8 bits of valid CNT)
	 * MDT_HYST works with MDT_THRE. 
	 * The interval that two thresholds defines a hysteresis area 
	 * (MDT_THRE >= hysteresis area >= (MDT_THRE - MDT_HYST)) which
     * is used to filter irregular spurious data that might impede motion detection.
	 */

	u8 motion_hysteresis;

	/*
	 * free_fall_threshold:
	 * The increment of Free-Fall internal counter is trigger by following equation:
	 * (|X| + |Y| + |Z|) <= FALL_THRE
	 * Once it¡¦s triggered, the threshold to be compared is changed to hysteresis, as
	 * (|X| + |Y| + |Z|) <= (FALL_THRE + FALL_HYST)
	 * If above equation is not satisfied, the Free-Fall internal counter stop incrementing.
	 */

	u8 free_fall_threshold;

	/*
	 * free_fall_hysteresis:
	 * FALL_HYST works with FALL_THRE. 
	 * The interval that two thresholds defines a hysteresis area ((FALL_THRE + FALL_HYST) >= hysteresis area >= FALL_THRE)
     * which is used to filter irregular spurious data that might impede the detection of Free-Fall.
	 */

	u8 free_fall_hysteresis;

	
	
	/*
	 * data_rate:
	 * Selects device bandwidth and output data rate(Hz).
	 */

#define AFA_ODR_400     0x0
#define AFA_ODR_200     0x01
#define AFA_ODR_100     0x02
#define AFA_ODR_50      0x03
#define AFA_ODR_25      0x04
#define AFA_ODR_12p5    0x05  
#define AFA_ODR_6p256   0x06
#define AFA_ODR_3p128   0x07
#define AFA_ODR_1p564   0x08
#define AFA_ODR_0p782   0x09
#define AFA_ODR_0p391   0x0A

	u8 data_rate;


	/*
	 * power_mode:
	 * wakeup mode: Wake up device from low-power to normal 
	 *              mode while FALL_FLG or MDT_FLG is asset.
	 */

#define AFA_NORMAL 0
#define AFA_LOW_PWR	1
#define AFA_PWR_DOWN (1 << 1)
#define AFA_Wakeup (1 << 2)

	u8 power_mode;

	/*
	 * fifo_mode:
	 * BYPASS The FIFO is bypassed
	 * FIFO   FIFO collects up to 32 values then stops collecting data
	 * STREAM FIFO holds the last 32 data values. Once full, the FIFO's
	 *        oldest data is lost as it is replaced with newer data
	 *
	 * DEFAULT should be ADXL_FIFO_STREAM
	 */

#define AFA_FIFO_EN   1
#define AFA_FIFO_CLEAN	(1 << 1)
#define AFA_FIFO_BYPASS  (1 << 2)
#define AFA_FIFO_STREAM  (1 << 3)
#define AFA_FIFO_TRIGGER  AFA_FIFO_BYPASS | AFA_FIFO_STREAM
#define AFA_FIFO_INT1  (0 << 4) //INT1
#define AFA_FIFO_INT2  (1 << 4) //INT2
 
	u8 fifo_mode;

	/*
	 * When acceleration measurements are received from the ADXL34x
	 * events are sent to the event subsystem. The following settings
	 * select the event type and event code for new x, y and z axis data
	 * respectively.
	 */
	u32 ev_type;	/* EV_ABS or EV_REL */

	u32 ev_code_x;	/* ABS_X,Y,Z or REL_X,Y,Z */
	u32 ev_code_y;	/* ABS_X,Y,Z or REL_X,Y,Z */
	u32 ev_code_z;	/* ABS_X,Y,Z or REL_X,Y,Z */


	/*
	 * A valid BTN or KEY Code for Free-Fall
	 * input event reporting.
	 */

	u32 ev_code_ff;	/* EV_KEY */

	/*
	 * Use AFA INT2 pin instead of INT1 pin for interrupt output
	 */
	u8 use_int2;

	/*
	 * AFA750 only ORIENTATION SENSING feature
	 * The orientation function of the AFA750 reports both 4-D and
	 * 6-D orientation concurrently.
	 */

#define AFA_EN_ORIENTATION_4D		1
#define AFA_EN_ORIENTATION_6D		0
    u8 orientation_sel;

	/*
	 * orientation_threshold:
	 * The increment of orientation internal counter 
	 * of each axis is trigger by one of following equation:
	 * If the axis¡¦ CNT >=0, then
	 * (CNT) >= ORN_THRE
	 * else
	 * (CNT) <= - ORN_THRE
	 */


	u8 orientation_threshold;

	/*
	 * wma_control:
	 * weighted moving average, data_num = 2^wma_num
	 */

#define AFA_WMA_CTL_0	0
#define AFA_WMA_CTL_1	1
#define AFA_WMA_CTL_2	2
#define AFA_WMA_CTL_3	3
#define AFA_WMA_CTL_4	4
#define AFA_WMA_CTL_5	5
#define AFA_WMA_CTL_6	6
#define AFA_WMA_CTL_7	7
#define AFA_WMA_CTL_8	8
#define AFA_WMA_CTL_9	9
#define AFA_WMA_CTL_10	10
#define AFA_WMA_CTL_11	11
#define AFA_WMA_CTL_12	12
#define AFA_WMA_CTL_13	13
#define AFA_WMA_CTL_14	14
#define AFA_WMA_CTL_15	15

	u8 wma_control;

	u32 ev_codes_orient_4d[4];	/* EV_KEY {+X, -X, +Y, -Y} */
	u32 ev_codes_orient_6d[6];	/* EV_KEY {+Z, +Y, +X, -X, -Y, -Z} */
};
#endif

