/* **************************************************
 * **  arch/arm/mach-imapx200/pwm.c
 * **
 * **
 * ** Copyright (c) 2009~2014 ShangHai Infotm .Ltd all rights reserved. 
 * **
 * ** Use of infoTM's code  is governed by terms and conditions 
 * ** stated in the accompanying licensing statment.
 * **   
 * ** Description: PWM TIMER driver for imapx200 SOC
 * **
 * ** Author:
 * **
 * **   Haixu Fu       <haixu_fu@infotm.com>
 * ** Revision History:
 * ** ----------------
 * **  1.1  03/06/2010   Haixu Fu 
 * **************************************************/


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <mach/irqs.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/sysdev.h>

#include <mach/imap-pwm.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/pad.h>

#include <mach/items.h>
#include <linux/cpufreq.h>

#define	IMAP_PWM_CHANNEL  5

#define PWM_DEBUG 0

/* debug code */
#define pwm_dbg(debug_level, msg...) do\
{\
	if (debug_level > 0)\
	{\
		printk(msg);\
	}\
} while (0)


typedef struct imap_pwm_chan_s  imap_pwm_chan_t;
static uint32_t save_tcfg0, save_tcfg1, save_tcntb2, save_tcmpb2, save_tcon,save_tcntb0, save_tcmpb0;

/* struct imap_pwm_chan_s */

struct imap_pwm_chan_s {
	void __iomem		*base;
	
	unsigned char     	channel;
	int			irq;

	struct sys_device	sysdev;

};

static struct resource imap_pwm_resource[1]={
	[0] = {
		.start = IMAP_PWM_BASE,
		.end   = IMAP_PWM_BASE + IMAP_PWM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	}
};

imap_pwm_chan_t imap_chans[IMAP_PWM_CHANNEL];

void  __iomem 		*ioaddr;
struct clk *apb_clk;
struct notifier_block           freq_transition;
long grate;
long gtcnt, gtcmp;
int gintensity = -1;
int gstart;
int glow_intensity;
int ghigh_intensity;
int intensity_range;
int gstep;
int gcur_tcnt, gcur_tcmp;
int gpwmchn = 0;


spinlock_t infotm_pwm_lock;


int pwm_change_width(int channel,int intensity);

#ifdef CONFIG_PM
int imap_pwm_suspend(struct sys_device *pdev, pm_message_t pm)
{
	printk("++ %s ++\n", __func__);
	/* Save the registers */
	save_tcfg0 = readl(ioaddr + IMAP_TCFG0);
	save_tcfg1 = readl(ioaddr + IMAP_TCFG1);
	save_tcon  = readl(ioaddr + IMAP_TCON);
	save_tcntb2 = readl(ioaddr + IMAP_TCNTB2);
	save_tcmpb2 = readl(ioaddr + IMAP_TCMPB2);
	save_tcntb0 = readl(ioaddr + IMAP_TCNTB0);
	save_tcmpb0 = readl(ioaddr + IMAP_TCMPB0);
	
	return 0;
}

#define PAD_SYSM_VA1  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000))
extern int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long gtcmp);
extern int imap_back_poweron(void);
int imap_back_poweron(void)
{
	unsigned int pad_mode;
	unsigned int pad_dir;
	unsigned int pad_value;
	int tmp;

	if(gintensity == -1){
		gintensity = item_integer("bl.def_intensity",0);	
	}

	if (likely(gintensity > glow_intensity)){
		tmp = (gintensity - glow_intensity) * 0xff / intensity_range + 1;
	}
	else{
		tmp = 0;
	}

	pwm_change_width(item_integer("bl.ctrl",1),tmp);
#if 0
	writel(save_tcfg0, ioaddr + IMAP_TCFG0);
	writel(save_tcfg1, ioaddr + IMAP_TCFG1);
	writel(save_tcntb2, ioaddr + IMAP_TCNTB2);
	writel(save_tcmpb2, ioaddr + IMAP_TCMPB2);
	writel(save_tcntb0, ioaddr + IMAP_TCNTB0);
	writel(save_tcmpb0, ioaddr + IMAP_TCMPB0);
	//writel(save_tcon | 0xb, ioaddr + IMAP_TCON);
	writel(0x1<<1, ioaddr + IMAP_TCON);
	writel(0x1<<1 | 0x1<<3, ioaddr + IMAP_TCON);
	writel(0x1<<1 | 0x1<<3 | 0x1, ioaddr + IMAP_TCON);
#endif
	mdelay(20);
	
	return 0;

}

