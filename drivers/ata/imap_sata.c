#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#include <linux/libata.h>
#include <mach/imap-iomap.h>
#include <mach/pad.h>
#include <mach/power-gate.h>
#include "imap_sata.h"

#define SATA_DEBUG
#ifdef SATA_DEBUG
#define sata_debug(x...)	printk(KERN_INFO x)
#else
#define sata_debug(x...)	
#endif

#define DRV_NAME	"imap-sata"
#define DRV_VERSION	"3.0"

static struct scsi_host_template imap_sata_sht = {
	AHCI_SHOST(DRV_NAME),
};

static const struct ata_port_info imap_sata_port_info[] = {
	{
		.flags          = IMAP_FLAG_COMMON, 
		.pio_mask       = ATA_PIO4,
		.udma_mask      = ATA_UDMA6,
		.port_ops       = &ahci_ops,
	},
};

static void imap_sata_save_initial_config(struct device *dev, struct ahci_host_priv *hpriv)
{
	unsigned int force_port_map = 0;
	unsigned int mask_port_map = 0;
	
	ahci_save_initial_config(dev, hpriv, force_port_map, mask_port_map);
}

static int imap_sata_reset_controller(struct ata_host *host)
{
	int ret;

	ret = ahci_reset_controller(host);

	return ret;
}

static void imap_sata_init_controller(struct ata_host *host)
{
	ahci_init_controller(host);
}

static void imap_sata_print_info(struct ata_host *host)
{
	ahci_print_info(host, "AHCI");
}

static void imap_sata_phy_config(void __iomem *base)/*TODO:some value need to be modified because of the given refclk*/
{
	writel(0x1, base + PHY_MPLL_CK_OFF);
	writel(0x10, base + PHY_ACJT_LVL);
	writel(0, base + PHY_RX_EQ_VAL_0);
	writel(0x6, base + PHY_MPLL_NCY);
	writel(0x2, base + PHY_MPLL_NCY5);
	writel(0x2, base + PHY_MPLL_PRESCALE);
	writel(0, base + PHY_MPLL_SS_SEL);
	writel(0, base + PHY_MPLL_SS_EN);
	writel(0, base + PHY_TX_EDGERATE_0);
	writel(0x12, base + PHY_LOS_LVL);
	writel(0x1, base + PHY_USE_REFCLK_ALT);
	writel(0, base + PHY_REF_CLK_SEL);
	writel(0x9, base + PHY_TX_LVL);
	writel(0, base + PHY_TX_ATTEN_0);
	writel(0xA, base + PHY_TX_BOOST_0);
	writel(0x3, base + PHY_RX_DPLL_MODE_0);
	writel(0x1, base + PHY_RX_ALIGN_EN);
	writel(0, base + PHY_MPLL_PROP_CTL);
	writel(0, base + PHY_MPLL_INT_CTL);
	writel(0x2, base + PHY_LOS_CTL_0);
	writel(0x1, base + PHY_RX_TERM_EN);
	writel(0x3, base + PHY_SATA_SPDMODE_CTL);
	writel(0x10, base + PHY_BS_CTL_CFG);
	writel(0, base + PHY_MPLL_CK_OFF);

	udelay(2000);
}

