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
 * File: etk-semaphore-umtx.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <fcntl.h"

#include <pthread.h"
#include <semaphore.h"

#include <sys/types.h>
#include <unistd.h"
#include <string.h"
#include <time.h"
#include <errno.h"

extern "C" {
/* use umtx instead of pshared-semaphore */
#include <machine/atomic.h"
#include <ucontext.h"
#include <sys/thr.h"
#include <sys/umtx.h"
} // extern "C"

#include <sys/stat.h"
#include <sys/mman.h"

#include "./../kernel/Kernel.h"
#include "./../support/String.h"

typedef struct etk_posix_sem_info {
	etk_posix_sem_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, E_OS_NAME_LENGTH + 1);
		latestHolderTeamId = E_INT64_CONSTANT(0);
		latestHolderThreadId = E_INT64_CONSTANT(0);
		count = E_INT64_CONSTANT(0);
		minAcquiringCount = E_INT64_CONSTANT(0);
		acquiringCount = E_INT64_CONSTANT(0);
		closed = false;
		seq = E_INT64_CONSTANT(0);
		refCount = 0;
	}

	void SetLatestHolderTeamId(eint64 id)
	{
		latestHolderTeamId = id;
	}

	void SetLatestHolderThreadId(eint64 id)
	{
		latestHolderThreadId = id;
	}

	bool LatestHolderTeamIsCurrent(void)
	{
		return(latestHolderTeamId == etk_get_current_team_id());
	}

	bool LatestHolderThreadIsCurrent(void)
	{
		return(latestHolderThreadId == etk_get_current_thread_id());
	}

	char			name[E_OS_NAME_LENGTH + 1];
	eint64			latestHolderTeamId;
	eint64			latestHolderThreadId;
	eint64			count;
	eint64			minAcquiringCount;
	eint64			acquiringCount;
	bool			closed;

	// for IPC
	struct umtx		locker;
	long			seq;

	// for local
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;

	euint32			refCount;
} etk_posix_sem_info;


typedef struct etk_posix_sem_t {
	etk_posix_sem_t()
		: mapping(NULL), semInfo(NULL), iMutex(NULL), iCond(NULL),
		  created(false), no_clone(false)
	{
	}

	~etk_posix_sem_t()
	{
		if(created)
		{
			created = false;
			etk_delete_sem((void*)this);
		}
	}

	// for IPC (name != NULL)
	void*			mapping;
	etk_posix_sem_info*	semInfo;

	struct umtx*		iLocker;
	long*			iSeq;

	// for local
	pthread_mutex_t*	iMutex;
	pthread_cond_t*		iCond;

	bool			created;
	bool			no_clone;
} etk_posix_sem_t;


static long etk_get_current_bsd_tid()
{
	long tid;
	thr_self(&tid);
	return tid;
}

#ifndef SEM_FAILED
#define SEM_FAILED	(-1)
#endif

class etk_posix_sem_locker_t {
public:
	sem_t *fSem;
	pthread_mutex_t fLocker;

	etk_posix_sem_locker_t()
		: fSem((sem_t*)SEM_FAILED)
	{
		pthread_mutex_init(&fLocker, NULL);
	}

	~etk_posix_sem_locker_t()
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

