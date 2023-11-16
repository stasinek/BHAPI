/*
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/


#include <arch_mmu.h>
#include <arch_cpu.h>


uint32_t 
page_table_entry::PrimaryHash(uint32_t virtualSegmentID, uint32_t virtualAddress)
{
	return (virtualSegmentID & 0x7ffff) ^ ((virtualAddress >> 12) & 0xffff);
}


uint32_t 
page_table_entry::SecondaryHash(uint32_t virtualSegmentID, uint32_t virtualAddress)
{
	return ~PrimaryHash(virtualSegmentID, virtualAddress);
}


uint32_t 
page_table_entry::SecondaryHash(uint32_t primaryHash)
{
	return ~primaryHash;
}


void 
ppc_get_page_table(page_table_entry_group **_pageTable, size_t *_size)
{
	uint32_t sdr1 = get_sdr1();

	*_pageTable = (page_table_entry_group *)(sdr1 & 0xffff0000);
	*_size = ((sdr1 & 0x1ff) + 1) << 16;
}


void 
ppc_set_page_table(page_table_entry_group *pageTable, size_t size)
{
	set_sdr1(((uint32_t)pageTable & 0xffff0000) | (((size -1) >> 16) & 0x1ff));
}

