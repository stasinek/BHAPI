/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
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
 * File: BufferIO.h
 *
 * --------------------------------------------------------------------------*/
 /*
 * Copyright 2007-2010 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_BUFFER_IO_H
#define BHAPI_BUFFER_IO_H

#include <DataIO.h>

class BBufferIO : public BPositionIO {
public:
								BBufferIO(BPositionIO* stream,
									size_t bufferSize = 65536L,
									bool ownsStream = true);
	virtual						~BBufferIO();

	// BPositionIO interface
	virtual	ssize_t				ReadAt(off_t pos, void* buffer, size_t size);
	virtual	ssize_t				WriteAt(off_t pos, const void* buffer,
									size_t size);
	virtual	off_t				Seek(off_t position, uint32_t seekMode);
	virtual	off_t				Position() const;
	virtual	status_t			SetSize(off_t size);
	virtual	status_t			Flush();

	// BBufferIO interface
			BPositionIO*		Stream() const;
			size_t				BufferSize() const;
			bool				OwnsStream() const;
			void				SetOwnsStream(bool ownsStream);

			void				PrintToStream() const;

private:
	virtual	status_t			_Reserved_BufferIO_0(void*);
	virtual	status_t			_Reserved_BufferIO_1(void*);
	virtual	status_t			_Reserved_BufferIO_2(void*);
	virtual	status_t			_Reserved_BufferIO_3(void*);
	virtual	status_t			_Reserved_BufferIO_4(void*);

private:
			off_t				fBufferStart;
			BPositionIO*		fStream;
			char*				fBuffer;
			size_t				fBufferSize;
			size_t				fBufferUsed;
			off_t				fPosition;

			uint32_t				_reserved_ints[4];

			bool				fBufferIsDirty;
			bool				fOwnsStream;

			bool				_reserved_bools[6];
};

#endif	// _BUFFER_IO_H
