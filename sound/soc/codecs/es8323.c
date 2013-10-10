/*
 * es8323.c -- es8323 ALSA SoC audio driver
 *
 * Copyright 2005 Openedhand Ltd.
 *
 * Author: Richard Purdie <richard@openedhand.com>
 *
 * Based on es8323.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <mach/items.h>
#include <linux/mfd/tps65910_imapx800.h>
#ifdef CONFIG_HHBF_FAST_REBOOT
#include <asm/reboot.h>
#endif

#include "es8323.h"

#define AUDIO_NAME "es8323"
#define es8323_VERSION "v0.12"
//#include <mach/imapx_gpio.h>
#include <asm/io.h>

static unsigned int system_mute;
static unsigned int system_mute_state;
static int mute_initial = 0;
/*
 * Debug
 */

//#define es8323_DEBUG 1

#ifdef es8323_DEBUG
#define dbg(format, arg...) \
	dbg(KERN_DEBUG AUDIO_NAME ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif
#define err(format, arg...) \
	dbg(KERN_ERR AUDIO_NAME ": " format "\n" , ## arg)
#define info(format, arg...) \
	dbg(KERN_INFO AUDIO_NAME ": " format "\n" , ## arg)
#define warn(format, arg...) \
	dbg(KERN_WARNING AUDIO_NAME ": " format "\n" , ## arg)

/* codec private data */
struct es8323_priv {
    enum snd_soc_control_type control_type;
	unsigned int sysclk;
    struct regulator *regulator;
};
static unsigned es8323_sysclk;

//void (*hhbf_audio_switch)(int flag) = NULL;
//EXPORT_SYMBOL(hhbf_audio_switch);

//chuanjin add for contrl spk default volume 
#define ITEM_CFG_SIZE  ITEM_MAX_LEN //sync to item
typedef struct _ItemAudioCfg_T
{
	int SpkVolume; 
}ItemAudioCfg_T;
	
static ItemAudioCfg_T AudioItemCfgObj;	

/*
this func only use to get cfg
*/
static void DoItemCfgOri(ItemAudioCfg_T *p)
{
	if(NULL==p)	 
	{
		printk("%s,%s, input NULL pointer!!!\n",__FILE__,__FUNCTION__);
		return;
	}
	
    if(item_exist("codec.volume"))
    {
        p->SpkVolume=item_integer("codec.volume", 0);
		printk("default volume:%d\n",p->SpkVolume);	
    }
    else
	{
        p->SpkVolume = -1;
    }

}

/*
this func only use cfg to set spk volume .
for digit volume have overflow risk,by EQ boost low Freqency.
so:
1, 90-100->DigtVal:0x08-0x02 -4dB~-1dB this range have overflow risk.
2, 90-100 ->AngVal:fix to Max gain 0x21 3dB
3, 0-90 -> DigtVal:0x20-0x08  -16dB~-4dB
4, 0-90 -> AngVal: 0x18 -0x21 -6dB~3dB
sum:
total gain range:-22dB~2dB
Ang gain range:-6dB~3dB
Digt gain range:-16dB~-1dB

*/
static void DoItemHALSetVolume(struct snd_soc_codec *codec ,ItemAudioCfg_T *p)
{
int AngVal,DigtVal;
	if(NULL==p)	 
	{
		printk("%s,%s, input NULL pointer!!!\n",__FILE__,__FUNCTION__);
		return;
	}

	if(p->SpkVolume < 0) return; //use orginal value, no config item
	if(p->SpkVolume >100 ) //security check
	{
		printk("item cfg spk volume is to big(%d),so reset to 100 \n",p->SpkVolume); 
		p->SpkVolume=100;
	}

	if(p->SpkVolume>= 90) //volume map to val
	{
		DigtVal=(620-(6*p->SpkVolume))/10;
		AngVal=0x21;
	}
	else
	{
		DigtVal=(2880-(24*p->SpkVolume))/90;
		AngVal=(240+(p->SpkVolume))/10;
	}


	snd_soc_write(codec, ES8323_DACCONTROL24,(unsigned int) AngVal); //0x2e
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL24));
	snd_soc_write(codec, ES8323_DACCONTROL25, (unsigned int)AngVal); //0x2f
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL25));

	snd_soc_write(codec, ES8323_DACCONTROL4, (unsigned int)DigtVal); //0x1a
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL4));
	snd_soc_write(codec, ES8323_DACCONTROL5, (unsigned int)DigtVal); //0x1b
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL5));


}
//extern void imap_iokey_spken(int);
/*
 * es8323 register cache
 * We can't read the es8323 register space when we
 * are using 2 wire for device control, so we cache them instead.
 */
