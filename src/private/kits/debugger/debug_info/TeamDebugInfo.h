/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2014-2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef TEAM_DEBUG_INFO_H
#define TEAM_DEBUG_INFO_H


#include <Locker.h>

#include <ObjectList.h>
#include <Referenceable.h>
#include <util/OpenHashTable.h>

#include "GlobalTypeLookup.h"
#include "ImageInfo.h"
#include "TeamFunctionSourceInformation.h"
#include "TeamTypeInformation.h"

class Architecture;
class DebuggerInterface;
class DisassembledCode;
class FileManager;
class FileSourceCode;
class Function;
class FunctionID;
class FunctionInstance;
class ImageDebugInfo;
class ImageDebugInfoLoadingState;
class ImageInfo;
class LocatableFile;
class SourceCode;
class SourceLocation;
class SpecificTeamDebugInfo;


class TeamDebugInfo : public BReferenceable, public GlobalTypeLookup,
	public TeamTypeInformation, public TeamFunctionSourceInformation {
public:
								TeamDebugInfo(
									DebuggerInterface* debuggerInterface,
									Architecture* architecture,
									FileManager* fileManager);
								~TeamDebugInfo();

			status_t			Init();

	virtual	status_t			GetType(GlobalTypeCache* cache,
									const BString& name,
									const TypeLookupConstraints& constraints,
									Type*& _type);
	virtual	bool				HasType(GlobalTypeCache* cache,
									const BString& name,
									const TypeLookupConstraints& constraints);

	virtual status_t			LookupTypeByName(const BString& name,
									const TypeLookupConstraints& constraints,
									Type*& _type);
	virtual	bool				TypeExistsByName(const BString& name,
									const TypeLookupConstraints& constraints);

	virtual	status_t			GetActiveSourceCode(FunctionDebugInfo* info,
									SourceCode*& _code);

			status_t			LoadImageDebugInfo(const ImageInfo& imageInfo,
									LocatableFile* imageFile,
									ImageDebugInfoLoadingState& state,
									ImageDebugInfo*& _imageDebugInfo);

			status_t			LoadSourceCode(LocatableFile* file,
									FileSourceCode*& _sourceCode);
										// returns reference
			void				ClearSourceCode(LocatableFile* file);

			status_t			DisassembleFunction(
									FunctionInstance* functionInstance,
									DisassembledCode*& _sourceCode);
										// returns reference
			FunctionInstance*	MainFunction() const
									{ return fMainFunction; }

			// team is locked
			status_t			AddImageDebugInfo(
									ImageDebugInfo* imageDebugInfo);
			void				RemoveImageDebugInfo(
									ImageDebugInfo* imageDebugInfo);
			ImageDebugInfo*		ImageDebugInfoByName(const char* name) const;

			Function*			FunctionAtSourceLocation(LocatableFile* file,
									const SourceLocation& location) const;
			Function*			FunctionByID(FunctionID* functionID) const;

private:
			struct FunctionHashDefinition;
			struct SourceFileEntry;
			struct SourceFileHashDefinition;

			typedef BObjectList<SpecificTeamDebugInfo> SpecificInfoList;
			typedef BObjectList<ImageDebugInfo> ImageList;
			typedef BOpenHashTable<FunctionHashDefinition> FunctionTable;
			typedef BOpenHashTable<SourceFileHashDefinition> SourceFileTable;

private:
			status_t			_AddFunction(Function* function);
			void				_RemoveFunction(Function* function);

private:
			BLocker				fLock;
			DebuggerInterface*	fDebuggerInterface;
			Architecture*		fArchitecture;
			FileManager*		fFileManager;
			SpecificInfoList	fSpecificInfos;
			ImageList			fImages;
			FunctionTable*		fFunctions;
			SourceFileTable*	fSourceFiles;
			GlobalTypeCache*	fTypeCache;
			FunctionInstance*	fMainFunction;
};


#endif	// TEAM_DEBUG_INFO_H
