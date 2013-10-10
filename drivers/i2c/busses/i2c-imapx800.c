#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <mach/imap-iic.h>
#include <mach/pad.h>
#include <mach/power-gate.h>

//#define I2C_DEBUG
#ifdef I2C_DEBUG
#define i2c_dbg(x...)	printk(KERN_ERR "IIC INFO:" x)
#else
#define i2c_dbg(x...)	
#endif

#define IMAPX800_PAD_CFG_ADDR	(0x21e09000)


static char *abort_sources[] = {
	[ABRT_7B_ADDR_NOACK]	= "the address sent was not acknowledged by any slave(7-bit mode)",
	[ABRT_10BADDR1_NOACK]	= "the first 10-bit address byte was not acknowledged by any slave(10-bit mode)",
	[ABRT_10BADDR2_NOACK]	= "the second address byte of 10-bit address was not acknowledged by any slave(10-bit mode)",
	[ABRT_TXDATA_NOACK]	= "data was not acknowledged",
	[ABRT_GCALL_NOACK]	= "no acknowledgement for a general call",
	[ABRT_GCALL_READ]	= "read after general call",
	[ABRT_HS_ACKDET]	= "the high speed master code was acknowledged",
	[ABRT_SBYTE_ACKDET]	= "start byte was acknowledged",
	[ABRT_HS_NORSTRT]	= "trying to use the master to transfer data in high speed mode when restart was disabled",
	[ABRT_SBYTE_NORSTRT]	= "trying to send start byte when restart is disabled",
	[ABRT_10B_RD_NORSTRT]	= "trying to read when restart is disabled (10bit mode)",
	[ABRT_MASTER_DIS]	= "trying to use disabled adapter",
	[ARB_LOST]		= "lost arbitration",
	[ABRT_SLVFLUSH_TXFIFO]	= "slave received read command when data exist in TX FIFO",
	[ABRT_SLV_ARBLOST]	= "slave lost bus when transmitting data",
	[ABRT_SLVRD_INTX]	= "write 1 to bit8 of IIC_DATA_CMD register when processor responds to a slave mode request for data to be transmit",
};

/*
 * struct ima_i2c_dev - private imapx800_i2c data
 * @dev: driver model device node
 * @base: IO registers pointer
 * @cmd_complete: tx completion indicator
 * @lock: protect this struct and IO registers
 * @clk: input reference clock
 * @cmd_err: run time hadware error code
 * @msgs: points to an array of messages currently being transferred
 * @msgs_num: the number of elements in msgs
 * @msg_write_idx: the element index of the current tx message in the msgs array
 * @tx_buf_len: the length of the current tx buffer
 * @tx_buf: the current tx buffer
 * @msg_read_idx: the element index of the current rx message in the msgs array
 * @rx_buf_len: the length of the current rx buffer
 * @rx_buf: the current rx buffer
 * @msg_err: error status of the current transfer
 * @status: i2c master status, one of STATUS_*
 * @abort_source: copy of the TX_ABRT_SOURCE register
 * @irq: interrupt number for the i2c master
 * @adapter: i2c subsystem adapter node
 * @tx_fifo_depth: depth of the hardware tx fifo
 * @rx_fifo_depth: depth of the hardware rx fifo
 */
struct imap_i2c_dev {
	struct device		*dev;
	void __iomem		*base;
	struct completion	cmd_complete;
	struct mutex		lock;
	struct clk		*clk;
	int			cmd_err;
	struct i2c_msg		*msgs;
	int			msgs_num;
	int			msg_write_idx;
	uint32_t		tx_buf_len;
	uint8_t			*tx_buf;
	int			msg_read_idx;
	uint32_t		rx_buf_len;
	uint8_t			*rx_buf;
	int			msg_err;
	unsigned int		status;
	uint32_t		abort_source;
	int			irq;
	struct i2c_adapter	adapter;
	unsigned int		tx_fifo_depth;
	unsigned int		rx_fifo_depth;
	int			suspended;

	int 			id;
};

#if 0
static uint32_t imap_i2c_scl_hcnt(uint32_t ic_clk_khz, uint32_t scl_high_100times, int offset)
{
	return (((ic_clk_khz*scl_high_100times)/100000)-8) < 6 ? 6 : (((ic_clk_khz*scl_high_100times)/100000)-8) + offset;
}