static const u16 es8323_reg[] = {
	0x00b7, 0x0097, 0x0000, 0x0000,  /*  0 */
	0x0000, 0x0008, 0x0000, 0x002a,  /*  4 */
	0x0000, 0x0000, 0x007F, 0x007F,  /*  8 */
	0x000f, 0x000f, 0x0000, 0x0000,  /* 12 */
	0x0080, 0x007b, 0x0000, 0x0032,  /* 16 */
	0x0000, 0x00E0, 0x00E0, 0x00c0,  /* 20 */
	0x0000, 0x000e, 0x0000, 0x0000,  /* 24 */
	0x0000, 0x0000, 0x0000, 0x0000,  /* 28 */
	0x0000, 0x0000, 0x0050, 0x0050,  /* 32 */
	0x0050, 0x0050, 0x0050, 0x0050,  /* 36 */
	0x0000, 0x0000, 0x0079,          /* 40 */
};

/*
 * read es8323 register cache
 */
static inline unsigned int es8323_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
#ifndef	CONFIG_HHTECH_MINIPMP
	if (reg > es8323_CACHE_REGNUM)
#else// mhfan
	if (reg > ARRAY_SIZE(es8323_reg))
#endif//CONFIG_HHTECH_MINIPMP
		return -1;
	return cache[reg];
}

/*
 * write es8323 register cache
 */
static inline void snd_soc_write_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;
#ifndef	CONFIG_HHTECH_MINIPMP
	if (reg > es8323_CACHE_REGNUM)
#else// mhfan
	if (reg > ARRAY_SIZE(es8323_reg))
#endif//CONFIG_HHTECH_MINIPMP
		return;
	cache[reg] = value;
}

#if 0
/* now, i2c read or write is used by system api */
static int es8323_read(struct snd_soc_codec *codec, unsigned int reg)
{
	u8 data[1];

	data[0] = reg ;
	codec->hw_write(codec->control_data, data, 1);
//	dbg("************data0 is %d\n", data[0]);
//	msleep(5);
	data[0]= 0;
	i2c_master_recv(codec->control_data, data,1);
//	dbg("************data1 is %d\n", data[0]);
	return 0;
}


static int es8323_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	u8 data[2];
	/* data is
	 *   D15..D9 es8323 register offset
	 *   D8...D0 register data
	 */
	data[0] = reg ;
	data[1] = value;
	if (codec->hw_write(codec->control_data, data, 2) == 2)
		return 0;
	else
	{
		dbg("i2c_transfer_error!\n");
		return -EIO;
	}
}
#endif

static const struct snd_kcontrol_new es8323_snd_controls[] = {

};

static const struct snd_soc_dapm_widget es8323_dapm_widgets[] = {

};

static const struct snd_soc_dapm_route audio_map[] = {

	{NULL, NULL, NULL},
};
static int es8323_add_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	snd_soc_dapm_new_controls(dapm, es8323_dapm_widgets, 
				ARRAY_SIZE(es8323_dapm_widgets));
#if 0
	/* set up audio path audio_mapnects */
	for(i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}
#endif

	snd_soc_dapm_add_routes(dapm, audio_map, ARRAY_SIZE(audio_map));
	snd_soc_dapm_new_widgets(dapm);
	return 0;
}

struct _coeff_div {
	u32 mclk;
	u32 rate;
	u16 fs;
	u8 sr:5;
	u8 usb:1;
};

