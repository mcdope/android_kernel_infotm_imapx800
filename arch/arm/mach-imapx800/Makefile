#
#Makefile for linux kernel
#
#


#IMAPX800 support files

obj-$(CONFIG_ARCH_IMAPX800)  += clock.o clock-ops.o \
    				timer-ca5.o timer-cmn.o \
    				pwm.o pad.o power-gate.o mem-reserve.o \
				devices.o usb-phy.o 
obj-$(CONFIG_PM) += pm.o sleep.o
obj-$(CONFIG_PL330_DMA) += dma-ops.o

#imdkx800 machine specific support
obj-$(CONFIG_MACH_IMAPX800)  += cpu.o 
obj-$(CONFIG_MACH_IMAPX800)  += items.o 
obj-$(CONFIG_SMP)            += platsmp.o headsmp.o
obj-$(CONFIG_HOTPLUG_CPU)    += hotplug.o

