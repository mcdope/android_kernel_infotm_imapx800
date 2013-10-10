/*
 * kernel/power/suspend.c - Suspend to RAM and standby functionality.
 *
 * Copyright (c) 2003 Patrick Mochel
 * Copyright (c) 2003 Open Source Development Lab
 * Copyright (c) 2009 Rafael J. Wysocki <rjw@sisk.pl>, Novell Inc.
 *
 * This file is released under the GPLv2.
 */

#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include <trace/events/power.h>
#include <linux/wakelock.h>
#include "power.h"

#include <mach/imap-pwm.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/pad.h>

/********************Added by Infotm SHANGHAI***********************/
#ifdef	CONFIG_FAKE_PM
#include <asm/io.h>

extern struct wake_lock anti_suspend;
int if_in_suspend ;
EXTERN_SYMBOL(if_in_suspend);
#endif
/*******************************************************************/

/*deleted by bob for debug*/
extern int imap_iokey_emu(int index);
extern int imapx_gpio_spken(int en) ;
extern int rt5631_flag;

const char *const pm_states[PM_SUSPEND_MAX] = {
#ifdef CONFIG_EARLYSUSPEND
	[PM_SUSPEND_ON]		= "on",
#endif
	[PM_SUSPEND_STANDBY]	= "standby",
	[PM_SUSPEND_MEM]	= "mem",
};
//#if defined(CONFIG_RESUME_BLOCK_DEBUG)
#if 1
static int block_times = 0;
struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
#define IMAP_RTC_ACCESS_BCD    0x01
#define IMAP_RTC_ACCESS_ALM    0x02
#define IMAP_RTC_ACCESS_READ   0x10
#define IMAP_RTC_ACCESS_WRITE  0x20
extern void __imap_rtc_access_debug(struct rtc_time *tm);
int rd_get_time(void)
{
	struct rtc_time tm;

	__imap_rtc_access_debug(&tm);

	return ((tm.tm_hour * 3600) + (tm.tm_min * 60) + tm.tm_sec);
}
EXPORT_SYMBOL(rd_get_time);

long rd_get_time_ext(void)
{
	struct rtc_time tm;

	__imap_rtc_access_debug(&tm);

	return ((tm.tm_hour * 3600) + (tm.tm_min * 60) + tm.tm_sec);
}
EXPORT_SYMBOL(rd_get_time_ext);

#endif

static const struct platform_suspend_ops *suspend_ops;

/**
 *	suspend_set_ops - Set the global suspend method table.
 *	@ops:	Pointer to ops structure.
 */
void suspend_set_ops(const struct platform_suspend_ops *ops)
{
	mutex_lock(&pm_mutex);
	suspend_ops = ops;
	mutex_unlock(&pm_mutex);
}

bool valid_state(suspend_state_t state)
{
	/*
	 * All states need lowlevel support and need to be valid to the lowlevel
	 * implementation, no valid callback implies that none are valid.
	 */
	return suspend_ops && suspend_ops->valid && suspend_ops->valid(state);
}

/**
 * suspend_valid_only_mem - generic memory-only valid callback
 *
 * Platform drivers that implement mem suspend only and only need
 * to check for that in their .valid callback can use this instead
 * of rolling their own .valid callback.
 */
int suspend_valid_only_mem(suspend_state_t state)
{
	return state == PM_SUSPEND_MEM;
}

static int suspend_test(int level)
{
#ifdef CONFIG_PM_DEBUG
	if (pm_test_level == level) {
		printk(KERN_INFO "suspend debug: Waiting for 5 seconds.\n");
		mdelay(5000);
		return 1;
	}
#endif /* !CONFIG_PM_DEBUG */
	return 0;
}

/**
 *	suspend_prepare - Do prep work before entering low-power state.
 *
 *	This is common code that is called for each state that we're entering.
 *	Run suspend notifiers, allocate a console and stop all processes.
 */
