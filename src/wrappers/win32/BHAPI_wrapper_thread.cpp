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
 * File: BHAPI_wrapper_thread.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_WIN32

#include "../../../kernel/Kernel.h"
#include "../../../kernel/Debug.h"
#include "../../../support/List.h"
#include "../../../support/StringClass.h"
#include "../../../support/Errors.h"
#include "../../../support/SimpleLocker.h"
#include "../../../support/SupportDefs.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <winsock2.h>
#include <windows.h>
#include <process.h>


#define SECS_BETWEEN_EPOCHS    B_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS	    B_INT64_CONSTANT(10000000)


typedef struct threadCallback {
	b_thread_func	func;
	void		*user_data;
} threadCallback;


typedef struct bhapi::win32_thread_t {
	HANDLE			handle;

	__be_int32			priority;
	__be_int32			running;
	bool			exited;
	status_t		status;
	__be_int64			ID;
	threadCallback	callback;
	BList			exit_callbacks;

	CRITICAL_SECTION	locker;
	HANDLE			cond;

	bool			existent;

	BList			private_threads;
} bhapi::win32_thread_t;


typedef struct bhapi::win32_thread_private_t {
	bhapi::win32_thread_t *thread;
	bool copy;
} bhapi::win32_thread_private_t;


static bhapi::win32_thread_t* __bhapi_create_thread_()
{
	bhapi::win32_thread_t *thread = new bhapi::win32_thread_t;
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


static void bhapi::delete_thread(bhapi::win32_thread_t *thread)
{
	if(thread == NULL) return;

	bhapi::win32_thread_private_t *priThread;
	while((priThread = (bhapi::win32_thread_private_t*)thread->private_threads.RemoveItems(0,1)) != NULL) delete priThread;

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)thread->exit_callbacks.RemoveItems(0,1)) != NULL) delete exitCallback;

	if(thread->handle != NULL) CloseHandle(thread->handle);
	if(thread->cond != NULL) CloseHandle(thread->cond);

	DeleteCriticalSection(&(thread->locker));

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
		bhapi::win32_thread_t *td;
		while((td = (bhapi::win32_thread_t*)fList.RemoveItems(0,1)) != NULL)
		{
			BHAPI_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			bhapi::delete_thread(td);
		}
	}

	bhapi::win32_thread_private_t* AddThread(bhapi::win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		bhapi::win32_thread_private_t *priThread = new bhapi::win32_thread_private_t;
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

	bhapi::win32_thread_private_t* RefThread(bhapi::win32_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		bhapi::win32_thread_private_t *priThread = new bhapi::win32_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	__be_int32 UnrefThread(bhapi::win32_thread_private_t *priThread)
	{
		bhapi::win32_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		__be_int32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	bhapi::win32_thread_private_t* OpenThread(__be_int64 tid)
	{
		if(tid == B_INT64_CONSTANT(0)) return NULL;
		for(__be_int32 i = 0; i < fList.CountItems(); i++)
		{
			bhapi::win32_thread_t* td = (bhapi::win32_thread_t*)fList.ItemAt(i);
			if(td->ID == tid) return RefThread(td);
		}
		return NULL;
	}
};


static EThreadsList __bhapi_thread_lists__;
#define BHAPI_ADD_THREAD_(td)	__bhapi_thread_lists__.AddThread(td)
#define BHAPI_REF_THREAD_(td)	__bhapi_thread_lists__.RefThread(td)
#define BHAPI_UNREF_THREAD_(td)	__bhapi_thread_lists__.UnrefThread(td)
#define BHAPI_OPEN_THREAD_(tid)	__bhapi_thread_lists__.OpenThread(tid)


BHAPI_EXPORT  __be_int64 bhapi::get_current_thread_id(void)
{
	return((__be_int64)GetCurrentThreadId());
}


static void bhapi::lock_thread_inter(bhapi::win32_thread_t *thread)
{
	EnterCriticalSection(&(thread->locker));
}


static void bhapi::unlock_thread_inter(bhapi::win32_thread_t *thread)
{
	LeaveCriticalSection(&(thread->locker));
}


#ifndef BHAPI_OS_CYGWIN
unsigned _stdcall b_spawn_thread_func(void *data)
#else
DWORD WINAPI b_spawn_thread_func(void *data)
#endif
{
	bhapi::win32_thread_t *thread = (bhapi::win32_thread_t*)data;
	bhapi::win32_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_REF_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();
		return 0;
	}
	BHAPI_UNLOCK_THREAD();

	if(bhapi::on_exit_thread((void (*)(void *))bhapi::delete_thread, priThread) != B_OK)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		bhapi::lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;
		thread->callback.func = NULL;

		SetEvent(thread->cond);

		BList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		bhapi::unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItems(0,1)) != NULL) delete exitCallback;

		bhapi::delete_thread(priThread);

		return 0;
	}

	bhapi::lock_thread_inter(thread);
	b_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	bhapi::unlock_thread_inter(thread);

	status_t status = (threadFunc == NULL ? B_ERROR : (*threadFunc)(userData));

	bhapi::lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;
	SetEvent(thread->cond);

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi::unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItems(0,1)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return 0;
}


