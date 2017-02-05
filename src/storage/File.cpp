/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: File.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32
#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif // _WIN32

#include "./../support/String.h>

#ifdef _WIN32
extern "C" char* etk_win32_convert_utf8_to_active(const char *str, eint32 length);
#endif // _WIN32

#include "Path.h"
#include "File.h"

extern e_status_t etk_path_expound(EString &path, const char *dir, const char *leaf, bool *normalize);

#ifndef _WIN32
inline int etk_file_openmode_to_flags(euint32 open_mode)
{
	int flags;

	if(open_mode & E_READ_WRITE) flags = O_RDWR;
	else if(open_mode & E_WRITE_ONLY) flags = O_WRONLY;
	else flags = O_RDONLY;

	if(open_mode & E_CREATE_FILE)
	{
		flags |= O_CREAT;
		if(open_mode & E_FAIL_IF_EXISTS) flags |= O_EXCL;
	}

	if(open_mode & E_ERASE_FILE) flags |= O_TRUNC;
	if(open_mode & E_OPEN_AT_END) flags |= O_APPEND;

#ifdef O_LARGEFILE
	flags |= O_LARGEFILE;
#endif

	return flags;
}
#else
inline DWORD etk_file_openmode_to_creation_disposition(euint32 open_mode)
{
	if(open_mode & E_CREATE_FILE)
	{
		if(open_mode & E_FAIL_IF_EXISTS) return CREATE_NEW;
		if(open_mode & E_ERASE_FILE) return CREATE_ALWAYS;
		return OPEN_ALWAYS;
	}

	if(open_mode & E_ERASE_FILE) return TRUNCATE_EXISTING;
	return OPEN_EXISTING;
}
#endif


#ifndef _WIN32
inline mode_t etk_file_access_mode_to_mode_t(euint32 access_mode)
{
	mode_t mode = 0;

	if(access_mode & E_USER_READ) mode |= S_IRUSR;
	if(access_mode & E_USER_WRITE) mode |= S_IWUSR;
	if(access_mode & E_USER_EXEC) mode |= S_IXUSR;

	if(access_mode & E_GROUP_READ) mode |= S_IRGRP;
	if(access_mode & E_GROUP_WRITE) mode |= S_IWGRP;
	if(access_mode & E_GROUP_EXEC) mode |= S_IXGRP;

	if(access_mode & E_OTHERS_READ) mode |= S_IROTH;
	if(access_mode & E_OTHERS_WRITE) mode |= S_IWOTH;
	if(access_mode & E_OTHERS_EXEC) mode |= S_IXOTH;

	return mode;
}
#endif


EFile::EFile()
	: fFD(NULL), fMode(0)
{
}


EFile::EFile(const char *path, euint32 open_mode, euint32 access_mode)
	: fFD(NULL), fMode(0)
{
	SetTo(path, open_mode, access_mode);
}


EFile::EFile(const EEntry *entry, euint32 open_mode, euint32 access_mode)
	: fFD(NULL), fMode(0)
{
	SetTo(entry, open_mode, access_mode);
}


EFile::EFile(const EDirectory *dir, const char *leaf, euint32 open_mode, euint32 access_mode)
	: fFD(NULL), fMode(0)
{
	SetTo(dir, leaf, open_mode, access_mode);
}


EFile::EFile(const EFile &from)
	: fFD(NULL), fMode(0)
{
	operator=(from);
}


EFile::~EFile()
{
	if(fFD != NULL)
	{
#ifndef _WIN32
		close(*((int*)fFD));
		free(fFD);
#else
		CloseHandle((HANDLE)fFD);
#endif
	}
}


e_status_t
EFile::InitCheck() const
{
	return(fFD == NULL ? E_NO_INIT : E_OK);
}


