/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file isp_drv.c
--
--  Description :
--		This file is isp linux driver.
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/04/18: first commit.
-- v1.0.2	arsor@2012/05/07: base flow is OK.
-- v1.0.3	arsor@2012/07/16: change sensor pmu manager to pwl layer.
--
------------------------------------------------------------------------------*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/mman.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>

#include <mach/power-gate.h>
#include <mach/pad.h>
#include <mach/items.h>

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <camsen_lib.h>
#include <camisp_api.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"ISPDRV_I:"
#define WARNHEAD	"ISPDRV_W:"
#define ERRHEAD		"ISPDRV_E:"
#define TIPHEAD		"ISPDRV_T:"

#define TIME_STATS	0

#define MAX_ITEMS   (2) //current is front&back

typedef struct{
	IM_UINT32 facing;
	IM_INT32 interface;
	IM_UINT32 dataType;
	IM_INT32 orientation;
	IM_TCHAR model[64];
}ispdrv_items_config_t;

typedef struct
{
    IM_INT32 connectsNum;   //number of camera sensor connect to isp interface
    ispdrv_items_config_t itemsCfg[MAX_ITEMS];
}ispdrv_init_info_t;

static ispdrv_init_info_t ginitInfo;

static cam_module_class_t gModuleClass;
static struct mutex gDrvlock;
static int gDrvProbed = 0;
static int gDrvOpened = 0;
static int gDrvModuleEn = 0;
static int gCamIspInited = 0;
static int gCamInterface = CAM_INTERFACE_UNKNOWN;
static camdev_config_t gCamIspConfig;

static struct clk *ispOsdClk = IM_NULL;
static struct clk *ispBusClk = IM_NULL;
//clock used for mipi-csi2
static struct clk *csiBusClk = IM_NULL;
static struct clk *dphyCfgClk = IM_NULL;
static struct clk *mipiPixClk = IM_NULL;

static unsigned int ispBusClkFrq = 0;
static unsigned int ispOsdClkFrq = 0;
static unsigned int mipiPixClkFrq = 0;

/* 
 * Function declaration.
 */
static int ispdrv_open(struct inode *inode, struct file *file);
static int ispdrv_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int ispdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ispdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static unsigned int ispdrv_poll(struct file *file, poll_table *wait);
static int ispdrv_probe(struct platform_device *pdev);
static int ispdrv_remove(struct platform_device *pdev);
static int ispdrv_suspend(struct platform_device *pdev, pm_message_t state);
static int ispdrv_resume(struct platform_device *pdev);


static int ispdrv_camsen_set_pwdn(void);
static int ispdrv_camsen_get_info(void *camsenPmuHand);
static int ispdrv_camsen_get_type(camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex);
static int ispdrv_module_enable(void);
static int ispdrv_module_disable(void);
static void ispdrv_module_reset(void);
static int ispdrv_stub(void);


static struct file_operations isp_fops = {
	.owner = THIS_MODULE,
	.open = ispdrv_open,
	.release = ispdrv_release,
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
    .unlocked_ioctl = ispdrv_ioctl,
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
	.ioctl = ispdrv_ioctl,
#endif
	.poll = ispdrv_poll,
};

static struct miscdevice isp_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &isp_fops,
	.name = "isp",
};

static int ispdrv_open(struct inode *inode, struct file *file)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	//
	mutex_lock(&gDrvlock);

	if(gDrvOpened != 0){
		IM_ERRMSG((IM_STR("Don't support multi-instance")));
		goto Fail;
	}

	gDrvOpened = 1;
	mutex_unlock(&gDrvlock);
	return 0;
Fail:
	mutex_unlock(&gDrvlock);
	return -1;
}

static int ispdrv_release(struct inode *inode, struct file *file)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	mutex_lock(&gDrvlock);
	IM_ASSERT(gDrvOpened == 1);

	if(gCamIspInited == 1)
	{
		ret = camisp_deinit(); 
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("camisp_deinit() failed")));
		}
		gCamIspInited = 0;
	}
	gDrvOpened = 0;

    mutex_unlock(&gDrvlock);
	return 0;
}


