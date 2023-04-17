/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_ARCH_X86_PAGING_PAE_PAGING_H
#define KERNEL_ARCH_X86_PAGING_PAE_PAGING_H


#include <OS.h>


#if B_HAIKU_PHYSICAL_BITS == 64


// page directory pointer table entry bits
#define X86_PAE_PDPTE_PRESENT			0x0000000000000001LL
#define X86_PAE_PDPTE_WRITE_THROUGH		0x0000000000000008LL
#define X86_PAE_PDPTE_CACHING_DISABLED	0x0000000000000010LL
#define X86_PAE_PDPTE_ADDRESS_MASK		0x000ffffffffff000LL

// page directory entry bits
#define X86_PAE_PDE_PRESENT				0x0000000000000001LL
#define X86_PAE_PDE_WRITABLE			0x0000000000000002LL
#define X86_PAE_PDE_USER				0x0000000000000004LL
#define X86_PAE_PDE_WRITE_THROUGH		0x0000000000000008LL
#define X86_PAE_PDE_CACHING_DISABLED	0x0000000000000010LL
#define X86_PAE_PDE_ACCESSED			0x0000000000000020LL
#define X86_PAE_PDE_IGNORED1			0x0000000000000040LL
#define X86_PAE_PDE_LARGE_PAGE			0x0000000000000080LL
#define X86_PAE_PDE_IGNORED2			0x0000000000000100LL
#define X86_PAE_PDE_IGNORED3			0x0000000000000200LL
#define X86_PAE_PDE_IGNORED4			0x0000000000000400LL
#define X86_PAE_PDE_IGNORED5			0x0000000000000800LL
#define X86_PAE_PDE_ADDRESS_MASK		0x000ffffffffff000LL
#define X86_PAE_PDE_NOT_EXECUTABLE		0x8000000000000000LL

// page table entry bits
#define X86_PAE_PTE_PRESENT				0x0000000000000001LL
#define X86_PAE_PTE_WRITABLE			0x0000000000000002LL
#define X86_PAE_PTE_USER				0x0000000000000004LL
#define X86_PAE_PTE_WRITE_THROUGH		0x0000000000000008LL
#define X86_PAE_PTE_CACHING_DISABLED	0x0000000000000010LL
#define X86_PAE_PTE_ACCESSED			0x0000000000000020LL
#define X86_PAE_PTE_DIRTY				0x0000000000000040LL
#define X86_PAE_PTE_PAT					0x0000000000000080LL
#define X86_PAE_PTE_GLOBAL				0x0000000000000100LL
#define X86_PAE_PTE_IGNORED1			0x0000000000000200LL
#define X86_PAE_PTE_IGNORED2			0x0000000000000400LL
#define X86_PAE_PTE_IGNORED3			0x0000000000000800LL
#define X86_PAE_PTE_ADDRESS_MASK		0x000ffffffffff000LL
#define X86_PAE_PTE_NOT_EXECUTABLE		0x8000000000000000LL
#define X86_PAE_PTE_PROTECTION_MASK		(X86_PAE_PTE_NOT_EXECUTABLE	\
											|X86_PAE_PTE_WRITABLE	\
											| X86_PAE_PTE_USER)
#define X86_PAE_PTE_MEMORY_TYPE_MASK	(X86_PAE_PTE_WRITE_THROUGH \
											| X86_PAE_PTE_CACHING_DISABLED)


static const uint32_t kPAEPageDirEntryCount = 512;
static const uint32_t kPAEPageTableEntryCount = 512;
static const size_t kPAEPageTableRange = kPAEPageTableEntryCount * B_PAGE_SIZE;
static const size_t kPAEPageDirRange
	= kPAEPageDirEntryCount * kPAEPageTableRange;


typedef uint64 pae_page_directory_pointer_table_entry;
typedef uint64 pae_page_directory_entry;
typedef uint64 pae_page_table_entry;


#endif	// B_HAIKU_PHYSICAL_BITS == 64


#endif	// KERNEL_ARCH_X86_PAGING_PAE_PAGING_H
