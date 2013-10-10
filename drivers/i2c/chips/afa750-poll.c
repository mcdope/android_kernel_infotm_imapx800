/*
 * AFA750 Three-Axis Digital Accelerometers
 *
 *
 * Copyright (C) 2012 Frances Chu, Afa Micro Corp.
 * Licensed under the GPL-2 or later.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
//#include <linux/input/afa750.h>
#include <mach/items.h>
#include <mach/pad.h>
#include <linux/module.h>
#include "afa750.h"

/* AFA750 Register Map */
#define THRESH_TAP	0x21	/* R/W Tap threshold */
#define TAP_MIN		0x24	/* R/W Tap min duration */
#define TAP_MAX		0x25	/* R/W Tap max duration */
#define LATENT		0x22	/* R/W Tap latency */
#define DLATENTCY	0x26	/* R/W DTap latency */
#define THRESH_ACT	0x1D	/* R/W Activity threshold */

#define THRESH_FF	0x1A	/* R/W Free-fall threshold */
#define FF_LATENCY	0x1B	/* R/W Free-fall time */
#define ACT_TAP_STATUS1	0x0D	/* R   Source of tap/double tap */
#define ACT_TAP_STATUS2 0x0E
#define DATA_RATE		0x05	/* R/W Data rate */
#define POWER_CTL	0x03	/* R/W Power saving features control :Frances*/
#define INT_ENABLE	0x09	/* R/W Interrupt enable control */
#define INT_MAP		0x0B	/* R/W Interrupt mapping control */
#define DATAX0		0x10	/* R   X-Axis Data 0 */
#define DATAX1		0x11	/* R   X-Axis Data 1 */
#define DATAY0		0x12	/* R   Y-Axis Data 0 */
#define DATAY1		0x13	/* R   Y-Axis Data 1 */
#define DATAZ0		0x14	/* R   Z-Axis Data 0 */
#define DATAZ1		0x15	/* R   Z-Axis Data 1 */
#define FIFO_CTL	0x04	/* R/W FIFO control */
#define FIFO_STATUS	0x0F	/* R   FIFO status */

#define WMA	0x07	 //Weighted moving average
#define NODR	0x05	 //output data rate



/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define CNT_DATA_RDY (1 << 0)
#define FIFO_EMPTY   (1 << 1)
#define FIFO_OVER    (1 << 2)
#define FIFO_FULL    (1 << 3)
#define FF_EN        (1 << 4)
#define MOTION_EN    (1 << 5)
#define TAP_EN       (1 << 6)
#define ORN_EN       (1 << 7) 

/* ACT_TAP_STATUS1 Bits */
#define FREE_FALL   (1 << 0)
#define MOTION      (1 << 1)
#define SINGLE_TAP  (1 << 2)
#define DOUBLE_TAP  (1 << 3)
#define ORIENTATION (1 << 4)

/* ACT_TAP_STATUS2 Bits */
#define CNT_RDY     (1 << 0)
#define FIFO_EMPTY  (1 << 1)
#define FIFO_OVER   (1 << 2)
#define FIFO_FULL   (1 << 3) 


/* DATA_RATE Bits */
#define ODR_400     0x0
#define ODR_200     0x01
#define ODR_100     0x02
#define ODR_50      0x03
#define ODR_25      0x04
#define ODR_12p5    0x05  
#define ODR_6p256   0x06
#define ODR_3p128   0x07
#define ODR_1p564   0x08
#define ODR_0p782   0x09
#define ODR_0p391   0x0A

#define RATE(x)		((x) & 0xF)

/* POWER_CTL Bits */
#define NORMAL 0
#define LOW_PWR	1
#define PWR_DOWN (1 << 1)
#define Wakeup (1 << 2)


/*
 * Maximum value our axis may get in full res mode for the input device
 * (signed 16 bits)
 */
#define AFA_FULLRES_MAX_VAL 32767 
#define AFA_FULLRES_MIN_VAL 32768 


