/*
 * for imapx800 gps module
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/cpumask.h>

#include <linux/spinlock.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/smp.h>

#include "imapx800-gps.h"
#include "gps-acq.h"

//#define GPS_DEBUG 1
#ifdef GPS_DEBUG
#define gps_dbg(x...) printk(x)
#else
#define gps_dbg(x...)
#endif

static struct acq_oper_control acq_oper;
//static struct acq_sat_info sat_info[ACQ_SAT_NUM];
static volatile unsigned int millisecond_int_flag = 0;

static inline uint32_t acq_cc_set(void __iomem *io_base, uint32_t state, struct acq_oper_sat_info *sat_info, struct acq_sty_info *sty)
{
	uint32_t i;
	uint32_t n = sat_info->sat_n;
	struct acq_sty *sty_xy = sty->strategy; 
	
	for(i = 0; i < n; i++){
		if(sat_info->cc_n[2 * i] == state){
		    sat_info->cc[i].crinc += sty_xy->fre_inc;
		    acq_cc_init(io_base, &sat_info->cc[i]);
		}
	}
	
        return 0;	
}

/*
 * acq oper init
 * 
 * set cc instr and enable acq irq
 * 
 * |             |               |               |
 * |--   x   --| |--  x  --| ... |--  x   --|    |
 * |           1 |         2 ... |         y     |
 *                                               |  <- done get result set ci_flag            
 * when x, y = 0 sets to 0 set cc_flag 
 * when y change sets to 1
 * 
 */
static uint32_t acq_oper_init(void __iomem *io_base, struct acq_oper_control *oper)
{
	uint32_t i;
	uint32_t tmp;
	uint32_t count = oper->ms_count;
	uint32_t state = oper->flag & 0xff;
	struct acq_oper_sat_info *info = &oper->info;
	struct acq_sty_info *sty = &oper->sty;
	
	/* x y strategy 
	 * second condition will happen when there more than one acq irq in 1ms
	 * */
	if((sty->sty_type == STY_X_Y) && sty->count == (count - 1)){
		sty->count++;
		if(sty->count == sty->total)
			oper->sty_flag = STY_DONE;
		/* cc set */
		if(!sty->x && !sty->y){
			info->cc_flag[0] = ENABLE;
			/* sets seq change to 0 */
			oper->set_seq = 0;
		}else
			oper->set_seq = 1;
		sty->x++;
		/* = x */
		if(sty->x == sty->strategy->coh_t){
			//  TODO
			// x y != 1 at the same time
			/* sets seq change to 2 */
			if(oper->set_seq)
				oper->set_seq = 2;
			sty->y++;
			/* x * y need get result */
			if((sty->x * sty->y) == sty->strategy->count){
				sty->inc_count++;
				for(i = 0; i < info->sat_n; i++){
					tmp = info->result_n[i * 2] - 1;
					info->ci_flag[tmp] = ENABLE;
				}
			}
			sty->x = 0;
		}
		/* = y */
		if(sty->y == sty->strategy->incoh_t)
			sty->y = 0;
	}
	
	/* set cc */
	if(oper->sets[oper->set_seq].sets_n > 1){
		for(i = 0; i < info->sat_n; i++){
			tmp = info->cc_n[i * 2] - 1;
			info->cc_flag[tmp] = ENABLE;
		}		
	}
	if(info->cc_flag[state - 1]){
		acq_cc_set(io_base, state, info, sty);
		info->cc_flag[state - 1] = DISABLE;
	}
	/* set instr */
        acq_instr_init(io_base, &oper->sets[oper->set_seq].set[state - 1]);
        /* enable acq irq */
	acq_operation_control(io_base, START);
	
	return 0;
}

/*
 * ci2 = ci1
 */
static inline void ci_set(struct CI_RESULT_STRUCT *ci1, struct CI_RESULT_STRUCT *ci2)
{
	ci2->cor_max = ci1->cor_max;
	ci2->cor_smax = ci1->cor_smax;
	ci2->cor_mean = ci1->cor_mean;
	ci2->cor_maxoff = ci1->cor_maxoff;
	ci2->cor_smaxoff = ci1->cor_smaxoff;	
}

