/*****************************************************************************
** regs-nand.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:	iMAP NAND Flash platform driver header file. 
**				Layout of the NAND Flash controller registers on iMAPx200.
**
** Author:
**     warits    <warits@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  02/10/2009  Initialized by warits.
*****************************************************************************/


#ifndef __ARM_IMAPX_REGS_NAND
#define __ARM_IMAPX_REGS_NAND

/* Register Layout */

#define iMAPX200_NAND_BASE

#define iMAPX200_NFCONF						0x00
#define iMAPX200_NFCONT						0x04
#define iMAPX200_NFCMD						0x08
#define iMAPX200_NFADDR						0x0c
#define iMAPX200_NFDATA						0x10
#define iMAPX200_NFMECCD0					0x14
#define iMAPX200_NFMECCD1					0x18
#define iMAPX200_NFMECCD2					0x1c
#define iMAPX200_NFSBLK						0x20
#define iMAPX200_NFEBLK						0x24
#define iMAPX200_NFSTAT						0x28
#define iMAPX200_NFESTAT0					0x2c
#define iMAPX200_NFESTAT1					0x30
#define iMAPX200_NFMECC0					0x34
#define iMAPX200_NFMECC1					0x38
#define iMAPX200_NFMECC2					0x3c
#define iMAPX200_NFESTAT2					0x40
#define iMAPX200_NFSECCD					0x44
#define iMAPX200_NFSECC						0x48
#define iMAPX200_NFDESPCFG					0x4c
#define iMAPX200_NFDESPADDR					0x50
#define iMAPX200_NFDESPCNT					0x54
#define iMAPX200_NFDECERRFLAG				0x58		/* if 1, spare ecc is error, else main ess is error */
#define iMAPX200_NFPAGECNT					0x5c		/* [0:3]: page count, [4:15]: unit count, means '6+1' counter */
#define iMAPX200_NFDMAADDR_A				0x80
#define iMAPX200_NFDMAC_A					0x84
#define iMAPX200_NFDMAADDR_B				0x88
#define iMAPX200_NFDMAC_B					0x8c
#define iMAPX200_NFDMAADDR_C				0x90
#define iMAPX200_NFDMAC_C					0x94
#define iMAPX200_NFDMAADDR_D				0x98
#define iMAPX200_NFDMAC_D					0x9c

#define iMAPX800_NAND_BASE

#define NF2AFTM				(0x00)		/* NAND Flash AsynIF timing */
#define NF2SFTM				(0x04)      	/* NAND Flash SynIF timing */
#define NF2TOUT				(0x08)      	/* NAND Flash RnB timeout cfg. */
#define NF2STSC				(0x0C)      	/* NAND Flash status check cfg. */
#define NF2STSR0			(0x10)      	/* NAND Flash Status register 0, read only */
#define NF2STSR1			(0x14)      	/* NAND Flash Status register 1, read only */
#define NF2ECCC				(0x18)       /*  ECC configuration */
#define NF2PGEC				(0x1C)       /*  Page configuration */
#define NF2RADR0			(0x20)		/* NAND Flash Row ADDR0 */
#define NF2RADR1			(0x24)		/* NAND Flash Row ADDR1 */
#define NF2RADR2			(0x28)		/* NAND Flash Row ADDR2 */
#define NF2RADR3			(0x2C)		/* NAND Flash Row ADDR3 */
#define NF2CADR				(0x30)       /* Column Address */
#define NF2AADR				(0x34)       /* ADMA descriptor address */
#define NF2SADR0			(0x38)       /* SDMA Ch0 address */
#define NF2SADR1			(0x3C)       /* SDMA Ch1 address */
#define NF2SBLKS			(0x40)       /* SDMA block size */
#define NF2SBLKN			(0x44)       /* SDMA block number */
#define NF2DMAC				(0x48)       /* DMA Configuration */
#define NF2CSR				(0x4C)       /* Chip select register */

