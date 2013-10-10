/*
 * camif.c
 *
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Main file of IMAPX platform camif driver, this driver just do what
 * a driver should do. It does not manage buffer, or process controller
 * register. It just provide a interrupt handle and sync process to
 * user space. And it provides i2c based sensor configuration operations.
 *
 * Actually a camera object to user is constructured by two main hard
 * ware parts, one is camera controller, the other one is camera sensor.
 * And this driver exports interfaces to user space just like a camera
 * object. It means that callers do not have to care about controller
 * or sensor, what they know is just a whole camera thing.
 *
 * Author:
 *	Sololz<sololz.luo@gmail.com>.
 *
 * Revision History:
 * 1.0  04/25/2011 Sololz.
 * 	Create this file.
 */

#include "camif.h"

static struct camif_internal camif_data = {
	.id = -1,
	.hclk = NULL,
	.reg = NULL,

	.status = ATOMIC_INIT(0),
	.irq_lock = SPIN_LOCK_UNLOCKED,
	.mark = CAMIF_IRQ_NONE,

	.sensor_pool = LIST_HEAD_INIT(camif_data.sensor_pool),
	.sensor_init = 0,
	.resolution = 0,
	.fps = 0,
	.lmode = 1,
	.seffect = 1,
	.ref = ATOMIC_INIT(0),
};

/*
 * Interrupt handle.
 */
static irqreturn_t camif_irq_handle(int irq ,void *dev_id)
{
	int status = readl(camif_data.reg + IMAP_CICPTSTATUS);;

	/* Clear interrupts, if not, this irq handler will deadly
	 * cycle called. */
	writel(status, camif_data.reg + IMAP_CICPTSTATUS);
	atomic_set(&camif_data.status, status);

	/* 
	 * Actually this irq handle do nothing but a wake up operation
	 * to sync user space tasks. Considering multicore arm cpu env,
	 * this spinlock protect does make sense. Because if Linux kernel
	 * if configured that one task(process) can run on different
	 * cpus, with lock sync is very much important.
	 */
	spin_lock(&camif_data.irq_lock);
	if (camif_data.mark == CAMIF_IRQ_NONE) {
		wake_up(&camif_data.wait);
	}
	camif_data.mark = CAMIF_IRQ_GEN;
	spin_unlock(&camif_data.irq_lock);

	return IRQ_HANDLED;
}

static int camif_open(struct inode *inode, struct file *file)
{
	/* XXX: P&W (stands for Peter and Warits, I don't even want to
	 * change any code of it because my egg aches). */
	if (!atomic_read(&camif_data.ref)) {
		IC_DBG("Decide camera sensor.\n");
		if (imapx200_cam_decide_sensor()) {
			IC_ERR("Unsupported sensor.\n");
			return -EFAULT;
		}
		camif_data.sensor->reset(camif_data.reg);
		camif_data.sensor->power_off();
	}
	atomic_inc(&camif_data.ref);
	return 0;
}

static int camif_release(struct inode *inode, struct file *file)
{
	atomic_dec(&camif_data.ref);
	if (camif_data.sensor_init && !atomic_read(&camif_data.ref)) {
		camif_data.sensor->set_mode(CLOSE_SENSOR, camif_data.reg);
		camif_data.sensor->power_off();
	}

	return 0;
}

/* This read system call is just for read interrupt status. */
static ssize_t camif_read(struct file *file, char *buf, size_t count, loff_t *pos)
{
	if (unlikely(count < 4)) {
		IC_ERR("Invalid read size.\n");
		return -EINVAL;
	}

	{
		int status = atomic_read(&camif_data.status);
		if (unlikely(copy_to_user(buf, &status, 4))) {
			IC_ERR("Copy irq status error.\n");
			return -EFAULT;
		}
	}

	return 4;
}

