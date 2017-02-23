/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: DataIO.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_DATA_IO__H
#define BHAPI_DATA_IO__H

#include "../support/SupportDefs.h"
#ifdef __cplusplus /* Just for C++ */


class IMPEXP_BHAPI EDataIO {
public:
	EDataIO();
	virtual ~EDataIO();

	virtual b_size_t		Read(void *buffer, size_t size) = 0;
	virtual b_size_t		Write(const void *buffer, size_t size) = 0;
};


class IMPEXP_BHAPI BPositionIO : public EDataIO {
public:
    BPositionIO();
    virtual ~BPositionIO();

	virtual b_size_t		Read(void *buffer, size_t size);
	virtual b_size_t		Write(const void *buffer, size_t size);

    virtual b_size_t		ReadAt(b_int64 pos, void *buffer, size_t size) = 0;
    virtual b_size_t		WriteAt(b_int64 pos, const void *buffer, size_t size) = 0;

    virtual b_int64		Seek(b_int64 position, b_uint32 seek_mode) = 0;
    virtual b_int64		Position() const = 0;
    virtual b_status_t	SetSize(b_int64 size) = 0;
};


class IMPEXP_BHAPI BMallocIO : public BPositionIO {
public:
    BMallocIO();
    virtual ~BMallocIO();

    virtual b_size_t		ReadAt(b_int64 pos, void *buffer, size_t size);
    virtual b_size_t		WriteAt(b_int64 pos, const void *buffer, size_t size);

    virtual b_int64		Seek(b_int64 position, b_uint32 seek_mode);
    virtual b_int64		Position() const;
    virtual b_status_t	SetSize(b_int64 size);

	void			SetBlockSize(size_t blocksize);
	const void		*Buffer() const;
	size_t			BufferLength();

private:
	char *fData;
	size_t fBlockSize;
	size_t fMallocSize;
	size_t fLength;
	size_t fPosition;
};


class IMPEXP_BHAPI BMemoryIO : public BPositionIO {
public:
    BMemoryIO(void *ptr, size_t length);
    BMemoryIO(const void *ptr, size_t length);
    virtual ~BMemoryIO();

    virtual b_size_t		ReadAt(b_int64 pos, void *buffer, size_t size);
    virtual b_size_t		WriteAt(b_int64 pos, const void *buffer, size_t size);

    virtual b_int64		Seek(b_int64 position, b_uint32 seek_mode);
    virtual b_int64		Position() const;
    virtual b_status_t	SetSize(b_int64 size);

private:
	bool fReadOnly;
	char *fBuffer;
	size_t fLen;
	size_t fRealLen;
	size_t fPosition;
};


#endif /* __cplusplus */

#endif /* BHAPI_DATA_IO__H */

