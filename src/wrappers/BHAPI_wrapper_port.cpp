/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: BHAPI_wrapper_port.cpp
 *
 * --------------------------------------------------------------------------*/

#include <os/kernel.h>
#include <os/debug.h>
#include <kits/support/String.h>
#include <kits/debug/Errors.h>
#include <kits/support/SimpleLocker.h>

namespace bhapi {
/*typedef struct port_info {
    port_info()
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
    int32_t			queue_length;
    int32_t			queue_count;
    int64			readerWaitCount;
    int64			writerWaitCount;
    bool			closed;
} port_info;
*/
typedef struct port_t {
    port_t()
        : iLocker(NULL), readerSem(NULL), writerSem(NULL), mapping(NULL), queueBuffer(NULL),
          openedIPC(false), portInfo(NULL), created(false), refCount(0)
    {
    }

    ~port_t()
    {
        if(created)
        {
            created = false;
            bhapi::delete_port((void*)this);
        }
    }

    void*			iLocker;
    void*			readerSem;
    void*			writerSem;

    // for IPC (name != NULL)
    void*			mapping;

    void*			queueBuffer;

    bool			openedIPC;

    bhapi::port_info*		portInfo;

    bool			created;
    uint32_t			refCount;
} port_t;

class port_locker_t {
public:
    void *fSem;
    BSimpleLocker fLocker;

    port_locker_t()
        : fSem(NULL)
    {
    }

    ~port_locker_t()
    {
        if(fSem != NULL)
        {
            // leave global semaphore, without "bhapi::delete_sem(fSem)"
            bhapi::delete_sem_etc(fSem, false);
        }
    }

    void Init()
    {
        if(fSem != NULL) return;

        if((fSem = bhapi::clone_sem("_port_global_")) == NULL)
            fSem = bhapi::create_sem(1, "_port_global_", BHAPI_AREA_ACCESS_ALL);
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
        bhapi::acquire_sem(fSem);
    }

    void UnlockIPC()
    {
        bhapi::release_sem(fSem);
    }
};

static port_locker_t __bhapi_port_locker__;
#define BHAPI_LOCK_IPC_PORT()		__bhapi_port_locker__.LockIPC()
#define BHAPI_UNLOCK_IPC_PORT()		__bhapi_port_locker__.UnlockIPC()
#define BHAPI_LOCKBHAPI_LOCAL_PORT()		__bhapi_port_locker__.LockLocal()
#define BHAPI_UNLOCKBHAPI_LOCAL_PORT()	__bhapi_port_locker__.UnlockLocal()

static bool is_port_for_IPC(const bhapi::port_t *port);
static void lock_port_inter(bhapi::port_t *port);
static void unlock_port_inter(bhapi::port_t *port);
static void* create_port_for_IPC(int32_t queue_length, const char *name, bhapi::area_access area_access);
static void* create_port_for_local(int32_t queue_length);
BHAPI_EXPORT void* open_port(const char *name);
} /* namespace */


static bool bhapi::is_port_for_IPC(const bhapi::port_t *port)
{
    if(!port) return false;
    return(port->mapping != NULL);
}
//-------------------------------------------------------------------------------------------------

static void bhapi::lock_port_inter(bhapi::port_t *port)
{
    if(bhapi::is_port_for_IPC(port))
        bhapi::acquire_sem(port->iLocker);
    else
        bhapi::lock_locker(port->iLocker);
}
//-------------------------------------------------------------------------------------------------

static void bhapi::unlock_port_inter(bhapi::port_t *port)
{
    if(bhapi::is_port_for_IPC(port))
        bhapi::release_sem(port->iLocker);
    else
        bhapi::unlock_locker(port->iLocker);
}
//-------------------------------------------------------------------------------------------------

#define BHAPI_PORT_PER_MESSAGE_LENGTH	(sizeof(int32_t) + sizeof(size_t) + BHAPI_MAX_PORT_BUFFER_SIZE)

//-------------------------------------------------------------------------------------------------