static int camif_ioctl(struct file *file, 
		unsigned int cmd, unsigned long arg) 
{
	unsigned int resolution = 0;
	unsigned int lmode = 0;
	unsigned int seffect = 0;

	switch (cmd) {
		/* Control. */
		case DWL_CAM_IOCTRL_SENSOR_INIT:	/* Initialize sensor. */
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_INIT\n");
			/* camif_data.sensor->reset(camif_data.reg); */
			camif_data.sensor->power_on();
			msleep(50);
			/* Initialize sensor connected to camif. */
			/* camif_data.sensor->init(camif_data.reg); */
			camif_data.sensor->set_mode(INIT_SENSOR, camif_data.reg);
			camif_data.sensor_init = 1;
			break;

		case DWL_CAM_IOCTRL_SENSOR_DEINIT:	/* Deinitialize sensor. */
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_DEINIT\n");
			camif_data.sensor->set_mode(CLOSE_SENSOR, camif_data.reg);
			camif_data.sensor->power_off();
			camif_data.sensor_init = 0;
			break;

		case DWL_CAM_IOCTRL_SENSOR_START:	/* Start sensor. */
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_START\n");
			break;
		case DWL_CAM_IOCTRL_SENSOR_STOP:	/* Stop sensor. */
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_STOP\n");
			break;
		case DWL_CAM_IOCTRL_SENSOR_CHECK_CONNECT:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_CHECK_CONNECT\n");
			break;

		/* Resolution. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_RESOLUTION:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_RESOLUTION\n");
			__put_user(camif_data.sensor->s_res, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_RESOLUTION:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_RESOLUTION\n");
			__put_user(camif_data.resolution, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_RESOLUTION:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_SET_RESOLUTION\n");
			__get_user(resolution, (unsigned int *)arg);
			if (unlikely(!(resolution & camif_data.sensor->s_res))) {
				IC_ERR("Unsupport resolution.\n");
				return -EINVAL;
			}
			switch (resolution) {
				case DWL_CAM_RES_UXGA:
					camif_data.sensor->set_mode(SWITCH_SENSOR_TO_HIGH_XUGA, 
							camif_data.reg);
					break;
				case DWL_CAM_RES_VGA:
					camif_data.sensor->set_mode(SENSOR_TO_HIGH_PREVIEW, 
							camif_data.reg);
					break;
				case DWL_CAM_RES_QVGA:
					camif_data.sensor->set_mode(SENSOR_TO_LOW_PREVIEW, 
							camif_data.reg);
					break;
				/* XXX: Currently only three resolutions are supported
				 * because we only need three. */
			}
			camif_data.resolution = resolution;
			camif_data.fps = camif_data.sensor->get_fps(resolution);
			break;

		/* Fps. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_FPS:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_FPS\n");
			__put_user(DWL_CAM_FPS_DEFAULT, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_FPS:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_FPS\n");
			__put_user(camif_data.fps, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_FPS:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_SET_FPS\n");
			/* XXX:
			 * Currently the fps can't be set by midware framework, because 
			 * different sensors on different boards have different fps
			 * supports. Set the resolution will just match an assigned fps.
			 */
			break;

		/* Light mode. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_LIGHT_MODE:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_LIGHT_MODE\n");
			__put_user(camif_data.sensor->s_lmode, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_LIGHT_MODE:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_LIGHT_MODE\n");
			__put_user(camif_data.lmode, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_LIGHT_MODE:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_SET_LIGHT_MODE\n");
			__get_user(lmode, (unsigned int *)arg);
			if (unlikely(!(lmode & camif_data.sensor->s_lmode))) {
				IC_ERR("Unsupported light mode 0x%08x.\n", lmode);
				return -EINVAL;
			}
			camif_data.sensor->set_wb(lmode);
			camif_data.lmode = lmode;
			break;

		/* Color saturation. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_COLOR_SATURATION:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_COLOR_SATURATION:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_COLOR_SATURATION:
			/* TODO */
			break;
		/* Brightness. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_BRIGHTNESS:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_BRIGHTNESS:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_BRIGHTNESS:
			/* TODO */
			break;
		/* Contrast. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_CONTRAST:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_CONTRAST:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_CONTRAST:
			/* TODO */
			break;

		/* Special effect. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_SPECIAL_EFFECT:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_SPECIAL_EFFECT\n");
			__put_user(camif_data.sensor->s_seffect, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_SPECIAL_EFFECT:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_GET_SPECIAL_EFFECT\n");
			__put_user(camif_data.seffect, (unsigned int *)arg);
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_SPECIAL_EFFECT:
			IC_DBG("DWL_CAM_IOCTRL_SENSOR_SET_SPECIAL_EFFECT\n");
			__get_user(seffect, (unsigned int *)arg);
			if (unlikely(!(seffect & camif_data.sensor->s_seffect))) {
				IC_ERR("Unsupported special effect 0x%08x.\n", seffect);
				return -EINVAL;
			}
			camif_data.sensor->set_effect(seffect);
			camif_data.seffect = seffect;
			break;

		/* Exposure. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_EXPOSURE:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_EXPOSURE:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_EXPOSURE:
			/* TODO */
			break;
		/* Sharpness. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_SHARPNESS:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_SHARPNESS:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_SHARPNESS:
			/* TODO */
			break;
		/* Mirrorandflip. */
		case DWL_CAM_IOCTRL_SENSOR_GET_SUPPORT_MIRRORANDFLIP:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_GET_MIRRORANDFLIP:
			/* TODO */
			break;
		case DWL_CAM_IOCTRL_SENSOR_SET_MIRRORANDFLIP:
			/* TODO */
			break;

		default:
			IC_ERR("Unknow IO control command, %d.\n", cmd);
			return -EINVAL;
	}

	return 0;
}

