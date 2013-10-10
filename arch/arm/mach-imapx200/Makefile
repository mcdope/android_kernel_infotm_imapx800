#
#Makefile for linux kernel
#
#


#IMAPX200 support files

obj-$(CONFIG_CPU_IMAPX200)		+= irq.o clock.o time.o devices.o pwm.o




#imdkx200 machine specific support
obj-$(CONFIG_MACH_IMAPX200)		+=cpu.o 
obj-$(CONFIG_MACH_IMAPX200)		+=hwid.o 
#obj-$(CONFIG_MACH_IMAPX200)		+=dma.o 
obj-$(CONFIG_MACH_IMAPX200)		+=dmaif_linux.o 
obj-$(CONFIG_MACH_IMAPX200)		+=dmalib.o 
obj-$(CONFIG_PM)                += pm_imap.o
obj-$(CONFIG_PM)            += pm_imapx200.o
obj-$(CONFIG_PM)                += sleep.o

obj-y		+= gpio/
