
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/mfd/axp152.h>
#include <mach/items.h>


/* supported DCDC1 voltages in milivolts */
static const u16 DCDC1_VSEL_table[] = {
	1700, 1800, 1900, 2000,
	2100, 2400, 2500, 2600,
	2700, 2800, 3000, 3100,
	3200, 3300, 3400, 3500
};

/* supported DCDC2 voltages in milivolts */
static const u16 DCDC2_VSEL_table[] = {
	700, 725, 750, 775, 800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175, 1200, 1225, 1250, 1275,
	1300, 1325, 1350, 1375, 1400, 1425, 1450, 1475, 1500, 1525, 1550, 1575,
	1600, 1625, 1650, 1675, 1700, 1725, 1750, 1775, 1800, 1825, 1850, 1875,
	1900, 1925, 1950, 1975, 2000, 2025, 2050, 2075, 2100, 2125, 2150, 2175,
	2200, 2225, 2250, 2275
};

/* supported DCDC3 voltages in milivolts */
static const u16 DCDC3_VSEL_table[] = {
	700, 750, 800, 850, 900, 950, 1000, 1050, 1100, 1150,
	1200, 1250, 1300, 1350, 1400, 1450, 1500, 1550, 1600, 1650,
	1700, 1750, 1800, 1850, 1900, 1950, 2000, 2050, 2100, 2150,
	2200, 2250, 2300, 2350, 2400, 2450, 2500, 2550, 2600, 2650,
	2700, 2750, 2800, 2850, 2900, 2950, 3000, 3050, 3100, 3150,
	3200, 3250, 3300, 3350, 3400, 3450, 3500
};

/* supported DCDC4 voltages in milivolts */
static const u16 DCDC4_VSEL_table[] = {
	700, 725, 750, 775, 800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175, 1200, 1225, 1250, 1275,
	1300, 1325, 1350, 1375, 1400, 1425, 1450, 1475, 1500, 1525, 1550, 1575,
	1600, 1625, 1650, 1675, 1700, 1725, 1750, 1775, 1800, 1825, 1850, 1875,
	1900, 1925, 1950, 1975, 2000, 2025, 2050, 2075, 2100, 2125, 2150, 2175,
	2200, 2225, 2250, 2275, 2300, 2325, 2350, 2375, 2400, 2425, 2450, 2475,
	2500, 2525, 2550, 2575, 2600, 2625, 2650, 2675, 2700, 2725, 2750, 2775,
	2800, 2825, 2850, 2875, 2900, 2925, 2950, 2975, 3000, 3025, 3050, 3075,
	3100, 3125, 3150, 3175, 3200, 3225, 3250, 3275, 3300, 3325, 3350, 3375,
	3400, 3425, 3450, 3475, 3500
};

/* supported RTC31 voltages in milivolts */
static const u16 RTC31_VSEL_table[] = {
	3100
};

/* supported RTC13 voltages in milivolts */
static const u16 RTC13_VSEL_table[] = {
	1300, 1800
};

/* supported LDO0 voltages in milivolts */
static const u16 LDO0_VSEL_table[] = {
	5000, 3300, 2800, 2500
};

/* supported LDO0 current in milliampere  */
static const u16 LDO0_CSEL_table[] = {
	0, 1500, 900, 500
};

/* supported ALDO1 voltages in milivolts */
static const u16 ALDO1_VSEL_table[] = {
	1200, 1300, 1400, 1500,
	1600, 1700, 1800, 1900,
	2000, 2500, 2700, 2800,
	3000, 3100, 3200, 3300
};

/* supported ALDO2 voltages in milivolts */
static const u16 ALDO2_VSEL_table[] = {
	1200, 1300, 1400, 1500,
	1600, 1700, 1800, 1900,
	2000, 2500, 2700, 2800,
	3000, 3100, 3200, 3300
};

/* supported DLDO1 voltages in milivolts */
static const u16 DLDO1_VSEL_table[] = {
	700, 800, 900, 1000, 1100, 1200, 1300, 1400,
	1500, 1600, 1700, 1800, 1900, 2000, 2100, 2200,
	2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000,
	3100, 3200, 3300, 3400, 3500
};