/* FIFO_CTL Bits */
#define FIFO_EN   1
#define FIFO_CLEAN	(1 << 1)
#define FIFO_BYPASS  (1 << 2)
#define FIFO_STREAM  (1 << 3)
#define FIFO_TRIGGER  AFA_FIFO_BYPASS | AFA_FIFO_STREAM
#define FIFO_INT1  (0 << 4) //INT1
#define FIFO_INT2  (1 << 4) //INT2

/* FIFO_STATUS Bits */
#define ENTRIES(x)	((x) & 0xF0)

/* WMA value */
#define WMA_CTL_0	0
#define WMA_CTL_1	1
#define WMA_CTL_2	2
#define WMA_CTL_3	3
#define WMA_CTL_4	4
#define WMA_CTL_5	5
#define WMA_CTL_6	6
#define WMA_CTL_7	7
#define WMA_CTL_8	8
#define WMA_CTL_9	9
#define WMA_CTL_10	10
#define WMA_CTL_11	11
#define WMA_CTL_12	12
#define WMA_CTL_13	13
#define WMA_CTL_14	14
#define WMA_CTL_15	15


#undef ADXL_DEBUG

#define ADXL_X_AXIS			0
#define ADXL_Y_AXIS			1
#define ADXL_Z_AXIS			2

#define AC_READ(ac, reg)	((ac)->bops->read((ac)->dev, reg))
#define AC_WRITE(ac, reg, val)	((ac)->bops->write((ac)->dev, reg, val))

#define SIGN_POS        1
#define SIGN_NEG        -1
#define AFA750_MAX_DELAY 200
static int x_sign = SIGN_POS;
static int y_sign = SIGN_POS;
static int z_sign = SIGN_POS;
static int xy_swap = true;

static u8 AFA750_IRQ_CNT;

struct axis_triple {
	int x;
	int y;
	int z;
};

struct afa750 {
	struct device *dev;
	atomic_t delay;
	atomic_t enable;
	struct delayed_work work;
	struct input_dev *input;
	struct mutex mutex;	/* reentrant protection for struct */
	struct mutex enable_mutex;
	struct afa750_platform_data pdata;
	struct axis_triple swcal;
	struct axis_triple hwcal;
	struct axis_triple saved;
	char phys[32];
	unsigned orient2d_saved;
	unsigned orient3d_saved;
	bool disabled;	/* P: mutex */
	bool opened;	/* P: mutex */
	bool suspended;	/* P: mutex */
	bool fifo_delay;
	int irq;
	unsigned model;
	unsigned int_mask;

	const struct afa750_bus_ops *bops;
};

static int tsp_irq_index = 75; 
static const struct afa750_platform_data afa750_default_init = {
	//using device default value
};

static void afa750_get_triple(struct afa750 *ac, struct axis_triple *axis)
{
	short buf[3]={0,0,0};

	ac->bops->read_block(ac->dev, DATAX0, DATAZ1 - DATAX0 + 1, buf);
 
        //printk("Frances314:  %d, %d, %d\n", buf[0], buf[1], buf[2]);

	mutex_lock(&ac->mutex);
	ac->saved.x = (s16) le16_to_cpu(buf[0]);
	axis->x = ac->saved.x;

	ac->saved.y = (s16) le16_to_cpu(buf[1]);
	axis->y = ac->saved.y;

	ac->saved.z = (s16) le16_to_cpu(buf[2]);
	axis->z = ac->saved.z;
	mutex_unlock(&ac->mutex);
}

static void afa750_service_ev_fifo(struct afa750 *ac)
{
	struct afa750_platform_data *pdata = &ac->pdata;
	struct axis_triple axis;
	int x, y, z, tmp;

	afa750_get_triple(ac, &axis);
/* Frances
        x = x_sign * (axis.x - ac->swcal.x);
        y = y_sign * (axis.y - ac->swcal.y);
        z = z_sign * (axis.z - ac->swcal.z);
*/
        x = axis.x;
        y = axis.y;
        z = axis.z;

       // printk("Frances314:  %d, %d, %d\n", x, y, z);
        
        if (xy_swap)
        {
                tmp = x;
                x = y;
                y = tmp;
        }

        //input_event(ac->input, pdata->ev_type, pdata->ev_code_x, x/16);
       // input_event(ac->input, pdata->ev_type, pdata->ev_code_y, y/16);
       // input_event(ac->input, pdata->ev_type, pdata->ev_code_z, z/16);
	input_report_abs(ac->input, ABS_X, x/13);
	input_report_abs(ac->input, ABS_Y, y/13);
	input_report_abs(ac->input, ABS_Z, z/13);
}

