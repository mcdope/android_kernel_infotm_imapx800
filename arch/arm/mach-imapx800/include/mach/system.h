/* arch/arm/mach-imap/include/mach/system.h
 *
 * Copyright (c) 2003 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * IMAPX200 - System function defines and includes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/io.h>
#include <mach/io.h>
#include <mach/idle.h>

void (*imap_idle)(void);
void (*imap_reset_hook) (void);

void imap_default_idle(void)
{
	/* idle the system by using the idle mode which will wait for an
	 * interrupt to happen before restarting the system.
	 */

	/* Warning: going into idle state upsets jtag scanning */
//    printk(KERN_ERR "idle: \n");
       asm("wfi");
}

static void arch_idle(void)
{
	if (imap_idle != NULL)
		(imap_idle)();
	else
		imap_default_idle();
}

//#include <mach/system-reset.h>
static void arch_reset(char mode, const char *cmd)
{
	printk("enter func %s, at line %d \n", __func__, __LINE__);
}

extern void imap_shutdown(void);
static void arch_shutdown(void)
{
    imap_shutdown();
}

