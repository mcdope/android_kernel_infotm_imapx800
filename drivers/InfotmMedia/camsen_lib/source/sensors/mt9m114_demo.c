#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"
#include "mt9m114_demo.h"

#define DBGINFO	        0  	
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"MT9M114_DEMO_I:"
#define WARNHEAD	"MT9M114_DEMO_W:"
#define ERRHEAD		"MT9M114_DEMO_E:"
#define TIPHEAD		"MT9M114_DEMO_T:"

#define MT9M114_DEMO_I2C_ADDR    0x48 
static struct mt9m114_demo_regval_list MT9M114_DEMO_ID = {
	.reg = {0x00, 0x00},
	.val = {0xFF, 0xFF},
};

typedef struct 
{
	IM_UINT32 pwdn;
	IM_UINT32 reset;

	IM_INT32 expVal;
} mt9m114_demo_context_t;

static mt9m114_demo_context_t gMt9m114_demo;
static pwl_handle_t gPwl = IM_NULL;

/*used at switching between video mode and capture mode
 *in capture mode, use 130w res, In720pRes = IM_FALSE, and no need to re-write 130w setting while taking pic
 *in video mode, use 720p res, if set video mode, set In720pRes to IM_TURE
 *if switch to capture mode from video mode, write 130w setting to sensor and set In720pRes to IM_FALSE
 */
static IM_BOOL In720pRes = IM_FALSE; 

//static IM_UINT32 CurRes = CAM_RES_130W; 
static IM_UINT32 CurRes = CAM_RES_VGA; 


#define cam_mt9m114_demo_read(b, c, d) camsenpwl_i2c_read(gPwl, c, b, 2, d)
IM_RET  cam_mt9m114_demo_write(struct mt9m114_demo_para_list* b) 
{
        //IM_INFOMSG((IM_STR("%s()++Reg: 0x%x, Val: 0x%x, Typ: %d"), IM_STR(_IM_FUNC_), b->reg, b->val, b->typ));

	IM_INT32 ret = 0;
	IM_UINT8 msg2[2], msg3[3], msg4[4];
	struct mt9m114_demo_regval_list d;

	d.reg[0] = (b->reg >> 8) & 0xff;
	d.reg[1] = b->reg & 0xff;	

	d.val[0] = (b->val >> 8) & 0xff;
	d.val[1] = b->val & 0xff;

	switch(b->typ)
	{
		case 0 : //delay time
			msleep(d.val[1]);
			break;
		case 1 : //reg 8, val 8
			msg2[0] = d.reg[1];
			msg2[1] = d.val[1];
			ret = camsenpwl_i2c_write(gPwl, (IM_UINT8*)(msg2), 2);
			break;
		case 2 : //reg 8, val 16
			msg3[0] = d.reg[1];
			msg3[1] = d.val[0];
			msg3[2] = d.val[1];
			ret = camsenpwl_i2c_write(gPwl, (IM_UINT8*)(msg3), 3);
			break;
		case 3 : //reg 16, val 8
			msg3[0] = d.reg[0];
			msg3[1] = d.reg[1];
			msg3[2] = d.val[1];
			ret = camsenpwl_i2c_write(gPwl, (IM_UINT8*)(msg3), 3);
			break;
		case 4 : //reg 16, val 16
			msg4[0] = d.reg[0];
			msg4[1] = d.reg[1];
			msg4[2] = d.val[0];
			msg4[3] = d.val[1];
			ret = camsenpwl_i2c_write(gPwl, (IM_UINT8*)(msg4), 4);
			break;
		default:
        		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

        		IM_ERRMSG((IM_STR("%s()+++++Ilegal mt9m114_demo_para_list.typ"), IM_STR(_IM_FUNC_)));
        		IM_ERRMSG((IM_STR("%s()+++++++++Reg: 0x%x, Val: 0x%x, Typ: %d"), IM_STR(_IM_FUNC_), b->reg, b->val, b->typ));
			
			return IM_RET_FAILED;

			break;
	}

        if(ret)
        {
        	IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

        	IM_ERRMSG((IM_STR("%s()+++++++++Reg: 0x%x, Val: 0x%x, Typ: %d"), IM_STR(_IM_FUNC_), b->reg, b->val, b->typ));
		
		return IM_RET_FAILED;
        }

        //IM_INFOMSG((IM_STR("%s()--Reg: 0x%x, Val: 0x%x, Typ: %d"), IM_STR(_IM_FUNC_), b->reg, b->val, b->typ));

	return IM_RET_OK;
}

