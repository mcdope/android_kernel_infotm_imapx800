#ifndef __IMAPX_DMA__
#define __IMAPX_DMA__

/*
 * PL330 can assign any channel to communicate with
 * any of the peripherals attched to the DMAC.
 * For the sake of consistency across client drivers,
 * We keep the channel names unchanged and only add
 * missing peripherals are added.
 * Order is not important since IMAPX800 PL330 API driver
 * use these just as IDs.
 */
enum dma_ch {
    IMAPX800_SSP0_TX,
    IMAPX800_SSP0_RX,
    IMAPX800_SSP1_TX,
    IMAPX800_SSP1_RX,
    IMAPX800_I2S_SLAVE_TX,
    IMAPX800_I2S_SLAVE_RX,
    IMAPX800_I2S_MASTER_TX,
    IMAPX800_I2S_MASTER_RX,
    IMAPX800_PCM0_TX,
    IMAPX800_PCM0_RX,
    IMAPX800_AC97_TX,
    IMAPX800_AC97_RX,
    IMAPX800_UART0_TX,
    IMAPX800_TOUCHSCREEN_RX,
    IMAPX800_UART1_TX,
    IMAPX800_UART1_RX,
    IMAPX800_PCM1_TX,
    IMAPX800_PCM1_RX,
    IMAPX800_UART3_TX,
    IMAPX800_UART3_RX,
    IMAPX800_SPDIF_TX,
    IMAPX800_SPDIF_RX,
    IMAPX800_PWMA_TX,
    IMAPX800_DMIC_RX, /*  by Larry */
    /*  END Marker, also used to denote a reserved channel */
    IMAPX800_MAX,
};

#include <linux/dmaengine.h>

struct imapx800_dma_client {
    char *name;
};

struct imapx800_dma_prep_info {
	enum dma_transaction_type cap;
	enum dma_data_direction direction;
	dma_addr_t buf;
	unsigned long period;
	unsigned long len;
	void (*fp)(void *data);
	void *fp_param;
};

struct imapx800_dma_info {
	enum dma_transaction_type cap;
	enum dma_data_direction direction;
	enum dma_slave_buswidth width;
	dma_addr_t fifo;
	struct imapx800_dma_client *client;
};

struct imapx800_dma_ops {
	unsigned (*request)(enum dma_ch ch, struct imapx800_dma_info *info);
	int (*release)(unsigned ch, struct imapx800_dma_client *client);
	int (*prepare)(unsigned ch, struct imapx800_dma_prep_info *info);
	int (*trigger)(unsigned ch);
	int (*started)(unsigned ch);
	int (*flush)(unsigned ch);
	int (*stop)(unsigned ch);
	/**added by ayakashi for get dma real transfer position */
	int (*dma_getposition)(unsigned ch, dma_addr_t *src, dma_addr_t *dst);
};

#endif
