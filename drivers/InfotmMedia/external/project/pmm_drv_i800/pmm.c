/***************************************************************************** 
** 
** Copyright (c) 2012~2112 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
**      
** Revision History: 
** ----------------- 
** v1.1.0	leo@2012/03/17: first commit.
** v1.2.1	leo@2012/04/05: fix ioctl() bug, sync with pmm.c in linux2_6 modification.
** v1.2.2	leo@2012/04/13: modify mmap(), fixed non-linear memory non-coherent bug.
** v1.2.3	leo@2012/04/23: modify some debug info.
** v1.2.4	leo@2012/04/28: add parameter check in pmmdrv_xxx().
**			leo@2012/05/02: add PMMLIB_IOCTL_MM_FLUSH_BUFFER ioctl.
** v1.2.7	leo@2012/07/11: add pmmdrv_dmmu_reset() interface.
** v1.2.8	leo@2012/08/16: In pmm_mmap(), support one nonlinear memory block has more pages.
** v1.2.9   leo@2012/09/10: add bigmem support.
** v1.2.12	leo@2012/11/30: modify ioctl(PMMLIB_IOCTL_MM_INIT_BIGMEM) parameters.
**
*****************************************************************************/ 


#include <InfotmMedia.h>
#include <IM_buffallocapi.h>
#include <IM_devmmuapi.h>
#include <pmm_lib.h>
#include <pmm_pwl.h>

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
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
#include <plat/imapx.h>
#endif
#include <linux/delay.h>


#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"PMMDRV_I:"
#define WARNHEAD	"PMMDRV_W:"
#define ERRHEAD		"PMMDRV_E:"
#define TIPHEAD		"PMMDRV_T:"

//
typedef struct{
	pmm_handle_t	handle;
	IM_BOOL		locked;
}pmm_instance_t;

//
typedef struct{
	struct mutex 	drvlock;
}pmm_drv_context_t;

static pmm_drv_context_t *gPmmDriver = IM_NULL;

/* 
 * Function declaration.
 */
static int pmm_open(struct inode *inode, struct file *file);
static int pmm_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int pmm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long pmm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int pmm_probe(struct platform_device *pdev);
static int pmm_remove(struct platform_device *pdev);
static int pmm_suspend(struct platform_device *pdev, pm_message_t state);
static int pmm_resume(struct platform_device *pdev);
static int pmm_mmap(struct file *file, struct vm_area_struct *vma);


static struct file_operations pmm_fops = {
    .owner = THIS_MODULE,
    .open = pmm_open,
    .release = pmm_release,
    .mmap = pmm_mmap,
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = pmm_ioctl,
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
    .unlocked_ioctl = pmm_ioctl,
#endif
};
static struct miscdevice pmm_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &pmm_fops,
    .name = "pmm",
};

IM_RET pmmdrv_open(OUT pmm_handle_t *phandle, IN char *owner)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if((phandle == IM_NULL) || ((owner == IM_NULL) || (strlen(owner) >= ALC_OWNER_LEN_MAX))){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}
	if(gPmmDriver == IM_NULL){
		if(pmm_probe(NULL) != 0){
			IM_ERRMSG((IM_STR("pmm_probe failed")));
			return IM_RET_FAILED;
		}
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_init(phandle, owner);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_open);

IM_RET pmmdrv_release(IN pmm_handle_t handle)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_deinit(handle);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_release);

IM_RET pmmdrv_mm_alloc(IN pmm_handle_t handle, IN IM_INT32 size, IN IM_INT32 flag, OUT alc_buffer_t *alcBuffer)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((handle == IM_NULL) || (size <= 0) || (alcBuffer == IM_NULL) || ((flag & 0xf) == 0)){// kernel don't support virtual memory allocation.
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_mm_alloc(handle, size, flag, alcBuffer);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_mm_alloc);

IM_RET pmmdrv_mm_free(IN pmm_handle_t handle, IN alc_buffer_t *alcBuffer)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if((handle == IM_NULL) || (alcBuffer == IM_NULL)){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_mm_free(handle, alcBuffer);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_mm_free);

alc_buffer_t * pmmdrv_getAlcBufferFromPhyAddr(IN pmm_handle_t handle, IN IM_UINT32 phyAddr)
{
	alc_buffer_t *abf;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_NULL;
	}
	
	mutex_lock(&gPmmDriver->drvlock);
	abf =pmmlib_getAlcBufferFromPhyAddr(handle, phyAddr);
	mutex_unlock(&gPmmDriver->drvlock);
	return abf;
}
EXPORT_SYMBOL(pmmdrv_getAlcBufferFromPhyAddr);

