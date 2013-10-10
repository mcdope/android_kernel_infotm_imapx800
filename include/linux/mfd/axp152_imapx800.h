#ifndef __AXP152_X820_H__
#define __AXP152_X820_H__

#include <linux/regulator/machine.h>
#include <linux/mfd/tps65910.h>
#include <linux/i2c.h>

static struct regulator_consumer_supply axp152_dcdc1_supply[] = {
	REGULATOR_SUPPLY("dcdc1", "imapx_axp152_dcdc1_null"),	/*default*/
};

static struct regulator_consumer_supply axp152_dcdc2_supply[] = {
	REGULATOR_SUPPLY("dcdc2", "imapx_axp152_dcdc2_null"),	/*default*/
};

static struct regulator_consumer_supply axp152_dcdc3_supply[] = {
	REGULATOR_SUPPLY("dcdc3", "imapx_axp152_dcdc3_null"),	/*default*/
};

static struct regulator_consumer_supply axp152_dcdc4_supply[] = {
	REGULATOR_SUPPLY("dcdc4", "imapx-ids"),
};

static struct regulator_consumer_supply axp152_ldo0_supply[] = {
	REGULATOR_SUPPLY("ldo0", "imapx-wifi"),
};

static struct regulator_consumer_supply axp152_aldo1_supply[] = {
	REGULATOR_SUPPLY("aldo1", "1-0010"),
	REGULATOR_SUPPLY("aldo1", "1-001a"),
};

static struct regulator_consumer_supply axp152_aldo2_supply[] = {
	REGULATOR_SUPPLY("aldo2", "imapx_axp152_aldo2_null"),	/*default*/
};

static struct regulator_consumer_supply axp152_dldo1_supply[] = {
	REGULATOR_SUPPLY("dldo1", "imapx-isp"),
};

static struct regulator_consumer_supply axp152_dldo2_supply[] = {
	REGULATOR_SUPPLY("dldo2", "imapx-isp"),
};

//static struct regulator_consumer_supply axp152_gpioldo_supply[] = {
//      REGULATOR_SUPPLY("gpioldo", "imapx_axp152_gpioldo_null"),       /*not use*/
//};

//static struct regulator_consumer_supply axp152_rtc31_supply[] = {
//	REGULATOR_SUPPLY("rtc31", "imapx_axp152_rtc31_null"),	/*not use*/
//};

//static struct regulator_consumer_supply axp152_rtc13_supply[] = {
//	REGULATOR_SUPPLY("rtc13", "imapx_axp152_rtc13_null"),	/*not use*/
//};

#define axp152_vreg_init_data(_name, _min, _max, _modes, _ops, consumer) { \
	.constraints = {\
		.name = _name,\
		.min_uV = _min,\
		.max_uV = _max,\
		.valid_modes_mask = _modes,\
		.valid_ops_mask = _ops,\
	},\
	.num_consumer_supplies = ARRAY_SIZE(consumer),\
	.consumer_supplies = consumer,\
}

static struct regulator_init_data axp152_verg_init_pdata[] = {
	axp152_vreg_init_data("dldo2", 700000, 3500000, REGULATOR_MODE_NORMAL,
				REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE, axp152_dldo2_supply),
	axp152_vreg_init_data("dldo1", 700000, 3500000, REGULATOR_MODE_NORMAL,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE, axp152_dldo1_supply),
	axp152_vreg_init_data("aldo2", 1200000, 3300000, REGULATOR_MODE_NORMAL,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE, axp152_aldo2_supply),
	axp152_vreg_init_data("aldo1", 1200000, 3300000, REGULATOR_MODE_NORMAL,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE, axp152_aldo1_supply),
	axp152_vreg_init_data("dcdc4", 700000, 3500000, REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE, axp152_dcdc4_supply),
	axp152_vreg_init_data("dcdc3", 700000, 3500000, REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE, axp152_dcdc3_supply),
	axp152_vreg_init_data("dcdc2", 700000, 2275000, REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE, axp152_dcdc2_supply),
	axp152_vreg_init_data("dcdc1", 1700000, 3500000, REGULATOR_MODE_NORMAL | REGULATOR_MODE_STANDBY,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE, axp152_dcdc1_supply),
	axp152_vreg_init_data("ldo0", 2500000, 5000000, REGULATOR_MODE_NORMAL,
			REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_CURRENT, axp152_ldo0_supply),
};

static struct axp152_platform_data axp152_platform_data = {
	.axp152_pmic_init_data = axp152_verg_init_pdata,
};

#endif