#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int ispdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ispdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	void *p;
	IM_RET ret = IM_RET_OK;
	cam_preview_config_t caps;
	ioctl_ds_set_config cfg;
	ioctl_ds_key_property pro;
	ioctl_ds_send_control sendCtrl;
	ioctl_ds_cancel_control cancelCtrl;
	workmodel_event_t evt;
	IM_Buffer buff;
	IM_INT32 modClsId;
	IM_INT32 boxIndex;
	IM_INT32 moduleIndex;


    IM_UINT32 pro_value = 0; 

	IM_INFOMSG((IM_STR("%s(cmd=0x%x)++"), IM_STR(_IM_FUNC_), cmd));

	switch(cmd){
		case CAMISP_IOCTL_CMD_GET_MODULES:
			IM_INFOMSG((IM_STR("%s(get modules)"), IM_STR(_IM_FUNC_)));
			copy_to_user((void*)arg, (void*)&gModuleClass, sizeof(cam_module_class_t));
			break;
		case CAMISP_IOCTL_CMD_OPEN:
			__get_user(modClsId, (IM_INT32 *)arg);
			IM_INFOMSG((IM_STR("%s(open: modClsId=0x%x)"), IM_STR(_IM_FUNC_), modClsId));
			moduleIndex = modClsId&0xffff;
			boxIndex = (modClsId&0xffff0000)>>16;
			//get sensorId form boxIndex and moduleIndex
			if(ispdrv_camsen_get_type(&gCamIspConfig, boxIndex, moduleIndex) == -1)
			{
				IM_ERRMSG((IM_STR("ispdrv_get_sensor_name() failed")));
                return -EINVAL;
			}

            //isp module enable
            if(gDrvModuleEn == 0)
            {
                if(ispdrv_module_enable() != 0)
                {
                    IM_ERRMSG((IM_STR("ispdrv_module_enable() failed")));
                    return -EINVAL;
                }
                gDrvModuleEn = 1;
            }

			ret = camisp_init(&gCamIspConfig);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("camisp_init() failed")));
                return -EINVAL;
			}
			gCamIspInited = 1;
			break;
		case CAMISP_IOCTL_CMD_CLOSE:
			IM_INFOMSG((IM_STR("%s(close)"), IM_STR(_IM_FUNC_)));
			ret = camisp_deinit(); 
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("camisp_deinit() failed")));
			}
			gCamIspInited = 0;

            //isp module disable
            if(gDrvModuleEn == 1)
            {
                ispdrv_module_disable();
                gDrvModuleEn = 0;
            }

            gCamInterface = CAM_INTERFACE_UNKNOWN;
			break;
		case CAMISP_IOCTL_CMD_SET_CFG:
			IM_INFOMSG((IM_STR("%s(set config)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&cfg, (void*)arg, sizeof(cfg));
			p = (void*)&cfg;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_GET_CFG:
			IM_INFOMSG((IM_STR("%s(get config)"), IM_STR(_IM_FUNC_)));
			p = (void*)&caps;
			ret = camisp_ioctl(cmd, p);
			if(ret == IM_RET_OK)
			{
				copy_to_user((void*)arg, p, sizeof(caps));
			}
			break;
		case CAMISP_IOCTL_CMD_SET_PRO:
			IM_INFOMSG((IM_STR("%s(set property)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&pro, (void*)arg, sizeof(pro));
			p = (void*)&pro;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_GET_PRO:
			IM_INFOMSG((IM_STR("%s(get property)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&pro, (void*)arg, sizeof(pro));
			p = (void*)&pro;
			ret = camisp_ioctl(cmd, p);
			if(ret == IM_RET_OK)
			{
				copy_to_user((void*)arg, p, sizeof(ioctl_ds_key_property));
			}
			break;
		case CAMISP_IOCTL_CMD_ASSIGN_BUF:
			IM_INFOMSG((IM_STR("%s(assign buffer)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&buff, (void*)arg, sizeof(buff));
			p = (void*)&buff;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_RELEASE_BUF:
			IM_INFOMSG((IM_STR("%s(release buffer)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&buff, (void*)arg, sizeof(buff));
			p = (void*)&buff;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_SEND_CTR:
			IM_INFOMSG((IM_STR("%s(send ctrl)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&sendCtrl, (void*)arg, sizeof(sendCtrl));
			p = (void*)&sendCtrl;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_CANCEL_CTR:
			IM_INFOMSG((IM_STR("%s(cancel ctrl)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&cancelCtrl, (void*)arg, sizeof(cancelCtrl));
			p = (void*)&cancelCtrl;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_START:
			IM_INFOMSG((IM_STR("%s(start)"), IM_STR(_IM_FUNC_)));
			p = IM_NULL;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_STOP:
			IM_INFOMSG((IM_STR("%s(stop)"), IM_STR(_IM_FUNC_)));
			p = IM_NULL;
			ret = camisp_ioctl(cmd, p);
			break;
		case CAMISP_IOCTL_CMD_GET_EVENT:
			IM_INFOMSG((IM_STR("%s(get event)"), IM_STR(_IM_FUNC_)));
			ret = camisp_get_event(&evt);
			if(ret == IM_RET_OK)
			{
				copy_to_user((void*)arg, (void*)&evt, sizeof(evt));
			}
			break;
		default:
			IM_ERRMSG((IM_STR("%s(This cmd is not support: cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));
			return -EINVAL;
	}

	IM_INFOMSG((IM_STR("%s()--"), IM_STR(_IM_FUNC_), cmd));
	return (ret==IM_RET_OK)?0:-EFAULT;
}

static unsigned int ispdrv_poll(struct file *file, poll_table *wait)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
#if TIME_STATS
	struct timeval _s, _e;
	IM_INT64 time;
	do_gettimeofday(&_s);
	IM_TIPMSG((IM_STR(" wait_event begin time.sec=%ds,time.usec=%d++ \n"),_s.tv_sec, _s.tv_usec));
#endif
	ret = camisp_wait_event();
#if TIME_STATS
	do_gettimeofday(&_e);
	IM_TIPMSG((IM_STR(" wait_event end time.sec=%ds,time.usec=%d-- \n"),_e.tv_sec, _e.tv_usec));
	time = (_e.tv_sec*1000000+_e.tv_usec - (_s.tv_sec*1000000+_s.tv_usec))/1000;
	IM_TIPMSG((IM_STR(" wait_event time=%lldms - \n"),time));
#endif
	if(ret != IM_RET_OK)
	{
		IM_WARNMSG((IM_STR("%s(camisp_wait_event timeout or error!)"), IM_STR(_IM_FUNC_)));
		return 0;
	}
	
	return POLLIN | POLLRDNORM;
}

static struct platform_driver isp_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-isp",
	},
	.probe = ispdrv_probe,
	.remove = ispdrv_remove,
	.suspend = ispdrv_suspend,
	.resume = ispdrv_resume,
};


static int ispdrv_probe(struct platform_device *pdev)
{
	char str[128] = "imapx820"; //default cpu type
    int ret = 0;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //get cpu type, it means imapx820 if no cpu info at items file
	if(item_exist("board.cpu")){
		item_string(str,"board.cpu",0);
	}

    //only imapx820 support isp now
    if(strcmp(str,"imapx820") != 0){
        IM_ERRMSG((IM_STR("this cpu not support isp!!!")));
        return -EINVAL;
    }

    //It maybe error when suspend/resume if this function not call, so it must be call. I don't know why?????
    ispdrv_stub();

	mutex_init(&gDrvlock);

	ret = ispdrv_camsen_get_info((void *)&pdev->dev);
    if(ret != 0)
    {
		IM_ERRMSG((IM_STR(" ispdrv_camsen_get_info() failed, %d."),ret));
        mutex_destroy(&gDrvlock);
        return ret;
    }

	/* Register misc device driver. */
	ret = misc_register(&isp_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register isp misc device driver error, %d."),ret));
		return ret;
	}

    gDrvProbed = 1;

	return 0;
}

static int ispdrv_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(gDrvProbed == 1)
    {
        misc_deregister(&isp_miscdev);

        mutex_destroy(&gDrvlock);
    }

    gDrvProbed = 0;

	return 0;
}

static int ispdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //isp module disable
    if(gDrvModuleEn == 1)
    {
        ispdrv_module_disable();
        gDrvModuleEn = 0;
    }

	return 0;
}

static int ispdrv_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//set all isp io mode to function mode	
	imapx_pad_cfg(IMAPX_CAM, PULL_ENABLE);
	
	ispdrv_camsen_set_pwdn();

    //isp module enable
    if((gDrvModuleEn == 0) && (gCamIspInited = 0))
    {
        IM_TIPMSG((IM_STR("%s(), Eenable isp module at resume!"), IM_STR(_IM_FUNC_)));
        if(ispdrv_module_enable() != 0)
        {
            IM_ERRMSG((IM_STR("ispdrv_module_enable() failed")));
            return -EINVAL;
        }
        gDrvModuleEn = 1;
    }

	return 0;
}

//==========================================================================================================
static int ispdrv_camsen_set_pwdn(void)
{
	char moduleName[128];
	IM_UINT32 facing;
	IM_INT32 i, j;
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	for(i=0; i<gModuleClass.boxNum; i++)
	{
		for(j=0; j<gModuleClass.boxes[i].moduleNum; j++)
		{
			strcpy(moduleName, gModuleClass.boxes[i].modules[j].desc);
			facing = gModuleClass.boxes[i].modules[j].facing;
			ret = camsen_pwdn(moduleName, facing);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("camsen_pwdn() failed")));
				return -1;
			}
		}
	}

	return 0;
}

static int ispdrv_camsen_get_info(void *camsenPmuHand)
{
	int i;
	char str[128];
	IM_INT32 moduleNum = 0;
	IM_UINT32 facing;
	camsen_handle_t handle;
	camsen_caps_t caps;
	IM_RET ret = IM_RET_OK;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memset((void*)&gModuleClass, 0, sizeof(cam_module_class_t));

	memset((void*)&ginitInfo, 0, sizeof(ispdrv_init_info_t));

    //
    //get items configs
    //
    /*front camera*/
	if(item_exist("camera.front.interface")){
        ginitInfo.itemsCfg[ginitInfo.connectsNum].facing = CAM_MODULE_FACING_FRONT;

        if(item_exist("camera.front.orientation")){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].orientation = item_integer("camera.front.orientation",0);
        }

        if(item_exist("camera.front.model")){
            item_string(ginitInfo.itemsCfg[ginitInfo.connectsNum].model,"camera.front.model",0);
        }

        if(item_exist("camera.front.data_type")){
            item_string(str,"camera.front.data_type",0);
            if(strcmp(str,"low8_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_LOW8_ALIGN;
            }else if(strcmp(str,"mid8_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_MID8_ALIGN;
            }else if(strcmp(str,"low10_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_LOW10_ALIGN;
            }else{
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_HIGH_ALIGN;
            }
        }

		item_string(str,"camera.front.interface",0);
		if(strcmp(str,"isp") == 0){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].interface = CAM_INTERFACE_ISP_DVP;
            ginitInfo.connectsNum++;
        }else if(strcmp(str,"isp_mipi") == 0){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].interface = CAM_INTERFACE_ISP_MIPI;
            ginitInfo.connectsNum++;
        }
	}

    /*rear camera*/
	if(item_exist("camera.rear.interface")){
        ginitInfo.itemsCfg[ginitInfo.connectsNum].facing = CAM_MODULE_FACING_BACK;

        if(item_exist("camera.rear.orientation")){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].orientation = item_integer("camera.rear.orientation",0);
        }

        if(item_exist("camera.rear.model")){
            item_string(ginitInfo.itemsCfg[ginitInfo.connectsNum].model,"camera.rear.model",0);
        }

        if(item_exist("camera.rear.data_type")){
            item_string(str,"camera.rear.data_type",0);
            if(strcmp(str,"low8_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_LOW8_ALIGN;
            }else if(strcmp(str,"mid8_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_MID8_ALIGN;
            }else if(strcmp(str,"low10_align")==0){
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_LOW10_ALIGN;
            }else{
                ginitInfo.itemsCfg[ginitInfo.connectsNum].dataType = CAM_DATATYPE_HIGH_ALIGN;
            }
        }

		item_string(str,"camera.rear.interface",0);
		if(strcmp(str,"isp") == 0){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].interface = CAM_INTERFACE_ISP_DVP;
            ginitInfo.connectsNum++;
        }else if(strcmp(str,"isp_mipi") == 0){
            ginitInfo.itemsCfg[ginitInfo.connectsNum].interface = CAM_INTERFACE_ISP_MIPI;
            ginitInfo.connectsNum++;
        }
	}

    if(ginitInfo.connectsNum == 0)
    {
        IM_ERRMSG((IM_STR("no isp interface list at items file!")));
        return -EINVAL;
    }

	//set all isp io mode to function mode	
	imapx_pad_cfg(IMAPX_CAM, PULL_ENABLE);

	camsen_set_pmu_handle(camsenPmuHand);

	//power down all front&rear camera sensor first
	if(item_exist("camera.front.interface")){
		item_string(str,"camera.front.interface",0);
		IM_INFOMSG((IM_STR("camera front interface type = %s "),str));
		if(strcmp(str,"isp") == 0){
			if(item_exist("camera.front.model")){
				item_string(str,"camera.front.model",0);
				//power down front sensor
				ret = camsen_pwdn(str, CAM_MODULE_FACING_FRONT);
				if(ret != IM_RET_OK){
					IM_ERRMSG((IM_STR("camsen_pwdn() failed")));
				}
			}
		}
	}

	if(item_exist("camera.rear.interface")){
		item_string(str,"camera.rear.interface",0);
		IM_INFOMSG((IM_STR("camera rear interface type = %s "),str));
		if(strcmp(str,"isp") == 0){
			if(item_exist("camera.rear.model")){
				item_string(str,"camera.rear.model",0);
				//power down rear sensor
				ret = camsen_pwdn(str, CAM_MODULE_FACING_BACK);
				if(ret != IM_RET_OK){
					IM_ERRMSG((IM_STR("camsen_pwdn() failed")));
				}
			}
		}
	}

	//check camera sensor connect & get some sensor infos
	for(i=0; i<2; i++)
	{
		if(i == 0)//front
		{
			facing = CAM_MODULE_FACING_FRONT;
			if(item_exist("camera.front.interface")){
				item_string(str,"camera.front.interface",0);
				IM_INFOMSG((IM_STR("camera front interface type = %s "),str));
				if(((strcmp(str,"isp")==0) || (strcmp(str,"isp_mipi")==0)) && item_exist("camera.front.model")){
					item_string(str,"camera.front.model",0);
				}else{
					continue;
				}
			}else{
				continue;
			}
		}
		else
		{
			facing = CAM_MODULE_FACING_BACK;
			if(item_exist("camera.rear.interface")){
				item_string(str,"camera.rear.interface",0);
				IM_INFOMSG((IM_STR("camera rear interface type = %s "),str));
				if(((strcmp(str,"isp")==0) || (strcmp(str,"isp_mipi")==0)) && item_exist("camera.rear.model")){
					item_string(str,"camera.rear.model",0);
				}else{
					continue;
				}
			}else{
				continue;
			}
		}
		ret = camsen_init(&handle, str, facing, IM_TRUE);
		if(ret == IM_RET_OK){
			IM_INFOMSG((IM_STR("%s(check id success!)"), IM_STR(_IM_FUNC_)));

			memset((void*)&caps, 0, sizeof(camsen_caps_t));
			camsen_get_caps(handle, &caps);

			strcpy(gModuleClass.boxes[0].modules[moduleNum].desc, str);
			gModuleClass.boxes[0].modules[moduleNum].id = i;
			gModuleClass.boxes[0].modules[moduleNum].resolution = caps.maxRes;
			//facing	
			gModuleClass.boxes[0].modules[moduleNum].facing = facing;
			//orientation		
			if(i==0)
			{
				gModuleClass.boxes[0].modules[moduleNum].orientation = item_integer("camera.front.orientation",0);
			}else{
				gModuleClass.boxes[0].modules[moduleNum].orientation = item_integer("camera.rear.orientation",0);
			}
			camsen_deinit(handle);

			moduleNum++;
			gModuleClass.boxes[0].moduleNum = moduleNum;
		}
		else
		{
			IM_ERRMSG((IM_STR("camsen_init() failed")));
		}
	}

	//isp can only support one box
	gModuleClass.boxNum = 1;

	IM_INFOMSG((IM_STR(" gModuleClass.boxNum = %d"), gModuleClass.boxNum));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0]:")));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum=%d\n"), gModuleClass.boxes[0].moduleNum));

	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0]:")));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0].desc=%s"), gModuleClass.boxes[0].modules[0].desc));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0].id=%d"), gModuleClass.boxes[0].modules[0].id));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0].facing=%d"), gModuleClass.boxes[0].modules[0].facing));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0].orientation=%d"), gModuleClass.boxes[0].modules[0].orientation));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[0].resolution=0x%x\n"), gModuleClass.boxes[0].modules[0].resolution));

	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1]: \n")));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1].desc=%s"), gModuleClass.boxes[0].modules[1].desc));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1].id=%d"), gModuleClass.boxes[0].modules[1].id));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1].facing=%d"), gModuleClass.boxes[0].modules[1].facing));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1].orientation=%d"), gModuleClass.boxes[0].modules[1].orientation));
	IM_INFOMSG((IM_STR(" gModuleClass.boxNum[0].moduleNum[1].resolution=0x%x\n"), gModuleClass.boxes[0].modules[1].resolution));

	return 0;
}