IM_RET pmmdrv_dmmu_init(IN pmm_handle_t handle, IN IM_UINT32 devid)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s(devid=%d)"), IM_STR(_IM_FUNC_), devid));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_dmmu_init(handle, devid);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_dmmu_init);

IM_RET pmmdrv_dmmu_deinit(IN pmm_handle_t handle)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_dmmu_deinit(handle);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_dmmu_deinit);

IM_RET pmmdrv_dmmu_reset(IN pmm_handle_t handle)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_dmmu_reset(handle);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_dmmu_reset);

IM_RET pmmdrv_dmmu_enable(IN pmm_handle_t handle)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_dmmu_enable(handle);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_dmmu_enable);

IM_RET pmmdrv_dmmu_disable(IN pmm_handle_t handle)
{
	IM_RET ret;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	if(handle == IM_NULL){
		IM_ERRMSG((IM_STR("invalid parameter")));
		return IM_RET_FAILED;
	}

	mutex_lock(&gPmmDriver->drvlock);
	ret = pmmlib_dmmu_disable(handle);
	mutex_unlock(&gPmmDriver->drvlock);
	return ret;
}
EXPORT_SYMBOL(pmmdrv_dmmu_disable);


static int pmm_open(struct inode *inode, struct file *file)
{
	pmm_instance_t *inst = (pmm_instance_t *)kmalloc(sizeof(pmm_instance_t), GFP_KERNEL);
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	if(inst == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(pmm_instance_t) failed")));
		return -EFAULT;
	}
	inst->handle = IM_NULL;
	inst->locked = IM_FALSE;
	file->private_data = (void *)inst;

	return 0;
}

static int pmm_release(struct inode *inode, struct file *file)
{
	pmm_instance_t *inst = (pmm_instance_t *)file->private_data;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(inst->locked == IM_TRUE){
		mutex_unlock(&gPmmDriver->drvlock);
		inst->locked = IM_FALSE;
	}
	if(inst->handle != IM_NULL){
		mutex_lock(&gPmmDriver->drvlock);
		pmmlib_deinit(inst->handle);
		mutex_unlock(&gPmmDriver->drvlock);
	}
	kfree(inst);

	return 0;
}

typedef struct{
	atomic_t refcount;
}vma_usage_tracker_t;

void pmm_vma_open(struct vm_area_struct * vma)
{
	int count;
	vma_usage_tracker_t *tracker = (vma_usage_tracker_t *)vma->vm_private_data;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	count = atomic_inc_return(&tracker->refcount);
	IM_INFOMSG((IM_STR("VMA open, VMA reference count incremented. VMA: 0x%08lx, reference count: %d"), (unsigned long)vma, count));
}

void pmm_vma_close(struct vm_area_struct * vma)
{
	int count;
	vma_usage_tracker_t *tracker = (vma_usage_tracker_t *)vma->vm_private_data;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	count = atomic_dec_return(&tracker->refcount);
	IM_INFOMSG((IM_STR("VMA close, VMA reference count decremented. VMA: 0x%08lx, reference count: %d"), (unsigned long)vma, count));

	if(count == 0){
		kfree((void *)tracker);
	}
}

static int pmm_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	void __user * address = vmf->virtual_address;

	IM_ERRMSG((IM_STR("Page-fault in PMM memory region caused by the CPU. VMA: 0x%08lx, virtual address: 0x%08lx\n"), (unsigned long)vma, (unsigned long)address));

	return VM_FAULT_SIGBUS;
}

static struct vm_operations_struct pmm_vma_ops = {
	.open = pmm_vma_open,
	.close = pmm_vma_close,
	.fault = pmm_vma_fault,
};

