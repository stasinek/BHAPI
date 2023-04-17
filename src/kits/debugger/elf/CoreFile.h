/*
 * Copyright 2016, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef CORE_FILE_H
#define CORE_FILE_H


#include <ElfFile.h>

#include <kits/support/String.h>


struct CoreFileTeamInfo {
								CoreFileTeamInfo();
			void				Init(int32_t id, int32_t uid, int32_t gid,
									const BString& args);

			int32_t				Id() const			{ return fId; }
			const BString&		Arguments() const	{ return fArgs; }

private:
			int32_t				fId;
			int32_t				fUid;
			int32_t				fGid;
			BString				fArgs;
};


struct CoreFileAreaInfo {
								CoreFileAreaInfo(ElfSegment* segment, int32_t id,
									uint64 baseAddress, uint64 size,
									uint64 ramSize, uint32_t locking,
									uint32_t protection, const BString& name);

			uint64				BaseAddress() const	{ return fBaseAddress; }
			uint64				Size() const		{ return fSize; }
			uint64				EndAddress() const
									{ return fBaseAddress + fSize; }
			uint32_t				Locking() const		{ return fLocking; }
			uint32_t				Protection() const	{ return fProtection; }

			ElfSegment*			Segment() const		{ return fSegment; }

private:
			ElfSegment*			fSegment;
			uint64				fBaseAddress;
			uint64				fSize;
			uint64				fRamSize;
			uint32_t				fLocking;
			uint32_t				fProtection;
			int32_t				fId;
			BString				fName;
};


struct CoreFileSymbolsInfo {
								CoreFileSymbolsInfo();
								~CoreFileSymbolsInfo();

			bool				Init(const void* symbolTable,
									uint32_t symbolCount,
									uint32_t symbolTableEntrySize,
									const char* stringTable,
									uint32_t stringTableSize);

			const void*			SymbolTable() const	{ return fSymbolTable; }
			const char*			StringTable() const	{ return fStringTable; }
			uint32_t				SymbolCount() const	{ return fSymbolCount; }
			uint32_t				SymbolTableEntrySize() const
									{ return fSymbolTableEntrySize; }
			uint32_t				StringTableSize() const
									{ return fStringTableSize; }

private:
			void*				fSymbolTable;
			char*				fStringTable;
			uint32_t				fSymbolCount;
			uint32_t				fSymbolTableEntrySize;
			uint32_t				fStringTableSize;
};


struct CoreFileImageInfo {
								CoreFileImageInfo(int32_t id, int32_t type,
									uint64 initRoutine, uint64 termRoutine,
									uint64 textBase, uint64 textSize,
									int64 textDelta,
									uint64 dataBase, uint64 dataSize,
									int32_t deviceId, int64 nodeId,
									uint64 symbolTable, uint64 symbolHash,
									uint64 stringTable,
									CoreFileAreaInfo* textArea,
									CoreFileAreaInfo* dataArea,
									const BString& name);
								~CoreFileImageInfo();

			int32_t				Id() const			{ return fId; }
			int32_t				Type() const		{ return fType; }
			uint64				TextBase() const	{ return fTextBase; }
			uint64				TextSize() const	{ return fTextSize; }
			int64				TextDelta() const	{ return fTextDelta; }
			uint64				DataBase() const	{ return fDataBase; }
			uint64				DataSize() const	{ return fDataSize; }
			uint64				SymbolTable() const	{ return fSymbolTable; }
			uint64				SymbolHash() const	{ return fSymbolHash; }
			uint64				StringTable() const	{ return fStringTable; }
			const BString&		Name() const		{ return fName; }
			CoreFileAreaInfo*	TextArea() const	{ return fTextArea; }
			CoreFileAreaInfo*	DataArea() const	{ return fDataArea; }

			const CoreFileSymbolsInfo* SymbolsInfo() const
									{ return fSymbolsInfo; }
			void				SetSymbolsInfo(
									CoreFileSymbolsInfo* symbolsInfo);

private:
			int32_t				fId;
			int32_t				fType;
			uint64				fInitRoutine;
			uint64				fTermRoutine;
			uint64				fTextBase;
			uint64				fTextSize;
			int64				fTextDelta;
			uint64				fDataBase;
			uint64				fDataSize;
			int32_t				fDeviceId;
			int64				fNodeId;
			uint64				fSymbolTable;
			uint64				fSymbolHash;
			uint64				fStringTable;
			CoreFileAreaInfo*	fTextArea;
			CoreFileAreaInfo*	fDataArea;
			BString				fName;
			CoreFileSymbolsInfo* fSymbolsInfo;
};


struct CoreFileThreadInfo {
								CoreFileThreadInfo(int32_t id, int32_t state,
									int32_t priority, uint64 stackBase,
									uint64 stackEnd, const BString& name);
								~CoreFileThreadInfo();

			bool				SetCpuState(const void* state, size_t size);
			const void*			GetCpuState() const
									{ return fCpuState; }
			size_t				CpuStateSize() const
									{ return fCpuStateSize; }

			int32_t				Id() const		{ return fId; }
			const BString&		Name() const	{ return fName; }

private:
			int32_t				fId;
			int32_t				fState;
			int32_t				fPriority;
			uint64				fStackBase;
			uint64				fStackEnd;
			BString				fName;
			void*				fCpuState;
			size_t				fCpuStateSize;
};


class CoreFile {
public:
								CoreFile();
								~CoreFile();

			status_t			Init(const char* fileName);

			ElfFile&			GetElfFile()
									{ return fElfFile; }

			const CoreFileTeamInfo& GetTeamInfo() const
									{ return fTeamInfo; }

			int32_t				CountAreaInfos() const
									{ return fAreaInfos.CountItems(); }
			const CoreFileAreaInfo* AreaInfoAt(int32_t index) const
									{ return fAreaInfos.ItemAt(index); }
			const CoreFileAreaInfo* AreaInfoForAddress(uint64 address) const
									{ return _FindArea(address); }

			int32_t				CountImageInfos() const
									{ return fImageInfos.CountItems(); }
			const CoreFileImageInfo* ImageInfoAt(int32_t index) const
									{ return fImageInfos.ItemAt(index); }
			const CoreFileImageInfo* ImageInfoForId(int32_t id) const
									{ return _ImageInfoForId(id); }

			int32_t				CountThreadInfos() const
									{ return fThreadInfos.CountItems(); }
			const CoreFileThreadInfo* ThreadInfoAt(int32_t index) const
									{ return fThreadInfos.ItemAt(index); }
			const CoreFileThreadInfo* ThreadInfoForId(int32_t id) const;

			status_t			CreateSymbolLookup(
									const CoreFileImageInfo* imageInfo,
									ElfSymbolLookup*& _lookup);

private:
			typedef BObjectList<CoreFileAreaInfo> AreaInfoList;
			typedef BObjectList<CoreFileImageInfo> ImageInfoList;
			typedef BObjectList<CoreFileThreadInfo> ThreadInfoList;

private:
			template<typename ElfClass>
			status_t			_Init();

			template<typename ElfClass>
			status_t			_ReadNotes();
			template<typename ElfClass>
			status_t			_ReadNotes(ElfSegment* segment);
			template<typename ElfClass>
			status_t			_ReadNote(const char* name, uint32_t type,
									const void* data, uint32_t dataSize);
			template<typename ElfClass>
			status_t			_ReadTeamNote(const void* data,
									uint32_t dataSize);
			template<typename ElfClass>
			status_t			_ReadAreasNote(const void* data,
									uint32_t dataSize);
			template<typename ElfClass>
			status_t			_ReadImagesNote(const void* data,
									uint32_t dataSize);
			template<typename ElfClass>
			status_t			_ReadSymbolsNote(const void* data,
									uint32_t dataSize);
			template<typename ElfClass>
			status_t			_ReadThreadsNote(const void* data,
									uint32_t dataSize);

			CoreFileAreaInfo*	_FindArea(uint64 address) const;
			ElfSegment*			_FindAreaSegment(uint64 address) const;

			CoreFileImageInfo*	_ImageInfoForId(int32_t id) const;

			template<typename Type>
			Type				_ReadValue(const void*& data, uint32_t& dataSize);
			template<typename Entry>
			void				_ReadEntry(const void*& data, uint32_t& dataSize,
									Entry& entry, size_t entrySize);
			void				_Advance(const void*& data, uint32_t& dataSize,
									size_t by);

			template<typename Value>
			Value				Get(const Value& value) const
									{ return fElfFile.Get(value); }

private:
			ElfFile				fElfFile;
			CoreFileTeamInfo	fTeamInfo;
			AreaInfoList		fAreaInfos;
			ImageInfoList		fImageInfos;
			ThreadInfoList		fThreadInfos;
};


#endif	// CORE_FILE_H
