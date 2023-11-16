/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku OS source-code, All Rights Reserved
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
 * File: StackOrHeapArray.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2012, Jonathan Schleifer <js@webkeks.org>. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	BHAPI_SUPPORT_STACKORHEAPARRAY_H
#define	BHAPI_SUPPORT_STACKORHEAPARRAY_H

#include <cstddef>
#include <new>
template <typename Type, int StackSize> class BStackOrHeapArray {
public:
	BStackOrHeapArray(size_t count)
	{
		if (count > StackSize)
			fData = new(std::nothrow) Type[count];
		else
			fData = fStackData;
	}

	~BStackOrHeapArray()
	{
		if (fData != fStackData)
			delete[] fData;
	}

	bool IsValid() const
	{
		return fData != NULL;
	}

	operator Type*()
	{
		return fData;
	}

private:
	Type	fStackData[StackSize];
	Type*	fData;
};

#endif /* BHAPI_STACKORHEAPARRAY__H */
