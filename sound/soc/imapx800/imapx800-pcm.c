#include <linux/clk.h>
#include <linux/io.h>
#include <mach/pad.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <linux/imap_pl330.h>
#include <mach/power-gate.h>
#include <mach/items.h>
#include "imapx800-dma.h"
#include "imapx800-pcm.h"

static struct imapx800_dma_client imapx800_pcm_dma_client_out = {
    .name       = "PCM Stereo out"
};

static struct imapx800_dma_client imapx800_pcm_dma_client_in = {
    .name       = "PCM Stereo in"
};

static struct imapx800_pcm_dma_params imapx800_pcm_stereo_out[] = {
    [0] = {
        .client     = &imapx800_pcm_dma_client_out,
        .channel    = IMAPX800_PCM0_TX,
        .dma_addr   = IMAP_PCM0_BASE + IMAPX800_PCM_TXFIFO,
        .dma_size   = 2,
    },
    [1] = {
        .client     = &imapx800_pcm_dma_client_out,
        .channel    = IMAPX800_PCM1_TX,
        .dma_addr   = IMAP_PCM1_BASE + IMAPX800_PCM_TXFIFO,
        .dma_size   = 2,
    },
};

static struct imapx800_pcm_dma_params imapx800_pcm_stereo_in[] = {
    [0] = {
        .client     = &imapx800_pcm_dma_client_in,
        .channel    = IMAPX800_PCM0_RX,
        .dma_addr   = IMAP_PCM0_BASE + IMAPX800_PCM_RXFIFO,
        .dma_size   = 2,
    },
    [1] = {
        .client     = &imapx800_pcm_dma_client_in,
        .channel    = IMAPX800_PCM1_RX,
        .dma_addr   = IMAP_PCM1_BASE + IMAPX800_PCM_RXFIFO,
        .dma_size   = 2,
    },
};

static struct imapx800_pcm_info imapx800_pcm[2];

static void imapx800_pcm_disable_module(struct imapx800_pcm_info *pcm)
{
    int rCLKCTL_value, rCFG_value;
    dev_dbg(pcm->dev, "Entered %s\n", __func__);

    /*   disable pcm clock */
    rCLKCTL_value = readl(pcm->regs + IMAPX800_PCM_CLKCTL);
    rCLKCTL_value &= ~(0xf << 0);
    writel(rCLKCTL_value, pcm->regs + IMAPX800_PCM_CLKCTL);
    /*  disable pcm module */
    writel(0x0, pcm->regs + IMAPX800_PCM_CTL);
    /*  disable pcm dma */
    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    rCFG_value &= ~(3 << 2);
    writel (rCFG_value, pcm->regs + IMAPX800_PCM_CFG);
}

static void imapx800_pcm_enable_module(struct imapx800_pcm_info *pcm, int dir)
{
    int rCLKCTL_value, rCFG_value;
    dev_dbg(pcm->dev, "Entered %s\n", __func__);

    /*    disable pcm clock */
    rCLKCTL_value = readl(pcm->regs + IMAPX800_PCM_CLKCTL);
    rCLKCTL_value |= (0xf << 0);
    writel(rCLKCTL_value, pcm->regs + IMAPX800_PCM_CLKCTL);
    /*   disable pcm module */
    writel(0x3D, pcm->regs + IMAPX800_PCM_CTL);
    /*   disable pcm dma */
    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    rCFG_value &= ~(3 << 2);
    rCFG_value |= (dir << 2);
    writel (rCFG_value, pcm->regs + IMAPX800_PCM_CFG);
}

static void imapx800_pcm_snd_rxctrl(struct imapx800_pcm_info *pcm, int on)
{
    int rCFG_value;
    dev_dbg(pcm->dev, "Entered %s\n", __func__);
    
    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    
    if (on) 
        imapx800_pcm_enable_module(pcm, 0x1);    
    else
        imapx800_pcm_disable_module(pcm);
}

static void imapx800_pcm_snd_txctrl(struct imapx800_pcm_info *pcm, int on)
{
    dev_dbg(pcm->dev, "Entered %s\n", __func__);

    if (on)
        imapx800_pcm_enable_module(pcm, 0x2);
    else
        imapx800_pcm_disable_module(pcm);
}

