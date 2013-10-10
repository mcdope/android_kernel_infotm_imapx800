#
# Makefile for the kernel multimedia device drivers.
#

include drivers/InfotmMedia/env.mk

EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/dmmu_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/pmm_lib/include

obj-$(CONFIG_INFOTM_MEDIA_PMM_I800_SUPPORT) += pmm_lib.o
obj-$(CONFIG_INFOTM_MEDIA_PMM_I800_SUPPORT) += pmm_pwl_linux.o

#EXTRA_CFLAGS += -DPMM_RESERVE_MEM_BASE=0x40000000 -DPMM_RESERVE_MEM_SIZE=0x0 -DPMM_RESERVE_MEM_THRESHOLD=0x40000

#EXTRA_CFLAGS += -DIM_DEBUG

