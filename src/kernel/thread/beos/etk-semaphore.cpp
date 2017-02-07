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

#include <be/kernel/OS.h"

#include "./../kernel/Kernel.h"
#include "./../support/StringMe.h"


inline team_id __bhapi_get_current_beos_team_id()
{
	thread_info curThreadInfo;
	return(get_thread_info(find_thread(NULL), &curThreadInfo) == B_OK ? curThreadInfo.team : -1);
}


typedef struct bhapi_beos_sem_info {
	bhapi_beos_sem_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, B_OS_NAME_LENGTH + 1);
		latestHolderTeamId = B_INT64_CONSTANT(0);
		latestHolderThreadId = B_INT64_CONSTANT(0);
		count = B_INT64_CONSTANT(0);
		minAcquiringCount = B_INT64_CONSTANT(0);
		acquiringCount = B_INT64_CONSTANT(0);
		closed = false;
		refCount = 0;
		Locker = -1;
		Cond = -1;
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
	b_int64			minAcquiringCount;
	b_int64			acquiringCount;
	bool			closed;

	b_uint32			refCount;
	sem_id			Locker;
	sem_id			Cond;
} bhapi_beos_sem_info;

typedef struct bhapi_beos_sem_t {
	bhapi_beos_sem_t()
		: mapping(NULL), semInfo(NULL), Locker(-1), Cond(-1),
		  created(false), no_clone(false)
	{
	}

	~bhapi_beos_sem_t()
	{
		if(created)
		{
			created = false;
			bhapi_delete_sem((void*)this);
		}
	}

	void			*mapping;
	bhapi_beos_sem_info	*semInfo;

	sem_id			Locker;
	sem_id			Cond;

	bool			created;
	bool			no_clone;
} bhapi_beos_sem_t;

class bhapi_beos_sem_locker_t {
public:
	bhapi_beos_sem_locker_t()
	{
		const char *lockerName = "_bhapi_global_";

		if((iLocker = find_port(lockerName)) < 0)
		{
			if((iLocker = create_port(1, lockerName)) >= 0)
			{
				char buf = 1;
				if(set_port_owner(iLocker, B_SYSTEM_TEAM) != B_OK || write_port(iLocker, 'bhapi_', &buf, 1) != B_OK)
				{
					delete_port(iLocker);
					iLocker = -1;
				}
			}
			if(iLocker >= 0) BHAPI_DEBUG("[KERNEL]: port for global semaphore locker created.");
		}
		else
		{
			port_info portInfo;
			if(get_port_info(iLocker, &portInfo) != B_OK || portInfo.capacity != 1) iLocker = -1;
			if(iLocker >= 0) BHAPI_DEBUG("[KERNEL]: port for global semaphore locker found.");
		}
		if(iLocker < 0) BHAPI_ERROR("[KERNEL]: Can't initialize global semaphore!");
	}

	void Lock()
	{
//		BHAPI_DEBUG("[KERNEL]: try locking global semaphore.");

		while(true)
		{
			int32 msgCode = 0;
			char buf = 0;
			ssize_t readBytes = read_port(iLocker, &msgCode, &buf, 1);
			if(readBytes < 1) continue;
			if(readBytes != 1 || msgCode != 'bhapi_' || buf != 1)
				BHAPI_ERROR("[KERNEL]: Unable to lock the locker for global semaphore.");
//			BHAPI_DEBUG("[KERNEL]: global semaphore locker locked.");
			break;
		}
	}

	void Unlock()
	{
		char buf = 1;
		if(write_port(iLocker, 'bhapi_', &buf, 1) != B_OK) BHAPI_ERROR("[KERNEL]: Unable to unlock the locker for global semaphore.");
//		BHAPI_DEBUG("[KERNEL]: global semaphore locker unlocked.");
	}

	port_id iLocker;
};

static bhapi_beos_sem_locker_t __bhapi_semaphore_locker__;

static void _BHAPI_LOCK_SEMAPHORE_()
{
	__bhapi_semaphore_locker__.Lock();
}