static irqreturn_t afa750_irq(int irq, void *handle)
{
//	printk("***************** %s ************\n", __func__);
	struct afa750 *ac = handle;
	int err;

    	 AFA750_IRQ_CNT++;

	//	if (imapx_pad_get_indat(tsp_irq_index) == 0){
		if(AFA750_IRQ_CNT == 13){
        		 AFA750_IRQ_CNT = 0;
	    		 afa750_service_ev_fifo(ac);
	    		 //schedule_work(&ac->work);
	    		 input_sync(ac->input);
       		}
//	}			

	return IRQ_HANDLED;
}

static void afa750_work_func(struct work_struct *work){
	struct afa750 *ac = container_of((struct delayed_work *)work, struct afa750, work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&ac->delay));

//	printk("************ %s ********\n", __func__);
//	mutex_lock(&ac->mutex);	
	afa750_service_ev_fifo(ac);
//	mutex_unlock(&ac->mutex);
	input_sync(ac->input);
	schedule_delayed_work(&ac->work,delay);	
}

static void __afa750_disable(struct afa750 *ac)
{
	/*
	 * A '0' places the AFA750 into standby mode
	 * with minimum power consumption.
	 */
	AC_WRITE(ac, POWER_CTL, NORMAL);
}

static void __afa750_enable(struct afa750 *ac)
{
	//AC_WRITE(ac, POWER_CTL, ac->pdata.power_mode | PCTL_MEASURE); Frances
	AC_WRITE(ac, POWER_CTL, NORMAL);
}

void afa750_suspend(struct afa750 *ac)
{
	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled && ac->opened)
		__afa750_disable(ac);

	ac->suspended = true;

	mutex_unlock(&ac->mutex);
}
EXPORT_SYMBOL_GPL(afa750_suspend);

void afa750_resume(struct afa750 *ac)
{
	mutex_lock(&ac->mutex);

	if (ac->suspended && !ac->disabled && ac->opened)
		__afa750_enable(ac);

	ac->suspended = false;

	mutex_unlock(&ac->mutex);
}
EXPORT_SYMBOL_GPL(afa750_resume);

static ssize_t afa750_disable_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct afa750 *ac = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", ac->disabled);
}

static ssize_t afa750_disable_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct afa750 *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);

	if (!ac->suspended && ac->opened) {
		if (val) {
			if (!ac->disabled)
				__afa750_disable(ac);
		} else {
			if (ac->disabled)
				__afa750_enable(ac);
		}
	}

	ac->disabled = !!val;

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(disable, 0664, afa750_disable_show, afa750_disable_store);

static ssize_t afa750_set_enable(struct device *dev, int enable){
//	struct i2c_client *client = to_i2c_client(dev);
	struct afa750 *ac = dev_get_drvdata(dev);// = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&ac->enable);
	
	mutex_lock(&ac->mutex);
	if(enable){
		if(pre_enable == 0){
		schedule_delayed_work(&ac->work,msecs_to_jiffies(atomic_read(&ac->delay)));
		atomic_set(&ac->enable, 1);
		}
	}else {
		if(pre_enable == 1){
		cancel_delayed_work_sync(&ac->work);
		atomic_set(&ac->enable, 0);
		}	
	}
	mutex_unlock(&ac->mutex);
}

static ssize_t afa750_enable_show(struct device *dev, struct device_attribute *attr, char *buf){
	struct afa750 *ac = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", atomic_read(&ac->enable));
}
static ssize_t afa750_enable_store(struct device *dev, struct device_attribute *attr, char *buf, size_t count){
        struct afa750 *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;
	
	error = strict_strtoul(buf, 10, &val);
//	printk("*************error = %d\n", error);
	if (error)
		return error;
//	val = val ? 1 : 0;
//	printk("*********val = %d\n", val);
	if((val == 0) || (val == 1))
		afa750_set_enable(dev, val);
	return count;
}

