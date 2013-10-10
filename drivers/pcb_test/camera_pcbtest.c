#include <linux/module.h>   
#include <linux/ioport.h>   
#include <mach/imap-iomap.h>
#include <mach/imap-mac.h>  
#include <asm/io.h>         
#include <linux/gpio.h>     
#include <mach/power-gate.h>
#include <mach/pad.h>       
#include <linux/init.h>     
#include <asm/delay.h>      
#include <linux/delay.h>    
#include <mach/items.h>    
#include <mach/imap-iis.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/cache.h>
#include <linux/io.h>
#include <linux/poll.h>
#include <asm/cacheflush.h>

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include <camsen_lib.h>
#include <camisp_api.h>
#include <camif_api.h>

#include <IM_buffallocapi.h>
#include <pmm_lib.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"CAMPCB_I:"
#define WARNHEAD	"CAMPCB_W:"
#define ERRHEAD		"CAMPCB_E:"
#define TIPHEAD		"CAMPCB_T:"

static int gFrontCamIntf = CAM_INTERFACE_UNKNOWN;
static int gRearCamIntf = CAM_INTERFACE_UNKNOWN;

unsigned int *regIDSVir;
int ids_open(IM_Buffer *buf, int width, int height);
int ids_close(void);
int ids_set_buffer(IM_Buffer *buf, int width, int height);

static cam_module_class_t gModuleClass;
static struct clk *ispOsdClk = IM_NULL;
static struct clk *ispBusClk = IM_NULL;
//clock used for mipi-csi2
static struct clk *csiBusClk = IM_NULL;
static struct clk *dphyCfgClk = IM_NULL;
static struct clk *mipiPixClk = IM_NULL;

static unsigned int ispBusClkFrq = 0;
static unsigned int ispOsdClkFrq = 0;
static unsigned int mipiPixClkFrq = 0;

extern int camera_pcb_get_modules(cam_module_class_t *moduleClass);
extern int camif_camera_pcb_get_modules(cam_module_class_t *moduleClass);
extern IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner);
extern IM_RET pmmdrv_release(IN pmm_handle_t handle);
extern IM_RET pmmdrv_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer);
extern IM_RET pmmdrv_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer);

static int isp_get_type(camdev_config_t *config, int boxIndex, int moduleIndex);
static int camif_get_type(camif_camdev_config_t *config, int boxIndex, int moduleIndex);
static int isp_module_enable(void);
static int isp_module_disable(void);
static int camif_module_enable(void);
static int camif_module_disable(void);
static void calc_frame_size(IM_UINT32 res, IM_UINT32 fmt, IM_INT32 *w, IM_INT32 *h, IM_INT32 *imageSize);
static int do_test_isp(int boxIndex, int moduleIndex);
static int do_test_camif(int boxIndex, int moduleIndex);
static int check_module_interface(void);

int camera_pcbtest(void) 
{
    int i, j;
    int ret = -1;
	int moduleIntf = -1;
    IM_INFOMSG((IM_STR("camera pcb test enter\n")));
    
	moduleIntf = check_module_interface();

	if(1 == moduleIntf)//isp and isp_mipi
		camera_pcb_get_modules(&gModuleClass);
	else if(0 == moduleIntf)//camif
		camif_camera_pcb_get_modules(&gModuleClass);
	else
		IM_ERRMSG((IM_STR("itms.itm is error!")));

    IM_INFOMSG((IM_STR("gModuleClass.boxes_num=%d, moduleNum=%d\n"), gModuleClass.boxNum, gModuleClass.boxes[0].moduleNum));

    for(i=0; i<gModuleClass.boxNum; i++)
    {
        for(j=0; j<gModuleClass.boxes[i].moduleNum; j++)
        {
            IM_INFOMSG((IM_STR("gModuleClass.boxes[%d].modules[%d] camera test ....\n"), i, j));
            
            if(1 == moduleIntf){
                ret = do_test_isp(i, j);
            }
            else if(0 == moduleIntf){    
                ret = do_test_camif(i, j);
            }
            else{
                IM_ERRMSG((IM_STR("itms.itm is error!")));
                return -1;
            }
            if(ret != 0)
            {
                IM_ERRMSG((IM_STR("gModuleClass.boxes[%d].modules[%d] camera test failed!\n"), i, j));
                return -1;
            }
        }
    }

    IM_INFOMSG((IM_STR("camera pcb test leave, ret=%d\n"), ret));
    return ret;
}

