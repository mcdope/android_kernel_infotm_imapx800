/***************************************************************************** 
 * ** linux/arch/arm/mach-imapx200/irq.c 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * **
 * ** Author:
 * **     Alex Zhang   <tao.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.2  25/11/2009  Alex Zhang
 * ** 2.1  25/02/2010  Raymond Wang
 * *****************************************************************************/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/declearations.h>
#include <asm/irq.h>

#include <mach/cpu.h>

#include <asm/mach/irq.h>

#define irqdbf(x...)
#define irqdbf2(x...)
extern int irq_power; 

static inline void imapx200_irq_mask(struct irq_data *data)
{
	unsigned int irqno = data->irq;
	unsigned long mask;
	if (irqno < 32)
	{
		mask = __raw_readl(rINTMSK);
		mask |= 1UL << irqno;
		__raw_writel(mask, rINTMSK);
	}
	else if (irqno < 56)
	{
		mask = __raw_readl(rINTMSK2);
		mask |= 1UL << (irqno-32);
		__raw_writel(mask, rINTMSK2);
	}
	else
	{
		printk("Wrong IRQ Number %d\n", irqno);
	}  
}

static inline void imapx200_irq_unmask (struct irq_data *data)
{
	unsigned int irqno = data->irq;
	unsigned long mask;
	unsigned long bitval1;

	if (irqno < 32)
	{
		bitval1 = (1UL << irqno);
		__raw_writel(bitval1, rINTPND);
		__raw_writel(bitval1, rSRCPND);

		mask = __raw_readl(rINTMSK);
		mask &= ~(1UL << irqno);
		__raw_writel(mask, rINTMSK);

	}
	else if (irqno < 56)
	{
		bitval1 = (1UL << (irqno-32));
		__raw_writel(bitval1, rINTPND2);
		__raw_writel(bitval1, rSRCPND2);

		mask = __raw_readl(rINTMSK2);
		mask &= ~(1UL << (irqno-32));
		__raw_writel(mask, rINTMSK2);
	}
	else
	{

		printk("Wrong IRQ Number %d\n", irqno);
	}
}

static inline void imapx200_irq_enable (struct irq_data *data)
{
	unsigned int irqno = data->irq;
	struct irq_desc *desc = irq_to_desc(irqno);
	unsigned long mask;

	if (irqno < 32)
	{
		mask = __raw_readl(rINTMSK);
		mask &= ~(1UL << irqno);
		__raw_writel(mask, rINTMSK);
	}
	else if (irqno < 56)
	{
		mask = __raw_readl(rINTMSK2);
		mask &= ~(1UL << (irqno-32));
		__raw_writel(mask, rINTMSK2);
	}
	else
	{
		printk("Wrong IRQ Number %d\n", irqno);
	}

	desc->status_use_accessors &= ~IRQD_IRQ_MASKED;

}

static inline void imapx200_irq_disable (struct irq_data *data)
{
	unsigned int irqno = data->irq;
        unsigned long mask, bitval;
	struct irq_desc *desc = irq_to_desc(irqno);

        if (irqno < 32)
        {
                mask = __raw_readl(rINTMSK);
                mask |= 1UL << irqno;
                __raw_writel(mask, rINTMSK);
	
		bitval = (1UL << irqno);
		__raw_writel(bitval, rINTPND);
		__raw_writel(0x0, rSRCPND);

        }
        else if (irqno < 56)
        {
                mask = __raw_readl(rINTMSK2);
                mask |= 1UL << (irqno-32);
                __raw_writel(mask, rINTMSK2);

		bitval = (1UL << (irqno-32));
		__raw_writel(bitval, rINTPND2);
		__raw_writel(0x0, rSRCPND2);

        }
        else
        {
                printk("Wrong IRQ Number %d\n", irqno);
        }

	desc->status_use_accessors |= IRQD_IRQ_MASKED;
}