static int suspend_prepare(void)
{
	int error;

	if (!suspend_ops || !suspend_ops->enter)
		return -EPERM;

	pm_prepare_console();

	error = pm_notifier_call_chain(PM_SUSPEND_PREPARE);
	if (error)
		goto Finish;

	error = usermodehelper_disable();
	if (error)
		goto Finish;

	error = suspend_freeze_processes();
	if (!error)
		return 0;

	suspend_thaw_processes();
	usermodehelper_enable();
 Finish:
	pm_notifier_call_chain(PM_POST_SUSPEND);
	pm_restore_console();
	return error;
}

/* default implementation */
void __attribute__ ((weak)) arch_suspend_disable_irqs(void)
{
	local_irq_disable();
}

/* default implementation */
void __attribute__ ((weak)) arch_suspend_enable_irqs(void)
{
	local_irq_enable();
}

/**
 *	suspend_enter - enter the desired system sleep state.
 *	@state:		state to enter
 *
 *	This function should be called after devices have been suspended.
 */
static int suspend_enter(suspend_state_t state)
{
	int error;

	if (suspend_ops->prepare) {
		error = suspend_ops->prepare();
		if (error)
			goto Platform_finish;
	}

	error = dpm_suspend_noirq(PMSG_SUSPEND);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to power down\n");
		goto Platform_finish;
	}

	if (suspend_ops->prepare_late) {
		error = suspend_ops->prepare_late();
		if (error)
			goto Platform_wake;
	}

	if (suspend_test(TEST_PLATFORM))
		goto Platform_wake;

	error = disable_nonboot_cpus();
	if (error || suspend_test(TEST_CPUS))
		goto Enable_cpus;

	arch_suspend_disable_irqs();
	BUG_ON(!irqs_disabled());

	error = syscore_suspend();
	if (!error) {
		if (!(suspend_test(TEST_CORE) || pm_wakeup_pending())) {
			error = suspend_ops->enter(state);
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
		{
			int resume_time = 0;
			while (!resume_time) {
				resume_time = rd_get_time();
			}
		}
#endif
			events_check_enabled = false;
		}
		syscore_resume();
	}

	arch_suspend_enable_irqs();
	BUG_ON(irqs_disabled());

 Enable_cpus:
	enable_nonboot_cpus();

 Platform_wake:
	if (suspend_ops->wake)
		suspend_ops->wake();

	dpm_resume_noirq(PMSG_RESUME);

 Platform_finish:
	if (suspend_ops->finish)
		suspend_ops->finish();

	return error;
}

/**
 *	suspend_devices_and_enter - suspend devices and enter the desired system
 *				    sleep state.
 *	@state:		  state to enter
 */
int suspend_devices_and_enter(suspend_state_t state)
{
	int error;
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	int resume_time = 0;
#endif
	gfp_t saved_mask;

	if (!suspend_ops)
		return -ENOSYS;

	trace_machine_suspend(state);
	if (suspend_ops->begin) {
		error = suspend_ops->begin(state);
		if (error)
			goto Close;
	}
	suspend_console();
	suspend_test_start();
	error = dpm_suspend_start(PMSG_SUSPEND);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to suspend\n");
		goto Recover_platform;
	}
	suspend_test_finish("suspend devices");
	if (suspend_test(TEST_DEVICES))
		goto Recover_platform;

	error = suspend_enter(state);

 Resume_devices:
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	while (!resume_time) {
		resume_time = rd_get_time();
	}
#endif
	suspend_test_start();
	dpm_resume_end(PMSG_RESUME);
	suspend_test_finish("resume devices");
	resume_console();
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	resume_time = rd_get_time() - resume_time;
	printk(KERN_ALERT "[RESUME DEBUG] All devices resume time cost %ds.\n", resume_time);
#endif

Close:
	if (suspend_ops->end)
		suspend_ops->end();
	trace_machine_suspend(PWR_EVENT_EXIT);
	return error;

 Recover_platform:
	if (suspend_ops->recover)
		suspend_ops->recover();
	goto Resume_devices;
}

