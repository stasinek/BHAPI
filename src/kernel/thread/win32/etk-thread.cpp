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

#include <stdio.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <process.h>

#include "./../config.h>
#include "./../kernel/Kernel.h>
#include "./../support/List.h>
#include "./../support/String.h>
#include "./../support/SimpleLocker.h>

#define SECS_BETWEEN_EPOCHS	E_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS		E_INT64_CONSTANT(10000000)


typedef struct _threadCallback_ {
	e_thread_func	func;
	void		*user_data;
} _threadCallback_;


typedef struct etk_win32_thread_t {
	HANDLE			handle;

	eint32			priority;
	eint32			running;
	bool			exited;
	e_status_t		status;
	eint64			ID;
	_threadCallback_	callback;
	EList			exit_callbacks;

	CRITICAL_SECTION	locker;
	HANDLE			cond;

	bool			existent;

	EList			private_threads;
} etk_win32_thread_t;


typedef struct etk_win32_thread_private_t {
	etk_win32_thread_t *thread;
	bool copy;
} etk_win32_thread_private_t;


static etk_win32_thread_t* __etk_create_thread__()
{
	etk_win32_thread_t *thread = new etk_win32_thread_t;
	if(thread == NULL) return NULL;

	thread->handle = NULL;
	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = E_OK;
	thread->ID = E_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;
	thread->cond = NULL;
	thread->existent = false;

	InitializeCriticalSection(&(thread->locker));

	return thread;
}


