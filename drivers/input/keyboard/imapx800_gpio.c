#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm-generic/bitops/non-atomic.h>
#include <mach/imap-iomap.h>
#include <mach/pad.h>
#include <mach/power-gate.h>
#include <mach/items.h>
#include <linux/reboot.h>

#ifdef CONFIG_FAKE_PM
#include <fake_pm.h>
extern struct completion power_key;
#endif

#define KEY_RESET       0x1314

extern int infotm_nand_lock(void); 
extern int infotm_nand_unlock(void); 

#define INTTYPE_LOW     (0)
#define INTTYPE_HIGH    (1)
#define INTTYPE_FALLING (2)
#define INTTYPE_RISING  (4)
#define INTTYPE_BOTH    (6)

//#define	DEBUG
#ifdef	DEBUG
#define	gpio_dbg(x...)	printk(KERN_ERR "GPIO DEBUG:" x)
#else
#define gpio_dbg(x...)
#endif
static int codec_spk = 0;
static int codec_hp = 0;
struct imapx_gpio {
	char	name[16];
	int	code;
	int	irq;
	int	index;		/*pad index*/
	int	num;		/*0~25*/
	int 	type;		/*interrupt type*/
	int	flt;		/*filter*/
	struct work_struct	work;
	struct input_dev	*input;

	struct delayed_work     power_work;
    struct delayed_work     emu_work;
	u32			key_poll_delay;
	bool			keydown;
};

struct imapx_gpio imapx_gpio[] = {
	{
		.name	= "key-home",
		.code	= KEY_HOME,
        .type   = INTTYPE_BOTH,
		.flt	= 0xff,
	},
	{
		.name   = "key-back",
		.code	= KEY_BACK,
		.type	= INTTYPE_BOTH,
		.flt	= 0xff,
	},

	{
		.name   = "key-power",
		.code   = KEY_POWER,
	},
 	{
		.name   = "btsd30",
		.code   = KEY_PHONE,
		.type   = INTTYPE_BOTH,
		.flt    = 0xff,
	},
	{
		.name   = "key-reset",
		.code	= KEY_RESET,
		.type	= INTTYPE_BOTH,
		.flt	= 0xff,
	},
	{
		.name   = "key-vol+",
		.code   = KEY_VOLUMEUP,
		.type   = INTTYPE_BOTH,
		.flt 	= 0xff,
	},
	{
		.name   = "key-vol-",
		.code   = KEY_VOLUMEDOWN,
		.type   = INTTYPE_BOTH,
		.flt 	= 0xff,
	},
};

static int loop;
static int invoke;
static struct timer_list spk_timer;
static unsigned long phonelt = 0; 
extern int imapx_audioRunStatus(void);
extern int imapx800_force_closeSpk(void);
int imapx_gpio_spken(int en) 
{
	printk(KERN_DEBUG"A en=%d\n",en);
	imapx_pad_set_mode(1,1,codec_spk);
	imapx_pad_set_dir(0,1,codec_spk);
	imapx_pad_get_indat(codec_hp)?: (en = 0);   
	if(item_exist("codec.hp_highlevel_en") && item_integer("codec.hp_highlevel_en", 1))
  	en =!en;
	printk(KERN_DEBUG"B en=%d\n",en);

    if(imapx800_force_closeSpk()) {
        en = 0;
    }
	
    if(0 == imapx_audioRunStatus())
		en=0;
	printk(KERN_DEBUG"C en=%d\n",en);			 
	imapx_pad_set_outdat(!!en,1,codec_spk);

	return 0;
}

void imapx_gpio_spk_delayed_en(unsigned long x)
{
	if(jiffies > (phonelt
				+ (500 * HZ) / 1000 - 1)) 
	{
		imapx_gpio_spken(1);
	}
}

EXPORT_SYMBOL(imapx_gpio_spken);

static void imapx_emu_key_work(struct delayed_work *work)
{
    struct imapx_gpio *ptr = container_of(work, struct imapx_gpio, emu_work);
    unsigned int stat = 0x10;

    if(item_exist("board.cpu") == 0 || 
            item_equal("board.cpu", "imapx820", 0)){
        stat = 0x10;
    }else if(item_equal("board.cpu", "i15", 0)){
        stat = 0x40;
    }

    if(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x58)) & stat) {
        loop++;

        if(loop >= 350 && invoke == 0) {
            invoke = 1;
            infotm_nand_lock();
        }
    }else {
        ptr->keydown = false;
        input_event(ptr->input, EV_KEY, ptr->code, 0);
        input_sync(ptr->input);
        infotm_nand_unlock();
    }

    if(ptr->keydown)
        schedule_delayed_work(&ptr->emu_work, msecs_to_jiffies(10));
}

