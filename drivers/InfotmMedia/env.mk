#
# 
#
IM_SYSTEM_TYPE := kernel/linux_3_0
IM_ROOT := /home/zone3/leo/release/android-4.0.3_r1_new_dev/bootable/linux-3.0.8/drivers/InfotmMedia
#IM_RELEASE_ROOT := /storage/sdb/user/leo/android-linux-3.0.8-live/drivers/InfotmMedia/release
EXTRA_CFLAGS += -DTARGET_SYSTEM=KERNEL_LINUX -DTS_VER_MAJOR=3 -DTS_VER_MINOR=0 -DTS_VER_PATCH=8


#
#
#
IM_BUILDER_ROOT := $(IM_ROOT)/builder/$(IM_SYSTEM_TYPE)
IM_EXTERNAL_ROOT := $(IM_ROOT)/external
IM_EXTERNAL_WORKCOPY_ROOT := $(IM_ROOT)/external/workcopy
IM_EXTERNAL_PROJECT_ROOT := $(IM_ROOT)/external/project



