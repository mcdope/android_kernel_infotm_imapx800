#ifndef STANDALONE
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/byteorder.h>
#else
#include <stdio.h>
#include <stdint.h>
#include "bch.h"
struct mtd_info;
#define EXPORT_SYMBOL(x)  /* x */

#define MODULE_LICENSE(x)	/* x */
#define MODULE_AUTHOR(x)	/* x */
#define MODULE_DESCRIPTION(x)	/* x */

#define printk printf
#define KERN_ERR		""
#endif

#define M	8
#define T   5
#define N   255
#define K	215
#define N_ZERO 23
#define D_LEN 192
#define MX_BN 192
#define R_LEN 40
#define MX_WN 6
#define RX_WN 2
#define GX_WN 2
#define RX_IDX 24

#define get_gf_ai(x) ai_gf[x]

// decoder
void cal_si(void);
void cal_epoly(void);
int  cal_eloc(void);
void fix_error(void);
void cal_gf_2m14(void);
void bch_dec(void);

// encoder
uint16_t bch_enc(void);
void cal_t_gx(void);
void update_rx (uint16_t kn);

// globals
static uint32_t *mx; //--1KB data bits, 32-bit word
static uint32_t *rx; //--336 data bits, 32-bit word
static uint32_t t_gx[GX_WN];
static uint16_t gf_2m14[N+1];  //--Gallios Field elements, a^13 leftmost.
static uint16_t ai_gf[N+1];

static uint32_t rxb[RX_WN];
static uint16_t si[2*T];     //--S1,S2, ..., S48
static uint16_t epoly[T+1];  //--error poly.: 1 + ep[0]*X + ep[1]*X^2 + ... + ep[T-1]**X^T
static uint16_t epoly_lu=0;  //--minimal power of error poly.

static uint16_t has_error=0;
static short nerrs=0;
static uint32_t rx_last;
static uint16_t err_loc[T];

static uint32_t gx[GX_WN] = {
             0x99BEE9D6, 0x88800000
};

// debug
#if 0
void __nand_dump(const uint8_t *dat, int len)
{
	char str[64];
	int i;

	str[0] = 0;
	for(i = 0; i < len; i++)
	{
		if(!(i & 0xf))
		  printk(KERN_ERR "%s\n", str);
		sprintf(str + 3 * (i & 0xf), "%02x ", dat[i]);
	}

	if(!(i & 0xf))
	  printk(KERN_ERR "%s\n", str);
}
#endif


// decoder

