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
** v1.0.1	leo@2012/04/28: first commit.
** v1.0.3	leo@2012/10/24: change this drvier node to ipcx to avoid conflict with system ipc.
**
*****************************************************************************/ 

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
#include <asm/page.h>
#include <linux/delay.h>

#include <InfotmMedia.h>
#include <IM_ipcapi.h>
#include <ipc_uk.h>

#define DBGINFO		0
#define DBGWARN		1
#define DBGERR		1
#define DBGTIP		1

#define INFOHEAD	"IPCDRV_I:"
#define WARNHEAD	"IPCDRV_W:"
#define ERRHEAD		"IPCDRV_E:"
#define TIPHEAD		"IPCDRV_T:"


//
// ipc object status.
//
#define IPC_STAT_UNKNOWN	(0)

#define IPC_STAT_SET		(1) // set&reset for sync object.
#define IPC_STAT_RESET		(2)

#define IPC_STAT_LOCK		(1) // lock&unlock for shm object.
#define IPC_STAT_UNLOCK		(2)

#define IPC_STAT_INIT		(1) // init for pipe object.

//
#define MIN(a, b)		(((a) < (b)) ? (a) : (b))

// sync object.
typedef struct{
	void *			reserved;
}ipc_sync_t;

// shm object.
typedef struct{
	IM_INT32		pgnum;		// page number.
	void **			virAddrs;	// virtual Address array.
	IM_UINT32 *		pfns;		// pfn array.
	IM_BOOL			locked;		// locked flag.
}ipc_shm_t;

// pipe object.
#define PIPE_DATA_LIST_MAX_LEN	(1024)
typedef struct{
	IM_INT32			usage;
	IM_INT32			size;	// pipe buffer size.
	im_list_handle_t	bufferlst;	// list of buffer(void *), used in non-single mode.
	void *				buffer;	// virtual address, used in single mode.
}ipc_pipe_t;

// ipc object.
typedef struct{
	IM_INT32		type;	// IPC_TYPE_xxx.
	IM_INT32		stat;	// IPC_STAT_xxx.
	IM_TCHAR 		keystr[IPC_KEYSTR_MAX_LEN + 1];
	atomic_t		refcnt;
	struct mutex		lock;
	wait_queue_head_t	signal;
	atomic_t		trigger;
	union{
		ipc_sync_t	sync;
		ipc_shm_t	shm;
		ipc_pipe_t	pipe;
	}object;
}ipc_instance_t;

// user handle.
typedef struct{
	IM_INT32		type;	// IPC_TYPE_xxx.
	IM_INT32		stat;	// IPC_STAT_xxx.
	ipc_instance_t		*inst;	// one user can only get one ipc object.
}user_context_t;

// global driver context.
typedef struct{
	struct mutex		drvlock;	// here can create multi-lock for per-IPC-object.
	atomic_t		refcnt;

	im_mempool_handle_t	mpl;
	im_list_handle_t	synclst;	// list of ipc_instance_t.
	im_list_handle_t	shmlst;		// list of ipc_instance_t.
	im_list_handle_t	pipelst;	// list of ipc_instance_t.
}ipc_drv_context_t;

static ipc_drv_context_t *gIpcDriver = IM_NULL;

//
// function description.
//
static int ipc_open(struct inode *inode, struct file *file);
static int ipc_release(struct inode *inode, struct file *file);
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ipc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int ipc_probe(struct platform_device *pdev);
static int ipc_remove(struct platform_device *pdev);
static int ipc_suspend(struct platform_device *pdev, pm_message_t state);
static int ipc_resume(struct platform_device *pdev);
static unsigned int ipc_poll(struct file *file, poll_table *wait);
static int ipc_mmap(struct file *file, struct vm_area_struct *vma);


