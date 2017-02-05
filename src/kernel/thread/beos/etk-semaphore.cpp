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
 * File: etk-semaphore.cpp
 *
 * --------------------------------------------------------------------------*/

#include <be/kernel/OS.h"

#include "./../kernel/Kernel.h"
#include "./../support/String.h"


inline team_id __etk_get_current_beos_team_id()
{
	thread_info curThreadInfo;
	return(get_thread_info(find_thread(NULL), &curThreadInfo) == B_OK ? curThreadInfo.team : -1);
}


typedef struct etk_beos_sem_info {
	etk_beos_sem_info()
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
		refCount = 0;
		Locker = -1;
		Cond = -1;
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

	euint32			refCount;
	sem_id			Locker;
	sem_id			Cond;
} etk_beos_sem_info;

typedef struct etk_beos_sem_t {
	etk_beos_sem_t()
		: mapping(NULL), semInfo(NULL), Locker(-1), Cond(-1),
		  created(false), no_clone(false)
	{
	}

	~etk_beos_sem_t()
	{
		if(created)
		{
			created = false;
			etk_delete_sem((void*)this);
		}
	}

	void			*mapping;
	etk_beos_sem_info	*semInfo;

	sem_id			Locker;
	sem_id			Cond;

	bool			created;
	bool			no_clone;
} etk_beos_sem_t;

class etk_beos_sem_locker_t {
public:
	etk_beos_sem_locker_t()
	{
		const char *lockerName = "_etk_global_";

		if((iLocker = find_port(lockerName)) < 0)
		{
			if((iLocker = create_port(1, lockerName)) >= 0)
			{
				char buf = 1;
				if(set_port_owner(iLocker, B_SYSTEM_TEAM) != B_OK || write_port(iLocker, 'etk_', &buf, 1) != B_OK)
				{
					delete_port(iLocker);
					iLocker = -1;
				}
			}
			if(iLocker >= 0) ETK_DEBUG("[KERNEL]: port for global semaphore locker created.");
		}
		else
		{
			port_info portInfo;
			if(get_port_info(iLocker, &portInfo) != B_OK || portInfo.capacity != 1) iLocker = -1;
			if(iLocker >= 0) ETK_DEBUG("[KERNEL]: port for global semaphore locker found.");
		}
		if(iLocker < 0) ETK_ERROR("[KERNEL]: Can't initialize global semaphore!");
	}

	void Lock()
	{
//		ETK_DEBUG("[KERNEL]: try locking global semaphore.");

		while(true)
		{
			int32 msgCode = 0;
			char buf = 0;
			ssize_t readBytes = read_port(iLocker, &msgCode, &buf, 1);
			if(readBytes < 1) continue;
			if(readBytes != 1 || msgCode != 'etk_' || buf != 1)
				ETK_ERROR("[KERNEL]: Unable to lock the locker for global semaphore.");
//			ETK_DEBUG("[KERNEL]: global semaphore locker locked.");
			break;
		}
	}

	void Unlock()
	{
		char buf = 1;
		if(write_port(iLocker, 'etk_', &buf, 1) != B_OK) ETK_ERROR("[KERNEL]: Unable to unlock the locker for global semaphore.");
//		ETK_DEBUG("[KERNEL]: global semaphore locker unlocked.");
	}

	port_id iLocker;
};

static etk_beos_sem_locker_t __etk_semaphore_locker__;

static void _ETK_LOCK_SEMAPHORE_()
{
	__etk_semaphore_locker__.Lock();
}

static void _ETK_UNLOCK_SEMAPHORE_()
{
	__etk_semaphore_locker__.Unlock();
}


static bool etk_is_sem_for_IPC(const etk_beos_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void etk_lock_sem_inter(etk_beos_sem_t *sem)
{
	team_id curTeam = __etk_get_current_beos_team_id();
	sem_info semInfo;

	while(true)
	{
		if(etk_is_sem_for_IPC(sem))
		{
			_ETK_LOCK_SEMAPHORE_();
			get_sem_info(sem->Locker, &semInfo);
			if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, curTeam);
			_ETK_UNLOCK_SEMAPHORE_();
		}

		if(acquire_sem(sem->Locker) == B_OK) break;
	}
}


