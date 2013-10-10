/***************************************************************************** 
 * /linux/arch/arm/mach-imapx200/cpu.c
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Inialization of the board related fuctions and devices.
 * **
 * ** Author:
 * **     Alex Zhang   <tao.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.2  25/11/2009  Alex Zhang   
 * *****************************************************************************/ 

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/input.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/hardware/iomd.h>
#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/proc-fns.h>
//#include <asm/cpu-single.h>
#include <plat/clock.h>
#include <mach/cpu.h>
#include <mach/devs.h>
#include <plat/plat_functions.h>
#include <mach/idle.h>
#include <plat/plat_functions.h>
#include <mach/regs-serial.h>
#include <mach/pm.h>
#include <linux/amba/bus.h>

#define FPGA_TEST



#define DEF_MCR	IMAPX200_MCR_SIRE_IRDA_DISABLE | IMAPX200_MCR_AFCE_AFC_DISABLE
#define DEF_LCR IMAPX200_LCR_DLS_8BIT |IMAPX200_LCR_STOP_ONE_STOP_BIT |IMAPX200_LCR_PEN_PARITY_DISABLE
#define DEF_FCR  IMAPX200_FCR_FIFOE_FIFO_ENABLE | IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_EMPTY | IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_ONE_CHAR 

extern void imap_mem_reserve(void);
extern struct sys_timer imapx200_init_timer;

static struct platform_device *imapx200_devices[] __initdata = {
	&imapx200_device_nand,	
	&imapx200_device_lcd,
	&imapx200_device_pwm,
	&imapx200_device_bl,
	&imapx200_button_device,
	&imapx200_device_sdi0,
	&imapx200_device_sdi1,
	&imapx200_device_sdi2,
	&imapx200_device_cf,
	&imapx200_device_usbhost20,
	&imapx200_device_usbhost11,
	&imapx200_device_usbotg,
	&imapx200_device_udc,
	&imapx200_device_camera,
#ifdef CONFIG_RGB2VGA_OUTPUT_SUPPORT
	&imapx200_device_rgb2vga,
#endif
#ifdef CONFIG_HDMI_EP932
	&imapx200_device_hdmi_ep932,
#endif
#ifdef CONFIG_HDMI_SI9022
	&imapx200_device_hdmi_si9022,
#endif
	&imapx200_device_mac,
	&imapx200_device_rda5868,
	&imapx200_device_venc,
	&imapx200_device_vdec,
	&imapx200_device_memalloc,
	&imapx200_device_dsp,
	&imapx200_device_gps,
	&imapx200_device_iic0,
	&imapx200_device_iic1,
	&imapx200_device_rtc,
	&imapx200_device_iis,
	&imapx200_device_ac97,
	&imapx200_device_ssim0,
	&imapx200_device_ssim1,
	&imapx200_device_ssim2,
	&imapx200_device_ssis,
	&imapx200_device_spi,
	&imapx200_device_keybd,
	&imapx200_device_pic0,
	&imapx200_device_pic1,
	&imapx200_device_graphic,
	&imapx200_device_orientation,
	&imapx200_device_backlights,
	&imapx200_devcie_watchdog,
	&imapx200_devcie_mtouch,
#ifdef CONFIG_ANDROID_PMEM
	&android_pmem_device,
	&android_pmem_adsp_device,
#endif
#ifdef	CONFIG_ANDROID_TIMED_GPIO
	&android_vibrator_device,
#endif
#ifdef CONFIG_BOSCH_BMA150
	&imapx200_device_BMA150,
#endif
	&android_wifi_switch_device,
#ifdef CONFIG_SWITCH_SENSOR
	&android_sensor_switch_device,
#endif
#ifdef	CONFIG_USB_ANDROID
	&android_usb_device,
#endif	
	&android_device_batt,
};

static struct map_desc imapx200_iodesc[] __initdata = {
};


static struct imapx200_uart_clksrc imapx200_serial_clocks[] = {