static int imapx800_pcm_set_sysclk(struct snd_soc_dai *cpu_dai,
        int clk_id, unsigned int freq, int dir)
{
    struct imapx800_pcm_info *pcm = snd_soc_dai_get_drvdata(cpu_dai);
    int format_value = 0, rCFG_value, rCLKCFG_value;
    int mclkdiv, bitclkdiv, syncdiv, bit_clk, pcm_div;

    dev_dbg(pcm->dev, "Entered %s : channel = %d, freq is %d, format is %d\n", 
            __FUNCTION__, clk_id, freq, dir);
    
    switch(dir){
        case 0:
        case 1:
            format_value = PCM_WL_REG_8;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            format_value = PCM_WL_REG_16;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
            format_value = PCM_WL_REG_24;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
            format_value = PCM_WL_REG_32;
            break;
    }
    /* set pcm cd length and channel num */
    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    rCFG_value &= ~((0x7 << 23) | (0x31 << 16));
    rCFG_value |= format_value << 16;
    rCFG_value |= (clk_id - 1) << 23;
    writel(rCFG_value, pcm->regs + IMAPX800_PCM_CFG);
    
    /* set bclk and mclk */
    if ((format_value + 16) > 32)
        format_value = 8;
    else
        format_value = PCM_WL_REG_16 + 16;
	
	if (freq == 96000)
		pcm_div = 128;
	else
		pcm_div = 256;
	bit_clk = freq * clk_id * format_value;
	bitclkdiv = pcm->iis_clock_rate / bit_clk;
    mclkdiv = bitclkdiv / (pcm_div / (format_value * clk_id));
    bitclkdiv = mclkdiv * (pcm_div / (format_value * clk_id));
    writel(mclkdiv - 1, pcm->regs + IMAPX800_PCM_MCLK_CFG);
    writel(bitclkdiv - 1, pcm->regs + IMAPX800_PCM_BCLK_CFG);
	dev_dbg(pcm->dev, "bclk is %d, mclk is %d\n", bitclkdiv - 1, mclkdiv - 1);
    /* set pcm syncdiv */
    syncdiv = format_value * clk_id;
    rCLKCFG_value = readl(pcm->regs + IMAPX800_PCM_CLKCTL);
    rCLKCFG_value &= ~(0x1ff << 8);
    rCLKCFG_value |= ((syncdiv-1) << 8);
    writel(rCLKCFG_value, pcm->regs + IMAPX800_PCM_CLKCTL);
     
    return 0;
}

static int imapx800_pcm_set_clkdiv(struct snd_soc_dai *cpu_dai,
        int div_id, int div)
{
    struct imapx800_pcm_info *pcm = snd_soc_dai_get_drvdata(cpu_dai);
    dev_dbg(pcm->dev, "Entered %s\n", __func__);

    return 0;
}

static int imapx800_pcm_trigger(struct snd_pcm_substream *substream, int cmd,
        struct snd_soc_dai *dai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct imapx800_pcm_info *pcm = snd_soc_dai_get_drvdata(rtd->cpu_dai);
    unsigned long flags;

    dev_dbg(pcm->dev, "Entered %s, state = %x\n", __func__, cmd);


    switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            spin_lock_irqsave(&pcm->lock, flags);

            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
                imapx800_pcm_snd_rxctrl(pcm, 1);
            else
                imapx800_pcm_snd_txctrl(pcm, 1);

            spin_unlock_irqrestore(&pcm->lock, flags);
            break;

        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            spin_lock_irqsave(&pcm->lock, flags);

            if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
                imapx800_pcm_snd_rxctrl(pcm, 0);
            else
                imapx800_pcm_snd_txctrl(pcm, 0);

            spin_unlock_irqrestore(&pcm->lock, flags);
            break;

        default:
            dev_err(pcm->dev, "Unsupport this cmd: %d\n", cmd);
            return -EINVAL;
    }

    return 0;
}

