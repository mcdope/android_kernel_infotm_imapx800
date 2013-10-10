/*------------------------------------------------------------------------------
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 		  --
-- 				   															  --
-- 	This program is free software; you can redistribute it and/or modify	  --
-- 	it under the terms of the GNU General Public License as published by	  --
-- 	the Free Software Foundation; either version 2 of the License, or		  --
-- 	(at your option) any later version.							              --
--------------------------------------------------------------------------------
--	RCSfile: isp_osd.c
--
--  Description :
--		osd module.
--
--	Author:
--     Arsor Fu   <arsor.fu@infotmic.com.cn>
--
--------------------------------------------------------------------------------
-- Revision History: 
-------------------- 
-- v1.0.1	arsor@2012/04/18: first commit.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include "isp_pwl.h"
#include "isp_reg_drv.h"
#include "isp_lib.h"
#include "isp_osd.h"

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"OSD_I:"
#define WARNHEAD	"OSD_W:"
#define ERRHEAD		"OSD_E:"
#define TIPHEAD		"OSD_T:"

//osd wndow0
/*------------------------------------------------------------------------------

    Function name: osd_wnd0_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_init(isp_osd_context_t *osd, osd_wnd0_context_t *wnd0)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(wnd0 != IM_NULL);

	isppwl_memset((void *)&osd->wnd0, 0, sizeof(osd_wnd0_context_t));
	
	IM_JIF(osd_wnd0_set_mapcolor(osd,&wnd0->mapclr));
	IM_JIF(osd_wnd0_set_coordinate(osd, &wnd0->coordinate));

	if(wnd0->enable)
	{
		//set window0 enable
		osd_wnd0_set_enable( osd);
	}
	else
	{
		//set window0 disable
		osd_wnd0_set_disable( osd);
	}

	isppwl_memcpy((void *)&osd->wnd0, (void *)wnd0, sizeof(osd_wnd0_context_t));

	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd0_deinit

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_deinit(isp_osd_context_t *osd)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(osd != IM_NULL);
	
	if(osd->wnd0.enable == IM_TRUE)
	{		
		osd_wnd0_set_disable(osd);
	}	
	isppwl_memset((void *)&osd->wnd0, 0, sizeof(osd_wnd0_context_t));

	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd0_set_mapcolor

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_set_mapcolor(isp_osd_context_t *osd, isp_osd_mapcolor_t *map)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(map != IM_NULL);

	// check parameters
	if((map->color < 0) || (map->color > 0xffffff))
	{
		IM_ERRMSG((IM_STR("Invalid mapcolor: mapcolor=%d"), map->color));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd0.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW0CMR_MAPCOLEN, (map->enable == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW0CMR_MAPCOLOR, map->color);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0CMR], osd->regVal[rISP_OVCW0CMR]));
	}

	isppwl_memcpy((void *)&osd->wnd0.mapclr, (void *)map, sizeof(isp_osd_mapcolor_t));

	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd0_set_coordinate

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_set_coordinate(isp_osd_context_t *osd, isp_osd_coordinate_t *coordinate)
{
	IM_INT32 x1, y1;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(coordinate != IM_NULL);

	// check parameters, only support window0 in-line  
	if((coordinate->w <= 0) || (coordinate->h <= 0) || (coordinate->x0 < 0)  || (coordinate->y0 < 0))
	{		
		IM_ERRMSG((IM_STR("Invalid coordinate, x0=%d, y0=%d, w=%d, h=%d"), coordinate->x0, coordinate->y0, coordinate->w, coordinate->h));
		return IM_RET_INVALID_PARAMETER;
	}
	
	x1 = ((IM_INT32)coordinate->w + coordinate->x0) - 1;	
	y1 = ((IM_INT32)coordinate->h + coordinate->y0) - 1;

	if((x1 > (IM_INT32)osd->outWidth - 1)
		||(y1 > (IM_INT32)osd->outHeight - 1))
	{		
		IM_ERRMSG((IM_STR("Invalid coordinate, x1=%d, y1=%d"), x1, y1));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd0.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW0PCAR_LEFTTOPX, coordinate->x0);
		SetIspRegister(osd->regVal, ISP_OVCW0PCAR_LEFTTOPY, coordinate->y0);
		SetIspRegister(osd->regVal, ISP_OVCW0PCBR_RIGHTBOTX, x1);
		SetIspRegister(osd->regVal, ISP_OVCW0PCBR_RIGHTBOTY, y1);

		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0PCAR], osd->regVal[rISP_OVCW0PCAR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0PCBR], osd->regVal[rISP_OVCW0PCBR]));
	}

	isppwl_memcpy((void *)&osd->wnd0.coordinate, (void *)coordinate, sizeof(isp_osd_coordinate_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd0_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_set_enable(isp_osd_context_t *osd)
{
	IM_INT32 x1, y1;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	
	x1 = ((IM_INT32)osd->wnd0.coordinate.w + osd->wnd0.coordinate.x0) - 1;	
	y1 = ((IM_INT32)osd->wnd0.coordinate.h + osd->wnd0.coordinate.y0) - 1;
	
	if((x1 > (IM_INT32)osd->outWidth - 1)
		||(y1 > (IM_INT32)osd->outHeight - 1))
	{		
		IM_ERRMSG((IM_STR("Invalid coordinate, x1=%d, y1=%d"), x1, y1));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//set window0 bppmode, it must be rgb0888
	SetIspRegister(osd->regVal, ISP_OVCW0CR_BPPMODE, ISP_OSD_IMAGE_RGB_BPP24_888);
	
	//set coordinate
	SetIspRegister(osd->regVal, ISP_OVCW0PCAR_LEFTTOPX, osd->wnd0.coordinate.x0);
	SetIspRegister(osd->regVal, ISP_OVCW0PCAR_LEFTTOPY, osd->wnd0.coordinate.y0);
	SetIspRegister(osd->regVal, ISP_OVCW0PCBR_RIGHTBOTX, x1);
	SetIspRegister(osd->regVal, ISP_OVCW0PCBR_RIGHTBOTY, y1);

	//set mapcolor
	SetIspRegister(osd->regVal, ISP_OVCW0CMR_MAPCOLEN, (osd->wnd0.mapclr.enable == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW0CMR_MAPCOLOR, osd->wnd0.mapclr.color);

	//set window0 enable
	SetIspRegister(osd->regVal, ISP_OVCW0CR_ENWIN, 1);

	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0PCAR], osd->regVal[rISP_OVCW0PCAR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0PCBR], osd->regVal[rISP_OVCW0PCBR]));

	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0CMR], osd->regVal[rISP_OVCW0CMR]));

	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0CR], osd->regVal[rISP_OVCW0CR]));

	osd->wnd0.enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd0_set_mapcolor

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd0_set_disable(isp_osd_context_t *osd)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	
	//set window0 disable
	SetIspRegister(osd->regVal, ISP_OVCW0CR_ENWIN, 0);
	
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW0CR], osd->regVal[rISP_OVCW0CR]));

	osd->wnd0.enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}


//osd wndow1
/*------------------------------------------------------------------------------

    Function name: osd_wnd1_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd1_init(isp_osd_context_t *osd, osd_wnd1_context_t *wnd1)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(wnd1 != IM_NULL);
		
	isppwl_memset((void *)&osd->wnd1, 0, sizeof(osd_wnd1_context_t));

	IM_JIF(osd_wnd1_set_swap(osd, &wnd1->swap));
	IM_JIF(osd_wnd1_set_mapcolor(osd, &wnd1->mapclr));
	IM_JIF(osd_wnd1_set_alpha(osd, &wnd1->alpha));
	IM_JIF(osd_wnd1_set_colorkey(osd, &wnd1->clrkey));
	
	if(wnd1->imgFormat <= ISP_OSD_IMAGE_PAL_BPP8)
	{
		if(wnd1->palette.table != IM_NULL)
		{
			IM_JIF(osd_wnd1_set_palette_format(osd, wnd1->imgFormat, &wnd1->palette));
		}
		else
		{
			IM_ERRMSG((IM_STR("Invalid palette table, NULL pointer")));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	else
	{
		IM_JIF(osd_wnd1_set_format(osd, wnd1->imgFormat));
	}

	IM_JIF(osd_wnd1_set_virtual_window(osd, &wnd1->vm));
	IM_JIF(osd_wnd1_set_coordinate(osd, &wnd1->coordinate));
	//IM_JIF(osd_wnd1_set_buffer_mode(osd, &wnd1->buffMode));
	IM_JIF(osd_wnd1_set_buffers(osd, &wnd1->buffer));

	if(wnd1->enable)
	{
		//set window1 enable
		osd_wnd1_set_enable( osd);
	}
	else
	{
		//set window1 disable
		osd_wnd1_set_disable( osd);
	}

	isppwl_memcpy((void *)&osd->wnd1, (void *)wnd1, sizeof(osd_wnd1_context_t));

	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd1_deinit

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd1_deinit(isp_osd_context_t *osd)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	IM_ASSERT(osd != IM_NULL);
	
	if(osd->wnd1.enable == IM_TRUE)
	{		
		osd_wnd1_set_disable(osd);
	}	
	if(osd->wnd1.palette.table != IM_NULL)
	{		
		isppwl_free(osd->wnd1.palette.table);
	}
	
	isppwl_memset((void *)&osd->wnd1, 0, sizeof(osd_wnd1_context_t));

	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd1_set_swap

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd1_set_swap(isp_osd_context_t *osd, isp_osd_swap_t *swap)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(swap != IM_NULL);
	
	if(osd->wnd1.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BITSWP, (swap->bitSwap == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BITS2SWP, (swap->bits2Swap == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BITS4SWP, (swap->bits4Swap == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BYTESWP, (swap->byteSwap == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CR_HAWSWP, (swap->halfwordSwap == IM_FALSE)?0:1);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));
	}

	isppwl_memcpy((void *)&osd->wnd1.swap, (void *)swap, sizeof(isp_osd_swap_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_wnd1_set_alpha

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_wnd1_set_alpha(isp_osd_context_t *osd, isp_osd_alpha_t *alpha)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(alpha != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	
	// check parameters.	
	if((alpha->path != ISP_OSD_ALPHA_PATH_0) && (alpha->path != ISP_OSD_ALPHA_PATH_1))
	{		
		IM_ERRMSG((IM_STR("Invalid alpha path %d"), alpha->path));
		return IM_RET_INVALID_PARAMETER;
	}	
	if((alpha->blendMode != ISP_OSD_BLEND_PER_PLANE) 
		&& (alpha->blendMode != ISP_OSD_BLEND_PER_PIXEL))
	{		
		IM_ERRMSG((IM_STR("Invalid blendMode %d"), alpha->blendMode));
		return IM_RET_INVALID_PARAMETER;
	}	
	if(alpha->blendMode == ISP_OSD_BLEND_PER_PLANE)
	{		
		if(alpha->path == ISP_OSD_ALPHA_PATH_0)
		{			
			if((alpha->alpha0_r > 16) || (alpha->alpha0_g > 16) || (alpha->alpha0_b > 16))
			{				
				IM_ERRMSG((IM_STR("Invalid alpha0 r=%d, g=%d, b=%d"), 
					alpha->alpha0_r, alpha->alpha0_g, alpha->alpha0_b));
				return IM_RET_INVALID_PARAMETER;
			}		
		}
		else if(alpha->path == ISP_OSD_ALPHA_PATH_1)
		{			
			if((alpha->alpha1_r > 16) || (alpha->alpha1_g > 16) || (alpha->alpha1_b > 16))
			{
				IM_ERRMSG((IM_STR("Invalid alpha1 r=%d, g=%d, b=%d"),
					alpha->alpha1_r, alpha->alpha1_g, alpha->alpha1_b));
				return IM_RET_INVALID_PARAMETER;
			}		
		}	
	}

	if(osd->wnd1.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0R, alpha->alpha0_r);
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0G, alpha->alpha0_g);
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0B, alpha->alpha0_b);
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1R, alpha->alpha1_r);
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1G, alpha->alpha1_g);
		SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1B, alpha->alpha1_b);
		
		SetIspRegister(osd->regVal, ISP_OVCW1CR_ALPHASEL, alpha->path);
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BLDPIX, alpha->blendMode);
		
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCCR], osd->regVal[rISP_OVCW1PCCR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));
	}

	isppwl_memcpy((void *)&osd->wnd1.alpha, (void *)alpha, sizeof(isp_osd_alpha_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_format(isp_osd_context_t *osd,  IM_UINT32 imgFormat)
{
	IM_INT32 i, l1, l2, l3;
	IM_INT32 lsize;
	IM_INT32 lpixelOffset, lBuffOffset;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	// check parameters.
	if(imgFormat <= ISP_OSD_IMAGE_PAL_BPP8)
	{			
		IM_ERRMSG((IM_STR("this is palette imgFormat or imgFormat not support, pls use osd_wnd1_set_palette_imgFormat()")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd1.enable == IM_TRUE)
	{
		//set image imgFormat
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BPPMODE, imgFormat);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

		l1 = 0;	// whether or not bit-per-pixel changed.
		lsize = -1;
		if(imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232){
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height;
			if((osd->wnd1.imgFormat != ISP_OSD_IMAGE_PAL_BPP8) 
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP8_1A232))
			{
				l1 = 1;
			}
		}
		else if((imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height << 1;
			if((osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP16_565) 
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP16_1A555)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP16_I555)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP16_4A444))
			{
				l1 = 1;
			}
		}
		else
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height << 2;
			if((osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP18_666)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP18_1A665)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP19_1A666)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP24_888)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP24_1A887)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP25_1A888)
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP28_4A888))
			{
				l1 = 1;
			}
		}

		if(l1 == 1)
		{
			for(i=0; i<4; i++){
				if((lsize != -1) && (osd->wnd1.buffer.mask[i]==IM_FALSE)){
					if(osd->wnd1.buffer.buff[i].size < lsize){
						IM_ERRMSG((IM_STR("vm size(%d) out of the buff[%d] size(%d)"), 
							lsize, i, osd->wnd1.buffer.buff[i].size));
						return IM_RET_FAILED;
					}
				}
			}

			if(osd->wnd1.coordinate.x0 < 0)
			{				
				l2 = 0 - osd->wnd1.coordinate.x0; // l1 used to record vm offset caused by coordinate move.
			}
			else
			{
				l2 = 0;
			}
			if(osd->wnd1.coordinate.y0 < 0)
			{
				l3 = 0 - osd->wnd1.coordinate.y0;// l2 used to record vm offset caused by coordinate move.	
			}
			else
			{
				l3 = 0;
			}
			
			// calc coordinate and buffer offset
			lpixelOffset = (l3 + osd->wnd1.vm.yOffset) * osd->wnd1.vm.width + (l2 + osd->wnd1.vm.xOffset);

			if(imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232)
			{
				lBuffOffset = lpixelOffset;
			}
			else if((imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555)
					||(imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
			{
				lBuffOffset = lpixelOffset << 1;
			}
			else
			{
				lBuffOffset = lpixelOffset << 2;
			}
	
			for(i=0; i<4; i++){
				if(osd->wnd1.buffer.mask[i] != IM_TRUE)
				{
					//set buffer offset
					SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr+lBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
				}
			}

			osd->wnd1.imageSize = lsize;
			osd->wnd1.buffOffset = lBuffOffset;
		}
	}
	
	osd->wnd1.imgFormat = imgFormat;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_palette_format(isp_osd_context_t *osd, IM_UINT32 imgFormat, isp_osd_palette_t *palette)
{
	IM_UINT32 ltblen = 0;
	IM_INT32 i, l1, l2, l3;
	IM_INT32 lsize;
	IM_INT32 lpixelOffset, lBuffOffset, lbitBuffOffset;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(palette != IM_NULL);

	// check parameters.
	if(imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
	{
		ltblen = 2;		
	}
	else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
	{			
		ltblen = 4;
	}
	else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
	{			
		ltblen = 16;
	}
	else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP8)
	{			
		ltblen = 256;
	}
	else
	{			
		IM_ERRMSG((IM_STR("this is not palette imgFormat, pls use osd_wnd1_set_imgFormat()")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(palette->table == IM_NULL)
	{		
		IM_ERRMSG((IM_STR("Invalid table, NULL pointer")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((palette->palFormat < ISP_OSD_PALETTE_FORMAT_A888) 
		|| (palette->palFormat > ISP_OSD_PALETTE_FORMAT_565))
	{		
		IM_ERRMSG((IM_STR("Invalid palette imgFormat %d"), palette->palFormat));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if((palette->tableLength > 256) || (palette->tableLength < ltblen))
	{		
		IM_ERRMSG((IM_STR("Invalid palette tablelength %d"), palette->tableLength));
		return IM_RET_INVALID_PARAMETER;
	}

	if(osd->wnd1.enable == IM_TRUE)
	{
		//set palette table to memory
		SetIspRegister(osd->regVal, ISP_OVCPCR_UPDATE_PAL, 1);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));
#if 0
		isppwl_ioctl(DWL_ISP_IOCTRL_SET_OSD_PAL,(void*)palette->table, palette->tableLength*4, IM_NULL, 0, IM_NULL);
#else
		for(i=0;i < palette->tableLength;i++)
		{	
			IM_JIF(isppwl_write_reg((ISP_OSD_PAL_TABLE+4*i), palette->table[i]));
		}
#endif		
		SetIspRegister(osd->regVal, ISP_OVCPCR_UPDATE_PAL, 0);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));

		//set image imgFormat and palette imgFormat
		SetIspRegister(osd->regVal, ISP_OVCW1CR_BPPMODE, imgFormat);
		SetIspRegister(osd->regVal, ISP_OVCPCR_W1PALFM, palette->palFormat);

		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));

		//maybe need to modified buffer and bit offset as image imgFormat change 
		l1 = 0;	// whether or not bit-per-pixel changed.
		lsize = -1;
		if(imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 3;
			if(osd->wnd1.imgFormat != ISP_OSD_IMAGE_PAL_BPP1)
			{
				l1 = 1;
			}
		}
		else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 2;
			if(osd->wnd1.imgFormat != ISP_OSD_IMAGE_PAL_BPP2)
			{
				l1 = 1;
			}
		}
		else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 1;
			if(osd->wnd1.imgFormat != ISP_OSD_IMAGE_PAL_BPP4)
			{
				l1 = 1;
			}
		}
		else /*if(imgFormat == ISP_OSD_IMAGE_PAL_BPP8)*/
		{
			lsize = osd->wnd1.vm.width * osd->wnd1.vm.height;
			if((osd->wnd1.imgFormat != ISP_OSD_IMAGE_PAL_BPP8) 
				&&(osd->wnd1.imgFormat != ISP_OSD_IMAGE_RGB_BPP8_1A232))
			{
				l1 = 1;
			}
		}

		if(l1 == 1){
			for(i=0; i<4; i++){
				if((lsize != -1) && (osd->wnd1.buffer.mask[i]==IM_FALSE)){
					if(osd->wnd1.buffer.buff[i].size < lsize){
						IM_ERRMSG((IM_STR("vm size(%d) out of the buff[%d] size(%d)"), 
							lsize, i, osd->wnd1.buffer.buff[i].size));
						return IM_RET_FAILED;
					}
				}
			}
			
			if(osd->wnd1.coordinate.x0 < 0)
			{				
				l2 = 0 - osd->wnd1.coordinate.x0; // l1 used to record vm offset caused by coordinate move.
			}
			else
			{
				l2 = 0;
			}
			if(osd->wnd1.coordinate.y0 < 0)
			{
				l3 = 0 - osd->wnd1.coordinate.y0;// l2 used to record vm offset caused by coordinate move.	
			}
			else
			{
				l3 = 0;
			}
			
			// calc coordinate and buffer offset
			lpixelOffset = (l3 + osd->wnd1.vm.yOffset) * osd->wnd1.vm.width + (l2 + osd->wnd1.vm.xOffset);
			if(imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
			{
				lBuffOffset = lpixelOffset >> 3;
				lbitBuffOffset = lpixelOffset & 0x7;
			}
			else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
			{
				lBuffOffset = lpixelOffset >> 2;
				lbitBuffOffset = (lpixelOffset<<1) & 0x7;
			}
			else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
			{
				lBuffOffset = lpixelOffset >> 1;
				lbitBuffOffset = (lpixelOffset<<2) & 0x7;
			}
			else if(imgFormat == ISP_OSD_IMAGE_PAL_BPP8)
			{
				lBuffOffset = lpixelOffset;
				lbitBuffOffset = 0;
			}
	
			for(i=0; i<4; i++){
				if(osd->wnd1.buffer.mask[i] != IM_TRUE)
				{
					//set bit offset
					SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, lbitBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
					
					//set buffer offset
					SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr+lBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
				}			
			}
			
			osd->wnd1.imageSize = lsize;
			osd->wnd1.buffOffset = lBuffOffset;
			osd->wnd1.bitBuffOffset = lbitBuffOffset;
		}
	}


	if(osd->wnd1.palette.table == IM_NULL)
	{			
		osd->wnd1.palette.table = (IM_UINT32 *)isppwl_malloc(palette->tableLength * 4);
	}
	else if(osd->wnd1.palette.tableLength < palette->tableLength)
	{
		isppwl_free(osd->wnd1.palette.table);
		osd->wnd1.palette.table = (IM_UINT32 *)isppwl_malloc(palette->tableLength * 4);
	}		

	if(osd->wnd1.palette.table == IM_NULL)
	{			
		IM_ERRMSG((IM_STR("pwl_malloc() failed")));
		return IM_RET_NOMEMORY;
	}		
	osd->wnd1.palette.palFormat = palette->palFormat;
	osd->wnd1.palette.tableLength = palette->tableLength;
	isppwl_memcpy((void *)osd->wnd1.palette.table, (void *)palette->table, palette->tableLength * 4);

	osd->wnd1.imgFormat = imgFormat;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_mapcolor(isp_osd_context_t *osd, isp_osd_mapcolor_t *map)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(map != IM_NULL);

	//check parameters
	if((map->color < 0) || (map->color > 0xffffff))
	{
		IM_ERRMSG((IM_STR("Invalid mapcolor: mapcolor=%d"), map->color));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd1.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW1CMR_MAPCOLEN, (map->enable == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CMR_MAPCOLOR, map->color);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CMR], osd->regVal[rISP_OVCW1CMR]));
	}

	isppwl_memcpy((void *)&osd->wnd1.mapclr, (void *)map, sizeof(isp_osd_mapcolor_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_colorkey(isp_osd_context_t *osd, isp_osd_colorkey_t *clrkey)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(clrkey != IM_NULL);
	
	// check parameters.	
	if((clrkey->matchMode != ISP_OSD_COLORKEY_DIR_MATCH_FORGROUND) 
		&& (clrkey->matchMode != ISP_OSD_COLORKEY_DIR_MATCH_BACKGROUND))
	{		
		IM_ERRMSG((IM_STR("Invalid matchMode %d"), clrkey->matchMode));
		return IM_RET_INVALID_PARAMETER;
	}

	if(osd->wnd1.enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCW1CKCR_KEYEN, (clrkey->enable == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CKCR_KEYBLEN, (clrkey->enableBlend == IM_FALSE)?0:1);
		SetIspRegister(osd->regVal, ISP_OVCW1CKCR_DIRCON, clrkey->matchMode);
		SetIspRegister(osd->regVal, ISP_OVCW1CKCR_COMPKEY, clrkey->mask);
		SetIspRegister(osd->regVal, ISP_OVCW1CKR, clrkey->color);
		
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CKCR], osd->regVal[rISP_OVCW1CKCR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CKR], osd->regVal[rISP_OVCW1CKR]));
	}

	isppwl_memcpy((void *)&osd->wnd1.clrkey, (void *)clrkey, sizeof(isp_osd_colorkey_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_buffer_mode(isp_osd_context_t *osd,  isp_osd_buffer_mode *bm)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(bm != IM_NULL);

	// check parameters
	if((bm->mode != ISP_OSD_BUFFER_SEL_AUTO) && (bm->mode != ISP_OSD_BUFFER_SEL_MANUAL))
	{
		IM_ERRMSG((IM_STR("Invalid mode: mode=%d"), bm->mode));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(bm->mode == ISP_OSD_BUFFER_SEL_AUTO)
	{
		if((bm->number > 4) || (bm->number < 2))
		{
			IM_ERRMSG((IM_STR("Invalid auto buffer number: mode=%d"), bm->number));
			return IM_RET_INVALID_PARAMETER;
		}
	}
	else
	{
		if((bm->selType > ISP_OSD_BUFSEL_BUF3) || (bm->selType < ISP_OSD_BUFSEL_BUF0))
		{
			IM_ERRMSG((IM_STR("Invalid selType: selType=%d"), bm->selType));
			return IM_RET_INVALID_PARAMETER;
		}
	}

	
	if(osd->wnd1.enable == IM_TRUE)
	{
		if(bm->mode == ISP_OSD_BUFFER_SEL_AUTO)
		{
			SetIspRegister(osd->regVal, ISP_OVCW1CR_BUFNUM, bm->number);
		}
		else
		{
			SetIspRegister(osd->regVal, ISP_OVCW1CR_BUFSEL, bm->selType);
		}

		SetIspRegister(osd->regVal, ISP_OVCW1CR_BUFAUTOEN, bm->mode);
		
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));
	}

	isppwl_memcpy((void *)&osd->wnd1.bm, (void *)bm, sizeof(isp_osd_buffer_mode));

	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_buffers(isp_osd_context_t *osd, isp_osd_buffer_t *buffer)
{
	IM_INT32 i, l1;
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(buffer != IM_NULL);

	if(osd->wnd1.enable == IM_TRUE)
	{
		l1 = 0x0; // buffer phy_addr alignment.
		if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565) ||
			(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555) ||
			(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) ||
			(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444) ){
			l1 = 0x01;
		}else if((osd->wnd1.imgFormat >= ISP_OSD_IMAGE_RGB_BPP18_666) &&
			(osd->wnd1.imgFormat <= ISP_OSD_IMAGE_RGB_BPP28_4A888) ){
			l1 = 0x03;
		}

		for(i=0; i<4; i++){
			if(buffer->mask[i] == IM_TRUE){
				continue;
			}

			if(osd->wnd1.imageSize != 0){
				if((buffer->buff[i].phy_addr == 0) || (buffer->buff[i].phy_addr & l1) ||
					(buffer->buff[i].size < osd->wnd1.imageSize)){
					IM_ERRMSG((IM_STR("Invalid buff[%d], phy=0x%x, size=%d, imageSize=%d"),
						i, buffer->buff[i].phy_addr, buffer->buff[i].size, osd->wnd1.imageSize));
					return IM_RET_FAILED; 
				}
			}		
		}

		//
		// flush
		//
		for(i=0; i<4; i++){
			if((osd->wnd1.imageSize != 0) && (buffer->mask[i] != IM_TRUE)){
				if(osd->wnd1.imgFormat <= ISP_OSD_IMAGE_PAL_BPP4){
					//set bit offset
					SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, osd->wnd1.bitBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
				}
				/*else
				{
					//set bit offset
					SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, 0);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
				}*/

				//set buffer offset
				SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr + osd->wnd1.buffOffset);
				IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
			}			
		}
	}

	isppwl_memcpy((void *)&osd->wnd1.buffer, (void *)buffer, sizeof(isp_osd_buffer_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_virtual_window(isp_osd_context_t *osd, isp_osd_vm_t *vm)
{
	IM_INT32 i, l1, l2, l3;
	IM_INT32 lsize;
	IM_INT32 lpixelOffset, lBuffOffset, lbitBuffOffset;
	
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(vm != IM_NULL);
	
	// check parameters.
	if(((vm->width <= 0) || (vm->width >= (1<<16))) || (vm->height <= 0)){
		IM_ERRMSG((IM_STR("Invalid vm->width %d or vm->height %d"), vm->width, vm->height));
		return IM_RET_INVALID_PARAMETER;
	}

	if((vm->xOffset >= vm->width) || (vm->yOffset >= vm->height)){
		IM_ERRMSG((IM_STR("Invalid vm->offset x=%d, y=%d"), vm->xOffset, vm->yOffset));
		return IM_RET_INVALID_PARAMETER;
	}

	if(osd->wnd1.enable == IM_TRUE)
	{
		//maybe need to modified buffer and bit offset as image imgFormat change 
		l1 = 0;	// whether or not bit-per-pixel changed.
		lsize = -1;

		// vm -- buffer
		if((vm->width != osd->wnd1.vm.width) 
			|| (vm->height != osd->wnd1.vm.height))
		{
			l1=1;
			if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
			{
				lsize = vm->width * vm->height >> 3;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
			{
				lsize = vm->width * vm->height >> 2;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
			{
				lsize = vm->width * vm->height >> 1;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232
					|| osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP8)
			{
				lsize = vm->width * vm->height;
			}
			else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555) 
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) 
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
			{
				lsize = vm->width * vm->height << 1;
			}
			else
			{
				lsize = vm->width * vm->height << 2;
			}

			for(i=0; i<4; i++){
				if((lsize != -1) && (osd->wnd1.buffer.mask[i] == IM_FALSE))
				{
					if(osd->wnd1.buffer.buff[i].size < lsize)
					{
						IM_ERRMSG((IM_STR("vm size(%d) out of the buff[%d] size(%d)"), 
							lsize, i, osd->wnd1.buffer.buff[i].size));
						return IM_RET_FAILED;
					}
				}
			}
		}

		// vm -- coordinate
		if((vm->xOffset != osd->wnd1.vm.xOffset) 
			|| (vm->yOffset != osd->wnd1.vm.yOffset))
		{
			l1=1;
		}
		
		if(l1 == 1)
		{
			if(osd->wnd1.coordinate.x0 < 0){
				l2 = 0 - osd->wnd1.coordinate.x0;// l2 used to record vm offset caused by coordinate move.
			}else{
				l2 = 0;
			}

			if(osd->wnd1.coordinate.y0 < 0){
				l3 = 0 - osd->wnd1.coordinate.y0;// l3 used to record vm offset caused by coordinate move.
			}else{
				l3 = 0;
			}

			if(osd->wnd1.coordinate.w > (vm->width - vm->xOffset)){
				IM_ERRMSG((IM_STR("coordinate width(x0=%d, w=%d) out of vm width(width=%d, xOffset=%d)"), 
					osd->wnd1.coordinate.x0, osd->wnd1.coordinate.w, vm->width, vm->xOffset));
				return IM_RET_FAILED;
			}
			if(osd->wnd1.coordinate.h > (vm->height - vm->yOffset)){
				IM_ERRMSG((IM_STR("coordinate height(y0=%d, h=%d) out of vm height(height=%d, yOffset=%d)"), 
					osd->wnd1.coordinate.y0, osd->wnd1.coordinate.h, vm->height, vm->yOffset));
				return IM_RET_FAILED;
			}

			// calc coordinate and buffer offset
			lpixelOffset = (l3 + vm->yOffset) * vm->width + (l2 + vm->xOffset);
			lBuffOffset = lbitBuffOffset = -1;
			if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
			{
				lBuffOffset = lpixelOffset >> 3;
				lbitBuffOffset = lpixelOffset & 0x7;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
			{
				lBuffOffset = lpixelOffset >> 2;
				lbitBuffOffset = (lpixelOffset<<1) & 0x7;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
			{
				lBuffOffset = lpixelOffset >> 1;
				lbitBuffOffset = (lpixelOffset<<2) & 0x7;
			}
			else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232)
				|| (osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP8))
			{
				lBuffOffset = lpixelOffset;
				lbitBuffOffset = 0;
			}else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555)
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) 
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
			{
				lBuffOffset = lpixelOffset << 1;
				lbitBuffOffset = 0;
			}else{
				lBuffOffset = lpixelOffset << 2;
				lbitBuffOffset = 0;
			}	

			for(i=0; i<4; i++)
			{
				if(osd->wnd1.buffer.mask[i] != IM_TRUE)
				{
					//set bit offset
					SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, lbitBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
					
					//set buffer offset
					SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr+lBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
				}
			}

			if(lBuffOffset != -1){
				osd->wnd1.buffOffset = lBuffOffset;
			}
			if(lbitBuffOffset != -1){
				osd->wnd1.bitBuffOffset = lbitBuffOffset;
			}
		}

		//set vm width to register
		SetIspRegister(osd->regVal, ISP_OVCW1VSSR_VW_WIDTH, vm->width);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));

		osd->wnd1.imageSize = lsize;
	}

	isppwl_memcpy((void *)&osd->wnd1.vm, (void *)vm, sizeof(isp_osd_vm_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_coordinate(isp_osd_context_t *osd, isp_osd_coordinate_t *coordinate)
{
	IM_INT32 i, l2, l3;
	IM_INT32 lx1, ly1;
	IM_INT32 xMax, yMax;
	IM_INT32 lpixelOffset, lBuffOffset, lbitBuffOffset;
	
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	IM_ASSERT(coordinate != IM_NULL);

	// check parameters.
	xMax = osd->outWidth -1; 
	yMax = osd->outHeight - 1;
	if((coordinate->w <= 0)
		|| (coordinate->h <= 0)
		|| ((IM_INT32)coordinate->w + coordinate->x0 < 1)
		|| ((IM_INT32)coordinate->h + coordinate->y0 < 1)
		|| (coordinate->x0 > xMax) 
		|| (coordinate->y0  > yMax))
	{
		IM_ERRMSG((IM_STR("Invalid coordinate, x0=%d, y0=%d, w=%d, h=%d"), coordinate->x0, coordinate->y0, coordinate->w, coordinate->h));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd1.enable == IM_TRUE)
	{
		if(coordinate->x0 < 0){
			l2 = 0 - coordinate->x0;// l2 used to record vm offset caused by coordinate move.
		}else{
			l2 = 0;
		}

		if(coordinate->y0 < 0){
			l3 = 0 - coordinate->y0;// l3 used to record vm offset caused by coordinate move.
		}else{
			l3 = 0;
		}

		if(coordinate->w > (osd->wnd1.vm.width - osd->wnd1.vm.xOffset)){
			IM_ERRMSG((IM_STR("coordinate width(x0=%d, w=%d) out of vm width(width=%d, xOffset=%d)"), 
				coordinate->x0, coordinate->w, osd->wnd1.vm.width, osd->wnd1.vm.xOffset));
			return IM_RET_FAILED;
		}
		if(coordinate->h > (osd->wnd1.vm.height - osd->wnd1.vm.yOffset)){
			IM_ERRMSG((IM_STR("coordinate height(y0=%d, h=%d) out of vm height(height=%d, yOffset=%d)"), 
				coordinate->y0, coordinate->h, osd->wnd1.vm.height, osd->wnd1.vm.yOffset));
			return IM_RET_FAILED;
		}

		lx1 = (IM_INT32)coordinate->w + coordinate->x0 - 1;
		ly1 = (IM_INT32)coordinate->h + coordinate->y0 - 1;
		if(lx1 >= (IM_INT32)osd->outWidth)
		{
			lx1 = (IM_INT32)osd->outWidth - 1;
		}
		if(ly1 >= (IM_INT32)osd->outHeight)
		{
			ly1 = (IM_INT32)osd->outHeight - 1;
		}
	
		if((osd->wnd1.coordinate.x0 >= 0)
			&&(osd->wnd1.coordinate.y0 >= 0)
			&&(coordinate->x0 >= 0)
			&&(coordinate->y0 >= 0))
		{
			//not need set buffer offset again
			lBuffOffset = -1;
			lbitBuffOffset = -1;
		}
		else
		{
			// calc coordinate and buffer offset
			lpixelOffset = (l3 + osd->wnd1.vm.yOffset) * osd->wnd1.vm.width + (l2 + osd->wnd1.vm.xOffset);
			lBuffOffset = lbitBuffOffset = -1;
			if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
			{
				lBuffOffset = lpixelOffset >> 3;
				lbitBuffOffset = lpixelOffset & 0x7;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
			{
				lBuffOffset = lpixelOffset >> 2;
				lbitBuffOffset = (lpixelOffset<<1) & 0x7;
			}
			else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
			{
				lBuffOffset = lpixelOffset >> 1;
				lbitBuffOffset = (lpixelOffset<<2) & 0x7;
			}
			else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232)
				|| (osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP8))
			{
				lBuffOffset = lpixelOffset;
				lbitBuffOffset = 0;
			}else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555)
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) 
				||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
			{
				lBuffOffset = lpixelOffset << 1;
				lbitBuffOffset = 0;
			}else{
				lBuffOffset = lpixelOffset << 2;
				lbitBuffOffset = 0;
			}			

			for(i=0; i<4; i++)
			{
				if(osd->wnd1.buffer.mask[i] != IM_TRUE)
				{
					//set bit offset
					SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, lbitBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
					
					//set buffer offset
					SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr+lBuffOffset);
					IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
				}
			}

			if(lBuffOffset != -1){
				osd->wnd1.buffOffset = lBuffOffset;
			}

			if(lbitBuffOffset != -1){
				osd->wnd1.bitBuffOffset = lbitBuffOffset;
			}
		}
		
		//set coordinate to register
		SetIspRegister(osd->regVal, ISP_OVCW1PCAR_LEFTTOPX, (coordinate->x0 < 0)?0:coordinate->x0);
		SetIspRegister(osd->regVal, ISP_OVCW1PCAR_LEFTTOPY, (coordinate->y0 < 0)?0:coordinate->y0);
		SetIspRegister(osd->regVal, ISP_OVCW1PCBR_RIGHTBOTX, lx1);
		SetIspRegister(osd->regVal, ISP_OVCW1PCBR_RIGHTBOTY, ly1);

		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCAR], osd->regVal[rISP_OVCW1PCAR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCBR], osd->regVal[rISP_OVCW1PCBR]));
	}
	
	isppwl_memcpy((void *)&osd->wnd1.coordinate, (void *)coordinate, sizeof(isp_osd_coordinate_t));
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_enable(isp_osd_context_t *osd)
{
	IM_INT32 i, l1, l2, l3;
	IM_INT32 lx1, ly1;
	IM_INT32 lsize;
	IM_INT32 lpixelOffset, lBuffOffset, lbitBuffOffset;
	IM_INT32 xMax, yMax;

	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	//set swap
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BITSWP, (osd->wnd1.swap.bitSwap == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BITS2SWP, (osd->wnd1.swap.bits2Swap == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BITS4SWP, (osd->wnd1.swap.bits4Swap == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BYTESWP, (osd->wnd1.swap.byteSwap == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CR_HAWSWP, (osd->wnd1.swap.halfwordSwap == IM_FALSE)?0:1);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

	//set alpha
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0R, osd->wnd1.alpha.alpha0_r);
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0G, osd->wnd1.alpha.alpha0_g);
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA0B, osd->wnd1.alpha.alpha0_b);
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1R, osd->wnd1.alpha.alpha1_r);
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1G, osd->wnd1.alpha.alpha1_g);
	SetIspRegister(osd->regVal, ISP_OVCW1PCCR_ALPHA1B, osd->wnd1.alpha.alpha1_b);
	
	SetIspRegister(osd->regVal, ISP_OVCW1CR_ALPHASEL, osd->wnd1.alpha.path);
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BLDPIX, osd->wnd1.alpha.blendMode);
		
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCCR], osd->regVal[rISP_OVCW1PCCR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

	//set map color
	SetIspRegister(osd->regVal, ISP_OVCW1CMR_MAPCOLEN, (osd->wnd1.mapclr.enable == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CMR_MAPCOLOR, osd->wnd1.mapclr.color);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CMR], osd->regVal[rISP_OVCW1CMR]));

	//set colorkey
	SetIspRegister(osd->regVal, ISP_OVCW1CKCR_KEYEN, (osd->wnd1.clrkey.enable == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CKCR_KEYBLEN, (osd->wnd1.clrkey.enableBlend == IM_FALSE)?0:1);
	SetIspRegister(osd->regVal, ISP_OVCW1CKCR_DIRCON, osd->wnd1.clrkey.matchMode);
	SetIspRegister(osd->regVal, ISP_OVCW1CKCR_COMPKEY, osd->wnd1.clrkey.mask);
	SetIspRegister(osd->regVal, ISP_OVCW1CKR, osd->wnd1.clrkey.color);
	
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CKCR], osd->regVal[rISP_OVCW1CKCR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CKR], osd->regVal[rISP_OVCW1CKR]));

	//set format
	SetIspRegister(osd->regVal, ISP_OVCW1CR_BPPMODE, osd->wnd1.imgFormat);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

	if(osd->wnd1.imgFormat <= ISP_OSD_IMAGE_PAL_BPP8)
	{
		IM_INFOMSG((IM_STR("it's palette format: imgFormat=%d"), osd->wnd1.imgFormat));
		if(osd->wnd1.palette.table == IM_NULL)
		{		
			IM_ERRMSG((IM_STR("Invalid palette table, NULL pointer")));
			return IM_RET_INVALID_PARAMETER;
		}
		
		//set palette table to memory
		SetIspRegister(osd->regVal, ISP_OVCPCR_UPDATE_PAL, 1);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));
