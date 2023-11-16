/*
 * Copyright 2005, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef _KERNEL_CONSOLE_H
#define _KERNEL_CONSOLE_H


#include <module.h>
#include <stdio.h>

struct kernel_args;


typedef struct {
	module_info	info;

	status_t (*get_size)(int32_t *_width, int32_t *_height);
	void (*move_cursor)(int32_t x, int32_t y);
	void (*put_glyph)(int32_t x, int32_t y, uint8 glyph, uint8 attr);
	void (*fill_glyph)(int32_t x, int32_t y, int32_t width, int32_t height, uint8 glyph, uint8 attr);
	void (*blit)(int32_t srcx, int32_t srcy, int32_t width, int32_t height, int32_t destx, int32_t desty);
	void (*clear)(uint8 attr);
} console_module_info;


#ifdef __cplusplus
extern "C" {
#endif

int con_init(struct kernel_args *args);
void kprintf(const char *fmt, ...) __PRINTFLIKE(1,2);
void kprintf_xy(int x, int y, const char *fmt, ...) __PRINTFLIKE(3,4);

#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL_CONSOLE_H */