static int imap_sata_init_one(struct platform_device *ofdev)
{
	struct ata_port_info pi = imap_sata_port_info[0];
	const struct ata_port_info *ppi[] = { &pi, NULL };
	struct ahci_host_priv *hpriv = NULL;
	struct ata_host *host;
	void __iomem *base = NULL;
	void __iomem *phy_base = NULL;
	int irq = 0;
	int ret = -ENXIO;
	int n_ports,i;

	sata_debug("ENTER sata ahci driver init\n");

	WARN_ON((int)ATA_MAX_QUEUE > AHCI_MAX_CMDS);

	/* pad config for sata */
	if(imapx_pad_cfg(IMAPX_SATA, 0) == -1)
	{
		printk(KERN_ERR "imapx800 SATA module init failed, please check case\n");
		goto err_exit;
	}

	base = of_iomap(ofdev->dev.of_node, 0);
	if(!base)
		goto err_exit;

	phy_base = IO_ADDRESS(SYSMGR_SATA_BASE);

	/* phy registers config */
	imap_sata_phy_config(phy_base);

	hpriv = kzalloc(sizeof(struct ahci_host_priv), GFP_KERNEL);
	if(!hpriv)
		goto err_exit;

	hpriv->mmio = base;
	hpriv->phy_base = phy_base;
	hpriv->flags |= (unsigned long)pi.private_data;

	/* save initial config */
	imap_sata_save_initial_config(&ofdev->dev, hpriv);

	/* prepare host */
	if(hpriv->cap & HOST_CAP_NCQ)
	{
		pi.flags |= ATA_FLAG_NCQ;

		/* Auto-activate optimization is supposed to be
		 * supported on all AHCI controllers indicating
		 * NCQ capability, but it seems to be broken on
		 * some chipsets including NVIDIAs */
		if(!(hpriv->flags & AHCI_HFLAG_NO_FPDMA_AA))
			pi.flags |= ATA_FLAG_FPDMA_AA;
	}

	if(hpriv->cap & HOST_CAP_PMP)
		pi.flags |= ATA_FLAG_PMP;

	/* for imapx800 sata ahci has no em func,
	 * so em statement will not be excuted,
	 * just reserved here */
	ahci_set_em_messages(hpriv, &pi);

	/* CAP.NP sometimes indicate the index of the last enabled
	 * port, at other times, that of the last possible port, so
	 * determining the maximum port number requires looking at
	 * both CAP.NP and port_map. */
	n_ports = max(ahci_nr_ports(hpriv->cap), fls(hpriv->port_map));

	host = ata_host_alloc_pinfo(&ofdev->dev, ppi, n_ports);
	if(!host)
	{
		printk(KERN_ERR "ata_host_alloc_pinfo failed\n");
		goto err_exit;
	}
	host->private_data = hpriv;

	if(!(hpriv->cap & HOST_CAP_SSS) || ahci_ignore_sss)
		host->flags |= ATA_HOST_PARALLEL_SCAN;
	else
		sata_debug("sata ahci: SSS flag set, parallel bus scan disabled\n");

	if(pi.flags & ATA_FLAG_EM)
		ahci_reset_em(host);

	for(i = 0;i < host->n_ports;i++)
	{
		struct ata_port *ap = host->ports[i];

		if(ap->flags & ATA_FLAG_EM)
			ap->em_message_type = hpriv->em_msg_type;

		if(!(hpriv->port_map & (1 << i)))
			ap->ops = &ata_dummy_port_ops;
	}

	ret = imap_sata_reset_controller(host);
	if(ret)
		goto err_exit;

	imap_sata_init_controller(host);
	imap_sata_print_info(host);

	irq = irq_of_parse_and_map(ofdev->dev.of_node, 0);
	if(irq == NO_IRQ)
	{
		printk(KERN_ERR "Get SATA DMA irq failed\n");
		goto err_exit;
	}
	hpriv->irq = irq;

	ret = ata_host_activate(host, hpriv->irq, ahci_interrupt, 0, &imap_sata_sht);
	if(ret)
	{
		printk(KERN_ERR "failed to activate host\n");
		goto err_exit;
	}

	dev_set_drvdata(&ofdev->dev, host);

	return 0;

err_exit:

	if(irq)
		free_irq(hpriv->irq, hpriv);

	if(base)
		iounmap(base);

	if(hpriv)
		kfree(hpriv);

	return ret;
}

static int imap_sata_remove_one(struct platform_device *ofdev)
{
	struct ata_host *host = dev_get_drvdata(&ofdev->dev);
	struct ahci_host_priv *hpriv = host->private_data;

	ata_host_detach(host);
	dev_set_drvdata(&ofdev->dev, NULL);
	free_irq(hpriv->irq, hpriv);
	//irq_dispose_mapping(hpriv->irq);
	iounmap(hpriv->mmio);
	kfree(hpriv);

	return 0;
}

#ifdef CONFIG_PM
static int imap_sata_device_suspend(struct platform_device *ofdev, pm_message_t mesg)
{
	struct ata_host *host = dev_get_drvdata(&ofdev->dev);

	return ata_host_suspend(host, mesg);
}

static int imap_sata_device_resume(struct platform_device *ofdev)
{
	struct ata_host *host = dev_get_drvdata(&ofdev->dev);
	int ret;

	if(ofdev->dev.power.power_state.event == PM_EVENT_SUSPEND)
	{
		ret = imap_sata_reset_controller(host);
		if(ret)
		{
			printk(KERN_ERR "SATA resume fail: reset controller fail\n");
			return ret;
		}
	}

	imap_sata_init_controller(host);

	ata_host_resume(host);

	return 0;
}
#endif

static const struct of_device_id imap_sata_match[] = {
	{
		.compatible = "company,sata-name",/*TODO:the string should be info corresponding to the compatible sata device*/
	},
	{}
};
MODULE_DEVICE_TABLE(of, imap_sata_match);

struct platform_driver imap_sata_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = imap_sata_match,
	},
	.probe		= imap_sata_init_one,
	.remove		= imap_sata_remove_one,
#ifdef CONFIG_PM
	.suspend	= imap_sata_device_suspend,
	.resume		= imap_sata_device_resume,
#endif
};

static int __init imap_sata_init(void)
{
	sata_debug("sata driver init entry\n");
	module_power_on(SYSMGR_SATA_BASE);
	return platform_driver_register(&imap_sata_driver);
}
module_init(imap_sata_init);

static void __exit imap_sata_exit(void)
{
	platform_driver_register(&imap_sata_driver);
}
module_exit(imap_sata_exit);

MODULE_AUTHOR("Jeff Garzik <jgarzik@pobox.com>");
MODULE_DESCRIPTION("IMAPx800 SATA AHCI low-level driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