static int camif_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	/* This map is for controller register map, so nocache. */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	return remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			size, vma->vm_page_prot);
}

static unsigned int camif_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &camif_data.wait, wait);
	spin_lock(&camif_data.irq_lock);
	if (camif_data.mark == CAMIF_IRQ_GEN) {
		mask = POLLIN | POLLRDNORM;
		camif_data.mark = CAMIF_IRQ_NONE;
	}
	spin_unlock(&camif_data.irq_lock);

	return mask;
}

static struct file_operations camif_ops = {
	.owner = THIS_MODULE,
	.open = camif_open,
	.release = camif_release,
	.read = camif_read,
	.unlocked_ioctl = camif_ioctl,
	.mmap = camif_mmap,
	.poll = camif_poll,
};

static struct miscdevice camif_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &camif_ops,
};

/* ############################################################################## */

static int camif_probe(struct platform_device *pdev)
{
	int ret = 0;

	/* Initialize camif internal data structure. */
	init_waitqueue_head(&camif_data.wait);

	/* First request for a camera device id. */
	if (camif_data.id < 0) {
		camif_data.id = DWL_MODULE_CAM_0;
		if (unlikely(camif_data.id < 0)) {
			IC_ERR("Get camera device id for camif error!\n");
			return -EFAULT;
		}
	}

	/* Request irq and disable it, irq should be enabled if any one is
	 * using camif. */
	if (unlikely(request_irq(IRQ_CAM, camif_irq_handle, IRQF_DISABLED, 
				dev_name(&pdev->dev), NULL))) {
		IC_ERR("Request irq for camif error!\n");
		return -EFAULT;
	}
	/* Map camif device registers. */
	camif_data.reg = ioremap_nocache(CAMIF_REGISTER_PHYS_BASE, 
			CAMIF_REGISTER_SIZE);
	if (unlikely(camif_data.reg == NULL)) {
		IC_ERR("Map camif register error!\n");
		return -EFAULT;
	}

	/* Register a device node in file system. */
	memset(camif_data.name, '\0', sizeof(camif_data.name));
	sprintf(camif_data.name, "%s%d", IC_DEV_NAME, camif_data.id);
	camif_miscdev.name = camif_data.name;
	ret = misc_register(&camif_miscdev);
	if (unlikely(ret)) {
		IC_ERR("misc_register() error, %d.\n", ret);
		return ret;
	}

	/* 
	 * CamIF depends on EPL and HCLK both, first Petter told me that
	 * camif just depends on EPL clock. But finally I found that it's 
	 * not enough just enable EPL. So you may ask why I talk about this,
	 * cuz I just want to fuck Petter.
	 * TODO: The clock should not just be enabled here, clock should be
	 * enabled at used.
	 */
	camif_data.hclk = clk_get(&pdev->dev, "camif");
	if (unlikely(IS_ERR(camif_data.hclk))) {
		IC_ERR("Get camif clock error!\n");
		return -EFAULT;
	}
	clk_enable(camif_data.hclk);

	/* Initialize camif hardware, actually set clock mode and mask power
	 * first. Camif will be powered on at sensor power on. */
	camif_hw_init();

	return 0;
}

static int camif_remove(struct platform_device *pdev)
{
	misc_deregister(&camif_miscdev);

	camif_hw_release();

	clk_disable(camif_data.hclk);
	disable_irq(IRQ_CAM);

	iounmap(camif_data.reg);
	camif_data.reg = NULL;

	return 0;
}

#if defined(CONFIG_PM)
static int camif_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (camif_data.sensor_init) {
		camif_data.sensor->set_mode(CLOSE_SENSOR, camif_data.reg);
		camif_data.sensor->power_off();
	}

	camif_dump_reg();
	camif_hw_release();

	return 0;
}

