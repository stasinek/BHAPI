/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
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

#ifndef BHAPI_FILE__H
#define BHAPI_FILE__H

#include "StorageDefs.h"
#include "Directory.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXP_BHAPI BFile {
public:
    BFile();
    BFile(const char *path, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
    BFile(const BEntry *entry, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
    BFile(const BDirectory *dir, const char *leaf, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
    BFile(const BFile &from);
    virtual ~BFile();

	b_status_t	InitCheck() const;
    b_status_t	SetTo(const char *path, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
    b_status_t	SetTo(const BEntry *entry, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
    b_status_t	SetTo(const BDirectory *dir, const char *leaf, b_uint32 open_mode, b_uint32 access_mode = B_USER_READ | B_USER_WRITE);
	void		Unset();

	bool		IsReadable() const;
	bool		IsWritable() const;

	b_size_t		Read(void *buffer, size_t size);
    b_size_t		ReadAt(b_int64 pos, void *buffer, size_t size);
	b_size_t		Write(const void *buffer, size_t size);
    b_size_t		WriteAt(b_int64 pos, const void *buffer, size_t size);

    b_int64		Seek(b_int64 position, b_uint32 seek_mode);
    b_int64		Position() const;
    b_status_t	SetSize(b_int64 size);

    BFile&		operator=(const BFile &from);

private:
	void *fFD;
    b_uint32 fMode;
};

#endif /* __cplusplus */

#endif /* BHAPI_FILE__H */