#if 0
		isppwl_ioctl(DWL_ISP_IOCTRL_SET_OSD_PAL,(void*)palette->table, palette->tableLength*4, IM_NULL, 0, IM_NULL);
#else
		for(i=0;i < osd->wnd1.palette.tableLength;i++)
		{	
			IM_JIF(isppwl_write_reg((ISP_OSD_PAL_TABLE+4*i), osd->wnd1.palette.table[i]));
		}
#endif		
		SetIspRegister(osd->regVal, ISP_OVCPCR_UPDATE_PAL, 0);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));

		//set palette imgFormat
		SetIspRegister(osd->regVal, ISP_OVCPCR_W1PALFM, osd->wnd1.palette.palFormat);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCPCR], osd->regVal[rISP_OVCPCR]));	
	}

	//set vm
	if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 3;
	}
	else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 2;
	}
	else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height >> 1;
	}
	else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232)
			|| (osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP8))
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height;
	}
	else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555) 
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) 
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height << 1;
	}
	else
	{
		lsize = osd->wnd1.vm.width * osd->wnd1.vm.height << 2;
	}

	SetIspRegister(osd->regVal, ISP_OVCW1VSSR_VW_WIDTH, osd->wnd1.vm.width);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));

	osd->wnd1.imageSize = lsize;

	//set coordinate
	xMax = (IM_INT32)osd->outWidth -1; 
	yMax = (IM_INT32)osd->outHeight - 1;
	if((osd->wnd1.coordinate.w <= 0)
	|| (osd->wnd1.coordinate.h <= 0)
	|| ((IM_INT32)osd->wnd1.coordinate.w + osd->wnd1.coordinate.x0 < 1)
	|| ((IM_INT32)osd->wnd1.coordinate.h + osd->wnd1.coordinate.y0 < 1)
	|| (osd->wnd1.coordinate.x0 > xMax) 
	|| (osd->wnd1.coordinate.y0 > yMax))
	{
		IM_ERRMSG((IM_STR("Invalid coordinate, x0=%d, y0=%d, w=%d, h=%d"), osd->wnd1.coordinate.x0, osd->wnd1.coordinate.y0, osd->wnd1.coordinate.w, osd->wnd1.coordinate.h));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->wnd1.coordinate.x0 < 0){
		l2 = 0 - osd->wnd1.coordinate.x0;// l2 used to record vm offset caused by coordinate move.
	}else{
		l2 = 0;
	}

	if(osd->wnd1.coordinate.y0 < 0){
		l3 = 0 - osd->wnd1.coordinate.y0;// l3 used to record vm offset caused by coordinate move.
	}else{
		l3 = 0;
	}

	if(osd->wnd1.coordinate.w > (osd->wnd1.vm.width - osd->wnd1.vm.xOffset)){
		IM_ERRMSG((IM_STR("coordinate width(x0=%d, w=%d) out of vm width(width=%d, xOffset=%d)"), 
			osd->wnd1.coordinate.x0, osd->wnd1.coordinate.w, osd->wnd1.vm.width, osd->wnd1.vm.xOffset));
		return IM_RET_FAILED;
	}
	if(osd->wnd1.coordinate.h > (osd->wnd1.vm.height - osd->wnd1.vm.yOffset)){
		IM_ERRMSG((IM_STR("coordinate height(y0=%d, h=%d) out of vm height(height=%d, yOffset=%d)"), 
			osd->wnd1.coordinate.y0, osd->wnd1.coordinate.h, osd->wnd1.vm.height, osd->wnd1.vm.yOffset));
		return IM_RET_FAILED;
	}

	lx1 = (IM_INT32)osd->wnd1.coordinate.w + osd->wnd1.coordinate.x0 - 1;
	ly1 = (IM_INT32)osd->wnd1.coordinate.h + osd->wnd1.coordinate.y0 - 1;
	if(lx1 >= (IM_INT32)osd->outWidth)
	{
		lx1 = (IM_INT32)osd->outWidth - 1;
	}
	if(ly1 >= (IM_INT32)osd->outHeight)
	{
		ly1 = (IM_INT32)osd->outHeight - 1;
	}
	
	SetIspRegister(osd->regVal, ISP_OVCW1PCAR_LEFTTOPX, (osd->wnd1.coordinate.x0 < 0)?0:osd->wnd1.coordinate.x0);
	SetIspRegister(osd->regVal, ISP_OVCW1PCAR_LEFTTOPY, (osd->wnd1.coordinate.y0 < 0)?0:osd->wnd1.coordinate.y0);
	SetIspRegister(osd->regVal, ISP_OVCW1PCBR_RIGHTBOTX, lx1);
	SetIspRegister(osd->regVal, ISP_OVCW1PCBR_RIGHTBOTY, ly1);

	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCAR], osd->regVal[rISP_OVCW1PCAR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1PCBR], osd->regVal[rISP_OVCW1PCBR]));

	//set buffers
	lpixelOffset = (l3 + osd->wnd1.vm.yOffset) * osd->wnd1.vm.width + (l2 + osd->wnd1.vm.xOffset);
	l1 = 0x0; // buffer phy_addr alignment.

	if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP1)
	{
		lBuffOffset = lpixelOffset >> 3;
		lbitBuffOffset = lpixelOffset & 0x7;
	}
	else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP2)
	{
		lBuffOffset = lpixelOffset >> 2;
		lbitBuffOffset = (lpixelOffset<<1) & 0x7;
	}
	else if(osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP4)
	{
		lBuffOffset = lpixelOffset >> 1;
		lbitBuffOffset = (lpixelOffset<<2) & 0x7;
	}
	else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP8_1A232)
		|| (osd->wnd1.imgFormat == ISP_OSD_IMAGE_PAL_BPP8))
	{
		lBuffOffset = lpixelOffset;
		lbitBuffOffset = 0;
	}else if((osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_565)
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_1A555)
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_I555) 
		||(osd->wnd1.imgFormat == ISP_OSD_IMAGE_RGB_BPP16_4A444))
	{
		lBuffOffset = lpixelOffset << 1;
		lbitBuffOffset = 0;
		l1 = 0x01;
	}else{
		lBuffOffset = lpixelOffset << 2;
		lbitBuffOffset = 0;
		l1 = 0x03;
	}

	for(i=0; i<4; i++){
		if(osd->wnd1.buffer.mask[i] == IM_TRUE){
			continue;
		}

		if(osd->wnd1.imageSize != 0){
			if((osd->wnd1.buffer.buff[i].phy_addr == 0) || (osd->wnd1.buffer.buff[i].phy_addr & l1) ||
				(osd->wnd1.buffer.buff[i].size < osd->wnd1.imageSize)){
				IM_ERRMSG((IM_STR("Invalid buff[%d], phy=0x%x, size=%d, imageSize=%d"),
					i, osd->wnd1.buffer.buff[i].phy_addr, osd->wnd1.buffer.buff[i].size, osd->wnd1.imageSize));
				return IM_RET_FAILED; 
			}
		}		
	}

	for(i=0; i<4; i++)
	{
		if(osd->wnd1.buffer.mask[i] != IM_TRUE)
		{
			//set bit offset
			SetIspRegister(osd->regVal, ISP_OVCW1VSSR_BUF0_BITADR-i, lbitBuffOffset);
			IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1VSSR], osd->regVal[rISP_OVCW1VSSR]));
			
			//set buffer offset
			SetIspRegister(osd->regVal, ISP_OVCW1B0SAR+i, osd->wnd1.buffer.buff[i].phy_addr+lBuffOffset);
			IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1B0SAR+i], osd->regVal[rISP_OVCW1B0SAR+i]));
		}
	}

	osd->wnd1.buffOffset = lBuffOffset;
	osd->wnd1.bitBuffOffset = lbitBuffOffset;


	//set window1 enable
	SetIspRegister(osd->regVal, ISP_OVCW1CR_ENWIN, 1);
	
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

	osd->wnd1.enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