/* codec hifi mclk clock divider coefficients */
static const struct _coeff_div coeff_div[] = {
	/* 8k */
	{12288000, 8000, 1536, 0x6, 0x0},
	{11289600, 8000, 1408, 0x16, 0x0},
	{18432000, 8000, 2304, 0x7, 0x0},
	{16934400, 8000, 2112, 0x17, 0x0},
	{12000000, 8000, 1500, 0x6, 0x1},

	/* 11.025k */
	{11289600, 11025, 1024, 0x18, 0x0},
	{16934400, 11025, 1536, 0x19, 0x0},
	{12000000, 11025, 1088, 0x19, 0x1},

	/* 16k */
	{12288000, 16000, 768, 0xa, 0x0},
	{18432000, 16000, 1152, 0xb, 0x0},
	{12000000, 16000, 750, 0xa, 0x1},

	/* 22.05k */
	{11289600, 22050, 512, 0x1a, 0x0},
	{16934400, 22050, 768, 0x1b, 0x0},
	{12000000, 22050, 544, 0x1b, 0x1},

	/* 32k */
	{12288000, 32000, 384, 0xc, 0x0},
	{18432000, 32000, 576, 0xd, 0x0},
	{12000000, 32000, 375, 0xc, 0x1},	// mhfan

	/* 44.1k */
	{11289600, 44100, 256, 0x10, 0x0},
	{16934400, 44100, 384, 0x11, 0x0},
	{12000000, 44100, 272, 0x11, 0x1},

	/* 48k */
	{12288000, 48000, 256, 0x0, 0x0},
	{18432000, 48000, 384, 0x1, 0x0},
	{12000000, 48000, 250, 0x0, 0x1},

	/* 88.2k */
	{11289600, 88200, 128, 0x1e, 0x0},
	{16934400, 88200, 192, 0x1f, 0x0},
	{12000000, 88200, 136, 0x1f, 0x1},

	/* 96k */
	{12288000, 96000, 128, 0xe, 0x0},
	{18432000, 96000, 192, 0xf, 0x0},
	{12000000, 96000, 125, 0xe, 0x1},
};

static inline int get_coeff(int mclk, int rate)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
		if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
			return i;
	}

	dbg(KERN_ERR "es8323: could not get coeff for mclk %d @ rate %d\n",
		mclk, rate);
	return -EINVAL;
}

#if 1 
static int es8323_set_dai_clkdiv(struct snd_soc_dai *codec_dai,
		int div_id, int div)
{
	return 0;
}

#endif
static int es8323_set_dai_sysclk(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir)
{
#ifndef	CONFIG_HHTECH_MINIPMP
	struct snd_soc_codec *codec = dai->codec;
	struct es8323_priv *es8323 = snd_soc_codec_get_drvdata(codec);
#endif//CONFIG_HHTECH_MINIPMP
//	dbg("freq is %ld \n",freq);

	switch (freq) {
	case 11289600:
	case 12000000:
	case 12288000:
	case 16934400:
	case 18432000:
#ifndef	CONFIG_HHTECH_MINIPMP
		es8323->sysclk = freq;
#else// mhfan
	case 24576000:
		es8323_sysclk = freq;
#endif//CONFIG_HHTECH_MINIPMP
		return 0;
	}
	return -EINVAL;
}

static int es8323_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	u16 iface = 0;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface = 0x0040;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= 0x0002;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= 0x0001;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= 0x0003;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface |= 0x0013;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface |= 0x0090;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= 0x0080;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface |= 0x0010;
		break;
	default:
		return -EINVAL;
	}

#if 1 
#ifdef	CONFIG_HHTECH_MINIPMP
	iface |= 0x20;	// XXX: mhfan
#endif//CONFIG_HHTECH_MINIPMP
#endif //lzcx
	/*
	if(iface != es8323_read_reg_cache(codec, es8323_IFACE))
		snd_soc_write(codec, es8323_IFACE, iface);
		*/
	return 0;
}

static int es8323_pcm_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;

		if (params_rate(params)== 8000)
		{
			//dbg("sound record!\n");
//			snd_soc_write(codec, ES8323_CHIPPOWER,  0x55);//0x0d
		}
		if (params_rate(params)== 44100)
		{
			//dbg("sound track!\n");
		//	snd_soc_write(codec, ES8323_CHIPPOWER,  0xaa);//0x0d
		}

	return 0;
}

static int es8323_mute(struct snd_soc_dai *dai, int mute)
{
	if (mute_initial == 0){
		//imap_iokey_spken(1);
		mute_initial = 1;
	}

	/* comment by mhfan */
	return 0;
}
#if defined (CONFIG_ARCH_IMAPX800)
static int es8323_set_bias_level(struct snd_soc_codec *codec,
		enum snd_soc_bias_level level)
{
	//u16 reg = wm8731_read_reg_cache(codec, WM8731_PWR) & 0xff7f;

	return 0;
}