static int camif_resume(struct platform_device *pdev)
{
	camif_hw_init();
	camif_recover_reg();
	if (camif_data.sensor_init) {
		camif_data.sensor->reset(camif_data.reg);
		camif_data.sensor->power_on();
		/* camif_data.sensor->init(camif_data.reg); */
		camif_data.sensor->set_mode(INIT_SENSOR, camif_data.reg);
	}

	return 0;
}
#endif

static struct platform_driver camif_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx200_camif",
	},
	.probe = camif_probe,
	.remove = camif_remove,
#if defined(CONFIG_PM)
	.suspend = camif_suspend,
	.resume = camif_resume,
#endif
};

/* ############################################################################## */

/*
 * Initialize camif and connected sensor, enable clock, enable irq, power 
 * on etc.
 */
static void camif_hw_init(void)
{
	unsigned int val;

	/* 
	 * Set camif clock configuration, set ratio and clock source.
	 * The ratio is set to be 19(0x13) and clock source is EPLL 
	 * output. 
	 */
	val = readl(rDIV_CFG1);
	val &= ~((3 << 16) | (0x1f << 18));
	val |= ((2 << 16) | (0x13 << 18));
	writel(val, rDIV_CFG1);

	val = readl(camif_data.reg + IMAP_CIGCTRL);
	val &= ~(1 << 5);       /* Set clk polarity. */
	val &= ~(1 << 4);       /* Set vsync. */
	val &= ~(1 << 3);       /* Set href. */
	val &= ~(1 << 2);       /* Set field. */
	val |= (1 << 1);        /* Set reset. */
	writel(val, camif_data.reg + IMAP_CIGCTRL);

	val = readl(camif_data.reg + IMAP_CISRCFMT);
	val |= (1 << 31);       /* Set itumode. */
	val &= ~(1 << 30);      /* Set uvoffset. */
	val &= ~(1 << 29);      /* Set scanmode. */
	val &= ~(3 << 14);      /* Set order. */
	writel(val, camif_data.reg + IMAP_CISRCFMT);

	/* GPL con all relates to camif, so config them them to be 10 mode
	 * all, it's reasonable just do like this. */
	imapx_gpio_setcfg(IMAPX_GPL_ALL, IG_CTRL0, IG_NORMAL);
}

/*
 * Release camif and connected sensor, disable clock, disable irq, power
 * off, etc.
 */
static void camif_hw_release(void)
{
	/* Currently do nothing. */
}

/*
 * Dump all regs to memory for suspend/resume use.
 */
static inline void camif_dump_reg(void)
{
	memcpy(camif_data.dump, camif_data.reg, CAMIF_REGISTER_SIZE);
}

/*
 * Recover all regs to memory for suspend/resume use.
 */
static inline void camif_recover_reg(void)
{
	memcpy(camif_data.reg, camif_data.dump, CAMIF_REGISTER_SIZE);
}

/* ############################################################################## */

static int __init camif_init(void)
{
	return platform_driver_register(&camif_driver);
}

static void __exit camif_exit(void)
{
	platform_driver_unregister(&camif_driver);
}

