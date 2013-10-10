#
# Makefile for the kernel multimedia device drivers.
#


include drivers/InfotmMedia/env.mk

ifeq ($(CONFIG_INFOTM_MEDIA_SUPPORT), y)
obj-y += foundations/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_PMM_I800_SUPPORT), y)
obj-y += external/project/pmm_drv_i800/
obj-y += pmm_lib/source/
obj-y += dmmu_lib_i800/source/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_IDS_I800_SUPPORT), y)
obj-y += external/project/ids_drv_i800/
obj-y += ids_lib_i800/source/
obj-y += dsi_lib_i800/source/
obj-y += dsi_api/source/
obj-y += i80_lib_i800/source/
obj-y += i80_api/source/
obj-y += lcd_lib_i800/source/
obj-y += lcd_api/source/
obj-y += hdmi_lib_i800/source/
obj-y += hdmi_api/source/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_IPC_SUPPORT), y)
obj-y += external/project/ipc_drv/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_UTLZ_SUPPORT), y)
obj-y += external/project/utlz_drv/
obj-y += utlz_lib/source/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_G2D_I800_SUPPORT), y)
obj-y += external/project/g2d_drv_i800/
obj-y += g2d_lib_i800/source/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_ISP_I800_SUPPORT), y)
obj-y += isp_lib_i800/source/
obj-y += csi_lib_i800/source/
obj-y += camsen_lib/source/
obj-y += camsen_lib/source/sensors/
obj-y += camisp_api/source/
obj-y += external/project/isp_drv_i800/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_CAMIF_I800_SUPPORT), y)
obj-y += camif_lib_i800/source/
obj-y += camsen_lib/source/
obj-y += camsen_lib/source/sensors/
obj-y += camif_api/source/
obj-y += external/project/camif_drv_i800/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_VDEC_G1_SUPPORT), y)
obj-y += external/project/vdec_drv_g1/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_VENC_8270_SUPPORT), y)
obj-y += external/project/venc_drv_8270/
endif

ifeq ($(CONFIG_INFOTM_MEDIA_DBT_SUPPORT), y)
obj-y += external/project/dbt_drv/
endif