static void* bhapi::create_port_for_IPC(int32_t queue_length, const char *name, bhapi::area_access area_access)
{
    if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH ||
       name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

    char *tmpSemName = bhapi::strdup_printf("%s ", name);
    if(!tmpSemName) return NULL;

    bhapi::port_t *port = new bhapi::port_t();
    if(!port)
    {
        free(tmpSemName);
        return NULL;
    }

    BHAPI_LOCK_IPC_PORT();
    if((port->mapping = bhapi::create_area(name, (void**)&(port->portInfo),
                        sizeof(bhapi::port_info) + (size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH,
                        B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN, area_access)) == NULL ||
       port->portInfo == NULL)
    {
        if(port->mapping) bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }

    bhapi::port_info *port_info = port->portInfo;
    port_info->InitData();
    memcpy(port_info->name, name, (size_t)strlen(name));
    port_info->queue_length = queue_length;

    if((port->iLocker = bhapi::create_sem(1, name, area_access)) == NULL)
    {
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    tmpSemName[strlen(tmpSemName) - 1] = 'r';
    if((port->readerSem = bhapi::create_sem(0, tmpSemName, area_access)) == NULL)
    {
        bhapi::delete_sem(port->iLocker);
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    tmpSemName[strlen(tmpSemName) - 1] = 'w';
    if((port->writerSem = bhapi::create_sem(0, tmpSemName, area_access)) == NULL)
    {
        bhapi::delete_sem(port->readerSem);
        bhapi::delete_sem(port->iLocker);
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    BHAPI_UNLOCK_IPC_PORT();

    free(tmpSemName);

    char *buffer = (char*)(port->portInfo);
    buffer += sizeof(bhapi::port_info);
    port->queueBuffer = (void*)buffer;

    port->openedIPC = false;
    port->created = true;

    return (void*)port;
}


BHAPI_EXPORT void* bhapi::open_port(const char *name)
{
    if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH - 1) return NULL;

    char *tmpSemName = bhapi::strdup_printf("%s ", name);
    if(!tmpSemName) return NULL;

    bhapi::port_t *port = new bhapi::port_t();
    if(!port)
    {
        free(tmpSemName);
        return NULL;
    }

    BHAPI_LOCK_IPC_PORT();
    if((port->mapping = bhapi::clone_area(name, (void**)&(port->portInfo),
                      B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_PORT_DOMAIN)) == NULL ||
       port->portInfo == NULL)
    {
        if(port->mapping) bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }

    if((port->iLocker = bhapi::clone_sem(name)) == NULL)
    {
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    tmpSemName[strlen(tmpSemName) - 1] = 'r';
    if((port->readerSem = bhapi::clone_sem(tmpSemName)) == NULL)
    {
        bhapi::delete_sem(port->iLocker);
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    tmpSemName[strlen(tmpSemName) - 1] = 'w';
    if((port->writerSem = bhapi::clone_sem(tmpSemName)) == NULL)
    {
        bhapi::delete_sem(port->readerSem);
        bhapi::delete_sem(port->iLocker);
        bhapi::delete_area(port->mapping);
        BHAPI_UNLOCK_IPC_PORT();
        delete port;
        free(tmpSemName);
        return NULL;
    }
    BHAPI_UNLOCK_IPC_PORT();

    free(tmpSemName);

    char *buffer = (char*)(port->portInfo);
    buffer += sizeof(bhapi::port_info);
    port->queueBuffer = (void*)buffer;

    port->openedIPC = true;
    port->created = true;

    return (void*)port;
}


BHAPI_EXPORT void* bhapi::open_port_by_source(void *data)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port || !port->portInfo) return NULL;

    if(bhapi::is_port_for_IPC(port)) return bhapi::open_port(port->portInfo->name);

    BHAPI_LOCKBHAPI_LOCAL_PORT();
    if(port->refCount == B_MAXUINT32 || port->refCount == 0 || port->portInfo->closed)
    {
        BHAPI_UNLOCKBHAPI_LOCAL_PORT();
        return NULL;
    }
    port->refCount += 1;
    BHAPI_UNLOCKBHAPI_LOCAL_PORT();

    return data;
}


static void* bhapi::create_port_for_local(int32_t queue_length)
{
    if(queue_length <= 0 || queue_length > BHAPI_VALID_MAX_PORT_QUEUE_LENGTH) return NULL;

    bhapi::port_t *port = new bhapi::port_t();
    if(!port) return NULL;

    if((port->iLocker = bhapi::create_locker()) == NULL)
    {
        delete port;
        return NULL;
    }
    if((port->readerSem = bhapi::create_sem(0, NULL)) == NULL)
    {
        bhapi::delete_locker(port->iLocker);
        delete port;
        return NULL;
    }
    if((port->writerSem = bhapi::create_sem(0, NULL)) == NULL)
    {
        bhapi::delete_sem(port->readerSem);
        bhapi::delete_locker(port->iLocker);
        delete port;
        return NULL;
    }

    if((port->portInfo = new bhapi::port_info()) == NULL)
    {
        bhapi::delete_sem(port->writerSem);
        bhapi::delete_sem(port->readerSem);
        bhapi::delete_locker(port->iLocker);
        delete port;
        return NULL;
    }

    if((port->queueBuffer = malloc((size_t)queue_length * BHAPI_PORT_PER_MESSAGE_LENGTH)) == NULL)
    {
        delete port->portInfo;
        bhapi::delete_sem(port->writerSem);
        bhapi::delete_sem(port->readerSem);
        bhapi::delete_locker(port->iLocker);
        delete port;
        return NULL;
    }

    port->portInfo->queue_length = queue_length;

    port->refCount = 1;
    port->created = true;

    return (void*)port;
}


BHAPI_EXPORT void* bhapi::create_port(int32_t queue_length, const char *name, bhapi::area_access area_access)
{
    return((name == NULL || *name == 0) ?
            bhapi::create_port_for_local(queue_length) :
            bhapi::create_port_for_IPC(queue_length, name, area_access));
}


BHAPI_EXPORT status_t bhapi::delete_port(void *data)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    if(bhapi::is_port_for_IPC(port))
    {
        bhapi::delete_area(port->mapping);
        bhapi::delete_sem(port->iLocker);
    }
    else
    {
        BHAPI_LOCKBHAPI_LOCAL_PORT();
        if(port->refCount == 0)
        {
            BHAPI_UNLOCKBHAPI_LOCAL_PORT();
            return B_ERROR;
        }
        uint32_t count = --(port->refCount);
        BHAPI_UNLOCKBHAPI_LOCAL_PORT();

        if(count > 0) return B_OK;

        free(port->queueBuffer);
        delete port->portInfo;
        bhapi::delete_locker(port->iLocker);
    }

    bhapi::delete_sem(port->writerSem);
    bhapi::delete_sem(port->readerSem);

    if(port->created)
    {
        port->created = false;
        delete port;
    }

    return B_OK;
}


BHAPI_EXPORT status_t bhapi::close_port(void *data)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    bhapi::lock_port_inter(port);
    if(port->portInfo->closed)
    {
        bhapi::unlock_port_inter(port);
        return B_ERROR;
    }
    port->portInfo->closed = true;
    bhapi::release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);
    bhapi::release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);
    bhapi::unlock_port_inter(port);

    return B_OK;
}


