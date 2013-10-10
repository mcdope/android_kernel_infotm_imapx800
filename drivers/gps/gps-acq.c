/*
 *
 */
#include "gps-reg.h"
#include "gps-acq.h"

#define ACQ_BUF_LEN     0x4000
#define BUF_LEN         (ACQ_BUF_LEN >> 2)

struct acq_sty a_strategy1 = ACQ_STY_SET(2, 5);
// TODO
struct acq_sty a_strategy2 = ACQ_STY_SET(1, 10);
struct acq_sty a_strategy3 = ACQ_STY_SET(20, 3);
struct acq_sty a_strategy4 = ACQ_STY_SET(20, 50);

struct acq_sty *a_sty[4] = {
		&a_strategy1,
		&a_strategy2,
		&a_strategy3,
                &a_strategy4
};

static inline void cc_set(struct instr *instr)
{
    instr->type = CC_INSTR;
    instr->dst = GPS_MEMA;
}

static inline void dma_set(struct instr *instr, uint32_t dst, uint32_t buf, uint32_t dir)
{
    instr->type = DMA_INSTR;
    instr->dst = dst;
    instr->buff = buf;
    instr->dir = dir;
}

static inline void ci_set(struct instr *instr, uint32_t dst, uint32_t srca, uint32_t srcb, uint32_t oper)
{
    instr->type = CI_INSTR;
    instr->dst = dst;
    instr->srca = srca;
    instr->srcb = srcb;
    instr->oper = oper;
}

/* TODO */
/* make sure cram_h has heen inited to zero */
#define CRAM_ZERO     GPS_MEMH
#define COH_INSTR_N_FIRST_STG1   6
#define CC_INC    0

static inline void coh_instr_set_first_stg1(struct instr *instr, uint32_t _r1, uint32_t _r2, 
		uint32_t _buf1, uint32_t _buf2)
{
	uint32_t count = 0;
    /* (CC_INSTR, CR_A, 0, 0, 0, 0, 0);
     * (DMA_INSTR, CRAM_ZERO, 0, 0, _buf1, CIMEM_TO_SYSMEM, 0);
     * (DMA_INSTR, CRAM_ZERO, 0, 0, _buf2, CIMEM_TO_SYSMEM, 0); 
     * (DMA_INSTR, _r1, 0, 0, _buf1, SYSMEM_TO_CIMEM, 0);
     * (CI_INSTR,  _r2, CR_A, _r1, 0, 0, COM_A_COM_TO_COM);
     * (DMA_INSTR, _r2, 0, 0, _buf1, CIMEM_TO_SYSMEM, 0); */
	cc_set(instr + count++);
	dma_set(instr + count++, CRAM_ZERO, _buf1, CIMEM_TO_SYSMEM);
	dma_set(instr + count++, CRAM_ZERO, _buf2, CIMEM_TO_SYSMEM);
	dma_set(instr + count++, _r1, _buf1, SYSMEM_TO_CIMEM);
	ci_set(instr + count++, _r2, GPS_MEMA, _r1, COM_A_COM_TO_COM);
	dma_set(instr + count++, _r2, _buf1, CIMEM_TO_SYSMEM);
}

#define COH_INSTR_N_STG1   4

static inline void coh_instr_set_stg1(struct instr *instr, uint32_t _r1, uint32_t _r2, 
		uint32_t _buf1)
{
	uint32_t count = 0;
    /* (CC_INSTR, CR_A, 0, 0, 0, 0, 0); 
     * (DMA_INSTR, _r1, 0, 0, _buf1, SYSMEM_TO_CIMEM, 0);
     * (CI_INSTR,  _r2, CR_A, _r1, 0, 0, COM_A_COM_TO_COM);
     * (DMA_INSTR, _r2, 0, 0, _buf1, CIMEM_TO_SYSMEM, 0); */
	cc_set(instr + count++);
	dma_set(instr + count++, _r1, _buf1, SYSMEM_TO_CIMEM);
	ci_set(instr + count++, _r2, GPS_MEMA, _r1, COM_A_COM_TO_COM);
	dma_set(instr + count++, _r2, _buf1, CIMEM_TO_SYSMEM);
}

