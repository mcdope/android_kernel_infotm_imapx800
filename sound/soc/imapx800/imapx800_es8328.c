/********************************************************************************
** linux-2.6.28.5/sound/soc/imapx800/imapx800_es8328.c
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
**     James Xu   <James Xu@infotmic.com.cn>
**
** Revision History:
**     1.0  09/15/2009    James Xu
**     1.0  04/26/2011    Warits Wang		(modify code to fit new gpio structure)
********************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h> 
#include <linux/gpio.h> 
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <mach/items.h>
#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <mach/pad.h>

//#include <mach/gpio.h>
#include <mach/hardware.h>
#include <asm/io.h>
//#include <mach/imapx_base_reg.h>
//#include <mach/imapx_gpio.h>
//#include <plat/gpio-bank-c.h>
//#include <plat/gpio-bank-d.h>
//#include <mach/imapx_sysmgr.h>
//#include <mach/spi-gpio.h>
/*******************************************************/
#ifdef CONFIG_CPU_S3C6400
#include <asm/arch/regs-s3c6400-clock.h>
#elif defined CONFIG_CPU_S3C6410
#include <plat/regs-clock.h>
#else

#endif
/********************************************************/
#include "../codecs/es8328.h"
#include "imapx800-dma.h"
//#include "s3c64xx-i2s.h"

/* define the scenarios */
#define imapx800_AUDIO_OFF		1
#define imapx800_CAPTURE_MIC1		3
#define imapx800_STEREO_TO_HEADPHONES	0
#define imapx800_CAPTURE_LINE_IN	2
//#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define imapx800dbg(x...) printk(x)
#else
#define imapx800dbg(x...)
#endif

static int imapx800_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	unsigned int pll_out = 0, bclk = 0;
	int ret = 0;
	unsigned int iisdiv = 0;

	imapx800dbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));
	imapx800dbg("Entered %s, channel = %d\n", __FUNCTION__, params_channels(params));
	imapx800dbg("CPU_DAI name is %s\n",cpu_dai->name);
	/********************************************************************/
	/*PCLK & SCLK gating enable*/
	//writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_IIS0, S3C_PCLK_GATE);
	//writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);
#if 0
    pclkmask = readl(rPCLK_MASK);
	pclkmask &= ~IMAP_CLKCON_PCLK_IIS;
	writel(pclkmask, rPCLK_MASK);
#endif
    /* TODO set pclk */
//	writel(readl(rPCLK_MASK)& ~IMAP_CLKCON_PCLK_IIS, rPCLK_MASK);
//	writel(readl(rSCLK_MASK)& ~IMAP_CLKCON_SCLK_IIS, rSCLK_MASK);
	/********************************************************************/
	switch (params_rate(params)) {
	case 8000:
	//	iisdiv = ES8328_1536FS;	
		iisdiv = ES8328_256FS;
		pll_out = 12288000;
		break;
	case 11025:
		iisdiv = ES8328_1536FS;	
		pll_out = 16934400;
		break;
	case 16000:
		iisdiv = ES8328_768FS;	
		pll_out = 12288000;
		break;
	case 22050:
		//iisdiv = ES8328_768FS;	
		iisdiv = ES8328_512FS;	
		//pll_out = 16934400;
		pll_out = 11289600;
		break;
	case 32000:
		iisdiv = ES8328_384FS;
		pll_out = 12288000;
		break;
	case 44100:
		//iisdiv = ES8328_384FS;
		iisdiv = ES8328_256FS;
		pll_out = 16934400;
		//pll_out = 11289600;
		break;
	case 48000:
		iisdiv = ES8328_384FS;
		pll_out = 18432000;
		break;
	case 96000:
		iisdiv = ES8328_128FS;
		pll_out = 12288000;
		break;
	}
	if (iisdiv > ES8328_256FS)
		iisdiv = ES8328_256FS;
//	iiscdclk_div = (imapx800_IIS_CLK - pll_out)/pll_out;
//	iisclk_dev = (imapx800_IIS_CLK - );
	/* set codec DAI configuration */
	ret = codec_dai->driver->ops->set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
	    SND_SOC_DAIFMT_CBM_CFM); 
	if (ret < 0)
		{ printk("aaa\n");
		return ret;
		}
	/* set cpu DAI configuration */
	/*******************************************************************/
	//in s3c6410, this function does nothing.
	ret = cpu_dai->driver->ops->set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
		SND_SOC_DAIFMT_CBM_CFM ); 
	if (ret < 0)
		{ printk("aaa1\n");
		return ret;
		}

	/*******************************************************************/
	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->driver->ops->set_sysclk(codec_dai, ES8328_MCLK, pll_out,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		{ printk("aaa2 pll_out %d \n",pll_out);
		return ret;
		}

	/* set MCLK division for sample rate */
    /**********************************************************************/
    if (item_equal("codec.data", "i2s", 0)) {
        ret = cpu_dai->driver->ops->set_sysclk(cpu_dai, iisdiv,
                params_rate(params), params_format(params) );
    } else if (item_equal("codec.data", "pcm", 0)) {
        ret = cpu_dai->driver->ops->set_sysclk(cpu_dai, params_channels(params),
                params_rate(params), params_format(params) );
    }
    if (ret < 0)
    { printk("aaa3\n");
        return ret;
    }

	/***********************************************************************/
	/* set codec BCLK division for sample rate */
	ret = codec_dai->driver->ops->set_clkdiv(codec_dai, ES8328_BCLKDIV, bclk);
	if (ret < 0)
		{ printk("aaa4\n");
		return ret;
		}

	/* set prescaler division for sample rate */
	/***********************************************************************/
#if 1
	ret = cpu_dai->driver->ops->set_clkdiv(cpu_dai, 0,0);
	if (ret < 0)
		{ printk("aaa5\n");
		return ret;
		}

#endif
	return 0;
	/************************************************************************/
}

