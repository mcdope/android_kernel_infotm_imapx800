/*------------------------------------------------------------------------------
--
-- 	Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
--
--	Use of this software is governed by terms and conditions 
--	stated in the accompanying licensing statement. 
--
--------------------------------------------------------------------------------
--	@file camsen_utils.c
--
--  Description :
--		
--
--	Author:
--  	Arsor Fu   <arsor.fu@infotmic.com.cn>
--------------------------------------------------------------------------------
-- Revision History:
--------------------
-- v1.0.1	arsor@2012/04/05: first commit.
-- v1.0.2	arsor@2012/07/16: add register gt2005_pc0am0008a & gt2005_pc0am0004b.
-- v1.0.3	jimmy@2012/07/20: add register hi704_yuv & hi253_yuv.
--
------------------------------------------------------------------------------*/

#include <InfotmMedia.h>
#include <IM_cameraapi.h>
#include "camsen_lib.h"
#include "camsen_pwl.h"
#include "camsen.h"

#define IMMAX(a,b) ((a) > (b) ? (a) : (b))
void im_camsen_register(camsen_ops *ops);

#if 0//need to get config info from driver board config
#define REGISTER_CAMSEN(X,x) { \
    extern camsen_ops x##_ops; \
    if(CONFIG_CAMSEN_##X) im_camsen_register(&x##_ops); }
#else
#define REGISTER_CAMSEN(x) { \
    extern camsen_ops x##_ops; \
    im_camsen_register(&x##_ops); }
#endif
/** head of registered ops linked list */
static camsen_ops *first_ops = NULL;

camsen_ops  *im_camsen_next(camsen_ops *ops)
{
    if(ops) return ops->next;
    else  return first_ops;
}

void im_camsen_register(camsen_ops *ops)
{
    camsen_ops **p;
    p = &first_ops;
    while (*p != NULL) p = &(*p)->next;
    *p = ops;
    ops->next = NULL;
}

static int match_ops(const char *name, const char *names)
{
    const char *p;
    int len, namelen;

    if (!name || !names)
        return 0;

    namelen = strlen(name);
    while ((p = strchr(names, ','))) {
        len = IMMAX(p - names, namelen);
        if (!strncasecmp(name, names, len))
            return 1;
        names = p+1;
    }
    return !strcasecmp(name, names);
}

camsen_ops *im_find_camsen(const char *name)
{
    camsen_ops *ops = NULL;
    while ((ops = im_camsen_next(ops))) {
        if (match_ops(name, ops->name))
            return ops;
    }
    return NULL;
}

void camsen_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;

    REGISTER_CAMSEN(gc0308_xyc);
	REGISTER_CAMSEN(sp2518_500w);
#if 0
	REGISTER_CAMSEN(gc0307_aih);
    REGISTER_CAMSEN(gc0308_demo);
    REGISTER_CAMSEN(gc0308_demo_130w);
    REGISTER_CAMSEN(gc0308_hy);
    REGISTER_CAMSEN(gc0308_hy_suoku);
    REGISTER_CAMSEN(gc0308_hy_130w);
    REGISTER_CAMSEN(gc0308_fyf);
    REGISTER_CAMSEN(gc0308_130w);
	REGISTER_CAMSEN(gc0308_w2);
	REGISTER_CAMSEN(gc0308_zyhb);
	REGISTER_CAMSEN(gc0309_zyhb);
	REGISTER_CAMSEN(gc0329_demo);
	REGISTER_CAMSEN(gc2015_zyhb);
	REGISTER_CAMSEN(gc2035_demo);
	REGISTER_CAMSEN(gc2035_demo_500w);
	REGISTER_CAMSEN(gt2005_demo);
	REGISTER_CAMSEN(gt2005_xyc);
	REGISTER_CAMSEN(gt2005_pc0am0008a);
	REGISTER_CAMSEN(gt2005_pc0am0004b);
	REGISTER_CAMSEN(gt2005_zyhb);
	REGISTER_CAMSEN(gt2005_500w);
	REGISTER_CAMSEN(hi253_yuv);
	REGISTER_CAMSEN(hi253_hy);
	REGISTER_CAMSEN(hi704_yuv);
	REGISTER_CAMSEN(hi704_hy);
	REGISTER_CAMSEN(hi704_gaof);
	REGISTER_CAMSEN(ov5640_zyhb);
	REGISTER_CAMSEN(ov5650_p48a);
	REGISTER_CAMSEN(ov5650_mipi);
	REGISTER_CAMSEN(ov5642_demo);
	REGISTER_CAMSEN(gc2035_gsg);
	REGISTER_CAMSEN(sp0838_zyhb);
	REGISTER_CAMSEN(sp2518_demo);
	REGISTER_CAMSEN(sp0838_130w);
	REGISTER_CAMSEN(ov5640_fxn);
	REGISTER_CAMSEN(mt9m114_demo);
	REGISTER_CAMSEN(mt9d115_demo);
#endif
}