#define INCOH_INSTR_N   7
#define INCOH_RESLUT_N  4
#define INCOH_CI_INC    2

static inline void incoh_instr_set(struct instr *instr, uint32_t _r1, uint32_t _r2, uint32_t _r3, 
		uint32_t _buf1, uint32_t _buf2)
{
	uint32_t count = 0;
    /* (CC_INSTR, CR_A, 0, 0, 0, 0, 0); 
     * (DMA_INSTR, _r1, 0, 0, _buf1, SYSMEM_TO_CIMEM, 0);
     * (DMA_INSTR, _r2, 0, 0, _buf2, SYSMEM_TO_CIMEM, 0);
     * (CI_INSTR,  _r3, CR_A, _r1, 0, 0, COM_A_COM_TO_COM);
     * (CI_INSTR,  _r1,  _r3, _r2, 0, 0, COM_A_REAL_TO_REAL);
     * (DMA_INSTR, CRAM_ZERO, 0, 0, _buf1, CIMEM_TO_SYSMEM, 0);
     * (DMA_INSTR, _r1, 0, 0, _buf2, CIMEM_TO_SYSMEM, 0); */
	cc_set(instr + count++);
	dma_set(instr + count++, _r1, _buf1, SYSMEM_TO_CIMEM);
	dma_set(instr + count++, _r2, _buf2, SYSMEM_TO_CIMEM);
	ci_set(instr + count++, _r3, GPS_MEMA, _r1, COM_A_COM_TO_COM);
	ci_set(instr + count++, _r1,  _r3, _r2, COM_A_REAL_TO_REAL);
	dma_set(instr + count++, CRAM_ZERO, _buf1, CIMEM_TO_SYSMEM);
	dma_set(instr + count++, _r1, _buf2, CIMEM_TO_SYSMEM);
}

static __pure uint32_t set_instr_sets(uint32_t instrn) 
{
	if(instrn < 18)
		return 1;
	else if(instrn < 36)
		return 2;
	else
		return 3;
}

#define BLOCK_FLAG         8
#define IS_BLOCK_SET(x)    ((x) & (1 << BLOCK_FLAG))

static inline uint32_t set_instr_block(uint32_t instr_type)
{
    return 	((1 << BLOCK_FLAG) | instr_type);
}

/*
 * get blockflag value
 */
static inline uint32_t get_blockflag_value(uint32_t type)
{
	type = type & 0xff;
	
	if(type == CC_INSTR)
		return 1;
	else if(type == CI_INSTR)
		return 4;
	else if(type == DMA_INSTR)
		return 8;
	else
		return 0;
}

/*
 * CC_INSTR CI_INSTR DMA_INSTR
 * 
 * COM_TO_COM COM_TO_REAL  COM_A_COM_TO_COM COM_S_COM_TO_COM
 * REAL_A_REAL_TO_REAL COM_A_COM_TO_REAL COM_A_REAL_TO_REAL REAL_M_REAL_TO_REAL
 * 
 * SYSMEM_TO_CIMEM CIMEM_TO_SYSMEM
 * 
 * GPS_MEMA ~ GPS_MEMH
 */
#define CC_INSTR_SET(block) \
	((block << AINSTR_BLOCKFLAG) | (CC_INSTR << AINSTR_INST_TYPE))
#define CI_INSTR_SET(src_a, src_b, dst, oper, block) \
	((CI_INSTR << AINSTR_INST_TYPE) | (oper << AINSTR_MODE) | (dst << AINSTR_DSTADDR) | \
	(src_a << AINSTR_SRCAADD) | (src_b << AINSTR_SRCBADD) | (block << AINSTR_BLOCKFLAG))


#define DMA_ADDR_OFF    (13 + 1 - AINSTR_HOSTADDR)
#define DMA_INSTR_SET(buf, dir, cram, block) \
    ((DMA_INSTR << AINSTR_INST_TYPE) | (dir << AINSTR_DIRECTION) | (cram << AINSTR_SRCA) | \
    (block << AINSTR_BLOCKFLAG) | ((buf & 0xffffc000) >> DMA_ADDR_OFF))

/*
 * acq instr generator
 * 
 * this is the function only to generator part of the acq instructions
 * you may use at the init, and store as part of instructions, get together while using instructions
 * to set to registers
 */
