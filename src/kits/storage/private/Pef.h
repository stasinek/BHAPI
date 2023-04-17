// Pef.h

#ifndef _PEF_H
#define _PEF_H

#include <Haiku.h>

typedef char	PefOSType[4];

// container header
struct PEFContainerHeader {
	PefOSType	tag1;
	PefOSType	tag2;
	PefOSType	architecture;
	uint32_t		formatVersion;
	uint32_t		dateTimeStamp;
	uint32_t		oldDefVersion;
	uint32_t		oldImpVersion;
	uint32_t		currentVersion;
	uint16		sectionCount;
	uint16		instSectionCount;
	uint32_t		reservedA;
};

const char	kPEFFileMagic1[4]		= { 'J', 'o', 'y', '!' };
const char	kPEFFileMagic2[4]		= { 'p', 'e', 'f', 'f' };
const char	kPEFArchitecturePPC[4]	= { 'p', 'w', 'p', 'c' };
const char	kPEFContainerHeaderSize	= 40;

// section header
struct PEFSectionHeader {
	int32_t	nameOffset;
	uint32_t	defaultAddress;
	uint32_t	totalSize;
	uint32_t	unpackedSize;
	uint32_t	packedSize;
	uint32_t	containerOffset;
	uint8	sectionKind;
	uint8	shareKind;
	uint8	alignment;
	uint8	reservedA;
};

const uint32_t kPEFSectionHeaderSize		= 28;

#endif	// _PEF_H


