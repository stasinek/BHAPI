/*
 * Parts from include/quotaops.h from the Linux Kernel 2.4.25
 *
 * Definitions for diskquota-operations. When diskquota is configured these
 * macros expand to the right source-code.
 *
 * Author:  Marco van Wieringen <mvw@planets.elm.net>
 *
 * Version: $Id: quotaops.h,v 1.1.1.1 2004/07/05 17:14:59 sadyc Exp $
 *
 */
#ifndef _LINUX_QUOTAOPS_
#define _LINUX_QUOTAOPS_

#include <linux/config.h>
#include <linux/smp_lock.h>

#if defined(CONFIG_QUOTA)

#error Quota is not supported --sadyc

#else

/*
 * NO-OP when quota not configured.
 */
#define sb_dquot_ops				(NULL)
#define sb_quotactl_ops				(NULL)
#define sync_dquots_dev(dev, type)		do { } while(0)
#define DQUOT_INIT(inode)			do { } while(0)
#define DQUOT_DROP(inode)			do { } while(0)
#define DQUOT_ALLOC_INODE(inode)		(0)
#define DQUOT_FREE_INODE(inode)			do { } while(0)
#define DQUOT_SYNC_DEV(dev)			do { } while(0)
#define DQUOT_SYNC_SB(sb)			do { } while(0)
#define DQUOT_OFF(sb)				do { } while(0)
#define DQUOT_TRANSFER(inode, iattr)		(0)
typedef __u64 qsize_t;          // taken from quota.h
extern __inline__ int DQUOT_PREALLOC_SPACE_NODIRTY(struct inode *inode, qsize_t nr)
{
	lock_kernel();
	inode_add_bytes(inode, nr);
	unlock_kernel();
	return 0;
}

extern __inline__ int DQUOT_PREALLOC_SPACE(struct inode *inode, qsize_t nr)
{
	DQUOT_PREALLOC_SPACE_NODIRTY(inode, nr);
	mark_inode_dirty(inode);
	return 0;
}

extern __inline__ int DQUOT_ALLOC_SPACE_NODIRTY(struct inode *inode, qsize_t nr)
{
	lock_kernel();
	inode_add_bytes(inode, nr);
	unlock_kernel();
	return 0;
}

extern __inline__ int DQUOT_ALLOC_SPACE(struct inode *inode, qsize_t nr)
{
	DQUOT_ALLOC_SPACE_NODIRTY(inode, nr);
	mark_inode_dirty(inode);
	return 0;
}

extern __inline__ void DQUOT_FREE_SPACE_NODIRTY(struct inode *inode, qsize_t nr)
{
	lock_kernel();
	inode_sub_bytes(inode, nr);
	unlock_kernel();
}

extern __inline__ void DQUOT_FREE_SPACE(struct inode *inode, qsize_t nr)
{
	DQUOT_FREE_SPACE_NODIRTY(inode, nr);
	mark_inode_dirty(inode);
}	

#endif /* CONFIG_QUOTA */

#define DQUOT_PREALLOC_BLOCK_NODIRTY(inode, nr)	DQUOT_PREALLOC_SPACE_NODIRTY(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)
#define DQUOT_PREALLOC_BLOCK(inode, nr)	DQUOT_PREALLOC_SPACE(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)
#define DQUOT_ALLOC_BLOCK_NODIRTY(inode, nr) DQUOT_ALLOC_SPACE_NODIRTY(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)
#define DQUOT_ALLOC_BLOCK(inode, nr) DQUOT_ALLOC_SPACE(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)
#define DQUOT_FREE_BLOCK_NODIRTY(inode, nr) DQUOT_FREE_SPACE_NODIRTY(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)
#define DQUOT_FREE_BLOCK(inode, nr) DQUOT_FREE_SPACE(inode, ((qsize_t)(nr)) << (inode)->i_sb->s_blocksize_bits)

#endif /* _LINUX_QUOTAOPS_ */