static int check_module_interface(void)
{
    char str[128] = {0};
	if(item_exist("camera.front.interface")){
		item_string(str,"camera.front.interface",0);
		if(strcmp(str,"isp") == 0){
			gFrontCamIntf = CAM_INTERFACE_ISP_DVP;
        }
		else if(strcmp(str,"isp_mipi") == 0){
			gFrontCamIntf = CAM_INTERFACE_ISP_MIPI;
		}
		else if(strcmp(str,"camif") == 0){
			gFrontCamIntf = CAM_INTERFACE_CAMIF_DVP;
		}
		else{
			return -1;
		}
	}

	if(item_exist("camera.rear.interface")){
		item_string(str,"camera.rear.interface",0);
		if(strcmp(str,"isp") == 0){
			gRearCamIntf = CAM_INTERFACE_ISP_DVP;
        }
		else if(strcmp(str,"isp_mipi") == 0){
			gRearCamIntf = CAM_INTERFACE_ISP_MIPI;
		}
		else if(strcmp(str,"camif") == 0){
			gRearCamIntf = CAM_INTERFACE_CAMIF_DVP;
		}
		else{
			return -1;
		}
	}

	//imapx820 and x15 can not support camif and isp
	if(gFrontCamIntf == CAM_INTERFACE_CAMIF_DVP)
	{
		if((gRearCamIntf != CAM_INTERFACE_UNKNOWN) && (gRearCamIntf != CAM_INTERFACE_CAMIF_DVP))
		return -1;
	}
	if(gRearCamIntf == CAM_INTERFACE_CAMIF_DVP)
	{
		if((gFrontCamIntf != CAM_INTERFACE_UNKNOWN) && (gFrontCamIntf != CAM_INTERFACE_CAMIF_DVP))
		return -1;
	}

	if((gFrontCamIntf == CAM_INTERFACE_CAMIF_DVP) 
			|| (gRearCamIntf == CAM_INTERFACE_CAMIF_DVP))
	{
		return 0;
	}
	else if((gFrontCamIntf == CAM_INTERFACE_ISP_DVP) 
		|| (gFrontCamIntf == CAM_INTERFACE_ISP_MIPI)
		|| (gRearCamIntf == CAM_INTERFACE_ISP_DVP)
		|| (gRearCamIntf == CAM_INTERFACE_ISP_MIPI))
	{
		return 1;
	}
	else
	{
		return -1;
	}

}
static void calc_frame_size(IM_UINT32 res, IM_UINT32 fmt, IM_INT32 *w, IM_INT32 *h, IM_INT32 *imageSize)
{
	IM_INT32 size = 0;
	IM_INFOMSG((IM_STR("%s(res=0x%x, fmt=0x%x)"), IM_STR(_IM_FUNC_), res, fmt));
	switch(res){
	case CAM_RES_QQCIF:
		size = 88*72;
		*w = 88;
		*h = 72;
		break;
	case CAM_RES_SUB_QCIF:
		size = 128*96;
		*w = 128;
		*h = 96;
		break;
	case CAM_RES_QQVGA:
		size = 160*120;
		*w = 160;
		*h = 120;
		break;
	case CAM_RES_QCIF:
		size = 176*144;
		*w = 176;
		*h = 144;
		break;
	case CAM_RES_QVGA:
		size = 320*240;
		*w = 320;
		*h = 240;
		break;
	case CAM_RES_CIF:
		size = 352*288;
		*w = 352;
		*h = 288;
		break;
	case CAM_RES_VGA:
		size = 640*480;
		*w = 640;
		*h = 480;
		break;
	case CAM_RES_480P:
		size = 640*480;
		*w = 640;
		*h = 480;
		break;
	case CAM_RES_PAL:
		size = 768*576;
		*w = 768;
		*h = 576;
		break;
	case CAM_RES_SVGA:
		size = 800*600;
		*w = 800;
		*h = 600;
		break;
	case CAM_RES_XVGA:
		size = 1024*768;
		*w = 1024;
		*h = 768;
		break;
	case CAM_RES_720P:
		size = 1280*720;
		*w = 1280;
		*h = 720;
		break;
	case CAM_RES_SXGA:
		size = 1280*1024;
		*w = 1280;
		*h = 1024;
		break;
	case CAM_RES_SXGAPlus:
		size = 1400*1050;
		*w = 1400;
		*h = 1050;
		break;
	case CAM_RES_UXGA:
		size = 1600*1200;
		*w = 1600;
		*h = 1200;
		break;
	case CAM_RES_1080P:
		size = 1920*1080;
		*w = 1920;
		*h = 1080;
		break;
	case CAM_RES_320W:
		size = 2048*1536;
		*w = 2048;
		*h = 1536;
		break;
	case CAM_RES_WQXGA:
		size = 2560*1600;
		*w = 2560;
		*h = 1600;
		break;
	case CAM_RES_500W:
		size = 2592*1936;
		*w = 2592;
		*h = 1936;
		break;
	case CAM_RES_QUXGA:
		size = 3200*2400;
		*w = 3200;
		*h = 2400;
		break;
	case CAM_RES_WQXGA_U:
		size = 3840*2400;
		*w = 3840;
		*h = 2400;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(res=0x%x)"), IM_STR(_IM_FUNC_), res));
		break;
	}

	switch(fmt){
	case CAM_PIXFMT_YUV420P:
		*imageSize = (size*3)>>1;
		break;
	case CAM_PIXFMT_YUV420SP:
		*imageSize = (size*3)>>1;
		break;
	case CAM_PIXFMT_YUV422P:
		*imageSize = size<<1;
		break;
	case CAM_PIXFMT_YUV422SP:
		*imageSize = size<<1;
		break;
	case CAM_PIXFMT_YUV444P:
		*imageSize = size*3;
		break;
	case CAM_PIXFMT_YUV422I:
		*imageSize = size<<1;
		break;
	case CAM_PIXFMT_16BPP_RGB565:
		*imageSize = size<<1;
		break;
	case CAM_PIXFMT_32BPP_RGB0888:
	case CAM_PIXFMT_32BPP_RGB8880:
	case CAM_PIXFMT_32BPP_BGR0888:
	case CAM_PIXFMT_32BPP_BGR8880:
		*imageSize = size<<2;
		break;
	default:
		IM_ERRMSG((IM_STR("%s(fmt=0x%x)"), IM_STR(_IM_FUNC_), fmt));
		break;
	}

	return;
}

