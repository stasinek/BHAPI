/*
 * Copyright 2009, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *              Bruno Albuquerque, bga@bug-br.org.br
 */

#include <DynamicBuffer.h>




#include <algorithm>

#include <Errors.h>
#include <Haiku.h>

#include <new>

DynamicBuffer::DynamicBuffer(size_t initialSize) :
	fBuffer(NULL),
	fBufferSize(0),
	fDataStart(0),
	fDataEnd(0),
	fInit(B_NO_INIT)
{
	fBuffer = new (std::nothrow) unsigned char[initialSize];
	if (fBuffer != NULL) {
		fBufferSize = initialSize;
		fInit = B_OK;
	}
}


DynamicBuffer::~DynamicBuffer()
{
	delete[] fBuffer;
	fBufferSize = 0;
	fDataStart = 0;
	fDataEnd = 0;
}


DynamicBuffer::DynamicBuffer(const DynamicBuffer& buffer) :
	fBuffer(NULL),
	fBufferSize(0),
	fDataStart(0),
	fDataEnd(0),
	fInit(B_NO_INIT)
{
	fInit = buffer.fInit;
	if (fInit == B_OK) {
		status_t result = _GrowToFit(buffer.fBufferSize, true);
		if (result == B_OK) {
			memcpy(fBuffer, buffer.fBuffer, fBufferSize);
			fDataStart = buffer.fDataStart;
			fDataEnd = buffer.fDataEnd;
		} else
			fInit = result;
	}
}


status_t DynamicBuffer::InitCheck() const
{
	return fInit;
}


ssize_t
DynamicBuffer::Write(const void* data, size_t size)
{
	if (fInit != B_OK)
		return fInit;

	status_t result = _GrowToFit(size);
	if (result != B_OK)
		return result;

	memcpy(fBuffer + fDataEnd, data, size);
	fDataEnd += size;

	return (ssize_t)size;
}


ssize_t
DynamicBuffer::Read(void* data, size_t size)
{
	if (fInit != B_OK)
		return fInit;

	size = std::min(size, Size());
	if (size == 0)
		return 0;

	memcpy(data, fBuffer + fDataStart, size);
	fDataStart += size;

	if (fDataStart == fDataEnd)
		fDataStart = fDataEnd = 0;

	return size;
}


unsigned char*  DynamicBuffer::Data() const
{
	return fBuffer + fDataStart;
}


size_t
DynamicBuffer::Size() const
{
	return fDataEnd - fDataStart;
}


size_t
DynamicBuffer::BytesRemaining() const
{
	return fBufferSize - fDataEnd;
}


void DynamicBuffer::PrintToStream()
{
	printf("Current buffer size : %ld\n", fBufferSize);
	printf("Data start position : %ld\n", fDataStart);
	printf("Data end position   : %ld\n", fDataEnd);
	printf("Bytes wasted        : %ld\n", fDataStart);
	printf("Bytes available     : %ld\n", fBufferSize - fDataEnd);
}


status_t DynamicBuffer::_GrowToFit(size_t size, bool exact)
{
	if (size <= fBufferSize - fDataEnd)
		return B_OK;

	size_t newSize;
	if (!exact)
		newSize = (fBufferSize + size) * 2;
	else
		newSize = size;

	unsigned char* newBuffer = new (std::nothrow) unsigned char[newSize];
	if (newBuffer == NULL)
		return B_NO_MEMORY;

	if (fDataStart != fDataEnd)
		memcpy(newBuffer, fBuffer + fDataStart, fDataEnd - fDataStart);

	delete[] fBuffer;
	fBuffer = newBuffer;
	fDataEnd -= fDataStart;
	fDataStart = 0;
	fBufferSize = newSize;

	return B_OK;
}
