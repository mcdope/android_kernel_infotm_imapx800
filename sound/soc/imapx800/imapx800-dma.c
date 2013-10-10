/********************************************************************************
** sound/soc/imapx800/imapx800-pcm.c 
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
**     1.0  12/21/2009    James Xu
********************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <mach/items.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>
#include <linux/dmaengine.h>
#include <mach/power-gate.h>
#include <asm/dma.h>
#include <mach/hardware.h>
#ifdef imapx800_pcm_debug
#define pr_dbg(fmt...) printk(fmt)
#else
#define pr_dbg(fmt...) 
#endif
#include "imapx800-dma.h"
int sound_dma_irq = 0;
int sound_capture_irq = 0;
EXPORT_SYMBOL(sound_dma_irq);
int sound_dma_irq_index = 0;
EXPORT_SYMBOL(sound_dma_irq_index);
int sound_capture_irq_index = 0;
EXPORT_SYMBOL(sound_capture_irq_index);

static void imapx800_pcm_enqueue(struct snd_pcm_substream *substream);
extern void *imapx800_dmadev_get_ops(void);

static const struct snd_pcm_hardware imapx800_pcm_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED |
				    SNDRV_PCM_INFO_BLOCK_TRANSFER |
				    SNDRV_PCM_INFO_MMAP |
				    SNDRV_PCM_INFO_MMAP_VALID |
				    SNDRV_PCM_INFO_PAUSE |
				    SNDRV_PCM_INFO_RESUME,
	.formats		= SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_U16_LE |
				    SNDRV_PCM_FMTBIT_U8 |
				    SNDRV_PCM_FMTBIT_S8,
	.channels_min		= 2,
	.channels_max		= 2,
	.buffer_bytes_max	= PAGE_SIZE*3,
	.period_bytes_min	= PAGE_SIZE,
	.period_bytes_max	= PAGE_SIZE,
	.periods_min		= 2,
	.periods_max		= 255,
	.fifo_size		= 64,
};

extern unsigned int dma_flags;
struct imapx800_runtime_data {
	spinlock_t lock;
	int state;
	unsigned int dma_loaded;
	unsigned int dma_limit;
	unsigned int dma_period;
	dma_addr_t dma_start;
	dma_addr_t dma_pos;
	dma_addr_t dma_end;
	struct imapx800_pcm_dma_params *params;
};

int dmic_exist = 0;
#define DMIC_DMA_SIZE		0x4000
extern uint32_t dmic_dai_sw(uint32_t *input, uint32_t *output, 
			uint32_t size, struct snd_soc_dai *dai);
struct snd_dma_buffer dmic;
int dmic_flag = 0;
/********************************************************************/
#ifdef TIME_DEBUG
static long long last1 = 0;
static long long __getns(void)
{
	ktime_t a;
	a = ktime_get_boottime();
	return a.tv64;
}
#endif
static void imapx800_audio_buffdone(void *data)
{
    struct snd_pcm_substream *substream = data;
    struct imapx800_runtime_data *prtd = substream->runtime->private_data;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    uint32_t *input, *output;
#ifdef TIME_DEBUG
	long long start1; 
	long long tmp;
#endif
    //pr_dbg("Entered %s\n", __func__);
    if (prtd->state & ST_RUNNING) {
        prtd->dma_pos += prtd->dma_period;
        if (prtd->dma_pos >= prtd->dma_end)
            prtd->dma_pos = prtd->dma_start;
        
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
            //sound_dma_irq = 1; /*why need? remove by ayakashi*/
            snd_pcm_period_elapsed(substream);
#ifdef TIME_DEBUG
			start1 = __getns();
			tmp = start1 - last1;
			do_div(tmp, 1000);
			//if(tmp > 22000)
			//printk(" %lld \n", tmp);
			last1 = start1;
#endif
        }
        if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
            //sound_capture_irq = 1;
            if (dmic_exist) {
                if (prtd->params->channel == IMAPX800_DMIC_RX) {
                    spin_lock(&prtd->lock);
                    input = (uint32_t *)((dmic_flag % 2)? (dmic.area + prtd->dma_period):
                            dmic.area);
                    output = (uint32_t *)((dmic_flag % 2)?(substream->dma_buffer.area + PAGE_SIZE):
                            substream->dma_buffer.area);
                    //printk("input is %x, output is %x, size is %d\n", input, output, prtd->dma_period);
                    dmic_dai_sw(input, output, prtd->dma_period, cpu_dai);
                    dmic_flag++;
                    if (dmic_flag == 2) 
                        dmic_flag = 0;
                    spin_unlock(&prtd->lock);
                }
            }
            snd_pcm_period_elapsed(substream);
        }
    }

}

