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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "./../config.h>
#include "./../kernel/Kernel.h>
#include "./../support/List.h>
#include "./../support/String.h>

typedef struct _threadCallback_ {
	e_thread_func	func;
	void		*user_data;
} _threadCallback_;


typedef struct etk_posix_thread_t {
	eint32			priority;
	eint32			running;
	bool			exited;
	e_status_t		status;
	eint64			ID;
	_threadCallback_	callback;
	EList			exit_callbacks;

	pthread_mutex_t		locker;
	pthread_cond_t		cond;

	bool			existent;

	EList			private_threads;
} etk_posix_thread_t;


typedef struct etk_posix_thread_private_t {
	etk_posix_thread_t *thread;
	bool copy;
} etk_posix_thread_private_t;


static etk_posix_thread_t* __etk_create_thread__()
{
	etk_posix_thread_t *thread = new etk_posix_thread_t;
	if(thread == NULL) return NULL;

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->status = E_OK;
	thread->ID = E_INT64_CONSTANT(0);
	thread->callback.func = NULL;
	thread->callback.user_data = NULL;

	pthread_mutex_init(&(thread->locker), NULL);
	pthread_cond_init(&(thread->cond), NULL);

	thread->existent = false;

	return thread;
}


static void __etk_delete_thread__(etk_posix_thread_t *thread)
{
	if(thread == NULL) return;

	etk_posix_thread_private_t *priThread;
	while((priThread = (etk_posix_thread_private_t*)thread->private_threads.RemoveItem(0)) != NULL) delete priThread;

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)thread->exit_callbacks.RemoveItem(0)) != NULL) delete exitCallback;

	pthread_mutex_destroy(&(thread->locker));
	pthread_cond_destroy(&(thread->cond));

	delete thread;
}


static pthread_mutex_t __etk_thread_locker__ = PTHREAD_MUTEX_INITIALIZER;
#define _ETK_LOCK_THREAD_()	pthread_mutex_lock(&__etk_thread_locker__)
#define _ETK_UNLOCK_THREAD_()	pthread_mutex_unlock(&__etk_thread_locker__)


static eint64 etk_convert_pthread_id_to_etk(pthread_t tid)
{
	if(sizeof(eint64) < sizeof(pthread_t))
	{
		// not support
		return E_INT64_CONSTANT(0);
	}

	eint64 cid = E_INT64_CONSTANT(0);

	if(memcpy(&cid, &tid, sizeof(pthread_t)) == NULL) return E_INT64_CONSTANT(0);

	return cid;
}


static pthread_t etk_convert_thread_id_to_pthread(eint64 cid)
{
	pthread_t tid;
	bzero(&tid, sizeof(pthread_t));

	if(sizeof(eint64) >= sizeof(pthread_t) && cid != E_INT64_CONSTANT(0)) memcpy(&tid, &cid, sizeof(pthread_t));

	return tid;
}


class EThreadsList {
public:
	EList fList;

	EThreadsList()
	{
	}

	~EThreadsList()
	{
		etk_posix_thread_t *td;
		while((td = (etk_posix_thread_t*)fList.RemoveItem(0)) != NULL)
		{
			ETK_WARNING("[KERNEL]: Thread %I64i leaked.", td->ID);
			__etk_delete_thread__(td);
		}
	}

	etk_posix_thread_private_t* AddThread(etk_posix_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() != 0 || fList.AddItem((void*)td, 0) == false) return NULL;
		etk_posix_thread_private_t *priThread = new etk_posix_thread_private_t;
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

	etk_posix_thread_private_t* RefThread(etk_posix_thread_t *td)
	{
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return NULL;
		etk_posix_thread_private_t *priThread = new etk_posix_thread_private_t;
		if(priThread == NULL || td->private_threads.AddItem((void*)priThread, 0) == false)
		{
			if(priThread != NULL) delete priThread;
			return NULL;
		}
		priThread->thread = td;
		priThread->copy = true;
		return priThread;
	}

	eint32 UnrefThread(etk_posix_thread_private_t *priThread)
	{
		etk_posix_thread_t *td = (priThread == NULL ? NULL : priThread->thread);
		if(td == NULL || td->private_threads.CountItems() == 0 || fList.IndexOf((void*)td) < 0) return -1;
		if(td->private_threads.RemoveItem((void*)priThread) == false) return -1;
		delete priThread;
		eint32 count = td->private_threads.CountItems();
		if(count == 0) fList.RemoveItem((void*)td);
		return count;
	}