static uint32_t imap_i2c_scl_lcnt(uint32_t ic_clk_khz, uint32_t scl_low_100times, int offset)
{
	return (((ic_clk_khz*scl_low_100times)/100000)-1) < 8 ? 8 : (((ic_clk_khz*scl_low_100times)/100000)-1) + offset;
}
#endif

static void imap_i2c_reg_init(struct imap_i2c_dev *dev)
{
	uint32_t i2c_con;
	uint16_t hcnt;
	uint16_t lcnt;

	/* Disable the adapter */
	writel(0, dev->base + IIC_ENABLE);

	switch(dev->id) {
		case 2:
			writel(0x80, dev->base + IIC_IGNORE_ACK0);

			hcnt = readl(dev->base + IIC_SS_SCL_HCNT);
			lcnt = readl(dev->base + IIC_SS_SCL_LCNT);
			writel((hcnt >> 1), dev->base + IIC_SS_SCL_HCNT);
			writel((lcnt >> 1), dev->base + IIC_SS_SCL_LCNT);

			/* Configure Tx/Rx FIFO threshold levels */
			writel(dev->tx_fifo_depth - 8, dev->base + IIC_TX_TL);
			writel(0, dev->base + IIC_RX_TL);

			break;

		default:
			writel(0xf0, dev->base + IIC_IGNORE_ACK0);

			/* Configure Tx/Rx FIFO threshold levels */
			writel(dev->tx_fifo_depth - 1, dev->base + IIC_TX_TL);
			writel(0, dev->base + IIC_RX_TL);

			break;
	}

//	if(dev->id == 2)
//		writel(0x80, dev->base + IIC_IGNORE_ACK0);
//	else
//		writel(0xf0, dev->base + IIC_IGNORE_ACK0);

	/* set speed deviders for standard/fast/high */
#if 0
	/* standard speed */
	hcnt = imap_i2c_scl_hcnt(clk_khz,
				 400,	/* 4us */
				 0);	/* no offset */
	lcnt = imap_i2c_scl_lcnt(clk_khz,
				 470,	/* 4.7us */
				 0);	/* no offset */
	writel(hcnt, dev->base + IIC_SS_SCL_HCNT);
	writel(lcnt, dev->base + IIC_SS_SCL_LCNT);
	printk(KERN_ERR "Standard-mode HCNT:LCNT = %d:%d\n", hcnt, lcnt);
	
	/* fast speed */
	hcnt = imap_i2c_scl_hcnt(clk_khz,
				 60,	/* 0.6us */
				 0);	/* no offset */
	lcnt = imap_i2c_scl_lcnt(clk_khz,
				 130,	/* 1.3us */
				 0);	/* no offset */
	writel(hcnt, dev->base + IIC_FS_SCL_HCNT);
	writel(lcnt, dev->base + IIC_FS_SCL_LCNT);
	i2c_dbg("%p Fast-mode HCNT:LCNT = %d:%d\n", dev, hcnt, lcnt);

	/* high speed */
	hcnt = imap_i2c_scl_hcnt(clk_khz,
				 6,	/* 0.06us */
				 0);	/* no offset*/
	lcnt = imap_i2c_scl_lcnt(clk_khz,
				 16,	/* 0.16us */
				 0);	/* no offset */
	writel(hcnt, dev->base + IIC_HS_SCL_HCNT);
	writel(lcnt, dev->base + IIC_HS_SCL_LCNT);
	i2c_dbg("%p High-mode HCNT:LCNT = %d:%d\n", dev, hcnt, lcnt);
#endif

//	if(dev->id == 2)
//	{
//		uint16_t hcnt;
//		uint16_t lcnt;

//		hcnt = readl(dev->base + IIC_SS_SCL_HCNT);
//		lcnt = readl(dev->base + IIC_SS_SCL_LCNT);
//		writel((hcnt >> 1), dev->base + IIC_SS_SCL_HCNT);
//		writel((lcnt >> 1), dev->base + IIC_SS_SCL_LCNT);
//	}

	/* Configure Tx/Rx FIFO threshold levels */
//	writel(dev->tx_fifo_depth - 1, dev->base + IIC_TX_TL);
//	writel(0, dev->base + IIC_RX_TL);

	/* configure the i2c master */
	i2c_con = IMAP_IIC_CON_MASTER | IMAP_IIC_CON_SLAVE_DISABLE |
			IMAP_IIC_CON_RESTART_EN | IMAP_IIC_CON_SPEED_SS;
	writel(i2c_con, dev->base + IIC_CON);
}

