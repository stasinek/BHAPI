/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: etk-port.cpp
 *
 * --------------------------------------------------------------------------*/

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/StringMe.h"
#include "../support/Errors.h"
#include "../support/SimpleLocker.h"

typedef struct b_port_info {
	b_port_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, B_OS_NAME_LENGTH + 1);
		queue_length = 0;
		queue_count = 0;
		readerWaitCount = B_INT64_CONSTANT(0);
		writerWaitCount = B_INT64_CONSTANT(0);
		closed = false;
	}

	char			name[B_OS_NAME_LENGTH + 1];
	b_int32			queue_length;
	b_int32			queue_count;
	b_int64			readerWaitCount;
	b_int64			writerWaitCount;
	bool			closed;
} b_port_info;

typedef struct b_port_t {
	b_port_t()
		: iLocker(NULL), readerSem(NULL), writerSem(NULL), mapping(NULL), queueBuffer(NULL),
		  openedIPC(false), portInfo(NULL), created(false), refCount(0)
	{
	}

	~b_port_t()
	{
		if(created)
		{
			created = false;
			b_delete_port((void*)this);
		}
	}

	void*			iLocker;
	void*			readerSem;
	void*			writerSem;

	// for IPC (name != NULL)
	void*			mapping;

	void*			queueBuffer;

	bool			openedIPC;

	b_port_info*		portInfo;

	bool			created;
	b_uint32			refCount;
} b_port_t;

class b_port_locker_t {
public:
	void *fSem;
	BSimpleLocker fLocker;

	b_port_locker_t()
		: fSem(NULL)
	{
	}

	~b_port_locker_t()
	{
		if(fSem != NULL)
		{
			// leave global semaphore, without "b_delete_sem(fSem)"
			b_delete_sem_etc(fSem, false);
		}
	}

	void Init()
	{
		if(fSem != NULL) return;

		if((fSem = b_clone_sem("_port_global_")) == NULL)
			fSem = b_create_sem(1, "_port_global_", BHAPI_AREA_ACCESS_ALL);
		if(fSem == NULL) BHAPI_ERROR("[KERNEL]: Can't initialize global port!");
	}

	void LockLocal()
	{
		fLocker.Lock();
	}

	void UnlockLocal()
	{
		fLocker.Unlock();
	}

	void LockIPC()
	{
		LockLocal();
		Init();
		UnlockLocal();
		b_acquire_sem(fSem);
	}

	void UnlockIPC()
	{
		b_release_sem(fSem);
	}
};

static b_port_locker_t __bhapi_port_locker__;
#define _BHAPI_LOCK_IPC_PORT_()		__bhapi_port_locker__.LockIPC()
#define _BHAPI_UNLOCK_IPC_PORT_()		__bhapi_port_locker__.UnlockIPC()
#define _BHAPI_LOCKLOCAL_BHAPI_PORT_()		__bhapi_port_locker__.LockLocal()
#define _BHAPI_UNLOCKLOCAL_BHAPI_PORT_()	__bhapi_port_locker__.UnlockLocal()


static bool b_is_port_for_IPC(const b_port_t *port)
{
	if(!port) return false;
	return(port->mapping != NULL);
}


static void b_lock_port_inter(b_port_t *port)
{
	if(b_is_port_for_IPC(port))
		b_acquire_sem(port->iLocker);
	else
		b_lock_locker(port->iLocker);
}


static void b_unlock_port_inter(b_port_t *port)
{
	if(b_is_port_for_IPC(port))
		b_release_sem(port->iLocker);
	else
		b_unlock_locker(port->iLocker);
}

#define BHAPI_PORT_PER_MESSAGE_LENGTH	(sizeof(b_int32) + sizeof(size_t) + BHAPI_MAX_PORT_BUFFER_SIZE)

