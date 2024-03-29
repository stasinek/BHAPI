#include "linux/fs.h"
#include "linux/minix_fs.h"
#include "linux/locks.h"
#include "asm/errno.h"
#include "vfs.h"
#include "linux/smp_lock.h"

enum {DEPTH = 3, DIRECT = 7};	/* Only double indirect */

typedef u16 block_t;	/* 16 bit, host order */

static __inline unsigned long block_to_cpu(block_t n)
{
	return n;
}

static __inline block_t cpu_to_block(unsigned long n)
{
	return (block_t)n;
}

static __inline block_t *i_data(struct inode *inode)
{
	return (block_t *)inode->u.minix_i.u.i1_data;
}

static int block_to_path(struct inode * inode, long block, int offsets[DEPTH])
{
	int n = 0;

	if (block < 0) {
		printk("minix_bmap: block<0");
	} else if (block >= (long)(inode->i_sb->u.minix_sb.s_max_size/BLOCK_SIZE)) {
		printk("minix_bmap: block>big");
	} else if (block < 7) {
		offsets[n++] = block;
	} else if ((block -= 7) < 512) {
		offsets[n++] = 7;
		offsets[n++] = block;
	} else {
		block -= 512;
		offsets[n++] = 8;
		offsets[n++] = block>>9;
		offsets[n++] = block & 511;
	}
	return n;
}

#include "itree_common.c"

int V1_minix_get_block(struct inode * inode, long block,
			struct buffer_head *bh_result, int create)
{
	return get_block(inode, block, bh_result, create);
}

void V1_minix_truncate(struct inode * inode)
{
	truncate(inode);
}