static uint32_t acq_get_result(void __iomem *io_base, struct acq_oper_sat_info *info, uint32_t state, uint32_t count)
{
    uint32_t i;
    uint32_t n = info->sat_n;
    uint32_t *result = info->result_n;
    struct CI_RESULT_STRUCT tmp;
    
    for(i = 0; i < n; i++){
	    if(result[2 * i] == state){
		    acq_result_get(io_base, result[2 * i + 1], &tmp);
		    if(tmp.cor_max > info->result[i].cor_max){
			    ci_set(&tmp, &info->result[i]);
			    info->count[i] = count; 
		    }
	    }
    }

    return 0;	
}
/*
 * gps acq irq
 */
static irqreturn_t gps_acq_irq(int irq, void *dev_id)
{
    uint32_t state;
    uint32_t seq;
    struct imapx_gps *gps = dev_id;
    void __iomem *io_base = gps->io_base;
    struct acq_oper_sat_info *info = &acq_oper.info;
	
    // TODO
    if(acq_state_check(io_base)){
	    /* check state */
	    state = acq_oper.flag & 0xff;
	    /* get result */
	    if(info->ci_flag[state - 1]){
		    acq_get_result(io_base, info, state, acq_oper.sty.inc_count);
		    info->ci_flag[state - 1] = DISABLE;
	    }
	    /* change state */
	    seq = acq_oper.set_seq;
	    if(state < acq_oper.sets[seq].sets_n){
		    /* config next acq oper */
		    acq_oper.flag = state + 1;
		    acq_oper_init(io_base, (struct acq_oper_control *)&acq_oper);
	    }else{
		    acq_oper.flag = ACQ_OPER_CONTINUAL;
	    }
    }

    return IRQ_HANDLED;
}

static int track_count = 0;
/*
 *   track                                                acq 
 * ---------                                           ---------                                                  
 *     |<---   should check acq_en                         |
 *     |          (instr sets and enable)                  |       <- if more than 1 instr sets(1 set less than 18 instrs) 
 *     |       should check whether acq done                             init next instr sets and enable
 *          1 ms                                           |         
 *     |                                               ----------
 *     |                                                   |
 *     |                                                           <-  may more than one acq should oper
 *     |                                                   |           but they need be done in 1ms
 *     |                                               ----------
 *  --------
 */
static irqreturn_t gps_track_irq(int irq, void *dev_id)
{
    struct imapx_gps *gps = dev_id;
    void __iomem *io_base = gps->io_base;

    if(track_state_check(io_base))
	    millisecond_int_flag++;

    if(millisecond_int_flag == 1000){
        gps_dbg("gps_track_irq    cpu : %d   \n", smp_processor_id());
	gps_dbg("track_count : %d    \n", track_count++);
	millisecond_int_flag = 0;
    }

    return IRQ_HANDLED;
}

static void imapx_gps_init(struct imapx_gps *gps)
{
    gps_dbg("imapx_gps_init   \n");
    __raw_writel(0, IO_ADDRESS(GPS_IROM_ADDR));
    module_power_on(SYSMGR_GPS_BASE);           
    imapx_pad_cfg(IMAPX_GPS, 0); 
}

static void imapx_gps_close(struct imapx_gps *gps)
{
    module_power_down(SYSMGR_GPS_BASE);
}

static int imapx_gps_enable(struct imapx_gps *gps)
{
    void __iomem *io_base = gps->io_base;

    gps_dbg("imapx_gps_enable   \n");
    acq_operation_control(io_base, INIT);
    millisecond_int_flag = 0;
    track_state_check(io_base);
    track_int_control(io_base, ENABLE); 

    return 0;
}

static void imapx_gps_disable(struct imapx_gps *gps)
{
    void __iomem *io_base = gps->io_base;

    acq_operation_control(io_base, CLOSE);
}

static struct gps_ops imapx_gps_ops= {
    .init    = imapx_gps_init,
    .close   = imapx_gps_close,
    .enable  = imapx_gps_enable,
    .disable = imapx_gps_disable,
};

void gps_test(struct imapx_gps *gps)
{
    gps_dbg("gps_test \n");	
    gps->ops->init(gps);
    gps->ops->enable(gps);
}

/*
 * gps probe
 */