/* supported DLDO2 voltages in milivolts */
static const u16 DLDO2_VSEL_table[] = {
	700, 725, 750, 775, 800, 825, 850, 875, 900, 925, 950, 975,
	1000, 1025, 1050, 1075, 1100, 1125, 1150, 1175, 1200, 1225, 1250, 1275,
	1300, 1325, 1350, 1375, 1400, 1425, 1450, 1475, 1500, 1525, 1550, 1575,
	1600, 1625, 1650, 1675, 1700, 1725, 1750, 1775, 1800, 1825, 1850, 1875,
	1900, 1925, 1950, 1975, 2000, 2025, 2050, 2075, 2100, 2125, 2150, 2175,
	2200, 2225, 2250, 2275, 2300, 2325, 2350, 2375, 2400, 2425, 2450, 2475,
	2500, 2525, 2550, 2575, 2600, 2625, 2650, 2675, 2700, 2725, 2750, 2775,
	2800, 2825, 2850, 2875, 2900, 2925, 2950, 2975, 3000, 3025, 3050, 3075,
	3100, 3125, 3150, 3175, 3200, 3225, 3250, 3275, 3300, 3325, 3350, 3375,
	3400, 3425, 3450, 3475, 3500
};

/* supported GPIOLDO voltages in milivolts */
static const u16 GPIOLDO_VSEL_table[] = {
	1800, 1900, 2000, 2100,
	2200, 2300, 2400, 2500,
	2600, 2700, 2800, 2900,
	3000, 3100, 3200, 3300
};

struct axp_info {
	const char *name;
	unsigned int min_uV;
	unsigned int max_uV;
	u8 table_len;
	const u16 *table;
};

static struct axp_info axp152_regs[] = {
	{
		.name   = "dldo2",
		.min_uV = 700000,
		.max_uV = 3500000,
		.table_len = ARRAY_SIZE(DLDO2_VSEL_table),
		.table  = DLDO2_VSEL_table,
	},
	{
		.name   = "dldo1",
		.min_uV = 700000,
		.max_uV = 3500000,
		.table_len = ARRAY_SIZE(DLDO1_VSEL_table),
		.table  = DLDO1_VSEL_table,
	},
	{
		.name   = "aldo2",
		.min_uV = 1200000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(ALDO2_VSEL_table),
		.table  = ALDO2_VSEL_table,
	},
	{
		.name   = "aldo1",
		.min_uV = 1200000,
		.max_uV = 3300000,
		.table_len = ARRAY_SIZE(ALDO1_VSEL_table),
		.table  = ALDO1_VSEL_table,
	},
	{
		.name   = "dcdc4",
		.min_uV = 700000,
		.max_uV = 3500000,
		.table_len = ARRAY_SIZE(DCDC4_VSEL_table),
		.table  = DCDC4_VSEL_table,
	},
	{
		.name   = "dcdc3",
		.min_uV = 700000,
		.max_uV = 3500000,
		.table_len = ARRAY_SIZE(DCDC3_VSEL_table),
		.table  = DCDC3_VSEL_table,
	},
	{
		.name   = "dcdc2",
		.min_uV = 700000,
		.max_uV = 2275000,
		.table_len = ARRAY_SIZE(DCDC2_VSEL_table),
		.table  = DCDC2_VSEL_table,
	},
	{
		.name   = "dcdc1",
		.min_uV = 1700000,
		.max_uV = 3500000,
		.table_len = ARRAY_SIZE(DCDC1_VSEL_table),
		.table  = DCDC1_VSEL_table,
	},
	{
		.name   = "ldo0",
		.min_uV = 2500000,
		.max_uV = 5000000,
		.table_len = ARRAY_SIZE(LDO0_VSEL_table),
		.table  = LDO0_VSEL_table,
	},
//	{
//		.name   = "gpioldo",
//		.min_uV	= 1800000,
//		.max_uV	= 3300000,
//		.table_len = ARRAY_SIZE(GPIOLDO_VSEL_table),
//		.table	= GPIOLDO_VSEL_table,
//	},
//	{
//		.name   = "rtc31",
//		.min_uV = 3100000,
//		.max_uV = 3100000,
//		.table_len = ARRAY_SIZE(RTC31_VSEL_table),
//		.table  = RTC31_VSEL_table,
//	},
//	{
//		.name   = "rtc13",
//		.min_uV = 1300000,
//		.max_uV = 1800000,
//		.table_len = ARRAY_SIZE(RTC13_VSEL_table),
//		.table  = RTC13_VSEL_table,
//	},
};

