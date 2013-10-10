/*
 * memalloc.c 
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description:
 * 	Main file of memalloc char driver to allocate physical address for 
 * 	multi-media. This char driver contains a lot of annotation to 
 * 	explain how and why I designed it to be. And still, there are
 * 	problems and bugs with it, so I hope anyone change or add something
 * 	to it in the future, write your name and reason/declaration.
 *
 * Author:
 * 	Sololz <sololz.luo@gmail.com>.
 *      
 * Revision Version: 3.0
 */

#include "memalloc.h"

/* Global variables in memalloc driver. */
static memalloc_global_t m_global;

/* Process memory block list node. */
static int m_reset_mem(memalloc_inst_t *inst);
static int m_insert_mem(unsigned int paddr, unsigned int size, memalloc_inst_t *inst);
#ifdef CONFIG_IMAP_MEMALLOC_USE_KMALLOC
static int m_free_mem(unsigned int paddr, memalloc_inst_t *inst);
#endif

/* Reserved memory management. */
static int rsv_alloc(unsigned int size, unsigned int *paddr, memalloc_inst_t *inst);
static int rsv_free(unsigned int paddr, memalloc_inst_t *inst);

/*
 * FUNCTION
 * memalloc_open()
 *
 * System call open of memalloc, /dev/memalloc can be open 256 times.
 */
static int memalloc_open(struct inode *inode, struct file *file)
{
    memalloc_inst_t *inst = NULL;

    /* Allocate instance data structure first, avoid kmalloc be called at
     * lock time. */
    inst = (memalloc_inst_t *)kmalloc(sizeof(memalloc_inst_t), GFP_KERNEL);
    if (unlikely(inst == NULL)) {
        memalloc_error("kmalloc() for instance error.\n");
        return -ENOMEM;
    }
    memset(inst, 0x00, sizeof(memalloc_inst_t));

    mutex_lock(&m_global.m_lock);
    if (unlikely(m_global.inst_count >= MEMALLOC_MAX_OPEN)) {
        mutex_unlock(&m_global.m_lock);

        memalloc_error("More than %d times have been openned, no more instances.\n",
                MEMALLOC_MAX_OPEN);
        kfree(inst);
        return -EPERM;
    }
    m_global.inst_count++;
    mutex_unlock(&m_global.m_lock);

    file->private_data = inst;
    memalloc_debug("Memalloc open OK.\n");

    return 0;
}

/*
 * FUNCTION
 * memalloc_release()
 *
 * All memory block should be released before memalloc_release. If you 
 * forget to, memalloc_release will check it and fix your mistake.
 */
static int memalloc_release(struct inode *inode, struct file *file)
{
    memalloc_inst_t *inst = (memalloc_inst_t *)file->private_data;

    /* Check argument. */
    if (unlikely(inst == NULL)) {
        memalloc_error("Argument error, instance private data unexpected to be NULL.\n");
        return -EINVAL;
    }

    mutex_lock(&m_global.m_lock);
    /* Free memalloc instance structure. */
    if (unlikely(inst == NULL)) {
        /* Process nothing. */
        memalloc_debug("File private data already NULL.\n");
    } else {
        /* Check whether allocated memory release normally, it's a MUST step. */
        m_reset_mem(inst);

        kfree(inst);
    }
    m_global.inst_count--;
    mutex_unlock(&m_global.m_lock);

    memalloc_debug("Memalloc release OK.\n");
    return 0;
}

/*
 * FUNCTION
 * memalloc_ioctl()
 *
 * Don't forget to call free ioctl if you have called any allocate ops.
 */
