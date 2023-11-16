/*
 * Copyright 2009 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		François Revol, revol@free.fr
 */
#ifndef KERNEL_BOOT_PLATFORM_UBOOT_UIMAGE_H
#define KERNEL_BOOT_PLATFORM_UBOOT_UIMAGE_H

#include <Haiku.h>

/* same type and constant names as U-Boot */

#define IH_TYPE_STANDALONE 1
#define IH_TYPE_KERNEL 2
#define IH_TYPE_RAMDISK 3
#define IH_TYPE_MULTI 4

#define IH_COMP_NONE 0

#define IH_MAGIC 0x27051956
#define IH_NMLEN 32

typedef struct image_header {
	uint32_t ih_magic;
	uint32_t ih_hcrc;
	uint32_t ih_time;
	uint32_t ih_size;
	uint32_t ih_load;
	uint32_t ih_ep;
	uint32_t ih_dcrc;
	uint8 ih_os;
	uint8 ih_arch;
	uint8 ih_type;
	uint8 ih_comp;
	char ih_name[IH_NMLEN];
} image_header_t;


#ifdef __cplusplus
extern "C" {
#endif

void dump_uimage(struct image_header *image);
bool image_multi_getimg(struct image_header *image, uint32_t idx, 
	uint32_t *data, uint32_t *size);

#ifdef __cplusplus
}
#endif


#endif	/* KERNEL_BOOT_PLATFORM_UBOOT_UIMAGE_H */