#define FRAME_MAX_NUM   100
static int do_test_isp(int boxIndex, int moduleIndex)
{
    camdev_config_t config;
    cam_preview_config_t caps;
    IM_RET ret = IM_RET_OK;
    pmm_handle_t pmmHand;
    char owner[256] = "camera_pcbtest";
    //int size = (2592*1936*3)/2; //large enough for 500W YUV420
    int size = (640*480*3)/2;
    int width = 640, height = 480;
    int num = 0, res = 0;
    alc_buffer_t alcBuffer[3];
	workmodel_event_t evt;
    IM_Buffer rdyBuf;
    IM_BOOL idsOpen = IM_FALSE;

    IM_INFOMSG((IM_STR("do_test_isp()++")));

    memset((void*)alcBuffer, 0x0, sizeof(alc_buffer_t)*3);

    res = isp_module_enable();
    if(res != 0)
    {
        IM_ERRMSG((IM_STR("isp_module_enable() failed")));
        return -1;
    }

    if(isp_get_type(&config, boxIndex, moduleIndex) != 0)
    {
        IM_ERRMSG((IM_STR("isp_get_type() failed")));
        res = -1;
        goto DisableModule;
    }

    ret = camisp_init(&config);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_init() failed")));
        res = -1;
        goto DisableModule;
    }

    ret = camisp_ioctl(CAMISP_IOCTL_CMD_GET_CFG, (void*)&caps);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_ioctl(get preview config) failed")));
        res = -1;
        goto Deinit;
    }
    
    calc_frame_size(caps.res, caps.fmt, &width, &height, &size);

    ret = pmmdrv_open(&pmmHand, owner);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_open() failed")));
        res = -1;
        goto Deinit;
    }

    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[0]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camisp_ioctl(CAMISP_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[0].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }
    
    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[1]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camisp_ioctl(CAMISP_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[1].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }

    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[2]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camisp_ioctl(CAMISP_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[2].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }

    ret = camisp_ioctl(CAMISP_IOCTL_CMD_START, IM_NULL);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }
    printk("=================START================");
	unsigned long to =  jiffies + 2*HZ;
   while(num <FRAME_MAX_NUM && jiffies < to )

   {
       num++;
       ret = camisp_wait_event();
       if(ret != IM_RET_OK)
       {
           IM_WARNMSG((IM_STR("%s(camisp_wait_event timeout or error!)"), IM_STR(_IM_FUNC_)));
           continue;
       }

       ret = camisp_get_event(&evt);
       if(ret == IM_RET_OK)
       {
           if(evt.type == WORKMODEL_EVENT_FRAME_DONE)
           {   
               rdyBuf.vir_addr = evt.contents.frm.buffer.vir_addr;
               rdyBuf.phy_addr = evt.contents.frm.buffer.phy_addr;
               rdyBuf.size = evt.contents.frm.buffer.size;
               rdyBuf.flag = evt.contents.frm.buffer.flag;

               if(idsOpen == IM_FALSE)
               {
                   res = ids_open(&rdyBuf, width, height);
                   if(res != 0){
                       IM_ERRMSG((IM_STR("ids_open() failed")));
                       break;
                   }
                   idsOpen = IM_TRUE;
               }
               else
               {
                   ids_set_buffer(&rdyBuf, width, height);
               }
               //release buffer of event
               ret = camisp_ioctl(CAMISP_IOCTL_CMD_RELEASE_BUF, (void*)&rdyBuf);
               if(ret != IM_RET_OK){
                   IM_ERRMSG((IM_STR("camisp_ioctl(release buffer) failed")));
                   break;
               }
           }   
       }
       else
       {
           IM_ERRMSG((IM_STR("camisp_ioctl(release buffer) failed")));
           break;
       }
   }
	printk(KERN_ERR"get fram ok\n");

