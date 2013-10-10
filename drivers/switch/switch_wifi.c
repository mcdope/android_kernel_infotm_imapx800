#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <asm/io.h>
//#include <plat/imapx.h>
#include <mach/pad.h>
#include <mach/items.h>
#include <mach/power-gate.h>
#include <linux/regulator/consumer.h>
#include <../arch/arm/mach-omap2/mux.h>
#include <linux/err.h>

#ifdef CONFIG_IG_WIFI_SDIO
#define __mmc_rescan(x) sdhci_mmc##x##_detect_change()
#define _mmc_rescan(x) __mmc_rescan(x)
extern void	_mmc_rescan(CONFIG_IG_WIFI_SDIO_CHANNEL);
#endif

int wifi_first_flag = 1;
char gMacAddr[6] = {0x0};
EXPORT_SYMBOL(wifi_first_flag);
EXPORT_SYMBOL(gMacAddr);

struct wifi_switch_data {
//	struct switch_dev sdev;
	unsigned gpio_power;
//	unsigned gpio_switch;
//	const char *name_on;
//	const char *name_off;
//	const char *state_on;
//	const char *state_off;
//	int irq;
    struct regulator *regulator;
};

static struct wifi_switch_data *switch_data;
static int reset = 153;

//static struct work_struct switch_wifi_work;

void wifi_power(int power)
{
	if (power == 1)
	{
		if (item_equal("wifi.module", "sdio", 0))
		{
			reset = item_integer("wifi.reset",1);
			imapx_pad_set_mode(1, 1, reset);
			imapx_pad_set_dir(0, 1, reset);
			imapx_pad_set_outdat(0, 1, reset);
			msleep(20);
			if (switch_data->regulator)
			{
				regulator_enable(switch_data->regulator);
			}
			else if (switch_data->gpio_power)
			{
				imapx_pad_set_mode(1,1,switch_data->gpio_power);
				imapx_pad_set_dir(0,1,switch_data->gpio_power);
				if(item_integer("wifi.power", 1) >= 0)
					imapx_pad_set_outdat(power,1,switch_data->gpio_power);
				else
					imapx_pad_set_outdat(!power,1,switch_data->gpio_power);
			}
			msleep(10);
			imapx_pad_set_outdat(1, 1, reset);
			msleep(30);
		}
		else
		{
			if (switch_data->regulator)
			{
				regulator_enable(switch_data->regulator);
			}
			else if (switch_data->gpio_power)
			{
				imapx_pad_set_mode(1,1,switch_data->gpio_power);
				imapx_pad_set_dir(0,1,switch_data->gpio_power);
				if(item_integer("wifi.power", 1) >= 0)
					imapx_pad_set_outdat(power,1,switch_data->gpio_power);
				else
					imapx_pad_set_outdat(!power,1,switch_data->gpio_power);
			}
		}
	}
	else 
	{
		if (item_equal("wifi.module", "sdio", 0))
		{
			msleep(20);
			if (switch_data->regulator)
				regulator_disable(switch_data->regulator);
			else if (switch_data->gpio_power)
			{
				if(item_integer("wifi.power", 1) >= 0)
					imapx_pad_set_outdat(power,1,switch_data->gpio_power);
				else
					imapx_pad_set_outdat(!power,1,switch_data->gpio_power);
			}

			if (item_exist("wifi.reset"))
			{
				reset = item_integer("wifi.reset",1);
				imapx_pad_set_outdat(0, 1, reset);
			}
		}
		else
		{
			if (switch_data->regulator)
				regulator_disable(switch_data->regulator);
			else if (switch_data->gpio_power)
			{
				if(item_integer("wifi.power", 1) >= 0)
					imapx_pad_set_outdat(power,1,switch_data->gpio_power);
				else
					imapx_pad_set_outdat(!power,1,switch_data->gpio_power);
			}
		}
	}
}
EXPORT_SYMBOL(wifi_power);
#if 0

static void gpio_switch_work(struct work_struct *work)
{
	//	printk("enter function %s, at line %d \n", __func__, __LINE__);

	//switch_set_state(&switch_data->sdev,
	//  imapx_gpio_getpin(switch_data->gpio_switch, IG_NORMAL));
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	//	printk("enter function %s, at line %d \n", __func__, __LINE__);
	//if(imapx_gpio_is_pending(switch_data->gpio_switch, 1))
	// schedule_work(&switch_wifi_work);

	return IRQ_HANDLED;
}

