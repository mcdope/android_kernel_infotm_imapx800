/* arch/arm/mach-imapx800/power-gate.c
 * 
 * control each module power gate
 *
 * by Larry Liu
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/spinlock.h>

#include <asm/mach/map.h>
#include <mach/hardware.h>
#include <mach/power-gate.h>
#include <mach/imap-iomap.h>

//#define IMAP_POWER_ADDR_OFF      (IO_ADDRESS(IMAP_SYSMGR_BASE))

#define __pow_readl(x)           __raw_readl(IO_ADDRESS(x))
#define __pow_writel(val, x)     __raw_writel(val, IO_ADDRESS(x))

static DEFINE_SPINLOCK(power_gate_lock);

#define POWER_TIMEOUT_MS    0xff

/*
 * module reset
 *
 * module addr : module pa addr
 * reset_set : in some module there are more than 1 bit to control sub modules to reset
 */
int module_reset(uint32_t module_addr, uint32_t reset_sel)
{
    unsigned long flags;
    int i = 5; 
	spin_lock_irqsave(&power_gate_lock, flags);

    __pow_writel(reset_sel, SYS_SOFT_RESET(module_addr));

	while(i--); //add 6 clk to wait reset valid by ayakashi @20120824
	__pow_writel(DISABLE, SYS_SOFT_RESET(module_addr));

	spin_unlock_irqrestore(&power_gate_lock, flags);

	return 0;
}

EXPORT_SYMBOL(module_reset);

/*
 * module power on
 *
 * module addr : should be the module off addr in the system manager
 * 
 * here we open all the clocks
 *
 * return : 1 means timeout error
 */
int module_power_on(uint32_t module_addr)
{
	uint32_t ret = 0;
	unsigned long flags;
	unsigned long timeout = 0;

	spin_lock_irqsave(&power_gate_lock, flags);
	/* module reset */
	__pow_writel(0xffffffff, SYS_SOFT_RESET(module_addr));
	/* close bus */
	__pow_writel(DISABLE, SYS_BUS_MANAGER(module_addr));
	/* enable clk */
	__pow_writel(0xffffffff, SYS_CLOCK_GATE_EN(module_addr));
	/* power up */
	__pow_writel(MODULE_POWERON, SYS_POWER_CONTROL(module_addr));

	ret = __pow_readl(SYS_POWER_CONTROL(module_addr));
	while(!(ret & (1 << MODULE_POWERON_ACK))){
		ret = __pow_readl(SYS_POWER_CONTROL(module_addr));
        timeout++;
		if(timeout == POWER_TIMEOUT_MS)
		  goto err;
	}
	/* set isolation module output zero */
	__pow_writel(MODULE_ISO_CLOSE, SYS_POWER_CONTROL(module_addr));
	/* bus signal isolation en */
	__pow_writel(ENABLE, SYS_BUS_ISOLATION_R(module_addr));
	/* enable bus */
	__pow_writel(MODULE_BUS_ENABLE, SYS_BUS_MANAGER(module_addr));
    __pow_writel(DISABLE, SYS_BUS_QOS_MANAGER1(module_addr));
	/* release reset */
	__pow_writel(DISABLE, SYS_SOFT_RESET(module_addr));
	
    spin_unlock_irqrestore(&power_gate_lock, flags);

    return 0;	
err:
	spin_unlock_irqrestore(&power_gate_lock, flags);
	return 1;
}

EXPORT_SYMBOL(module_power_on);

/*
 * module ce
 */
int module_power_down(uint32_t module_addr)
{
    uint32_t ret = 0;
    unsigned long flags;
	unsigned long timeout = 0;

	spin_lock_irqsave(&power_gate_lock, flags);

	/* module reset */
	__pow_writel(0xffffffff, SYS_SOFT_RESET(module_addr));
	/* close bus */
	__pow_writel(DISABLE, SYS_BUS_MANAGER(module_addr));
    __pow_writel(DISABLE, SYS_BUS_ISOLATION_R(module_addr));
    __pow_writel(ENABLE, SYS_BUS_QOS_MANAGER1(module_addr)); 
	/* power down */
	__pow_writel(MODULE_POWERDOWN, SYS_POWER_CONTROL(module_addr));
	/* wait for power */
	ret = __pow_readl(SYS_POWER_CONTROL(module_addr));
	while(ret & (1 << MODULE_POWERON_ACK)){
		ret = __pow_readl(SYS_POWER_CONTROL(module_addr));
		timeout++;
		if(timeout == POWER_TIMEOUT_MS)
		  goto error;
	}
	/* release reset */
	__pow_writel(DISABLE, SYS_SOFT_RESET(module_addr));

	spin_unlock_irqrestore(&power_gate_lock, flags);

	return 0;

error:
	/* release reset */
	__pow_writel(DISABLE, SYS_SOFT_RESET(module_addr));

	spin_unlock_irqrestore(&power_gate_lock, flags);
	return 1;
}

EXPORT_SYMBOL(module_power_down);

#define CONFIG_CORE_NUMBER 2
void reset_core(int index, uint32_t hold_base)
{   
	uint32_t tmp;

	if(index < 0 || index >= CONFIG_CORE_NUMBER)
	  return ;

	if(hold_base & 0xff) {
		printk("core cold base must be 256B aligned.\n");
		return ;
	}

	/* set hold base address */
	writel((hold_base >> 8) & 0xff, IO_ADDRESS(SYSMGR_RTC_BASE)	+ 0x30);
	writel((hold_base >> 16) & 0xff, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x34);
	writel((hold_base >> 24) & 0xff, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x38);
	writel(0x3b, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x2c);

	/* set hold base init */
	writel(0, IO_ADDRESS(hold_base) + index * 4);

	/* reset core */
	tmp = readl(IO_ADDRESS(SYSMGR_CPU_BASE));
	writel(tmp & ~(1 << (4 + index)), IO_ADDRESS(SYSMGR_CPU_BASE));
	udelay(20);
	writel(tmp |  (1 << (4 + index)), IO_ADDRESS(SYSMGR_CPU_BASE));

	/* wait core ready */
	while(!readl(IO_ADDRESS(hold_base) + index * 4));

	/* clear hold base */
	writel(0, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x2c);
	writel(0, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x30);
	writel(0, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x38);
}

EXPORT_SYMBOL(reset_core);

void imap_reset_module(void)
{
        __raw_writel(0xff, IO_ADDRESS(SYSMGR_CRYPTO_BASE));
}
