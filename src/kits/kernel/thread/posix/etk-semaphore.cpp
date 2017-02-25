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
 * File: etk-semaphore.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef BHAPI_OS_LINUX

#include <stdlib.h>
#include <fcntl.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "../config.h"
#include "../BHAPIBuild.h"

#ifdef BHAPI_OS_CYGWIN
	#warning "Posix IPC semaphore won't to be support at all."
	#warning "It's recommended that you use the WIN32-Thread implemented."
#endif

#if !defined(_POSIX_THREAD_PROCESS_SHARED) || _POSIX_THREAD_PROCESS_SHARED == -1
	#undef BHAPI_PTHREAD_SHARED
	#ifndef HAVE_SEM_TIMEDWAIT
		#error "Posix thread DON'T support Process-shared synchronization, nor do support sem_timedwait!"
	#endif
#else
	#define BHAPI_PTHREAD_SHARED
#endif

#include "../kernel/Kernel.h"
#include "../support/StringMe.h"

typedef struct bhapi_posix_sem_info {
	bhapi_posix_sem_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, B_OS_NAME_LENGTH + 1);
		latestHolderTeamId = B_INT64_CONSTANT(0);
		latestHolderThreadId = B_INT64_CONSTANT(0);
		count = B_INT64_CONSTANT(0);
#ifndef BHAPI_PTHREAD_SHARED
		minAcquiringCount = B_INT64_CONSTANT(0);
#endif
		acquiringCount = B_INT64_CONSTANT(0);
		closed = false;
		refCount = 0;
	}

	void SetLatestHolderTeamId(b_int64 id)
	{
		latestHolderTeamId = id;
	}

	void SetLatestHolderThreadId(b_int64 id)
	{
		latestHolderThreadId = id;
	}

	bool LatestHolderTeamIsCurrent(void)
	{
		return(latestHolderTeamId == bhapi_get_current_team_id());
	}

	bool LatestHolderThreadIsCurrent(void)
	{
		return(latestHolderThreadId == bhapi_get_current_thread_id());
	}

	char			name[B_OS_NAME_LENGTH + 1];
	b_int64			latestHolderTeamId;
	b_int64			latestHolderThreadId;
	b_int64			count;
#ifndef BHAPI_PTHREAD_SHARED
	b_int64			minAcquiringCount;
#endif
	b_int64			acquiringCount;
	bool			closed;

#ifdef BHAPI_PTHREAD_SHARED
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
#else
	sem_t			isem;
	sem_t			sem;
#endif
	b_uint32			refCount;
} bhapi_posix_sem_info;


typedef struct bhapi_posix_sem_t {
	bhapi_posix_sem_t()
		: mapping(NULL), semInfo(NULL), iMutex(NULL), iCond(NULL),
#ifndef BHAPI_PTHREAD_SHARED
		  remoteiSem(NULL), remoteSem(NULL),
#endif
		  created(false), no_clone(false)
	{
	}

	~bhapi_posix_sem_t()
	{
		if(created)
		{
			created = false;
			bhapi_delete_sem((void*)this);
		}
	}

	// for IPC (name != NULL)
	void*			mapping;
	bhapi_posix_sem_info*	semInfo;

	// for local
	pthread_mutex_t*	iMutex;
	pthread_cond_t*		iCond;

#ifndef BHAPI_PTHREAD_SHARED
	sem_t*			remoteiSem;
	sem_t*			remoteSem;
#endif

	bool			created;
	bool			no_clone;
} bhapi_posix_sem_t;


// return value must be free by "free()"
static char* bhapi_global_sem_ipc_name()
{
	const char *prefix, *slash;

	if((prefix = getenv("POSIX_SEM_IPC_PREFIX")) == NULL)
	{
#ifdef POSIX_SEM_IPC_PREFIX
		prefix = POSIX_SEM_IPC_PREFIX;
#else
		prefix = "/";
#endif
	}

	slash = (prefix[strlen(prefix) - 1] == '/') ? "" : "/";

	return b_strdup_printf("%s%s%s", prefix, slash, "_bhapi_global_");
}

#ifndef SEM_FAILED
#define SEM_FAILED	(-1)
#endif

class bhapi_posix_sem_locker_t {
public:
	sem_t *fSem;
	pthread_mutex_t fLocker;

