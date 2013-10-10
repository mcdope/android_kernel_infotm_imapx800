#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <mach/items.h>
#include "../imapx800/imapx800-dma.h"

extern struct i2c_driver es8323_i2c_driver;
extern struct i2c_driver es8328_i2c_driver;
extern struct i2c_driver rt5631_i2c_driver;
extern struct platform_driver imapnull_driver;

extern int rt5631_switch_spk(int dir);
extern int imapx_gpio_spken(int en);

/*add hdmi control*/
static int closeSpk = 0;

static struct imapx800_item_params items[] = {
    [0] = {
        .name       = "es8328",
        .i2c_addr   = 0x10,
        .dai_link   = &es8328_i2c_driver,
        .imap_switch = imapx_gpio_spken,
    },
    [1] = {
        .name       = "rt5631",
        .i2c_addr   = 0x1a,
        .dai_link   = &rt5631_i2c_driver,
        .imap_switch = rt5631_switch_spk,
    },
    [2] = {
        .name       = "es8323",
        .i2c_addr   = 0x10,
        .dai_link   = &es8323_i2c_driver,
        .imap_switch = imapx_gpio_spken,
    },
    [3] = {
        .name       = "virtual",
        .dai_link   = &imapnull_driver,
        .imap_switch = imapx_gpio_spken,
    },
    [4] = {
        .name       = "spdif",
        .dai_link   = &imapnull_driver,
    }
};

static int i = 0;

int imapx800_switch_spk(int dir)
{
    if (i == (ARRAY_SIZE(items) - 1))
        return -1;
    if (items[i].imap_switch)
        items[i].imap_switch(dir);
    else
        return -1;
    return 0;
}  
EXPORT_SYMBOL(imapx800_switch_spk);

int imapx800_switch_spk_hdmi(int dir)
{
    if (i == (ARRAY_SIZE(items) - 1))
        return -1;
    if(dir)
        closeSpk = 0;
    else
        closeSpk = 1;
    if (items[i].imap_switch)
        items[i].imap_switch(dir);
    else
        return -1;
    return 0;
}  
EXPORT_SYMBOL(imapx800_switch_spk_hdmi);

int imapx800_force_closeSpk(void)
{
    return closeSpk;
}  
EXPORT_SYMBOL(imapx800_force_closeSpk);

extern int snd_disabled(void);
static int __init codec_item_modinit(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    char codec_name[ITEM_MAX_LEN];
    int ret;

    if(snd_disabled())
	    return -ENOMEM;

    if (!item_exist("codec.model")) {
        printk("No codec machine\n");
        return -ENOMEM;
    }
    item_string(codec_name, "codec.model", 0);
    for (i = 0;i < ARRAY_SIZE(items);i++) {
        if (!strcmp(codec_name, items[i].name)) {
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
            if (item_equal("codec.ctrl", "i2c", 0)) {
                memset(&info, 0, sizeof(struct i2c_board_info));
                info.addr = items[i].i2c_addr;
                strlcpy(info.type, items[i].name, I2C_NAME_SIZE);

                adapter = i2c_get_adapter(item_integer("codec.ctrl", 1));
                if (!adapter) {
                    printk("*******get_adapter error!\n");
                    return -1;
                }
                client = i2c_new_device(adapter, &info);
                if (!client) {
                    printk("can't add i2c device at 0x%x\n",
                            (unsigned int)info.addr);
                    return -1;
                }
                ret = i2c_add_driver((struct i2c_driver *)
                        (items[i].dai_link));
                if (ret != 0)
                    printk("ES8987: Unable to register I2C driver: %d\n", ret);
                return ret;
            }
#endif
            return platform_driver_register((struct platform_driver *)
                            (items[i].dai_link));
        }
        if (i == (ARRAY_SIZE(items) - 1)) {
            printk("Not support this codec: %s\n", codec_name);
            return -ENOMEM;
        }
    }
    return 0;
}
module_init(codec_item_modinit);

static void __exit codec_item_exit(void)
{
    if (item_equal("codec.model", "virtual", 0)) {
        platform_driver_unregister(&imapnull_driver);
        return;
    }
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
    i2c_del_driver((struct i2c_driver *)
            (items[i].dai_link));
#endif
}
module_exit(codec_item_exit);

MODULE_DESCRIPTION("ASoC es8328 driver");
MODULE_AUTHOR("Sun");
MODULE_LICENSE("GPL");
