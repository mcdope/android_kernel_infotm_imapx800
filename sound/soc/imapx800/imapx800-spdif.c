#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <linux/imap_pl330.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <mach/items.h>
#include "imapx800-dma.h"
#include "imapx800-spdif.h"

//#define SPDIF_DEBUG
#ifdef SPDIF_DEBUG
#define spdif_dbg(x...) printk(x)
#else
#define spdif_dbg(x...)
#endif

#ifdef INFOTM_MEDIA_IDS_I800_SUPPORT
static int imapx800_i2s_to_hdmi(struct snd_pcm_hw_params *params)
{
    hdmi_audio_t audio_cfg;

    audio_cfg.aInterfaceType = IM_I2S;
    audio_cfg.aCodingType = IM_PCM;
    audio_cfg.channelAllocation = 0;
    audio_cfg.sampleSize = params_format(params);
    audio_cfg.samplingFreq = params_rate(params);

    hdmi_set_audio_basic_config(&audio_cfg);
}
#endif

static int imapx800_spdif_set_channel(struct imapx800_spdif_info *spdif, int clk_id)
{
    int rtxconfig_value;
    
    if (clk_id < 0 || clk_id > 2) {
        dev_err(spdif->dev, "Spdif channel num err: %d\n", clk_id);
        return -1;
    }
    rtxconfig_value = readl(spdif->regs + SP_TXCONFIG);
    rtxconfig_value &= ~(0x1 << 17);
    rtxconfig_value |= clk_id - 1;
    writel(rtxconfig_value, spdif->regs + SP_TXCONFIG);
    
    return 0;
}

static struct imapx800_dma_client imapx800_pch = {
    .name = "Spdif Stereo out"
};

static struct imapx800_pcm_dma_params imapx800_spdif_stereo_out = {
    .client     = &imapx800_pch,
    .channel    = IMAPX800_SPDIF_TX,
    .dma_addr   = IMAP_SPDIF_BASE + SP_TXFIFO,
    .dma_size   = 2,
};

static void imapx800_spdif_playback(struct imapx800_spdif_info *spdif, int flags)
{
    int rSP_CTRL = 0, rSP_TXCONFIG = 0;
    
    //enable the module or not 
    rSP_CTRL = readl(spdif->regs + SP_CTRL);
    rSP_CTRL &= ~(0x3f);
    if (flags)
        rSP_CTRL |= 0x3f;
    writel(rSP_CTRL, spdif->regs + SP_CTRL);
    //enable dma or not
    rSP_TXCONFIG = readl(spdif->regs + SP_TXCONFIG);
    rSP_TXCONFIG &= ~(0x1<<7);
    if (flags)
        rSP_TXCONFIG |= 0x1<<7;
    writel(rSP_TXCONFIG, spdif->regs + SP_TXCONFIG);
}

static int imapx800_spdif_set_sysclk(struct snd_soc_dai *dai,
        int clk_id, unsigned int freq, int format)
{
    struct imapx800_spdif_info *spdif = snd_soc_dai_get_drvdata(dai);
    int format_value, rSP_TXCONFIG, mdiv, rSP_CTRL;

    spdif_dbg("channel is %d, freq is %d, format is %d\n",
            clk_id, freq, format);

    imapx800_spdif_set_channel(spdif, clk_id);
    //set word length
    switch (format) {
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
        default:
            dev_err(spdif->dev, "Not support this format: %d\n", format);
            return -EINVAL;
    }
    rSP_TXCONFIG = readl(spdif->regs + SP_TXCONFIG);
    rSP_TXCONFIG &= ~(0xf<<28);
    rSP_TXCONFIG |= (format_value - 16)<<28;
    writel(rSP_TXCONFIG, spdif->regs + SP_TXCONFIG);
    //set clock source
    rSP_CTRL = readl(spdif->regs + SP_CTRL);
    rSP_CTRL &= ~(0x3 << 6);
    rSP_CTRL |= 0x3 << 6;
    writel(rSP_CTRL, spdif->regs + SP_CTRL);
    //set rate ratio
    mdiv = spdif->iis_clock_rate / (128 * freq) - 1;
    printk("The spdif div is %d\n", mdiv);
    rSP_TXCONFIG = readl(spdif->regs + SP_TXCONFIG);
    rSP_TXCONFIG &= ~(0xff<<20);
    rSP_TXCONFIG |= (mdiv<<20);
    writel(rSP_TXCONFIG, spdif->regs + SP_TXCONFIG);
    
    return 0;
}