struct axp152_reg {
	struct regulator_desc *desc;
	struct axp152 *mfd;
	struct regulator_dev **rdev;
	struct axp_info **info;
	struct mutex mutex;
	int num_regulators;
	int mode;
	int (*get_en_reg)(int);
	int (*get_ctrl_reg)(int);
};

static inline int axp152_read(struct axp152_reg *pmic, u8 reg)
{
	u8 val;
	int err;

	err = pmic->mfd->read(pmic->mfd, reg, 1, &val);
	if(err)
		return err;

	return val;
}

static inline int axp152_write(struct axp152_reg *pmic, u8 reg, u8 val)
{
	return pmic->mfd->write(pmic->mfd, reg, 1, &val);
}

static int axp152_modify_bits(struct axp152_reg *pmic, u8 reg,
					u8 set_mask, u8 clear_mask)
{
	int err, data;

	mutex_lock(&pmic->mutex);

	data = axp152_read(pmic, reg);
	if(data < 0) {
		dev_err(pmic->mfd->dev, "Read from reg 0x%x failed\n", reg);
		err = data;
		goto out;
	}

	data &= ~clear_mask;
	data |= set_mask;
	err = axp152_write(pmic, reg, data);
	if(err)
		dev_err(pmic->mfd->dev, "Write for reg 0x%x failed\n", reg);

out:
	mutex_unlock(&pmic->mutex);
	return err;
}

static int axp152_reg_read(struct axp152_reg *pmic, u8 reg)
{
	int data;

	mutex_lock(&pmic->mutex);

	data = axp152_read(pmic, reg);
	if(data < 0)
		dev_err(pmic->mfd->dev, "Read from reg 0x%x failed\n", reg);

	mutex_unlock(&pmic->mutex);

	return data;
}

static int axp152_reg_write(struct axp152_reg *pmic, u8 reg, u8 val)
{
	int err;

	mutex_lock(&pmic->mutex);

	err = axp152_write(pmic, reg, val);
	if(err < 0)
		dev_err(pmic->mfd->dev, "Write for reg 0x%x failed\n", reg);

	mutex_unlock(&pmic->mutex);

	return err;
}

static int axp152_get_enable_register(int id)
{
	switch (id) {
	case AXP152_REG_DCDC1:
	case AXP152_REG_DCDC2:
	case AXP152_REG_DCDC3:
	case AXP152_REG_DCDC4:
	case AXP152_REG_ALDO1:
	case AXP152_REG_ALDO2:
	case AXP152_REG_DLDO1:
	case AXP152_REG_DLDO2:
		return AXP152_POWER_CTL;
	case AXP152_REG_LDO0:
		return AXP152_LDO0;
	case AXP152_REG_GPIOLDO:
		return AXP152_GPIO2;
	default:
		return -EINVAL;
	}
}

static int axp152_get_ctrl_register(int id)
{
	switch (id) {
	case AXP152_REG_DCDC1:
		return AXP152_DCDC1;
	case AXP152_REG_DCDC2:
		return AXP152_DCDC2;
	case AXP152_REG_DCDC3:
		return AXP152_DCDC3;
	case AXP152_REG_DCDC4:
		return AXP152_DCDC4;
	case AXP152_REG_LDO0:
		return AXP152_LDO0;
	case AXP152_REG_ALDO1:
		return AXP152_ALDO;
	case AXP152_REG_ALDO2:
		return AXP152_ALDO;
	case AXP152_REG_DLDO1:
		return AXP152_DLDO1;
	case AXP152_REG_DLDO2:
		return AXP152_DLDO2;
	case AXP152_REG_GPIOLDO:
		return AXP152_GPIO2_LDO;
	default:
		return -EINVAL;
	}
}