static int ispdrv_camsen_get_type(camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex)
{
	char str[128];
    int i;
	if(boxIndex >= gModuleClass.boxNum)
	{
		IM_ERRMSG((IM_STR("boxIndex error")));
		return -1;
	}
	if(moduleIndex >= gModuleClass.boxes[boxIndex].moduleNum) 
	{
		IM_ERRMSG((IM_STR("moduleIndex error")));
		return -1;
	}

	strcpy(config->camsenName, gModuleClass.boxes[boxIndex].modules[moduleIndex].desc); 
	config->facing = gModuleClass.boxes[boxIndex].modules[moduleIndex].facing;
	IM_INFOMSG((IM_STR("(%s: facing=%d)"), config->camsenName, config->facing));

    for(i=0;i<ginitInfo.connectsNum;i++)
    {
       if(ginitInfo.itemsCfg[i].facing == config->facing)
       {
           config->dataType = ginitInfo.itemsCfg[i].dataType;
           gCamInterface = ginitInfo.itemsCfg[i].interface;
		   if(gCamInterface == CAM_INTERFACE_ISP_MIPI)
		   {
			   config->itfType = CAMISP_INTERFACE_MIPI;
		   }
		   else
		   {
			   config->itfType = CAMISP_INTERFACE_DVP;
		   }
           break;
       }
    }

    if(i == ginitInfo.connectsNum)
	{
		IM_ERRMSG((IM_STR("sensor module index error!")));
		return -1;
	}
#if 0
	if(config->facing == CAM_MODULE_FACING_FRONT)
	{
        config->dataType = CAM_DATATYPE_LOW8_ALIGN;//default value
        if(item_exist("camera.front.data_type")){
            item_string(str,"camera.front.data_type",0);
            if(strcmp(str,"low8_align")==0){
                config->dataType = CAM_DATATYPE_LOW8_ALIGN;
            }else if(strcmp(str,"mid8_align")==0){
                config->dataType = CAM_DATATYPE_MID8_ALIGN;
            }else if(strcmp(str,"low10_align")==0){
                config->dataType = CAM_DATATYPE_LOW10_ALIGN;
            }else{
                config->dataType = CAM_DATATYPE_HIGH_ALIGN;
            }
        }
        IM_INFOMSG((IM_STR("open front sensor(%s: dataType=%d)"), config->camsenName, config->dataType));
	}
	else if(config->facing == CAM_MODULE_FACING_BACK)
	{
        config->dataType = CAM_DATATYPE_LOW8_ALIGN;//default value
        if(item_exist("camera.rear.data_type")){
            item_string(str,"camera.rear.data_type",0);
            if(strcmp(str,"low8_align")==0){
                config->dataType = CAM_DATATYPE_LOW8_ALIGN;
            }else if(strcmp(str,"mid8_align")==0){
                config->dataType = CAM_DATATYPE_MID8_ALIGN;
            }else if(strcmp(str,"low10_align")==0){
                config->dataType = CAM_DATATYPE_LOW10_ALIGN;
            }else{
                config->dataType = CAM_DATATYPE_HIGH_ALIGN;
            }
        }
		IM_INFOMSG((IM_STR("open rear sensor(%s: dataType=%d)"), config->camsenName, config->dataType));
	}
	else
	{
		IM_ERRMSG((IM_STR("sensor name error!")));
		return -1;
	}
#endif
	return 0;
}