BHAPI_EXPORT status_t bhapi::write_port_etc(void *data,  int32_t code, const void *buf, size_t buf_size,  uint32_t flags, bigtime_t microseconds_timeout)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    if((!buf && buf_size > 0) || buf_size > BHAPI_MAX_PORT_BUFFER_SIZE || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

    bigtime_t currentTime = b_real_time_clock_usecs();
    bool wait_forever = false;

    if(flags != B_ABSOLUTE_TIMEOUT)
    {
        if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
            wait_forever = true;
        else
            microseconds_timeout += currentTime;
    }

    bhapi::lock_port_inter(port);

    if(port->portInfo->closed)
    {
        bhapi::unlock_port_inter(port);
        return B_ERROR;
    }
    else if(port->portInfo->queue_count < port->portInfo->queue_length)
    {
        size_t offset = (size_t)port->portInfo->queue_count * BHAPI_PORT_PER_MESSAGE_LENGTH;
        char* buffer = (char*)(port->queueBuffer);
        buffer += offset;
        memcpy(buffer, &code, sizeof(int32_t)); buffer += sizeof(int32_t);
        memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
        if(buf_size > 0) memcpy(buffer, buf, buf_size);

        port->portInfo->queue_count++;

        bhapi::release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

        bhapi::unlock_port_inter(port);
        return B_OK;
    }
    else if(microseconds_timeout == currentTime && !wait_forever)
    {
        bhapi::unlock_port_inter(port);
        return B_WOULD_BLOCK;
    }

    port->portInfo->writerWaitCount += B_INT64_CONSTANT(1);

    status_t retval = B_ERROR;

    while(true)
    {
        bhapi::unlock_port_inter(port);
        status_t status = (wait_forever ?
                        bhapi::acquire_sem(port->readerSem) :
                        bhapi::acquire_sem_etc(port->readerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
        bhapi::lock_port_inter(port);

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
            memcpy(buffer, &code, sizeof(int32_t)); buffer += sizeof(int32_t);
            memcpy(buffer, &buf_size, sizeof(size_t)); buffer += sizeof(size_t);
            if(buf_size > 0) memcpy(buffer, buf, buf_size);

            port->portInfo->queue_count++;
            bhapi::release_sem_etc(port->writerSem, port->portInfo->readerWaitCount, 0);

            retval = B_OK;
            break;
        }
    }

    port->portInfo->writerWaitCount -= B_INT64_CONSTANT(1);

    bhapi::unlock_port_inter(port);

    return retval;
}


BHAPI_EXPORT  ssize_t bhapi::port_buffer_size_etc(void *data,  uint32_t flags, bigtime_t microseconds_timeout)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    if(microseconds_timeout < B_INT64_CONSTANT(0)) return (ssize_t)B_BAD_VALUE;

    bigtime_t currentTime = b_real_time_clock_usecs();
    bool wait_forever = false;

    if(flags != B_ABSOLUTE_TIMEOUT)
    {
        if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
            wait_forever = true;
        else
            microseconds_timeout += currentTime;
    }

    bhapi::lock_port_inter(port);

    if(port->portInfo->queue_count > 0)
    {
        const char* buffer = (const char*)(port->queueBuffer);
        size_t msgLen = 0;

        buffer += sizeof(int32_t);
        memcpy(&msgLen, buffer, sizeof(size_t));

        bhapi::unlock_port_inter(port);
        return (ssize_t)msgLen;
    }
    else if(port->portInfo->closed)
    {
        bhapi::unlock_port_inter(port);
        return B_ERROR;
    }
    else if(microseconds_timeout == currentTime && !wait_forever)
    {
        bhapi::unlock_port_inter(port);
        return B_WOULD_BLOCK;
    }

    port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

    status_t retval = B_ERROR;

    while(true)
    {
        bhapi::unlock_port_inter(port);
        status_t status = (wait_forever ?
                        bhapi::acquire_sem(port->writerSem) :
                        bhapi::acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
        bhapi::lock_port_inter(port);

        if(status != B_OK)
        {
            retval = status;
            break;
        }

        if(port->portInfo->queue_count > 0)
        {
            const char* buffer = (const char*)(port->queueBuffer);
            size_t msgLen = 0;

            buffer += sizeof(int32_t);
            memcpy(&msgLen, buffer, sizeof(size_t));

            retval = (status_t)msgLen;
            break;
        }
        else if(port->portInfo->closed)
        {
            retval = B_ERROR;
            break;
        }
    }

    port->portInfo->readerWaitCount -= B_INT64_CONSTANT(1);

    bhapi::unlock_port_inter(port);

    return (ssize_t)retval;
}


BHAPI_EXPORT status_t bhapi::read_port_etc(void *data,  int32_t *code, void *buf, size_t buf_size,  uint32_t flags, bigtime_t microseconds_timeout)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    if(!code || (!buf && buf_size > 0) || microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

    bigtime_t currentTime = b_real_time_clock_usecs();
    bool wait_forever = false;

    if(flags != B_ABSOLUTE_TIMEOUT)
    {
        if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
            wait_forever = true;
        else
            microseconds_timeout += currentTime;
    }

    bhapi::lock_port_inter(port);

    if(port->portInfo->queue_count > 0)
    {
        char* buffer = (char*)(port->queueBuffer);
        size_t msgLen = 0;
        memcpy(code, buffer, sizeof(int32_t)); buffer += sizeof(int32_t);
        memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
        if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
        if(port->portInfo->queue_count > 1)
        {
            buffer = (char*)(port->queueBuffer);
            memmove(buffer, buffer + BHAPI_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * BHAPI_PORT_PER_MESSAGE_LENGTH);
        }

        port->portInfo->queue_count--;

        bhapi::release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

        bhapi::unlock_port_inter(port);
        return B_OK;
    }
    else if(port->portInfo->closed)
    {
        bhapi::unlock_port_inter(port);
        return B_ERROR;
    }
    else if(microseconds_timeout == currentTime && !wait_forever)
    {
        bhapi::unlock_port_inter(port);
        return B_WOULD_BLOCK;
    }

    port->portInfo->readerWaitCount += B_INT64_CONSTANT(1);

    status_t retval = B_ERROR;

    while(true)
    {
        bhapi::unlock_port_inter(port);
        status_t status = (wait_forever ?
                        bhapi::acquire_sem(port->writerSem) :
                        bhapi::acquire_sem_etc(port->writerSem, 1, B_ABSOLUTE_TIMEOUT, microseconds_timeout));
        bhapi::lock_port_inter(port);

        if(status != B_OK)
        {
            retval = status;
            break;
        }

        if(port->portInfo->queue_count > 0)
        {
            char* buffer = (char*)(port->queueBuffer);
            size_t msgLen = 0;
            memcpy(code, buffer, sizeof(int32_t)); buffer += sizeof(int32_t);
            memcpy(&msgLen, buffer, sizeof(size_t)); buffer += sizeof(size_t);
            if(msgLen > 0 && buf_size > 0) memcpy(buf, buffer, min_c(msgLen, buf_size));
            if(port->portInfo->queue_count > 1)
            {
                buffer = (char*)(port->queueBuffer);
                memmove(buffer, buffer + BHAPI_PORT_PER_MESSAGE_LENGTH, (size_t)(port->portInfo->queue_count - 1) * BHAPI_PORT_PER_MESSAGE_LENGTH);
            }

            port->portInfo->queue_count--;

            bhapi::release_sem_etc(port->readerSem, port->portInfo->writerWaitCount, 0);

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

    bhapi::unlock_port_inter(port);

    return retval;
}


BHAPI_EXPORT status_t bhapi::write_port(void *data,  int32_t code, const void *buf, size_t buf_size)
{
    return bhapi::write_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}

BHAPI_EXPORT  ssize_t bhapi::port_buffer_size(void *data)
{
    return bhapi::port_buffer_size_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_EXPORT status_t bhapi::read_port(void *data,  int32_t *code, void *buf, size_t buf_size)
{
    return bhapi::read_port_etc(data, code, buf, buf_size, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_EXPORT  int32_t bhapi::port_count(void *data)
{
    bhapi::port_t *port = (bhapi::port_t*)data;
    if(!port) return B_BAD_VALUE;

    bhapi::lock_port_inter(port);
    int32_t retval = port->portInfo->queue_count;
    bhapi::unlock_port_inter(port);

    return retval;
}