module_init(camif_init);
module_exit(camif_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
MODULE_DESCRIPTION("IMAPX camera driver");

/* ############################################################################## */
/* XXX: P&W
 * Following code is from Peter and Warits, I don't even want to 
 * change anything because this makes me sick. */

static int imapx200_cam_decide_sensor(void)
{
	struct sensor_ops *p;

	list_for_each_entry(p, &camif_data.sensor_pool, link)
	{
		camif_data.sensor = p;

#if defined(CONFIG_IG_CAM_CMOS_ID)
		if(!p || !p->idlist) continue;

		p->reset(camif_data.reg);
		p->power_on();
		msleep(10);

		{ uint32_t *q, id = p->get_id();
			for(q = p->idlist; *q; q++)
				if(*q == id) {p->power_off();
					goto __cam_detected__;}
		}

		p->power_off();
#elif defined(CONFIG_IG_CAM_HWID)
		if(p && (p->hwid ==
					imap_product_getid(IMAP_PRODUCT_CAMERA)))
			goto __cam_detected__;
#else
		/* use the first sensor in list */
		goto __cam_detected__;
#endif
	}

	printk(KERN_INFO "no camif sensor is available.\n");
	return -1;

__cam_detected__:
	if(p->name)
		printk(KERN_INFO "sensor %s is detected.\n", p->name);

	return 0;
}

static int imapx200_cam_do_en(int en)
{
	/*
	 * camera interface power supply enable
	 *
	 */
	uint32_t pow_addr, pwdn_addr;

	pow_addr = __imapx_name_to_gpio(CONFIG_IG_CAMIF0_SUPPLY);
	if(pow_addr == IMAPX_GPIO_ERROR) {                       
		printk(KERN_ERR "failed to get pow_supply pin.\n");   
	} else {                                                        
		imapx_gpio_setcfg(pow_addr, IG_OUTPUT, IG_NMSL);               
		imapx_gpio_setpin(pow_addr, !!en, IG_NMSL);               
		imapx_gpio_chmod(pow_addr, (en?IG_NORMAL: IG_SLEEP));
	}

	/*
	 *    camera inerface pwdn enable
	 *
	 */
	pwdn_addr = __imapx_name_to_gpio(CONFIG_IG_CAMIF0_PND);
	if(pow_addr == IMAPX_GPIO_ERROR) {                       
		printk(KERN_ERR "failed to get pwdn pin.\n");   
	} else {
		imapx_gpio_setcfg(pwdn_addr, IG_OUTPUT, IG_NMSL);               
		imapx_gpio_setpin(pwdn_addr, (!!en) ^ (!!camif_data.sensor->pwdn), IG_NMSL);
		imapx_gpio_chmod(pow_addr, (en?IG_NORMAL: IG_SLEEP));
	}

	printk(KERN_ERR "camif en=%d\n", en);

	return 0;
}

static inline int imapx200_cam_default_on(void)
{
	return imapx200_cam_do_en(1);
}
static inline int imapx200_cam_default_off(void)
{
	return imapx200_cam_do_en(0);
}

static int imapx200_cam_i2c_read(uint8_t *buf,
		uint8_t *addr, uint32_t size, uint32_t len)
{
	struct i2c_msg msgs[] = {{.addr = camif_data.sensor->addr, .flags = I2C_M_NOSTART,
		.len = size, .buf = addr}, {.addr = camif_data.sensor->addr,
			.flags = I2C_M_RD, .len = len, .buf = buf}};

	if(camif_data.sensor->adapter)
		if(2 == i2c_transfer(camif_data.sensor->adapter, msgs, 2))
			return 0;
	return -1;
}

static int imapx200_cam_i2c_write(uint8_t *buf, uint32_t len)
{
	struct i2c_msg msgs[] = {{.addr = camif_data.sensor->addr, .flags= 0,
		.len = len, .buf = buf} };

	if(camif_data.sensor->adapter)
		if(1 == i2c_transfer(camif_data.sensor->adapter, msgs, 1))
			return 0;
	return -1;
}

static int imapx200_cam_fill_pointer(struct sensor_ops *ops)
{
	/* fill the pointer is not specified. */
	if(!ops->power_on)
		ops->power_on = imapx200_cam_default_on;
	if(!ops->power_off)
		ops->power_off = imapx200_cam_default_off;
	if(!ops->i2c_write)
		ops->i2c_write = imapx200_cam_i2c_write;
	if(!ops->i2c_read)
		ops->i2c_read = imapx200_cam_i2c_read;

	if(!(ops->adapter = i2c_get_adapter(CONFIG_IG_CAMIF0_I2C + 1)))
	{
		printk(KERN_ERR "can not get i2c adapter for camif0.\n");
		return -1;
	}

	return 0;
}

int imapx200_cam_sensor_register(struct sensor_ops *ops)     
{                                                                
	if(ops) {
		imapx200_cam_fill_pointer(ops);
		list_add_tail(&ops->link, &camif_data.sensor_pool);
		if(ops->name)
			printk(KERN_INFO "adding camif sensor: %s\n", ops->name);
	} else {
		printk(KERN_ERR "try to register NULL camif ops\n");
		return -1;
	}

	return 0;
}                                                                

int imapx200_cam_sensor_unregister(struct sensor_ops *ops)
{                       
	if(ops) {
		list_del_init(&ops->link);
		if(ops->name)
			printk(KERN_INFO "camif sensor: %s, unregistered\n", ops->name);
	} else {
		printk(KERN_ERR "try to unregister NULL camif ops\n");
		return -1;
	}

	return 0;
}