static int ispdrv_module_enable()
{
	IM_UINT32 res;
	struct clk *mipiDphyCfgClk = IM_NULL;

	module_power_on(SYSMGR_ISP_BASE);
	
    //isp osd clock
	ispOsdClk = clk_get(NULL, "isp-osd");
	if(ispOsdClk == IM_NULL)
	{
		IM_ERRMSG((IM_STR("get isp osd clock failed!")));
		return -1;
	}

	res = clk_set_rate(ispOsdClk, 266000); //266MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("isp osd clk set rate failed! ")));
		goto Fail;
	}
	ispOsdClkFrq = (IM_UINT32)clk_get_rate(ispOsdClk);

	if(clk_enable(ispOsdClk) != 0)
	{
		IM_ERRMSG((IM_STR("isp osd clock enable failed!")));
		goto Fail;
	}

    //isp bus clock(bus5)
	ispBusClk = clk_get(NULL, "isp");
	if(ispBusClk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get isp bus clock failed!")));
		clk_disable(ispOsdClk);
		goto Fail;
	}

	// 160 MHz should be enough, and reduce it from 300MHz to 160MHz inorder to make IDS more fluent.
	res = clk_set_rate(ispBusClk, 160000); //160MHz, orginal 266MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("isp bus clk set rate failed! ")));
		clk_disable(ispOsdClk);
		goto Fail;
	}
	ispBusClkFrq = (IM_UINT32)clk_get_rate(ispBusClk);

	if(clk_enable(ispBusClk) != 0)
	{
		IM_ERRMSG((IM_STR("isp bus clock enable failed!")));
		clk_disable(ispOsdClk);
		goto Fail;
	}

    if(gCamInterface == CAM_INTERFACE_ISP_MIPI)
    {
        module_power_on(SYSMGR_MIPI_BASE);

#if 0 //it is also bus5 clk, we set ispBusClk(bus5 clk) before, so here we not need set it again
        csiBusClk = clk_get(NULL, "mipi-csi");
        if(csiBusClk ==  IM_NULL)
        {
            IM_ERRMSG((IM_STR("get isp mipi clock failed!")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            goto Fail;
        }

        res = clk_set_rate(csiBusClk, 266000); //266MHz
        if (res < 0){
            IM_ERRMSG((IM_STR("isp mipi clk set rate failed! ")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            goto Fail;
        }

        if(clk_enable(csiBusClk) != 0)
        {
            IM_ERRMSG((IM_STR("isp mipi clock enable failed!")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            goto Fail;
        }
#endif

		//dphyCfgClk also used in mipi-dsi, so only enable here, not disable
		dphyCfgClk = clk_get(NULL, "mipi-dphy-con");
		if (dphyCfgClk == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-con) failed")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            //clk_disable(csiBusClk);
            goto Fail;
		}
        res = clk_set_rate(dphyCfgClk,27000); 
        if (res < 0){
            IM_ERRMSG((IM_STR(" clk_set_rate(mipi-dphy-con) failed - ")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            //clk_disable(csiBusClk);
            goto Fail;
        }
		clk_enable(dphyCfgClk);

		//
		//NOTE important: mipiPixClk must larger than twice of mipi sensor out pixclk
		//mpiPixClk only used for mipi-csi2
		mipiPixClk = clk_get(NULL, "mipi-dphy-pixel");
		if (mipiPixClk == NULL) {
			IM_ERRMSG((IM_STR("clk_get(mipi-dphy-pixel) failed")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            //clk_disable(csiBusClk);
            goto Fail;
		}
        //here we set 96MHz, so mipi sensor out pixclk must less than 45MHz,
		//in fact we find the range of mipiPixClk must be right value depend on sensor out pixclk,
		//so it can not be any value as we like, also can not be a big value(maybe it is a bug).
        res = clk_set_rate(mipiPixClk, 90000);  
        if (res < 0){
            IM_ERRMSG((IM_STR(" clk_set_rate(mipi-dphy-pixel) failed - ")));
            clk_disable(ispOsdClk);
            clk_disable(ispBusClk);
            //clk_disable(csiBusClk);
            goto Fail;
        }
		clk_enable(mipiPixClk);
    }

    IM_INFOMSG((IM_STR("isp ispBusClk frq= %d, isp ispOsdClk frq=%d, mipiPixClk frq=%d!"), ispBusClkFrq, ispOsdClkFrq, mipiPixClkFrq));

    // module reset
    ispdrv_module_reset();

	return 0;
Fail:
	if(ispOsdClk != IM_NULL)
	{
		clk_put(ispOsdClk);
		ispOsdClk = IM_NULL;
	}

	if(ispBusClk != IM_NULL)
	{
		clk_put(ispBusClk);
		ispBusClk = IM_NULL;
	}

#if 0
	if(csiBusClk != IM_NULL)
	{
		clk_put(csiBusClk);
		csiBusClk = IM_NULL;
	}
#endif

	return -1;
}

static int ispdrv_module_disable(void)
{
	if(ispOsdClk != IM_NULL)
	{
		clk_disable(ispOsdClk);
		clk_put(ispOsdClk);
		ispOsdClkFrq = 0;
		ispOsdClk = IM_NULL;
	}

	if(ispBusClk != IM_NULL)
	{
		clk_disable(ispBusClk);
		clk_put(ispBusClk);
		ispBusClkFrq = 0;
		ispBusClk = IM_NULL;
	}

    if(gCamInterface == CAM_INTERFACE_ISP_MIPI)
    {
		if(mipiPixClk != IM_NULL)
		{
			clk_disable(mipiPixClk);
			clk_put(mipiPixClk);
			mipiPixClkFrq = 0;
			mipiPixClk = IM_NULL;
		}
#if 0
        if(csiBusClk != IM_NULL)
        {
            clk_disable(csiBusClk);
            clk_put(csiBusClk);
            csiBusClk = IM_NULL;
        }
#endif
		module_power_down(SYSMGR_MIPI_BASE);
    }

    module_power_down(SYSMGR_ISP_BASE);

    return 0;
}

void ispdrv_module_reset(void)
{
	module_reset(SYSMGR_ISP_BASE, 1);
}

static int ispdrv_stub(void)
{
	ispBusClk = clk_get(NULL, "isp");
	if(ispBusClk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get isp bus clock failed!")));
        return -1;
	}

	if(clk_enable(ispBusClk) != 0)
	{
		IM_ERRMSG((IM_STR("isp bus clock enable failed!")));
        return -1;
	}

	if(ispBusClk != IM_NULL)
	{
		clk_disable(ispBusClk);
		clk_put(ispBusClk);
		ispBusClkFrq = 0;
		ispBusClk = IM_NULL;
	}
    return 0;
}

int camera_pcb_get_modules(cam_module_class_t *moduleClass)
{
    if(moduleClass == IM_NULL)
    {
        return -1;
    }
    memcpy((void*)moduleClass, (void*)&gModuleClass, sizeof(cam_module_class_t));
    
    return 0;
}

EXPORT_SYMBOL(camera_pcb_get_modules);                                       

static int __init ispdrv_init(void)
{
	int ret = 0;
	printk("####ispdrv init####\n");
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //register all camera sensors on supported list
    camsen_register_all();

	/* Register platform device driver. */
	ret = platform_driver_register(&isp_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register isp platform device driver error, %d."),ret));
		return ret;
	}

	return 0;
}

static void __exit ispdrv_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	platform_driver_unregister(&isp_plat);
}

//module_init(ispdrv_init);
late_initcall(ispdrv_init);
module_exit(ispdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arsor of InfoTM");
MODULE_DESCRIPTION("isp driver");
