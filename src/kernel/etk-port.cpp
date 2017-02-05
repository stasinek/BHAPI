/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#include "./../kernel/Kernel.h>
#include "./../support/String.h>
#include "./../support/SimpleLocker.h>

typedef struct etk_port_info {
	etk_port_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, E_OS_NAME_LENGTH + 1);
		queue_length = 0;
		queue_count = 0;
		readerWaitCount = E_INT64_CONSTANT(0);
		writerWaitCount = E_INT64_CONSTANT(0);
		closed = false;
	}

	char			name[E_OS_NAME_LENGTH + 1];
	eint32			queue_length;
	eint32			queue_count;
	eint64			readerWaitCount;
	eint64			writerWaitCount;
	bool			closed;
} etk_port_info;

typedef struct etk_port_t {
	etk_port_t()
		: iLocker(NULL), readerSem(NULL), writerSem(NULL), mapping(NULL), queueBuffer(NULL),
		  openedIPC(false), portInfo(NULL), created(false), refCount(0)
	{
	}

	~etk_port_t()
	{
		if(created)
		{
			created = false;
			etk_delete_port((void*)this);
		}
	}

	void*			iLocker;
	void*			readerSem;
	void*			writerSem;

	// for IPC (name != NULL)
	void*			mapping;

	void*			queueBuffer;

	bool			openedIPC;

	etk_port_info*		portInfo;

	bool			created;
	euint32			refCount;
} etk_port_t;

class etk_port_locker_t {
public:
	void *fSem;
	ESimpleLocker fLocker;

	etk_port_locker_t()
		: fSem(NULL)
	{
	}

	~etk_port_locker_t()
	{
		if(fSem != NULL)
		{
			// leave global semaphore, without "etk_delete_sem(fSem)"
			etk_delete_sem_etc(fSem, false);
		}
	}

	void Init()
	{
		if(fSem != NULL) return;

		if((fSem = etk_clone_sem("_port_global_")) == NULL)
			fSem = etk_create_sem(1, "_port_global_", ETK_AREA_ACCESS_ALL);
		if(fSem == NULL) ETK_ERROR("[KERNEL]: Can't initialize global port!");
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
		etk_acquire_sem(fSem);
	}

	void UnlockIPC()
	{
		etk_release_sem(fSem);
	}
};

static etk_port_locker_t __etk_port_locker__;
#define _ETK_LOCK_IPC_PORT_()		__etk_port_locker__.LockIPC()
#define _ETK_UNLOCK_IPC_PORT_()		__etk_port_locker__.UnlockIPC()
#define _ETK_LOCK_LOCAL_PORT_()		__etk_port_locker__.LockLocal()
#define _ETK_UNLOCK_LOCAL_PORT_()	__etk_port_locker__.UnlockLocal()


static bool etk_is_port_for_IPC(const etk_port_t *port)
{
	if(!port) return false;
	return(port->mapping != NULL);
}


static void etk_lock_port_inter(etk_port_t *port)
{
	if(etk_is_port_for_IPC(port))
		etk_acquire_sem(port->iLocker);
	else
		etk_lock_locker(port->iLocker);
}


static void etk_unlock_port_inter(etk_port_t *port)
{
	if(etk_is_port_for_IPC(port))
		etk_release_sem(port->iLocker);
	else
		etk_unlock_locker(port->iLocker);
}

#define ETK_PORT_PER_MESSAGE_LENGTH	(sizeof(eint32) + sizeof(size_t) + ETK_MAX_PORT_BUFFER_SIZE)

