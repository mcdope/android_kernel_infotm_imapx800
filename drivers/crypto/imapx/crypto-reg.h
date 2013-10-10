#ifndef __CRYPTO_REG_H__
#define __CRYPTO_REG_H__

#include "imapx-crypto.h"

/*
 * DMA
 * 
 * there four kinds of DMA,   BRDMA : BLCK receive DMA     BTDMA : BLCK transmission DMA
 *                            HRDMA : Hsah receive DMA     PKADMA: PKA Bi-Directional DMA
 * 
 * x depends on the DMA choosen
 */  
#define	SEC_DMA_START(x)		    (x * 0x100 + 0x000)	/* DMA */
#define	SEC_DMA_ADMA_CFG(x)	            (x * 0x100 + 0x004)	/* DMA ADMA config register */
#define	SEC_DMA_ADMA_ADDR(x)	            (x * 0x100 + 0x008)	/* DMA ADMA address register */
#define	SEC_DMA_SDMA_CTRL0(x)	            (x * 0x100 + 0x00c)	/* DMA SDMA control register0 */
#define	SEC_DMA_SDMA_CTRL1(x)	            (x * 0x100 + 0x010)	/* DMA SDMA control register1 */
#define	SEC_DMA_SDMA_ADDR0(x)	            (x * 0x100 + 0x014)	/* DMA SDMA address register0 */
#define	SEC_DMA_SDMA_ADDR1(x)	            (x * 0x100 + 0x018)	/* DMA SDMA address register1 */
#define	SEC_DMA_INT_ACK(x)  	            (x * 0x100 + 0x01c)	/* DMA interrupt ack register */
#define	SEC_DMA_STATE(x)		    (x * 0x100 + 0x020)	/* DMA state register */
#define	SEC_DMA_STATUS(x)		    (x * 0x100 + 0x024)	/* DMA status register */
#define	SEC_DMA_INT_MASK(x)	            (x * 0x100 + 0x028)	/* DMA interrupt mask register */
#define	SEC_DMA_INT_STATUS(x)	            (x * 0x100 + 0x02c)	/* DMA interrupt status register */
                                