static long memalloc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    memalloc_inst_t *inst = NULL;

    inst = (memalloc_inst_t *)file->private_data;
    if (unlikely(inst == NULL)) {
        memalloc_error("IO control system call error file.\n");
        return -EFAULT;
    }

    /* Check command. */
    if (unlikely(_IOC_TYPE(cmd) != MEMALLOC_MAGIC)) {
        memalloc_error("IO control command magic error.\n");
        return -ENOTTY;
    }
    if (unlikely(_IOC_NR(cmd) > MEMALLOC_MAX_CMD)) {
        memalloc_error("IO control command number error.\n");
        return -ENOTTY;
    }
    if (_IOC_DIR(cmd) & _IOC_READ) {
        if (unlikely(!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))) {
            memalloc_error("IO control command requires read access but buffer unwritable.\n");
            return -EFAULT;
        }
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        if (unlikely(!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd)))) {
            memalloc_error("IO control command requires write access but buffer unreadable.\n");
            return -EFAULT;
        }
    }

    switch (cmd) {
        /* Reset current instance memory blocks. */
        case MEMALLOC_RESET:
            mutex_lock(&m_global.m_lock);
            m_reset_mem(inst);
            mutex_unlock(&m_global.m_lock);
            break;

            /*
             * Memory alloc by kmalloc with flags GFP_DMA | GFP_ATOMIC
             * the max size may be 8MB, 16MB, 32MB, you can change or
             * get this in kernel configuration.
             */
        case MEMALLOC_GET_BUF:
            {
                memalloc_param_t param;

                /* Copy user data first to avoid be called at lock time. */
                if (unlikely(copy_from_user(&param, (memalloc_param_t *)arg, sizeof(memalloc_param_t)))) {
                    memalloc_error("copy_from_user() error.\n");
                    return -EFAULT;
                }
                /* Check parameters. */
                if (unlikely((param.size == 0) || (param.size > MEMALLOC_MAX_ALLOC_SIZE))) {
                    memalloc_error("Get buffer parameters error.\n");
                    return -EINVAL;
                }

                mutex_lock(&m_global.m_lock);
                if (unlikely(inst->alloc_count >= MEMALLOC_MAX_ALLOC)) {
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("No more allocation allowed in this instance.\n");
                    return -ENOMEM;
                }

                /*
                 * Allocation method: 
                 * Reserved memory is default alloc memory region, if any error occurs
                 * while allocating reserved memory, program will transfer to dynamic
                 * kernel memory resource alloc.
                 */
                if (unlikely(rsv_alloc(param.size, &(param.paddr), inst) != 0)) {
#ifdef CONFIG_IMAP_MEMALLOC_USE_KMALLOC
                    mutex_unlock(&m_global.m_lock);
                    memalloc_debug("Alloc reserved memory error.\n");

                    /* 
                     * Here I use kmalloc() with flag GFP_KERNEL but not GFP_ATOMIC because I suppose
                     * some sleep here is reasonable. The disadvantage is it makes the code structure
                     * complicated considering the lock area. Code within lock area should be as fast
                     * as possible, best be atomic.
                     */
                    vaddr = (void *)kmalloc(param.size, GFP_KERNEL);
                    if (vaddr == NULL) {
                        memalloc_error("kmalloc() for buffer failed!\n");
                        return -ENOMEM;
                    }

                    /* Recheck and relock, it's very important!! */
                    mutex_lock(&m_global.m_lock);
                    if (unlikely(inst->alloc_count >= MEMALLOC_MAX_ALLOC)) {
                        mutex_unlock(&m_global.m_lock);
                        memalloc_error("No more allocation allowed in this instance.\n");
                        kfree(vaddr);
                        return -ENOMEM;
                    }

                    param.paddr = (unsigned int)virt_to_phys((unsigned long *)vaddr);
                    /* Each instance has a memory allocation list, it's max management number is 256
                     * each time alloc ioctl called, file private data will update */
                    m_insert_mem(param.paddr, param.size, inst);
#else
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("Alloc memory error.\n");

                    return -ENOMEM;
#endif
                }
                inst->alloc_count++;
                mutex_unlock(&m_global.m_lock);

                if (unlikely(copy_to_user((memalloc_param_t *)arg, &param, sizeof(memalloc_param_t)))) {
                    memalloc_error("copy_to_user() error.\n");
                    return -EFAULT;
                }

                break;
            }

        case MEMALLOC_FREE_BUF:
            {
                memalloc_param_t param;

                /* Copy first. */
                if (unlikely(copy_from_user(&param, (memalloc_param_t *)arg, sizeof(memalloc_param_t)))) {
                    memalloc_error("copy_from_user() error.\n");
                    return -EFAULT;
                }

                mutex_lock(&m_global.m_lock);
                if (unlikely(inst->alloc_count <= 0)) {
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("Nothing to be free.\n");
                    return -ENOMEM;
                }

                /* 
                 * First find whether physical address correspond memory region is in 
                 * reserved memory or not. if yes correspond memory region will be released.
                 * If memory region is not allocate by reserved memory, I will call kfree
                 * to free memory region.
                 */
                if (unlikely(rsv_free(param.paddr, inst) != 0)) {
#ifdef CONFIG_IMAP_MEMALLOC_USE_KMALLOC
                    /* memalloc_debug("rsv_free() error.\n"); */
                    if (unlikely(m_free_mem(param.paddr, inst))) {
                        mutex_unlock(&m_global.m_lock);
                        return -EFAULT;
                    }
#else
                    mutex_unlock(&m_global.m_lock);

                    memalloc_error("Free memory error.\n");
                    return -EFAULT;
#endif
                }
                inst->alloc_count--;
                mutex_unlock(&m_global.m_lock);

                param.size = 0;
                if (unlikely(copy_to_user((memalloc_param_t *)arg, &param, sizeof(memalloc_param_t)))) {
                    memalloc_error("copy_to_user() error.\n");
                    return -EFAULT;
                }

                break;
            }

        case MEMALLOC_FLUSH_RSV:
	    /* Do nothing here, because this interface is too much dangerous. */
	    break;

        case MEMALLOC_SET_CACHED:
            mutex_lock(&m_global.m_lock);
            __get_user(inst->cached_mark, (unsigned int *)arg);
            mutex_unlock(&m_global.m_lock);
            break;

        case MEMALLOC_GET_TOTAL_SIZE:
            __put_user(m_global.rsv_size, (uint32_t *)arg);
            break;

        case MEMALLOC_GET_LEFT_SIZE:
            __put_user(m_global.rsv_size - m_global.trace_memory, (uint32_t *)arg);
            break;

        case MEMALLOC_ALLOC_WITH_KEY:
            {
                rsv_mem_t *tmp_cur = NULL;
                awk_param_t awk;

                if (unlikely(copy_from_user(&awk, (awk_param_t *)arg, 
                                sizeof(awk_param_t)))) {
                    memalloc_error("copy_from_user() error.\n");
                    return -EFAULT;
                }

                mutex_lock(&m_global.m_lock);
                if (unlikely(m_global.shared_inst.alloc_count >= MEMALLOC_MAX_ALLOC)) {
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("No more shared memory block to alloc.\n");
                    return -EINVAL;
                }

                /* Find key. */
                tmp_cur = m_global.rsv_head;
                while (tmp_cur != NULL) {
                    if (tmp_cur->key == awk.key) {
                        break;
                    }
                    tmp_cur = tmp_cur->next;
                }
                if (tmp_cur == NULL) {	/* Does not exist, create a new one. */
                    if (unlikely(rsv_alloc(awk.size, &awk.addr, 
                                    &m_global.shared_inst))) {
                        mutex_unlock(&m_global.m_lock);
                        memalloc_error("Allocate shared memalloc block error.\n");
                        return -ENOMEM;
                    }
                    m_global.shared_inst.alloc_count++;
                    /* Find phys. */
                    tmp_cur = m_global.rsv_head;
                    while (tmp_cur != NULL) {
                        if (tmp_cur->phys == awk.addr) {
                            break;
                        }
                        tmp_cur = tmp_cur->next;
                    }
                    if (unlikely(tmp_cur == NULL)) {
                        mutex_unlock(&m_global.m_lock);
                        memalloc_error("Shared memalloc blocks flow error.\n");
                        return -EFAULT;
                    }
                    tmp_cur->key = awk.key;
                    tmp_cur->ref = 1;
                    awk.fresh = 1;
                    memalloc_debug("Allocate a new shared block %x.\n", awk.key);
                } else {		/* Get existing one. */
                    tmp_cur->ref++;
                    awk.addr = tmp_cur->phys;
                    awk.size = tmp_cur->size;
                    awk.fresh = 0;
                    memalloc_debug("Reuse a shared block %x.\n", awk.key);
                }
                mutex_unlock(&m_global.m_lock);

                if (unlikely(copy_to_user((awk_param_t *)arg,
                                &awk, 
                                sizeof(awk_param_t)))) {
                    memalloc_error("copy_to_user() error.\n");
                    return -EFAULT;
                }
                break;
            }

        case MEMALLOC_FREE_WITH_KEY:
            {
                rsv_mem_t *tmp_cur = NULL;
                awk_param_t awk;

                if (unlikely(copy_from_user(&awk, (awk_param_t *)arg, 
                                sizeof(awk_param_t)))) {
                    memalloc_error("copy_from_user() error.\n");
                    return -EFAULT;
                }

                mutex_lock(&m_global.m_lock);
                if (unlikely(m_global.shared_inst.alloc_count <= 0)) {
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("Error shared block free.\n");
                    return -EINVAL;
                }

                /* Find memory block. */
                tmp_cur = m_global.rsv_head;
                while (tmp_cur != NULL) {
                    if (tmp_cur->key == awk.key) {
                        break;
                    }
                    tmp_cur = tmp_cur->next;
                }
                if (likely(tmp_cur != NULL)) {
                    tmp_cur->ref--;
                    tmp_cur->ref = (tmp_cur->ref < 0) ? 0 : tmp_cur->ref;
                    if (!tmp_cur->ref) {
                        rsv_free(tmp_cur->phys, &m_global.shared_inst);
                        m_global.shared_inst.alloc_count--;
                    }
                } else {
                    mutex_unlock(&m_global.m_lock);
                    memalloc_error("Unexisting shared memalloc block.\n");
                    return -EINVAL;
                }
                memalloc_alert("Shared block %x freed.\n", awk.key);
                mutex_unlock(&m_global.m_lock);

                /*
                   if (unlikely(copy_to_user((awk_param_t *)arg,
                   &awk, 
                   sizeof(awk_param_t)))) {
                   memalloc_error("copy_to_user() error.\n");
                   return -EFAULT;
                   }
                   */
                break;
            }

        default:
            memalloc_error("Unknown ioctl command.\n");
            return -EFAULT;
    }

    memalloc_debug("Memalloc ioctl OK.\n");

    return 0;
}

