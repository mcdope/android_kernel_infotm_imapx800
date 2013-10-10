/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/devs.c
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.2  25/11/2009    Raymond Wang
********************************************************************************/
 
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/ata_platform.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/amba/bus.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/pmu.h>

#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>

#include <mach/hardware.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/amba/pl330.h>
#include <linux/imap_pl330.h>
#include <linux/amba/serial.h>

#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
#include <linux/usb/android_composite.h>
#endif

#if 1  //dma for uart1(bluetooth)
static inline bool pl011_filter(struct dma_chan *chan, void *param)
{
	struct dma_pl330_peri *peri = chan->private;
	return peri->peri_id == (unsigned)param;
}
struct amba_pl011_data imap_pl011_data1 = {
	.dma_tx_param = (void *)IMAPX800_UART1_TX,
	.dma_rx_param = (void *)IMAPX800_UART1_RX,
	.dma_filter   = pl011_filter,
};
struct amba_pl011_data imap_pl011_data3 = {
	.dma_tx_param = (void *)IMAPX800_UART3_TX,
	.dma_rx_param = (void *)IMAPX800_UART3_RX,
	.dma_filter   = pl011_filter,
};
#define imap_uart1_data (&imap_pl011_data1)
#else
#define imap_uart1_data NULL
#endif

