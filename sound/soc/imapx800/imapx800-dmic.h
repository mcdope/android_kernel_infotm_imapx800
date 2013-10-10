#ifndef __IMAPX800_DMIC_H__
#define __IMAPX800_DMIC_H__

#define DMIC_CTL                      0x00
#define DMIC_CFG                      0x04
#define DMIC_IRQ_STAT                 0x08
#define DMIC_FIFO_STAT                0x0c
#define DMIC_FIFO                     0x10

/* DMIC_CTL */
#define DMIC_DMIC_ON                 0
#define DMIC_FIFO_ENABLE             1
#define DMIC_CLK_ENABLE              2

//#define DMIC_CIC_OUT                 4       /* no use imapx800 */
//#define DMIC_CICCOMP_OUT             5       /* no use imapx800*/

/* DMIC_CFG */
#define DMIC_CLK_RATIO               0       /* clk ratio pclk/(ratio + 1) = mic_clk * 2     0 ~ 7 */
#define DMIC_FIFO_DIPSTICK           8       /* 8 ~ 12 */
#define DMIC_DIRECT_FMT              13      /* DMIC_DFMT_16 or DMIC_DFMT_1 */
#define DMIC_SAMPLE_MODE             14      /* DMIC_NOSAMPLE, DMIC_RISING, DMIC_FALLING or DMIC_BOTH */
#define DMIC_BIT_SWAP                16
#define DMIC_BYTE_SWAP               17
#define DMIC_HW_SWP                  18
#define DMIC_DIRECT_EN               19
#define DMIC_DMA_ENABLE              20      
#define DMIC_FIFO_UNDER_IRQ_EN       28      /* still read when it is empty */
#define DMIC_FIFO_OVER_IRQ_EN        29      /* still transfer when it is full */
#define DMIC_FIFO_AFULL_IRQ_EN       30      /* over the (32 - fifo dipstick) */
#define DMIC_IRQ_EN                  31      

#define DMIC_NOSAMPLE                0
#define DMIC_RISING                  1
#define DMIC_FALLING                 2
#define DMIC_BOTH                    3

/* rDMIC_IRQ_STAT */
#define DMIC_FIFO_AFULL_INT          2
#define DMIC_FIFO_OVER_INT           1
#define DMIC_FIFO_UNDER_INT          0

/* rDMIC_FIFO_STAT */
#define DMIC_FIFO_EMPTY              0
#define DMIC_FIFO_FULL               1
#define DMIC_FIFO_AEMPTY             2
#define DMIC_FIFO_AFULL              3
#define DMIC_FIFO_WORD_CNT           4      /* 4 ~ 9 */

#define DMIC_DFMT_16                 0      /* low 16 bit store left data    high 16 bit store high data */
#define DMIC_DFMT_1                  1      /* left right 1 bit to 1 bit */

#define ENABLE                       1
#define DISABLE                      0

struct dmic_info {
	uint32_t clk_ratio;              /* 0 ~ 7 */
	uint32_t fifo_dipstick;          /* 00 0000 fifo deep is 32 - fifo_dipstick */
	uint32_t state;                  /* */
};

/* dmic state */
#define DMIC_SAMPLE_MODE_BIT             0  /* bit 0 : 1 DMIC_NOSAMPLE, DMIC_RISING, DMIC_FALLING, DMIC_BOTH */
#define DMIC_FMT_BIT                     2  /* bit 2 DMIC_DFMT_16 or DMIC_DFMT_1 */
#define DMIC_IRQ_EN_BIT                  3  /* bit 3 interrupt enable or disable */
#define DMIC_FIFO_OVER_IRQ_EN_BIT        4  /* bit 4 fifo over irq */
#define DMIC_FIFO_UNDER_IRQ_EN_BIT       5  /* bit 5 fifo under irq */
#define DMIC_FIFO_AFULL_IRQ_EN_BIT       6  /* bit 6 fifo afull irq */

#include <linux/io.h>

struct imapx_dmic {
	struct device   *dev;
	void __iomem    *io_base;
	struct clk       *clk;
    int clk_rate;
	struct dmic_info *dmic;
    struct dmic_soft_info *dmic_soft;
	spinlock_t lock;
};

#define DMIC_FIFO_DEEP                 32

#define DMIC_DEFAULT_CLK_R             2
#define DMIC_DEFAULT_FIFO_LEVEL        16
#define DMIC_DEFAULT_STATE_VALUE       0

/* TODO
 *
 * right?? DMIC_CLK_HW_CON shoudl be configed depends on the hardware
 *  */
#define DMIC_LEFT_TRACK                DMIC_FALLING
#define DMIC_RIGHT_TRACK               DMIC_RISING
#define DMIC_DOUBLE_TRACK              DMIC_BOTH

#define DMIC_CLK_HW_CON                DMIC_LEFT_TRACK

#define DMIC_FIFO_SET(x)               (DMIC_FIFO_DEEP - (x))

#define DMIC_IF_EN                     0x21E13820

#endif /* imapx-dmic.h */