/*
 * FUNCTION
 * memalloc_mmap
 *
 * mmap system call to map physical address to user space virtual address.
 */
static int memalloc_mmap(struct file *file, struct vm_area_struct *vma)
{
    size_t size = vma->vm_end - vma->vm_start;
    memalloc_inst_t	*inst	= (memalloc_inst_t *)file->private_data;

    /* Set memory map access to be uncached if cached memory is not required. */
    if (!inst->cached_mark) {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    }

    /* Do map. */
    if (unlikely(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
                    size, vma->vm_page_prot))) {
        memalloc_error("mmap for user space virtual address error.\n");
        return -EAGAIN;
    }

    return 0;
}

/* File operations structure basic for a char device driver. */
static struct file_operations memalloc_fops = {
    .owner = THIS_MODULE,
    .open = memalloc_open,
    .release = memalloc_release,
    .unlocked_ioctl = memalloc_ioctl,
    .mmap = memalloc_mmap,
};

#if defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_MISC_MODE)
/* 
 * Misc driver model structure, the reason to use misc to register memalloc driver
 * is because preversion method just use the device_create(), which is supposed to
 * the basic method, to do driver register and device node creation. The disadvantage
 * is that you must create a driver class and asign major and minor device number 
 * for it. So while the char device driver increasing, the major number of device 
 * will might be running out. The misc device driver use 10 major number, so it can
 * contains at most 256 different devices by asigning different minor from 0 to 256.
 * So there will still be a problem of number overflow, but much better than manual
 * asign causing conflicts.
 */
