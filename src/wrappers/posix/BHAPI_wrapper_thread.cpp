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
#ifdef BHAPI_OS_LINUX



#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include <kits/config.h>
#include <os/kernel.h>
#include <kits/support/List.h>
#include <kits/support/String.h>

typedef struct threadCallback {
	b_thread_func	func;
	void		*user_data;
} threadCallback;


typedef struct b_posix_thread_t {
	int32_t			priority;
	int32_t			running;
	bool			exited;
	status_t		status;
	int64			ID;
	threadCallback	callback;
	BList			exit_callbacks;

	pthread_mutex_t		locker;
	pthread_cond_t		cond;

	bool			existent;

	BList			private_threads;
} b_posix_thread_t;


typedef struct b_posix_thread_private_t {
	b_posix_thread_t *thread;
	bool copy;
} b_posix_thread_private_t;


static b_posix_thread_t* __bhapi_create_thread_()
{
	b_posix_thread_t *thread = new b_posix_thread_t;
	if(thread == NULL) return NULL;

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = B_OK;
	thread->ID = B_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;

	pthread_mutex_init(&(thread->locker), NULL);
	pthread_cond_init(&(thread->cond), NULL);

	thread->existent = false;

	return thread;
}


static void bhapi::delete_thread(b_posix_thread_t *thread)
{
	if(thread == NULL) return;

	b_posix_thread_private_t *priThread;
	while((priThread = (b_posix_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	pthread_mutex_destroy(&(thread->locker));
	pthread_cond_destroy(&(thread->cond));

	delete thread;
}


static pthread_mutex_t __bhapi_thread_locker__ = PTHREAD_MUTEX_INITIALIZER;
#define BHAPI_LOCK_THREAD()	pthread_mutex_lock(&__bhapi_thread_locker__)
#define BHAPI_UNLOCK_THREAD()	pthread_mutex_unlock(&__bhapi_thread_locker__)


static  int64 b_convert_pthread_id_to_etk(pthread_t tid)
{
	if(sizeof(int64) < sizeof(pthread_t))
	{
		// not support
		return B_INT64_CONSTANT(0);
	}

	int64 cid = B_INT64_CONSTANT(0);

	if(memcpy(&cid, &tid, sizeof(pthread_t)) == NULL) return B_INT64_CONSTANT(0);

	return cid;
}


static pthread_t b_convert_thread_id_to_pthread(int64 cid)
{
	pthread_t tid;
	bzero(&tid, sizeof(pthread_t));

	if(sizeof(int64) >= sizeof(pthread_t) && cid != B_INT64_CONSTANT(0)) memcpy(&tid, &cid, sizeof(pthread_t));

	return tid;
}


class EThreadsList {
public:
	BList fList;

	EThreadsList()
	{
	}

	~EThreadsList()
	{
		b_posix_thread_t *td;
		while((td = (b_posix_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			BHAPI_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			bhapi::delete_thread(td);
		}
	}

	b_posix_thread_private_t* AddThread(b_posix_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		b_posix_thread_private_t *priThread = new b_posix_thread_private_t;
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

	b_posix_thread_private_t* RefThread(b_posix_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		b_posix_thread_private_t *priThread = new b_posix_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	int32_t UnrefThread(b_posix_thread_private_t *priThread)
	{
		b_posix_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		int32_t count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	b_posix_thread_private_t* OpenThread(int64 tid)
	{
		if(tid == B_INT64_CONSTANT(0)) return NULL;
		for(int32_t i = 0; i < fList.CountItems(); i++)
		{
			b_posix_thread_t* td = (b_posix_thread_t*)fList.ItemAt(i);
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


BHAPI_IMPEXP  int64 bhapi::get_current_thread_id(void)
{
	return(b_convert_pthread_id_to_etk(pthread_self()));
}


static void bhapi::lock_thread_inter(b_posix_thread_t *thread)
{
	pthread_mutex_lock(&(thread->locker));
}


static void bhapi::unlock_thread_inter(b_posix_thread_t *thread)
{
	pthread_mutex_unlock(&(thread->locker));
}


static void* b_spawn_thread_func(void *data)
{
	b_posix_thread_t *thread = (b_posix_thread_t*)data;
	b_posix_thread_private_t *priThread = NULL;

	bhapi::lock_thread_inter(thread);
	pthread_cond_wait(&(thread->cond), &(thread->locker));
	if(thread->callback.func == NULL)
	{
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		bhapi::unlock_thread_inter(thread);
		return NULL;
	}
	b_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	bhapi::unlock_thread_inter(thread);

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_REF_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();

		bhapi::lock_thread_inter(thread);
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		bhapi::unlock_thread_inter(thread);

		return NULL;
	}
	BHAPI_UNLOCK_THREAD();

	if(bhapi::on_exit_thread((void (*)(void *))bhapi::delete_thread, priThread) != B_OK)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		bhapi::lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;

		pthread_cond_broadcast(&(thread->cond));

		BList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		bhapi::unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		bhapi::delete_thread(priThread);

		return NULL;
	}

	status_t status = (threadFunc == NULL ? B_ERROR : (*threadFunc)(userData));

	bhapi::lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;
	pthread_cond_broadcast(&(thread->cond));

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi::unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return NULL;
}


BHAPI_IMPEXP void* bhapi::create_thread_by_current_thread(void)
{
	b_posix_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_OPEN_THREAD_(bhapi::get_current_thread_id())) != NULL)
	{
		BHAPI_UNREF_THREAD_(priThread);
		BHAPI_UNLOCK_THREAD();
		return NULL;
	}

	b_posix_thread_t *thread = __bhapi_create_thread_();
	if(thread == NULL) return NULL;

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


BHAPI_IMPEXP void* bhapi::create_thread(b_thread_func threadFunction,
				     int32_t priority,
				    void *arg,
				     int64 *threadId)
{
	if(!threadFunction) return NULL;

	b_posix_thread_t *thread = __bhapi_create_thread_();
	if(thread == NULL) return NULL;

	thread->callback.func = threadFunction;
	thread->callback.user_data = arg;

	pthread_t posixThreadId;

	pthread_attr_t posixThreadAttr;
	pthread_attr_init(&posixThreadAttr);
	pthread_attr_setdetachstate(&posixThreadAttr, PTHREAD_CREATE_JOINABLE);
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	pthread_attr_setstacksize(&posixThreadAttr, 0x40000);
#endif // _POSIX_THREAD_ATTR_STACKSIZE

	if(pthread_create(&posixThreadId, &posixThreadAttr, b_spawn_thread_func, (void*)thread) != 0)
	{
		pthread_attr_destroy(&posixThreadAttr);
		BHAPI_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		bhapi::delete_thread(thread);
		return NULL;
	}
	pthread_attr_destroy(&posixThreadAttr);

	b_posix_thread_private_t *priThread = NULL;

	BHAPI_LOCK_THREAD();
	if((priThread = BHAPI_ADD_THREAD_(thread)) == NULL)
	{
		BHAPI_UNLOCK_THREAD();

		BHAPI_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		bhapi::lock_thread_inter(thread);
		thread->callback.func = NULL;
		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			bhapi::unlock_thread_inter(thread);
			b_snooze(500);
			bhapi::lock_thread_inter(thread);
		}
		bhapi::unlock_thread_inter(thread);

		pthread_join(posixThreadId, NULL);

		bhapi::delete_thread(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = b_convert_pthread_id_to_etk(posixThreadId);
	thread->existent = false;

	BHAPI_UNLOCK_THREAD();

	b_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


BHAPI_IMPEXP void* bhapi::open_thread(int64 threadId)
{
	BHAPI_LOCK_THREAD();
	b_posix_thread_private_t *priThread = BHAPI_OPEN_THREAD_(threadId);
	BHAPI_UNLOCK_THREAD();

	return (void*)priThread;
}


BHAPI_IMPEXP status_t bhapi::delete_thread(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(priThread == NULL || thread == NULL) return B_BAD_VALUE;

	bool threadIsCopy = priThread->copy;

	BHAPI_LOCK_THREAD();
	int32_t count = BHAPI_UNREF_THREAD_(priThread);
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

			pthread_cond_broadcast(&(thread->cond));

			exitCallbackList = thread->exit_callbacks;
			thread->exit_callbacks.MakeEmpty();
		}
		bhapi::unlock_thread_inter(thread);

		threadCallback *exitCallback;
		while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
		{
			if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
			delete exitCallback;
		}
	}

	if(count > 0) return B_OK;

	if(thread->existent == false)
	{
		pthread_t posixThreadId = b_convert_thread_id_to_pthread(thread->ID);
		if(pthread_equal(posixThreadId, pthread_self()) == 0)
		{
			bhapi::lock_thread_inter(thread);
			thread->callback.func = NULL;
			while(thread->exited == false && thread->running != 1)
			{
				pthread_cond_broadcast(&(thread->cond));
				bhapi::unlock_thread_inter(thread);
				b_snooze(500);
				bhapi::lock_thread_inter(thread);
			}
			bhapi::unlock_thread_inter(thread);

			pthread_join(posixThreadId, NULL);
		}
		else
		{
			pthread_detach(posixThreadId);
		}
	}

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	bhapi::delete_thread(thread);

	return B_OK;
}


BHAPI_IMPEXP status_t bhapi::resume_thread(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	bhapi::lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		retVal = B_OK;

		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			bhapi::unlock_thread_inter(thread);
			b_snooze(500);
			bhapi::lock_thread_inter(thread);
		}
	}
	bhapi::unlock_thread_inter(thread);

	return retVal;
}


BHAPI_IMPEXP status_t b_suspend_thread(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_BAD_VALUE;

	status_t retVal = B_ERROR;

	bhapi::lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == bhapi::get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			retVal = pthread_cond_wait(&(thread->cond), &(thread->locker)) != 0 ? B_ERROR : B_OK;
			if(retVal != B_OK) bhapi::lock_thread_inter(thread);
			thread->running = 1;
		}
		else
		{
			// TODO
			BHAPI_WARNING("[KERNEL]: %s --- Only supported to suspend the current thread !!!", __PRETTY_FUNCTION__);
		}
	}
	bhapi::unlock_thread_inter(thread);

	return retVal;
}


BHAPI_IMPEXP  int64 bhapi::get_thread_id(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return B_INT64_CONSTANT(0);

	bhapi::lock_thread_inter(thread);
	int64 thread_id = thread->ID;
	bhapi::unlock_thread_inter(thread);

	return thread_id;
}


BHAPI_IMPEXP  uint32_t bhapi::get_thread_run_state(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return BHAPI_THREAD_INVALID;

	uint32_t retVal = BHAPI_THREAD_INVALID;

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


BHAPI_IMPEXP status_t b_set_thread_priority(void *data,  int32_t new_priority)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	if(new_priority < 0) new_priority = 15;
	else if(new_priority > 120) new_priority = 120;

	int policy = (new_priority >= 100 ? SCHED_RR : SCHED_OTHER);
	sched_param param;
	int priority_max, priority_min;

	bhapi::lock_thread_inter(thread);

	if(thread->exited || (priority_max = sched_get_priority_max(policy)) < 0 || (priority_min = sched_get_priority_min(policy)) < 0)
	{
		BHAPI_WARNING("[KERNEL]: %s --- %s", __PRETTY_FUNCTION__,
			    thread->exited ? "Thread exited." : "Sched get priority region failed.");
		bhapi::unlock_thread_inter(thread);
		return B_ERROR;
	}

	if(new_priority < 100)
		param.sched_priority = priority_min + (int)(((float)new_priority / 100.f) * (float)(priority_max - priority_min));
	else
		param.sched_priority = priority_min + (int)(((float)(new_priority - 100) / 20.f) * (float)(priority_max - priority_min));

//	BHAPI_DEBUG("[KERNEL]: POLICY: %d, PRIORITY_MAX: %d, PRIORITY_MIN: %d, Current Priority: %d",
//		  policy, priority_max, priority_min, param.sched_priority);

	if(pthread_setschedparam(b_convert_thread_id_to_pthread(thread->ID), policy, &param) != 0)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Set thread priority failed.", __PRETTY_FUNCTION__);
		bhapi::unlock_thread_inter(thread);
		return B_ERROR;
	}

	int32_t old_priority = thread->priority;
	thread->priority = new_priority;

	bhapi::unlock_thread_inter(thread);

	return old_priority;
}


BHAPI_IMPEXP  int32_t bhapi::get_thread_priority(void *data)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	bhapi::lock_thread_inter(thread);
	int32_t priority = thread->priority;
	bhapi::unlock_thread_inter(thread);

	return priority;
}


BHAPI_IMPEXP status_t bhapi::on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return B_BAD_VALUE;

	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)bhapi::open_thread(bhapi::get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	b_posix_thread_t *thread = priThread->thread;

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


BHAPI_IMPEXP void b_exit_thread(status_t status)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)bhapi::open_thread(bhapi::get_current_thread_id());
	if(priThread == NULL)
	{
		BHAPI_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	b_posix_thread_t *thread = priThread->thread;

	bhapi::lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	pthread_cond_broadcast(&(thread->cond));

	BList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	bhapi::unlock_thread_inter(thread);

	threadCallback *exitCallback;
	while((exitCallback = (threadCallback*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	bhapi::delete_thread(priThread);

	pthread_exit(NULL);
}


BHAPI_IMPEXP status_t bhapi::wait_for_thread_etc(void *data, status_t *thread_return_value,  uint32_t flags, bigtime_t microseconds_timeout)
{
	b_posix_thread_private_t *priThread = (b_posix_thread_private_t*)data;
	b_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

	pthread_t posixThreadId = b_convert_thread_id_to_pthread(thread->ID);

	if(pthread_equal(posixThreadId, pthread_self()) != 0)
	{
		BHAPI_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		bhapi::unlock_thread_inter(thread);
		return B_ERROR;
	}
	else if(thread->exited)
	{
		bhapi::unlock_thread_inter(thread);
		pthread_join(posixThreadId, NULL);
		return B_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi::unlock_thread_inter(thread);
		return B_WOULD_BLOCK;
	}

	status_t retVal = B_ERROR;

	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			bhapi::unlock_thread_inter(thread);
			b_snooze(500);
			bhapi::lock_thread_inter(thread);
		}
	}

	struct timespec ts;
	ts.tv_sec = (long)(microseconds_timeout / B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout % B_INT64_CONSTANT(1000000)) * 1000L;

	while(true)
	{
		if(thread->exited)
		{
			*thread_return_value = thread->status;

			retVal = B_OK;
			break;
		}

		int ret = (wait_forever ? pthread_cond_wait(&(thread->cond), &(thread->locker)) :
					  pthread_cond_timedwait(&(thread->cond), &(thread->locker), &ts));

		if(ret != 0)
		{
			if(ret == ETIMEDOUT && !wait_forever)
			{
				bhapi::unlock_thread_inter(thread);
				return B_TIMED_OUT;
			}
			else return B_ERROR;
		}
	}

	bhapi::unlock_thread_inter(thread);

	if(retVal == B_OK) pthread_join(posixThreadId, NULL);

	return retVal;
}


BHAPI_IMPEXP status_t bhapi::wait_for_thread(void *data, status_t *thread_return_value)
{
	return bhapi::wait_for_thread_etc(data, thread_return_value, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_IMPEXP status_t b_snooze(bigtime_t microseconds)
{
	if(microseconds <= 0) return B_ERROR;

	microseconds += b_real_time_clock_usecs();

	struct timespec ts;
	ts.tv_sec = (long)(microseconds / B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds % B_INT64_CONSTANT(1000000)) * 1000L;

	pthread_mutex_t mptr;
	pthread_cond_t cptr;

	pthread_mutex_init(&mptr, NULL);
	pthread_cond_init(&cptr, NULL);

	pthread_mutex_lock(&mptr);
	int ret = pthread_cond_timedwait(&cptr, &mptr, &ts);
	pthread_mutex_unlock(&mptr);

	pthread_mutex_destroy(&mptr);
	pthread_cond_destroy(&cptr);

	if(ret == 0 || ret == ETIMEDOUT)
		return B_OK;
	else
		return B_ERROR;
}


BHAPI_IMPEXP status_t b_snooze_until(bigtime_t time, int timebase)
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

	struct timespec ts;
	ts.tv_sec = (long)(time / B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(time % B_INT64_CONSTANT(1000000)) * 1000L;

	pthread_mutex_t mptr;
	pthread_cond_t cptr;

	pthread_mutex_init(&mptr, NULL);
	pthread_cond_init(&cptr, NULL);

	pthread_mutex_lock(&mptr);
	int ret = pthread_cond_timedwait(&cptr, &mptr, &ts);
	pthread_mutex_unlock(&mptr);

	pthread_mutex_destroy(&mptr);
	pthread_cond_destroy(&cptr);

	if(ret == 0 || ret == ETIMEDOUT)
		return B_OK;
	else
		return B_ERROR;
}


#ifdef BHAPI_OS_LINUX
static pthread_mutex_t __bhapi_team_id_locker__ = PTHREAD_MUTEX_INITIALIZER;
class __bhapi_pid_impl__ {
public:
	bool warning;
	int64 fTeam;

	__bhapi_pid_impl_()
		: warning(true)
	{
		fTeam = (int64)getpid();
	}

	~__bhapi_pid_impl_()
	{
	}

	int64 Team()
	{
		pid_t id = getpid();

		if((pid_t)fTeam != id)
		{
			pthread_mutex_lock(&__bhapi_team_id_locker__);
			if(warning)
			{
				fprintf(stdout, "\x1b[31m[KERNEL]: You need GNU C Library that support for NPTL.\x1b[0m\n");
				warning = false;
			}
			pthread_mutex_unlock(&__bhapi_team_id_locker__);
		}

		return fTeam;
	}
};
static __bhapi_pid_impl__ __bhapi_team_id__;
#endif // BHAPI_OS_LINUX


BHAPI_IMPEXP  int64 bhapi::get_current_team_id(void)
{
#ifdef BHAPI_OS_LINUX
	return __bhapi_team_id__.Team();
#else
	return((int64)getpid());
#endif
}
#endif // BHAPI_OS_LINUX

