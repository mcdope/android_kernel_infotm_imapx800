#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <mach/usb-phy.h>
#include <mach/items.h>

#define IMAP_DEBUG	 0 

#if IMAP_DEBUG
#define IMAP_EHCI_DEBUG(debugs, ...)  printk(KERN_INFO debugs,  ##__VA_ARGS__)
#else
#define IMAP_EHCI_DEBUG(debugs, ...) do{}while(0)
#endif

static struct clk *bus_clk;
static struct clk *ref_clk;
static struct usb_hcd *imap_ehci_hcd;

static int ehci_imapx800_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int ret = 0;

	IMAP_EHCI_DEBUG("++ ehci_imapx800_init ++\n");
	

	ehci->caps = hcd->regs;
	IMAP_EHCI_DEBUG("hcd->regs is 0x%x hcs_params is 0x%x \r\n",hcd->regs,ehci_readl(ehci,&ehci->caps->hc_capbase));
	ehci->regs = hcd->regs + HC_LENGTH(ehci, ehci_readl(ehci,&ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci,&ehci->caps->hcs_params);


	IMAP_EHCI_DEBUG("ehci->caps is 0x%x \r\n",ehci->caps);
	IMAP_EHCI_DEBUG("ehci->regs is 0x%x \r\n",ehci->regs);
	
	ret = ehci_halt(ehci);
	if (ret)
		return ret;


	ret = ehci_init(hcd);
	if(ret)
		return ret;

	ehci_reset(ehci);
	IMAP_EHCI_DEBUG("--ehci_imapx800_init \r\n");
	return ret;
}

static int usb_hcd_imapx800_probe(const struct hc_driver *driver,struct platform_device *pdev)
{
	struct platform_data *pInfo;
	struct usb_hcd *hcd;
	struct resource *r_mem;
	int irq, ret = 0;
	int clk_rate;
	
	IMAP_EHCI_DEBUG("++usb_hcd_imapx800_probe\r\n");
	pInfo = pdev->dev.platform_data;
	if(pInfo)
	{
		dev_err(&pdev->dev,"This Paramer <pdev> err !");
		return -ENODEV;
	}
	
	irq = platform_get_irq(pdev,0);
	if(irq < 0)
	{
		dev_err(&pdev->dev,"no resource of IORESOURCE_IRQ");
		return -ENXIO;
	}

	IMAP_EHCI_DEBUG("USB Resource IRQ is 0x%x \r\n",irq);

	r_mem = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(!r_mem)
	{
		dev_err(&pdev->dev,"no resource of IORESOURCE_MEM");
		return -ENXIO;
	}

	hcd = usb_create_hcd(driver, &pdev->dev,dev_name(&pdev->dev));
	if(!hcd)
	{
		dev_err(&pdev->dev,"usb_create_hcd failed!");
		return  -ENOMEM;
	}

	imap_ehci_hcd = hcd;
	hcd->rsrc_start = r_mem->start;
	hcd->rsrc_len = r_mem->end - r_mem->start + 1;
	IMAP_EHCI_DEBUG("USB Resource MEM is 0x%x \r\n",r_mem->start);
	IMAP_EHCI_DEBUG("USB Resource len is 0x%x \r\n",hcd->rsrc_len);
	
	if(!request_mem_region(hcd->rsrc_start,hcd->rsrc_len,driver->description))
	{
		dev_err(&pdev->dev,"request_mem_region failed");
		ret = -EBUSY;
		goto err_put;
	}

	bus_clk = clk_get(NULL, "usb ehci");
	if(IS_ERR(bus_clk)) {
		dev_err(&pdev->dev, "can't get usb bus clock\n");
		ret = PTR_ERR(bus_clk);
		goto err_mem;
	}
        clk_enable(bus_clk);

	ref_clk = clk_get(NULL, "usb-ref");
	if(IS_ERR(ref_clk)) {
		dev_err(&pdev->dev, "can't get phy ref clock\n");
		ret = PTR_ERR(ref_clk);
		goto err_clk;
	}
        clk_enable(ref_clk);
	
	hcd->regs = ioremap(hcd->rsrc_start,hcd->rsrc_len);
	if(!hcd->regs)
	{
		dev_err(&pdev->dev,"ioremap_nocache failed");
		ret = -ENOMEM;
		goto err_ioremap;
	}

	IMAP_EHCI_DEBUG("USB Register Map Address 0x%x \r\n",hcd->regs);

	clk_rate = clk_get_rate(ref_clk);
	printk("usb host ref_clock = %d\n", clk_rate);
        if(item_exist("otg.function"))
        {
            if(item_equal("otg.function", "usb", 0))
	        otg_phy_config(clk_rate, 1, 0);
        }

	host_phy_config(clk_rate, 1);

	dev_set_drvdata(&pdev->dev,hcd);	
	ret = usb_add_hcd(hcd,irq,IRQF_DISABLED);
	if(ret)
		goto err_ioremap;

	IMAP_EHCI_DEBUG("--usb_hcd_imapx800_probe\r\n");

	return ret;

err_ioremap:
	iounmap(hcd->regs);
	clk_put(ref_clk);
err_clk:
	clk_put(bus_clk);
err_mem:	
	release_mem_region(hcd->rsrc_start,hcd->rsrc_len);
err_put:
	usb_put_hcd(hcd);
	return ret;
}

static int usb_hcd_imapx800_remove(struct usb_hcd *hcd,struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start,hcd->rsrc_len);
	usb_put_hcd(hcd);

	return 0;
}

