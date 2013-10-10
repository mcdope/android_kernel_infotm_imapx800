/*
 * Direct UBI block device access
 *
 * Author: dmitry pervushin <dimka@embeddedalley.com>
 * Change by warits,besto 
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2008 Embedded Alley Solutions, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Based on mtdblock by:
 * (C) 2000-2003 Nicolas Pitre <nico@cam.org>
 * (C) 1999-2003 David Woodhouse <dwmw2@infradead.org>
 */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mtd/ubi.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "ftl-ubi-core.h"

#if 0
#define START_DEBUG
#endif

#if 1
#define START_DEBUG_LINE
#endif


#if 0
#define END_DEBUG
#endif


#ifdef START_DEBUG
#define dbg_fn printk(KERN_ERR "USER DEBUG:%s\n",__func__)
#else 
#define dbg_fn 
#endif

#ifdef START_DEBUG_LINE
#define dbg_l  printk(KERN_ERR "USER DEBUG:%d\n",__LINE__)
#else
#define dbg_l
#endif 

#ifdef END_DEBUG
#define dbg_e  printk(KERN_ERR "USER DEBUG:%s end!!\n",__func__)
#else
#define dbg_e
#endif 

#define SECTOR_SIZE 512 

static LIST_HEAD(ubiblk_devices);

/**
 * ubiblk_dev - the structure representing translation layer
 *
 * @m: interface to mtd_blktrans
 * @ubi_num: UBI device number
 * @ubi_vol: UBI volume ID
 * @usecount: reference count
 * @ubi: ubi volume point to write or write form nand/ubi
 * @write_mutex: mutex to make more than one thread can work together
 * @auto_flush_work: use jiffies to make data can be sync to the nand automaticly
 *
 **/
struct ubiblk_dev {
      struct mtd_blktrans_dev m;

      int               ubi_num;
      int               ubi_vol;
      int               usecount;
      struct ubi_volume_desc *ubi;

      struct mutex	write_mutex;

      struct list_head list;

      struct work_struct unbind;

      struct workqueue_struct *auto_flush_queue;
      struct delayed_work  auto_flush_work;
};

static struct mtd_info *mbp;
extern struct mutex mtd_table_mutex;



static int ubiblock_open(struct mtd_blktrans_dev *mbd);
static int ubiblock_release(struct mtd_blktrans_dev *mbd);
static int ubiblock_flush(struct mtd_blktrans_dev *mbd);
static int ubiblock_readsect(struct mtd_blktrans_dev *mbd,
                        unsigned long block, char *buf);
static int ubiblock_writesect(struct mtd_blktrans_dev *mbd,
                        unsigned long block, char *buf);
static int ubiblock_getgeo(struct mtd_blktrans_dev *mbd,
            struct hd_geometry *geo);
static void *ubiblk_add(int ubi_num, int ubi_vol_id);
static void *ubiblk_add_locked(int ubi_num, int ubi_vol_id);
static int ubiblk_del(struct ubiblk_dev *u);
static int ubiblk_del_locked(struct ubiblk_dev *u);
/*
 * These two routines are just to satify mtd_blkdev's requirements
 */
static void ubiblock_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
      return;
}

static void ubiblock_remove_dev(struct mtd_blktrans_dev *mbd)
{
      return;
}

static struct mtd_blktrans_ops ubiblock_tr = {

      .name       = "ubiblk",
      .major      = 94,              /* assign dynamically  */
      .part_bits  = 0,              /* allow up to 8 parts */
      .blksize    = SECTOR_SIZE,

      .open       = ubiblock_open,
      .release    = ubiblock_release,
      .flush      = ubiblock_flush,
      .readsect   = ubiblock_readsect,
      .writesect  = ubiblock_writesect,
      .getgeo     = ubiblock_getgeo,

      .add_mtd    = ubiblock_add_mtd,
      .remove_dev = ubiblock_remove_dev,
      .owner      = THIS_MODULE,
};
	
	
static int ubiblock_getgeo(struct mtd_blktrans_dev *bdev,
            struct hd_geometry *geo)
{
      return -ENOTTY;
}

/**
 * ubiblock_writesect - write the sector
 *
 * Allocate the cache, if necessary and perform actual write using
 */
static int ubiblock_writesect(struct mtd_blktrans_dev *mbd,
                        unsigned long sector, char *buf)
{
      struct ubiblk_dev *u = container_of(mbd, struct ubiblk_dev, m);
      int err = 0;

      mutex_lock(&u->write_mutex);
      err = ftl_core_io_write(u->ubi, sector, buf); 
      mutex_unlock(&u->write_mutex);

      return err;
}

/**
 * ubiblk_readsect - read the sector
 *
 */
