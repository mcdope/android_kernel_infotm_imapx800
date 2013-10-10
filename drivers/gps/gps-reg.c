/*
 *
 */
#include <mach/imap-gps.h>

#include "gps-reg.h"

#define __gps_readl(base, off)           __raw_readl(base + off)
#define __gps_writel(val, base, off)    __raw_writel(val, base + off)

/***************************************************
 * search engine
 **************************************************/

/*
 * acquisition control
 * 
 * mode : ENABLE or DISABLE
 */
static inline void acq_control(void __iomem *base, uint32_t mode)
{
	__gps_writel(mode, base, ADRAM_EN);
}

/*
 * acquisition start
 * 
 * mode : ENABLE or DISABLE 
 */
static inline void acq_start(void __iomem *base, uint32_t mode)
{
	__gps_writel(mode, base, ASTACQ);	
}

/*
 * acquisition satellite ID set
 * 
 * id : satellite ID  0 ~ 5
 * off: offset of CA code 6 ~ 17
 * n : 0 ~ 5
 */
static inline void acq_sat_id_set(void __iomem *base, uint32_t n, uint32_t id, uint32_t off)
{
	__gps_writel((((id & 0x3f) << ASATID_SATID) | ((off & 0xfff) << ASATID_CAOFF)),
		 base, ASATIDn(n));	
}

/*
 * acquisition generator set
 * 
 * this is set basic parameters
 * cainc:  c/a increment
 * caphs:  c/a initial phase
 * crinc: carrier increment
 * crphs: carrier phase
 * n : 0 ~ 5
 */
static inline void acq_generator_data_set(void __iomem *base, uint32_t n, uint32_t cainc, 
		uint32_t caphs, uint32_t crinc, uint32_t crphs)
{

	__gps_writel(cainc, base, ACAINCn(n));	
	__gps_writel(caphs, base, ACAPHSn(n));	
	__gps_writel(crinc, base, ACRINCn(n));	
	__gps_writel(crphs, base, ACRPHSn(n));	
}

#if 0
/*
 * acquisition carrier generator threshold set
 * 
 * acq carrier generator quantize threshold for sin, 5 effective bit
 * and it is the threshold that sin convert from 1 to 3
 * and other values depend on this threshold
 */
static inline void acq_cr_thrd_set(void __iomem *base, uint32_t val)
{
	__gps_writel(val, base, ACRTHRD);	
}

/*
 * acquisition fft cut set
 * 
 * for data fft
 * dfft_f : BLOCK_FLOATION or SCALE_SCHEDULE
 * stage : scale schedule 
 * cut : cut configuration of complex multiply 0 ~ 3
 * n : 0 ~ 5
 */
static inline void acq_fft_cut_set(void __iomem *base, uint32_t n, uint32_t dfft_f, 
		uint32_t *stage, uint32_t cut)
{
	uint32_t ret  = ((dfft_f & 0x1) << ACUTCFG_DFFTSCALE) | ((stage[0] & 0xf) << (FFT_STAGE1 + ACUTCFG_STAGE_SHIFT))|
	                ((stage[1] & 0xf) << (FFT_STAGE2 + ACUTCFG_STAGE_SHIFT)) | ((stage[2] & 0xf) << (FFT_STAGE3 + ACUTCFG_STAGE_SHIFT)) |
	                ((stage[3] & 0xf) << (FFT_STAGE4 + ACUTCFG_STAGE_SHIFT)) | ((stage[4] & 0xf) << (FFT_STAGE5 + ACUTCFG_STAGE_SHIFT)) |
	                ((stage[5] & 0xf) << (FFT_STAGE6 + ACUTCFG_STAGE_SHIFT)) | ((cut & 0xf) << ACUTCFG_CUTCFG);
        __gps_writel(ret, base, ACUTCFGn(n));	
}

/*
 * acquisition ifft scale set
 * 
 * for invert fft
 * ifft_f : BLOCK_FLOATION or SCALE_SCHEDULE
 * stage : scale schedule
 * n : 0 ~ 5
 */
static inline void acq_iff_scale_set(void __iomem *base, uint32_t n, uint32_t ifft_f, uint32_t *stage)
{
	uint32_t ret  = ((ifft_f & 0x1) << AFFT_IFFTSCALE) | ((stage[0] & 0xf) << FFT_STAGE1 + ACUTCFG_STAGE_SHIFT)| 
	                ((stage[1] & 0xf) << FFT_STAGE2 + ACUTCFG_STAGE_SHIFT) | ((stage[2] & 0xf) << FFT_STAGE3 + ACUTCFG_STAGE_SHIFT) |
	                ((stage[3] & 0xf) << FFT_STAGE4 + ACUTCFG_STAGE_SHIFT) | ((stage[4] & 0xf) << FFT_STAGE5 + ACUTCFG_STAGE_SHIFT) |
                        ((stage[5] & 0xf) << FFT_STAGE6 + ACUTCFG_STAGE_SHIFT); 
        __gps_writel(ret, base, AFFTSCALEn(n));	
}

