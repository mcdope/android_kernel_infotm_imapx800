#include <linux/suspend.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/input.h>

#include <asm/hardware/gic.h>
#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/imap-iomap.h>
#include <mach/imap-rtc.h>
#include <mach/power-gate.h>
#include <mach/belt.h>
#include <mach/items.h>

#define PM_INFO_LOG
#ifdef PM_INFO_LOG
#define PM_INFO(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
#define PM_INFO(fmt, ...)
#endif

extern void imapx800_suspend_clock(void);
extern void imapx800_resume_clock(void);

extern void imapx800_cpu_resume(void);
extern int  imapx800_cpu_save(unsigned long *saveblk, long);
extern void l2x0_flush_all(void); 
extern void versatile_secondary_startup(void);

extern struct imapx_gpio {
	char	name[16];
	int	code;
	int	irq;
	int	index;		/*pad index*/
	int	num;		/*0~25*/
	int 	type;		/*interrupt type*/
	int	flt;		/*filter*/
	struct work_struct	work;
	struct input_dev	*input;

	struct delayed_work     power_work;
	u32			key_poll_delay;
	bool			keydown;
};
extern struct imapx_gpio imapx_gpio[4];

void (*pm_cpu_sleep)(void);


static unsigned int gic_enable_reg[256];
static unsigned int gic_secure_reg[256];
static unsigned int gic_priority_reg[256];
static unsigned int gic_target_reg[256];
static unsigned int gic_config_reg[256];
static unsigned int gic_ctrl;
static unsigned int scu_control;
static unsigned int cpui_primask;
static unsigned int cpui_ctrl;
static unsigned int l2x0_aux_ctrl;
static unsigned int l2x0_ctrl;
static unsigned int pad_casemode;

extern int flag_i7av0_host;

static void pad_case_save(){
	
	void __iomem *base = IO_ADDRESS(IMAP_SYSMGR_BASE) + 0x9000;
	pad_casemode = readl(base);

}

static void pad_case_resume(){

	void __iomem *base = IO_ADDRESS(IMAP_SYSMGR_BASE) + 0x9000;
	writel(pad_casemode, base);
}

static void l2x0_save(){
    
	void __iomem *base = IO_ADDRESS(IMAP_SCU_BASE) + 0x2000;

	l2x0_aux_ctrl = readl(base + L2X0_AUX_CTRL);
	l2x0_ctrl = readl(base + L2X0_CTRL);

}

static void l2x0_resume(){

	void __iomem *base = IO_ADDRESS(IMAP_SCU_BASE) + 0x2000;
	
	writel(l2x0_aux_ctrl, base + L2X0_AUX_CTRL);
	//l2x0_inv_all();
	writel(l2x0_ctrl, base + L2X0_CTRL);

}

static void imapx800_gic_save()
{
	int i = 0;
	int length = 0;
	int count = 0;
	void __iomem *scu_base = IO_ADDRESS(IMAP_SCU_BASE);
	void __iomem *gic_dist_base = scu_base + 0x1000; 
	void __iomem *gic_cpu_base = scu_base + 0x100; 

	/*save gic ctrl register*/
	gic_ctrl = readl(gic_dist_base + GIC_DIST_CTRL);

	/*save gic set enable register*/
	length = 0x17c - 0x100;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		gic_enable_reg[i] = readl(gic_dist_base + GIC_DIST_ENABLE_SET + i * 4);
	}

	/*save gic set secure register*/
	length = 0xfc - 0x84;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		gic_secure_reg[i] = readl(gic_dist_base + GIC_DIST_SECURE + 0x4 + i * 4);	
	}

	/*save gic priority register*/
	length = 0x7f8 - 0x400;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		gic_priority_reg[i] = readl(gic_dist_base + GIC_DIST_PRI + i * 4);	
	}
	
	/*save gic target register*/
	length = 0xBF8 - 0x820;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		gic_target_reg[i] = readl(gic_dist_base + GIC_DIST_TARGET + 0x20 + i * 4);	
	}

	/*save gic config register*/
	length = 0xcfc - 0xc04;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		gic_config_reg[i] = readl(gic_dist_base + GIC_DIST_CONFIG + 0x4 + i * 4);	
	}

	cpui_primask = readl(gic_cpu_base + GIC_CPU_PRIMASK);
	cpui_ctrl = readl(gic_cpu_base + GIC_CPU_CTRL);

	scu_control = readl(scu_base);
}