static uint32_t acq_instr_add_block(uint32_t len, struct instr *src, struct INSTR_STRUCT *instr)
{
	uint32_t cflag[8];
	uint32_t type_flag[3];
	uint32_t i;
	uint32_t blockflag;
	uint32_t srca;
	uint32_t srcb;
	uint32_t dst;
	
	for(i = 0; i < 8; i++)
		cflag[i] = 0;
	type_flag[0] = 0;
	type_flag[1] = 0;
	type_flag[2] = 0;
	
	instr->number = len;
	for(i = 0; i < len; i++){
		blockflag = 0;
		switch(src[i].type){
		case CC_INSTR:
			if(IS_BLOCK_SET(cflag[GPS_MEMA]))
				blockflag = get_blockflag_value(cflag[GPS_MEMA]);
			if(type_flag[CC_INSTR])
				blockflag |= 1;
			type_flag[CC_INSTR] = 1;
			cflag[GPS_MEMA] = set_instr_block(CC_INSTR);
			instr->instr[i] = CC_INSTR_SET(blockflag);
			break;
		case CI_INSTR:
			srca = src[i].srca;
			srcb = src[i].srcb;
			dst = src[i].dst;
			if(IS_BLOCK_SET(cflag[srca]))
				blockflag = get_blockflag_value(cflag[srca]);
			if(IS_BLOCK_SET(cflag[srcb]))
				blockflag |= get_blockflag_value(cflag[srcb]);
			if(IS_BLOCK_SET(cflag[dst]))
				blockflag |= get_blockflag_value(cflag[dst]);
			if(type_flag[CI_INSTR])
				blockflag |= 4;
			type_flag[CI_INSTR] = 1;
			cflag[srca] = set_instr_block(CI_INSTR);
			cflag[srcb] = set_instr_block(CI_INSTR);
			cflag[dst] = set_instr_block(CI_INSTR);
			instr->instr[i] = CI_INSTR_SET(srca, srcb, dst, src[i].oper, blockflag);
			break;
		case DMA_INSTR:
			dst = src[i].dst;
			if(IS_BLOCK_SET(cflag[dst]))
				blockflag = get_blockflag_value(cflag[dst]);
			if(type_flag[DMA_INSTR])
				blockflag |= 8;
			type_flag[DMA_INSTR] = 1;
			cflag[dst] = set_instr_block(DMA_INSTR);
			instr->instr[i] = DMA_INSTR_SET(src[i].buff, src[i].dir, dst, blockflag);
			break;
		default:
			return 1;
	    }
	}
	acq_instr_bit_set(&instr->instr[len - 1], AINSTR_LASTFLAG, 1);
	
	return 0;
}

static inline uint32_t instr_sets_set(uint32_t satn, uint32_t instr_n, struct instr *instr, struct instr_sets *sets)
{
	uint32_t i;
	uint32_t instrn = satn * instr_n;
	uint32_t setn = set_instr_sets(instrn - 1);
	
	if(instrn > MAX_INSTR_NUM)
		return 1;
	sets->sets_n = setn;
        for(i = 0; i < setn; i++){
    	    if(instrn > 18) {
    	        acq_instr_add_block(18, &instr[18 * i], &sets->set[i]);
        	instrn -= 18;
           }
    	   else
    		acq_instr_add_block(instrn, &instr[18 * i], &sets->set[i]);
        }
	
        return 0;	
}
// TODO
#if 0
static uint32_t cram_init(void)
{
    uint32_t i = 0;
    uint32_t buf[BUF_LEN];
    
    for(i = 0; i < BUF_LEN; i++)
    	buf[i] = 0;
    if(gps_dma_reg_cpu_set(buf, CRAM_ZERO, BUF_LEN))
    	return 1;
    return 0;
}
#endif
static inline uint32_t get_instr_position(uint32_t off, uint32_t start_n, uint32_t type, struct instr *instr)
{
    uint32_t i;
    uint32_t count = 0;
    
    for(i = start_n; i < off; i++){
    	if(instr[i].type == type)
    		count++;
    }
    return count;
}

