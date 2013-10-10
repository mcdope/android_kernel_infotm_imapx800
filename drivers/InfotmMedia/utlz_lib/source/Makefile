#
# Makefile for the kernel multimedia device drivers.
#

include drivers/InfotmMedia/env.mk

EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/utlz_lib/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/utlz_lib/source

obj-$(CONFIG_INFOTM_MEDIA_UTLZ_SUPPORT) += utlz_lib.o
obj-$(CONFIG_INFOTM_MEDIA_UTLZ_SUPPORT) += utlz_pwl_linux.o

#EXTRA_CFLAGS += -DIM_DEBUG

