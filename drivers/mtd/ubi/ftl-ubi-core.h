extern int ftl_core_io_init(struct ubi_volume_desc *ubi, int _leb_num, int _page_num, int _page_size, int sector_num);
extern int ftl_core_io_read(struct ubi_volume_desc *ubi, long sector, char *buf);
extern int ftl_core_io_write(struct ubi_volume_desc *ubi, long sector, char *buf);
extern int ftl_core_io_flush(struct ubi_volume_desc *ubi);
extern int ftl_core_io_auto_flush(struct ubi_volume_desc *ubi);