/*
 * acq instructions sets generator
 * 
 * @buf  use to store coh or incoh data should be multi of 4K (align) buf[0] .. buf[n]
 * @sty  depend on sty choose diff strategy type
 * @oper mainly to set oper info about instruction
 */
static uint32_t acq_instr_xy_sets_generator(uint32_t *buf, struct sty_xy_init *sty, struct acq_oper_control *oper)
{
	uint32_t i = 0;
	uint32_t tmp;
	uint32_t sat_n = sty->sat_n;
	uint32_t level = sty->sat_stg - 1;
	struct instr instr[MAX_INSTR_NUM];
        
        // TODO	
//	if(cram_init())
//		return 1;
	
	switch(level){
	case 0:   /*  */
		break;
	case 1:
		if((sat_n != 1) && (sat_n != 2))
			goto error;
		sat_n = 2 * sat_n;
		break;
	case 2:
		if(sat_n != 1)
			goto error;
		sat_n = 3;
		break;
	case 3:
		if(sat_n != 1)
			goto error;
		sat_n = 4;
		break;
	default:
		goto error;
	}
	oper->instr_n = 3;
	/* coh first 
	 * x = 0, y = 0*/
	for(i = 0; i < sat_n; i++){
		coh_instr_set_first_stg1(&instr[i * COH_INSTR_N_FIRST_STG1], GPS_MEMB, GPS_MEMC, buf[i * 2], buf[2 * i + 1]);
		tmp = i * COH_INSTR_N_FIRST_STG1 + CC_INC;
		/* TODO
		 * as all this is sets[0] 3 cc  set1[1] 1 cc 
		 */
		oper->info.cc_n[i * 2] = set_instr_sets(tmp);
		oper->info.cc_n[i * 2 + 1] = get_instr_position(tmp, 18 * (set_instr_sets(tmp) - 1), 
				CC_INSTR, instr);
		
	}
	if(instr_sets_set(sat_n, COH_INSTR_N_FIRST_STG1, instr, &oper->sets[0]))
		goto error;

	/* coh */
	for(i = 0; i < sat_n; i++){
		coh_instr_set_stg1(&instr[i * COH_INSTR_N_STG1], GPS_MEMB, GPS_MEMC, buf[i * 2]);
	}
	if(instr_sets_set(sat_n, COH_INSTR_N_STG1, instr, &oper->sets[1]))
		goto error;
	
    /* incoh */
	for(i = 0; i < sat_n; i++){
		incoh_instr_set(&instr[i * INCOH_INSTR_N], GPS_MEMB, GPS_MEMC, GPS_MEMD, buf[2 * i], buf[2 * i + 1]);
		tmp = i * INCOH_INSTR_N + INCOH_RESLUT_N;
		oper->info.result_n[i * 2] = set_instr_sets(tmp);
		oper->info.result_n[i * 2 + 1] = get_instr_position(tmp, 18 * (set_instr_sets(tmp) - 1), 
				CI_INSTR, instr);
	}
	if(instr_sets_set(sat_n, INCOH_INSTR_N, instr, &oper->sets[2]))
		goto error;
	
	return 0;
error:
    return 1;
}

/*
 * acq instruction strategy xy generator
 */
uint32_t acq_instr_xy_generator(void *sty_xy, struct acq_oper_control *oper)
{
	uint32_t i = 0;
	uint32_t buf[16];
	struct sty_xy_init *sty = (struct sty_xy_init *)sty_xy;
	struct acq_sty_info *oper_sty = &oper->sty;
	
	// TODO 
	// buf
	for(i = 0; i < 16; i++)
		buf[i] = oper->buf_addr + i * ACQ_BUF_LEN;
	
	oper_sty->x = 0;
	oper_sty->y = 0;
	oper_sty->count = 0;
	oper_sty->sty_type = STY_X_Y;	
	oper_sty->inc_count = 0;
	oper_sty->strategy = a_sty[sty->sty_n];
	oper_sty->level = sty->sat_stg;
	// TODO
	// what if sat_stg is 3 that could not get int
	oper_sty->total = (oper_sty->strategy->total / sty->sat_stg);

	oper->info.sty = oper_sty;
	if(acq_instr_xy_sets_generator(buf, sty, oper))
		return 1;
	
    return 0;	
}