static int axp152_is_enabled(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev);
	unsigned char mask;

	reg = pmic->get_en_reg(id);
	if(reg < 0)
		return reg;

	value = axp152_reg_read(pmic, reg);
	if(value < 0)
		return value;

	if(id < 8)
		mask = 1 << id;
	else if(id == 8)
		mask = 1 << 7;
	else
		return -EINVAL;

	return value & mask;
}

static int axp152_is_enabled_gpioldo(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int value = axp152_reg_read(pmic, AXP152_GPIO2);

	return (value & 0x7) == 0x2;
}

static int axp152_enable(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	struct axp152 *mfd = pmic->mfd;
	int reg, id = rdev_get_id(dev);
	unsigned char mask;

	reg = pmic->get_en_reg(id);
	if(reg < 0)
		return reg;

	if(id < 8)
		mask = 1 << id;
	else if(id == 8)
		mask = 1 << 7;
	else
		return -EINVAL;

	return axp152_set_bits(mfd, reg, mask);
}

static int axp152_enable_gpioldo(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	u8 value;
	
	value = axp152_reg_read(pmic, AXP152_GPIO2);
	if(value < 0)
		return value;

	value &= 0xf8;
	value |= 0x2;
	axp152_reg_write(pmic, AXP152_GPIO2, value);

	return value;
}

static int axp152_disable(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	struct axp152 *mfd = pmic->mfd;
	int reg, id = rdev_get_id(dev);
	unsigned char mask;

	reg = pmic->get_en_reg(id);
	if(reg < 0)
		return reg;

	if(id < 8)
		mask = 1 << id;
	else
		mask = 1 << 7;

	return axp152_clear_bits(mfd, reg, mask);
}

static int axp152_disable_gpioldo(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	u8 value;
	
	value = axp152_reg_read(pmic, AXP152_GPIO2);
	if(value < 0)
		return value;

	value &= 0xf8;
	value |= 0x7;
	axp152_reg_write(pmic, AXP152_GPIO2, value);

	return value;
}

static int axp152_get_voltage(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int reg, value, id = rdev_get_id(dev), voltage = 0;

	reg = pmic->get_ctrl_reg(id);
	if(reg < 0)
		return reg;

	value = axp152_reg_read(pmic, reg);
	if(value < 0)
		return value;

	switch (id) {
	case AXP152_REG_DCDC1:
	case AXP152_REG_DCDC2:
	case AXP152_REG_DCDC3:
	case AXP152_REG_DCDC4:
	case AXP152_REG_GPIOLDO:
		break;

	case AXP152_REG_ALDO1:
		value = (value & 0xf0) >> 4;
		break;

	case AXP152_REG_ALDO2:
		value &= 0x0f;
		break;

	case AXP152_REG_DLDO1:
	case AXP152_REG_DLDO2:
		value &= 0x1f;
		break;

	case AXP152_REG_LDO0:
		value = (value & 0x30) >> 4;
		break;

	default:
		return -EINVAL;
	}

	voltage = pmic->info[id]->table[value] * 1000;

	return voltage;
}

static int axp152_set_voltage(struct regulator_dev *dev,
	       			int min_uV, int max_uV)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int reg, val, vsel, id = rdev_get_id(dev);

	if(min_uV < pmic->info[id]->min_uV ||
			min_uV > pmic->info[id]->max_uV)
		goto err;
	if(max_uV < pmic->info[id]->min_uV ||
			max_uV > pmic->info[id]->max_uV)
		goto err;

	for(vsel = 0; vsel < pmic->info[id]->table_len; vsel++) {
		int Val = pmic->info[id]->table[vsel];
		int uV = Val * 1000;

		if(min_uV <= uV && uV <= max_uV)
			break;
	}

	if(vsel >= pmic->info[id]->table_len)
		goto err;

	reg = pmic->get_ctrl_reg(id);
	if(reg < 0)
		return reg;
	val = axp152_reg_read(pmic, reg);
	if(val < 0)
		return val;

	switch(id) {
	case AXP152_REG_DCDC1:  
	case AXP152_REG_DCDC2:  
	case AXP152_REG_DCDC3:  
	case AXP152_REG_DCDC4:  
	case AXP152_REG_GPIOLDO:
		val = vsel;
		break;

	case AXP152_REG_ALDO1:
		val &= 0x0f;
		val |= vsel << 4;
		break;

	case AXP152_REG_ALDO2:
		val &= 0xf0;
		val |= vsel;
		break;

	case AXP152_REG_DLDO1:
	case AXP152_REG_DLDO2:
		val &= 0xe0;
		val |= vsel;
		break;

	case AXP152_REG_LDO0:
			printk("val=0x%02x, vsel=%d\n", val, vsel);
		val &= 0xcf;
		val |= vsel << 4;
			printk("val=0x%02x\n", val);
		break;

	default:
		return -EINVAL;
	}

	val = axp152_reg_write(pmic, reg, val);

	return 0;