static struct miscdevice memalloc_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MEMALLOC_DEV_NAME,
    .fops = &memalloc_fops,
};
#endif

/*
 * FUNCTION
 * memalloc_probe()
 *
 * Register char device, alloc memory for mutex it's called by init function.
 */
static int memalloc_probe(struct platform_device *pdev)
{
    /* Initualize global variables. */
    memset(&m_global, 0x00, sizeof(memalloc_global_t));

#if defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_CLASS_MODE)
    /*
     * FIXME
     * The origin code is still not removed because if any problem with misc
     * driver model. I have met similar problems because of file system device
     * node create bug. So if you got some device node generate problems, use
     * this device_create().
     */

    /* 0 refers to dynamic alloc major number. */
    m_global.major = register_chrdev(MEMALLOC_DEFAULT_MAJOR, "memalloc", &memalloc_fops);
    if (unlikely(m_global.major < 0)) {
        memalloc_error("Register char device for memalloc error.\n");
        return -EFAULT;
    }

    m_global.major = MEMALLOC_DEFAULT_MAJOR;
    m_global.minor = MEMALLOC_DEFAULT_MINOR;

    m_global.m_class = class_create(THIS_MODULE, "memalloc");
    if (unlikely(m_global.m_class == NULL)) {
        memalloc_error("Create char device driver class error.\n");
        return -EFAULT;
    }

    device_create(m_global.m_class, NULL, MKDEV(m_global.major, m_global.minor), NULL, "memalloc");
#elif defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_MISC_MODE)
    if (unlikely(misc_register(&memalloc_miscdev))) {
        memalloc_error("Register misc memalloc driver error.\n");
        return -EFAULT;
    }
#else
#error "memalloc driver register mode not asigned!"
#endif

    /* Initialize globel memalloc lock. */
    mutex_init(&m_global.m_lock);

    /* RESERVED MEMORY */
    /*
     * Reserved memory start address is supposed to page size aligned. Reserved 
     * memory is set in kernel start boot config, best size and start address 
     * should be mutiple of 4KB(page size). Cuz if allocated buffer address is 
     * not page size aligned, mmap might fail(must fail in Android bionic lib).
     */
    {
#if defined(CONFIG_IMAP_MEMALLOC_MANUAL_RESERVE)
        m_global.rsv_phys = MEMALLOC_RSV_ADDR;
        m_global.rsv_size = MEMALLOC_RSV_SIZE;
        m_global.rsv_phys_end = m_global.rsv_phys + m_global.rsv_size;
#elif defined(CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE)
        m_global.rsv_phys = (unsigned int)imap_get_reservemem_paddr(RESERVEMEM_DEV_MEMALLOC);
        m_global.rsv_size = (unsigned int)imap_get_reservemem_size(RESERVEMEM_DEV_MEMALLOC);
        m_global.rsv_phys_end = m_global.rsv_phys + m_global.rsv_size;
        if (unlikely((m_global.rsv_phys == 0) || (m_global.rsv_size == 0))) {
            memalloc_error("memalloc reserved memory physical address or size is invalid.\n");
            return -EFAULT;
        }
#else
#error "Unknow memalloc reserved memory type!"
#endif
    }

    /*
     * Initialize reserved memory list header, at start of system boot, there is
     * just only one big memory region. This region is the size of reserved memory,
     * and start address is the reserved memory, any following allocation will devide
     * this origin block into many small regions. I only use one list to hold all 
     * memory block, so all connected memory regions is address linear. It will be 
     * very easy to merge consecutive idle block.
     */
    {
        m_global.rsv_head = (rsv_mem_t *)kmalloc(sizeof(rsv_mem_t), GFP_KERNEL);
        if (unlikely(m_global.rsv_head == NULL)) {
            memalloc_error("Alloc structure memory for reserved list head error.\n");
            return -ENOMEM;
        }
        memset(m_global.rsv_head, 0x00, sizeof(rsv_mem_t));
        m_global.rsv_head->mark = 0;	/* Supposed to be idle block. */
        m_global.rsv_head->pre = NULL;
        m_global.rsv_head->next = NULL;
        m_global.rsv_head->phys = m_global.rsv_phys;
        m_global.rsv_head->size = m_global.rsv_size;
    }

    /* PROCESS EXPORT RELATED VARIABLES */
    memset(&m_global.shared_inst, 0x00, sizeof(memalloc_inst_t));

    return 0;
}

