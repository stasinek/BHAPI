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
 * File: BlockCache.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright (c) 2003 Marcus Overhagen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef BHAPI_BLOCK_CACHE_H
#define BHAPI_BLOCK_CACHE_H

#include <Locker.h>

// The allocation type to be used in the constructor
enum {
	B_OBJECT_CACHE = 0,
	B_MALLOC_CACHE = 1
};

class BBlockCache {
	public:
        BBlockCache(uint32_t a_blockCount, size_t a_blockSize,
            uint32_t a_allocationType);
		virtual	~BBlockCache();

        void*	Get(size_t a_blockSize);
        void	Save(void *a_pointer, size_t a_blockSize);

	private:
		virtual	void _ReservedBlockCache1();
		virtual	void _ReservedBlockCache2();

		BBlockCache(const BBlockCache &);
		BBlockCache	&operator=(const BBlockCache &);

		struct _FreeBlock;

		_FreeBlock*	fFreeList;
		size_t		fBlockSize;
		int32_t		fFreeBlocks;
		int32_t		fBlockCount;
		BLocker		fLocker;
        void*		(*fAlloc)(size_t a_size);
        void		(*fFree)(void *a_pointer);
		uint32_t		_reserved[2];
};

#endif	// _BLOCK_CACHE_H
