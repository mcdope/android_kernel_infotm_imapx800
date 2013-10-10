#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/time.h>
#include <linux/cache.h>
#include <mach/items.h>
#include <linux/spinlock.h>
#include <mach/imap-iomap.h>
#include <mach/power-gate.h>
#include <mach/pad.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include "imapx-crypto.h"
#include "crypto-reg.h"

#define CRYPTO_DEBUG 1
#ifdef CRYPTO_DEBUG
#define crypto_dbg(x...) printk(x)
#else
#define crypto_dbg(x...)
#endif

static struct imapx_crypto *i_crypto;
static spinlock_t crypto_lock = SPIN_LOCK_UNLOCKED;

static uint32_t crypto_aes_init(void __iomem *base, uint32_t type)
{
    struct aes_struct *aes = (struct aes_struct *)&i_crypto->aes;

    module_reset(SYSMGR_CRYPTO_BASE, 0x3);
    asm("dsb;");
    security_cke_control(base, SEC_BLCK_CKE, ENABLE);
    aes->keysize = AES_KEY_128;
    aes->keytype = SECURITY_KEY;
    if(aes_init(base, type, aes))
        return 1;

    return 0;
}

static void print_out_data(uint32_t *data, uint32_t len)
{
    int i;

    for(i = 0; i < len; i++){
        crypto_dbg("%8.8x", data[i]);
    }
    crypto_dbg("\n");
}

static uint32_t aes_encryption_init(struct crypto_access_desc *desc)
{
    uint32_t ret;
    uint32_t i;
    uint32_t n = (desc->len) >> 4;
    void __iomem *base = i_crypto->io_base;
    uint32_t *indata = i_crypto->indata;
    uint32_t *outdata = i_crypto->outdata;
  
//    print_out_data(indata, n * 4);
    spin_lock(&crypto_lock);
    ret = crypto_aes_init(base, ENCRYPTION);	
    if(!ret){
       for(i = 0; i < n; i++){
           aes_set_data(base, &indata[i * 4]);
//	   asm("dsb;");
           aes_get_result(base, &outdata[i * 4]);
       }
    }
    spin_unlock(&crypto_lock);
 //   print_out_data(outdata, n * 4);

    return ret;
}

static uint32_t aes_decryption_init(struct crypto_access_desc *desc)
{
    uint32_t ret;
    uint32_t i;
    uint32_t n = (desc->len) >> 4;
    void __iomem *base = i_crypto->io_base;
    uint32_t *indata = i_crypto->indata;
    uint32_t *outdata = i_crypto->outdata;
  
   // print_out_data(indata, n * 4);
    spin_lock(&crypto_lock);
    ret = crypto_aes_init(base, DECRYPTION);	
    if(!ret){
       for(i = 0; i < n; i++){
           aes_set_data(base, &indata[i * 4]);
//	   asm("dsb;");
           aes_get_result(base, &outdata[i * 4]);
       }
    }
    spin_unlock(&crypto_lock);
//    print_out_data(outdata, n * 4);

    return ret;
}

#if 0
uint32_t data_print_out(uint32_t *data, uint32_t len)
{
    uint32_t i;

    for(i = 0; i < len; i++){
        crypto_dbg("%8.8x", data[i]);
    }
    crypto_dbg("\n");
}