static camsenpwl_pmu_info_t gPmuInfo = 
{
	2, 	//useChs
	//channels
	{
		//channel0
		{
			"iovdd",	//pwName
			2800000,	//volt(mv)
		},
		//channel1
		{
			"dvdd",		//pwName
			1800000,	//volt(mv)
		},
	},
};

IM_INT32 mt9m114_demo_switch_vga(void)
{
	IM_INT32 i, ret;
	
	for(i = 0; i < (sizeof(mt9m114_demo_vga_regs) / 6); i++)
	{
		ret = cam_mt9m114_demo_write(&(mt9m114_demo_vga_regs[i]));
		if(ret == IM_RET_FAILED)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}
 	return 0;	
}

IM_INT32 mt9m114_demo_switch_720p(void)
{
	IM_INT32 i, ret;
	
	for(i = 0; i < (sizeof(mt9m114_demo_720p_regs) / 6); i++)
	{
		ret = cam_mt9m114_demo_write(&(mt9m114_demo_720p_regs[i]));
		if(ret == IM_RET_FAILED)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret; 
		}
	}
 	return 0;	
}


IM_INT32 mt9m114_demo_switch_130w(void)
{
	IM_INT32 i, ret;
	
	for(i = 0; i < (sizeof(mt9m114_demo_130w_regs) / 6); i++)
	{
		ret = cam_mt9m114_demo_write(&(mt9m114_demo_130w_regs[i]));
		if(ret == IM_RET_FAILED)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
			return ret;
		}
	}

 	return 0;	
}




IM_INT32 mt9m114_demo_set_exposure(IM_INT32 value)
{
	IM_INFOMSG((IM_STR("%s(value = %d)"), IM_STR(_IM_FUNC_), value));

	IM_RET ret; 

	if(value == gMt9m114_demo.expVal)
	{
		return IM_RET_OK;
	}

	switch (value)
	{
		case -4:							/* EV -2 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[0]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[1]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case -3:							/* EV -1.5 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[2]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[3]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case -2:							/* EV -1 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[4]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[5]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case -1:							/* EV -0.5 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[6]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[7]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case 0:								/* EV 0 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[8]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[9]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case 1:							/* EV +0.5 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[10]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[11]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case 2:							/* EV +1 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[12]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[13]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case 3:							/* EV +1.5 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[14]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[15]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		case 4:							/* EV +2 */
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[16]));
			ret = cam_mt9m114_demo_write(&(mt9m114_demo_exposure_regs[17]));
			if (ret == IM_RET_FAILED)
			{
        			IM_ERRMSG((IM_STR("%s()+++++++++Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
        			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));

				return IM_RET_FAILED;
			}
			break;
		default:
        		IM_ERRMSG((IM_STR("%s()++++++++Unvaild Exposure Value: %d"), IM_STR(_IM_FUNC_), value));
			return IM_RET_FAILED;
	}

	gMt9m114_demo.expVal = value;

	msleep(20);

	return IM_RET_OK;
}

IM_RET mt9m114_demo_pwdn(IM_UINT32 padNum)
{
	IM_INFOMSG((IM_STR("%s(MT9M114_DEMO_PWDN pad num= %d)"), IM_STR(_IM_FUNC_), padNum));

	camsenpwl_io_set_mode(padNum, 1);
	camsenpwl_io_set_dir(padNum, 0);

	camsenpwl_io_set_outdat(padNum, 0);

	return IM_RET_OK;
}

IM_RET mt9m114_demo_get_pmu_info(camsenpwl_pmu_info_t *pmuInfo)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy(pmuInfo, (void*)&gPmuInfo, sizeof(camsenpwl_pmu_info_t));

	return IM_RET_OK;
}