static ipc_instance_t * sync_init(IM_TCHAR *keyStr)
{
	int count;
	ipc_instance_t *inst;
	IM_INFOMSG((IM_STR("%s(keyStr=%s)"), IM_STR(_IM_FUNC_), keyStr));

	mutex_lock(&gIpcDriver->drvlock);
	inst = (ipc_instance_t *)im_list_begin(gIpcDriver->synclst);
	while(inst != IM_NULL){
		if(strcmp(keyStr, inst->keystr) == 0){
			break;
		}
		inst = (ipc_instance_t *)im_list_next(gIpcDriver->synclst);
	}

	if(inst == IM_NULL){
		inst = (ipc_instance_t *)kmalloc(sizeof(ipc_instance_t), GFP_KERNEL);
		if(inst == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc(ipc_instance_t) failed")));
			goto Fail;
		}
		memset((void *)inst, 0, sizeof(ipc_instance_t));

		inst->type = IPC_TYPE_SYNC;
		inst->stat = IPC_STAT_RESET;
		strcpy(inst->keystr, keyStr);
		mutex_init(&inst->lock);
		init_waitqueue_head(&inst->signal);
		atomic_set(&inst->trigger, 0);
		
		IM_ASSERT(im_list_put_back(gIpcDriver->synclst, (void *)inst) == IM_RET_OK);
	}

	mutex_unlock(&gIpcDriver->drvlock);

	//
	count = atomic_inc_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("sync(%s) refcnt=%d"), inst->keystr, count));

	return inst;
Fail:
	mutex_unlock(&gIpcDriver->drvlock);
	return IM_NULL;
}

static IM_RET sync_deinit(ipc_instance_t *inst)
{
	int count;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	count = atomic_dec_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("sync(%s) refcnt=%d"), inst->keystr, count));

	if(count == 0){
		mutex_lock(&gIpcDriver->drvlock);
		im_list_erase(gIpcDriver->synclst, (void *)inst);
		mutex_unlock(&gIpcDriver->drvlock);
		
		// Because refcnt==0, so now no anyone use this object still.
		mutex_destroy(&inst->lock);
		kfree((void *)inst);
	}
		
	return IM_RET_OK;
}

static IM_RET sync_set(ipc_instance_t *inst)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	//mutex_lock(&inst->lock);
	atomic_set(&inst->trigger, 1);
	//mutex_unlock(&inst->lock);

	wake_up(&inst->signal);

	return IM_RET_OK;
}

static IM_RET sync_reset(ipc_instance_t *inst)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	//mutex_lock(&inst->lock);
	atomic_set(&inst->trigger, 0);
	//mutex_unlock(&inst->lock);

	return IM_RET_OK;
}

static ipc_instance_t * pipe_init(ipc_ioctl_pipe_init_t *ds_pipe_init)
{
	int count;
	ipc_instance_t *inst;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, usage=0x%x, size=%d)"), 
		IM_STR(_IM_FUNC_), ds_pipe_init->keyStr, ds_pipe_init->usage, ds_pipe_init->size));

	mutex_lock(&gIpcDriver->drvlock);
	inst = (ipc_instance_t *)im_list_begin(gIpcDriver->pipelst);
	while(inst != IM_NULL){
		if(strcmp(ds_pipe_init->keyStr, inst->keystr) == 0){
			break;
		}
		inst = (ipc_instance_t *)im_list_next(gIpcDriver->pipelst);
	}

	if(inst == IM_NULL){
		inst = (ipc_instance_t *)kmalloc(sizeof(ipc_instance_t), GFP_KERNEL);
		if(inst == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc(ipc_instance_t) failed")));
			goto Fail;
		}
		memset((void *)inst, 0, sizeof(ipc_instance_t));

		if(ds_pipe_init->usage & IPC_PIPE_USAGE_SINGLE){
			inst->object.pipe.buffer = kmalloc(ds_pipe_init->size, GFP_KERNEL);
			if(inst->object.pipe.buffer == IM_NULL){
				IM_ERRMSG((IM_STR("kmalloc(pipe.buffer) failed")));
				kfree((void *)inst);
				goto Fail;
			}
		}else{
			inst->object.pipe.bufferlst = im_list_init(0, gIpcDriver->mpl);
			if(inst->object.pipe.bufferlst == IM_NULL){
				IM_ERRMSG((IM_STR("im_list_init(pipe.bufferlst) failed")));
				kfree((void *)inst);
				goto Fail;
			}
		}
		inst->object.pipe.usage = ds_pipe_init->usage;
		inst->object.pipe.size = ds_pipe_init->size;

		inst->type = IPC_TYPE_PIPE;
		inst->stat = IPC_STAT_INIT;
		strcpy(inst->keystr, ds_pipe_init->keyStr);
		mutex_init(&inst->lock);
		init_waitqueue_head(&inst->signal);
		atomic_set(&inst->trigger, 0);

		IM_ASSERT(im_list_put_back(gIpcDriver->pipelst, (void *)inst) == IM_RET_OK);
	}

	mutex_unlock(&gIpcDriver->drvlock);

	//
	count = atomic_inc_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("pipe(%s) refcnt=%d"), inst->keystr, count));

	return inst;