static int pmm_mmap(struct file *file, struct vm_area_struct *vma)
{
	IM_INT32 i, j;
	vma_usage_tracker_t *tracker;
	alc_buffer_t *abf;
	pmmpwl_pagemem_t *pagemem;
	pmm_instance_t *inst = (pmm_instance_t *)file->private_data;
	pmm_handle_t pmm = inst->handle;
	IM_UINT32 vm_seg_start, vm_seg_size;

	IM_INFOMSG((IM_STR("%s(phy_addr=0x%lx, size=%ld)"), IM_STR(_IM_FUNC_), vma->vm_pgoff << PAGE_SHIFT, vma->vm_end - vma->vm_start));
	IM_ASSERT(PMM_PAGE_SHIFT == PAGE_SHIFT);

	// tracker.
	tracker = (vma_usage_tracker_t *)kmalloc(sizeof(vma_usage_tracker_t), GFP_KERNEL);
	if(tracker == IM_NULL){
		IM_ERRMSG((IM_STR("kmalloc(vma_uaage_tracker_t) failed")));
		return -EAGAIN;
	}
	vma->vm_private_data = (void *)tracker;

	// mapping.
	mutex_lock(&gPmmDriver->drvlock);

	abf = pmmlib_getAlcBufferFromPhyAddr(pmm, vma->vm_pgoff << PAGE_SHIFT);
	IM_ASSERT(abf != IM_NULL);
	pagemem = (pmmpwl_pagemem_t *)abf->privData;
	IM_ASSERT(pagemem->mapped == IM_FALSE);
	IM_ASSERT((pagemem->totalPageNum << PMM_PAGE_SHIFT) >= (vma->vm_end - vma->vm_start));

	// forcing the CPU to use cache.
	if (!(vma->vm_flags & VM_SHARED)){
		vma->vm_flags = vma->vm_flags | VM_SHARED | VM_MAYSHARE;
	}

	// by setting this flag, during a process fork; the child process will not have the parent mappings.
	vma->vm_flags |= VM_DONTCOPY;

	vma->vm_flags |= VM_RESERVED | VM_IO;
	if(!pagemem->isCache){
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	}

	if(pagemem->linear == IM_TRUE){
		if(unlikely(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot))){
			IM_ERRMSG((IM_STR("remap_pfn_range() failed")));
			goto Fail;
		}
	}
    else{
#if 1
		vm_seg_start = vma->vm_start;
		for(i=0; i<pagemem->blockNum; i++){
			vm_seg_size = pagemem->blocks[i].pgNum << PAGE_SHIFT;
            IM_ASSERT(pagemem->blocks[i].phyAddr != 0);
			if(unlikely(remap_pfn_range(vma, vm_seg_start, pagemem->blocks[i].phyAddr >> PAGE_SHIFT, vm_seg_size, vma->vm_page_prot))){
				IM_ERRMSG((IM_STR("remap_pfn_range() failed, vm_seg_start=%d, phyAddr=0x%x, vm_seg_size=%d"), 
					vm_seg_start, pagemem->blocks[i].phyAddr, vm_seg_size));
				goto Fail;
			}
			vm_seg_start += vm_seg_size;
#else
		vm_seg_start = vma->vm_start;
		for(i=0; i<pagemem->blockNum; i++){
            for(j=0; j<pagemem->blocks[i].pgNum; j++){
                if(unlikely(remap_pfn_range(vma, vm_seg_start, (pagemem->blocks[i].phyAddr >> PAGE_SHIFT) + j, PAGE_SIZE, vma->vm_page_prot))){
                    IM_ERRMSG((IM_STR("remap_pfn_range() failed, vm_seg_start=%d, phyAddr=0x%x"), 
                                vm_seg_start, pagemem->blocks[i].phyAddr + (j << PAGE_SHIFT)));
                    goto Fail;
                }
                vm_seg_start += PAGE_SIZE;
            }
#endif
        }
    }
	
	mutex_unlock(&gPmmDriver->drvlock);

	// register vma_ops.	
	atomic_set(&tracker->refcount, 1); // this can later be increased if process is forked, see pmm_vma_open().
	vma->vm_ops = &pmm_vma_ops;
	return 0;
Fail:
	kfree((void *)tracker);
	mutex_unlock(&gPmmDriver->drvlock);
	return -EAGAIN;
}