static int imapx800_hifi_hw_free(struct snd_pcm_substream *substream)
{
	return 0;
}

/*
 * Neo1973 ES8328 HiFi DAI opserations.
 */
static struct snd_soc_ops imapx800_hifi_ops = {
	.hw_params = imapx800_hifi_hw_params,
	.hw_free = imapx800_hifi_hw_free,
};
#if 0
static int imapx800_scenario = 0;

static int imapx800_get_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = imapx800_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	switch(imapx800_scenario) {
	imapx800dbg("imapx800_scenario is %d\n",imapx800_scenario);
	case imapx800_AUDIO_OFF:
		printk(KERN_INFO "----------imapx800_AUDIO_OFF!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic Bias");
		snd_soc_dapm_disable_pin(codec, "Line In Jack");
		break;
	case imapx800_STEREO_TO_HEADPHONES:
		printk(KERN_INFO "-----------imapx800_STEREO_TO_HEADPHONE!\n");
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
#if 0
	case imapx800_CAPTURE_MIC1:
		printk(KERN_INFO "-----------imapx800_CAPTURE_MIC1!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_disable_pin(codec, "Line In Jack");
		break;
	case imapx800_CAPTURE_LINE_IN:
		printk(KERN_INFO "------------imapx800_CAPTURE_LINE_IN!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
#endif
	default:
		printk(KERN_INFO "-------------default!\n");
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
	}

	snd_soc_dapm_sync(codec);

	return 0;
}

static int imapx800_set_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (imapx800_scenario == ucontrol->value.integer.value[0])
		return 0;

	imapx800_scenario = ucontrol->value.integer.value[0];
	set_scenario_endpoints(codec, imapx800_scenario);
	return 1;
}
#endif
static const struct snd_soc_dapm_widget es8328_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic Bias", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};


/* example machine audio_mapnections */
static const struct snd_soc_dapm_route audio_map[] = {

	{"Headphone Jack", NULL, "LOUT2"},
	{"Headphone Jack", NULL, "ROUT2"}, 

//	mic is connected to line2  
	{ "LINPUT2", NULL, "Mic Bias" }, 
	{ "Mic Bias", NULL, "Mic Jack" },

	{"LINPUT1", NULL, "Line In Jack"},
	{"RINPUT1", NULL, "Line In Jack"},

#if 0
	/* Connect the ALC pins */
	{"ACIN", NULL, "ACOP"},
#endif
		
	{NULL, NULL, NULL},
};

static const char *smdk_scenarios[] = {
	"Speaker",
	"Mute",
#if 0
	"Capture Line In",
	"Headphones",
	"Capture Mic1",
#endif
};

static const struct soc_enum smdk_scenario_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(smdk_scenarios),smdk_scenarios),
};

static const struct snd_kcontrol_new es8328_imapx800_controls[] = {
#if 0
	SOC_ENUM_EXT("SMDK Mode", smdk_scenario_enum[0],
		imapx800_get_scenario, imapx800_set_scenario),
#endif
};

/*
 * This is an example machine initialisation for a es8328 connected to a
 * imapx800. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int imapx800_es8328_init(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;

	/* set endpoints to default mode */
//	set_scenario_endpoints(codec, imapx800_AUDIO_OFF);

	/* Add imapx800 specific widgets */
    snd_soc_dapm_new_controls(dapm, es8328_dapm_widgets,
            ARRAY_SIZE(es8328_dapm_widgets));

	/* add imapx800 specific controls */
	snd_soc_add_controls(codec, es8328_imapx800_controls,
				ARRAY_SIZE(es8328_imapx800_controls));

	/* set up imapx800 specific audio path audio_mapnects */
#if 0
	for (i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}
#endif

	snd_soc_dapm_add_routes(dapm, audio_map, ARRAY_SIZE(audio_map));
	/* always connected */
	snd_soc_dapm_enable_pin(dapm, "Mic Bias");
	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_enable_pin(dapm, "Line In Jack");
	snd_soc_dapm_sync(dapm);
	return 0;
}

struct snd_soc_dai_link imapx800_es8328_dai[1] = {
    { /* Hifi Playback - for similatious use with voice below */
        .name = "IMPX800 ES8328",
        .stream_name = "ES8328 HiFi",
        .codec_dai_name = "es8328_dai",
        .platform_name = "imapx800-audio",
        .init = imapx800_es8328_init,
        .ops = &imapx800_hifi_ops,
    },
};
EXPORT_SYMBOL(imapx800_es8328_dai);
