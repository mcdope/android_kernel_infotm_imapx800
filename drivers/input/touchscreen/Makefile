#
# Makefile for the touchscreen drivers.
#

# Each configuration option enables a list of files.

wm97xx-ts-y := wm97xx-core.o

obj-$(CONFIG_TOUCHSCREEN_88PM860X)	+= 88pm860x-ts.o
obj-$(CONFIG_TOUCHSCREEN_AD7877)	+= ad7877.o
obj-$(CONFIG_TOUCHSCREEN_AD7879)	+= ad7879.o
obj-$(CONFIG_TOUCHSCREEN_AD7879_I2C)	+= ad7879-i2c.o
obj-$(CONFIG_TOUCHSCREEN_AD7879_SPI)	+= ad7879-spi.o
#obj-$(CONFIG_TOUCHSCREEN_ADS7846)	+= ads7846.o

# touchscreen
obj-$(CONFIG_TP_TI2046)			+= ads7846.o
obj-$(CONFIG_TP_UOR6150)		+= uor6150.o
obj-$(CONFIG_TP_UOR6153)		+= uor6153.o
obj-$(CONFIG_TP_EGL7200)		+= egalax7200.o
obj-$(CONFIG_TP_ITE7260)		+= ite7260.o
obj-$(CONFIG_TP_UC6511)			+= uc6511.o
obj-$(CONFIG_CTP_MSG2032)		+= msg2032.o
obj-$(CONFIG_CTP_EP0700)		+= ep0700m01.o
obj-$(CONFIG_TP_SX8651)			+= sx8651.o
obj-$(CONFIG_CTP_EP0700)		+= ep0700m01.o
obj-$(CONFIG_TP_SX8651)			+= sx8651.o
obj-$(CONFIG_CTP_CT36X)			+= ct36x_i2c_ts.o
obj-$(CONFIG_CTP_FT5304)                += ft5304.o
obj-$(CONFIG_CTP_FT5X0X)                += ft5x0x.o
obj-$(CONFIG_CTP_PIXCIR)                += pixcir_i2c_ts.o
obj-$(CONFIG_CTP_FT5316)				+= ft5316.o
obj-$(CONFIG_CTP_GT828)                 += gt82x.o
obj-$(CONFIG_CTP_GT8105)                += gt81x.o
obj-$(CONFIG_CTP_GT828AVM)	      += gt82x_avm.o

#Added by Ricky
obj-$(CONFIG_CTP_GSL2682)		+= gsl2682.o
obj-$(CONFIG_CTP_GSLX680)                 += gslX680.o gslX680E_COB.o gslX680_ogs.o gslX680E_COB_l.o gslX1680e.o
obj-$(CONFIG_CTP_GSLX680_L)                 += gslX680_l.o
obj-$(CONFIG_CTP_GSLX680M75D)             += gslX680_m75d.o
obj-$(CONFIG_CTP_GSLX680M746)             += gslX680_m746.o
obj-$(CONFIG_CTP_GSLX680GDQ8)             += gslX680_GDQ8.o

