/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_USER_MUTEX_H
#define _KERNEL_USER_MUTEX_H


#include <Haiku.h>


#ifdef __cplusplus
extern "C" {
#endif

void		user_mutex_init();

status_t	_user_mutex_lock(int32_t* mutex, const char* name, uint32_t flags,
				bigtime_t timeout);
status_t	_user_mutex_unlock(int32_t* mutex, uint32_t flags);
status_t	_user_mutex_switch_lock(int32_t* fromMutex, int32_t* toMutex,
				const char* name, uint32_t flags, bigtime_t timeout);
status_t	_user_mutex_sem_acquire(int32_t* sem, const char* name, uint32_t flags,
				bigtime_t timeout);
status_t	_user_mutex_sem_release(int32_t* sem);

#ifdef __cplusplus
}
#endif


#endif	/* _KERNEL_USER_MUTEX_H */
