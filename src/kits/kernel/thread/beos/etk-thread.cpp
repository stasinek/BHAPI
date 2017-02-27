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
 * File: etk-thread.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS

#include <be/kernel/OS.h>

#include "../kernel/Kernel.h"
#include "../support/SimpleLocker.h"
#include "../support/List.h"
#include "../support/StringMe.h"


typedef struct threadCallback {
	b_thread_func func;
	void* user_data;
} threadCallback;


typedef struct b_beos_thread_t {
	b_int32			priority;
	b_int32			running;
	bool			exited;
	b_status_t		status;
	b_int64			ID;
	threadCallback	callback;
	BList			exit_callbacks;

	sem_id			locker;
	sem_id			cond;

	bool			existent;

	BList			private_threads;
} b_beos_thread_t;


typedef struct b_beos_thread_private_t {
	b_beos_thread_t *thread;
	bool copy;
} b_beos_thread_private_t;


static b_beos_thread_t* __bhapi_create_thread__()
{
	b_beos_thread_t *thread = new b_beos_thread_t;
	if(thread == NULL) return NULL;

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = B_OK;
	thread->ID = B_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;
	thread->locker = create_sem(1, NULL);
	thread->cond = create_sem(0, NULL);
	thread->existent = false;

	if(thread->locker < 0 || thread->cond < 0)
	{
		if(thread->locker >= 0) delete_sem(thread->locker);
		if(thread->cond >= 0) delete_sem(thread->cond);
		delete thread;
		return NULL;
	}

	return thread;
}


