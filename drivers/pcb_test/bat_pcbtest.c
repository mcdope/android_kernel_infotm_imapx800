#include <linux/module.h>
#include <linux/ioport.h>
#include <mach/imap-iomap.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/init.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <mach/items.h>
#include <mach/imap-adc.h>
 
int (*batt_get_voltage)(void);
int (*batt_get_cap)(void);
void __imapx_register_pcbtest_batt_v(int (*func1)(void), int (*func2)(void))
{
         batt_get_voltage = func1;
         batt_get_cap = func2;
}
EXPORT_SYMBOL(__imapx_register_pcbtest_batt_v);
 
int (*charger_st)(void);
void __imapx_register_pcbtest_chg_state(int (*func1)(void))
{
     charger_st = func1;
}
EXPORT_SYMBOL(__imapx_register_pcbtest_chg_state);

int battery_pcbtest(void){

	int batt_val, batt_val_min, batt_val_max;
	int batt_cap;
		  
	printk("========== battery_pcbtest ==========\n");
	 
	if(item_exist("pmu.model"))
	{
	             if(item_equal("pmu.model", "tps65910", 0) || item_equal("pmu.model", "axp152", 0))
	             {
	                 printk("pmu.model is tps65910 or axp152\n");
	                 if(item_exist("batt.dischg"))
	                     batt_val_min = item_integer("batt.dischg", 11);
	                 else
	                     batt_val_min = 3200;
	                 printk("batt_min is %d\n", batt_val_min);
	                 batt_val_max = 4200;
	                 if(batt_val_min < 3000)
	                    goto ERR;
	             }
	             else if(item_equal("pmu.model", "axp202", 0))
	             {
	                 printk("pmu.model is axp202\n");
	                 batt_val_min = item_integer("batt.start", 0);
	                 batt_val_max = 4200;
	                 if(batt_val_min < 3000)
	                     goto ERR;
	             }
	             else
	                 goto ERR;
	         }
	         else
	         {
	             goto ERR;
	         }
	 
	         batt_val = batt_get_voltage();
		 printk("batt_val is %d\n", batt_val);
	         if((batt_val >= batt_val_min) && (batt_val <= batt_val_max))
	         {
	             if((batt_cap >= 0) && (batt_cap <= 100))
	             {
	                 printk("========== battery ok ==========\n");
	                 return batt_val;
	             }
	         }

 ERR:
         printk("========== battery fail ==========\n");
         return -1;
}
 
int charger_state(void)
{
         int ret;

         printk(" ========== charger test ==========\n");

         ret = charger_st();
         return ret;
}