#else
static int es8323_dapm_event(struct snd_soc_codec *codec, int event)
{
	u16 pwr_reg = es8323_read_reg_cache(codec, es8323_PWR1) & 0xfe3e;
	pwr_reg |= 0x2;//lzcx micbias on
	switch (event) {
	case SNDRV_CTL_POWER_D0: /* full On */
		/* set vmid to 50k and unmute dac */
		snd_soc_write(codec, es8323_PWR1, pwr_reg | 0x00c0);
		break;
	case SNDRV_CTL_POWER_D1: /* partial On */
		snd_soc_write(codec, es8323_PWR1, pwr_reg | 0x01c0); 
		break;
	case SNDRV_CTL_POWER_D2: /* partial On */
		/* set vmid to 5k for quick power up */
		snd_soc_write(codec, es8323_PWR1, pwr_reg | 0x01c1);
		break;
	case SNDRV_CTL_POWER_D3hot: /* Off, with power */
		/* mute dac and set vmid to 500k, enable VREF */
		snd_soc_write(codec, es8323_PWR1, pwr_reg | 0x0141);
		break;
	case SNDRV_CTL_POWER_D3cold: /* Off, without power */
		snd_soc_write(codec, es8323_PWR1, 0x0001);
		break;
	}
//	codec->dapm_state = event;
	return 0;
}
#endif
#define es8323_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define es8323_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
	SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_ops es8323_ops = {
	.hw_params = es8323_pcm_hw_params,
	.set_fmt = es8323_set_dai_fmt,
	.set_sysclk = es8323_set_dai_sysclk,
	.digital_mute = es8323_mute,
	.set_clkdiv = es8323_set_dai_clkdiv,
};


static struct snd_soc_dai_driver es8323_dai = {
	.name = "es8323_dai",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = es8323_RATES,
		.formats = es8323_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = es8323_RATES,
		.formats = es8323_FORMATS,},
	.ops = &es8323_ops,	
};

