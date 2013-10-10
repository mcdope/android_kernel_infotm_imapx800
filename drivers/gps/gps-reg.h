#ifndef __GPS_REG_H__
#define __GPS_REG_H__

#include <linux/io.h>
/*
 * bit define 
 */
#define ENABLE            1
#define DISABLE           0

/* ADRAMSTAT */
#define ADRAMSTAT_DRAMSTAT   0    // control GPS data ram manually for test

/* rACRTHRD */
#define ACRTHRD_THRD         25   // 25 ~ 29 for sin thresholde

/* ASATID */
#define ASATID_SATID      0      // satellite id that search engine acquire  0 ~ 5
#define ASATID_CAOFF      6      // offset of ca code initial acquisition    6 ~ 17

/* rADFFTBEXPn or rAIFFTBEXPn */
#define FFT_STAGE1        0      // bit exp 0000
#define FFT_STAGE2        4
#define FFT_STAGE3        8
#define FFT_STAGE4        12
#define FFT_STAGE5        16
#define FFT_STAGE6        20

/* ACUTCFG */
#define ACUTCFG_CUTCFG       0     // cut configuration of complex multiply    0 ~ 3 
#define ACUTCFG_STAGE_SHIFT  4     // offset to stage
#define ACUTCFG_DFFTSCALE    28    // GPS data FFT in circular correlation     0: block floating   1: scale schedule mode

#define BLOCK_FLOATION       0
#define SCALE_SCHEDULE       1

/* rAFFTSCALEn */
#define AFFT_IFFTSCALE       28    // IFFT in circular correlation  0: block floating   1: scale schedule mode

/* ACORMAXOFF */
#define ACORMAXOFF_CORSMAXOFF       0    // C/A code offset according to second max result in Coh/Incoh integrate
#define ACORMAXOFF_CORMAXOFF       12    // C/A code offset according to max result in Coh/Incoh integrate

/* TSATID */
#define TSATID_SATID0        0    // satellite ID 0     0 ~ 5
#define TSATID_SATID1        8    // satellite ID 1     8 ~ 15
#define TSATID_SATID2        16   // satellite ID 2     16 ~ 21

/* TDATASET */
#define TDATA0               0
#define TDATA1               1
#define TDATA2               2

#define TCAOFF               0
#define TCAINC               1
#define TCAPHS               2
#define TCRINC               3
#define TCRPHS               4

/* rTCRTHRD */
#define TCRTHRD_THRD         25   // 25 ~ 29 for sin thresholde

// dma
#define GPS_MEMA             0
#define GPS_MEMB             1
#define GPS_MEMC             2
#define GPS_MEMD             3
#define GPS_MEME             4
#define GPS_MEMF             5
#define GPS_MEMG             6
#define GPS_MEMH             7

#define DEFAULT_FFT_CUT      7

struct FFT_STRUCT{
    uint32_t ifft_f;        /* BLOCK_FLOATION or SCALE_SCHEDULE */
    uint32_t dfft_f;        /* BLOCK_FLOATION or SCALE_SCHEDULE */
    uint32_t cut;           /* cut configuration of complex multiply    0 ~ 3 */
    uint32_t dfft_stage[6]; /* 0000  scale schedule used by GPS data FFT */
    uint32_t ifft_stage[6]; /* 0000  scale schedule used by GPS invert FFT */
};

struct CC_STRUCT{
	uint32_t number;        /* the number of CC , should be 0 ~ 5 */
	uint32_t sat_id;        /* satellite id */
	uint32_t caoff;         /* offset of ca code initial acquisition */
	uint32_t cainc;         /* C/A generator increment */
	uint32_t caphs;         /* C/A generator initial phase */
	uint32_t crinc;         /* carrier generator increment */
	uint32_t crphs;         /* carrier generator initial phase */
//	uint32_t fft_set_flag;  /* ENABLE or DISABLE     whether set fft or not */
//	uint32_t sin_thrd_flag; /* ENABLE or DISABLE */
//	uint32_t sin_thrd;
//	struct FFT_STRUCT fft; 
};

/*
 * ci result struct 
 * 
 * you may only use it when you teke coh/incoh operation
 */
struct CI_RESULT_STRUCT{ 
	uint32_t cor_max;        /* max result */
	uint32_t cor_smax;       /* second max result */
	uint32_t cor_mean;       /* mean result */
	uint32_t cor_maxoff;     /* c/a code off set according to max result      	12bit */
	uint32_t cor_smaxoff;    /* c/a code off according to second max result     12bit */
};