/* amba devices define */
struct amba_device imap_uart0_device = {
	.dev = {
		.init_name = "imap-uart.0", 
		.platform_data = NULL,
	},
	.res = {
		.start = IMAP_UART0_BASE,
		.end   = IMAP_UART0_BASE + IMAP_UART0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { GIC_UART0_ID, NO_IRQ },
	.periphid = 0x00041011,
};

struct amba_device imap_uart1_device = {
	.dev = {
		.init_name = "imap-uart.1",
		.platform_data = imap_uart1_data,
	},
	.res = {
		.start = IMAP_UART1_BASE,
		.end   = IMAP_UART1_BASE + IMAP_UART1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { GIC_UART1_ID, NO_IRQ },
	.periphid = 0x00041011,
};

struct amba_device imap_uart2_device = {
	.dev = {
		.init_name = "imap-uart.2",
		.platform_data = NULL,
	},
	.res = {
		.start = IMAP_UART2_BASE,
		.end   = IMAP_UART2_BASE + IMAP_UART2_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { GIC_UART2_ID, NO_IRQ },
	.periphid = 0x00041011,
};

struct amba_device imap_uart3_device = {
	.dev = {
		.init_name = "imap-uart.3",
		.platform_data = NULL,
	},
	.res = {
		.start = IMAP_UART3_BASE,
		.end   = IMAP_UART3_BASE + IMAP_UART3_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { GIC_UART3_ID, NO_IRQ },
	.periphid = 0x00041011,
};

struct amba_device imap_pic0_device = {
    .dev            = {
	.init_name = "imap-pic.0",
	.platform_data = 0,
	.coherent_dma_mask = ~0,
    },
    .res            = {
	.start  = IMAP_PIC0_BASE,
	.end    = IMAP_PIC0_BASE + IMAP_PIC0_SIZE -1,
	.flags  = IORESOURCE_MEM,
    },
    .irq            = { GIC_PIC0_ID, NO_IRQ },
    .periphid       = 0x00041050,
};

struct amba_device imap_pic1_device = {
    .dev            = {
	.init_name = "imap-pic.1",
	.platform_data = 1,
	.coherent_dma_mask = ~0,
    },
    .res            = {
	.start  = IMAP_PIC1_BASE,
	.end    = IMAP_PIC1_BASE + IMAP_PIC1_SIZE -1,
	.flags  = IORESOURCE_MEM,
    },
    .irq            = { GIC_PIC1_ID, NO_IRQ },
    .periphid       = 0x00041050,
};

static struct dma_pl330_peri imap_pl330_peri[24] = {
    {
        .peri_id = IMAPX800_SSP0_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_SSP0_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_SSP1_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_SSP1_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_I2S_SLAVE_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_I2S_SLAVE_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_I2S_MASTER_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_I2S_MASTER_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_PCM0_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_PCM0_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_AC97_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_AC97_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_UART0_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_TOUCHSCREEN_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_UART1_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_UART1_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_PCM1_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_PCM1_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_UART3_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_UART3_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_SPDIF_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_SPDIF_RX,
        .rqtype  = DEVTOMEM,
    },
    {
        .peri_id = IMAPX800_PWMA_TX,
        .rqtype  = MEMTODEV,
    },
    {
        .peri_id = IMAPX800_DMIC_RX,
        .rqtype  = DEVTOMEM,
    },
};

static struct dma_pl330_platdata imap_pl330_platdata = {
    .nr_valid_peri = 24,
    .peri          = imap_pl330_peri,
    .mcbuf_sz      = 512,
};

struct amba_device imap_dma_device = {
    .dev            = {
        .init_name  = "dma-pl330",
        .platform_data  = &imap_pl330_platdata,
        .coherent_dma_mask = ~0,
    },
    .res            = {
        .start      = IMAP_GDMA_BASE + 0x100000,                    //for secure mode
        .end        = IMAP_GDMA_BASE + 0x100000 + IMAP_GDMA_SIZE -1,
        .flags      = IORESOURCE_MEM,
    },
    .irq            = { GIC_DMA0_ID, GIC_DMA1_ID,
        GIC_DMA2_ID, GIC_DMA3_ID,
        GIC_DMA4_ID, GIC_DMA5_ID,
        GIC_DMA6_ID, GIC_DMA7_ID,
        GIC_DMA8_ID, GIC_DMA9_ID,
        GIC_DMA10_ID, GIC_DMA11_ID,
        GIC_DMABT_ID, NO_IRQ },
    .periphid       = 0x00041330,

};


/* for audio dma module */
static u64 audio_dmamask = DMA_BIT_MASK(32);

struct platform_device imap_asoc_device = {
	.name			= "imapx800-audio",
	.id				= -1,
	.dev            = {
		.dma_mask = &audio_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

/* platform device normal define */
static struct resource imap_nand_resource[]={
	[0] = {
		.start = IMAP_NAND_BASE,
		.end   = IMAP_NAND_BASE + IMAP_NAND_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_NAND_ID,
		.end	= GIC_NAND_ID,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device imap_nand_device = {
	.name		= "imap-nand",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_nand_resource),
	.resource	= imap_nand_resource,
};

static struct resource imap_gpio_source[] = {
	[0] = {
		.start	= IMAP_GPIO_BASE,
		.end	= IMAP_GPIO_BASE + IMAP_GPIO_SIZE -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_GPG_ID,
		.end	= GIC_GPG_ID,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= GIC_GP1_ID,
		.end	= GIC_GP1_ID,
		.flags	= IORESOURCE_IRQ,
	},
	[3] = {
		.start	= GIC_GP2_ID,
		.end	= GIC_GP2_ID,
		.flags	= IORESOURCE_IRQ,
	},

};

struct platform_device imap_gpio_device  = {
	.name		= "imap-gpio",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imap_gpio_source),
	.resource	= imap_gpio_source,
};

/* sdmmc_0_dev */
static int imap_mmc0_get_bus_wd(u32 slot_id)
{
    return 8;
}

static int imap_mmc0_init(u32 slot_id, irq_handler_t handler, void *data)
{
    	int ret;
	
	ret = imapx_pad_cfg(IMAPX_SD0, 1);
	module_power_on(SYSMGR_MMC0_BASE);
	return 0;
}

static struct dw_mci_board imap_mmc0_platdata = {
	.num_slots	= 1,
	.quirks		= DW_MCI_QUIRK_BROKEN_CARD_DETECTION,
	.bus_hz		= 100*1000*1000,
	.detect_delay_ms = 2000,
	.init		= imap_mmc0_init,
	.get_bus_wd	= imap_mmc0_get_bus_wd,
};

static struct resource imap_mmc0_resource[]={
	[0] = {
		.start = IMAP_MMC0_BASE,
		.end   = IMAP_MMC0_BASE + IMAP_MMC0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_MMC0_ID,
		.end	= GIC_MMC0_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

static u64 sdmmc0_dma_mask = DMA_BIT_MASK(32);
struct platform_device imap_mmc0_device = {
	.name		= "imap-mmc",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_mmc0_resource),
	.resource	= imap_mmc0_resource,
	.dev		= {
		.platform_data	=&imap_mmc0_platdata,
		.dma_mask = &sdmmc0_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(32),

	},

};

/* sdmmc_1_dev */
static int imap_mmc1_get_bus_wd(u32 slot_id)
{
    return 4;
}

static int imap_mmc1_init(u32 slot_id, irq_handler_t handler, void *data)
{
	imapx_pad_cfg(IMAPX_SD1, 1);
	module_power_on(SYSMGR_MMC1_BASE);
	return 0;
}

static struct dw_mci_board imap_mmc1_platdata = {
	.num_slots	= 1,
	.quirks		= DW_MCI_QUIRK_HIGHSPEED, 
	.bus_hz		= 50*1000*1000,
	.detect_delay_ms = 500,
	.init		= imap_mmc1_init,
	.get_bus_wd	= imap_mmc1_get_bus_wd,
};

static struct resource imap_mmc1_resource[]={
	[0] = {
		.start = IMAP_MMC1_BASE,
		.end   = IMAP_MMC1_BASE + IMAP_MMC1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_MMC1_ID,
		.end	= GIC_MMC1_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

static u64 sdmmc1_dma_mask = DMA_BIT_MASK(32);
struct platform_device imap_mmc1_device = {
        .name           = "imap-mmc",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(imap_mmc1_resource),
        .resource       = imap_mmc1_resource,
        .dev            = {
                .platform_data = &imap_mmc1_platdata,
                .dma_mask = &sdmmc1_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(32),
        },
};

/* sdmmc_2_dev */
static int imap_mmc2_get_bus_wd(u32 slot_id)
{
    return 4;
}

static int imap_mmc2_init(u32 slot_id, irq_handler_t handler, void *data)
{
	imapx_pad_cfg(IMAPX_SD2, 0);
	module_power_on(SYSMGR_MMC2_BASE);
#if 0        
        imapx_pad_set_mode(MODE_GPIO, 1, 153);
        imapx_pad_set_dir(DIRECTION_OUTPUT, 1, 153);
        imapx_pad_set_outdat(OUTPUT_1, 1, 153);
#endif        
	return 0;
}

static struct dw_mci_board imap_mmc2_platdata = {
	.num_slots	= 1,
	.quirks		= DW_MCI_QUIRK_BROKEN_CARD_DETECTION, 
	.bus_hz		= 100*1000*1000,
	.detect_delay_ms = 2000,
	.init		= imap_mmc2_init,
	.get_bus_wd	= imap_mmc2_get_bus_wd,
};

static struct resource imap_mmc2_resource[]={
	[0] = {
		.start = IMAP_MMC2_BASE,
		.end   = IMAP_MMC2_BASE + IMAP_MMC2_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_MMC2_ID,
		.end	= GIC_MMC2_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

static u64 sdmmc2_dma_mask = DMA_BIT_MASK(32);
struct platform_device imap_mmc2_device = {
	.name		= "imap-mmc",
	.id 		= 2,
	.num_resources	= ARRAY_SIZE(imap_mmc2_resource),
	.resource	= imap_mmc2_resource,
	.dev		= {
		.platform_data = &imap_mmc2_platdata,
		.dma_mask = &sdmmc2_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(32),

	},
};

/************ PMU ************/
static struct resource imap_pmu_resource[]={
    [0] = {
	.start = GIC_PMU0_ID,
	.end = GIC_PMU0_ID,
	.flags = IORESOURCE_IRQ,
    },
    [1] = {
	.start = GIC_PMU1_ID,
	.end = GIC_PMU1_ID,
	.flags = IORESOURCE_IRQ,
    },
};

struct platform_device imap_pmu_device = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.resource 	= imap_pmu_resource,
	.num_resources	= ARRAY_SIZE(imap_pmu_resource),	
};

/* sata controller */
static struct resource imap_sata_resource[]={
        [0] = {
                .start = IMAP_SATA_BASE,
                .end   = IMAP_SATA_BASE + IMAP_SATA_SIZE - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start  = GIC_SATA_ID,
                .end    = GIC_SATA_ID,
                .flags  = IORESOURCE_IRQ,
        }
};

static u64 sata_dma_mask = DMA_BIT_MASK(32);

struct platform_device imap_sata_device = {
        .name           = "imap-sata",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(imap_sata_resource),
        .resource       = imap_sata_resource,
        .dev = {
                .dma_mask = &sata_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(32),
        }
};

/* usb ohci controller device */
static u64 ohci_dma_mask=0xffffffffUL;
static struct resource imap_ohci_resource[]={
	[0] = {
		.start = IMAP_OHCI_BASE,
		.end   = IMAP_OHCI_BASE + IMAP_OHCI_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_OHCI_ID,
		.end	= GIC_OHCI_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ohci_device = {
	.name		= "imap-ohci",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_ohci_resource),
	.resource	= imap_ohci_resource,
	.dev		= {
		.dma_mask = &ohci_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}
};

/* usb ehci controller device */
static u64 ehci_dma_mask=0xffffffffUL;
static struct resource imap_ehci_resource[]={
	[0] = {
		.start = IMAP_EHCI_BASE,
		.end   = IMAP_EHCI_BASE + IMAP_EHCI_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_EHCI_ID,
		.end	= GIC_EHCI_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ehci_device = {
	.name		= "imap-ehci",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_ehci_resource),
	.resource	= imap_ehci_resource,
	.dev		= {
		.dma_mask = &ehci_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}

};

/* usb otg controller device */
static u64 otg_dma_mask=0xffffffffUL;
static struct resource imap_otg_resource[]={
	[0] = {
		.start = IMAP_OTG_BASE,
		.end   = IMAP_OTG_BASE + IMAP_OTG_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_OTG_ID,
		.end	= GIC_OTG_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_otg_device = {
	.name		= "dwc_otg",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_otg_resource),
	.resource	= imap_otg_resource,
	.dev		= {
		.dma_mask = &otg_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}
};


#ifdef CONFIG_USB_G_ANDROID

struct platform_device android_storage_device = {
	.name       = "android_usb",
	.id			= -1,
	.dev		= {
		.platform_data = NULL,
	},
};

#endif
/***********************************************************/

static struct resource imap_sensor_resource[]={

};

struct platform_device imap_sensor_device = {
	.name		= "imap-sensor",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_sensor_resource),
	.resource	= imap_sensor_resource,
};

static struct resource imap_backlight_resource[]={

};

struct platform_device imap_backlight_device = {
	.name           = "imap-backlight",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(imap_backlight_resource),
	.resource       = imap_backlight_resource,
};


/*********MAC*************/
static struct resource imap_mac_resource[]={
	[0] = {
		.start = IMAP_MAC_BASE,
		.end   = IMAP_MAC_BASE + IMAP_MAC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_MAC_ID,
		.end	= GIC_MAC_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_mac_device = {
	.name		= "imap-mac",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_mac_resource),
	.resource	= imap_mac_resource,
	.dev            = {
		.dma_mask = &imap_mac_device.dev.coherent_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}
};

struct platform_device imap_rda5868_device = {
	.name		= "rda5868-bt",
	.id			= -1,
};


/***************Memory Alloc******************/
static struct resource imap_memalloc_resource[]={
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device imap_memalloc_device = 
{
	.name		= "imap-memalloc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imap_memalloc_resource),
	.resource	= imap_memalloc_resource,
};


/*******************GPS***********************/
static struct resource imap_gps_resource[]={
	[0] = {
		.start = IMAP_GPS_BASE,
		.end   = IMAP_GPS_BASE + IMAP_GPS_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
	    .start = GIC_GPSACQ_ID,
		.end   = GIC_GPSACQ_ID,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
	    .start = GIC_GPSTCK_ID,
		.end   = GIC_GPSTCK_ID,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 gps_dma_mask = DMA_BIT_MASK(32);

struct platform_device imap_gps_device = 
{
	.name		= "imap-gps",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imap_gps_resource),
	.resource	= imap_gps_resource,
	.dev = {
		.dma_mask = &gps_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

/********************* crypto ********************************/
static struct resource imap_crypto_resource[] = {
    [0] = {
        .start = IMAP_CRYPTO_BASE,
		.end   = IMAP_CRYPTO_BASE + IMAP_CRYPTO_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = GIC_CRYPTO_ID,
		.end   = GIC_CRYPTO_ID,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device imap_crypto_device = {
    .name          = "imap-crypto",
	.id            = -1,
	.num_resources = ARRAY_SIZE(imap_crypto_resource),
	.resource      = imap_crypto_resource,
};

/*********PWM**************/
static struct resource imap_pwm_resource[]={
	[0] = {
		.start = IMAP_PWM_BASE,
		.end   = IMAP_PWM_BASE + IMAP_PWM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_PWM0_ID,
		.end	= GIC_PWM0_ID,
		.flags	= IORESOURCE_IRQ,  
	},
	[2] = {
		.start	= GIC_PWM1_ID,
		.end	= GIC_PWM1_ID,
		.flags	= IORESOURCE_IRQ,  
	},
	[3] = {
		.start	= GIC_PWM2_ID,
		.end	= GIC_PWM2_ID,
		.flags	= IORESOURCE_IRQ,  
	},
	[4] = {
		.start	= GIC_PWM3_ID,
		.end	= GIC_PWM4_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_pwm_device = {
	.name		= "imap-pwm",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_pwm_resource),
	.resource	= imap_pwm_resource,
};


/************IIC***************/
static struct resource imap_iic0_resource[]={
	[0] = {
		.start = IMAP_IIC0_BASE,
		.end   = IMAP_IIC0_BASE + IMAP_IIC0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_IIC0_ID,
		.end	= GIC_IIC0_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_iic0_device = {
	.name		= "imap-iic",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_iic0_resource),
	.resource	= imap_iic0_resource,
};

static struct resource imap_iic1_resource[]={
	[0] = {
		.start = IMAP_IIC1_BASE,
		.end   = IMAP_IIC1_BASE + IMAP_IIC1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_IIC1_ID,
		.end	= GIC_IIC1_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_iic1_device = {
	.name		= "imap-iic",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(imap_iic1_resource),
	.resource	= imap_iic1_resource,
};

static struct resource imap_iic2_resource[] = {
	[0] = {
		.start	= IMAP_IIC2_BASE,
		.end	= IMAP_IIC2_BASE + IMAP_IIC2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start  = GIC_IIC2_ID,
		.end    = GIC_IIC2_ID,
		.flags  = IORESOURCE_IRQ,
	}
};

struct platform_device imap_iic2_device = {
	.name           = "imap-iic",
	.id             = 2,
	.num_resources  = ARRAY_SIZE(imap_iic2_resource),
	.resource       = imap_iic2_resource,
};

static struct resource imap_iic3_resource[] = {
	[0] = {
		.start  = IMAP_IIC3_BASE,
		.end    = IMAP_IIC3_BASE + IMAP_IIC3_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = GIC_IIC3_ID,
		.end    = GIC_IIC3_ID,
		.flags  = IORESOURCE_IRQ,
	}
};

struct platform_device imap_iic3_device = {
	.name           = "imap-iic",
	.id             = 3,
	.num_resources  = ARRAY_SIZE(imap_iic3_resource),
	.resource       = imap_iic3_resource,
};

static struct resource imap_iic4_resource[] = {
	[0] = {
		.start  = IMAP_IIC4_BASE,
		.end    = IMAP_IIC4_BASE + IMAP_IIC4_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = GIC_IIC4_ID,
		.end	= GIC_IIC4_ID,
		.flags  = IORESOURCE_IRQ,
	}
};

struct platform_device imap_iic4_device = {
	.name           = "imap-iic",
	.id             = 4,
	.num_resources  = ARRAY_SIZE(imap_iic4_resource),
	.resource       = imap_iic4_resource,
};

static struct resource imap_iic5_resource[] = {
	[0] = {
		.start  = IMAP_IIC5_BASE,
		.end    = IMAP_IIC5_BASE + IMAP_IIC5_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = GIC_IIC5_ID,
		.end    = GIC_IIC5_ID,
		.flags  = IORESOURCE_IRQ,
	}
};

struct platform_device imap_iic5_device = {
	.name           = "imap-iic",
	.id             = 5,
	.num_resources  = ARRAY_SIZE(imap_iic5_resource),
	.resource       = imap_iic5_resource,
};

/*************RTC******************/
static struct resource imap_rtc_resource[]={
	[0] = {
		.start = SYSMGR_RTC_BASE,
		.end   = SYSMGR_RTC_BASE + 0x3ff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_RTCINT_ID,
		.end	= GIC_RTCINT_ID,
		.flags	= IORESOURCE_IRQ,  
	},
	[2] = {
		.start	= GIC_RTCTCK_ID,
		.end	= GIC_RTCTCK_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_rtc_device = {
	.name		= "imap-rtc",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_rtc_resource),
	.resource	= imap_rtc_resource,
};

/*****************IIS*****************/
static struct resource imap_iis_resource0[]={        
	[0] = {
		.start = IMAP_IIS0_BASE,
		.end   = IMAP_IIS0_BASE + 0xfff,            
		.flags = IORESOURCE_MEM,                    
	},
	[1] = {
		.start  = GIC_IIS0_ID,                      
		.end    = GIC_IIS0_ID,
		.flags  = IORESOURCE_IRQ,                   
	}                                               
};                                                  

struct platform_device imap_iis_device0 = {          
	.name       = "imapx800-iis0",                       
	.id         = -1, 
	.num_resources  = ARRAY_SIZE(imap_iis_resource0),
	.resource   = imap_iis_resource0,                
};                                                  

static struct resource imap_iis_resource1[]={
	[0] = {
		.start = IMAP_IIS1_BASE,
		.end   = IMAP_IIS1_BASE + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_IIS1_ID,
		.end	= GIC_IIS1_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_iis_device1 = {
	.name		= "imapx800-iis1",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_iis_resource1),
	.resource	= imap_iis_resource1,
};

struct platform_device imap_es8328_device = {
	.name   = "ES8328_I2C_Codec",
	.id = 0,
};

struct platform_device imap_null_device = {
	.name	= "virtual-codec",
	.id = 0,
};

struct platform_device imap_rt5631_device = {
    .name   = "rt5631",
    .id = 0,
};

/***************PCM***********************/
static struct resource imap_pcm0_resource[]={
    [0] = {
        .start = IMAP_PCM0_BASE,
        .end   = IMAP_PCM0_BASE + 0xfff,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = GIC_PCM0_ID,
        .end   = GIC_PCM0_ID,
        .flags = IORESOURCE_IRQ, 
    }   
};

struct platform_device imap_pcm0_device = {
    .name       = "imapx800-pcm",
    .id         = 0,
    .num_resources  = ARRAY_SIZE(imap_pcm0_resource),
    .resource   = imap_pcm0_resource,
};

static struct resource imap_pcm1_resource[]={        
    [0] = {                                         
        .start = IMAP_PCM1_BASE,                    
        .end   = IMAP_PCM1_BASE + 0xfff,
        .flags = IORESOURCE_MEM,                    
    },                                              
    [1] = {                                         
        .start = GIC_PCM0_ID,                       
        .end   = GIC_PCM0_ID,                       
        .flags = IORESOURCE_IRQ,                    
    }                                               
};                                                  

struct platform_device imap_pcm1_device = {          
    .name       = "imapx800-pcm",
    .id         = 1,                               
    .num_resources  = ARRAY_SIZE(imap_pcm1_resource),
    .resource   = imap_pcm1_resource,                
}; 

/**************spdif********************/
static struct resource imap_spdif_resource[]={
    [0] = {
        .start = IMAP_SPDIF_BASE,
        .end   = IMAP_SPDIF_BASE + 0xfff,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = GIC_SPDIF_ID,
        .end   = GIC_SPDIF_ID,
        .flags = IORESOURCE_IRQ, 
    }
};

struct platform_device imap_spdif_device = {
    .name       = "imapx800-spdif",
    .id         = -1,
    .num_resources  = ARRAY_SIZE(imap_spdif_resource),
    .resource   = imap_spdif_resource,
};

/**************AC97*********************/
static struct resource imap_ac97_resource[]={
	[0] = {
		.start = IMAP_AC97_BASE,
		.end   = IMAP_AC97_BASE + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_AC97_ID,
		.end	= GIC_AC97_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ac97_device = {
	.name		= "imap-ac97",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_ac97_resource),
	.resource	= imap_ac97_resource,
};


static struct resource imap_ssp0_resource[]={
	[0] = {
		.start = IMAP_SSP0_BASE,
		.end   = IMAP_SSP0_BASE + IMAP_SSP0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_SSP0_ID,
		.end	= GIC_SSP0_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ssp0_device = {
	.name		= "imap-ssp.0",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_ssp0_resource),
	.resource	= imap_ssp0_resource,
};

static struct resource imap_ssp1_resource[]={
	[0] = {
		.start = IMAP_SSP1_BASE,
		.end   = IMAP_SSP1_BASE + IMAP_SSP1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_SSP1_ID,
		.end	= GIC_SSP1_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ssp1_device = {
	.name		= "imap-ssp.1",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(imap_ssp1_resource),
	.resource	= imap_ssp1_resource,
};

/********************* dmic ********************************/
static struct resource imap_dmic_resource[] = {
    [0] = {
        .start = IMAP_DMIC_BASE,
		.end   = IMAP_DMIC_BASE + IMAP_DMIC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = GIC_DMIC_ID,
		.end   = GIC_DMIC_ID,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device imap_dmic_device = {
    .name          = "imap-dmic",
	.id            = -1,
	.num_resources = ARRAY_SIZE(imap_dmic_resource),
	.resource      = imap_dmic_resource,
};

/******************* pwma **********************************/
static struct resource imap_pwma_resource[] = {
    [0] = {
        .start = IMAP_PWMA_BASE,
        .end   = IMAP_PWMA_BASE + IMAP_PWMA_SIZE - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = GIC_PWMA_ID,
        .end   = GIC_PWMA_ID,
        .flags = IORESOURCE_IRQ,
    }
};

struct platform_device imap_pwma_device = {
    .name          = "imap-pwma",
    .id            = -1,
    .num_resources = ARRAY_SIZE(imap_pwma_resource),
    .resource      = imap_pwma_resource,
};

/******************* watchdog ******************************/
static struct resource imap_wtd_resource[]={
	[0] = {
		.start = IMAP_WTD_BASE,
		.end   = IMAP_WTD_BASE + IMAP_WTD_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = GIC_WTD_ID,
		.end = GIC_WTD_ID,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device imap_wtd_device = {
	.name           ="imap-wdt",
	.id             =0,
	.num_resources  = ARRAY_SIZE(imap_wtd_resource),
	.resource       = imap_wtd_resource,
};

/************Keyboard*************/
static struct resource imap_keybd_resource[]={
	[0] = {
		.start = IMAP_KEYBD_BASE,
		.end   = IMAP_KEYBD_BASE + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_KEYBD_ID,
		.end	= GIC_KEYBD_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_keybd_device = {
	.name		= "imap-keybd",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_keybd_resource),
	.resource	= imap_keybd_resource,
};

/************ adc **********/
static struct resource imap_adc_resource[]={
	[0] = {
		.start	= IMAP_ADC_BASE,
		.end	= IMAP_ADC_BASE + IMAP_ADC_SIZE -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_ADC_ID,
		.end	= GIC_ADC_ID,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device imap_adc_device = {
	.name		= "imap-adc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imap_adc_resource),
	.resource	= imap_adc_resource,
};

/************ emif **********/
static struct resource imap_emif_resource[]={
	[0] = {
		.start	= IMAP_EMIF_BASE,
		.end	= IMAP_EMIF_BASE + IMAP_EMIF_SIZE -1,
		.flags	= IORESOURCE_MEM,
	}
};

struct platform_device imap_emif_device = {
	.name		= "imap-emif",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imap_emif_resource),
	.resource	= imap_emif_resource,
};


//android pmem
#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data android_pmem_pdata = {                                      
	.name = "pmem",
	.no_allocator = 1, 
	.cached = 1,                                                                                 
};                                                                                                   

static struct android_pmem_platform_data android_pmem_adsp_pdata = {                                 
	.name = "pmem_adsp",                                                                         
	.no_allocator = 0,
	.cached = 0,
};      

struct platform_device android_pmem_device = {                                                       
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },                                             
};      

struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",                                                                      
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },                                        
};      
#endif

#ifdef CONFIG_SWITCH_WIFI
static struct gpio_switch_platform_data android_wifi_switch_pdata = {
	.name = "switch-wifi",
	.gpio = GIC_GP1_ID,
	.name_on = "switch-wifi",
	.name_off = "switch-wifi",
	.state_on = "1",
	.state_off = "0",
};

struct platform_device android_wifi_switch_device = {
	.name = "switch-wifi",
	.id = 0,
	.dev = {
		.platform_data = &android_wifi_switch_pdata
	},
};
#endif

/*******LCD*********/
static struct resource imap_lcd_resource[] = {
	[0] = {
		.start = IMAP_IDS_BASE,
		.end   = IMAP_IDS_BASE + IMAP_IDS_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= GIC_IDS0_ID,
		.end	= GIC_IDS0_ID,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_lcd_device = {
	.name		= "imap-fb",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_lcd_resource),
	.resource	= imap_lcd_resource,
};


#ifdef CONFIG_INFOTM_MEDIA_SUPPORT

#ifdef CONFIG_INFOTM_MEDIA_PMM_I800_SUPPORT
static struct resource imap_pmm_resource[] = {
	[0] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_pmm_device = {
	.name		= "imapx-pmm",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_pmm_resource),
	.resource	= imap_pmm_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_G2D_I800_SUPPORT
static struct resource imap_g2d_resource[] = {
	[0] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_g2d_device = {
	.name		= "imapx-g2d",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_g2d_resource),
	.resource	= imap_g2d_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_IPC_SUPPORT
static struct resource imap_ipc_resource[] = {
	[0] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ipc_device = {
	.name		= "imapx-ipcx",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_ipc_resource),
	.resource	= imap_ipc_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_ISP_I800_SUPPORT
static struct resource imap_isp_resource[] = {
	[0] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_isp_device = {
	.name		= "imapx-isp",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_isp_resource),
	.resource	= imap_isp_resource,
};

#endif

#ifdef CONFIG_INFOTM_MEDIA_CAMIF_I800_SUPPORT
static struct resource imap_camif_resource[] = {
	[0] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_camif_device = {
	.name		= "imapx-camif",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_camif_resource),
	.resource	= imap_camif_resource,
};

#endif


#ifdef CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT
static struct resource imap_ids_resource[] = {
	[0] = {
		.start = 0,
		.end	= 0,
		.flags = IORESOURCE_IRQ,  
	},
	[1] = {
		.start = 0,
		.end	= 0,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imap_ids_device = {
	.name		= "imapx-ids",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_ids_resource),
	.resource	= imap_ids_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_VDEC_G1_SUPPORT
static struct resource imap_vdec_resource[] = {
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_vdec_device = {
	.name		= "imap-vdec",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imap_vdec_resource),
	.resource	= imap_vdec_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_VENC_8270_SUPPORT
static struct resource imap_venc_resource[]={
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_venc_device = {
	.name		= "imap-venc",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_venc_resource),
	.resource	= imap_venc_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_DBT_SUPPORT
static struct resource imap_dbt_resource[]={
	[0] = {
		.start = 0, 
		.end   = 0, 
		.flags = IORESOURCE_MEM,
	},   
	[1] = {
		.start  = 0, 
		.end    = 0, 
		.flags  = IORESOURCE_IRQ,  
	}    
};

struct platform_device imap_dbt_device = {
	.name       = "imapx-dbt",
	.id         = -1,
	.num_resources  = ARRAY_SIZE(imap_dbt_resource),
	.resource   = imap_dbt_resource,
};
#endif

#ifdef CONFIG_INFOTM_MEDIA_UTLZ_SUPPORT
static struct resource imap_utlz_resource[]={
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= 0,
		.end	= 0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imap_utlz_device = {
	.name		= "imapx-utlz",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imap_utlz_resource),
	.resource	= imap_utlz_resource,
};
#endif

#endif

#ifdef CONFIG_RGB2VGA_OUTPUT_SUPPORT
struct platform_device imap_rgb2vga_device = {
    .name	= "imap-vga",
    .id	= 0,
};
#endif

static struct resource imap_batt_resource[] = {
	[0] = {
		.start = GIC_MGRSD_ID,
		.end = GIC_MGRSD_ID,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device android_device_batt = {       
        //.name = "battery",                               
	.name = "imap820_battery",
	.id = -1, 
       .num_resources = ARRAY_SIZE(imap_batt_resource),
	.resource = imap_batt_resource,
};                                                  
EXPORT_SYMBOL(android_device_batt);                  
#if 0
static struct gpio_switch_platform_data android_wifi_switch_pdata = {
    .name = "switch-wifi",
    .gpio = IRQ_GPIO,
    .name_on = "switch-wifi",
    .name_off = "switch-wifi",
    .state_on = "1",
    .state_off = "0",
};
#endif

struct platform_device android_wifi_switch_device = {
    .name = "switch-wifi",
    .id = 0,
    //.dev = {
        //.platform_data = &android_wifi_switch_pdata
    //},
};

#ifdef CONFIG_IMAPX_LED
struct platform_device imap_led_device = {
    .name = "led",
    .id = 0,
};
#endif

struct platform_device android_vibrator_device = {
	.name	= "timed-gpio",
	.id	= -1,
};



static struct platform_device *imap_devices[] __initdata = {
	&imap_nand_device,
	&imap_gpio_device,
	//&imap_mmc0_device,
	&imap_mmc1_device,
	&imap_mmc2_device,
	&imap_pmu_device,
	&imap_sata_device,
	&imap_ohci_device,
	&imap_ehci_device,
	&imap_otg_device,
#ifdef	CONFIG_USB_G_ANDROID
	&android_storage_device,
#endif	
	&imap_sensor_device,
//	&imap_backlight_device,
	&imap_mac_device,
	&imap_memalloc_device,
	&imap_gps_device,
	&imap_crypto_device,
	&imap_pwm_device,
	&imap_iic0_device,
	&imap_iic1_device,
	&imap_iic2_device,
	&imap_iic3_device,
	&imap_iic4_device,
	&imap_iic5_device,
	&imap_rtc_device,
	&imap_es8328_device,
	&imap_null_device,
    &imap_rt5631_device,
	&imap_iis_device0,
	&imap_iis_device1,
    &imap_pcm0_device,
    &imap_pcm1_device,
    &imap_spdif_device,
	&imap_ac97_device,
	&imap_asoc_device,
	&imap_ssp0_device,
	&imap_ssp1_device,
	&imap_dmic_device,     
    &imap_pwma_device, 
	&imap_wtd_device,
	&imap_adc_device,
    &android_device_batt,
#ifdef CONFIG_ANDROID_PMEM
	&android_pmem_device,
	&android_pmem_adsp_device,
#endif
#ifdef	CONFIG_ANDROID_TIMED_GPIO
	&android_vibrator_device,
#endif
#ifdef CONFIG_BOSCH_BMA150
#endif
//#ifdef CONFIG_SWITCH_WIFI
	&android_wifi_switch_device,
//#endif

#ifdef CONFIG_INFOTM_MEDIA_SUPPORT
#ifdef CONFIG_INFOTM_MEDIA_PMM_I800_SUPPORT
	&imap_pmm_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_UTLZ_SUPPORT
	&imap_utlz_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_G2D_I800_SUPPORT
	&imap_g2d_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_IPC_SUPPORT
	&imap_ipc_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_ISP_I800_SUPPORT
	&imap_isp_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_CAMIF_I800_SUPPORT
	&imap_camif_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT
	&imap_ids_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_VDEC_G1_SUPPORT
	&imap_vdec_device,
#endif
#ifdef CONFIG_INFOTM_MEDIA_VENC_8270_SUPPORT
	&imap_venc_device,
#endif
#endif
	&imap_lcd_device,
	&imap_backlight_device,
	&imap_emif_device,
#ifdef CONFIG_IMAPX_LED
    &imap_led_device,
#endif
};

static struct platform_device *imap_dev_simple[] __initdata = {
	&imap_nand_device,
	&imap_gpio_device,
	&imap_pmu_device,
    &imap_ohci_device,
    &imap_ehci_device,
    &imap_otg_device,
	&imap_memalloc_device,
	&imap_pwm_device,
	&imap_iic0_device,
	&imap_iic1_device,
	&imap_iic2_device,
	&imap_iic3_device,
	&imap_iic4_device,
	&imap_iic5_device,
	&imap_rtc_device,
	&imap_null_device,
	&imap_ssp0_device,
	&imap_ssp1_device,
	&imap_wtd_device,
	&imap_adc_device,
	&android_device_batt,
#ifdef CONFIG_ANDROID_PMEM
	&android_pmem_device,
	&android_pmem_adsp_device,
#endif
#ifdef	CONFIG_ANDROID_TIMED_GPIO
	&android_vibrator_device,
#endif

	&imap_ids_device,
	&imap_lcd_device,
	&imap_backlight_device,
	&imap_emif_device,
#ifdef CONFIG_IMAPX_LED
	&imap_led_device,
#endif
};

static struct amba_device *amba_devs[] __initdata = {
	&imap_uart0_device,
	&imap_uart1_device,
	&imap_uart2_device,
	&imap_uart3_device,
	&imap_pic0_device,
    &imap_dma_device,
};

void usb_disable(void);
void snd_disable(void);
void logo_disable(void);
void tp_disable(void);
void __init imap_init_devices(void)
{
    int i;

    /* Register the AMBA devices in the AMBA bus abstraction layer */
    for(i=0; i< ARRAY_SIZE(amba_devs); i++) {
	struct amba_device *d = amba_devs[i];
	amba_device_register(d, &iomem_resource);
    }	

    if(strstr(boot_command_line, "charger")) {
	    //usb_disable();
	    snd_disable();
	    logo_disable();
	    tp_disable();
	    platform_add_devices(imap_dev_simple, ARRAY_SIZE(imap_dev_simple));
    }
    else
	    platform_add_devices(imap_devices, ARRAY_SIZE(imap_devices));
}

EXPORT_SYMBOL(imap_init_devices);

