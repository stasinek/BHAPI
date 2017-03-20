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

#include "BlockCache.h"
#include "../kernel/Debug.h"


#include <new>

#define MAGIC1		0x9183f4d9
#define MAGIC2		0xa6b3c87d

struct BBlockCache::_FreeBlock {
	DEBUG_ONLY(	uint32		magic1;	)
				_FreeBlock *next;
	DEBUG_ONLY(	uint32		magic2;	)
};

// The requirements set by the BeBook's description of the destructor,
// as well as Get() function, allowing the caller to dispose of the
// memory, do not allow to allocate one large block to be used as pool.
// Thus we need to create multiple small ones.
// We maintain a list of free blocks.

BBlockCache::BBlockCache(uint32 a_blockCount, size_t a_blockSize,uint32 a_allocationType)
	:
	fFreeList(0),
    fBlockSize(a_blockSize),
	fFreeBlocks(0),
    fBlockCount(a_blockCount),
	fLocker("some BBlockCache lock"),
	fAlloc(0),
	fFree(0)
{
    switch (a_allocationType) {
		case B_OBJECT_CACHE:
			fAlloc = &operator new[];
			fFree = &operator delete[];
			break;
		case B_MALLOC_CACHE:
		default:
			fAlloc = &malloc;
			fFree = &free;
			break;
	}

	// To properly maintain a list of free buffers, a buffer must be
	// large enough to contain the _FreeBlock struct that is used.
    if (a_blockSize < sizeof(_FreeBlock))
        a_blockSize = sizeof(_FreeBlock);

	// should have at least one block
    if (a_blockCount == 0)
        a_blockCount = 1;

	// create blocks and put them into the free list
    while (a_blockCount--) {
        _FreeBlock *block = reinterpret_cast<_FreeBlock *>(fAlloc(a_blockSize));
		if (!block)
			break;
		fFreeBlocks++;
		block->next = fFreeList;
		fFreeList = block;
		DEBUG_ONLY(block->magic1 = MAGIC1);
		DEBUG_ONLY(block->magic2 = MAGIC2 + (uint32)(addr_t)block->next);
	}
}

BBlockCache::~BBlockCache()
{
	// walk the free list and deallocate all blocks
	fLocker.Lock();
	while (fFreeList) {
		ASSERT(fFreeList->magic1 == MAGIC1);
		ASSERT(fFreeList->magic2 == MAGIC2 + (uint32)(addr_t)fFreeList->next);
		void *pointer = fFreeList;
		fFreeList = fFreeList->next;
		DEBUG_ONLY(memset(pointer, 0xCC, sizeof(_FreeBlock)));
		fFree(pointer);
	}
	fLocker.Unlock();
}

void *BBlockCache::Get(size_t a_blockSize)
{
	if (!fLocker.Lock())
		return 0;
	void *pointer;
    if (a_blockSize == fBlockSize && fFreeList != 0) {
		// we can take a block from the list
		ASSERT(fFreeList->magic1 == MAGIC1);
		ASSERT(fFreeList->magic2 == MAGIC2 + (uint32)(addr_t)fFreeList->next);
		pointer = fFreeList;
		fFreeList = fFreeList->next;
		fFreeBlocks--;
		DEBUG_ONLY(memset(pointer, 0xCC, sizeof(_FreeBlock)));
	} else {
        if (a_blockSize < sizeof(_FreeBlock))
            a_blockSize = sizeof(_FreeBlock);
		pointer = fAlloc(blockSize);
		DEBUG_ONLY(if (pointer) memset(pointer, 0xCC, sizeof(_FreeBlock)));
	}
	fLocker.Unlock();
	return pointer;
}

void BBlockCache::Save(void *a_pointer, size_t a_blockSize)
{
	if (!fLocker.Lock())
		return;
    if (a_blockSize == fBlockSize && fFreeBlocks < fBlockCount) {
		// the block needs to be returned to the cache
		_FreeBlock *block = reinterpret_cast<_FreeBlock *>(pointer);
		block->next = fFreeList;
		fFreeList = block;
		fFreeBlocks++;
		DEBUG_ONLY(block->magic1 = MAGIC1);
		DEBUG_ONLY(block->magic2 = MAGIC2 + (uint32)(addr_t)block->next);
	} else {
		DEBUG_ONLY(memset(pointer, 0xCC, sizeof(_FreeBlock)));
        fFree(a_pointer);
	}
	fLocker.Unlock();
}

void BBlockCache::_ReservedBlockCache1() {}
void BBlockCache::_ReservedBlockCache2() {}
