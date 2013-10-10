#ifndef __IMAPX_PAD_H__
#define __IMAPX_PAD_H__

#define INTTYPE_LOW     (0)
#define INTTYPE_HIGH    (1)
#define INTTYPE_FALLING (2)
#define INTTYPE_RISING  (4)
#define INTTYPE_BOTH    (6)
#define FILTER_MAX	(0xff)

enum imapx_padgroup {
	IMAPX_NAND = 0,
	IMAPX_SD0,
	IMAPX_SD1,
	IMAPX_SD2,
	IMAPX_PHY,
	IMAPX_TSP1GRP,
	IMAPX_TSP2GRP,
	IMAPX_GPS,
	IMAPX_I2C0,
	IMAPX_I2C1,
	IMAPX_I2C2,
	IMAPX_I2C3,
	IMAPX_I2C4,
	IMAPX_I2C5,
	IMAPX_UART0,
	IMAPX_UART1,
	IMAPX_UART2,
	IMAPX_UART3,
	IMAPX_PWM,
	IMAPX_PWM1,
	IMAPX_PWM2,
	IMAPX_SYS,
	IMAPX_OTG,
	IMAPX_HDMI,
	IMAPX_RGB0,
	IMAPX_RGB1,
	IMAPX_IIS0,
	IMAPX_IIS1,
	IMAPX_PCM0,
	IMAPX_PCM1,
	IMAPX_SSP0,
	IMAPX_SSP1,
	IMAPX_SSP2,
	IMAPX_SPDIF,
	IMAPX_PS2_0,
	IMAPX_PS2_1,
	IMAPX_KB,
	IMAPX_SATA,
	IMAPX_SIM0,
	IMAPX_SIM1,
	IMAPX_NOR,
	IMAPX_AC97,
	IMAPX_CAM,
	IMAPX_BOOT,
	IMAPX_DMIC,
	IMAPX_PWMA,
	IMAPX_EXTCLK,
	IMAPX_DBG
};

enum pad_index {
	GPA0,GPA1,GPA2,GPA3,GPA4,GPA5,GPA6,GPA7,GPA8,GPA9,GPA10,GPA11,GPA12,GPA13,GPA14,GPA15,
	GPB0,GPB1,GPB2,GPB3,GPB4,GPB5,GPB6,GPB7,GPB8,GPB9,GPB10,GPB11,GPB12,GPB13,GPB14,
	GPC0,GPC1,GPC2,GPC3,GPC4,GPC5,GPC6,
	GPD0,GPD1,GPD2,GPD3,GPD4,GPD5,GPD6,GPD7,GPD8,GPD9,GPD10,GPD11,GPD12,GPD13,GPD14,GPD15,
	GPE0,GPE1,GPE2,GPE3,GPE4,GPE5,GPE6,GPE7,GPE8,GPE9,GPE10,GPE11,GPE12,GPE13,GPE14,GPE15,
	GPF0,GPF1,GPF2,GPF3,GPF4,GPF5,GPF6,GPF7,GPF8,GPF9,GPF10,GPF11,
	GPM0,GPM1,GPM2,GPM3,GPM4,GPM5,GPM6,GPM7,GPM8,GPM9,
	GPG0,GPG1,GPG2,GPG3,GPG4,GPG5,
	GPH0,GPH1,GPH2,GPH3,GPH4,GPH5,GPH6,GPH7,GPH8,GPH9,GPH10,GPH11,
	GPI0,GPI1,GPI2,GPI3,GPI4,GPI5,GPI6,GPI7,GPI8,GPI9,GPI10,GPI11,GPI12,GPI13,GPI14,GPI15,
	GPJ0,GPJ1,GPJ2,GPJ3,GPJ4,GPJ5,GPJ6,GPJ7,
	GPK0,GPK1,GPK2,GPK3,GPK4,GPK5,GPK6,GPK7,GPK8,
	GPL0,GPL1,GPL2,GPL3,GPL4,GPL5,GPL6,GPL7,GPL8,GPL9,GPL10,GPL11,GPL12,
	GPN0,GPN1,GPN2,GPN3
};

enum pad_dir {
	DIRECTION_OUTPUT = 0,
	DIRECTION_INPUT
};

enum pad_mode {
	MODE_FUNCTION = 0,
	MODE_GPIO
};

enum pad_pull {
	PULL_DISABLE = 0,
	PULL_ENABLE
};

enum pad_outdat {
	OUTPUT_0 = 0,
	OUTPUT_1
};

int imapx_pad_set_dir(enum pad_dir dir, int num,...);
int imapx_pad_set_dir_range(enum pad_dir dir, int start, int end);
int imapx_pad_set_mode(enum pad_mode mode, int num,...);
int imapx_pad_set_mode_range(enum pad_mode mode, int start, int end);
int imapx_pad_set_pull(enum pad_index index,enum pad_pull pullen,bool pullup);
int imapx_pad_set_pull_range(int start,int end,enum pad_pull pullen,bool pullup);
int imapx_pad_set_outdat(enum pad_outdat outdat, int num,...);
int imapx_pad_set_outdat_range(enum pad_outdat outdat, int start, int end);
int imapx_pad_get_indat(enum pad_index index);
int imapx_pad_cfg(enum imapx_padgroup pg,int pull);
int imapx_pad_irqgroup_cfg(void);
int imapx_pad_index(const char *name);
int imapx_pad_irq_number(int index);
int imapx_pad_irq_group_num(int index);
int imapx_pad_irq_config(int index, int type, int filter);
int imapx_pad_irq_pending(int index);
int imapx_pad_irq_clear(int index);
int imapx_pad_irq_mask(int index);
int imapx_pad_irq_unmask(int index);

int rtc_gpio_dir(uint32_t index, int dir);
int rtc_gpio_outdat(uint32_t index, int outdat);
int rtc_gpio_indat(uint32_t index);
int rtc_gpio_pulldown_en(uint32_t index, int en);

#endif