IM_RET mt9m114_demo_init(pwl_handle_t pwl, IM_BOOL checkOnly)
{
        IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_INT32 i, ret;
	gPwl = pwl;

	camsenpwl_memset((void*)&gMt9m114_demo, 0x0, sizeof(gMt9m114_demo));

	gMt9m114_demo.expVal = 0;

	gMt9m114_demo.pwdn = camsenpwl_get_pwdn_padnum(gPwl);
	gMt9m114_demo.reset = camsenpwl_get_reset_padnum(gPwl);

	camsenpwl_io_set_mode(gMt9m114_demo.reset, 1);
	camsenpwl_io_set_dir(gMt9m114_demo.reset, 0);

	camsenpwl_io_set_mode(gMt9m114_demo.pwdn, 1);
	camsenpwl_io_set_dir(gMt9m114_demo.pwdn, 0);
	msleep(5);

	camsenpwl_io_set_outdat(gMt9m114_demo.reset, 0);
	msleep(10);
	camsenpwl_io_set_outdat(gMt9m114_demo.reset, 1);
	msleep(10);

	camsenpwl_io_set_outdat(gMt9m114_demo.pwdn, 0);
	msleep(20);
	
	camsenpwl_clock_enable(gPwl, 24000000);
	msleep(100);

//------- disable 115
	camsenpwl_io_set_mode(142, 1);
	camsenpwl_io_set_dir(142, 0);
	camsenpwl_io_set_outdat(142, 0);
//---------

	if(checkOnly == IM_TRUE)
	{
		cam_mt9m114_demo_read((IM_UINT8 *)(&MT9M114_DEMO_ID.reg), &MT9M114_DEMO_ID.val, 2);

	        if((MT9M114_DEMO_ID.val[0] != 0x24) || (MT9M114_DEMO_ID.val[1] != 0x81))
		{
			IM_ERRMSG((IM_STR("Mt9m114_demo id error!")));
			goto Fail;
		}
		return IM_RET_OK;
	}
	
	
	for(i = 0; i < (sizeof(mt9m114_demo_init_regs) / 6); i++)
	{
		ret = cam_mt9m114_demo_write(&(mt9m114_demo_init_regs[i]));
		if(ret == IM_RET_FAILED)
		{
			IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                	goto Fail;
		}
	}

	CurRes = CAM_RES_VGA;
	
#if 0
	ret = mt9m114_demo_switch_vga();
	if(ret == IM_RET_FAILED)
	{
		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                goto Fail;
	}
	CurRes = CAM_RES_VGA;

//#else
	ret = mt9m114_demo_switch_130w();
	if(ret == IM_RET_FAILED)
	{
		IM_ERRMSG((IM_STR("Failed to transfer data to i2c!")));
                goto Fail;
	}
	CurRes = CAM_RES_130W;
#endif

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return IM_RET_OK;

Fail:
	
	camsenpwl_io_set_outdat(gMt9m114_demo.pwdn, 0);
	camsenpwl_io_set_outdat(gMt9m114_demo.reset, 0);
	camsenpwl_clock_disable(gPwl);

	return IM_RET_FAILED;
}

IM_RET mt9m114_demo_deinit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;

	camsenpwl_io_set_outdat(gMt9m114_demo.pwdn, 0);
	camsenpwl_io_set_outdat(gMt9m114_demo.reset, 0);
	camsenpwl_clock_disable(gPwl);

	gPwl = IM_NULL;

	return ret;
}

IM_RET mt9m114_demo_start(void)
{
	IM_INFOMSG((IM_STR("%s()++"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;
	
	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_)));

	return ret;
}

IM_RET mt9m114_demo_stop(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;
	
	return ret;
}

IM_RET mt9m114_demo_get_caps(camsen_caps_t *caps)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;

	caps->supportRes = CAM_RES_VGA | CAM_RES_720P | CAM_RES_130W;
	caps->maxRes = CAM_RES_130W; 
	//caps->initRes = CAM_RES_130W;
	caps->initRes = CAM_RES_VGA;

	return ret;
}

