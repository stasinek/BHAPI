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
 * File: File.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_FILE_H__
#define __ETK_FILE_H__ 

#include "./StorageDefs.h"
#include "./Directory.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EFile {
public:
	EFile();
	EFile(const char *path, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	EFile(const EEntry *entry, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	EFile(const EDirectory *dir, const char *leaf, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	EFile(const EFile &from);
	virtual ~EFile();

	e_status_t	InitCheck() const;
	e_status_t	SetTo(const char *path, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	e_status_t	SetTo(const EEntry *entry, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	e_status_t	SetTo(const EDirectory *dir, const char *leaf, euint32 open_mode, euint32 access_mode = E_USER_READ | E_USER_WRITE);
	void		Unset();

	bool		IsReadable() const;
	bool		IsWritable() const;

	ssize_t		Read(void *buffer, size_t size);
	ssize_t		ReadAt(eint64 pos, void *buffer, size_t size);
	ssize_t		Write(const void *buffer, size_t size);
	ssize_t		WriteAt(eint64 pos, const void *buffer, size_t size);

	eint64		Seek(eint64 position, euint32 seek_mode);
	eint64		Position() const;
	e_status_t	SetSize(eint64 size);

	EFile&		operator=(const EFile &from);

private:
	void *fFD;
	euint32 fMode;
};

#endif /* __cplusplus */

#endif /* __ETK_FILE_H__ */

