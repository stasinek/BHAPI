/*
 * Copyright 2005, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef _BOOT_NET_CHAIN_BUFFER_H
#define _BOOT_NET_CHAIN_BUFFER_H

#include <Haiku.h>

class ChainBuffer {
public:

	ChainBuffer(void *data = 0, uint32_t size = 0, ChainBuffer *next = NULL,
		bool freeData = false);
	~ChainBuffer();

	void *Data() const			{ return fData; }
	uint32_t Size() const			{ return fSize; }
	uint32_t TotalSize() const	{ return fTotalSize; }

	ChainBuffer *Next() const	{ return fNext; }
	ChainBuffer *DetachNext();

	void Append(ChainBuffer *next);

	void Flatten(void *_buffer) const;

private:
// TODO: Implement Create() and Delete(). Make new and delete operators private.
	enum {
		CHAIN_BUFFER_HEAD			= 0x1,
		CHAIN_BUFFER_EMBEDDED_DATA	= 0x2,
		CHAIN_BUFFER_FREE_DATA		= 0x4,
		CHAIN_BUFFER_ON_STACK		= 0x8,
	};

	void _Init(void *data, uint32_t size, ChainBuffer *next, uint32_t flags);
	void _Destroy();

	uint32_t		fFlags:4;
	uint32_t		fSize:14;
	uint32_t		fTotalSize:14;
	void		*fData;
	ChainBuffer	*fNext;
	uint8		fBuffer[0];
};

#endif	// _BOOT_NET_CHAIN_BUFFER_H