Fail:
	mutex_unlock(&gIpcDriver->drvlock);
	return IM_NULL;
}

static IM_RET pipe_deinit(ipc_instance_t *inst)
{
	int count;
	void *buffer;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	count = atomic_dec_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("pipe(%s) refcnt=%d"), inst->keystr, count));

	if(count == 0){
		mutex_lock(&gIpcDriver->drvlock);
		im_list_erase(gIpcDriver->pipelst, (void *)inst);
		mutex_unlock(&gIpcDriver->drvlock);
		
		// Because refcnt==0, so now no anyone use this object still.
		mutex_destroy(&inst->lock);
		if(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE){
			kfree(inst->object.pipe.buffer);
		}else{
			buffer = im_list_begin(inst->object.pipe.bufferlst);
			while(buffer != IM_NULL){
				kfree(buffer);
				buffer = im_list_erase(inst->object.pipe.bufferlst, buffer);
			}
			im_list_deinit(inst->object.pipe.bufferlst);
		}
		kfree((void *)inst);
	}
		
	return IM_RET_OK;
}

static IM_RET pipe_read(ipc_instance_t *inst, ipc_ioctl_pipe_rw_t *ds_pipe_rw)
{
	void *buffer;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	mutex_lock(&inst->lock);
	if(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE){
		buffer = inst->object.pipe.buffer;
	}else{
		buffer = im_list_begin(inst->object.pipe.bufferlst);
	}
	IM_ASSERT(buffer != IM_NULL);

	if(unlikely(copy_to_user(ds_pipe_rw->data, buffer, MIN(inst->object.pipe.size, ds_pipe_rw->size)))){
		IM_ERRMSG((IM_STR("copy_to_user(pipe) failed!")));
		mutex_unlock(&inst->lock);
		return IM_RET_FAILED;
	}

	if(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE){
		atomic_set(&inst->trigger, 0);
	}else{
		kfree(buffer);
		im_list_erase(inst->object.pipe.bufferlst, buffer);	
		if(im_list_size(inst->object.pipe.bufferlst) == 0){
			atomic_set(&inst->trigger, 0);
		}else{
			wake_up(&inst->signal);
		}
	}

	mutex_unlock(&inst->lock);
	return IM_RET_OK;
}

static IM_RET pipe_write(ipc_instance_t *inst, ipc_ioctl_pipe_rw_t *ds_pipe_rw)
{
	void *buffer;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	
	mutex_lock(&inst->lock);
	if(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE){
		buffer = inst->object.pipe.buffer;
	}else{
		buffer = kmalloc(inst->object.pipe.size, GFP_KERNEL);
		if(buffer == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc() failed")));
			mutex_unlock(&inst->lock);
			return IM_RET_FAILED;
		}
	}

	if(unlikely(copy_from_user(buffer, ds_pipe_rw->data, MIN(inst->object.pipe.size, ds_pipe_rw->size)))){
		IM_ERRMSG((IM_STR("copy_from_user(pipe) failed!")));
		if(!(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE)){
			kfree(buffer);
		}
		mutex_unlock(&inst->lock);
		return IM_RET_FAILED;
	}

	if(inst->object.pipe.usage & IPC_PIPE_USAGE_SINGLE){
		atomic_set(&inst->trigger, 1);
		wake_up(&inst->signal);
	}else{
		im_list_put_back(inst->object.pipe.bufferlst, buffer);
		if(im_list_size(inst->object.pipe.bufferlst) == 1){
			atomic_set(&inst->trigger, 1);
		}
		wake_up(&inst->signal);
	}
	
	mutex_unlock(&inst->lock);
	return IM_RET_OK;
}