static DEVICE_ATTR(enable, 0666, afa750_enable_show, afa750_enable_store);

static ssize_t afa750_delay_show(struct device *dev, struct device_attribute *attr, char *buf){
	struct afa750 *ac = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", atomic_read(&ac->delay));
}

static ssize_t afa750_delay_store(struct device *dev,
		struct device_attribute *attr,const char *buf, size_t count){
	unsigned long data;
	int error;
	struct afa750 *ac = dev_get_drvdata(dev);// = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if(data > AFA750_MAX_DELAY)
		data = AFA750_MAX_DELAY;
	atomic_set(&ac->delay, (unsigned int) data);
	return count;
}
static DEVICE_ATTR(delay, 0666, afa750_delay_show, afa750_delay_store);


static ssize_t afa750_calibrate_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct afa750 *ac = dev_get_drvdata(dev);
	ssize_t count;

	mutex_lock(&ac->mutex);
	count = sprintf(buf, "%d,%d,%d\n",
			0,
			0,
			0);
	mutex_unlock(&ac->mutex);

	return count;
}

static ssize_t afa750_calibrate_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{

	/*
	 * Hardware offset calibration has a resolution of 15.6 mg/LSB.
	 * We use HW calibration and handle the remaining bits in SW. (4mg/LSB)
	 */

	

	return count;
}

static DEVICE_ATTR(calibrate, 0664,
		   afa750_calibrate_show, afa750_calibrate_store);

static ssize_t afa750_rate_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%u\n", 400);
}

static ssize_t afa750_rate_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct afa750 *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);

	ac->pdata.data_rate = RATE(val);
	AC_WRITE(ac, DATA_RATE,
		 ac->pdata.data_rate);

	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(rate, 0664, afa750_rate_show, afa750_rate_store);

static ssize_t afa750_autosleep_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%u\n",
		0);
}

static ssize_t afa750_autosleep_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);
	if (error)
		return error;



	return count;
}

static DEVICE_ATTR(autosleep, 0664,
		   afa750_autosleep_show, afa750_autosleep_store);

static ssize_t afa750_x_invert_show(struct device *dev,
                                                        struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", (x_sign == SIGN_POS ? 0 : 1));
}

static ssize_t afa750_x_invert_store(struct device *dev,
                                                        struct device_attribute *attr,
                                                        const char *buf, size_t count)
{
        struct afa750 *ac = dev_get_drvdata(dev);
        unsigned long val;
        int error;

        error = strict_strtoul(buf, 10, &val);
        if (error)
                return error;

        mutex_lock(&ac->mutex);

        x_sign = (val ? SIGN_NEG : SIGN_POS);

        mutex_unlock(&ac->mutex);

        return count;
}

static DEVICE_ATTR(xinv, 0664,
                        afa750_x_invert_show, afa750_x_invert_store);

static ssize_t afa750_y_invert_show(struct device *dev,
                                                        struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", (y_sign == SIGN_POS ? 0 : 1));
}

static ssize_t afa750_y_invert_store(struct device *dev,
                                                        struct device_attribute *attr,
                                                        const char *buf, size_t count)
{
        struct afa750 *ac = dev_get_drvdata(dev);
        unsigned long val;
        int error;

        error = strict_strtoul(buf, 10, &val);
        if (error)
                return error;

        mutex_lock(&ac->mutex);

        y_sign = (val ? SIGN_NEG : SIGN_POS);

        mutex_unlock(&ac->mutex);

        return count;
}

static DEVICE_ATTR(yinv, 0664,
                        afa750_y_invert_show, afa750_y_invert_store);

static ssize_t afa750_z_invert_show(struct device *dev,
                                                        struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", (z_sign == SIGN_POS ? 0 : 1));
}