static void b_delete_thread(b_beos_thread_t *thread)
{
	if(thread == NULL) return;

	b_beos_thread_private_t *priThread;
	while((priThread = (b_beos_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	delete_sem(thread->locker);
	delete_sem(thread->cond);

	delete thread;
}


static BSimpleLocker __bhapi_thread_locker__;
#define BHAPI_LOCK_THREAD()	__bhapi_thread_locker__.Lock()
#define BHAPI_UNLOCK_THREAD()	__bhapi_thread_locker__.Unlock()


class EThreadsList {
public:
	BList fList;

	EThreadsList()
	{
	}

	~EThreadsList()
	{
		b_beos_thread_t *td;
		while((td = (b_beos_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			BHAPI_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			b_delete_thread(td);
		}
	}

	b_beos_thread_private_t* AddThread(b_beos_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		b_beos_thread_private_t *priThread = new b_beos_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			fList.RemoveItem((void*)td);
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = false;
		return priThread;
	}

	b_beos_thread_private_t* RefThread(b_beos_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		b_beos_thread_private_t *priThread = new b_beos_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	b_int32 UnrefThread(b_beos_thread_private_t *priThread)
	{
		b_beos_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		b_int32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	b_beos_thread_private_t* OpenThread(b_int64 tid)
	{
		if(tid == B_INT64_CONSTANT(0)) return NULL;
		for(b_int32 i = 0; i < fList.CountItems(); i++)
		{
			b_beos_thread_t* td = (b_beos_thread_t*)fList.ItemAt(i);
			if(td->ID == tid) return RefThread(td);
		}
		return NULL;
	}
};


static EThreadsList __bhapi_thread_lists__;
#define _BHAPI_ADD_THREAD_(td)	__bhapi_thread_lists__.AddThread(td)
#define _BHAPI_REF_THREAD_(td)	__bhapi_thread_lists__.RefThread(td)
#define _BHAPI_UNREF_THREAD_(td)	__bhapi_thread_lists__.UnrefThread(td)
#define _BHAPI_OPEN_THREAD_(tid)	__bhapi_thread_lists__.OpenThread(tid)


IMPEXP_BHAPI b_int64 b_get_current_thread_id(void)
{
	return((b_int64)find_thread(NULL));
}


static void b_lock_thread_inter(b_beos_thread_t *thread)
{
	acquire_sem(thread->locker);
}


static void b_unlock_thread_inter(b_beos_thread_t *thread)
{
	release_sem(thread->locker);
}


int32 b_spawn_thread_func(void *data)
{
	b_beos_thread_t *thread = (b_beos_thread_t*)data;
	b_beos_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = _BHAPI_REF_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();
		return 0;
	}
	BHAPI_UNLOCK_THREAD();

	if(b_on_exit_thread((void (*)(void *))b_delete_thread, priThread) != B_OK)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		b_lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;
		thread->callback.func = NULL;

		while(true)
		{
			int32 semCount = 0;
			if(get_sem_count(thread->cond, &semCount) != B_OK) break;
			if(semCount > 0) break;
			release_sem(thread->cond);
		}

		BList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		b_unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		b_delete_thread(priThread);

		return 0;
	}

	b_lock_thread_inter(thread);
	b_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	b_unlock_thread_inter(thread);

	b_status_t status = (threadFunc == NULL ? B_ERROR : (*threadFunc)(userData));

	b_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	while(true)
	{
		int32 semCount = 0;
		if(get_sem_count(thread->cond, &semCount) != B_OK) break;
		if(semCount > 0) break;
		release_sem(thread->cond);
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	b_unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return 0;
}


IMPEXP_BHAPI void* b_create_thread_by_current_thread(void)
{
	b_beos_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = _BHAPI_OPEN_THREAD_(b_get_current_thread_id())) != NULL)
	{
		_BHAPI_UNREF_THREAD_(priThread);
		BHAPI_UNLOCK_THREAD();
		return NULL;
	}

	b_beos_thread_t *thread = __bhapi_create_thread__();
	if(thread == NULL) return NULL;

	if((priThread = _BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();
		b_delete_thread(thread);
		return NULL;
	}

	thread->priority = 0;
	thread->running = 1;
	thread->exited = false;
	thread->ID = b_get_current_thread_id();
	thread->existent = true;

	BHAPI_UNLOCK_THREAD();

	return (void*)priThread;
}


IMPEXP_BHAPI void* b_create_thread(b_thread_func threadFunction,
				    b_int32 priority,
				    void *arg,
				    b_int64 *threadId)
{
	if(threadFunction == NULL) return NULL;

	b_beos_thread_t *thread = __bhapi_create_thread__();
	if(thread == NULL) return NULL;

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

	thread_id beThreadId;
	if((beThreadId = spawn_thread(b_spawn_thread_func, NULL, priority, (void*)thread)) < 0)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- spawn_thread failed.", __PRETTY_FUNCTION__);

		b_delete_thread(thread);
		return NULL;
	}

	b_beos_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = _BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();

		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		thread->callback.func = NULL;
		resume_thread(beThreadId);
		status_t beStatus;
		wait_for_thread(beThreadId, &beStatus);

		b_delete_thread(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = (b_int64)beThreadId;
	thread->existent = false;

	BHAPI_UNLOCK_THREAD();

	b_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


IMPEXP_BHAPI void* b_open_thread(b_int64 threadId)
{
	BHAPI_LOCK_THREAD();
	b_beos_thread_private_t *priThread = _BHAPI_OPEN_THREAD_(threadId);
	BHAPI_UNLOCK_THREAD();

	return (void*)priThread;
}


IMPEXP_BHAPI b_status_t b_delete_thread(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(priThread == NULL || thread == NULL) return B_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	BHAPI_LOCK_THREAD();
	b_int32 count = _BHAPI_UNREF_THREAD_(priThread);
	BHAPI_UNLOCK_THREAD();

	if(count < 0) return B_ERROR;

	if(thread->existent && !threadIsCopy)
	{
		BList exitCallbackList;

		b_lock_thread_inter(thread);
		if(thread->ID == b_get_current_thread_id())
		{
			thread->running = 0;
			thread->exited = true;
			thread->status = B_OK;

			while(true)
			{
				int32 semCount = 0;
				if(get_sem_count(thread->cond, &semCount) != B_OK) break;
				if(semCount > 0) break;
				release_sem(thread->cond);
			}

			exitCallbackList = thread->exit_callbacks;
			thread->exit_callbacks.MakeEmpty();
		}
		b_unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
		{
			if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if(count > 0) return B_OK;

	if(thread->ID != b_get_current_thread_id() && thread->existent == false)
	{
		if(thread->callback.func)
		{
			thread->callback.func = NULL;
			resume_thread((thread_id)thread->ID);
		}

		status_t beStatus;
		wait_for_thread((thread_id)thread->ID, &beStatus);
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	b_delete_thread(thread);

	return B_OK;
}


IMPEXP_BHAPI b_status_t b_resume_thread(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	b_status_t retVal = B_ERROR;

	b_lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		resume_thread((thread_id)thread->ID);
		thread->running = 1;
		retVal = B_OK;
	}
	b_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_status_t b_suspend_thread(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	b_status_t retVal = B_ERROR;

	b_lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == b_get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			b_unlock_thread_inter(thread);

			retVal = suspend_thread((thread_id)thread->ID) != B_OK ? B_ERROR : B_OK;

			b_lock_thread_inter(thread);
			thread->running = 1;
		}
		else if(suspend_thread((thread_id)thread->ID) == B_OK)
		{
			thread->running = 2;
			retVal = B_OK;
		}
	}
	b_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_int64 b_get_thread_id(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_INT64_CONSTANT(0);

	b_lock_thread_inter(thread);
	b_int64 thread_id = thread->ID;
	b_unlock_thread_inter(thread);

	return thread_id;
}


IMPEXP_BHAPI b_uint32 b_get_thread_run_state(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return BHAPI_THREAD_INVALID;

	b_uint32 retVal = BHAPI_THREAD_INVALID;

	b_lock_thread_inter(thread);

	if(thread->exited)
	{
		if(thread->running == 0)
			retVal = BHAPI_THREAD_EXITED;
	}
	else switch(thread->running)
	{
		case 0:
			retVal = BHAPI_THREAD_READY;
			break;

		case 1:
			retVal = BHAPI_THREAD_RUNNING;
			break;

		case 2:
			retVal = BHAPI_THREAD_SUSPENDED;
			break;

		default:
			break;
	}

	b_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_status_t b_set_thread_priority(void *data, b_int32 new_priority)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;
	if(new_priority < 0) new_priority = 15;

	b_lock_thread_inter(thread);
	if(set_thread_priority((thread_id)(thread->ID), new_priority) < 0)
	{
		b_unlock_thread_inter(thread);
		return B_ERROR;
	}
	b_int32 old_priority = thread->priority;
	thread->priority = new_priority;
	b_unlock_thread_inter(thread);

	return old_priority;
}


IMPEXP_BHAPI b_int32 b_get_thread_priority(void *data)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	b_lock_thread_inter(thread);
	b_int32 priority = thread->priority;
	b_unlock_thread_inter(thread);

	return priority;
}


IMPEXP_BHAPI b_status_t b_on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return B_BAD_VALUE;

	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)b_open_thread(b_get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	b_beos_thread_t *thread = priThread->thread;

	threadCallback *exitCallback = new threadCallback;
	if(exitCallback == NULL)
	{
		b_delete_thread(priThread);
		return B_NO_MEMORY;
	}

	exitCallback->func = (b_thread_func)callback;
	exitCallback->user_data = user_data;

	b_status_t retVal = B_OK;

	b_lock_thread_inter(thread);
	if(thread->exited || thread->exit_callbacks.AddItem((void*)exitCallback, 0) == false)
	{
		delete exitCallback;
		retVal = B_ERROR;
	}
	b_unlock_thread_inter(thread);

	b_delete_thread(priThread);

	return retVal;
}


IMPEXP_BHAPI void b_exit_thread(b_status_t status)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)b_open_thread(b_get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	b_beos_thread_t *thread = priThread->thread;

	b_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	while(true)
	{
		int32 semCount = 0;
		if(get_sem_count(thread->cond, &semCount) != B_OK) break;
		if(semCount > 0) break;
		release_sem(thread->cond);
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	b_unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	b_delete_thread(priThread);

	exit_thread(0);
}


IMPEXP_BHAPI b_status_t b_wait_for_thread_etc(void *data, b_status_t *thread_return_value, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_beos_thread_private_t *priThread = (b_beos_thread_private_t*)data;
	b_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL || microseconds_timeout < B_INT64_CONSTANT(0) || thread_return_value == NULL) return B_BAD_VALUE;

	b_bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	b_lock_thread_inter(thread);

	if(thread->ID == b_get_current_thread_id())
	{
		BHAPI_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		b_unlock_thread_inter(thread);
		return B_ERROR;
	}
	else if(thread->exited)
	{
		b_unlock_thread_inter(thread);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		b_unlock_thread_inter(thread);
		return B_WOULD_BLOCK;
	}

	if(thread->callback.func || thread->running == 2) resume_thread((thread_id)thread->ID);

	b_unlock_thread_inter(thread);

	b_status_t retval = B_OK;

	status_t status = (wait_forever ? acquire_sem(thread->cond) :
					  acquire_sem_etc(thread->cond, 1,
						  	  B_ABSOLUTE_TIMEOUT,
							  (bigtime_t)(microseconds_timeout - b_system_boot_time())));

	b_lock_thread_inter(thread);

	if(status != B_OK)
	{
		b_unlock_thread_inter(thread);

		if(status == B_WOULD_BLOCK) retval = B_WOULD_BLOCK;
		else if(status == B_TIMED_OUT) retval = B_TIMED_OUT;
		else retval = B_ERROR;
	}
	else
	{
		retval = B_OK;

		if(thread->existent == false)
		{
			status_t beStatus;
			wait_for_thread((thread_id)thread->ID, &beStatus);
		}

		if(thread->exited)
			*thread_return_value = thread->status;
		else
			retval = B_ERROR;

		b_unlock_thread_inter(thread);
	}

	return retval;
}


IMPEXP_BHAPI b_status_t b_wait_for_thread(void *data, b_status_t *thread_return_value)
{
	return b_wait_for_thread_etc(data, thread_return_value, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


IMPEXP_BHAPI b_status_t b_snooze(b_bigtime_t microseconds)
{
	if(microseconds <= 0) return B_ERROR;

	if(snooze(microseconds) == B_OK) return B_OK;
	else return B_ERROR;
}


IMPEXP_BHAPI b_status_t b_snooze_until(b_bigtime_t time, int timebase)
{
	if(time < B_INT64_CONSTANT(0)) return B_ERROR;

	int btimebase = -1;
	switch(timebase)
	{
		case B_SYSTEM_TIMEBASE:
			time += b_system_boot_time();
			btimebase = B_SYSTEM_TIMEBASE;
			break;

		case B_REAL_TIME_TIMEBASE:
			break;

		default:
			return B_ERROR;
	}

	if(snooze_until(time, btimebase) == B_OK) return B_OK;
	else return B_ERROR;
}


IMPEXP_BHAPI b_int64 b_get_current_team_id(void)
{
	thread_info threadInfo;
	if(get_thread_info(find_thread(NULL), &threadInfo) != B_OK) return B_INT64_CONSTANT(0);
	return (b_int64)threadInfo.team;
}
#endif


