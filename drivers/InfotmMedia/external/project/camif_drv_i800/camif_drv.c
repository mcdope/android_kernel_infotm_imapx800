/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camif_drv.c
--
--  Description :
--		This file is camif linux driver.
--
--	Author:
--  	Jimmy Shu   <jimmy.shu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	Jimmy@2012/10/16: first commit.
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
#include <camif_api.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMIFDRV_I:"
#define WARNHEAD	"CAMIFDRV_W:"
#define ERRHEAD		"CAMIFDRV_E:"
#define TIPHEAD		"CAMIFDRV_T:"

#define TIME_STATS	0

static cam_module_class_t gModuleClass;
static struct mutex gDrvlock;
static int gDrvProbed = 0;
static int gDrvOpened = 0;
static int gDrvModuleEn = 0;
static int gCamIfInited = 0;
static camif_camdev_config_t gCamIfConfig;

static struct clk *busclk = IM_NULL;
static unsigned int busclkFrq = 0;

/* 
 * Function declaration.
 */
static int camifdrv_open(struct inode *inode, struct file *file);
static int camifdrv_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int camifdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int camifdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static unsigned int camifdrv_poll(struct file *file, poll_table *wait);
static int camifdrv_probe(struct platform_device *pdev);
static int camifdrv_remove(struct platform_device *pdev);
static int camifdrv_suspend(struct platform_device *pdev, pm_message_t state);
static int camifdrv_resume(struct platform_device *pdev);


static int camifdrv_camsen_set_pwdn(void);
static int camifdrv_camsen_get_info(void *camsenPmuHand);
static int camifdrv_camsen_get_type(camif_camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex);
static int camifdrv_module_enable(void);
static int camifdrv_module_disable(void);
static void camifdrv_module_reset(void);
static int camifdrv_stub(void);


static struct file_operations camif_fops = {
	.owner = THIS_MODULE,
	.open = camifdrv_open,
	.release = camifdrv_release,
#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
    .unlocked_ioctl = camifdrv_ioctl,
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
	.ioctl = camifdrv_ioctl,
#endif
	.poll = camifdrv_poll,
};

static struct miscdevice camif_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &camif_fops,
	.name = "camif",
};

static int camifdrv_open(struct inode *inode, struct file *file)
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

static int camifdrv_release(struct inode *inode, struct file *file)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	mutex_lock(&gDrvlock);
	IM_ASSERT(gDrvOpened == 1);

	if(gCamIfInited == 1)
	{
		ret = camif_deinit(); 
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("camif_deinit() failed")));
		}
		gCamIfInited = 0;
	}
	gDrvOpened = 0;

	mutex_unlock(&gDrvlock);
	return 0;
}


