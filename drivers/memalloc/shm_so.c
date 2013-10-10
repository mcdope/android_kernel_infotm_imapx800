/*******************************************************************************
 * shm_so.c
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: 
 * 	Shared memory support for memalloc on Android system.
 *
 * Author:
 *	Sololz <sololz.luo@gmail.com>.
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.0  2011/01/03 Sololz
 * 	Create this file.
 ******************************************************************************/

#include "shm_so.h"

static shm_so_global_t shm_global;

/*
 * shm_so_open()
 */
static int shm_so_open(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

/*
 * shm_so_release()
 *
 * This release will check whether shared memory is valid and decrease the depend
 * user count. If reduced to 0, shared memory will be released.
 */
static int shm_so_release(struct inode *inode, struct file *file)
{
	shm_so_t *shm = (shm_so_t *)file->private_data;

	if (likely(shm != NULL)) {
		mutex_lock(&shm_global.gshms_lock);
		if (shm->deps > 0) {
			shm->deps--;
		} 
		if (shm->deps == 0) {
			/* Free current shared memory if no one depends on it any more. */
			shm_so_t *tmp = shm_global.gshms;

			while (tmp != NULL) {
				/* Found. */
				if (tmp == shm) {
					if (tmp->pre != NULL) {
						tmp->pre->next = tmp->next;
					}
					if (tmp->next != NULL) {
						tmp->next->pre = tmp->pre;
					}
					if (tmp == shm_global.gshms) {
						shm_global.gshms = shm_global.gshms->next;
					}

					if (likely(tmp->virt != NULL)) {
						kfree(tmp->virt);
					}
					kfree(tmp);

					memalloc_debug("Shared memory released.\n");
					break;
				}

				tmp = tmp->next;
				if (unlikely(tmp == NULL)) {
					memalloc_error("No correspond shared memory found.\n");
				}
			}
		}
		mutex_unlock(&shm_global.gshms_lock);
	}
	file->private_data = NULL;

	return 0;
}

/*
 * shm_so_read()
 */
static ssize_t shm_so_read(struct file * file, char __user * buf,
		size_t count, loff_t *ppos)
{
	ssize_t sz = count;
	shm_so_t *shm = (shm_so_t *)(file->private_data);

	if (unlikely(shm == NULL)) {
		memalloc_error("Unallocated shared memory.\n");
		return -EINVAL;
	} else if (unlikely(count <= 0)) {
		memalloc_error("Nothing to read.\n");
		return -EINVAL;
	}

	/* Check read overflow. */
	if (unlikely(count + shm->start > shm->size)) {
		up(&shm->sem);
		memalloc_error("Read size overflow.\n");
		return -EPERM;
	}

	if (unlikely(copy_to_user(buf, (void *)((unsigned int)shm->virt + shm->start), count))) {
		up(&shm->sem);
		memalloc_error("copy_to_user() error.\n");
		return -EFAULT;
	}
	up(&shm->sem);

	return sz;
}

/*
 * shm_so_write()
 */
static ssize_t shm_so_write(struct file * file, const char __user * buf,
		size_t count, loff_t *ppos)
{
	ssize_t sz = 0;
	shm_so_t *shm = (shm_so_t *)(file->private_data);

	if (unlikely(shm == NULL)) {
		memalloc_error("Unallocated shared memory.\n");
		return -EINVAL;
	} else if (unlikely(count <= 0)) {
		memalloc_error("Nothing to write.\n");
		return -EINVAL;
	}

	/* Check write overflow. */
	if (unlikely(count + shm->start > shm->size)) {
		up(&shm->sem);
		memalloc_error("Write size overflow.\n");
		return -EPERM;
	}

	if (unlikely(copy_from_user((void *)((unsigned int)shm->virt + shm->start), buf, count))) {
		up(&shm->sem);
		memalloc_error("copy_from_user() error.\n");
		return -EFAULT;
	}
	up(&shm->sem);

	return sz;
}

/*
 * shm_so_lseek()
 */
static loff_t shm_so_lseek(struct file * file, loff_t offset, int orig)
{
	loff_t cur = 0;
	shm_so_t *shm = (shm_so_t *)(file->private_data);

	if (unlikely(shm == NULL)) {
		memalloc_error("Unallocated shared memory.\n");
		return -EINVAL;
	}

	/* Here, lseek will hold the semaphore, till a read or write operation
	 * is done. */
	down(&shm->sem);
	switch (orig) {
		case 0:
		case 1:
		case 2:
			cur = offset;
			shm->start = offset;
			break;

		default:
			up(&shm->sem);
			memalloc_error("Error seek flag.\n");
			return -EINVAL;
	}

	return cur;
}

/*
 * shm_so_ioctl()
 */
static long shm_so_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	shm_so_get_t get;
	/* shm_so_t *shm = (shm_so_t *)(file->private_data); */
	shm_so_t *tmp = NULL;

	/* Check IO control command access permission validation. */
	if (unlikely(_IOC_TYPE(cmd) != SHM_SO_IOCMD_MAGIC)) {
		memalloc_error("Unknow IO control command magic.\n");
		return -EINVAL;
	} else if (unlikely(_IOC_NR(cmd) > SHM_SO_IOCMD_MAX_NUM)) {
		memalloc_error("Overflow IO control index.\n");
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ) {
		if (unlikely(!access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd)))) {
			memalloc_error("IO control request read but buffer unwritable.\n");
			return -EINVAL;
		}
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (unlikely(!access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd)))) {
			memalloc_error("IO control request write but buffer unreadable.\n");
			return -EINVAL;
		}
	}

	switch (cmd) {
		case SHM_SO_IOCMD_ALLOC:
			if (unlikely(copy_from_user(&get, (void *)arg, sizeof(shm_so_get_t)))) {
				memalloc_error("copy_from_user() error.\n");
				return -EFAULT;
			}
			if (unlikely((get.size <= 0) || (get.size > SHM_SO_MAX_ALLOC_SIZE))) {
				memalloc_error("Required shared memory size invalid.\n");
				return -EINVAL;
			}

			/* Check key and allocate. */
			mutex_lock(&shm_global.gshms_lock);
			tmp = shm_global.gshms;
			while (tmp != NULL) {
				/* Key corrspond shared memory has been allocated already. */
				if (tmp->key == get.key) {
					if (unlikely(tmp->size < get.size)) {
						mutex_unlock(&shm_global.gshms_lock);
						memalloc_error("Requred shared memory size overflow to origin.\n");
						return -EINVAL;
					}
					file->private_data = (void *)tmp;
					down(&tmp->sem);
					tmp->deps++;
					up(&tmp->sem);

					memalloc_alert("Shared memory reuse, 0x%08x.\n", get.key);
					break;
				}

				tmp = tmp->next;
			}

			/* If key correspond shared memory does not exist. */
			if (file->private_data == NULL) {
				/* Allocate shared memory data structure. */
				tmp = (shm_so_t *)kmalloc(sizeof(shm_so_t), GFP_KERNEL);
				if (unlikely(tmp == NULL)) {
					mutex_unlock(&shm_global.gshms_lock);
					memalloc_error("kmalloc() for new shared memory data struture error.\n");
					return -ENOMEM;
				}
				memset(tmp, 0x00, sizeof(shm_so_t));
				/* Allocate shared memory. */
				tmp->virt = (void *)kmalloc(get.size, GFP_KERNEL);
				if (unlikely(tmp->virt == NULL)) {
					mutex_unlock(&(shm_global.gshms_lock));
					memalloc_error("kmalloc for shared memory error.\n");
					kfree(tmp);
					return -ENOMEM;
				}
				/* Set allocated memory buffer content to be all 0. */
				memset(tmp->virt, 0x00, get.size);
				init_MUTEX(&tmp->sem);
				tmp->key = get.key;
				tmp->size = get.size;
				tmp->deps = 1;
				tmp->start = 0;
				tmp->pre = NULL;
				tmp->next = NULL;

				/* Insert allocated shared memory structure into global list. */
				if (shm_global.gshms == NULL) {
					shm_global.gshms = tmp;
				} else {
					tmp->next = shm_global.gshms;
					shm_global.gshms->pre = tmp;
					shm_global.gshms = tmp;
				}

				file->private_data = (void *)tmp;
				memalloc_alert("Shared memory allocated, 0x%08x.\n", get.key);
			}
			mutex_unlock(&shm_global.gshms_lock);

			break;

		default:
			memalloc_error("Error IO control command.\n");
			return -EINVAL;
	}

	return 0;
}

static struct file_operations shm_so_ops = {
	.owner = THIS_MODULE,
	.open = shm_so_open,
	.release = shm_so_release,
	.read = shm_so_read,
	.write = shm_so_write,
	.llseek = shm_so_lseek,
	.unlocked_ioctl = shm_so_ioctl,
};

static struct miscdevice shm_so_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SHM_SO_DEV_NAME,
	.fops = &shm_so_ops,
};

/*
 * shm_so_init()
 */
static int __init shm_so_init(void)
{
	memset(&shm_global, 0x00, sizeof(shm_so_global_t));

	mutex_init(&shm_global.gshms_lock);
	return misc_register(&shm_so_miscdev);
}

/*
 * shm_so_exit()
 */
static void __exit shm_so_exit(void)
{
	mutex_destroy(&shm_global.gshms_lock);
	misc_deregister(&shm_so_miscdev);
}

module_init(shm_so_init);
module_exit(shm_so_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