	[0] = {
		.name		= "pclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},

#if 0 /* HS UART Source is changed from epll to mpll */
	[1] = {
		.name		= "ext_uart",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},

#if defined (CONFIG_SERIAL_S3C64XX_HS_UART)

	[2] = {
		.name		= "epll_clk_uart_192m",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 4000000,
	}
#endif

#endif

};


static struct imapx200_uartcfg imapx200_uartcfgs[] = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
#if 1
	[3] = {
		.hwport	     = 3,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	}
#endif
};


/* uart management */

static int nr_uarts __initdata = 0;
static struct imapx200_uartcfg uart_cfgs[4];

/* imap_init_uartdevs
 *
 * copy the specified platform data and configuration into our central
 * set of devices, before the data is thrown away after the init process.
 *
 * This also fills in the array passed to the serial driver for the
 * early initialisation of the console.
 */
void __init imap_init_uartdevs(char *name, struct imap_uart_resources *res,
		struct imapx200_uartcfg *cfg, int no)
{
	struct platform_device *platdev;
	struct imapx200_uartcfg *cfgptr = uart_cfgs;
	struct imap_uart_resources *resp;
	int uart;

	memcpy(cfgptr, cfg, sizeof(struct imapx200_uartcfg) * no);

	for (uart = 0; uart < no; uart++, cfg++, cfgptr++)
	{
		platdev = imap_uart_src[cfgptr->hwport];

		resp = res + cfgptr->hwport;

		imap_uart_devs[uart] = platdev;

		platdev->name = name;
		platdev->resource = resp->resources;
		platdev->num_resources = resp->nr_resources;

		platdev->dev.platform_data = cfgptr;
	}

	nr_uarts = no;
}

void __init imapx200_init_uarts(struct imapx200_uartcfg *cfg, int no) 
{
	unsigned long tmp;    
	imap_init_uartdevs("imapx200-uart", imapx200_uart_resources, cfg, no);

	//      imap_device_lcd.name = "imap-lcd";
	//      imap_device_nand.name = "imap-nand";
}


static void imapx200_idle(void)
{
	unsigned long tmp;
	/* ensure our idle mode is to go to idle */

	/* Set WFI instruction to SLEEP mode */
#if 1
	tmp = __raw_readl(rGPOW_CFG);
	tmp &= ~(0x7);
	tmp |= 0x1;
//	__raw_writel(tmp, rGPOW_CFG);
	cpu_do_idle();
#endif
}

struct sysdev_class imapx200_sysclass = { 
	.name           = "imapx200-core",
};

static struct sys_device imapx200_sysdev = { 
	.cls            = &imapx200_sysclass,
};

static int __init imapx200_core_init(void)
{
	return sysdev_class_register(&imapx200_sysclass);
}
core_initcall(imapx200_core_init);

void __init _imapx200_map_io(struct map_desc *mach_desc, int size)
{
	/* register our io-tables */
	iotable_init(mach_desc, size);
	/* rename any peripherals used differing from the s3c2412 */
	imap_idle = imapx200_idle;
}


int __init imapx200_init(void)
{
	int ret;

	printk("imapx200: Initialising architecture\n");

	ret = sysdev_register(&imapx200_sysdev);

	if(ret != 0)
		printk(KERN_ERR "failed to register sysdev for iampx200\n");
	return ret;
}


static void __init imapx200_fixup(struct machine_desc *desc, struct tag *tags,
		char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = 0x40000000;
	mi->bank[0].size = 256*1024*1024;
	mi->bank[0].node = 0;
}

static struct platform_device fsg_platform_device = 
{
	.name = "usb_mass_storage",
	.id = -1,
};

static void __init imapx200_machine_init(void)
{
	platform_add_devices(imapx200_devices, ARRAY_SIZE(imapx200_devices));
	amba_device_register(&imap_ps2_device, &(imap_ps2_device.res));
	(void)platform_device_register(&fsg_platform_device);
}

/* XXX XXX XXX XXX XXX XXX XXX */

