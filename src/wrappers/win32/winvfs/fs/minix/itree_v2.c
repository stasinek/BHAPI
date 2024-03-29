#include "linux/fs.h"
#include "linux/minix_fs.h"
#include "linux/locks.h"
#include "asm/errno.h"
#include "vfs.h"
#include "linux/smp_lock.h"

enum {DIRECT = 7, DEPTH = 4};	/* Have triple indirect */

typedef u32 block_t;	/* 32 bit, host order */

static __inline unsigned long block_to_cpu(block_t n)
{
	return n;
}

static inline block_t cpu_to_block(unsigned long n)
{
	return n;
}

static __inline block_t *i_data(struct inode *inode)
{
	return (block_t *)inode->u.minix_i.u.i2_data;
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
	} else if ((block -= 7) < 256) {
		offsets[n++] = 7;
		offsets[n++] = block;
	} else if ((block -= 256) < 256*256) {
		offsets[n++] = 8;
		offsets[n++] = block>>8;
		offsets[n++] = block & 255;
	} else {
		block -= 256*256;
		offsets[n++] = 9;
		offsets[n++] = block>>16;
		offsets[n++] = (block>>8) & 255;
		offsets[n++] = block & 255;
	}
	return n;
}

#include "itree_common.c"

int V2_minix_get_block(struct inode * inode, long block,
			struct buffer_head *bh_result, int create)
{
	return get_block(inode, block, bh_result, create);
}

void V2_minix_truncate(struct inode * inode)
{
	truncate(inode);
}