/*
 * FUNCTION
 * memalloc_remove()
 *
 * Platform driver model remove function interface.
 */
static int memalloc_remove(struct platform_device *pdev)
{
    /* Recycle system resources, delete reserved memory list, and free all
     * allocated memory. */
    {
        rsv_mem_t *tmp_cur = NULL;
        rsv_mem_t *tmp_pre = NULL;

        if (likely(m_global.rsv_head != NULL)) {
            /* Head node's pre node pointer is supposed to be NULL. */
            tmp_cur = tmp_pre = m_global.rsv_head->next;

            do {
                /* Reach the end of reserved memory list. */
                if (tmp_cur == NULL) {
                    break;
                }

                tmp_cur = tmp_cur->next;
                kfree(tmp_pre);
                tmp_pre = tmp_cur;
            } while (1);

            kfree(m_global.rsv_head);
        }

        m_global.rsv_phys = 0;
        m_global.rsv_size = 0;
    }

#if defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_CLASS_MODE)
    device_destroy(m_global.m_class, MKDEV(m_global.major, m_global.minor));
    class_destroy(m_global.m_class);
    unregister_chrdev(m_global.major, "memalloc");
#elif defined(CONFIG_IMAP_MEMALLOC_DRIVER_REGISTER_MISC_MODE)
    misc_deregister(&memalloc_miscdev);
#endif

    mutex_destroy(&m_global.m_lock);
    mutex_destroy(&m_global.mpdma_lock);

    return 0;
}

