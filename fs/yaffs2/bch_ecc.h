extern int nand_correct_data_bch(unsigned char *buf,
   unsigned char *read_ecc, unsigned char *calc_ecc);
extern int nand_calculate_ecc_bch(const unsigned char *buf,
   unsigned char *code);
extern void bch_init(void);
