#ifndef __GPS_ACQ_H__
#define __GPS_ACQ_H__
   
#define MAX_INSTR_NUM           48

struct instr{
	uint32_t type;
	uint32_t dst;
	uint32_t srca;
	uint32_t srcb;
	uint32_t buff;
	uint32_t dir;
	uint32_t oper;
};

/******************************************************************************************/
/*
 * acq strategy 
 */
struct acq_sty{
	uint32_t coh_t;       /* coherent integration time ms */       
	uint32_t incoh_t;     /* incoherent integration times */
	uint32_t fre_inc;     /* frequency increase Hz */
	uint32_t coh_num;     /* 10kHz / (1s / (2 * coh_t))  sub oper num */
	uint32_t count;       
	uint32_t total;       /* total oper times */
};

#define FRE_EOF                  4
#define ACQ_CARRIER_RANGE        100000    /* 10kHz */
#define MS_NUM_1_S               1000      /* 1000ms */
#define FRE_INCREASE(x)          (MS_NUM_1_S * 262 / FRE_EOF * x)    //(MS_NUM_1_S * (double)0x100000000/ (FRE_EOF * x * 16367667))    /* x is coh_t */
#define GET_COH_NUM(x)           (ACQ_CARRIER_RANGE * FRE_EOF * x / MS_NUM_1_S) /* x is coh_t sub */
#define GET_TOAL(x, y)           ((x) * (y) * GET_COH_NUM(x))

#define ACQ_STY_SET(_x, _y) \
{ \
    _x, \
    _y, \
    FRE_INCREASE(_x), \
    GET_COH_NUM(_x),  \
    (_x * _y),        \
    GET_TOAL(_x, _y), \
}


#define STY_X_Y           0

struct acq_sty_info{
	uint32_t sty_type;
	uint32_t x;
	uint32_t y;
	uint32_t count;
	uint32_t inc_count;
	uint32_t total;
	uint32_t level;
	struct acq_sty *strategy;  /* strategy */
};      

/******************************************************************************************/

#define MAX_INSTR_SETS        3    
#define MAX_INSTR_SETS_N      3     
#define MAX_ACQ_SAT_N         4

#define ENABLE             1
#define DISABLE            0

struct acq_oper_sat_info{
	uint32_t sat_n;                                 /* num of sat to acq */
	uint32_t cc_flag[MAX_INSTR_SETS]; 
	uint32_t ci_flag[MAX_INSTR_SETS];
	uint32_t result_n[MAX_ACQ_SAT_N * 2];           /* get result in instr order */
	                                                /* 2 * i store set_n,   2 * i + 1 store instr_n */
	uint32_t cc_n[MAX_ACQ_SAT_N * 2];               /* same as result_n */
	uint32_t count[MAX_ACQ_SAT_N];                  /* count of result to set */
	struct acq_sty_info *sty;
	struct CC_STRUCT cc[MAX_ACQ_SAT_N];             
	struct CI_RESULT_STRUCT result[MAX_ACQ_SAT_N];  
};

/*
 * one instruction sets
 * like if there are 36 instrs we need 2 instr
 */
struct instr_sets{
	uint32_t sets_n;     /* num of instr sets */
	struct INSTR_STRUCT set[MAX_INSTR_SETS];
};

/* sty_type */
/* STY_X_Y  
 * sty     : 1, 2, 3, ..., n mean strategy 1, 2, ...,n
 * sat_n   : sat num 1 ~ 4
 * sat_stg : strong level 1, 2, 3 or 4 it means the num of acq same sat id
 * */
struct sty_xy_init{
	uint32_t sty_n;
	uint32_t sat_n;
	uint32_t sat_stg;
};

struct acq_oper_init_msg{
	uint32_t sat_num;
	uint32_t sat_id[MAX_ACQ_SAT_N];
	uint32_t sty_type;
	void *cc_point[MAX_ACQ_SAT_N];
	void *sty;
};

struct acq_oper_control{
	uint32_t en;         /* ENABLE or DISABLE if set means have acq operation*/
	uint32_t flag;       /* flag of acq */
	uint32_t instr_n;    /* total instr num */
	uint32_t ms_count;   /* ms count */
	uint32_t set_seq;    /* point to num of instr sets  set[set_seq]*/ 
	uint32_t sty_flag;  
        uint32_t buf_addr;	
	struct acq_sty_info sty;
	struct instr_sets sets[MAX_INSTR_SETS_N];   /* one acq oper may have lots of diff instr sets like */ 
	                                            /* first instr are diff each */
	struct acq_oper_sat_info info;
};

#define STY_ON              0
#define STY_DONE            1

/* flag  0 : 7  state */
#define ACQ_OPER_DONE       0    /*  */       
#define ACQ_OPER_INTER_1    1
#define ACQ_OPER_INTER_2    2
#define ACQ_OPER_INTER_3    3
#define ACQ_OPER_CONTINUAL  7
#define ACQ_OPER_ERROR      0xff

extern uint32_t acq_instr_xy_generator(void *sty_xy, struct acq_oper_control *oper);

#endif /* gps-acq.h */