IM_RET osd_wnd1_set_disable(isp_osd_context_t *osd)
{
	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
	
	//set window1 disable
	SetIspRegister(osd->regVal, ISP_OVCW1CR_ENWIN, 0);
	
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCW1CR], osd->regVal[rISP_OVCW1CR]));

	osd->wnd1.enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

//osd global
/*------------------------------------------------------------------------------

    Function name: osd_init

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_init(isp_osd_context_t *osd, osd_config_t *cfg)
{
	IM_RET ret;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	if(cfg == IM_NULL)
	{
		IM_ERRMSG((IM_STR("Invalid cfg: osd cfg is NULL")));
		return IM_RET_INVALID_PARAMETER;
	}
	
	//some bit must set to 0
	SetIspRegister(osd->regVal, ISP_OVCDCR_MUSTSET0, 0);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCDCR], osd->regVal[rISP_OVCDCR]));
	
	if(cfg->enable == IM_TRUE)
	{
		
		/*set osd background color*/		
		SetIspRegister(osd->regVal, ISP_OVCBCR, cfg->bgColor);

		/*set osd out size*/
		SetIspRegister(osd->regVal, ISP_OVCWPxR_WIDTH, cfg->outWidth - 1);
		SetIspRegister(osd->regVal, ISP_OVCWPyR_HEIGHT, cfg->outHeight -1);

		/*set osd enable*/
		SetIspRegister(osd->regVal, ISP_GLB_CFG2_OSDBYPASS, 0);

		//isppwl_write_regs()
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCBCR], osd->regVal[rISP_OVCBCR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPxR], osd->regVal[rISP_OVCWPxR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPyR], osd->regVal[rISP_OVCWPyR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_GLB_CFG2], osd->regVal[rISP_GLB_CFG2]));
	}
	else
	{
		/*set osd disable*/
		SetIspRegister(osd->regVal, ISP_GLB_CFG2_OSDBYPASS, 1);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_GLB_CFG2], osd->regVal[rISP_GLB_CFG2]));
	}
	
	osd->bgColor = cfg->bgColor;
	osd->outWidth = cfg->outWidth;
	osd->outHeight = cfg->outHeight;

	//init wnd0
	ret = osd_wnd0_init(osd, &(cfg->wnd0));
	
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("osd_wnd0_init() failed")));
		return IM_RET_FAILED;
	}

	//init wnd1
	ret = osd_wnd1_init(osd, &(cfg->wnd1));
	
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("osd_wnd1_init() failed")));
		return IM_RET_FAILED;
	}
	
	isppwl_memcpy((void *)&osd->wnd0, (void *)&cfg->wnd0, sizeof(osd_wnd0_context_t));
	isppwl_memcpy((void *)&osd->wnd1, (void *)&cfg->wnd1, sizeof(osd_wnd1_context_t));

	osd->enable = cfg->enable;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_deinit

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_deinit(isp_osd_context_t *osd)
{
	IM_RET ret;
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	//deinit wnd0
	ret = osd_wnd0_deinit(osd);
	
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("osd_wnd0_init() failed")));
		return IM_RET_FAILED;
	}

	//deinit wnd1
	ret = osd_wnd1_deinit(osd);
	
	if(ret != IM_RET_OK)
	{
		IM_ERRMSG((IM_STR("osd_wnd1_deinit() failed")));
		return IM_RET_FAILED;
	}
	
	return IM_RET_OK;
}