e_status_t
EFile::SetTo(const char *path, euint32 open_mode, euint32 access_mode)
{
	if(path == NULL || *path == 0) return E_BAD_VALUE;

	EString strPath;
	etk_path_expound(strPath, path, NULL, NULL);
	if(strPath.Length() <= 0) return E_BAD_VALUE;

#ifndef _WIN32
	int newFD = open(strPath.String(), etk_file_openmode_to_flags(open_mode), etk_file_access_mode_to_mode_t(access_mode));
	if(newFD == -1) return E_FILE_ERROR;
	if(fFD != NULL)
	{
		close(*((int*)fFD));
	}
	else if((fFD = malloc(sizeof(int))) == NULL)
	{
		close(newFD);
		return E_NO_MEMORY;
	}
	*((int*)fFD) = newFD;
#else
	strPath.ReplaceAll("/", "\\");
	char *active_str = etk_win32_convert_utf8_to_active(strPath.String(), -1);
	if(active_str != NULL)
	{
		strPath = active_str;
		free(active_str);
	}
	HANDLE newFD = CreateFile(strPath.String(),
				  (open_mode & E_READ_WRITE) ? (GENERIC_WRITE | GENERIC_READ) :
				  	(open_mode & E_WRITE_ONLY ? GENERIC_WRITE : GENERIC_READ),
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL,
				  etk_file_openmode_to_creation_disposition(open_mode),
				  FILE_ATTRIBUTE_NORMAL,
				  NULL);
	if(newFD == INVALID_HANDLE_VALUE) return E_FILE_ERROR;
	if(fFD != NULL) CloseHandle((HANDLE)fFD);
	fFD = (void*)newFD;
	if(open_mode & E_OPEN_AT_END) SetFilePointer(newFD, 0, NULL, FILE_END);
#endif

	fMode = open_mode;

	return E_OK;
}


e_status_t
EFile::SetTo(const EEntry *entry, euint32 open_mode, euint32 access_mode)
{
	if(entry == NULL) return E_BAD_VALUE;

	EPath path;
	if(entry->GetPath(&path) != E_OK) return E_BAD_VALUE;

	return SetTo(path.Path(), open_mode, access_mode);
}


e_status_t
EFile::SetTo(const EDirectory *dir, const char *leaf, euint32 open_mode, euint32 access_mode)
{
	if(dir == NULL || leaf == NULL) return E_BAD_VALUE;

	EEntry entry;
	if(dir->GetEntry(&entry) != E_OK) return E_BAD_VALUE;

	EPath path;
	if(entry.GetPath(&path) != E_OK) return E_BAD_VALUE;

	if(path.Append(leaf, false) != E_OK) return E_BAD_VALUE;

	return SetTo(path.Path(), open_mode, access_mode);
}


void
EFile::Unset()
{
	if(fFD != NULL)
	{
#ifndef _WIN32
		close(*((int*)fFD));
		free(fFD);
#else
		CloseHandle((HANDLE)fFD);
#endif
	}

	fFD = NULL;
}


bool
EFile::IsReadable() const
{
	return(fFD == NULL ? false : true);
}


bool
EFile::IsWritable() const
{
	if(fFD == NULL) return false;
	return((fMode & (E_WRITE_ONLY | E_READ_WRITE)) ? true : false);
}


ssize_t
EFile::Read(void *buffer, size_t size)
{
	if(!IsReadable() || buffer == NULL) return -1;
#ifndef _WIN32
	return read(*((int*)fFD), buffer, size);
#else
	DWORD nRead = (DWORD)size;
	if(ReadFile((HANDLE)fFD, buffer, nRead, &nRead, NULL) == 0) return -1;
	return((ssize_t)nRead);
#endif
}


ssize_t
EFile::ReadAt(eint64 pos, void *buffer, size_t size)
{
	if(!IsReadable() || buffer == NULL) return -1;
	eint64 savePosition = Position();
	if(Seek(pos, E_SEEK_SET) < E_INT64_CONSTANT(0)) return -1;
	ssize_t retVal = Read(buffer, size);
	Seek(savePosition, E_SEEK_SET);
	return retVal;
}


ssize_t
EFile::Write(const void *buffer, size_t size)
{
	if(!IsWritable() || buffer == NULL) return -1;
#ifndef _WIN32
	return write(*((int*)fFD), buffer, size);
#else
	DWORD nWrote = (DWORD)size;
	if(WriteFile((HANDLE)fFD, buffer, nWrote, &nWrote, NULL) == 0) return -1;
	return((ssize_t)nWrote);
#endif
}


ssize_t
EFile::WriteAt(eint64 pos, const void *buffer, size_t size)
{
	if(!IsWritable() || buffer == NULL) return -1;
	eint64 savePosition = Position();
	if(Seek(pos, E_SEEK_SET) < E_INT64_CONSTANT(0)) return -1;
	ssize_t retVal = Write(buffer, size);
	Seek(savePosition, E_SEEK_SET);
	return retVal;
}


