#ifndef __IMAPX200_IDS_H__
#define __IMAPX200_IDS_H__

#include "hdmi.h"

typedef struct{
/*LCDCON1*/
	uint32_t VCLK;
	uint32_t EACH_FRAME;
	uint32_t LCD_PANNEL;
	uint32_t BPP_MODE;
	uint32_t LCD_OUTPUT;

/*LCDCON2*/
	uint32_t VBPD;
	uint32_t VACTIVE;
	uint32_t VFPD;
	uint32_t VSPW;

/*LCDCON3*/	
	uint32_t VACTIVE_HIGHBIT;
	uint32_t HBPD;
	uint32_t HACTIVE;
	uint32_t HFPD;

/*LCDCON4*/	
	uint32_t HSPW;

/*LCDCON5*/
	uint32_t COLOR_MODE;
	uint32_t BPP24BL;
	uint32_t FRM565;
	uint32_t INVVCLK;
	uint32_t INVVLINE;
	uint32_t INVVFRAME;
	uint32_t INVVD;
	uint32_t INVVDEN;
	uint32_t INVPWREN;
	uint32_t INVENDLINE;
	uint32_t PWREN;
	uint32_t ENLEND;
	uint32_t BSWP;
	uint32_t HWSWP;
}lcd_timing_param_t;

typedef struct{
/*IMAP_TVCCR*/
	uint32_t Clock_enable;
	uint32_t TV_PCLK_mode;
	uint32_t Inv_clock;
	uint32_t clock_sel;
       	uint32_t Clock_div;

/*TVICR*/
	uint32_t tvif_enable;
	uint32_t ITU601_656n;
	uint32_t Bit16ofITU60;
	uint32_t Direct_data;
	uint32_t Bitswap;
	uint32_t Data_order;
	uint32_t Inv_vsync;
	uint32_t Inv_hsync;
	uint32_t Inv_href;
	uint32_t Inv_field;
	uint32_t Begin_with_EAV;

/*TVCMCR*/
	uint32_t Matrix_mode;
	uint32_t Passby;
	uint32_t Inv_MSB_in;
	uint32_t Inv_MSB_out;
	uint32_t Matrix_oft_b;
	uint32_t Matrix_oft_a;

/*TVUBA1
 *
 *VBPD+VSPW
 *
 */
	uint32_t UBA1_LEN;

/*TVUNBA
 *
 *vACTIVE
 *
 */	
	uint32_t UNBA_LEN;

/*TVUBA2
 *
 *VFPD
 *
 */
	uint32_t UNBA2_LEN;

/*TVLBA1
 *
 *VBPD+VSPW
 *
 */
	uint32_t LBA1_LEN;

/*TVLNBA
 *
 *VACTIVE
 *
 */
	uint32_t LNBA_LEN;

/*TVLBA2
 *
 *VFPD
 *
 */
	uint32_t LBA2_LEN;

/*TVBLEN
 *
 *HFPD+HSPW+HBPD-8
 *
 */
	uint32_t BLANK_LEN;

/*TVVLEN
 *
 *HACTIVE
 *
 */	
	uint32_t VIDEO_LEN;

/*TVHSCR
 *
 *0
 *HFPD
 *HSPW
 *
 */
	uint32_t Hsync_VB1_ctrl;
	uint32_t Hsync_delay;
	uint32_t Hsync_extend;

/*TVVSHCR
 *
 *HFPD
 *VSPW*HTOTAL-1
 *
 */
	uint32_t Vsync_delay_upper;
	uint32_t Vsync_extend_upper;

/*TVVSLCR
 *
 *HFPD
 *VSPW*HTOTAL-1
 */
	uint32_t Vsync_delay_lower;
	uint32_t Vsync_extend_lower;

/*TVXSIZE
 *
 *HACTIVE-1
 *
 */
	uint32_t DISP_XSIZE;

/*TVYSIZE
 *
 *VACTIVE-1
 *
 */
	uint32_t DISP_YSIZE;
	
}tvif_timing_param_t;

typedef struct{
/*DPLLCFG*/
	uint32_t DPLLCFG;

/*DIVCFG4*/	
	uint32_t DIVCFG4;
}lds_clk_param_t;

void lcd_change_timing(video_timing timing, uint8_t tv_IF, uint8_t open_tvif);
void osd_excursion_bottom(video_timing timing);
void osd_excursion(video_timing timing);
void detect_overflow(void);

#endif	/*__IMAPX200_IDS_H__*/