static ipc_instance_t * shm_init(ipc_ioctl_shm_init_t *ds_shm_init)
{
	int count, size, i;
	ipc_instance_t *inst = IM_NULL;
	IM_INFOMSG((IM_STR("%s(keyStr=%s, size=%d)"), IM_STR(_IM_FUNC_), ds_shm_init->keyStr, ds_shm_init->size));

	mutex_lock(&gIpcDriver->drvlock);
	inst = (ipc_instance_t *)im_list_begin(gIpcDriver->shmlst);
	while(inst != IM_NULL){
		if(strcmp(ds_shm_init->keyStr, inst->keystr) == 0){
			break;
		}
		inst = (ipc_instance_t *)im_list_next(gIpcDriver->shmlst);
	}

	if(inst == IM_NULL){
		inst = (ipc_instance_t *)kmalloc(sizeof(ipc_instance_t), GFP_KERNEL);
		if(inst == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc(ipc_instance_t) failed")));
			goto Fail;
		}
		memset((void *)inst, 0, sizeof(ipc_instance_t));

		size = (ds_shm_init->size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
		inst->object.shm.pgnum = size >> PAGE_SHIFT;
		
		inst->object.shm.virAddrs = (void **)kmalloc(sizeof(void *) * inst->object.shm.pgnum, GFP_KERNEL);
		if(inst->object.shm.virAddrs == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc(virAddrs) failed")));
			goto Fail;
		}
		memset((void *)inst->object.shm.virAddrs, 0, sizeof(void *) * inst->object.shm.pgnum);
		
		inst->object.shm.pfns = (IM_UINT32 *)kmalloc(sizeof(IM_UINT32) * inst->object.shm.pgnum, GFP_KERNEL);
		if(inst->object.shm.pfns == IM_NULL){
			IM_ERRMSG((IM_STR("kmalloc(pfns) failed")));
			goto Fail;
		}
		memset((void *)inst->object.shm.pfns, 0, sizeof(IM_UINT32) * inst->object.shm.pgnum);

		for(i=0; i<inst->object.shm.pgnum; i++){
			inst->object.shm.virAddrs[i] = (void *)__get_free_page(GFP_KERNEL);
			if(inst->object.shm.virAddrs[i] == IM_NULL){
				IM_ERRMSG((IM_STR("__get_free_page() failed")));
				goto Fail;
			}
			inst->object.shm.pfns[i] = __pa(inst->object.shm.virAddrs[i]) >> PAGE_SHIFT;
			IM_INFOMSG((IM_STR("shm: i=%d, vir=0x%x, pfn=%d"), i, (IM_INT32)inst->object.shm.virAddrs[i], inst->object.shm.pfns[i]));
		}
		inst->object.shm.locked = IM_FALSE;

		inst->type = IPC_TYPE_SHM;
		inst->stat = IPC_STAT_UNLOCK;
		strcpy(inst->keystr, ds_shm_init->keyStr);
		mutex_init(&inst->lock);
		init_waitqueue_head(&inst->signal);

		atomic_set(&inst->trigger, 1);	// initial is unlocked.
		wake_up(&inst->signal);
		
		IM_ASSERT(im_list_put_back(gIpcDriver->shmlst, (void *)inst) == IM_RET_OK);
	}
	
	ds_shm_init->size = inst->object.shm.pgnum << PAGE_SHIFT;
	ds_shm_init->phyAddr = inst->object.shm.pfns[0] << PAGE_SHIFT;

	mutex_unlock(&gIpcDriver->drvlock);

	//
	count = atomic_inc_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("shm(%s) refcnt=%d"), inst->keystr, count));

	return inst;
