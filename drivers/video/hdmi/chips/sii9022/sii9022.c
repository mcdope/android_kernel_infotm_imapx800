#include <linux/gpio.h>
#include <plat/imapx.h>
#include "SIITPI.h"
#include "SIITPI_Access.h"
#include "Externals.h"
#include "SIITPI_Regs.h"
#include "SIIConstants.h"
#include "SIIMacros.h"
#include "SIIedid.h"
#include "SIIdefs.h"
#include "SIIAV_Config.h"
#include "../../hdmi.h"


/*0 success 1 failure*/
int sii9022_chip_init(void)
{
	return !TPI_Init();
}

void sii9022_power(uint32_t on_off)
{
	unsigned long gpio;     

	gpio = __imapx_name_to_gpio(CONFIG_HDMI_SI9022_POWER);

	if(gpio == IMAPX_GPIO_ERROR)    
		/* power gpio is not configurated */
		return ;

	imapx_gpio_setcfg(gpio, IG_OUTPUT, IG_NMSL);
	if(on_off)
		imapx_gpio_setpin(gpio, 1, IG_NMSL);
	else
		imapx_gpio_setpin(gpio, 0, IG_NMSL);
}

int sii9022_get_connection_status(void)
{
	return ((ReadByteTPI(TPI_INTERRUPT_STATUS_REG) & HOT_PLUG_STATE) >>2);
}

void sii9022_mute_output(video_timing timing, uint32_t mute)
{
	if(mute)
//		OnHdmiCableDisconnected();
		DisableTMDS();
	else{
		OnHdmiCableConnected(timing);                                 
		ReadModifyWriteIndexedRegister(INDEXED_PAGE_0, 0x0A, 0x08, 0x08);
	}
}

struct hdmi_ops sii9022_ops = {
	.chip_init		= sii9022_chip_init,
	.power			= sii9022_power,
	.get_connection_status	= sii9022_get_connection_status,	
	.mute_output		= sii9022_mute_output,
};
EXPORT_SYMBOL(sii9022_ops);