void crypto_test(void)
{
    uint32_t indata[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint32_t outdata[4];
    uint32_t data3[4];
	
    crypto_dbg("crypto_test");
    data_print_out(indata, 4);
    aes_encryption_init();
    aes_data_set(indata);
    aes_data_get(outdata);
    data_print_out(outdata, 4); 
    aes_data_set(outdata);
    aes_data_get(data3);
    data_print_out(data3, 4); 
    aes_decryption_init();
    aes_data_set(data3);
    aes_data_get(outdata);
    data_print_out(outdata, 4);  
    aes_data_set(outdata);
    aes_data_get(indata);
    data_print_out(indata, 4);  
}
#endif

static int crypto_open(struct inode *inode, struct file *file)
{
    uint32_t flag;
    struct imapx_crypto *crypto = i_crypto;
   
    flag = crypto->flag;

//    crypto_dbg("crypto open \n");
    if(flag > CRYPTO_CLOSE){
	printk(KERN_ERR "%s: crypto node is already opened.\n",
	       __func__);
		return -EINVAL; 
    }
    if(file){
        clk_enable(crypto->clk_dma);
        clk_enable(crypto->clk);
        asm("dsb;");
        module_power_on(SYSMGR_CRYPTO_BASE);
        module_reset(SYSMGR_CRYPTO_BASE, 0x3);
        crypto->flag = CRYPTO_OPEN;
        file->private_data = crypto;

    }
    
    return 0;
}

static int crypto_release(struct inode *inode, struct file *file)
{
    uint32_t flag;
    struct imapx_crypto *crypto = (struct crypto_struct *)file->private_data;
    
    flag = crypto->flag;

  //  crypto_dbg("crypto release \n");
    if(flag > CRYPTO_CLOSE){
        clk_disable(crypto->clk_dma);
        clk_disable(crypto->clk);
        asm("dsb;");
        module_power_down(SYSMGR_CRYPTO_BASE);
    }
    file->private_data = NULL;
    crypto->flag = CRYPTO_CLOSE;
    
    return 0;
}

static int crypto_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    uint32_t flag;
    uint32_t len;
    struct crypto_access_desc desc;
    struct imapx_crypto *crypto = (struct crypto_struct *)file->private_data;
    
    flag = crypto->flag;

   // crypto_dbg("crypto ioctl  %x \n", cmd);
    if(_IOC_TYPE(cmd) != CRYPTO_IOCMD_MAGIC || _IOC_NR(cmd) > CRYPTO_IOCMD_MAX_NUM)
        return -ENOTTY;
    switch(cmd){
        case CRYPTO_IOCMD_AES_ENCPT:
	    if(flag != CRYPTO_OPEN)
		    return 1;
	    if(copy_from_user(&desc, (struct crypto_access_desc *)arg, 
	                      sizeof(struct crypto_access_desc)))
		    return -EFAULT;
            len = desc.len;
	    if((len & 0xf) || (len > CRYPTO_MAX_BUF)){
		    printk(KERN_ERR "len error. \n");
		    return -EFAULT;
	    }
            if(copy_from_user(i_crypto->indata, desc.buf, len)){
		    printk(KERN_ERR "data copy error. \n");
		    return -EFAULT;
	    }
	    if(aes_encryption_init(&desc)){
		    printk(KERN_ERR "aes encryption error. \n");
		    return -EFAULT; 
	    }
	    crypto->flag = CRYPTO_DONE;
	    return 0;
        case CRYPTO_IOCMD_AES_DECPT:
	    if(flag != CRYPTO_OPEN)
		    return 1;
	    if(copy_from_user(&desc, (struct crypto_access_desc *)arg, 
	                      sizeof(struct crypto_access_desc)))
		    return -EFAULT;
            len = desc.len;
	    if((len & 0xf) || (len > CRYPTO_MAX_BUF)){
		    printk(KERN_ERR "len error. \n");
		    return -EFAULT;
	    }
            if(copy_from_user(i_crypto->indata, desc.buf, len)){
		    printk(KERN_ERR "data copy error. \n");
		    return -EFAULT;
	    }
	    if(aes_decryption_init(&desc)){
		    printk(KERN_ERR "aes decryption error. \n");
		    return -EFAULT; 
	    }
	    crypto->flag = CRYPTO_DONE;
	    return 0;
	case CRYPTO_IOCMD_AES_RESULT:
	    if(flag != CRYPTO_DONE)
		    return 0;
	    if(copy_from_user(&desc, (struct crypto_access_desc *)arg, 
	                      sizeof(struct crypto_access_desc)))
		    return -EFAULT;
            len = desc.len;
	    if((len & 0xf) || (len > CRYPTO_MAX_BUF)){
		    printk(KERN_ERR "len error. \n");
		    return -EFAULT;
	    }
            if(copy_to_user(desc.buf, i_crypto->outdata, len)){
		    printk(KERN_ERR "data copy error. \n");
		    return -EFAULT;
	    }
	    crypto->flag = CRYPTO_OPEN;
	    return 0; 
	default:
	    return -EFAULT;
    }

    return 0;	
}