struct INSTR_STRUCT{
	uint32_t number;        /* the numbers to set */
	uint32_t instr[18];     /* max is 18 */
};

/*
 * instruction
 * 
 *     31      |  30 ~ 28   |     27     |     26     |     25     |    24      |
 * -------------------------------------------------------------------------------------
 *  last flag  | instr type | blockflag3 | blockflag2 | blockflag1 | blockflag0 | 
 * 
 * instr cc
 * 
 *      23 ~ 2    |    	1 ~ 0
 * ----------------------------------------------------
 *     reserved   |     cc type (now , only have CC 0)
 * 
 * instr Coh/Incoh
 * 
 *   23 ~ 16  |    15 ~ 12  |   11 ~ 8   |    7 ~ 4    |   3 ~ 0    |
 * ---------------------------------------------------------------------
 *  reserved  |     mode    |  Dst addr  |  Src addr B | Src addr A |
 * 
 * instr Dma
 * 
 *     23    |    22      |    21 ~ 4    |  0 ~ 3  |
 * ---------------------------------------------------
 *  reserved | Direction  |   host addr  |  Src a  |
 */

/* AINSTR */
#define AINSTR_BLOCKFLAG     24    /* 0 ~ 3  */
#define AINSTR_BLOCKFLAG0    24    /* 0: will not be blocked    1: be blocked   (previous Circular Correlation 0) */
#define AINSTR_BLOCKFLAG1    25    /* 0: will not be blocked    1: be blocked   (previous Circular Correlation 1) */
#define AINSTR_BLOCKFLAG2    26    /* 0: will not be blocked    1: be blocked   (previous Coh/Incoh integration instruction) */
#define AINSTR_BLOCKFLAG3    27    /* 0: will not be blocked    1: be blocked   (previous DMA transition instructinon) */
#define AINSTR_INST_TYPE     28    /* current instruction type: 000 CC instr  001 Coh/Incon integration instr  010 DMA transition instr */
#define AINSTR_LASTFLAG      31    /* last instruction indicate flag  0: not last  1: last  and gnerate a interrupt after this instruction */

/* for Circular Correlation mode */
#define AINSTR_CC_TYPE       0     /* circular correlation Type */
/* for Coh/Incoh integration mode */
#define AINSTR_SRCAADD       0     /* source a address in coherent/incoherent integrate  0 ~ 3 */
#define AINSTR_SRCBADD       4     /* source b address in coherent/incoherent integrate  4 ~ 7 */
#define AINSTR_DSTADDR       8     /* Destination address in coherent/incoherent integrate */
#define AINSTR_MODE          12    /* the mode of Coh/Incoh integration */
/* for DMA transition mode */
#define AINSTR_SRCA          0     /* the number of coh/incoh memory */
#define AINSTR_HOSTADDR      4     /* Base Address of sysytem memory, align with the 16KB  4 ~ 21 */
#define AINSTR_DIRECTION     22    /* 0 : system memory -> Coh/Incoh integration memory   1 : Coh/Incoh integration memory -> system memory */

#define NOT_BE_BLOCKED       0
#define BE_BLOCKED           1

#define CC_INSTR             0
#define CI_INSTR             1
#define DMA_INSTR            2

#define PERFORM_CC0          0
#define PERFORM_CC1          1
#define PERFORM_BOTHCC       2    /* 3 is also ok */

#define COM_TO_COM           0
#define COM_TO_REAL          1
#define COM_A_COM_TO_COM     2
#define COM_S_COM_TO_COM     3
#define REAL_A_REAL_TO_REAL  4
#define COM_A_COM_TO_REAL    5
#define COM_A_REAL_TO_REAL   6
#define REAL_M_REAL_TO_REAL  7

#define SYSMEM_TO_CIMEM      0
#define CIMEM_TO_SYSMEM      1

/*
 * operation
 */
#define INIT                 0
#define CLOSE                1
#define START                2
#define END                  3

#define TRACK_UNIT_NUM       12
#define INSTR_MAX_NUM        18   

#define NORMAL_MODE          0     
#define TEST_MODE1           1    /* always operate the data ram 0 */
#define TEST_MODE2           2    /* always operate the data ram 1 */
#define TEST_MODE3           3    /* input GPS data of tracking module will come from data ram in acquisition module */

/*
 * fuction
 */
extern void acq_inter_control(void __iomem *base, uint32_t mode);
extern uint32_t acq_state_check(void __iomem *base);
extern uint32_t acq_cc_init(void __iomem *base, struct CC_STRUCT *cc);