#ifdef CONFIG_PM
static int imapx200_irq_wake (struct irq_data *data, unsigned int flag)
{

}
#else
#define imapx200_irq_wake NULL
#endif

static inline void imapx200_timer_mask(struct irq_data *data)
{
	unsigned int irqno = data->irq;
	unsigned long mask;

	mask = __raw_readl(rINTMSK);
	mask |= 1UL << irqno;
	__raw_writel(mask, rINTMSK);
}

static inline void imapx200_timer_unmask(struct irq_data *data)
{
	unsigned int irqno = data->irq;
	unsigned long mask;

	mask = __raw_readl(rINTMSK);
	mask &= ~(1UL << irqno);
	__raw_writel(mask, rINTMSK);
}

static struct irq_chip imapx200_irq_level_chip = {
	.irq_mask_ack = imapx200_irq_mask,
	.irq_mask = imapx200_irq_mask,
	.irq_unmask = imapx200_irq_unmask,
	.irq_enable = imapx200_irq_enable,
	.irq_disable = imapx200_irq_disable,
	.irq_set_wake = imapx200_irq_wake
};

static struct irq_chip imapx200_timer_chip = {
	.name = "timer0",
	.irq_mask_ack = imapx200_timer_mask,
	.irq_mask = imapx200_timer_mask,
	.irq_unmask = imapx200_timer_unmask,
};

/* --------------------------------------------------
 *  imap_init_irq
 *
 *  Initialise imapx200 IRQ system
 * --------------------------------------------------
 */
#ifdef CONFIG_CPU_IMAPX200
void __init imap_plat_init_irq(void)
{
	int irqno;

	/*Clear all interrupts pending... */

	__raw_writel(0xFFFFFFFF, rINTMSK);
	__raw_writel(0xFFFFFFFF, rINTMSK2);

 	__raw_writel(0xFFFFFFFF, rSRCPND);
 	__raw_writel(0xFFFFFFFF, rSRCPND2);

 	__raw_writel(0xFFFFFFFF, rINTPND);
 	__raw_writel(0xFFFFFFFF, rINTPND2);

// 	__raw_writel(0xFFFFFFFF, rSUBSRCPND);
	__raw_writel(0x4, rPOW_STB);
 
	/* register the internal interrupts */
	irqdbf("imap_init_irq: registering imapx200 interrupt handlers\n");

	for (irqno = IRQ_EINT0; irqno <= IRQ_RESERVED3; irqno++){
		switch (irqno) {
			case IRQ_RESERVED0:
			case IRQ_RESERVED1:
			case IRQ_RESERVED2:
			case IRQ_RESERVED3:
				/* No irq */
				break;
			case IRQ_TIMER0:
				//irq_set_chip(irqno, &imapx200_timer_chip);
				//irq_set_handler(irqno, handle_edge_irq);
				irq_set_chip_and_handler(irqno, &imapx200_timer_chip, handle_edge_irq);
				irq_set_chip_data(irqno, (void *)(1UL << irqno));
				set_irq_flags(irqno, IRQF_VALID);
				break;
			default:
				//irq_set_chip(irqno, &imapx200_irq_level_chip);
				//irq_set_handler(irqno, handle_level_irq);
				irq_set_chip_and_handler(irqno, &imapx200_irq_level_chip, handle_level_irq);
				irq_set_chip_data(irqno, (void *)(1UL << irqno));
				set_irq_flags(irqno, IRQF_VALID | IRQF_PROBE);
				break;
		}
	}

}
#endif

static int imapx200_irq_add(struct sys_device *sysdev)
{
	return 0;
}

static struct sysdev_driver imapx200_irq_driver = {
	.add		= imapx200_irq_add,
//	.suspend	= imap_irq_suspend,
//	.resume		= imap_irq_resume,
};

static int imapx200_irq_init(void)
{
	return sysdev_driver_register(&imapx200_sysclass, &imapx200_irq_driver);
}

arch_initcall(imapx200_irq_init);