static void _BHAPI_UNLOCK_SEMAPHORE_()
{
	__bhapi_semaphore_locker__.Unlock();
}


static bool bhapi_is_sem_for_IPC(const bhapi_beos_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void bhapi_lock_sem_inter(bhapi_beos_sem_t *sem)
{
	team_id curTeam = __bhapi_get_current_beos_team_id();
	sem_info semInfo;

	while(true)
	{
		if(bhapi_is_sem_for_IPC(sem))
		{
			_BHAPI_LOCK_SEMAPHORE_();
			get_sem_info(sem->Locker, &semInfo);
			if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, curTeam);
			_BHAPI_UNLOCK_SEMAPHORE_();
		}

		if(acquire_sem(sem->Locker) == B_OK) break;
	}
}


static void bhapi_unlock_sem_inter(bhapi_beos_sem_t *sem)
{
	if(bhapi_is_sem_for_IPC(sem))
	{
		_BHAPI_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Locker, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, __bhapi_get_current_beos_team_id());
		release_sem(sem->Locker);
		_BHAPI_UNLOCK_SEMAPHORE_();
	}
	else
	{
		release_sem(sem->Locker);
	}
}


static void* bhapi_create_sem_for_IPC(b_int64 count, const char *name, bhapi_area_access area_access)
{
	if(count < B_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4) // because of length of area's name can't exceeds B_OS_NAME_LENGTH - 4
	{
		BHAPI_WARNING("\n=======================================================================\n[KERNEL]: %s --- Length of semaphore's name exceeds %d.\n=======================================================================\n", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}

	bhapi_beos_sem_t *sem = new bhapi_beos_sem_t();
	if(!sem) return NULL;

	_BHAPI_LOCK_SEMAPHORE_();

	if((sem->mapping = bhapi_create_area(name, (void**)&(sem->semInfo), sizeof(bhapi_beos_sem_info),
					   B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed.", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	bhapi_beos_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

	if((sem->Locker = create_sem(1, NULL)) < 0)
	{
		if(sem->Locker >= 0) delete_sem(sem->Locker);
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}
	if((sem->Cond = create_sem(0, NULL)) < 0)
	{
		delete_sem(sem->Locker);
		if(sem->Cond >= 0) delete_sem(sem->Cond);
		bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->semInfo->Locker = sem->Locker;
	sem->semInfo->Cond = sem->Cond;

	_BHAPI_UNLOCK_SEMAPHORE_();

	sem->created = true;

//	BHAPI_DEBUG("[KERNEL]: %s --- SEMAPHORE [%s] created.", __PRETTY_FUNCTION__, name);

	return (void*)sem;
}


_IMPEXP_BHAPI void* bhapi_clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4) // because of length of area's name can't exceeds B_OS_NAME_LENGTH - 4
	{
		BHAPI_WARNING("[KERNEL]: %s --- Length of semaphore's name exceeds %d.", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}

	bhapi_beos_sem_t *sem = new bhapi_beos_sem_t();
	if(!sem) return NULL;

	_BHAPI_LOCK_SEMAPHORE_();

	if((sem->mapping = bhapi_clone_area(name, (void**)&(sem->semInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= B_MAXUINT32)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi_delete_area(sem->mapping);
		_BHAPI_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->Locker = sem->semInfo->Locker;
	sem->Cond = sem->semInfo->Cond;

	sem->semInfo->refCount += 1;

	team_id curTeam = __bhapi_get_current_beos_team_id();
	sem_info semInfo;
	get_sem_info(sem->Locker, &semInfo);
	if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, curTeam);
	get_sem_info(sem->Cond, &semInfo);
	if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, curTeam);

	_BHAPI_UNLOCK_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


_IMPEXP_BHAPI void* bhapi_clone_sem_by_source(void *data)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	_BHAPI_LOCK_SEMAPHORE_();

	if(bhapi_is_sem_for_IPC(sem))
	{
		_BHAPI_UNLOCK_SEMAPHORE_();
		return bhapi_clone_sem(sem->semInfo->name);
	}
	else if(sem->no_clone || sem->semInfo->refCount >= B_MAXUINT32)
	{
		_BHAPI_UNLOCK_SEMAPHORE_();
		return NULL;
	}

	sem->semInfo->refCount += 1;

	_BHAPI_UNLOCK_SEMAPHORE_();

	return data;
}


static void* bhapi_create_sem_for_local(b_int64 count)
{
	if(count < B_INT64_CONSTANT(0)) return NULL;

	bhapi_beos_sem_t *sem = new bhapi_beos_sem_t();

	if(!sem) return NULL;

	if((sem->semInfo = new bhapi_beos_sem_info()) == NULL ||
	   (sem->Locker = create_sem(1, NULL)) < 0 ||
	   (sem->Cond = create_sem(0, NULL)) < 0)
	{
		if(sem->Cond >= 0) delete_sem(sem->Cond);
		if(sem->Locker >= 0) delete_sem(sem->Locker);
		if(sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->created = true;

	return (void*)sem;
}


_IMPEXP_BHAPI void* bhapi_create_sem(b_int64 count, const char *name, bhapi_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			bhapi_create_sem_for_local(count) :
			bhapi_create_sem_for_IPC(count, name, area_access));
}


_IMPEXP_BHAPI b_status_t bhapi_get_sem_info(void *data, bhapi_sem_info *info)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
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


_IMPEXP_BHAPI b_status_t bhapi_delete_sem(void *data)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	_BHAPI_LOCK_SEMAPHORE_();

	if(sem->semInfo->refCount == 0)
	{
		_BHAPI_UNLOCK_SEMAPHORE_();
		return B_ERROR;
	}

	b_uint32 count = --(sem->semInfo->refCount);

	team_id curTeam = __bhapi_get_current_beos_team_id();

	if(bhapi_is_sem_for_IPC(sem))
	{
		if(count != 0)
		{
			sem_info semInfo;
			get_sem_info(sem->Locker, &semInfo);
			if(semInfo.team == curTeam) set_sem_owner(sem->Locker, B_SYSTEM_TEAM);
			get_sem_info(sem->Cond, &semInfo);
			if(semInfo.team == curTeam) set_sem_owner(sem->Cond, B_SYSTEM_TEAM);
		}
		else
		{
			set_sem_owner(sem->Locker, curTeam); delete_sem(sem->Locker);
			set_sem_owner(sem->Cond, curTeam); delete_sem(sem->Cond);
		}

		bhapi_delete_area(sem->mapping);
	}

	_BHAPI_UNLOCK_SEMAPHORE_();

	if(!bhapi_is_sem_for_IPC(sem))
	{
		if(count > 0) return B_OK;

		delete_sem(sem->Locker);
		delete_sem(sem->Cond);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


_IMPEXP_BHAPI b_status_t bhapi_delete_sem_etc(void *data, bool no_clone)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	_BHAPI_LOCK_SEMAPHORE_();

	if(sem->semInfo->refCount == 0)
	{
		_BHAPI_UNLOCK_SEMAPHORE_();
		return B_ERROR;
	}

	b_uint32 count = --(sem->semInfo->refCount);

	team_id curTeam = __bhapi_get_current_beos_team_id();

	if(bhapi_is_sem_for_IPC(sem))
	{
		if(count != 0)
		{
			sem_info semInfo;
			get_sem_info(sem->Locker, &semInfo);
			if(semInfo.team == curTeam) set_sem_owner(sem->Locker, B_SYSTEM_TEAM);
			get_sem_info(sem->Cond, &semInfo);
			if(semInfo.team == curTeam) set_sem_owner(sem->Cond, B_SYSTEM_TEAM);
		}
		else if(no_clone)
		{
			set_sem_owner(sem->Locker, curTeam); delete_sem(sem->Locker);
			set_sem_owner(sem->Cond, curTeam); delete_sem(sem->Cond);
		}

		bhapi_delete_area_etc(sem->mapping, no_clone);
	}
	else if(no_clone)
	{
		sem->no_clone = true;
	}

	_BHAPI_UNLOCK_SEMAPHORE_();

	if(!bhapi_is_sem_for_IPC(sem))
	{
		if(count > 0) return B_OK;

		delete_sem(sem->Locker);
		delete_sem(sem->Cond);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


_IMPEXP_BHAPI b_status_t bhapi_close_sem(void *data)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	if(bhapi_is_sem_for_IPC(sem))
	{
		_BHAPI_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __bhapi_get_current_beos_team_id());
		_BHAPI_UNLOCK_SEMAPHORE_();
	}

	bhapi_lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		bhapi_unlock_sem_inter(sem);
		return B_ERROR;
	}
	sem->semInfo->closed = true;

	release_sem(sem->Cond);

	bhapi_unlock_sem_inter(sem);

	return B_OK;
}


_IMPEXP_BHAPI b_status_t bhapi_acquire_sem_etc(void *data, b_int64 count, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
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

	if(bhapi_is_sem_for_IPC(sem))
	{
		_BHAPI_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __bhapi_get_current_beos_team_id());
		_BHAPI_UNLOCK_SEMAPHORE_();
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
	if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
	   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

	b_status_t retval = B_ERROR;

	while(true)
	{
		bhapi_unlock_sem_inter(sem);

		status_t status = (wait_forever ? acquire_sem(sem->Cond) :
						  acquire_sem_etc(sem->Cond, 1,
							  	  B_ABSOLUTE_TIMEOUT,
								  (bigtime_t)(microseconds_timeout - bhapi_system_boot_time())));

		bhapi_lock_sem_inter(sem);

		if(status != B_OK)
		{
//			BHAPI_DEBUG("[KERNEL]: %s --- %s(%s)failed, error_code: B_GENERAL_ERROR_BASE + 0x%x",
//				  __PRETTY_FUNCTION__, wait_forever ? "acquire_sem" : "acquire_sem_etc", sem->semInfo->name,
//				  status - B_GENERAL_ERROR_BASE);

			if(status == B_WOULD_BLOCK) retval = B_WOULD_BLOCK;
			else if(status == B_TIMED_OUT) retval = B_TIMED_OUT;
			else continue;

			break;
		}

		if(sem->semInfo->count - count >= B_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(bhapi_get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(bhapi_get_current_thread_id());
			retval = B_OK;
			break;
		}
		else if(sem->semInfo->closed) break;

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		release_sem(sem->Cond);
	}

	sem->semInfo->acquiringCount -= count;

	if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = B_INT64_CONSTANT(0);
	release_sem(sem->Cond);

	bhapi_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_BHAPI b_status_t bhapi_acquire_sem(void *data)
{
	return bhapi_acquire_sem_etc(data, B_INT64_CONSTANT(1), B_TIMEOUT, B_INFINITE_TIMEOUT);
}


_IMPEXP_BHAPI b_status_t bhapi_release_sem_etc(void *data, b_int64 count, b_uint32 flags)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem || count < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	if(bhapi_is_sem_for_IPC(sem))
	{
		_BHAPI_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __bhapi_get_current_beos_team_id());
		_BHAPI_UNLOCK_SEMAPHORE_();
	}

	bhapi_lock_sem_inter(sem);

	b_status_t retval = B_ERROR;

	if(sem->semInfo->closed == false && (B_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;
		if(flags != B_DO_NOT_RESCHEDULE && sem->semInfo->acquiringCount > B_INT64_CONSTANT(0)) release_sem(sem->Cond);
		retval = B_OK;
	}

	bhapi_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_BHAPI b_status_t bhapi_release_sem(void *data)
{
	return bhapi_release_sem_etc(data, B_INT64_CONSTANT(1), 0);
}


_IMPEXP_BHAPI b_status_t bhapi_get_sem_count(void *data, b_int64 *count)
{
	bhapi_beos_sem_t *sem = (bhapi_beos_sem_t*)data;
	if(!sem || !count) return B_BAD_VALUE;

	bhapi_lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= B_INT64_CONSTANT(0) ?
			sem->semInfo->count : B_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	bhapi_unlock_sem_inter(sem);

	return B_OK;
}