static int es8323_suspend(struct snd_soc_codec *codec, pm_message_t state)
{
	es8323_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int es8323_codec_init(struct snd_soc_codec *codec)
{
	es8323_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
//	schedule_delayed_work(&codec->delayed_work, msecs_to_jiffies(1000));
#ifdef	CONFIG_HHTECH_MINIPMP
	// init first mutes
	//dbg("*********now, init every register!\n");
	snd_soc_write(codec, ES8323_MASTERMODE, 0x00);	// 0x08
	dbg("ES8323 ES8323_MASTERMODE,%x\n", snd_soc_read(codec, ES8323_MASTERMODE));
	//es8323_read(codec, ES8323_MASTERMODE);
	snd_soc_write(codec, ES8323_CHIPPOWER, 0xf3);  //0x02
	dbg("ES8323 ES8323_CHIPPOWER,%x\n", snd_soc_read(codec, ES8323_CHIPPOWER));
	//es8323_read(codec, ES8323_CONTROL1);
	snd_soc_write(codec,ES8323_CONTROL1 , 0x36);  //0x00
	dbg("ES8323 ES8323_CONTROL1%x\n", snd_soc_read(codec, ES8323_CONTROL1));
	//es8323_read(codec, ES8323_CHIPPOWER);

	snd_soc_write(codec, ES8323_CONTROL2, 0x40);	// 0x01
	 dbg("ES8323 ES8323_CONTROL2,%x\n", snd_soc_read(codec, ES8323_CONTROL2));
	 snd_soc_write(codec, ES8323_DACPOWER, 0x30);
	 dbg("ES8323 ES8323_DACPOWER,%x\n", snd_soc_read(codec, ES8323_DACPOWER));
/*	
snd_soc_write(codec, ES8323_CONTROL2, 0x070);	// 0x01, low power
	snd_soc_write(codec, ES8323_CHIPLOPOW1, 0x0fb);	// 0x05, low power
	snd_soc_write(codec, ES8323_CHIPLOPOW2, 0x0ff);	// 0x06 low power
	snd_soc_write(codec, ES8323_ANAVOLMANAG, 0x07a);	// 0x07 low power
	*/
	snd_soc_write(codec, ES8323_ADCPOWER, 0x0000);	// 0x03
	dbg("ES8323 ES8323_ADCPOWER,%x\n", snd_soc_read(codec, ES8323_ADCPOWER));
	snd_soc_write(codec, ES8323_DACPOWER,  0x3c);	// 0x04
	dbg("ES8323 ES8323_DACPOWER,%x\n", snd_soc_read(codec, ES8323_DACPOWER));
	snd_soc_write(codec, ES8323_ADCCONTROL1,  0x88);	// 0x09
	dbg("ES8323 ES8323_ADCCONTROL1,%x\n", snd_soc_read(codec, ES8323_ADCCONTROL1));
	snd_soc_write(codec, ES8323_ADCCONTROL2,  0x50); //0x0a
	dbg("ES8323 ES8323_ADCCONTROL2,%x\n", snd_soc_read(codec, ES8323_ADCCONTROL2));
	snd_soc_write(codec, ES8323_ADCCONTROL3,  0x02);  //0x0b
	dbg("ES8323 ES8323_ADCCONTROL3%x\n", snd_soc_read(codec, ES8323_ADCCONTROL3));
	
#endif//CONFIG_HHTECH_MINIPMP
	/* for fpga */
	snd_soc_write(codec, ES8323_DACCONTROL21, 0x80); 
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL21));
    if (item_equal("codec.data", "i2s", 0))
	    snd_soc_write(codec, ES8323_ADCCONTROL4,  0x4c);  //0x0c
    else if (item_equal("codec.data", "pcm", 0))
	    snd_soc_write(codec, ES8323_ADCCONTROL4,  0x2F);
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL4));
	//snd_soc_write(codec, ES8323_ADCCONTROL4,  0x10);  //0x0c
	snd_soc_write(codec, ES8323_ADCCONTROL5,  0x02);//0x0d
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL5));
	//snd_soc_write(codec, ES8323_ADCCONTROL5,  0x00);//0x0d
	snd_soc_write(codec, ES8323_ADCCONTROL7,  0xf0);
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL7));

	snd_soc_write(codec, ES8323_ADCCONTROL8, 0x00);//0x10
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL8));
	snd_soc_write(codec, ES8323_ADCCONTROL9, 0x00);//0x11
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL9));

	snd_soc_write(codec, ES8323_ADCCONTROL10,  0xe2);//0x12
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL10));

	
	snd_soc_write(codec, ES8323_ADCCONTROL11, 0xa0); //0x13
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL11));
	snd_soc_write(codec, ES8323_ADCCONTROL12, 0x12); //0x14
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL12));
	snd_soc_write(codec, ES8323_ADCCONTROL13, 0x06); //0x15
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL13));
//	snd_soc_write(codec, ES8323_ADCCONTROL14, 0xc1); //0x16
	snd_soc_write(codec, ES8323_ADCCONTROL14, 0x9b); //0x16
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_ADCCONTROL14));
    if (item_equal("codec.data", "i2s", 0))
        snd_soc_write(codec, ES8323_DACCONTROL1, 0x18); //0x17
    else if (item_equal("codec.data", "pcm", 0))
        snd_soc_write(codec, ES8323_DACCONTROL1, 0x5E);
    dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL1));
	//snd_soc_write(codec, ES8323_DACCONTROL1, 0x20); //0x17
	snd_soc_write(codec, ES8323_DACCONTROL2, 0x02); //0x18
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL2));
	snd_soc_write(codec, ES8323_DACCONTROL4, 0x08); //0x1a
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL4));
	snd_soc_write(codec, ES8323_DACCONTROL5, 0x08); //0x1b
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL5));
  snd_soc_write(codec, 0x1E, 0x01);    //for 22uF capacitors ,12db Bass@100Hz,Fs=44100
  snd_soc_write(codec, 0x1F, 0x86);
  snd_soc_write(codec, 0x20, 0x37);
  snd_soc_write(codec, 0x21, 0xCF);
  snd_soc_write(codec, 0x22, 0x20);
  snd_soc_write(codec, 0x23, 0x6C);
  snd_soc_write(codec, 0x24, 0xAD);
  snd_soc_write(codec, 0x25, 0xFF);  
	snd_soc_write(codec, ES8323_DACCONTROL16, 0x09); //0x26
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL16));
	snd_soc_write(codec, ES8323_DACCONTROL17, 0xb8); //0x27
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL17));
	snd_soc_write(codec, ES8323_DACCONTROL18, 0x38); //0x28
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL18));
	snd_soc_write(codec, ES8323_DACCONTROL19, 0x38); //0x29
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL19));
	snd_soc_write(codec, ES8323_DACCONTROL20, 0xb8); //0x2a
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL20));
	snd_soc_write(codec, ES8323_DACCONTROL26, 0x1e); //0x30--0x1f
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL24));
	snd_soc_write(codec, ES8323_DACCONTROL27, 0x1e); //0x31--0x1f
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_DACCONTROL25));
	snd_soc_write(codec, ES8323_CHIPPOWER, 0x00); //0x02
	dbg("ES8323 %x\n", snd_soc_read(codec, ES8323_CHIPPOWER));

	return 0;
}

