/*
 * Copyright 2009-2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef ELF_FILE_H
#define ELF_FILE_H

#include <sys/types.h>

#include <ByteOrder.h>
#include <Haiku.h>
#include <ObjectList.h>

#include <elf_private.h>
#include <util/DoublyLinkedList.h>

#include <Types.h>


class ElfSymbolLookup;
class ElfSymbolLookupSource;


class ElfSection {
public:
								ElfSection(const char* name, uint32_t type,
									int fd, uint64 offset, uint64 size,
									target_addr_t loadAddress, uint32_t flags,
									uint32_t linkIndex);
								~ElfSection();

			const char*			Name() const	{ return fName; }
			uint32_t				Type() const	{ return fType; }
			uint64				Offset() const	{ return fOffset; }
			uint64				Size() const	{ return fSize; }
			const void*			Data() const	{ return fData; }
			target_addr_t		LoadAddress() const
									{ return fLoadAddress; }
			bool				IsWritable() const
									{ return (fFlags & SHF_WRITE) != 0; }

			status_t			Load();
			void				Unload();
			bool				IsLoaded() const	{ return fLoadCount > 0; }
			uint32_t				LinkIndex() const	{ return fLinkIndex; }

private:
			const char*			fName;
			uint32_t				fType;
			int					fFD;
			uint64				fOffset;
			uint64				fSize;
			void*				fData;
			target_addr_t		fLoadAddress;
			uint32_t				fFlags;
			int32_t				fLoadCount;
			uint32_t				fLinkIndex;
};


class ElfSegment {
public:
								ElfSegment(uint32_t type, uint64 fileOffset,
									uint64 fileSize, target_addr_t loadAddress,
									target_size_t loadSize, uint32_t flags);
								~ElfSegment();

			uint32_t				Type()				{ return fType; }
			uint64				FileOffset() const	{ return fFileOffset; }
			uint64				FileSize() const	{ return fFileSize; }
			target_addr_t		LoadAddress() const	{ return fLoadAddress; }
			target_size_t		LoadSize() const	{ return fLoadSize; }
			uint32_t				Flags() const		{ return fFlags; }
			bool				IsWritable() const
									{ return (fFlags & PF_WRITE) != 0; }

private:
			uint64				fFileOffset;
			uint64				fFileSize;
			target_addr_t		fLoadAddress;
			target_size_t		fLoadSize;
			uint32_t				fType;
			uint32_t				fFlags;
};


struct ElfClass32 {
	typedef uint32_t					Address;
	typedef uint32_t					Size;
	typedef Elf32_Ehdr				Ehdr;
	typedef Elf32_Phdr				Phdr;
	typedef Elf32_Shdr				Shdr;
	typedef Elf32_Sym				Sym;
	typedef Elf32_Nhdr				Nhdr;
	typedef Elf32_Note_Team			NoteTeam;
	typedef Elf32_Note_Area_Entry	NoteAreaEntry;
	typedef Elf32_Note_Image_Entry	NoteImageEntry;
	typedef Elf32_Note_Thread_Entry	NoteThreadEntry;
};


struct ElfClass64 {
	typedef uint64					Address;
	typedef uint64					Size;
	typedef Elf64_Ehdr				Ehdr;
	typedef Elf64_Phdr				Phdr;
	typedef Elf64_Shdr				Shdr;
	typedef Elf64_Sym				Sym;
	typedef Elf64_Nhdr				Nhdr;
	typedef Elf64_Note_Team			NoteTeam;
	typedef Elf64_Note_Area_Entry	NoteAreaEntry;
	typedef Elf64_Note_Image_Entry	NoteImageEntry;
	typedef Elf64_Note_Thread_Entry	NoteThreadEntry;
};


class ElfFile {
public:
								ElfFile();
								~ElfFile();

			status_t			Init(const char* fileName);

			int					FD() const	{ return fFD; }

			bool				Is64Bit() const	{ return f64Bit; }
			bool				IsByteOrderSwapped() const
									{ return fSwappedByteOrder; }
			uint16				Type() const	{ return fType; }
			uint16				Machine() const	{ return fMachine; }

			int32_t				CountSection() const
									{ return fSections.CountItems(); }
			ElfSection*			SectionAt(int32_t index) const
									{ return fSections.ItemAt(index); }
			ElfSection*			GetSection(const char* name);
			void				PutSection(ElfSection* section);
			ElfSection*			FindSection(const char* name) const;
			ElfSection*			FindSection(uint32_t type) const;

			int32_t				CountSegments() const
									{ return fSegments.CountItems(); }
			ElfSegment*			SegmentAt(int32_t index) const
									{ return fSegments.ItemAt(index); }

			ElfSegment*			TextSegment() const;
			ElfSegment*			DataSegment() const;

			ElfSymbolLookupSource* CreateSymbolLookupSource(uint64 fileOffset,
									uint64 fileLength,
									uint64 memoryAddress) const;
			status_t			CreateSymbolLookup(uint64 textDelta,
									ElfSymbolLookup*& _lookup) const;

			template<typename Value>
			Value				Get(const Value& value) const
									{ return StaticGet(value,
										fSwappedByteOrder); }

	template<typename Value>
	static	Value				StaticGet(const Value& value,
									bool swappedByteOrder);

private:
			struct SymbolLookupSource;

			typedef BObjectList<ElfSection> SectionList;
			typedef BObjectList<ElfSegment> SegmentList;

private:
			template<typename ElfClass>
			status_t			_LoadFile(const char* fileName);

			bool				_FindSymbolSections(ElfSection*& _symbolSection,
									ElfSection*& _stringSection,
									uint32_t type) const;

			bool				_CheckRange(uint64 offset, uint64 size) const;

			template<typename ElfClass>
			bool				_CheckElfHeader(
									typename ElfClass::Ehdr& elfHeader);

	static	uint8				_Swap(const uint8& value)
									{ return value; }
	static	uint16				_Swap(const uint16& value)
									{ return (uint16)B_SWAP_INT16(value); }
	static	int32_t				_Swap(const int32_t& value)
									{ return B_SWAP_INT32(value); }
	static	uint32_t				_Swap(const uint32_t& value)
									{ return (uint32_t)B_SWAP_INT32(value); }
	static	int64				_Swap(const int64& value)
									{ return B_SWAP_INT64(value); }
	static	uint64				_Swap(const uint64& value)
									{ return (uint64)B_SWAP_INT64(value); }

private:
			uint64				fFileSize;
			int					fFD;
			uint16				fType;
			uint16				fMachine;
			bool				f64Bit;
			bool				fSwappedByteOrder;
			SectionList			fSections;
			SegmentList			fSegments;
};


template<typename Value>
/*static*/ inline Value
ElfFile::StaticGet(const Value& value, bool swappedByteOrder)
{
	if (!swappedByteOrder)
		return value;
	return _Swap(value);
}


#endif	// ELF_FILE_H
