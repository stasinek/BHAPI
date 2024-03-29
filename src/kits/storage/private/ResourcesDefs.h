// ResourcesDefs.h

#ifndef _DEF_RESOURCES_H
#define _DEF_RESOURCES_H

#include <Haiku.h>

// x86 resource file constants
const char		kX86ResourceFileMagic[4]		= { 'R', 'S', 0, 0 };
const uint32_t	kX86ResourcesOffset				= 0x00000004;

// PPC resource file constants
const char		kPPCResourceFileMagic[4]		= { 'r', 'e', 's', 'f' };
const uint32_t	kPPCResourcesOffset				= 0x00000028;

// ELF file related constants
const uint32_t	kELFMinResourceAlignment		= 32;

// the unused data pattern
const uint32_t	kUnusedResourceDataPattern[3]	= {
	0xffffffff, 0x000003e9, 0x00000000
};


// the resources header
struct resources_header {
	uint32_t	rh_resources_magic;
	uint32_t	rh_resource_count;
	uint32_t	rh_index_section_offset;
	uint32_t	rh_admin_section_size;
	uint32_t	rh_pad[13];
};

const uint32_t	kResourcesHeaderMagic			= 0x444f1000;
const uint32_t	kResourceIndexSectionOffset		= 0x00000044;
const uint32_t	kResourceIndexSectionAlignment	= 0x00000600;
const uint32_t	kResourcesHeaderSize			= 68;


// the header of the index section
struct resource_index_section_header {
	uint32_t	rish_index_section_offset;
	uint32_t	rish_index_section_size;
	uint32_t	rish_unused_data1;
	uint32_t	rish_unknown_section_offset;
	uint32_t	rish_unknown_section_size;
	uint32_t	rish_unused_data2[25];
	uint32_t	rish_info_table_offset;
	uint32_t	rish_info_table_size;
	uint32_t	rish_unused_data3;
};

const uint32_t	kUnknownResourceSectionSize		= 0x00000168;
const uint32_t	kResourceIndexSectionHeaderSize	= 132;


// an entry of the index table
struct resource_index_entry {
	uint32_t	rie_offset;
	uint32_t	rie_size;
	uint32_t	rie_pad;
};

const uint32_t	kResourceIndexEntrySize			= 12;


// a resource info
struct resource_info {
	int32_t	ri_id;
	int32_t	ri_index;
	uint16	ri_name_size;
	char	ri_name[1];
};

const uint32_t	kMinResourceInfoSize			= 10;


// a resource info block
struct resource_info_block {
	type_code		rib_type;
	resource_info	rib_info[1];
};

const uint32_t	kMinResourceInfoBlockSize		= 4 + kMinResourceInfoSize;


// the structure separating resource info blocks
struct resource_info_separator {
	uint32_t	ris_value1;
	uint32_t	ris_value2;
};

const uint32_t	kResourceInfoSeparatorSize		= 8;


// the end of the info table
struct resource_info_table_end {
	uint32_t	rite_check_sum;
	uint32_t	rite_terminator;
};

const uint32_t	kResourceInfoTableEndSize		= 8;


#endif	// _DEF_RESOURCES_H