static void imapx800_gic_restore()
{
	int i = 0;
	int length = 0;
	int count = 0;
	void __iomem *scu_base = IO_ADDRESS(IMAP_SCU_BASE);
	void __iomem *gic_dist_base = scu_base + 0x1000; 
	void __iomem *gic_cpu_base = scu_base + 0x100; 

	/*restore gic set enable register*/
	length = 0x17c - 0x100;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		writel(gic_enable_reg[i], gic_dist_base + GIC_DIST_ENABLE_SET + i * 4);	
	}

	/*restore gic set secure register*/
	length = 0xfc - 0x84;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		writel(gic_secure_reg[i], gic_dist_base + GIC_DIST_SECURE + 0x4 + i * 4);	
	}

	/*restore gic priority register*/
	length = 0x7f8 - 0x400;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		writel(gic_priority_reg[i], gic_dist_base + GIC_DIST_PRI + i * 4);	
	}
	
	/*restore gic target register*/
	length = 0xBF8 - 0x820;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		writel(gic_target_reg[i], gic_dist_base + GIC_DIST_TARGET + 0x20 + i * 4);	
	}

	/*restore gic config register*/
	length = 0xcfc - 0xc04;
	count = length/4 + 1;
	for(i=0; i<count; i++)
	{
		writel(gic_config_reg[i], gic_dist_base + GIC_DIST_CONFIG + 0x4 + i * 4);	
	}

	writel(cpui_primask, gic_cpu_base + GIC_CPU_PRIMASK);
	writel(cpui_ctrl, gic_cpu_base + GIC_CPU_CTRL);

	/*restore gic ctrl register*/
	writel(gic_ctrl, gic_dist_base + GIC_DIST_CTRL);

	writel(scu_control, scu_base);
}


/* callback from assembly code */
void imapx800_pm_cb_flushcache(void)
{
    flush_cache_all();
    outer_flush_all();
}

static int imapx800_pm_prepare(void)
{
    void __iomem *rtl_sys = IO_ADDRESS(SYSMGR_RTC_BASE);
    unsigned int val = virt_to_phys(imapx800_cpu_resume);
    PM_INFO("++ %s 0x%x ++\n", __func__, val);

    /* store address of resume. */
    __raw_writel(val&0xff, rtl_sys + SYS_INFO_0);
    __raw_writel((val>>8)&0xff, rtl_sys + SYS_INFO_1);
    __raw_writel((val>>16)&0xff, rtl_sys + SYS_INFO_2);
    __raw_writel((val>>24)&0xff, rtl_sys + SYS_INFO_3);

    return 0;
}

void imap_resume_l2x0(void)
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
    l2x0_resume();
}


#ifdef CONFIG_CACHE_L2X0
extern void __init imap_init_l2x0(void);
extern int cmn_timer_resume(void);
#endif

static int imapx800_pm_enter(suspend_state_t state)
{
    volatile int tmp = 0;

    if (pm_cpu_sleep == NULL) {
	printk(KERN_ERR "%s: error: no cpu sleep function\n", __func__);
	return -EINVAL;
    }

    imapx800_suspend_clock();    
    pad_case_save();	
    l2x0_save();
    imapx800_gic_save();
    /* call cpu specific preparation */
    imapx800_pm_prepare();
    /* flush cache back to ram */
    //imapx800_pm_cb_flushcache();

    imapx800_cpu_save(0, PLAT_PHYS_OFFSET - PAGE_OFFSET);
#ifdef CONFIG_CACHE_L2X0
    outer_inv_all();
    imap_resume_l2x0();
#endif    
    cpu_init();

    writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));//mask int
    writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE + 0x4));//clear int

    imapx800_gic_restore();
    //module_power_on(SYSMGR_CMNTIMER_BASE);
    //cmn_timer_resume();
    pad_case_resume(); 
    imapx800_resume_clock();
    module_power_on(SYSMGR_GPIO_BASE);

    return 0;
}