Fail:
	if(inst){
		if(inst->object.shm.pfns){
			for(i=0; i<inst->object.shm.pgnum; i++){
				if(inst->object.shm.virAddrs[i] != IM_NULL){
					free_page((unsigned int)inst->object.shm.virAddrs[i]);
				}
			}
			kfree((void *)inst->object.shm.virAddrs);
			kfree((void *)inst->object.shm.pfns);
		}
		kfree((void *)inst);
	}	

	mutex_unlock(&gIpcDriver->drvlock);
	return IM_NULL;
}

static IM_RET shm_deinit(ipc_instance_t *inst)
{
	int count, i;
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	count = atomic_dec_return(&inst->refcnt);
	IM_INFOMSG((IM_STR("shm(%s) refcnt=%d"), inst->keystr, count));

	if(count == 0){
		mutex_lock(&gIpcDriver->drvlock);
		im_list_erase(gIpcDriver->shmlst, (void *)inst);
		mutex_unlock(&gIpcDriver->drvlock);
		
		// Because refcnt==0, so now no anyone use this object still.
		for(i=0; i<inst->object.shm.pgnum; i++){
			free_page((unsigned int)inst->object.shm.virAddrs[i]);
		}
		kfree((void *)inst->object.shm.virAddrs);
		kfree((void *)inst->object.shm.pfns);

		mutex_destroy(&inst->lock);
		kfree((void *)inst);
	}
		
	return IM_RET_OK;
}

static IM_RET shm_lock(ipc_instance_t *inst)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	mutex_lock(&inst->lock);
	IM_ASSERT(inst->object.shm.locked == IM_FALSE);
	inst->object.shm.locked = IM_TRUE;
	mutex_unlock(&inst->lock);
	return IM_RET_OK;
}

static IM_RET shm_unlock(ipc_instance_t *inst)
{
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));
	mutex_lock(&inst->lock);
	inst->object.shm.locked = IM_FALSE;
	atomic_set(&inst->trigger, 1);
	wake_up(&inst->signal);
	mutex_unlock(&inst->lock);
	return IM_RET_OK;
}

static struct file_operations ipcx_fops = {
    .owner = THIS_MODULE,
    .open = ipc_open,
    .release = ipc_release,
    .mmap = ipc_mmap,
#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
    .ioctl = ipc_ioctl,
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
    .unlocked_ioctl = ipc_ioctl,
#endif
    .poll = ipc_poll,
};
static struct miscdevice ipcx_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &ipcx_fops,
    .name = "ipcx",
};

static int ipc_open(struct inode *inode, struct file *file)
{
	int count;
	user_context_t *user; 
	IM_INFOMSG((IM_STR("%s() "), IM_STR(_IM_FUNC_)));

	user = (user_context_t *)kmalloc(sizeof(user_context_t), GFP_KERNEL);
	if(user == IM_NULL){
		IM_ERRMSG((IM_STR("kmalloc(user_context_t) failed")));
		return -EFAULT;
	}
	memset((void *)user, 0, sizeof(user_context_t));
	user->stat = IPC_STAT_UNKNOWN;
	file->private_data = (void *)user;

	//
	count = atomic_inc_return(&gIpcDriver->refcnt);
	IM_INFOMSG((IM_STR("gIpcDriver->refcnt=%d"), count));

	return 0;
}

static int ipc_release(struct inode *inode, struct file *file)
{
	int count;
	user_context_t *user = (user_context_t *)file->private_data;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	if(user->stat != IPC_STAT_UNKNOWN){
		if(user->type == IPC_TYPE_SYNC){
			sync_deinit(user->inst);
		}else if(user->type == IPC_TYPE_PIPE){
			pipe_deinit(user->inst);
		}else if(user->type == IPC_TYPE_SHM){
			if(user->stat == IPC_STAT_LOCK){
				shm_unlock(user->inst);
			}
			shm_deinit(user->inst);
		}
	}
	kfree(user);

	//
	count = atomic_dec_return(&gIpcDriver->refcnt);
	IM_INFOMSG((IM_STR("gIpcDriver->refcnt=%d"), count));

	return 0;
}