static void* etk_create_port_for_IPC(eint32 queue_length, const char *name, etk_area_access area_access)
{
	if(queue_length <= 0 || queue_length > ETK_VALID_MAX_PORT_QUEUE_LENGTH ||
	   name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = e_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	etk_port_t *port = new etk_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_ETK_LOCK_IPC_PORT_();
	if((port->mapping = etk_create_area(name, (void**)&(port->portInfo),
					    sizeof(etk_port_info) + (size_t)queue_length * ETK_PORT_PER_MESSAGE_LENGTH,
					    E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_PORT_DOMAIN, area_access)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	etk_port_info *port_info = port->portInfo;
	port_info->InitData();
	memcpy(port_info->name, name, (size_t)strlen(name));
	port_info->queue_length = queue_length;

	if((port->iLocker = etk_create_sem(1, name, area_access)) == NULL)
	{
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = etk_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		etk_delete_sem(port->iLocker);
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = etk_create_sem(0, tmpSemName, area_access)) == NULL)
	{
		etk_delete_sem(port->readerSem);
		etk_delete_sem(port->iLocker);
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_ETK_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(etk_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = false;
	port->created = true;

	return (void*)port;
}


_IMPEXP_ETK void* etk_open_port(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH - 1) return NULL;

	char *tmpSemName = e_strdup_printf("%s ", name);
	if(!tmpSemName) return NULL;

	etk_port_t *port = new etk_port_t();
	if(!port)
	{
		free(tmpSemName);
		return NULL;
	}

	_ETK_LOCK_IPC_PORT_();
	if((port->mapping = etk_clone_area(name, (void**)&(port->portInfo),
					  E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_PORT_DOMAIN)) == NULL ||
	   port->portInfo == NULL)
	{
		if(port->mapping) etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}

	if((port->iLocker = etk_clone_sem(name)) == NULL)
	{
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'r';
	if((port->readerSem = etk_clone_sem(tmpSemName)) == NULL)
	{
		etk_delete_sem(port->iLocker);
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	tmpSemName[strlen(tmpSemName) - 1] = 'w';
	if((port->writerSem = etk_clone_sem(tmpSemName)) == NULL)
	{
		etk_delete_sem(port->readerSem);
		etk_delete_sem(port->iLocker);
		etk_delete_area(port->mapping);
		_ETK_UNLOCK_IPC_PORT_();
		delete port;
		free(tmpSemName);
		return NULL;
	}
	_ETK_UNLOCK_IPC_PORT_();

	free(tmpSemName);

	char *buffer = (char*)(port->portInfo);
	buffer += sizeof(etk_port_info);
	port->queueBuffer = (void*)buffer;

	port->openedIPC = true;
	port->created = true;

	return (void*)port;
}


_IMPEXP_ETK void* etk_open_port_by_source(void *data)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port || !port->portInfo) return NULL;

	if(etk_is_port_for_IPC(port)) return etk_open_port(port->portInfo->name);

	_ETK_LOCK_LOCAL_PORT_();
	if(port->refCount == E_MAXUINT32 || port->refCount == 0 || port->portInfo->closed)
	{
		_ETK_UNLOCK_LOCAL_PORT_();
		return NULL;
	}
	port->refCount += 1;
	_ETK_UNLOCK_LOCAL_PORT_();

	return data;
}


static void* etk_create_port_for_local(eint32 queue_length)
{
	if(queue_length <= 0 || queue_length > ETK_VALID_MAX_PORT_QUEUE_LENGTH) return NULL;

	etk_port_t *port = new etk_port_t();
	if(!port) return NULL;

	if((port->iLocker = etk_create_locker()) == NULL)
	{
		delete port;
		return NULL;
	}
	if((port->readerSem = etk_create_sem(0, NULL)) == NULL)
	{
		etk_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}
	if((port->writerSem = etk_create_sem(0, NULL)) == NULL)
	{
		etk_delete_sem(port->readerSem);
		etk_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->portInfo = new etk_port_info()) == NULL)
	{
		etk_delete_sem(port->writerSem);
		etk_delete_sem(port->readerSem);
		etk_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	if((port->queueBuffer = malloc((size_t)queue_length * ETK_PORT_PER_MESSAGE_LENGTH)) == NULL)
	{
		delete port->portInfo;
		etk_delete_sem(port->writerSem);
		etk_delete_sem(port->readerSem);
		etk_delete_locker(port->iLocker);
		delete port;
		return NULL;
	}

	port->portInfo->queue_length = queue_length;

	port->refCount = 1;
	port->created = true;

	return (void*)port;
}


_IMPEXP_ETK void* etk_create_port(eint32 queue_length, const char *name, etk_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			etk_create_port_for_local(queue_length) :
			etk_create_port_for_IPC(queue_length, name, area_access));
}


_IMPEXP_ETK e_status_t etk_delete_port(void *data)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	if(etk_is_port_for_IPC(port))
	{
		etk_delete_area(port->mapping);
		etk_delete_sem(port->iLocker);
	}
	else
	{
		_ETK_LOCK_LOCAL_PORT_();
		if(port->refCount == 0)
		{
			_ETK_UNLOCK_LOCAL_PORT_();
			return E_ERROR;
		}
		euint32 count = --(port->refCount);
		_ETK_UNLOCK_LOCAL_PORT_();

		if(count > 0) return E_OK;

		free(port->queueBuffer);
		delete port->portInfo;
		etk_delete_locker(port->iLocker);
	}

	etk_delete_sem(port->writerSem);
	etk_delete_sem(port->readerSem);

	if(port->created)
	{
		port->created = false;
		delete port;
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_close_port(void *data)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	etk_lock_port_inter(port);
	if(port->portInfo->closed)
	{
		etk_unlock_port_inter(port);
		return E_ERROR;
	}
	port->portInfo->closed = true;
	etk_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);
	etk_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);
	etk_unlock_port_inter(port);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_write_port_etc(void *data, eint32 code, const void *buf, size_t buf_size, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	if((!buf && buf_size > 0) || buf_size > ETK_MAX_PORT_BUFFER_SIZE || microseconds_timeout < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_port_inter(port);

	if(port->portInfo->closed)
	{
		etk_unlock_port_inter(port);
		return E_ERROR;
	}
	else if(port->portInfo->queue_count < port->portInfo->queue_length)
	{
		size_t offset = (size_t)port->portInfo->queue_count * ETK_PORT_PER_MESSAGE_LENGTH;
		char* buffer = (char*)(port->queueBuffer);
		buffer += offset;
		memcpy(buffer, &code, sizeof(eint32)); buffer += sizeof(eint32);
		memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
		if(buf_size > 0) memcpy(buffer, buf, buf_size);

		port->portInfo->queue_count++;

		etk_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

		etk_unlock_port_inter(port);
		return E_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_port_inter(port);
		return E_WOULD_BLOCK;
	}

	port->portInfo->writerWaitCount += E_INT64_CONSTANT(1);

	e_status_t retval = E_ERROR;

	while(true)
	{
		etk_unlock_port_inter(port);
		e_status_t status = (wait_forever ?
						etk_acquire_sem(port->readerSem) :
						etk_acquire_sem_etc(port->readerSem, 1, E_ABSOLUTE_TIMEOUT, microseconds_timeout));
		etk_lock_port_inter(port);

		if(status != E_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->closed)
		{
			retval = E_ERROR;
			break;
		}
		else if(port->portInfo->queue_count < port->portInfo->queue_length)
		{
			size_t offset = (size_t)port->portInfo->queue_count * ETK_PORT_PER_MESSAGE_LENGTH;
			char* buffer = (char*)(port->queueBuffer);
			buffer += offset;
			memcpy(buffer, &code, sizeof(eint32)); buffer += sizeof(eint32);
			memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
			if(buf_size > 0) memcpy(buffer, buf, buf_size);

			port->portInfo->queue_count++;
			etk_release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

			retval = E_OK;
			break;
		}
	}

	port->portInfo->writerWaitCount -= E_INT64_CONSTANT(1);

	etk_unlock_port_inter(port);

	return retval;
}


_IMPEXP_ETK ssize_t etk_port_buffer_size_etc(void *data, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	if(microseconds_timeout < E_INT64_CONSTANT(0)) return (ssize_t)E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_port_inter(port);

	if(port->portInfo->queue_count > 0)
	{
		const char* buffer = (const char*)(port->queueBuffer);
		size_t msgLen = 0;

		buffer += sizeof(eint32);
		memcpy(&msgLen, buffer, sizeof(size_t));

		etk_unlock_port_inter(port);
		return (ssize_t)msgLen;
	}
	else if(port->portInfo->closed)
	{
		etk_unlock_port_inter(port);
		return E_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_port_inter(port);
		return E_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += E_INT64_CONSTANT(1);

	e_status_t retval = E_ERROR;

	while(true)
	{
		etk_unlock_port_inter(port);
		e_status_t status = (wait_forever ?
						etk_acquire_sem(port->writerSem) :
						etk_acquire_sem_etc(port->writerSem, 1, E_ABSOLUTE_TIMEOUT, microseconds_timeout));
		etk_lock_port_inter(port);

		if(status != E_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->queue_count > 0)
		{
			const char* buffer = (const char*)(port->queueBuffer);
			size_t msgLen = 0;

			buffer += sizeof(eint32);
			memcpy(&msgLen, buffer, sizeof(size_t));

			retval = (e_status_t)msgLen;
			break;
		}
		else if(port->portInfo->closed)
		{
			retval = E_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= E_INT64_CONSTANT(1);

	etk_unlock_port_inter(port);

	return (ssize_t)retval;
}


_IMPEXP_ETK e_status_t etk_read_port_etc(void *data, eint32 *code, void *buf, size_t buf_size, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	if(!code || (!buf && buf_size > 0) || microseconds_timeout < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_port_inter(port);

	if(port->portInfo->queue_count > 0)
	{
		char* buffer = (char*)(port->queueBuffer);
		size_t msgLen = 0;
		memcpy(code, buffer, sizeof(eint32)); buffer += sizeof(eint32);
		memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
		if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
		if(port->portInfo->queue_count > 1)
		{
			buffer = (char*)(port->queueBuffer);
			memmove(buffer, buffer + ETK_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * ETK_PORT_PER_MESSAGE_LENGTH);
		}

		port->portInfo->queue_count--;

		etk_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

		etk_unlock_port_inter(port);
		return E_OK;
	}
	else if(port->portInfo->closed)
	{
		etk_unlock_port_inter(port);
		return E_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_port_inter(port);
		return E_WOULD_BLOCK;
	}

	port->portInfo->readerWaitCount += E_INT64_CONSTANT(1);

	e_status_t retval = E_ERROR;

	while(true)
	{
		etk_unlock_port_inter(port);
		e_status_t status = (wait_forever ?
						etk_acquire_sem(port->writerSem) :
						etk_acquire_sem_etc(port->writerSem, 1, E_ABSOLUTE_TIMEOUT, microseconds_timeout));
		etk_lock_port_inter(port);

		if(status != E_OK)
		{
			retval = status;
			break;
		}

		if(port->portInfo->queue_count > 0)
		{
			char* buffer = (char*)(port->queueBuffer);
			size_t msgLen = 0;
			memcpy(code, buffer, sizeof(eint32)); buffer += sizeof(eint32);
			memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
			if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
			if(port->portInfo->queue_count > 1)
			{
				buffer = (char*)(port->queueBuffer);
				memmove(buffer, buffer + ETK_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * ETK_PORT_PER_MESSAGE_LENGTH);
			}

			port->portInfo->queue_count--;

			etk_release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

			retval = E_OK;
			break;
		}
		else if(port->portInfo->closed)
		{
			retval = E_ERROR;
			break;
		}
	}

	port->portInfo->readerWaitCount -= E_INT64_CONSTANT(1);

	etk_unlock_port_inter(port);

	return retval;
}


_IMPEXP_ETK e_status_t etk_write_port(void *data, eint32 code, const void *buf, size_t buf_size)
{
	return etk_write_port_etc(data, code, buf, buf_size, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK ssize_t etk_port_buffer_size(void *data)
{
	return etk_port_buffer_size_etc(data, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_read_port(void *data, eint32 *code, void *buf, size_t buf_size)
{
	return etk_read_port_etc(data, code, buf, buf_size, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK eint32 etk_port_count(void *data)
{
	etk_port_t *port = (etk_port_t*)data;
	if(!port) return E_BAD_VALUE;

	etk_lock_port_inter(port);
	eint32 retval = port->portInfo->queue_count;
	etk_unlock_port_inter(port);

	return retval;
}

