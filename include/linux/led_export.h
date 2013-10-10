/*
 * led_export.h
 *
 * Copyright (c) 2012~2014 ShangHai InfoTM Ltd all rights reserved.
 *
 * Use of Infotm's code is governed by terms and conditions
 * stated in the accompanying licensing statement.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sololz <sololz.luo@gmail.com> or <sololz.luo@infotmic.com.cn>.
 */

#ifndef __LED_EXPORT_H__
#define __LED_EXPORT_H__

/**
 * Light working led. Working led is the RUN led mutex level,
 * so, if it's called, other RUN level led will be turned off.
 */
void led_working_light(void);

/**
 * Turn off working led. This function is not supposed to be 
 * called directly to turn off working led because kernel logical
 * code will guarantee working led turned off at power off.
 */
void led_working_dark(void);

/**
 * Light suspending led if in supend.
 */
void led_suspending_light(void);

/**
 * Turn off suspending led.
 */
void led_suspending_dark(void);

#endif  /* __LED_EXPORT_H__ */
