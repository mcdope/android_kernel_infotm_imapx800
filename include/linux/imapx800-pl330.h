/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef	__DMA_PL330_H_
#define	__DMA_PL330_H_

#define IMAPX800_DMAF_AUTOSTART		(1 << 0)
#define IMAPX800_DMAF_CIRCULAR		(1 << 1)

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
    IMAPX800_PCM_TX,
    IMAPX800_PCM_RX,
    IMAPX800_AC97_TX,
    IMAPX800_AC97_RX,
	IMAPX800_UART0_TX,
	IMAPX800_UART0_RX,
	IMAPX800_UART1_TX,
	IMAPX800_UART1_RX,
	IMAPX800_UART2_TX,
	IMAPX800_UART2_RX,
	IMAPX800_UART3_TX,
	IMAPX800_UART3_RX,
    IMAPX800_SPDIF_TX,
    IMAPX800_SPDIF_RX,
    IMAPX800_PWMA_TX,
    IMAPX800_DMIC_RX, /* by Larry */
	/* END Marker, also used to denote a reserved channel */
	IMAPX800_MAX,
};

/*
 * Every PL330 DMAC has max 32 peripheral interfaces,
 * of which some may be not be really used in your
 * DMAC's configuration.
 * Populate this array of 32 peri i/fs with relevant
 * channel IDs for used peri i/f and IMAPX800_MAX for
 * those unused.
 *
 * The platforms just need to provide this info
 * to the DMA API driver for PL330.
 */
struct imapx800_pl330_platdata {
    enum dma_ch peri[32];
};

static inline bool imapx800_dma_has_circular(void)
{
	return true;
}

enum imapx800_dma_buffresult {
    IMAPX800_RES_OK,
    IMAPX800_RES_ERR,
    IMAPX800_RES_ABORT
};

enum imapx800_dmasrc {
    IMAPX800_DMASRC_HW,     /*  source is memory */
    IMAPX800_DMASRC_MEM     /*  source is hardware */
};

/* enum imapx800_chan_op
 * 
 * operation codes passed to the DMA code by the user, and also used
 * to inform the current channel owner of any changes to the system state
 */

enum imapx800_chan_op {
    IMAPX800_DMAOP_START,
    IMAPX800_DMAOP_STOP,
    IMAPX800_DMAOP_PAUSE,
    IMAPX800_DMAOP_RESUME,
    IMAPX800_DMAOP_FLUSH,
    IMAPX800_DMAOP_TIMEOUT,     /*  internal signal to handler */
    IMAPX800_DMAOP_STARTED,     /*  indicate channel started */
};

struct imapx800_dma_client {
    char *name;
};

struct imapx800_dma_chan;

/* imapx800_dma_cbfn_t
 *
 * buffer callback routine type
 */

typedef void (*imapx800_dma_cbfn_t)(struct imapx800_dma_chan *,
        void *buf, int size,
        enum imapx800_dma_buffresult result);

typedef int  (*imapx800_dma_opfn_t)(struct imapx800_dma_chan *,
        enum imapx800_chan_op );



/* imapx800_dma_request
 * 
 * request a dma channel exclusivley
 */

extern int imapx800_dma_request(unsigned int channel,
        struct imapx800_dma_client *, void *dev);


/* imapx800_dma_ctrl
 * 
 * change the state of the dma channel
 */

extern int imapx800_dma_ctrl(unsigned int channel, enum imapx800_chan_op op);

/* imapx800_dma_setflags
 * 
 * set the channel's flags to a given state
 */

extern int imapx800_dma_setflags(unsigned int channel,
        unsigned int flags);

/* imapx800_dma_free
 *
 * free the dma channel (will also abort any outstanding operations)
 */

extern int imapx800_dma_free(unsigned int channel, struct imapx800_dma_client *);

/* imapx800_dma_enqueue
 *
 * place the given buffer onto the queue of operations for the channel.
 * The buffer must be allocated from dma coherent memory, or the Dcache/WB
 * drained before the buffer is given to the DMA system.
 */

extern int imapx800_dma_enqueue(unsigned int channel, void *id,
        dma_addr_t data, int size);

/* imapx800_dma_config
 *
 * configure the dma channel
 */

extern int imapx800_dma_config(unsigned int channel, int xferunit, int brst_len);

/* imapx800_dma_devconfig
 *
 * configure the device we're talking to
 */

extern int imapx800_dma_devconfig(unsigned int channel,
        enum imapx800_dmasrc source, unsigned long devaddr);

/* imapx800_dma_devconfig
 *
 * configure the device we're talking to
 */

extern int imapx800_dma_devconfig(unsigned int channel,
        enum imapx800_dmasrc source, unsigned long devaddr);

/* imapx800_dma_getposition
 * 
 * get the position that the dma transfer is currently at
 */

extern int imapx800_dma_getposition(unsigned int channel,
        dma_addr_t *src, dma_addr_t *dest);

extern int imapx800_dma_set_opfn(unsigned int, imapx800_dma_opfn_t rtn);
extern int imapx800_dma_set_buffdone_fn(unsigned int, imapx800_dma_cbfn_t rtn);

#endif	/* __DMA_PL330_H_ */