static ssize_t afa750_z_invert_store(struct device *dev,
                                                        struct device_attribute *attr,
                                                        const char *buf, size_t count)
{
        struct afa750 *ac = dev_get_drvdata(dev);
        unsigned long val;
        int error;

        error = strict_strtoul(buf, 10, &val);
        if (error)
                return error;

        mutex_lock(&ac->mutex);

        z_sign = (val ? SIGN_NEG : SIGN_POS);

        mutex_unlock(&ac->mutex);

        return count;
}

static DEVICE_ATTR(zinv, 0664,
                        afa750_z_invert_show, afa750_z_invert_store);

static ssize_t afa750_xy_swap_show(struct device *dev,
                                                        struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", xy_swap);
}

static ssize_t afa750_xy_swap_store(struct device *dev,
                                                        struct device_attribute *attr,
                                                        const char *buf, size_t count)
{
        struct afa750 *ac = dev_get_drvdata(dev);
        unsigned long val;
        int error;

        error = strict_strtoul(buf, 10, &val);
        if (error)
                return error;

        mutex_lock(&ac->mutex);

        xy_swap = !!val;

        mutex_unlock(&ac->mutex);

        return count;
}

static DEVICE_ATTR(xyswap, 0664,
                        afa750_xy_swap_show, afa750_xy_swap_store);

static ssize_t afa750_position_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct afa750 *ac = dev_get_drvdata(dev);
	ssize_t count;

	mutex_lock(&ac->mutex);
	count = sprintf(buf, "(%d, %d, %d)\n",
			ac->saved.x, ac->saved.y, ac->saved.z);
	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(position, S_IRUGO, afa750_position_show, NULL);

#ifdef ADXL_DEBUG
static ssize_t afa750_write_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct afa750 *ac = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	/*
	 * This allows basic ADXL register write access for debug purposes.
	 */
	error = strict_strtoul(buf, 16, &val);
	if (error)
		return error;

	mutex_lock(&ac->mutex);
	//AC_WRITE(ac, val >> 8, val & 0xFF); Frances
	mutex_unlock(&ac->mutex);

	return count;
}

static DEVICE_ATTR(write, 0664, NULL, afa750_write_store);
#endif

static struct attribute *afa750_attributes[] = {
	&dev_attr_disable.attr,
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_rate.attr,
	&dev_attr_autosleep.attr,
	&dev_attr_position.attr,
        &dev_attr_xinv.attr,
        &dev_attr_yinv.attr,
        &dev_attr_zinv.attr,
        &dev_attr_xyswap.attr,
#ifdef ADXL_DEBUG
	&dev_attr_write.attr,
#endif
	NULL
};

static const struct attribute_group afa750_attr_group = {
	.attrs = afa750_attributes,
};

static int afa750_input_open(struct input_dev *input)
{
	struct afa750 *ac = input_get_drvdata(input);

	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled)
		__afa750_enable(ac);

	ac->opened = true;

	mutex_unlock(&ac->mutex);

	return 0;
}

static void afa750_input_close(struct input_dev *input)
{
	struct afa750 *ac = input_get_drvdata(input);

	mutex_lock(&ac->mutex);

	if (!ac->suspended && !ac->disabled)
		__afa750_disable(ac);

	ac->opened = false;

	mutex_unlock(&ac->mutex);
}