BHAPI_EXPORT void* bhapi::create_thread_by_current_thread(void)
{
	bhapi::win32_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_OPEN_THREAD_(bhapi::get_current_thread_id())) != NULL)
	{
		BHAPI_UNREF_THREAD_(priThread);
		BHAPI_UNLOCK_THREAD();
		return NULL;
	}

	bhapi::win32_thread_t *thread = __bhapi_create_thread_();
	if(thread == NULL || (thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		if(thread) bhapi::delete_thread(thread);
		return NULL;
	}

	if((priThread = BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();
		bhapi::delete_thread(thread);
		return NULL;
	}

	thread->priority = 0;
	thread->running = 1;
	thread->exited = false;
	thread->ID = bhapi::get_current_thread_id();
	thread->existent = true;

	BHAPI_UNLOCK_THREAD();

	return (void*)priThread;
}


BHAPI_EXPORT void* bhapi::create_thread(b_thread_func threadFunction,
				     __be_int32 priority,
				    void *arg,
				     __be_int64 *threadId)
{
	if(threadFunction == NULL) return NULL;

	bhapi::win32_thread_t *thread = __bhapi_create_thread_();
	if(thread == NULL) return NULL;
	if((thread->cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		bhapi::delete_thread(thread);
		return NULL;
	}

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

#ifndef BHAPI_OS_CYGWIN
	unsigned winThreadId;
	if((thread->handle = (HANDLE)_beginthreadex(NULL, 0x40000, b_spawn_thread_func, (void*)thread,
						    CREATE_SUSPENDED, &winThreadId)) == NULL)
#else
	DWORD winThreadId;
	if((thread->handle = CreateThread(NULL, 0x40000, b_spawn_thread_func, (void*)thread,
					  CREATE_SUSPENDED, &winThreadId)) == NULL)
#endif
	{
		BHAPI_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		bhapi::delete_thread(thread);
		return NULL;
	}

	bhapi::win32_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();

		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		thread->callback.func = NULL;
		ResumeThread(thread->handle);
		WaitForSingleObject(thread->handle, INFINITE);

		bhapi::delete_thread(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = (__be_int64)winThreadId;
	thread->existent = false;

	BHAPI_UNLOCK_THREAD();

	b_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


BHAPI_EXPORT void* bhapi::open_thread(__be_int64 threadId)
{
	BHAPI_LOCK_THREAD();
	bhapi::win32_thread_private_t *priThread = BHAPI_OPEN_THREAD_(threadId);
	BHAPI_UNLOCK_THREAD();

	return (void*)priThread;
}


BHAPI_EXPORT status_t bhapi::delete_thread(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(priThread == NULL || thread == NULL) return B_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	BHAPI_LOCK_THREAD();
	__be_int32 count = BHAPI_UNREF_THREAD_(priThread);
	BHAPI_UNLOCK_THREAD();

	if(count < 0) return B_ERROR;

	if(thread->existent && !threadIsCopy)
	{
		BList exitCallbackList;

		bhapi::lock_thread_inter(thread);
		if(thread->ID == bhapi::get_current_thread_id())
		{
			thread->running = 0;
			thread->exited = true;
			thread->status = B_OK;

			SetEvent(thread->cond);

			exitCallbackList = thread->exit_callbacks;
			thread->exit_callbacks.MakeEmpty();
		}
		bhapi::unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItems(0,1)) != NULL)
		{
			if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if(count > 0) return B_OK;

	if(thread->ID != bhapi::get_current_thread_id() && thread->existent == false)
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

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItems(0,1)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	bhapi::delete_thread(thread);

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::resume_thread(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	bhapi::lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) ||  thread->running == 2) &&
	   thread->exited == false)
	{
		ResumeThread(thread->handle);
		thread->running = 1;
		retVal = B_OK;
	}
	bhapi::unlock_thread_inter(thread);

	return retVal;
}


BHAPI_EXPORT status_t b_suspend_thread(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	bhapi::lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == bhapi::get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			bhapi::unlock_thread_inter(thread);

			retVal = SuspendThread(thread->handle) == (DWORD)-1 ? B_ERROR : B_OK;

			bhapi::lock_thread_inter(thread);
			thread->running = 1;
		}
		else if(SuspendThread(thread->handle) != (DWORD)-1)
		{
			thread->running = 2;
			retVal = B_OK;
		}
	}
	bhapi::unlock_thread_inter(thread);

	return retVal;
}


BHAPI_EXPORT  __be_int64 bhapi::get_thread_id(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_INT64_CONSTANT(0);

	bhapi::lock_thread_inter(thread);
	__be_int64 thread_id = thread->ID;
	bhapi::unlock_thread_inter(thread);

	return thread_id;
}


