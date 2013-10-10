#ifndef __IMAPX800_GPS_H__
#define __IMAPX800_GPS_H__

#include <linux/io.h>
#include "gps-reg.h"

#define ACQ_SAT_NUM     32

struct acq_sat_info{
    uint32_t result_n;
    uint32_t crinc;
    struct CC_STRUCT cc;
    struct CI_RESULT_STRUCT result;
};

struct imapx_gps;

struct gps_ops{
    void    (*init)(struct imapx_gps *);
    void    (*close)(struct imapx_gps *);
    int     (*enable)(struct imapx_gps *);    
    void    (*disable)(struct imapx_gps *);
};

struct imapx_gps{
    struct device    *dev;
    struct gps_ops   *ops;
    void __iomem     *io_base;
    struct clk       *clk;
    int acq_irq;
    int tck_irq;
    resource_size_t  rsrc_start;
    resource_size_t  rsrc_len;
};

#define GPS_IROM_ADDR     0x21e0ac10

#endif /* imapx800-gps.h */