static const struct hc_driver ehci_imapx800_hc_driver = 
{
	.description = hcd_name,
	.product_desc = "Infotm Ehci Controller",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	.irq = ehci_irq,
	.flags = HCD_USB2 | HCD_MEMORY,

	.reset = ehci_imapx800_init,
	.start = ehci_run,
	.stop = ehci_stop,
	.shutdown = ehci_shutdown,

	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,
	.endpoint_reset = ehci_endpoint_reset,

	.get_frame_number = ehci_get_frame,

	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
#if defined(CONFIG_PM)	
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
#endif	
	.relinquish_port = ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

uint32_t imap_ehci_port_state(void)
{
	uint32_t temp;
	struct ehci_hcd *ehci = hcd_to_ehci(imap_ehci_hcd);

	temp = ehci_readl(ehci, &ehci->regs->port_status[2]);
	return temp&0x1;	
}
EXPORT_SYMBOL(imap_ehci_port_state);

void imap_ehci_port_power(int flag)
{
	uint32_t temp;
	struct ehci_hcd *ehci = hcd_to_ehci(imap_ehci_hcd);

	temp = ehci_readl(ehci, &ehci->regs->port_status[2]);
	if(flag)
	{
		temp |= PORT_POWER;
		ehci_writel(ehci, temp, &ehci->regs->port_status[2]);
	}
	else
	{
		temp &=~ PORT_POWER;
		ehci_writel(ehci, temp, &ehci->regs->port_status[2]);
	}
	
}
EXPORT_SYMBOL(imap_ehci_port_power);

static int ehci_imapx800_drv_probe(struct platform_device *pdev)
{
        if(item_exist("usb.enable"))
        {
            if(item_integer("usb.enable", 0) == 0)
                return -ENODEV;
        }
	if(usb_disabled())
		return -ENODEV;

	return usb_hcd_imapx800_probe(&ehci_imapx800_hc_driver,pdev);
}

static int ehci_imapx800_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_imapx800_remove(hcd,pdev);
	return 0;
}


#ifdef CONFIG_PM

static int ehci_hcd_imapx800_drv_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	unsigned long flags;
	int rc;

	rc = 0;

	if (time_before(jiffies, ehci->next_statechange))
		msleep(10);

	/* Root hub was already suspended. Disable irq emission and
	 * mark HW unaccessible, bail out if RH has been resumed. Use
	 * the spinlock to properly synchronize with possible pending
	 * RH suspend or resume activity.
	 *
	 * This is still racy as hcd->state is manipulated outside of
	 * any locks =P But that will be a different fix.
	 */
	ehci_prepare_ports_for_controller_suspend(ehci, device_may_wakeup(dev));
	spin_lock_irqsave(&ehci->lock, flags);
	ehci_writel(ehci, 0, &ehci->regs->intr_enable);
	(void)ehci_readl(ehci, &ehci->regs->intr_enable);

	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	spin_unlock_irqrestore(&ehci->lock, flags);

	// could save FLADJ in case of Vaux power loss
	// ... we'd only use it to handle clock skew

	return rc;

}

static int ehci_hcd_imapx800_drv_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
#if 0
	int clk_rate;

	clk_rate = clk_get_rate(ref_clk);
	printk("usb host ref_clock = %d\n", clk_rate);
	host_phy_config(clk_rate, 1);
	otg_phy_config(clk_rate, 0, 1);
#endif
	// maybe restore FLADJ

	if (time_before(jiffies, ehci->next_statechange))
		msleep(100);

	/* Mark hardware accessible again as we are out of D3 state by now */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);

	/* If CF is still set, we maintained PCI Vaux power.
	 * Just undo the effect of ehci_pci_suspend().
	 */
	if (ehci_readl(ehci, &ehci->regs->configured_flag) == FLAG_CF) {
		int	mask = INTR_MASK;

		ehci_prepare_ports_for_controller_resume(ehci);
		if (!hcd->self.root_hub->do_remote_wakeup)
			mask &= ~STS_PCD;
		ehci_writel(ehci, mask, &ehci->regs->intr_enable);
		ehci_readl(ehci, &ehci->regs->intr_enable);
		return 0;
	}

	ehci_dbg(ehci, "lost power, restarting\n");
	usb_root_hub_lost_power(hcd->self.root_hub);

	/* Else reset, to cope with power loss or flush-to-storage
	 * style "resume" having let BIOS kick in during reboot.
	 */
	(void) ehci_halt(ehci);
	(void) ehci_reset(ehci);

	/* emptying the schedule aborts any urbs */
	spin_lock_irq(&ehci->lock);
	if (ehci->reclaim)
		end_unlink_async(ehci);
	ehci_work(ehci);
	spin_unlock_irq(&ehci->lock);

	ehci_writel(ehci, ehci->command, &ehci->regs->command);
	ehci_writel(ehci, FLAG_CF, &ehci->regs->configured_flag);
	ehci_readl(ehci, &ehci->regs->command);	/* unblock posted writes */

	/* here we "know" root ports should always stay powered */
	ehci_port_power(ehci, 1);

	hcd->state = HC_STATE_SUSPENDED;

	return 0;

}	

static struct dev_pm_ops imapx800_ehci_pmops = {
	.suspend = ehci_hcd_imapx800_drv_suspend,
	.resume = ehci_hcd_imapx800_drv_resume,
};

#define IMAPX800_EHCI_PMOPS &imapx800_ehci_pmops

#else
#define IMAPX800_EHCI_PMOPS NULL
#endif


MODULE_ALIAS("platform:imap-ehci");

static struct platform_driver ehci_imapx800_driver = 
{
	.probe = ehci_imapx800_drv_probe,
	.remove = ehci_imapx800_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.name = "imap-ehci",
		.owner	= THIS_MODULE,
		.pm	= IMAPX800_EHCI_PMOPS,
	},
};
