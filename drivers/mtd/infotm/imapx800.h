
#ifndef __MTD_IMAPX800_H__
#define __MTD_IMAPX800_H__

#define NAND_BADBLOCK_MAJOR	(1)
#define NAND_BADBLOCK_MINOR	(1)

#define IMAPX800_NAND_16BIT	(0)

#define IMAPX800_NAND_IDLE	(0)
#define IMAPX800_NAND_READ	(1)
#define IMAPX800_NAND_WRITE	(2)

#define MAX_DMABUF_CNT		(2)

int nand_get_device(struct mtd_info *mtd, int new_state);
void nand_release_device(struct mtd_info *mtd);

int imap_nand_read_oob(struct mtd_info *mtd, uint8_t *oobbuf, int page, int oobbytes);

int imap_nand_read_page(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes);

int imap_nand_read_raw(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobbuf, int page, int bytes, int oobbytes);

int imap_nand_write_page(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobuf, int page, int bytes, int oobbytes);

int imap_nand_write_raw(struct mtd_info *mtd, uint8_t *pagebuf, uint8_t *oobuf, int page, int bytes, int oobbytes);

int imapx800_nand_read_status(struct mtd_info *mtd, unsigned int *status);

int imapx800_nand_erase(struct mtd_info *mtd, int page, int cycle, unsigned int *status);

int imapx800_nand_block_markbad(struct mtd_info *mtd, loff_t ofs);

int imapx800_nand_get_badmark(struct mtd_info *mtd, int page);

int nand_readid(struct mtd_info *mtd, uint8_t buf[], int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw);

int nf2_timing_init(int interface, int timing, int rnbtimeout, int phyread, int phydelay, int busw);

int nand_init_randomizer(struct nand_config *cfg);

int nf2_ecc_num (int ecc_type, int page_num, int half_page_en);

int nf2_active_sync(struct mtd_info *mtd);

int nf2_active_async(struct mtd_info *mtd);

int nf2_asyn_reset(struct mtd_info *mtd);

int imap_nand_valied_vm(uint32_t vm_addr, int len);

int imap_nand_hwinit(void);

int imap_set_retry_param_26(struct mtd_info *mtd, int retrylevel, unsigned char *buf);

int imap_set_retry_param_21(struct mtd_info *mtd, int retrylevel, unsigned char *buf);

int imap_get_retry_param_26(struct mtd_info *mtd, unsigned char *buf);

int imap_get_retry_param_21(struct mtd_info *mtd, unsigned char *buf);

int imap_get_retry_param_20(struct mtd_info *mtd, unsigned int *buf, int len, int param);

int isbad(struct mtd_info *mtd, loff_t start);

int imap_get_retry_param_20_from_page(struct mtd_info *mtd, unsigned int *buf, int len, int page);

int imap_set_retry_param_20(struct mtd_info *mtd, int retrylevel, unsigned char * otp_table, unsigned char * reg_buf);

int imap_nand_read_page_random(struct mtd_info *mtd, uint8_t *pagebuf, int page, int bytes);

int imap_set_retry_param_micro_20(struct mtd_info *mtd, int retrylevel);

int init_retry_param_sandisk19(struct mtd_info *mtd);

int imap_set_retry_param_sandisk19(struct mtd_info *mtd, int retrylevel);

int get_second_otp_table(struct mtd_info *mtd, loff_t start);

int rtc_reboot(void);
#endif

