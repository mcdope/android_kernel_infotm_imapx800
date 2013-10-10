#
# Makefile for the imap framebuffer driver
#

ifeq ($(CONFIG_FB_IMAP),y)
include drivers/InfotmMedia/env.mk
obj-y	+= imapfb.o
obj-y	+= isi.o
obj-$(CONFIG_IG_LOGO_ALT)	+= logos/

EXTRA_CFLAGS += -Idrivers/InfotmMedia/external/kernel/linux_3_0/ids_drv_i800/
EXTRA_CFLAGS += -Idrivers/InfotmMedia/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/ids_lib_i800/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/ids_lib_i800/source
EXTRA_CFLAGS += -Idrivers/InfotmMedia/lcd_api/include
EXTRA_CFLAGS += -Idrivers/InfotmMedia/lcd_lib_i800/include
endif