static void __etk_delete_thread__(etk_win32_thread_t *thread)
{
	if(thread == NULL) return;

	etk_win32_thread_private_t *priThread;
	while((priThread = (etk_win32_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	if(thread->handle != NULL) CloseHandle(thread->handle);
	if(thread->cond != NULL) CloseHandle(thread->cond);

	DeleteCriticalSection(&(thread->locker));

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
		etk_win32_thread_t *td;
		while((td = (etk_win32_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			ETK_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			__etk_delete_thread__(td);
		}
	}

	etk_win32_thread_private_t* AddThread(etk_win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		etk_win32_thread_private_t *priThread = new etk_win32_thread_private_t;
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

	etk_win32_thread_private_t* RefThread(etk_win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		etk_win32_thread_private_t *priThread = new etk_win32_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	eint32 UnrefThread(etk_win32_thread_private_t *priThread)
	{
		etk_win32_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		eint32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	etk_win32_thread_private_t* OpenThread(eint64 tid)
	{
		if(tid == E_INT64_CONSTANT(0)) return NULL;
		for(eint32 i = 0; i < fList.CountItems(); i++)
		{
			etk_win32_thread_t* td = (etk_win32_thread_t*)fList.ItemAt(i);
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
	return((eint64)GetCurrentThreadId());
}


static void etk_lock_thread_inter(etk_win32_thread_t *thread)
{
	EnterCriticalSection(&(thread->locker));
}


static void etk_unlock_thread_inter(etk_win32_thread_t *thread)
{
	LeaveCriticalSection(&(thread->locker));
}


#ifndef ETK_OS_CYGWIN 
unsigned _stdcall etk_spawn_thread_func(void *data)
#else
DWORD WINAPI etk_spawn_thread_func(void *data)
#endif
{
	etk_win32_thread_t *thread = (etk_win32_thread_t*)data;
	etk_win32_thread_private_t *priThread = NULL;

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

		SetEvent(thread->cond);

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
	SetEvent(thread->cond);

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
	etk_win32_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_OPEN_THREAD_(etk_get_current_thread_id())) != NULL)
	{
		_ETK_UNREF_THREAD_(priThread);
		_ETK_UNLOCK_THREAD_();
		return NULL;
	}

	etk_win32_thread_t *thread = __etk_create_thread__();
	if(thread == NULL || (thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		if(thread) __etk_delete_thread__(thread);
		return NULL;
	}

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

	etk_win32_thread_t *thread = __etk_create_thread__();
	if(thread == NULL) return NULL;
	if((thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		__etk_delete_thread__(thread);
		return NULL;
	}

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

#ifndef ETK_OS_CYGWIN 
	unsigned winThreadId;
	if((thread->handle = (HANDLE)_beginthreadex(NULL, 0x40000, etk_spawn_thread_func, (void*)thread,
						    CREATE_SUSPENDED, &winThreadId)) == NULL)
#else
	DWORD winThreadId;
	if((thread->handle = CreateThread(NULL, 0x40000, etk_spawn_thread_func, (void*)thread,
					  CREATE_SUSPENDED, &winThreadId)) == NULL)
#endif
	{
		ETK_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		__etk_delete_thread__(thread);
		return NULL;
	}

	etk_win32_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_ADD_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();

		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		thread->callback.func = NULL;
		ResumeThread(thread->handle);
		WaitForSingleObject(thread->handle, INFINITE);

		__etk_delete_thread__(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = (eint64)winThreadId;
	thread->existent = false;

	_ETK_UNLOCK_THREAD_();

	etk_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


_IMPEXP_ETK void* etk_open_thread(eint64 threadId)
{
	_ETK_LOCK_THREAD_();
	etk_win32_thread_private_t *priThread = _ETK_OPEN_THREAD_(threadId);
	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


_IMPEXP_ETK e_status_t etk_delete_thread(void *data)
{
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

			SetEvent(thread->cond);

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
			ResumeThread(thread->handle);
		}
		WaitForSingleObject(thread->handle, INFINITE);
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
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_BAD_VALUE;

	e_status_t retVal = E_ERROR;

	etk_lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) ||  thread->running == 2) &&
	   thread->exited == false)
	{
		ResumeThread(thread->handle);
		thread->running = 1;
		retVal = E_OK;
	}
	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_suspend_thread(void *data)
{
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

			retVal = SuspendThread(thread->handle) == (DWORD)-1 ? E_ERROR : E_OK;

			etk_lock_thread_inter(thread);
			thread->running = 1;
		}
		else if(SuspendThread(thread->handle) != (DWORD)-1)
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
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_INT64_CONSTANT(0);

	etk_lock_thread_inter(thread);
	eint64 thread_id = thread->ID;
	etk_unlock_thread_inter(thread);

	return thread_id;
}


_IMPEXP_ETK euint32 etk_get_thread_run_state(void *data)
{
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;
	if(new_priority < 0) new_priority = 15;

	int win32_priority;

	if(new_priority <= 5) win32_priority = THREAD_PRIORITY_LOWEST;
	else if(new_priority <= 15) win32_priority = THREAD_PRIORITY_NORMAL;
	else if(new_priority <= 99) win32_priority = THREAD_PRIORITY_ABOVE_NORMAL;
	else win32_priority = THREAD_PRIORITY_HIGHEST;

	etk_lock_thread_inter(thread);
	if(thread->exited || SetThreadPriority(thread->handle, win32_priority) == 0)
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
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	etk_lock_thread_inter(thread);
	eint32 priority = thread->priority;
	etk_unlock_thread_inter(thread);

	return priority;
}


_IMPEXP_ETK e_status_t etk_on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return E_BAD_VALUE;

	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	etk_win32_thread_t *thread = priThread->thread;

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
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	etk_win32_thread_t *thread = priThread->thread;

	etk_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	SetEvent(thread->cond);

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

#ifndef ETK_OS_CYGWIN 
	_endthreadex(0);
#else
	ExitThread(0);
#endif
}


_IMPEXP_ETK e_status_t etk_wait_for_thread_etc(void *data, e_status_t *thread_return_value, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_win32_thread_private_t *priThread = (etk_win32_thread_private_t*)data;
	etk_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

	if(thread->callback.func || thread->running == 2) ResumeThread(thread->handle);

	HANDLE handles[2] = {NULL, NULL};
	handles[0] = thread->cond;
	if(!wait_forever)
	{
		HANDLE timer = NULL;
		LARGE_INTEGER due;
		due.QuadPart = microseconds_timeout * E_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
		timer = CreateWaitableTimer(NULL, TRUE, NULL);

		if(!timer || SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
		{
			if(timer) CloseHandle(timer);
			etk_unlock_thread_inter(thread);
			return E_ERROR;
		}

		handles[1] = timer;
	}

	etk_unlock_thread_inter(thread);

	e_status_t retVal = E_OK;

	DWORD status = (handles[1] == NULL ?
				WaitForSingleObject(handles[0], INFINITE) :
				WaitForMultipleObjects(2, handles, FALSE, INFINITE));

	if(handles[1] != NULL)
	{
		CloseHandle(handles[1]);

		if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
		{
			retVal = E_TIMED_OUT;
		}
		else if(status != WAIT_OBJECT_0)
		{
			retVal = E_ERROR;
		}
	}
	else if(status != WAIT_OBJECT_0)
	{
		retVal = E_ERROR;
	}

	if(retVal == E_OK)
	{
		etk_lock_thread_inter(thread);

		if(thread->existent == false) WaitForSingleObject(thread->handle, INFINITE);

		if(thread->exited)
			*thread_return_value = thread->status;
		else
			retVal = E_ERROR;

		etk_unlock_thread_inter(thread);
	}

	return retVal;
}


_IMPEXP_ETK e_status_t etk_wait_for_thread(void *data, e_status_t *thread_return_value)
{
	return etk_wait_for_thread_etc(data, thread_return_value, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_snooze(e_bigtime_t microseconds)
{
	if(microseconds < E_INT64_CONSTANT(0)) return E_ERROR;

	HANDLE timer = NULL;
	LARGE_INTEGER due;

	due.QuadPart = -microseconds * E_INT64_CONSTANT(10);
	timer = CreateWaitableTimer(NULL, TRUE, NULL);

	if(timer == NULL) return E_ERROR;
	if(!SetWaitableTimer(timer, &due, 0, NULL, NULL, 0))
	{
		CloseHandle(timer);
		return E_ERROR;
	}

	DWORD status = WaitForSingleObject(timer, INFINITE);

	CloseHandle(timer);

	if(status != WAIT_OBJECT_0)
		return E_ERROR;
	else
		return E_OK;
}


_IMPEXP_ETK e_status_t etk_snooze_until(e_bigtime_t time, int timebase)
{
	if(time < E_INT64_CONSTANT(0)) return E_ERROR;

	switch(timebase)
	{
		case E_SYSTEM_TIMEBASE:
			time += etk_system_boot_time();
			break;

		case E_REAL_TIME_TIMEBASE:
			break;

		default:
			return E_ERROR;
	}

	HANDLE timer = NULL;
	LARGE_INTEGER due;

	due.QuadPart = time * E_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
	timer = CreateWaitableTimer(NULL, TRUE, NULL);

	if(timer == NULL) return E_ERROR;
	if(!SetWaitableTimer(timer, &due, 0, NULL, NULL, 0))
	{
		CloseHandle(timer);
		return E_ERROR;
	}

	DWORD status = WaitForSingleObject(timer, INFINITE);

	CloseHandle(timer);

	if(status != WAIT_OBJECT_0)
		return E_ERROR;
	else
		return E_OK;
}


_IMPEXP_ETK eint64 etk_get_current_team_id(void)
{
	return((eint64)GetCurrentProcessId());
}