static void imapx800_pm_finish(void)
{
    PM_INFO("++ %s ++\n", __func__);
}

static struct platform_suspend_ops imapx800_pm_ops = {
    .enter = imapx800_pm_enter,
    .prepare = imapx800_pm_prepare,
    .finish = imapx800_pm_finish,
    .valid = suspend_valid_only_mem,
};

static void __ensure_hw_state(void)
{
//    writel(0xff, IO_ADDRESS(SYSMGR_GBUS_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_GDMA_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_EMIF_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_CPU_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_IRAM_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_PAD_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_EFUSE_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_BOOTM_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_RTC_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_CLKGEN_BASE	));
	
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x130));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x140));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x150));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x160));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x170));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x180));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x190));

    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x134));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x144));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x154));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x164));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x174));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x184));
    writel(3, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x194));

    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x13c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x14c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x15c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x16c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x17c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x18c));
    writel(0, IO_ADDRESS(SYSMGR_CLKGEN_BASE + 0x19c));

//    writel(0xff, IO_ADDRESS(SYSMGR_NIF_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_IROM_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_SYSMGR_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_G2D_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_CMND_BASE	));
//    writel(0xff, IO_ADDRESS(SYSMGR_MMP_BASE		));
//    writel(0xff, IO_ADDRESS(SYSMGR_BROM_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_IIS_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_CMNTIMER_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_PWM_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_IIC_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_PCM_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_SSP_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_SPDIF_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_SIMC_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_UART_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_PIC_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_AC97_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_KEYBD_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_ADC_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_DMIC_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_GPIO_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_PWDT_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_PWMA_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_IDS0_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_IDS1_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_GPS_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_CRYPTO_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_GPU_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_VDEC_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_VENC_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_ISP_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_MIPI_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_TSIF_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_USBH_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_OTG_BASE		));
    writel(0xff, IO_ADDRESS(SYSMGR_MMC1_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_MMC2_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_MMC0_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_NAND_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_SATA_BASE	));
    writel(0xff, IO_ADDRESS(SYSMGR_MAC_BASE		));

}

static void imapx800_gic_disable()
{
    int length = 0, count = 0;
    int i = 0;
    void __iomem *gic_dist_base = IO_ADDRESS(IMAP_SCU_BASE) + 0x1000; 

    __raw_writel(0x0, gic_dist_base + GIC_DIST_CTRL);

    /*gic set clear enable register*/
    length = 0x1fc - 0x180;
    count = length/4 + 1;
    for(i=0; i<count; i++)
	__raw_writel(0xffffffff, gic_dist_base + GIC_DIST_ENABLE_CLEAR + i * 4);
}