#if (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0)
static int camifdrv_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int camifdrv_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	void *p;
	IM_RET ret = IM_RET_OK;
	cam_preview_config_t caps;
	camif_ioctl_ds_set_config cfg;
	camif_ioctl_ds_key_property pro;
	camif_ioctl_ds_send_control sendCtrl;
	camif_ioctl_ds_cancel_control cancelCtrl;
	camif_workmodel_event_t evt;
	IM_Buffer buff;
	IM_INT32 modClsId;
	IM_INT32 boxIndex;
	IM_INT32 moduleIndex;


    IM_UINT32 pro_value = 0; 

	IM_INFOMSG((IM_STR("%s(cmd=0x%x)++"), IM_STR(_IM_FUNC_), cmd));

	switch(cmd){
		case CAMIF_IOCTL_CMD_GET_MODULES:
			IM_INFOMSG((IM_STR("%s(get modules)"), IM_STR(_IM_FUNC_)));
			copy_to_user((void*)arg, (void*)&gModuleClass, sizeof(cam_module_class_t));
			break;
		case CAMIF_IOCTL_CMD_OPEN:
            //camif module enable
            if(gDrvModuleEn == 0)
            {
                if(camifdrv_module_enable() != 0)
                {
                    IM_ERRMSG((IM_STR("camifdrv_module_enable() failed")));
                    return -EINVAL;
                }
                gDrvModuleEn = 1;
            }

            __get_user(modClsId, (IM_INT32 *)arg);
            IM_INFOMSG((IM_STR("%s(open: modClsId=0x%x)"), IM_STR(_IM_FUNC_), modClsId));
			moduleIndex = modClsId&0xffff;
			boxIndex = (modClsId&0xffff0000)>>16;
			//get sensorId form boxIndex and moduleIndex
			if(camifdrv_camsen_get_type(&gCamIfConfig, boxIndex, moduleIndex) == -1)
			{
				IM_ERRMSG((IM_STR("camifdrv_get_sensor_name() failed")));
			}

			ret = camif_init(&gCamIfConfig);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("camif_init() failed")));
			}
			gCamIfInited = 1;
			break;
		case CAMIF_IOCTL_CMD_CLOSE:
			IM_INFOMSG((IM_STR("%s(close)"), IM_STR(_IM_FUNC_)));
			ret = camif_deinit(); 
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("camif_deinit() failed")));
			}
			gCamIfInited = 0;

            //camif module disable
            if(gDrvModuleEn == 1)
            {
                camifdrv_module_disable();
                gDrvModuleEn = 0;
            }
			break;
		case CAMIF_IOCTL_CMD_SET_CFG:
			IM_INFOMSG((IM_STR("%s(set config)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&cfg, (void*)arg, sizeof(cfg));
			p = (void*)&cfg;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_GET_CFG:
			IM_INFOMSG((IM_STR("%s(get config)"), IM_STR(_IM_FUNC_)));
			p = (void*)&caps;
			ret = camif_ioctl(cmd, p);
			if(ret == IM_RET_OK)
			{
				copy_to_user((void*)arg, p, sizeof(caps));
			}
			break;
		case CAMIF_IOCTL_CMD_SET_PRO:
			IM_INFOMSG((IM_STR("%s(set property)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&pro, (void*)arg, sizeof(pro));
			p = (void*)&pro;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_GET_PRO:
			IM_INFOMSG((IM_STR("%s(get property)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&pro, (void*)arg, sizeof(pro));
			p = (void*)&pro;
			ret = camif_ioctl(cmd, p);
			if(ret == IM_RET_OK)
			{
				copy_to_user((void*)arg, p, sizeof(camif_ioctl_ds_key_property));
			}
			break;
		case CAMIF_IOCTL_CMD_ASSIGN_BUF:
			IM_INFOMSG((IM_STR("%s(assign buffer)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&buff, (void*)arg, sizeof(buff));
			p = (void*)&buff;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_RELEASE_BUF:
			IM_INFOMSG((IM_STR("%s(release buffer)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&buff, (void*)arg, sizeof(buff));
			p = (void*)&buff;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_SEND_CTR:
			IM_INFOMSG((IM_STR("%s(send ctrl)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&sendCtrl, (void*)arg, sizeof(sendCtrl));
			p = (void*)&sendCtrl;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_CANCEL_CTR:
			IM_INFOMSG((IM_STR("%s(cancel ctrl)"), IM_STR(_IM_FUNC_)));
			copy_from_user((void*)&cancelCtrl, (void*)arg, sizeof(cancelCtrl));
			p = (void*)&cancelCtrl;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_START:
			IM_INFOMSG((IM_STR("%s(start)"), IM_STR(_IM_FUNC_)));
			p = IM_NULL;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_STOP:
			IM_INFOMSG((IM_STR("%s(stop)"), IM_STR(_IM_FUNC_)));
			p = IM_NULL;
			ret = camif_ioctl(cmd, p);
			break;
		case CAMIF_IOCTL_CMD_GET_EVENT:
			IM_INFOMSG((IM_STR("%s(get event)"), IM_STR(_IM_FUNC_)));
			ret = camif_get_event(&evt);
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

static unsigned int camifdrv_poll(struct file *file, poll_table *wait)
{
	IM_RET ret = IM_RET_OK;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
#if TIME_STATS
	struct timeval _s, _e;
	IM_INT64 time;
	do_gettimeofday(&_s);
	IM_TIPMSG((IM_STR(" wait_event begin time.sec=%ds,time.usec=%d++ \n"),_s.tv_sec, _s.tv_usec));
#endif
	ret = camif_wait_event();
#if TIME_STATS
	do_gettimeofday(&_e);
	IM_TIPMSG((IM_STR(" wait_event end time.sec=%ds,time.usec=%d-- \n"),_e.tv_sec, _e.tv_usec));
	time = (_e.tv_sec*1000000+_e.tv_usec - (_s.tv_sec*1000000+_s.tv_usec))/1000;
	IM_TIPMSG((IM_STR(" wait_event time=%lldms - \n"),time));
#endif
	if(ret != IM_RET_OK)
	{
		IM_WARNMSG((IM_STR("%s(camif_wait_event timeout or error!)"), IM_STR(_IM_FUNC_)));
		return 0;
	}
	
	return POLLIN | POLLRDNORM;
}

static struct platform_driver camif_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-camif",
	},
	.probe = camifdrv_probe,
	.remove = camifdrv_remove,
	.suspend = camifdrv_suspend,
	.resume = camifdrv_resume,
};


static int camifdrv_probe(struct platform_device *pdev)
{
	char str[128] = "imapx820"; //default cpu type
    int ret = 0;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //get cpu type, it means imapx820 if no cpu info at items file
	if(item_exist("board.cpu")){
		item_string(str,"board.cpu",0);
	}

    //only i15 support camif now
    if(strcmp(str,"i15") != 0){
        IM_ERRMSG((IM_STR("this cpu not support camif!!!")));
        return -EINVAL;
    }

    //It maybe error when suspend/resume if this function not call, so it must be call. I don't know why?????
    //maybe not need for camif
    camifdrv_stub();

	mutex_init(&gDrvlock);

	ret = camifdrv_camsen_get_info((void *)&pdev->dev);
    if(ret != 0)
    {
		IM_ERRMSG((IM_STR(" camifdrv_camsen_get_info() failed, %d."),ret));
        mutex_destroy(&gDrvlock);
        return ret;
    }

	/* Register misc device driver. */
	ret = misc_register(&camif_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register camif misc device driver error, %d."),ret));
		return ret;
	}

    gDrvProbed = 1;

    return 0;
}

static int camifdrv_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    if(gDrvProbed == 1)
    {
        misc_deregister(&camif_miscdev);

        mutex_destroy(&gDrvlock);
    }

    gDrvProbed = 0;

    return 0;
}

static int camifdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //camif module disable
    if(gDrvModuleEn == 1)
    {
        camifdrv_module_disable();
        gDrvModuleEn = 0;
    }

	return 0;
}

static int camifdrv_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//set all camif io mode to function mode	
	imapx_pad_cfg(IMAPX_CAM, PULL_ENABLE);
	
	camifdrv_camsen_set_pwdn();

    //camif module enable
    if(gDrvModuleEn == 0)
    {
        if(camifdrv_module_enable() != 0)
        {
            IM_ERRMSG((IM_STR("camifdrv_module_enable() failed")));
            return -EINVAL;
        }
        gDrvModuleEn = 1;
    }

    return 0;
}

//==========================================================================================================
static int camifdrv_camsen_set_pwdn(void)
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

static int camifdrv_camsen_get_info(void *camsenPmuHand)
{
	int i;
	char str[128];
	IM_INT32 moduleNum = 0;
	IM_UINT32 facing;
	camsen_handle_t handle;
	camsen_caps_t caps;
	IM_RET ret = IM_RET_OK;
    IM_BOOL hasCamif = IM_FALSE;

	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	memset((void*)&gModuleClass, 0, sizeof(cam_module_class_t));

    //check whether has camif module
	if(item_exist("camera.front.interface")){
		item_string(str,"camera.front.interface",0);
		if(strcmp(str,"camif") == 0){
            hasCamif = IM_TRUE;
		}
	}
	if(item_exist("camera.rear.interface")){
		item_string(str,"camera.rear.interface",0);
		if(strcmp(str,"camif") == 0){
            hasCamif = IM_TRUE;
		}
	}

    if(hasCamif == IM_FALSE)
    {
        return -EINVAL;
    }

	//set all camif io mode to function mode	
	imapx_pad_cfg(IMAPX_CAM, PULL_ENABLE);

	camsen_set_pmu_handle(camsenPmuHand);

	//power down all front&rear camera sensor first
	if(item_exist("camera.front.interface")){
		item_string(str,"camera.front.interface",0);
		IM_INFOMSG((IM_STR("camera front interface type = %s "),str));
		if(strcmp(str,"camif") == 0){
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
		if(strcmp(str,"camif") == 0){
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
				if((strcmp(str,"camif")==0) && item_exist("camera.front.model")){
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
				if((strcmp(str,"camif")==0) && item_exist("camera.rear.model")){
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

	//camif can only support one box
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

static int camifdrv_camsen_get_type(camif_camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex)
{
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

	if(config->facing == CAM_MODULE_FACING_FRONT)
	{
		item_string(config->camsenDataType,"camera.front.data_type", 0);
		IM_INFOMSG((IM_STR("open front sensor(%s: dataype=%s)"), config->camsenName, config->camsenDataType));
	}
	else if(config->facing == CAM_MODULE_FACING_BACK)
	{
		item_string(config->camsenDataType,"camera.rear.data_type", 0);
		IM_INFOMSG((IM_STR("open rear sensor(%s: dataype=%s)"), config->camsenName, config->camsenDataType));
	}
	else
	{
		IM_ERRMSG((IM_STR("sensor name error!")));
		return -1;
	}

	return 0;
}


static int camifdrv_module_enable(void)
{
	IM_UINT32 res;

	module_power_on(SYSMGR_ISP_BASE);
	
    busclk = clk_get(NULL, "isp");
	if(busclk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get camif bus clock failed!")));
		goto Fail;
	}

	res = clk_set_rate(busclk, 201000); //201MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("camif bus clk set rate failed! ")));
		goto Fail;
	}
	busclkFrq = (IM_UINT32)clk_get_rate(busclk);

	if(clk_enable(busclk) != 0)
	{
		IM_ERRMSG((IM_STR("camif bus clock enable failed!")));
		goto Fail;
	}

	IM_INFOMSG((IM_STR("camif busclk frq= %d!"), busclkFrq));

	// module reset
	camifdrv_module_reset();

	return 0;
Fail:
	if(busclk != IM_NULL)
	{
		clk_put(busclk);
		busclk = IM_NULL;
	}
	return -1;
}

static int camifdrv_module_disable(void)
{
    
	if(busclk != IM_NULL)
	{
		clk_disable(busclk);
		clk_put(busclk);
		busclkFrq = 0;
		busclk = IM_NULL;
	}

	module_power_down(SYSMGR_ISP_BASE);
	return 0;
}

void camifdrv_module_reset(void)
{
	module_reset(SYSMGR_ISP_BASE, 1);
}

static int camifdrv_stub(void)
{
    if(camifdrv_module_enable() != 0)
    {
        IM_ERRMSG((IM_STR("camifdrv_module_enable() failed")));
        return -EINVAL;
    }

    camifdrv_module_disable();

    return 0;
}

int camif_camera_pcb_get_modules(cam_module_class_t *moduleClass)
{
    if(moduleClass == IM_NULL)
    {
        return -1;
    }
    memcpy((void*)moduleClass, (void*)&gModuleClass, sizeof(cam_module_class_t));
    
    return 0;
}

EXPORT_SYMBOL(camif_camera_pcb_get_modules);                                       

static int __init camifdrv_init(void)
{
	int ret = 0;
	printk("####camifdrv init####\n");
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

    //register all camera sensors on supported list
    camsen_register_all();

	/* Register platform device driver. */
	ret = platform_driver_register(&camif_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register camif platform device driver error, %d."),ret));
		return ret;
	}

	return 0;
}

static void __exit camifdrv_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	platform_driver_unregister(&camif_plat);
}

//module_init(camifdrv_init);
late_initcall(camifdrv_init);
module_exit(camifdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jimmy of InfoTM");

