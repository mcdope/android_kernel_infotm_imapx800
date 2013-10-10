/***************************************************************************** 
** drivers/video/infotm/imapfb.c
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of Display Controller.
**
** Author:
**     Sam Ye<weize_ye@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0.1	 Sam@2012/3/20 :  first commit
** 1.0.6	 Sam@2012/6/12 :  add item & pmu support
** 1.0.6 	 Sam@2012/6/13    remove item judge, replace them in ids drv
**
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/decompress/inflate.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/param.h>

#include "../edid.h"
#include <InfotmMedia.h>
#include "imapfb.h"
#include <IM_idsapi.h>
#include <ids_lib.h>
#include <IM_buffallocapi.h>
#include <mach/items.h>
#include <mach/mem-reserve.h>
#include "logo_565.h"
#include "logo_888.h"
#include <asm/cacheflush.h>
#include "isi.h"

//#define IMAPX_800_IDS_TEST
#ifdef IMAPX_800_IDS_TEST
	#define IMAPX_FBLOG(str,args...) printk("kernel sam : "str,##args);
#else 
	#define IMAPX_FBLOG(str,args...)
#endif

#define FB_MEM_ALLOC_FROM_PMM

static imapfb_fimd_info_t imapfb_fimd;
static idslib_fb_info_t gFBInfo;
static dispdev_info_t  gDevInfo;

static alc_buffer_t galcBuffer;
static void 		*pmmHandle;
typedef void *	pmm_handle_t;
IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner);
IM_RET pmmdrv_release(IN pmm_handle_t handle);
IM_RET pmmdrv_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer);
IM_RET pmmdrv_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer);

IM_RET idsdrv_fblayer_init(fblayer_handle_t *handle);
IM_RET idsdrv_fblayer_get_fbinfo(fblayer_handle_t handle,  idslib_fb_info_t *fbinfo);
IM_RET idsdrv_fblayer_get_device(fblayer_handle_t handle, dispdev_info_t *info);
IM_RET idsdrv_fblayer_deinit(fblayer_handle_t handle);
IM_RET idsdrv_fblayer_open(fblayer_handle_t handle);
IM_RET idsdrv_fblayer_close(fblayer_handle_t handle);
IM_RET idsdrv_fblayer_swap_buffer(fblayer_handle_t handle, IM_Buffer *buffer);
IM_RET idsdrv_fblayer_bank_check(IM_Buffer *buf, IM_UINT32 size, IM_INT32 *bEn);

//Imap Framebuffer Stucture Array
imapfb_info_t imapfb_info[IMAPFB_NUM];
/* Set RGB IF Data Line and Control Singal Line */

void imapfb_poweron(void)
{
	idsdrv_fblayer_open(imapfb_info[0].fbhandle);
}
EXPORT_SYMBOL(imapfb_poweron);

void imapfb_shutdown(void)
{
	idsdrv_fblayer_close(imapfb_info[0].fbhandle);
}
EXPORT_SYMBOL(imapfb_shutdown);

int fb_get_addr_and_size(unsigned int *addr, unsigned int *size)
{
	if (imapfb_info[0].map_dma_f1 == 0 || imapfb_info[0].map_size_f1 == 0){
		printk(" framebuffer address not allocated ,check it - \n");
		return -1;
	}
	*addr = imapfb_info[0].map_dma_f1;
	*size = imapfb_info[0].map_size_f1;
	return 0;
}
EXPORT_SYMBOL(fb_get_addr_and_size);

int get_lcd_width(void)
{
    return gFBInfo.width;
}

int get_lcd_height(void)
{
    return gFBInfo.height;
}

static int new_logo = 0;
static int new_logo_exist(unsigned char *p, int wid, int hei)
{
	typedef struct{
		unsigned char scan;
		unsigned char gray;
		unsigned short w;
		unsigned short h;
		unsigned char is565;
		unsigned char rgb;
	}logo_head_t;

    typedef struct {
        uint32_t magic;
        uint32_t type;
        uint32_t flag;
        uint32_t size;
        uint32_t load;
        uint32_t entry;
        uint32_t time;
        uint32_t hcrc;
        uint8_t  dcheck[32];
        uint8_t  name[192];
        uint8_t  extra[256];
    }isi_hdr;

    struct isi_hdr *logo_hdr;
    unsigned char *logo_p;
	logo_head_t *header_p;
    logo_p = p;

	if( NULL == p)
		return 0;

	header_p = (logo_head_t *)(p);
    logo_hdr = (struct isi_hdr *)(logo_p);

    //check isi logo
    if (!isi_check_header(logo_hdr))
    {
        return 1;
    }

    if((wid == header_p->w) && (hei == header_p->h))
    {
        if (header_p->gray == 0x10 && header_p->is565 == 0x1){
            //printk("#current logo formate is RGB565#\n");
            return 565;
        }else if(header_p->gray == 0x20){
            //printk("#current logo formate is RGB888#\n");
            return 888;
        }
    }
    return 0;
}

