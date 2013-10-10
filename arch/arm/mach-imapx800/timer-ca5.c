/*
 *  linux/arch/arm/mach-imapx800/timer-ca5.c
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
#include <linux/slab.h>

#include <asm/hardware/arm_timer.h>
#include <mach/timer-ca5.h>

#include <linux/cpufreq.h>
#include "core.h"

struct clocksource_ca5g {
	void __iomem *reg;
	struct clocksource cs;
	struct notifier_block		freq_transition;
};

struct clocksource_ca5g *ca5g;

static inline struct clocksource_ca5g *to_ca5g(struct clocksource *c)
{
	return container_of(c, struct clocksource_ca5g, cs);
}

static long __time_twist = 100;
static long __init __get_clock_rate(const char *name)
{
	long rate;
	long long tmp;

#if !defined(CONFIG_IMAPX800_FPGA_PLATFORM)
	struct clk *clk;
	int err;

	clk = clk_get(NULL, "gtm-clk");
	if (IS_ERR(clk)) {
	    pr_err("gtimer: %s clock not found: %d\n", name,
		    (int)PTR_ERR(clk));
	    return PTR_ERR(clk);
	}

	err = clk_enable(clk);
	if (err) {
	    pr_err("gtimer: %s clock failed to enable: %d\n", name, err);
	    clk_put(clk);
	    return err;
	}

	rate = clk_get_rate(clk);
	if (rate < 0) {
	    pr_err("gtimer: %s clock failed to get rate: %ld\n", name, rate);
	    clk_disable(clk);
	    clk_put(clk);
	}
//	printk(KERN_ERR "Got ca5-timer rate: %ld\n", rate);

	//rate = 100500000;
#else
	rate =  10000000;
#endif

	tmp = rate * 100ll;
	do_div(tmp, __time_twist);
	return tmp;
}

static cycle_t ca5_gtimer_read(struct clocksource *c)
{   
	unsigned long long  l, h, t;

	/* read counter value */
	t = readl(to_ca5g(c)->reg + G_TIMER_COUNTER_H);
	l = readl(to_ca5g(c)->reg + G_TIMER_COUNTER_L);

	/* incase h32 changed */
	if((h = readl(to_ca5g(c)->reg + G_TIMER_COUNTER_H))
				            != t)
	    l = readl(to_ca5g(c)->reg + G_TIMER_COUNTER_L);

	return (h << 32 | l);
}               

static void __init ca5_gtimer_start(struct clocksource *c, cycle_t n)
{
	/* stop timer */
	writel(0, to_ca5g(c)->reg + G_TIMER_CONTROL);

	/* reset counter value */
	writel((u32)(n & 0xffffffffull),
			to_ca5g(c)->reg + G_TIMER_COUNTER_L);
	writel((u32)(n >> 32), to_ca5g(c)->reg + G_TIMER_COUNTER_H);

	/* enable timer
	 * here we use CPU_PERIPHYCIAL_CLOCK(CPC) directly without
	 * any prescaler. CPC is 100MHz approximately, thus 5,000
	 * years is need to make this timer exceed.
	 */
	writel(1, to_ca5g(c)->reg + G_TIMER_CONTROL);
}

#ifdef CONFIG_PM
static cycle_t ca5_cycles;

static void ca5_gtimer_suspend(struct clocksource *cs)
{
	printk("ca5_gtimer_suspend\n");
	ca5_cycles = ca5_gtimer_read(cs);
}

static void ca5_gtimer_resume(struct clocksource *cs)
{
	printk("ca5_gtimer_resume\n");
	ca5_gtimer_start(cs, ca5_cycles);
}
#endif

#ifdef CONFIG_CPU_FREQ

extern void twd_timer_update(long rate);
extern void cmn_timer_update(long rate);
void ca5_timer_update(long rate)
{
	//	printk(KERN_ERR "ca5 timer frequency updated (%ld Hz).\n", rate);
	//	printk(KERN_ERR "prechange: mult: %d, shift: %d\n", ca5g->cs.mult,
	//		ca5g->cs.shift);
	__clocksource_updatefreq_hz(&ca5g->cs, rate);
	if(ca5g->cs.mult == 0)
		BUG();
	timekeeping_notify(&ca5g->cs);
//	printk(KERN_ERR "set ca5: %ld\n", rate);
	//	printk(KERN_ERR "aftchange: mult: %d, shift: %d\n", ca5g->cs.mult,
	//		ca5g->cs.shift);
}

void timers_update(void *data)
{
	twd_timer_update((long)data);

	return ;
}

void imap_time_twist(int t)
{
	long rate;
	if(t < 10 || t > 200){
		printk(KERN_ERR "<<< time twist region [10, 200]\n");
		return;
	}
	__time_twist = t;
		
	rate = __get_clock_rate(ca5g->cs.name);
	ca5_timer_update(rate);
	cmn_timer_update(rate);
	smp_call_function_single(0, timers_update, (void *)rate, 1);
	smp_call_function_single(1, timers_update, (void *)rate, 1);
}
EXPORT_SYMBOL(imap_time_twist);


static int ca5_gtimer_cpufreq_transition(struct notifier_block *nb,
					     unsigned long val, void *data)
{
    struct cpufreq_freqs *freqs = data;
    long rate;

	if(!ca5g) {
		printk(KERN_ERR "ca5 timer transation: wrong.\n");
		return -EFAULT;
	}

	rate = __get_clock_rate(ca5g->cs.name);
	if (rate < 0)
	    return -EFAULT;

	if (val == CPUFREQ_POSTCHANGE || val == CPUFREQ_RESUMECHANGE) {
		if(freqs->cpu == 0) {
			ca5_timer_update(rate);
			cmn_timer_update(rate);
		}

		smp_call_function_single(freqs->cpu, timers_update,
				(void *)rate, 1);
	}

    return NOTIFY_OK;
}

static inline int ca5_gtimer_cpufreq_register(void)
{
    ca5g->freq_transition.notifier_call = ca5_gtimer_cpufreq_transition;
    return cpufreq_register_notifier(&ca5g->freq_transition,
	    CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void ca5_gtimer_cpufreq_deregister(void)
{
    cpufreq_unregister_notifier(&ca5g->freq_transition,
	    CPUFREQ_TRANSITION_NOTIFIER);
}

static int __init ca5_register_freqfb(void)
{
    return ca5_gtimer_cpufreq_register();
}

core_initcall(ca5_register_freqfb);
#else
static inline int ca5_gtimer_cpufreq_register(void) { return 0; }
static inline void ca5_gtimer_cpufreq_deregister(void) { }
#endif


int ca5_gtimer_init(void __iomem *reg, const char *name)
{
	long rate = __get_clock_rate(name);

	if (rate < 0)
		return -EFAULT;

	ca5g = kzalloc(sizeof(struct clocksource_ca5g), GFP_KERNEL);
	if (!ca5g)
	  return -ENOMEM;

	ca5g->reg       = reg;
	ca5g->cs.name   = name;
	ca5g->cs.rating = 300;
	ca5g->cs.mask   = CLOCKSOURCE_MASK(64);
	ca5g->cs.flags  = CLOCK_SOURCE_IS_CONTINUOUS;
	ca5g->cs.read   = ca5_gtimer_read;
#if 1
	ca5g->cs.suspend = ca5_gtimer_suspend;
	ca5g->cs.resume  = ca5_gtimer_resume;
#endif

	/* start at count zero */
	ca5_gtimer_start(&ca5g->cs, 0);

	core_msg("%s registered\n", name);
	return clocksource_register_hz(&ca5g->cs, rate);
}

