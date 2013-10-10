#include "crypto-reg.h"
#include <linux/kernel.h>

#define __crypto_readl(base, off)       __raw_readl(base + off)
#define __crypto_writel(val, base, off) __raw_writel(val, base + off)

//#define CRYPTO_DEBUG 1
#ifdef CRYPTO_DEBUG
#define crypto_dbg(x...) printk(x)
#else
#define crypto_dbg(x...)
#endif

static inline void block_cipher_control(void __iomem *base, uint32_t mode)
{
    uint32_t val;
	
    val = __crypto_readl(base, SEC_BLKC_CONTROL);
    val &= ~(1 << BLKC_ENABLE);
    val |= (mode << BLKC_ENABLE);
    __crypto_writel(val, base, SEC_BLKC_CONTROL);
}

static inline void block_cipher_fifo(void __iomem *base, uint32_t mode)
{
    __crypto_writel(mode, base, SEC_BLKC_FIFO_MODE_EN);
}

static void block_cipher_rst(void __iomem *base)
{
    uint32_t val;
	
    val = __crypto_readl(base, SEC_BLKC_CONTROL);
    val |= 0x80000000;
    __crypto_writel(val, base, SEC_BLKC_CONTROL);
	
    // FIXME TODO
    // make sure the bit is cleared
    // here we do not set time out 
    val = __crypto_readl(base, SEC_BLKC_CONTROL);
    while(val & (1 << 31) != 0){
        val = __crypto_readl(base, SEC_BLKC_CONTROL);
    }
}

static inline void block_cipher_clean(void __iomem *base)
{
    __crypto_writel(0, base, SEC_BLKC_CONTROL);
    block_cipher_rst(base);
}

/* ENCRYPTION or DECRYPTION */
static inline void block_cipher_crypto(void __iomem *base, uint32_t mode)
{
    uint32_t val;
	
    val = __crypto_readl(base, SEC_BLKC_CONTROL);
    val &= ~(1 << BLKC_MODE);
    val |= (mode << BLKC_MODE);
    __crypto_writel(val, base, SEC_BLKC_CONTROL);
}

static void block_cipher_aes_init(void __iomem *base, uint32_t type, struct aes_struct *aes)
{
    uint32_t val;

    val = (ECB << BLKC_CHAIN_MODE) | (type << BLKC_MODE) | ((aes->keysize) << AES_KEYSIZE) |
	  ((aes->keytype) << USE_DEVICEID);
    __crypto_writel(val, base, SEC_BLKC_CONTROL);
}

static inline void block_cipher_status_set(void __iomem *base, uint32_t val)
{
    __crypto_writel(val, base, SEC_BLKC_STATUS);
}

static inline uint32_t block_cipher_status_check(void __iomem *base)
{
    return __crypto_readl(base, SEC_BLKC_STATUS);
}

static inline void block_cipher_aes_indata_set(void __iomem *base, uint32_t *data)
{
    __crypto_writel(data[0], base, SEC_AES_INDATA1);
    __crypto_writel(data[1], base, SEC_AES_INDATA2);
    __crypto_writel(data[2], base, SEC_AES_INDATA3);
    __crypto_writel(data[3], base, SEC_AES_INDATA4);
}

static inline void block_cipher_aes_outdata_get(void __iomem *base, uint32_t *data)
{
    data[0] = __crypto_readl(base, SEC_AES_OUTDATA1);
    data[1] = __crypto_readl(base, SEC_AES_OUTDATA2);
    data[2] = __crypto_readl(base, SEC_AES_OUTDATA3);
    data[3] = __crypto_readl(base, SEC_AES_OUTDATA4);
}

uint32_t aes_set_data(void __iomem *base, uint32_t *data)
{
    uint32_t val = block_cipher_status_check(base);

    if(!(val & (1 << AES_IN_READY)))
        return 1;

    block_cipher_aes_indata_set(base, data);

    return 0;
}

uint32_t aes_get_result(void __iomem *base, uint32_t *data)
{
    uint32_t val = block_cipher_status_check(base);

    if(!(val & (1 << AES_OUT_READY)))
        return 1;
    
    block_cipher_aes_outdata_get(base, data);
    block_cipher_status_set(base, (1 << AES_OUT_READY));

    return 0;
}

/*
 * AES init
 *
 * use to init block cipher for aes 
 */
uint32_t aes_init(void __iomem *base, uint32_t type, struct aes_struct *aes)
{
    block_cipher_control(base, DISABLE);
    crypto_dbg("crypto_aes_init  1 \n");
    block_cipher_clean(base);
    crypto_dbg("crypto_aes_init  2 \n");
    block_cipher_fifo(base, DISABLE);
    crypto_dbg("crypto_aes_init  3 \n");
    block_cipher_aes_init(base, type, aes);
    crypto_dbg("crypto_aes_init  4 \n");
    block_cipher_control(base, ENABLE);

    return 0;
}

/*
 * security cke control
 * 
 * module_cke : SEC_BLCK_CKE, SEC_HASH_CKE, SEC_PKA_CKE
 * mode : ENABLE or DISABLE
 */
void security_cke_control(void __iomem *base, uint32_t module_cke, uint32_t mode)
{
    uint32_t val;
 
    val = __crypto_readl(base, SEC_CG_CFG);
    val &= ~(1 << module_cke);
    val |= (mode << module_cke);
    __crypto_writel(val, base, SEC_CG_CFG);
}