/* imapx800_pcm_enqueue
 *
 * place a dma buffer onto the queue for the dma system
 * to handle.
*/
static void imapx800_pcm_enqueue(struct snd_pcm_substream *substream)
{
	struct imapx800_runtime_data *prtd = substream->runtime->private_data;
	dma_addr_t pos = prtd->dma_pos;
	int limit;
	struct imapx800_dma_prep_info dma_info;

	pr_dbg("Entered %s\n", __func__);
    
    limit = (prtd->dma_end - prtd->dma_start) / prtd->dma_period;
    
    pr_debug("%s: loaded %d, limit %d\n",
            __func__, prtd->dma_loaded, limit);

#if 0    
    dma_info.cap = DMA_CYCLIC;
    dma_info.direction =                                                 
            (substream->stream == SNDRV_PCM_STREAM_PLAYBACK                  
                 ? DMA_TO_DEVICE : DMA_FROM_DEVICE);                              
    dma_info.fp = imapx800_audio_buffdone;                                        
    dma_info.fp_param = substream;                                       
    dma_info.period = prtd->dma_period;                                  
    dma_info.len = prtd->dma_period;                               
                                                                         

    while (prtd->dma_loaded < limit) {
        pr_dbg("dma_loaded: %d\n", prtd->dma_loaded);
        if ((pos + dma_info.period) > prtd->dma_end) {
            dma_info.period  = prtd->dma_end - pos;
            pr_dbg(KERN_DEBUG "%s: corrected dma len %ld\n",
                    __func__, dma_info.period);
        }
        dma_info.buf = pos;
        prtd->params->ops->prepare(prtd->params->ch, &dma_info); 

        prtd->dma_loaded++;     
        pos += prtd->dma_period;
        if (pos >= prtd->dma_end)
            pos = prtd->dma_start;
    }

    prtd->dma_pos = pos;
#else
    dma_info.cap = DMA_LLI;
    dma_info.direction =                                                 
            (substream->stream == SNDRV_PCM_STREAM_PLAYBACK                  
                 ? DMA_TO_DEVICE : DMA_FROM_DEVICE);                              
    dma_info.fp = imapx800_audio_buffdone;                                        
    dma_info.fp_param = substream;                                       
    dma_info.period = prtd->dma_period; 
    dma_info.len = prtd->dma_period*limit;                               
    dma_info.buf = pos;

    prtd->params->ops->prepare(prtd->params->ch, &dma_info); 

    prtd->dma_pos = pos;
#endif    
}

static int imapx800_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx800_runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct imapx800_pcm_dma_params *dma; 
	unsigned long totbytes = params_buffer_bytes(params);
    struct imapx800_dma_info dma_info;
	
    pr_debug("Entered %s\n", __func__);

	dma = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!dma)
		return 0;
	/* this may get called several times by oss emulation
     * with different params -HW */
    if (prtd->params == NULL) {
        /* prepare DMA */
        prtd->params = dma;

        pr_dbg("params %p, client %p, channel %d\n", prtd->params,
                prtd->params->client, prtd->params->channel);

        prtd->params->ops = imapx800_dmadev_get_ops();

        dma_info.cap = DMA_CYCLIC;
        dma_info.client = prtd->params->client;
        dma_info.direction = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK?
                DMA_TO_DEVICE : DMA_FROM_DEVICE);
        dma_info.width = prtd->params->dma_size;
        dma_info.fifo = prtd->params->dma_addr; 
        prtd->params->ch = prtd->params->ops->request(
                prtd->params->channel, &dma_info);    

    }

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = totbytes;

	spin_lock_irq(&prtd->lock);
	prtd->dma_loaded = 0;
	prtd->dma_limit = runtime->hw.periods_min;

    if (dmic_exist) {
        if (prtd->params->channel == IMAPX800_DMIC_RX) {
            prtd->dma_start = dmic.addr;
            prtd->dma_period = DMIC_DMA_SIZE / 2;
            prtd->dma_end = prtd->dma_start + DMIC_DMA_SIZE;
        } else {
            prtd->dma_start = runtime->dma_addr;
            prtd->dma_period = params_period_bytes(params);
            prtd->dma_end = prtd->dma_start + totbytes;
        }
    } else {
        prtd->dma_start = runtime->dma_addr;
        prtd->dma_period = params_period_bytes(params);
        prtd->dma_end = prtd->dma_start + totbytes;
    }
	prtd->dma_pos = prtd->dma_start;
	spin_unlock_irq(&prtd->lock);

	pr_dbg("pcm_hw_params_5!\n\n");	
	return 0;
}

