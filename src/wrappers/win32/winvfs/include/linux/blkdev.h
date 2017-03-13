#ifndef _LINUX_BLKDEV_H
#define _LINUX_BLKDEV_H

#include "linux/kdev_t.h"

//who includes this will need to #include <ntifs.h>

static int get_hardsect_size(kdev_t dev)
{
	int retval = 512;
	if (dev->SectorSize)
		retval = dev->SectorSize;
	return retval;
}

static int blocksize_bits(int size)
{
	unsigned int bits = 8;
	do {
		bits++;
		size >>= 1;
	} while (size > 256);
	return bits;
}

#endif