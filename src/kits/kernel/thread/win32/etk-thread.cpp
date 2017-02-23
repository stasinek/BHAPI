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
#ifdef BHAPI_OS_WIN32

#include "../../../kernel/Kernel.h"
#include "../../../kernel/Debug.h"
#include "../../../support/List.h"
#include "../../../support/StringMe.h"
#include "../../../support/Errors.h"
#include "../../../support/SimpleLocker.h"
#include "../../../support/SupportDefs.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>

#define SECS_BETWEEN_EPOCHS    B_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS	    B_INT64_CONSTANT(10000000)


typedef struct _threadCallback_ {
	b_thread_func	func;
	void		*user_data;
} _threadCallback_;


typedef struct bhapi_win32_thread_t {
	HANDLE			handle;

	b_int32			priority;
	b_int32			running;
	bool			exited;
	b_status_t		status;
	b_int64			ID;
	_threadCallback_	callback;
	BList			exit_callbacks;

	CRITICAL_SECTION	locker;
	HANDLE			cond;

	bool			existent;

	BList			private_threads;
} bhapi_win32_thread_t;


typedef struct bhapi_win32_thread_private_t {
	bhapi_win32_thread_t *thread;
	bool copy;
} bhapi_win32_thread_private_t;


static bhapi_win32_thread_t* __bhapi_create_thread__()
{
	bhapi_win32_thread_t *thread = new bhapi_win32_thread_t;
	if(thread == NULL) return NULL;

	thread->handle = NULL;
	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = B_OK;
	thread->ID = B_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;
	thread->cond = NULL;
	thread->existent = false;

	InitializeCriticalSection(&(thread->locker));

	return thread;
}


