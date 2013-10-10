#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <mach/power-gate.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mach/items.h>
#include "imapx800_wdt.h"

#define AUTO_START_WATCHDOG
#define TRACE_WATCHDOG
/*
 * set time(unit s)   set in register
 *      1 s                11
 *      2 s                12
 *      4 s                13
 *      8 s                14
 *      16 s               15
 */

extern void imap_reset(int type);
static int imapx_gettime(void);

static char expect_close = 0;
static int nowayout = 0;
static int tmr_margin = 15;
struct imapx_wtd_params *wtd_params;

void imapx_enable_clock(void)
{
    uint32_t ret;

    ret = readl(IO_ADDRESS(0x21e0ac08));
    ret |= (0x1 << 4);
    writel(ret, IO_ADDRESS(0x21e0ac08));
}

void imapx_wtd_ctrl(struct imapx_wtd_params *params, uint32_t mode)
{   
    uint32_t ret = 0;

    ret = readl(params->io_base + WDT_CR);
    ret &= ~(0x1 << 0);
    ret |= mode << 0;
    writel(ret, params->io_base + WDT_CR);
}   

void imapx_wtd_mode(struct imapx_wtd_params *params, uint32_t mode)
{
    uint32_t ret = 0;

    ret = readl(params->io_base + WDT_CR);
    ret &= ~(1 << 1);
    ret |= mode << 1;
    writel(ret, params->io_base + WDT_CR);
}

void imapx_wtd_set_time(struct imapx_wtd_params *params, uint32_t value)
{  
    writel(value, params->io_base + WDT_TORR); 
}

void imapx_wtd_restart(struct imapx_wtd_params *params)
{  
    writel(WDT_RESTART_DEFAULT_VALUE, params->io_base + WDT_CRR); 
}

int imapx_wtd_set_heartbeat(struct imapx_wtd_params *params)
{
    int timeout = 0;
    int freq = WDT_MAX_FREQ_VALUE / 6;
    uint64_t count = (uint64_t)(params->ms) * freq;

    wtd_dbg("%s: %d\n", __func__, params->ms);
    while(count >>= 1)
        timeout++;
        
    timeout++;  //for greater than what you set time
    if (timeout < 16)
        timeout = 0;
    if (timeout > 31)
        timeout = 15;
    else 
        timeout -= 16;

    wtd_dbg("imapx_wtd_set_heartbeat %d\n", timeout);
    tmr_margin = timeout; 
    imapx_wtd_set_time(params, timeout);
    imapx_wtd_restart(params);
    imapx_wtd_ctrl(params, ENABLE);
    //printk("!!!!!!!!!!!!!! %x\n", readl(params->io_base + 0x00));
    //printk("!!!!!!!!!!!!!! %x\n", readl(params->io_base + 0x04));
    //printk("!!!!!!!!!!!!!! %x\n", readl(params->io_base + 0x08));
    //printk("!!!!!!!!!!!!!! %x\n", readl(params->io_base + 0x0c));
    return 0;
}

static int imapx_wtd_open(struct inode *inode, struct file *file)
{
    wtd_dbg("Enter %s\n", __func__);
    return 0;
}

static ssize_t imapx_wtd_write(struct file *file, const char __user *data,
        size_t len, loff_t *ppos)
{
    if (len) {
        if (!nowayout) {
            size_t i;

            /*  In case it was set long ago */
            expect_close = 0;

            for (i = 0; i != len; i++) {
                char c;

                if (get_user(c, data + i))
                    return -EFAULT;
                if (c == 'V')
                    expect_close = 42;
            }
        }
    }
    return len;
}

static int imapx_wtd_release(struct inode *inode, struct file *file)
{

    wtd_dbg("Enter %s\n", __func__);
    return 0;
}

static const struct watchdog_info imapx_wtd_ident = {
    .options          = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
    .firmware_version = 0,
    .identity         = "IMAPX800 Watchdog",
};

static long imapx_wtd_ioctl(struct file *file,    unsigned int cmd,
        unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int __user *p = argp;
    int new_margin;

   // wtd_dbg("%s: cmd is %d\n", __func__, cmd);

    switch (cmd) {
        case WDIOC_GETSUPPORT:
            wtd_dbg("WDIOC_GETSUPPORT\n");
            return copy_to_user(argp, &imapx_wtd_ident, 
                    sizeof(struct watchdog_info)) ? -EFAULT : 0;
        case WDIOC_GETSTATUS:
        case WDIOC_GETBOOTSTATUS:
            wtd_dbg("WDIOC_GETSTATUS\n");
            return put_user(0, p);
        case WDIOC_KEEPALIVE:
            wtd_dbg("WDIOC_KEEPALIVE\n");
            wtd_params->last_settime = imapx_gettime();
            imapx_wtd_restart(wtd_params);
            return 0;
        case WDIOC_SETTIMEOUT:
            wtd_dbg("WDIOC_SETTIMEOUT\n");
            if (get_user(new_margin, p))
                return -EFAULT;
            wtd_params->ms = new_margin;
            if (imapx_wtd_set_heartbeat(wtd_params))
                return -EINVAL;
            return put_user(tmr_margin, p);
        case WDIOC_GETTIMEOUT:
            wtd_dbg("WDIOC_GETTIMEOUT\n");
            return put_user(tmr_margin, p);
        case WDIOC_KILLTIME:
            wtd_dbg("WDIOC_KILLTIME\n");
            return hrtimer_cancel(&wtd_params->feed_timer);
        default:
            return -ENOTTY;
    }
}