/* minimal IO mapping */
static struct map_desc imap_iodesc[] __initdata = {
	{
		.virtual	= (unsigned long)IMAP_VA_GPIO,
		.pfn		= __phys_to_pfn(GPIO_BASE_REG_PA),
		.length		= SZ_1M,
		.type		= MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_IRQ,
		.pfn		= __phys_to_pfn(INTR_BASE_REG_PA),
		.length		=  SZ_1M,
		.type		= MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_UART,
		.pfn		= __phys_to_pfn(UART0_BASE_ADDR),
		.length		= SZ_16K,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_TIMER,
		.pfn		= __phys_to_pfn(TIMER_BASE_REG_PA),
		.length		= SZ_4K,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_SYSMGR,
		.pfn		= __phys_to_pfn(SYSMGR_BASE_REG_PA),
		.length		= SZ_1M,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_FB,
		.pfn		= __phys_to_pfn(LCD_BASE_REG_PA),
		.length		= SZ_16K,
		.type           = MT_DEVICE,
	},
};


struct chip_tab {
	unsigned long	idcode;
	unsigned long	idmask;
	void		(*map_io)(struct map_desc *mach_desc, int size);
	void		(*init_irq)(void);
	void		(*init_uarts)(struct imapx200_uartcfg *cfg, int no);   //uncertain
	void		(*init_clocks)(int xtal);
	int		(*init)(void);
	const char	*name;
};

/* table of supported CPUs */
static const char name_imapx200[]  = "iMAPx200";

extern void __init imapx200_init_clocks(int xtal);
extern void __init imap_plat_init_irq(void);
static struct chip_tab cpu_ids[] __initdata = {
	{
		.idcode         = 0x13AB2000,
		.idmask         = 0xfffffff0,
		.map_io         = _imapx200_map_io,
		.init_irq		= imap_plat_init_irq,
		.init_clocks    = imapx200_init_clocks,
		.init_uarts     = imapx200_init_uarts,
		.init           = imapx200_init,
		.name           = name_imapx200
	},
};

static struct chip_tab * __init imap_lookup_cpu(unsigned long idcode)
{
	struct chip_tab *tab;
	int count;

	tab = cpu_ids;
	for (count = 0; count < ARRAY_SIZE(cpu_ids); count++, tab++) {
		if ((idcode & tab->idmask) == tab->idcode)
			return tab;
	}

	return NULL;
}


/* board information */
static struct imap_board *board;

static struct imap_board imapx200_board __initdata = { 
	.devices        = imapx200_devices,
	.devices_count  = ARRAY_SIZE(imapx200_devices)
};

void imap_set_board(struct imap_board *b)
{
	int i;

	board = b;

	if (b->clocks_count != 0) {
		struct clk **ptr = b->clocks;

		for (i = b->clocks_count; i > 0; i--, ptr++)
			if (imap_register_clock(*ptr) < 0)
				printk(KERN_ERR "failed to register clock.\n");
	}
}


/* cpu information */

static struct chip_tab *cpu;

static unsigned long imap_read_idcode_v6(void)
{
	return 0x13AB2000;
}

static unsigned long imap_read_idcode_v5(void)
{
	return 1UL;	/* don't look like an 2400 */
}

static unsigned long imap_read_idcode_v4(void)
{
	return 0UL;
}

#if 0
void __init imap_init_irq(void)
{
	if (cpu == NULL)
		panic("imap_init_clocks: no cpu setup?\n");

	if (cpu->init_irq == NULL)
		panic("imap_init_clocks: cpu has no clock init\n");
	else
		(cpu->init_irq)();

}
#endif