end:

   if(idsOpen == IM_TRUE)
   {
       ids_close();
   }
   printk("========1==========\n");
   camisp_ioctl(CAMISP_IOCTL_CMD_STOP, IM_NULL);

MemFree:

   if(alcBuffer[0].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[0]);
   }
   printk("========2==========\n");
   if(alcBuffer[1].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[1]);
   }
   if(alcBuffer[2].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[2]);
   }

   pmmdrv_release(pmmHand);

Deinit:
	printk("========3==========\n");

   camisp_deinit();

DisableModule:

   isp_module_disable();

   IM_INFOMSG((IM_STR("do_test_isp()--")));
   return res;
}
static int do_test_camif(int boxIndex, int moduleIndex)
{
    camif_camdev_config_t config;
    cam_preview_config_t caps;
    IM_RET ret = IM_RET_OK;
    pmm_handle_t pmmHand;
    char owner[256] = "camera_pcbtest";
    //int size = (2592*1936*3)/2; //large enough for 500W YUV420
    int size = (640*480*3)/2;
    int width = 640, height = 480;
    int num = 0, res = 0;
    alc_buffer_t alcBuffer[3];
	camif_workmodel_event_t evt;
    IM_Buffer rdyBuf;
    IM_BOOL idsOpen = IM_FALSE;

    IM_INFOMSG((IM_STR("do_test_camif()++")));

    memset((void*)alcBuffer, 0x0, sizeof(alc_buffer_t)*3);

    res = camif_module_enable();
    if(res != 0)
    {
        IM_ERRMSG((IM_STR("camif_module_enable() failed")));
        return -1;
    }

    if(camif_get_type(&config, boxIndex, moduleIndex) != 0)
    {
        IM_ERRMSG((IM_STR("camif_get_type() failed")));
        res = -1;
        goto DisableModule;
    }

    ret = camif_init(&config);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camisp_init() failed")));
        res = -1;
        goto DisableModule;
    }

    ret = camif_ioctl(CAMIF_IOCTL_CMD_GET_CFG, (void*)&caps);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camif_ioctl(get preview config) failed")));
        res = -1;
        goto Deinit;
    }
    
    calc_frame_size(caps.res, caps.fmt, &width, &height, &size);

    ret = pmmdrv_open(&pmmHand, owner);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_open() failed")));
        res = -1;
        goto Deinit;
    }

    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[0]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camif_ioctl(CAMIF_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[0].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camif_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }
    
    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[1]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camif_ioctl(CAMIF_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[1].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camif_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }

    ret =  pmmdrv_mm_alloc(pmmHand, size, ALC_FLAG_PHY_MUST, &alcBuffer[2]);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("pmmdrv_mm_alloc() failed")));
        res = -1;
        goto MemFree;
    }

    ret = camif_ioctl(CAMIF_IOCTL_CMD_ASSIGN_BUF, (void*)&alcBuffer[2].buffer);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camif_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }

    ret = camif_ioctl(CAMIF_IOCTL_CMD_START, IM_NULL);
    if(ret != IM_RET_OK){
        IM_ERRMSG((IM_STR("camif_ioctl(assign buffer) failed")));
        res = -1;
        goto MemFree;
    }

	printk("=================START================\n");
	while(num <FRAME_MAX_NUM)
	{
		num++;

       ret = camif_wait_event();
       if(ret != IM_RET_OK)
       {
           IM_WARNMSG((IM_STR("%s(camisp_wait_event timeout or error!)"), IM_STR(_IM_FUNC_)));
           continue;
       }

       ret = camif_get_event(&evt);
       if(ret == IM_RET_OK)
       {
           if(evt.type == WORKMODEL_EVENT_FRAME_DONE)
           {   
               rdyBuf.vir_addr = evt.contents.frm.buffer.vir_addr;
               rdyBuf.phy_addr = evt.contents.frm.buffer.phy_addr;
               rdyBuf.size = evt.contents.frm.buffer.size;
               rdyBuf.flag = evt.contents.frm.buffer.flag;

               if(idsOpen == IM_FALSE)
               {
                   res = ids_open(&rdyBuf, width, height);
                   if(res != 0){
                       IM_ERRMSG((IM_STR("ids_open() failed")));
                       break;
                   }
                   idsOpen = IM_TRUE;
               }
               else
               {
                   ids_set_buffer(&rdyBuf, width, height);
               }
               //release buffer of event
               ret = camif_ioctl(CAMIF_IOCTL_CMD_RELEASE_BUF, (void*)&rdyBuf);
               if(ret != IM_RET_OK){
                   IM_ERRMSG((IM_STR("camif_ioctl(release buffer) failed")));
                   break;
               }
           }   
       }
       else
       {
           IM_ERRMSG((IM_STR("camif_ioctl(release buffer) failed")));
           break;
       }
   }
	printk("===========get frame ok============\n");