/*------------------------------------------------------------------------------

    Function name: osd_set_background_color

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_set_background_color(isp_osd_context_t *osd, IM_UINT32 bgColor)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	if((bgColor < 0) || (bgColor > 0xffffff))
	{
		IM_ERRMSG((IM_STR("Invalid bgColor: bgColor=%d"), bgColor));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCBCR, bgColor);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCBCR], osd->regVal[rISP_OVCBCR]));
	}

	osd->bgColor = bgColor;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_set_out_size

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_set_out_size(isp_osd_context_t *osd, IM_UINT32 width, IM_UINT32 height)
{
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	if((width < 1) || (width > 4096)
		||(height < 1) || (height > 4096))
	{
		IM_ERRMSG((IM_STR("Invalid out size: width=%d, height=%d"), width, height));
		return IM_RET_INVALID_PARAMETER;
	}
	
	if(osd->enable == IM_TRUE)
	{
		SetIspRegister(osd->regVal, ISP_OVCWPxR_WIDTH, width - 1);
		SetIspRegister(osd->regVal, ISP_OVCWPyR_HEIGHT, height -1);
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPxR], osd->regVal[rISP_OVCWPxR]));
		IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPyR], osd->regVal[rISP_OVCWPyR]));
	}

	osd->outWidth = width;
	osd->outHeight = height;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;	
}

/*------------------------------------------------------------------------------

    Function name: osd_set_enable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_set_enable(isp_osd_context_t *osd/*, IM_BOOL wnd0En, IM_BOOL wnd1En*/)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);
