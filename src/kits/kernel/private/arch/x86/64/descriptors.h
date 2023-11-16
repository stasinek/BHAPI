/*
 * Copyright 2012, Alex Smith, alex@alex-smith.me.uk.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_X86_64_DESCRIPTORS_H
#define _KERNEL_ARCH_X86_64_DESCRIPTORS_H


// Segment definitions.
// Note that the ordering of these is important to SYSCALL/SYSRET.
#define KERNEL_CODE_SEGMENT		1
#define KERNEL_DATA_SEGMENT		2
#define USER_DATA_SEGMENT		3
#define USER_CODE_SEGMENT		4

#define BOOT_GDT_SEGMENT_COUNT	(USER_CODE_SEGMENT + 1)

#define KERNEL_CODE_SELECTOR	((KERNEL_CODE_SEGMENT << 3) | DPL_KERNEL)
#define KERNEL_DATA_SELECTOR	((KERNEL_DATA_SEGMENT << 3) | DPL_KERNEL)

#define USER_CODE_SELECTOR	((USER_CODE_SEGMENT << 3) | DPL_USER)
#define USER_DATA_SELECTOR	((USER_DATA_SEGMENT << 3) | DPL_USER)


#ifndef _ASSEMBLER


// Structure of a segment descriptor.
struct segment_descriptor {
	uint32_t limit0 : 16;
	uint32_t base0 : 24;
	uint32_t type : 4;
	uint32_t desc_type : 1;
	uint32_t dpl : 2;
	uint32_t present : 1;
	uint32_t limit1 : 4;
	uint32_t available : 1;
	uint32_t long_mode : 1;
	uint32_t d_b : 1;
	uint32_t granularity : 1;
	uint32_t base1 : 8;
} _PACKED;

struct tss {
	uint32_t _reserved1;
	uint64 sp0;
	uint64 sp1;
	uint64 sp2;
	uint64 _reserved2;
	uint64 ist1;
	uint64 ist2;
	uint64 ist3;
	uint64 ist4;
	uint64 ist5;
	uint64 ist6;
	uint64 ist7;
	uint64 _reserved3;
	uint16 _reserved4;
	uint16 io_map_base;
} _PACKED;


static inline void clear_segment_descriptor(segment_descriptor* desc)
{
	*(uint64*)desc = 0;
}


static inline void set_segment_descriptor(segment_descriptor* desc, uint8 type, uint8 dpl)
{
	clear_segment_descriptor(desc);

	// In 64-bit mode the CPU ignores the base/limit of code/data segments,
	// it always treats base as 0 and does no limit checks.
	desc->base0 = 0;
	desc->base1 = 0;
	desc->limit0 = 0xffff;
	desc->limit1 = 0xf;
	desc->granularity = 1;

	desc->type = type;
	desc->desc_type = DT_CODE_DATA_SEGMENT;
	desc->dpl = dpl;
	desc->present = 1;

	desc->long_mode = (type & DT_CODE_EXECUTE_ONLY) ? 1 : 0;
		// Must be set to 1 for code segments only.
}


#endif	/* _ASSEMBLER */

#endif	/* _KERNEL_ARCH_X86_64_DESCRIPTORS_H */