static int imapx800_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct imapx800_runtime_data *prtd = substream->runtime->private_data;

	pr_dbg("Entered %s\n", __func__);

	/* TODO - do we need to ensure DMA flushed */
	snd_pcm_set_runtime_buffer(substream, NULL);

	if (prtd->params) {
        prtd->params->ops->flush(prtd->params->ch); 
        prtd->params->ops->release(prtd->params->ch,
                prtd->params->client);          
		prtd->params = NULL;
	}

	return 0;
}

static int imapx800_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct imapx800_runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!prtd->params)
		return 0;

	/* flush the DMA channel */
    prtd->params->ops->flush(prtd->params->ch);

	prtd->dma_loaded = 0;
	prtd->dma_pos = prtd->dma_start;

	/* enqueue dma buffers */
    imapx800_pcm_enqueue(substream);
	return ret;
}

/*merge it from i2s for audio ctrl*/
extern int imapx_gpio_spken(int en) ;
extern int imapx800_force_closeSpk(void);
static int AudioRunStatus=0;
int imapx_audioRunStatus(void)
{
    printk(KERN_DEBUG"audios Run status %d\n",AudioRunStatus);
    return AudioRunStatus;  
}
EXPORT_SYMBOL(imapx_audioRunStatus);

int imapx_set_audioRunStatus(void)
{
    AudioRunStatus=1;
}
EXPORT_SYMBOL(imapx_set_audioRunStatus);
extern int rt5631_spk_switch_Delay_mute(int dir);
extern int rt5631_switch_spk(int dir);
extern int imapx800_switch_spk(int dir);


static int imapx800_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct imapx800_runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;

	pr_debug("Entered %s, cmd = %x", __func__, cmd);

	spin_lock(&prtd->lock);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        prtd->state |= ST_RUNNING;

		/**added by ayakashi @20130117
		**when resume, when must prepare the dma description again
		**because the description will be delete by dma flush(suspend). 
		**Some Registers are wrong when alc5631  or es8328 resume. 
		**TODO:make codec registers restore correctly  when resume*/
		if(cmd == SNDRV_PCM_TRIGGER_RESUME)
			imapx800_pcm_prepare(substream);

        if (dmic_exist) {
            dmic_flag = 0;
        }
        prtd->params->ops->trigger(prtd->params->ch);

        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        {
            AudioRunStatus=1;
            if (!imapx800_force_closeSpk())
            {
                imapx_gpio_spken(1);
                rt5631_spk_switch_Delay_mute(1); 
            }
            printk(KERN_DEBUG"AAAA--en spk\n");
       	}
		break;

	case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        prtd->state &= ~ST_RUNNING;
        prtd->params->ops->stop(prtd->params->ch);
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) 
        {
            AudioRunStatus=0;
            if(!imapx800_force_closeSpk())
                imapx_gpio_spken(0);
            printk(KERN_DEBUG"AAAB--dis spk\n");
            rt5631_spk_switch_Delay_mute(0);            
        }
		break;

	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock(&prtd->lock);

	return ret;
}

static snd_pcm_uframes_t
imapx800_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx800_runtime_data *prtd = runtime->private_data;
	unsigned long res;
	int ret;
	dma_addr_t src;
	dma_addr_t dst;

	//pr_dbg("Entered %s\n", __func__);

	spin_lock(&prtd->lock);

	ret = prtd->params->ops->dma_getposition(prtd->params->ch, &src, &dst);
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		//res = prtd->dma_pos - prtd->dma_start;
		res = dst - prtd->dma_start;
		res  &=  ~(PAGE_SIZE - 1);
		
        if (dmic_exist) {
            if (sound_capture_irq == 1) {
                sound_capture_irq = 0;
                if (res >= (DMIC_DMA_SIZE / 2)){
                    sound_capture_irq_index = 2;
                    res = 0x1000;
                }
                else {
                    sound_capture_irq_index = 1;
                    res = 0x00;
                }
            }else {

                if (sound_capture_irq_index ==2)
                    res = 0x1000;
                else if (sound_capture_irq_index ==1)
                    res = 0x0;
                else
                    res = 0;

            }
		}
	}
	else{
		res = src - prtd->dma_start;
		res &= ~(PAGE_SIZE - 1);
		pr_dbg("res = %d \n", res);
	}
	spin_unlock(&prtd->lock);
	//pr_dbg("Pointer %x %x, dma is %x\n", src, dst, prtd->dma_start);