err:
	dev_err(pmic->mfd->dev, "AXP152 set voltage failed!\n");
	return -EINVAL;
}

static int axp152_list_voltage(struct regulator_dev *dev, unsigned selector)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int id = rdev_get_id(dev), voltage;

	if(id < AXP152_REG_DLDO2 || id > AXP152_REG_GPIOLDO)
		return -EINVAL;

	if(selector >= pmic->info[id]->table_len)
		return -EINVAL;
	else
		voltage = pmic->info[id]->table[selector] * 1000;

	return voltage;
}

static int axp152_get_current_ldo0(struct regulator_dev *dev)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int value;
	int cur = 0;

	value = axp152_reg_read(pmic, AXP152_LDO0);
	if(value < 0)
		return value;

	value &= 0x3;

	cur = LDO0_CSEL_table[value] * 1000;

	return cur;
}

static int axp152_set_current_ldo0(struct regulator_dev *dev, 
					int min_uA, int max_uA)
{
	struct axp152_reg *pmic = rdev_get_drvdata(dev);
	int val, vsel;

	if(min_uA < 0 || min_uA > 1500000 || 
			max_uA < 0 || max_uA > 1500000)
		goto err;

	for(vsel = 0; vsel < ARRAY_SIZE(LDO0_CSEL_table); vsel++){
		int uA = LDO0_CSEL_table[vsel] * 1000;

		if(min_uA <= uA && uA <= max_uA)
			break;
	}

	if(vsel >= ARRAY_SIZE(LDO0_CSEL_table))
		goto err;

	val = axp152_reg_read(pmic, AXP152_LDO0);
	if(val < 0)
		return val;
	val &= 0xfc;
	val |= vsel;
	val = axp152_reg_write(pmic, AXP152_LDO0, val);

	return 0;

err:
	dev_err(pmic->mfd->dev, "AXP152 set ldo0 current limit failed!\n");
	return -EINVAL;
}

/* Regulator ops (except RTCLDO) */
static struct regulator_ops axp152_ops_gpioldo = {
	.is_enabled	= axp152_is_enabled_gpioldo,
	.enable		= axp152_enable_gpioldo,
	.disable	= axp152_disable_gpioldo,
	.get_voltage	= axp152_get_voltage,
	.set_voltage	= axp152_set_voltage,
	.list_voltage	= axp152_list_voltage,
};

static struct regulator_ops axp152_ops_ldo0 = {
	.is_enabled     = axp152_is_enabled,
	.enable         = axp152_enable,
	.disable        = axp152_disable,
	.get_voltage    = axp152_get_voltage,
	.set_voltage    = axp152_set_voltage,
	.get_current_limit = axp152_get_current_ldo0,
	.set_current_limit = axp152_set_current_ldo0,
	.list_voltage   = axp152_list_voltage,
};

static struct regulator_ops axp152_ops = {
	.is_enabled     = axp152_is_enabled,
	.enable         = axp152_enable,
	.disable        = axp152_disable,
	.get_voltage    = axp152_get_voltage,
	.set_voltage    = axp152_set_voltage,
	.list_voltage   = axp152_list_voltage,
};