static struct file_operations crypto_ops = 
{
    owner: THIS_MODULE,
    open: crypto_open,
    release: crypto_release,
    unlocked_ioctl: crypto_ioctl,
};

static struct miscdevice crypto_miscdev = 
{
    minor: MISC_DYNAMIC_MINOR,
    name: CRYPTO_DEV_NAME,
    fops: &crypto_ops
};

/*
 * crypto device probe
 */
static int imapx_crypto_probe(struct platform_device *pdev)
{
    int err = 0;
    struct resource *res;
    struct imapx_crypto *crypto;

    printk("imapx800 crypto init.  \n");

    crypto = kzalloc(sizeof(struct imapx_crypto), GFP_KERNEL);
    if(crypto == NULL){
        dev_err(&pdev->dev, "crypto unable to alloc data struct\n");
	return -ENOMEM;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res){
        dev_err(&pdev->dev, "crypto no MEM resouce\n");
	err = -ENODEV;
	goto err_alloc;
    }
    if(!request_mem_region(res->start, resource_size(res), pdev->name)){
	dev_err(&pdev->dev, "res request mem region err\n");
    	err = -EBUSY;
	goto err_alloc;
    }

    crypto->io_base = ioremap(res->start, resource_size(res));
    if(!crypto->io_base){
	dev_err(&pdev->dev, "crypto ioremap error\n");
    	err = -ENOMEM;
	goto err_alloc;
    }

    crypto->clk = clk_get(&pdev->dev, "crypto");
    if(!crypto->clk){
        dev_err(&pdev->dev, "crypto no clk info\n");
    	err = -ENODEV; 
	goto err_iomap;
    }
//    clk_enable(crypto->clk);

    crypto->clk_dma = clk_get(&pdev->dev, "crypto-dma");
    if(!crypto->clk){
        dev_err(&pdev->dev, "crypto no clk info\n");
    	err = -ENODEV; 
	goto err_iomap;
    }
   // clk_enable(crypto->clk_dma);
//    val1 = clk_set_rate(crypto->clk, 192000);
//    val2 = clk_set_rate(crypto->clk_dma, 192000);
    crypto->clk_rate = clk_get_rate(crypto->clk);
    crypto->clk_dma_rate = clk_get_rate(crypto->clk_dma);
    dev_set_drvdata(&pdev->dev, crypto);
    crypto->flag = CRYPTO_CLOSE;
    i_crypto = crypto;
    asm("dsb;");
    misc_register(&crypto_miscdev);

    printk("imapx800 crypto init done.  \n");
   // crypto_test();

    return 0;

err_iomap:
    iounmap(crypto->io_base);
err_alloc:
    kfree(crypto);

    return err;
}

static int imapx_crypto_remove(struct platform_device *pdev)
{
    struct imapx_crypto *crypto =  dev_get_drvdata(&pdev->dev);
    struct resource *res;

    module_power_down(SYSMGR_CRYPTO_BASE);
    clk_disable(crypto->clk);
	clk_put(crypto->clk);

    iounmap(crypto->io_base);
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    release_mem_region(res->start, resource_size(res));
    kfree(crypto);

    return 0;
}

static int imapx_crypto_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

static void imapx_crypto_resume(struct platform_device *pdev)
{

}

static struct platform_driver imapx_crypto_driver = {
	.probe     =  imapx_crypto_probe,
	.remove    =  imapx_crypto_remove,
        .suspend   =  imapx_crypto_suspend,
        .resume    =  imapx_crypto_resume,
	.driver    = {
		.name  = "imap-crypto",
		.owner = THIS_MODULE,
	},
};

static int __init imapx_crypto_module_init(void)
{
    if (item_equal("board.cpu", "i15", 0))
        return -1;
    return platform_driver_register(&imapx_crypto_driver);
}


static void __exit imapx_crypto_module_exit(void)
{
    platform_driver_unregister(&imapx_crypto_driver);
}

module_init(imapx_crypto_module_init);
module_exit(imapx_crypto_module_exit);

MODULE_AUTHOR("Larry Liu");
MODULE_DESCRIPTION("imapx crypto");
MODULE_LICENSE("GPL");