	bhapi_posix_sem_locker_t()
		: fSem((sem_t*)SEM_FAILED)
	{
		pthread_mutex_init(&fLocker, NULL);
	}

	~bhapi_posix_sem_locker_t()
	{
		pthread_mutex_destroy(&fLocker);

		if(fSem != (sem_t*)SEM_FAILED)
		{
			sem_close(fSem);
			// leave global semaphore, without sem_unlink
		}
	}

	void Init()
	{
		if(fSem != (sem_t*)SEM_FAILED) return;

		char *semName = bhapi_global_sem_ipc_name();
		if(semName)
		{
			if((fSem = (sem_t*)sem_open(semName, O_CREAT | O_EXCL, 0666, 1)) == (sem_t*)SEM_FAILED)
			{
//				BHAPI_DEBUG("[KERNEL]: Unable to create global semaphore, errno: %d", errno);
				fSem = (sem_t*)sem_open(semName, 0);
			}
			free(semName);
		}
		if(fSem == (sem_t*)SEM_FAILED)
			BHAPI_ERROR("[KERNEL]: Can't initialize global semaphore! errno: %d", errno);
	}

	void LockLocal()
	{
		pthread_mutex_lock(&fLocker);
	}

	void UnlockLocal()
	{
		pthread_mutex_unlock(&fLocker);
	}

	void LockIPC()
	{
		LockLocal();
		Init();
		UnlockLocal();
		sem_wait(fSem);
	}

	void UnlockIPC()
	{
		sem_post(fSem);
	}
};

static bhapi_posix_sem_locker_t __bhapi_semaphore_locker__;

#define _BHAPI_LOCK_IPC_SEMAPHORE_()		__bhapi_semaphore_locker__.LockIPC()
#define _BHAPI_UNLOCK_IPC_SEMAPHORE_()		__bhapi_semaphore_locker__.UnlockIPC()
#define _BHAPI_LOCK_LOCAL_SEMAPHORE_()		__bhapi_semaphore_locker__.LockLocal()
#define _BHAPI_UNLOCK_LOCAL_SEMAPHORE_()		__bhapi_semaphore_locker__.UnlockLocal()