/* Block cipher */
#define	SEC_BLKC_CONTROL		    (0x400)	        /* blcok cipher control R */
#define	SEC_BLKC_FIFO_MODE_EN	            (0x404)	        /* enable BLCK FIFO mode */
#define	SEC_BLKC_SWAP   		    (0x408)	        /* BLKC SWAP R */
#define	SEC_BLKC_STATUS		            (0x40c)	        /* BLKC Status R */
#define	SEC_AES_CNTDATA(x)		    (0x410 + x * 4)	/* AES counter data register */
#define	SEC_AES_CNTDATA1		    (0x410)	        /* AES counter data register 1 */
#define SEC_AES_CNTDATA2		    (0x414)	        /* AES counter data register 2 */
#define	SEC_AES_CNTDATA3		    (0x418)	        /* AES counter data register 3 */
#define	SEC_AES_CNTDATA4		    (0x41c)	        /* AES counter data register 4 */
#define SEC_AES_KEYDATA(x)                  (0x420 + x * 4)	/* AES key data register */
#define	SEC_AES_KEYDATA1		    (0x420)	        /* AES key data register 1 */
#define	SEC_AES_KEYDATA2		    (0x424)	        /* AES key data register 2 */
#define	SEC_AES_KEYDATA3		    (0x428)	        /* AES key data register 3 */
#define	SEC_AES_KEYDATA4		    (0x42c)	        /* AES key data register 4 */
#define	SEC_AES_KEYDATA5		    (0x430)	        /* AES key data register 5 */
#define	SEC_AES_KEYDATA6		    (0x434)	        /* AES key data register 6 */
#define	SEC_AES_KEYDATA7		    (0x438)	        /* AES key data register 7 */
#define	SEC_AES_KEYDATA8	     	    (0x43c)	        /* AES key data register 8 */
#define	SEC_AES_IVDATA(x)		    (0x440 + x * 4)	/* AES initial vector data register */
#define	SEC_AES_IVDATA1		            (0x440)	        /* AES initial vector data register 1 */
#define	SEC_AES_IVDATA2		            (0x444)	        /* AES initial vector data register 2 */
#define	SEC_AES_IVDATA3		            (0x448)	        /* AES initial vector data register 3 */
#define	SEC_AES_IVDATA4		            (0x44c)	        /* AES initial vector data register 4 */
#define	SEC_AES_INDATA(x)		    (0x450 + x * 4)	/* AES input data register 1 */
#define	SEC_AES_INDATA1		            (0x450)	        /* AES input data register 1 */
#define	SEC_AES_INDATA2		            (0x454)	        /* AES input data register 2 */
#define	SEC_AES_INDATA3		            (0x458)	        /* AES input data register 3 */
#define	SEC_AES_INDATA4		            (0x45c)	        /* AES input data register 4 */
#define	SEC_AES_OUTDATA(x)		    (0x460 + x * 4)	/* AES output data register */
#define	SEC_AES_OUTDATA1		    (0x460)	        /* AES output data register 1 */
#define	SEC_AES_OUTDATA2		    (0x464)	        /* AES output data register 2 */
#define	SEC_AES_OUTDATA3		    (0x468)	        /* AES output data register 3 */
#define	SEC_AES_OUTDATA4		    (0x46c)	        /* AES output data register 4 */
#define SEC_TDES_CNTDATA(x)	 	    (0x470 + x * 4)	/* triple DES counter data register */
#define	SEC_TDES_CNTDATA1	   	    (0x470)	        /* triple DES counter data register 1 */
#define	SEC_TDES_CNTDATA2		    (0x474)	        /* triple DES counter data register 2 */
#define SEC_TDES_KEYDATA(x, y)              (0x478 + x * 8 + y * 4)	/* triple DES key  data register */
#define	SEC_TDES_KEYDATA11		    (0x478)	        /* triple DES key 1 data register 1 */
#define	SEC_TDES_KEYDATA12	  	    (0x47c)	        /* triple DES key 1 data register 2 */
#define	SEC_TDES_KEYDATA21		    (0x480)	        /* triple DES key 2 data register 1 */
#define	SEC_TDES_KEYDATA22		    (0x484)             /* triple DES key 2 data register 2 */
#define	SEC_TDES_KEYDATA31		    (0x488)	        /* triple DES key 3 data register 1 */
#define	SEC_TDES_KEYDATA32		    (0x48c)	        /* triple DES key 3 data register 2 */
#define SEC_TDES_IVDATA(x)		    (0x490 + x * 4)	/* triple DES initial vector data register */
#define	SEC_TDES_IVDATA1		    (0x490)	        /* triple DES initial vector data register 1 */
#define	SEC_TDES_IVDATA2		    (0x494)	        /* triple DES initial vector data register 2 */
#define SEC_TDES_INDATA(x)	            (0x498 + x * 4)	/* triple DES input data register */
#define	SEC_TDES_INDATA1	    	    (0x498)	        /* triple DES input data register 1 */
#define	SEC_TDES_INDATA2		    (0x49c)	        /* triple DES input data register 2 */
#define	SEC_TDES_OUTDATA(x)		    (0x4a0 + x * 4)	/* triple DES output data register */
#define	SEC_TDES_OUTDATA1		    (0x4a0)	        /* triple DES output data register 1 */
#define	SEC_TDES_OUTDATA2		    (0x4a4)	        /* triple DES output data register 2 */
#define	SEC_RC4_KEYDATA(x)		    (0x4a8 + x * 4)	/* RC4 key data register */
#define	SEC_RC4_KEYDATA1		    (0x4a8)	        /* RC4 key data register 1 */
#define	SEC_RC4_KEYDATA2		    (0x4ac)	        /* RC4 key data register 2 */
#define	SEC_RC4_KEYDATA3		    (0x4b0)	        /* RC4 key data register 3 */
#define	SEC_RC4_KEYDATA4		    (0x4b4)	        /* RC4 key data register 4 */
#define	SEC_RC4_KEYDATA5		    (0x4b8)	        /* RC4 key data register 5 */
#define	SEC_RC4_KEYDATA6		    (0x4bc)	        /* RC4 key data register 6 */
#define	SEC_RC4_KEYDATA7		    (0x4c0)	        /* RC4 key data register 7 */
#define	SEC_RC4_KEYDATA8		    (0x4c4)	        /* RC4 key data register 8 */
#define	SEC_RC4_INDATA 		            (0x4c8)	        /* RC4 input data register */
#define	SEC_RC4_OUTDATA		            (0x4cc)	        /* RC4 output data register */
#define	SEC_BLCK_INT_MASK	  	    (0x4d0)	        /* BLCK interrupt mask register */
#define	SEC_BLCK_INT_STATUS  	            (0x4d4)	        /* BLCK interrupt status register */
                                