	etk_posix_thread_private_t* OpenThread(eint64 tid)
	{
		if(tid == E_INT64_CONSTANT(0)) return NULL;
		for(eint32 i = 0; i < fList.CountItems(); i++)
		{
			etk_posix_thread_t* td = (etk_posix_thread_t*)fList.ItemAt(i);
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
	return(etk_convert_pthread_id_to_etk(pthread_self()));
}


static void etk_lock_thread_inter(etk_posix_thread_t *thread)
{
	pthread_mutex_lock(&(thread->locker));
}


static void etk_unlock_thread_inter(etk_posix_thread_t *thread)
{
	pthread_mutex_unlock(&(thread->locker));
}


static void* etk_spawn_thread_func(void *data)
{
	etk_posix_thread_t *thread = (etk_posix_thread_t*)data;
	etk_posix_thread_private_t *priThread = NULL;

	etk_lock_thread_inter(thread);
	pthread_cond_wait(&(thread->cond), &(thread->locker));
	if(thread->callback.func == NULL)
	{
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		etk_unlock_thread_inter(thread);
		return NULL;
	}
	e_thread_func threadFunc = thread->callback.func;
	void *userData = thread->callback.user_data;
	thread->callback.func = NULL;
	thread->running = 1;
	etk_unlock_thread_inter(thread);

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_REF_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();

		etk_lock_thread_inter(thread);
		thread->exited = true;
		pthread_cond_broadcast(&(thread->cond));
		etk_unlock_thread_inter(thread);

		return NULL;
	}
	_ETK_UNLOCK_THREAD_();

	if(etk_on_exit_thread((void (*)(void *))etk_delete_thread, priThread) != E_OK)
	{
		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		etk_lock_thread_inter(thread);

		thread->running = 0;
		thread->exited = true;

		pthread_cond_broadcast(&(thread->cond));

		EList exitCallbackList(thread->exit_callbacks);
		thread->exit_callbacks.MakeEmpty();

		etk_unlock_thread_inter(thread);

		_threadCallback_ *exitCallback;
		while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL) delete exitCallback;

		etk_delete_thread(priThread);

		return NULL;
	}

	e_status_t status = (threadFunc == NULL ? E_ERROR : (*threadFunc)(userData));

	etk_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;
	pthread_cond_broadcast(&(thread->cond));

	EList exitCallbackList(thread->exit_callbacks);
	thread->exit_callbacks.MakeEmpty();

	etk_unlock_thread_inter(thread);

	_threadCallback_ *exitCallback;
	while((exitCallback = (_threadCallback_*)exitCallbackList.RemoveItem(0)) != NULL)
	{
		if(exitCallback->func) (*(exitCallback->func))(exitCallback->user_data);
		delete exitCallback;
	}

	return NULL;
}


_IMPEXP_ETK void* etk_create_thread_by_current_thread(void)
{
	etk_posix_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_OPEN_THREAD_(etk_get_current_thread_id())) != NULL)
	{
		_ETK_UNREF_THREAD_(priThread);
		_ETK_UNLOCK_THREAD_();
		return NULL;
	}

	etk_posix_thread_t *thread = __etk_create_thread__();
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
	if(!threadFunction) return NULL;

	etk_posix_thread_t *thread = __etk_create_thread__();
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

	if(pthread_create(&posixThreadId, &posixThreadAttr, etk_spawn_thread_func, (void*)thread) != 0)
	{
		pthread_attr_destroy(&posixThreadAttr);
		ETK_WARNING("[KERNEL]: %s --- Not enough system resources to create a new thread.", __PRETTY_FUNCTION__);

		__etk_delete_thread__(thread);
		return NULL;
	}
	pthread_attr_destroy(&posixThreadAttr);

	etk_posix_thread_private_t *priThread = NULL;

	_ETK_LOCK_THREAD_();
	if((priThread = _ETK_ADD_THREAD_(thread)) == NULL)
	{
		_ETK_UNLOCK_THREAD_();

		ETK_WARNING("[KERNEL]: %s --- Unexpected error! Thread WON'T RUN!", __PRETTY_FUNCTION__);

		etk_lock_thread_inter(thread);
		thread->callback.func = NULL;
		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			etk_unlock_thread_inter(thread);
			e_snooze(500);
			etk_lock_thread_inter(thread);
		}
		etk_unlock_thread_inter(thread);

		pthread_join(posixThreadId, NULL);

		__etk_delete_thread__(thread);
		return NULL;
	}

	thread->priority = -1;
	thread->running = 0;
	thread->exited = false;
	thread->ID = etk_convert_pthread_id_to_etk(posixThreadId);
	thread->existent = false;

	_ETK_UNLOCK_THREAD_();

	etk_set_thread_priority(priThread, priority);

	if(threadId) *threadId = thread->ID;
	return (void*)priThread;
}