/*
 * acquisition get fft stat
 * 
 * n : 0 ~ 5
 */
static inline void acq_get_fft_stat(void __iomem *base, uint32_t n, uint32_t *buf)
{
        buf[0] = __gps_readl(base, ADFFTBEXPn(n));
        buf[1] = __gps_readl(base, AIFFTBEXPn(n));
        buf[2] = __gps_readl(base, AFFTSCALEn(n));
}
#endif

/*
 * acquisition instr set
 * 
 * cmd : cmd data
 * n : 0 ~ 17
 */
static inline void acq_instr_set(void __iomem *base, uint32_t n, uint32_t instr)
{
	__gps_writel(instr, base, AINSTRn(n));
}

/*
 * acquisition coherent/incoherent result get
 * 
 * n : 0 ~ 17
 * data : should be 4 , 32bit
 */
static inline void acq_ci_result_get(void __iomem *base, uint32_t n, uint32_t *data)
{
        data[0] = __gps_readl(base, ACORMAXn(n));
        data[1] = __gps_readl(base, ACORSMAXn(n));
        data[2] = __gps_readl(base, ACORMEANn(n));
        data[3] = __gps_readl(base, ACORMAXOFFn(n));
}

/*
 * acquisition inter control
 * 
 * mode : ENALBE or DISABLE
 */
inline void acq_inter_control(void __iomem *base, uint32_t mode)
{
	__gps_writel(mode, base, AINTEN);	
}

/*
 * acquisition state check (polling)
 * 
 * if do not use interrupt, you also can check
 * inter state, to check whether last instruction is finished or not
 */
inline uint32_t acq_state_check(void __iomem *base)
{
    uint32_t ret = 0;
    
    ret = __gps_readl(base, AINTSTAT);
    if(ret){
    	__gps_writel(0, base, AINTSTAT);
    }
    return ret;
}

/*
 * acquisition dram set
 * 
 * dram_mode : NORMAL_MODE, TEST_MODE1, TEST_MODE2 or TEST_MODE3
 */
static inline void acq_dram_set(void __iomem *base, uint32_t dram_mode)
{   
    __gps_writel((dram_mode & 0x3), base, ADRAMSTAT);
}

/*
 * acquisition cc basic data set
 * 
 * init cc basic data, such as C/A increment, C/A initial phase, Carrier increment, Carrier initial phase etc.
 */
static inline void acq_cc_basic_data_set(void __iomem *base, struct CC_STRUCT *cc)
{
	acq_sat_id_set(base, cc->number, cc->sat_id, cc->caoff);
	acq_generator_data_set(base, cc->number, cc->cainc, cc->caphs, cc->crinc, cc->crphs);
}

/*
 * acquisition cc init
 * 
 * init cc
 * there is a maximum of 6 that you can set the Registers
 */
uint32_t acq_cc_init(void __iomem *base, struct CC_STRUCT *cc)
{
	if(cc->number > 5)
		return 1;
	
	acq_cc_basic_data_set(base, cc);

	return 0;
}

/*
 * acquisition instr bit set
 * 
 * it uses to set the flag , like, AINSTR_LASTFLAG AINSTR_BLOCKFLAG3 
 *  AINSTR_BLOCKFLAG2 AINSTR_BLOCKFLAG1 AINSTR_BLOCKFLAG0
 * 
 * instr, is the instruction
 * bit, is the position of the bit
 * value, 0 or 1
 */
inline void acq_instr_bit_set(uint32_t *instr, uint32_t bit, uint32_t value)
{
	if(value)
		*instr |= 1 << bit;
	else
		*instr &= ~(1 << bit);
}

/*
 * acquisition instr init
 * 
 * set the instrucion
 * before the operation, the instr should have been set
 * and when you set the lastflag, should set the number 
 * to the struct
 * number is 1 more than count in the array
 */
inline uint32_t acq_instr_init(void __iomem *base, struct INSTR_STRUCT *instr)
{
    uint32_t i = 0;
	
    if(instr->number > 18)
    	return 1;
    for(i = 0; i < instr->number; i++)
    	acq_instr_set(base, i, instr->instr[i]);
    
    return 0;
}

