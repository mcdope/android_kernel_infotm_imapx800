#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <stdarg.h>
#include <mach/imap-iomap.h>
#include <mach/pad.h>
#include <mach/items.h>
#include <mach/power-gate.h>


#define PAD_SYSM_VA  (IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9000))

#define	PADS_SDIO_IN_EN	(PAD_SYSM_VA + 0x08)	/* enable input */
#define PADS_SDIO_PULL  (PAD_SYSM_VA + 0x0c)
#define PAD_IO_PULLEN	(PAD_SYSM_VA + 0x14)
#define PAD_IO_GPIOEN	(PAD_SYSM_VA + 0x64)
#define PAD_IO_GPDIR	(PAD_SYSM_VA + 0xB4)
#define PAD_IO_ODAT	(PAD_SYSM_VA + 0x104)
#define PAD_IO_IDAT	(PAD_SYSM_VA + 0x154)
#define PAD_IO_PULLUP	(PAD_SYSM_VA + 0x1A4)

static void __iomem* gpio_base;

void __init imap_init_gpio(void)
{
	gpio_base = ioremap_nocache(IMAP_GPIO_BASE,0x4400);
    module_power_on(SYSMGR_GPIO_BASE);
}

#define GPIO_RDAT(n)    (gpio_base + (n)*(0x40) + 0x0)
#define GPIO_WDAT(n)    (gpio_base + (n)*(0x40) + 0x4)
#define GPIO_DIR(n)     (gpio_base + (n)*(0x40) + 0x8)
#define GPIO_INTMSK(n)  (gpio_base + (n)*(0x40) + 0xC)
#define GPIO_INTGMSK(n) (gpio_base + (n)*(0x40) + 0x10)
#define GPIO_INTPEND(n) (gpio_base + (n)*(0x40) + 0x14)
#define GPIO_INTTYPE(n) (gpio_base + (n)*(0x40) + 0x18)
#define GPIO_FILTER(n)  (gpio_base + (n)*(0x40) + 0x1C)
#define GPIO_CLKDIV(n)  (gpio_base + (n)*(0x40) + 0x20)
#define GPIO_INTPENDGLB(n) (gpio_base + (n)*4 + 0x4000)

#define RTC_SYSM_VA	(IO_ADDRESS(IMAP_SYSMGR_BASE + 0x9c00))
#define RTC_GPIO_DIR	(RTC_SYSM_VA + 0x4c)
#define RTC_GPIO_ODAT1	(RTC_SYSM_VA + 0x50)
#define RTC_GPIO_ODAT2	(RTC_SYSM_VA + 0x7c)
#define RTC_GPIO_IDAT	(RTC_SYSM_VA + 0x58)
#define RTC_GPIO_PULLDOWNEN	(RTC_SYSM_VA + 0x54)

//#define PAD_DEBUG
#ifdef PAD_DEBUG
#define pad_dbg(x...)	printk(KERN_ERR "PAD_INFO:" x)
#else
#define pad_dbg(x...)	
#endif

static spinlock_t pad_lock = SPIN_LOCK_UNLOCKED;

static int pad_dir_x820(uint32_t index, int dir)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int real_idx;
	int reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx < 0 || real_idx > 159 || dir < 0 || dir > 1)
	{
		return -1;
	}

	spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_GPDIR + cnt * 4);
	if(dir == DIRECTION_INPUT)/*input*/
	{
		tmp |= 1 << num;
	}
	else/*output*/
	{
		tmp &= ~(1 << num);
	}
	writel(tmp,(PAD_IO_GPDIR + cnt * 4));

	spin_unlock(&pad_lock);

	//pad_dbg("pad %d direction set to %s\n", index, dir?"'input'":"'output'");

	return 0;
}

static int pad_dir_i15(uint32_t index, int dir)
{
	int real_idx;
	int reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 136 || dir < 0 || dir > 1)
        return -1;

    spin_lock(&pad_lock);
    writel(!dir, GPIO_DIR(real_idx));
    spin_unlock(&pad_lock);

    //pad_dbg("pad %d direction set to %s\n", index, !dir?"'output'":"'input'");

    return 0;
}

static int rtc_gpio_dir_x820(uint32_t index, int dir)
{
    int real_idx;
    volatile unsigned int tmp = 0;
    int reverse = index & 0x80000000;

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 3 || dir < 0 || dir > 1)
	return -1;

    spin_lock(&pad_lock);
    tmp = readl(RTC_GPIO_DIR);
    if(dir)
	tmp |= 1 << real_idx;
    else
	tmp &= ~(1 << real_idx);
    writel(tmp, RTC_GPIO_DIR);
    spin_unlock(&pad_lock);
    pad_dbg("RTCGPIO-%d direction set to %d\n", real_idx, dir);

    return 0;
}

static int rtc_gpio_dir_i15(uint32_t index, int dir)
{
    int real_idx;
    volatile unsigned int tmp = 0;
    int reverse = index & 0x80000000;

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 5 || dir < 0 || dir > 1)
	return -1;

    spin_lock(&pad_lock);
    tmp = readl(RTC_GPIO_DIR);
    if(dir)
	tmp |= 1 << real_idx;
    else
	tmp &= ~(1 << real_idx);
    writel(tmp, RTC_GPIO_DIR);
    spin_unlock(&pad_lock);
    pad_dbg("RTCGPIO-%d direction set to %d\n", real_idx, dir);

    return 0;
}

static int pad_dir(uint32_t index,enum pad_dir dir)
{
    int ret = -1;

    if((item_exist("board.cpu") == 0) ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = pad_dir_x820(index, dir);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = pad_dir_i15(index, dir);
    }

    return ret;
}

int rtc_gpio_dir(uint32_t index, int dir)
{
    int ret = -1;

    if((item_exist("board.cpu") == 0) ||
	    item_equal("board.cpu", "imapx820", 0)) {
	ret = rtc_gpio_dir_x820(index, dir);
    }else if(item_equal("board.cpu", "i15", 0)) {
	ret = rtc_gpio_dir_i15(index, dir);
    }

    return ret;
}
EXPORT_SYMBOL(rtc_gpio_dir);

static int pad_mode(uint32_t index,enum pad_mode mode)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int real_idx;
	int reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx < 0 || real_idx > 159 || mode < 0 || mode > 1)
	{
		return -1;
	}

	spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_GPIOEN + cnt * 4);
	if(mode == MODE_GPIO)/*gpio*/
	{
		tmp |= 1 << num;
	}
	else/*funcion*/
	{
		tmp &= ~(1 << num);
	}
	writel(tmp,(PAD_IO_GPIOEN + cnt * 4));

	spin_unlock(&pad_lock);

	//pad_dbg("pad %d mode set to %s\n", index, mode?"'gpio'":"'funcion'");

	return 0;
}

static int pad_pull(uint32_t index,enum pad_pull pullen)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int real_idx;
	int reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx < 0 || real_idx > 159 || pullen < 0 || pullen > 1)
	{
		return -1;
	}

	spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_PULLEN + cnt * 4);
	if(pullen == PULL_ENABLE)/*pull enable*/
	{
		tmp |= 1 << num;
	}
	else/*pull disable*/
	{
		tmp &= ~(1 << num);
	}
	writel(tmp,(PAD_IO_PULLEN + cnt * 4));

	spin_unlock(&pad_lock);

	//pad_dbg("pad %d pull set to %s\n", index, pullen?"'enable'":"'disable'");

	return 0;
}

static int pad_pull_up(uint32_t index,bool pullup)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int real_idx;
	int reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx < 0 || real_idx > 159)
		return -1;

	spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_PULLUP + cnt * 4);
	if(pullup)/*pull up*/
	{
		tmp |= 1 << num;
	}
	else/*pull down*/
	{
		tmp &= ~(1 << num);
	}
	writel(tmp,(PAD_IO_PULLUP + cnt * 4));

	spin_unlock(&pad_lock);

	//pad_dbg("pad %d pull set to %s\n", index, pullup?"'pull-up'":"'pull-down'");

	return 0;
}

static int pad_outdat_x820(uint32_t index,enum pad_outdat outdat)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int real_idx;
	uint32_t reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx > 159 || outdat < 0 || outdat > 1)
	{
		return -1;
	}

	spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_ODAT + cnt * 4);
	if(outdat == OUTPUT_1)/*output 1*/
	{
		if(!reverse)
			tmp |= 1 << num;
		else
			tmp &= ~(1 << num);
	}
	else/*output 0*/
	{
		if(!reverse)
			tmp &= ~(1 << num);
		else
			tmp |= 1 << num;
	}
	writel(tmp,(PAD_IO_ODAT + cnt * 4));

	spin_unlock(&pad_lock);

	pad_dbg("pad %d output data set to 0x%x\n", real_idx, readl(PAD_IO_ODAT + cnt * 4));

	return 0;
}

static int pad_outdat_i15(uint32_t index, int outdat)
{
    uint32_t reverse = index & 0x80000000;
	int real_idx;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	//printk("real_idx == %d\n", real_idx);

    if(real_idx > 136 || outdat < 0 || outdat > 1)
        return -1;

    spin_lock(&pad_lock);
    if(reverse)
        writel(!outdat, GPIO_WDAT(real_idx));
    else
        writel(outdat, GPIO_WDAT(real_idx));

    spin_unlock(&pad_lock);

    //printk("pad %d output data set to 0x%x\n", real_idx, readl(GPIO_WDAT(real_idx)));

    return 0;
}

/*if pad need reverse, we sign the pad with its index OR (1 << 31)*/
static int pad_outdat(uint32_t index,enum pad_outdat outdat)
{
    int ret = -1;

    if((item_exist("board.cpu") == 0) ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = pad_outdat_x820(index, outdat);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = pad_outdat_i15(index, outdat);
    }

    return ret;
}

static int rtc_gpio_outdat_x820(uint32_t index, int outdat)
{
    int real_idx;
    uint32_t reverse = index & 0x80000000;
    volatile unsigned int tmp = 0;

    if(!reverse)
	real_idx = index;              
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 3 || outdat < 0 || outdat > 1)
	return -1;

    spin_lock(&pad_lock);

    if(reverse) {
	if(real_idx != 0) {
	    tmp = readl(RTC_GPIO_ODAT1);
	    if(outdat)
		tmp &= ~(1<<real_idx);
	    else
		tmp |= 1<<real_idx;
	    writel(tmp, RTC_GPIO_ODAT1);
	}else {
	    tmp = readl(RTC_GPIO_ODAT2);
	    if(outdat)
		tmp &= ~(0x1);
	    else
		tmp |= 0x1;
	    writel(tmp, RTC_GPIO_ODAT2);
	}
    }else {
	if(real_idx != 0) {
	    tmp = readl(RTC_GPIO_ODAT1);
	    if(outdat)
		tmp |= 1<<real_idx;
	    else
		tmp &= ~(1<<real_idx);
	    writel(tmp, RTC_GPIO_ODAT1);
	}else {
	    tmp = readl(RTC_GPIO_ODAT2);
	    if(outdat)
		tmp |= 0x1;
	    else
		tmp &= ~(0x1);
	    writel(tmp, RTC_GPIO_ODAT2);
	}
    }
    spin_unlock(&pad_lock);
    pad_dbg("RTCGPIO-%d wirte outdat %d, reverse=%d\n", real_idx, outdat, reverse);
    return 0;
}

static int rtc_gpio_outdat_i15(uint32_t index, int outdat)
{
    int real_idx;
    uint32_t reverse = index & 0x80000000;
    volatile unsigned int tmp = 0;

    if(!reverse)
	real_idx = index;              
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 5 || outdat < 0 || outdat > 1)
	return -1;

    spin_lock(&pad_lock);

    if(reverse) {
	if(real_idx != 0) {
	    tmp = readl(RTC_GPIO_ODAT1);
	    if(outdat)
		tmp &= ~(1<<real_idx);
	    else
		tmp |= 1<<real_idx;
	    writel(tmp, RTC_GPIO_ODAT1);
	}else {
	    tmp = readl(RTC_GPIO_ODAT2);
	    if(outdat)
		tmp &= ~(0x1);
	    else
		tmp |= 0x1;
	    writel(tmp, RTC_GPIO_ODAT2);
	}
    }else {
	if(real_idx != 0) {
	    tmp = readl(RTC_GPIO_ODAT1);
	    if(outdat)
		tmp |= 1<<real_idx;
	    else
		tmp &= ~(1<<real_idx);
	    writel(tmp, RTC_GPIO_ODAT1);
	}else {
	    tmp = readl(RTC_GPIO_ODAT2);
	    if(outdat)
		tmp |= 0x1;
	    else
		tmp &= ~(0x1);
	    writel(tmp, RTC_GPIO_ODAT2);
	}
    }
    spin_unlock(&pad_lock);
    pad_dbg("RTCGPIO-%d wirte outdat %d, reverse=%d\n", real_idx, outdat, reverse);
    return 0;
}

int rtc_gpio_outdat(uint32_t index, int outdat)
{
    int ret = -1;

    if((item_exist("board.cpu") == 0) ||
	    item_equal("board.cpu", "imapx820", 0)) {
	ret = rtc_gpio_outdat_x820(index, outdat);
    }else if(item_equal("board.cpu", "i15", 0)) {
	ret = rtc_gpio_outdat_i15(index, outdat);
    }

    return ret;
}
EXPORT_SYMBOL(rtc_gpio_outdat);

static int pad_indat_x820(uint32_t index)
{
	volatile unsigned int tmp = 0;
	int cnt = 0;
	int num = 0;
	int indat = 0;
	int real_idx;
	uint32_t reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

	if(real_idx > 159)
	{
		return -1;
	}

	//spin_lock(&pad_lock);

	cnt = real_idx / 8;
	num = real_idx % 8;

	tmp = readl(PAD_IO_IDAT + cnt * 4);

	//spin_unlock(&pad_lock);

	indat = (tmp & (1 << num)) >> num;

	return indat;
}