static bool bhapi_is_sem_for_IPC(const bhapi_posix_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void bhapi_lock_sem_inter(bhapi_posix_sem_t *sem)
{
#ifndef BHAPI_PTHREAD_SHARED
	if(bhapi_is_sem_for_IPC(sem))
	{
		sem_wait(sem->remoteiSem);
	}
	else
	{
#endif
		pthread_mutex_lock(sem->iMutex);
#ifndef BHAPI_PTHREAD_SHARED
	}
#endif
}


static void bhapi_unlock_sem_inter(bhapi_posix_sem_t *sem)
{
#ifndef BHAPI_PTHREAD_SHARED
	if(bhapi_is_sem_for_IPC(sem))
	{
		sem_post(sem->remoteiSem);
	}
	else
	{
#endif
		pthread_mutex_unlock(sem->iMutex);
#ifndef BHAPI_PTHREAD_SHARED
	}
#endif
}


static void* bhapi_create_sem_for_IPC(b_int64 count, const char *name, bhapi_area_access area_access)
{
	if(count < B_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	bhapi_posix_sem_t *sem = new bhapi_posix_sem_t();
	if(!sem) return NULL;

	_BHAPI_LOCK_IPC_SEMAPHORE_();

	if((sem->mapping = bhapi_create_area(name, (void**)&(sem->semInfo), sizeof(bhapi_posix_sem_info),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	bhapi_posix_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

#ifndef BHAPI_PTHREAD_SHARED
	if(sem_init(&(sem_info->isem), 1, 1) != 0)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : sem_init failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	if(sem_init(&(sem_info->sem), 1, 0) != 0)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : sem_init failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		sem_destroy(&(sem_info->isem));
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->remoteiSem = &(sem_info->isem);
	sem->remoteSem = &(sem_info->sem);
#else // BHAPI_PTHREAD_SHARED
	b_uint32 successFlags = 0;

	pthread_mutexattr_t mattr;
	pthread_condattr_t cattr;

	if(pthread_mutexattr_init(&mattr) != 0) successFlags |= (1 << 1);
	else if(pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED) != 0) successFlags |= (1 << 2);
	if(pthread_condattr_init(&cattr) != 0) successFlags |= (1 << 3);
	else if(pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED) != 0) successFlags |= (1 << 4);

	if(successFlags != 0)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : init mutex/cond attr failed(%lu) --- \"%s\"", __PRETTY_FUNCTION__, successFlags, name);
		if(!(successFlags & (1 << 1))) pthread_mutexattr_destroy(&mattr);
		if(!(successFlags & (1 << 3))) pthread_condattr_destroy(&cattr);
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	if(pthread_mutex_init(&(sem_info->mutex), &mattr) != 0) successFlags |= (1 << 1);
	if(pthread_cond_init(&(sem_info->cond), &cattr) != 0) successFlags |= (1 << 2);

	pthread_mutexattr_destroy(&mattr);
	pthread_condattr_destroy(&cattr);

	if(successFlags != 0)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : create mutex/cond failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(!(successFlags & (1 << 1))) pthread_mutex_destroy(&(sem_info->mutex));
		if(!(successFlags & (1 << 2))) pthread_cond_destroy(&(sem_info->cond));
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->iMutex = &(sem_info->mutex);
	sem->iCond = &(sem_info->cond);
#endif // BHAPI_PTHREAD_SHARED

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	_BHAPI_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


IMPEXP_BHAPI void* bhapi_clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	bhapi_posix_sem_t *sem = new bhapi_posix_sem_t();
	if(!sem) return NULL;

	_BHAPI_LOCK_IPC_SEMAPHORE_();

	if((sem->mapping = bhapi_clone_area(name, (void**)&(sem->semInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= B_MAXUINT32)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	bhapi_posix_sem_info *sem_info = sem->semInfo;

#ifndef BHAPI_PTHREAD_SHARED
	sem->remoteiSem = &(sem_info->isem);
	sem->remoteSem = &(sem_info->sem);

	int sval;
	int err1 = (sem_getvalue(sem->remoteiSem, &sval) != 0 ? errno : 0);
	int err2 = (sem_getvalue(sem->remoteSem, &sval) != 0 ? errno : 0);
	if(err1 != 0 || err2 != 0)
	{
		BHAPI_WARNING("[KERNEL]: %s --- System seems not support process-shared semaphore, errno: %d - %d.",
			    __PRETTY_FUNCTION__, err1, err2);
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}
#else // BHAPI_PTHREAD_SHARED
	sem->iMutex = &(sem_info->mutex);
	sem->iCond = &(sem_info->cond);
#endif // BHAPI_PTHREAD_SHARED

	sem->semInfo->refCount += 1;

	_BHAPI_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


IMPEXP_BHAPI void* bhapi_clone_sem_by_source(void *data)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	if(bhapi_is_sem_for_IPC(sem)) return bhapi_clone_sem(sem->semInfo->name);

	_BHAPI_LOCK_LOCAL_SEMAPHORE_();
	if(sem->no_clone || sem->semInfo->refCount >= B_MAXUINT32 || sem->semInfo->refCount == 0)
	{
		_BHAPI_UNLOCK_LOCAL_SEMAPHORE_();
		return NULL;
	}
	sem->semInfo->refCount += 1;
	_BHAPI_UNLOCK_LOCAL_SEMAPHORE_();

	return data;
}


static void* bhapi_create_sem_for_local(b_int64 count)
{
	if(count < B_INT64_CONSTANT(0)) return NULL;

	bhapi_posix_sem_t *sem = new bhapi_posix_sem_t();
	if(!sem) return NULL;

	if((sem->semInfo = new bhapi_posix_sem_info()) == NULL ||
	   (sem->iMutex = new pthread_mutex_t) == NULL ||
	   (sem->iCond = new pthread_cond_t) == NULL)
	{
		if(sem->iMutex) delete sem->iMutex;
		if(sem->iCond) delete sem->iCond;
		if(sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	b_uint32 successFlags = 0;

	if(pthread_mutex_init(sem->iMutex, NULL) != 0) successFlags |= (1 << 1);
	if(pthread_cond_init(sem->iCond, NULL) != 0) successFlags |= (1 << 2);

	if(successFlags != 0)
	{
		if(!(successFlags & (1 << 1))) pthread_mutex_destroy(sem->iMutex);
		if(!(successFlags & (1 << 2))) pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->created = true;

	return (void*)sem;
}


IMPEXP_BHAPI void* bhapi_create_sem(b_int64 count, const char *name, bhapi_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			bhapi_create_sem_for_local(count) :
			bhapi_create_sem_for_IPC(count, name, area_access));
}


IMPEXP_BHAPI b_status_t bhapi_get_sem_info(void *data, bhapi_sem_info *info)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || !info) return B_BAD_VALUE;

	bzero(info->name, B_OS_NAME_LENGTH + 1);

	bhapi_lock_sem_inter(sem);

	if(bhapi_is_sem_for_IPC(sem)) strcpy(info->name, sem->semInfo->name);
	info->latest_holder_team = sem->semInfo->latestHolderTeamId;
	info->latest_holder_thread = sem->semInfo->latestHolderThreadId;
	info->count = sem->semInfo->count - sem->semInfo->acquiringCount;
	info->closed = sem->semInfo->closed;

	bhapi_unlock_sem_inter(sem);

	return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_delete_sem(void *data)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	if(bhapi_is_sem_for_IPC(sem)) _BHAPI_LOCK_IPC_SEMAPHORE_();
	else _BHAPI_LOCK_LOCAL_SEMAPHORE_();

	b_uint32 count = --(sem->semInfo->refCount);

	if(bhapi_is_sem_for_IPC(sem)) _BHAPI_UNLOCK_IPC_SEMAPHORE_();
	else _BHAPI_UNLOCK_LOCAL_SEMAPHORE_();

	if(bhapi_is_sem_for_IPC(sem))
	{
		if(count == 0)
		{
#ifndef BHAPI_PTHREAD_SHARED
			sem_destroy(sem->remoteiSem);
			sem_destroy(sem->remoteSem);
#else
			pthread_mutex_destroy(sem->iMutex);
			pthread_cond_destroy(sem->iCond);
#endif
		}
		bhapi_delete_area(sem->mapping);
	}
	else
	{
		if(count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_delete_sem_etc(void *data, bool no_clone)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	if(bhapi_is_sem_for_IPC(sem)) _BHAPI_LOCK_IPC_SEMAPHORE_();
	else _BHAPI_LOCK_LOCAL_SEMAPHORE_();

	if(!bhapi_is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	b_uint32 count = --(sem->semInfo->refCount);

	if(bhapi_is_sem_for_IPC(sem)) _BHAPI_UNLOCK_IPC_SEMAPHORE_();
	else _BHAPI_UNLOCK_LOCAL_SEMAPHORE_();

	if(bhapi_is_sem_for_IPC(sem))
	{
		if(count == 0 && no_clone)
		{
#ifndef BHAPI_PTHREAD_SHARED
			sem_destroy(sem->remoteiSem);
			sem_destroy(sem->remoteSem);
#else
			pthread_mutex_destroy(sem->iMutex);
			pthread_cond_destroy(sem->iCond);
#endif
		}
		bhapi_delete_area_etc(sem->mapping, no_clone);
	}
	else
	{
		if(count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_close_sem(void *data)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	bhapi_lock_sem_inter(sem);
	if(sem->semInfo->closed)
	{
		bhapi_unlock_sem_inter(sem);
		return B_ERROR;
	}
	sem->semInfo->closed = true;
#ifndef BHAPI_PTHREAD_SHARED
	if(bhapi_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->acquiringCount > B_INT64_CONSTANT(0)) sem_post(sem->remoteSem);
	}
	else
	{
#endif
		pthread_cond_broadcast(sem->iCond);
#ifndef BHAPI_PTHREAD_SHARED
	}
#endif
	bhapi_unlock_sem_inter(sem);

	return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_acquire_sem_etc(void *data, b_int64 count, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	if(microseconds_timeout < B_INT64_CONSTANT(0) || count < B_INT64_CONSTANT(1)) return B_BAD_VALUE;

	b_bigtime_t currentTime = bhapi_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi_lock_sem_inter(sem);

	if(sem->semInfo->count - count >= B_INT64_CONSTANT(0))
	{
		sem->semInfo->count -= count;
		sem->semInfo->SetLatestHolderTeamId(bhapi_get_current_team_id());
		sem->semInfo->SetLatestHolderThreadId(bhapi_get_current_thread_id());
		bhapi_unlock_sem_inter(sem);
		return B_OK;
	}
	else if(sem->semInfo->closed)
	{
		bhapi_unlock_sem_inter(sem);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi_unlock_sem_inter(sem);
		return B_WOULD_BLOCK;
	}
	if(count > B_MAXINT64 - sem->semInfo->acquiringCount)
	{
		bhapi_unlock_sem_inter(sem);
		return B_ERROR;
	}

	sem->semInfo->acquiringCount += count;
#ifndef BHAPI_PTHREAD_SHARED
	if(bhapi_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;
	}
#endif

	struct timespec ts;
	ts.tv_sec = (long)(microseconds_timeout / B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout % B_INT64_CONSTANT(1000000)) * 1000L;

	b_status_t retval = B_ERROR;

	while(true)
	{
#ifndef BHAPI_PTHREAD_SHARED
		if(bhapi_is_sem_for_IPC(sem))
		{
			sem_t *psem = sem->remoteSem;
			bhapi_unlock_sem_inter(sem);
			int ret = (wait_forever ? sem_wait(psem) : sem_timedwait(psem, &ts));
			bhapi_lock_sem_inter(sem);
			if(ret != 0)
			{
				if(errno == ETIMEDOUT && !wait_forever) retval = B_TIMED_OUT;
				break;
			}
		}
		else
		{
#endif
			int ret = (wait_forever ? pthread_cond_wait(sem->iCond, sem->iMutex) :
						  pthread_cond_timedwait(sem->iCond, sem->iMutex, &ts));

			if(ret != 0)
			{
				if(ret == ETIMEDOUT && !wait_forever)
					retval = B_TIMED_OUT;
				else
					bhapi_lock_sem_inter(sem);
				break;
			}
#ifndef BHAPI_PTHREAD_SHARED
		}
#endif

		if(sem->semInfo->count - count >= B_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(bhapi_get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(bhapi_get_current_thread_id());
			retval = B_OK;
			break;
		}
		else if(sem->semInfo->closed)
		{
			break;
		}

#ifndef BHAPI_PTHREAD_SHARED
		if(!bhapi_is_sem_for_IPC(sem)) continue;

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		sem_post(sem->remoteSem);
#endif
	}

	sem->semInfo->acquiringCount -= count;
#ifndef BHAPI_PTHREAD_SHARED
	if(bhapi_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = B_INT64_CONSTANT(0);
		sem_post(sem->remoteSem);
	}
#endif

	bhapi_unlock_sem_inter(sem);

	return retval;
}


IMPEXP_BHAPI b_status_t bhapi_acquire_sem(void *data)
{
	return bhapi_acquire_sem_etc(data, B_INT64_CONSTANT(1), B_TIMEOUT, B_INFINITE_TIMEOUT);
}


IMPEXP_BHAPI b_status_t bhapi_release_sem_etc(void *data, b_int64 count, b_uint32 flags)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || count < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bhapi_lock_sem_inter(sem);

	b_status_t retval = B_ERROR;

	if(sem->semInfo->closed == false && (B_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;

		if(flags != B_DO_NOT_RESCHEDULE)
		{
#ifndef BHAPI_PTHREAD_SHARED
			if(bhapi_is_sem_for_IPC(sem))
			{
				if(sem->semInfo->acquiringCount > B_INT64_CONSTANT(0)) sem_post(sem->remoteSem);
			}
			else
			{
#endif
				pthread_cond_broadcast(sem->iCond);
#ifndef BHAPI_PTHREAD_SHARED
			}
#endif
		}

		retval = B_OK;
	}

	bhapi_unlock_sem_inter(sem);

	return retval;
}


IMPEXP_BHAPI b_status_t bhapi_release_sem(void *data)
{
	return bhapi_release_sem_etc(data, B_INT64_CONSTANT(1), 0);
}


IMPEXP_BHAPI b_status_t bhapi_get_sem_count(void *data, b_int64 *count)
{
	bhapi_posix_sem_t *sem = (bhapi_posix_sem_t*)data;
	if(!sem || !count) return B_BAD_VALUE;

	bhapi_lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= B_INT64_CONSTANT(0) ?
			sem->semInfo->count : B_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	bhapi_unlock_sem_inter(sem);

	return B_OK;
}
#endif