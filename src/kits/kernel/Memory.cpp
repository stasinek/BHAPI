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
 * File: Memory.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Memory.h"

#include "../kernel/Debug.h"

struct BHAPI_LOCAL b_mem {
	void (*destroy_func)(void*);
	void *data;
};


void*
BMemory::Malloc(size_t size, void (*destroy_func)(void*))
{
	b_mem *mem = NULL;

	if(~((size_t)0) - sizeof(b_mem) < size ||
	   (mem = (b_mem*)malloc(sizeof(b_mem) + size)) == NULL) return NULL;

	mem->destroy_func = destroy_func;
	mem->data = (unsigned char*)mem + sizeof(b_mem);

	return(mem->data);
}


void 
BMemory::Free(void *data)
{
	b_mem *mem = NULL;

	if(data == NULL) return;

	mem = (b_mem*)((unsigned char*)data - sizeof(b_mem));
	if(mem->data != data) BHAPI_ERROR("[PRIVATE]: %s --- Invalid pointer.", __PRETTY_FUNCTION__);
	if(mem->destroy_func != NULL) mem->destroy_func(mem->data);
	free(mem);
}

