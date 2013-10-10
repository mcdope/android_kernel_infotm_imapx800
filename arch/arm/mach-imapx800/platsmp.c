/*
 *  linux/arch/arm/mach-imapx800/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 *  Copyright (C) 2009 Palm
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/hardware/gic.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/smp_scu.h>
#include <asm/unified.h>

#include <mach/imap-iomap.h>
#include <mach/power-gate.h>

#ifdef CONFIG_VFP
#include <asm/vfp.h>
#endif


#define IMAPX800_SYS_PCADDR (IO_ADDRESS(IMAP_IRAM_BASE) + 0x17004)
#define IMAPX800_SYS_CALLED_1 (IO_ADDRESS(IMAP_IRAM_BASE) + 0x1700c)
#define IMAPX800_RESET_STATE	(IO_ADDRESS(SYSMGR_SYSMGR_BASE) + 0x10)
static DEFINE_SPINLOCK(boot_lock);
static void __iomem *scu_base = (void __iomem *)IMAP_SCU_VA;
extern void versatile_secondary_startup(void);
extern void imapx800_cpu_resume(void);

volatile int pen_release = -1;

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	printk("platform_secondary_init\n");
	gic_secondary_init(0);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}
#define SWIRL_POINT 0x3c017000
int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    	unsigned long timeout;
	volatile int i = 0;
	volatile unsigned int tmp = 0;

	printk("boot_secondary\n");
	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	reset_core(1, SWIRL_POINT);
	/* put the old boot vector back */

	writel(BSYM(virt_to_phys(versatile_secondary_startup)),
				IMAPX800_SYS_PCADDR);
	smp_wmb();
	flush_cache_all();
	//outer_flush_all();	
	//outer_inv_all();	

#if 0
	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
	}
#endif

	// wait 50us to make sure core1 jumped away
	udelay(50);

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);
	printk("boot_secondary --\n");
	return 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
void __init smp_init_cpus(void)
{
	unsigned int i, ncores = scu_get_core_count(scu_base);

	if (ncores > NR_CPUS) {
		printk(KERN_ERR "Tegra: no. of cores (%u) greater than configured (%u), clipping\n",
			ncores, NR_CPUS);
		ncores = NR_CPUS;
	}

	for (i = 0; i < ncores; i++)
		cpu_set(i, cpu_possible_map);

	set_smp_cross_call(gic_raise_softirq);
}

void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
	int i;
	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

	scu_enable(scu_base);
}
