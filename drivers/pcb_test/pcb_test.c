
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/io.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <mach/items.h>
#include <mach/imap-iomap.h>

struct rtc_time{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_month;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdat;
};

struct pcbtest_audio_frame {
    char *buf;
    int result;
    int frames;
};

struct pcbtest_register {
	unsigned int base_addr;
	unsigned int map_size;
};

#define PCB_DEV_NAME  "pcbtest"
#define PCB_IOCMD_MAGIC		'P'
#define PCB_IOCMD_ETH_GETID		_IOR(PCB_IOCMD_MAGIC, 1, int)
#define PCB_IOCMD_BAT_GET		_IOR(PCB_IOCMD_MAGIC, 2, int)
#define PCB_IOCMD_CHARGER_ST     _IOR(PCB_IOCMD_MAGIC, 3, int)
#define PCB_IOCMD_AUDIO_RECORD            _IOR(PCB_IOCMD_MAGIC, 4, int)
#define PCB_IOCMD_AUDIO_PLAY            _IOR(PCB_IOCMD_MAGIC, 5, int)
#define PCB_IOCMD_AUDIO_SEND_FRAME      _IOR(PCB_IOCMD_MAGIC, 6, int)
#define PCB_IOCMD_WIFI_POWER_ON          _IOR(PCB_IOCMD_MAGIC, 7, int)
#define PCB_IOCMD_WIFI_POWER_SDIO_ON          _IOR(PCB_IOCMD_MAGIC, 0xd, int)
#define PCB_IOCMD_HDMI_TEST              _IOR(PCB_IOCMD_MAGIC, 8, int)
#define PCB_IOCMD_CAMERA_TEST          _IOR(PCB_IOCMD_MAGIC, 9, int)
#define PCB_IOCMD_RTC_TEST          _IOR(PCB_IOCMD_MAGIC, 0xc, int)

#define PCB_IOCMD_REGISTER_READ          _IOR(PCB_IOCMD_MAGIC, 10, int)
#define PCB_IOCMD_REGISTER_WRITE         _IOR(PCB_IOCMD_MAGIC, 11, int)

#define PCB_IOCMD_MAX_NUM	13

extern int eth_readid(void);
extern int battery_pcbtest(void);
extern int charger_state(void);
extern int camera_pcbtest(void);
extern int audio_play (void);
extern int audio_send_frame(char *buf, int frames);
extern void wifi_power(int power);
extern void mmc_scan(int flag);
extern int hdmi_pcbtest(void);
extern int audio_record(void);
extern int pcbtest_audio_total;
extern void msleep(unsigned int msecs);
extern int RDABT_get_chipid(void);
extern int imap_rtc_gettime(struct device *dev,struct rtc_time *rtc_tm);

