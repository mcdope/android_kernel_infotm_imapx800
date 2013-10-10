#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <mach/items.h>
#include "imapx800-dma.h"
#include "imapx800-pwma.h"

//#define PWMA_DEBUG
#ifdef PWMA_DEBUG
#define pwma_dbg(x...) printk(x)
#else
#define pwma_dbg(x...)
#endif

static struct imapx800_dma_client imapx800_pch = {
    .name = "PWMA Stereo out"
};

static struct imapx800_pcm_dma_params imapx800_pwma_stereo_out = {
    .client     = &imapx800_pch,
    .channel    = IMAPX800_PWMA_TX,
    .dma_addr   = IMAP_PWMA_BASE + PWM_AUDIO_FIFO,
    .dma_size   = 4,
};

static void imapx800_pwma_txctrl_on(struct imapx800_pwma_info *pwma)
{
    int rTCFG2_value;
    int rCON_value;
    int tmp; 
    pwma_dbg("Entered %s\n", __func__);    
    /* set the first tcmpb */
    rTCFG2_value = readl(pwma->regs + PWM_AUDIO_TCFG2);
    rTCFG2_value |= 0x1;
    writel(rTCFG2_value, pwma->regs + PWM_AUDIO_TCFG2);
#if 1
	while(1) {
        tmp = readl(pwma->regs + PWM_AUDIO_TINT);
        if(tmp & 0x1) {
            writel(0x1, pwma->regs + PWM_AUDIO_TINT);
            rTCFG2_value = readl(pwma->regs + PWM_AUDIO_TCFG2);
            rTCFG2_value &= ~0x1;
            writel(rTCFG2_value, pwma->regs + PWM_AUDIO_TCFG2);
			break;
        }
    }
#endif
    rCON_value = readl(pwma->regs + PWM_AUDIO_TCON);
    rCON_value |= ((0x1<<3) | (0x1));
#if 0 //double channel can not open this why?
    if (pwma->chan_num == PWM_AUDIO_DOUBLE) 
        rCON_value |= (0x1<<11) | (0x1<<8);
#endif
    writel(rCON_value, pwma->regs + PWM_AUDIO_TCON);
    //printk("imapx800_pwma_txctrl_on %x\n", readl(pwma->regs + PWM_AUDIO_TCFG2));
}

static void imapx800_pwma_txctrl_off(struct imapx800_pwma_info *pwma)
{
    int rCON_value;

    pwma_dbg("Entered %s\n", __func__);
    
    rCON_value = readl(pwma->regs + PWM_AUDIO_TCON);
    rCON_value &= ~(0x1<<3);
    if (pwma->chan_num == PWM_AUDIO_DOUBLE)
        rCON_value &= ~(0x1<<11);
    writel(rCON_value, pwma->regs + PWM_AUDIO_TCON);
}

static int imapx800_pwma_set_sysclk(struct snd_soc_dai *dai,
        int clk_id, unsigned int freq, int format)
{
    struct imapx800_pwma_info *pwma = snd_soc_dai_get_drvdata(dai);
    int tcnt_val, pwm_audio_mux = 8;  //mux can be 2<=>8, 4<=>9, 8<=>10, 16<=>11 
    int rCON_value, rTCFG1_value, rTCFG2_value, format_value;
    pwma_dbg("Entered %s\n", __func__);
	pwma_dbg("channel is %d, freq is %d, format is %d\n",
				clk_id, freq, format);

    switch (clk_id) {
        case 2:
            pwma->chan_num = PWM_AUDIO_DOUBLE;
            break;
        case 1:
            pwma->chan_num = PWM_AUDIO_SINGLE;
            break;
        default:
            dev_err(pwma->dev, "Not support this channel: %d\n", clk_id);
            return -EINVAL;
	}
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
		default:
			dev_err(pwma->dev, "Not support this format: %d\n", format);
			return -EINVAL;
    }
    /*   set channel and width */
    rTCFG2_value = readl(pwma->regs + PWM_AUDIO_TCFG2);
    rTCFG2_value &= ~(0x3<<1);
    rTCFG2_value |= (pwma->chan_num << 2 | (format_value / 16) << 1);
    writel(rTCFG2_value, pwma->regs + PWM_AUDIO_TCFG2);
	
	/*  set mux */
	rTCFG1_value = readl(pwma->regs + PWM_AUDIO_CFG1);
	rTCFG1_value &= ~0xf;
	rTCFG1_value |= (pwm_audio_mux);
	if (pwma->chan_num == PWM_AUDIO_DOUBLE) {
		rTCFG1_value &= ~(0xf << 4);
		rTCFG1_value |= (pwm_audio_mux << 4);
	}   
	writel(rTCFG1_value, pwma->regs + PWM_AUDIO_CFG1);

    /*   set pwma clock */
    tcnt_val = pwma->iis_clock_rate / (freq * 2);  
    writel(tcnt_val, pwma->regs + PWM_AUDIO_TCNTB0);
    if (pwma->chan_num == PWM_AUDIO_DOUBLE) {
        //rCON_value |= (0x1<<9);
         writel(tcnt_val, pwma->regs + PWM_AUDIO_TCNTB1);
    }
	rCON_value = readl(pwma->regs + PWM_AUDIO_TCON);
	rCON_value |= (0x1<<1);
	writel(rCON_value, pwma->regs + PWM_AUDIO_TCON);
    
    return 0;
}