static int ubiblock_readsect(struct mtd_blktrans_dev *mbd,
                        unsigned long sector, char *buf)
{
      struct ubiblk_dev *u = container_of(mbd, struct ubiblk_dev, m);
      int err = 0;

      mutex_lock(&u->write_mutex);
      err = ftl_core_io_read(u->ubi, sector, buf);
      mutex_unlock(&u->write_mutex);

      return err;
}
/**
 * auto flush buffer.
 * If some other thread get the mutex to operate the nand or the buffer, the function wull return directly.
 */
static void auto_flush_buf(struct work_struct *work){
	struct ubiblk_dev *u = container_of((struct delayed_work *)work,
			struct ubiblk_dev, auto_flush_work);
	int ret = 0;
	ret = mutex_trylock(&u->write_mutex);
	if(!ret) goto end;
	ftl_core_io_auto_flush(u->ubi);
	mutex_unlock(&u->write_mutex);
end:
	queue_delayed_work(u->auto_flush_queue, &u->auto_flush_work, 199);
}


static int ubiblock_flush(struct mtd_blktrans_dev *mbd)
{
	struct ubiblk_dev *u = container_of(mbd, struct ubiblk_dev, m);
	int err = 0;

	mutex_lock(&u->write_mutex);
	err =ftl_core_io_flush(u->ubi);
	mutex_unlock(&u->write_mutex);

	return err; 
}


static int ubiblock_open(struct mtd_blktrans_dev *mbd)
{
	struct ubiblk_dev *u = container_of(mbd, struct ubiblk_dev, m);

	dbg_fn;

	if (u->usecount == 0) {
		u->ubi = ubi_open_volume(u->ubi_num, u->ubi_vol, UBI_READWRITE);
		if (IS_ERR(u->ubi))
			return PTR_ERR(u->ubi);
	}

	u->usecount++;

	return 0;
}

static int ubiblock_release(struct mtd_blktrans_dev *mbd)
{
	struct ubiblk_dev *u = container_of(mbd, struct ubiblk_dev, m);

	dbg_fn;

	if (--u->usecount == 0) {
		ubiblock_flush(mbd);
		ubi_close_volume(u->ubi);
		u->ubi = NULL;
	}

	return 0;
}

/*
 * sysfs routines. The ubiblk creates two entries under /sys/block/ubiblkX:
 *  - volume, R/O, which is read like "ubi0:volume_name"
 *  - unbind, W/O; when user writes something here, the block device is
 *  removed
 *
 *  unbind schedules a work item to perform real unbind, because sysfs entry
 *  handler cannot delete itself :)
 */
ssize_t volume_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct gendisk *gd = dev_to_disk(dev);
	struct mtd_blktrans_dev *m = gd->private_data;
	struct ubiblk_dev *u = container_of(m, struct ubiblk_dev, m);

	dbg_fn;
	return sprintf(buf, "%d:%d\n", u->ubi_num, u->ubi_vol);
}

static void ubiblk_unbind(struct work_struct *ws)
{
	struct ubiblk_dev *u = container_of(ws, struct ubiblk_dev, unbind);

	dbg_fn;
	ubiblk_del(u);
}

ssize_t unbind_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct gendisk *gd = dev_to_disk(dev);
	struct mtd_blktrans_dev *m = gd->private_data;
	struct ubiblk_dev *u = container_of(m, struct ubiblk_dev, m);

	dbg_fn;
	INIT_WORK(&u->unbind, ubiblk_unbind);
	schedule_work(&u->unbind);
	return count;
}

DEVICE_ATTR(unbind, 0644, NULL, unbind_store);
DEVICE_ATTR(volume, 0644, volume_show, NULL);

static int ubiblk_sysfs(struct gendisk *hd, int add)
{
	int r = 0;

	dbg_fn;
	if (add) {
		r = device_create_file(disk_to_dev(hd), &dev_attr_unbind);
		if (r < 0)
			goto out;
		r = device_create_file(disk_to_dev(hd), &dev_attr_volume);
		if (r < 0)
			goto out1;
		return 0;
	}

	device_remove_file(disk_to_dev(hd), &dev_attr_unbind);
out1:
	device_remove_file(disk_to_dev(hd), &dev_attr_volume);
out:
	return r;
}

/**
 * add the FTL by registering it with mtd_blkdevs
 */
static void *ubiblk_add(int ubi_num, int ubi_vol_id)
{
	void *p;

	dbg_fn;
	mutex_lock(&mtd_table_mutex);
	p = ubiblk_add_locked(ubi_num, ubi_vol_id);
	mutex_unlock(&mtd_table_mutex);

	return p;
}