int imap_iokey_emu(int index)
{
    if(imapx_gpio[index].code != KEY_POWER) {
	    input_event(imapx_gpio[index].input, EV_KEY, imapx_gpio[index].code, 1);
	    input_sync(imapx_gpio[index].input);
	    input_event(imapx_gpio[index].input, EV_KEY, imapx_gpio[index].code, 0);
	    input_sync(imapx_gpio[index].input);
    }else {
        loop = 0;
        invoke = 0;
        imapx_gpio[index].keydown = true;
        input_event(imapx_gpio[index].input, EV_KEY, imapx_gpio[index].code, 1);
        input_sync(imapx_gpio[index].input);

        schedule_delayed_work(&imapx_gpio[index].emu_work, msecs_to_jiffies(10));
    }

	printk("imap_iokey_emu\n");
	return 0;
}

EXPORT_SYMBOL(imap_iokey_emu);

extern void imapfb_shutdown(void);
static void imapx_gpio_work(struct work_struct *work)
{
	struct imapx_gpio *ptr = container_of(work, struct imapx_gpio,work);

	if(unlikely(ptr->code == KEY_PHONE))
	{
		/* head phone status changed */
		phonelt = jiffies;
		mod_timer(&spk_timer,
				jiffies + msecs_to_jiffies(1000));
	}
    /*SOFT RESET  added by peter*/
    else if(unlikely(ptr->code == KEY_RESET))
    {
        //printk(KERN_ERR "KEY_reset pressed\n");
        sys_sync();
        imapfb_shutdown();
        __raw_writel(0x2, IO_ADDRESS(SYSMGR_RTC_BASE));        
        __raw_writel(0xff, IO_ADDRESS(SYSMGR_RTC_BASE + 0x28));
        __raw_writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0x7c));   
    }
	else
	{
		input_event(ptr->input, EV_KEY, ptr->code, !imapx_pad_get_indat(ptr->index));
		input_sync(ptr->input);
	}
	gpio_dbg("Reporting KEY:name=%s, code=%d, val=%d\n", ptr->name, ptr->code, !imapx_pad_get_indat(ptr->index));

	return;
}

static void __onsleep_sync(struct work_struct *work)
{
	printk(KERN_ERR "Pre-syncing fs ...\n");
	sys_sync();
	printk(KERN_ERR "Pre-syncing done.\n");
}
static DECLARE_WORK(onsleep_sync_work, __onsleep_sync);
struct workqueue_struct *onsleep_work_queue;

static void imapx_pwkey_work(struct work_struct *work)
{
	struct imapx_gpio *ptr = container_of(to_delayed_work(work), struct imapx_gpio,power_work);
    unsigned int pwkey_stat = 0x10;

    if(item_exist("board.cpu") == 0 || 
            item_equal("board.cpu", "imapx820", 0)){
        pwkey_stat = 0x10;
    }else if(item_equal("board.cpu", "i15", 0)){
        pwkey_stat = 0x40;
    }

	if(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x58)) & pwkey_stat)
	{
		loop++;

		if(!ptr->keydown)
		{
			/* key down */
			ptr->keydown = true;
			input_event(ptr->input, EV_KEY, ptr->code, 1);
			input_sync(ptr->input);
			gpio_dbg("Reporting KEY Down:name=%s, code=%d\n", ptr->name, ptr->code);

			/* system may shutdown suddenly */
			queue_work(onsleep_work_queue, &onsleep_sync_work);
		}

		if(loop >= 350 && invoke == 0){
			invoke = 1;
			//orderly_poweroff(1);
			infotm_nand_lock();
		}
	}
	else
	{
		/* key up */
		ptr->keydown = false;
		input_event(ptr->input, EV_KEY, ptr->code, 0);
		input_sync(ptr->input);
		gpio_dbg("Reporting KEY Up:name=%s, code=%d\n", ptr->name, ptr->code);
		infotm_nand_unlock();		
	}

	if(ptr->keydown)
	{
		schedule_delayed_work(&ptr->power_work, msecs_to_jiffies(ptr->key_poll_delay));
	}
	else
	{
		writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));
	}
}

static irqreturn_t imapx_gpio_isr(int irq,void *dev_id)
{
	struct imapx_gpio *ptr = dev_id;

	//printk("%s invoked with name %s\n", __func__, ptr->name);
    //gpio_dbg("%s invoked with name %s\n", __func__, ptr->name);

	if(ptr->code == KEY_POWER)
	{
		if(readl(IO_ADDRESS(SYSMGR_RTC_BASE + 0x8)) & 0x3)//check int status
		{
#ifdef CONFIG_FAKE_PM
			if(if_in_suspend == 1)
			{
				complete(&power_key);
			}
#endif

			writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));//mask int
			writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE + 0x4));//clear int

			loop = 0;
			invoke = 0;

			schedule_delayed_work(&ptr->power_work, msecs_to_jiffies(ptr->key_poll_delay));
		}
	}
	else if(imapx_pad_irq_pending(ptr->index))
	{
		imapx_pad_irq_clear(ptr->index);
		schedule_work(&ptr->work);
	}

	return IRQ_HANDLED;
}

