/*
 * Copyright 2013, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold, ingo_weinhold@gmx.de
 */
#ifndef _VIRTUAL_DIRECTORY_ENTRY_LIST_H
#define _VIRTUAL_DIRECTORY_ENTRY_LIST_H


#include <kits/support/StringList.h>

#include <MergedDirectory.h>

#include <EntryIterator.h>


namespace BPrivate {

class Model;

class VirtualDirectoryEntryList : public EntryListBase {
public:
								VirtualDirectoryEntryList(Model* model);
								VirtualDirectoryEntryList(
									const node_ref& definitionFileRef,
									const BStringList& directoryPaths);
	virtual						~VirtualDirectoryEntryList();

	virtual	status_t			InitCheck() const;

	virtual	status_t			GetNextEntry(BEntry* entry,
									bool traverse = false);
	virtual	status_t			GetNextRef(entry_ref* ref);
	virtual	int32_t				GetNextDirents(struct dirent* buffer,
									size_t length, int32_t count = INT_MAX);

	virtual	status_t			Rewind();
	virtual	int32_t				CountEntries();

private:

			status_t			_InitMergedDirectory(
									const BStringList& directoryPaths);
private:
			node_ref			fDefinitionFileRef;
			BMergedDirectory	fMergedDirectory;
};

} // namespace BPrivate


#endif	// _VIRTUAL_DIRECTORY_ENTRY_LIST_H
