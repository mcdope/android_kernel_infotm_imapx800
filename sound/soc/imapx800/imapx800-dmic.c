#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/time.h>
#include <linux/cache.h>
#include <linux/io.h>
#include <mach/items.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/spinlock.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/mfd/tps65910_imapx800.h>
#include "imapx800-dmic.h"
#include "imapx800-dma.h"
#include "imapx800-dmic-soft.h"

//#define DMIC_DEBUG
#ifdef DMIC_DEBUG
#define dmic_dbg(x...) printk(x)
#else
#define dmic_dbg(x...)
#endif

//#define DMIC_TEST  1

static void dmic_soft_struct_free(struct dmic_soft_info *dmic);
static uint32_t dmic_soft_struct_init(struct dmic_soft_info *dmic_soft, uint32_t size);

/* function */
static inline uint32_t __dmic_readl(void __iomem *io_base, uint32_t offset)
{
    return __raw_readl(io_base + offset);
}

static inline void __dmic_writel(void __iomem *io_base, uint32_t offset, uint32_t val)
{
    __raw_writel(val, io_base + offset);
}

/*
 * just clear DMIC_CTL and DMIC_CFG to 0
 */
inline void dmic_clean(void __iomem *io_base)
{
    __dmic_writel(io_base, DMIC_CTL, 0);
	__dmic_writel(io_base, DMIC_CFG, 0);
	__dmic_writel(io_base, DMIC_IRQ_STAT, 0xf);
}

/*
 * mode : ENABLE or DISABLE
 */
inline void dmic_control(void __iomem *io_base, uint32_t mode)
{
    uint32_t val;

	val = __dmic_readl(io_base, DMIC_CTL);
	val &= ~(1 << DMIC_DMIC_ON);
	val |= mode << DMIC_DMIC_ON;
	__dmic_writel(io_base, DMIC_CTL, val);
}

inline void dmic_fifo_control(void __iomem *io_base, uint32_t mode)
{
    uint32_t val;

    val = __dmic_readl(io_base, DMIC_CTL);
    val &= ~(1 << DMIC_FIFO_ENABLE);
    val |= mode << DMIC_FIFO_ENABLE;
     __dmic_writel(io_base, DMIC_CTL, val);
}

/*
 * enable clk
 */
inline void dmic_clk_control(void __iomem *io_base, uint32_t mode)
{
    uint32_t val;

	val = __dmic_readl(io_base, DMIC_CTL);
	val &= ~(1 << DMIC_CLK_ENABLE);
	val |= mode << DMIC_CLK_ENABLE;
	__dmic_writel(io_base, DMIC_CTL, val);
}

inline void dmic_start(void __iomem *io_base)
{
    dmic_fifo_control(io_base, ENABLE);
    dmic_clk_control(io_base, ENABLE);
	dmic_control(io_base, ENABLE);
}

inline void dmic_stop(void __iomem *io_base)
{
    dmic_fifo_control(io_base, DISABLE);
    dmic_control(io_base, DISABLE);
	dmic_clk_control(io_base, DISABLE);
}

static inline void dmic_cfg_config(void __iomem *io_base, uint32_t val)
{
    __dmic_writel(io_base, DMIC_CFG, val);
}

/*
 * @clk_r : clk ratio, that default clk will divider 2 inside itself
 *          it means clk src is 10MHz, and clk_r = 1, and clk out is 10MHz / 4
 * @fifo_dip : 32 - fifo_dip is the level to trigger dma or interrupt
 * @sample_mode : mainly following, rising, and both following and rising
 *                it depends on the out module
 * @fmt : DMIC_DFMT_1 or DMIC_DFMT_16
 *        it mainly uses under both clk mode
 *        DMIC_DFMT_1 means store left data 1bit then right 1bit from low bit to high
 *        DMIC_DFTM_16 means low 16 bit store left data, high 16 bit store right data
 *
 * here default using dma, so set dma_en enable
 */
