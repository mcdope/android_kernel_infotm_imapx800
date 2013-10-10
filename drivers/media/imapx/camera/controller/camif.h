/*
 * camif.h
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
 * IMAPX CamIF configuration and feature definition head file.
 *
 * Author:
 *	Sololz<sololz.luo@gmail.com>.
 *
 * Revision History:
 * 1.0  04/26/2011 Sololz.
 * 	Create this file.
 */

#if !defined(__CAMIF_H__)
#define __CAMIF_H__

#include <linux/interrupt.h>
#include <linux/clk.h>

#include <asm/atomic.h>

#include <plat/imapx.h>

#include <mach/imap_hwid.h>

#include "../camera.h"
#include "../sensor/sensor.h"

struct camif_internal {
	int id;
	char name[256];
	struct clk *hclk;
	void __iomem *reg;

	atomic_t status;
	spinlock_t irq_lock;
	wait_queue_head_t wait;
#define CAMIF_IRQ_NONE		(0)
#define CAMIF_IRQ_GEN		(0x2b2b2b2b)
	int mark;
	unsigned int dump[CAMIF_REGISTER_SIZE >> 2];

	struct list_head sensor_pool;
	struct sensor_ops *sensor;
	int sensor_init;
	unsigned int resolution;
	unsigned int fps;
	unsigned int lmode;
	unsigned int seffect;

	atomic_t ref;
};

static void camif_hw_init(void);
static void camif_hw_release(void);
static inline void camif_dump_reg(void);
static inline void camif_recover_reg(void);

/* XXX: I hate this, yes, I'm not kidding. */
static int imapx200_cam_decide_sensor(void);
static int imapx200_cam_do_en(int en);
static inline int imapx200_cam_default_on(void);
static inline int imapx200_cam_default_off(void);
static int imapx200_cam_i2c_read(uint8_t *buf,
		uint8_t *addr, uint32_t size, uint32_t len);
static int imapx200_cam_i2c_write(uint8_t *buf, uint32_t len);
static int imapx200_cam_fill_pointer(struct sensor_ops *ops);

#endif	/* __CAMIF_H__ */
