
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/io.h>
#include <linux/input.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/smp_twd.h>

#include <asm/hardware/iomd.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/hardware/gic.h>

#include <asm/mach/map.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <mach/hardware.h>
#include <mach/idle.h>
#include <mach/timer-cmn.h>
#include <mach/timer-ca5.h>
#include <mach/power-gate.h>
#include <mach/items.h>
#include <mach/io.h>
#include <mach/clock.h>
#include <mach/imap-rtc.h>
#include <mach/belt.h>

#include "core.h"

int ctp_auto_detect = 0;
int sensor_auto_detect = 0;

extern void rtc_set_delayed_alarm(u32 delay);
extern void imapfb_shutdown(void);
extern int imap_set_retry_param_default(void);
extern int read_rtc_gpx(int io_num);

int  __charger_in(void)
{
	return !!read_rtc_gpx(2);
}

static int __abnmbt = 0;

int imap_unexpect_shut(void)
{
    return __abnmbt;
}
EXPORT_SYMBOL(imap_unexpect_shut);

void imap_reset(int type)
{
    imapfb_shutdown();

    writel(type, IO_ADDRESS(SYSMGR_RTC_BASE + 0x3c));
    printk(KERN_EMERG "sysreboot: %s\n", (type == 2)?
		    "charger": ((type == 1)? "recovery":
			    "normal"));

    writel(0x1, IO_ADDRESS(SYSMGR_RTC_BASE + 0x2c));
    writel(0x1, IO_ADDRESS(SYSMGR_RTC_BASE + 0x44));
    
    imap_set_retry_param_default();
    
    writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE));
    while(1);
}
EXPORT_SYMBOL(imap_reset);

void imap_shutdown(void)
{
    int try_unmask = 1;

    /* as knowing: there are two bug during shuting down
     *
     * 1: charger power mask on && charger in
     *    enter bug state whether soft or force shutting down
     * 2: charger power mask on && charger not in
     *    enter bug state if force shutting down
     *
     * so we must use a trick to avoid the machine from entering
     * bug state if user wants to shut down in these situations.
     *
     * charger.pwron is to be depracated as it's not a standard
     * method to perform charging
     */

    if(item_exist("charger.enable") && item_equal("charger.enable", "1", 0)) {
	    if(__charger_in()) {
		    imap_reset(2);
	    }
    }
    else if (item_exist("charger.pwron") && item_equal("charger.pwron", "1", 0)) {
    }
    else {
	    try_unmask = 0;
    }

 
    if(belt_scene_get() & SCENE_RTC_WAKEUP) {
	    printk(KERN_ERR "prepare RTC power on.\n");

	    writel(0xff, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x04);
	    writel(0x1f, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x28);
	    writel(readl(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60) | 0x2,
			    IO_ADDRESS(SYSMGR_RTC_BASE) + 0x60);
	    rtc_set_delayed_alarm(belt_get_rtc());
	    writel(readl(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x78) | 0x4,
			    IO_ADDRESS(SYSMGR_RTC_BASE) + 0x78);
    } else {
	    writel(0xff, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x04);
	    writel(readl(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x78) & ~0x4,
			    IO_ADDRESS(SYSMGR_RTC_BASE) + 0x78);
    }

    if(try_unmask && !__charger_in())
	    writel(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x60)) & ~0x2,
		    IO_ADDRESS(SYSMGR_RTC_BASE + 0x60));

    /* shut down */
    __raw_writel(0xff, IO_ADDRESS(SYSMGR_RTC_BASE + 0x28));
    __raw_writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0x7c));
    __raw_writel(0x2, IO_ADDRESS(SYSMGR_RTC_BASE));

    asm("wfi");
}

void imap_powerkey_antishake(void)
{
    uint32_t pwkey_stat = 0 ;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        pwkey_stat = 0x10;
    }else if(item_equal("board.cpu", "i15", 0)){
        pwkey_stat = 0x40;
    }   


    if(readl(IO_ADDRESS(SYSMGR_RTC_BASE) + 0x20) & 0x4) {
		// power on by RTC
		writel(0x4, IO_ADDRESS(SYSMGR_RTC_BASE) + 0x24);
		return;
	}

    if(item_exist("board.antishake")) {
	if(item_integer("board.antishake", 0)) {
	    printk(KERN_ERR "antishake: check power key state ...\n");

	    if(strstr(boot_command_line, "charger") ||
		    strstr(boot_command_line, "recovery") ||
		    readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x44)) & 1 /* INFO6 */) {
		printk(KERN_ERR "Not cold boot, continue boot.\n");
		writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0x44));
		return ;
	    }


	    if(!(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x58)) & pwkey_stat)
                && !__charger_in()) {
		printk(KERN_ERR "No power key, shutdown.\n");
		imap_shutdown();
	    } else
		printk(KERN_ERR "continue boot.\n");
	}
    }
}
EXPORT_SYMBOL(imap_powerkey_antishake);