static inline uint32_t dmic_cfg_val_set(uint32_t clk_r, uint32_t fifo_dip,
			uint32_t sample_mode, uint32_t fmt)
{
    return ((clk_r & 0xff) << DMIC_CLK_RATIO) | ((fifo_dip & 0x1f) << DMIC_FIFO_DIPSTICK ) |
           ((sample_mode & 0x3) << DMIC_SAMPLE_MODE) | ((fmt & 0x1) << DMIC_DIRECT_FMT ) | 
		   (ENABLE << DMIC_DMA_ENABLE);
}

/*
 * dmic inter control
 *
 * it likes dmic cfg val set, only set the tmp val
 * usually, dmic inters are not been used, but, fifo_over and 
 * fifo_under are recommended to use as to remind errors
 */
static inline int dmic_inter_control(uint32_t val, uint32_t irq_en, 
			uint32_t fifo_over, uint32_t fifo_under, uint32_t fifo_afull)
{
	val &= ~((1 << DMIC_IRQ_EN) | (1 << DMIC_FIFO_OVER_IRQ_EN) | (1 << DMIC_FIFO_UNDER_IRQ_EN) |
			(1 << DMIC_FIFO_AFULL_IRQ_EN));
	val |= ((irq_en & 0x1) << DMIC_IRQ_EN) | ((fifo_over & 0x1) << DMIC_FIFO_OVER_IRQ_EN) | 
		   ((fifo_under & 0x1) << DMIC_FIFO_UNDER_IRQ_EN) | ((fifo_afull & 0x1) << DMIC_FIFO_AFULL_IRQ_EN);
	return val;
}

/*
 * dmic init
 */
inline uint32_t dmic_init(void __iomem *io_base, struct dmic_info *dmic)
{
    uint32_t cfg;
	struct dmic_info *p = dmic;

	/* make sure dmic at intial state */
    dmic_clean(io_base);
    /* set cfg */
	cfg = dmic_cfg_val_set(p->clk_ratio, p->fifo_dipstick, (p->state >> DMIC_SAMPLE_MODE_BIT), (p->state >> DMIC_FMT_BIT));
    cfg = dmic_inter_control(cfg, (p->state >> DMIC_IRQ_EN_BIT), (p->state >> DMIC_FIFO_OVER_IRQ_EN_BIT), 
				(p->state >> DMIC_FIFO_UNDER_IRQ_EN_BIT), (p->state >> DMIC_FIFO_AFULL_IRQ_EN_BIT));
    dmic_cfg_config(io_base, cfg);

    return 0;
}

/*************************************************************************
 ************************************************************************/

static struct imapx800_dma_client imapx800_dmic_pdm_in = {
    .name = "imapx800 dmic pdm in",
};

static struct imapx800_pcm_dma_params imapx800_dmic_pdm_stereo_in = {
    .client   = &imapx800_dmic_pdm_in,
	.channel  = IMAPX800_DMIC_RX,
	.dma_addr = IMAP_DMIC_BASE + DMIC_FIFO,
	.dma_size = 4,
//	.brst_len = 16,
};

static int dmic_dai_hw_params(struct snd_pcm_substream *substream, 
			struct snd_pcm_hw_params *params,struct snd_soc_dai *dai)
{
    struct imapx800_pcm_dma_params *dma_data;
	struct imapx_dmic *dmic = snd_soc_dai_get_drvdata(dai);
	void __iomem *io_base = dmic->io_base;

    dmic_dbg("Entered %s\n", __func__);
	if(substream->stream == SNDRV_PCM_STREAM_CAPTURE)
	    dma_data = &imapx800_dmic_pdm_stereo_in;
	else{
        dev_err(dmic->dev, "imapx800 dmic only support capture\n");
		return -EINVAL;
	}

    snd_soc_dai_set_dma_data(dai, substream, dma_data);