static int imapx800_pcm_hw_params(struct snd_pcm_substream *substream,
        struct snd_pcm_hw_params *params, struct snd_soc_dai *socdai)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct imapx800_pcm_info *pcm = snd_soc_dai_get_drvdata(rtd->cpu_dai);
    struct imapx800_pcm_dma_params *dma_data;
    unsigned long flags;
    int rCFG_value, rIRQCTL_value;
  
    dev_dbg(pcm->dev, "Entered %s\n", __func__);
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        dma_data = pcm->dma_playback;
    else
        dma_data = pcm->dma_capture;
    snd_soc_dai_set_dma_data(rtd->cpu_dai, substream, dma_data);

    spin_lock_irqsave(&pcm->lock, flags);
    /* set fifo size */
    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    rCFG_value &= ~(0x3ff<<4);
    rCFG_value |= (16 << 10) | (16 << 4);
    /* set pcm transfer */
    rCFG_value &= ~(0x3 << 21);
    /* MSB pos */
    rCFG_value &= ~(0x3 << 0);
    writel(rCFG_value, pcm->regs + IMAPX800_PCM_CFG);
    /* set irq */
    rIRQCTL_value = TX_FIFO_ALMOST_EMPTY | TX_FIFO_EMPTY |
        EN_IRQ_TO_ARM;
    writel(rIRQCTL_value, pcm->regs + IMAPX800_PCM_IRQ_CTL);
    spin_unlock_irqrestore(&pcm->lock, flags); 
    return 0;
}

static int imapx800_pcm_set_fmt(struct snd_soc_dai *cpu_dai,
        unsigned int fmt)
{
    struct imapx800_pcm_info *pcm = snd_soc_dai_get_drvdata(cpu_dai); 
    int ret = 0, rCLKCFG_value, rCFG_value;
    unsigned long flags;
    dev_dbg(pcm->dev, "Entered %s\n", __func__);

    spin_lock_irqsave(&pcm->lock, flags);
    rCLKCFG_value = readl(pcm->regs + IMAPX800_PCM_CLKCTL);
    rCLKCFG_value &= ~(0x3 << 5);
    switch(fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
            break;
        case SND_SOC_DAIFMT_NB_IF:
            rCLKCFG_value |= (0x1 << 5);
            break;
        case SND_SOC_DAIFMT_IB_NF:
            rCLKCFG_value |= (0x2 << 5);
            break;
        case SND_SOC_DAIFMT_IB_IF:
            rCLKCFG_value |= (0x3 << 4);
            break;
        default:
            dev_err(pcm->dev, "Unsupported this mode clock inversion: %d\n", fmt);
            ret = -EINVAL;                                      
            goto exit;
    }
	writel(rCLKCFG_value, pcm->regs + IMAPX800_PCM_CLKCTL);

    rCFG_value = readl(pcm->regs + IMAPX800_PCM_CFG);
    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
        case SND_SOC_DAIFMT_CBM_CFM:
            rCFG_value &= ~(1 << 27);
            break;
        default:
            dev_err(pcm->dev, "Now only support master mode: %d\n", 
                    fmt & SND_SOC_DAIFMT_MASTER_MASK);
            ret = -EINVAL;
            goto exit;
    }
    writel (rCFG_value, pcm->regs + IMAPX800_PCM_CFG);
exit:
    spin_unlock_irqrestore(&pcm->lock, flags);
    return ret;
}

static struct snd_soc_dai_ops imapx800_pcm_dai_ops = {
    .set_sysclk = imapx800_pcm_set_sysclk,
    .set_clkdiv = imapx800_pcm_set_clkdiv,
    .trigger    = imapx800_pcm_trigger,
    .hw_params  = imapx800_pcm_hw_params,
    .set_fmt    = imapx800_pcm_set_fmt,
};

#define IMAPX800_PCM_RATES  SNDRV_PCM_RATE_8000_96000

#define IMAPX800_PCM_DAI_DECLARE            \
    .symmetric_rates = 1,                   \
.ops = &imapx800_pcm_dai_ops,               \
.playback = {                       \
    .channels_min   = 2,                \
    .channels_max   = 2,                \
    .rates      = IMAPX800_PCM_RATES,       \
    .formats    = SNDRV_PCM_FMTBIT_S16_LE,  \
},                          \
.capture = {                        \
    .channels_min   = 2,                \
    .channels_max   = 2,                \
    .rates      = IMAPX800_PCM_RATES,       \
    .formats    = SNDRV_PCM_FMTBIT_S16_LE,  \
}