static int imap_i2c_wait_bus_ready(struct imap_i2c_dev *dev)
{
	int timeout = IMAP_IIC_TIMEOUT;

	while(readl(dev->base + IIC_STATUS) & IMAP_IIC_STATUS_ACTIVITY)
	{
		if (timeout <= 0)
		{
			printk(KERN_ERR "timeout waiting for bus ready\n");
			return -ETIMEDOUT;
		}
		timeout--;
		mdelay(1);
	}

	return 0;
}

static void imap_i2c_xfer_init(struct imap_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	uint32_t i2c_con;

	/* Disable the adapter */
	writel(0, dev->base + IIC_ENABLE);

	/* set the slave (target) address */
	writel(msgs[dev->msg_write_idx].addr, dev->base + IIC_TAR);

	/* if the slave address is ten bit address, enable 10BITADDR */
	i2c_con = readl(dev->base + IIC_CON);
	if(msgs[dev->msg_write_idx].flags & I2C_M_TEN)
		i2c_con |= IMAP_IIC_CON_10BITADDR_MASTER;
	else
		i2c_con &= ~IMAP_IIC_CON_10BITADDR_MASTER;
	writel(i2c_con,dev->base + IIC_CON);

	/* enable the adapter */
	writel(1, dev->base + IIC_ENABLE);

	/* enable interrupt*/
	writel(IMAP_IIC_INTR_DEFAULT_MASK, dev->base + IIC_INTR_MASK);
}

static int imap_i2c_handle_tx_abort(struct imap_i2c_dev *dev)
{
	unsigned long abort_source = dev->abort_source;
	int i;

	if(abort_source & IMAP_IIC_TX_ABRT_NOACK)
	{
		for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
			printk("%s: %s\n", __func__, abort_sources[i]);
		return -EREMOTEIO;
	}

	for_each_set_bit(i, &abort_source, ARRAY_SIZE(abort_sources))
		printk("%s: %s\n", __func__, abort_sources[i]);

	if(abort_source & IMAP_IIC_TX_ARB_LOST)
		return -EAGAIN;
	else if(abort_source & IMAP_IIC_TX_ABRT_GCALL_READ)
		return -EINVAL;
	else
		return -EIO;
}

static int imap_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct imap_i2c_dev *dev = i2c_get_adapdata(adap);
	int ret;

	i2c_dbg("%s: msgs: %d\n", __func__, num);

	if(dev->suspended == 1)
		return -EIO;

	mutex_lock(&dev->lock);

	INIT_COMPLETION(dev->cmd_complete);
	dev->msgs = msgs;
	dev->msgs_num = num;
	dev->cmd_err = 0;
	dev->msg_write_idx = 0;
	dev->msg_read_idx = 0;
	dev->msg_err = 0;
	dev->status = STATUS_IDLE;
	dev->abort_source = 0;

	ret = imap_i2c_wait_bus_ready(dev);
	if(ret < 0)
		goto end;

	/* start the transfers */
	imap_i2c_xfer_init(dev);

	/* wait for tx to complete */
	ret = wait_for_completion_interruptible_timeout(&dev->cmd_complete, HZ);
	if(!ret)
	{
		printk(KERN_ERR "controller timed out\n");
		imap_i2c_reg_init(dev);
		ret = -ETIMEDOUT;
		goto end;
	}
	else if(ret < 0)
		goto end;

	if (dev->msg_err)
	{
		ret = dev->msg_err;
		goto end;
	}

	/* no error */
	if (likely(!dev->cmd_err))
	{
		writel(0, dev->base + IIC_ENABLE);
		ret = num;
		goto end;
	}

	/* error occur */
	if (dev->cmd_err == IMAP_IIC_ERR_TX_ABRT)
	{
		ret = imap_i2c_handle_tx_abort(dev);
		goto end;
	}

	ret = -EIO;