void error(char *x)
{
    printk("error: %s !!!\n", x);
}

// to get the framebuffer relative paramters form item.
// if no parameters got, use the default values.
static int fb_screen_param_init(void)
{
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	memset((void *)&imapfb_fimd, 0, sizeof(imapfb_fimd_info_t));
    
	// default value
	imapfb_fimd.sync = 0;
	imapfb_fimd.cmap_static = 1;
	imapfb_fimd.osd_xoffset = 0;
	imapfb_fimd.osd_yoffset = 0;
	imapfb_fimd.hsync_len = gDevInfo.hspw;
	imapfb_fimd.vsync_len = gDevInfo.vspw;
	imapfb_fimd.left_margin = gDevInfo.hfpd;
	imapfb_fimd.upper_margin = gDevInfo.vfpd;
	imapfb_fimd.right_margin = gDevInfo.hbpd;
	imapfb_fimd.lower_margin = gDevInfo.vbpd;
	// pixel clock in pico second 
	imapfb_fimd.pixclock = 1000000000 / (gFBInfo.width + gDevInfo.hfpd + gDevInfo.hbpd)
		/ (gFBInfo.height + gDevInfo.vfpd + gDevInfo.vbpd) * 1000000 / gDevInfo.fpsx1000;

	if(gFBInfo.pixfmt == IDSLIB_PIXFMT_16BPP_RGB565){
		imapfb_fimd.bpp = 16;
		imapfb_fimd.bytes_per_pixel = 2;
	}
	else if (gFBInfo.pixfmt == IDSLIB_PIXFMT_32BPP_RGB0888){
		imapfb_fimd.bpp = 32;
		imapfb_fimd.bytes_per_pixel = 4;
	}
	imapfb_fimd.xres = gDevInfo.phyWidth;
	imapfb_fimd.yres = gDevInfo.phyHeight;
	imapfb_fimd.osd_xres = gFBInfo.width;
	imapfb_fimd.osd_yres = gFBInfo.height;
	imapfb_fimd.osd_xres_virtual = gFBInfo.width;
	imapfb_fimd.osd_yres_virtual = gFBInfo.height;

	IMAPX_FBLOG("sam : width=%d, height=%d,pixfmt=%d, phyWidth=%d, phyHeight=%d  ##  \n",gFBInfo.width,gFBInfo.height,gFBInfo.pixfmt,
			gDevInfo.phyWidth,gDevInfo.phyHeight);
	IMAPX_FBLOG("sam : hfpd=%d,hbpd=%d,vfpd=%d,vbpd=%d, pixclock=%d- \n",gDevInfo.hfpd,gDevInfo.hbpd,gDevInfo.vfpd,gDevInfo.vbpd,
			imapfb_fimd.pixclock);
	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_map_video_memory(imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**		1. Allocate framebuffer memory for input framebuffer window.
**		2. Save physical address, virtual address and size of framebuffer memory.
**		3. If input framebuffer window is win0 and kernel logo is configured, then show this logo.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Return
**    -ENOMEM	: Failure
**	0			: Success
**
*****************************************************************************/
static int imapfb_map_video_memory(imapfb_info_t *fbi)
{
	IM_RET ret;
	IM_UINT32 flag, bankEn;
	IM_Buffer buff;
	IMAPX_FBLOG(" imapfb_map_video_memory size=%d -- \n",fbi->fb.fix.smem_len);
	//Allocate framebuffer memory and save physical address, virtual address and size
	fbi->map_size_f1 = PAGE_ALIGN(fbi->fb.fix.smem_len);

#ifdef FB_MEM_ALLOC_FROM_PMM
	memset((void *)&buff, 0, sizeof (IM_Buffer));
	bankEn = 0;
	idsdrv_fblayer_bank_check(&buff, fbi->map_size_f1, &bankEn);
	if (bankEn == 0)
	{
		flag = ALC_FLAG_PHY_MUST | ALC_FLAG_DEVADDR;
		ret = pmmdrv_mm_alloc(pmmHandle, fbi->map_size_f1, flag, &galcBuffer);
		if (ret == IM_RET_FAILED){
			printk("pmmdrv_mm_alloc failed -- \n");
			return -ENOMEM;
		}
		fbi->map_dma_f1 = galcBuffer.buffer.phy_addr;
		fbi->map_cpu_f1 = galcBuffer.buffer.vir_addr;

		IMAPX_FBLOG(" pmm buffer phy_addr=0x%x , vir_addr=0x%x, size=%d, devAddr=0x%x - ",galcBuffer.buffer.phy_addr,(IM_UINT32)galcBuffer.buffer.vir_addr,galcBuffer.buffer.size,galcBuffer.devAddr);
	}
	else{
		fbi->map_dma_f1 = buff.phy_addr;
		fbi->map_cpu_f1 = buff.vir_addr;
		IMAPX_FBLOG("sam : size=%d, fbi->map_dma_f1=0x%x, map_cpu_f1=0x%x - fb_num=%d \n",fbi->map_size_f1, fbi->map_dma_f1,fbi->map_cpu_f1,CONFIG_FB_IMAP_BUFFER_NUM);
	}
#else
	fbi->map_cpu_f1 = dma_alloc_writecombine(fbi->dev, fbi->map_size_f1, &fbi->map_dma_f1, GFP_KERNEL);
#endif
	fbi->map_size_f1 = fbi->fb.fix.smem_len;

	//If succeed in allocating framebuffer memory, then init the memory with some color or kernel logo 
	if (fbi->map_cpu_f1)
	{
		memset(fbi->map_cpu_f1, 0x0, fbi->map_size_f1);

		//Set physical and virtual address for future use
		fbi->fb.screen_base = fbi->map_cpu_f1;
		fbi->fb.fix.smem_start = fbi->map_dma_f1;
	}
	else
	{
		printk(KERN_ERR "[imapfb_map_video_memory]: win%d fail to allocate framebuffer memory\n", fbi->win_id);
		return -ENOMEM;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_unmap_video_memory(imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**		1. Free framebuffer memory for input framebuffer window.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_unmap_video_memory(imapfb_info_t *fbi)
{
	IM_UINT32 bankEn;
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
#ifdef FB_MEM_ALLOC_FROM_PMM
	idsdrv_fblayer_bank_check(NULL, fbi->map_size_f1, &bankEn);
	if (bankEn == 0)
		pmmdrv_mm_free(pmmHandle, &galcBuffer);
#else
	dma_free_writecombine(fbi->dev, fbi->map_size_f1, fbi->map_cpu_f1,  fbi->map_dma_f1);
#endif
}

/*****************************************************************************
** -Function:
**    imapfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**		1. Check input video params of 'var'. If a value doesn't fit, round it up. If it's too big,
**		    return -EINVAL.
**
** -Input Param
**    *var	Variable Screen Info. Structure Pointer
**    *info	Framebuffer Structure Pointer
**
** -Output Param
**    *var	Variable Screen Info. Structure Pointer
**
** -Return
**    none
**
*****************************************************************************/ 
static int imapfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	IMAPX_FBLOG(" %d  %s - bits_per_pixel = %d - \n",__LINE__,__func__,var->bits_per_pixel);
	if (!var || !info)
	{
		printk(KERN_ERR "[imapfb_check_var]: input argument null\n");
		return -EINVAL;
	}
	
	switch (var->bits_per_pixel)
	{			
		case 8:
			var->red = imapfb_a1rgb232_8.red;
			var->green = imapfb_a1rgb232_8.green;
			var->blue = imapfb_a1rgb232_8.blue;
			var->transp = imapfb_a1rgb232_8.transp;
			imapfb_fimd.bpp = 8;
			imapfb_fimd.bytes_per_pixel = 1;
			break;

		case 16:
			var->red = imapfb_rgb565_16.red;
			var->green = imapfb_rgb565_16.green;
			var->blue = imapfb_rgb565_16.blue;
			var->transp = imapfb_rgb565_16.transp;
			imapfb_fimd.bpp = 16;
			imapfb_fimd.bytes_per_pixel = 2;
			break;
		
		case 18:
			var->red = imapfb_rgb666_18.red;
			var->green = imapfb_rgb666_18.green;
			var->blue = imapfb_rgb666_18.blue;
			var->transp = imapfb_rgb666_18.transp;
			imapfb_fimd.bpp = 18;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 19:
			var->red = imapfb_a1rgb666_19.red;
			var->green = imapfb_a1rgb666_19.green;
			var->blue = imapfb_a1rgb666_19.blue;
			var->transp = imapfb_a1rgb666_19.transp;
			imapfb_fimd.bpp = 19;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 24:
			var->red = imapfb_rgb888_24.red;
			var->green = imapfb_rgb888_24.green;
			var->blue = imapfb_rgb888_24.blue;
			var->transp = imapfb_rgb888_24.transp;
			imapfb_fimd.bpp = 24;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 25:
			var->red = imapfb_a1rgb888_25.red;
			var->green = imapfb_a1rgb888_25.green;
			var->blue = imapfb_a1rgb888_25.blue;
			var->transp = imapfb_a1rgb888_25.transp;
			imapfb_fimd.bpp = 25;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 28:
			var->red = imapfb_a4rgb888_28.red;
			var->green = imapfb_a4rgb888_28.green;
			var->blue = imapfb_a4rgb888_28.blue;
			var->transp = imapfb_a4rgb888_28.transp;
			imapfb_fimd.bpp = 28;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 32:
			var->red = imapfb_a8rgb888_32.red;
			var->green = imapfb_a8rgb888_32.green;
			var->blue = imapfb_a8rgb888_32.blue;
			var->transp = imapfb_a8rgb888_32.transp;
			imapfb_fimd.bpp = 32;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		default:
			printk(KERN_ERR "[imapfb_check_var]: input bits_per_pixel of var invalid\n");
			return -EINVAL;
	}

	var->width = gDevInfo.phyWidth;
	var->height = gDevInfo.phyHeight;

	info->fix.line_length = imapfb_fimd.bytes_per_pixel * info->var.xres;
		
	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_par(struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**		1. According to input framebuffer struct, set new var struct value and set these new
**		    values to relevant registers.
**
** -Input Param
**    *info	Framebuffer Structure Pointer
**
** -Output Param
**	*info	Framebuffer Structure Pointer
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/ 
static int imapfb_set_par(struct fb_info *info)
{	
	struct fb_var_screeninfo *var = NULL;
	imapfb_info_t *fbi = (imapfb_info_t*) info;
	int ret = 0;
	IMAPX_FBLOG(" %d  %s  --- \n",__LINE__,__func__);

	if (!info)
	{
		printk(KERN_ERR "[imapfb_set_par]: input argument null\n");
		return -EINVAL;
	}
	
	var = &info->var;

	//Set Visual Color Type
	if (var->bits_per_pixel == 8 || var->bits_per_pixel == 16 || var->bits_per_pixel == 18 || var->bits_per_pixel == 19
		|| var->bits_per_pixel == 24 || var->bits_per_pixel == 25 || var->bits_per_pixel == 28 || var->bits_per_pixel == 32)
		fbi->fb.fix.visual = FB_VISUAL_TRUECOLOR;
	else if (var->bits_per_pixel == 1 || var->bits_per_pixel == 2 || var->bits_per_pixel == 4)
		fbi->fb.fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
	{
		printk(KERN_ERR "[imapfb_set_par]: input bits_per_pixel invalid\n");
		ret = -EINVAL;
		goto out;
	}

	//Check Input Params
	ret = imapfb_check_var(var, info);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_set_par]: fail to check var\n");
		ret = -EINVAL;
		goto out;
	}	

out:
	return ret;
}

/*****************************************************************************
** -Function:
**    imapfb_blank(int blank_mode, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. According to input blank mode, Lcd display blank mode in screen.
**		a. No Blank
**		b. Vsync Suspend
**		c. Hsync Suspend
**		d. Power Down
**
** -Input Param
**	blank_mode	Blank Mode
**	*info		Framebuffer Structure
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_blank(int blank_mode, struct fb_info *info)
{
	imapfb_info_t *fbi = (imapfb_info_t *)info;

	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	switch (blank_mode)
	{
		// fblayer open , But backlight ? 
		case FB_BLANK_UNBLANK :
			idsdrv_fblayer_open(fbi->fbhandle);
			break;

		/* Lcd on, Backlight off */
		case FB_BLANK_VSYNC_SUSPEND :
		case FB_BLANK_HSYNC_SUSPEND :
			break;

		/* Lcd and Backlight off */
		case FB_BLANK_POWERDOWN   :
			idsdrv_fblayer_close(fbi->fbhandle);
			break;

		default:
			printk(KERN_ERR "[imapfb_blank]: input blank mode %d invalid\n", blank_mode);
			return -EINVAL;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_fb_addr(const imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**        1. According to virtual offset in both direction, set new start address of input
**		window in framebuffer ram which equals framebuffer start address plus offset.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_set_fb_addr(const imapfb_info_t *fbi)
{
	UINT32 video_phy_temp_f1 = fbi->map_dma_f1;	/* framebuffer start address */
	UINT32 start_address;						/* new start address */
	UINT32 start_offset;									/* virtual offset */
	IM_Buffer buffer;
	static int num = 0;
	fblayer_handle_t handle = fbi->fbhandle;

	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	//printk("sam : imapfb yoffset = 0x%x - \n",fbi->fb.var.yoffset);
	if (num < 10){
		num ++;
		return ;
	}
	
	//Get Start Address Offset
	//start_offset = (fbi->fb.var.xres_virtual * fbi->fb.var.yoffset + fbi->fb.var.xoffset) * imapfb_fimd.bytes_per_pixel;
	if (fbi->fb.var.yoffset < 0x4000000){
		// here means this ioctl is from recovery/minui
		// yoffset means the offset from physical base address
		start_offset = (fbi->fb.var.xres_virtual * fbi->fb.var.yoffset + fbi->fb.var.xoffset) * imapfb_fimd.bytes_per_pixel;

		//New Start Address with Offset
		start_address = video_phy_temp_f1 + start_offset;
		IMAPX_FBLOG(" %d  %s - map_dma_f1=0x%x ,start_offset=0x%x - \n",__LINE__,__func__,video_phy_temp_f1,start_offset);
		IMAPX_FBLOG(" (%d) -- xres_virtual=%d, yoffset=%d , xoffset=%d -- \n",__LINE__,fbi->fb.var.xres_virtual, fbi->fb.var.yoffset,fbi->fb.var.xoffset);
		buffer.vir_addr = (void *)((UINT32)fbi->map_cpu_f1 + start_offset);
		buffer.phy_addr = start_address;
		buffer.size = fbi->fb.fix.smem_len / CONFIG_FB_IMAP_BUFFER_NUM;
		buffer.flag = IM_BUFFER_FLAG_PHY;
	}
	else{
		// here means this ioctl is from gralloc 
		//     // yoffset means the real physical address
		start_offset = 0;
		start_address = fbi->fb.var.yoffset;
		buffer.vir_addr = 0;
		buffer.phy_addr = start_address;
		buffer.size = fbi->fb.fix.smem_len / CONFIG_FB_IMAP_BUFFER_NUM;
		buffer.flag = IM_BUFFER_FLAG_PHY;
	}

	idsdrv_fblayer_swap_buffer(handle, &buffer);
	
}

/*****************************************************************************
** -Function:
**    imapfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. Pan the display using 'xoffset' and 'yoffset' fields of input var structure.
**
** -Input Param
**	*var		Variable Screen Parameter Structure
**	*info	Framebuffer Structure
**
** -Output Param
**    *info	Framebuffer Structure
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	imapfb_info_t *fbi = (imapfb_info_t*)info;
	int ret;

	IMAPX_FBLOG(" %d  %s - var->xoffset=%d, var->yoffset=%d - \n",__LINE__,__func__,var->xoffset,var->yoffset);
	if (!var || !info)
	{
		printk(KERN_ERR "[imapfb_pan_display]: input arguments null\n");
		return -EINVAL;
	}

	if (var->xoffset + info->var.xres > info->var.xres_virtual)
	{
		printk(KERN_ERR "[imapfb_pan_display]: pan display out of range in horizontal direction\n");
		return -EINVAL;
	}

	if (var->yoffset + info->var.yres > info->var.yres_virtual && var->yoffset < 0x80000000)
	{
		printk(KERN_ERR "[imapfb_pan_display]: pan display out of range in vertical direction\n");
		return -EINVAL;
	}

	fbi->fb.var.xoffset = var->xoffset;
	fbi->fb.var.yoffset = var->yoffset;

	//Check Input Params
	ret = imapfb_check_var(&fbi->fb.var, info);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_pan_display]: fail to check var\n");
		return -EINVAL;
	}

	imapfb_set_fb_addr(fbi);

	return 0;
}

static inline UINT32 imapfb_chan_to_field(UINT32 chan, struct fb_bitfield bf)
{
#if defined (CONFIG_FB_IMAP_BPP16)
	chan &= 0xffff;
	chan >>= 16 - bf.length;
#elif defined (CONFIG_FB_IMAP_BPP32)
	chan &= 0xffffffff;
	chan >>= 32 - bf.length;
#endif

	return chan << bf.offset;
}

/*****************************************************************************
** -Function:
**    imapfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
**		unsigned int blue, unsigned int transp, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. Set color register
**		a. Fake palette for true color: for some special use
**		b. Real palette for paletter color: need to modify registers
**
** -Input Param
**	regno	Number of Color Register
**	red		Red Part of input Color
**	green	Green Part of input Color
**	blue		Blue Part of input Color
**	transp	Transparency Part of input Color
**	*info	Framebuffer Structure
**
** -Output Param
**    *info	Framebuffer Structure
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
	unsigned int blue, unsigned int transp, struct fb_info *info)
{
//	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	imapfb_info_t *fbi = (imapfb_info_t*)info;
	UINT32 val;

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_setcolreg]: input info null\n");
		return -EINVAL;
	}

	switch (fbi->fb.fix.visual)
	{
		/* Modify Fake Palette of 16 Colors */ 
		case FB_VISUAL_TRUECOLOR:
			//if (regno < 16)
			if (regno < 256)	/* Modify Fake Palette of 256 Colors */ 
			{				
				unsigned int *pal = fbi->fb.pseudo_palette;

				val = imapfb_chan_to_field(red, fbi->fb.var.red);
				val |= imapfb_chan_to_field(green, fbi->fb.var.green);
				val |= imapfb_chan_to_field(blue, fbi->fb.var.blue);
				val |= imapfb_chan_to_field(transp, fbi->fb.var.transp);			

				pal[regno] = val;
			}
			else
			{
				printk(KERN_ERR "[imapfb_setcolreg]: input register number %d invalid\n", regno);
				return -EINVAL;
			}
			break;

		case FB_VISUAL_PSEUDOCOLOR:
		default:
			printk(KERN_ERR "[imapfb_setcolreg]: unknown color type\n");
			return -EINVAL;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
**
** -Description:
**    This function implement special features. The process is,
**        1. According input command code, perform some operations
**		a. Set the position and size of some window
**		b. Open/close some window
**		c. Set value of alpha0/1 of some window
**		d. Start/stop color key function of some window
**		e. Start/stop alpha blending in color key function of some window
**		f. Set color key info. of some window
**		g. Power up/down lcd screen
**		h. Get edid info. of lcd screen
**		i. Detect lcd connection
**		j. Get framebuffer physical address info. of some window
**
** -Input Param
**    *info	Framebuffer Structure
**    cmd		Command Code
**    arg		Command Argument
**
** -Output Param
**    *info	Framebuffer Structure
**    arg		Command Argument
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	imapfb_info_t *fbi = (imapfb_info_t *)info;
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_ioctl]: input info null\n");
		return -EINVAL;
	}
		
	switch(cmd)
	{
		case IMAPFB_SWITCH_ON:
			idsdrv_fblayer_open(fbi->fbhandle);
			break;

		case IMAPFB_SWITCH_OFF:
			idsdrv_fblayer_close(fbi->fbhandle);
			break;

#ifdef FB_MEM_ALLOC_FROM_PMM
		case IMAPFB_GET_PMM_DEVADDR:
			__put_user(galcBuffer.devAddr, (unsigned int*)arg);
			break;
#endif
		case IMAPFB_GET_PHY_ADDR:
			    __put_user(imapfb_info[0].map_dma_f1, (unsigned int*)arg);
			    break;
		default:
			printk(KERN_ERR "[imapfb_ioctl]: unknown command type\n");
			return -EFAULT;
	}

	return 0;
}

/* Framebuffer operations structure */
struct fb_ops imapfb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var		= imapfb_check_var,
	.fb_set_par		= imapfb_set_par, // may called after imapfb_init_registers
	.fb_blank 		= imapfb_blank,
	.fb_pan_display	= imapfb_pan_display,
	.fb_setcolreg		= imapfb_setcolreg,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
	.fb_ioctl			= imapfb_ioctl,
};

static nologo = 0;
void logo_disable(void) {
	nologo = 1;
}

/*****************************************************************************
** -Function:
**    imapfb_init_fbinfo(imapfb_info_t *finfo, char *drv_name, int index)
**
** -Description:
**    This function implement special features. The process is,
**		1. Init framebuffer struct of input imap framebuffer struct, including fix and var struct.
**		2. If input index is 0, then init hardware setting.
**
** -Input Param
**    *fbi        		Imap Framebuffer Structure Pointer
**	drv_name	Driver Name
**	index		Window ID
**
** -Output Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Return
**	none
**
*****************************************************************************/
static void imapfb_init_fbinfo(imapfb_info_t *finfo, UINT8 *drv_name, UINT32 index)
{
	int i = 0;

	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	if (!finfo)
	{
		printk(KERN_ERR "[imapfb_init_fbinfo]: input finfo null\n");
		return;
	}

	if (index >= IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_init_fbinfo]: input index invalid\n");
		return;
	}

	strcpy(finfo->fb.fix.id, drv_name);

	finfo->win_id = index;
	finfo->fb.fix.type = FB_TYPE_PACKED_PIXELS;
	finfo->fb.fix.type_aux = 0;
	finfo->fb.fix.xpanstep = 0;
	finfo->fb.fix.ypanstep = 1;
	finfo->fb.fix.ywrapstep = 0;
	finfo->fb.fix.accel = FB_ACCEL_NONE;
	finfo->fb.fix.mmio_start = IMAP_IDS_BASE;
	finfo->fb.fix.mmio_len = SZ_16K;

	finfo->fb.fbops = &imapfb_ops;
	finfo->fb.flags = FBINFO_FLAG_DEFAULT;

	finfo->fb.pseudo_palette = &finfo->pseudo_pal;

	finfo->fb.var.nonstd = 0;
	finfo->fb.var.activate = FB_ACTIVATE_NOW;
	finfo->fb.var.accel_flags = 0;
	finfo->fb.var.vmode = FB_VMODE_NONINTERLACED;

	finfo->fb.var.xres = imapfb_fimd.osd_xres;
	finfo->fb.var.yres = imapfb_fimd.osd_yres;
	finfo->fb.var.xres_virtual = imapfb_fimd.osd_xres_virtual;
	finfo->fb.var.yres_virtual = imapfb_fimd.osd_yres_virtual;
	finfo->fb.var.xoffset = imapfb_fimd.osd_xoffset;
	finfo->fb.var.yoffset = imapfb_fimd.osd_yoffset;

	finfo->fb.var.width = imapfb_fimd.xres;
	finfo->fb.var.height = imapfb_fimd.yres;
	
	finfo->fb.var.bits_per_pixel = imapfb_fimd.bpp;
	finfo->fb.var.pixclock = imapfb_fimd.pixclock;
	finfo->fb.var.hsync_len = imapfb_fimd.hsync_len;
	finfo->fb.var.left_margin = imapfb_fimd.left_margin;
	finfo->fb.var.right_margin = imapfb_fimd.right_margin;
	finfo->fb.var.vsync_len = imapfb_fimd.vsync_len;
	finfo->fb.var.upper_margin = imapfb_fimd.upper_margin;
	finfo->fb.var.lower_margin = imapfb_fimd.lower_margin;
	finfo->fb.var.sync = imapfb_fimd.sync;
	finfo->fb.var.grayscale = imapfb_fimd.cmap_grayscale;

	finfo->fb.fix.smem_len = CONFIG_FB_IMAP_BUFFER_NUM * finfo->fb.var.xres_virtual * finfo->fb.var.yres_virtual * imapfb_fimd.bytes_per_pixel;

	finfo->fb.fix.line_length = finfo->fb.var.xres * imapfb_fimd.bytes_per_pixel;

	for (i = 0; i < 256; i++)
		finfo->palette_buffer[i] = IMAPFB_PALETTE_BUFF_CLEAR;
}

