#include <asm/io.h>
#include <linux/rtc.h>
#include <mach/imap-rtc.h>
#include "imapx800.h"

extern void    __iomem     *sys_rtc_regs;

void rtc_check_time_for_reboot(void){

	uint8_t sec0, sec1, sec2, week0, week1;
	while(1){
		sec0 = readl(sys_rtc_regs + SYS_RTC_SECR0);
		sec1 = readl(sys_rtc_regs + SYS_RTC_SECR1);
		sec2 = readl(sys_rtc_regs + SYS_RTC_SECR2);
		week0 = readl(sys_rtc_regs + SYS_RTC_WEEK0);
		week1 = readl(sys_rtc_regs + SYS_RTC_WEEK1);

		if((sec0 == 1) && (sec1 == 0) && (sec2 == 0)){
			break;
		}
	}
	printk("set time end\n");
}

void rtc_set_alarm_for_reboot(void){

	writel(0x6, sys_rtc_regs + SYS_ALARM_SEC0);
	writel(0x0, sys_rtc_regs + SYS_ALARM_SEC1);
	writel(0x0, sys_rtc_regs + SYS_ALARM_SEC2);
	writel(0x0, sys_rtc_regs + SYS_ALARM_WEEK0);
	writel(0x0, sys_rtc_regs + SYS_ALARM_WEEK1);
	writel(0x3, sys_rtc_regs + SYS_ALARM_WEN);
}

void rtc_set_time_for_reboot(void){

	writel(0x1, sys_rtc_regs + SYS_RTC_SEC0);
	writel(0x0, sys_rtc_regs + SYS_RTC_SEC1);
	writel(0x0, sys_rtc_regs + SYS_RTC_SEC2);
	writel(0x0, sys_rtc_regs + SYS_RTC_MSEC0);
	writel(0x0, sys_rtc_regs + SYS_RTC_MSEC1);
	writel(0x0, sys_rtc_regs + SYS_RTC_WEEK0);
	writel(0x0, sys_rtc_regs + SYS_RTC_WEEK1);
	writel(0x1, sys_rtc_regs + SYS_RTC_WEEKWEN);
	writel(0x1, sys_rtc_regs + SYS_RTC_SECWEN);
	writel(0x1, sys_rtc_regs + SYS_RTC_MSECWEN);

}

int rtc_reboot(void){

	uint8_t tmp;
#if 1
	rtc_set_alarm_for_reboot();
	rtc_set_time_for_reboot();

	writel(0xff, sys_rtc_regs + SYS_INT_CLR);

	tmp = readl(sys_rtc_regs + SYS_ALARM_WEN);
	tmp |= 0x4;
	writel(tmp, sys_rtc_regs + SYS_ALARM_WEN);

	rtc_check_time_for_reboot();

	//writel(0x7f, SYS_WAKEUP_MASK);
	writel(0x2, sys_rtc_regs + SYS_POWMASK); //mask charing
#endif
	writel(0x2, sys_rtc_regs + SYS_CFG_CMD);

	return 0;
}

