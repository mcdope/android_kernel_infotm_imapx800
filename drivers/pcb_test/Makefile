#
# Makefile of soops.
#
# InfoTM all rights reserved.
#
# Sololz <sololz.luo@gmail.com>.
#
#
include drivers/InfotmMedia/env.mk
EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/ids_lib_i800/include/
EXTRA_CFLAGS += -Idrivers/InfotmMedia/hdmi_api/include/
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camisp_api/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camisp_api/source
EXTRA_CFLAGS += -Idrivers/InfotmMedia/isp_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/isp_lib_i800/source
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camsen_lib/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camif_api/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camif_api/source
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camif_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/camif_lib_i800/source
EXTRA_CFLAGS += -Idrivers/InfotmMedia/pmm_lib/include
#EXTRA_CFLAGS += -DIM_DEBUG

obj-$(CONFIG_PCB_TEST) += hdmi_pcbtest.o
obj-$(CONFIG_PCB_TEST) += camera_pcbtest.o


obj-$(CONFIG_PCB_TEST) += pcb_test.o
obj-$(CONFIG_PCB_TEST) += eth_pcbtest.o
obj-$(CONFIG_PCB_TEST) += audio_pcbtest.o
obj-$(CONFIG_PCB_TEST) += bat_pcbtest.o