    /* TODO 
	 * 
	 * init dmic module others parameters??
	 * as dmic_init is here, whehter calling dmic_dai_hw_params should
	 * aftering using other config dmic_info funtions
	 */
    dmic_init(io_base, dmic->dmic);
    
    return 0;
}

static int dmic_dai_trigger(struct snd_pcm_substream *substream, 
			int cmd, struct snd_soc_dai *dai)
{
   struct imapx_dmic *dmic = snd_soc_dai_get_drvdata(dai);
   void __iomem *io_base = dmic->io_base;
   unsigned long flags;
   int ret = 0;
    
   dmic_dbg("Entered %s, cmd is %d\n", __func__, cmd); 
   if(substream->stream != SNDRV_PCM_STREAM_CAPTURE)
	 return -EINVAL;

   spin_lock_irqsave(&dmic->lock, flags); 
   switch(cmd){
	   case SNDRV_PCM_TRIGGER_START:
	   case SNDRV_PCM_TRIGGER_RESUME:
	   case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		   dmic_start(io_base);
		   break;

	   case SNDRV_PCM_TRIGGER_STOP:
	   case SNDRV_PCM_TRIGGER_SUSPEND:
	   case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		   dmic_stop(io_base);
		   break;
       default:
		   ret = -EINVAL;
		   break;
   }
	spin_unlock_irqrestore(&dmic->lock, flags);
	printk("ret is %d\n", ret);
    return ret;
}

static int dmic_dai_set_sysclk(struct snd_soc_dai *dai, int clk_id,
			unsigned int freq, int dir)
{
    struct imapx_dmic *dmic = snd_soc_dai_get_drvdata(dai);

    dmic_dbg("Entered %s\n", __func__);
    dmic_dbg("channel is %d, freq is %d, format is %d\n",
                        clk_id, freq, dir);

    switch(dir){
        case SNDRV_PCM_FORMAT_S16_LE:
            dmic->dmic->state |= (DMIC_DFMT_16 << DMIC_FMT_BIT);
            break;
        default:
            dev_err(dmic->dev, "imapx800 dmic only support 16bit format: %d\n",
                    dir);
            return -EINVAL;
    }
	dmic->dmic->clk_ratio = dmic->clk_rate / (2 * 64 * freq) - 1;
	printk("freq is %d, clk rate is %d, clk ratio is %d\n", freq, dmic->clk_rate, dmic->dmic->clk_ratio);
    return 0;
}

static int dmic_dai_set_clkdiv(struct snd_soc_dai *dai, int div_id,
			int div)
{
    dmic_dbg("Entered %s\n", __func__);
    return 0;
}

static int dmic_dai_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    dmic_dbg("Entered %s\n", __func__);
	return 0;
}

static struct snd_soc_dai_ops dmic_dai_ops = {
	.hw_params  = dmic_dai_hw_params,
	.trigger    = dmic_dai_trigger,
	.set_sysclk = dmic_dai_set_sysclk,
	.set_clkdiv = dmic_dai_set_clkdiv,
	.set_fmt    = dmic_dai_set_fmt,
};

/*
 * dmic dai sw
 *
 * @ *input : dma buff addr
 * @ *output : pcm buff addr
 * @ size : input data size by byte
 */
uint32_t dmic_dai_sw(uint32_t *input, uint32_t *output, uint32_t size, struct snd_soc_dai *dai)
{
    struct imapx_dmic *dmic = snd_soc_dai_get_drvdata(dai);
    struct dmic_soft_info *dmic_soft = dmic->dmic_soft;
	unsigned long flags;
	dmic_dbg("Entered %s\n", __func__);
	
	if ((u32)input != L1_CACHE_ALIGN((u32)input) || 
            (u32)output != L1_CACHE_ALIGN((u32)output)) {
        printk("input or output addr is not align 32bit\n");
        return -1;
    }

	spin_lock_irqsave(&dmic->lock, flags);
    if(pdm_to_pcm_sw(input, (short *)output, dmic_soft->comp, dmic_soft->halfband1, 
                dmic_soft->halfband2, size, dmic_soft)) {
		spin_unlock_irqrestore(&dmic->lock, flags);
        return -1;
	}
	spin_unlock_irqrestore(&dmic->lock, flags);
	dmic_dbg("Entered %s end\n", __func__);
	return 0;
}
EXPORT_SYMBOL(dmic_dai_sw);