static int ipc_mmap(struct file *file, struct vm_area_struct *vma)
{
	IM_UINT32 i, num;	
	user_context_t *user = (user_context_t *)file->private_data;
	num = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	IM_ASSERT(user->type == IPC_TYPE_SHM);
	IM_ASSERT(num == user->inst->object.shm.pgnum);
	IM_ASSERT(vma->vm_pgoff == user->inst->object.shm.pfns[0]);

	IM_INFOMSG((IM_STR("%s(phy_addr=0x%lx, size=%ld)"), IM_STR(_IM_FUNC_), vma->vm_pgoff << PAGE_SHIFT, vma->vm_end - vma->vm_start));

	mutex_lock(&user->inst->lock);
	vma->vm_flags |= VM_RESERVED | VM_IO;
	for(i=0; i<num; i++){
		remap_pfn_range(vma, vma->vm_start + (i << PAGE_SHIFT), user->inst->object.shm.pfns[i], PAGE_SIZE, vma->vm_page_prot);
	}
	mutex_unlock(&user->inst->lock);

	return 0;
}

static unsigned int ipc_poll(struct file *file, poll_table *wait)
{
	int trigger, mask = 0;
	user_context_t *user = (user_context_t *)file->private_data;
	IM_ASSERT(user != IM_NULL);

	//IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	
	poll_wait(file, &user->inst->signal, wait);

	trigger = atomic_read(&user->inst->trigger);
	if(trigger != 0){
		mask = POLLIN | POLLRDNORM;
		if(user->type != IPC_TYPE_PIPE){	// pipe clear trigger at pipe_read().
			atomic_set(&user->inst->trigger, 0);
		}
	}

	return mask;
}