static const struct file_operations imapx_wtd_fops = {
    .owner          = THIS_MODULE,
    .llseek         = no_llseek,
    .write          = imapx_wtd_write,
    .unlocked_ioctl = imapx_wtd_ioctl,
    .open           = imapx_wtd_open,
    .release        = imapx_wtd_release,
};

static struct miscdevice imapx_wtd_miscdev = {
    .minor      = WATCHDOG_MINOR,
    .name       = "watchdog",
    .fops       = &imapx_wtd_fops,
};

static long long __getns(void)
{
    ktime_t a;
    a = ktime_get();
    return a.tv64;
}

static int imapx_gettime(void)
{
    long long a = __getns();

    do_div(a, 1000000000ll);
    wtd_dbg("Watchdog time is %lld\n", a);
    return (int)a;
}
#if 0
static void imapx_reboot_work(struct work_struct *work)
{
    wtd_dbg("Enter %s\n", __func__);
    imap_reset(0);
}
#endif

static enum hrtimer_restart imapx_wtd_timer(struct hrtimer *handle)
{
    struct imapx_wtd_params *wtd = container_of(handle, 
            struct imapx_wtd_params, feed_timer);
    ktime_t period;
    int count;

    wtd_dbg("Enter %s\n", __func__);
    
    if ((wtd->bootmode == NORMAL_MODE) &&
            imapx_gettime() > KERNEL_KEEPWATCHDOG) {
        wtd_err("System start err, %d\n", imapx_gettime());
        //schedule_work(&wtd->rebootWork); 
        return HRTIMER_NORESTART;
    }

    wtd->last_settime = imapx_gettime();
    imapx_wtd_restart(wtd);
    period = ktime_set(WATCHDOG_PERIOD, 0);
    count = hrtimer_forward_now(handle, period);
    return HRTIMER_RESTART; 
}

static enum hrtimer_restart imapx_trace_timer(struct hrtimer *handle)
{
    struct imapx_wtd_params *wtd = container_of(handle,
            struct imapx_wtd_params, watch_timer);
    ktime_t period;
    int count;
    int time;

    time = imapx_gettime();
    wtd_dbg("Watchdog last settime is %d\n", wtd->last_settime);
    if (time >= (wtd->last_settime + WATCHDOG_TIMEOUT - 1) 
            && wtd->last_settime > 0) {
        wtd_err("Watchdog maybe restart the system\n");
        period = ktime_set(0, 200000000);
        count = hrtimer_forward_now(handle, period);
        return HRTIMER_RESTART;
    }

    period = ktime_set(0, 500000000);
    count = hrtimer_forward_now(handle, period);
    return HRTIMER_RESTART;
}

static irqreturn_t imapx_wtd_irq(int irqno, void *param)
{
    wtd_dbg("watchdog timer expired (irq)\n");
    return IRQ_HANDLED;
}

static int __devinit imapx_wtd_probe(struct platform_device *pdev)
{
    struct imapx_wtd_params *wtd;
    struct resource *mem, *irq;
    int ret = 0;
    
    module_power_on(SYSMGR_PWDT_BASE);
    module_reset(SYSMGR_PWDT_BASE, 0);

    printk("Imapx820 WatchDog init\n");
    wtd = kzalloc(sizeof(struct imapx_wtd_params), GFP_KERNEL);
    if (wtd == NULL) {
        wtd_err("wtd unable to alloc data mem");
        return -ENOMEM;
    }
    wtd->dev = &pdev->dev;

    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!mem) {
        wtd_err("wtd no MEM resouce\n");
        ret = -ENODEV;
        goto err0;
    }
    if(!request_mem_region(mem->start, resource_size(mem), pdev->name)){
        wtd_err("wtd request mem region err\n");
        ret = -EBUSY;
        goto err0;
    }
    wtd->io_base = ioremap(mem->start, resource_size(mem));
    if (!wtd->io_base) {
        wtd_err("wtd ioremap error\n");
        ret = -ENOMEM;
        goto err0;
    }
    irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (!irq) {
        wtd_err("wtd no irq resource\n");
        ret = -ENODEV;
        goto err1;
    }
    ret = request_irq(irq->start, imapx_wtd_irq, 0, pdev->name, pdev);
    if (ret != 0) {
        wtd_err("failed to request irq (%d)\n", ret);
        goto err1;
    }
    spin_lock_init(&wtd->lock);
    ret = misc_register(&imapx_wtd_miscdev);
    if (ret) {
        wtd_err("cannot register miscdev on minor=%d (%d)\n",
                WATCHDOG_MINOR, ret);
        goto err2;
    }
    if(strstr(boot_command_line, "charger"))
        wtd->bootmode = CHARGER_MODE;
    else if (strstr(boot_command_line, "recovery"))
        wtd->bootmode = RECOVERY_MODE;
    else {
        wtd_dbg("bootmode in normal mode\n");
        wtd->bootmode = NORMAL_MODE;
    }
    //INIT_WORK(&wtd->rebootWork, imapx_reboot_work);

    imapx_wtd_ctrl(wtd, DISABLE);
    imapx_enable_clock();
    imapx_wtd_mode(wtd, WDT_SYS_RST);
    wtd->ms = item_integer("board.wdt", 0);
    if (wtd->ms < 0) {
        wtd_err("Do not exist wtd.timer");
        wtd->ms = WATCHDOG_TIMEOUT;   //default value (unit s)
        //ret = -ENODEV;
        //goto err2;
    }
    
    imapx_wtd_set_heartbeat(wtd);
