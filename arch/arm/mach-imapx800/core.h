
#ifndef __IMAP_CORE_H__
#define __IMAP_CORE_H__

/* this is implemented in core.c */
extern void __init imap_init_devices(void);

/* this is implemented in clock-ops.c */
extern void __init imap_init_clocks(void);

/* this is implemented in gpio.c */
extern void __init imap_init_gpio(void);

/* this is implemented in mem_reserve.c */
extern void imap_mem_reserve(void);

#define core_msg(x...) printk(KERN_ERR "imap: " x)

#endif /* __IMAP_CORE_H__ */