end:
	mutex_unlock(&dev->lock);

	return ret;
}

static uint32_t imap_i2c_func(struct i2c_adapter *adap)
{
	return  I2C_FUNC_I2C |
		I2C_FUNC_10BIT_ADDR |
		I2C_FUNC_SMBUS_BYTE |
		I2C_FUNC_SMBUS_BYTE_DATA |
		I2C_FUNC_SMBUS_WORD_DATA |
		I2C_FUNC_SMBUS_I2C_BLOCK;
}

static uint32_t imap_i2c_read_clear_intrbits(struct imap_i2c_dev *dev)
{
	uint32_t stat;

	stat = readl(dev->base + IIC_INTR_STAT);

	/*
	 * Do not use the IC_CLR_INTR register to clear interrupts,
	 * instead, use the separately-prepared IC_CLR_* registers.
	 */
	if(stat & IMAP_IIC_INTR_RX_UNDER)
		readl(dev->base + IIC_CLR_RX_UNDER);
	if(stat & IMAP_IIC_INTR_RX_OVER)
		readl(dev->base + IIC_CLR_RX_OVER);
	if(stat & IMAP_IIC_INTR_TX_OVER)
		readl(dev->base + IIC_CLR_TX_OVER);
	if(stat & IMAP_IIC_INTR_RD_REQ)
		readl(dev->base + IIC_CLR_RD_REQ);
	if(stat & IMAP_IIC_INTR_TX_ABRT)
	{
		/* read IIC_CLR_TX_ABRT register will clear IIC_TX_ABRT_SOURCE 
		 * register,so we have to read IIC_TX_ABRT_SOURCE first*/
		dev->abort_source = readl(dev->base + IIC_TX_ABRT_SOURCE);
		readl(dev->base + IIC_CLR_TX_ABRT);
	}
	if(stat & IMAP_IIC_INTR_RX_DONE)
		readl(dev->base + IIC_CLR_RX_DONE);
	if(stat & IMAP_IIC_INTR_ACTIVITY)
		readl(dev->base + IIC_CLR_ACTIVITY);
	if(stat & IMAP_IIC_INTR_STOP_DET)
		readl(dev->base + IIC_CLR_STOP_DET);
	if(stat & IMAP_IIC_INTR_START_DET)
		readl(dev->base + IIC_CLR_START_DET);
	if(stat & IMAP_IIC_INTR_GEN_CALL)
		readl(dev->base + IIC_CLR_GEN_CALL);

	return stat;
}