obj-$(CONFIG_CTP_GT801)                 += gt801.o
obj-$(CONFIG_TOUCHSCREEN_ATMEL_MXT)	+= atmel_mxt_ts.o
obj-$(CONFIG_TOUCHSCREEN_ATMEL_TSADCC)	+= atmel_tsadcc.o
obj-$(CONFIG_TOUCHSCREEN_BITSY)		+= h3600_ts_input.o
obj-$(CONFIG_TOUCHSCREEN_BU21013)       += bu21013_ts.o
obj-$(CONFIG_TOUCHSCREEN_CY8CTMG110)	+= cy8ctmg110_ts.o
obj-$(CONFIG_TOUCHSCREEN_DA9034)	+= da9034-ts.o
obj-$(CONFIG_TOUCHSCREEN_DYNAPRO)	+= dynapro.o
obj-$(CONFIG_TOUCHSCREEN_HAMPSHIRE)	+= hampshire.o
obj-$(CONFIG_TOUCHSCREEN_GUNZE)		+= gunze.o
obj-$(CONFIG_TOUCHSCREEN_EETI)		+= eeti_ts.o
obj-$(CONFIG_TOUCHSCREEN_ELO)		+= elo.o
obj-$(CONFIG_TOUCHSCREEN_FUJITSU)	+= fujitsu_ts.o
obj-$(CONFIG_TOUCHSCREEN_INEXIO)	+= inexio.o
obj-$(CONFIG_TOUCHSCREEN_INTEL_MID)	+= intel-mid-touch.o
obj-$(CONFIG_TOUCHSCREEN_LPC32XX)	+= lpc32xx_ts.o
obj-$(CONFIG_TOUCHSCREEN_MAX11801)	+= max11801_ts.o
obj-$(CONFIG_TOUCHSCREEN_MC13783)	+= mc13783_ts.o
obj-$(CONFIG_TOUCHSCREEN_MCS5000)	+= mcs5000_ts.o
obj-$(CONFIG_TOUCHSCREEN_MIGOR)		+= migor_ts.o
obj-$(CONFIG_TOUCHSCREEN_MTOUCH)	+= mtouch.o
obj-$(CONFIG_TOUCHSCREEN_MK712)		+= mk712.o
obj-$(CONFIG_TOUCHSCREEN_HP600)		+= hp680_ts_input.o
obj-$(CONFIG_TOUCHSCREEN_HP7XX)		+= jornada720_ts.o
obj-$(CONFIG_TOUCHSCREEN_HTCPEN)	+= htcpen.o
obj-$(CONFIG_TOUCHSCREEN_USB_COMPOSITE)	+= usbtouchscreen.o
obj-$(CONFIG_TOUCHSCREEN_PCAP)		+= pcap_ts.o
obj-$(CONFIG_TOUCHSCREEN_PENMOUNT)	+= penmount.o
obj-$(CONFIG_TOUCHSCREEN_S3C2410)	+= s3c2410_ts.o
obj-$(CONFIG_TOUCHSCREEN_ST1232)	+= st1232.o
obj-$(CONFIG_TOUCHSCREEN_STMPE)		+= stmpe-ts.o
obj-$(CONFIG_TOUCHSCREEN_TNETV107X)	+= tnetv107x-ts.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_RMI)	+= synaptics_i2c_rmi.o
obj-$(CONFIG_TOUCHSCREEN_TOUCHIT213)	+= touchit213.o
obj-$(CONFIG_TOUCHSCREEN_TOUCHRIGHT)	+= touchright.o
obj-$(CONFIG_TOUCHSCREEN_TOUCHWIN)	+= touchwin.o
obj-$(CONFIG_TOUCHSCREEN_TSC2005)	+= tsc2005.o
obj-$(CONFIG_TOUCHSCREEN_TSC2007)	+= tsc2007.o
obj-$(CONFIG_TOUCHSCREEN_UCB1400)	+= ucb1400_ts.o
obj-$(CONFIG_TOUCHSCREEN_WACOM_W8001)	+= wacom_w8001.o
obj-$(CONFIG_TOUCHSCREEN_WM831X)	+= wm831x-ts.o
obj-$(CONFIG_TOUCHSCREEN_WM97XX)	+= wm97xx-ts.o
wm97xx-ts-$(CONFIG_TOUCHSCREEN_WM9705)	+= wm9705.o
wm97xx-ts-$(CONFIG_TOUCHSCREEN_WM9712)	+= wm9712.o
wm97xx-ts-$(CONFIG_TOUCHSCREEN_WM9713)	+= wm9713.o
obj-$(CONFIG_TOUCHSCREEN_WM97XX_ATMEL)	+= atmel-wm97xx.o
obj-$(CONFIG_TOUCHSCREEN_WM97XX_MAINSTONE)	+= mainstone-wm97xx.o
obj-$(CONFIG_TOUCHSCREEN_WM97XX_ZYLONITE)	+= zylonite-wm97xx.o
obj-$(CONFIG_TOUCHSCREEN_W90X900)	+= w90p910_ts.o
obj-$(CONFIG_TOUCHSCREEN_TPS6507X)	+= tps6507x-ts.o
obj-$(CONFIG_ADC_IMAPX800)		+= imapx800_adc.o
obj-$(CONFIG_CTP_ZET6221)       += zet622x.o
obj-$(CONFIG_CTP_SSD2533) += ssd2533.o
obj-$(CONFIG_CTP_UOR6153)                += uor6153_7inch.o
obj-$(CONFIG_CTP_MG8698)                += mg8698s.o
obj-$(CONFIG_CTP_MG8697)                += mg8697s.o
obj-$(CONFIG_CTP_MG8697_I7)                += mg8697s_i7a.o
obj-$(CONFIG_CTP_MG8697_I73)                += mg8697s_i7a3.o
obj-$(CONFIG_CTP_MG8696)                += mg8696s.o
