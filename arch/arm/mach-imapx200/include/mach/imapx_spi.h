#ifndef __IMAPX_SPI__
#define __IMAPX_SPI__


#define rSPCON0     (0x00 )	//SPI0 control
#define rSPSTA0     (0x04 )	//SPI0 status
#define rSPPIN0     (0x08 )	//SPI0 pin control
#define rSPPRE0     (0x0c )	//SPI0 baud rate prescaler
#define rSPTDAT0    (0x10 )	//SPI0 Tx data
#define rSPRDAT0    (0x14 )	//SPI0 Rx data

#define rSSI_CTLR0_M						 (0x0 )
#define rSSI_CTLR1_M						 (0x4 )
#define rSSI_ENR_M						 (0x8 )
#define rSSI_MWCR_M						 (0xC )
#define rSSI_SER_M						 (0x10 )
#define rSSI_BAUDR_M						 (0x14 )
#define rSSI_TXFTLR_M						 (0x18 )
#define rSSI_RXFTLR_M						 (0x1C )
#define rSSI_TXFLR_M						 (0x20 )
#define rSSI_RXFLR_M						 (0x24 )
#define rSSI_SR_M						 (0x28 )
#define rSSI_IMR_M						 (0x2C )
#define rSSI_ISR_M						 (0x30 )
#define rSSI_RISR_M						 (0x34 )
#define rSSI_TXOICR_M						 (0x38 )
#define rSSI_RXOICR_M						 (0x3C )
#define rSSI_RXUICR_M						 (0x40 )
#define rSSI_MSTICR_M						 (0x44 )
#define rSSI_ICR_M						 (0x48 )
#define rSSI_DMACR_M						 (0x4C )
#define rSSI_DMATDLR_M					     	 (0x50 )
#define rSSI_DMARDLR_M					     	 (0x54 )
#define rSSI_IDR_M						 (0x58 )
#define rSSI_VERSION_M					     	 (0x5C )
#define rSSI_DR_M						 (0x60 )



#define rSSI_CTLR0_M_1						 (0x0 )
#define rSSI_CTLR1_M_1						 (0x4 )
#define rSSI_ENR_M_1						 (0x8 )
#define rSSI_MWCR_M_1						 (0xC )
#define rSSI_SER_M_1						 (0x10 )
#define rSSI_BAUDR_M_1						 (0x14 )
#define rSSI_TXFTLR_M_1						 (0x18 )
#define rSSI_RXFTLR_M_1						 (0x1C )
#define rSSI_TXFLR_M_1						 (0x20 )
#define rSSI_RXFLR_M_1						 (0x24 )
#define rSSI_SR_M_1						 (0x28 )
#define rSSI_IMR_M_1						 (0x2C )
#define rSSI_ISR_M_1						 (0x30 )
#define rSSI_RISR_M_1						 (0x34 )
#define rSSI_TXOICR_M_1						 (0x38 )
#define rSSI_RXOICR_M_1						 (0x3C )
#define rSSI_RXUICR_M_1						 (0x40 )
#define rSSI_MSTICR_M_1						 (0x44 )
#define rSSI_ICR_M_1						 (0x48 )
#define rSSI_DMACR_M_1						 (0x4C )
#define rSSI_DMATDLR_M_1					 (0x50 )
#define rSSI_DMARDLR_M_1					 (0x54 )
#define rSSI_IDR_M_1						 (0x58 )
#define rSSI_VERSION_M_1					 (0x5C )
#define rSSI_DR_M_1						 (0x60 )



#define rSSI_CTLR0_M_2						 (0x0 )
#define rSSI_CTLR1_M_2						 (0x4 )
#define rSSI_ENR_M_2						 (0x8 )
#define rSSI_MWCR_M_2						 (0xC )
#define rSSI_SER_M_2						 (0x10 )
#define rSSI_BAUDR_M_2						 (0x14 )
#define rSSI_TXFTLR_M_2						 (0x18 )
#define rSSI_RXFTLR_M_2						 (0x1C )
#define rSSI_TXFLR_M_2						 (0x20 )
#define rSSI_RXFLR_M_2						 (0x24 )
#define rSSI_SR_M_2						 (0x28 )
#define rSSI_IMR_M_2						 (0x2C )
#define rSSI_ISR_M_2						 (0x30 )
#define rSSI_RISR_M_2						 (0x34 )
#define rSSI_TXOICR_M_2						 (0x38 )
#define rSSI_RXOICR_M_2						 (0x3C )
#define rSSI_RXUICR_M_2						 (0x40 )
#define rSSI_MSTICR_M_2						 (0x44 )
#define rSSI_ICR_M_2						 (0x48 )
#define rSSI_DMACR_M_2						 (0x4C )
#define rSSI_DMATDLR_M_2					 (0x50 )
#define rSSI_DMARDLR_M_2					 (0x54 )
#define rSSI_IDR_M_2						 (0x58 )
#define rSSI_VERSION_M_2					 (0x5C )
#define rSSI_DR_M_2						 (0x60 )



//========================================================================
// SSI SLAVE 
//========================================================================
#define rSSI_CTLR0_S						 (0x0 )
#define rSSI_CTLR1_S						 (0x4 )
#define rSSI_ENR_S						 (0x8 )
#define rSSI_MWCR_S						 (0xC )
#define rSSI_SER_S						 (0x10 )
#define rSSI_BAUDR_S						 (0x14 )
#define rSSI_TXFTLR_S						 (0x18 )
#define rSSI_RXFTLR_S						 (0x1C )
#define rSSI_TXFLR_S						 (0x20 )
#define rSSI_RXFLR_S						 (0x24 )
#define rSSI_SR_S						 (0x28 )
#define rSSI_IMR_S						 (0x2C )
#define rSSI_ISR_S						 (0x30 )
#define rSSI_RISR_S						 (0x34 )
#define rSSI_TXOICR_S						 (0x38 )
#define rSSI_RXOICR_S						 (0x3C )
#define rSSI_RXUICR_S						 (0x40 )
#define rSSI_MSTICR_S						 (0x44 )
#define rSSI_ICR_S						 (0x48 )
#define rSSI_DMACR_S						 (0x4C )
#define rSSI_DMATDLR_S					 	 (0x50 )
#define rSSI_DMARDLR_S					 	 (0x54 )
#define rSSI_IDR_S						 (0x58 )
#define rSSI_VERSION_S					 	 (0x5C )
#define rSSI_DR_S						 (0x60 )

#endif
