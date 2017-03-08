/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_DATA_IO_H
#define BHAPI_DATA_IO_H

#include "../support/SupportDefs.h"
#ifdef __cplusplus /* Just for C++ */
class IMPEXPBHAPI BDataIO {
public:
    BDataIO();
    virtual ~BDataIO();

	virtual  __be_size_t		Read(void *buffer, size_t size) = 0;
	virtual  __be_size_t		Write(const void *buffer, size_t size) = 0;
};


class IMPEXPBHAPI BPositionIO : public BDataIO {
public:
    BPositionIO();
    virtual ~BPositionIO();

	virtual  __be_size_t		Read(void *buffer, size_t size);
	virtual  __be_size_t		Write(const void *buffer, size_t size);

    virtual  __be_size_t		ReadAt(__be_int64 pos, void *buffer, size_t size) = 0;
    virtual  __be_size_t		WriteAt(__be_int64 pos, const void *buffer, size_t size) = 0;

    virtual  __be_int64		Seek(__be_int64 position,  __be_uint32 seek_mode) = 0;
    virtual  __be_int64		Position() const = 0;
    virtual status_t	SetSize(__be_int64 size) = 0;
};


class IMPEXPBHAPI BMallocIO : public BPositionIO {
public:
    BMallocIO();
    virtual ~BMallocIO();

    virtual  __be_size_t		ReadAt(__be_int64 pos, void *buffer, size_t size);
    virtual  __be_size_t		WriteAt(__be_int64 pos, const void *buffer, size_t size);

    virtual  __be_int64		Seek(__be_int64 position,  __be_uint32 seek_mode);
    virtual  __be_int64		Position() const;
    virtual status_t	SetSize(__be_int64 size);

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

class IMPEXPBHAPI BMemoryIO : public BPositionIO {
public:
    BMemoryIO(void *ptr, size_t length);
    BMemoryIO(const void *ptr, size_t length);
    virtual ~BMemoryIO();

    virtual  __be_size_t		ReadAt(__be_int64 pos, void *buffer, size_t size);
    virtual  __be_size_t		WriteAt(__be_int64 pos, const void *buffer, size_t size);

    virtual  __be_int64		Seek(__be_int64 position,  __be_uint32 seek_mode);
    virtual  __be_int64		Position() const;
    virtual status_t	SetSize(__be_int64 size);

private:
	bool fReadOnly;
	char *fBuffer;
	size_t fLen;
	size_t fRealLen;
	size_t fPosition;
};

#endif /* __cplusplus */
#endif /* BHAPI_DATA_IO_H */