static void __bhapi_delete_thread__(bhapi_win32_thread_t *thread)
{
	if(thread == NULL) return;

	bhapi_win32_thread_private_t *priThread;
	while((priThread = (bhapi_win32_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	if(thread->handle != NULL) CloseHandle(thread->handle);
	if(thread->cond != NULL) CloseHandle(thread->cond);

	DeleteCriticalSection(&(thread->locker));

	delete thread;
}


static BSimpleLocker __bhapi_thread_locker__;
#define _BHAPI_LOCK_THREAD_()	__bhapi_thread_locker__.Lock()
#define _BHAPI_UNLOCK_THREAD_()	__bhapi_thread_locker__.Unlock()


class EThreadsList {
public:
	BList fList;

	EThreadsList()
	{
	}

	~EThreadsList()
	{
		bhapi_win32_thread_t *td;
		while((td = (bhapi_win32_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			BHAPI_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			__bhapi_delete_thread__(td);
		}
	}

	bhapi_win32_thread_private_t* AddThread(bhapi_win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		bhapi_win32_thread_private_t *priThread = new bhapi_win32_thread_private_t;
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

	bhapi_win32_thread_private_t* RefThread(bhapi_win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		bhapi_win32_thread_private_t *priThread = new bhapi_win32_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	b_int32 UnrefThread(bhapi_win32_thread_private_t *priThread)
	{
		bhapi_win32_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		b_int32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	bhapi_win32_thread_private_t* OpenThread(b_int64 tid)
	{
		if(tid == B_INT64_CONSTANT(0)) return NULL;
		for(b_int32 i = 0; i < fList.CountItems(); i++)
		{
			bhapi_win32_thread_t* td = (bhapi_win32_thread_t*)fList.ItemAt(i);
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


IMPEXP_BHAPI b_int64 bhapi_get_current_thread_id(void)
{
	return((b_int64)GetCurrentThreadId());
}


static void bhapi_lock_thread_inter(bhapi_win32_thread_t *thread)
{
	EnterCriticalSection(&(thread->locker));
}


static void bhapi_unlock_thread_inter(bhapi_win32_thread_t *thread)
{
	LeaveCriticalSection(&(thread->locker));
}


#ifndef BHAPI_OS_CYGWIN
unsigned _stdcall bhapi_spawn_thread_func(void *data)
#else
DWORD WINAPI bhapi_spawn_thread_func(void *data)
#endif
{
	bhapi_win32_thread_t *thread = (bhapi_win32_thread_t*)data;
	bhapi_win32_thread_private_t *priThread = NULL;

	_BHAPI_LOCK_THREAD_();
	if((priThread = _BHAPI_REF_THREAD_(thread)) == NULL)
	{
		_BHAPI_UNLOCK_THREAD_();
		return 0;
	}
	_BHAPI_UNLOCK_THREAD_();

	if(bhapi_on_exit_thread((void (*)(void *))bhapi_delete_thread, priThread) != B_OK)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		bhapi_lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;
		thread->callback.func = NULL;

		SetEvent(thread->cond);

		BList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		bhapi_unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		bhapi_delete_thread(priThread);

		return 0;
	}

	bhapi_lock_thread_inter(thread);
	b_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	bhapi_unlock_thread_inter(thread);

	b_status_t status = (threadFunc == NULL ? B_ERROR : (*threadFunc)(userData));

	bhapi_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;
	SetEvent(thread->cond);

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi_unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return 0;
}


IMPEXP_BHAPI void* bhapi_create_thread_by_current_thread(void)
{
	bhapi_win32_thread_private_t *priThread = NULL;

	_BHAPI_LOCK_THREAD_();
	if((priThread = _BHAPI_OPEN_THREAD_(bhapi_get_current_thread_id())) != NULL)
	{
		_BHAPI_UNREF_THREAD_(priThread);
		_BHAPI_UNLOCK_THREAD_();
		return NULL;
	}

	bhapi_win32_thread_t *thread = __bhapi_create_thread__();
	if(thread == NULL || (thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		if(thread) __bhapi_delete_thread__(thread);
		return NULL;
	}

	if((priThread = _BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		_BHAPI_UNLOCK_THREAD_();
		__bhapi_delete_thread__(thread);
		return NULL;
	}

	thread->priority = 0;
	thread->running = 1;
	thread->exited = false;
	thread->ID = bhapi_get_current_thread_id();
	thread->existent = true;

	_BHAPI_UNLOCK_THREAD_();

	return (void*)priThread;
}


IMPEXP_BHAPI void* bhapi_create_thread(b_thread_func threadFunction,
				    b_int32 priority,
				    void *arg,
				    b_int64 *threadId)
{
	if(threadFunction == NULL) return NULL;

	bhapi_win32_thread_t *thread = __bhapi_create_thread__();
	if(thread == NULL) return NULL;
	if((thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		__bhapi_delete_thread__(thread);
		return NULL;
	}

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

#ifndef BHAPI_OS_CYGWIN
	unsigned winThreadId;
	if((thread->handle = (HANDLE)_beginthreadex(NULL, 0x40000, bhapi_spawn_thread_func, (void*)thread,
						    CREATE_SUSPENDED, &winThreadId)) == NULL)
#else
	DWORD winThreadId;
	if((thread->handle = CreateThread(NULL, 0x40000, bhapi_spawn_thread_func, (void*)thread,
					  CREATE_SUSPENDED, &winThreadId)) == NULL)
#endif
	{
		BHAPI_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		__bhapi_delete_thread__(thread);
		return NULL;
	}

	bhapi_win32_thread_private_t *priThread = NULL;

	_BHAPI_LOCK_THREAD_();
	if((priThread = _BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		_BHAPI_UNLOCK_THREAD_();

		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		thread->callback.func = NULL;
		ResumeThread(thread->handle);
		WaitForSingleObject(thread->handle, INFINITE);

		__bhapi_delete_thread__(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = (b_int64)winThreadId;
	thread->existent = false;

	_BHAPI_UNLOCK_THREAD_();

	bhapi_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


IMPEXP_BHAPI void* bhapi_open_thread(b_int64 threadId)
{
	_BHAPI_LOCK_THREAD_();
	bhapi_win32_thread_private_t *priThread = _BHAPI_OPEN_THREAD_(threadId);
	_BHAPI_UNLOCK_THREAD_();

	return (void*)priThread;
}


IMPEXP_BHAPI b_status_t bhapi_delete_thread(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(priThread == NULL || thread == NULL) return B_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	_BHAPI_LOCK_THREAD_();
	b_int32 count = _BHAPI_UNREF_THREAD_(priThread);
	_BHAPI_UNLOCK_THREAD_();

	if(count < 0) return B_ERROR;

	if(thread->existent && !threadIsCopy)
	{
		BList exitCallbackList;

		bhapi_lock_thread_inter(thread);
		if(thread->ID == bhapi_get_current_thread_id())
		{
			thread->running = 0;
			thread->exited = true;
			thread->status = B_OK;

			SetEvent(thread->cond);

			exitCallbackList = thread->exit_callbacks;
			thread->exit_callbacks.MakeEmpty();
		}
		bhapi_unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
		{
			if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if(count > 0) return B_OK;

	if(thread->ID != bhapi_get_current_thread_id() && thread->existent == false)
	{
		if(thread->callback.func)
		{
			thread->callback.func = NULL;
			ResumeThread(thread->handle);
		}
		WaitForSingleObject(thread->handle, INFINITE);
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	__bhapi_delete_thread__(thread);

	return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_resume_thread(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	b_status_t retVal = B_ERROR;

	bhapi_lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) ||  thread->running == 2) &&
	   thread->exited == false)
	{
		ResumeThread(thread->handle);
		thread->running = 1;
		retVal = B_OK;
	}
	bhapi_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_status_t bhapi_suspend_thread(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	b_status_t retVal = B_ERROR;

	bhapi_lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == bhapi_get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			bhapi_unlock_thread_inter(thread);

			retVal = SuspendThread(thread->handle) == (DWORD)-1 ? B_ERROR : B_OK;

			bhapi_lock_thread_inter(thread);
			thread->running = 1;
		}
		else if(SuspendThread(thread->handle) != (DWORD)-1)
		{
			thread->running = 2;
			retVal = B_OK;
		}
	}
	bhapi_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_int64 bhapi_get_thread_id(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_INT64_CONSTANT(0);

	bhapi_lock_thread_inter(thread);
	b_int64 thread_id = thread->ID;
	bhapi_unlock_thread_inter(thread);

	return thread_id;
}


IMPEXP_BHAPI b_uint32 bhapi_get_thread_run_state(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return BHAPI_THREAD_INVALID;

	b_uint32 retVal = BHAPI_THREAD_INVALID;

	bhapi_lock_thread_inter(thread);

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

	bhapi_unlock_thread_inter(thread);

	return retVal;
}


IMPEXP_BHAPI b_status_t bhapi_set_thread_priority(void *data, b_int32 new_priority)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;
	if(new_priority < 0) new_priority = 15;

	int win32_priority;

	if(new_priority <= 5) win32_priority = THREAD_PRIORITY_LOWEST;
	else if(new_priority <= 15) win32_priority = THREAD_PRIORITY_NORMAL;
	else if(new_priority <= 99) win32_priority = THREAD_PRIORITY_ABOVE_NORMAL;
	else win32_priority = THREAD_PRIORITY_HIGHEST;

	bhapi_lock_thread_inter(thread);
	if(thread->exited || SetThreadPriority(thread->handle, win32_priority) == 0)
	{
		bhapi_unlock_thread_inter(thread);
		return B_ERROR;
	}
	b_int32 old_priority = thread->priority;
	thread->priority = new_priority;
	bhapi_unlock_thread_inter(thread);

	return old_priority;
}


IMPEXP_BHAPI b_int32 bhapi_get_thread_priority(void *data)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	bhapi_lock_thread_inter(thread);
	b_int32 priority = thread->priority;
	bhapi_unlock_thread_inter(thread);

	return priority;
}


IMPEXP_BHAPI b_status_t bhapi_on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return B_BAD_VALUE;

	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)bhapi_open_thread(bhapi_get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	bhapi_win32_thread_t *thread = priThread->thread;

	_threadCallback_ *exitCallback = new _threadCallback_;
	if(exitCallback == NULL)
	{
		bhapi_delete_thread(priThread);
		return B_NO_MEMORY;
	}

	exitCallback->func = (b_thread_func)callback;
	exitCallback->user_data = user_data;

	b_status_t retVal = B_OK;

	bhapi_lock_thread_inter(thread);
	if(thread->exited || thread->exit_callbacks.AddItem((void*)exitCallback, 0) == false)
	{
		delete exitCallback;
		retVal = B_ERROR;
	}
	bhapi_unlock_thread_inter(thread);

	bhapi_delete_thread(priThread);

	return retVal;
}


IMPEXP_BHAPI void bhapi_exit_thread(b_status_t status)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)bhapi_open_thread(bhapi_get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	bhapi_win32_thread_t *thread = priThread->thread;

	bhapi_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	SetEvent(thread->cond);

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi_unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	bhapi_delete_thread(priThread);

#ifndef BHAPI_OS_CYGWIN
	_endthreadex(0);
#else
	ExitThread(0);
#endif
}


IMPEXP_BHAPI b_status_t bhapi_wait_for_thread_etc(void *data, b_status_t *thread_return_value, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	bhapi_win32_thread_private_t *priThread = (bhapi_win32_thread_private_t*)data;
	bhapi_win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL || microseconds_timeout < B_INT64_CONSTANT(0) || thread_return_value == NULL) return B_BAD_VALUE;

	b_bigtime_t currentTime = bhapi_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi_lock_thread_inter(thread);

	if(thread->ID == bhapi_get_current_thread_id())
	{
		BHAPI_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		bhapi_unlock_thread_inter(thread);
		return B_ERROR;
	}
	else if(thread->exited)
	{
		bhapi_unlock_thread_inter(thread);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi_unlock_thread_inter(thread);
		return B_WOULD_BLOCK;
	}

	if(thread->callback.func || thread->running == 2) ResumeThread(thread->handle);

	HANDLE handles[2] = {NULL, NULL};
	handles[0] = thread->cond;
	if(!wait_forever)
	{
		HANDLE timer = NULL;
		LARGE_INTEGER due;
		due.QuadPart = microseconds_timeout * B_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
		timer = CreateWaitableTimer(NULL, TRUE, NULL);

		if(!timer || SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
		{
			if(timer) CloseHandle(timer);
			bhapi_unlock_thread_inter(thread);
			return B_ERROR;
		}

		handles[1] = timer;
	}

	bhapi_unlock_thread_inter(thread);

	b_status_t retVal = B_OK;

	DWORD status = (handles[1] == NULL ?
				WaitForSingleObject(handles[0], INFINITE) :
				WaitForMultipleObjects(2, handles, FALSE, INFINITE));

	if(handles[1] != NULL)
	{
		CloseHandle(handles[1]);

		if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
		{
			retVal = B_TIMED_OUT;
		}
		else if(status != WAIT_OBJECT_0)
		{
			retVal = B_ERROR;
		}
	}
	else if(status != WAIT_OBJECT_0)
	{
		retVal = B_ERROR;
	}

	if(retVal == B_OK)
	{
		bhapi_lock_thread_inter(thread);

		if(thread->existent == false) WaitForSingleObject(thread->handle, INFINITE);

		if(thread->exited)
			*thread_return_value = thread->status;
		else
			retVal = B_ERROR;

		bhapi_unlock_thread_inter(thread);
	}

	return retVal;
}


IMPEXP_BHAPI b_status_t bhapi_wait_for_thread(void *data, b_status_t *thread_return_value)
{
	return bhapi_wait_for_thread_etc(data, thread_return_value, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


IMPEXP_BHAPI b_status_t bhapi_snooze(b_bigtime_t microseconds)
{
	if(microseconds < B_INT64_CONSTANT(0)) return B_ERROR;

	HANDLE timer = NULL;
	LARGE_INTEGER due;

	due.QuadPart = -microseconds * B_INT64_CONSTANT(10);
	timer = CreateWaitableTimer(NULL, TRUE, NULL);

	if(timer == NULL) return B_ERROR;
	if(!SetWaitableTimer(timer, &due, 0, NULL, NULL, 0))
	{
		CloseHandle(timer);
		return B_ERROR;
	}

	DWORD status = WaitForSingleObject(timer, INFINITE);

	CloseHandle(timer);

	if(status != WAIT_OBJECT_0)
		return B_ERROR;
	else
		return B_OK;
}


IMPEXP_BHAPI b_status_t bhapi_snooze_until(b_bigtime_t time, int timebase)
{
	if(time < B_INT64_CONSTANT(0)) return B_ERROR;

	switch(timebase)
	{
		case B_SYSTEM_TIMEBASE:
			time += bhapi_system_boot_time();
			break;

		case B_REAL_TIME_TIMEBASE:
			break;

		default:
			return B_ERROR;
	}

	HANDLE timer = NULL;
	LARGE_INTEGER due;

	due.QuadPart = time * B_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
	timer = CreateWaitableTimer(NULL, TRUE, NULL);

	if(timer == NULL) return B_ERROR;
	if(!SetWaitableTimer(timer, &due, 0, NULL, NULL, 0))
	{
		CloseHandle(timer);
		return B_ERROR;
	}

	DWORD status = WaitForSingleObject(timer, INFINITE);

	CloseHandle(timer);

	if(status != WAIT_OBJECT_0)
		return B_ERROR;
	else
		return B_OK;
}


IMPEXP_BHAPI b_int64 bhapi_get_current_team_id(void)
{
	return((b_int64)GetCurrentProcessId());
}
#endif // BHAPI_OS_WIN32