IM_RET mt9m114_demo_get_out_mode(camsen_out_mode_t *outMode)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;

	return ret;
}

IM_RET mt9m114_demo_set_out_mode(camsen_out_mode_t *outMode)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	IM_RET ret = IM_RET_OK;
	IM_INT32 i;
	IM_UINT32 res, fps;
	res = outMode->res;
        fps = outMode->fps;	
	//printk("set out mode is 0x%x  ==============\n\n\n\n",res);
	if(res == CurRes)
	{
		IM_INFOMSG((IM_STR("it is already this resolution!")));
		return IM_RET_OK;	
	}

	if(res == CAM_RES_VGA)
	{
		//printk("114 set out mode, use vga tttttttttttt \n\n\n\n");
		ret = mt9m114_demo_switch_vga();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9m114_demo_switch_vga() failed!")));
			return IM_RET_FAILED;
		}
		CurRes = CAM_RES_VGA;
		
	}
	else if(res == CAM_RES_720P)
	{
		//printk("114 set out mode, use 720p tttttttttttt \n\n\n\n");
		ret = mt9m114_demo_switch_720p();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9m114_demo_switch_720p() failed!")));
			return IM_RET_FAILED;
		}
		CurRes = CAM_RES_720P;
	}
	else if (res == CAM_RES_130W)
	{
		//printk("114 set out mode, use 130w tttttttttttt \n\n\n\n");
		ret = mt9m114_demo_switch_130w();
		if(ret != 0)
		{
			IM_ERRMSG((IM_STR("mt9m114_demo_switch_130w() failed!")));
			return IM_RET_FAILED;
		}
		CurRes = CAM_RES_130W;
	}
	else 
	{
		IM_ERRMSG((IM_STR("this resolution(0x%x)&fps(0x%x)is not supported!"), res, fps));
		return IM_RET_FAILED; 
	}

	return ret;
}

IM_RET mt9m114_demo_set_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memcpy((void*)&value, p, sizeof(value));

//	if(property == CAM_KEY_RW_EXPOSURE_COMPENSATION)
//	{
//		mt9m114_demo_set_exposure(value);
//	}

	return ret;
}

IM_RET mt9m114_demo_get_property(IM_UINT32 property, void *p)
{
	IM_RET ret = IM_RET_OK;
	IM_INT32 value = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	switch(property)
	{
		case CAM_KEY_R_CAPS:
			value =	/*CAM_CAP_EXPOSURE |*/
				0x0;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_MAX_EXPOSURE_COMPENSATION:
			value = 4;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_MIN_EXPOSURE_COMPENSATION:
			value = -4;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_R_EXPOSURE_COMPENSATION_STEP:
			value = 1;
			memcpy(p, (void*)&value, sizeof(value));
			break;
		case CAM_KEY_RW_EXPOSURE_COMPENSATION:
			memcpy(p, (void*)&gMt9m114_demo.expVal, sizeof(IM_INT32));
			break;
		default:
			memcpy(p, (void*)&value, sizeof(value));
			break;
	}

	return ret;
}

camsen_ops mt9m114_demo_ops = {
	.name                   = "mt9m114_demo",
	.i2c_dev_addr           = MT9M114_DEMO_I2C_ADDR,

	.sen_pwdn		= mt9m114_demo_pwdn,
	.sen_get_pmu_info	= mt9m114_demo_get_pmu_info,
	.sen_init		= mt9m114_demo_init,
	.sen_deinit		= mt9m114_demo_deinit,
	.sen_start		= mt9m114_demo_start,
	.sen_stop		= mt9m114_demo_stop,
	.sen_get_caps		= mt9m114_demo_get_caps,
	.sen_set_out_mode	= mt9m114_demo_set_out_mode,
	.sen_get_out_mode	= mt9m114_demo_get_out_mode,
	.sen_set_property	= mt9m114_demo_set_property,
	.sen_get_property	= mt9m114_demo_get_property,
};