		const char *semName = "/_etk_global_";
		if((fSem = (sem_t*)sem_open(semName, O_CREAT | O_EXCL, 0666, 1)) == (sem_t*)SEM_FAILED)
		{
//			ETK_DEBUG("[KERNEL]: Unable to create global semaphore, errno: %d", errno);
			fSem = (sem_t*)sem_open(semName, 0);
		}
		if(fSem == (sem_t*)SEM_FAILED)
			ETK_ERROR("[KERNEL]: Can't initialize global semaphore! errno: %d", errno);
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


static etk_posix_sem_locker_t __etk_semaphore_locker__;
#define _ETK_LOCK_IPC_SEMAPHORE_()		__etk_semaphore_locker__.LockIPC()
#define _ETK_UNLOCK_IPC_SEMAPHORE_()		__etk_semaphore_locker__.UnlockIPC()
#define _ETK_LOCK_LOCAL_SEMAPHORE_()		__etk_semaphore_locker__.LockLocal()
#define _ETK_UNLOCK_LOCAL_SEMAPHORE_()		__etk_semaphore_locker__.UnlockLocal()


static bool etk_is_sem_for_IPC(const etk_posix_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void etk_lock_sem_inter(etk_posix_sem_t *sem)
{
	if(etk_is_sem_for_IPC(sem))
		umtx_lock(sem->iLocker, etk_get_current_bsd_tid());
	else
		pthread_mutex_lock(sem->iMutex);
}


static void etk_unlock_sem_inter(etk_posix_sem_t *sem)
{
	if(etk_is_sem_for_IPC(sem))
		umtx_unlock(sem->iLocker, etk_get_current_bsd_tid());
	else
		pthread_mutex_unlock(sem->iMutex);
}


static void* etk_create_sem_for_IPC(eint64 count, const char *name, etk_area_access area_access)
{
	if(count < E_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	etk_posix_sem_t *sem = new etk_posix_sem_t();
	if(!sem) return NULL;

	_ETK_LOCK_IPC_SEMAPHORE_();

	if((sem->mapping = etk_create_area(name, (void**)&(sem->semInfo), sizeof(etk_posix_sem_info),
					  E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	etk_posix_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

	umtx_init(sem->iLocker = &(sem_info->locker));
	sem->iSeq = &(sem_info->seq);

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	_ETK_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	etk_posix_sem_t *sem = new etk_posix_sem_t();
	if(!sem) return NULL;

	_ETK_LOCK_IPC_SEMAPHORE_();

	if((sem->mapping = etk_clone_area(name, (void**)&(sem->semInfo),
					  E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= E_MAXUINT32)
	{
//		ETK_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_IPC_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	etk_posix_sem_info *sem_info = sem->semInfo;

	sem->iLocker = &(sem_info->locker);
	sem->iSeq = &(sem_info->seq);

	sem->semInfo->refCount += 1;

	_ETK_UNLOCK_IPC_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem_by_source(void *data)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	if(etk_is_sem_for_IPC(sem)) return etk_clone_sem(sem->semInfo->name);

	_ETK_LOCK_LOCAL_SEMAPHORE_();
	if(sem->no_clone || sem->semInfo->refCount >= E_MAXUINT32 || sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_LOCAL_SEMAPHORE_();
		return NULL;
	}
	sem->semInfo->refCount += 1;
	_ETK_UNLOCK_LOCAL_SEMAPHORE_();

	return data;
}


static void* etk_create_sem_for_local(eint64 count)
{
	if(count < E_INT64_CONSTANT(0)) return NULL;

	etk_posix_sem_t *sem = new etk_posix_sem_t();
	if(!sem) return NULL;

	if((sem->semInfo = new etk_posix_sem_info()) == NULL ||
	   (sem->iMutex = new pthread_mutex_t) == NULL ||
	   (sem->iCond = new pthread_cond_t) == NULL)
	{
		if(sem->iMutex) delete sem->iMutex;
		if(sem->iCond) delete sem->iCond;
		if(sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	euint32 successFlags = 0;

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


_IMPEXP_ETK void* etk_create_sem(eint64 count, const char *name, etk_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			etk_create_sem_for_local(count) :
			etk_create_sem_for_IPC(count, name, area_access));
}


_IMPEXP_ETK e_status_t etk_get_sem_info(void *data, etk_sem_info *info)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || !info) return E_BAD_VALUE;

	bzero(info->name, E_OS_NAME_LENGTH + 1);

	etk_lock_sem_inter(sem);

	if(etk_is_sem_for_IPC(sem)) strcpy(info->name, sem->semInfo->name);
	info->latest_holder_team = sem->semInfo->latestHolderTeamId;
	info->latest_holder_thread = sem->semInfo->latestHolderThreadId;
	info->count = sem->semInfo->count - sem->semInfo->acquiringCount;
	info->closed = sem->semInfo->closed;

	etk_unlock_sem_inter(sem);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_delete_sem(void *data)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	if(etk_is_sem_for_IPC(sem)) _ETK_LOCK_IPC_SEMAPHORE_();
	else _ETK_LOCK_LOCAL_SEMAPHORE_();

	euint32 count = --(sem->semInfo->refCount);

	if(etk_is_sem_for_IPC(sem)) _ETK_UNLOCK_IPC_SEMAPHORE_();
	else _ETK_UNLOCK_LOCAL_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
		etk_delete_area(sem->mapping);
	}
	else
	{
		if(count > 0) return E_OK;

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

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_delete_sem_etc(void *data, bool no_clone)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	if(etk_is_sem_for_IPC(sem)) _ETK_LOCK_IPC_SEMAPHORE_();
	else _ETK_LOCK_LOCAL_SEMAPHORE_();

	if(!etk_is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	euint32 count = --(sem->semInfo->refCount);

	if(etk_is_sem_for_IPC(sem)) _ETK_UNLOCK_IPC_SEMAPHORE_();
	else _ETK_UNLOCK_LOCAL_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
		etk_delete_area_etc(sem->mapping, no_clone);
	}
	else
	{
		if(count > 0) return E_OK;

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

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_close_sem(void *data)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}
	sem->semInfo->closed = true;

	if(etk_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->acquiringCount > E_INT64_CONSTANT(0))
		{
			*(sem->iSeq) += 1;
			umtx_wake((struct umtx*)sem->iSeq, 1);
		}
	}
	else
	{
		pthread_cond_broadcast(sem->iCond);
	}

	etk_unlock_sem_inter(sem);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_acquire_sem_etc(void *data, eint64 count, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem) return E_BAD_VALUE;

	if(microseconds_timeout < E_INT64_CONSTANT(0) || count < E_INT64_CONSTANT(1)) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_sem_inter(sem);

	if(sem->semInfo->count - count >= E_INT64_CONSTANT(0))
	{
		sem->semInfo->count -= count;
		sem->semInfo->SetLatestHolderTeamId(etk_get_current_team_id());
		sem->semInfo->SetLatestHolderThreadId(etk_get_current_thread_id());
		etk_unlock_sem_inter(sem);
		return E_OK;
	}
	else if(sem->semInfo->closed)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_sem_inter(sem);
		return E_WOULD_BLOCK;
	}
	if(count > E_MAXINT64 - sem->semInfo->acquiringCount)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}

	sem->semInfo->acquiringCount += count;
	if(etk_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;
	}

	struct timespec ts;
	e_bigtime_t timeout = microseconds_timeout - currentTime;
	ts.tv_sec = (long)(microseconds_timeout / E_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout % E_INT64_CONSTANT(1000000)) * 1000L;

	e_status_t retval = E_ERROR;

	while(true)
	{
		if(etk_is_sem_for_IPC(sem))
		{
			long seq = *(sem->iSeq);

			etk_unlock_sem_inter(sem);

			int ret;
			if(wait_forever)
			{
				ret = umtx_wait((struct umtx*)sem->iSeq, seq, NULL);
			}
			else
			{
				struct timespec ts2;
				ts2.tv_sec = (long)(timeout / E_INT64_CONSTANT(1000000));
				ts2.tv_nsec = (long)(timeout % E_INT64_CONSTANT(1000000)) * 1000L;

				ret = (timeout < 0 ? ETIMEDOUT : umtx_wait((struct umtx*)sem->iSeq, seq, &ts2));

				timeout = microseconds_timeout - etk_real_time_clock_usecs();
			}

			etk_lock_sem_inter(sem);
			if(ret != 0)
			{
				if(ret == ETIMEDOUT && !wait_forever) retval = E_TIMED_OUT;
				break;
			}
		}
		else
		{
			int ret = (wait_forever ? pthread_cond_wait(sem->iCond, sem->iMutex) :
						  pthread_cond_timedwait(sem->iCond, sem->iMutex, &ts));

			if(ret != 0)
			{
				if(ret == ETIMEDOUT && !wait_forever)
					retval = E_TIMED_OUT;
				else
					etk_lock_sem_inter(sem);
				break;
			}
		}

		if(sem->semInfo->count - count >= E_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(etk_get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(etk_get_current_thread_id());
			retval = E_OK;
			break;
		}
		else if(sem->semInfo->closed)
		{
			break;
		}

		if(!etk_is_sem_for_IPC(sem)) continue;

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		*(sem->iSeq) += 1;
		umtx_wake((struct umtx*)sem->iSeq, 1);
	}

	sem->semInfo->acquiringCount -= count;
	if(etk_is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = E_INT64_CONSTANT(0);
		*(sem->iSeq) += 1;
		umtx_wake((struct umtx*)sem->iSeq, 1);
	}

	etk_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_ETK e_status_t etk_acquire_sem(void *data)
{
	return etk_acquire_sem_etc(data, E_INT64_CONSTANT(1), E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_release_sem_etc(void *data, eint64 count, euint32 flags)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || count < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);

	e_status_t retval = E_ERROR;

	if(sem->semInfo->closed == false && (E_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;

		if(flags != E_DO_NOT_RESCHEDULE)
		{
			if(etk_is_sem_for_IPC(sem))
			{
				if(sem->semInfo->acquiringCount > E_INT64_CONSTANT(0))
				{
					*(sem->iSeq) += 1;
					umtx_wake((struct umtx*)sem->iSeq, 1);
				}
			}
			else
			{
				pthread_cond_broadcast(sem->iCond);
			}
		}

		retval = E_OK;
	}

	etk_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_ETK e_status_t etk_release_sem(void *data)
{
	return etk_release_sem_etc(data, E_INT64_CONSTANT(1), 0);
}


_IMPEXP_ETK e_status_t etk_get_sem_count(void *data, eint64 *count)
{
	etk_posix_sem_t *sem = (etk_posix_sem_t*)data;
	if(!sem || !count) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= E_INT64_CONSTANT(0) ?
			sem->semInfo->count : E_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	etk_unlock_sem_inter(sem);

	return E_OK;
}

