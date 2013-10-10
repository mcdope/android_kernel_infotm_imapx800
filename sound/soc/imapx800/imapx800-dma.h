#ifndef _IMAPX800_DMA_H_
#define _IMAPX800_DMA_H_

#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)

#include <linux/imap_pl330.h>

struct imapx800_pcm_dma_params {
	struct imapx800_dma_client *client;	/* stream identifier */
	int channel;
    dma_addr_t dma_addr;
	int dma_size;			/* Size of the DMA transfer */
    unsigned ch;
    struct imapx800_dma_ops *ops;
};

struct imapx800_item_params {
    char *name;
    int  i2c_addr;
    void *dai_link;
    int num_links;
    int (*imap_switch)(int dir);
};

#endif
