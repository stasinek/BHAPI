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

#include <stdlib.h>

#include "./../kernel/Kernel.h"
#include "./../support/StringMe.h"
#include "./../support/SimpleLocker.h"

typedef struct bhapi_port_info {
	bhapi_port_info()
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
} bhapi_port_info;

typedef struct bhapi_port_t {
	bhapi_port_t()
		: iLocker(NULL), readerSem(NULL), writerSem(NULL), mapping(NULL), queueBuffer(NULL),
		  openedIPC(false), portInfo(NULL), created(false), refCount(0)
	{
	}

	~bhapi_port_t()
	{
		if(created)
		{
			created = false;
			bhapi_delete_port((void*)this);
		}
	}

	void*			iLocker;
	void*			readerSem;
	void*			writerSem;

	// for IPC (name != NULL)
	void*			mapping;

	void*			queueBuffer;

	bool			openedIPC;

	bhapi_port_info*		portInfo;

	bool			created;
	b_uint32			refCount;
} bhapi_port_t;

class bhapi_port_locker_t {
public:
	void *fSem;
	BSimpleLocker fLocker;

	bhapi_port_locker_t()
		: fSem(NULL)
	{
	}

	~bhapi_port_locker_t()
	{
		if(fSem != NULL)
		{
			// leave global semaphore, without "bhapi_delete_sem(fSem)"
			bhapi_delete_sem_etc(fSem, false);
		}
	}

	void Init()
	{
		if(fSem != NULL) return;

		if((fSem = bhapi_clone_sem("_port_global_")) == NULL)
			fSem = bhapi_create_sem(1, "_port_global_", BHAPI_AREA_ACCESS_ALL);
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
		bhapi_acquire_sem(fSem);
	}

	void UnlockIPC()
	{
		bhapi_release_sem(fSem);
	}
};

static bhapi_port_locker_t __bhapi_port_locker__;
#define _BHAPI_LOCK_IPC_PORT_()		__bhapi_port_locker__.LockIPC()
#define _BHAPI_UNLOCK_IPC_PORT_()		__bhapi_port_locker__.UnlockIPC()
#define _BHAPI_LOCK_LOCAL_PORT_()		__bhapi_port_locker__.LockLocal()
#define _BHAPI_UNLOCK_LOCAL_PORT_()	__bhapi_port_locker__.UnlockLocal()


static bool bhapi_is_port_for_IPC(const bhapi_port_t *port)
{
	if(!port) return false;
	return(port->mapping != NULL);
}


static void bhapi_lock_port_inter(bhapi_port_t *port)
{
	if(bhapi_is_port_for_IPC(port))
		bhapi_acquire_sem(port->iLocker);
	else
		bhapi_lock_locker(port->iLocker);
}


static void bhapi_unlock_port_inter(bhapi_port_t *port)
{
	if(bhapi_is_port_for_IPC(port))
		bhapi_release_sem(port->iLocker);
	else
		bhapi_unlock_locker(port->iLocker);
}

#define BHAPI_PORT_PER_MESSAGE_LENGTH	(sizeof(b_int32) + sizeof(size_t) + BHAPI_MAX_PORT_BUFFER_SIZE)