static void* b_create_port_for_IPC(b_int32 queue_length, const char *name, b_area_access area_access)
{
	if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH ||
	   name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	b_port_t *port = new b_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_BHAPI_LOCK_IPC_PORT_();
	if((port->mapping = b_create_area(name, (void**)&(port->portInfo),
					    sizeof(b_port_info) + (size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH,
					    B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN, area_access)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	b_port_info *port_info = port->portInfo;
	port_info->InitData();
	memcpy(port_info->name, name, (size_t)strlen(name));
	port_info->queue_length = queue_length;

	if((port->iLocker = b_create_sem(1, name, area_access)) == NULL)
	{
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = b_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		b_delete_sem(port->iLocker);
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = b_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		b_delete_sem(port->readerSem);
		b_delete_sem(port->iLocker);
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_BHAPI_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(b_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = false;
	port->created = true;

	return (void*)port;
}


EXPORT_BHAPI void* b_open_port(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	b_port_t *port = new b_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_BHAPI_LOCK_IPC_PORT_();
	if((port->mapping = b_clone_area(name, (void**)&(port->portInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	if((port->iLocker = b_clone_sem(name)) == NULL)
	{
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = b_clone_sem(tmpSemName)) == NULL)
	{
		b_delete_sem(port->iLocker);
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = b_clone_sem(tmpSemName)) == NULL)
	{
		b_delete_sem(port->readerSem);
		b_delete_sem(port->iLocker);
		b_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_BHAPI_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(b_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = true;
	port->created = true;

	return (void*)port;
}


EXPORT_BHAPI void* b_open_port_by_source(void *data)
{
	b_port_t *port = (b_port_t*)data;
	if(!port || !port->portInfo) return NULL;

	if(b_is_port_for_IPC(port)) return b_open_port(port->portInfo->name);

	_BHAPI_LOCKLOCAL_BHAPI_PORT_();
	if(port->refCount == B_MAXUINT32 || port->refCount == 0 || port->portInfo->closed)
	{
		_BHAPI_UNLOCKLOCAL_BHAPI_PORT_();
		return NULL;
	}
	port->refCount += 1;
	_BHAPI_UNLOCKLOCAL_BHAPI_PORT_();

	return data;
}


static void* b_create_port_for_local(b_int32 queue_length)
{
	if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH) return NULL;

	b_port_t *port = new b_port_t();
	if(!port) return NULL;

	if((port->iLocker = b_create_locker()) == NULL)
	{
		delete port;
		return NULL;
	}
	if((port->readerSem = b_create_sem(0, NULL)) == NULL)
	{
		b_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}
	if((port->writerSem = b_create_sem(0, NULL)) == NULL)
	{
		b_delete_sem(port->readerSem);
		b_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->portInfo = new b_port_info()) == NULL)
	{
		b_delete_sem(port->writerSem);
		b_delete_sem(port->readerSem);
		b_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->queueBuffer = malloc((size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH)) == NULL)
	{
		delete port->portInfo;
		b_delete_sem(port->writerSem);
		b_delete_sem(port->readerSem);
		b_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	port->portInfo->queue_length = queue_length;

	port->refCount = 1;
	port->created = true;

	return (void*)port;
}


EXPORT_BHAPI void* b_create_port(b_int32 queue_length, const char *name, b_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			b_create_port_for_local(queue_length) :
			b_create_port_for_IPC(queue_length, name, area_access));
}


EXPORT_BHAPI b_status_t b_delete_port(void *data)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if(b_is_port_for_IPC(port))
	{
		b_delete_area(port->mapping);
		b_delete_sem(port->iLocker);
	}
	else
	{
		_BHAPI_LOCKLOCAL_BHAPI_PORT_();
		if(port->refCount == 0)
		{
			_BHAPI_UNLOCKLOCAL_BHAPI_PORT_();
			return B_ERROR;
		}
		b_uint32 count = --(port->refCount);
		_BHAPI_UNLOCKLOCAL_BHAPI_PORT_();

		if(count > 0) return B_OK;

		free(port->queueBuffer);
		delete port->portInfo;
		b_delete_locker(port->iLocker);
	}

	b_delete_sem(port->writerSem);
	b_delete_sem(port->readerSem);

	if(port->created)
	{
		port->created = false;
		delete port;
	}

	return B_OK;
}


EXPORT_BHAPI b_status_t b_close_port(void *data)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

	b_lock_port_inter(port);
	if(port->portInfo->closed)
	{
		b_unlock_port_inter(port);
		return B_ERROR;
	}
	port->portInfo->closed = true;
	b_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);
	b_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);
	b_unlock_port_inter(port);

	return B_OK;
}


EXPORT_BHAPI b_status_t b_write_port_etc(void *data, b_int32 code, const void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if((!buf && buf_size > 0) || buf_size > BHAPI_MAX_PORT_BUFFER_SIZE || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	b_bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	b_lock_port_inter(port);

	if(port->portInfo->closed)
	{
		b_unlock_port_inter(port);
		return B_ERROR;
	}
	else if(port->portInfo->queue_count < port->portInfo->queue_length)
	{
		size_t offset = (size_t)port->portInfo->queue_count * BHAPI_PORT_PER_MESSAGE_LENGTH;
		char* buffer = (char*)(port->queueBuffer);
		buffer += offset;
		memcpy(buffer, &code, sizeof(b_int32)); buffer += sizeof(b_int32);
		memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
		if(buf_size > 0) memcpy(buffer, buf, buf_size);

		port->portInfo->queue_count++;

		b_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

		b_unlock_port_inter(port);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		b_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->writerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		b_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						b_acquire_sem(port->readerSem) :
						b_acquire_sem_etc(port->readerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		b_lock_port_inter(port);

		if(status != B_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->closed)
		{
			retval = B_ERROR;
			break;
		}
		else if(port->portInfo->queue_count < port->portInfo->queue_length)
		{
			size_t offset = (size_t)port->portInfo->queue_count * BHAPI_PORT_PER_MESSAGE_LENGTH;
			char* buffer = (char*)(port->queueBuffer);
			buffer += offset;
			memcpy(buffer, &code, sizeof(b_int32)); buffer += sizeof(b_int32);
			memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
			if(buf_size > 0) memcpy(buffer, buf, buf_size);

			port->portInfo->queue_count++;
			b_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

			retval = B_OK;
			break;
		}
	}

	port->portInfo->writerWaitCount -= B_INT64_CONSTANT(1);

	b_unlock_port_inter(port);

	return retval;
}


EXPORT_BHAPI b_size_t b_port_buffer_size_etc(void *data, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

    if(microseconds_timeout < B_INT64_CONSTANT(0)) return (b_size_t)B_BAD_VALUE;

	b_bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	b_lock_port_inter(port);

	if(port->portInfo->queue_count > 0)
	{
		const char* buffer = (const char*)(port->queueBuffer);
		size_t msgLen = 0;

		buffer += sizeof(b_int32);
		memcpy(&msgLen, buffer, sizeof(size_t));

		b_unlock_port_inter(port);
		return (b_size_t)msgLen;
	}
	else if(port->portInfo->closed)
	{
		b_unlock_port_inter(port);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		b_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		b_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						b_acquire_sem(port->writerSem) :
						b_acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		b_lock_port_inter(port);

		if(status != B_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->queue_count > 0)
		{
			const char* buffer = (const char*)(port->queueBuffer);
			size_t msgLen = 0;

			buffer += sizeof(b_int32);
			memcpy(&msgLen, buffer, sizeof(size_t));

			retval = (b_status_t)msgLen;
			break;
		}
		else if(port->portInfo->closed)
		{
			retval = B_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= B_INT64_CONSTANT(1);

	b_unlock_port_inter(port);

	return (b_size_t)retval;
}


EXPORT_BHAPI b_status_t b_read_port_etc(void *data, b_int32 *code, void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if(!code || (!buf && buf_size > 0) || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	b_bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	b_lock_port_inter(port);

	if(port->portInfo->queue_count > 0)
	{
		char* buffer = (char*)(port->queueBuffer);
		size_t msgLen = 0;
		memcpy(code, buffer, sizeof(b_int32)); buffer += sizeof(b_int32);
		memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
		if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
		if(port->portInfo->queue_count > 1)
		{
			buffer = (char*)(port->queueBuffer);
			memmove(buffer, buffer + BHAPI_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * BHAPI_PORT_PER_MESSAGE_LENGTH);
		}

		port->portInfo->queue_count--;

		b_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

		b_unlock_port_inter(port);
		return B_OK;
	}
	else if(port->portInfo->closed)
	{
		b_unlock_port_inter(port);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		b_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		b_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						b_acquire_sem(port->writerSem) :
						b_acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		b_lock_port_inter(port);

		if(status != B_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->queue_count > 0)
		{
			char* buffer = (char*)(port->queueBuffer);
			size_t msgLen = 0;
			memcpy(code, buffer, sizeof(b_int32)); buffer += sizeof(b_int32);
			memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
			if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
			if(port->portInfo->queue_count > 1)
			{
				buffer = (char*)(port->queueBuffer);
				memmove(buffer, buffer + BHAPI_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * BHAPI_PORT_PER_MESSAGE_LENGTH);
			}

			port->portInfo->queue_count--;

			b_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

			retval = B_OK;
			break;
		}
		else if(port->portInfo->closed)
		{
			retval = B_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= B_INT64_CONSTANT(1);

	b_unlock_port_inter(port);

	return retval;
}


EXPORT_BHAPI b_status_t b_write_port(void *data, b_int32 code, const void *buf, size_t buf_size)
{
	return b_write_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}

EXPORT_BHAPI b_size_t b_port_buffer_size(void *data)
{
	return b_port_buffer_size_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


EXPORT_BHAPI b_status_t b_read_port(void *data, b_int32 *code, void *buf, size_t buf_size)
{
	return b_read_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


EXPORT_BHAPI b_int32 b_port_count(void *data)
{
	b_port_t *port = (b_port_t*)data;
	if(!port) return B_BAD_VALUE;

	b_lock_port_inter(port);
	b_int32 retval = port->portInfo->queue_count;
	b_unlock_port_inter(port);

	return retval;
}

