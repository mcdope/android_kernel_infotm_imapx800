#ifndef __USB_PHY_H__
#define __USB_PHY_H__


int host_phy_config(int32_t ref_clk, int utmi_16bit);
int otg_phy_config(int32_t ref_clk, int otg_flag, int utmi_16bit);
int otg_phy_reconfig(int utmi_16bit, int otg_flag);

#endif