#define NF2DATC			(0x50)       /* data interrupt configuration */
#define NF2FFCE			(0x54)       /* FIFO Configuration enable */
#define NF2AFPT			(0x58)       /* AFIFO access register */
#define NF2AFST			(0x5C)       /* AFIFO status register, read only */
#define NF2TFPT			(0x60)       /* TRX-AFIFO access register */
#define NF2TFST			(0x64)       /* TRX-AFIFO status register, read only */
#define NF2TFDE			(0x68)       /* TRX-AFIFO debug enable */
#define NF2DRDE			(0x6C)       /* DataRAM debug enable */
#define NF2DRPT			(0x70)       /* DataRAM read access register, read only */
#define NF2ERDE			(0x74)       /* ECCRAM debug enable */
#define NF2ERPT			(0x78)       /* ECCRAM access register */
#define NF2FFTH			(0x7C)       /* FiFo Threshold */

#define NF2SFTR			(0x80)       /* Soft reset register */
#define NF2STRR			(0x84)       /* Start register */
#define NF2INTA			(0x88)       /* interactive interrrupt ack register */
#define NF2INTR			(0x8C)       /* interrrupt status register */
#define NF2INTE			(0x90)       /* interrrupt enable register */
#define NF2WSTA			(0x94)       /* Work status register, read only */
#define NF2SCADR	    	(0x98)       /* SDMA current address, read only */
#define NF2ACADR	    	(0x9C)       /* ADMA current address, read only */
#define NF2IFST			(0xA0)       /* ISFIFO status register, read only */
#define NF2OFST			(0xA4)       /* OSFIFO status register, read only */
#define NF2TFST2	    	(0xA8)       /* TRX-FIFO status register2, read only */
#define NF2WDATL	    	(0xAC)       /* Write data low 32 bits. */
#define NF2WDATH	    	(0xB0)       /* Write data high 32 bits. */
#define NF2DATC2	    	(0xB4)       /* data interrupt configuration2 */

#define NF2PBSTL		(0xC0)       /* PHY burst length, byte unit */
#define NF2PSYNC	    	(0xC4)       /* PHY Sync mode cfg. */
#define NF2PCLKM	    	(0xC8)       /* PHY Clock mode cfg. */
#define NF2PCLKS	    	(0xCC)       /* PHY Clock STOP cfg. */
#define NF2PRDC			(0xD0)       /* PHY Read cfg. */
#define NF2PDLY			(0xD4)       /* PHY Delay line cfg. */
#define NF2RESTCNT		(0xD8)
#define NF2TRXST		(0xDC)

#define NF2STSR2			(0xE0)      	/* NAND Flash Status register 2, read only */
#define NF2STSR3			(0xE4)      	/* NAND Flash Status register 3, read only */
#define NF2ECCINFO0			(0xE8)		/* 0-3 block ecc */
#define NF2ECCINFO1			(0xEC)		/* 4-7 block ecc */
#define NF2ECCINFO2			(0xF0)		/* 8-11 block ecc */
#define NF2ECCINFO3			(0xF4)		/* 12-15 block ecc */
#define NF2ECCINFO4			(0xF8)		/* 16-19 block ecc */
#define NF2ECCINFO5			(0xFC)		/* 20-23 block ecc */
#define NF2ECCINFO6			(0x100)		/* 24-27 block ecc */
#define NF2ECCINFO7			(0x104)		/* 28-31 block ecc */
#define NF2ECCINFO8			(0x108)		/* ECC unfixed info */
#define NF2ECCINFO9			(0x10C)		/* secc ecc info, bit 7 vailed, bit 6 unfixed, 3-0 bit errors */

