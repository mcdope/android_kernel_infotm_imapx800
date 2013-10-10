#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/suspend.h>

#include <asm/system.h>

#include <mach/hardware.h>
#include <mach/items.h>
#include <mach/imap-pwm.h>

spinlock_t lock;

static int cpufreq_trust_lvl = 5, cpufreq_bench_lvl = 4,
	   cpufreq_top_lvl = 5, // set 804M as our default value
	   cpufreq_fix_lvl = -1;
static struct cpufreq_frequency_table imapx800_freq_table[] = {
    { 0,  1032000},                                             
    { 1,  996000 },                                             
    { 2,  948000 },                                             
    { 3,  900000 },                                             
    { 4,  852000},                                             
    { 5,  804000 },                                             
    { 6,  756000 },
    { 7,  708000 }, 
    { 8,  660000 },
    { 9,  612000 },
    { 10, 564000 },
    { 11, 516000 }, 
    { 12, 468000 },
    { 13, 420000 },
    { 14, 372000 },
    { 15, 324000 },
    { 16, 276000 },
    { 17, 228000 },
    { 18, 180000 },
    { 19, 132000 },
    { 20, CPUFREQ_TABLE_END },                                  
};

static struct clk *apll_clk;

//static unsigned long target_cpu_speed[NR_CPUS];
static DEFINE_MUTEX(imapx800_cpu_lock);
static bool is_suspended;

void imapx800_lvl_up(void) {
	printk(KERN_ERR "top lvl increased.\n");
	cpufreq_top_lvl = cpufreq_bench_lvl;
}
void imapx800_lvl_down(void) {
	printk(KERN_ERR "top lvl decreased.\n");
	cpufreq_top_lvl = cpufreq_trust_lvl;
}
EXPORT_SYMBOL(imapx800_lvl_up);
EXPORT_SYMBOL(imapx800_lvl_down);

void imapx800_freq_message(int m, int v)
{
	static int a[4] = {0, 0, 0, 0};

	if(m < 0 || m > 3) return ;
	a[m] = v;
	printk(KERN_DEBUG "f%d: %4d %4d %4d\n",
			raw_smp_processor_id(), a[0], a[1], a[2]);
}
EXPORT_SYMBOL(imapx800_freq_message);

void imapx800_cpufreq_fix(int lvl)
{
	if(lvl < 0) lvl = -1;
	else {
		if(lvl > 19) lvl = 19;
		if(lvl < cpufreq_top_lvl) lvl = cpufreq_top_lvl;
	}

	cpufreq_fix_lvl = lvl;
}
EXPORT_SYMBOL(imapx800_cpufreq_fix);
void imapx800_cpufreq_setlvl(int lvl)
{
	if(lvl > 19) lvl = 19;
	if(lvl < 0) imapx800_cpufreq_fix(-1);
	else imapx800_cpufreq_fix(19 - lvl);
}
EXPORT_SYMBOL(imapx800_cpufreq_setlvl);

int imapx800_cpufreq_getval(void)
{
	return (clk_get_rate(apll_clk) / 1000000);
}

int imapx800_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy, imapx800_freq_table);
}

unsigned int imapx800_getspeed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return (clk_get_rate(apll_clk) / 1000);
}

static long long __getns(void)
{
	ktime_t a;
	a = ktime_get_boottime();
	return a.tv64;
}

