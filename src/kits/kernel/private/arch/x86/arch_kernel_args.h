/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef KERNEL_ARCH_x86_KERNEL_ARGS_H
#define KERNEL_ARCH_x86_KERNEL_ARGS_H

#ifndef KERNEL_BOOT_KERNEL_ARGS_H
#	error This file is included from <boot/kernel_args.h> only
#endif


#include <util/FixedWidthPointer.h>


#define MAX_BOOT_PTABLES 8

#define _PACKED __attribute__((packed))

// kernel args
typedef struct {
	// architecture specific
	uint32_t	system_time_cv_factor;
	uint64	cpu_clock_speed;
	uint32_t	phys_pgdir;
	uint64	vir_pgdir;
	uint32_t	num_pgtables;
	uint32_t	pgtables[MAX_BOOT_PTABLES];
	uint64	virtual_end;
	uint64	page_hole;
	// smp stuff
	uint32_t	apic_time_cv_factor; // apic ticks per second
	uint32_t	apic_phys;
	FixedWidthPointer<void> apic;
	uint32_t	ioapic_phys;
	uint32_t	cpu_apic_id[SMP_MAX_CPUS];
	uint32_t	cpu_apic_version[SMP_MAX_CPUS];
	// hpet stuff
	uint32_t	hpet_phys;
	FixedWidthPointer<void> hpet;
} _PACKED arch_kernel_args;

#endif	/* KERNEL_ARCH_x86_KERNEL_ARGS_H */
