/*
 * Copyright 2005-2009, Haiku Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ARCH_X86_DEBUGGER_H
#define _ARCH_X86_DEBUGGER_H


typedef struct x86_fp_register {
	uint8		value[10];
	uint8		reserved[6];
} x86_fp_register;


typedef struct x86_xmm_register {
	uint8		value[16];
} x86_xmm_register;


typedef struct x86_extended_registers {
	uint16				control;
	uint16				status;
	uint8				tag;
	uint8				reserved1;
	uint16				opcode;
	uint32_t				instruction_pointer;
	uint16				cs;
	uint16				reserved2;
	uint32_t				data_pointer;
	uint16				ds;
	uint16				reserved3;
	uint32_t				mxcsr;
	uint32_t				mxcsr_mask;
	union {
		x86_fp_register	fp_registers[8];	// st0-st7
		x86_fp_register	mmx_registers[8];	// mm0-mm7
	};
	x86_xmm_register	xmm_registers[8];	// xmm0-xmm7
	uint8				reserved4[224];		// 288 - 512
} x86_extended_registers;


struct x86_debug_cpu_state {
	x86_extended_registers	extended_registers;

	uint32_t	gs;
	uint32_t	fs;
	uint32_t	es;
	uint32_t	ds;
	uint32_t	edi;
	uint32_t	esi;
	uint32_t	ebp;
	uint32_t	esp;
	uint32_t	ebx;
	uint32_t	edx;
	uint32_t	ecx;
	uint32_t	eax;
	uint32_t	vector;
	uint32_t	error_code;
	uint32_t	eip;
	uint32_t	cs;
	uint32_t	eflags;
	uint32_t	user_esp;
	uint32_t	user_ss;
} __attribute__((aligned(16)));


#endif	// _ARCH_X86_DEBUGGER_H
