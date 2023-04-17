/*
 * Copyright 2009, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BUFFER_GROUP_H
#define _BUFFER_GROUP_H


#include <MediaDefs.h>


class BBuffer;
namespace BPrivate {
	struct SharedBufferList;
}


class BBufferGroup {
public:
							BBufferGroup(size_t size, int32_t count = 3,
								uint32_t placement = B_ANY_ADDRESS,
								uint32_t lock = B_FULL_LOCK);
	explicit				BBufferGroup();
							BBufferGroup(int32_t count,
								const media_buffer_id* buffers);
							~BBufferGroup();

			status_t		InitCheck();

			status_t		AddBuffer(const buffer_clone_info& info,
								BBuffer** _buffer = NULL);

			BBuffer*		RequestBuffer(size_t size,
								bigtime_t timeout = B_INFINITE_TIMEOUT);
			status_t		RequestBuffer(BBuffer* buffer,
								bigtime_t timeout = B_INFINITE_TIMEOUT);

			status_t		RequestError();

			status_t		CountBuffers(int32_t* _count);
			status_t		GetBufferList(int32_t bufferCount,
								BBuffer** _buffers);

			status_t		WaitForBuffers();
			status_t		ReclaimAllBuffers();

private:
			// deprecated BeOS R4 API
			status_t 		AddBuffersTo(BMessage* message, const char* name,
								bool needLock);

							BBufferGroup(const BBufferGroup& other);
			BBufferGroup&	operator=(const BBufferGroup& other);

			status_t		_Init();

private:
	friend struct BPrivate::SharedBufferList;

			status_t		fInitError;
			status_t		fRequestError;
			int32_t			fBufferCount;
			BPrivate::SharedBufferList* fBufferList;
			sem_id			fReclaimSem;

			uint32_t			_reserved[9];
};


#endif	// _BUFFER_GROUP_H