static void imap_i2c_xfer_msg(struct imap_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	uint32_t intr_mask;
	int tx_limit, rx_limit;
	uint32_t addr = msgs[dev->msg_write_idx].addr;
	uint32_t buf_len = dev->tx_buf_len;
	uint8_t *buf = dev->tx_buf;
	intr_mask = IMAP_IIC_INTR_DEFAULT_MASK;

	for(; dev->msg_write_idx < dev->msgs_num; dev->msg_write_idx++)
	{
		if(msgs[dev->msg_write_idx].addr != addr)
		{
			/*
			 * if target address has changed, we need to
			 * reprogram the target address in the i2c
			 * adapter when we are done with this transfer
			 */
			printk(KERN_ERR "%s: invalid target address\n", __func__);
			dev->msg_err = -EINVAL;
			break;
		}

		if(msgs[dev->msg_write_idx].len == 0)
		{
			printk(KERN_ERR "%s: invalid message length\n", __func__);
			dev->msg_err = -EINVAL;
			break;
		}

		if (!(dev->status & STATUS_WRITE_IN_PROGRESS))
		{
			/* new i2c_msg */
			buf = msgs[dev->msg_write_idx].buf;
			buf_len = msgs[dev->msg_write_idx].len;
//			printk("buf_len is %d\n", buf_len);
//			for (i = 0;i < buf_len;i++)
//			  printk("buf[%d] is %x\n", i, buf[i]);
		}

		tx_limit = dev->tx_fifo_depth - readl(dev->base + IIC_TXFLR);
		rx_limit = dev->rx_fifo_depth - readl(dev->base + IIC_RXFLR);
//		printk("tx_limit is %x, rx_limit is %x\n", tx_limit, rx_limit);
//		printk("tx_fifo_depth is %d, rx_fifo_depth is %d, IIC_TXFLR is %d, IIC_RXFLR is %d\n", 
//					dev->tx_fifo_depth, dev->rx_fifo_depth, readl(dev->base + IIC_TXFLR), 
//					readl(dev->base + IIC_RXFLR));
		while(buf_len > 0 && tx_limit > 0 && rx_limit > 0)
		{
			if(msgs[dev->msg_write_idx].flags & I2C_M_RD)
			{
		//		printk("I2C_M_RD\n");
				writel(0x100, dev->base + IIC_DATA_CMD);
				rx_limit--;
			}
			else {
		//		printk("write is %x\n", *buf);
				writel(*buf++, dev->base + IIC_DATA_CMD);
			}

			tx_limit--;
			buf_len--;
		}

		dev->tx_buf = buf;
		dev->tx_buf_len = buf_len;

		if(buf_len > 0)
		{
			dev->status |= STATUS_WRITE_IN_PROGRESS;
			break;
		}
		else
			dev->status &= ~STATUS_WRITE_IN_PROGRESS;
	}

	/*
	 * If i2c_msg index search is completed, we
	 * do not need TX_EMPTY interrupt any more
	 */
	if(dev->msg_write_idx == dev->msgs_num)
		intr_mask &= ~IMAP_IIC_INTR_TX_EMPTY;

	if(dev->msg_err)
		intr_mask = 0;

	writel(intr_mask, dev->base + IIC_INTR_MASK);
}

static void imap_i2c_read(struct imap_i2c_dev *dev)
{
	struct i2c_msg *msgs = dev->msgs;
	int rx_valid;
	uint32_t len;
	uint8_t *buf;

	for(; dev->msg_read_idx < dev->msgs_num; dev->msg_read_idx++)
	{
		if(!(msgs[dev->msg_read_idx].flags & I2C_M_RD))
			continue;

		if(!(dev->status & STATUS_READ_IN_PROGRESS))
		{
			len = msgs[dev->msg_read_idx].len;
			buf = msgs[dev->msg_read_idx].buf;
			//printk("1: len = %x\n",len);
		}
		else
		{
			len = dev->rx_buf_len;
			buf = dev->rx_buf;
			//printk("2: len = %x\n",len);
		}

		rx_valid = readl(dev->base + IIC_RXFLR);
		//printk("len is %x, rx_valid is %x\n", len, rx_valid);
		for(; len > 0 && rx_valid > 0; len--, rx_valid--, buf++) {
			*buf = readl(dev->base + IIC_DATA_CMD);
		//	printk("read is %x\n", *buf);
		}

		if(len > 0)
		{
			dev->status |= STATUS_READ_IN_PROGRESS;
			dev->rx_buf_len = len;
			dev->rx_buf = buf;
			return;
		}
		else
			dev->status &= ~STATUS_READ_IN_PROGRESS;
	}
}

static irqreturn_t imap_i2c_isr(int irq, void *dev_id)
{
	struct imap_i2c_dev *dev = dev_id;
	uint32_t stat;

	stat = imap_i2c_read_clear_intrbits(dev);

	if(stat & IMAP_IIC_INTR_TX_ABRT)
	{
		dev->cmd_err |= IMAP_IIC_ERR_TX_ABRT;
		dev->status = STATUS_IDLE;

		printk(KERN_ERR "%s: irq=%d stat=0x%x\n",__func__, irq, stat);
		writel(0, dev->base + IIC_INTR_MASK);
		goto exit;
	}

	if(stat & IMAP_IIC_INTR_RX_FULL)
		imap_i2c_read(dev);

	if(stat & IMAP_IIC_INTR_TX_EMPTY)
		imap_i2c_xfer_msg(dev);

exit:
	if ((stat & (IMAP_IIC_INTR_TX_ABRT | IMAP_IIC_INTR_STOP_DET)) || dev->msg_err)
		complete(&dev->cmd_complete);

	return IRQ_HANDLED;
}