end:

   if(idsOpen == IM_TRUE)
   {
       ids_close();
   }
   camif_ioctl(CAMIF_IOCTL_CMD_STOP, IM_NULL);

MemFree:

   if(alcBuffer[0].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[0]);
   }
   if(alcBuffer[1].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[1]);
   }
   if(alcBuffer[2].privData != IM_NULL)
   {
       pmmdrv_mm_free(pmmHand, &alcBuffer[2]);
   }

   pmmdrv_release(pmmHand);

Deinit:

   camif_deinit();

DisableModule:

   camif_module_disable();

   IM_INFOMSG((IM_STR("do_test_camif()--")));
   return res;
}

static int isp_module_enable(void)
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

    if((gFrontCamIntf == CAM_INTERFACE_ISP_MIPI)
		|| (gRearCamIntf == CAM_INTERFACE_ISP_MIPI))
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

static int camif_module_enable(void)
{
    IM_UINT32 res;

    module_power_on(SYSMGR_ISP_BASE);

	ispBusClk = clk_get(NULL, "isp");
	if(ispBusClk ==  IM_NULL)
	{
		IM_ERRMSG((IM_STR("get camif bus clock failed!")));
		goto Fail;
	}

	res = clk_set_rate(ispBusClk, 201000); //201MHz
	if (res < 0){
		IM_ERRMSG((IM_STR("camif bus clk set rate failed! ")));
		goto Fail;
	}
	ispBusClkFrq = (IM_UINT32)clk_get_rate(ispBusClk);

	if(clk_enable(ispBusClk) != 0)
	{
		IM_ERRMSG((IM_STR("isp bus clock enable failed!")));
		goto Fail;
	}

	IM_INFOMSG((IM_STR("camif ispBusClk frq= %d!"), ispBusClkFrq));

	return 0;
Fail:
	
    if(ispBusClk != IM_NULL)
	{
		clk_put(ispBusClk);
		ispBusClk = IM_NULL;
	}
	return -1;
}

