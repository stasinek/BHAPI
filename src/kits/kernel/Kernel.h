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
 * File: Kernel.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_KERNEL__H
#define BHAPI_KERNEL__H
#include "../support/SupportDefs.h"
#include "OS.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* time functions */
namespace bhapi {
IMPEXP_BHAPI b_uint32    real_time_clock(void);
IMPEXP_BHAPI b_bigtime_t real_time_clock_usecs(void);
IMPEXP_BHAPI b_bigtime_t system_boot_time(void); /* system boot time in microseconds */
IMPEXP_BHAPI b_bigtime_t system_time(void); /* time since booting in microseconds */

/* area functions */
typedef struct area_info {
    char		name[B_OS_NAME_LENGTH + 1];
	size_t		size;
    b_uint32		protection;
	void		*address;
	char		domain[5];
} area_info;

#define BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN	"ssem"
#define BHAPI_AREA_SYSTEM_PORT_DOMAIN		"spot"
#define BHAPI_AREA_USER_DOMAIN              "user"

typedef enum area_access {
	BHAPI_AREA_ACCESS_OWNER = 0,
	BHAPI_AREA_ACCESS_GROUP_READ = 1,
	BHAPI_AREA_ACCESS_GROUP_WRITE = 1 << 1,
	BHAPI_AREA_ACCESS_OTHERS_READ = 1 << 2,
	BHAPI_AREA_ACCESS_OTHERS_WRITE = 1 << 3,
	BHAPI_AREA_ACCESS_ALL = 0xFF
} area_access;

#ifdef __cplusplus
IMPEXP_BHAPI void*	create_area(const char *name, void **start_addr, size_t size, b_uint32 protection,
				        const char *domain, bhapi::area_access area_access = BHAPI_AREA_ACCESS_OWNER);
#else
IMPEXP_BHAPI void*	create_area(const char *name, void **start_addr, size_t size, b_uint32 protection,
				        const char *domain, bhapi::area_access area_access);
#endif
IMPEXP_BHAPI void*	clone_area(const char *name, void **dest_addr, b_uint32 protection, const char *domain);
IMPEXP_BHAPI void*	clone_area_by_source(void *source_area, void **dest_addr, b_uint32 protection);
IMPEXP_BHAPI b_status_t	get_area_info(void *area, bhapi::area_info *info);
IMPEXP_BHAPI b_status_t	delete_area(void *area);
IMPEXP_BHAPI b_status_t	delete_area_etc(void *area, bool no_clone);
/* bhapi::resize_area:
 * 	Only the original area that created by "bhapi::create_area" is allowed resizing.
 * 	When it was resized, the clone-area must reclone to get the valid address.
 * */
IMPEXP_BHAPI b_status_t	resize_area(void *area, void **start_addr, size_t new_size);
IMPEXP_BHAPI b_status_t	set_area_protection(void *area, b_uint32 new_protection);
/* locker functions */
IMPEXP_BHAPI void*	create_locker(void);
IMPEXP_BHAPI void*	clone_locker(void* locker);
IMPEXP_BHAPI b_status_t	delete_locker(void* locker);
/* after you calling "bhapi::close_locker":
 * 	1. the next "bhapi::lock_locker..." function call will be failed
 * */
IMPEXP_BHAPI b_status_t	close_locker(void* locker);
IMPEXP_BHAPI b_status_t	lock_locker(void *locker);
IMPEXP_BHAPI b_status_t	lock_locker_etc(void *locker, b_uint32 flags, b_bigtime_t timeout);
IMPEXP_BHAPI b_status_t	unlock_locker(void *locker);
/* bhapi::count_locker_locks:
 * 	return count of locks when locked by current thread,
 * 	return less than 0 when locked by other thread or invalid,
 * 	return 0 when it isn't locked or valid.
 * */
IMPEXP_BHAPI b_int64	count_locker_locks(void *locker);
/* bhapi::*_simple_locker:
 *	The "simple_locker" DO NOT support nested-locking
 * */
IMPEXP_BHAPI void*	create_simple_locker(void);
IMPEXP_BHAPI b_status_t	delete_simple_locker(void* slocker);
IMPEXP_BHAPI bool	lock_simple_locker(void *slocker);
IMPEXP_BHAPI void	unlock_simple_locker(void *slocker);
#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
/* bhapi::memory_tracing_*:
 *	The BHAPI++ use this to handle synchronization problem.
 * */
IMPEXP_BHAPI bool	bhapi::memory_tracing_lock(void);
IMPEXP_BHAPI void	bhapi::memory_tracing_unlock(void);
#endif
/* semaphore functions */
typedef struct sem_info {
    char		name[B_OS_NAME_LENGTH + 1];
    b_int64		latest_holder_team;
    b_int64		latest_holder_thread;
    b_int64		count;
	bool		closed;
} sem_info;

#ifdef __cplusplus
IMPEXP_BHAPI void*	create_sem(b_int64 count, const char *name, bhapi::area_access area_access = BHAPI_AREA_ACCESS_OWNER);
#else
IMPEXP_BHAPI void*	create_sem(b_int64 count, const char *name, bhapi::area_access area_access);
#endif
IMPEXP_BHAPI void*	clone_sem(const char *name);
IMPEXP_BHAPI void*	clone_sem_by_source(void *sem);
IMPEXP_BHAPI b_status_t	get_sem_info(void *sem, bhapi::sem_info *info);
IMPEXP_BHAPI b_status_t	delete_sem(void *sem);
IMPEXP_BHAPI b_status_t	delete_sem_etc(void *sem, bool no_clone);

/* after you calling "bhapi::close_sem()":
 * 	1. the next "bhapi::release_sem..." function call will be failed
 * 	2. the next "bhapi::acquire_sem..." function call will be failed when the sem's count <= 0
 * */
IMPEXP_BHAPI b_status_t	close_sem(void* sem);
IMPEXP_BHAPI b_status_t	acquire_sem(void *sem);
IMPEXP_BHAPI b_status_t	release_sem(void *sem);
IMPEXP_BHAPI b_status_t	acquire_sem_etc(void *sem, b_int64 count, b_uint32 flags, b_bigtime_t timeout);
IMPEXP_BHAPI b_status_t	release_sem_etc(void *sem, b_int64 count, b_uint32 flags);
IMPEXP_BHAPI b_status_t	get_sem_count(void *sem, b_int64 *count);

/* thread functions */
/* Default stack size of thread: 256KB */
IMPEXP_BHAPI b_status_t	snooze(b_bigtime_t microseconds);
IMPEXP_BHAPI b_status_t	snooze_until(b_bigtime_t time, int timebase);
IMPEXP_BHAPI b_int64	get_current_team_id(void);
IMPEXP_BHAPI b_int64	get_current_thread_id(void);
IMPEXP_BHAPI void*	create_thread_by_current_thread(void);
IMPEXP_BHAPI void*	create_thread(b_thread_func threadFunction,
                      b_int32 priority,
					  void *arg,
                      b_int64 *threadId);
IMPEXP_BHAPI void*	open_thread(b_int64 threadId);
IMPEXP_BHAPI b_status_t	delete_thread(void *thread);

/* bhapi::suspend_thread():
 * 	Be careful please !!!
 * 	In POSIX-Thread implementation only supported to suspend the current thread.
 * 	It return B_OK if successed. */
IMPEXP_BHAPI b_status_t	suspend_thread(void *thread);
IMPEXP_BHAPI b_status_t	resume_thread(void *thread);
IMPEXP_BHAPI b_status_t	on_exit_thread(void (*callback)(void *), void *user_data);
IMPEXP_BHAPI b_int64	get_thread_id(void *thread);

enum {
	BHAPI_THREAD_INVALID = 0,
	BHAPI_THREAD_READY,
	BHAPI_THREAD_RUNNING,
	BHAPI_THREAD_EXITED,
	BHAPI_THREAD_SUSPENDED,
};
IMPEXP_BHAPI b_uint32	get_thread_run_state(void *thread);
IMPEXP_BHAPI b_status_t	set_thread_priority(void *thread, b_int32 new_priority);
IMPEXP_BHAPI b_int32	get_thread_priority(void *thread);
IMPEXP_BHAPI void	exit_thread(b_status_t status);
IMPEXP_BHAPI b_status_t	wait_for_thread(void *thread, b_status_t *thread_return_value);
IMPEXP_BHAPI b_status_t	wait_for_thread_etc(void *thread, b_status_t *thread_return_value, b_uint32 flags, b_bigtime_t timeout);

#define BHAPI_MAX_PORT_BUFFER_SIZE		((size_t)4096)
#define BHAPI_VALID_MAX_PORT_QUEUE_LENGTH		((b_int32)300)

/* port functions */
#ifdef __cplusplus
IMPEXP_BHAPI void*	create_port(b_int32 queue_length, const char *name, bhapi::area_access area_access = BHAPI_AREA_ACCESS_OWNER);
#else
IMPEXP_BHAPI void*	create_port(b_int32 queue_length, const char *name, bhapi::area_access area_access);
#endif
IMPEXP_BHAPI void*	open_port(const char *name);
IMPEXP_BHAPI void*	open_port_by_source(void *port);
IMPEXP_BHAPI b_status_t	delete_port(void *port);

/* after you calling "bhapi::close_port":
 * 	1. the next "bhapi::write_port..." function call will be failed
 * 	2. the next "bhapi::read_port..." function call will be failed when queue is empty
 * */
IMPEXP_BHAPI b_status_t	close_port(void *port);

IMPEXP_BHAPI b_status_t	write_port(void *port, b_int32 code, const void *buf, size_t buf_size);
IMPEXP_BHAPI b_size_t	port_buffer_size(void *port);
IMPEXP_BHAPI b_status_t	read_port(void *port, b_int32 *code, void *buf, size_t buf_size);

IMPEXP_BHAPI b_status_t	write_port_etc(void *port, b_int32 code, const void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t timeout);
IMPEXP_BHAPI b_size_t	port_buffer_size_etc(void *port, b_uint32 flags, b_bigtime_t timeout);
IMPEXP_BHAPI b_status_t	read_port_etc(void *port, b_int32 *code, void *buf, size_t buf_size, b_uint32 flags, b_bigtime_t timeout);

IMPEXP_BHAPI b_int32	port_count(void *port);


/* image functions */

IMPEXP_BHAPI void*	load_addon(const char* path);
IMPEXP_BHAPI b_status_t	unload_addon(void *image);
IMPEXP_BHAPI b_status_t	get_image_symbol(void *image, const char *name, void **ptr);

} /* namespace bhapi */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* BHAPI_KERNEL__H */