EXPORT_SYMBOL(imap_back_poweron);
int imap_pwm_resume(struct sys_device *pdev)
{
	module_power_on(SYSMGR_PWM_BASE);
	imapx_pad_cfg(IMAPX_PWM, 0);

	/* Restore the registers */
	writel(save_tcfg0, ioaddr + IMAP_TCFG0);
	writel(save_tcfg1, ioaddr + IMAP_TCFG1);
	writel(save_tcntb2, ioaddr + IMAP_TCNTB2);
	writel(save_tcmpb2, ioaddr + IMAP_TCMPB2);
	writel(save_tcntb0, ioaddr + IMAP_TCNTB0);
	writel(0x0, ioaddr + IMAP_TCMPB0);
	writel(save_tcon | 0xb, ioaddr + IMAP_TCON);

	return 0;
}
#else
#define imap_pwm_suspend   NULL
#define	imap_pwm_resume    NULL
#endif

struct sysdev_class pwm_sysclass = {
	.name 		= "imap-pwm",
	//.suspend	= imap_pwm_suspend,
	//.resume		= imap_pwm_resume,
};

void pwm_writel(void __iomem *base, int offset,int value)
{
	__raw_writel(value,base+offset);
}

u32 pwm_readl(void __iomem  *base,int offset)
{
	return __raw_readl(base+offset);
}
/* imap PWM initialisation */

int imap_pwm_start(int chan)
{
	unsigned long tcon;
	
	tcon = pwm_readl(ioaddr,IMAP_TCON);
	
	switch(chan){
		case 0:
			tcon |= IMAP_TCON_T0START;
			tcon &= ~IMAP_TCON_T0MU_ON;
			break;
		case 1:
			tcon |= IMAP_TCON_T1START;
			tcon &= ~IMAP_TCON_T1MU_ON;
			break;
		case 2:
			tcon |= IMAP_TCON_T2START;
			tcon &= ~IMAP_TCON_T2MU_ON;
			break;
		case 3:	
			tcon |= IMAP_TCON_T3START;
			tcon &= ~IMAP_TCON_T3MU_ON;
			break;
		case 4 :
			tcon |= IMAP_TCON_T4START;
			tcon &= ~IMAP_TCON_T4MU_ON;
	}
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
	return 0;
}	


int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long g_tcmp)
{
	unsigned long tcon;

	//printk("++ %s ++\n", __func__);
	tcon = pwm_readl(ioaddr,IMAP_TCON);

	switch(channel)
	{
		case 3:
		case 4:
		 	printk(KERN_INFO "Only Timer 2 supported right now\n.");
			return -1;
		case 1:
			tcon &= ~(7 << 8);
			tcon |= IMAP_TCON_T1RL_ON;
			break;
		case 0:
			tcon &= ~(7);
			tcon |= IMAP_TCON_T0RL_ON;
			break;
		case 2:
			tcon &= ~(7<<12);
			tcon |= IMAP_TCON_T2RL_ON;
			break;
		default:
			printk(KERN_ERR "segment invalid!\n");
			break;	
	}
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
#if 0
	gtcnt = g_tcnt;
	gtcmp = g_tcmp;
	//g_tcnt = (long long)(gtcnt * 1000000);
	if(grate != 1){	
		//tcnt = (unsigned long)((gtcnt * 100000) / (grate / 10));
		tcnt = gtcnt;
		tcmp = (unsigned long)((gtcmp * 100000) / (grate / 10));
	}else{
		tcnt = gtcnt;
		tcmp = gtcmp;	
	}
#endif
	pwm_writel(ioaddr,IMAP_TCNTB(channel),g_tcnt);
	pwm_writel(ioaddr,IMAP_TCMPB(channel),g_tcmp);
	switch(channel)
	{
		case 0:
			tcon |= IMAP_TCON_T0MU_ON;
			break;
		case 1:
			tcon |= IMAP_TCON_T1MU_ON;
			break;
		case 2:	
			tcon |= IMAP_TCON_T2MU_ON;
			break;
		case 3:
			tcon |= IMAP_TCON_T3MU_ON;
			break;
		case 4:
			tcon |= IMAP_TCON_T4MU_ON;
			break;
		default:
			printk(KERN_ERR "segment invalid\n");
			break;
	} 
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
	
	imap_pwm_start(channel);

	//printk("rate %d, %d\n", g_tcnt, g_tcmp);	
	return 0;
}