static int imapx800_update_cpu_speed(unsigned long rate)
{
	int ret = 0, i;
	struct cpufreq_freqs freqs;
	unsigned long flags;
//#define CONFIG_CFSHOW_TIME
#ifdef CONFIG_CFSHOW_TIME
	long long a, b;
	
	a = __getns();
#endif

	freqs.old = imapx800_getspeed(0);
	if(cpufreq_fix_lvl > 0)
		freqs.new = imapx800_freq_table[cpufreq_fix_lvl].frequency;
	else
		freqs.new = min(rate, imapx800_freq_table[cpufreq_top_lvl].frequency);
//	freqs.new = rate;

	if (freqs.old == freqs.new)
		return ret;

	for_each_online_cpu(freqs.cpu)
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

    for (i=0;imapx800_freq_table[i].frequency != CPUFREQ_TABLE_END;i++)
        if (imapx800_freq_table[i].frequency == freqs.new)
            break;
    if (i == ARRAY_SIZE(imapx800_freq_table)) {
        ret = -1;
        goto err;
    }

	spin_lock_irqsave(&lock, flags);

	pwm_cpufreq_prechange(imapx800_freq_table[i].frequency*1000);

	ret = clk_set_rate(apll_clk, imapx800_freq_table[i].frequency*1000);
	pwm_cpufreq_trans();

	spin_unlock_irqrestore(&lock, flags);
	if (ret < 0) {
		pr_err("cpu-imapx800: Failed to set cpu frequency to %d kHz\n",
			freqs.new);
        goto err;
	}
	
    ret = 0;
err:
	for_each_online_cpu(freqs.cpu)
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

#ifdef CONFIG_CFSHOW_TIME
	b = __getns();

	a = b - a;
	do_div(a, 1000);
	
	printk(KERN_ERR "cpuf->%uM %lldus\n", freqs.new / 1000, a);
#else
//	printk(KERN_DEBUG "cpuf->%uM\n", freqs.new / 1000);
	imapx800_freq_message(0, freqs.new / 1000);
#endif
    return ret;
}

#if 0
static unsigned long imapx800_cpu_highest_speed(void)
{
	unsigned long rate = 0;
	int i;

	for_each_online_cpu(i)
		rate = max(rate, target_cpu_speed[i]);
	return rate;
}
#endif

static int imapx800_target(struct cpufreq_policy *policy,
		       unsigned int target_freq,
		       unsigned int relation)
{
	int idx;
	unsigned int freq;
	int ret = 0;

	mutex_lock(&imapx800_cpu_lock);

	if (is_suspended) {
		ret = -EBUSY;
		goto out;
	}

	cpufreq_frequency_table_target(policy, imapx800_freq_table, target_freq,
		relation, &idx);

	freq = imapx800_freq_table[idx].frequency;

//	target_cpu_speed[policy->cpu] = freq;
	ret = imapx800_update_cpu_speed(freq);

out:
	mutex_unlock(&imapx800_cpu_lock);
	return ret;
}

static int imapx800_pm_notify(struct notifier_block *nb, unsigned long event,
	void *dummy)
{
	mutex_lock(&imapx800_cpu_lock);
	if (event == PM_SUSPEND_PREPARE) {
		is_suspended = true;
#if 0
		pr_info("imapx800 cpufreq suspend: setting frequency to %d kHz\n",
			imapx800_freq_table[0].frequency);
		imapx800_update_cpu_speed(imapx800_freq_table[0].frequency);
#endif
	} else if (event == PM_POST_SUSPEND) {
		is_suspended = false;
	}
	mutex_unlock(&imapx800_cpu_lock);

	return NOTIFY_OK;
}

static struct notifier_block imapx800_cpu_pm_notifier = {
	.notifier_call = imapx800_pm_notify,
};

#if 0
static void imapx800_set_freq_table(void)
{
    struct cpufreq_frequency_table *freq;

    freq = imapx800_freq_table;                                        
    if (clk_get_rate(cpu_clk) != CPU_SET_IN_NORMAL) {         
        while (freq->frequency != CPUFREQ_TABLE_END) {              
            freq->frequency = clk_get_rate(cpu_clk) /                  
                ((freq->index + 1) * 1000);           
            freq++;
        }            
    }                                                                  

    while (freq->frequency != CPUFREQ_TABLE_END) {                     
        unsigned long r;                                               

        /*  Check for frequencies we can generate */                    
        r = clk_round_rate(cpu_clk, ratio[freq->index].cpu_axi_apb);   
        if (r != ratio[freq->index].cpu_axi_apb) {                     
            pr_err("cpufreq: %dkHz unsupported by clock\n",            
                    freq->frequency);                                     
            freq->frequency = CPUFREQ_ENTRY_INVALID;                   
        }                                                              

        /* frequency is the maximum we can support. */                 
        if (freq->frequency > imapx800_getspeed(0))
            freq->frequency = CPUFREQ_ENTRY_INVALID;                   

        freq++;                                                        
    }                                                                  
}
#endif

