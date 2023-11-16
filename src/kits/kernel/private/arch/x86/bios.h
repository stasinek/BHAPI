/*
 * Copyright 2004, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef ARCH_x86_BIOS_H
#define ARCH_x86_BIOS_H


#include <Haiku.h>


#ifndef __x86_64__

#define BIOS32_PCI_SERVICE	'ICP$'

struct bios32_service {
	uint32_t	base;
	uint32_t	size;
	uint32_t	offset;
};


#ifdef __cplusplus
extern "C" {
#endif

status_t get_bios32_service(uint32_t identifier, struct bios32_service *service);
status_t bios_init(void);

#ifdef __cplusplus
}
#endif

#endif	/* __x86_64__ */

#endif	/* ARCH_x86_BIOS_H */