#if (TS_VER_MAJOR == 2 && TS_VER_MINOR == 6)
static int ipc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
#elif (TS_VER_MAJOR == 3 && TS_VER_MINOR == 0 )
static long ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
#endif
{
#define JIF(exp)	if(unlikely(exp)){goto Fail;}

	IM_RET ret = IM_RET_FAILED;
	user_context_t *user = (user_context_t *)file->private_data;
	IM_TCHAR keystr[IPC_KEYSTR_MAX_LEN + 1];
	ipc_ioctl_shm_init_t ds_shm_init;
	ipc_ioctl_pipe_init_t ds_pipe_init;
	ipc_ioctl_pipe_rw_t ds_pipe_rw;

	IM_INFOMSG((IM_STR("%s(cmd=0x%x)"), IM_STR(_IM_FUNC_), cmd));

	switch(cmd)
	{
	case IPC_IOCTL_SYNC_INIT:
		IM_ASSERT(user->inst == IM_NULL);
		JIF(copy_from_user((void *)keystr, (void *)arg, sizeof(keystr)));
		user->inst = sync_init(keystr);
		if(user->inst == IM_NULL){
			IM_ERRMSG((IM_STR("sync_init() failed")));
			goto Fail;
		}
		user->type = IPC_TYPE_SYNC;
		user->stat = IPC_STAT_RESET;
		break;
	case IPC_IOCTL_SYNC_DEINIT:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		ret = sync_deinit(user->inst);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("sync_deinit() failed")));
			goto Fail;
		}
		user->inst = IM_NULL;
		user->stat = IPC_STAT_UNKNOWN;
		break;
	case IPC_IOCTL_SYNC_SET:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		ret = sync_set(user->inst);	
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("sync_set() failed")));
			goto Fail;
		}
		user->stat = IPC_STAT_SET;
		break;
	case IPC_IOCTL_SYNC_RESET:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		ret = sync_reset(user->inst);	
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("sync_reset() failed")));
			goto Fail;
		}
		user->stat = IPC_STAT_RESET;
		break;
	case IPC_IOCTL_SHM_INIT:
		IM_ASSERT(user->inst == IM_NULL);
		JIF(copy_from_user((void *)&ds_shm_init, (void *)arg, sizeof(ipc_ioctl_shm_init_t)));
		user->inst = shm_init(&ds_shm_init);
		if(user->inst == IM_NULL){
			IM_ERRMSG((IM_STR("shm_init() failed")));
			goto Fail;
		}
		__put_user(ds_shm_init.size, (IM_INT32 *)(&(((ipc_ioctl_shm_init_t *)arg)->size)));
		__put_user(ds_shm_init.phyAddr, (IM_INT32 *)(&(((ipc_ioctl_shm_init_t *)arg)->phyAddr)));
		user->type = IPC_TYPE_SHM;
		user->stat = IPC_STAT_UNLOCK;
		break;
	case IPC_IOCTL_SHM_DEINIT:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		if(user->stat == IPC_STAT_LOCK){
			ret = shm_unlock(user->inst);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("shm_unlock() failed")));
				goto Fail;
			}
			user->stat = IPC_STAT_UNLOCK;
		}
		ret = shm_deinit(user->inst);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("shm_deinit() failed")));
			goto Fail;
		}
		user->inst = IM_NULL;
		user->stat = IPC_STAT_UNKNOWN;
		break;
	case IPC_IOCTL_SHM_LOCK:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		if(user->stat == IPC_STAT_UNLOCK){
			ret = shm_lock(user->inst);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("shm_lock() failed")));
				goto Fail;
			}
			user->stat = IPC_STAT_LOCK;
		}
		break;
	case IPC_IOCTL_SHM_UNLOCK:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		if(user->stat == IPC_STAT_LOCK){
			ret = shm_unlock(user->inst);
			if(ret != IM_RET_OK){
				IM_ERRMSG((IM_STR("shm_unlock() failed")));
				goto Fail;
			}
			user->stat = IPC_STAT_UNLOCK;
		}
		break;
	case IPC_IOCTL_PIPE_INIT:
		IM_ASSERT(user->inst == IM_NULL);
		JIF(copy_from_user((void *)&ds_pipe_init, (void *)arg, sizeof(ipc_ioctl_pipe_init_t)));
		user->inst = pipe_init(&ds_pipe_init);
		if(user->inst == IM_NULL){
			IM_ERRMSG((IM_STR("pipe_init() failed")));
			goto Fail;
		}
		user->type = IPC_TYPE_PIPE;
		user->stat = IPC_STAT_INIT;
		break;
	case IPC_IOCTL_PIPE_DEINIT:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		ret = pipe_deinit(user->inst);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("pipe_deinit() failed")));
			goto Fail;
		}
		user->stat = IPC_STAT_UNKNOWN;
		break;
	case IPC_IOCTL_PIPE_WRITE:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		JIF(copy_from_user((void *)&ds_pipe_rw, (void *)arg, sizeof(ipc_ioctl_pipe_rw_t)));
		ret = pipe_write(user->inst, &ds_pipe_rw);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("pipe_write() failed")));
			goto Fail;
		}
		break;
	case IPC_IOCTL_PIPE_READ:
		IM_ASSERT(user->inst != IM_NULL);
		IM_ASSERT(user->stat != IPC_STAT_UNKNOWN);
		JIF(copy_from_user((void *)&ds_pipe_rw, (void *)arg, sizeof(ipc_ioctl_pipe_rw_t)));
		ret = pipe_read(user->inst, &ds_pipe_rw);
		if(ret != IM_RET_OK){
			IM_ERRMSG((IM_STR("pipe_read() failed")));
			goto Fail;
		}
		break;
	default:
		IM_ERRMSG((IM_STR("unsupport cmd 0x%x"), cmd));
		goto Fail;
		break;
	}

	ret = IM_RET_OK;