EXPORT_SYMBOL(imap_timer_setup);

static int imap_pwm_div(void)
{
	uint32_t tmp, i = 0, j = 0;
	
	i |= (0x8);
	tmp = readl(ioaddr + IMAP_TCFG1);
	tmp &= ~0xffff;
	tmp |= ((i << 0) | (i << 4) | (i << 8) | (i << 12));
	writel(tmp, ioaddr + IMAP_TCFG1);

	save_tcfg1 = tmp;

	tmp = readl(ioaddr + IMAP_TCFG0);
	tmp &= ~(0xffff);
	tmp |= (j | (j << 8));
	writel(tmp, ioaddr + IMAP_TCFG0);

	save_tcfg0 = tmp;
	
	return 0;
}

static int __init imap_init_pwm(void)
{
	imap_pwm_chan_t *cp ;
 	int channel;
	int ret;
	uint32_t tcon;
	struct resource  *res0,*res1;


	spin_lock_init(&infotm_pwm_lock);

    if(item_equal("bl.ctrl", "pwm", 0))
    {
        gpwmchn = item_integer("bl.ctrl",1);
    } else 
        gpwmchn = 0;

	gintensity = -1;
	
	if (item_exist("bl.start"))
	{
		gstart = item_integer("bl.start",0);
	}
	else
	{
		gstart = 6136;
	}
	if (item_exist("bl.low_intensity"))
	{
		glow_intensity = item_integer("bl.low_intensity",0);
	}
	else
	{
		glow_intensity = 20;
	}
	if (item_exist("bl.high_intensity"))
	{
		ghigh_intensity = item_integer("bl.high_intensity",0);
	}
	else
	{
		ghigh_intensity = 255;
	}

	intensity_range = ghigh_intensity - glow_intensity;
	
	gstep = (gstart * 8) / 134;

	module_power_on(SYSMGR_PWM_BASE);

	printk(KERN_ERR "iMAP PWM Driver Init.\n");

	res0 = &imap_pwm_resource[0];

	res1 = request_mem_region(res0->start,resource_size(res0),"imap-pwm");
	if(!res1)
	{
		printk(KERN_ERR " [imap_pwm]-FAILED TO RESERVE MEM REGION \n");
		return -ENXIO;
	}

	ioaddr = ioremap_nocache(res0->start,resource_size(res0));
 	if(!ioaddr)
	{
		printk(KERN_ERR "[imap_pwm]-FAILED TO MEM REGIDSTERS\n");
		return -ENXIO;	
	}	

	ret = sysdev_class_register(&pwm_sysclass);
	if(ret != 0)
	{
		printk(KERN_ERR "[imap_pwm]-pwm system device registration failed\n");
		return  -ENODEV;
	}
	
	for(channel = 0 ;channel < IMAP_PWM_CHANNEL;channel++)
	{
		cp = &imap_chans[channel];
		
		memset(cp, 0, sizeof(imap_pwm_chan_t));
		cp->channel = channel;
		
		cp->irq = channel + GIC_PWM0_ID;

		/* register sysdev */
		ret = sysdev_register(&cp->sysdev); 	
	}

	apb_clk = clk_get(NULL, "apb-clk");

	imap_pwm_div();

	tcon = pwm_readl(ioaddr,IMAP_TCON);
	tcon &= ~(0x1<<4); 
	pwm_writel(ioaddr,IMAP_TCON,tcon);

	return 0;
}