static void* bhapi_create_port_for_IPC(b_int32 queue_length, const char *name, bhapi_area_access area_access)
{
	if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH ||
	   name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	bhapi_port_t *port = new bhapi_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_BHAPI_LOCK_IPC_PORT_();
	if((port->mapping = bhapi_create_area(name, (void**)&(port->portInfo),
					    sizeof(bhapi_port_info) + (size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH,
					    B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN, area_access)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	bhapi_port_info *port_info = port->portInfo;
	port_info->InitData();
	memcpy(port_info->name, name, (size_t)strlen(name));
	port_info->queue_length = queue_length;

	if((port->iLocker = bhapi_create_sem(1, name, area_access)) == NULL)
	{
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = bhapi_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		bhapi_delete_sem(port->iLocker);
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = bhapi_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		bhapi_delete_sem(port->readerSem);
		bhapi_delete_sem(port->iLocker);
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_BHAPI_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(bhapi_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = false;
	port->created = true;

	return (void*)port;
}


_IMPEXP_BHAPI void* bhapi_open_port(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = b_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	bhapi_port_t *port = new bhapi_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_BHAPI_LOCK_IPC_PORT_();
	if((port->mapping = bhapi_clone_area(name, (void**)&(port->portInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	if((port->iLocker = bhapi_clone_sem(name)) == NULL)
	{
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = bhapi_clone_sem(tmpSemName)) == NULL)
	{
		bhapi_delete_sem(port->iLocker);
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = bhapi_clone_sem(tmpSemName)) == NULL)
	{
		bhapi_delete_sem(port->readerSem);
		bhapi_delete_sem(port->iLocker);
		bhapi_delete_area(port->mapping);
		_BHAPI_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_BHAPI_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(bhapi_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = true;
	port->created = true;

	return (void*)port;
}


_IMPEXP_BHAPI void* bhapi_open_port_by_source(void *data)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port || !port->portInfo) return NULL;

	if(bhapi_is_port_for_IPC(port)) return bhapi_open_port(port->portInfo->name);

	_BHAPI_LOCK_LOCAL_PORT_();
	if(port->refCount == B_MAXUINT32 || port->refCount == 0 || port->portInfo->closed)
	{
		_BHAPI_UNLOCK_LOCAL_PORT_();
		return NULL;
	}
	port->refCount += 1;
	_BHAPI_UNLOCK_LOCAL_PORT_();

	return data;
}


static void* bhapi_create_port_for_local(b_int32 queue_length)
{
	if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH) return NULL;

	bhapi_port_t *port = new bhapi_port_t();
	if(!port) return NULL;

	if((port->iLocker = bhapi_create_locker()) == NULL)
	{
		delete port;
		return NULL;
	}
	if((port->readerSem = bhapi_create_sem(0, NULL)) == NULL)
	{
		bhapi_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}
	if((port->writerSem = bhapi_create_sem(0, NULL)) == NULL)
	{
		bhapi_delete_sem(port->readerSem);
		bhapi_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->portInfo = new bhapi_port_info()) == NULL)
	{
		bhapi_delete_sem(port->writerSem);
		bhapi_delete_sem(port->readerSem);
		bhapi_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->queueBuffer = malloc((size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH)) == NULL)
	{
		delete port->portInfo;
		bhapi_delete_sem(port->writerSem);
		bhapi_delete_sem(port->readerSem);
		bhapi_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	port->portInfo->queue_length = queue_length;

	port->refCount = 1;
	port->created = true;

	return (void*)port;
}


_IMPEXP_BHAPI void* bhapi_create_port(b_int32 queue_length, const char *name, bhapi_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			bhapi_create_port_for_local(queue_length) :
			bhapi_create_port_for_IPC(queue_length, name, area_access));
}


_IMPEXP_BHAPI b_status_t bhapi_delete_port(void *data)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if(bhapi_is_port_for_IPC(port))
	{
		bhapi_delete_area(port->mapping);
		bhapi_delete_sem(port->iLocker);
	}
	else
	{
		_BHAPI_LOCK_LOCAL_PORT_();
		if(port->refCount == 0)
		{
			_BHAPI_UNLOCK_LOCAL_PORT_();
			return B_ERROR;
		}
		b_uint32 count = --(port->refCount);
		_BHAPI_UNLOCK_LOCAL_PORT_();

		if(count > 0) return B_OK;

		free(port->queueBuffer);
		delete port->portInfo;
		bhapi_delete_locker(port->iLocker);
	}

	bhapi_delete_sem(port->writerSem);
	bhapi_delete_sem(port->readerSem);

	if(port->created)
	{
		port->created = false;
		delete port;
	}

	return B_OK;
}


_IMPEXP_BHAPI b_status_t bhapi_close_port(void *data)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

	bhapi_lock_port_inter(port);
	if(port->portInfo->closed)
	{
		bhapi_unlock_port_inter(port);
		return B_ERROR;
	}
	port->portInfo->closed = true;
	bhapi_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);
	bhapi_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);
	bhapi_unlock_port_inter(port);

	return B_OK;
}


_IMPEXP_BHAPI b_status_t bhapi_write_port_etc(void *data, b_int32 code, const void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if((!buf && buf_size > 0) || buf_size > BHAPI_MAX_PORT_BUFFER_SIZE || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	b_bigtime_t currentTime = bhapi_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi_lock_port_inter(port);

	if(port->portInfo->closed)
	{
		bhapi_unlock_port_inter(port);
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

		bhapi_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

		bhapi_unlock_port_inter(port);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->writerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		bhapi_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						bhapi_acquire_sem(port->readerSem) :
						bhapi_acquire_sem_etc(port->readerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		bhapi_lock_port_inter(port);

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
			bhapi_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

			retval = B_OK;
			break;
		}
	}

	port->portInfo->writerWaitCount -= B_INT64_CONSTANT(1);

	bhapi_unlock_port_inter(port);

	return retval;
}


_IMPEXP_BHAPI ssize_t bhapi_port_buffer_size_etc(void *data, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

    if(microseconds_timeout < B_INT64_CONSTANT(0)) return (ssize_t)B_BAD_VALUE;

	b_bigtime_t currentTime = bhapi_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi_lock_port_inter(port);

	if(port->portInfo->queue_count > 0)
	{
		const char* buffer = (const char*)(port->queueBuffer);
		size_t msgLen = 0;

		buffer += sizeof(b_int32);
		memcpy(&msgLen, buffer, sizeof(size_t));

		bhapi_unlock_port_inter(port);
		return (ssize_t)msgLen;
	}
	else if(port->portInfo->closed)
	{
		bhapi_unlock_port_inter(port);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		bhapi_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						bhapi_acquire_sem(port->writerSem) :
						bhapi_acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		bhapi_lock_port_inter(port);

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

	bhapi_unlock_port_inter(port);

	return (ssize_t)retval;
}


_IMPEXP_BHAPI b_status_t bhapi_read_port_etc(void *data, b_int32 *code, void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

	if(!code || (!buf && buf_size > 0) || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	b_bigtime_t currentTime = bhapi_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi_lock_port_inter(port);

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

		bhapi_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

		bhapi_unlock_port_inter(port);
		return B_OK;
	}
	else if(port->portInfo->closed)
	{
		bhapi_unlock_port_inter(port);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi_unlock_port_inter(port);
		return B_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

	b_status_t retval = B_ERROR;

	while(true)
	{
		bhapi_unlock_port_inter(port);
		b_status_t status = (wait_forever ?
						bhapi_acquire_sem(port->writerSem) :
						bhapi_acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
		bhapi_lock_port_inter(port);

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

			bhapi_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

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

	bhapi_unlock_port_inter(port);

	return retval;
}


_IMPEXP_BHAPI b_status_t bhapi_write_port(void *data, b_int32 code, const void *buf, size_t buf_size)
{
	return bhapi_write_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


_IMPEXP_BHAPI ssize_t bhapi_port_buffer_size(void *data)
{
	return bhapi_port_buffer_size_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


_IMPEXP_BHAPI b_status_t bhapi_read_port(void *data, b_int32 *code, void *buf, size_t buf_size)
{
	return bhapi_read_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


_IMPEXP_BHAPI b_int32 bhapi_port_count(void *data)
{
	bhapi_port_t *port = (bhapi_port_t*)data;
	if(!port) return B_BAD_VALUE;

	bhapi_lock_port_inter(port);
	b_int32 retval = port->portInfo->queue_count;
	bhapi_unlock_port_inter(port);

	return retval;
}

