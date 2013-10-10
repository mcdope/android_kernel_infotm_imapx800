#
# Makefile for the kernel multimedia device drivers.
#

media-objs	:= media-device.o media-devnode.o media-entity.o

ifeq ($(CONFIG_MEDIA_CONTROLLER),y)
  obj-$(CONFIG_MEDIA_SUPPORT) += media.o
endif

obj-y += common/ rc/ video/

obj-$(CONFIG_VIDEO_DEV) += radio/
obj-$(CONFIG_DVB_CORE)  += dvb/
obj-$(CONFIG_IMAP_DECODE)	+= decode/
obj-$(CONFIG_IMAP_ENCODE)	+= encode/
obj-$(CONFIG_IMAP_CAMIF)        += camif/

# Added by Sololz.
obj-$(CONFIG_MACH_IMAPX200) += imapx/