#if 0
	if((wnd0En==IM_FALSE) && (wnd1En==IM_TRUE))
	{
		IM_ERRMSG((IM_STR("osd not support this type")));
		return IM_RET_OK;	
	}
#endif

#if 0	
	//set osd wnd0 enable or disable 
	if(wnd0En == IM_TRUE)
	{
		osd_wnd0_set_enable(osd);
	}
	else
	{
		osd_wnd0_set_disable(osd);
	}

	//set osd wnd1 enable or disable 
	if(wnd1En == IM_TRUE)
	{
		osd_wnd1_set_enable(osd);
	}
	else
	{
		osd_wnd1_set_disable(osd);
	}
#endif

	/*set osd background color*/		
	SetIspRegister(osd->regVal, ISP_OVCBCR, osd->bgColor);

	/*set osd out size*/
	SetIspRegister(osd->regVal, ISP_OVCWPxR_WIDTH, osd->outWidth - 1);
	SetIspRegister(osd->regVal, ISP_OVCWPyR_HEIGHT, osd->outHeight -1);

	/*set osd enable*/
	SetIspRegister(osd->regVal, ISP_GLB_CFG2_OSDBYPASS, 0);

	//isppwl_write_regs()
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCBCR], osd->regVal[rISP_OVCBCR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPxR], osd->regVal[rISP_OVCWPxR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_OVCWPyR], osd->regVal[rISP_OVCWPyR]));
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_GLB_CFG2], osd->regVal[rISP_GLB_CFG2]));

	osd->enable = IM_TRUE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

/*------------------------------------------------------------------------------

    Function name: osd_set_disable

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
IM_RET osd_set_disable(isp_osd_context_t *osd)
{ 
 	IM_INFOMSG((IM_STR("%s"), IM_STR(_IM_FUNC_)));
	
	IM_ASSERT(osd != IM_NULL);
	IM_ASSERT(osd->regVal != IM_NULL);
	IM_ASSERT(osd->regOfst != IM_NULL);

	if(osd->enable != IM_TRUE)
	{
		IM_INFOMSG((IM_STR("osd has been disabled")));
		return IM_RET_OK;	
	}
	//disable osd
	SetIspRegister(osd->regVal, ISP_GLB_CFG2_OSDBYPASS, 1);
	IM_JIF(isppwl_write_reg(osd->regOfst[rISP_GLB_CFG2], osd->regVal[rISP_GLB_CFG2]));

	osd->enable = IM_FALSE;
	return IM_RET_OK;
	
Fail:
	return IM_RET_FAILED;
}

