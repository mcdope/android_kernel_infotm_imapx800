
#include <linux/module.h>
#include <linux/ioport.h>
#include <mach/imap-iomap.h>
#include <mach/imap-mac.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/init.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <mach/items.h>

//#include "gmac-univ_ethernet.h"

#if 1
#define SWR                     (0x1<<0)
#define PHYBUSY		0x1
#define DELAY		10000
#define MAC_SYSM_RESET              (SYSMGR_MAC_BASE + 0x0)
#define MAC_SYSM_CLK_EN             (SYSMGR_MAC_BASE + 0x4)
#define MAC_SYSM_POWM               (SYSMGR_MAC_BASE + 0x8)
#define MAC_SYSM_BUSOE              (SYSMGR_MAC_BASE + 0xc)
#define MAC_SYSM_RMII               (SYSMGR_MAC_BASE + 0x20)
#define MAC_SYSM_FPGA0              (SYSMGR_MAC_BASE + 0x28)
#define MAC_SYSM_FPGA1              (SYSMGR_MAC_BASE + 0x2c)
#endif

typedef struct {
	int id0;
	int id1;
}ETHPHY_ID;

ETHPHY_ID eth_id[] = {
	{0x0243, 0x0c54}, //IP101A
	{0x0, 0x0},
};

void pcb_gmac_hw_init(void)
{
#if defined(CONFIG_ARCH_IMAPX200)
	imapx_gpio_setcfg(IMAPX_GPK_ALL, IG_CTRL0, IG_NORMAL);
	imapx_gpio_setcfg(IMAPX_GPJ8, IG_CTRL0, IG_NORMAL);
#else
    /*  for X800 or later */
    module_power_on(SYSMGR_MAC_BASE);
	/* MAC pads */
	imapx_pad_cfg(IMAPX_PHY, 1);
	/* configure eth to 100MHZ */
	writel(0x3, IO_ADDRESS(MAC_SYSM_RMII));

    /* reset eth in system manager */
    writel(0xff, IO_ADDRESS(MAC_SYSM_RESET));
    //udelay(5);
    msleep(1);
    writel(0x0, IO_ADDRESS(MAC_SYSM_RESET));
#endif
}

static int pcb_gmac_reset()
{
	uint32_t ret = -1;
	uint32_t tmp, i = 0;

	void __iomem	*io_addr;	//Register I/O base address

	io_addr = ioremap(IMAP_MAC_BASE, 0x2000);
	/* reset device */
	tmp = readl(io_addr + rBUSMODE);                                            
	tmp |= SWR;
	writel(tmp, io_addr + rBUSMODE);         //reset all Gmac registers
	for (i=0; i<DELAY; i++) 
	{                                
		if ((readl(io_addr + rBUSMODE) & SWR) == 0)
		{
			ret = 0;
			break;
		}
	}                                                            
	                                                             
	if (i >= DELAY)                                              
	{
		printk("gmac reset failed, no clk input\n");          
	}

	return ret;
}

static int pcb_gmac_phy_read(int reg)
{
	int ret = -1, i = 0;
	void __iomem	*io_addr;	//Register I/O base address

	io_addr = ioremap(IMAP_MAC_BASE, 0x2000);
//	mutex_lock(&db->addr_lock);
	while (readl(io_addr + rGMIIAddr) & PHYBUSY)
	{
		;		//wait for free
	}

	/* Fill the phyxcer register into GMII Address register */
	writel((0x1<<11) | (reg<<6) | (0x1<<3) |(0x0<<1) | PHYBUSY, io_addr + rGMIIAddr);
	for (i=0; i<DELAY; i++)
	{
		if ((readl(io_addr + rGMIIAddr) & PHYBUSY) == 0)
		{
			break;
		}
	}

	if (i == DELAY)
	{
	    printk("phy read timed out\n");
	}else{
		ret = readl(io_addr + rGMIIData);
		printk("eth id = 0x%x\n", ret);
	}

//	mutex_unlock(&db->addr_lock);
	return ret;
}

int eth_readid(void){
	
	int reg0, reg1;
	int id0, id1;

    eth_id[0].id0 = item_integer("eth.id0",0);
    eth_id[0].id1 = item_integer("eth.id1",0);
	printk("get items 0x%x, 0x%x\n", eth_id[0].id0, eth_id[0].id1);

	    pcb_gmac_hw_init();
	if(pcb_gmac_reset()!=0)
		return -1;

	reg0 = 2;
	id0 = pcb_gmac_phy_read(reg0);
	reg1 = 3;
	id1 = pcb_gmac_phy_read(reg1);
	if((id0 != eth_id[0].id0) || (id1 != eth_id[0].id1))
		return -1;

	return 0;

}