eint64
EFile::Seek(eint64 position, euint32 seek_mode)
{
	if(fFD == NULL || (seek_mode == E_SEEK_SET && position < E_INT64_CONSTANT(0))) return E_INT64_CONSTANT(-1);

#ifndef _WIN32
	int whence = SEEK_SET;
	if(seek_mode == E_SEEK_CUR) whence = SEEK_CUR;
	else if(seek_mode == E_SEEK_END) whence = SEEK_END;

	off_t pos = (off_t)-1;
	if(sizeof(off_t) > 4 || pos < (eint64)E_MAXUINT32) pos = lseek(*((int*)fFD), (off_t)position, whence);
	if(pos == (off_t)-1) return E_INT64_CONSTANT(-1);
	return (eint64)pos;
#else
	DWORD whence = FILE_BEGIN;
	if(seek_mode == E_SEEK_CUR) whence = FILE_CURRENT;
	else if(seek_mode == E_SEEK_END) whence = FILE_END;

	LARGE_INTEGER li;
	li.QuadPart = position;
	li.LowPart = SetFilePointer((HANDLE)fFD, li.LowPart, &li.HighPart, whence);
	if(li.LowPart == (DWORD)-1/*INVALID_SET_FILE_POINTER*/) return E_INT64_CONSTANT(-1);
	return li.QuadPart;
#endif
}


eint64
EFile::Position() const
{
	if(fFD == NULL) return E_INT64_CONSTANT(-1);

#ifndef _WIN32
	off_t pos = lseek(*((int*)fFD), 0, SEEK_CUR);
	if(pos == (off_t)-1) return E_INT64_CONSTANT(-1);
	return (eint64)pos;
#else
	LARGE_INTEGER li;
	li.HighPart = 0;
	li.LowPart = SetFilePointer((HANDLE)fFD, 0, &li.HighPart, FILE_CURRENT);
	if(li.LowPart == (DWORD)-1/*INVALID_SET_FILE_POINTER*/) return E_INT64_CONSTANT(-1);
	return li.QuadPart;
#endif
}


e_status_t
EFile::SetSize(eint64 size)
{
	if(fFD == NULL || size < E_INT64_CONSTANT(0) || size == E_MAXINT64) return E_BAD_VALUE;
#ifndef _WIN32
	int status = -1;
	if(sizeof(off_t) > 4 || size < (eint64)E_MAXUINT32) status = ftruncate(*((int*)fFD), (off_t)size);
	if(status != 0) return E_FILE_ERROR;

	lseek(*((int*)fFD), 0, SEEK_SET);
	return E_OK;
#else
	eint64 oldPos = Position();
	if(Seek(size, E_SEEK_SET) < E_INT64_CONSTANT(0) || SetEndOfFile((HANDLE)fFD) == 0)
	{
		Seek(oldPos, E_SEEK_SET);
		return E_FILE_ERROR;
	}

	Seek(0, E_SEEK_SET);
	return E_OK;
#endif
}


EFile&
EFile::operator=(const EFile &from)
{
#ifndef _WIN32
	int newFD = (from.fFD == NULL ? -1 : dup(*((int*)from.fFD)));
	if(newFD == -1)
	{
		if(fFD != NULL)
		{
			close(*((int*)fFD));
			free(fFD);
			fFD = NULL;
		}
	}
	else
	{
		if(fFD != NULL)
		{
			close(*((int*)fFD));
			*((int*)fFD) = newFD;
		}
		else if((fFD = malloc(sizeof(int))) == NULL)
		{
			close(newFD);
		}
		else
		{
			*((int*)fFD) = newFD;
		}
	}
#else
	if(fFD != NULL)
	{
		CloseHandle((HANDLE)fFD);
		fFD = NULL;
	}

	HANDLE newfFD = NULL;
	if(from.fFD) DuplicateHandle(GetCurrentProcess(), (HANDLE)from.fFD,
				     GetCurrentProcess(), &newfFD,
				     0, FALSE, DUPLICATE_SAME_ACCESS);
	if(newfFD) fFD = (void*)newfFD;
#endif

	fMode = from.fMode;

	return *this;
}