_IMPEXP_ETK void* etk_open_thread(eint64 threadId)
{
	_ETK_LOCK_THREAD_();
	etk_posix_thread_private_t *priThread = _ETK_OPEN_THREAD_(threadId);
	_ETK_UNLOCK_THREAD_();

	return (void*)priThread;
}


_IMPEXP_ETK e_status_t etk_delete_thread(void *data)
{
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

			pthread_cond_broadcast(&(thread->cond));

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

	if(thread->existent == false)
	{
		pthread_t posixThreadId = etk_convert_thread_id_to_pthread(thread->ID);
		if(pthread_equal(posixThreadId, pthread_self()) == 0)
		{
			etk_lock_thread_inter(thread);
			thread->callback.func = NULL;
			while(thread->exited == false && thread->running != 1)
			{
				pthread_cond_broadcast(&(thread->cond));
				etk_unlock_thread_inter(thread);
				e_snooze(500);
				etk_lock_thread_inter(thread);
			}
			etk_unlock_thread_inter(thread);

			pthread_join(posixThreadId, NULL);
		}
		else
		{
			pthread_detach(posixThreadId);
		}
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
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_BAD_VALUE;

	e_status_t retVal = E_ERROR;

	etk_lock_thread_inter(thread);
	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		retVal = E_OK;

		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			etk_unlock_thread_inter(thread);
			e_snooze(500);
			etk_lock_thread_inter(thread);
		}
	}
	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_suspend_thread(void *data)
{
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_BAD_VALUE;

	e_status_t retVal = E_ERROR;

	etk_lock_thread_inter(thread);
	bool suspend_cur_thread = (thread->ID == etk_get_current_thread_id());
	if(thread->running == 1 && thread->exited == false)
	{
		if(suspend_cur_thread)
		{
			thread->running = 2;
			retVal = pthread_cond_wait(&(thread->cond), &(thread->locker)) != 0 ? E_ERROR : E_OK;
			if(retVal != E_OK) etk_lock_thread_inter(thread);
			thread->running = 1;
		}
		else
		{
			// TODO
			ETK_WARNING("[KERNEL]: %s --- Only supported to suspend the current thread !!!", __PRETTY_FUNCTION__);
		}
	}
	etk_unlock_thread_inter(thread);

	return retVal;
}


_IMPEXP_ETK eint64 etk_get_thread_id(void *data)
{
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return E_INT64_CONSTANT(0);

	etk_lock_thread_inter(thread);
	eint64 thread_id = thread->ID;
	etk_unlock_thread_inter(thread);

	return thread_id;
}


_IMPEXP_ETK euint32 etk_get_thread_run_state(void *data)
{
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	if(new_priority < 0) new_priority = 15;
	else if(new_priority > 120) new_priority = 120;

	int policy = (new_priority >= 100 ? SCHED_RR : SCHED_OTHER);
	sched_param param;
	int priority_max, priority_min;

	etk_lock_thread_inter(thread);

	if(thread->exited || (priority_max = sched_get_priority_max(policy)) < 0 || (priority_min = sched_get_priority_min(policy)) < 0)
	{
		ETK_WARNING("[KERNEL]: %s --- %s", __PRETTY_FUNCTION__,
			    thread->exited ? "Thread exited." : "Sched get priority region failed.");
		etk_unlock_thread_inter(thread);
		return E_ERROR;
	}

	if(new_priority < 100)
		param.sched_priority = priority_min + (int)(((float)new_priority / 100.f) * (float)(priority_max - priority_min));
	else
		param.sched_priority = priority_min + (int)(((float)(new_priority - 100) / 20.f) * (float)(priority_max - priority_min));

//	ETK_DEBUG("[KERNEL]: POLICY: %d, PRIORITY_MAX: %d, PRIORITY_MIN: %d, Current Priority: %d",
//		  policy, priority_max, priority_min, param.sched_priority);

	if(pthread_setschedparam(etk_convert_thread_id_to_pthread(thread->ID), policy, &param) != 0)
	{
		ETK_WARNING("[KERNEL]: %s --- Set thread priority failed.", __PRETTY_FUNCTION__);
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
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
	if(thread == NULL) return -1;

	etk_lock_thread_inter(thread);
	eint32 priority = thread->priority;
	etk_unlock_thread_inter(thread);

	return priority;
}


_IMPEXP_ETK e_status_t etk_on_exit_thread(void (*callback)(void *), void *user_data)
{
	if(!callback) return E_BAD_VALUE;

	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- Thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	etk_posix_thread_t *thread = priThread->thread;

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
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)etk_open_thread(etk_get_current_thread_id());
	if(priThread == NULL)
	{
		ETK_WARNING("[KERNEL]: %s --- thread wasn't created by this toolkit!", __PRETTY_FUNCTION__);
		return;
	}

	etk_posix_thread_t *thread = priThread->thread;

	etk_lock_thread_inter(thread);

	thread->running = 0;
	thread->exited = true;
	thread->status = status;

	pthread_cond_broadcast(&(thread->cond));

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

	pthread_exit(NULL);
}


