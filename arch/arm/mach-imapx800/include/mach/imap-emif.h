#ifndef __IMAP_EMIF_H__
#define __IMAP_EMIF_H__

/* uMCTL Registers */
#define UMCTL_PCFG(x)               (0x400 + 4 * x)     /* Port x */     
                                                                                               
#define UMCTL_CCFG                  (0x480)             /* Controller Configuration Register */
#define UMCTL_DCFG                  (0x484)             /* DRAM Configuration Register */       
#define UMCTL_CSTAT                 (0x488)             /* Controller Status Register */        
#define UMCTL_CCFG1                 (0x48c)             /* Controller Status Register 1     add */

#endif /* imap-emif.h */
