/********************************************************************************
** linux-2.6.28.5/sound/soc/imapx800/imapx800-ac97.c
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
********************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <mach/imap-iomap.h>
#include <mach/imap-iis.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include "imapx800-dma.h"
#include <mach/items.h>

//for hdmi
#ifdef CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT
#include <InfotmMedia.h>
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <ids_pwl.h>
#include <hdmi_api.h>
#endif
#if 0
#define imapx800(x...) printk(x)
#else
#define imapx800(x...)
#endif
/**********************************************************/
static u32 rIER_pm;
static u32 rIRER_pm;
static u32 rITER_pm;
static u32 rCER_pm;
static u32 rCCR_pm;
//static u32 rCDR_pm;
static u32 rMCR_pm;
static u32 rBCR_pm;
static u32 rTER_pm;
static u32 rRER_pm;
static u32 rTER_pm;
static u32 rRCR_pm;
static u32 rTCR_pm;
static u32 rIMR_pm;
static u32 rRFCR_pm; 
static u32 rTFCR_pm;

extern int imapx800_force_closeSpk(void);

/**********************************************************/
static struct imapx800_dma_client imapx800_pch = {
    .name = "I2S PCM Stereo out"
};

static struct imapx800_dma_client imapx800_pch_in = {
    .name = "I2S PCM Stereo in"
};

static struct imapx800_pcm_dma_params imapx800_i2s_pcm_stereo_out = {
	.client		= &imapx800_pch,
	/****************************************************************/
    .channel    = IMAPX800_I2S_MASTER_TX,
	.dma_addr	= IMAP_IIS0_BASE + rTXDMA,
	.dma_size	= 2,
//	.brst_len	= 4,
	/****************************************************************/
};

static struct imapx800_pcm_dma_params imapx800_i2s_pcm_stereo_in = {
	.client		= &imapx800_pch_in,
	/***************************************************************/
	.channel    = IMAPX800_I2S_MASTER_RX,
	.dma_addr	= IMAP_IIS0_BASE + rRXDMA,
	.dma_size	= 2,
//	.brst_len   = 4,
	/**************************************************************/
};

struct imapx800_i2s_info {
	void __iomem	*regs;
    void __iomem    *mclk;
    void __iomem    *bclk;
	void __iomem	*interface;
	struct clk	*iis_clk;
	unsigned int    iis_clock_rate;
	int master;
};

static struct imapx800_i2s_info imapx800_i2s;
/**********************************************************
 * in this funciton, if in on condition, the program active the iis interface, start the operation.
 * If in off condition, the program disable the iis interface.
*/
void imapx800_snd_txctrl(int on)
{
	int rITER_value, rTER0_value, rCER_value, rIER_value, rTFF0_value;  
	imapx800("Entered %s : on = %d \n", __FUNCTION__, on);

	if (on) {
		rITER_value = 0x1;         	//iis transmitter block enable register
		rTER0_value = 0x1;		//iis transmitter enable register0	
		rCER_value = 0x1;		//clock enable register
		rIER_value = 0x1;		//iis enable register
		writel(rITER_value, imapx800_i2s.regs +  rITER);	
		writel(rTER0_value, imapx800_i2s.regs + rTER0);	
		writel(rCER_value, imapx800_i2s.regs + rCER);
		writel(rIER_value, imapx800_i2s.regs + rIER);
	} else {
		rTFF0_value = 0x1; //flush the tx fifo
		rITER_value = 0x0;         	//iis transmitter block enable register
		rTER0_value = 0x0;		//iis transmitter enable register0	
		writel(rTFF0_value, imapx800_i2s.regs + rTFF0);	
		writel(rITER_value, imapx800_i2s.regs + rITER);	
		writel(rTER0_value, imapx800_i2s.regs + rTER0);	
	}

}
EXPORT_SYMBOL(imapx800_snd_txctrl);
/**********************************************************
 * in this funciton, if in on condition, the program active the iis interface, start the operation.
 * If in off condition, the program disable the iis interface.
 */
