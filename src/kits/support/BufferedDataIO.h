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
 * File: BufferedDataIO.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2011 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_BUFFERED_DATA_IO_H
#define BHAPI_BUFFERED_DATA_IO_H

#include <DataIO.h>
class BBufferedDataIO : public BDataIO {
public:
								BBufferedDataIO(BDataIO& stream,
									size_t bufferSize = 65536L,
									bool ownsStream = true,
									bool partialReads = false);
	virtual						~BBufferedDataIO();

			status_t			InitCheck() const;

			BDataIO*			Stream() const;
			size_t				BufferSize() const;
			bool				OwnsStream() const;
			void				SetOwnsStream(bool ownsStream);
			status_t			Flush();

	// BDataIO interface
	virtual	ssize_t				Read(void* buffer, size_t size);
	virtual	ssize_t				Write(const void* buffer, size_t size);

private:
								BBufferedDataIO(const BBufferedDataIO& other);
									// not implemented

	virtual	status_t			_Reserved0(void*);
	virtual	status_t			_Reserved1(void*);
	virtual	status_t			_Reserved2(void*);
	virtual	status_t			_Reserved3(void*);
	virtual	status_t			_Reserved4(void*);

private:
			BDataIO&			fStream;
			uint8*				fBuffer;
			size_t				fBufferSize;
			size_t				fPosition;
			size_t				fSize;

			uint32				_reserved_ints[4];

			bool				fDirty;
			bool				fOwnsStream;
			bool				fPartialReads;

			bool				_reserved_bools[5];
};

#endif	// _BUFFERED_DATA_IO_H