static ssize_t switch_wifi_print_state(struct switch_dev *sdev, char *buf)
{
	const char *state;
	//printk("enter function %s, at line %d \n", __func__, __LINE__);
	if (switch_get_state(sdev))
		state = switch_data->state_on;
	else
		state = switch_data->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);

	return -1;
}
#endif
static int wifi_switch_probe(struct platform_device *pdev)
{
	//	struct gpio_switch_platform_data *pdata = pdev->dev.platform_data;
	int ret;
	char buf[64];
	char channel[100];
	int index;

	//printk("enter function %s, at line %d \n", __func__, __LINE__);


	switch_data = kzalloc(sizeof(struct wifi_switch_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;


#ifdef CONFIG_IG_KEYS_POWERS
	//	switch_data->gpio_switch = __imapx_name_to_gpio(CONFIG_IG_KEYS_WIFI);
#else
	//	switch_data->gpio_switch = IMAPX_GPIO_ERROR;
#endif

	if(item_exist("wifi.power"))
	{
		item_string(buf, "wifi.power", 0);
		if(!strncmp(buf,"pads",4))
		{
			index = item_integer("wifi.power", 1);
			index = (index < 0)? (-index):index;
			imapx_pad_set_mode(1,1,index);
			imapx_pad_set_dir(0,1,index);
			imapx_pad_get_indat(index);
			switch_data->gpio_power = index;
			printk("pads type wifi power, index is %d \n",index);
		}
		//else if(!strcmp(buf,"pmu"))
		else if(!strncmp(buf,"pmu",3))
		{
			item_string(buf, "wifi.power", 1);
			// sprintf(channel, "tps65910_%s", buf);
			printk("wifi.power is %s\n", buf);
			switch_data->regulator = regulator_get(&pdev->dev, buf);
			//switch_data->regulator = regulator_get(&pdev->dev, "TPS65910_VAUX1");
			if(IS_ERR(switch_data->regulator))
			{
				printk("%s: get regulator fail\n", __func__);
				return -1;
			}
			if (item_equal("pmu.model", "axp152", 0))
				regulator_set_voltage(switch_data->regulator, 3300000, 3300000);
			else
			{
				if (!strncmp(buf,"vaux1",5))
				{
					regulator_set_voltage(switch_data->regulator, 2800000, 2800000);
				}
				else if (!strncmp(buf,"vmmc",4))
				{
					regulator_set_voltage(switch_data->regulator, 3300000, 3300000);
				}
				else 
					regulator_set_voltage(switch_data->regulator, 3300000, 3300000);
			}

		}
		else
			printk("wif.power  %s is congiged to wrong type\n", channel);
	}
	else
		printk("do not config wifi.power in board.itms\n");

	wifi_power(1);
	wifi_power(0);

	//if(switch_data->gpio_switch == IMAPX_GPIO_ERROR)
	//{
	//	printk(KERN_ERR "get wifi powers/switch pins failed.\n");
	//	return -ENOTTY;
	//}
#if 0

	switch_data->sdev.name = pdata->name;
	//	switch_data->gpio = pdata->gpio;
	switch_data->name_on = pdata->name_on;
	switch_data->name_off = pdata->name_off;
	switch_data->state_on = pdata->state_on;
	switch_data->state_off = pdata->state_off;
	switch_data->sdev.print_state = switch_wifi_print_state;

	//	printk("switch_gpio is %d\n", switch_data->gpio);

	INIT_WORK(&switch_wifi_work, gpio_switch_work);

	//switch_data->irq = imapx_gpio_to_irq(switch_data->gpio_switch);
	//ret = request_irq(switch_data->irq, gpio_irq_handler,
	//  IRQF_DISABLED, pdev->name, switch_data);

	//if (ret < 0)
	// return ret;

	ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0)
		return ret;

	//imapx_gpio_setcfg(switch_data->gpio_switch, IG_INPUT, IG_NORMAL);
	/* Perform initial detection */
	schedule_work(&switch_wifi_work);
	//imapx_gpio_setirq(switch_data->gpio_switch, FILTER_MAX, IG_BOTH, 1);

	//	printk("enter function %s, at line %d \n", __func__, __LINE__);
#endif
	return 0;
}

static int __devexit wifi_switch_remove(struct platform_device *pdev)
{
	//	struct wifi_switch_data *switch_data = platform_get_drvdata(pdev);

	//cancel_work_sync(&switch_wifi_work);
	//switch_dev_unregister(&switch_data->sdev);
	kfree(switch_data);
	return 0;
}

static int wifi_switch_suspend(struct platform_device *pdev, pm_message_t state)
{
//	struct wifi_switch_data *switch_data = platform_get_drvdata(pdev);

	//schedule_work(&switch_wifi_work);
	return 0;
}

static int wifi_switch_resume(struct platform_device *pdev)
{
	//struct wifi_switch_data *switch_data = platform_get_drvdata(pdev);

//	schedule_work(&switch_wifi_work);
	return 0;
}

static struct platform_driver wifi_switch_driver = {
	.probe		= wifi_switch_probe,
	.remove		= __devexit_p(wifi_switch_remove),
	.suspend	= wifi_switch_suspend,
	.resume		= wifi_switch_resume,
	.driver		= {
		.name	= "switch-wifi",
		.owner	= THIS_MODULE,
	},
};

static int __init wifi_switch_init(void)
{
	printk("wifi_switch module init\n");
	return platform_driver_register(&wifi_switch_driver);
}

static void __exit wifi_switch_exit(void)
{
	platform_driver_unregister(&wifi_switch_driver);
}

module_init(wifi_switch_init);
//late_initcall(wifi_switch_init);
module_exit(wifi_switch_exit);

MODULE_AUTHOR("Bob.yang <Bob.yang@infotmic.com.cn>");
/* modified by warits on apr.28 to fit new gpio structure */
MODULE_DESCRIPTION("WIFI Switch driver");
MODULE_LICENSE("GPL");