const struct pcbtest_register base_map_info[] = {
{IMAP_IIS0_BASE, IMAP_IIS0_SIZE},
{IMAP_IIS1_BASE, IMAP_IIS1_SIZE},
{IMAP_TIMER_BASE,IMAP_TIMER_SIZE},
{IMAP_PWM_BASE,IMAP_PWM_SIZE},
{IMAP_IIC0_BASE,IMAP_IIC0_SIZE},
{IMAP_IIC1_BASE,IMAP_IIC1_SIZE},
{IMAP_IIC2_BASE,IMAP_IIC2_SIZE},
{IMAP_IIC3_BASE,IMAP_IIC3_SIZE},
{IMAP_IIC4_BASE,IMAP_IIC4_SIZE},
{IMAP_IIC5_BASE,IMAP_IIC5_SIZE},
{IMAP_PCM0_BASE	, IMAP_PCM0_SIZE},
{IMAP_PCM1_BASE,IMAP_PCM1_SIZE},
{IMAP_SSP0_BASE,IMAP_SSP0_SIZE},
{IMAP_SSP1_BASE,IMAP_SSP1_SIZE},
{IMAP_SSP2_BASE,IMAP_SSP2_SIZE},
{IMAP_SPDIF_BASE,IMAP_SPDIF_SIZE},
{IMAP_SIMC0_BASE,IMAP_SIMC0_SIZE},
{IMAP_SIMC1_BASE,IMAP_SIMC1_SIZE},
{IMAP_UART0_BASE,IMAP_UART0_SIZE},
{IMAP_UART1_BASE,IMAP_UART1_SIZE},
{IMAP_UART2_BASE,IMAP_UART2_SIZE},
{IMAP_UART3_BASE,IMAP_UART3_SIZE},
{IMAP_PIC0_BASE,IMAP_PIC0_SIZE},
{IMAP_PIC1_BASE,IMAP_PIC1_SIZE},
{IMAP_AC97_BASE,IMAP_AC97_SIZE},
{IMAP_KEYBD_BASE,IMAP_KEYBD_SIZE},
{IMAP_ADC_BASE,IMAP_ADC_SIZE},
{IMAP_DMIC_BASE,IMAP_DMIC_SIZE},
{IMAP_HDMI_BASE,IMAP_HDMI_SIZE},
{IMAP_GPIO_BASE,IMAP_GPIO_SIZE},
{IMAP_PWMA_BASE,IMAP_PWMA_SIZE},
{IMAP_GDMA_BASE,IMAP_GDMA_SIZE},
{IMAP_EMIF_BASE,IMAP_EMIF_SIZE},
{IMAP_SCU_BASE,IMAP_SCU_SIZE},
{IMAP_SYSMGR_BASE,IMAP_SYSMGR_SIZE},
{IMAP_IDS_BASE,IMAP_IDS_SIZE},
{IMAP_GPS_BASE,IMAP_GPS_SIZE},
{IMAP_CRYPTO_BASE,IMAP_CRYPTO_SIZE},
{IMAP_VDEC_BASE,IMAP_VDEC_SIZE},
{IMAP_VENC_BASE,IMAP_VENC_SIZE},
{IMAP_OHCI_BASE,IMAP_OHCI_SIZE},
{IMAP_EHCI_BASE,IMAP_EHCI_SIZE},
{IMAP_OTG_BASE,IMAP_OTG_SIZE},
{IMAP_MMC0_BASE,IMAP_MMC0_SIZE},
{IMAP_MMC1_BASE,IMAP_MMC1_SIZE},
{IMAP_MMC2_BASE,IMAP_MMC2_SIZE},
{IMAP_NAND_BASE,IMAP_NAND_SIZE},
{IMAP_SATA_BASE,IMAP_SATA_SIZE},
{IMAP_MAC_BASE,IMAP_MAC_SIZE}
};

static int pcb_open(struct inode *inode, struct file *file)
{
	return 0;
}

/* 
 * FIXME: 
 * This system call function should not be designed to be too much feature relates.
 */
