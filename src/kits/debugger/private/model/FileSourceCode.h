/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef FILE_SOURCE_CODE_H
#define FILE_SOURCE_CODE_H


#include <Locker.h>

#include <Array.h>

#include <SourceCode.h>


class LocatableFile;
class SourceFile;


class FileSourceCode : public SourceCode {
public:
								FileSourceCode(LocatableFile* file,
									SourceFile* sourceFile,
									SourceLanguage* language);
	virtual						~FileSourceCode();

			status_t			Init();
			status_t			AddSourceLocation(
									const SourceLocation& location);

	virtual	bool				Lock();
	virtual	void				Unlock();

	virtual	SourceLanguage*		GetSourceLanguage() const;

	virtual	bool				GetStatementLocationRange(
									const SourceLocation& location,
									SourceLocation& _start,
									SourceLocation& _end) const;

	virtual	LocatableFile*		GetSourceFile() const;

	// LineDataSource
	virtual	int32_t				CountLines() const;
	virtual	const char*			LineAt(int32_t index) const;
	virtual int32_t				LineLengthAt(int32_t index) const;

private:
			int32_t				_FindSourceLocationIndex(
									const SourceLocation& location,
									bool& _foundMatch) const;

private:
			BLocker				fLock;
			LocatableFile*		fFile;
			SourceFile*			fSourceFile;
			SourceLanguage*		fLanguage;
			Array<SourceLocation> fSourceLocations;
};


#endif	// FILE_BASED_SOURCE_CODE_H