static int pad_indat_i15(uint32_t index)
{
    int indat = 0;
	int real_idx;
    uint32_t reverse = index & 0x80000000;

	if(!reverse)
		real_idx = index;
	else
		real_idx = (~(index-1)) & 0xff;

    if(real_idx > 136)
        return -1;

    //spin_lock(&pad_lock);
    indat = readl(GPIO_RDAT(real_idx));
    //spin_unlock(&pad_lock);

    return indat;
}

/*if pad need reverse, we sign the pad with its index OR (1 << 31)*/
static int pad_indat(uint32_t index)
{
    int ret = -1;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = pad_indat_x820(index);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = pad_indat_i15(index);
    }

    return ret;
}

static int rtc_gpio_indat_x820(uint32_t index)
{
    int indat;
    int real_idx;
    uint32_t reverse = index & 0x80000000; 

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 3)
	return -1;

    indat = readl(RTC_GPIO_IDAT);
    indat = (indat & (1<<real_idx)) >> real_idx;
    pad_dbg("RTCGPIO-%d indat=%d\n", real_idx, indat);

    return indat;
}

static int rtc_gpio_indat_i15(uint32_t index)
{
    int indat;
    int real_idx;
    uint32_t reverse = index & 0x80000000; 

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 5)
	return -1;

    indat = readl(RTC_GPIO_IDAT);
    indat = (indat & (1<<real_idx)) >> real_idx;
    pad_dbg("RTCGPIO-%d indat=%d\n", real_idx, indat);

    return indat;
}

int rtc_gpio_indat(uint32_t index)
{
    int ret = -1;

    if(item_exist("board.cpu") == 0 ||
	    item_equal("board.cpu", "imapx820", 0)){
	ret = rtc_gpio_indat_x820(index);
    }else if(item_equal("board.cpu", "i15", 0)){
	ret = rtc_gpio_indat_i15(index);
    }

    return ret;
}
EXPORT_SYMBOL(rtc_gpio_indat);

static int rtc_gpio_pulldown_en_x820(uint32_t index, int en)
{
    int real_idx;
    volatile unsigned int tmp = 0;
    uint32_t reverse = index & 0x80000000;

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 3)
	return -1;

    spin_lock(&pad_lock);
    tmp = readl(RTC_GPIO_PULLDOWNEN);
    if(en)
	tmp |= 1<<real_idx;
    else
	tmp &= ~(1<<real_idx);
    writel(tmp, RTC_GPIO_PULLDOWNEN);
    pad_dbg("RTCGPIO-%d pulldown en write %d\n", real_idx, en);
    return 0;
}

static int rtc_gpio_pulldown_en_i15(uint32_t index, int en)
{
    int real_idx;
    volatile unsigned int tmp = 0;
    uint32_t reverse = index & 0x80000000;

    if(!reverse)
	real_idx = index;
    else
	real_idx = (~(index-1)) & 0xff;

    if(real_idx < 0 || real_idx > 5)
	return -1;

    spin_lock(&pad_lock);
    tmp = readl(RTC_GPIO_PULLDOWNEN);
    if(en)
	tmp |= 1<<real_idx;
    else
	tmp &= ~(1<<real_idx);
    writel(tmp, RTC_GPIO_PULLDOWNEN);
    pad_dbg("RTCGPIO-%d pulldown en write %d\n", real_idx, en);
    return 0;
}


int rtc_gpio_pulldown_en(uint32_t index, int en)
{
    int ret = -1;

    if(item_exist("board.cpu") == 0 ||
	    item_equal("board.cpu", "imapx820", 0)){
	ret = rtc_gpio_pulldown_en_x820(index, en);
    }else if(item_equal("board.cpu", "i15", 0)){
	ret = rtc_gpio_pulldown_en_i15(index, en);
    }

    return ret;
}
EXPORT_SYMBOL(rtc_gpio_pulldown_en);

/*
 *Function:     imapx_pad_set_dir
 *Parameter:    dir,the i/o direction of pad:0 - output,1 - input;
 *		        num,the number of pads to be set;
 *		        ...,the pad index;
 *Description:	set discontinuous pads to input/output
 *Return:	    success:0
*		        fail:	-1
 */
int imapx_pad_set_dir(enum pad_dir dir,int num,...)
{
	va_list index;
	int count;
	int ret;

	va_start(index,num);

	for(count = 0;count < num;count++)
	{
		ret = pad_dir(va_arg(index,int),dir);
		if(ret < 0)
		{
			va_end(index);
			return -1;
		}
	}

	va_end(index);

	return 0;
}
EXPORT_SYMBOL(imapx_pad_set_dir);

/*
 *Function:	    imapx_pad_set_dir_range
 *Parameter:	dir,the i/o direction of pad:0 - output,1 - input;
 *		        start,the start pad index;
 *		        end,the end pad index;
 *Description:	set continuous pads to input/output
 *Return:	    success:0
 *		        fail:	-1
 */
int imapx_pad_set_dir_range(enum pad_dir dir,int start,int end)
{
	int i;
	int ret;

	for(i = start;i < (end+1);i++)
	{
		ret = pad_dir(i,dir);
		if(ret < 0)
			return -1;
	}

	return 0;
}

/*
 *Function:     imapx_pad_set_mode
 *Parameter:	mode,the mode of pad:0 - function,1 - gpio;
 *		        num,the number of pads to be set;
 *		        ...,the pad index;
 *Description:	set discontinuous pads mode to func/gpio
 *Return:	    success:0
 *		        fail:	-1
 */
int imapx_pad_set_mode(enum pad_mode mode,int num,...)
{
	va_list index;
	int count;
	int ret;

	va_start(index,num);

	for(count = 0;count < num;count++)
	{
		ret = pad_mode(va_arg(index,int),mode);
		if(ret < 0)
		{
			va_end(index);
			return -1;
		}
	}

	va_end(index);

	return 0;
}
EXPORT_SYMBOL(imapx_pad_set_mode);


/*
 *Function:	    imapx_pad_set_mode_range
 *Parameter:	mode,the mode of pad:0 - function,1 - gpio;
 *		        start,the start pad index;
 *		        end,the end pad index;
 *Description:	set continuous pads mode to func/gpio
 *Return:	    success:0
 *		        fail:	-1
 */
int imapx_pad_set_mode_range(enum pad_mode mode,int start,int end)
{
	int i;
	int ret;

	for(i = start;i < (end+1);i++)
	{
		ret = pad_mode(i,mode);
		if(ret < 0)
			return -1;
	}

	return 0;
}

/*
 *Function:	    imapx_pad_set_pull
 *Parameter:	index,the pad index;
 *		        pullen,enable/disable pull of pad:
 *                  0 - pull disable,1 - pull enable;
 *              pullup:0 - pull down,1 - pull up;
 *Description:	set pad to pull enable/disable,and if 
 *              pull enable,set out data to 0/1;
 *Return:	    success:0
 *		        fail:	-1
 */
int imapx_pad_set_pull(enum pad_index index,enum pad_pull pullen,bool pullup)
{
	int ret;

	ret = pad_pull(index,pullen);
	if(ret < 0)
		return -1;
	
	if(pullen)
	{	
		ret = pad_pull_up(index,pullup);
		if(ret < 0)
			return -1;
	}

	return 0;
}

/*
 *Function:	    imapx_pad_set_pull_range
 *Parameter:	pullen,enable/disable pull of pad:
 *                  0 - pull disable,1 - pull enable;
 *		        pullup:0 - pull down,1 - pull up;
 *		        start,the start pad index;
 *		        end,the end pad index;
 *Description:	set continuous pads to pull enable/disable,and 
 *              set their out data to 0/1 when pullen is 1;
 *Return:	    success:0
 *		        fail:   -1
 */
int imapx_pad_set_pull_range(int start,int end,enum pad_pull pullen,bool pullup)
{
	int i;
	int ret;

	for(i = start;i < (end+1);i++)
	{
		ret = imapx_pad_set_pull(i,pullen,pullup);
		if(ret < 0)
			return -1;
	}

	return 0;
}

/*
 *Function:	    imapx_pad_set_outdat
 *Parameter:	outdat,the output data of pad:
 *                  0 - output 0,1 - output 1;
 *		        num,the number of pads to be set;
 *		        ...,the pad index;
 *Description:	set discontinuous pads output 0/1
 *Return:	    success:0
 *		        fail:	-1
 */
int imapx_pad_set_outdat(enum pad_outdat outdat,int num,...)
{
	va_list index;
	int count;
	int ret;

	va_start(index,num);

	for(count = 0;count < num;count++)
	{
		ret = pad_outdat(va_arg(index,int),outdat);
		if(ret < 0)
		{
			va_end(index);
			return -1;
		}
	}

	va_end(index);

	return 0;
}
EXPORT_SYMBOL(imapx_pad_set_outdat);
/*
 *Function:     imapx_pad_set_outdat_range
 *Parameter:	outdat,the output data of pad:
 *                  0 - output 0,1 - output 1;
 *		        start,the start pad index;
 *		        end,the end pad index;
 *Description:	set continuous pads output 0/1
 *Return:       0 when success while 1 when fail
 */
int imapx_pad_set_outdat_range(enum pad_outdat outdat,int start,int end)
{
	int i;
	int ret;

	for(i = start;i < (end+1);i++)
	{
		ret = pad_outdat(i,outdat);
		if(ret < 0)
			return -1;
	}

	return 0;
}

/*
 *Function:	    imapx_pad_get_indat
 *Parameter:	index,pad index number;
 *Description:	get pad input data
 *Return:       0/1 when success while -1 when fail
 */
int imapx_pad_get_indat(uint32_t index)
{
	return pad_indat(index);
}

