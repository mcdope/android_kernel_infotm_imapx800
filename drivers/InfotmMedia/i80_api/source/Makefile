#
# Makefile InfotmMeida ids i800 driver.
#
# Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
#
#

include drivers/InfotmMedia/env.mk

obj-$(CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT) += i80_api.o
obj-$(CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT) += i80_HX834724CO_1024x600.o
obj-$(CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT) += i80_ILI9320_800x480.o

EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/ids_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/i80_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/i80_api/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/i80_api/source


EXTRA_CFLAGS += -DIM_DEBUG

