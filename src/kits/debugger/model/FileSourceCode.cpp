/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <FileSourceCode.h>

#include <kits/support/String.h>

#include <LocatableFile.h>
#include <SourceFile.h>
#include <SourceLanguage.h>
#include <SourceLocation.h>


FileSourceCode::FileSourceCode(LocatableFile* file, SourceFile* sourceFile,
	SourceLanguage* language)
	:
	fLock("source code"),
	fFile(file),
	fSourceFile(sourceFile),
	fLanguage(language)
{
	fFile->AcquireReference();
	fSourceFile->AcquireReference();
	fLanguage->AcquireReference();
}


FileSourceCode::~FileSourceCode()
{
	fLanguage->ReleaseReference();
	fSourceFile->ReleaseReference();
	fFile->ReleaseReference();
}


status_t FileSourceCode::Init()
{
	return fLock.InitCheck();
}


status_t FileSourceCode::AddSourceLocation(const SourceLocation& location)
{
	// Find the insertion index; don't insert twice.
	bool foundMatch;
	int32_t index = _FindSourceLocationIndex(location, foundMatch);
	if (foundMatch)
		return B_OK;

	return fSourceLocations.Insert(location, index) ? B_OK : B_NO_MEMORY;
}


bool FileSourceCode::Lock()
{
	return fLock.Lock();
}


void FileSourceCode::Unlock()
{
	fLock.Unlock();
}


SourceLanguage*
FileSourceCode::GetSourceLanguage() const
{
	return fLanguage;
}


int32_t FileSourceCode::CountLines() const
{
	return fSourceFile->CountLines();
}


const char*  FileSourceCode::LineAt(int32_t index) const
{
	return fSourceFile->LineAt(index);
}


int32_t FileSourceCode::LineLengthAt(int32_t index) const
{
	return fSourceFile->LineLengthAt(index);
}


bool FileSourceCode::GetStatementLocationRange(const SourceLocation& location,
	SourceLocation& _start, SourceLocation& _end) const
{
	int32_t lineCount = CountLines();
	if (location.Line() >= lineCount)
		return false;

	bool foundMatch;
	int32_t index = _FindSourceLocationIndex(location, foundMatch);

	if (!foundMatch) {
		if (index == 0)
			return false;
		index--;
	}

	_start = fSourceLocations[index];
	_end = index + 1 < lineCount
		? fSourceLocations[index + 1] : SourceLocation(lineCount);
	return true;
}


LocatableFile*
FileSourceCode::GetSourceFile() const
{
	return fFile;
}


int32_t FileSourceCode::_FindSourceLocationIndex(const SourceLocation& location,
	bool& _foundMatch) const
{
	int32_t lower = 0;
	int32_t upper = fSourceLocations.Size();
	while (lower < upper) {
		int32_t mid = (lower + upper) / 2;
		if (location <= fSourceLocations[mid])
			upper = mid;
		else
			lower = mid + 1;
	}

	_foundMatch = lower < fSourceLocations.Size()
		&& location == fSourceLocations[lower];
	return lower;
}