extern void acq_instr_bit_set(uint32_t *instr, uint32_t bit, uint32_t value);
extern uint32_t acq_instr_init(void __iomem *base, struct INSTR_STRUCT *instr);
extern void acq_result_get(void __iomem *base, uint32_t n, struct CI_RESULT_STRUCT *ci);
extern void acq_dram_control(void __iomem *io_base, uint32_t dram_mode);

extern uint32_t acq_operation_control(void __iomem *base, uint32_t oper);

/* --------------------- tracking unit ---------------------------*/

/* TCRSEL02 */
#define TCRSEL_CASEL0        0    /* ACC select C/A generator                                    0 ~ 1  */
#define TCRSEL_CASFTSEL0     2    /* ACC select C/A code that shift from prompt code int chips   2 ~ 6 */
#define TCRSEL_CRSEL0        7    /* ACC select carrier generator                                7 ~ 8 */
#define TCRSEL_CASEL1        9    /* ACC select C/A generator                                    9 ~ 10  */
#define TCRSEL_CASFTSEL1     11   /* ACC select C/A code that shift from prompt code int chips   11 ~ 15 */
#define TCRSEL_CRSEL1        16   /* ACC select carrier generator                                16 ~ 17 */
#define TCRSEL_CASEL2        18   /* ACC select C/A generator                                    18 ~ 19 */
#define TCRSEL_CASFTSEL2     20   /* ACC select C/A code that shift from prompt code int chips   20 ~ 24 */
#define TCRSEL_CRSEL2        25   /* ACC select carrier generator                                25 ~ 26 */

#define SEL_GENERATOR0       0
#define SEL_GENERATOR1       1
#define SEL_GENERATOR2       2

struct TRACK_CHANNEL_DATA_STRUCT{
	uint32_t caoff;                       /* C/A offset */
	uint32_t cainc;                       /* C/A increment */
	uint32_t caphs;                       /* C/A phase */
	uint32_t crinc;                       /* carrier increment */
	uint32_t crphs;                       /* carrier phase */
};

struct TRACK_CHANNEL_RESULT_STRUCT{
	uint32_t iacc[9];                     /* cos acc i channel 0 ~ 8 */
	uint32_t qacc[9];                     /* sin acc q channel 0 ~ 8 */
	uint32_t tcnt[3];                     /* time count 0 ~ 2 */
	uint32_t tcaphso[3];                  /* correlator 0 ~ 2 c/a phase out */
	uint32_t tcrphso[3];                  /* correlator 0 ~ 2 carrier phase out */
};

struct TRACK_CHANNEL_STRUCT{
	uint32_t flag;                        /* ENABLE or DISABLE */
        uint32_t sat_id[3];                   /* satellite id 0, 1, 2 */
        uint32_t ca_sel[3];                   /* tracking channel 02 35 68 */
        uint32_t prompt[9];                   /* 00000   0 ~ 8 acc begin after cnt + prompt */
        struct TRACK_CHANNEL_DATA_STRUCT tdata[3];     /* tracking channel data */
        struct TRACK_CHANNEL_RESULT_STRUCT t_result;   /* tracking channel result */
};

struct TRACK_UNIT_STRUCT{
	uint32_t dma_flag;                    /* ENABLE or DISABLE */
	uint32_t dma_end_addr;                /* tracking unit dma end addr */
	uint32_t sin_thrd_flag;               /* ENABLE or DISABLE */
	uint32_t sin_thrd;                    
	struct TRACK_CHANNEL_STRUCT tchannel[TRACK_UNIT_NUM];   /* tracking channel 0 ~ 11 */
};

extern void track_int_control(void __iomem *base, uint32_t mode);
extern uint32_t track_state_check(void __iomem *base);
extern void track_unit_dma_transfer(void __iomem *base, uint32_t addr);
extern uint32_t track_unit_dma_check(void __iomem *base);

/* --------------------- PPS ---------------------------*/

struct PPS_STRUCT{
	uint32_t ppsset;                  /* count value */
	uint32_t ppsnum;                  /* counter threshold */
	uint32_t ppstype;                 /* PPS_PULSE or PPS_LEVEL */
	uint32_t ppswidth;                /* pps pulse width,  unit is GPS clock period */
};

/* pps */
#define PPS_PULSE            0    /* PPS signal is pulse per second */
#define PPS_LEVEL            1    /* PPS signal`s level will be invert per second */

/* function */
extern uint32_t PPS_init(void __iomem *base, struct PPS_STRUCT *pps);
extern uint32_t PPS_read(void __iomem *base);

#endif /* gps-reg.h  */