#ifdef AUTO_START_WATCHDOG
    hrtimer_init(&wtd->feed_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    wtd->feed_timer.function = imapx_wtd_timer;
    hrtimer_start(&wtd->feed_timer, ktime_set(WATCHDOG_PERIOD, 0), HRTIMER_MODE_REL);
#endif
#ifdef TRACE_WATCHDOG
    if (wtd->bootmode == NORMAL_MODE) {
        hrtimer_init(&wtd->watch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        wtd->watch_timer.function = imapx_trace_timer;
        hrtimer_start(&wtd->watch_timer, ktime_set(0, 500000000), HRTIMER_MODE_REL);
    }
#endif
    dev_set_drvdata(&pdev->dev, wtd);
    wtd_params = wtd; 

    return 0;
err2:
    free_irq(irq->start, pdev);
err1:
    iounmap(wtd->io_base);
err0:
    kfree(wtd);
    return ret;
}

static int __devexit imapx_wtd_remove(struct platform_device *dev)
{
    struct imapx_wtd_params *wtd = dev_get_drvdata(&dev->dev);
    int ret;
#ifdef TRACE_WATCHDOG
    ret = hrtimer_cancel(&wtd->watch_timer);
    if (ret) 
        wtd_err("The timer was still in use...\n");
#endif
    free_irq(wtd->irq, dev);
    iounmap(wtd->io_base);
    misc_deregister(&imapx_wtd_miscdev);
    
    kfree(wtd);
    return 0;
}

static void imapx_wtd_shutdown(struct platform_device *dev)
{
    struct imapx_wtd_params *wtd = dev_get_drvdata(&dev->dev);
    
    spin_lock(&wtd->lock);
    imapx_wtd_ctrl(wtd, DISABLE);
    spin_unlock(&wtd->lock);
}

#ifdef CONFIG_PM
static int imapx_wtd_suspend(struct platform_device *dev, pm_message_t state)
{
    return 0;
}

static int imapx_wtd_resume(struct platform_device *dev)
{
    struct imapx_wtd_params *wtd = dev_get_drvdata(&dev->dev);

    wtd_dbg("Enter %s\n", __func__);
    module_power_on(SYSMGR_PWDT_BASE);
    module_reset(SYSMGR_PWDT_BASE, 0);

    imapx_wtd_ctrl(wtd, DISABLE);
    imapx_enable_clock();
    imapx_wtd_mode(wtd, WDT_SYS_RST);
    imapx_wtd_set_heartbeat(wtd);

    return 0;
}
#else
#define s3c2410wdt_suspend NULL
#define s3c2410wdt_resume  NULL
#endif /*  CONFIG_PM */

static struct platform_driver imapx_wtd_driver = {
    .probe      = imapx_wtd_probe,   
    .remove     = __devexit_p(imapx_wtd_remove),
    .shutdown   = imapx_wtd_shutdown,
    .suspend    = imapx_wtd_suspend,
    .resume     = imapx_wtd_resume,
    .driver     = {
        .owner  = THIS_MODULE,
        .name   = "imap-wdt",
    },
};

static __init int imapx_wtd_init(void)
{
    if (item_integer("board.wdt", 0) == 0)
        return -1;

    return platform_driver_register(&imapx_wtd_driver);
}

static __exit void imapx_wtd_exit(void)
{
    platform_driver_unregister(&imapx_wtd_driver);
}

module_init(imapx_wtd_init);
module_exit(imapx_wtd_exit);

MODULE_AUTHOR("sun");
MODULE_DESCRIPTION("IMAPX800 Watchdog Device Driver");
MODULE_LICENSE("GPL");
