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

#include "../imapx800/imapx800-pwma.h"

#define IMAPX800_PWMA_RATES  SNDRV_PCM_RATE_8000_48000 | SNDRV_PCM_RATE_128000 \
	| SNDRV_PCM_RATE_132300


static struct snd_soc_dai_driver null_dai[] = {
    {
		.name = "virtual-hifi",
		.playback = {
			.stream_name = "virtual Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = IMAPX800_PWMA_RATES,
			.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE,},
	},{
		.name = "virtual-hp",
		.capture = {
			.stream_name = "virtual Capture",
			.channels_min = 2,
			.channels_max = 2,
			//.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050,
            .rates = SNDRV_PCM_RATE_8000_48000,
            .formats = SNDRV_PCM_FMTBIT_S16_LE,},
	},
    {
        .name = "virtual-spdif",
        .playback = {
            .stream_name = "Spdif Playback",
            .channels_min = 2,
            .channels_max = 2,
            .rates = SNDRV_PCM_RATE_8000_192000,
            .formats = SNDRV_PCM_FMTBIT_S16_LE |  SNDRV_PCM_FMTBIT_S24_LE,},
    }
};

static struct snd_soc_codec_driver imapnull = {

};

static __devinit int imapnull_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev,
				&imapnull, null_dai, ARRAY_SIZE(null_dai));
}

static int __devexit imapnull_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

struct platform_driver imapnull_driver = {
	.driver = {
		.name = "virtual-codec",
		.owner = THIS_MODULE,
	},
	.probe = imapnull_probe,
	.remove = __devexit_p(imapnull_remove),
};
EXPORT_SYMBOL(imapnull_driver);
