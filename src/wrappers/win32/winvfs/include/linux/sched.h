#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H


static unsigned long event;

#include <linux/config.h>
#include <linux/capability.h> //from #include <linux/binfmts.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/semaphore.h>
#include <asm/page.h>
#include <linux/fs_struct.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <asm/current.h>

extern int in_group_p(gid_t); 

struct task_struct {
		uid_t uid,euid,suid,fsuid;
		gid_t gid,egid,sgid,fsgid;
		struct fs_struct *fs;
/* file system info */
		int link_count, total_link_count;
};

#define CURRENT_TIME VfsGetCurrentTime()
extern ULONG VfsGetCurrentTime();
#define wake_up(x)

#include <asm/current.h>

/*
 * capable() checks for a particular capability.  
 * New privilege checks should use this interface, rather than suser() or
 * fsuser(). See include/linux/capability.h for defined capabilities.
 */

static inline int capable(int cap)
{
#if 0 // does nothing now, just for compatibility --sadyc
#if 1 /* ok now */
	if (cap_raised(current->cap_effective, cap))
#else
	if (cap_is_fs_cap(cap) ? current->fsuid == 0 : current->euid == 0)
#endif
	{
		current->flags |= PF_SUPERPRIV;
		return 1;
	}
	return 0;
#endif
	return 1;
} 

#endif