static struct i2c_algorithm imap_i2c_algo = {
	.master_xfer    = imap_i2c_xfer,
	.functionality  = imap_i2c_func,
};

static int imap_i2c_module_init(int num)
{
	int ret = -1;

	switch(num)
	{
		case 0:
			ret = imapx_pad_cfg(IMAPX_I2C0,0);
			break;

		case 1:
			ret = imapx_pad_cfg(IMAPX_I2C1,0);
			break;

		case 2:
			ret = imapx_pad_cfg(IMAPX_I2C2,0);
			break;

		case 3:
			ret = imapx_pad_cfg(IMAPX_I2C3,0);
			break;

		case 4:
			ret = imapx_pad_cfg(IMAPX_I2C4,0);
			break;

		case 5:
			ret = imapx_pad_cfg(IMAPX_I2C5,0);
			break;
	}

	return ret;
}

static int __devinit imap_i2c_probe(struct platform_device *pdev)
{
	struct imap_i2c_dev *dev;
	struct i2c_adapter *adap;
	struct resource *mem, *ioarea;
	int irq, ret;
	void    __iomem     *iopad;
	unsigned int i2c_pad;
    cpumask_var_t mask;
    mask->bits[0] = 0x2;

	//uint32_t param;

	printk(KERN_ERR "+++%s invoked by %p+++\n",__func__,pdev);

	//TODO: iic pull disable 
	iopad = ioremap(IMAPX800_PAD_CFG_ADDR, 0x100000);

	if(iopad == NULL){
		printk("i2c iopad remap failed\n");
	}

	i2c_pad = readl(iopad + 0x2c);
	i2c_pad &= ~(0x3<<6);
	writel(i2c_pad, iopad + 0x2c);

	i2c_pad = readl(iopad + 0x30);
	i2c_pad &= ~(0x1f<<0);
	writel(i2c_pad, iopad + 0x30);
	
	/* driver uses the static register mapping */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!mem)
	{
		printk(KERN_ERR "no mem resource!\n");
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, 0);
	if(irq < 0)
	{
		printk(KERN_ERR "no irq resource!\n");
		return irq;
	}

	ioarea = request_mem_region(mem->start, resource_size(mem),pdev->name);
	if(!ioarea)
	{
		printk(KERN_ERR "I2C region already claimed!\n");
		return -EBUSY;
	}

	dev = kzalloc(sizeof(struct imap_i2c_dev), GFP_KERNEL);
	if(!dev)
	{
		ret = -ENOMEM;
		goto release_region;
	}

	init_completion(&dev->cmd_complete);
	mutex_init(&dev->lock);
	dev->dev = get_device(&pdev->dev);
	dev->irq = irq;
	platform_set_drvdata(pdev, dev);

	if(imap_i2c_module_init(pdev->id) == -1)
	{
		i2c_dbg("imapx800 i2c module init failed, please sure case configure\n");
		ret = -EINVAL;
		goto free_mem;
	}

	switch(pdev->id)
	{
		case 0:
			dev->clk = clk_get(NULL, "i2c0");
			break;

		case 1:
			dev->clk = clk_get(NULL, "i2c1");
			break;

		case 2:
			dev->clk = clk_get(NULL, "i2c2");
			break;

		case 3:
			dev->clk = clk_get(NULL, "i2c3");
			break;

		case 4:
			dev->clk = clk_get(NULL, "i2c4");
			break;

		case 5:
			dev->clk = clk_get(NULL, "i2c5");
			break;
	}

	if (IS_ERR(dev->clk))
	{
		ret = -ENODEV;
		goto free_mem;
	}
	clk_enable(dev->clk);

	dev->base = ioremap(mem->start, resource_size(mem));
	if(dev->base == NULL)
	{
		printk(KERN_ERR "failure mapping io resources!\n");
		ret = -EBUSY;
		goto unuse_clock;
	}

	//param = readl(dev->base + IIC_COMP_PARAM_1);
	//dev->tx_fifo_depth = ((param >> 16) & 0xff) + 1;
	//dev->rx_fifo_depth = ((param >> 8) & 0xff) + 1;
	dev->tx_fifo_depth = 16;
	dev->rx_fifo_depth = 16;
	dev->id = pdev->id;

	imap_i2c_reg_init(dev);

	writel(0, dev->base + IIC_INTR_MASK); /* disable IRQ */
	ret = request_irq(dev->irq, imap_i2c_isr, IRQF_DISABLED, pdev->name, dev);
	if(ret)
	{
		printk(KERN_ERR "failure requesting irq %i!\n", dev->irq);
		goto iounmap;
	}
    ret = irq_set_affinity(dev->irq, mask);
    if(ret){
	    printk(KERN_ERR "irq:%d set affinity cpu1 error \n", dev->irq);
	    goto iounmap;
    }

	adap = &dev->adapter;
	i2c_set_adapdata(adap, dev);
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	strlcpy(adap->name, "Imapx800 I2C adapter",sizeof(adap->name));
	adap->algo = &imap_i2c_algo;
	adap->dev.parent = &pdev->dev;

	adap->nr = pdev->id;
	ret = i2c_add_numbered_adapter(adap);
	if (ret)
	{
		printk(KERN_ERR "failure adding adapter!\n");
		goto free_irq;
	}

	return 0;