/**
 *	suspend_finish - Do final work before exiting suspend sequence.
 *
 *	Call platform code to clean up, restart processes, and free the
 *	console that we've allocated. This is not called for suspend-to-disk.
 */
static void suspend_finish(void)
{
	suspend_thaw_processes();
	usermodehelper_enable();
	pm_notifier_call_chain(PM_POST_SUSPEND);
	pm_restore_console();
}

/********************Added by Infotm SHANGHAI***********************/
#ifdef CONFIG_FAKE_PM
struct completion     power_key;
EXTERL_SYMBOL(power_key);
extern int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long gtcmp);
extern int imap_pwm_suspend(struct sys_device *pdev, pm_message_t pm);
extern int imap_pwm_resume(struct sys_device *pdev);
#endif
/******************************************************************/

/**
 *	enter_state - Do common work of entering low-power state.
 *	@state:		pm_state structure for state we're entering.
 *
 *	Make sure we're the only ones trying to enter a sleep state. Fail
 *	if someone has beat us to it, since we don't want anything weird to
 *	happen when we wake up.
 *	Then, do the setup for suspend, enter the state, and cleaup (after
 *	we've woken up).
 */
int enter_state(suspend_state_t state)
{
#ifdef CONFIG_FAKE_PM
	unsigned int temp;

	printk("PM: Syncing filesystems ... ");

	init_completion(&power_key);

	imap_pwm_suspend(NULL, PMSG_SUSPEND);

	imapx_pad_cfg(IMAPX_PWM, 0);

	if_in_suspend = 1;

	wait_for_completion(&power_key);

	printk("PM: Finishing wakeup.\n");

	if_in_suspend = 0;

	imap_pwm_resume(NULL);

	imapx_pad_cfg(IMAPX_PWM, 1);

	return 0;
#else
	int error;
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	int rd_time_interval = 0;
#endif

	if (!valid_state(state))
		return -ENODEV;

	if (!mutex_trylock(&pm_mutex))
		return -EBUSY;

	printk(KERN_INFO "PM: Syncing filesystems ... ");
	sys_sync();
	printk("done.\n");

	pr_debug("PM: Preparing system for %s sleep\n", pm_states[state]);
	error = suspend_prepare();
	if (error)
		goto Unlock;

	if (suspend_test(TEST_FREEZER))
		goto Finish;

	pr_debug("PM: Entering %s sleep\n", pm_states[state]);
	pm_restrict_gfp_mask();
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	rd_time_interval = rd_get_time();
#endif
	error = suspend_devices_and_enter(state);
	pm_restore_gfp_mask();

 Finish:
	pr_debug("PM: Finishing wakeup.\n");
	suspend_finish();
#ifdef CONFIG_FAKE_PM
	wake_lock_timeout(&anti_suspend,HZ * 15);
	/*deleted by bob for debug*/
#endif
	imap_iokey_emu(2);
    imapx_gpio_spken(0);
    rt5631_flag = 2;
Unlock:
	mutex_unlock(&pm_mutex);
#if defined(CONFIG_RESUME_BLOCK_DEBUG)
	rd_time_interval = rd_get_time() - rd_time_interval;
	printk(KERN_ALERT "[RESUME DEBUG] Time cost from suspend to resume finish, %ds, block times %d.\n\n", 
			rd_time_interval, (rd_time_interval > 20) ? (++block_times) : block_times);
#endif
	return error;
#endif
}

/**
 *	pm_suspend - Externally visible function for suspending system.
 *	@state:		Enumerated value of state to enter.
 *
 *	Determine whether or not value is within range, get state
 *	structure, and enter (above).
 */
int pm_suspend(suspend_state_t state)
{
	if (state > PM_SUSPEND_ON && state <= PM_SUSPEND_MAX)
		return enter_state(state);
	return -EINVAL;
}
EXPORT_SYMBOL(pm_suspend);