void imapx800_snd_rxctrl(int on)
{
	int rIRER_value, rRER0_value, rCER_value, rIER_value, rRFF0_value;  
	imapx800("Entered %s : on = %d \n", __FUNCTION__, on);

	if (on) {
		rIRER_value = 0x1;         	//iis transmitter block enable register
		rRER0_value = 0x1;		//iis transmitter enable register0	
		rCER_value = 0x1;		//clock enable register
		rIER_value = 0x1;		//iis enable register
		writel(rIRER_value, imapx800_i2s.regs + rIRER);	
		writel(rRER0_value, imapx800_i2s.regs + rRER0);	
		writel(rCER_value, imapx800_i2s.regs +rCER);
		writel(rIER_value, imapx800_i2s.regs +rIER);
	} else {
		rRFF0_value = 0x1; //flush the tx fifo
		rIRER_value = 0x0;         	//iis transmitter block enable register
		rRER0_value = 0x0;		//iis transmitter enable register0	
		writel(rRFF0_value, imapx800_i2s.regs + rRFF0);	
		writel(rIRER_value, imapx800_i2s.regs + rIRER);	
		writel(rRER0_value, imapx800_i2s.regs + rRER0);	
	}

}
EXPORT_SYMBOL(imapx800_snd_rxctrl);

static int imapx800_i2s_set_fmt(struct snd_soc_dai *dai,
		unsigned int fmt)
{
	return 0;
}

#ifdef CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT
static int imapx800_i2s_to_hdmi(struct snd_pcm_hw_params *params)
{
    hdmi_audio_t audio_cfg;
	memset((void *)&audio_cfg, 0, sizeof(hdmi_audio_t));

    audio_cfg.aInterfaceType = IM_I2S;
    audio_cfg.aCodingType = IM_PCM;
    audio_cfg.channelAllocation = 0;
    audio_cfg.sampleSize = params_format(params);
    audio_cfg.samplingFreq = params_rate(params);

    hdmi_set_audio_basic_config(&audio_cfg);
}
#endif

static int imapx800_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *socdai)
{
	//int rITER_value, rTER0_value, rIRER_value, rRER0_value, rIER_value;  
	int rIMR0_value;
	unsigned int rCCR_value, rTCR0_value, rRCR0_value, rTFCR0_value, rRFCR0_value;
	struct imapx800_pcm_dma_params *dma_data;
	imapx800("Entered %s\n", __FUNCTION__);

	

	imapx800("substream->stream : %d, format is %d\n", 
            substream->stream, params_format(params));
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		dma_data = &imapx800_i2s_pcm_stereo_out;
	/* TODO dma */
    } else {
		dma_data = &imapx800_i2s_pcm_stereo_in;
    /* TODO dma */
	}

	snd_soc_dai_set_dma_data(socdai, substream, dma_data);
#ifdef CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT
    //for hdmi interface
    imapx800_i2s_to_hdmi(params);
#endif

	rIMR0_value = (0x1<<5) | (0x1<<1);
	writel(rIMR0_value, imapx800_i2s.regs + rIMR0);
	/*set fifo*/	
	rTFCR0_value = readl(imapx800_i2s.regs + rTFCR0);
	//rTFCR0_value = 0x8;
	rTFCR0_value = 0xc;
	writel(rTFCR0_value, imapx800_i2s.regs + rTFCR0);
	rRFCR0_value = readl(imapx800_i2s.regs + rRFCR0);
	rRFCR0_value = 0x8;
	writel(rRFCR0_value, imapx800_i2s.regs + rRFCR0);
	/*set bit mode and gata*/	
	rCCR_value = readl(imapx800_i2s.regs + rCCR);
	rTCR0_value = readl(imapx800_i2s.regs + rTCR0);
	rRCR0_value = readl(imapx800_i2s.regs + rRCR0);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		rCCR_value = 0x3 <<3;
		rTCR0_value = 0x0;
		rRCR0_value = 0x0;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		rCCR_value = 0x2 <<3;
		rTCR0_value = 0x02;
		rRCR0_value = 0x02;
		break;

	case SNDRV_PCM_FORMAT_S24_LE:
		rCCR_value = 0x2 <<3;
		rTCR0_value = 0x04;
		rRCR0_value = 0x04;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		rCCR_value = 0x2 <<3;
		rTCR0_value = 0x05;
		rRCR0_value = 0x05;
		break;
	default:
		return -EINVAL;
	}
	writel(rCCR_value, imapx800_i2s.regs + rCCR);
	writel(rTCR0_value, imapx800_i2s.regs + rTCR0);
	writel(rRCR0_value, imapx800_i2s.regs + rRCR0);

	/*by sun for disable iis 1,2,3*/
	//printk("shut down i2s 1,2,3\n");
	writel(0, imapx800_i2s.regs + rRER1);
	writel(0, imapx800_i2s.regs + rTER1);
	writel(0, imapx800_i2s.regs + rRER2);
	writel(0, imapx800_i2s.regs + rTER2);
	writel(0, imapx800_i2s.regs + rRER3);
	writel(0, imapx800_i2s.regs + rTER3);