static int es8323_resume(struct snd_soc_codec *codec)
{
   es8323_set_bias_level(codec, SND_SOC_BIAS_ON);
	es8323_codec_init(codec);
	return 0;
}

/* *********************************************************************************************************/
static int es8323_probe(struct snd_soc_codec *codec)
{
    struct es8323_priv *es8323 = snd_soc_codec_get_drvdata(codec);
    int ret = 0;
    info("Audio Codec Driver %s", es8323_VERSION);
    ret = snd_soc_codec_set_cache_io(codec, 8, 8, es8323->control_type);
    if (ret < 0) {
        dev_err(codec->dev, "Failed to set cache I/O: %d\n", ret);
        return ret;
    }
    es8323_codec_init(codec);    
    es8323_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	snd_soc_add_controls(codec, es8323_snd_controls, 
				ARRAY_SIZE(es8323_snd_controls));
    es8323_add_widgets(codec);

    return ret;
}

/* power down chip */
static int es8323_remove(struct snd_soc_codec *codec)
{
    es8323_set_bias_level(codec, SND_SOC_BIAS_OFF);
    return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_es8323 = {
    .probe =    es8323_probe,
    .remove =   es8323_remove,
    .suspend =  es8323_suspend,
    .resume =   es8323_resume,
    .set_bias_level = es8323_set_bias_level,
#if 0
    .reg_cache_size = ARRAY_SIZE(es8323_reg),
    .reg_word_size = sizeof(u16),
    .reg_cache_default = es8323_reg,
#endif
};

/*******************************************************************************************************/
#if defined (CONFIG_I2C) || defined (CONFIG_I2C_MODULE)

static int es8323_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
    struct es8323_priv *es8323;
    char buf[ITEM_MAX_LEN];
    //char channel[100];
    int ret;

    es8323 = kzalloc(sizeof(struct es8323_priv), GFP_KERNEL);
    if (es8323 == NULL)
        return -ENOMEM;

    item_string(buf, "codec.power", 1);
    //sprintf(channel, "tps65910_%s", buf);
    printk("buf is %s\n", buf);
#if 0
    es8323->regulator = regulator_get(&i2c->dev, buf);
    if (IS_ERR(es8323->regulator)) {
        printk("%s: get regulator fail\n", __func__);
        return -1;
    }
    ret = regulator_set_voltage(es8323->regulator, 3300000, 3300000);
    if(ret) {
        printk("%s: set regulator fail\n", __func__);
        return -1;
    }

    regulator_enable(es8323->regulator);
    printk("%s: regulator enable\n", __func__);
#endif
    udelay(2000);

	i2c_set_clientdata(i2c, es8323);
    es8323->control_type = SND_SOC_I2C;
    
    ret = snd_soc_register_codec(&i2c->dev,
            &soc_codec_dev_es8323, &es8323_dai, 1);
    if (ret < 0) {
		err("cannot register codec\n");
        kfree(es8323);
	}
	return ret;
}

static int es8323_i2c_remove(struct i2c_client *client)
{
    struct es8323_priv *es8323 = i2c_get_clientdata(client);

    //regulator_put(es8323->regulator);
    snd_soc_unregister_codec(&client->dev);
	kfree(i2c_get_clientdata(client));
	return 0;
}

static const struct i2c_device_id es8323_i2c_id[] = {
	{ "es8323", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, es8323_i2c_id);

struct i2c_driver es8323_i2c_driver = {
	.driver = {
		.name = "es8323",
		.owner = THIS_MODULE,
	},
	.probe    = es8323_i2c_probe,
	.remove   = es8323_i2c_remove,
	.id_table = es8323_i2c_id,
};
EXPORT_SYMBOL(es8323_i2c_driver);
#endif
