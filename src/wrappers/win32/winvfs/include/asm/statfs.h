#pragma once

#include "linux/types.h"

//
//	/include/asm-i386/statfs.h
//

typedef __kernel_fsid_t fsid_t;

struct statfs {
	long f_type;
	long f_bsize;
	long f_blocks;
	long f_bfree;
	long f_bavail;
	long f_files;
	long f_ffree;
	__kernel_fsid_t f_fsid;
	long f_namelen;
	long f_spare[6];
};
