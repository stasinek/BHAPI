/*
 * Copyright 2009 Haiku Inc.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_MIPSEL_CPU_H
#define _KERNEL_ARCH_MIPSEL_CPU_H


#warning IMPLEMENT arch_cpu.h


#include <arch/mipsel/arch_thread_types.h>
#include <kernel.h>


struct iframe {
#warning struct iframe
	uint32_t r31;
	uint32_t r30;
	uint32_t r29;
	uint32_t r28;
	uint32_t r27;
	uint32_t r26;
	uint32_t r25;
	uint32_t r24;
	uint32_t r23;
	uint32_t r22;
	uint32_t r21;
	uint32_t r20;
	uint32_t r19;
	uint32_t r18;
	uint32_t r17;
	uint32_t r16;
	uint32_t r15;
	uint32_t r14;
	uint32_t r13;
	uint32_t r12;
	uint32_t r11;
	uint32_t r10;
	uint32_t r9;
	uint32_t r8;
	uint32_t r7;
	uint32_t r6;
	uint32_t r5;
	uint32_t r4;
	uint32_t r3;
	uint32_t r2;
	uint32_t r1;
	uint32_t r0;
};

typedef struct arch_cpu_info {
	int null;
} arch_cpu_info;


#ifdef __cplusplus
extern "C" {
#endif


extern long long get_time_base(void);

void __mipsel_setup_system_time(vint32_t *cvFactor);
	// defined in libroot: os/arch/system_time.c

int64 __mipsel_get_time_base(void);
	// defined in libroot: os/arch/system_time_asm.S

extern void mipsel_context_switch(void **_oldStackPointer,
	void *newStackPointer);

extern bool mipsel_set_fault_handler(addr_t *handlerLocation, addr_t handler)
	__attribute__((noinline));

#ifdef __cplusplus
}
#endif


/* TODO */
enum mipsel_processor_version {
	FAKE1		= 0x0001,
	FAKE2		= 0x0002,
	FAKE3		= 0x0003,
};


#endif	/* _KERNEL_ARCH_MIPSEL_CPU_H */