int pwm_change_width(int channel, int intensity){
	
	int clk;
	int cur_tcnt;
	int cur_tcmp;

	spin_lock(&infotm_pwm_lock);
	//end = item_integer("bl.end",0);
	//printk("bl.low_intensity = %d\n", glow_intensity);
	
	gintensity = glow_intensity + (intensity * intensity_range) / 0xff ;	//intensity is from 0 to 0xff

	//printk("gintensity = %d\n", gintensity);

	clk = clk_get_rate(apb_clk);
	clk /= 1000000;
	cur_tcnt = gstart - (((134 - clk) / 8) * gstep);
	//cur_tcmp = (cur_tcnt - end) * intensity / 0xff + end;
	cur_tcmp = (cur_tcnt - 0) * gintensity / 0xff + 0;

	imap_timer_setup(channel, cur_tcnt, cur_tcmp);
	spin_unlock(&infotm_pwm_lock);

	return 0;
}


#ifdef CONFIG_CPU_FREQ

int pwm_cpufreq_prechange(int pre_clk){

	int clk;

	spin_lock(&infotm_pwm_lock);

	clk = pre_clk/6;
	clk /= 1000000;
	gcur_tcnt = gstart - (((134 - clk) / 8) * gstep);

	gcur_tcmp = (gcur_tcnt - 0) * gintensity / 0xff + 0;

	pwm_writel(ioaddr,IMAP_TCNTB(gpwmchn),gcur_tcnt);
	pwm_writel(ioaddr,IMAP_TCMPB(gpwmchn),gcur_tcmp);
	spin_unlock(&infotm_pwm_lock);
}

int pwm_cpufreq_trans(void){

	int tcon;	
	//printk("gintensity = %x\n", gintensity);
	//pwm_change_width(gintensity);
	spin_lock(&infotm_pwm_lock);
	//imap_timer_setup(0, gcur_tcnt, gcur_tcmp);

	tcon = pwm_readl(ioaddr,IMAP_TCON);
    switch(gpwmchn){                       
        case 0:                         
            tcon |= IMAP_TCON_T0MU_ON; 
            break;                      
        case 1:                         
            tcon |= IMAP_TCON_T1MU_ON; 
            break;                      
        case 2:                         
            tcon |= IMAP_TCON_T2MU_ON; 
            break;                      
        case 3:                         
            tcon |= IMAP_TCON_T3MU_ON; 
            break;                      
        case 4 :                        
            tcon |= IMAP_TCON_T4MU_ON; 
    }                                   
	pwm_writel(ioaddr,IMAP_TCON,tcon);

	spin_unlock(&infotm_pwm_lock);
	return 0;
#if 0
	long clk;
	clk = clk_get_rate(apb_clk);
	printk("clk = %d\n", clk);
	switch(clk){
		case 134000000:
			gtcnt = 6136;	
			break;
		case 126000000:
			gtcnt = 5769;
			break;
		case 118000000:
			gtcnt = 5403;
			break;
		case 110000000:
			gtcnt = 5037;
			break;
		case 102000000:
			gtcnt = 4671;
			break;
		case 94000000:
			gtcnt = 4305;
			break;
		case 86000000:
			gtcnt = 3939;
			break;
		case 78000000:
			gtcnt = 3573;
			break;
		case 70000000:
			gtcnt = 3207;
			break;
		case 62000000:
			gtcnt = 2841;
			break;
		case 54000000:
			gtcnt = 2475;
			break;
		case 46000000:
			gtcnt = 2109;
			break;
		case 38000000:
			gtcnt = 1743;
			break;
		case 30000000:
			gtcnt = 1377;
			break;
		case 22000000:
			gtcnt = 1011;
			break;
		default:
			break;	
	
	}

	grate = apb_clk_val / (clk / 1000000);
	imap_timer_setup(0, gtcnt, gtcmp);
	//printk("pwm:clk = %d, grate=%d, %x, %x\n", clk, grate, gtcnt, gtcmp);
	//printk("tcnt = %x, tcmp = %x\n", pwm_readl(ioaddr,IMAP_TCNTB(0)), pwm_readl(ioaddr,IMAP_TCMPB(0)));
#endif
}
#else
#endif

__initcall(imap_init_pwm);