static int imapx800_pwma_set_clkdiv(struct snd_soc_dai *dai,
        int div_id, int div)
{
    pwma_dbg("Entered %s\n", __func__);
    return 0;
}

static int imapx800_pwma_trigger(struct snd_pcm_substream *substream,
        int cmd, struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct imapx800_pwma_info *pwma = snd_soc_dai_get_drvdata(rtd->cpu_dai);

    pwma_dbg("Entered %s\n", __func__);

    switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
                dev_err(pwma->dev, "PWMA only support playback\n");
                return -EINVAL;
            } else
                imapx800_pwma_txctrl_on(pwma);
            break;

        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
                dev_err(pwma->dev, "PWMA only support playback\n");
                return -EINVAL;
            } else
                imapx800_pwma_txctrl_off(pwma);
            break;

        default:
            dev_err(pwma->dev, "Unsupport this cmd: %d\n", cmd);
            return -EINVAL;
    }
    return 0;
}

static int imapx800_pwma_hw_params(struct snd_pcm_substream *substream,
        struct snd_pcm_hw_params *params, struct snd_soc_dai *socdai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct imapx800_pwma_info *pwma = snd_soc_dai_get_drvdata(rtd->cpu_dai);
    struct imapx800_pcm_dma_params *dma_data;
    int rINT_value, rINTSTATE_value, rTCFG2_value;

    pwma_dbg("Entered %s, stream is %d\n", __func__, substream->stream);
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        dma_data = &imapx800_pwma_stereo_out;
    else {
        dev_err(pwma->dev, "Not support this operate\n");
        return -EINVAL;
    }
    snd_soc_dai_set_dma_data(rtd->cpu_dai, substream, dma_data);
    
    /* set irq */
    rINT_value = readl(pwma->regs + PWM_AUDIO_TINTEN);
    rINT_value &= ~(0x3 << 0);
    writel(rINT_value, pwma->regs + PWM_AUDIO_TINTEN);
    rINTSTATE_value = readl(pwma->regs + PWM_AUDIO_TSTSEN);
    rINTSTATE_value = (0x1 << 0);
    writel(rINTSTATE_value, pwma->regs + PWM_AUDIO_TSTSEN);

    /* set sample-repeat */
    rTCFG2_value = readl(pwma->regs + PWM_AUDIO_TCFG2);
    rTCFG2_value &= ~(0x3<<8);
    rTCFG2_value |= (0x1 << 8);
    /* dma enable */
    rTCFG2_value |= (0x1<<3);
	/* set fifo */
	rTCFG2_value &= ~(0xf << 4);
	rTCFG2_value |= (0xa << 4);
    writel(rTCFG2_value, pwma->regs + PWM_AUDIO_TCFG2);

    return 0;
}

static int imapx800_pwma_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    struct imapx800_pwma_info *pwma = snd_soc_dai_get_drvdata(dai);
    int rCON_value;
    pwma_dbg("Entered %s\n", __func__);

    rCON_value = readl(pwma->regs + PWM_AUDIO_TCON);
    switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_IF:
            rCON_value &= ~(0x1<<2);
            rCON_value |= 0x1<<10;
            break;
        default:
            dev_err(pwma->dev, "Not support this fmt: %d\n", 
                    fmt & SND_SOC_DAIFMT_INV_MASK);
            return -EINVAL;
    }
    writel(rCON_value, pwma->regs + PWM_AUDIO_TCON);

    return 0;
}

static struct snd_soc_dai_ops imapx800_pwma_dai_ops = {
    .set_sysclk = imapx800_pwma_set_sysclk,
    .set_clkdiv = imapx800_pwma_set_clkdiv,
    .trigger    = imapx800_pwma_trigger,
    .hw_params  = imapx800_pwma_hw_params,
    .set_fmt    = imapx800_pwma_set_fmt,
};

#define IMAPX800_PWMA_RATES  SNDRV_PCM_RATE_8000_48000 | SNDRV_PCM_RATE_128000 \
                               | SNDRV_PCM_RATE_132300

