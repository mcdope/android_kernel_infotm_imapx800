#ifndef __IMAPX_CRYPTO_H__
#define __IMAPX_CRYPTO_H__

#include <linux/io.h>

struct imapx_crypto;

struct aes_struct{
    uint32_t keysize;    /* AES_KEY_128, AES_KEY_192, AES_KEY_256*/
    uint32_t keytype;    /* CUSTOM_KEY, DEVICEID, SECURITY_KEY */ 
};

struct imapx_crypto{
    struct device     *dev;
    void __iomem      *io_base;
    struct clk        *clk;
    struct clk        *clk_dma;
    int               clk_rate;
    int               clk_dma_rate;
    resource_size_t   rsrc_start;
    resource_size_t   rsrc_len;
    uint32_t          flag;
    uint32_t          indata[32];
    uint32_t          outdata[32];
    struct aes_struct aes;
};

#define CRYPTO_MAX_BUF                  128

#define CRYPTO_DEV_NAME                 "crypto-hw"
#define CRYPTO_IOCMD_MAGIC 		'C'
#define CRYPTO_IOCMD_AES_ENCPT		_IOWR(CRYPTO_IOCMD_MAGIC, 1, int)
#define CRYPTO_IOCMD_AES_DECPT		_IOWR(CRYPTO_IOCMD_MAGIC, 2, int)
#define CRYPTO_IOCMD_AES_RESULT		_IOWR(CRYPTO_IOCMD_MAGIC, 3, int)
#define CRYPTO_IOCMD_MAX_NUM	11

struct crypto_access_desc {
    uint32_t len;    /* len is the number of buf, should be int multiples of 16 bytes */
                     /* and max is 128bytes */
    char *buf;  
};

/* flag */
#define CRYPTO_CLOSE                 1
#define CRYPTO_OPEN                  2
#define CRYPTO_ENCPT                 3
#define CRYPTO_DECPT                 4
#define CRYPTO_DONE                  5 


#define ENABLE                       1
#define DISABLE                      0

#define ENCRYPTION                   1
#define DECRYPTION                   0

#define ECB                          0x1
#define CBC                          0x2
#define CFB                          0x4
#define OFB                          0x8
#define CTR                          0x10

#define AES_KEY_128                  0
#define AES_KEY_192                  1
#define AES_KEY_256                  2

#define CUSTOM_KEY                   0
#define DEVICEID                     1
#define SECURITY_KEY                 3

/* security clk */
#define SEC_BLCK_CKE                 0
#define SEC_HASH_CKE                 1
#define SEC_PKA_CKE                  2

#endif /* imapx-crypto.h */

