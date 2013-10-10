
ifeq ($(CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT), y)
include drivers/InfotmMedia/env.mk
EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/ids_lib_i800/include/
EXTRA_CFLAGS += -Idrivers/InfotmMedia/hdmi_api/include/
endif

snd-soc-imapx800-dma-objs := imapx800-dma.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-dma.o

snd-soc-imapx800-i2s-objs := imapx800-i2s.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-i2s.o

snd-soc-imapx800-pcm-objs := imapx800-pcm.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-pcm.o

snd-soc-imapx800-es8328-objs := imapx800_es8328.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-es8328.o

snd-soc-imapx800-es8323-objs := imapx800_es8323.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-es8323.o
snd-soc-imapx800-alc5631-objs := imapx800_alc5631.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-alc5631.o


snd-soc-imapx800-dmic-objs := imapx800-dmic.o imapx800-dmic-soft.o cic_filter.o fir_filter.o imapx800-pwma.o imapx800-snd.o
obj-$(CONFIG_SND_IMAPX800_SOC) += snd-soc-imapx800-dmic.o

snd-soc-imapx800-spdif-objs := imapx800-spdif.o imapx800_snd_spdif.o
obj-$(CONFIG_SND_IMAPX800_SOC)	+= snd-soc-imapx800-spdif.o

snd-soc-imapx800-item-mode-objs := imapx800-item-mode.o
obj-$(CONFIG_SND_IMAPX800_SOC)	+= imapx800-item-mode.o