struct snd_soc_dai_driver imapx800_pwma_dai = {
        .name   = "imapx800-pwma",
        .playback = {                  
            .channels_min   = 2,                
            .channels_max   = 2,                
            .rates      = IMAPX800_PWMA_RATES,       
            .formats    = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE,  
        },
        .ops = &imapx800_pwma_dai_ops,        
};

static __devinit int imapx800_pwma_probe (struct platform_device *pdev)
{
    struct imapx800_pwma_info *pwma;
    struct resource *mem, *ioarea;
    int ret = 0;
    
    module_power_on(SYSMGR_PWMA_BASE);

    if (pdev == NULL) {
        dev_err(&pdev->dev, "No platform data\n");
        return -ENODEV;
    }

    if( (item_exist("board.cpu") == 0 )||item_equal("board.cpu", "imapx820", 0) ) 
    {
        writel(0x1, IO_ADDRESS(0x21e14C20));
        imapx_pad_cfg(IMAPX_IIS0,1);
    } else if(item_equal("board.cpu", "i15", 0))
    {
        imapx_pad_cfg(IMAPX_PWMA,1);
    }
    pwma = (struct imapx800_pwma_info *)kzalloc(
            sizeof(struct imapx800_pwma_info), GFP_KERNEL);
    if (IS_ERR(pwma)) {
        dev_err(&pdev->dev, "Cannot malloc for pwma\n");
        return -ENXIO;
    }
    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem) {
        dev_err(&pdev->dev, "Unable to get register mem\n");
        return -ENXIO; 
    }
    ioarea = request_mem_region(mem->start, resource_size(mem),pdev->name);
    if (!ioarea) {
        dev_err(&pdev->dev, "PWMA region already claimed!\n");
        return -EBUSY;
    }
    pwma->extclk = clk_get(&pdev->dev, "audio-clk");
    if (IS_ERR(pwma->extclk)) {
        ret = -ENODEV;
        goto free_mem;
    }
    clk_enable(pwma->extclk);
    pwma->iis_clock_rate = clk_get_rate(pwma->extclk);
    pwma_dbg("Pwma clock is %d\n", pwma->iis_clock_rate);

    pwma->regs = ioremap(mem->start, resource_size(mem));
    if (!pwma->regs) {
        dev_err(&pdev->dev, "failure mapping io resources!\n");
        ret = -EBUSY;
        goto free_clk;
    }
    pwma->dev = &pdev->dev;
    ret = snd_soc_register_dai(&pdev->dev, &imapx800_pwma_dai);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get pcm_clock\n");
        ret = -EBUSY;
        goto free_remap;
    }
    dev_set_drvdata(&pdev->dev, pwma);
    
	return 0;

free_remap:
    iounmap(pwma->regs);
free_clk:
    clk_disable(pwma->extclk);
    clk_put(pwma->extclk);
free_mem:
    release_mem_region(mem->start, resource_size(mem));
    
    return ret;
}

static __devexit int imapx800_pwma_remove(struct platform_device *pdev)
{
    struct imapx800_pwma_info *pwma = dev_get_drvdata(&pdev->dev);
    struct resource *mem_res;

    snd_soc_unregister_dai(&pdev->dev);
    
    iounmap(pwma->regs); 
    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(mem_res->start, resource_size(mem_res));
    clk_disable(pwma->extclk);
    clk_put(pwma->extclk);
    
    kfree(pwma);
    return 0;
}

static int imapx800_pwma_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static void imapx800_pwma_resume(struct platform_device *pdev)
{
    module_power_on(SYSMGR_PWMA_BASE);
    
    if( (item_exist("board.cpu") == 0 )||item_equal("board.cpu", "imapx820", 0) ) 
    {
        writel(0x1, IO_ADDRESS(0x21e14C20));
        imapx_pad_cfg(IMAPX_IIS0,1);
    } else if(item_equal("board.cpu", "i15", 0))
    {
        imapx_pad_cfg(IMAPX_PWMA,1);
    }
}

static struct platform_driver imapx800_pwma_driver = {
    .probe  = imapx800_pwma_probe,
    .remove = imapx800_pwma_remove,
    .suspend = imapx800_pwma_suspend,
    .resume  = imapx800_pwma_resume,
    .driver = {
        .name   = "imap-pwma",
        .owner  = THIS_MODULE,
    },
};

static int __init imapx800_pwma_init(void)
{
    if (!item_equal("codec.playback", "pwma", 0))
        return -1;
    return platform_driver_register(&imapx800_pwma_driver);
}
module_init(imapx800_pwma_init);

static void __exit imapx800_pwma_exit(void)
{
    platform_driver_unregister(&imapx800_pwma_driver);
}
module_exit(imapx800_pwma_exit);

MODULE_AUTHOR("SUN");
MODULE_DESCRIPTION("IMAPX800 PCM Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imapx800-pwma");
