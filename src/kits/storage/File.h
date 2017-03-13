/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_FILE_H
#define BHAPI_FILE_H


#ifdef __cplusplus /* Just for C++ */
#include "Directory.h"
#include "StorageDefs.h"
#include <Haiku.h>
class BHAPI_IMPEXP BFile {
public:
    BFile();
    BFile(const char *path,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    BFile(const BEntry *entry,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    BFile(const BDirectory *dir, const char *leaf,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    BFile(const BFile &from);
    virtual ~BFile();

    status_t	InitCheck() const;
    status_t	SetTo(const char *path,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    status_t	SetTo(const BEntry *entry,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    status_t	SetTo(const BDirectory *dir, const char *leaf,  __be_uint32 open_mode,  __be_uint32 access_mode = bhapi::B_USER_READ | bhapi::B_USER_WRITE);
    void		Unset();

    bool		IsReadable() const;
    bool		IsWritable() const;

    __be_size_t		Read(void *buffer, size_t size);
     __be_size_t		ReadAt(__be_int64 pos, void *buffer, size_t size);
    __be_size_t		Write(const void *buffer, size_t size);
     __be_size_t		WriteAt(__be_int64 pos, const void *buffer, size_t size);

     __be_int64		Seek(__be_int64 position,  __be_uint32 seek_mode);
     __be_int64		Position() const;
    status_t	SetSize(__be_int64 size);

    BFile&		operator=(const BFile &from);

private:
    void *fFD;
     __be_uint32 fMode;
};
#endif /* __cplusplus */
#define BFILE_I
#endif /* BHAPI_FILE_H */

