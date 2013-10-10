#ifndef __IMPAX_WTD_H__
#define __IMPAX_WTD_H__

//#define WTD_DEBUG
#ifdef WTD_DEBUG
#define wtd_dbg(x...)     printk(x)
#else
#define wtd_dbg(x...)
#endif
#define wtd_err(x...)     printk(x)

#define KERNEL_KEEPWATCHDOG     180
#define WATCHDOG_TIMEOUT        8
#define WATCHDOG_PERIOD         2

#define WDT_SYS_RST                     0
#define WDT_SYS_RST_WITH_INTER          1

#define WDT_RESTART_DEFAULT_VALUE       0x76
#define WDT_MAX_FREQ_VALUE              804000000       //apll freq (HZ)

#define ENABLE  1
#define DISABLE 0

#define WDT_CR                     0x00           // receive buffer register
#define WDT_TORR                   0x04           // interrupt enable register
#define WDT_CCVR                   0x08           // interrupt identity register
#define WDT_CRR                    0x0C           // line control register
#define WDT_STAT                   0x10           // modem control register
#define WDT_EOI                    0x14           // line status register

enum {
    CHARGER_MODE,
    RECOVERY_MODE,
    NORMAL_MODE,
    ERROR_MODE,
};

struct imapx_wtd_params {
    struct device *dev; 
    void __iomem  *io_base; 
    struct hrtimer feed_timer;
    struct hrtimer watch_timer;
    spinlock_t lock;
    int last_settime;
    int irq;
    int ms;
    int bootmode;
    struct work_struct rebootWork;
};

#endif
