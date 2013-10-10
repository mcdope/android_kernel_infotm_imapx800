/*
 * =====================================================================================
 *
 *       Filename:  bq2415x.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/07/2012 09:24:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Judy (), woshizhouli@gmail.com
 *        Company:  infoTM
 *
 * =====================================================================================
 */
/* ****************************************************************
 * ******************************************************************
 * *
 * bq24157 I2C Code
 * *
 * *
 * TI Confidential for Samsung
 * *
 * ******************************************************************/
#define BQ2415x_START_CHARGING (1 << 0)
#define BQ2415x_STOP_CHARGING (1 << 1)
#define BQ2415x_CHARGER_FAULT (1 << 2)

#define BQ2415x_CHARGE_DONE 0x20
#define BQ2415x_FAULT_VBUS_OVP 0x31
#define BQ2415x_FAULT_SLEEP 0x32
#define BQ2415x_FAULT_BAD_ADAPTOR 0x33
#define BQ2415x_FAULT_BAT_OVP 0x34
#define BQ2415x_FAULT_THERMAL_SHUTDOWN 0x35
#define BQ2415x_FAULT_TIMER 0x36
#define BQ2415x_FAULT_NO_BATTERY 0x37

struct bq2415x_platform_data {
    int max_charger_currentmA;
    int max_charger_voltagemV;
    int charge_currentmA;
    int charge_voltagemV;
    int termination_currentmA;
    bool enableTerm;
};

#define BQ2415X_VOREG_POS 2
#define BQ2415X_ICHG_POS  4
#define BQ2415X_MAXICHG_POS 4

unsigned int Reg0zVal;
unsigned int Reg1zVal;
unsigned int Reg2zVal;
unsigned int Reg3zVal;
unsigned int Reg4zVal;
unsigned int Reg5zVal;
unsigned int Reg6zVal;

#define Reg0Add 0x00
#define Reg1Add 0x01
#define Reg2Add 0x02
#define Reg3Add 0x03
#define Reg4Add 0x04
#define Reg5Add 0x05
#define Reg6Add 0x06

#define REG_STATUS_CONTROL          Reg0Add
#define REG_CONTROL_REGISTER        Reg1Add
#define REG_BATTERY_VOLTAGE         Reg2Add
#define REG_PART_REVISION           Reg3Add
#define REG_BATTERY_CURRENT         Reg4Add
#define REG_SPECIAL_CHARGER_VOLTAGE Reg5Add
#define REG_SAFETY_LIMIT            Reg6Add

#define DevID157 0x6b
#define DevID153 0x6b
#define BQ24156 0x00
#define BQ24151 0x00
#define BQ24153 0x10
#define BQ24158 0x10
//REG_STATUS_CONTROL
#define TIMER_RST (0x1<<7)
//REG_CONTROL_REGISTER
#define ENABLE_STAT_PIN  (0x1<<2)
//REG_BATTERY_VOLTAGE
#define VOLTAGE_VALUE   0x0a 
#define BQ24153_CURRENT_SHIFT 
#define RESET_BIT (1<<7)
//REG_BATTERY_CURRENT
#define CURRENT_VALUE 0x01 
#define BQ2415x_WATCHDOG_TIMEOUT 2000
