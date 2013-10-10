/********************************************************************************
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
**     James Xu   <James Xu@infotmic.com.cn>
**
** Revision History:
**     1.0  09/15/2009    James Xu
**     1.0  04/26/2011    Warits Wang		(modify code to fit new gpio structure)
********************************************************************************/
#include <linux/string.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h> 
#include <linux/gpio.h> 
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <mach/pad.h>

#include <mach/hardware.h>
#include <asm/io.h>
/********************************************************/
#include "../codecs/alc5631.h"
#include "imapx800-dma.h"
#include <mach/items.h>

#ifdef CONFIG_SND_DEBUG
#define imapx800dbg(x...) printk(x)
#else
#define imapx800dbg(x...)
#endif

static struct platform_device *imapx800_snd_device;
extern struct snd_soc_dai_link imapx800_snd_dai[3];
extern struct snd_soc_dai_link imapx800_es8328_dai[1];
extern struct snd_soc_dai_link imapx800_es8323_dai[1];
extern struct snd_soc_dai_link imapx800_rt5631_dai[1];
extern struct snd_soc_dai_link imapx800_spdif_dai[1];

static struct imapx800_item_params items[] = {
    [0] = {
        .name       = "es8328",
        .i2c_addr   = 0x10,
        .dai_link   = imapx800_es8328_dai,  
        .num_links  = ARRAY_SIZE(imapx800_es8328_dai),
    },
    [1] = {
        .name       = "rt5631",
        .i2c_addr   = 0x1a,
        .dai_link   = imapx800_rt5631_dai,  
        .num_links  = ARRAY_SIZE(imapx800_rt5631_dai),   
    },
    [2] = {
        .name       = "virtual",
        .dai_link   = imapx800_snd_dai,
        .num_links  = ARRAY_SIZE(imapx800_snd_dai),
    },
    [3] = {
        .name       = "spdif",
        .dai_link   = imapx800_spdif_dai,
        .num_links  = ARRAY_SIZE(imapx800_spdif_dai),
    },
    [4] = {
        .name       = "es8323",
        .i2c_addr   = 0x10,
        .dai_link   = imapx800_es8323_dai,  
        .num_links  = ARRAY_SIZE(imapx800_es8323_dai),
    },
};

static struct snd_soc_card imapx800 = {
    .name = "imapx800",
};

static int imapx800_cpu_dai_name (void)
{
    struct snd_soc_dai_link *imapx800_dai = 
        imapx800.dai_link;
    char *cpu_name = kzalloc(ITEM_MAX_LEN, GFP_KERNEL);
    int num;
    
    num = item_integer("codec.data", 1);
    if (item_equal("codec.data", "i2s", 0)) {
        if (!num) {
            imapx800_dai[0].cpu_dai_name = "imapx800-iis0";
        } else {
            printk("i2s: only support i2s0, err %d\n", num);
            return -1;
        }
    } else if (item_equal("codec.data", "pcm", 0)) {
        if (num > 2 || num < 0) {
            printk("pcm: only support pcm0 or pcm1, err:%d", num);
            return -1;
        }
        sprintf(cpu_name, "imapx800-pcm.%d", num);
        imapx800_dai[0].cpu_dai_name = cpu_name;
    }
    return 0;
}

static int imapx800_init_card (void) 
{
    char *codec_name = kzalloc(ITEM_MAX_LEN, GFP_KERNEL);
    int i = 0;

    item_string(codec_name, "codec.model", 0);
    for (i = 0;i < ARRAY_SIZE(items);i++)
    {   
        if (!strcmp(codec_name, items[i].name)) {
            imapx800.dai_link = (struct snd_soc_dai_link *)(items[i].dai_link);
            imapx800.num_links = items[i].num_links; 
            if (item_equal("codec.ctrl", "i2c", 0)) {
                sprintf(codec_name + strlen(codec_name), ".%d-00%x", 
                        item_integer("codec.ctrl", 1), items[i].i2c_addr);
                (imapx800.dai_link)[0].codec_name = codec_name;
            }
            if (imapx800_cpu_dai_name() < 0)
                return -1;

            return 0;
        } 
        if (i == (ARRAY_SIZE(items) - 1)) {
            printk("Not support this codec: %s\n", codec_name);
            return -ENOMEM;
        }
    }

    return 0;
}

static int nosnd = 0;
void snd_disable(void) {
	nosnd = 1;
}
EXPORT_SYMBOL(snd_disable);

int snd_disabled(void) {
	return nosnd;
}
EXPORT_SYMBOL(snd_disabled);

static int __init imapx800_init(void)
{
	int ret;

	if(nosnd)
		return -ENOMEM;

    if (!item_exist("codec.model")) {
        printk("No codec machine\n");
        return -ENOMEM;
    }
    if (imapx800_init_card())
        return -1;

	imapx800_snd_device = platform_device_alloc("soc-audio", -1);
	if (!imapx800_snd_device)
		return -ENOMEM;
    
    printk("codec name %s, cpu dai name is %s\n", imapx800.dai_link->codec_name,
                    imapx800.dai_link->cpu_dai_name);
	platform_set_drvdata(imapx800_snd_device, &imapx800);
	ret = platform_device_add(imapx800_snd_device);
	if (ret)
		platform_device_put(imapx800_snd_device);
    
    return ret;
}

static void __exit imapx800_exit(void)
{
    kfree(imapx800.dai_link->cpu_dai_name);
    kfree(imapx800.dai_link->codec_name);
	platform_device_unregister(imapx800_snd_device);
}

fs_initcall(imapx800_init);
//late_initcall(imapx800_init);
module_exit(imapx800_exit);

/* Module information */
MODULE_AUTHOR("James xu, James Xu@infotmic.com.cn");
MODULE_DESCRIPTION("ALSA SoC IMAPX800");
MODULE_LICENSE("GPL");