static int pcb_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long pcb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int reg[2], i;
	unsigned int __user _reg;
	void __iomem    *map_reg;
	//printk("=========pcb_ioctl==========\n");

	/* Check IO control command access permission validation. */
	if (_IOC_TYPE(cmd) != PCB_IOCMD_MAGIC) {
		printk("Unknow IO control command magic.\n");
		return -EINVAL;
	} else if (_IOC_NR(cmd) > PCB_IOCMD_MAX_NUM){
		printk("Overflow IO control index.\n");
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ) {
		if (!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd))) {
			printk("IO control request read but buffer unwritable.\n");
			return -EINVAL;
		}
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd))) {
			printk("IO control request write but buffer unreadable.\n");
			return -EINVAL;
		}
	}

	/* Process all IO control requests. */
	switch (cmd) {
		case PCB_IOCMD_ETH_GETID:
			printk("PCB_IOCMD_ETH_GETID\n");
			ret = eth_readid();
			if(ret != 0)
				printk("eth getid failed\n");
			break;
		case PCB_IOCMD_BAT_GET:
			ret = battery_pcbtest();
			if(ret <= 0)
				printk("battery get value failed\n");
                        break;
                case PCB_IOCMD_CHARGER_ST:
                        ret = charger_state();
                        if(ret<0)
                            printk("Failed to get charger state\n");
                        break;
                case PCB_IOCMD_CAMERA_TEST:
                        ret = camera_pcbtest();
                        if(ret != 0)
                            printk("camera test failed\n");
                        break;
                case PCB_IOCMD_AUDIO_SEND_FRAME:
                        {
                            struct pcbtest_audio_frame xferi;
                            struct pcbtest_audio_frame __user *_xferi = (void __user *)arg;

                            if (copy_from_user(&xferi, _xferi, sizeof(xferi)))
                                return -EFAULT;   
                            if (audio_send_frame(xferi.buf, xferi.frames) < 0) {
                                printk("audio send frames err\n");
                                return -1;
                            }
                            break;
                        }
                case PCB_IOCMD_AUDIO_PLAY:
                        if (audio_play() < 0) {
                            printk("play song err\n");
                        }
                        pcbtest_audio_total = 0;
                        break;
                case PCB_IOCMD_RTC_TEST:
                        {
                            struct rtc_time tm;
                            imap_rtc_gettime(NULL,&tm);
                            if((tm.tm_sec < 60)&&(tm.tm_min < 60)&&(tm.tm_hour < 25)&&(tm.tm_mday < 32)&&(tm.tm_month < 13)){
                                printk("rtc pcbtest succed\n");
                            }
                            else
                                printk("rtc pcbtest failed\n");
                            ret = 0;
                            break;
                        }
                case PCB_IOCMD_AUDIO_RECORD:
                        ret = audio_record();
                        if(ret==0){
                            printk("Audio record failed\n");
                        }
                        ret = 0;
                        break;
                case PCB_IOCMD_WIFI_POWER_SDIO_ON:
                        printk("pcb enable wifi power\n");
                        wifi_power(1);
                        msleep(100);
                        mmc_scan(0);
                        break;
                case PCB_IOCMD_WIFI_POWER_ON:
                        printk("pcb enable wifi power\n");
                        wifi_power(1);
                        break;
                case PCB_IOCMD_HDMI_TEST:
                        ret = hdmi_pcbtest();
                        if(ret != 0)
                            printk("hdmi pcb test failed\n");
                        break;
                        /*************************************************************/
                case PCB_IOCMD_REGISTER_READ:
                        _reg = (void __user *)arg;
                        if (copy_from_user(reg, _reg, sizeof(unsigned int)))
                            return -EFAULT;
                        printk(KERN_DEBUG"PCB_IOCMD_REGISTER_READ copy from user register is 0x%x\n", reg[0]);
                        for(i=0; i<sizeof(base_map_info)/sizeof(base_map_info[0]); i++)//find which position to map
                            if((reg[0] >= base_map_info[i].base_addr) && 
                                    (reg[0] <= base_map_info[i].base_addr + base_map_info[i].map_size)) {
                                break;
                            }
                        if(i == sizeof(base_map_info)/sizeof(base_map_info[0])) {
                            printk("We can not find the register now!\n");
                            ret = -EINVAL;
                        }
                        map_reg = ioremap(base_map_info[i].base_addr, base_map_info[i].map_size);
                        reg[1]= readl(map_reg+reg[0]-base_map_info[i].base_addr);
                        printk(KERN_DEBUG"register 0x%x value is 0x%x \n ", reg[0], reg[1]);

                        copy_to_user(&(((int  __user *)arg)[1]), 
                                (const void *)&(reg[1]), sizeof(int));

                        ret =1;
                        break;
                case PCB_IOCMD_REGISTER_WRITE:
                        _reg = (void __user *)arg;
                        if (copy_from_user(reg, _reg, sizeof(unsigned int)*2))
                            return -EFAULT;
                        printk(KERN_DEBUG"PCB_IOCMD_REGISTER_WRITE copy from user register is 0x%x ,val=0x%x\n", reg[0],reg[1]);
                        for(i=0; i<sizeof(base_map_info)/sizeof(base_map_info[0]); i++)
                            if((reg[0] >= base_map_info[i].base_addr) && 
                                    (reg[0] <= base_map_info[i].base_addr + base_map_info[i].map_size) ){
                                break;
                            }
                        if(i == sizeof(base_map_info)/sizeof(base_map_info[0])) {
                            printk("We can not find the register now!");
                            ret = -EINVAL;
                        }
                        map_reg = ioremap(base_map_info[i].base_addr, base_map_info[i].map_size);
                        writel(reg[1],map_reg+reg[0]-base_map_info[i].base_addr);
                        printk(KERN_DEBUG"register 0x%x value is 0x%x \n", reg[0],  readl(map_reg+reg[0]-base_map_info[i].base_addr));
                        ret =1;
                        break;
                        break;	
                        /*************************************************************/
                default:
                        printk("Do not support for this command\n");
                        ret = -EINVAL;
                        break;
        }

        return ret;
}

static struct file_operations pcb_ops = 
{
	owner: THIS_MODULE,
	open: pcb_open,
	release: pcb_release,
	unlocked_ioctl: pcb_ioctl,
};

static struct miscdevice pcb_miscdev = 
{
	minor: MISC_DYNAMIC_MINOR,
	name: PCB_DEV_NAME,
	fops: &pcb_ops
};

static int __init pcb_init(void)
{

	if(item_integer("pcb_test.en",0)!=1)
		return -1;
	printk("=========pcb_init==========\n");

	return misc_register(&pcb_miscdev);
}

static void __exit pcb_exit(void)
{
	misc_deregister(&pcb_miscdev);
}

module_init(pcb_init);
module_exit(pcb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");

