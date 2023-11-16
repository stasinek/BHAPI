/*
 * Copyright 2002-2006, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef _KERNEL_ARCH_X86_32_DESCRIPTORS_H
#define _KERNEL_ARCH_X86_32_DESCRIPTORS_H


// Segments common for all CPUs.
#define KERNEL_CODE_SEGMENT			1
#define KERNEL_DATA_SEGMENT			2

#define USER_CODE_SEGMENT			3
#define USER_DATA_SEGMENT			4

#define BOOT_GDT_SEGMENT_COUNT		(USER_DATA_SEGMENT + 1)

#define APM_CODE32_SEGMENT			5
#define APM_CODE16_SEGMENT			6
#define APM_DATA_SEGMENT			7

#define BIOS_DATA_SEGMENT			8

// Per-CPU segments.
#define TSS_SEGMENT					9
#define DOUBLE_FAULT_TSS_SEGMENT	10
#define KERNEL_TLS_SEGMENT			11
#define USER_TLS_SEGMENT			12
#define APM_SEGMENT					13

#define GDT_SEGMENT_COUNT			14


#define KERNEL_CODE_SELECTOR	((KERNEL_CODE_SEGMENT << 3) | DPL_KERNEL)
#define KERNEL_DATA_SELECTOR	((KERNEL_DATA_SEGMENT << 3) | DPL_KERNEL)

#define USER_CODE_SELECTOR	((USER_CODE_SEGMENT << 3) | DPL_USER)
#define USER_DATA_SELECTOR	((USER_DATA_SEGMENT << 3) | DPL_USER)

#define KERNEL_TLS_SELECTOR	((KERNEL_TLS_SEGMENT << 3) | DPL_KERNEL)


#ifndef _ASSEMBLER
	// this file can also be included from assembler as well
	// (and is in arch_interrupts.S)

// defines entries in the GDT/LDT

struct segment_descriptor {
	uint16 limit_00_15;				// bit	 0 - 15
	uint16 base_00_15;				//		16 - 31
	uint32_t base_23_16 : 8;			//		 0 -  7
	uint32_t type : 4;				//		 8 - 11
	uint32_t desc_type : 1;			//		12		(0 = system, 1 = code/data)
	uint32_t privilege_level : 2;		//		13 - 14
	uint32_t present : 1;				//		15
	uint32_t limit_19_16 : 4;			//		16 - 19
	uint32_t available : 1;			//		20
	uint32_t zero : 1;				//		21
	uint32_t d_b : 1;					//		22
	uint32_t granularity : 1;			//		23
	uint32_t base_31_24 : 8;			//		24 - 31
};

struct interrupt_descriptor {
	uint32_t a;
	uint32_t b;
};

struct tss {
	uint16 prev_task;
	uint16 unused0;
	uint32_t sp0;
	uint32_t ss0;
	uint32_t sp1;
	uint32_t ss1;
	uint32_t sp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint32_t es, cs, ss, ds, fs, gs;
	uint32_t ldt_seg_selector;
	uint16 unused1;
	uint16 io_map_base;
};

typedef segment_descriptor global_descriptor_table[GDT_SEGMENT_COUNT];
extern global_descriptor_table gGDTs[];


static inline void clear_segment_descriptor(segment_descriptor* desc)
{
	*(long long*)desc = 0;
}


static inline void set_segment_descriptor_base(segment_descriptor* desc, addr_t base)
{
	desc->base_00_15 = (addr_t)base & 0xffff;	// base is 32 bits long
	desc->base_23_16 = ((addr_t)base >> 16) & 0xff;
	desc->base_31_24 = ((addr_t)base >> 24) & 0xff;
}


static inline void set_segment_descriptor(segment_descriptor* desc, addr_t base, uint32_t limit,
	uint8 type, uint8 privilegeLevel)
{
	set_segment_descriptor_base(desc, base);

	// limit is 20 bits long
	if (limit & 0xfff00000) {
		desc->limit_00_15 = ((addr_t)limit >> 12) & 0x0ffff;
		desc->limit_19_16 = ((addr_t)limit >> 28) & 0xf;
		desc->granularity = 1;	// 4 KB granularity
	} else {
		desc->limit_00_15 = (addr_t)limit & 0x0ffff;
		desc->limit_19_16 = ((addr_t)limit >> 16) & 0xf;
		desc->granularity = 0;	// 1 byte granularity
	}

	desc->type = type;
	desc->desc_type = DT_CODE_DATA_SEGMENT;
	desc->privilege_level = privilegeLevel;

	desc->present = 1;
	desc->available = 0;	// system available bit is currently not used
	desc->d_b = 1;			// 32-bit code

	desc->zero = 0;
}


static inline void set_tss_descriptor(segment_descriptor* desc, addr_t base, uint32_t limit)
{
	// the TSS descriptor has a special layout different from the standard descriptor
	set_segment_descriptor_base(desc, base);

	desc->limit_00_15 = (addr_t)limit & 0x0ffff;
	desc->limit_19_16 = 0;

	desc->type = DT_TSS;
	desc->desc_type = DT_SYSTEM_SEGMENT;
	desc->privilege_level = DPL_KERNEL;

	desc->present = 1;
	desc->granularity = 0;	// 1 Byte granularity
	desc->available = 0;	// system available bit is currently not used
	desc->d_b = 0;

	desc->zero = 0;
}


static inline segment_descriptor*
get_gdt(int32_t cpu)
{
	return gGDTs[cpu];
}


#endif	/* _ASSEMBLER */

#endif	/* _KERNEL_ARCH_X86_32_DESCRIPTORS_H */