#define NF2ECCLEVEL			(0x11C)
#define NF2SECCDBG0			(0x120)
#define NF2SECCDBG1			(0x124)
#define NF2SECCDBG2			(0x128)
#define NF2SECCDBG3			(0x12C)
#define NF2DATSEED0			(0x140)
#define NF2DATSEED1			(0x144)
#define NF2DATSEED2			(0x148)
#define NF2DATSEED3			(0x14C)
#define NF2ECCSEED0			(0x150)
#define NF2ECCSEED1			(0x154)
#define NF2ECCSEED2			(0x158)
#define NF2ECCSEED3			(0x15C)
#define NF2RANDMP			(0x160)
#define NF2RANDME			(0x164)
#define NF2DEBUG0			(0x168)
#define NF2DEBUG1			(0x16c)
#define NF2DEBUG2			(0x170)
#define NF2DEBUG3			(0x174)
#define NF2DEBUG4			(0x178)
#define NF2DEBUG5			(0x17c)
#define NF2DEBUG6			(0x180)
#define NF2DEBUG7			(0x184)
#define NF2DEBUG8			(0x188)
#define NF2DEBUG9			(0x18c)
#define NF2DEBUG10			(0x190)
#define NF2DEBUG11			(0x194)

#define NF2BSWMODE			(0x1B8) //bit[0] = b'1' (16bit mode), bit[0] = b'0' (8bit mode)
#define NF2BSWF0			(0x1BC)
#define NF2BSWF1			(0x1C0)

#define NF2RAND0			(0x1C4) //bit[0:15] = seed, bit[16:31] = cycles
#define NF2RAND1			(0x1C8) //bit[0] = start, frist wirte bit[0] = b'1' then write bit[0] = b'0'
#define NF2RAND2			(0x1CC) //bit[16] = state [0: finished] [1: busy], bit[0:15] = result

#define DIVCFG4_ECC			(SYSMGR_BASE_ADDR+0x060)
/* Defination of NFCONF */

#define iMAPX200_NFCONF_TWRPH0_(x)			((x)<<8)
#define iMAPX200_NFCONF_TWRPH1_(x)			((x)<<4)
#define iMAPX200_NFCONF_TACLS_(x)			((x)<<12)
#define iMAPX200_NFCONF_TMSK				(0x07)
#define iMAPX200_NFCONF_ECCTYPE4			(1<<24)		/* 1 for MLC(4-bit), 0 for SLC(1-bit). */
#define iMAPX200_NFCONF_BusWidth16			(1<<0)		/* 1 for 16bit, 0 for 8bit. */

/* Defination of NFCONT */

#define iMAPX200_NFCONT_MODE				(1<<0)
#define iMAPX200_NFCONT_Reg_nCE0			(1<<1)
#define iMAPX200_NFCONT_Reg_nCE1			(1<<2)
#define iMAPX200_NFCONT_InitSECC			(1<<4)
#define iMAPX200_NFCONT_InitMECC			(1<<5)
#define iMAPX200_NFCONT_SpareECCLock		(1<<6)
#define iMAPX200_NFCONT_MainECCLock			(1<<7)
#define iMAPX200_NFCONT_RnB_TransMode		(1<<8)
#define iMAPX200_NFCONT_EnRnBINT			(1<<9)
#define iMAPX200_NFCONT_EnIllegalAccINT		(1<<10)
#define iMAPX200_NFCONT_EnECCDecINT			(1<<12)
#define iMAPX200_NFCONT_EnECCEncINT			(1<<13)
#define iMAPX200_NFCONT_DMACompleteINT		(1<<14)
#define iMAPX200_NFCONT_DMABlockEndINT		(1<<15)
#define iMAPX200_NFCONT_INTMSK				(0x3f << 9)
#define iMAPX200_NFCONT_SoftLock			(1<<16)
#define iMAPX200_NFCONT_LockTight			(1<<17)
#define iMAPX200_NFCONT_ECCDirectionEnc		(1<<18)

/* Defination of NFSTAT */

