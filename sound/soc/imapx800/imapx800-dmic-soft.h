#ifndef __IMAPX800_DMIC_SOFT_H__
#define __IMAPX800_DMIC_SOFT_H__

#include <linux/kernel.h>

/* 
 * cic_info
 *
 * it`s used in function cic_filter and the code depends on the
 * struct ptr,sig,old,   so if want to add parameters please add after
 * old[5]
 */
struct cic_info{
    const uint8_t *ptr;      /* point to tatble */
	uint32_t sig[5];
	uint32_t old[5];
};

#define NUM_COEFFS          21
#define FIR_BUF_SIZE        (2 * NUM_COEFFS)
#define FIR_DECIMATION      2

/*
 * fir 16
 *
 * it`s used in function fir_filter and the code depends on the 
 * *h and buf, so if want to add parameters please add after buf
 */
struct fir_16{
    const short *h;          /* filter coefficients */
	short buf[FIR_BUF_SIZE];
}__attribute__((aligned(4)));

struct dmic_soft_info{
	void *comp;              /* comp, halfband1, and halfband2 are bufs for compute*/
	void *halfband1;
	void *halfband2;
    struct cic_info cic;     /* cic info */
	struct fir_16 fir[3];    /* fir1, fir2, fir3 */
};

extern void dmic_soft_init(struct dmic_soft_info *dmic);
/* pdm, pcm, comp, halfband1, and halfband2 should be 32 aligned 
 * cacheline_aligned is recommended
 */
extern uint32_t pdm_to_pcm_sw(uint32_t *pdm, short *pcm, void *comp, void *halfband1,
			void *halfband2, uint32_t size, struct dmic_soft_info *dmic);

#endif /* imapx800-dmic-soft.h */