static void etk_unlock_sem_inter(etk_beos_sem_t *sem)
{
	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Locker, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, __etk_get_current_beos_team_id());
		release_sem(sem->Locker);
		_ETK_UNLOCK_SEMAPHORE_();
	}
	else
	{
		release_sem(sem->Locker);
	}
}


static void* etk_create_sem_for_IPC(eint64 count, const char *name, etk_area_access area_access)
{
	if(count < E_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4) // because of length of area's name can't exceeds B_OS_NAME_LENGTH - 4
	{
		ETK_WARNING("\n=======================================================================\n[KERNEL]: %s --- Length of semaphore's name exceeds %d.\n=======================================================================\n", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}

	etk_beos_sem_t *sem = new etk_beos_sem_t();
	if(!sem) return NULL;

	_ETK_LOCK_SEMAPHORE_();

	if((sem->mapping = etk_create_area(name, (void**)&(sem->semInfo), sizeof(etk_beos_sem_info),
					   E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
//		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed.", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	etk_beos_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

	if((sem->Locker = create_sem(1, NULL)) < 0)
	{
		if(sem->Locker >= 0) delete_sem(sem->Locker);
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}
	if((sem->Cond = create_sem(0, NULL)) < 0)
	{
		delete_sem(sem->Locker);
		if(sem->Cond >= 0) delete_sem(sem->Cond);
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->semInfo->Locker = sem->Locker;
	sem->semInfo->Cond = sem->Cond;

	_ETK_UNLOCK_SEMAPHORE_();

	sem->created = true;

//	ETK_DEBUG("[KERNEL]: %s --- SEMAPHORE [%s] created.", __PRETTY_FUNCTION__, name);

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4) // because of length of area's name can't exceeds B_OS_NAME_LENGTH - 4
	{
		ETK_WARNING("[KERNEL]: %s --- Length of semaphore's name exceeds %d.", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}

	etk_beos_sem_t *sem = new etk_beos_sem_t();
	if(!sem) return NULL;

	_ETK_LOCK_SEMAPHORE_();

	if((sem->mapping = etk_clone_area(name, (void**)&(sem->semInfo),
					  E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= E_MAXUINT32)
	{
//		ETK_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		delete sem;
		return NULL;
	}

	sem->Locker = sem->semInfo->Locker;
	sem->Cond = sem->semInfo->Cond;

	sem->semInfo->refCount += 1;

	team_id curTeam = __etk_get_current_beos_team_id();
	sem_info semInfo;
	get_sem_info(sem->Locker, &semInfo);
	if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Locker, curTeam);
	get_sem_info(sem->Cond, &semInfo);
	if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, curTeam);

	_ETK_UNLOCK_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem_by_source(void *data)
{
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	_ETK_LOCK_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return etk_clone_sem(sem->semInfo->name);
	}
	else if(sem->no_clone || sem->semInfo->refCount >= E_MAXUINT32)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return NULL;
	}

	sem->semInfo->refCount += 1;

	_ETK_UNLOCK_SEMAPHORE_();

	return data;
}


static void* etk_create_sem_for_local(eint64 count)
{
	if(count < E_INT64_CONSTANT(0)) return NULL;

	etk_beos_sem_t *sem = new etk_beos_sem_t();

	if(!sem) return NULL;

	if((sem->semInfo = new etk_beos_sem_info()) == NULL ||
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


_IMPEXP_ETK void* etk_create_sem(eint64 count, const char *name, etk_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			etk_create_sem_for_local(count) :
			etk_create_sem_for_IPC(count, name, area_access));
}


_IMPEXP_ETK e_status_t etk_get_sem_info(void *data, etk_sem_info *info)
{
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
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
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	_ETK_LOCK_SEMAPHORE_();

	if(sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return E_ERROR;
	}

	euint32 count = --(sem->semInfo->refCount);

	team_id curTeam = __etk_get_current_beos_team_id();

	if(etk_is_sem_for_IPC(sem))
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

		etk_delete_area(sem->mapping);
	}

	_ETK_UNLOCK_SEMAPHORE_();

	if(!etk_is_sem_for_IPC(sem))
	{
		if(count > 0) return E_OK;

		delete_sem(sem->Locker);
		delete_sem(sem->Cond);
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
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	_ETK_LOCK_SEMAPHORE_();

	if(sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return E_ERROR;
	}

	euint32 count = --(sem->semInfo->refCount);

	team_id curTeam = __etk_get_current_beos_team_id();

	if(etk_is_sem_for_IPC(sem))
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

		etk_delete_area_etc(sem->mapping, no_clone);
	}
	else if(no_clone)
	{
		sem->no_clone = true;
	}

	_ETK_UNLOCK_SEMAPHORE_();

	if(!etk_is_sem_for_IPC(sem))
	{
		if(count > 0) return E_OK;

		delete_sem(sem->Locker);
		delete_sem(sem->Cond);
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
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem) return E_BAD_VALUE;

	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __etk_get_current_beos_team_id());
		_ETK_UNLOCK_SEMAPHORE_();
	}

	etk_lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}
	sem->semInfo->closed = true;

	release_sem(sem->Cond);

	etk_unlock_sem_inter(sem);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_acquire_sem_etc(void *data, eint64 count, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
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

	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __etk_get_current_beos_team_id());
		_ETK_UNLOCK_SEMAPHORE_();
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
	if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
	   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

	e_status_t retval = E_ERROR;

	while(true)
	{
		etk_unlock_sem_inter(sem);

		status_t status = (wait_forever ? acquire_sem(sem->Cond) :
						  acquire_sem_etc(sem->Cond, 1,
							  	  B_ABSOLUTE_TIMEOUT,
								  (bigtime_t)(microseconds_timeout - etk_system_boot_time())));

		etk_lock_sem_inter(sem);

		if(status != B_OK)
		{
//			ETK_DEBUG("[KERNEL]: %s --- %s(%s)failed, error_code: B_GENERAL_ERROR_BASE + 0x%x",
//				  __PRETTY_FUNCTION__, wait_forever ? "acquire_sem" : "acquire_sem_etc", sem->semInfo->name,
//				  status - B_GENERAL_ERROR_BASE);

			if(status == B_WOULD_BLOCK) retval = E_WOULD_BLOCK;
			else if(status == B_TIMED_OUT) retval = E_TIMED_OUT;
			else continue;

			break;
		}

		if(sem->semInfo->count - count >= E_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(etk_get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(etk_get_current_thread_id());
			retval = E_OK;
			break;
		}
		else if(sem->semInfo->closed) break;

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		release_sem(sem->Cond);
	}

	sem->semInfo->acquiringCount -= count;

	if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = E_INT64_CONSTANT(0);
	release_sem(sem->Cond);

	etk_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_ETK e_status_t etk_acquire_sem(void *data)
{
	return etk_acquire_sem_etc(data, E_INT64_CONSTANT(1), E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_release_sem_etc(void *data, eint64 count, euint32 flags)
{
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem || count < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_LOCK_SEMAPHORE_();
		sem_info semInfo;
		get_sem_info(sem->Cond, &semInfo);
		if(semInfo.team == B_SYSTEM_TEAM) set_sem_owner(sem->Cond, __etk_get_current_beos_team_id());
		_ETK_UNLOCK_SEMAPHORE_();
	}

	etk_lock_sem_inter(sem);

	e_status_t retval = E_ERROR;

	if(sem->semInfo->closed == false && (E_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;
		if(flags != E_DO_NOT_RESCHEDULE && sem->semInfo->acquiringCount > E_INT64_CONSTANT(0)) release_sem(sem->Cond);
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
	etk_beos_sem_t *sem = (etk_beos_sem_t*)data;
	if(!sem || !count) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= E_INT64_CONSTANT(0) ?
			sem->semInfo->count : E_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	etk_unlock_sem_inter(sem);

	return E_OK;
}