static int isp_module_disable(void)
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

    if((gFrontCamIntf == CAM_INTERFACE_ISP_MIPI)
		|| (gRearCamIntf == CAM_INTERFACE_ISP_MIPI))
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

static int camif_module_disable(void)
{

	if(ispBusClk != IM_NULL)
	{
		clk_disable(ispBusClk);
		clk_put(ispBusClk);
		ispBusClkFrq = 0;
		ispBusClk = IM_NULL;
	}

	module_power_down(SYSMGR_ISP_BASE);

	return 0;
}

static int isp_get_type(camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex)
{
	char str[128];
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

		IM_INFOMSG((IM_STR("open front sensor(%s: dataype=%s)"), config->camsenName, config->dataType));
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

		IM_INFOMSG((IM_STR("open rear sensor(%s: dataype=%s)"), config->camsenName, config->dataType));
	}
	else
	{
		IM_ERRMSG((IM_STR("sensor name error!")));
		return -1;
	}

	return 0;
}
static int camif_get_type(camif_camdev_config_t *config, IM_INT32 boxIndex, IM_INT32 moduleIndex)
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

static void ids_reg_read(unsigned int addr, unsigned int *val)
{
    *val = readl((unsigned int)regIDSVir + addr);
}
static void ids_write_reg(unsigned int addr, unsigned int val)
{
    writel(val, (unsigned int)regIDSVir + addr);
}
static void ids_write_regbit(unsigned int addr, unsigned int bit, unsigned int width, unsigned int val)
{
    unsigned int tmp;
    ids_reg_read(addr, &tmp);
    tmp &= ~(((0xFFFFFFFF << (32 - width)) >> (32 - width)) << bit);
    tmp |= val << bit;
    ids_write_reg(addr, tmp);
}

int ids_open(IM_Buffer *buf, int width, int height)
{
    int val, disp_width, disp_height, right, bottom;
    regIDSVir = NULL;
    regIDSVir = (unsigned int*)ioremap_nocache(0x22000000, 0x7000);
    IM_INFOMSG((IM_STR("ids_open(width=%d, height=%d)\n"), width, height));
    if (regIDSVir == NULL){
        IM_ERRMSG((IM_STR("ioremap IDS1 failed -\n")));
        return -1;
    }
    ids_write_regbit(0x1100 , 0, 1, 0); // close window 1
    ids_write_regbit(0x1180 , 0, 1, 0); // close window 2
    ids_write_regbit(0x1200 , 0, 1, 0); // close window 3

    // get the screen resolution
    ids_reg_read(0x0018, &val);
    disp_width = val & 0x7FF;
    disp_height = (val >> 16) & 0x7FF;
    //printk("disp_width=%d, disp_height=%d -\n", disp_width,disp_height);
    //printk("width=%d, height=%d -\n", width,height);

    ids_write_regbit(0x1100, 1, 5, 17);
    ids_write_reg(0x110C, 0xFFF000);
    ids_write_reg(0x1110, width);
	right = right > disp_width? disp_width:right;
    bottom = bottom > disp_height? disp_height:bottom;
    int top = 120;
    int left = 160;
    ids_write_reg(0x1104, (160 << 0) | (120 << 16));//left top
    right = (disp_width > width+160 ) ? width+160 : disp_width;
    bottom = (disp_height > height+ 120) ? height+120 : disp_height;
    ids_write_reg(0x1108, (right-1)<< 0 | (bottom-1)<< 16);//right bottom
    
    ids_write_reg(0x1120, buf->phy_addr);

    ids_write_reg(0x1300, buf->phy_addr + width * height);

    dmac_flush_range(buf->vir_addr, (void*)((unsigned int)buf->vir_addr + buf->size));
    outer_flush_range(buf->phy_addr, buf->phy_addr + buf->size);

    ids_write_regbit(0x1100 , 0, 1, 1); // open window 2 

    return 0;
}

int ids_set_buffer(IM_Buffer *buf, int width, int height)
{ 
    ids_write_reg(0x1120, buf->phy_addr);                                                             
    ids_write_reg(0x1300, buf->phy_addr + width * height);
    return 0;
}

int ids_close(void)
{
    IM_INFOMSG((IM_STR("ids_close()\n")));
    ids_write_regbit(0x1100 , 0, 1, 0);
    if (regIDSVir != NULL){
        iounmap(regIDSVir);
    }
    return 0;
}