#ifdef CONFIG_CACHE_L2X0
void __init imap_init_l2x0(void)
{
    int lt, ld; // latency_tag, latency_data
    void __iomem *base = IO_ADDRESS(IMAP_SCU_BASE) + 0x2000;

    /* 256KB (16KB/way), 16-way associativity,
     * evmon/parity/share enabled
     * Bits:  .... ...0 0111 0011 0000 .... .... .... */
    writel(0x2, IO_ADDRESS(IMAP_SYSMGR_BASE + 0x884c));

    lt  = readl_relaxed(base + L2X0_TAG_LATENCY_CTRL);
    lt &= ~(0x7<<8 | 0x7<<4 | 0x7);
    lt |=  (0x0<<8 | 0x1<<4 | 0x0);
    writel_relaxed(lt, base + L2X0_TAG_LATENCY_CTRL);

    ld  = readl_relaxed(base + L2X0_DATA_LATENCY_CTRL);
    ld &= ~(0x7<<8 | 0x7<<4 | 0x7);
    ld |=  (0x0<<8 | 0x1<<4 | 0x0);
    writel_relaxed(ld, base + L2X0_DATA_LATENCY_CTRL);

    /* common l2x0 init */
	ld  = readl_relaxed(base + L2X0_PREFETCH_CTRL);
    ld |=  0xffffff00;
    writel_relaxed(ld, base + L2X0_PREFETCH_CTRL);

    /* common l2x0 init */
    l2x0_init(base, 0x70730000, 0xfe000fff);
}
#endif


static void __init imap_init_machine(void)
{
    imap_reset_clock();
    imap_reset_module();
#ifdef CONFIG_CACHE_L2X0
    imap_init_l2x0();
#endif	
    imap_init_devices();
    imap_init_gpio();
}

static struct map_desc imap_iodesc[] __initdata = {
    {
	.virtual	= (unsigned long)IMAP_SYSMGR_VA,
	.pfn		= __phys_to_pfn(IMAP_SYSMGR_BASE),
	.length		=  IMAP_SYSMGR_SIZE,
	.type		= MT_DEVICE,
    },{
	.virtual	= (unsigned long)IMAP_SCU_VA,
	.pfn		= __phys_to_pfn(IMAP_SCU_BASE),
	.length		= IMAP_SCU_SIZE,
	.type           = MT_DEVICE,
    },{
	.virtual	= (unsigned long)IMAP_TIMER_VA,
	.pfn		= __phys_to_pfn(IMAP_TIMER_BASE),
	.length		= IMAP_TIMER_SIZE,
	.type           = MT_DEVICE,
    },{
	.virtual	= (unsigned long)IMAP_IRAM_VA,
	.pfn		= __phys_to_pfn(IMAP_IRAM_BASE),
	.length		= IMAP_IRAM_SIZE,
	.type           = MT_DEVICE,
    },{
	.virtual	= (unsigned long)IMAP_EMIF_VA,
	.pfn		= __phys_to_pfn(IMAP_EMIF_BASE),
	.length		= IMAP_EMIF_SIZE,
	.type		= MT_DEVICE,
    },
};

static void __init imap_map_io(void) {
    iotable_init(imap_iodesc, ARRAY_SIZE(imap_iodesc));
    imap_init_clocks();
}

static void __init imap_init_early(void) {
    //reset_APLL();
    item_init(IO_ADDRESS(IMAP_IRAM_BASE), ITEM_SIZE_NORMAL);
    imap_mem_reserve();

    if(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x44)) & 2) {
        __abnmbt = 1;
        writel(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x44)) & ~0x2,
                IO_ADDRESS(SYSMGR_RTC_BASE + 0x44));
    }

    /* change iMAPx820 case number according to config */
    if(item_exist("board.case")) {
	int _c = item_integer("board.case", 0);
	printk(KERN_ERR "Set iMAPx820 case to: %d\n", _c); 
	writel(_c, IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000));
    }
}

static void __init imap_init_irq(void) {
    
    /* start from 29 to enable local timer */
    gic_init(0, 29, IO_ADDRESS(IMAP_GIC_DIST_BASE),
	    IO_ADDRESS(IMAP_GIC_CPU_BASE));
}

static void __init imap_init_timer(void)
{
#ifdef CONFIG_LOCAL_TIMERS
    twd_base = IO_ADDRESS(SCU_PRIVATE_TIMER);
#endif

    /* very good clock source */
    ca5_gtimer_init(IO_ADDRESS(SCU_GLOBAL_TIMER),
	    "timer-ca5");

    /* lowlevel clock event */
    module_power_on(SYSMGR_CMNTIMER_BASE);
    cmn_timer_init(IO_ADDRESS(IMAP_TIMER_BASE),
	    GIC_CMNT0_ID, "imap-cmn-timer");
}

extern int cmn_timer_resume(void);
static void imap_resume_timer(void) {
	module_power_on(SYSMGR_CMNTIMER_BASE);
	cmn_timer_resume();
};

#ifdef CONFIG_LOCAL_TIMERS
int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
    /* very good clock event: will be used for hres */
    evt->irq = IRQ_LOCALTIMER;
    twd_timer_setup(evt);
    return 0;
}
#endif

static struct sys_timer imap_timer = {
    .init   = imap_init_timer,
	.resume = imap_resume_timer,
};

static void __init imap_fixup(struct machine_desc *desc, struct tag *tags,
	char **cmdline, struct meminfo *mi)
{
    mi->nr_banks = 1;
    mi->bank[0].start = IMAP_SDRAM_BASE;
    mi->bank[0].size = 1024*1024*1024;
}

MACHINE_START(IMAPX800, "iMAPx800")
    .boot_params  =  IMAP_SDRAM_BASE + 0x100,
    .init_irq     =  imap_init_irq,
    .init_early   =  imap_init_early,
    .init_machine =  imap_init_machine,
    .map_io       =  imap_map_io,
    .fixup        =  imap_fixup,
    .timer        = &imap_timer,
MACHINE_END