static int __devinit imapx_gpio_probe(struct platform_device *pdev)
{
	struct input_dev *input;
	int i;
	int error;
	char buf[ITEM_MAX_LEN];

	setup_timer(&spk_timer, imapx_gpio_spk_delayed_en, 0);
    
	if(item_exist("codec.spk"))
		codec_spk = item_integer("codec.spk",1); 
	else
		codec_spk = 53;
    
	if(item_exist("codec.hp"))
		codec_hp = item_integer("codec.hp",1); 
	else
		codec_hp = 30;

	input = input_allocate_device();
	if(!input)
	{
		printk(KERN_ERR "allocate input device failed.\n");
		return -ENOMEM;
	}

	/* work queue must be created before IRQ is registered */
	printk(KERN_ERR "Create workqueue for onsleep sync.\n");
	onsleep_work_queue = create_singlethread_workqueue("ossync");
	if (onsleep_work_queue == NULL) {
		printk(KERN_ERR "FaiLED to create onsleep work queue.\n");
	}
		
	for(i = 0;i < ARRAY_SIZE(imapx_gpio);i++)
	{
		if(imapx_gpio[i].code == KEY_POWER)
		{
			writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));

            INIT_DELAYED_WORK(&imapx_gpio[i].emu_work, imapx_emu_key_work);
			INIT_DELAYED_WORK(&imapx_gpio[i].power_work, imapx_pwkey_work);
			imapx_gpio[i].irq = GIC_MGRPM_ID;
			imapx_gpio[i].key_poll_delay = 10;
			imapx_gpio[i].keydown = false;

			imapx_gpio[i].input = input;
			input_set_capability(input, EV_KEY, imapx_gpio[i].code);
			error = request_irq(imapx_gpio[i].irq, imapx_gpio_isr,
					0, imapx_gpio[i].name, &imapx_gpio[i]);
			if(error)
			{
				printk(KERN_ERR "claim irq %d fail,name is %s\n",
						imapx_gpio[i].irq,imapx_gpio[i].name);
				goto __fail_exit__;
			}
			else
			{
				printk(KERN_ERR "claim irq %d success,name is %s\n",
						imapx_gpio[i].irq,imapx_gpio[i].name);
			}
		}
		else
		{
			INIT_WORK(&imapx_gpio[i].work, imapx_gpio_work);

			if(imapx_gpio[i].code == KEY_HOME)
			{
				if(item_exist("keys.home"))
				{
					item_string(buf, "keys.home", 0);
					if(!strncmp(buf,"pads",4))
					{
						imapx_gpio[i].index = item_integer("keys.home", 1);
					}else
						continue;
				} else
					continue;
			}
			else if(imapx_gpio[i].code == KEY_BACK)
			{
				if(item_exist("keys.back"))
				{
					item_string(buf, "keys.back", 0);
					if(!strncmp(buf,"pads",4))
					{
						imapx_gpio[i].index = item_integer("keys.back", 1);
					} else 
						continue;
				} else
					continue;
			}
			else if(imapx_gpio[i].code == KEY_RESET)
			{
				if(item_exist("keys.reset"))
				{
					item_string(buf, "keys.reset", 0);
					if(!strncmp(buf,"pads",4))
					{
						imapx_gpio[i].index = item_integer("keys.reset", 1);
					} else 
						continue;
				} else
					continue;
			}
			else if(imapx_gpio[i].code == KEY_VOLUMEUP)
			{
				if(item_exist("keys.volup"))
				{
					item_string(buf, "keys.volup", 0);
					if(!strncmp(buf,"pads",4))
					{
						imapx_gpio[i].index = item_integer("keys.volup", 1);
					} else 
						continue;
				} else
					continue;
			}
			else if(imapx_gpio[i].code == KEY_VOLUMEDOWN)
			{
				if(item_exist("keys.voldown"))
				{
					item_string(buf, "keys.voldown", 0);
					if(!strncmp(buf,"pads",4))
					{
						imapx_gpio[i].index = item_integer("keys.voldown", 1);
					} else 
						continue;
				} else
					continue;
			}
			else if(imapx_gpio[i].code == KEY_PHONE)
			{
				if(item_exist("board.cpu") == 0 || 
						item_equal("board.cpu", "imapx820", 0)){
					imapx_gpio[i].index = imapx_pad_index(imapx_gpio[i].name);
				}else if(item_equal("board.cpu", "i15", 0)){
					if(item_exist("codec.hp")){
						item_string(buf, "codec.hp", 0);
						if(!strncmp(buf,"pads",4)){
							imapx_gpio[i].index = item_integer("codec.hp", 1);
						}else
							continue;
					}else
						continue;
				}
			}
            else
                continue;

			if(imapx_gpio[i].index < 0)
				continue;

			imapx_gpio[i].irq = imapx_pad_irq_number(imapx_gpio[i].index);
			imapx_pad_set_mode(0, 1, imapx_gpio[i].index);
			imapx_pad_set_dir(1, 1, imapx_gpio[i].index);
			imapx_pad_irq_config(imapx_gpio[i].index, imapx_gpio[i].type, imapx_gpio[i].flt);

			imapx_gpio[i].input = input;
			input_set_capability(input, EV_KEY, imapx_gpio[i].code);

            if(item_exist("board.cpu") == 0 ||
                    item_equal("board.cpu", "imapx820", 0)){
			    imapx_gpio[i].num = imapx_pad_irq_group_num(imapx_gpio[i].index);
			    if((imapx_gpio[i].num >= 10) && (imapx_gpio[i].num <= 25))
			    {
				    error = request_irq(imapx_gpio[i].irq, imapx_gpio_isr, 
				    		IRQF_SHARED, imapx_gpio[i].name, &imapx_gpio[i]);
			    }
			    else if((imapx_gpio[i].num < 10) && (imapx_gpio[i].num >= 0))
			    {
				    error = request_irq(imapx_gpio[i].irq, imapx_gpio_isr, 0, imapx_gpio[i].name, &imapx_gpio[i]);
			    }
			    else
			    {
				    error = -EINVAL;
			    }
            }else if(item_equal("board.cpu", "i15", 0)){
                error = request_irq(imapx_gpio[i].irq, imapx_gpio_isr,
                        IRQF_SHARED, imapx_gpio[i].name, &imapx_gpio[i]);
            }

			if(error)
			{
				printk(KERN_ERR "claim irq %d fail,name is %s\n",
						imapx_gpio[i].irq,imapx_gpio[i].name);
				goto __fail_exit__;
			}
			else
			{
				printk(KERN_ERR "claim irq %d success,name is %s\n",
						imapx_gpio[i].irq,imapx_gpio[i].name);
			}
		}
	}

	input->name = "imap-gpio";
	input->phys = "imap-gpio/input0";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	error = input_register_device(input);
	if(error)
	{
		pr_err("imap-gpio: Unable to register input device, "
				"error: %d\n", error);
		goto __fail_exit__;
	}

	return 0;