//---------------------------------------------------------------------------
//void cal_gf_2m14(void)
void bch_init(void)
{
    uint16_t b[M], newb[M];
	uint16_t i, k, j;

    gf_2m14[0] = 1;   //a^0
	ai_gf[1] = 0;

    b[0]=0x1;
    for(i=1; i<M;i++)
        b[i]=0x0;

    for(k=1; k<N; k++){
        newb[0]=b[7];
        newb[1]=b[0];
        newb[2]=b[1] ^ b[7];
        newb[3]=b[2] ^ b[7];
        newb[4]=b[3] ^ b[7];
        newb[5]=b[4];
        newb[6]=b[5];
        newb[7]=b[6];

        gf_2m14[k]=0;
        for(j=0; j<M; j++) {
            int b_val = 0x0;
            if(newb[j]) b_val=0x1;
            gf_2m14[k] = gf_2m14[k] | (b_val << j);
        }

		ai_gf[gf_2m14[k]] = k;

        for(j=0; j<M; j++)
            b[j]=newb[j];
    }


    gf_2m14[N] = 1;   //a^16383=1=a^0
}
//---------------------------------------------------------------------------
void cal_si(void)
{
    uint32_t data;
    uint16_t bit_idx, word_idx, degree, si_idx, gf, index;

    bit_idx=RX_IDX, degree=0;

    for(si_idx=0; si_idx<2*T; si_idx++)
        si[si_idx] = 0;

    for(word_idx=0; word_idx<MX_WN+RX_WN; word_idx++){
        if(word_idx<RX_WN)
            data = rx[RX_WN-1-word_idx];
        else
            data = mx[MX_WN+RX_WN-1-word_idx];
        data = data >> bit_idx;

        for(; bit_idx<32; bit_idx++) {
            if(data & 0x1) {
                for(si_idx=0; si_idx<T; si_idx++) {
                    index = ((si_idx*2+1) * degree) % N;
                    gf = gf_2m14[index];

                    si[si_idx*2] = si[si_idx*2] ^ gf;
                }
            }
            data = data >> 1;
            degree++;
        }

        bit_idx = 0;

        if(word_idx==RX_WN-1)
            degree=degree+N_ZERO;  //--fill zero
    }

    for(si_idx=1; si_idx<=T; si_idx++) {
        if(si[si_idx-1]==0)
            si[si_idx*2-1]=0;
        else{
            index = get_gf_ai(si[si_idx-1]);
            index = (index * 2) % N;
            si[si_idx*2-1] = gf_2m14[index];
        }
    }

    for(si_idx=0; si_idx<2*T; si_idx++) {
        if(si[si_idx]) {
            has_error=1;
            break;
        }
    }
}
//---------------------------------------------------------------------------
void cal_epoly(void)
{
    uint16_t delta_ep[T+1];
    uint16_t du_div_dp_ai, i, t;
    uint16_t du_p1;
    short x_pow;

    short px2=-1;
    short lp=0;
    uint16_t dp=1;
    uint16_t p_epoly[T+1];

    p_epoly[0]=1;
    for(i=1; i<=T; i++)
        p_epoly[i]=0;

    short u=0;
    short lu=0;
    uint16_t du=si[0];
    uint16_t u_epoly[T+1];

    u_epoly[0]=1;
    for(i=1; i<=T; i++)
        u_epoly[i]=0;

    epoly[0]=1;
    for(i=1; i<=T; i++)
        epoly[i]=0;
    epoly_lu = 0;

    for(t=1; t<=T; t++) {
        //--calculate delta_ep(u+1) = pow(x,2(u-p)) * epoly(p)
        if(du) {
            x_pow=2*u-px2;

            for(i=0; i<x_pow; i++)
                delta_ep[i]=0;
            for(i=x_pow; i<=T; i++)
                delta_ep[i]=p_epoly[i-x_pow];

            //--calculate delta_ep * du/dp
            uint16_t du_ai = get_gf_ai(du);
            uint16_t dp_ai = get_gf_ai(dp);
            uint16_t dp_inv_ai = N-dp_ai;
            du_div_dp_ai = du_ai + dp_inv_ai;

            //multiply delta_ep*du/dp
            for(i=0; i<=T; i++){
                if(delta_ep[i]) {
                    uint16_t delta_ep_ai = get_gf_ai(delta_ep[i]);
                    uint16_t sum_ai = (delta_ep_ai + du_div_dp_ai) % N;
                    delta_ep[i] = gf_2m14[sum_ai];
                }
                else {
                    delta_ep[i] = 0;
                }
            }

            //--calculate epoly = d(u+1)
            for(i=0; i<=T; i++){
                epoly[i] = u_epoly[i] ^ delta_ep[i];
                if(epoly[i]) epoly_lu=i;

                if(epoly[i] > N)
                    printk("warning: epoly[i] should be less that N");
            }
        }

        //--calculate d(u+1)
        du_p1=si[2*u+3-1];
        for(i=1; i<=epoly_lu; i++) {
            uint16_t delta_ai, delta = 0;
            uint16_t c_epoly = epoly[i];
            uint16_t t_si = si[2*u+3-i-1];

            if(c_epoly && t_si) {
                delta_ai=(get_gf_ai(c_epoly) + get_gf_ai(t_si)) % N;
                delta=gf_2m14[delta_ai];
            }
            else
                delta = 0;

            du_p1 = du_p1 ^ delta;
        }

        //--update p's parameters
        short px2_p_lp = px2 - lp;
        short ux2_p_lu = 2*u - lu;

        if((ux2_p_lu > px2_p_lp) && du) {
            px2=u*2;
            lp=lu;
            dp=du;

            for(i=0; i<=T; i++)
                p_epoly[i]=u_epoly[i];
        }

        //--update u's parameters
        u++;
        lu = epoly_lu;
        du = du_p1;
        for(i=0; i<=T; i++)
            u_epoly[i]=epoly[i];
    }
}

