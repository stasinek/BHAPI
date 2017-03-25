/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_X86_DEBUG_H
#define _KERNEL_ARCH_X86_DEBUG_H


#include <Haiku.h>


struct arch_debug_registers {
	addr_t	bp;
};


#endif	// _KERNEL_ARCH_X86_DEBUG_H