static int imapx800_spdif_set_clkdiv(struct snd_soc_dai *dai,
        int div_id, int div)
{
    spdif_dbg("Entered %s\n", __func__);
    return 0;
}

static int imapx800_spdif_trigger(struct snd_pcm_substream *substream,
        int cmd, struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct imapx800_spdif_info *spdif = snd_soc_dai_get_drvdata(rtd->cpu_dai);
    
    spdif_dbg("Entered %s\n", __func__);
    
    spin_lock(&spdif->lock);
    switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
                dev_err(spdif->dev, "Spdif only support playback\n");
                return -EINVAL;
            } else
                imapx800_spdif_playback(spdif, 1);
            break;
        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
                dev_err(spdif->dev, "Spdif only support playback\n");
                return -EINVAL;
            } else
                imapx800_spdif_playback(spdif, 0);
            break;
        default:
            dev_err(spdif->dev, "Unsupport this cmd: %d\n", cmd);
            spin_unlock(&spdif->lock);
            return -EINVAL;
    }
    spin_unlock(&spdif->lock);

    return 0;
}

static int imapx800_spdif_hw_params(struct snd_pcm_substream *substream,
        struct snd_pcm_hw_params *params, struct snd_soc_dai *socdai)
{
    struct imapx800_spdif_info *spdif = snd_soc_dai_get_drvdata(socdai);
    struct imapx800_pcm_dma_params *dma_data; 
    int rSP_TXCONFIG, rSP_RXCONFIG, rSP_INTMASK;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        dma_data = &imapx800_spdif_stereo_out;
    else {
        dev_err(spdif->dev, "Now spdif only support playback mode\n");
        return -ENODEV;
    }
    snd_soc_dai_set_dma_data(socdai, substream, dma_data);
#ifdef INFOTM_MEDIA_IDS_I800_SUPPORT
    imapx800_i2s_to_hdmi(params);
#endif
    //set fifo level
    rSP_TXCONFIG = readl(spdif->regs + SP_TXCONFIG);
    rSP_TXCONFIG &= ~0x1f;
    rSP_TXCONFIG |= 0x4;
    writel(rSP_TXCONFIG, spdif->regs + SP_TXCONFIG);
    rSP_RXCONFIG = readl(spdif->regs + SP_RXCONFIG);
    rSP_RXCONFIG &= ~0x1f;
    rSP_RXCONFIG |= 0x4;
    writel(rSP_RXCONFIG, spdif->regs + SP_RXCONFIG);
    //save rx chan and user data
    rSP_RXCONFIG = readl(spdif->regs + SP_RXCONFIG);
    rSP_RXCONFIG &= ~(0xf << 18);
    rSP_RXCONFIG |= 0xf << 18;
    writel(rSP_RXCONFIG, spdif->regs + SP_RXCONFIG);
    //set tx validy
    rSP_TXCONFIG = readl(spdif->regs + SP_TXCONFIG);
    rSP_TXCONFIG &= ~(0x1 << 6);
    rSP_TXCONFIG |= 0x1 << 6;
    writel(rSP_TXCONFIG, spdif->regs + SP_TXCONFIG);
    //mask irq
    rSP_INTMASK = readl(spdif->regs + SP_INTMASK);
    rSP_INTMASK |= 0xffffffff;
    rSP_INTMASK &= ~(0x3 << 24);
    writel(rSP_INTMASK, spdif->regs + SP_INTMASK);

    return 0;
}

static int imapx800_spdif_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    struct imapx800_spdif_info *spdif = snd_soc_dai_get_drvdata(dai);

    spdif_dbg("Entered %s\n", __func__);

    //only for reset the control
    writel(0, spdif->regs + SP_TXCONFIG);
    writel(0, spdif->regs + SP_RXCONFIG);
    writel(0, spdif->regs + SP_TXCHST);
    writel(0, spdif->regs + SP_INTMASK);
    writel(0, spdif->regs + SP_CTRL);
    return 0;
}