free_irq:
	free_irq(dev->irq, dev);

iounmap:
	iounmap(dev->base);

unuse_clock:
	clk_disable(dev->clk);
	clk_put(dev->clk);
	dev->clk = NULL;

free_mem:
	platform_set_drvdata(pdev, NULL);
	put_device(&pdev->dev);
	kfree(dev);

release_region:
	release_mem_region(mem->start, resource_size(mem));

	return ret;
}

static int __devexit imap_i2c_remove(struct platform_device *pdev)
{
	struct imap_i2c_dev *dev = platform_get_drvdata(pdev);
	struct resource *mem;

	platform_set_drvdata(pdev, NULL);
	i2c_del_adapter(&dev->adapter);
	put_device(&pdev->dev);

	clk_disable(dev->clk);
	clk_put(dev->clk);
	dev->clk = NULL;

	writel(0, dev->base + IIC_ENABLE);
	free_irq(dev->irq, dev);
	kfree(dev);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(mem->start, resource_size(mem));

	return 0;
}

static int imap_i2c_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct imap_i2c_dev *dev = platform_get_drvdata(pdev);

	dev->suspended = 1;

	return 0;
}

static int imap_i2c_resume(struct platform_device *pdev)
{
    int ret;
	struct imap_i2c_dev *dev = platform_get_drvdata(pdev);
    cpumask_var_t mask;
    mask->bits[0] = 0x2;
	//uint32_t param;
        ret = irq_set_affinity(dev->irq, mask);
        if(ret){
            printk(KERN_ERR "irq:%d set affinity cpu1 error \n", dev->irq);
            return -1;
        }

	if(pdev->id == 0)
		module_power_on(SYSMGR_IIC_BASE);
	if(imap_i2c_module_init(pdev->id) == -1)
	{
		printk("i2c-%d resume failed\n", pdev->id);
		return -1;
	}

	dev->tx_fifo_depth = 16;
	dev->rx_fifo_depth = 16;
	imap_i2c_reg_init(dev);

	dev->suspended = 0;

	return 0;
}

MODULE_ALIAS("platform:imap_i2c");

static struct platform_driver imap_i2c_driver = {
	.probe		= imap_i2c_probe,
	.remove		= __devexit_p(imap_i2c_remove),
	.suspend	= imap_i2c_suspend,
	.resume		= imap_i2c_resume,
	.driver         = {
		.name	= "imap-iic",
		.owner  = THIS_MODULE,
	},
};

static int __init imap_i2c_init(void)
{
//	printk(KERN_INFO "imap iic driver init\n");
	module_power_on(SYSMGR_IIC_BASE);
	return platform_driver_register(&imap_i2c_driver);
}
//module_init(imap_i2c_init);
arch_initcall(imap_i2c_init);

static void __exit imap_i2c_exit(void)
{
	platform_driver_unregister(&imap_i2c_driver);
}
module_exit(imap_i2c_exit);

MODULE_AUTHOR("Baruch Siach <baruch@tkos.co.il>");
MODULE_DESCRIPTION("IMAPX800 I2C bus adapter");
MODULE_LICENSE("GPL");
