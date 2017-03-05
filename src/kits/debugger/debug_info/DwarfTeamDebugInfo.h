/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2014-2016, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef DWARF_TEAM_DEBUG_INFO_H
#define DWARF_TEAM_DEBUG_INFO_H

#include "SpecificTeamDebugInfo.h"


class Architecture;
class DebuggerInterface;
class DwarfManager;
class FileManager;
class ImageInfo;
class GlobalTypeCache;
class GlobalTypeLookup;
class TeamFunctionSourceInformation;
class TeamMemory;


class DwarfTeamDebugInfo : public SpecificTeamDebugInfo {
public:
								DwarfTeamDebugInfo(Architecture* architecture,
									DebuggerInterface* interface,
									FileManager* fileManager,
									GlobalTypeLookup* typeLookup,
									TeamFunctionSourceInformation* sourceInfo,
									GlobalTypeCache* typeCache);
	virtual						~DwarfTeamDebugInfo();

			status_t			Init();

	virtual	status_t			CreateImageDebugInfo(const ImageInfo& imageInfo,
									LocatableFile* imageFile,
									ImageDebugInfoLoadingState& _state,
									SpecificImageDebugInfo*& _imageDebugInfo);

private:
			Architecture*		fArchitecture;
			DebuggerInterface*	fDebuggerInterface;
			FileManager*		fFileManager;
			DwarfManager*		fManager;
			GlobalTypeLookup*	fTypeLookup;
			TeamFunctionSourceInformation* fSourceInfo;
			GlobalTypeCache*	fTypeCache;
};


#endif	// DWARF_TEAM_DEBUG_INFO_H