BHAPI_EXPORT  __be_uint32 bhapi::get_thread_run_state(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return BHAPI_THREAD_INVALID;

	__be_uint32 retVal = BHAPI_THREAD_INVALID;

	bhapi::lock_thread_inter(thread);

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

	bhapi::unlock_thread_inter(thread);

	return retVal;
}


BHAPI_EXPORT status_t b_set_thread_priority(void *data,  __be_int32 new_priority)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;
	if(new_priority < 0) new_priority = 15;

	int win32_priority;

	if(new_priority <= 5) win32_priority = THREAD_PRIORITY_LOWEST;
	else if(new_priority <= 15) win32_priority = THREAD_PRIORITY_NORMAL;
	else if(new_priority <= 99) win32_priority = THREAD_PRIORITY_ABOVE_NORMAL;
	else win32_priority = THREAD_PRIORITY_HIGHEST;

	bhapi::lock_thread_inter(thread);
	if(thread->exited || SetThreadPriority(thread->handle, win32_priority) == 0)
	{
		bhapi::unlock_thread_inter(thread);
		return B_ERROR;
	}
	__be_int32 old_priority = thread->priority;
	thread->priority = new_priority;
	bhapi::unlock_thread_inter(thread);

	return old_priority;
}


BHAPI_EXPORT  __be_int32 bhapi::get_thread_priority(void *data)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	bhapi::lock_thread_inter(thread);
	__be_int32 priority = thread->priority;
	bhapi::unlock_thread_inter(thread);

	return priority;
}


BHAPI_EXPORT status_t bhapi::on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return B_BAD_VALUE;

	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)bhapi::open_thread(bhapi::get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	bhapi::win32_thread_t *thread = priThread->thread;

	threadCallback *exitCallback = new threadCallback;
	if(exitCallback == NULL)
	{
		bhapi::delete_thread(priThread);
		return B_NO_MEMORY;
	}

	exitCallback->func = (b_thread_func)callback;
	exitCallback->user_data = user_data;

	status_t retVal = B_OK;

	bhapi::lock_thread_inter(thread);
	if(thread->exited || thread->exit_callbacks.AddItem((void*)exitCallback, 0) == false)
	{
		delete exitCallback;
		retVal = B_ERROR;
	}
	bhapi::unlock_thread_inter(thread);

	bhapi::delete_thread(priThread);

	return retVal;
}


BHAPI_EXPORT void b_exit_thread(status_t status)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)bhapi::open_thread(bhapi::get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	bhapi::win32_thread_t *thread = priThread->thread;

	bhapi::lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	SetEvent(thread->cond);

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi::unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItems(0,1)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	bhapi::delete_thread(priThread);

#ifndef BHAPI_OS_CYGWIN
	_endthreadex(0);
#else
	ExitThread(0);
#endif
}


BHAPI_EXPORT status_t bhapi::wait_for_thread_etc(void *data, status_t *thread_return_value,  __be_uint32 flags, bigtime_t microseconds_timeout)
{
	bhapi::win32_thread_private_t *priThread = (bhapi::win32_thread_private_t*)data;
	bhapi::win32_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL || microseconds_timeout < B_INT64_CONSTANT(0) || thread_return_value == NULL) return B_BAD_VALUE;

	bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi::lock_thread_inter(thread);

	if(thread->ID == bhapi::get_current_thread_id())
	{
		BHAPI_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		bhapi::unlock_thread_inter(thread);
		return B_ERROR;
	}
	else if(thread->exited)
	{
		bhapi::unlock_thread_inter(thread);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi::unlock_thread_inter(thread);
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
			bhapi::unlock_thread_inter(thread);
			return B_ERROR;
		}

		handles[1] = timer;
	}

	bhapi::unlock_thread_inter(thread);

	status_t retVal = B_OK;

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
		bhapi::lock_thread_inter(thread);

		if(thread->existent == false) WaitForSingleObject(thread->handle, INFINITE);

		if(thread->exited)
			*thread_return_value = thread->status;
		else
			retVal = B_ERROR;

		bhapi::unlock_thread_inter(thread);
	}

	return retVal;
}


BHAPI_EXPORT status_t bhapi::wait_for_thread(void *data, status_t *thread_return_value)
{
	return bhapi::wait_for_thread_etc(data, thread_return_value, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_EXPORT status_t b_snooze(bigtime_t microseconds)
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


BHAPI_EXPORT status_t b_snooze_until(bigtime_t time, int timebase)
{
	if(time < B_INT64_CONSTANT(0)) return B_ERROR;

	switch(timebase)
	{
		case B_SYSTEM_TIMEBASE:
			time += b_system_boot_time();
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


BHAPI_EXPORT  __be_int64 bhapi::get_current_team_id(void)
{
	return((__be_int64)GetCurrentProcessId());
}
#endif // BHAPI_OS_WIN32