#if 0
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rITER_value = 0x1;         	//iis transmitter block enable register
		rTER0_value = 0x1;		//iis transmitter enable register0	
		rIER_value = 0x1;		//iis enable register
	       	writel(rITER_value, imapx800_i2s.regs +  rITER);	
	       	writel(rTER0_value, imapx800_i2s.regs + rTER0);	
		writel(rIER_value, imapx800_i2s.regs + rIER);
	}else {
		rIRER_value = 0x1;         	//iis transmitter block enable register
		rRER0_value = 0x1;		//iis transmitter enable register0	
		rIER_value = 0x1;		//iis enable register
	       	writel(rIRER_value, imapx800_i2s.regs + rIRER);	
	       	writel(rRER0_value, imapx800_i2s.regs + rRER0);	
		writel(rIER_value, imapx800_i2s.regs +rIER);
	}
#endif
	return 0;

}
extern int imapx_gpio_spken(int en) ;

static int AudioRunStatus=0;
int imapx_set_RunStatus(void)
{
	printk(KERN_DEBUG"audios Run status %d\n",AudioRunStatus);
	return AudioRunStatus;	
}
EXPORT_SYMBOL(imapx_set_RunStatus);

int imapx_set_AudioRunStatus1(void)
{
    AudioRunStatus=1;
}
EXPORT_SYMBOL(imapx_set_AudioRunStatus1);

static int imapx800_i2s_trigger(struct snd_pcm_substream *substream, 
			int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;

	imapx800("Entered %s: cmd = %x\n", __FUNCTION__, cmd);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			imapx800_snd_rxctrl(1);
		else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			{
				imapx800_snd_txctrl(1);
				AudioRunStatus=1;
                if (!imapx800_force_closeSpk())
                    imapx_gpio_spken(1);
				printk(KERN_DEBUG"AAAA--en spk\n");
			}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			imapx800_snd_rxctrl(0);
		else if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) 
			{
				imapx800_snd_txctrl(0);
				AudioRunStatus=0;
                if(!imapx800_force_closeSpk())
                imapx_gpio_spken(0);
				printk(KERN_DEBUG"AAAB--dis spk\n");
			}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static void imapx800_i2s_shutdown(struct snd_pcm_substream *substream, 
			struct snd_soc_dai *dai)
{
	imapx800("Entered %s\n", __FUNCTION__);
}

/*
 * Set S3C24xx Clock source
 */
static int imapx800_i2s_set_sysclk(struct snd_soc_dai *dai,
	int clk_id, unsigned int freq, int format)
{

	int mclkdiv, bitclkdiv, format_value = 0;
	int bit_clk, codec_clk;
	imapx800("Entered %s : clk_id = %d, freq is %d, format is %d\n", __FUNCTION__, clk_id, freq, format);
	/***********************************************************/
	switch(format){
		case 0:
		case 1:
			format_value = 8;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			format_value = 16;
			break;
		case 6:
		case 7:
		case 8:
		case 9:
			format_value = 24;
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			format_value = 32;
			break;


	}

	codec_clk = clk_id * freq;
	while(imapx800_i2s.iis_clock_rate < codec_clk) {
		printk("i2s div error\n");
		freq = freq / 2;
		codec_clk = clk_id * freq;
	}
    bit_clk = freq * format_value * 2;
    codec_clk = clk_id * freq;
    mclkdiv = imapx800_i2s.iis_clock_rate / codec_clk - 1;
   // bitclkdiv = (mclkdiv + 1) * (clk_id / (format_value * 2)) - 1;
	bitclkdiv = (mclkdiv + 1) * (clk_id / (2 * format_value * 2)) - 1;
    
    imapx800("mclkdiv = %d, bitclkdiv = %d\n", mclkdiv, bitclkdiv);
    
    writel(mclkdiv, imapx800_i2s.mclk);
    writel(bitclkdiv, imapx800_i2s.bclk);
	writel(0x6, imapx800_i2s.interface);
	
    return 0;
}

