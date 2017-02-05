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
 * File: etk-thread.cpp
 *
 * --------------------------------------------------------------------------*/

#include <be/kernel/OS.h>

#include "./../kernel/Kernel.h>
#include "./../support/SimpleLocker.h>
#include "./../support/List.h>
#include "./../support/String.h>


typedef struct _threadCallback_ {
	e_thread_func func;
	void* user_data;
} _threadCallback_;


typedef struct etk_beos_thread_t {
	eint32			priority;
	eint32			running;
	bool			exited;
	e_status_t		status;
	eint64			ID;
	_threadCallback_	callback;
	EList			exit_callbacks;

	sem_id			locker;
	sem_id			cond;

	bool			existent;

	EList			private_threads;
} etk_beos_thread_t;


typedef struct etk_beos_thread_private_t {
	etk_beos_thread_t *thread;
	bool copy;
} etk_beos_thread_private_t;


static etk_beos_thread_t* __etk_create_thread__()
{
	etk_beos_thread_t *thread = new etk_beos_thread_t;
	if(thread == NULL) return NULL;

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = E_OK;
	thread->ID = E_INT64_CONSTANT(0);
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


static void __etk_delete_thread__(etk_beos_thread_t *thread)
{
	if(thread == NULL) return;

	etk_beos_thread_private_t *priThread;
	while((priThread = (etk_beos_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	delete_sem(thread->locker);
	delete_sem(thread->cond);

	delete thread;
}


static ESimpleLocker __etk_thread_locker__;
#define _ETK_LOCK_THREAD_()	__etk_thread_locker__.Lock()
#define _ETK_UNLOCK_THREAD_()	__etk_thread_locker__.Unlock()


class EThreadsList {
public:
	EList fList;

	EThreadsList()
	{
	}

	~EThreadsList()
	{
		etk_beos_thread_t *td;
		while((td = (etk_beos_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			ETK_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			__etk_delete_thread__(td);
		}
	}

	etk_beos_thread_private_t* AddThread(etk_beos_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		etk_beos_thread_private_t *priThread = new etk_beos_thread_private_t;
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

	etk_beos_thread_private_t* RefThread(etk_beos_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		etk_beos_thread_private_t *priThread = new etk_beos_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	eint32 UnrefThread(etk_beos_thread_private_t *priThread)
	{
		etk_beos_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		eint32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	etk_beos_thread_private_t* OpenThread(eint64 tid)
	{
		if(tid == E_INT64_CONSTANT(0)) return NULL;
		for(eint32 i = 0; i < fList.CountItems(); i++)
		{
			etk_beos_thread_t* td = (etk_beos_thread_t*)fList.ItemAt(i);
			if(td->ID == tid) return RefThread(td);
		}
		return NULL;
	}
};


static EThreadsList __etk_thread_lists__;
#define _ETK_ADD_THREAD_(td)	__etk_thread_lists__.AddThread(td)
#define _ETK_REF_THREAD_(td)	__etk_thread_lists__.RefThread(td)
#define _ETK_UNREF_THREAD_(td)	__etk_thread_lists__.UnrefThread(td)
#define _ETK_OPEN_THREAD_(tid)	__etk_thread_lists__.OpenThread(tid)


_IMPEXP_ETK eint64 etk_get_current_thread_id(void)
{
	return((eint64)find_thread(NULL));
}


static void etk_lock_thread_inter(etk_beos_thread_t *thread)
{
	acquire_sem(thread->locker);
}


static void etk_unlock_thread_inter(etk_beos_thread_t *thread)
{
	release_sem(thread->locker);
}


int32 etk_spawn_thread_func(void *data)
{
	etk_beos_thread_t *thread = (etk_beos_thread_t*)data;
	etk_beos_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_REF_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();
		return 0;
	}
	_ETK_UNLOCK_THREAD_();

	if(etk_on_exit_thread((void (*)(void *))etk_delete_thread, priThread) != E_OK)
	{
		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		etk_lock_thread_inter(thread);

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

		EList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		etk_unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		etk_delete_thread(priThread);

		return 0;
	}

	etk_lock_thread_inter(thread);
	e_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	etk_unlock_thread_inter(thread);

	e_status_t status = (threadFunc == NULL ? E_ERROR : (*threadFunc)(userData));

	etk_lock_thread_inter(thread);

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

	EList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	etk_unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return 0;
}


_IMPEXP_ETK void* etk_create_thread_by_current_thread(void)
{
	etk_beos_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_OPEN_THREAD_(etk_get_current_thread_id())) != NULL)
	{
		_ETK_UNREF_THREAD_(priThread);
		_ETK_UNLOCK_THREAD_();
		return NULL;
	}

	etk_beos_thread_t *thread = __etk_create_thread__();
	if(thread == NULL) return NULL;

	if((priThread = _ETK_ADD_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();
		__etk_delete_thread__(thread);
		return NULL;
	}

	thread->priority = 0;
	thread->running = 1;
	thread->exited = false;
	thread->ID = etk_get_current_thread_id();
	thread->existent = true;

	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


_IMPEXP_ETK void* etk_create_thread(e_thread_func threadFunction,
				    eint32 priority,
				    void *arg,
				    eint64 *threadId)
{
	if(threadFunction == NULL) return NULL;

	etk_beos_thread_t *thread = __etk_create_thread__();
	if(thread == NULL) return NULL;

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

	thread_id beThreadId;
	if((beThreadId = spawn_thread(etk_spawn_thread_func, NULL, priority, (void*)thread)) < 0)
	{
		ETK_DEBUG("[KERNEL]: %s --- spawn_thread failed.", __PRETTY_FUNCTION__);

		__etk_delete_thread__(thread);
		return NULL;
	}

	etk_beos_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_ADD_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();

		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		thread->callback.func = NULL;
		resume_thread(beThreadId);
		status_t beStatus;
		wait_for_thread(beThreadId, &beStatus);

		__etk_delete_thread__(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = (eint64)beThreadId;
	thread->existent = false;

	_ETK_UNLOCK_THREAD_();

	etk_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


_IMPEXP_ETK void* etk_open_thread(eint64 threadId)
{
	_ETK_LOCK_THREAD_();
	etk_beos_thread_private_t *priThread = _ETK_OPEN_THREAD_(threadId);
	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


_IMPEXP_ETK e_status_t etk_delete_thread(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(priThread == NULL || thread == NULL) return E_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	_ETK_LOCK_THREAD_();
	eint32 count = _ETK_UNREF_THREAD_(priThread);
	_ETK_UNLOCK_THREAD_();

	if(count < 0) return E_ERROR;

	if(thread->existent && !threadIsCopy)
	{
		EList exitCallbackList;

		etk_lock_thread_inter(thread);
		if(thread->ID == etk_get_current_thread_id())
		{
			thread->running = 0;
			thread->exited = true;
			thread->status = E_OK;

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
		etk_unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
		{
			if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if(count > 0) return E_OK;

	if(thread->ID != etk_get_current_thread_id() && thread->existent == false)
	{
		if(thread->callback.func)
		{
			thread->callback.func = NULL;
			resume_thread((thread_id)thread->ID);
		}

		status_t beStatus;
		wait_for_thread((thread_id)thread->ID, &beStatus);
	}

	EList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	__etk_delete_thread__(thread);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_resume_thread(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_BAD_VALUE;

	e_status_t retVal = E_ERROR;

	etk_lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		resume_thread((thread_id)thread->ID);
		thread->running = 1;
		retVal = E_OK;
	}
	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_suspend_thread(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_BAD_VALUE;

	e_status_t retVal = E_ERROR;

	etk_lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == etk_get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			etk_unlock_thread_inter(thread);

			retVal = suspend_thread((thread_id)thread->ID) != B_OK ? E_ERROR : E_OK;

			etk_lock_thread_inter(thread);
			thread->running = 1;
		}
		else if(suspend_thread((thread_id)thread->ID) == B_OK)
		{
			thread->running = 2;
			retVal = E_OK;
		}
	}
	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK eint64 etk_get_thread_id(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_INT64_CONSTANT(0);

	etk_lock_thread_inter(thread);
	eint64 thread_id = thread->ID;
	etk_unlock_thread_inter(thread);

	return thread_id;
}


_IMPEXP_ETK euint32 etk_get_thread_run_state(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return ETK_THREAD_INVALID;

	euint32 retVal = ETK_THREAD_INVALID;

	etk_lock_thread_inter(thread);

	if(thread->exited)
	{
		if(thread->running == 0)
			retVal = ETK_THREAD_EXITED;
	}
	else switch(thread->running)
	{
		case 0:
			retVal = ETK_THREAD_READY;
			break;

		case 1:
			retVal = ETK_THREAD_RUNNING;
			break;

		case 2:
			retVal = ETK_THREAD_SUSPENDED;
			break;

		default:
			break;
	}

	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_set_thread_priority(void *data, eint32 new_priority)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;
	if(new_priority < 0) new_priority = 15;

	etk_lock_thread_inter(thread);
	if(set_thread_priority((thread_id)(thread->ID), new_priority) < 0)
	{
		etk_unlock_thread_inter(thread);
		return E_ERROR;
	}
	eint32 old_priority = thread->priority;
	thread->priority = new_priority;
	etk_unlock_thread_inter(thread);

	return old_priority;
}


_IMPEXP_ETK eint32 etk_get_thread_priority(void *data)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	etk_lock_thread_inter(thread);
	eint32 priority = thread->priority;
	etk_unlock_thread_inter(thread);

	return priority;
}


_IMPEXP_ETK e_status_t etk_on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return E_BAD_VALUE;

	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	etk_beos_thread_t *thread = priThread->thread;

	_threadCallback_ *exitCallback = new _threadCallback_;
	if(exitCallback == NULL)
	{
		etk_delete_thread(priThread);
		return E_NO_MEMORY;
	}

	exitCallback->func = (e_thread_func)callback;
	exitCallback->user_data = user_data;

	e_status_t retVal = E_OK;

	etk_lock_thread_inter(thread);
	if(thread->exited || thread->exit_callbacks.AddItem((void*)exitCallback, 0) == false)
	{
		delete exitCallback;
		retVal = E_ERROR;
	}
	etk_unlock_thread_inter(thread);

	etk_delete_thread(priThread);

	return retVal;
}


_IMPEXP_ETK void etk_exit_thread(e_status_t status)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	etk_beos_thread_t *thread = priThread->thread;

	etk_lock_thread_inter(thread);

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

	EList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	etk_unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	etk_delete_thread(priThread);

	exit_thread(0);
}


_IMPEXP_ETK e_status_t etk_wait_for_thread_etc(void *data, e_status_t *thread_return_value, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_beos_thread_private_t *priThread = (etk_beos_thread_private_t*)data;
	etk_beos_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL || microseconds_timeout < E_INT64_CONSTANT(0) || thread_return_value == NULL) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_thread_inter(thread);

	if(thread->ID == etk_get_current_thread_id())
	{
		ETK_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		etk_unlock_thread_inter(thread);
		return E_ERROR;
	}
	else if(thread->exited)
	{
		etk_unlock_thread_inter(thread);
		return E_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_thread_inter(thread);
		return E_WOULD_BLOCK;
	}

	if(thread->callback.func || thread->running == 2) resume_thread((thread_id)thread->ID);

	etk_unlock_thread_inter(thread);

	e_status_t retval = E_OK;

	status_t status = (wait_forever ? acquire_sem(thread->cond) :
					  acquire_sem_etc(thread->cond, 1,
						  	  B_ABSOLUTE_TIMEOUT,
							  (bigtime_t)(microseconds_timeout - etk_system_boot_time())));

	etk_lock_thread_inter(thread);

	if(status != B_OK)
	{
		etk_unlock_thread_inter(thread);

		if(status == B_WOULD_BLOCK) retval = E_WOULD_BLOCK;
		else if(status == B_TIMED_OUT) retval = E_TIMED_OUT;
		else retval = E_ERROR;
	}
	else
	{
		retval = E_OK;

		if(thread->existent == false)
		{
			status_t beStatus;
			wait_for_thread((thread_id)thread->ID, &beStatus);
		}

		if(thread->exited)
			*thread_return_value = thread->status;
		else
			retval = E_ERROR;

		etk_unlock_thread_inter(thread);
	}

	return retval;
}


_IMPEXP_ETK e_status_t etk_wait_for_thread(void *data, e_status_t *thread_return_value)
{
	return etk_wait_for_thread_etc(data, thread_return_value, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_snooze(e_bigtime_t microseconds)
{
	if(microseconds <= 0) return E_ERROR;

	if(snooze(microseconds) == B_OK) return E_OK;
	else return E_ERROR;
}


_IMPEXP_ETK e_status_t etk_snooze_until(e_bigtime_t time, int timebase)
{
	if(time < E_INT64_CONSTANT(0)) return E_ERROR;

	int btimebase = -1;
	switch(timebase)
	{
		case E_SYSTEM_TIMEBASE:
			time += etk_system_boot_time();
			btimebase = B_SYSTEM_TIMEBASE;
			break;

		case E_REAL_TIME_TIMEBASE:
			break;

		default:
			return E_ERROR;
	}

	if(snooze_until(time, btimebase) == B_OK) return E_OK;
	else return E_ERROR;
}


_IMPEXP_ETK eint64 etk_get_current_team_id(void)
{
	thread_info threadInfo;
	if(get_thread_info(find_thread(NULL), &threadInfo) != B_OK) return E_INT64_CONSTANT(0);
	return (eint64)threadInfo.team;
}