_IMPEXP_ETK e_status_t etk_wait_for_thread_etc(void *data, e_status_t *thread_return_value, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_posix_thread_private_t *priThread = (etk_posix_thread_private_t*)data;
	etk_posix_thread_t *thread = (priThread == NULL ? NULL : priThread->thread);
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

	pthread_t posixThreadId = etk_convert_thread_id_to_pthread(thread->ID);

	if(pthread_equal(posixThreadId, pthread_self()) != 0)
	{
		ETK_WARNING("[KERNEL]: %s --- Can't wait self.", __PRETTY_FUNCTION__);
		etk_unlock_thread_inter(thread);
		return E_ERROR;
	}
	else if(thread->exited)
	{
		etk_unlock_thread_inter(thread);
		pthread_join(posixThreadId, NULL);
		return E_OK;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_thread_inter(thread);
		return E_WOULD_BLOCK;
	}

	e_status_t retVal = E_ERROR;

	if(((thread->callback.func != NULL && thread->running == 0) || thread->running == 2) &&
	   thread->exited == false)
	{
		while(thread->exited == false && thread->running != 1)
		{
			pthread_cond_broadcast(&(thread->cond));
			etk_unlock_thread_inter(thread);
			e_snooze(500);
			etk_lock_thread_inter(thread);
		}
	}

	struct timespec ts;
	ts.tv_sec = (long)(microseconds_timeout / E_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout % E_INT64_CONSTANT(1000000)) * 1000L;

	while(true)
	{
		if(thread->exited)
		{
			*thread_return_value = thread->status;

			retVal = E_OK;
			break;
		}

		int ret = (wait_forever ? pthread_cond_wait(&(thread->cond), &(thread->locker)) :
					  pthread_cond_timedwait(&(thread->cond), &(thread->locker), &ts));

		if(ret != 0)
		{
			if(ret == ETIMEDOUT && !wait_forever)
			{
				etk_unlock_thread_inter(thread);
				return E_TIMED_OUT;
			}
			else return E_ERROR;
		}
	}

	etk_unlock_thread_inter(thread);

	if(retVal == E_OK) pthread_join(posixThreadId, NULL);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_wait_for_thread(void *data, e_status_t *thread_return_value)
{
	return etk_wait_for_thread_etc(data, thread_return_value, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_snooze(e_bigtime_t microseconds)
{
	if(microseconds <= 0) return E_ERROR;

	microseconds += etk_real_time_clock_usecs();

	struct timespec ts;
	ts.tv_sec = (long)(microseconds / E_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds % E_INT64_CONSTANT(1000000)) * 1000L;

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
		return E_OK;
	else
		return E_ERROR;
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

	struct timespec ts;
	ts.tv_sec = (long)(time / E_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(time % E_INT64_CONSTANT(1000000)) * 1000L;

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
		return E_OK;
	else
		return E_ERROR;
}


#ifdef ETK_OS_LINUX
static pthread_mutex_t __etk_team_id_locker__ = PTHREAD_MUTEX_INITIALIZER;
class __etk_pid_impl__ {
public:
	bool warning;
	eint64 fTeam;

	__etk_pid_impl__()
		: warning(true)
	{
		fTeam = (eint64)getpid();
	}

	~__etk_pid_impl__()
	{
	}

	eint64 Team()
	{
		pid_t id = getpid();

		if((pid_t)fTeam != id)
		{
			pthread_mutex_lock(&__etk_team_id_locker__);
			if(warning)
			{
				fprintf(stdout, "\x1b[31m[KERNEL]: You need GNU C Library that support for NPTL.\x1b[0m\n");
				warning = false;
			}
			pthread_mutex_unlock(&__etk_team_id_locker__);
		}

		return fTeam;
	}
};
static __etk_pid_impl__ __etk_team_id__;
#endif // ETK_OS_LINUX


_IMPEXP_ETK eint64 etk_get_current_team_id(void)
{
#ifdef ETK_OS_LINUX
	return __etk_team_id__.Team();
#else
	return((eint64)getpid());
#endif
}