/*
 * Set S3C24xx Clock dividers
 */
static int imapx800_i2s_set_clkdiv(struct snd_soc_dai *dai,
	int div_id, int div)
{
	imapx800("Entered %s : div_id = %d, div = %d\n", __FUNCTION__, div_id, div);

	return 0;
}

extern int audio_auto_playback_init(void);
extern int audio_play (bool auto_playback);

static int imapx800_i2s_probe(struct snd_soc_dai *dai)
{
	imapx800("Entered %s\n", __FUNCTION__);
	
	module_power_on(SYSMGR_IIS_BASE);
    
    imapx_pad_cfg(IMAPX_IIS0,1);
	imapx800("Entered %s\n", __FUNCTION__);
	imapx800_i2s.regs = ioremap(IMAP_IIS0_BASE, SZ_4K);
    imapx800_i2s.mclk = ioremap(rI2S_MCLKDIV, 4);
    imapx800_i2s.bclk = ioremap(rI2S_BCLKDIV, 4);
	imapx800_i2s.interface = ioremap(rI2S_INTERFACE, 4);
	if (imapx800_i2s.regs == NULL || imapx800_i2s.mclk == NULL
            || imapx800_i2s.bclk == NULL || imapx800_i2s.interface == NULL)
	{
		printk(KERN_ERR "regs is null, exit!\n");
		return -ENXIO;
	}
#if 1
	imapx800_i2s.iis_clk = clk_get(dai->dev, "audio-clk");
	if (imapx800_i2s.iis_clk == NULL) {
		printk(KERN_ERR "failed to get iis_clock\n");
		return -ENODEV;
	}
	clk_enable(imapx800_i2s.iis_clk);
	imapx800_i2s.iis_clock_rate = clk_get_rate(imapx800_i2s.iis_clk);	
    /* set mclk and rlclk */
	//imapx800_i2s_set_sysclk(dai,384,44100,13);
	//printk("**********   iis_clock_rate is %d\n", imapx800_i2s.iis_clock_rate);
#endif
        if (!audio_auto_playback_init()) {
            printk("playback now........\n");
            audio_play(false);
        }
	return 0;
}

#ifdef CONFIG_PM
static int imapx800_i2s_suspend(struct snd_soc_dai *dai)
{
	imapx800("Entered %s\n", __FUNCTION__);

    clk_disable(imapx800_i2s.iis_clk);
	rIER_pm = readl(imapx800_i2s.regs + rIER);;
	rIRER_pm = readl(imapx800_i2s.regs + rIRER);
	rITER_pm = readl(imapx800_i2s.regs + rITER);
	rCER_pm = readl(imapx800_i2s.regs + rCER);
	rCCR_pm = readl(imapx800_i2s.regs + rCCR);
	rMCR_pm = readl(imapx800_i2s.mclk);
    rBCR_pm = readl(imapx800_i2s.bclk);
    rRER_pm = readl(imapx800_i2s.regs + rRER0);
	rTER_pm = readl(imapx800_i2s.regs + rTER0);
	rRCR_pm = readl(imapx800_i2s.regs + rRCR0);
	rTCR_pm = readl(imapx800_i2s.regs + rTCR0);
	rIMR_pm = readl(imapx800_i2s.regs + rIMR0);
	rRFCR_pm = readl(imapx800_i2s.regs + rRFCR0); 
	rTFCR_pm = readl(imapx800_i2s.regs + rTFCR0);

	return 0;                          
}