#define iMAPX200_NFSTAT_RnB_ro				(1<<0)		/* 0 busy, 1 ready */
#define iMAPX200_NFSTAT_nFCE_ro				(1<<2)
#define iMAPX200_NFSTAT_nCE_ro				(1<<3)
#define iMAPX200_NFSTAT_RnB_TransDetect		(1<<4)		/* write 1 to clear */
#define iMAPX200_NFSTAT_IllegalAccess		(1<<5)		/* write 1 to clear(to be confirmed) */
#define iMAPX200_NFSTAT_ECCDecDone			(1<<6)		/* write 1 to clear */
#define iMAPX200_NFSTAT_ECCEncDone			(1<<7)		/* write 1 to clear */
#define iMAPX200_NFSTAT_DMA_COMPLETE		(1<<8)
#define iMAPX200_NFSTAT_DMA_BLOCKEND		(1<<9)
#define iMAPX200_NFSTAT_ProgramErr			(1<<10)		/* Err=1, OK=0, write 1 to clear */
#define iMAPX200_NFSTAT_DespEnd				(1<<11)		/* OK=1 */
#define iMAPX200_NFSTAT_DespECCErr			(1<<12)		/* if intr en in description, Err=1, OK=0, write 1 to clear */
#define iMAPX200_NFSTAT_ECCErrResult		(1<<13)

/* Defination of SLC NFESTAT0 */

#define iMAPX200_NFESTAT0_SLC_MErrType_ro_		(0)
#define iMAPX200_NFESTAT0_SLC_SErrType_ro_		(2)
#define iMAPX200_NFESTAT0_SLC_Err_MSK			(0x03)
#define iMAPX200_NFESTAT0_SLC_MByte_Loc_ro_		(7)
#define iMAPX200_NFESTAT0_SLC_SByte_Loc_ro_		(21)
#define iMAPX200_NFESTAT0_SLC_MByte_Loc_MSK		(0x7ff)
#define iMAPX200_NFESTAT0_SLC_SByte_Loc_MSK		(0xf)
#define iMAPX200_NFESTAT0_SLC_MBit_Loc_ro_		(4)
#define iMAPX200_NFESTAT0_SLC_SBit_Loc_ro_		(18)
#define iMAPX200_NFESTAT0_SLC_Bit_MSK			(0x7)

/* Defination of MLC NFESTAT0 & NFESTAT1 & NFESTAT2*/

#define iMAPX200_NFESTAT0_Busy_ro				(1<<31)
#define iMAPX200_NFESTAT0_Ready_ro				(1<<30)
#define iMAPX200_NFESTAT0_MLC_MErrType_ro_		(27)
#define iMAPX200_NFESTAT0_MLC_MErrType_MSK		(0x7)
#define	iMAPX200_NFESTAT0_MLC_Loc1_ro_			(0)
#define	iMAPX200_NFESTAT1_MLC_Loc2_ro_			(0)
#define	iMAPX200_NFESTAT1_MLC_Loc3_ro_			(9)
#define	iMAPX200_NFESTAT1_MLC_Loc4_ro_			(18)
#define iMAPX200_NFESTAT0_MLC_PT1_ro_			(16)
#define iMAPX200_NFESTAT2_MLC_PT2_ro_			(0)
#define iMAPX200_NFESTAT2_MLC_PT3_ro_			(9)
#define iMAPX200_NFESTAT2_MLC_PT4_ro_			(18)
#define iMAPX200_NFESTATX_MLC_PT_MSK			(0x1ff)
#define iMAPX200_NFESTATX_MLC_Loc_MSK			(0x1ff)

/* Defination of DMAC */
#define iMAPX200_NFDMAC_DMADIROut						(1<<25)
#define iMAPX200_NFDMAC_DMAAUTO							(1<<26)
#define iMAPX200_NFDMAC_DMAALT							(1<<27)
#define iMAPX200_NFDMAC_DMARPT							(1<<28)
#define iMAPX200_NFDMAC_DMAWIND							(1<<29)
#define iMAPX200_NFDMAC_DMARST							(1<<30)
#define iMAPX200_NFDMAC_DMAEN							(1<<31)

#endif