//---------------------------------------------------------------------------
int cal_eloc(void)
{
	uint16_t i, j;

    uint16_t ai_pow, sum;
    uint16_t delta_ai;
    uint16_t err_num = 0;

    for(i=1; i<=N; ){
        ai_pow=i;
        sum = epoly[0];

        for(j=1; j<=epoly_lu; j++) {
            uint16_t delta = epoly[j];
            if( delta ) {
                delta_ai = (get_gf_ai(delta) + ai_pow*j) % N;
                delta = gf_2m14[delta_ai];
                sum = sum ^ delta;
            }
        }

        if(sum == 0){
            err_loc[err_num] = N - ai_pow;
            err_num ++;
        }

        i++;
        if(i==(MX_BN+1)) i=i+N_ZERO;
    }

    if((err_num==0) || (err_num>=T) || (epoly_lu != err_num))
        return -1;

    return err_num;
}

//---------------------------------------------------------------------------
void fix_error(void)
{
    uint16_t abs_loc=0;
    uint16_t word_loc, bit_loc, i;

    uint16_t start_idx = N-MX_BN;

    for(i=0; i<nerrs; i++) {
        abs_loc = err_loc[i] % N;

        if(abs_loc >= start_idx ){
            abs_loc = abs_loc - start_idx;
            word_loc = MX_WN - 1 - (abs_loc >> 5);
            bit_loc = abs_loc & 0x1f;
            mx[word_loc] = mx[word_loc] ^ (0x1 << bit_loc);
        }
    }
}

//---------------------------------------------------------------------------
int nand_correct_data_bch(unsigned char *buf,
   unsigned char *read_ecc, unsigned char *calc_ecc)
{
    nerrs = 0;
    has_error = 0;

	// set mx, rx
	mx = (uint32_t *)buf;
	rxb[0] = *(uint32_t *)calc_ecc;
	rxb[1] = (*(calc_ecc + 4)) << 24;
	rx = rxb;

    cal_si();

    if( !has_error ) 
	  return 0;

    cal_epoly();

    nerrs = cal_eloc();
    if(nerrs>0)  fix_error();

	if(nerrs)
	  printk(KERN_ERR "bch: %d bits error found.\n", nerrs);
	return nerrs;
}
//---------------------------------------------------------------------------

// encoder

//----------------------------------------------------------
void update_rx ( uint16_t kn )
{
    uint16_t t_kn = kn + R_LEN + 1;
    uint16_t wn = t_kn >> 5;
    uint16_t bn = 31 - (t_kn & 0x1f);
    uint32_t mask = 0x1 << bn;

    uint32_t msg = 0x0;
    if(wn < MX_WN) msg = mx[wn];

    uint32_t bit_val = msg & mask;

    if(bit_val)
        bit_val = 0x00800000;
    else
        bit_val = 0x0;

    rx[0] = (rx[0] << 1) | (rx[1] >> 31);
    rx[1] = (rx[1] << 1) | bit_val;
}

//----------------------------------------------------------
void cal_t_gx ()
{
    uint32_t msb = rx[0] & 0x80000000;
	uint16_t i;

    for(i=0; i<GX_WN; i++) {
        if(msb) t_gx[i] = gx[i];
        else t_gx[i] = 0x0;
    }
}

//----------------------------------------------------------
int nand_calculate_ecc_bch(const unsigned char *buf,
   unsigned char *code)
{
    uint16_t wn, kn;

	// init mx, rx
	mx = (uint32_t *)buf;
//	rx = (uint32_t *)code;
	rx = rxb;

    //-- initialize rx[GX_WN]
    rx[0] = mx[0];
    rx[1] = mx[1] & 0xff800000;

    for(kn=0; kn<K; kn++) {
        cal_t_gx();

        rx[0] = rx[0] ^ t_gx[0];
        rx[1] = rx[1] ^ t_gx[1];

        update_rx( kn );
    }

	*(uint32_t *)code = rx[0];
	*(code + 4) = (rx[1] >> 24) & 0xff;

    return 0;
}

EXPORT_SYMBOL(nand_correct_data_bch);
EXPORT_SYMBOL(nand_calculate_ecc_bch);
EXPORT_SYMBOL(bch_init);