static int imapx_pad_cfg_x820(int pg, int pull)
{
	int case_num;
	
	case_num = readl(PAD_SYSM_VA); 

	if(pg < 0 || pg > IMAPX_DBG || case_num < 0 || case_num > 31)
		return -1;

	switch(pg)
	{
		case IMAPX_NAND:
			if(case_num == 1 || case_num == 3 || case_num == 8 || case_num == 10 || case_num == 12 || 
					case_num == 14 || case_num == 23 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else if(case_num == 7 || case_num == 9 || case_num == 11 || case_num == 13)
			{
				imapx_pad_set_mode_range(0,0,15);
				imapx_pad_set_mode_range(0,25,30);
				imapx_pad_set_mode(0,2,18,19);
				if(pull)
				{
					imapx_pad_set_pull_range(0,9,1,1);
					imapx_pad_set_pull_range(12,14,1,1);
					imapx_pad_set_pull_range(25,30,1,1);
					imapx_pad_set_pull(10,1,0);
					imapx_pad_set_pull(11,1,0);
					imapx_pad_set_pull(15,1,0);
					imapx_pad_set_pull(18,1,1);
					imapx_pad_set_pull(19,1,1);
				}
			}
			else
			{
				imapx_pad_set_mode_range(0,0,15);
				if(pull)
				{
					imapx_pad_set_pull_range(0,9,1,1);
					imapx_pad_set_pull_range(12,14,1,1);
					imapx_pad_set_pull(10,1,0);
					imapx_pad_set_pull(11,1,0);
					imapx_pad_set_pull(15,1,0);
				}
			}

			break;

		case IMAPX_SD0:
			if(case_num != 1 && case_num != 3 && case_num != 8 && case_num != 10 && case_num != 12 && 
					case_num != 14 && case_num != 23 && case_num != 28 && case_num != 31)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode_range(0,0,11);
				if(pull)
				{
					imapx_pad_set_pull_range(0,7,1,1);
					imapx_pad_set_pull(8,1,0);
					imapx_pad_set_pull(9,1,1);
					imapx_pad_set_pull(10,1,1);
				}
			}

			break;

		case IMAPX_SD1:

                        writel(0x7f, PADS_SDIO_IN_EN);
                        writel(0x6f, PADS_SDIO_PULL);
			imapx_pad_set_mode_range(0,31,37);
			if(pull)
			{
				imapx_pad_set_pull_range(31,34,1,1);
				imapx_pad_set_pull(35,1,1);
				imapx_pad_set_pull(36,1,1);
				imapx_pad_set_pull(37,1,1);
			}

			break;

		case IMAPX_SD2:
			if(case_num == 6 || case_num == 26 || case_num == 31)
			{
				return -1;
			}
			else if(case_num == 0 || case_num == 1 || case_num == 2 || case_num == 3 || case_num == 15 || 
					case_num == 17 || case_num == 21 || case_num == 22 || case_num == 23 || case_num == 27)
			{
				imapx_pad_set_mode_range(0,38,44);
				if(pull)
				{
					imapx_pad_set_pull_range(38,44,1,1);
				}
			}
			else if(case_num == 4 || case_num == 5 || case_num == 7 || case_num == 8 || case_num == 9 || case_num == 10 || case_num == 11 ||
					case_num == 12 || case_num == 13 || case_num == 14 || case_num == 18 || case_num == 24 || case_num == 25 || case_num == 28)
			{
				imapx_pad_set_mode_range(0,38,43);
				if(pull)
				{
					imapx_pad_set_pull_range(38,43,1,1);
				}
			}
			else if(case_num == 16 || case_num == 19 || case_num == 20 || case_num == 29 || case_num == 30)
			{
				imapx_pad_set_mode_range(0,38,49);
				if(pull)
				{
					imapx_pad_set_pull_range(38,49,1,1);
				}
			}

			break;

		case IMAPX_PHY:
			if(case_num == 2 || case_num == 3 || case_num == 7 || case_num == 8 || case_num == 9 || case_num == 10 || 
					case_num == 11 || case_num == 12 || case_num == 13 || case_num == 14 || case_num == 16)
			{
				return -1;
			}
			else if(case_num == 0 || case_num == 1 || case_num == 4 || case_num == 5 || case_num == 6 || case_num == 15 || case_num == 17 ||
					case_num == 18 || case_num == 19 || case_num == 20 || case_num == 21 || case_num == 28 || case_num == 31)
			{
				imapx_pad_set_mode_range(0,17,25);
				if(pull)
				{
					imapx_pad_set_pull(17,1,0);
					imapx_pad_set_pull(18,1,1);
					imapx_pad_set_pull(19,1,1);
					imapx_pad_set_pull_range(20,25,1,0);
				}
			}
			else if(case_num == 22 || case_num == 23 || case_num == 24 || case_num == 25 || 
					case_num == 26 || case_num == 27 || case_num == 29 || case_num == 30)
			{
				imapx_pad_set_mode_range(0,16,30);
				if(pull)
				{
					imapx_pad_set_pull(16,1,0);
					imapx_pad_set_pull(17,1,0);
					imapx_pad_set_pull(18,1,1);
					imapx_pad_set_pull(19,1,1);
					imapx_pad_set_pull_range(20,30,1,0);
				}
			}

			break;

		case IMAPX_TSP1GRP:
			if(case_num == 6 || case_num == 7 || case_num == 8 || case_num == 9 || case_num == 10 || case_num == 16 ||
					case_num == 19 || case_num == 20 || case_num == 26 || case_num == 29 || case_num == 30 || case_num == 31)
			{
				return -1;
			}
			else if(case_num == 0 || case_num == 1 || case_num == 2 || case_num == 3 || case_num == 4 || case_num == 5 || case_num == 11 || case_num == 12 ||
					case_num == 13 || case_num == 14 || case_num == 21 || case_num == 22 || case_num == 23 || case_num == 24 || case_num == 25 || case_num == 28)
			{
				imapx_pad_set_mode_range(0,45,49);
			}
			else if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode_range(0,137,142);
				imapx_pad_set_mode(0,1,131);
			}

			break;

		case IMAPX_TSP2GRP:
			if(case_num != 6 && case_num != 9 && case_num != 10 && case_num != 15 && case_num != 17 && 
					case_num != 18 && case_num != 26 && case_num != 27 && case_num != 31)
			{
				return -1;
			}
			else if(case_num == 6 || case_num == 26 || case_num == 31)
			{
				imapx_pad_set_mode_range(0,38,42);
			}
			else if(case_num == 9 || case_num == 10)
			{
				imapx_pad_set_mode_range(0,20,24);
			}
			else if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode_range(0,132,136);
			}

			break;

		case IMAPX_GPS:
			if(case_num == 5 || case_num == 6 || case_num == 15 || case_num == 17 || case_num == 18 || 
					case_num == 25 || case_num == 26 || case_num == 27 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode_range(0,50,53);
			}

			break;

		case IMAPX_I2C0:
			imapx_pad_set_mode(0,2,54,55);
			if(pull)
			{
				imapx_pad_set_pull(54,1,1);
				imapx_pad_set_pull(55,1,1);
			}

			break;

		case IMAPX_I2C1:
			imapx_pad_set_mode(0,2,56,57);

			break;

		case IMAPX_I2C2:
			if(case_num == 5 || case_num == 6 || case_num == 15 || case_num == 17 || case_num == 18 ||
					case_num == 25 || case_num == 26 || case_num == 27 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode(0,2,58,59);
			}

			break;

		case IMAPX_I2C3:
			if(case_num == 5 || case_num == 6 || case_num == 15 || case_num == 16 || case_num == 17 || case_num == 18 ||
					case_num == 24 || case_num == 25 || case_num == 26 || case_num == 27 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode(0,2,60,61);
			}

			break;

		case IMAPX_I2C4:
			if(case_num == 1 || case_num == 3 || case_num == 23 || case_num == 28 || case_num == 31)
			{
				imapx_pad_set_mode(0,2,12,13);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_I2C5:
			if(case_num == 3 || case_num == 12 || case_num == 14)
			{
				imapx_pad_set_mode(0,2,14,15);
			}
			else if(case_num == 2 || case_num == 5 || case_num == 9 || case_num == 10 || case_num == 28)
			{
				imapx_pad_set_mode(0,2,29,30);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_UART0:
			if(case_num == 27)
			{
				imapx_pad_set_mode(0,2,62,63);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_UART1:
			if(case_num == 2 || case_num == 3 || case_num == 7 || case_num == 8 || case_num == 9 ||
					case_num == 10 || case_num == 11 || case_num == 12 || case_num == 13 || case_num == 14)
			{
				imapx_pad_set_mode_range(0,62,65);
				imapx_pad_set_mode_range(0,74,77);
			}
			else if(case_num == 27)
			{
				imapx_pad_set_mode(0,2,64,65);
			}
			else
			{
				imapx_pad_set_mode_range(0,62,65);
			}

			break;

		case IMAPX_UART2:
			if(case_num == 5 || case_num == 6 || case_num == 7 || case_num == 8 ||
					case_num == 25 || case_num == 26 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode(0,2,66,67);
			}
			
			break;

		case IMAPX_UART3:

			imapx_pad_set_mode(0,2,68,69);
			if(pull)
			{
				imapx_pad_set_pull(68,1,1);
				imapx_pad_set_pull(69,1,1);
			}

			break;

		case IMAPX_PWM:
			if (pull)
				imapx_pad_set_mode(0,1,72);
			else
				imapx_pad_set_mode(1,1,72);

			break;

		case IMAPX_PWM1:
			if(pull)
				imapx_pad_set_mode(0,1,71);
			else
				imapx_pad_set_mode(1,1,71);

		case IMAPX_PWM2:
			if(pull)
			{
				if(case_num == 6 || case_num == 13 || case_num == 24 || case_num == 26 || case_num == 31)
					return -1;
				else if(case_num == 21)
					imapx_pad_set_mode(0,1,120);
				else
					imapx_pad_set_mode(0,1,70);
			}
			else
			{
				if(case_num == 6 || case_num == 13 || case_num == 24 || case_num == 26 || case_num == 31)
					return -1;
				else if(case_num == 21)
					imapx_pad_set_mode(1,1,120);
				else
					imapx_pad_set_mode(1,1,70);
			}

		case IMAPX_SYS:
			if(case_num == 21)
			{
				imapx_pad_set_mode(0,2,70,73);
			}
			else
			{
				imapx_pad_set_mode(0,1,73);
			}

			break;

		case IMAPX_OTG:

			imapx_pad_set_mode(0,1,78);

			break;

		case IMAPX_HDMI:

			imapx_pad_set_mode_range(0,79,81);

			break;

		case IMAPX_RGB0:
			if(case_num == 11 || case_num == 12)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode_range(0,82,109);
			}

			break;

		case IMAPX_RGB1:
			if(case_num != 5 && case_num != 25 && case_num != 28)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode_range(0,75,77);
				imapx_pad_set_mode_range(0,126,142);
			}

			break;

		case IMAPX_IIS0:
			if(case_num == 17)
			{
				return -1;
			}
			else if(case_num == 18)
			{
				imapx_pad_set_mode_range(0,110,117);
			}
			else
			{
				imapx_pad_set_mode_range(0,110,114);
			}

			break;

		case IMAPX_IIS1:
			if(case_num == 21)
			{
				imapx_pad_set_mode_range(0,121,125);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_PCM0:
			if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				return -1;
			}
			else if(case_num == 5 || case_num == 6 || case_num == 26 || case_num == 28 || case_num == 31)
			{
				imapx_pad_set_mode_range(0,115,117);
				imapx_pad_set_mode(0,2,119,120);
			}
			else
			{
				imapx_pad_set_mode_range(0,115,119);
			}

			break;

		case IMAPX_PCM1:
			if(case_num == 10 || case_num == 11 || case_num == 12)
			{
				imapx_pad_set_mode_range(0,101,103);
				imapx_pad_set_mode(0,2,86,94);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SSP0:
			if(case_num == 2 || case_num == 3 || case_num == 9 || case_num == 10)
			{
				imapx_pad_set_mode_range(0,25,28);
			}
			else if(case_num == 11 || case_num == 12)
			{
				imapx_pad_set_mode_range(0,82,85);
			}
			else if(case_num == 13 || case_num == 14)
			{
				imapx_pad_set_mode_range(0,128,131);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SSP1:
			if(case_num == 25 || case_num == 27)
			{
				return -1;
			}
			else if(case_num == 6 || case_num == 15 || case_num == 17 || case_num == 18 || case_num == 31)
			{
				imapx_pad_set_mode_range(0,27,30);
				if(pull)
				{
					imapx_pad_set_pull(27,1,1);
					imapx_pad_set_pull(28,1,1);
					imapx_pad_set_pull(29,1,0);
					imapx_pad_set_pull(30,1,1);
				}
			}
			else if(case_num == 4 || case_num == 21 || case_num == 22 || case_num == 23 || 
					case_num == 24 || case_num == 26 || case_num == 29 || case_num == 30)
			{
				imapx_pad_set_mode_range(0,74,77);
				if(pull)
				{
					imapx_pad_set_pull(74,1,1);
					imapx_pad_set_pull(75,1,1);
					imapx_pad_set_pull(76,1,0);
					imapx_pad_set_pull(77,1,1);
				}
			}
			else
			{
				imapx_pad_set_mode_range(0,121,124);
				if(pull)
				{
					imapx_pad_set_pull(121,1,1);
					imapx_pad_set_pull(122,1,1);
					imapx_pad_set_pull(123,1,0);
					imapx_pad_set_pull(124,1,1);
				}
			}

			break;

		case IMAPX_SSP2:
			if(case_num == 7 || case_num == 8 || case_num == 9 || case_num == 10)
			{
				imapx_pad_set_mode_range(0,46,49);
			}
			else if(case_num == 11 || case_num == 12)
			{
				imapx_pad_set_mode_range(0,90,93);
			}
			else if(case_num == 13 || case_num == 14)
			{
				imapx_pad_set_mode_range(0,132,135);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SPDIF:
			if(case_num == 0 || case_num == 1 || case_num == 2 || case_num == 3 || case_num == 19 || case_num == 20 || case_num == 25)
			{
				imapx_pad_set_mode(0,1,125);
			}
			else if(case_num == 5 || case_num == 6 || case_num == 26 || case_num == 28 || case_num == 31)
			{
				imapx_pad_set_mode(0,2,118,125);
			}
			else if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode(0,2,118,119);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_PS2_0:
			if(case_num == 5 || case_num == 6 || case_num == 25 ||
				       	case_num == 26 || case_num == 28 || case_num == 31)
			{
				imapx_pad_set_mode(0,2,60,61);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_PS2_1:
			if(case_num == 5 || case_num == 25 || case_num ==28)
			{
				imapx_pad_set_mode(0,2,58,59);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_KB:
			if(case_num == 7 || case_num == 8 || case_num == 11 || case_num == 12 || case_num == 13 || case_num == 14)
			{
				imapx_pad_set_mode_range(0,16,30);
			}
			else if(case_num == 6 || case_num == 26 || case_num == 31)
			{
				imapx_pad_set_mode_range(0,44,49);
				imapx_pad_set_mode_range(0,126,140);
				imapx_pad_set_mode(0,5,58,59,66,67,70);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SATA:
			if(case_num == 5 || case_num == 25 || case_num == 28)
			{
				imapx_pad_set_mode(0,1,74);
			}
			else if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode_range(0,74,77);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SIM0:
			if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode_range(0,46,51);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_SIM1:
			if(case_num == 15 || case_num == 17 || case_num == 18 || case_num == 27)
			{
				imapx_pad_set_mode(0,6,52,53,58,59,60,61);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_NOR:
			if(case_num == 16)
			{
				imapx_pad_set_mode_range(0,0,30);
				imapx_pad_set_mode(0,2,136,137);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_AC97:
			if(case_num == 17)
			{
				imapx_pad_set_mode_range(0,110,114);
			}
			else
			{
				return -1;
			}

			break;

		case IMAPX_CAM:
			if(case_num == 5 || case_num == 6 || case_num == 15 || case_num == 17 || case_num == 18 || 
					case_num == 25 || case_num == 26 || case_num == 27 || case_num == 28 || case_num == 31)
			{
				return -1;
			}
			else if(case_num == 0 || case_num == 1 || case_num == 2 || case_num == 3 || case_num == 4 || case_num == 7 || case_num == 8 || 
					case_num == 9 || case_num == 10 || case_num == 11 || case_num == 12 || case_num == 19 || case_num == 20 || case_num == 21)
			{
				imapx_pad_set_mode_range(0,126,142);
			}
			else if(case_num == 16 || case_num == 23 || case_num == 24 || case_num == 29 || case_num == 30)
			{
				imapx_pad_set_mode_range(0,126,135);
				imapx_pad_set_mode_range(0,138,142);
			}
			else if(case_num == 13 || case_num == 14)
			{
				imapx_pad_set_mode(0,1,138);
			}
			else if(case_num == 22)
			{
				imapx_pad_set_mode_range(0,126,133);
				imapx_pad_set_mode_range(0,138,142);
			}

			break;

		case IMAPX_BOOT:

			imapx_pad_set_mode_range(0,143,147);

			break;

		case IMAPX_DBG:
			if(case_num == 12)
			{
				imapx_pad_set_mode_range(0,148,155);
			}
			else
			{
				imapx_pad_set_mode_range(0,148,152);
				imapx_pad_set_mode(0,2,154,155);
			}

			break;

		case IMAPX_DMIC:
			if(case_num == 17)
			{
				return -1;
			}
			else
			{
				imapx_pad_set_mode(0,2,71,113);
			}

			break;

		default:
			return -1;
	}

	return 0;
}

static int imapx_pad_cfg_i15(int pg, int pull)
{
	int case_num = readl(PAD_SYSM_VA);

	if(pg < 0 || pg > IMAPX_DBG || case_num < 0 || case_num > 31)
		return -1;

	switch(pg) {
	case IMAPX_NAND:
		if(case_num == 0 || case_num == 2 || case_num == 4 || case_num == 6){
			imapx_pad_set_mode_range(0, 0, 7);
			imapx_pad_set_mode_range(0, 16, 23);
			if(pull){
				imapx_pad_set_pull_range(0, 7, 1, 1);
				imapx_pad_set_pull_range(16, 23, 1, 1);
			}
		}else if(case_num == 1 || case_num == 3 || case_num == 5 || case_num == 7 ||
				case_num == 9 || case_num == 12 || case_num == 14){
			imapx_pad_set_mode_range(0, 19, 23);
			if(pull){
				imapx_pad_set_pull_range(19, 23, 1, 1);
			}
		}else if(case_num == 8 || case_num == 10 || case_num == 11 || case_num == 13){
			imapx_pad_set_mode_range(0, 0, 23);
			if(pull){
				imapx_pad_set_pull_range(0, 7, 1, 1);
				imapx_pad_set_pull_range(8, 15, 1, 0);
				imapx_pad_set_pull_range(16, 23, 1, 1);
			}
		}
		break;

	case IMAPX_SD0:
		if(case_num == 1 || case_num == 3 || case_num == 5 || case_num == 7 ||
				case_num == 9 || case_num == 12 || case_num == 14){
			imapx_pad_set_mode_range(0, 0, 7);
			imapx_pad_set_mode_range(0, 16, 18);
			if(pull){
				imapx_pad_set_pull_range(0, 7, 1, 1);
				imapx_pad_set_pull_range(16, 18, 1, 1);
			}
		}else{
			return -1;
		}
		break;

	case IMAPX_SD1:
		 writel(0x7f, PADS_SDIO_IN_EN);
		 writel(0x6f, PADS_SDIO_PULL);
		 imapx_pad_set_mode_range(0, 29, 35);
		 if(pull){
			 imapx_pad_set_pull_range(29, 35, 1, 1);
		 }
		 break;

	case IMAPX_SD2:
		 imapx_pad_set_mode_range(0, 36, 41);
		 if(pull){
			 imapx_pad_set_pull_range(36, 41, 1, 1);
		 }
		 break;

	case IMAPX_PHY:
		 if(case_num == 8 || case_num == 10 || case_num == 11 || case_num == 13){
			 imapx_pad_set_mode(0, 1, 24);
			 if(pull){
				 imapx_pad_set_pull(24, 1, 0);
			 }
		 }else{
			 imapx_pad_set_mode_range(0, 8, 15);
			 imapx_pad_set_mode(0, 1, 24);
			 if(pull){
				 imapx_pad_set_pull_range(8, 15, 1, 0);
				 imapx_pad_set_pull(24, 1, 0);
			 }
		 }
		 break;

	case IMAPX_I2C0:
		 imapx_pad_set_mode(0, 2, 43, 44);
		 if(pull){
			 imapx_pad_set_pull_range(43, 44, 1, 1);
		 }
		 break;

	case IMAPX_I2C1:
		 imapx_pad_set_mode(0, 2, 45, 46);
		 if(pull){
			 imapx_pad_set_pull_range(45, 46, 1, 1);
		 }
		 break;

	case IMAPX_I2C2:
		 imapx_pad_set_mode(0, 2, 47, 48);
		 if(pull){
			 imapx_pad_set_pull_range(47, 48, 1, 1);
		 }
		 break;

	case IMAPX_I2C3:
		 if(case_num == 2){
			 return -1;
		 }else{
			 imapx_pad_set_mode(0, 2, 128, 129);
			 if(pull){
				 imapx_pad_set_pull_range(128, 129, 1, 1);
			 }
		 }
		 break;
	
	case IMAPX_UART0:
		 imapx_pad_set_mode(0, 2, 25, 26);
		 if(pull){
			 imapx_pad_set_pull_range(25, 26, 1, 0);
		 }
		 break;

	case IMAPX_UART1:
		 imapx_pad_set_mode_range(0, 49, 52);
		 if(pull){
			 imapx_pad_set_pull_range(49, 52, 1, 1);
		 }
		 break;

	case IMAPX_UART2:
		 imapx_pad_set_mode(0, 2, 53, 54);
		 if(pull){
			 imapx_pad_set_pull_range(53, 54, 1, 1);
		 }
		 break;

	case IMAPX_UART3:
		 imapx_pad_set_mode(0, 2, 55, 56);
		 if(pull){
			 imapx_pad_set_pull_range(55, 56, 1, 1);
		 }
		 break;

	case IMAPX_PWM:/*PWM0*/
		 if(pull){
			 imapx_pad_set_mode(0, 1, 59);
			 imapx_pad_set_pull(59, 1, 0);
		 }else{
			 imapx_pad_set_mode(1, 1, 59);
		 }
		 break;

	case IMAPX_PWM1:
		 if(pull){
			 imapx_pad_set_mode(0, 1, 58);
			 imapx_pad_set_pull(58, 1, 0);
		 }else{
			 imapx_pad_set_mode(1, 1, 58);
		 }
		 break;

	case IMAPX_PWM2:
		 if(pull){
			 imapx_pad_set_mode(0, 1, 57);
			 imapx_pad_set_pull(57, 1, 0);
		 }else{
			 imapx_pad_set_mode(1, 1, 57);
		 }
		 break;

	case IMAPX_SYS:
		 imapx_pad_set_mode(0, 1, 60);
		 if(pull){
			 imapx_pad_set_pull(60, 1, 0);
		 }
		 break;

	case IMAPX_OTG:
		 imapx_pad_set_mode(0, 1, 61);
		 if(pull){
			 imapx_pad_set_pull(61, 1, 0);
		 }
		 break;

	case IMAPX_HDMI:
		 imapx_pad_set_mode_range(0, 62, 64);
		 if(pull){
			 imapx_pad_set_pull_range(62, 64, 1, 1);
		 }
		 break;

	case IMAPX_RGB0:
		 imapx_pad_set_mode_range(0, 66, 93);
		 if(pull){
			 imapx_pad_set_pull_range(66, 75, 1, 0);
			 imapx_pad_set_pull_range(76, 93, 1, 1);
		 }
		 break;

	case IMAPX_IIS0:
		 if(case_num == 2 || case_num == 3 || case_num == 6 || case_num == 7){
			 return -1;
		 }else{
			 imapx_pad_set_mode_range(0, 95, 99);
			 if(pull){
				 imapx_pad_set_pull_range(95, 99, 1, 0);
			 }
		 }
		 break;

	case IMAPX_PCM0:
		 imapx_pad_set_mode_range(0, 100, 104);
		 if(pull){
			 imapx_pad_set_pull_range(100, 104, 1, 0);
		 }
		 break;

	case IMAPX_PCM1:
		 if(case_num == 11 || case_num == 12 || case_num == 13 || case_num == 14){
			 imapx_pad_set_mode_range(0, 105, 109);
			 if(pull){
				 imapx_pad_set_pull_range(105, 109, 1, 0);
			 }
		 }else{
			 return -1;
		 }
		 break;

	case IMAPX_SSP0:
		 if(case_num == 4 || case_num == 5 || case_num == 6 || case_num == 7){
			 return -1;
		 }else{
			 imapx_pad_set_mode_range(0, 110, 113);
			 if(pull){
				 imapx_pad_set_pull_range(110, 113, 1, 0);
			 }
		 }
		 break;

	case IMAPX_SSP1:
		 if(case_num == 11 || case_num == 12 || case_num == 13 || case_num == 14){
			 imapx_pad_set_mode_range(0, 114, 117);
			 if(pull){
				 imapx_pad_set_pull_range(114, 117, 1, 0);
			 }
		 }else{
			 return -1;
		 }
		 break;

	case IMAPX_PS2_0:
		 if(case_num == 4 || case_num == 5 || case_num == 6 || case_num == 7){
			 imapx_pad_set_mode(0, 2, 110, 111);
			 if(pull){
				 imapx_pad_set_pull_range(110, 111, 1, 0);
			 }
		 }else{
			 return -1;
		 }
		 break;

	case IMAPX_CAM:
		 imapx_pad_set_mode_range(0, 120, 127);
		 imapx_pad_set_mode_range(0, 130, 134);
		 if(pull){
			 imapx_pad_set_pull_range(120, 127, 1, 0);
			 imapx_pad_set_pull_range(130, 134, 1, 1);
		 }
		 break;

	case IMAPX_DMIC:
		 if(case_num == 2 || case_num == 3 || case_num == 6 || case_num == 7){
			 imapx_pad_set_mode(0, 2, 99, 113);
			 if(pull){
				 imapx_pad_set_pull(99, 1, 0);
				 imapx_pad_set_pull(113, 1, 0);
			 }
		 }else{
			 return -1;
		 }
		 break;

	case IMAPX_PWMA:
		 if(case_num == 2 || case_num == 3 || case_num == 6 || case_num == 7){
			 imapx_pad_set_mode_range(0, 95, 98);
			 if(pull){
				 imapx_pad_set_pull_range(95, 98, 1, 0);
			 }
		 }else{
			 return -1;
		 }
		 break;

	case IMAPX_EXTCLK:
		 imapx_pad_set_mode(0, 1, 65);
		 if(pull){
			 imapx_pad_set_pull(65, 1, 0);
		 }

	default:
		 return -1;
	}

	return 0;
}

/*               
 *Function:     imapx_pad_cfg
 *Parameter:    pg,defined module description;
 *              pull,if consider the pad pull:
 *                  0 - not consider,1 - consider;
 *Description:  config pad for module
 *Return:       sucessful: 0
 *              error:    -1
 */                      
int imapx_pad_cfg(enum imapx_padgroup pg,int pull)
{
	int ret = -1;

	if((item_exist("board.cpu") == 0) ||
			item_equal("board.cpu", "imapx820", 0)) {
		ret = imapx_pad_cfg_x820(pg, pull);
	}else if(item_equal("board.cpu", "i15", 0)) {
		ret = imapx_pad_cfg_i15(pg, pull);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(imapx_pad_cfg);

/*
 *Function:	    imapx_pad_irqgroup_cfg
 *Parameter:	none;
 *Description:	config pads of irq func, GPIO0~GPIO25, for each case;
 *Return:	    success:0
 *		        fail:	-1
 */
/*this func is not necessary for i15*/
int imapx_pad_irqgroup_cfg(void)
{
	int case_num;

	case_num = readl(PAD_SYSM_VA);

	if(case_num < 0 || case_num > 31)
		return -1;

	switch(case_num)
	{
		case 0:
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode(0,3,16,120,153);
			break;

		case 1:
			imapx_pad_set_mode_range(0,14,16);
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode(0,3,11,120,153);
			break;

		case 2:
			imapx_pad_set_mode_range(0,16,24);
			imapx_pad_set_mode(0,2,120,153);
			break;

		case 3:
			imapx_pad_set_mode_range(0,16,24);
			imapx_pad_set_mode(0,5,11,29,30,120,153);
			break;

		case 4:
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode_range(0,120,125);
			imapx_pad_set_mode(0,3,16,44,153);
			break;

		case 5:
			imapx_pad_set_mode_range(0,26,28);
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode(0,5,16,44,66,67,153);
			break;

		case 6:
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode_range(0,121,124);
			imapx_pad_set_mode(0,6,16,26,43,141,142,153);
			break;

		case 7:
			imapx_pad_set_mode(0,7,44,45,66,67,120,125,153);
			break;

		case 8:
			imapx_pad_set_mode_range(0,11,15);
			imapx_pad_set_mode(0,7,44,45,66,67,120,125,153);
			break;

		case 9:
			imapx_pad_set_mode_range(0,16,19);
			imapx_pad_set_mode(0,5,44,45,120,125,153);
			break;

		case 10:
			imapx_pad_set_mode_range(0,11,19);
			imapx_pad_set_mode(0,5,44,45,120,125,153);
			break;

		case 11:
			imapx_pad_set_mode_range(0,86,89);
			imapx_pad_set_mode_range(0,94,109);
			imapx_pad_set_mode(0,4,44,120,125,153);
			break;

		case 12:
			imapx_pad_set_mode_range(0,11,13);
			imapx_pad_set_mode_range(0,86,89);
			imapx_pad_set_mode_range(0,94,109);
			imapx_pad_set_mode(0,3,44,120,125);
			break;

		case 13:
			imapx_pad_set_mode_range(0,125,127);
			imapx_pad_set_mode_range(0,139,142);
			imapx_pad_set_mode(0,6,44,70,120,136,137,153);
			break;

		case 14:
			imapx_pad_set_mode_range(0,11,13);
			imapx_pad_set_mode_range(0,125,127);
			imapx_pad_set_mode_range(0,139,142);
			imapx_pad_set_mode(0,5,44,120,136,137,153);
			break;

		case 15:
			imapx_pad_set_mode_range(0,115,117);
			imapx_pad_set_mode_range(0,120,130);
			imapx_pad_set_mode(0,4,16,26,45,153);
			break;

		case 16:
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode(0,5,60,61,120,125,153);
			break;

		case 17:
			imapx_pad_set_mode_range(0,115,117);
			imapx_pad_set_mode_range(0,120,130);
			imapx_pad_set_mode(0,4,16,26,45,153);
			break;

		case 18:
			imapx_pad_set_mode_range(0,120,130);
			imapx_pad_set_mode(0,5,16,26,44,45,153);
			break;

		case 19:
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode(0,3,16,120,153);
			break;

		case 20:
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode(0,3,16,120,153);
			break;

		case 21:
			imapx_pad_set_mode_range(0,26,30);
			imapx_pad_set_mode(0,2,16,153);
			break;

		case 22:
			imapx_pad_set_mode_range(0,123,125);
			imapx_pad_set_mode_range(0,134,137);
			imapx_pad_set_mode(0,2,120,153);
			break;

		case 23:
			imapx_pad_set_mode_range(0,123,125);
			imapx_pad_set_mode(0,7,11,14,15,120,136,137,153);
			break;

		case 24:
			imapx_pad_set_mode_range(0,123,125);
			imapx_pad_set_mode(0,8,44,60,61,70,120,136,137,153);
			break;

		case 25:
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode(0,7,44,66,67,120,123,124,153);
			break;

		case 26:
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode(0,6,43,123,124,141,142,153);
			break;

		case 27:
			imapx_pad_set_mode_range(0,115,117);
			imapx_pad_set_mode_range(0,123,130);
			imapx_pad_set_mode(0,3,45,120,153);
			break;

		case 28:
			imapx_pad_set_mode_range(0,14,16);
			imapx_pad_set_mode_range(0,26,28);
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode(0,5,11,44,66,67,153);
			break;

		case 29:
			imapx_pad_set_mode_range(0,123,125);
			imapx_pad_set_mode(0,4,120,136,137,153);
			break;

		case 30:
			imapx_pad_set_mode_range(0,123,125);
			imapx_pad_set_mode(0,7,120,136,137,153);
			break;

		case 31:
			imapx_pad_set_mode_range(0,14,16);
			imapx_pad_set_mode_range(0,50,53);
			imapx_pad_set_mode_range(0,74,77);
			imapx_pad_set_mode_range(0,121,124);
			imapx_pad_set_mode(0,6,11,26,43,141,142,153);
			break;
	}

	return 0;
}

static int imapx_pad_index_x820(const char *name)
{
	int index = 0;
	int len = 0;
//	int ok = 0;
	int i;
	int gpio_num = 0;
	int case_num;

	if(unlikely(name == NULL)) 
	{
		printk(KERN_ERR "%s: PAD name is NULL\n", name);
		return -1;
	}

	len = strlen(name);

	if(unlikely(len < 3 || len > 7)) 
	{
		printk(KERN_ERR "%s: PAD name length illegal\n", name);
		return -1;
	}

	if(len == 7) 
	{
		if(name[0]!='j' || name[1]!='c' || name[2]!= 'n' || name[3]!='t' ||
		   name[4]!='r' || name[5]!='s' || name[6]!='t') 
		{
			printk(KERN_ERR "%s: PAD name illegal, len %d\n", name, len);
			return -1;
		}
	}
	else if(len == 6) {
		if((name[0] == 'b' && (name[1] != 't' || name[2] != 's' || name[3] != 'd')) ||
		   (name[0] == 'c' && ((name[1] != 'o' && name[1] != 'a') || (name[2] != 'm' && name[2] != 'n') || name[3] != 'd')) ||
		   (name[0] == 'd' && (name[1] != 'i' || name[2] != 's' || name[3] != 'p')) ||
		   (name[0] == 'e' && (name[1] != 'x' || name[2] != 't' || name[3] != 'c' || name[4] != 'l' || name[5] != 'k')) ||
		   (name[0] == 'G' && (name[1] != 'P' || name[2] != 'I' || name[3] != 'O')) ||
		   (name[0] == 'j' && (name[1] != 'c' || name[2] != 'r' || name[3] != 't' || name[4] != 'c' || name[5] != 'k')) ||
		   (name[0] == 'r' && (name[1] != 't' || name[2] != 'c' || name[3] != 'g' || name[4] != 'p')))
		{
			printk(KERN_ERR "%s: PAD name illegal, len %d\n", name, len);
			return -1;
		}
	}
	else if(len == 5) {
		if((name[0] == 'b' && (name[1] != 't' || name[2] != 's' || name[3] != 'd')) ||
		   (name[0] == 'c' && ((name[1] != 'o' && name[1] != 'a') || (name[2] != 'm' && name[2] != 'n') || name[3] != 'd')) ||
		   (name[0] == 'd' && (name[1] != 'i' || name[2] != 's' || name[3] != 'p')) ||
		   (name[0] == 'G' && (name[1] != 'P' || name[2] != 'I' || name[3] != 'O')) ||
		   (name[0] == 'g' && (name[1] != 'p' || (name[2] != 'a' && name[2] != 'b' && name[2] != 'c' && name[2] != 'd' && name[2] != 'e' &&
							  name[2] != 'f' && name[2] != 'g' && name[2] != 'h' && name[2] != 'i' && name[2] != 'j' &&
							  name[2] != 'k' && name[2] != 'l' && name[2] != 'm' && name[2] != 'n'))) ||
		   (name[0] == 'j' && (name[1] != 'c' || (name[2] != 't' && name[2] != 'd') || (name[3] != 'b' && name[3] != 'c' && name[3] != 'd' && name[3] != 'm') ||
				       (name[4] != 'g' && name[4] != 'i' && name[4] != 'k' && name[4] != 'o' && name[4] != 's'))) ||
		   (name[0] == 'u' && (name[1] != 's' || name[2] != 'r' || name[3] != 'd')) ||
		   (name[0] == 'a' && (name[1] != 'u' || name[2] != 'd')))
		{
			printk(KERN_ERR "%s: PAD name illegal, len %d\n", name, len);
			return -1;
		}
	}
	else if(len == 4) {
		if((name[0] == 'a' && (name[1] != 'u' || name[2] != 'd')) ||
		   (name[0] == 'g' && (name[1] != 'p' || (name[2] != 'a' && name[2] != 'b' && name[2] != 'c' && name[2] != 'd' && name[2] != 'e' &&
							  name[2] != 'f' && name[2] != 'g' && name[2] != 'h' && name[2] != 'i' && name[2] != 'j' &&
							  name[2] != 'k' && name[2] != 'l' && name[2] != 'm' && name[2] != 'n'))))
		{
			printk(KERN_ERR "%s: PAD name illegal, len %d\n", name, len);
			return -1;
		}
	}
	else if(len == 3) {
		if(name[0] != 'o' || name[1] != 'm') {
			printk(KERN_ERR "%s: PAD name illegal, len %d\n", name, len);
			return -1;
		}
	}

	if(name[0] == 'a')
	{
		for(i=0;i<(len-3);i++)
		{
			if(isdigit(name[i+3]))
			{
				index = index*10 + name[i+3] - '0';
//				ok = 1;
			}
//			else if(name[i+3] == '#')
//			{
//				if(!ok || (i != len - 4)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		index += 110;
	}

	if(name[0] == 'b') 
	{
		for(i=0;i<(len-4);i++) 
		{
			if(isdigit(name[i+4])) 
			{
				index = index*10 + name[i+4] - '0';
//				ok = 1;
			}
//			else if(name[i+4] == '#') 
//			{
//				if(!ok || (i != len - 5)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else 
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
	}

	if(name[0] == 'c') 
	{
		if(name[1] == 'a') 
		{
			for(i=0;i<(len-4);i++) 
			{
				if(isdigit(name[i+4])) 
				{
					index = index*10 + name[i+4] - '0';
//					ok = 1;
				}
//				else if(name[i+4] == '#')
//				{
//					if(!ok || (i != len - 5)) {
//						printk(KERN_ERR "GPIO illegal.\n");
//						return -1;
//					}
//				}
				else
				{
					printk(KERN_ERR "%s:PAD name number illegal.\n", name);
					return -1;
				}
			}

			index += 126;
		}
		else if(name[1] == 'o')
		{
			if(name[2] == 'm')
			{
				for(i=0;i<(len-4);i++)
				{
					if(isdigit(name[i+4]))
					{
						index = index*10 + name[i+4] - '0';
//						ok = 1;
					}
//					else if(name[i+4] == '#')
//					{
//						if(!ok || (i != len - 5)) {
//							printk(KERN_ERR "GPIO illegal.\n");
//							return -1;
//						}
//					}
					else
					{
						printk(KERN_ERR "%s:PAD name number illegal.\n", name);
						return -1;
					}
				}

				index += 38;
			}
			else if(name[2] == 'n')
			{
				for(i=0;i<(len-4);i++)
				{
					if(isdigit(name[i+4]))
					{
						index = index*10 + name[i+4] - '0';
//						ok = 1;
					}
//					else if(name[i+4] == '#')
//					{
//						if(!ok || (i != len - 5)) {
//							printk(KERN_ERR "GPIO illegal.\n");
//							return -1;
//						}
//					}
					else
					{
						printk(KERN_ERR "%s:PAD name number illegal.\n", name);
						return -1;
					}
				}

				index += 54;
			}
		}
	}

	if(name[0] == 'd')
	{
		for(i=0;i<(len-4);i++)
		{
			if(isdigit(name[i+4]))
			{
				index = index*10 + name[i+4] - '0';
//				ok = 1;
			}
//			else if(name[i+4] == '#')
//			{
//				if(!ok || (i != len - 5)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		index += 82;
	}

	if(name[0] == 'e')
	{
		index = 155;
	}

	if(name[0] == 'g')
	{
		for(i=0;i<(len-3);i++)
		{
			if(isdigit(name[i+3]))
			{
				index = index*10 + name[i+3] - '0';
//				ok = 1;
			}
//			else if(name[i+3] == '#')
//			{
//				if(!ok || (i != len - 4)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		if(name[2] == 'a')
			index += 0;
		else if(name[2] == 'b')
			index += 16;
		else if(name[2] == 'c')
			index += 31;
		else if(name[2] == 'd')
			index += 38;
		else if(name[2] == 'e')
			index += 54;
		else if(name[2] == 'f')
			index += 70;
		else if(name[2] == 'g')
			index += 92;
		else if(name[2] == 'h')
			index += 98;
		else if(name[2] == 'i')
			index += 110;
		else if(name[2] == 'j')
			index += 126;
		else if(name[2] == 'k')
			index += 134;
		else if(name[2] == 'l')
			index += 143;
		else if(name[2] == 'm')
			index += 82;
		else if(name[2] == 'n')
			index += 156;
	}

	if(name[0] == 'G')
	{
		for(i=0;i<(len-4);i++)
		{
			if(isdigit(name[i+4]))
			{
				gpio_num = gpio_num*10 + name[i+4] - '0';
//				ok = 1;
			}
//			else if(name[i+4] == '#') {
//				if(!ok || (i != len - 5)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		case_num = readl(PAD_SYSM_VA);
		switch(case_num)
		{
			case 0:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 25;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 68;
				else if(gpio_num == 10)
					index = 120;
				else if(gpio_num == 11)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 1:
				if(gpio_num == 0)
					index = 11;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 13;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 22;
				else if(gpio_num == 9 || gpio_num == 10 || gpio_num == 11 || gpio_num == 12)
					index = gpio_num + 65;
				else if(gpio_num == 13)
					index = 120;
				else if(gpio_num == 14)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 2:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 ||
					     	gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 16;
				else if(gpio_num == 9)
					index = 120;
				else if(gpio_num == 10)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 3:
				if(gpio_num == 0)
					index = 11;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5 || 
						gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 15;
				else if(gpio_num == 10 || gpio_num == 11)
					index = gpio_num + 19;
				else if(gpio_num == 12)
					index = 120;
				else if(gpio_num == 13)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 4:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 25;
				else if(gpio_num == 6)
					index = 44;
				else if(gpio_num == 7 || gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11 || gpio_num == 12)
					index = gpio_num + 113;
				else if(gpio_num == 13)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 5:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 25;
				else if(gpio_num == 4)
					index = 44;
				else if(gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 45;
				else if(gpio_num == 9 || gpio_num == 10)
					index = gpio_num + 57;
				else if(gpio_num == 11)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 6:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1)
					index = 26;
				else if(gpio_num == 2)
					index = 43;
				else if(gpio_num == 3 || gpio_num == 4 || gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 47;
				else if(gpio_num == 7 || gpio_num == 8 || gpio_num == 9 || gpio_num == 10)
					index = gpio_num + 67;
				else if(gpio_num == 11 || gpio_num == 12 || gpio_num == 13 || gpio_num == 14)
					index = gpio_num + 110;
				else if(gpio_num == 15 || gpio_num == 16)
					index = gpio_num + 126;
				else if(gpio_num == 17)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 7:
				if(gpio_num == 0 || gpio_num == 1)
					index = gpio_num + 44;
				else if(gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 64;
				else if(gpio_num == 4)
					index = 120;
				else if(gpio_num == 5)
					index = 125;
				else if(gpio_num == 6)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 8:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4)
					index = gpio_num + 11;
				else if(gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 39;
				else if(gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 59;
				else if(gpio_num == 9)
					index = 120;
				else if(gpio_num == 10)
					index = 125;
				else if(gpio_num == 11)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 9:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 16;
				else if(gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 40;
				else if(gpio_num == 6)
					index = 120;
				else if(gpio_num == 7)
					index = 125;
				else if(gpio_num == 8)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 10:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || 
				   gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 11;
				else if(gpio_num == 9 || gpio_num == 10)
					index = gpio_num + 35;
				else if(gpio_num == 11)
					index = 120;
				else if(gpio_num == 12)
					index = 125;
				else if(gpio_num == 13)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 11:
				if(gpio_num == 0)
					index = 44;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4)
					index = gpio_num + 85;
				else if(gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9 || gpio_num == 10 ||
				      	gpio_num == 11 || gpio_num == 12 || gpio_num == 13 || gpio_num == 14 || gpio_num == 15 ||
				      	gpio_num == 16 || gpio_num == 17 || gpio_num == 18 || gpio_num == 19 || gpio_num == 20)
					index = gpio_num + 89;
				else if(gpio_num == 21)
					index = 120;
				else if(gpio_num == 22)
					index = 125;
				else if(gpio_num == 23)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 12:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2)
					index = gpio_num + 11;
				else if(gpio_num == 3)
					index = 44;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6 || gpio_num == 7)
					index = gpio_num + 82;
				else if(gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11 || gpio_num == 12 || gpio_num == 13 ||
				      	gpio_num == 14 || gpio_num == 15 || gpio_num == 16 || gpio_num == 17 || gpio_num == 18 ||
				      	gpio_num == 19 || gpio_num == 20 || gpio_num == 21 || gpio_num == 22 || gpio_num == 23)
					index = gpio_num + 86;
				else if(gpio_num == 24)
					index = 120;
				else if(gpio_num == 25)
					index = 125;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 13:
				if(gpio_num == 0)
					index = 44;
				else if(gpio_num == 1)
					index = 70;
				else if(gpio_num == 2)
					index = 120;
				else if(gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 122;
				else if(gpio_num == 6 || gpio_num == 7)
					index = gpio_num + 130;
				else if(gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11)
					index = gpio_num + 131;
				else if(gpio_num == 12)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 14:
				if(gpio_num == 0 || gpio_num == 1 || gpio_num == 2)
					index = gpio_num + 11;
				else if(gpio_num == 3)
					index = 44;
				else if(gpio_num == 4)
					index = 120;
				else if(gpio_num == 5 || gpio_num == 6 || gpio_num == 7)
					index = gpio_num + 120;
				else if(gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 128;
				else if(gpio_num == 10 || gpio_num == 11 || gpio_num == 12 || gpio_num == 13)
					index = gpio_num + 129;
				else if(gpio_num == 14)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 15:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1)
					index = 26;
				else if(gpio_num == 2)
					index = 45;
				else if(gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 112;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11 ||
				      	gpio_num == 12 || gpio_num == 13 || gpio_num == 14 || gpio_num == 15 || gpio_num == 16)
					index = gpio_num + 114;
				else if(gpio_num == 17)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 16:
				if(gpio_num == 0 || gpio_num == 1)
					index = gpio_num + 60;
				else if(gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 72;
				else if(gpio_num == 6)
					index = 120;
				else if(gpio_num == 7)
					index = 125;
				else if(gpio_num == 8)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 17:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1)
					index = 26;
				else if(gpio_num == 2)
					index = 45;
				else if(gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 112;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11 ||
					gpio_num == 12 || gpio_num == 13 || gpio_num == 14 || gpio_num == 15 || gpio_num == 16)
					index = gpio_num + 114;
				else if(gpio_num == 17)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 18:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1)
					index = 26;
				else if(gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 42;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9 ||
				      	gpio_num == 10 || gpio_num == 11 || gpio_num == 12 || gpio_num == 13 || gpio_num == 14)
					index = gpio_num + 116;
				else if(gpio_num == 15)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 19:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 25;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 68;
				else if(gpio_num == 10)
					index = 120;
				else if(gpio_num == 11)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 20:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 25;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 68;
				else if(gpio_num == 10)
					index = 120;
				else if(gpio_num == 11)
					index = 153;
				else 
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 21:
				if(gpio_num == 0)
					index = 16;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 25;
				else if(gpio_num == 6)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 22:
				if(gpio_num == 0)
					index = 120;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 122;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6 || gpio_num == 7)
					index = gpio_num + 130;
				else if(gpio_num == 8)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 23:
				if(gpio_num == 0)
					index = 11;
				else if(gpio_num == 1 || gpio_num == 2)
					index = gpio_num + 13;
				else if(gpio_num == 3)
					index = 120;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 119;
				else if(gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 129;
				else if(gpio_num == 9)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 24:
				if(gpio_num == 0)
					index = 44;
				else if(gpio_num == 1 || gpio_num == 2)
					index = gpio_num + 59;
				else if(gpio_num == 3)
					index = 70;
				else if(gpio_num == 4)
					index = 120;
				else if(gpio_num == 5 || gpio_num == 6 || gpio_num == 7)
					index = gpio_num + 118;
				else if(gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 128;
				else if(gpio_num == 10)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 25:
				if(gpio_num == 0)
					index = 44;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4)
					index = gpio_num + 49;
				else if(gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 61;
				else if(gpio_num == 7)
					index = 120;
				else if(gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 115;
				else if(gpio_num == 10)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 26:
				if(gpio_num == 0)
					index = 43;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3 || gpio_num == 4)
					index = gpio_num + 49;
				else if(gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 118;
				else if(gpio_num == 7 || gpio_num == 8)
					index = gpio_num + 134;
				else if(gpio_num == 9)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 27:
				if(gpio_num == 0)
					index = 45;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 114;
				else if(gpio_num == 4)
					index = 120;
				else if(gpio_num == 5 || gpio_num == 6 || gpio_num == 7 || gpio_num == 8 ||
				      	gpio_num == 9 || gpio_num == 10 || gpio_num == 11 || gpio_num == 12)
					index = gpio_num + 118;
				else if(gpio_num == 13)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 28:
				if(gpio_num == 0)
					index = 11;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 13;
				else if(gpio_num == 4 || gpio_num == 5 || gpio_num == 6)
					index = gpio_num + 22;
				else if(gpio_num == 7)
					index = 44;
				else if(gpio_num == 8 || gpio_num == 9 || gpio_num == 10 || gpio_num == 11)
					index = gpio_num + 42;
				else if(gpio_num == 12 || gpio_num == 13)
					index = gpio_num + 54;
				else if(gpio_num == 14)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 29:
				if(gpio_num == 0)
					index = 120;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 122;
				else if(gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 132;
				else if(gpio_num == 6)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 30:
				if(gpio_num == 0)
					index = 120;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 122;
				else if(gpio_num == 4 || gpio_num == 5)
					index = gpio_num + 132;
				else if(gpio_num == 6)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;

			case 31:
				if(gpio_num == 0)
					index = 11;
				else if(gpio_num == 1 || gpio_num == 2 || gpio_num == 3)
					index = gpio_num + 13;
				else if(gpio_num == 4)
					index = 26;
				else if(gpio_num == 5)
					index = 43;
				else if(gpio_num == 6 || gpio_num == 7 || gpio_num == 8 || gpio_num == 9)
					index = gpio_num + 44;
				else if(gpio_num == 10 || gpio_num == 11 || gpio_num == 12 || gpio_num == 13)
					index = gpio_num + 64;
				else if(gpio_num == 14 || gpio_num == 15 || gpio_num == 16 || gpio_num == 17)
					index = gpio_num + 107;
				else if(gpio_num == 18 || gpio_num == 19)
					index = gpio_num + 123;
				else if(gpio_num == 20)
					index = 153;
				else
				{
					printk(KERN_ERR "GPIO %d is not used in case %d\n", gpio_num, case_num);
					index = -1;
				}
				break;
		}
	}

	if(name[0] == 'j')
	{
		if(name[2] == 'd')
			index = 154;
		else if(name[2] == 'n')
			index = 148;
		else if(name[2] == 'r')
			index = 153;
		else if(name[2] == 't')
		{
			if(name[3] == 'c')
				index = 149;
			else if(name[3] == 'm')
				index = 150;
			else if(name[3] == 'd')
			{
				if(name[4] == 'i')
					index = 151;
				else if(name[4] == 'o')
					index = 152;
			}
		}
	}

	if(name[0] == 'o')
	{
		for(i=0;i<(len-2);i++)
		{
			if(isdigit(name[i+2]))
			{
				index = index*10 + name[i+4] - '0';
//				ok = 1;
			}
//			else if(name[i+2] == '#')
//			{
//				if(!ok || (i != len - 3)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		index += 143;
	}

	if(name[0] == 'r')
	{
		for(i=0;i<(len-5);i++)
		{
			if(isdigit(name[i+5]))
			{
				index = index*10 + name[i+5] - '0';
//				ok = 1;
			}
//			else if(name[i+5] == '#')
//			{
//				if(!ok || (i != len - 6)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		index += 156;
	}

	if(name[0] == 'u')
	{
		for(i=0;i<(len-4);i++)
		{
			if(isdigit(name[i+4]))
			{
				index = index*10 + name[i+4] - '0';
//				ok = 1;
			}
//			else if(name[i+4] == '#')
//			{
//				if(!ok || (i != len - 5)) {
//					printk(KERN_ERR "GPIO illegal.\n");
//					return -1;
//				}
//			}
			else
			{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}

		index += 31;
	}

	return index;
}

static int imapx_pad_index_i15(const char *name)
{
	int index = 0;
	int len = 0;
//	int ok = 0;
	int i;
//	int gpio_num = 0;
//	int case_num;

	if(unlikely(name == NULL)) 
	{
		printk(KERN_ERR "%s: PAD name is NULL\n", name);
		return -1;
	}

	len = strlen(name);

	if(unlikely(len < 4 || len > 6)) 
	{
		printk(KERN_ERR "%s: PAD name length illegal\n", name);
		return -1;
	}else if(len == 4){
		if(strncmp(name, "aud", 3) || strncmp(name, "GPA", 3) || strncmp(name, "GPB", 3) || 
		   strncmp(name, "GPC", 3) || strncmp(name, "GPD", 3) || strncmp(name, "GPE", 3) || strncmp(name, "GPF", 3)){
			printk(KERN_ERR "%s: PAD name len illegal, len %d\n", name, len);
			return -1;
		}
	}else if(len == 5){
		if(strncmp(name, "btsd", 4) || strncmp(name, "usrd", 4) || strncmp(name, "comd", 4) || strncmp(name, "cond", 4) || 
		   strncmp(name, "disp", 4) || strncmp(name, "camd", 4) || strncmp(name, "aud", 3) || strncmp(name, "GPA", 3) || 
		   strncmp(name, "GPB", 3) || strncmp(name, "GPC", 3) || strncmp(name, "GPD", 3) || strncmp(name, "GPE", 3) || strncmp(name, "GPF", 3)){
			printk(KERN_ERR "%s: PAD name len illegal, len %d\n", name, len);
			return -1;
		}
	}else if(len == 6){
		if(strncmp(name, "btsd", 4) || strncmp(name, "usrd", 4) || strncmp(name, "comd", 4) || 
		   strncmp(name, "cond", 4) || strncmp(name, "disp", 4) || strncmp(name, "camd", 4) || strncmp(name, "extclk", 6)){
			printk(KERN_ERR "%s: PAD name len illegal, len %d\n", name, len);
			return -1;
		}
	}

	if(name[0] == 'a'){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 95;

	}else if(name[0] == 'b'){
		for(i=0;i<(len-4);i++){
			if(isdigit(name[i+4])){
				index = index*10 + name[i+4] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 0;

	}else if(name[0] == 'c'){
		if(name[1] == 'a'){
			for(i=0;i<(len-4);i++){
				if(isdigit(name[i+4])){
					index = index*10 + name[i+4] - '0';
				}else{
					printk(KERN_ERR "%s:PAD name number illegal.\n", name);
					return -1;
				}
			}
			index += 120;

		}else if(name[1] == 'o'){
			if(name[2] == 'm'){
				for(i=0;i<(len-4);i++){
					if(isdigit(name[i+4])){
						index = index*10 + name[i+4] - '0';
					}else{
						printk(KERN_ERR "%s:PAD name number illegal.\n", name);
						return -1;
					}
				}
				index += 36;

			}else if(name[2] == 'n'){
				for(i=0;i<(len-4);i++){
					if(isdigit(name[i+4])){
						index = index*10 + name[i+4] - '0';
					}else{
						printk(KERN_ERR "%s:PAD name number illegal.\n", name);
						return -1;
					}
				}
				index += 43;
			}
		}
	}else if(name[0] == 'd'){
		for(i=0;i<(len-4);i++){
			if(isdigit(name[i+4])){
				index = index*10 + name[i+4] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 66;

	}else if(name[0] == 'e'){
		index = 65;

	}else if(name[0] == 'u'){
		for(i=0;i<(len-4);i++){
			if(isdigit(name[i+4])){
				index = index*10 + name[i+4] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 29;
	}else if(strncmp(name, "GPA", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 0;

	}else if(strncmp(name, "GPB", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 29;

	}else if(strncmp(name, "GPC", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 43;

	}else if(strncmp(name, "GPD", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 66;

	}else if(strncmp(name, "GPE", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 95;

	}else if(strncmp(name, "GPF", 3) == 0){
		for(i=0;i<(len-3);i++){
			if(isdigit(name[i+3])){
				index = index*10 + name[i+3] - '0';
			}else{
				printk(KERN_ERR "%s:PAD name number illegal.\n", name);
				return -1;
			}
		}
		index += 120;
	}

	return index;
}

/*
 *Function:     imapx_pad_index
 *Parameter:	name,the pad name;
 *Description:	get pad index correspond to given name;
 *Return:	    success:pad index
 *		        fail:	-1
 */
int imapx_pad_index(const char *name)
{
	int ret = -1;

	if(item_exist("board.cpu") == 0 || 
			item_equal("board.cpu", "imapx820", 0)){
		ret = imapx_pad_index_x820(name);
	}else if(item_equal("board.cpu", "i15", 0)){
		ret = imapx_pad_index_i15(name);
	}

	return ret;
}

static int imapx_pad_irq_number_x820(int index)
{
	int irq = 0;
	int case_num;

	case_num = readl(PAD_SYSM_VA);
	switch(case_num)
	{
		case 0:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				irq = index + 39;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				irq = index;
			else if(index == 120 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 1:
			if(index == 11)
				irq = 64;
			else if(index == 14 || index == 15 || index == 16)
				irq = index + 51;
			else if(index == 26 || index == 27)
				irq = index + 42;
			else if(index == 28 || index == 29 || index == 30)
				irq = index + 46;
			else if(index == 74)
				irq = 77;
			else if(index == 75 || index == 76 || index == 77 || index == 120 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 2:
			if(index == 16 || index == 17 || index == 18 || index == 19 || index == 20 || index == 21)
				irq = index + 48;
			else if(index == 22 || index == 23 || index == 24)
				irq = index + 52;
			else if(index == 120)
				irq = 77;
			else if(index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 3:
			if(index == 11)
				irq = 64;
			else if(index == 16 || index == 17 || index == 18 || index == 19 || index == 20)
				irq = index + 49;
			else if(index == 21 || index == 22 || index == 23 || index == 24)
				irq = index + 53;
			else if(index == 29 || index == 30 || index == 120 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 4:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				irq = index + 39;
			else if(index == 44)
				irq = 74;
			else if(index == 120 || index == 121 || index == 122)
				irq = index - 45;
			else if(index == 123 || index == 124 || index == 125 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 5:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28)
				irq = index + 39;
			else if(index == 44)
				irq = 68;
			else if(index == 50)
				irq = 69;
			else if(index == 51 || index == 52 || index == 53)
				irq = index + 23;
			else if(index == 66)
				irq = 77;
			else if(index == 67 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 6:
			if(index == 16)
				irq = 64;
			else if(index == 26)
				irq = 65;
			else if(index == 43)
				irq = 66;
			else if(index == 50 || index == 51 || index == 52)
				irq = index + 17;
			else if(index == 53)
				irq = 74;
			else if(index == 74 || index == 75 || index == 76)
				irq = index + 1;
			else if(index == 77 || index == 121 || index == 122 || index == 123 || index == 124 || index == 141 || index == 142 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 7:
			if(index == 44 || index == 45)
				irq = index + 20;
			else if(index == 66 || index == 67)
				irq = index;
			else if(index == 120)
				irq = 68;
			else if(index == 125)
				irq = 69;
			else if(index == 153)
				irq = 74;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 8:
			if(index == 11 || index == 12 || index == 13 || index == 14 || index == 15)
				irq = index + 53;
			else if(index == 44)
				irq = 69;
			else if(index == 45)
				irq = 74;
			else if(index == 66 || index == 67)
				irq = index + 9;
			else if(index == 120)
				irq = 77;
			else if(index == 125 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq func\n", index, case_num);
			break;

		case 9:
			if(index == 16 || index == 17 || index == 18 || index == 19)
				irq = index + 48;
			else if(index == 44 || index == 45)
				irq = index + 24;
			else if(index == 120)
				irq = 74;
			else if(index == 125)
				irq = 75;
			else if(index == 153)
				irq = 76;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 10:
			if(index == 11 || index == 12 || index == 13 || index == 14 || index == 15 || index == 16)
				irq = index + 53;
			else if(index == 17 || index == 18 || index == 19)
				irq = index + 57;
			else if(index == 44)
				irq = 77;
			else if(index == 45 || index == 120 || index == 125 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 11:
			if(index == 44)
				irq = 64;
			else if(index == 86 || index == 87 || index == 88 || index == 89)
				irq = index - 21;
			else if(index == 94)
				irq = 69;
			else if(index == 95 || index == 96 || index == 97 || index == 98)
				irq = index - 21;
			else if(index == 99 || index == 100 || index == 101 || index == 102 || index == 103 || index == 104 || index == 105 ||
				index == 106 || index == 107 || index == 108 || index == 109 || index == 120 || index == 125 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 12:
			if(index == 11 || index == 12 || index == 13)
				irq = index + 53;
			else if(index == 44)
				irq = 67;
			else if(index == 86 || index == 87)
				irq = index -18;
			else if(index == 88 || index == 89)
				irq = index - 14;
			else if(index == 94 || index == 95)
				irq = index - 18;
			else if(index == 96 || index == 97 || index == 98 || index == 99 || index == 100 || index == 101 || index == 102 || index == 103 ||
				index == 104 || index == 105 || index == 106 || index == 107 || index == 108 || index == 109 || index == 120 || index == 125)
				irq = 63;
			else 
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 13:
			if(index == 44)
				irq = 64;
			else if(index == 70)
				irq = 65;
			else if(index == 120)
				irq = 66;
			else if(index == 125 || index == 126 || index == 127)
				irq = index - 58;
			else if(index == 136 || index == 137)
				irq = index - 62;
			else if(index == 139 || index == 140)
				irq = index - 63;
			else if(index == 141 || index == 142 || index == 153)
				irq = 63;
			else 
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 14:
			if(index == 11 || index == 12 || index == 13)
				irq = index + 53;
			else if(index == 44)
				irq = 67;
			else if(index == 120)
				irq = 68;
			else if(index == 125)
				irq = 69;
			else if(index == 126 || index == 127)
				irq = index - 52;
			else if(index == 136 || index == 137)
				irq = index - 60;
			else if(index == 139 || index == 140 || index == 141 || index == 142 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 15:
			if(index == 16)
				irq = 64;
			else if(index == 26)
				irq = 65;
			else if(index == 45)
				irq = 66;
			else if(index == 115 || index == 116 || index == 117)
				irq = index - 48;
			else if(index == 120 || index == 121 || index == 122 || index == 123)
				irq = index - 46;
			else if(index == 124 || index == 125 || index == 126 || index == 127 || index == 128 || index == 129 || index == 130 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 16:
			if(index == 60 || index == 61)
				irq = index + 4;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				irq = index - 8;
			else if(index == 120)
				irq = 74;
			else if(index == 125)
				irq = 75;
			else if(index == 153)
				irq = 76;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 17:
			if(index == 16)
				irq = 64;
			else if(index == 26)
				irq = 65;
			else if(index == 45)
				irq = 66;
			else if(index == 115 || index == 116 || index == 117)
				irq = index - 48;
			else if(index == 120 || index == 121 || index == 122 || index == 123)
				irq = index - 46;
			else if(index == 124 || index == 125 || index == 126 || index == 127 || index == 128 || index == 129 || index == 130 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 18:
			if(index == 16)
				irq = 64;
			else if(index == 26)
				irq = 65;
			else if(index == 44 || index == 45)
				irq = index + 22;
			else if(index == 120 || index == 121)
				irq = index - 52;
			else if(index == 122 || index == 123 || index == 124 || index == 125)
				irq = index - 48;
			else if(index == 126 || index == 127 || index == 128 || index == 129 || index == 130 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 19:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				irq = index + 39;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				irq = index;
			else if(index == 120 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 20:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				irq = index + 39;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				irq = index;
			else if(index == 120 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 21:
			if(index == 16)
				irq = 64;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				irq = index + 39;
			else if(index == 153)
				irq = 74;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 22:
			if(index == 120)
				irq = 64;
			else if(index == 123 || index == 124 || index == 125)
				irq = index - 58;
			else if(index == 134 || index == 135)
				irq = index - 66;
			else if(index == 136 || index == 137)
				irq = index - 62;
			else if(index == 153)
				irq = 76;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 23:
			if(index == 11)
				irq = 64;
			else if(index == 14 || index == 15)
				irq = index + 51;
			else if(index == 120)
				irq = 67;
			else if(index == 123 || index == 124)
				irq = index - 55;
			else if(index == 125)
				irq = 74;
			else if(index == 136 || index == 137)
				irq = index - 61;
			else if(index == 153)
				irq = 77;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 24:
			if(index == 44)
				irq = 64;
			else if(index == 60 || index == 61)
				irq = index + 5;
			else if(index == 70)
				irq = 67;
			else if(index == 120)
				irq = 68;
			else if(index == 123)
				irq = 69;
			else if(index == 124 || index == 125)
				irq = index - 70;
			else if(index == 136 || index == 137)
				irq = index - 70;
			else if(index == 153)
				irq = 63;
			else 
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 25:
			if(index == 44)
				irq = 64;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				irq = index + 15;
			else if(index == 66)
				irq = 69;
			else if(index == 67)
				irq = 74;
			else if(index == 120)
				irq = 75;
			else if(index == 123 || index == 124)
				irq = index - 47;
			else if(index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 26:
			if(index == 43)
				irq = 64;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				irq = index + 15;
			else if(index == 123)
				irq = 69;
			else if(index == 124)
				irq = 74;
			else if(index == 141 || index == 142)
				irq = index - 66;
			else if(index == 153)
				irq = 77;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 27:
			if(index == 45)
				irq = 64;
			else if(index == 115 || index == 116 || index == 117)
				irq = index - 50;
			else if(index == 120)
				irq = 68;
			else if(index == 123)
				irq = 69;
			else if(index == 124 || index == 125 || index == 126 || index == 127)
				irq = index - 50;
			else if(index == 128 || index == 129 || index == 130 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 28:
			if(index == 11)
				irq = 64;
			else if(index == 14 || index == 15 || index == 16)
				irq = index + 51;
			else if(index == 26 || index == 27)
				irq = index + 42;
			else if(index == 28)
				irq = 74;
			else if(index == 44)
				irq = 75;
			else if(index == 50 || index == 51)
				irq = index + 26;
			else if(index == 52 || index == 53 || index == 66 || index == 67 || 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 29:
			if(index == 120)
				irq = 64;
			else if(index == 123 || index == 124 || index == 125)
				irq = index - 58;
			else if(index == 136 || index == 137)
				irq = index - 68;
			else if(index == 153)
				irq = 74;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 30:
			if(index == 120)
				irq = 64;
			else if(index == 123 || index == 124 || index == 125)
				irq = index - 58;
			else if(index == 136 || index == 137)
				irq = index - 68;
			else if(index == 153)
				irq = 74;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;

		case 31:
			if(index == 11)
				irq = 64;
			else if(index == 14 || index == 15 || index == 16)
				irq = index + 51;
			else if(index == 26)
				irq = 68;
			else if(index == 43)
				irq = 69;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				irq = index + 24;
			else if(index == 74 || index == 75 || index == 76 || index == 77 || index == 121 || index == 122 ||
				index == 123 || index == 124 || index == 141 || index == 142 || index == 153)
				irq = 63;
			else
				printk(KERN_ERR "pad %d in case%d don't have irq\n", index, case_num);
			break;
	}

	return irq;
}
/*
unsigned int imapx_pad_irq_flag(int type)
{
	unsigned int flag = 0;

	switch(type)
	{
		case INTTYPE_LOW:
			flag = IRQF_TRIGGER_LOW;
			break;

		case INTTYPE_HIGH:
			flag = IRQF_TRIGGER_HIGH;
			break;

		case INTTYPE_FALLING:
			flag = IRQF_TRIGGER_FALLING;
			break;

		case INTTYPE_RISING:
			flag = IRQF_TRIGGER_RISING;
			break;

		case INTTYPE_BOTH:
			flag = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING;
			break;
	}

	return flag;
}
*/

static int imapx_pad_irq_number_i15(int index)
{
    int irq = 0;

    if(index >= 0 || index <= 136){
        if(index < 96)
            irq = 64 + (int)(index/16);
        else
            irq = 68 + (int)(index/16);
    }

    return irq;
}

/*
 *Function:	    imapx_pad_irq_number
 *Parameter:	index,the pad index;
 *Description:	get irq num correspond to given index;
 *Return:	    success:the pad irq num
 *		        fail:	0
 */
int imapx_pad_irq_number(int index)
{
    int ret = 0;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = imapx_pad_irq_number_x820(index);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = imapx_pad_irq_number_i15(index);
    }

    return ret;
}

static int imapx_pad_irq_group_num_x820(int index)
{
	int num = -1;
	int case_num;

	case_num = readl(PAD_SYSM_VA);
	switch(case_num)
	{
		case 0:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 25;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 68;
			else if(index == 120)
				num = 10;
			else if(index == 153)
				num = 11;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 1:
			if(index == 11)
				num = 0;
			else if(index == 14 || index == 15 || index == 16)
				num = index - 13;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 22;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 65;
			else if(index == 120)
				num = 13;
			else if(index == 153)
				num = 14;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 2:
			if(index == 16 || index == 17 || index == 18 || index == 19 || index == 20 ||
			   index == 21 || index == 22 || index == 23 || index == 24)
				num = index - 16;
			else if(index == 120)
				num = 9;
			else if(index == 153)
				num = 10;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 3:
			if(index == 11)
				num = 0;
			else if(index == 16 || index == 17 || index == 18 || index == 19 || index == 20 ||
			      	index == 21 || index == 22 || index == 23 || index == 24)
				num = index - 15;
			else if(index == 29 || index == 30)
				num = index - 19;
			else if(index == 120)
				num = 12;
			else if(index == 153)
				num = 13;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 4:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 25;
			else if(index == 44)
				num = 6;
			else if(index == 120 || index == 121 || index == 122 || index == 123 || index == 124 || index == 125)
				num = index - 113;
			else if(index == 153)
				num = 13;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 5:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28)
				num = index - 25;
			else if(index == 44)
				num = 4;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 45;
			else if(index == 66 || index == 67)
				num = index - 57;
			else if(index == 153)
				num = 11;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 6:
			if(index == 16)
				num = 0;
			else if(index == 26)
				num = 1;
			else if(index == 43)
				num = 2;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 47;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 67;
			else if(index == 121 || index == 122 || index == 123 || index == 124)
				num = index - 110;
			else if(index == 141 || index == 142)
				num = index - 126;
			else if(index == 153)
				num = 17;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 7:
			if(index == 44 || index == 45)
				num = index - 44;
			else if(index == 66 || index == 67)
				num = index - 64;
			else if(index == 120)
				num = 4;
			else if(index == 125)
				num = 5;
			else if(index == 153)
				num = 6;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 8:
			if(index == 11 || index == 12 || index == 13 || index == 14 || index == 15)
				num = index - 11;
			else if(index == 44 || index == 45)
				num = index - 39;
			else if(index == 66 || index == 67)
				num = index - 59;
			else if(index == 120)
				num = 9;
			else if(index == 125)
				num = 10;
			else if(index == 153)
				num = 11;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 9:
			if(index == 16 || index == 17 || index == 18 || index == 19)
				num = index - 16;
			else if(index == 44 || index == 45)
				num = index - 40;
			else if(index == 120)
				num = 6;
			else if(index == 125)
				num = 7;
			else if(index == 153)
				num = 8;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 10:
			if(index == 11 || index == 12 || index == 13 || index == 14 || index == 15 ||
			   index == 16 || index == 17 || index == 18 || index == 19)
				num = index - 11;
			else if(index == 44 || index == 45)
				num = index - 35;
			else if(index == 120)
				num = 11;
			else if(index == 125)
				num = 12;
			else if(index == 153)
				num = 13;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 11:
			if(index == 44)
				num = 0;
			else if(index == 86 || index == 87 || index == 88 || index == 89)
				num = index - 85;
			else if(index == 94 || index == 95 || index == 96 || index == 97 || index == 98 || index == 99 ||
			      	index == 100 || index == 101 || index == 102 || index == 103 || index == 104 ||
			      	index == 105 || index == 106 || index == 107 || index == 108 || index == 109)
				num = index - 89;
			else if(index == 120)
				num = 21;
			else if(index == 125)
				num = 22;
			else if(index == 153)
				num = 23;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 12:
			if(index == 11 || index == 12 || index == 13)
				num = index - 11;
			else if(index == 44)
				num = 3;
			else if(index == 86 || index == 87 || index == 88 || index == 89)
				num = index - 82;
			else if(index == 94 || index == 95 || index == 96 || index == 97 || index == 98 || index == 99 || 
				index == 100 || index == 101 || index == 102 || index == 103 || index == 104 || 
				index == 105 || index == 106 || index == 107 || index == 108 || index == 109)
				num = index - 86;
			else if(index == 120)
				num = 24;
			else if(index == 125)
				num = 25;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 13:
			if(index == 44)
				num = 0;
			else if(index == 70)
				num = 1;
			else if(index == 120)
				num = 2;
			else if(index == 125 || index == 126 || index == 127)
				num = index - 122;
			else if(index == 136 || index == 137)
				num = index - 130;
			else if(index == 139 || index == 140 || index == 141 || index == 142)
				num = index - 131;
			else if(index == 153)
				num = 12;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 14:
			if(index == 11 || index == 12 || index == 13)
				num = index - 11;
			else if(index == 44)
				num = 3;
			else if(index == 120)
				num = 4;
			else if(index == 125 || index == 126 || index == 127)
				num = index - 120;
			else if(index == 136 || index == 137)
				num = index - 128;
			else if(index == 139 || index == 140 || index == 141 || index == 142)
				num = index - 129;
			else if(index == 153)
				num = 14;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 15:
			if(index == 16)
				num = 0;
			else if(index == 26)
				num = 1;
			else if(index == 45)
				num = 2;
			else if(index == 115 || index == 116 || index == 117)
				num = index - 112;
			else if(index == 120 || index == 121 || index == 122 || index == 123 || index == 124 || index == 125 ||
			      	index == 126 || index == 127 || index == 128 || index == 129 || index == 130)
				num = index - 114;
			else if(index == 153)
				num = 17;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 16:
			if(index == 60 || index == 61)
				num = index - 60;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 72;
			else if(index == 120)
				num = 6;
			else if(index == 125)
				num = 7;
			else if(index == 153)
				num = 8;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 17:
			if(index == 16)
				num = 0;
			else if(index == 26)
				num = 1;
			else if(index == 45)
				num = 2;
			else if(index == 115 || index == 116 || index == 117)
				num = index - 112;
			else if(index == 120 || index == 121 || index == 122 || index == 123 || index == 124 || index == 125 || 
				index == 126 || index == 127 || index == 128 || index == 129 || index == 130)
				num = index - 114;
			else if(index == 153)
				num = 17;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 18:
			if(index == 16)
				num = 0;
			else if(index == 26)
				num = 1;
			else if(index == 44 || index == 45)
				num = index - 42;
			else if(index == 120 || index == 121 || index == 122 || index == 123 || index == 124 || index == 125 || 
				index == 126 || index == 127 || index == 128 || index == 129 || index == 130)
				num = index - 116;
			else if(index == 153)
				num = 15;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 19:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 25;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 68;
			else if(index == 120)
				num = 10;
			else if(index == 153)
				num = 11;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 20:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 25;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 68;
			else if(index == 120)
				num = 10;
			else if(index == 153)
				num = 11;
			else 
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 21:
			if(index == 16)
				num = 0;
			else if(index == 26 || index == 27 || index == 28 || index == 29 || index == 30)
				num = index - 25;
			else if(index == 153)
				num = 6;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 22:
			if(index == 120)
				num = 0;
			else if(index == 123 || index == 124 || index == 125)
				num = index - 122;
			else if(index == 134 || index == 135 || index == 136 || index == 137)
				num = index - 130;
			else if(index == 153)
				num = 8;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 23:
			if(index == 11)
				num = 0;
			else if(index == 14 || index == 15)
				num = index - 13;
			else if(index == 120)
				num = 3;
			else if(index == 123 || index == 124 || index == 125)
				num = index - 119;
			else if(index == 136 || index == 137)
				num = index - 129;
			else if(index == 153)
				num = 9;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 24:
			if(index == 44)
				num = 0;
			else if(index == 60 || index == 61)
				num = index - 59;
			else if(index == 70)
				num = 3;
			else if(index == 120)
				num = 4;
			else if(index == 123 || index == 124 || index == 125)
				num = index - 118;
			else if(index == 136 || index == 137)
				num = index - 128;
			else if(index == 153)
				num = 10;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 25:
			if(index == 44)
				num = 0;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 49;
			else if(index == 66 || index == 67)
				num = index - 61;
			else if(index == 120)
				num = 7;
			else if(index == 123 || index == 124)
				num = index - 115;
			else if(index == 153)
				num = 10;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 26:
			if(index == 43)
				num = 0;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 49;
			else if(index == 123 || index == 124)
				num = index - 118;
			else if(index == 141 || index == 142)
				num = index - 134;
			else if(index == 153)
				num = 9;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 27:
			if(index == 45)
				num = 0;
			else if(index == 115 || index == 116 || index == 117)
				num = index - 114;
			else if(index == 120)
				num = 4;
			else if(index == 123 || index == 124 || index == 125 || index == 126 ||
			      	index == 127 || index == 128 || index == 129 || index == 130)
				num = index - 118;
			else if(index == 153)
				num = 13;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 28:
			if(index == 11)
				num = 0;
			else if(index == 14 || index == 15 || index == 16)
				num = index - 13;
			else if(index == 26 || index == 27 || index == 28)
				num = index - 22;
			else if(index == 44)
				num = 7;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 42;
			else if(index == 66 || index == 67)
				num = index - 54;
			else if(index == 153)
				num = 14;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 29:
			if(index == 120)
				num = 0;
			else if(index == 123 || index == 124 || index == 125)
				num = index - 122;
			else if(index == 136 || index == 137)
				num = index - 132;
			else if(index == 153)
				num = 6;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 30:
			if(index == 120)
				num = 0;
			else if(index == 123 || index == 124 || index == 125)
				num = index - 122;
			else if(index == 136 || index == 137)
				num = index - 132;
			else if(index == 153)
				num = 6;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;

		case 31:
			if(index == 11)
				num = 0;
			else if(index == 14 || index == 15 || index == 16)
				num = index - 13;
			else if(index == 26)
				num = 4;
			else if(index == 43)
				num = 5;
			else if(index == 50 || index == 51 || index == 52 || index == 53)
				num = index - 44;
			else if(index == 74 || index == 75 || index == 76 || index == 77)
				num = index - 64;
			else if(index == 121 || index == 122 || index == 123 || index == 124)
				num = index - 107;
			else if(index == 141 || index == 142)
				num = index - 123;
			else if(index == 153)
				num = 20;
			else
				printk(KERN_ERR "pad %d in case%d is not used for irq\n", index, case_num);
			break;
	}

	return num;
}

/*This func is not necessary for i15*/
static int imapx_pad_irq_group_num_i15(int index)
{
	return -1;
}

int imapx_pad_irq_group_num(int index)
{
	int ret = -1;

	if(item_exist("board.cpu") == 0 || 
			item_equal("board.cpu", "imapx820", 0)){
		ret = imapx_pad_irq_group_num_x820(index);
	}else if(item_equal("board.cpu", "i15", 0)){
		ret = imapx_pad_irq_group_num_i15(index);
	}
	
	return ret;
}

static int imapx_pad_irq_config_x820(int index, int type, int filter)
{
	int num;

	num = imapx_pad_irq_group_num(index);
	if(num < 0)
		return -EINVAL;

	spin_lock(&pad_lock);	

	/*mask intr*/
	writel(1,GPIO_INTMSK(num));
	writel(1,GPIO_INTGMSK(num));

	/*set direction to input*/
	writel(1,GPIO_DIR(num));

	/*set intr type*/
	writel(type,GPIO_INTTYPE(num));

	/*set filter*/
	writel(filter&0xff,GPIO_FILTER(num));
	writel((filter>>8)&0xff,GPIO_CLKDIV(num));

	/*enable intr*/
	writel(0,GPIO_INTPEND(num));
	writel(0,GPIO_INTMSK(num));

	/*open global mask when interrupt not shared*/
	if(num < 10)
	{
		writel(1,GPIO_INTGMSK(num));
	}
	else
	{
		writel(0,GPIO_INTGMSK(num));
	}

	spin_unlock(&pad_lock);

	return 0;
}

static int imapx_pad_irq_config_i15(int index, int type, int filter)
{
    if(index < 0 || index > 136)
        return -EINVAL;

    spin_lock(&pad_lock);

    /*mask intr*/
    writel(1, GPIO_INTMSK(index));
    writel(1, GPIO_INTGMSK(index));

    /*set direction to input*/
    writel(0, GPIO_DIR(index));

    /*set intr type*/
    writel(type, GPIO_INTTYPE(index));

    /*set filter*/
    writel(filter&0xff,GPIO_FILTER(index));
    writel((filter>>8)&0xff,GPIO_CLKDIV(index));

    /*enable intr*/
    writel(0, GPIO_INTPEND(index));
    writel(0, GPIO_INTMSK(index));
    writel(0, GPIO_INTGMSK(index));

    spin_unlock(&pad_lock);

    return 0;
}

int imapx_pad_irq_config(int index, int type, int filter)
{
    int ret = -EINVAL;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = imapx_pad_irq_config_x820(index, type, filter);
    }else if(item_equal("board.cpu", "i15", 0)){
        /*first set pad mode to gpio*/
        imapx_pad_set_mode(1, 1, index);
        ret = imapx_pad_irq_config_i15(index, type, filter);
    }

    return ret;
}

static int imapx_pad_irq_pending_x820(int index)
{
	unsigned int val = 0;
	int num;
	//unsigned long flag;

	num = imapx_pad_irq_group_num(index);
	if(num < 0)
		goto exit;

	if(num < 10)
	{
		//spin_lock_irqsave(&pad_lock,flag);
		val = readl(GPIO_INTPEND(num));
		//spin_unlock_irqrestore(&pad_lock,flag);
	}
	else
	{
		//spin_lock_irqsave(&pad_lock,flag);
		val = readl(GPIO_INTPENDGLB(0));
		val = (val & (1 << num)) ? 1 : 0;
		//spin_unlock_irqrestore(&pad_lock,flag);
	}

exit:
	return val;
}

static int imapx_pad_irq_pending_i15(int index)
{
    unsigned int val = 0;

    if(index < 0 || index > 136)
        goto exit;

    val = readl(GPIO_INTPEND(index));
    val = val ? 1 : 0;

exit:
    return val;
}

/*check pad irq pending status*/
int imapx_pad_irq_pending(int index)
{
    int ret = 0;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = imapx_pad_irq_pending_x820(index);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = imapx_pad_irq_pending_i15(index);
    }

    return ret;
}

static int imapx_pad_irq_clear_x820(int index)
{
	int num;
	//unsigned long flag;

	num = imapx_pad_irq_group_num(index);
	if(num < 0)
		return -EINVAL;

	//spin_lock_irqsave(&pad_lock,flag);
	writel(0,GPIO_INTPEND(num));
	//spin_unlock_irqrestore(&pad_lock,flag);

	return 0;

}

static int imapx_pad_irq_clear_i15(int index)
{
    if(index < 0 || index > 136)
        return -EINVAL;

    writel(0, GPIO_INTPEND(index));

    return 0;
}

/*clear pad corresponding irq pending bit*/
int imapx_pad_irq_clear(int index)
{
    int ret = -EINVAL;

    if(item_exist("board.cpu") == 0 ||
            item_equal("board.cpu", "imapx820", 0)){
        ret = imapx_pad_irq_clear_x820(index);
    }else if(item_equal("board.cpu", "i15", 0)){
        ret = imapx_pad_irq_clear_i15(index);
    }

    return ret;
}

static int imapx_pad_irq_mask_x820(int index)
{
	int num;
	//unsigned long flag;

	num = imapx_pad_irq_group_num(index);
	if(num < 0)
		return -EINVAL;

	//spin_lock_irqsave(&pad_lock,flag);
	writel(1,GPIO_INTMSK(num));
	writel(1,GPIO_INTGMSK(num));
	//spin_unlock_irqrestore(&pad_lock,flag);

	return 0;
}

static int imapx_pad_irq_mask_i15(int index)
{
	if(index < 0 || index > 136)
		return -EINVAL;

	writel(1,GPIO_INTMSK(index));
	writel(1,GPIO_INTGMSK(index));

	return 0;
}

/*mask pad corresponding irq pending bit*/
int imapx_pad_irq_mask(int index)
{
	int ret = -EINVAL;

	if(item_exist("board.cpu") == 0 || 
			item_equal("board.cpu", "imapx820", 0)){
		ret = imapx_pad_irq_mask_x820(index);
	}else if(item_equal("board.cpu", "i15", 0)){
		ret = imapx_pad_irq_mask_i15(index);
	}

	return ret;
}

static int imapx_pad_irq_unmask_x820(int index)
{
	int num;
	//unsigned long flag;

	num = imapx_pad_irq_group_num(index);
	if(num < 0)
		return -EINVAL;

	//spin_lock_irqsave(&pad_lock,flag);
	writel(0,GPIO_INTMSK(num));
	writel(0,GPIO_INTGMSK(num));
	//spin_unlock_irqrestore(&pad_lock,flag);

	return 0;
}

static int imapx_pad_irq_unmask_i15(int index)
{
	if(index < 0 || index > 136)
		return -EINVAL;

	writel(0,GPIO_INTMSK(index));
	writel(0,GPIO_INTGMSK(index));

	return 0;
}

/*unmask pad corresponding irq pending bit*/
int imapx_pad_irq_unmask(int index)
{
	int ret = -EINVAL;

	if(item_exist("board.cpu") == 0 ||
			item_equal("board.cpu", "imapx820", 0)){
		ret = imapx_pad_irq_unmask_x820(index);
	}else if(item_equal("board.cpu", "i15", 0)){
		ret = imapx_pad_irq_unmask_i15(index);
	}

	return ret;
}
