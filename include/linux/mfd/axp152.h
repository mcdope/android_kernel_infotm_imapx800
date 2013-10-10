#ifndef __LINUX_MFD_AXP152_H
#define __LINUX_MFD_AXP152_H

#define AXP152			0

#define AXP152_REG_DLDO2	0
#define AXP152_REG_DLDO1	1
#define AXP152_REG_ALDO2	2
#define AXP152_REG_ALDO1	3
#define AXP152_REG_DCDC4	4
#define AXP152_REG_DCDC3	5
#define AXP152_REG_DCDC2	6
#define AXP152_REG_DCDC1	7
#define AXP152_REG_LDO0		8
#define AXP152_REG_GPIOLDO	9

/*
 * List of registers for component AXP152
 */

/*power ctrl*/
#define AXP152_WORK_STATUS	0x01
#define AXP152_POWER_CTL	0x12
#define AXP152_ALDO_MODE	0x13
#define AXP152_LDO0		0x15
#define AXP152_DCDC2		0x23
#define AXP152_DCDC2_SP		0x25
#define AXP152_DCDC1		0x26
#define AXP152_DCDC3		0x27
#define AXP152_ALDO		0x28
#define AXP152_DLDO1		0x29
#define AXP152_DLDO2		0x2a
#define AXP152_DCDC4		0x2b
#define AXP152_VOFF		0x31
#define AXP152_POWER_DOWN	0x32
#define AXP152_POK		0x36
#define AXP152_DCDC_FRQ		0x37
#define AXP152_DCDC_MODE	0x80
#define AXP152_MONITOR		0x81
#define AXP152_TIMER		0x8a
#define AXP152_OP		0x8f

/*gpio ctrl*/
#define AXP152_GPIO0		0x90
#define AXP152_GPIO1		0x91
#define AXP152_GPIO2		0x92/*SYSEN*/
#define AXP152_GPIO3		0x93/*PWREN*/
#define AXP152_GPIO2_LDO	0x96
#define AXP152_GPIO_INPUT	0x97
#define AXP152_PWM0_FRQ		0x98
#define AXP152_PWM0_DENO	0x99/*pwm0 denominator*/
#define AXP152_PWM0_MOLE	0x9a/*pwm0 molecusar*/
#define AXP152_PWM1_FRQ		0x9b
#define AXP152_PWM1_DENO	0x9c
#define AXP152_PWM1_MOLE	0x9d

/*irq ctrl*/
#define AXP152_IRQ_EN1		0x40
#define AXP152_IRQ_EN2		0x41
#define AXP152_IRQ_EN3		0x42
#define AXP152_IRQ_ST1		0x48
#define AXP152_IRQ_ST2		0x49
#define AXP152_IRQ_ST3		0x4a

#define AXP152_MAX_REGISTER	0x9d

/*
 * struct axp152_platform_data
 * Board platform data may be used to initialize regulators.
 */
struct axp152_platform_data {
	struct regulator_init_data *axp152_pmic_init_data;
};

/*
 * struct axp152 - axp152 sub-driver chip access routines
 */
struct axp152 {
	struct device *dev;
	struct i2c_client *i2c_client;
	struct mutex io_mutex;
	unsigned int id;
	int (*read)(struct axp152 *axp152, u8 reg, int size, void *dest);
	int (*write)(struct axp152 *axp152, u8 reg, int size, void *src);
};

int axp152_set_bits(struct axp152 *axp152, u8 reg, u8 mask);
int axp152_clear_bits(struct axp152 *axp152, u8 reg, u8 mask);

#endif /* __LINUX_MFD_AXP152_H */