static int __init imapfb_probe(struct platform_device *pdev)
{
	struct fb_info *fbinfo;
	imapfb_info_t *info;
    UINT8 driver_name[] = "imapfb";
	UINT32 index;
	int ret,cir, pad_width,pad_height;
	fblayer_handle_t handle ;
	UINT32 start_offset;									/* virtual offset */
	IM_Buffer buffer;
	unsigned char *logo_data = NULL; 
    void * hex_from_bin;
    int mFormat;
      
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	printk(KERN_INFO "Imap Framebuffer Driver Initialization Start!\n");
	
	//Check Input Argument
	if (!pdev)
	{
		printk(KERN_ERR "[imapfb_probe]: input argument null\n");
		return -EINVAL;
	}

	//Allocate one Framebuffer Structure
	fbinfo = framebuffer_alloc(sizeof(imapfb_info_t), &pdev->dev);
	if (!fbinfo)
	{
		printk(KERN_ERR "[imapfb_probe]: fail to allocate framebuffer\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, fbinfo);
	info = fbinfo->par;
	info->dev = &pdev->dev;

	for (index = 0; index < 1 /*IMAPFB_NUM*/; index++)
	{
		memset((void *)&galcBuffer, 0 , sizeof(alc_buffer_t));
		memset((void *)&gDevInfo, 0, sizeof (dispdev_info_t));
		memset((void *)&gDevInfo, 0, sizeof (dispdev_info_t));
		handle = NULL;
		ret = idsdrv_fblayer_init(&handle);
		if(ret < 0){
			printk(KERN_ERR "[imap_probe] : idsdrv_fblayer_init failed  --\n");
			goto dealloc_fb;
		}
		imapfb_info[index].fbhandle = (void *)handle;

		idsdrv_fblayer_get_fbinfo(handle, &gFBInfo);
		idsdrv_fblayer_get_device(handle, &gDevInfo);
		
		info->fbhandle = (void *)handle;

		fb_screen_param_init();

		ret = pmmdrv_open(&pmmHandle, "imapfb");
		if(ret != IM_RET_OK){
			printk(" imapfb pmmdrv_open failed ");
			goto deinit_fblayer;
		}

		imapfb_info[index].clk1 = info->clk1;
		imapfb_info[index].clk2 = info->clk2;

		imapfb_init_fbinfo(&imapfb_info[index], driver_name, index);

		/* Initialize video memory */
		ret = imapfb_map_video_memory(&imapfb_info[index]);
		if (ret)
		{
			printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate framebuffer video ram\n", index);
			ret = -ENOMEM;
			//goto release_clock2;
			goto release_pmm;
		}

		ret = imapfb_check_var(&imapfb_info[index].fb.var, &imapfb_info[index].fb);
		if (ret)
		{
			printk(KERN_ERR "[imapfb_probe]: win %d fail to check var\n", index);
			ret = -EINVAL;
			goto free_video_memory;
		}
		
		if (index < 2)
		{
			if (fb_alloc_cmap(&imapfb_info[index].fb.cmap, 256, 0) < 0)
			{
				printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate color map\n", index);
				goto free_video_memory;
			}
		}
		else
		{
			if (fb_alloc_cmap(&imapfb_info[index].fb.cmap, 16, 0) < 0)
			{
				printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate color map\n", index);
				goto free_video_memory;
			}
		}
		
		printk(KERN_ALERT "Before register frame buffer.\n");
		ret = register_framebuffer(&imapfb_info[index].fb);
		printk(KERN_ALERT "After register frame buffer.\n");
		if (ret < 0)
		{
			printk(KERN_ERR "[imapfb_probe]: failed to register framebuffer device %d\n", ret);
			goto dealloc_cmap;
		}

		printk(KERN_INFO "fb%d: %s frame buffer device\n", imapfb_info[index].fb.node, imapfb_info[index].fb.fix.id);
	}

	// power-up logo configuration 
#if 1
    printk("power-up logo \n");

    hex_from_bin =  phys_to_virt(RESERVED_BASE_LOGO);
    if(!hex_from_bin) printk("Error: phys_to_virt  hex_from_bin\n");

    start_offset = imapfb_info[0].map_size_f1 / CONFIG_FB_IMAP_BUFFER_NUM;
    buffer.vir_addr = (void *)((UINT32)imapfb_info[0].map_cpu_f1 + start_offset);
    buffer.phy_addr = (unsigned int)imapfb_info[0].map_dma_f1 + start_offset;
    buffer.size = imapfb_info[0].map_size_f1 / CONFIG_FB_IMAP_BUFFER_NUM;
    // printk("buffer.sizee = %d\n", buffer.size);
    buffer.flag = IM_BUFFER_FLAG_PHY;
    /*
     *   check logo valid ?
     */
    pad_width = imapfb_fimd.osd_xres;
    pad_height = imapfb_fimd.osd_yres;

    if(nologo)
	    goto no_logo;

    new_logo = new_logo_exist(hex_from_bin, pad_width, pad_height);

    // printk(KERN_INFO "imapfb_probe:fpp= %d,x=%d,y=%d,new_logo = %d\n",imapfb_fimd.bytes_per_pixel,
    //     imapfb_info[0].fb.var.xres_virtual, imapfb_info[0].fb.var.yres_virtual,new_logo);
    mFormat = item_integer("ids.fb.pixfmt",0);
    if (new_logo == mFormat){
        logo_data = hex_from_bin ; //logo new you add 
    }else if (new_logo == 1) { //isi logo
        logo_data = NULL;
        gunzip(hex_from_bin+512, buffer.size, NULL, NULL, buffer.vir_addr, 0, error);
    }else{
        logo_data = NULL; 
        cir  =0;
       
        if(mFormat == 565){
            printk("#565 def logo\n");
            hex_from_bin = (void *)&gImage_logo_565[0];
            for(;cir<105; cir++)
                memcpy(buffer.vir_addr + cir*pad_width*2 , hex_from_bin + cir*180, 180);
        }
        else if (mFormat == 888){
            printk("#888 def logo\n");
            hex_from_bin = (void *)&gImage_logo_888[0];
            for(; cir<105;cir++)
                memcpy(buffer.vir_addr + cir*pad_width*4 , hex_from_bin + cir*360 ,360);
        }else{
            printk("unkown pixel_format \n");
        }
    }

no_logo:
    if(nologo)
	    memset(buffer.vir_addr, 0, buffer.size);
    else if (logo_data){
        memcpy(buffer.vir_addr, logo_data+8, buffer.size);
    }

    dmac_flush_range(buffer.vir_addr, (void*)((unsigned int)buffer.vir_addr + buffer.size));
    outer_flush_range(buffer.phy_addr, buffer.phy_addr + buffer.size);

	idsdrv_fblayer_swap_buffer(handle, &buffer);
    if(hex_from_bin) 
    {
        hex_from_bin = NULL;
    } 
    printk("end of logo\n");
#endif

	idsdrv_fblayer_open(imapfb_info[0].fbhandle);

	/* ######################################################################### */

	printk(KERN_INFO "Imap Framebuffer Driver Initialization OK!\n");

	return 0;

dealloc_cmap:
	fb_dealloc_cmap(&imapfb_info[index].fb.cmap);

free_video_memory:
	imapfb_unmap_video_memory(&imapfb_info[index]);

release_pmm:
	pmmdrv_release(pmmHandle);

deinit_fblayer:
	idsdrv_fblayer_deinit(imapfb_info[0].fbhandle);

dealloc_fb:
	framebuffer_release(fbinfo);
	return ret;
}

static int imapfb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);
	imapfb_info_t *info = fbinfo->par;
	UINT32 index = 0;
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);

	pmmdrv_release(pmmHandle);
	idsdrv_fblayer_deinit(info->fbhandle); 

	msleep(1);

	//Free Framebuffer Memory and Unregister Framebuffer Device
	for (index = 0; index < IMAPFB_NUM; index++)
	{
		imapfb_unmap_video_memory((imapfb_info_t*)&imapfb_info[index]);
		unregister_framebuffer(&imapfb_info[index].fb);
	}

	//Release Framebuffer Structure
	framebuffer_release(fbinfo);	

	return 0;
}

int imapfb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	imapfb_info_t *info = fbinfo->par;
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);

    idsdrv_fblayer_close(info->fbhandle);

	return 0;
}

int imapfb_resume(struct platform_device *dev)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	imapfb_info_t *info = fbinfo->par;
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);

    idsdrv_fblayer_open(info->fbhandle);
	return 0;
}

static struct platform_driver imapfb_driver = {
	.probe		= imapfb_probe,
	.remove		= imapfb_remove,
//	.suspend		= imapfb_suspend,
//	.resume		= imapfb_resume,
	.driver		= {
		.name	= "imap-fb",
		.owner	= THIS_MODULE,
	},
};

extern void imap_powerkey_antishake(void);
int __devinit imapfb_init(void)
{
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	imap_powerkey_antishake();
	return platform_driver_register(&imapfb_driver);
}
static void __exit imapfb_cleanup(void)
{
	IMAPX_FBLOG(" %d  %s -- \n",__LINE__,__func__);
	platform_driver_unregister(&imapfb_driver);
}

module_init(imapfb_init);
module_exit(imapfb_cleanup);

MODULE_AUTHOR("Sam Ye");
MODULE_DESCRIPTION("IMAP Framebuffer Driver");
MODULE_LICENSE("GPL");