/*
 * Guarantee that the kzalloc'd memory is cacheline aligned.
 */
static void * kzalloc_cacheline_aligned(size_t size, gfp_t flags, void **base)
{
	dmic_dbg("Entered %s\n", __func__);
	/* see if kzalloc will give us cachline aligned memory by default */
	*base = kzalloc(size, flags);
	if (*base == NULL)
		return NULL;

	if ((u32)*base == L1_CACHE_ALIGN((u32)*base))
		return *base;

	kfree(*base);

	/* nope, we'll have to do it ourselves */
	*base = kzalloc(size + L1_CACHE_BYTES, flags);
	if (*base == NULL)
		return NULL;

	return (void *)L1_CACHE_ALIGN((u32)*base);
}


/*
 * @ size : by byte, or, size of half dma ping-pang buf
 */
static uint32_t dmic_soft_struct_init(struct dmic_soft_info *dmic_soft, uint32_t size)
{
    void *comp;
    void *halfband1;
    void *halfband2;
	dmic_dbg("Entered %s\n", __func__);
    comp = kzalloc_cacheline_aligned((size << 1), GFP_KERNEL, &comp);
    halfband1 = kzalloc_cacheline_aligned(size, GFP_KERNEL, &halfband1);
    halfband2 = kzalloc_cacheline_aligned((size >> 1), GFP_KERNEL, &halfband2);
    
	if(comp == NULL){
        printk("dmic comp buf alloc error..........\n");
	    goto _error;
    }
    if(halfband1 == NULL){
        printk("dmic halfband1 buf alloc error..........\n");
 	    goto _error;
    }
    if(halfband2 == NULL){
        printk("dmic halfband2 buf alloc error..........\n");
	    goto _error;
    }
    dmic_soft_init(dmic_soft);
	dmic_soft->comp = comp;
	dmic_soft->halfband1 = halfband1;
	dmic_soft->halfband2 = halfband2;

	return 0;

_error:
	if (halfband2)
		kfree(halfband2);
    if (halfband1)
		kfree(halfband1);
    if (comp)
		kfree(comp);
    return -1;
}

static void dmic_soft_struct_free(struct dmic_soft_info *dmic)
{
	dmic_dbg("Entered %s\n", __func__);
    kfree(dmic->comp);
	kfree(dmic->halfband1);
	kfree(dmic->halfband2);
}

