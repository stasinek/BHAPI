/*
 * Copyright 2005-2009, Haiku Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ARCH_M68K_DEBUGGER_H
#define _ARCH_M68K_DEBUGGER_H

struct m68k_debug_cpu_state {
	uint32_t	d0;
	uint32_t	d1;
	uint32_t	d2;
	uint32_t	d3;
	uint32_t	d4;
	uint32_t	d5;
	uint32_t	d6;
	uint32_t	d7;
	uint32_t	a0;
	uint32_t	a1;
	uint32_t	a2;
	uint32_t	a3;
	uint32_t	a4;
	uint32_t	a5;
	uint32_t	a6;
	uint32_t	a7;
	uint32_t	pc;
	uint16	sr;
//#warning M68K: missing members!
	uint32_t	dummy;
} __attribute__((aligned(8)));

#endif	// _ARCH_M68K_DEBUGGER_H