#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int pmm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long pmm_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
	IM_RET ret;
	IM_BOOL unlock = IM_TRUE;
	pmm_instance_t *inst = (pmm_instance_t *)file->private_data;
	pmm_handle_t pmm = inst->handle;
	IM_TCHAR owner[ALC_OWNER_LEN_MAX];
	IM_UINT32 devid;
	pmmlib_ioctl_ds_mm_alloc_t ds_ma;
	alc_buffer_t alcBuffer;
	pmmlib_ioctl_ds_mm_map_useraddr_t ds_mu;
	pmmlib_ioctl_ds_mm_flush_buffer_t ds_fb;
	pmmlib_ioctl_ds_mm_add_useraddr_for_statc_t ds_acfs;
	void *uVirAddr;
	IM_INT32 num, tmp;
	pmmlib_ioctl_ds_statc_get_owner_t ds_go;
	pmmlib_ioctl_ds_statc_get_owner_buffer_num_t ds_gobn;
	pmmlib_ioctl_ds_statc_get_owner_buffer_t ds_gob;
	pmmlib_ioctl_ds_mm_init_bigmem_t ds_ib;

	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

	if(inst->locked == IM_FALSE){
		mutex_lock(&gPmmDriver->drvlock);
		inst->locked = IM_TRUE;
	}

	switch(cmd){
	case PMMLIB_IOCTL_INIT:
		if(unlikely(copy_from_user((void *)owner, (void *)arg, sizeof(IM_TCHAR) * ALC_OWNER_LEN_MAX))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_init(&pmm, owner);
		if(ret == IM_RET_OK){
			inst->handle = pmm;
		}
		break;
	case PMMLIB_IOCTL_DEINIT:
		ret = pmmlib_deinit(pmm);
		inst->handle = IM_NULL;
		break;
	case PMMLIB_IOCTL_DMMU_INIT:
		__get_user(devid,(IM_UINT32 *)arg);
		ret = pmmlib_dmmu_init(pmm, devid);
		break;
	case PMMLIB_IOCTL_DMMU_DEINIT:
		ret = pmmlib_dmmu_deinit(pmm);
		break;
	case PMMLIB_IOCTL_DMMU_ENABLE:
		ret = pmmlib_dmmu_enable(pmm);
		break;
	case PMMLIB_IOCTL_DMMU_DISABLE:
		ret = pmmlib_dmmu_disable(pmm);
		break;
    case PMMLIB_IOCTL_MM_INIT_BIGMEM:
		if(unlikely(copy_from_user((void *)&ds_ib, (void *)arg, sizeof(pmmlib_ioctl_ds_mm_init_bigmem_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
        ret = pmmlib_mm_init_bigmem(pmm, ds_ib.blockSize, ds_ib.blockNum, ds_ib.isCache);
        break;
    case PMMLIB_IOCTL_MM_DEINIT_BIGMEM:
        ret = pmmlib_mm_deinit_bigmem(pmm);
        break;
	case PMMLIB_IOCTL_MM_ALLOC:
		if(unlikely(copy_from_user((void *)&ds_ma, (void *)arg, sizeof(pmmlib_ioctl_ds_mm_alloc_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_alloc(pmm, ds_ma.size, ds_ma.flag, &ds_ma.alcBuffer);
		if(ret == IM_RET_OK){
			if(unlikely(copy_to_user((void *)(&((pmmlib_ioctl_ds_mm_alloc_t *)arg)->alcBuffer), (void *)&ds_ma.alcBuffer, sizeof(alc_buffer_t)))){
				IM_ERRMSG((IM_STR("copy_to_user() failed!")));
				ret = IM_RET_FAILED;
				break;
			}
		}
		break;
	case PMMLIB_IOCTL_MM_FREE:
		if(unlikely(copy_from_user((void *)&alcBuffer, (void *)arg, sizeof(alc_buffer_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_free(pmm, &alcBuffer);
		break;
	case PMMLIB_IOCTL_MM_MAP_USERADDR:
		if(unlikely(copy_from_user((void *)&ds_mu, (void *)arg, sizeof(pmmlib_ioctl_ds_mm_map_useraddr_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_map_useraddr(pmm, ds_mu.uVirAddr, ds_mu.size, &ds_mu.alcBuffer);
		if(ret == IM_RET_OK){
			if(unlikely(copy_to_user((void *)(&((pmmlib_ioctl_ds_mm_map_useraddr_t *)arg)->alcBuffer), (void *)&ds_mu.alcBuffer, sizeof(alc_buffer_t)))){
				IM_ERRMSG((IM_STR("copy_to_user() failed!")));
				ret = IM_RET_FAILED;
				break;
			}
		}
		break;
	case PMMLIB_IOCTL_MM_UNMAP_USERADDR:
		if(unlikely(copy_from_user((void *)&alcBuffer, (void *)arg, sizeof(alc_buffer_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_unmap_useraddr(pmm, &alcBuffer);
		break;
	case PMMLIB_IOCTL_MM_FLUSH_BUFFER:
		if(unlikely(copy_from_user((void *)&ds_fb, (void *)arg, sizeof(pmmlib_ioctl_ds_mm_flush_buffer_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_flush_buffer(pmm, &ds_fb.alcBuffer, ds_fb.flag);
		break;
	case PMMLIB_IOCTL_MM_ADD_USERADDR_FOR_STATC:
		if(unlikely(copy_from_user((void *)&ds_acfs, (void *)arg, sizeof(pmmlib_ioctl_ds_mm_add_useraddr_for_statc_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_mm_add_useraddr_for_statc(pmm, ds_acfs.uVirAddr, ds_acfs.size);
		break;
	case PMMLIB_IOCTL_MM_DEL_USERADDR_FOR_STATC:
		__get_user(uVirAddr, (void **)arg);
		ret = pmmlib_mm_del_useraddr_for_statc(pmm, uVirAddr);
		break;
	case PMMLIB_IOCTL_STATC_LOCK_ACCESS:
		unlock = IM_FALSE;
		ret = IM_RET_OK;
		break;
	case PMMLIB_IOCTL_STATC_UNLOCK_ACCESS:
		ret = IM_RET_OK;
		break;
	case PMMLIB_IOCTL_STATC_GET_OWNER_NUM:
		unlock = IM_FALSE;
		ret = pmmlib_statc_get_owner_num(pmm, &num);
		__put_user(num, (IM_INT32 *)arg);
		break;
	case PMMLIB_IOCTL_STATC_GET_OWNER:
		unlock = IM_FALSE;
		if(unlikely(copy_from_user((void *)&ds_go, (void *)arg, sizeof(pmmlib_ioctl_ds_statc_get_owner_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_statc_get_owner(pmm, ds_go.index, ds_go.owner);
		if(ret == IM_RET_OK){
			if(unlikely(copy_to_user((void *)(((pmmlib_ioctl_ds_statc_get_owner_t *)arg)->owner), (void *)&ds_go.owner, sizeof(IM_TCHAR) * ALC_OWNER_LEN_MAX))){
				IM_ERRMSG((IM_STR("copy_from_user() failed!")));
				ret = IM_RET_FAILED;
				break;
			}
		}
		break;
	case PMMLIB_IOCTL_STATC_GET_OWNER_BUFFER_NUM:
		unlock = IM_FALSE;
		if(unlikely(copy_from_user((void *)&ds_gobn, (void *)arg, sizeof(pmmlib_ioctl_ds_statc_get_owner_buffer_num_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_statc_get_owner_buffer_num(pmm, ds_gobn.owner, &ds_gobn.num);
		__put_user(ds_gobn.num, (IM_INT32 *)(&(((pmmlib_ioctl_ds_statc_get_owner_buffer_num_t *)arg)->num)));
		break;
	case PMMLIB_IOCTL_STATC_GET_OWNER_BUFFER:
		unlock = IM_FALSE;
		if(unlikely(copy_from_user((void *)&ds_gob, (void *)arg, sizeof(pmmlib_ioctl_ds_statc_get_owner_buffer_t)))){
			IM_ERRMSG((IM_STR("copy_from_user() failed!")));
			ret = IM_RET_FAILED;
			break;
		}
		ret = pmmlib_statc_get_owner_buffer(pmm, ds_gob.owner, ds_gob.index, &ds_gob.alcBuffer);
		if(ret == IM_RET_OK){
			if(unlikely(copy_to_user((void *)(&((pmmlib_ioctl_ds_statc_get_owner_buffer_t *)arg)->alcBuffer), (void *)&ds_gob.alcBuffer, sizeof(alc_buffer_t)))){
				IM_ERRMSG((IM_STR("copy_to_user() failed!")));
				ret = IM_RET_FAILED;
				break;
			}
		}
		break;
	default:
		ret = IM_RET_FAILED;
		break;
	}

	if(unlock == IM_TRUE){
		mutex_unlock(&gPmmDriver->drvlock);
		inst->locked = IM_FALSE;
	}
	return IM_FAILED(ret) ? -EFAULT : 0;
}

static struct platform_driver pmm_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-pmm",
	},
	.probe = pmm_probe,
	.remove = pmm_remove,
	.suspend = pmm_suspend,
	.resume = pmm_resume,
};

static int pmm_probe(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	gPmmDriver = (pmm_drv_context_t *)kmalloc(sizeof(pmm_drv_context_t), GFP_KERNEL);
	if(gPmmDriver == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(gPmmDriver) failed")));
		return -1;
	}
	memset((void *)gPmmDriver, 0, sizeof(pmm_drv_context_t));

	mutex_init(&gPmmDriver->drvlock);

	return 0;
}

static int pmm_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_destroy(&gPmmDriver->drvlock);
	kfree(gPmmDriver);
	return 0;
}

static int pmm_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}
static int pmm_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}

static int __init pmm_init(void)
{
	int ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* Register platform device driver. */
	ret = platform_driver_register(&pmm_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register pmm platform device driver error, %d."),ret));
		return ret;
	}

	/* Register misc device driver. */
	ret = misc_register(&pmm_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register pmm misc device driver error, %d."),ret));
		return ret;
	}

	return 0;
}
static void __exit pmm_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	misc_deregister(&pmm_miscdev);
	platform_driver_unregister(&pmm_plat);
}
module_init(pmm_init);
module_exit(pmm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leo.Zhang of InfoTM");
MODULE_DESCRIPTION("Page-based memory management and device mmu driver");