void __init imap_init_io(struct map_desc *mach_desc, int size)
{
	unsigned long idcode = 0x0;

	/* initialise the io descriptors we need for initialisation */
	iotable_init(imap_iodesc, ARRAY_SIZE(imap_iodesc));

	if (cpu_architecture() >= CPU_ARCH_ARMv6) {
		idcode = imap_read_idcode_v6();
	} else if (cpu_architecture() >= CPU_ARCH_ARMv5) {
		idcode = imap_read_idcode_v5();
	} else if (cpu_architecture() >= CPU_ARCH_ARMv4) {
		idcode = imap_read_idcode_v4();
	} else  {
		panic("Unknown CPU Architecture");
		idcode = 1UL; /* Unknown and error */
	}

	cpu = imap_lookup_cpu(idcode);

	if (cpu == NULL) {
		printk(KERN_ERR "Unknown CPU type 0x%08lx\n", idcode);
		panic("Unknown IMAP CPU");
	}

	printk("CPU %s (id 0x%08lx)\n", cpu->name, idcode);

	if (cpu->map_io == NULL || cpu->init == NULL) {
		printk(KERN_ERR "CPU %s support not enabled\n", cpu->name);
		panic("Unsupported IMAP CPU");
	}

	//	arm_pm_restart = s3c24xx_pm_restart;

	(cpu->map_io)(mach_desc, size);
}

void __init imap_init_uarts(struct imapx200_uartcfg *cfg, int no)
{
	if (cpu == NULL)
		return;

	if (cpu->init_uarts == NULL)
	{
		printk(KERN_ERR "imap_init_uarts: cpu has no uart init\n");
	}
	else
		(cpu->init_uarts)(cfg, no);
}


/* imap_init_clocks
 *
 * Initialise the clock subsystem and associated information from the
 * given master crystal value.
 *
 * xtal  = 0 -> use default PLL crystal value (normally 12MHz)
 *      != 0 -> PLL crystal value in Hz
 */

void __init imap_init_clocks(int xtal)
{
#if defined(FPGA_TEST)
	if (xtal == 0)
		xtal = 12*1000*1000;
#else
	xtal = 40*1000*1000;
#endif
	if (cpu == NULL)
		panic("imap_init_clocks: no cpu setup?\n");

	if (cpu->init_clocks == NULL)
		panic("imap_init_clocks: cpu has no clock init\n");
	else
		(cpu->init_clocks)(xtal);
}

static void __init imapx200_map_io(void)
{
	imap_init_io(imapx200_iodesc, ARRAY_SIZE(imapx200_iodesc));
	imap_init_clocks(0);
	imap_init_uarts(imapx200_uartcfgs, ARRAY_SIZE(imapx200_uartcfgs));
	imap_mem_reserve();
	imap_pm_init();
}

static int __init imap_arch_init(void)
{
	int ret;

	// do the correct init for cpu

	if (cpu == NULL)
		panic("imap_arch_init: NULL cpu\n");

	ret = (cpu->init)();
	if (ret != 0)
		return ret;

	ret = platform_add_devices(imap_uart_devs, nr_uarts);
	if (ret != 0)
		return ret;
	
	printk(KERN_INFO "leaving imap_arch_init\n");
	return ret;
}

arch_initcall(imap_arch_init);

struct sysdev_class imap_sysclass = { 
        .name   = "imap-core",
};

static struct sys_device imap_sysdev = { 
        .cls    = &imap_sysclass,
};


static __init int imap_sysdev_init(void)
{
        sysdev_class_register(&imap_sysclass);
        return sysdev_register(&imap_sysdev);
}

core_initcall(imap_sysdev_init);
/* XXX XXX XXX XXX XXX XXX XXX */

#if defined(CONFIG_MACH_IMAPX200)
MACHINE_START(IMAPX200, "IMAPX200")
#endif
//	.phys_io	= UART0_BASE_ADDR,
//	.io_pg_offst	= (((u32)UART0_BASE_ADDR) >> 18) & 0xfffc,
	.boot_params	= IMAPX200_SDRAM_PA + 0x100,

	.init_irq	= imap_plat_init_irq,
	.map_io		= imapx200_map_io,
	.fixup		= imapx200_fixup,
	.timer		= &imapx200_init_timer,
	.init_machine	= imapx200_machine_init,
MACHINE_END	