#ifdef DMIC_TEST
static int dmic_dai_probe(struct snd_soc_dai *dai)
{
#define TOTAL_TEST_SIZE    0x500000
#define INPUT_SIZE         16384
	uint32_t i;
	uint32_t n = TOTAL_TEST_SIZE/INPUT_SIZE;
	void *pdm;
    void *pcm;
	void *comp;
	void *halfband1;
	void *halfband2;
    struct dmic_soft_info *dmic;
    struct timeval time_s;
	struct timeval time_e;

	printk("dmic test init\n");

    dmic = kzalloc(sizeof(struct dmic_soft_info), GFP_KERNEL);
    pdm = kzalloc_cacheline_aligned(INPUT_SIZE, GFP_KERNEL, &pdm);
	pcm = kzalloc_cacheline_aligned((TOTAL_TEST_SIZE >> 1), GFP_KERNEL, &pcm);
	comp = kzalloc_cacheline_aligned((INPUT_SIZE << 1), GFP_KERNEL, &comp);
	halfband1 = kzalloc_cacheline_aligned(INPUT_SIZE, GFP_KERNEL, &halfband1);
	halfband2 = kzalloc_cacheline_aligned((INPUT_SIZE >> 1), GFP_KERNEL, &halfband2);

	if(dmic == NULL || pdm == NULL || pcm == NULL || comp == NULL ||
            halfband1 == NULL || halfband2 == NULL){
	    printk("alloc test mem error\n");
        return -ENOMEM;
	}
    printk("pdm addr: %x, pcm addr : %x, comp addr : %x, halfband1 addr : %x, halfband2 addr : %x    \n", 
				pdm, pcm, comp, halfband1, halfband2);

	dmic_soft_init(dmic);

	do_gettimeofday(&time_s);

//	asm("b  .;");
	for(i = 0; i < n; i++){
	    pdm_to_pcm_sw((uint32_t *)pdm,(short *)(pcm + (i * (INPUT_SIZE >> 1))),
					comp, halfband1, halfband2, INPUT_SIZE, dmic);
	}
 
	do_gettimeofday(&time_e);	
	printk("start time:---- %ld.%6.6ld ----\n",time_s.tv_sec, time_s.tv_usec);
	printk(" end  time:---- %ld.%6.6ld ----\n",time_e.tv_sec, time_e.tv_usec);
	printk("test done ...................\n");

	while(1);

    return 0;
}
#else
static int dmic_dai_probe(struct snd_soc_dai *dai)
{
    return 0;
}
#endif

//#define IMAPX800_PCM_RATES  (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |SNDRV_PCM_RATE_22050)
#define IMAPX800_PCM_RATES  SNDRV_PCM_RATE_8000_48000

static struct snd_soc_dai_driver dmic_dai = {
    .name = "imapx800-dmic",
	.probe = dmic_dai_probe,
	.capture = {
	    .stream_name = "Capture",
        .channels_min = 2,
		.channels_max = 2,
		.rates = IMAPX800_PCM_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,         //filter only support 16bit 
	},
	.ops = &dmic_dai_ops,
}; 

static struct dmic_info dmic_info_struct = {
    .clk_ratio     = DMIC_DEFAULT_CLK_R,
	.fifo_dipstick = DMIC_FIFO_SET(DMIC_DEFAULT_FIFO_LEVEL),
	.state         = DMIC_DEFAULT_STATE_VALUE | 
		(DMIC_CLK_HW_CON << DMIC_SAMPLE_MODE_BIT),
};

/*
 * dmic device probe
 */
//struct regulator *regulator;