Fail:
	return IM_FAILED(ret) ? -EFAULT : 0;

#undef JIF	
}

static struct platform_driver ipcx_plat = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx-ipcx",
	},
	.probe = ipc_probe,
	.remove = ipc_remove,
	.suspend = ipc_suspend,
	.resume = ipc_resume,
};

static void *ipc_malloc(IM_INT32 size)
{
	return kmalloc(size, GFP_KERNEL);
}

static int ipc_probe(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	//
	gIpcDriver = (ipc_drv_context_t *)kmalloc(sizeof(ipc_drv_context_t), GFP_KERNEL);
	if(gIpcDriver == IM_NULL){
		IM_ERRMSG((IM_STR("malloc(gIpcDriver) failed")));
		return -1;
	}
	memset((void *)gIpcDriver, 0, sizeof(ipc_drv_context_t));

	//
	mutex_init(&gIpcDriver->drvlock);

	gIpcDriver->mpl = im_mpool_init((func_mempool_malloc_t)ipc_malloc, (func_mempool_free_t)kfree);
	if(gIpcDriver->mpl == IM_NULL){
		IM_ERRMSG((IM_STR("im_mpool_init() failed")));
		goto Fail;
	}

	gIpcDriver->synclst = im_list_init(0, gIpcDriver->mpl);
	if(gIpcDriver->synclst == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init(synclst) failed")));
		goto Fail;
	}

	gIpcDriver->shmlst = im_list_init(0, gIpcDriver->mpl);
	if(gIpcDriver->shmlst == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init(shmlst) failed")));
		goto Fail;
	}

	gIpcDriver->pipelst = im_list_init(0, gIpcDriver->mpl);
	if(gIpcDriver->pipelst == IM_NULL){
		IM_ERRMSG((IM_STR("im_list_init(pipelst) failed")));
		goto Fail;
	}

	return 0;
Fail:
	if(gIpcDriver->pipelst){
		im_list_deinit(gIpcDriver->pipelst);
		gIpcDriver->pipelst = IM_NULL;
	}
	if(gIpcDriver->shmlst){
		im_list_deinit(gIpcDriver->shmlst);
		gIpcDriver->shmlst = IM_NULL;
	}
	if(gIpcDriver->synclst){
		im_list_deinit(gIpcDriver->synclst);
		gIpcDriver->synclst = IM_NULL;
	}
	if(gIpcDriver->mpl){
		im_list_deinit(gIpcDriver->mpl);
		gIpcDriver->mpl = IM_NULL;
	}
	mutex_destroy(&gIpcDriver->drvlock);
	return -1;
}

static int ipc_remove(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	mutex_destroy(&gIpcDriver->drvlock);
	im_list_deinit(gIpcDriver->pipelst);
	im_list_deinit(gIpcDriver->shmlst);
	im_list_deinit(gIpcDriver->synclst);
	im_list_deinit(gIpcDriver->mpl);
	kfree(gIpcDriver);
	return 0;
}

static int ipc_suspend(struct platform_device *pdev, pm_message_t state)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}
static int ipc_resume(struct platform_device *pdev)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	return 0;
}

static int __init ipcx_init(void)
{
	int ret = 0;
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));

	/* Register platform device driver. */
	ret = platform_driver_register(&ipcx_plat);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register ipc platform device driver error, %d."),ret));
		return ret;
	}

	/* Register misc device driver. */
	ret = misc_register(&ipcx_miscdev);
	if (unlikely(ret != 0)) {
		IM_ERRMSG((IM_STR(" Register ipc misc device driver error, %d."),ret));
		return ret;
	}

	return 0;
}
static void __exit ipcx_exit(void)
{
	IM_INFOMSG((IM_STR("%s()"), IM_STR(_IM_FUNC_)));
	misc_deregister(&ipcx_miscdev);
	platform_driver_unregister(&ipcx_plat);
}
module_init(ipcx_init);
module_exit(ipcx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leo.Zhang of InfoTM");
MODULE_DESCRIPTION("IPC driver");