int imapx800_i2s_resume(struct snd_soc_dai *dai)
{
	imapx800("Entered %s\n", __FUNCTION__);


    module_power_on(SYSMGR_IIS_BASE);
    imapx_pad_cfg(IMAPX_IIS0,1);
    
    clk_enable(imapx800_i2s.iis_clk);
	writel(0x0, imapx800_i2s.regs + rIER);;
	writel(0x0, imapx800_i2s.regs + rIRER);
	writel(0x0, imapx800_i2s.regs + rITER);
	writel(0x0, imapx800_i2s.regs + rCER);
	
	writel(0, imapx800_i2s.regs + rRER1);
	writel(0, imapx800_i2s.regs + rTER1);
	writel(0, imapx800_i2s.regs + rRER2);
	writel(0, imapx800_i2s.regs + rTER2);
	writel(0, imapx800_i2s.regs + rRER3);
	writel(0, imapx800_i2s.regs + rTER3);
	writel(rCCR_pm, imapx800_i2s.regs + rCCR);
    writel(rMCR_pm, imapx800_i2s.mclk);
    writel(rBCR_pm, imapx800_i2s.bclk);
	writel(rRER_pm, imapx800_i2s.regs + rRER0);
	writel(rTER_pm, imapx800_i2s.regs + rTER0);
	writel(rRCR_pm, imapx800_i2s.regs + rRCR0);
	writel(rTCR_pm, imapx800_i2s.regs + rTCR0);
	writel(rIMR_pm, imapx800_i2s.regs + rIMR0);
	writel(rRFCR_pm, imapx800_i2s.regs + rRFCR0); 
	writel(rTFCR_pm, imapx800_i2s.regs + rTFCR0);

	writel(rIER_pm, imapx800_i2s.regs + rIER);;
	writel(rIRER_pm, imapx800_i2s.regs + rIRER);
	writel(rITER_pm, imapx800_i2s.regs + rITER);
	writel(rCER_pm, imapx800_i2s.regs + rCER);
	return 0;
}

#else
#define imapx800_i2s_suspend	NULL
#define imapx800_i2s_resume	NULL
#endif

#define IMAPX800_I2S_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)
static struct snd_soc_dai_ops imapx800_i2s_dai_ops = {
		.shutdown = imapx800_i2s_shutdown,
		.trigger = imapx800_i2s_trigger,
		.hw_params = imapx800_i2s_hw_params,
		.set_fmt = imapx800_i2s_set_fmt,
		.set_clkdiv = imapx800_i2s_set_clkdiv,
		.set_sysclk = imapx800_i2s_set_sysclk,
};


static struct snd_soc_dai_driver imapx800_i2s_dai[] = {
	{
	.probe = imapx800_i2s_probe,
	.suspend = imapx800_i2s_suspend,
	.resume = imapx800_i2s_resume,
	.playback = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = IMAPX800_I2S_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = IMAPX800_I2S_RATES,
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE},
	.ops = &imapx800_i2s_dai_ops,
	},
};

static __devinit int imapx800_iis_dev_probe(struct platform_device *pdev)
{
    return snd_soc_register_dai(&pdev->dev, imapx800_i2s_dai);
}

static __devexit int imapx800_iis_dev_remove(struct platform_device *pdev)
{
    snd_soc_unregister_dai(&pdev->dev);
    return 0;
}

static struct platform_driver imapx800_iis_driver = {
    .probe  = imapx800_iis_dev_probe,
    .remove = imapx800_iis_dev_remove,
    .driver = {
        .name = "imapx800-iis0",
        .owner = THIS_MODULE,
    },
};

static int __init imapx800_i2s_init(void)
{
    if ((!item_equal("codec.data", "i2s", 0)) && 
            (!item_equal("codec.model", "virtual", 0)))
        return -1;
    return platform_driver_register(&imapx800_iis_driver);
}
module_init(imapx800_i2s_init);

static void __exit imapx800_i2s_exit(void)
{
    platform_driver_unregister(&imapx800_iis_driver);
}
module_exit(imapx800_i2s_exit);

MODULE_AUTHOR("James xu, James Xu@infotmic.com.cn");
MODULE_DESCRIPTION("imapx800 I2S SoC Interface");
MODULE_LICENSE("GPL");