/*
 * acquisition get coh/incoh result
 * 
 * after coh/incoh operation, if you wanna get the 
 * result of the data, then use this interface
 * n is the instr num of the order
 */
inline void acq_result_get(void __iomem *base, uint32_t n, struct CI_RESULT_STRUCT *ci)
{
	uint32_t buf[4];
	
	acq_ci_result_get(base, n, buf);
	ci->cor_max = buf[0];
	ci->cor_smax = buf[1];
	ci->cor_mean = buf[2];
	ci->cor_maxoff = buf[3] >> ACORMAXOFF_CORMAXOFF;
	ci->cor_smaxoff = buf[3] & 0xfff;
}

/*
 * acquisition operation control
 * 
 * when want to use it set INIT
 * after config, set START
 * STOP will end up one operation
 * if do not want to use the mould any more, set CLOSE  
 */
inline uint32_t acq_operation_control(void __iomem *base, uint32_t oper)
{
    if(oper == INIT)
    	acq_control(base, ENABLE);
    else if(oper == START)
    	acq_start(base, ENABLE);
    else if(oper == END)
    	acq_start(base, DISABLE);
    else if(oper == CLOSE)
    	acq_control(base, DISABLE);
    else
    	return 1;
    return 0;
}

/*
 * acquisition dram control
 * 
 * dram_mode : NORMAL_MODE, TEST_MODE1, TEST_MODE2 or TEST_MODE3
 */
inline void acq_dram_control(void __iomem *io_base, uint32_t dram_mode)
{
	acq_dram_set(io_base, dram_mode);
}

/*---------------------------- tracking unit -------------------------------------*/

// TODO
#if 0
/*
 * tracking unit control
 * 
 * as tracking unit both c/a and carrier need to work together
 * so we need to control both R
 * n : 0 ~ 11
 * mode : ENABLE or DISABLE
 * num : for , 0, 1, 2
 */
void track_reg_control(uint32_t n, uint32_t mode, uint32_t num)
{
	if(mode == ENABLE){
		rTCAGENn(n) |= mode << num;
		rTCRGENn(n) |= mode << num; 
	}
	else{
		rTCAGENn(n) &= ~(mode << num);
		rTCRGENn(n) &= ~(mode << num);
	}
}

/*
 * tracking unit satellite ID set
 * 
 * ID : the id of satellite
 * n : 0 ~ 11
 */
void track_sat_id_set(uint32_t n, uint32_t id0, uint32_t id1, uint32_t id2)
{
	rTSATIDn(n) = ((id0 & 0x3f) << TSATID_SATID0) | ((id1 & 0x3f) << TSATID_SATID1) |
	              ((id2 & 0x3f) << TSATID_SATID2);
}

/*
 * tracking unit channel carrier sel
 * 
 * value should be 3 uint32_t array, as 0 is for 02 , 
 * 1 is for 35, 2 is for 68
 * n : 0 ~ 11
 */
void track_channel_ca_sel(uint32_t n, uint32_t *value)
{
	rTCRSEL(n, 0) = *(value + 0);
	rTCRSEL(n, 1) = *(value + 1);
	rTCRSEL(n, 2) = *(value + 2);
}

/*
 * tracking unit channel data set
 * 
 * n : 0 ~ 11
 * tdata : 0 ~ 2 TDATA0, TDATA1, TDATA2
 * type : TCAOFF, TCAINC, TCAPHS, TCRINC, TCRPHS
 */
void track_channel_data_set(uint32_t n, uint32_t tdata, uint32_t type, uint32_t value)
{
	rTDATASET(n, tdata, type) = value;
}

/*
 * tracking unit outdata get 
 * 
 * n : 0 ~ 11
 * outdata should be 3
 */
void track_outdata_get(uint32_t n, uint32_t *tcnt, uint32_t *caphso, uint32_t *crphso)
{
	uint32_t i = 0;
	
    for(i = 0; i < 3; i++){
    	tcnt[i] = rTTCNTn(n, i);
    	caphso[i] = rTCAPHSOn(n, i);
    	crphso[i] = rTCRPHSOn(n, i);
    }
} 

/*
 * tracking unit iacc get 
 * 
 * n : 0 ~ 11
 * outdata should be 9
 */
void track_iacc_get(uint32_t n, uint32_t *iacc)
{
	uint32_t i = 0;
	
    for(i = 0; i < 9; i++){
    	iacc[i] = rTIACC0n(n, i);
    }
}

/*
 * tracking unit qacc get 
 * 
 * n : 0 ~ 11
 * outdata should be 9
 */