/* aes custom sbox */
#define	SEC_AES_CUSTOM_SBOX(x)	            (0x500 + x * 4)	/* AES custom SBOX  0x500 ~ 0x5fc */
                                
/* des custom sbox */
#define	SEC_DES_CUSTOM_SBOX(x)	            (0x600 + x * 4)	/* DES custom SBOX  0x600 ~ 0x6fc */
                                
/* rc4 custom sbox */
#define	SEC_RC4_CUSTOM_SBOX(x)	            (0x700 + x * 4)	/* RC4 custom SBOX  0x700 ~ 0x7fc */
                                
/* clk */                          
#define SEC_CG_CFG                          (0x2000)	        /* blck, hash, pka cke */

/******************************************************************************************************/

/*
 * block cipher
 */
#define BLKC_ENABLE                  0      /* 1:enable   0:disable */
#define BLKC_CHAIN_MODE              1      /* block cipher chain mode  [5:1]  1:ECB 2:CBC 3:CFB 4:OFB 5:CTR */
#define BLKC_MODE                    6      /* 1: encryption   0: decryption */
#define BLKC_ENGINE_SEL              7      /* [8:7] 00:AES  01:DES  10:TDES  11:RC4 */
#define BLKC_STREAM_MODE             9      /* [12:9] 0000: 8bit  0001: 16bit  .. */
#define AES_KEYSIZE                  13     /* [14:13] 00: 128   01: 192   10: 256 */
#define TDES_MODE                    15     /* [17:15] 15bit: DES1  16bit: DES2  17bit: DES3 */
#define USE_CUSTOM_SBOX              18     /* 0: default   1: custom */
#define USE_DEVICEID                 19     /* 0: custom key 1: device ID */
#define RC4_KEYSIZE                  21     /* [24:20] byte number: 00000: 1  00001: 2 ... */
#define RC4_USE_SW_KSA               26     
#define RC4_SW_KSA_DONE              29     /* aftern sw set the sw , it should be written to 1 */
#define RC4_RST                      31     /* when you set sw and re-init, you need to reset the rc4 */

/* blkc status */
#define AES_CNTDATA_SETUP            0      /* setup done */
#define AES_CNTDATA_ERROR            1       
#define AES_KEYDATA_SETUP            2      
#define AES_KEYDONE                  3      /* key expansion */
#define AES_IVDATA_SETUP             4     
#define AES_IVDATA_ERROR             5      
#define AES_BUSY                     6     
#define AES_IN_READY                 7     
#define AES_OUT_READY                8     

extern void security_cke_control(void __iomem *base, uint32_t module_cke, uint32_t mode);
extern uint32_t aes_set_data(void __iomem *base, uint32_t *data);
extern uint32_t aes_get_result(void __iomem *base, uint32_t *data);
extern uint32_t aes_init(void __iomem *base, uint32_t type, struct aes_struct *aes);

#endif /* crypto-reg.h */

