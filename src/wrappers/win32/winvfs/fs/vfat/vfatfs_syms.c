/*
 * linux/fs/msdos/vfatfs_syms.c
 *
 * Exported kernel symbols for the VFAT filesystem.
 * These symbols are used by dmsdos.
 */

#include "todo.h"
#include "vfs.h"

#include <linux/module.h>
#include <linux/init.h>

#include <linux/mm.h>
#include <linux/msdos_fs.h>

DECLARE_FSTYPE_DEV(vfat_fs_type, "vfat", vfat_read_super);

EXPORT_SYMBOL(vfat_create);
EXPORT_SYMBOL(vfat_unlink);
EXPORT_SYMBOL(vfat_mkdir);
EXPORT_SYMBOL(vfat_rmdir);
EXPORT_SYMBOL(vfat_rename);
EXPORT_SYMBOL(vfat_read_super);
EXPORT_SYMBOL(vfat_lookup);

extern int init_fat_fs(void);

int fs_init_module(void)
{	
	init_fat_fs();
	return register_filesystem(&vfat_fs_type);
}

void fs_cleanup_module(void)
{
	unregister_filesystem(&vfat_fs_type);
}

