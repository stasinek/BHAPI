/*
 * Copyright 2016, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef ELF_SYMBOL_LOOKUP_H
#define ELF_SYMBOL_LOOKUP_H


#include <Referenceable.h>

#include <ElfFile.h>
#include <SymbolInfo.h>


class ElfSymbolLookupSource : public BReferenceable {
public:
	virtual	ssize_t				Read(uint64 address, void* buffer,
									size_t size) = 0;
};


class ElfSymbolLookup {
public:
	static	const uint32_t		kGetSymbolCountFromHash = ~(uint32_t)0;

public:
	virtual						~ElfSymbolLookup();

	static	status_t			Create(ElfSymbolLookupSource* source,
									uint64 symbolTable, uint64 symbolHash,
									uint64 stringTable, uint32_t symbolCount,
									uint32_t symbolTableEntrySize,
									uint64 textDelta, bool is64Bit,
									bool swappedByteOrder, bool cacheSource,
									ElfSymbolLookup*& _lookup);

	virtual	status_t			Init(bool cacheSource) = 0;
	virtual	status_t			NextSymbolInfo(uint32_t& index,
									SymbolInfo& _info) = 0;
	virtual	status_t			GetSymbolInfo(const char* name,
									uint32_t symbolType, SymbolInfo& _info) = 0;
};


#endif	// ELF_SYMBOL_LOOKUP_H