static int imapx_gps_probe(struct platform_device *pdev)
{
    int err = 0;
    int ret;
    struct imapx_gps *gps;
    struct resource *res;
    struct device *dev = &pdev->dev;
    // TODO  
    // irq affinity
    cpumask_var_t mask;
    mask->bits[0] = 0x2;
    printk("imapx800 gps init.   \n"); 
 
    gps = kzalloc(sizeof(struct imapx_gps), GFP_KERNEL);
    if(gps == NULL){
	    dev_err(dev, "unable to alloc data struct \n");
	    err = -ENOMEM;
	    goto err_alloc;
    }
    platform_set_drvdata(pdev, gps);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res){
	    dev_err(dev, "no MEM resource \n");
	    err = -ENODEV;
	    goto error;
    }

    gps->rsrc_start = res->start;
    gps->rsrc_len = resource_size(res);
    if(!request_mem_region(gps->rsrc_start, gps->rsrc_len, pdev->name)){
	    dev_err(dev, "requeset mem region err \n");
	    err = -EBUSY;
	    goto error;
    }

    gps->io_base = ioremap(gps->rsrc_start, gps->rsrc_len);
    if(!gps->io_base){
	    dev_err(dev, "ioremap error \n");
	    err = -ENXIO;
	    goto err_iomap;
    }

    gps->acq_irq = platform_get_irq(pdev, 0);
    if(gps->acq_irq < 0){
	    dev_err(dev, "get acq irq error \n");
	    err = -ENXIO;
	    goto err_acq_irq;
    }

    ret = request_irq(gps->acq_irq, gps_acq_irq, IRQF_DISABLED, dev_name(dev), 
		    gps);
    if(ret){
	    dev_err(dev, "unalbe to set up acq irq request \n");
	    err = -ENXIO;
	    goto err_acq_irq;
    }
    
    ret = irq_set_affinity(gps->acq_irq, mask);
    if(ret){
	    dev_err(dev, "acq irq set affinity cpu1 error \n");
	    err = -ENXIO;
	    goto err_acq_irq;
    }

    gps->tck_irq = platform_get_irq(pdev, 1);
    if(gps->tck_irq < 0){
	    dev_err(dev, "get track irq error \n");
	    err = -ENXIO;
	    goto err_tck_irq;
    }

    ret = request_irq(gps->tck_irq, gps_track_irq, IRQF_DISABLED, dev_name(dev), 
		    gps);
    if(ret){	
	    dev_err(dev, "unalbe to set up track irq request \n");
    	    err = -ENXIO;
	    goto err_tck_irq;
    }

    ret = irq_set_affinity(gps->tck_irq, mask);
    if(ret){
	    dev_err(dev, "tck irq set affinity cpu1 error \n");
	    err = -ENXIO;
	    goto err_tck_irq;
    }

    gps->clk = clk_get(dev, "gps");
    if(IS_ERR(gps->clk)){
            dev_err(dev, "clk error \n");
	    err = -ENODEV;
	    goto err_clk;
    }

    gps->ops = &imapx_gps_ops;
    
    printk("imapx800 gps init done.    \n");
   // gps_test(gps);

    return 0;
err_clk:

err_tck_irq:
    free_irq(gps->tck_irq, gps);
err_acq_irq:
    free_irq(gps->acq_irq, gps);
err_iomap:
    iounmap(gps->io_base);
error:
    platform_set_drvdata(pdev, NULL);
err_alloc:
    kfree(gps);

    return err;
}

/*
 * gps remove
 */
static int imapx_gps_remove(struct platform_device *pdev)
{
    struct imapx_gps *gps = platform_get_drvdata(pdev);
    
    iounmap(gps->io_base);
    if(gps->acq_irq >= 0)
  	    free_irq(gps->acq_irq, gps);
    if(gps->tck_irq >= 0)
  	    free_irq(gps->tck_irq, gps);
    kfree(gps);
    platform_set_drvdata(pdev, NULL);

    return 0;
}

static struct platform_driver imapx_gps_driver = {
    .probe  = imapx_gps_probe,
    .remove = imapx_gps_remove,
    .driver = {
	    .name  = "imap-gps",
	    .owner = THIS_MODULE,
    },
};

static int __init imapx_gps_module_init(void)
{
    return platform_driver_register(&imapx_gps_driver);
}

static void __exit imapx_gps_module_exit(void)
{
    platform_driver_unregister(&imapx_gps_driver);
}

module_init(imapx_gps_module_init);
module_exit(imapx_gps_module_exit);

MODULE_AUTHOR("Larry Liu");
MODULE_DESCRIPTION("imapx800 gps");
MODULE_LICENSE("GPL");