struct snd_soc_dai_driver imapx800_pcm_dai[] = {
    [0] = {
        .name   = "imapx800-pcm.0",
        IMAPX800_PCM_DAI_DECLARE,
    },
    [1] = {
        .name   = "imapx800-pcm.1",
        IMAPX800_PCM_DAI_DECLARE,
    }
};

static __devinit int imapx800_pcm_probe (struct platform_device *pdev)
{
    struct imapx800_pcm_info *pcm;
    struct resource *mem, *ioarea;
    int ret = 0;

    if ((pdev->id < 0) || pdev->id >= ARRAY_SIZE(imapx800_pcm)) {
        dev_err(&pdev->dev, "id %d out of range\n", pdev->id);
        return -EINVAL;
    }
    if(pdev->id == 0)
        imapx_pad_cfg(IMAPX_PCM0,1);
    else
        imapx_pad_cfg(IMAPX_PCM1,1);

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem) {
        dev_err(&pdev->dev, "Unable to get register mem\n");
        return -ENXIO; 
    }
    ioarea = request_mem_region(mem->start, resource_size(mem),pdev->name);
    if (!ioarea) {
        dev_err(&pdev->dev, "PCM region already claimed!\n");
        return -EBUSY;
    }
    
    pcm = &imapx800_pcm[pdev->id];
    pcm->dev = &pdev->dev;
    
    spin_lock_init(&pcm->lock);
    
    switch (pdev->id) {
        case 0:
            pcm->extclk = clk_get(&pdev->dev, "audio-clk");
        case 1:
            pcm->extclk = clk_get(&pdev->dev, "audio-clk");
    }
    if (IS_ERR(pcm->extclk)) {
        ret = -ENODEV;
        goto free_mem;
    }
    clk_enable(pcm->extclk);
    pcm->iis_clock_rate = clk_get_rate(pcm->extclk);

    pcm->regs = ioremap(mem->start, resource_size(mem));
    if (!pcm->regs) {
        dev_err(&pdev->dev, "failure mapping io resources!\n");
        ret = -EBUSY;
        goto free_clk;
    }
       
    ret = snd_soc_register_dai(&pdev->dev, &imapx800_pcm_dai[pdev->id]);
    if (ret != 0) {
        dev_err(&pdev->dev, "failed to get pcm_clock\n");
        ret = -EBUSY;
        goto free_remap;
    }
    dev_set_drvdata(&pdev->dev, pcm);
    pcm->dma_playback = &imapx800_pcm_stereo_out[pdev->id];
    pcm->dma_capture = &imapx800_pcm_stereo_in[pdev->id];
    imapx800_pcm_disable_module(pcm);
    
	return 0;

free_remap:
    iounmap(pcm->regs);
free_clk:
    clk_disable(pcm->extclk);
    clk_put(pcm->extclk);
free_mem:
    release_mem_region(mem->start, resource_size(mem));
    
    return ret;
}

static __devexit int imapx800_pcm_remove(struct platform_device *pdev)
{
    struct imapx800_pcm_info *pcm = &imapx800_pcm[pdev->id];
    struct resource *mem_res;

    snd_soc_unregister_dai(&pdev->dev);

    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(mem_res->start, resource_size(mem_res));
    clk_disable(pcm->extclk);
    clk_put(pcm->extclk);

    return 0;
}

static struct platform_driver imapx800_pcm_driver = {
    .probe  = imapx800_pcm_probe,
    .remove = imapx800_pcm_remove,
    .driver = {
        .name   = "imapx800-pcm",
        .owner  = THIS_MODULE,
    },
};

static int __init imapx800_pcm_init(void)
{
    if (!item_equal("codec.data", "pcm", 0))
        return -1;
    module_power_on(SYSMGR_PCM_BASE);
    return platform_driver_register(&imapx800_pcm_driver);
}
module_init(imapx800_pcm_init);

static void __exit imapx800_pcm_exit(void)
{
    platform_driver_unregister(&imapx800_pcm_driver);
}
module_exit(imapx800_pcm_exit);

MODULE_AUTHOR("SUN");
MODULE_DESCRIPTION("IMAPX800 PCM Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imapx800-pcm");