struct afa750 *afa750_probe(struct device *dev, int irq,
			      bool fifo_delay_default,
			      const struct afa750_bus_ops *bops)
{
	struct afa750 *ac;
	struct input_dev *input_dev;
	const struct afa750_platform_data *pdata;
	int err;
	int index;

//	index = item_integer("sensor.grivaty.int", 1);
//	imapx_pad_set_mode(0, 1, index);
//	irq = imapx_pad_irq_number(index);
//	printk("******irq = %d******\n", irq);
//	if (!irq) {
//		dev_err(dev, "no IRQ?\n");
//		err = -ENODEV;
//		goto err_out;
//	}

        AFA750_IRQ_CNT = 0;

	ac = kzalloc(sizeof(*ac), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ac || !input_dev) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	ac->fifo_delay = fifo_delay_default;

	pdata = dev->platform_data;
	if (!pdata) {
		dev_dbg(dev,
			"No platform data: Using default initialization\n");
		pdata = &afa750_default_init;
	}

	ac->pdata = *pdata;
	pdata = &ac->pdata;

	ac->input = input_dev;
	ac->dev = dev;
	ac->irq = irq;
	ac->bops = bops;

	mutex_init(&ac->mutex);
	mutex_init(&ac->enable_mutex);
	input_dev->name = "afa750";
     

	snprintf(ac->phys, sizeof(ac->phys), "%s/input0", dev_name(dev));

	input_dev->phys = ac->phys;
	input_dev->dev.parent = dev;
	input_dev->id.product = 314; //Frances
	input_dev->id.bustype = bops->bustype;
	input_dev->open = afa750_input_open;
	input_dev->close = afa750_input_close;

	input_set_drvdata(input_dev, ac);

	__set_bit(ac->pdata.ev_type, input_dev->evbit);

	if (ac->pdata.ev_type == EV_REL) {
		__set_bit(REL_X, input_dev->relbit);
		__set_bit(REL_Y, input_dev->relbit);
		__set_bit(REL_Z, input_dev->relbit);
	} else {
		/* EV_ABS */
//		__set_bit(ABS_X, input_dev->absbit);
//		__set_bit(ABS_Y, input_dev->absbit);
//		__set_bit(ABS_Z, input_dev->absbit);
		set_bit(EV_ABS, input_dev->evbit);
		input_set_abs_params(input_dev, ABS_X, -AFA_FULLRES_MIN_VAL, AFA_FULLRES_MAX_VAL, 0, 0);
		input_set_abs_params(input_dev, ABS_Y, -AFA_FULLRES_MIN_VAL, AFA_FULLRES_MAX_VAL, 0, 0);
		input_set_abs_params(input_dev, ABS_Z, -AFA_FULLRES_MIN_VAL, AFA_FULLRES_MAX_VAL, 0, 0);
	}
     

    
	ac->bops->write(dev, POWER_CTL, NORMAL);

//	INIT_WORK(&ac->work, afa750_work);
//	imapx_pad_irq_config(index, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, FILTER_MAX);
//	err = request_threaded_irq(ac->irq, NULL, afa750_irq, 0, "afa750", ac);
//	err = request_irq(ac->irq, afa750_irq, 0, dev_name(dev), ac);
//	if (err) {
//		dev_err(dev, "irq %d busy?\n", ac->irq);
//		goto err_free_mem;
//	}
	
	INIT_DELAYED_WORK(&ac->work, afa750_work_func);
	atomic_set(&ac->delay, AFA750_MAX_DELAY);
	atomic_set(&ac->enable, 0);


	err = sysfs_create_group(&dev->kobj, &afa750_attr_group);
	if (err){
		printk("*********sysfs_create_group error*******\n");
		goto err_free_irq;
	}
	err = input_register_device(input_dev);
	if (err){
		printk("**********input_register_device error *************\n");
		goto err_remove_attr;
	}

        AC_WRITE(ac, WMA, WMA_CTL_5);
        //AC_WRITE(ac, AFM314_NODR, 0x04);
     

	return ac;

 err_remove_attr:
	sysfs_remove_group(&dev->kobj, &afa750_attr_group);
 err_free_irq:
	free_irq(ac->irq, ac);
 err_free_mem:
	input_free_device(input_dev);
	kfree(ac);
 err_out:
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(afa750_probe);

int afa750_remove(struct afa750 *ac)
{
	sysfs_remove_group(&ac->dev->kobj, &afa750_attr_group);
	free_irq(ac->irq, ac);
	input_unregister_device(ac->input);
	dev_dbg(ac->dev, "unregistered accelerometer\n");
	kfree(ac);

	return 0;
}
EXPORT_SYMBOL_GPL(afa750_remove);

MODULE_AUTHOR("Frances Chu<franceschu@afamicro.com>");
MODULE_DESCRIPTION("AFA750 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