/*
 * FUNCTION
 * memalloc_suspend()
 *
 * Platform driver model suspend function interface.
 */
static int memalloc_suspend(struct platform_device *pdev, pm_message_t state)
{
    return 0;
}

/*
 * FUNCTION
 * memalloc_resume()
 *
 * Platform driver model resume function interface.
 */
static int memalloc_resume(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver memalloc_driver = {
    .probe = memalloc_probe,
    .remove = memalloc_remove,
    .suspend = memalloc_suspend,
    .resume = memalloc_resume,
    .driver = {
        .owner = THIS_MODULE,
        .name = "imap-memalloc",
    },
};

static int __init memalloc_init(void)
{
    if (unlikely(platform_driver_register(&memalloc_driver))) {
        memalloc_error("Register platform device error.\n");
        return -EPERM;
    }

    memalloc_debug("Memalloc initualize OK\n");

    return 0;
}

static void __exit memalloc_exit(void)
{
    platform_driver_unregister(&memalloc_driver);

    memalloc_debug("Memalloc exit OK.\n");
}

module_init(memalloc_init);
module_exit(memalloc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
MODULE_DESCRIPTION("Memory tool for Decode and Encode mainly");

/*  
 * FUNCTION
 * m_reset_mem()
 *
 * This function free all memory allocated in current instance.
 */
int m_reset_mem(memalloc_inst_t *inst)
{
    int i = 0;

    for (i = 0; i < (sizeof(inst->pmemblock) / sizeof(inst->pmemblock[0])); i++) {
        if (inst->pmemblock[i].phys != 0) {
            memalloc_debug("Memory alloced by /dev/memalloc didn't free normally, fix here.\n");
            /* Check whether this memory block is in reserved memory. */
            if (unlikely(rsv_free(inst->pmemblock[i].phys, inst) != 0)) {
#ifdef CONFIG_IMAP_MEMALLOC_USE_KMALLOC
                /* FIXME: This code based on reserved memory is set at end of physical memory. */
                if (likely((inst->pmemblock[i].phys < m_global.rsv_phys) ||
                            (inst->pmemblock[i].phys > m_global.rsv_phys_end))) {
                    kfree((unsigned int *)phys_to_virt(inst->pmemblock[i].phys));
                }
#else
                memalloc_debug("Memory block is in instance structure, but not in reserved memory.\n");
#endif
            }

            inst->pmemblock[i].phys = 0;
            inst->pmemblock[i].size = 0;
        }
    }

    return 0;
}

/*
 * FUNCTION
 * m_insert_mem()
 *
 * This function insert a new allocated memory block in to instance record.
 */
int m_insert_mem(unsigned int paddr, unsigned int size, memalloc_inst_t *inst)
{
    int i = 0;

    for (i = 0; i < MEMALLOC_MAX_ALLOC; i++) {
        if (inst->pmemblock[i].phys == 0) {
            inst->pmemblock[i].phys = paddr;
            inst->pmemblock[i].size = size;
            memalloc_debug("Memory block get inserted.\n");
            break;
        }
    }

    return 0;
}

#ifdef CONFIG_IMAP_MEMALLOC_USE_KMALLOC
/*
 * FUNCTION
 * m_free_mem()
 *
 * This function free one memory block by its address,
 * I have to admit that it's not a good algorithm to locate.
 */
int m_free_mem(unsigned int paddr, memalloc_inst_t *inst)
{
    int i = 0;

    for (i = 0; i < MEMALLOC_MAX_ALLOC; i++) {
        if (inst->pmemblock[i].phys == paddr) {
            memalloc_debug("Memory block located to free.\n");
            if (likely(paddr < m_global.rsv_phys)) {
                kfree((unsigned long *)phys_to_virt(paddr));
            }

            inst->pmemblock[i].phys = 0;
            inst->pmemblock[i].size = 0;

            break;
        }

        if (unlikely(i == MEMALLOC_MAX_ALLOC - 1)) {
            memalloc_error("No such memory block.\n");
            return -ENOMEM;
        }
    }

    return 0;
}
#endif	/* CONFIG_IMAP_MEMALLOC_USE_KMALLOC */

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ RESERVED MEMORY FUNCTIONS */

/*
 * FUNCITON
 * rsv_alloc()
 *
 * HOWTO:
 * Here just use one memory region list to preserve memory block in
 * reserved memory area, and alloc method is low address first, and 
 * connected memory regions must be address sequence. This design will
 * be quite easy to guarantee that after free reserved memory region,
 * connected idle reserved memory region will be merged. But the disadvantage
 * of this method is that, if most of the allocated memory size is quite
 * so small, it will take a quite long time to find an suitable idle 
 * reserved memory block. O(n) time cost should be improved in the 
 * future.
 * Considering the reserved memory region is mainly used by media player
 * and display, most of the reserved memory is allocated at start of media
 * player start, and released at media player ends. So I set a current node
 * mark to mark for lastest allocation block. And next allocation will 
 * check from this pointer node and save alot of alloc time. 
 * This function allocate memory region in reserved memory, if success, 
 * will return 0, else a negative interger. This function not only do alloc
 * but also insert allocated memory region into instance structure.
 */
int rsv_alloc(unsigned int size, unsigned int *paddr, memalloc_inst_t *inst)
{
    rsv_mem_t *cur = NULL;
    rsv_mem_t *tmp = NULL;

    /* Size and paddr, inst validation need no check. */
    memalloc_debug("Get into reserved memory alloc.\n");

    /* 
     * From current list next node start, find fits blank memory block, rsv_cur 
     * is supposed to pointer to last allocate memory structure. Normally,
     * next node of rsv_cur should be a large blank memory region.
     */
    cur = m_global.rsv_head;
    do {
        if (cur == NULL) {
            memalloc_error("Can't find fit memory region in reserved memory.\n");
            return -ENOMEM;
        }

        if ((cur->mark == 0) && (cur->size >= size)) {
            /* Find suitable memory region. */
            if (cur->size == size) {
                cur->mark = 1;
                *paddr = cur->phys;
            } else {
                /* Current block is larger than required size, so devide current region. */
                tmp = (rsv_mem_t *)kmalloc(sizeof(rsv_mem_t), GFP_KERNEL);
                if (unlikely(tmp == NULL)) {
                    memalloc_error("kmalloc() for reserved memory node structure error.\n");
                    return -ENOMEM;
                }
                memset(tmp, 0x00, sizeof(rsv_mem_t));

                /* Update list structure. */
                /* Reset third node. */
                if (cur->next != NULL) {
                    cur->next->pre = tmp;
                }
                tmp->next = cur->next;
                tmp->pre = cur;
                cur->next = tmp;

                /* Reset devided memory region size, and address. */
                tmp->mark = 0;
                tmp->size = cur->size - size;
                tmp->phys = cur->phys + size;

                cur->mark = 1;
                cur->size = size;

                *paddr = cur->phys;
            }

            /* Insert current memory block into instance structure. */
            m_insert_mem(*paddr, size, inst);

            m_global.trace_memory += size;
#ifdef CONFIG_IMAP_MEMALLOC_MEMORY_USAGE_TRACE
            memalloc_alert("Alloc address 0x%08x, total memory usage %d.\n", *paddr, m_global.trace_memory);
#endif

            break;
        } else {
            cur = cur->next;
        }
    } while (1);

    memalloc_debug("Reserved memory alloc OK.\n");
    return 0;
}

/*
 * FUNCTION
 * rsv_free()
 *
 * First find correspond physical address in reserved memory, if memory
 * region found, it will be deleted from instance structure. Else, return
 * negative interger.
 */
int rsv_free(unsigned int paddr, memalloc_inst_t *inst)
{
    int i = 0;
    rsv_mem_t *cur = NULL;
    rsv_mem_t *tmp = NULL;

    /* Size and paddr, inst validation need no check. */
    memalloc_debug("Get into reserved memory free.\n");

    /*
     * XXX: At first this part code is located after checking in global memory
     * list. But suppose a situation like this, there are two threads/tasks 
     * access reserved memory, tA & tB. First A allocates one memory and the
     * address is aA, then tA free aA correspond memory. Then tB allocates
     * a piece of memory and the address and size is just as the aA in tA.
     * Then, tA unexpectly free aA again, in this case, the aA will be released
     * that we do not want it to. So at free, the address must be checked 
     * whether allocated by current fd, then do free.
     */
    /* Clean instance structure. */
    for (i = 0; i < MEMALLOC_MAX_ALLOC; i++) {
        if (inst->pmemblock[i].phys == paddr) {
            memalloc_debug("Memory block located to free.\n");
            inst->pmemblock[i].phys = 0;
            inst->pmemblock[i].size = 0;
            break;
        }

        if (i == MEMALLOC_MAX_ALLOC - 1) {
            memalloc_error("No such memory block in instance structure.\n");
            return -ENOMEM;
        }
    }

    /* 
     * Find correspond reserved memory region in reserved memory list, I have not
     * found a better method to find the paddr in reserved memory list. Currently, 
     * just find node from head.
     */
    cur = m_global.rsv_head;
    do {
        if (cur == NULL) {
            memalloc_debug("No such reserved memory in reserved memory list.\n");
            return -EINVAL;
        }

        if (cur->phys == paddr) {
            tmp = cur;	/* Tmp records fit memory block node pointer. */
            /* If someone free one memory block more than once, there will
             * be bug on memory usage trace. */
            if (tmp->mark) {
                tmp->mark = 0;	/* Set current memory block to be idle. */
            } else {
                memalloc_error("DON'T, current memory has been released.\n");
                break;
            }

            m_global.trace_memory -= cur->size;
#ifdef CONFIG_IMAP_MEMALLOC_MEMORY_USAGE_TRACE
            memalloc_alert("Free address 0x%08x, total memory usage %d.\n", paddr, m_global.trace_memory);
#endif

            if (tmp->pre != NULL) {
                /* Merge block, save prenode. */
                if (tmp->pre->mark == 0) {
                    cur = tmp->pre;

                    cur->size = cur->size + tmp->size;
                    cur->next = tmp->next;
                    if (cur->next != NULL) {
                        cur->next->pre = cur;
                    }
                }
            }

            if (cur->next != NULL) {
                if (cur->next->mark == 0) {
                    rsv_mem_t *trans = NULL;

                    cur->size = cur->size + cur->next->size;
                    trans = cur->next;
                    cur->next = cur->next->next;
                    if (cur->next != NULL) {
                        cur->next->pre = cur;
                    }

                    if (trans != NULL) {
                        kfree(trans);
                    }
                }
            }

            if (cur != tmp) {
                kfree(tmp);
            }

            break;
        }

        cur = cur->next;
    } while (1);

    memalloc_debug("Reserved memory free OK.\n");

    return 0;
}
