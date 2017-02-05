/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: DataIO.cpp
 *
 * --------------------------------------------------------------------------*/

#include "DataIO.h"


EDataIO::EDataIO()
{
}


EDataIO::~EDataIO()
{
}


EPositionIO::EPositionIO()
	: EDataIO()
{
}


EPositionIO::~EPositionIO()
{
}


ssize_t
EPositionIO::Read(void *buffer, size_t size)
{
	return ReadAt(0, buffer, size);
}


ssize_t
EPositionIO::Write(const void *buffer, size_t size)
{
	return WriteAt(0, buffer, size);
}


EMallocIO::EMallocIO()
	: EPositionIO(), fData(NULL), fBlockSize(256), fMallocSize(0), fLength(0), fPosition(0)
{
}


EMallocIO::~EMallocIO()
{
	if(fData != NULL) free(fData);
}


ssize_t
EMallocIO::ReadAt(eint64 pos, void *buffer, size_t size)
{
	if(buffer == NULL) return E_BAD_VALUE;

	if(size == 0 || EMallocIO::Seek(pos, E_SEEK_CUR) < 0) return 0;
	if(fPosition >= fLength) return 0;

	size = min_c(size, fLength - fPosition);
	if(memcpy(buffer, fData + fPosition, size) == NULL) return E_ERROR;

	fPosition += size;
	return size;
}


ssize_t
EMallocIO::WriteAt(eint64 pos, const void *buffer, size_t size)
{
	if(buffer == NULL) return E_BAD_VALUE;

	if(size == 0 || EMallocIO::Seek(pos, E_SEEK_CUR) < 0) return 0;

	if(fMallocSize >= fPosition + size ||
	   EMallocIO::SetSize((eint64)(fPosition + size)) == E_OK ||
	   (fMallocSize >= fPosition ? (size = min_c(size, fMallocSize - fPosition)) > 0 : false))
	{
		if(memcpy(fData + fPosition, buffer, size) == NULL) return E_ERROR;
		fPosition += size;
		if(fLength < fPosition) fLength = fPosition;
		return size;
	}
	return E_NO_MEMORY;
}


eint64
EMallocIO::Seek(eint64 position, euint32 seek_mode)
{
	eint64 retVal = E_INT64_CONSTANT(-1);

	switch(seek_mode)
	{
		case E_SEEK_SET:
			if(!(position < 0 || position > (eint64)~((size_t)0)))
				fPosition = (size_t)(retVal = position);
			break;

		case E_SEEK_CUR:
			if(position < 0 ? (eint64)fPosition >= -position : position <= (eint64)(~((size_t)0) - fPosition))
			{
				if(position < 0) fPosition -= (size_t)(-position);
				else fPosition += (size_t)position;
				retVal = (eint64)fPosition;
			}
			break;

		case E_SEEK_END:
			if(position < 0 ? (eint64)fLength >= -position : position <= (eint64)(~((size_t)0) - fLength))
			{
				if(position < 0) fPosition = fLength - (size_t)(-position);
				else fPosition = fLength + (size_t)position;
				retVal = (eint64)fPosition;
			}
			break;

		default:
			break;
	}

	return retVal;
}


eint64
EMallocIO::Position() const
{
	return (eint64)fPosition;
}


e_status_t
EMallocIO::SetSize(eint64 size)
{
	if(size < 0) return E_BAD_VALUE;
	if(size == 0)
	{
		if(fData) free(fData);
		fData = NULL;
		fPosition = fLength = fMallocSize = 0;
		return E_OK;
	}

	eint64 alloc_size = size >= E_MAXINT64 - fBlockSize ?
				E_MAXINT64 : ((size + (eint64)fBlockSize - 1) & ~((eint64)fBlockSize - 1));

	if(alloc_size > (eint64)~((size_t)0)) alloc_size = (eint64)~((size_t)0);
	if(alloc_size != (eint64)fMallocSize)
	{
		char *data = (char*)realloc(fData, (size_t)alloc_size);
		if(data == NULL)
		{
			if(size > (eint64)fMallocSize) return E_NO_MEMORY;
		}
		else
		{
			fData = data;
			fMallocSize = (size_t)alloc_size;
		}
	}

	fLength = (size_t)size;

	return E_OK;
}


void
EMallocIO::SetBlockSize(size_t blocksize)
{
	fBlockSize = blocksize;
}


const void*
EMallocIO::Buffer() const
{
	return((const void*)fData);
}


size_t
EMallocIO::BufferLength()
{
	return fLength;
}


EMemoryIO::EMemoryIO(void *ptr, size_t length)
	: EPositionIO(), fReadOnly(false), fBuffer((char*)ptr), fLen(length), fRealLen(length), fPosition(0)
{
	if(fBuffer == NULL) fRealLen = fLen = 0;
}


EMemoryIO::EMemoryIO(const void *ptr, size_t length)
	: EPositionIO(), fReadOnly(true), fBuffer((char*)ptr), fLen(length), fRealLen(length), fPosition(0)
{
	if(fBuffer == NULL) fRealLen = fLen = 0;
}


EMemoryIO::~EMemoryIO()
{
}


ssize_t
EMemoryIO::ReadAt(eint64 pos, void *buffer, size_t size)
{
	if(buffer == NULL) return E_BAD_VALUE;

	if(size == 0 || EMemoryIO::Seek(pos, E_SEEK_CUR) < 0) return 0;
	if(fPosition >= fLen) return 0;

	size = min_c(size, fLen - fPosition);
	if(memcpy(buffer, fBuffer + fPosition, size) == NULL) return E_ERROR;

	fPosition += size;
	return size;
}


ssize_t
EMemoryIO::WriteAt(eint64 pos, const void *buffer, size_t size)
{
	if(fReadOnly) return E_NOT_ALLOWED;
	if(buffer == NULL) return E_BAD_VALUE;

	if(size == 0 || EMemoryIO::Seek(pos, E_SEEK_CUR) < 0) return 0;

	if(fLen > fPosition)
	{
		size = min_c(size, fLen - fPosition);
		if(memcpy(fBuffer + fPosition, buffer, size) == NULL) return E_ERROR;
		fPosition += size;
		return size;
	}

	return 0;
}


eint64
EMemoryIO::Seek(eint64 position, euint32 seek_mode)
{
	eint64 retVal = E_INT64_CONSTANT(-1);

	switch(seek_mode)
	{
		case E_SEEK_SET:
			if(!(position < 0 || position > (eint64)fRealLen))
				fPosition = (size_t)(retVal = position);
			break;

		case E_SEEK_CUR:
			if(position < 0 ? (eint64)fPosition >= -position : position <= (eint64)(fRealLen - fPosition))
			{
				if(position < 0) fPosition -= (size_t)(-position);
				else fPosition += (size_t)position;
				retVal = (eint64)fPosition;
			}
			break;

		case E_SEEK_END:
			if(position < 0 ? (eint64)fLen >= -position : position <= (eint64)(fRealLen - fLen))
			{
				if(position < 0) fPosition = fLen - (size_t)(-position);
				else fPosition = fLen + (size_t)position;
				retVal = (eint64)fPosition;
			}
			break;

		default:
			break;
	}

	return retVal;
}


eint64
EMemoryIO::Position() const
{
	return (eint64)fPosition;
}


e_status_t
EMemoryIO::SetSize(eint64 size)
{
	if(size > (eint64)fRealLen) return E_NO_MEMORY;
	fLen = (size_t)size;
	return E_OK;
}