static void *ubiblk_add_locked(int ubi_num, int ubi_vol_id)
{
	struct ubiblk_dev *u = kzalloc(sizeof(*u), GFP_KERNEL);
	struct ubi_volume_info uvi;
	struct ubi_volume_desc *ubi;

	int page_size, page_num, ret;

	dbg_fn;
	if (!u) {
		u = ERR_PTR(-ENOMEM);
		goto out;
	}

	ubi = ubi_open_volume(ubi_num, ubi_vol_id, UBI_READWRITE);

	u->ubi = ubi;
	if (IS_ERR(u->ubi)) {
		pr_err("USER DEBUG:cannot open the volume\n");
		u = (void *)u->ubi;
		goto out;
	}

	ubi_get_volume_info(ubi, &uvi);

	u->m.mtd = mbp;
	u->m.devnum = -1;
	u->m.tr = &ubiblock_tr;

	u->ubi_num = ubi_num;
	u->ubi_vol = ubi_vol_id;

	page_size = (u->m.mtd)->writesize;
	page_num = uvi.usable_leb_size/page_size;

	/*tell the filesystem how many sectors can be used,here I keep some blocks for recycle block.*/
	u->m.size = (uvi.size - uvi.size/10) * (uvi.usable_leb_size/SECTOR_SIZE); 
	printk(KERN_INFO "FTL: The volume of this partition is %luMBytes.\n", u->m.size >> 11);

	u->usecount = 0;

	mutex_init(&u->write_mutex);

	INIT_LIST_HEAD(&u->list);

	list_add_tail(&u->list, &ubiblk_devices);
	add_mtd_blktrans_dev(&u->m);
	ubiblk_sysfs(u->m.disk, true);

	ret = ftl_core_io_init(u->ubi, uvi.size, page_num, page_size, u->m.size);
	if(ret){
		printk(KERN_ERR "FTL: err = %d\n", ret);
		BUG();
	}

	u->auto_flush_queue = create_singlethread_workqueue("ubiblk");
	if(!u->auto_flush_queue) {
		printk(KERN_ERR "FTL: create work queue for cache_work failed.\n");
		BUG();
	}   
	INIT_DELAYED_WORK(&u->auto_flush_work, auto_flush_buf);

	auto_flush_buf(&(u->auto_flush_work.work));

	ubi_close_volume(ubi);

out:
	return u;
}

static int ubiblk_del(struct ubiblk_dev *u)
{
	int r;
	dbg_fn;
	mutex_lock(&mtd_table_mutex);
	r = ubiblk_del_locked(u);
	mutex_unlock(&mtd_table_mutex);
	return r;
}

static int ubiblk_del_locked(struct ubiblk_dev *u)
{
	dbg_fn;
	if (u->usecount != 0)
		return -EBUSY;
	ubiblk_sysfs(u->m.disk, false);
	del_mtd_blktrans_dev(&u->m);
	list_del(&u->list);
	kfree(u);
	return 0;
}

static struct ubiblk_dev *ubiblk_find(int num, int vol)
{
	struct ubiblk_dev *pos;

	dbg_fn;
	list_for_each_entry(pos, &ubiblk_devices, list)
		if (pos->ubi_num == num && pos->ubi_vol == vol)
			return pos;
	return NULL;
}

static int ubiblock_notification(struct notifier_block *blk,
		unsigned long type, void *v)
{
	struct ubi_notification *nt = v;
	struct ubiblk_dev *u;

	dbg_fn;
	switch (type) {
		case UBI_VOLUME_ADDED:
			ubiblk_add(nt->vi.ubi_num, nt->vi.vol_id);
			break;
		case UBI_VOLUME_REMOVED:
			u = ubiblk_find(nt->vi.ubi_num, nt->vi.vol_id);
			if (u)
				ubiblk_del(u);
			break;
		case UBI_VOLUME_RENAMED:
		case UBI_VOLUME_RESIZED:
			break;
	}
	return NOTIFY_OK;
}

static struct notifier_block ubiblock_nb = {
	.notifier_call = ubiblock_notification,
};

int ubiblock_init(void)
{
	int r;

	dbg_fn;


	mbp =  get_mtd_device_nm("local");

	r = register_mtd_blktrans(&ubiblock_tr);
	if (r)
		goto out;
	r = ubi_register_volume_notifier(&ubiblock_nb, 0);
	if (r)
		goto out_unreg;

	return 0;

out_unreg:
	deregister_mtd_blktrans(&ubiblock_tr);
out:
	return 0;
}
EXPORT_SYMBOL(ubiblock_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Caching block device emulation access to UBI devices");
MODULE_AUTHOR("dmitry pervushin <dimka@embeddedalley.com>,warits,besto");