__fail_exit__:
	printk("GPIO ERROR EXIT\n");
	input_free_device(input);

	return error;
}

static int __devexit imapx_gpio_remove(struct platform_device *pdev)
{
	input_unregister_device(imapx_gpio->input);
	return 0;
}


#ifdef	CONFIG_PM
static int imapx_gpio_suspend(struct platform_device *pdev,pm_message_t state)
{
	printk("imapx_gpio_suspend\n");
	writel(0x3, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));//mask int
	imapx_gpio[2].keydown = false;
	return 0;
}

static int imapx_gpio_resume(struct platform_device *pdev)
{
	int i;

	writel(0, IO_ADDRESS(SYSMGR_RTC_BASE + 0xC));

	for(i = 0;i < ARRAY_SIZE(imapx_gpio);i++)
	{
		if(imapx_gpio[i].index)
		{
			imapx_pad_set_mode(0, 1, imapx_gpio[i].index);
			imapx_pad_set_dir(1, 1, imapx_gpio[i].index);
			imapx_pad_irq_config(imapx_gpio[i].index, imapx_gpio[i].type, imapx_gpio[i].flt);
		}
	}

	return 0;
}
#else

#define	imapx_gpio_suspend	NULL
#define	imapx_gpio_resume	NULL

#endif

static struct platform_driver imapx_gpio_device_driver = {
	.probe		= imapx_gpio_probe,
	.remove		= __devexit_p(imapx_gpio_remove),
	.suspend	= imapx_gpio_suspend,
	.resume		= imapx_gpio_resume,
	.driver		= {
		.name	= "imap-gpio",
		.owner	= THIS_MODULE,
	}
};

static int __init imapx_gpio_init(void)
{
	//module_power_on(SYSMGR_GPIO_BASE);
	return platform_driver_register(&imapx_gpio_device_driver);
}
module_init(imapx_gpio_init);

static void __exit imapx_gpio_exit(void)
{
	platform_driver_unregister(&imapx_gpio_device_driver);
}
module_exit(imapx_gpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:imap-gpio");