static __devinit int axp152_probe(struct platform_device *pdev)
{
	struct axp152 *axp152 = dev_get_drvdata(pdev->dev.parent);
	struct axp_info *info;
	struct regulator_init_data *reg_data;
	struct regulator_dev *rdev;
	struct axp152_reg *pmic;
	struct axp152_platform_data *pmic_plat_data;
	int i, err;

	pmic_plat_data = dev_get_platdata(axp152->dev);
	if(!pmic_plat_data)
		return -EINVAL;

	reg_data = pmic_plat_data->axp152_pmic_init_data;

	pmic = kzalloc(sizeof(*pmic), GFP_KERNEL);
	if(!pmic)
		return -ENOMEM;

	mutex_init(&pmic->mutex);
	pmic->mfd = axp152;
	platform_set_drvdata(pdev, pmic);

	pmic->get_en_reg = axp152_get_enable_register;
	pmic->get_ctrl_reg = axp152_get_ctrl_register;
	pmic->num_regulators = ARRAY_SIZE(axp152_regs);
	info = axp152_regs;

	pmic->desc = kcalloc(pmic->num_regulators,
				sizeof(struct regulator_desc), GFP_KERNEL);
	if(!pmic->desc) {
		err = -ENOMEM;
		goto err_free_pmic;
	}

	pmic->info = kcalloc(pmic->num_regulators,
				sizeof(struct axp_info *), GFP_KERNEL);
	if(!pmic->info) {
		err = -ENOMEM;
		goto err_free_desc;
	}

	pmic->rdev = kcalloc(pmic->num_regulators,
				sizeof(struct regulator_dev *), GFP_KERNEL);
	if(!pmic->rdev) {
		err = -ENOMEM;
		goto err_free_info;
	}

	for(i = 0; i < pmic->num_regulators; i++, info++, reg_data++) {
		/* Register the regulators */
		pmic->info[i] = info;

		pmic->desc[i].name = info->name;
		pmic->desc[i].id = i;
		pmic->desc[i].n_voltages = info->table_len;

		if(i == AXP152_REG_GPIOLDO)
			pmic->desc[i].ops = &axp152_ops_gpioldo;
		else if(i == AXP152_REG_LDO0)
			pmic->desc[i].ops = &axp152_ops_ldo0;
		else
			pmic->desc[i].ops = &axp152_ops;

		pmic->desc[i].type = REGULATOR_VOLTAGE;
		pmic->desc[i].owner = THIS_MODULE;

		rdev = regulator_register(&pmic->desc[i], axp152->dev, reg_data, pmic);
		if(IS_ERR(rdev)) {
			dev_err(axp152->dev,
				"failed to register %s regulator\n",
				pdev->name);
			err = PTR_ERR(rdev);
			goto err_unregister_regulator;
		}

		/* Save regulator for cleanup */
		pmic->rdev[i] = rdev;
	}

	printk("%s done\n", __func__);
	return 0;

err_unregister_regulator:
	while (--i >= 0)
		regulator_unregister(pmic->rdev[i]);
	kfree(pmic->rdev);
err_free_info:
	kfree(pmic->info);
err_free_desc:
	kfree(pmic->desc);
err_free_pmic:
	kfree(pmic);

	return err;
}

static int __devexit axp152_remove(struct platform_device *pdev)
{
	struct axp152_reg *pmic = platform_get_drvdata(pdev);
	int i;

	for(i = 0; i < pmic->num_regulators; i++)
		regulator_unregister(pmic->rdev[i]);

	kfree(pmic->rdev);
	kfree(pmic->info);
	kfree(pmic->desc);
	kfree(pmic);

	return 0;
}

static struct platform_driver axp152_driver = {
	.driver = {
		.name = "axp152-pmic",
		.owner = THIS_MODULE,
	},
	.probe = axp152_probe,
	.remove = __devexit_p(axp152_remove),
};

static int __init axp152_init(void)
{
	if(item_exist("pmu.model")) {
		if(item_equal("pmu.model", "axp152", 0))
			return platform_driver_register(&axp152_driver);
		else
			printk(KERN_ERR "%s: pmu model is not axp152\n", __func__);
	}else
		printk(KERN_ERR "%s: pmu model is not exist\n", __func__);

	return -1;
}
module_init(axp152_init);

static void __exit axp152_cleanup(void)
{
	platform_driver_unregister(&axp152_driver);
}
module_exit(axp152_cleanup);

MODULE_AUTHOR("zhanglei <lei_zhang@infotm.com>");
MODULE_DESCRIPTION("AXP152 voltage regulator driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:axp152-pmic");

