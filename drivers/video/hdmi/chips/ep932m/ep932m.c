#include <linux/gpio.h>
#include <plat/imapx.h>
#include "../../hdmi.h"
#include "EP932api.h"
#include "EP932Controller.h"

int ep932m_chip_init(void)
{
	//EP932_IIC_Initial();
	//EP_EP932M_Reset(); 
	EP_HDMI_Init();			// modify - Eric.Lu
	return 0;
}

void ep932m_power(uint32_t on_off)
{
	unsigned long gpio;

	gpio = __imapx_name_to_gpio(CONFIG_HDMI_EP932_POWER);

	if(gpio == IMAPX_GPIO_ERROR)
		/* power gpio is not configurated */
		return ;

	imapx_gpio_setcfg(gpio, IG_OUTPUT, IG_NMSL);

	if(on_off)
		imapx_gpio_setpin(gpio, 1, IG_NMSL);
	else
		imapx_gpio_setpin(gpio, 0, IG_NMSL);
}

int ep932m_get_connection_status(void)
{
	return EP932_HotPlugMonitorInt();
}

void ep932m_mute_output(video_timing timing, uint32_t mute)
{
	if(mute)
	{
		HDMI_Tx_Mute_Enable();
		HDMI_Tx_Power_Down(); 
	}else{
		 hdmi_main(timing);
	}

}

struct hdmi_ops ep932m_ops = {
	.chip_init              = ep932m_chip_init,
	.power                  = ep932m_power,
	.get_connection_status  = ep932m_get_connection_status,
	.mute_output            = ep932m_mute_output,
};
EXPORT_SYMBOL(ep932m_ops);