static int __devinit imapx_dmic_probe(struct platform_device *pdev)
{
    int err = 0;
	int ret;
	struct resource *mem;
    struct imapx_dmic *dmic;
    char buf[ITEM_MAX_LEN];

    module_power_on(SYSMGR_DMIC_BASE);
    __raw_writel(1, IO_ADDRESS(DMIC_IF_EN));
    imapx_pad_cfg(IMAPX_DMIC, 0);

    item_string(buf, "codec.power", 1);
    printk("buf is %s\n", buf);
#if 0
    regulator = regulator_get(&pdev->dev, buf);
    if (IS_ERR(regulator)) {
        printk("%s: get regulator fail\n", __func__);
        return -1;
    }
    ret = regulator_set_voltage(regulator, 3300000, 3300000);
    if(ret) {
        printk("%s: set regulator fail\n", __func__);
        return -1;
    }

    regulator_enable(regulator);
    printk("%s: regulator enable\n", __func__);
#endif
    /* malloc struct dmic */
	dmic = kzalloc(sizeof(struct imapx_dmic), GFP_KERNEL);
	if(dmic == NULL){
	    dev_err(&pdev->dev, "dmic unable to alloc data struct\n");
		return -ENOMEM;
	}
	dmic->dmic_soft = kzalloc(sizeof(struct dmic_soft_info), GFP_KERNEL);
	if(dmic->dmic_soft == NULL){
		dev_err(&pdev->dev, "dmic unable to alloc data struct\n");	
		goto err_alloc;
	}
	ret = dmic_soft_struct_init(dmic->dmic_soft, 0x2000);
	if (ret < 0)
		goto err_alloc;
    dmic->dev = &pdev->dev;
	spin_lock_init(&dmic->lock);
	/* get resouce */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!mem){
		dev_err(&pdev->dev, "dmic no MEM resouce\n");
		err = -ENODEV;
		goto err_alloc;
	}
	if(!request_mem_region(mem->start, resource_size(mem), pdev->name)){
		dev_err(&pdev->dev, "dmic request mem region err\n");
		err = -EBUSY;
		goto err_alloc;
	}

    /* io remap and get io_base */
	dmic->io_base = ioremap(mem->start, resource_size(mem));
	if(!dmic->io_base){
		dev_err(&pdev->dev, "dmic ioremap error\n");
		err = -ENOMEM;
		goto err_alloc;
	}

	/*  initializing clock */
    dmic->clk = clk_get(&pdev->dev, "dmic");
	if(IS_ERR(dmic->clk)){
		dev_err(&pdev->dev, "dmic no clk info\n");
		err = -ENODEV; 
		goto err_iomap;
	}
    clk_enable(dmic->clk);
    dmic->clk_rate = clk_get_rate(dmic->clk);
	printk("clk rate is %d\n", dmic->clk_rate);
	/* register dai dmic */
	ret = snd_soc_register_dai(&pdev->dev, &dmic_dai);
	if(ret){
	    dev_err(&pdev->dev, "dmic dai register error\n");
		err = ret;
        goto err_clk;
	}
    dev_set_drvdata(&pdev->dev, dmic);
    dmic->dmic = &dmic_info_struct;

    return 0;

err_clk:
    clk_disable(dmic->clk);
    clk_put(dmic->clk);
err_iomap:
	iounmap(dmic->io_base);
err_alloc:
	if (dmic->dmic_soft)
	  kfree(dmic->dmic_soft);
	kfree(dmic);

	return err;
}

static int __devexit imapx_dmic_remove(struct platform_device *pdev)
{
    struct imapx_dmic *dmic =  dev_get_drvdata(&pdev->dev);
    struct resource *mem_res;

    snd_soc_unregister_dai(&pdev->dev);
    
    //regulator_put(regulator);    
    clk_disable(dmic->clk);
	clk_put(dmic->clk);

    iounmap(dmic->io_base);
    mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(mem_res->start, resource_size(mem_res));
	
	dmic_soft_struct_free(dmic->dmic_soft);
	kfree(dmic->dmic_soft);
	kfree(dmic);

    return 0;
}

static int imapx_dmic_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static void imapx_dmic_resume(struct platform_device *pdev)
{
    module_power_on(SYSMGR_DMIC_BASE);
    __raw_writel(1, IO_ADDRESS(DMIC_IF_EN));
    imapx_pad_cfg(IMAPX_DMIC, 0);
}

static struct platform_driver imapx_dmic_driver = {
	.probe     =  imapx_dmic_probe,
	.remove    =  imapx_dmic_remove,
    .suspend   =  imapx_dmic_suspend,
    .resume    =  imapx_dmic_resume,
	.driver    = {
		.name  = "imap-dmic",
		.owner = THIS_MODULE,
	},
};

static int __init imapx_dmic_module_init(void)
{
    if (!item_equal("codec.capture", "dmic", 0))
        return -1;
    return platform_driver_register(&imapx_dmic_driver);
}


static void __exit imapx_dmic_module_exit(void)
{
    platform_driver_unregister(&imapx_dmic_driver);
}

module_init(imapx_dmic_module_init);
module_exit(imapx_dmic_module_exit);

MODULE_AUTHOR("Larry Liu");
MODULE_DESCRIPTION("imapx800 dmic");
MODULE_LICENSE("GPL");
