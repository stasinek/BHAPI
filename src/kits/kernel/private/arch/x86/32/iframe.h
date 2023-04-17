/*
 * Copyright 2002-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef _KERNEL_ARCH_X86_32_IFRAME_H
#define _KERNEL_ARCH_X86_32_IFRAME_H


struct iframe {
	uint32_t type;	// iframe type
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t di;
	uint32_t si;
	uint32_t bp;
	uint32_t sp;
	uint32_t bx;
	uint32_t dx;
	uint32_t cx;
	uint32_t ax;
	uint32_t orig_eax;
	uint32_t orig_edx;
	uint32_t vector;
	uint32_t error_code;
	uint32_t ip;
	uint32_t cs;
	uint32_t flags;

	// user_sp and user_ss are only present when the iframe is a userland
	// iframe (IFRAME_IS_USER()). A kernel iframe is shorter.
	uint32_t user_sp;
	uint32_t user_ss;
};

#define IFRAME_IS_USER(f)	((f)->cs == USER_CODE_SELECTOR \
								|| ((f)->flags & 0x20000) != 0)


#endif	/* _KERNEL_ARCH_X86_32_IFRAME_H */