//	res = 0x2000;
/*
	if (dma_flags == 1)
		res = 0x1000;
	else
		res = src - prtd->dma_start;
	spin_unlock(&prtd->lock);
*/
	/* we seem to be getting the odd error from the pcm library due
	 * to out-of-bounds pointers. this is maybe due to the dma engine
	 * not having loaded the new values for the channel before being
	 * callled... (todo - fix )
	 */

	if (res >= snd_pcm_lib_buffer_bytes(substream)) {
		if (res == snd_pcm_lib_buffer_bytes(substream))
			res = 0;
	}

	return bytes_to_frames(substream->runtime, res);
}

static int imapx800_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx800_runtime_data *prtd;

	pr_dbg("Entered %s\n", __func__);

	snd_soc_set_runtime_hwparams(substream, &imapx800_pcm_hardware);

	prtd = kzalloc(sizeof(struct imapx800_runtime_data), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	spin_lock_init(&prtd->lock);

	runtime->private_data = prtd;
	return 0;
}

static int imapx800_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx800_runtime_data *prtd = runtime->private_data;

	pr_dbg("Entered %s\n", __func__);

	if (!prtd)
		pr_dbg("imapx800_pcm_close called with prtd == NULL\n");

	kfree(prtd);

	return 0;
}

static int imapx800_pcm_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	pr_dbg("Entered %s\n", __func__);
	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
				     runtime->dma_area,
				     runtime->dma_addr,
				     runtime->dma_bytes);
}

static struct snd_pcm_ops imapx800_pcm_ops = {
	.open		= imapx800_pcm_open,
	.close		= imapx800_pcm_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= imapx800_pcm_hw_params,
	.hw_free	= imapx800_pcm_hw_free,
	.prepare	= imapx800_pcm_prepare,
	.trigger	= imapx800_pcm_trigger,
	.pointer	= imapx800_pcm_pointer,
	.mmap		= imapx800_pcm_mmap,
};

static int imapx800_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = imapx800_pcm_hardware.buffer_bytes_max;

	pr_dbg("Entered %s\n", __func__);

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_writecombine(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;
    if (dmic_exist) {
        if (dmic.area == NULL) {
            dmic.dev.type = SNDRV_DMA_TYPE_DEV;
            dmic.dev.dev = pcm->card->dev;
            dmic.area = dma_alloc_writecombine(pcm->card->dev, DMIC_DMA_SIZE,
                    &dmic.addr, GFP_KERNEL);
            if (!dmic.area) {
                pr_dbg("can not alloc mem for dmic pcm\n");
                return -ENOMEM;
            }
            dmic.bytes = DMIC_DMA_SIZE;
        }
    }
	return 0;
}

static void imapx800_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	pr_dbg("Entered %s\n", __func__);

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
    if (dmic_exist) {
		dma_free_writecombine(pcm->card->dev, dmic.bytes,
					dmic.area, dmic.addr);
		dmic.area = NULL;
    }
}

static u64 imapx800_pcm_dmamask = DMA_BIT_MASK(32);

static int imapx800_pcm_new(struct snd_card *card,
	struct snd_soc_dai *dai, struct snd_pcm *pcm)
{
	int ret = 0;

	pr_dbg("Entered %s\n", __func__);

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &imapx800_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;

	if (dai->driver->playback.channels_min) {
		ret = imapx800_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (dai->driver->capture.channels_min) {
		ret = imapx800_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}
out:
	return ret;
}

static struct snd_soc_platform_driver imapx800_soc_platform = {
	.ops 	= &imapx800_pcm_ops,
	.pcm_new	= imapx800_pcm_new,
	.pcm_free	= imapx800_pcm_free_dma_buffers,
};

static int __devinit imapx800_asoc_platform_probe(struct platform_device *pdev)
{
    if (item_equal("codec.capture", "dmic", 0))
        dmic_exist = 1;
    return snd_soc_register_platform(&pdev->dev, &imapx800_soc_platform);
}


static int __devexit imapx800_asoc_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver asoc_dma_driver = {
    .driver = {
        .name = "imapx800-audio",
        .owner = THIS_MODULE,
    },

    .probe = imapx800_asoc_platform_probe,
    .remove = __devexit_p(imapx800_asoc_platform_remove),
};

static int __init imapx800_asoc_init(void)
{
    return platform_driver_register(&asoc_dma_driver);
}
module_init(imapx800_asoc_init);

static void __exit imapx800_asoc_exit(void)
{
    platform_driver_unregister(&asoc_dma_driver);
}
module_exit(imapx800_asoc_exit);

MODULE_AUTHOR("James xu, <james_xu@infotmic.com.cn>");
MODULE_DESCRIPTION("INFOTM imapx800 PCM DMA module");
MODULE_LICENSE("GPL");