void track_qacc_get(uint32_t n, uint32_t *qacc)
{
	uint32_t i = 0;
	
    for(i = 0; i < 9; i++){
    	qacc[i] = rTQACC0n(n, i);
    }
} 
#endif

/*
 * tracking unit DMA start
 * 
 * only write 1, when it completes, this bit will be cleared 
 * 
 *    63  :  0          1 unit   * 12
 *  iacc 1,  i0         ---           
 *    i3  ,  i2          |   
 *    i5  ,  i4          |   
 *    i7  ,  i6          |   
 *  qacc 0,  i8          |   
 *    q2  ,  q1          |   
 *    q4  ,  q3          |   
 *    q6  ,  q5       16 * 2 
 *    q8  ,  q7          |   
 *  tcnt1 , tcnt0        |   
 *  ca_po0, tcnt2        |   
 *  ca_po2, ca_po1       |   
 *  cr_po1, cr_po0       |   
 *     0  , cr_po2       |   
 *     0  ,   0          |   
 *     0  ,   0         ---
 */
static inline void track_dma_start(void __iomem *base)
{
	__gps_writel(ENABLE, base, TDMACTRL);
}

/*
 * tracking unit DMA check
 * 
 * return the state of the DMA
 */

inline uint32_t track_dma_state_check(void __iomem *base)
{
	 return __gps_readl(base, TDMACTRL);
}

/*
 * tracking unit dma end addr set
 * 
 * it should align to 16kb
 */
static inline void track_dma_end_addr_set(void __iomem *base, uint32_t addr)
{
	__gps_writel(addr, base, TDMAHADDR);
}

inline void track_unit_dma_transfer(void __iomem *base, uint32_t addr)
{
        track_dma_end_addr_set(base, addr);
	track_dma_start(base);
}

/*
 * tracking unit inter control
 * 
 * mode : ENABLE or DISABLE
 */
inline void track_int_control(void __iomem *base, uint32_t mode)
{
	__gps_writel(mode, base, TINTEN);	
}

/*
 * tracking unit state check (polling)
 * 
 * if do not use interrupt, you also can check
 * inter state, to check whether last instruction is finished or not
 */
inline uint32_t track_state_check(void __iomem *base)
{
    uint32_t ret = 0;
    
    ret = __gps_readl(base, TINTSTAT);
    if(ret){
    	__gps_writel(0, base, TINTSTAT);
    }
    return ret;
}

#if 0
/*
 * track carrier generator threshold set
 * 
 * same to acquisition
 */
static inline void track_cr_thrd_set(void __iomem *base, uint32_t value)
{
	__gps_writel(value, base, TCRTHRD);	
}
#endif

/*---------------------------- PPS -------------------------------------*/
/*
 * pps get count
 * 
 * read back the current PPS count value
 * and the counter will still count
 * 
 * 0 ~ 23
 */
static inline uint32_t PPS_get_count(void __iomem *base)
{
	return __gps_readl(base, PPSCNT);
}

/*
 * pps set count
 * 
 * set the PPS count value
 * and counter will update the value in the next millisecond interrupt
 * 
 * 0 ~ 23
 */
static inline void PPS_set_count(void __iomem *base, uint32_t val)
{
	__gps_writel((val & 0xffffff), base, PPSSET);	
}

/*
 * PPS set threshold
 * 
 * pps counter threshold , when pps counter equal PPSNUM
 * pps counter will be set to zero
 */
static inline void PPS_set_threshold(void __iomem *base, uint32_t val)
{
	__gps_writel((val & 0xffffff), base, PPSNUM);
}

/*
 * pps type set
 * 
 * type : PPS_PULSE or PPS_LEVEL
 */
static inline void PPS_type_set(void __iomem *base, uint32_t type)
{
	__gps_writel(type, base, PPSTYPE);
}

/*
 * pps width set
 * 
 * 0 ~ 23
 * it only uses in pps pulse, and make sure that
 * width is less then sum
 */
static inline void PPS_width_set(void __iomem *base, uint32_t val)
{
	__gps_writel((val & 0xffffff), base, PPSWID);
}

/*
 * pps init
 */
uint32_t PPS_init(void __iomem *base, struct PPS_STRUCT *pps)
{
	if(pps->ppstype == PPS_PULSE)
		PPS_width_set(base, pps->ppswidth);
	PPS_set_threshold(base, pps->ppsnum);
	PPS_set_count(base, pps->ppsset);
	PPS_type_set(base, pps->ppstype);
	
	return 0;
}

/*
 * pps read
 * 
 * return the value of count
 */
inline uint32_t PPS_read(void __iomem *base)
{
	return PPS_get_count(base);
}