static struct snd_soc_dai_ops imapx800_spdif_dai_ops = {
    .set_sysclk = imapx800_spdif_set_sysclk,
    .set_clkdiv = imapx800_spdif_set_clkdiv,
    .trigger    = imapx800_spdif_trigger,
    .hw_params  = imapx800_spdif_hw_params,
    .set_fmt    = imapx800_spdif_set_fmt,
};

struct snd_soc_dai_driver spdif_dai = {
    .name   = "imap-spdif",
    .playback = {
        .channels_min   = 2,
        .channels_max   = 2,
        .rates      = SNDRV_PCM_RATE_8000_192000,
        .formats    = SNDRV_PCM_FMTBIT_S16_LE |  SNDRV_PCM_FMTBIT_S24_LE,
    },
    .ops = &imapx800_spdif_dai_ops,
};

static void imapx800_spdif_module (void)
{
    module_power_on(SYSMGR_SPDIF_BASE);
    imapx_pad_cfg(IMAPX_SPDIF, 1);
}

static __devinit int imapx800_spdif_probe (struct platform_device *pdev)
{
    struct imapx800_spdif_info *spdif;
    struct resource *mem, *ioarea;
    int ret = 0;
    
    imapx800_spdif_module();

    if (!pdev) {
        dev_err(&pdev->dev, "No spdif platform data\n");
        return -ENODEV;
    }
    spdif = (struct imapx800_spdif_info *)kzalloc(
            sizeof(struct imapx800_spdif_info), GFP_KERNEL);
    if (IS_ERR(spdif)) {
        dev_err(&pdev->dev, "malloc spdif info err\n");
        return -ENXIO;
    }
    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem) {
        dev_err(&pdev->dev, "Get register mem err\n");
        goto err_alloc;
    }
    ioarea = request_mem_region(mem->start, resource_size(mem),pdev->name);
    if (!ioarea) {
        dev_err(&pdev->dev, "Mem have already claimed\n");
        goto err_alloc;
    }
    spdif->extclk = clk_get(&pdev->dev, "spdif");
    if (IS_ERR(spdif->extclk)) {
        dev_err(&pdev->dev, "Spdif clock get err\n");
        goto err_alloc;
    }
    clk_enable(spdif->extclk);
    spdif->iis_clock_rate = clk_get_rate(spdif->extclk);
    spdif_dbg("Spdif clock is %d\n", spdif->iis_clock_rate);

    spdif->regs = ioremap(mem->start, resource_size(mem));
    if (!spdif->regs) {
        dev_err(&pdev->dev, "Spdif ioremap err\n");
        goto err_alloc;
    }
    spdif->dev = &pdev->dev;
    spin_lock_init(&spdif->lock);

    ret = snd_soc_register_dai(&pdev->dev, &spdif_dai);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to register spdif dai\n");
        ret = -EBUSY;
        goto err_unmap;
    }
    dev_set_drvdata(&pdev->dev, spdif);

    return 0;

err_unmap:
    iounmap(spdif->regs);
err_alloc:
    kfree(spdif);

    return ret;
}

static __devexit int imapx800_spdif_remove(struct platform_device *pdev)
{
    struct imapx800_spdif_info *spdif = dev_get_drvdata(&pdev->dev);
    struct resource *mem_res;

    snd_soc_unregister_dai(&pdev->dev);

    iounmap(spdif->regs);
    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(mem_res->start, resource_size(mem_res));
    clk_disable(spdif->extclk);
    clk_put(spdif->extclk);

    kfree(spdif);
    return 0;
}

static struct platform_driver imapx800_spdif_driver = {
    .probe  = imapx800_spdif_probe,
    .remove = imapx800_spdif_remove,
    .driver = {
        .name   = "imapx800-spdif",
        .owner  = THIS_MODULE,
    },
};

static int __init imapx800_spdif_init(void)
{
    if (!item_equal("codec.model", "spdif", 0))
        return -1;
    return platform_driver_register(&imapx800_spdif_driver);
}
module_init(imapx800_spdif_init);

static void __exit imapx800_spdif_exit(void)
{
    platform_driver_unregister(&imapx800_spdif_driver);
}
module_exit(imapx800_spdif_exit);

MODULE_AUTHOR("SUN");
MODULE_DESCRIPTION("IMAPX800 SPDIF Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imapx800-spdif");
