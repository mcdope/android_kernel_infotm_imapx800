/*
 *  linux/arch/arm/mach-imapx800/timer-cmn.c
 *
 *  Copyright (C) 1999 - 2003 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ported from timer-sp.c: by warits Mar.13 2012
 */
#include <linux/clk.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/module.h>

#include <asm/hardware/arm_timer.h>
#include "core.h"

static long __init cmn_get_clock_rate(const char *name)
{
	long rate;

#if !defined(CONFIG_IMAPX800_FPGA_PLATFORM)
	struct clk *clk;
	int err;

	clk = clk_get(NULL, "cmn-timer0");
	if (IS_ERR(clk)) {
		pr_err("cmn: %s clock not found: %d\n", name,
			(int)PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	err = clk_enable(clk);
	if (err) {
		pr_err("cmn: %s clock failed to enable: %d\n", name, err);
		clk_put(clk);
		return err;
	}

	rate = clk_get_rate(clk);
	if (rate < 0) {
		pr_err("cmn: %s clock failed to get rate: %ld\n", name, rate);
		clk_disable(clk);
		clk_put(clk);
	}

//	printk(KERN_ERR "Got cmn-timer rate: %d\n", rate);
#else
	rate = 10000000;
#endif

	return rate;
}

static void __iomem *clkevt_base;
static unsigned long clkevt_reload;

/*
 * IRQ handler for the timer
 */
static irqreturn_t cmn_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	/* clear the interrupt */
	readl(clkevt_base + TIMER_INTCLR);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static int cmn_enabled = 0;

static void cmn_set_mode(enum clock_event_mode mode,
	struct clock_event_device *evt)
{
	/* disable timer */
	writel(0, clkevt_base + TIMER_CTRL);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		writel(clkevt_reload, clkevt_base + TIMER_LOAD);

		/* auto-reload & enable */
		writel(0x3, clkevt_base + TIMER_CTRL);
//		printk(KERN_ERR "common timer is enabled. %d\n", mode);
		cmn_enabled = 1;
		break;

	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		printk(KERN_ERR "common timer is disabled. %d\n", mode);
		cmn_enabled = 0;
		break;
	}

}

static struct clock_event_device cmn_clockevent = {
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_PERIODIC,
	.set_mode	= cmn_set_mode,
	.rating		= 250,		// only for boot use, switch to twd after smp enabled
	.cpumask	= cpu_all_mask,
};

static struct irqaction cmn_timer_irq = {
	.name		= "cmn-timer0",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= cmn_timer_interrupt,
	.dev_id		= &cmn_clockevent,
};

void __init cmn_timer_init(void __iomem *base, unsigned int irq,
	const char *name)
{
	struct clock_event_device *evt = &cmn_clockevent;
	long rate = cmn_get_clock_rate(name);

	if (rate < 0)
		return;

	clkevt_base = base;
	clkevt_reload = DIV_ROUND_CLOSEST(rate, HZ);

	evt->name = name;
	evt->irq = irq;
	evt->mult = div_sc(rate, NSEC_PER_SEC, evt->shift);
	evt->max_delta_ns = clockevent_delta2ns(0xffffffff, evt);
	evt->min_delta_ns = clockevent_delta2ns(0xf, evt);

	setup_irq(irq, &cmn_timer_irq);

	clockevents_register_device(evt);
	core_msg("timer-cmn registered.\n");
}

#ifdef CONFIG_CPU_FREQ
void cmn_timer_update(long x)
{
    long rate = cmn_get_clock_rate(cmn_clockevent.name);
    clkevt_reload = DIV_ROUND_CLOSEST(rate, HZ);
    cmn_clockevent.mult = div_sc(rate, NSEC_PER_SEC, cmn_clockevent.shift);
    if(cmn_enabled)
	cmn_set_mode(CLOCK_EVT_MODE_PERIODIC, &cmn_clockevent);
//	printk(KERN_ERR "set cmn: %ld\n", rate);
}
#endif

int cmn_timer_resume(void) {
	cmn_set_mode(CLOCK_EVT_MODE_PERIODIC, NULL);
	return 0;
}