static int imapx800_cpu_init(struct cpufreq_policy *policy)
{
	if (policy->cpu >= NR_CPUS)
		return -EINVAL;

	apll_clk = clk_get(NULL, "apll");
	if (IS_ERR(apll_clk))
		return PTR_ERR(apll_clk);
	clk_enable(apll_clk);
    
        //imapx800_set_freq_table();
	cpufreq_frequency_table_cpuinfo(policy, imapx800_freq_table);
	cpufreq_frequency_table_get_attr(imapx800_freq_table, policy->cpu);
	policy->cur = imapx800_getspeed(policy->cpu);
//	target_cpu_speed[policy->cpu] = policy->cur;

	/* FIXME: what's the actual transition time? */
	policy->cpuinfo.transition_latency = 100 * 1000;

	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	cpumask_copy(policy->related_cpus, cpu_possible_mask);
	if (policy->cpu == 0)
		register_pm_notifier(&imapx800_cpu_pm_notifier);

	return 0;
}

static int imapx800_cpu_exit(struct cpufreq_policy *policy)
{
	cpufreq_frequency_table_cpuinfo(policy, imapx800_freq_table);
	clk_disable(apll_clk);
    clk_put(apll_clk);
	return 0;
}

static struct freq_attr *imapx800_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver imapx800_cpufreq_driver = {
	.verify		= imapx800_verify_speed,
	.target		= imapx800_target,
	.get		= imapx800_getspeed,
	.init		= imapx800_cpu_init,
	.exit		= imapx800_cpu_exit,
	.name		= "imapx800",
	.attr		= imapx800_cpufreq_attr,
};

extern void infotm_freq_init(void);
static int __init imapx800_cpufreq_init(void)
{

	infotm_freq_init();
	spin_lock_init(&lock);

	if(item_exist("cpu.freq.adjust")) {
		int f0 = item_integer("cpu.freq.adjust", 0) * 1000,
		    f1 = item_integer("cpu.freq.adjust", 1) * 1000, i;
		printk(KERN_ERR "CPU frequency gain from item: %d.%d\n",
				f0 / 1000, f1 / 1000);

		f0 += imapx800_freq_table[cpufreq_trust_lvl].frequency;
		f1 += imapx800_freq_table[cpufreq_trust_lvl].frequency;

		for(i = 0; imapx800_freq_table[i].frequency
				!= CPUFREQ_TABLE_END; i++)
			if(f0 >= imapx800_freq_table[i].frequency) {
				cpufreq_trust_lvl = i;
				break;
			}

		for(i = 0; imapx800_freq_table[i].frequency
				!= CPUFREQ_TABLE_END; i++)
			if(f1 >= imapx800_freq_table[i].frequency) {
				cpufreq_bench_lvl = i;
				break;
			}

	} else printk(KERN_ERR "CPU scaling frequency: (default)\n");

	if(cpufreq_bench_lvl > cpufreq_trust_lvl)
		cpufreq_bench_lvl = cpufreq_trust_lvl;

	cpufreq_top_lvl = cpufreq_trust_lvl;
	printk(KERN_ERR "CPU scaling frequency: %d.%d\n", cpufreq_trust_lvl, cpufreq_bench_lvl);

    return cpufreq_register_driver(&imapx800_cpufreq_driver);
}
module_init(imapx800_cpufreq_init);

static void __exit imapx800_cpufreq_exit(void)
{
    cpufreq_unregister_driver(&imapx800_cpufreq_driver);
}
module_exit(imapx800_cpufreq_exit);

MODULE_AUTHOR("Sun");
MODULE_DESCRIPTION("cpufreq driver for imapx800");
MODULE_LICENSE("GPL");