void rtc_set_delayed_alarm(u32 delay);
static void imapx800_cpu_suspend(void)
{
    volatile unsigned int hw_ddr_lowpow = 0;
    unsigned long tmp;
    void __iomem *rtc_sys = IO_ADDRESS(SYSMGR_RTC_BASE);
    void __iomem *mgr_sys = IO_ADDRESS(SYSMGR_SYSMGR_BASE);
    void __iomem *emif_base = IO_ADDRESS(IMAP_EMIF_BASE);
    int flag_charger_func = 0;
    int flag_charger_state = 0;// 0-not exist, 1-exist
    int flag_i7av0 = 0;
    int reg_data;

    if(item_exist("charger.pwron"))
    {
        if(item_equal("charger.pwron","1",0))
        {
            flag_charger_func = 1;
            if(readl(rtc_sys+ SYS_RTC_IO_IN)& 0x04)
            {
                flag_charger_state = 1;
            }
        }
    } 

    if(item_exist("charger.enable"))
    {
        if(item_equal("charger.enable","1",0))
        {
            flag_charger_func = 1;
            if(readl(rtc_sys+ SYS_RTC_IO_IN)& 0x04)
            {
                flag_charger_state = 1;
            }
        }
    } 

    if(item_exist("charger.pattern"))
    {
        if(item_equal("charger.pattern", "mode1", 0))
        {
            flag_i7av0 = 1;
        }
        else
        {
            flag_i7av0 = 0;
        }
    }

    imapx800_gic_disable();
    imapx800_pm_cb_flushcache();
   
    hw_ddr_lowpow = __raw_readl(emif_base);
    hw_ddr_lowpow |= 0x1;
    __raw_writel(hw_ddr_lowpow, emif_base);

    __raw_writel(0xff, rtc_sys + SYS_INT_CLR);
    __raw_writel(0x03, rtc_sys + SYS_RST_CLR);
    __raw_writel(0x03, rtc_sys + SYS_POWUP_CLR);
    __raw_writel(0xfe, rtc_sys + SYS_INT_MASK);
    __raw_writel(0xf,  rtc_sys + SYS_IO_CFG);

    if(belt_scene_get() & SCENE_RTC_WAKEUP) {
	    __raw_writel(0x0f, rtc_sys + SYS_WAKEUP_MASK);
	    printk(KERN_ERR "prepare RTC wake up.\n");
	    rtc_set_delayed_alarm(belt_get_rtc());
    } else {
	    //__raw_writel(0x1f, rtc_sys + SYS_WAKEUP_MASK); /* wake up all mask*/
	    if(flag_charger_func)
	    {
		    if(flag_charger_state)
		    {
			    //charger is on
			    reg_data = readl(rtc_sys + SYS_IO_CFG);
			    reg_data |= 0x40;// polarity is low
			    writel(reg_data, rtc_sys +SYS_IO_CFG);

			    reg_data = readl(rtc_sys + SYS_WAKEUP_MASK);
			    reg_data &= ~0x04;// enable 
			    writel(reg_data, rtc_sys + SYS_WAKEUP_MASK);
		    }
		    else
		    {
			    //charger is not exist
			    reg_data = readl(rtc_sys + SYS_IO_CFG);
			    reg_data &= ~0x40;// polarity is high
			    writel(reg_data, rtc_sys + SYS_IO_CFG);

			    reg_data = readl(rtc_sys + SYS_WAKEUP_MASK);
			    reg_data &= ~0x04;// enable
			    writel(reg_data, rtc_sys + SYS_WAKEUP_MASK);

		    }
	    }    
    }

    printk("---> step 2 <---\n");

    if(flag_i7av0)
    {
        if(flag_i7av0_host)
        {
            //reg_data = readl(rtc_sys + SYS_IO_CFG);
			//reg_data &= ~0x40;// polarity is high
			//writel(reg_data, rtc_sys + SYS_IO_CFG);

			//reg_data = readl(rtc_sys + SYS_WAKEUP_MASK);
			//reg_data &= ~0x04;// enable
			//writel(reg_data, rtc_sys + SYS_WAKEUP_MASK);


            reg_data = readl(rtc_sys + SYS_WAKEUP_MASK);
            reg_data |= 0x04;
            writel(reg_data, rtc_sys + SYS_WAKEUP_MASK);
        }
    }

    __ensure_hw_state();
    __raw_writel(0x01, mgr_sys + 0x14);


    tmp = 0;
    asm("b 1f\n\t"
	".align 5\n\t"
	"1:\n\t"
	"mcr p15, 0, %0, c7, c10, 5\n\t"
	"mcr p15, 0, %0, c7, c10, 4\n\t"
	"wfi" : : "r" (tmp));

    panic("sleep resumed to originator?");
}

static int __init imap_pm_init(void)
{
    PM_INFO("++ %s ++\n", __FUNCTION__);
    pm_cpu_sleep = imapx800_cpu_suspend;
    suspend_set_ops(&imapx800_pm_ops);
    return 0;
}

device_initcall(imap_pm_init);
