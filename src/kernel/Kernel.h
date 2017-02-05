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
 * File: Kernel.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_KERNEL_H__
#define __ETK_KERNEL_H__

#include "OS.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* time functions */
_IMPEXP_ETK euint32	etk_real_time_clock(void);
_IMPEXP_ETK e_bigtime_t	etk_real_time_clock_usecs(void);
_IMPEXP_ETK e_bigtime_t	etk_system_boot_time(void); /* system boot time in microseconds */
_IMPEXP_ETK e_bigtime_t etk_system_time(void); /* time since booting in microseconds */

/* area functions */
typedef struct etk_area_info {
	char		name[E_OS_NAME_LENGTH + 1];
	size_t		size;
	euint32		protection;
	void		*address;
	char		domain[5];
} etk_area_info;

#define ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN	"ssem"
#define ETK_AREA_SYSTEM_PORT_DOMAIN		"spot"
#define ETK_AREA_USER_DOMAIN			"user"

typedef enum etk_area_access {
	ETK_AREA_ACCESS_OWNER = 0,
	ETK_AREA_ACCESS_GROUP_READ = 1,
	ETK_AREA_ACCESS_GROUP_WRITE = 1 << 1,
	ETK_AREA_ACCESS_OTHERS_READ = 1 << 2,
	ETK_AREA_ACCESS_OTHERS_WRITE = 1 << 3,
	ETK_AREA_ACCESS_ALL = 0xFF
} etk_area_access;

#ifdef __cplusplus
_IMPEXP_ETK void*	etk_create_area(const char *name, void **start_addr, size_t size, euint32 protection,
				        const char *domain, etk_area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
_IMPEXP_ETK void*	etk_create_area(const char *name, void **start_addr, size_t size, euint32 protection,
				        const char *domain, etk_area_access area_access);
#endif
_IMPEXP_ETK void*	etk_clone_area(const char *name, void **dest_addr, euint32 protection, const char *domain);
_IMPEXP_ETK void*	etk_clone_area_by_source(void *source_area, void **dest_addr, euint32 protection);
_IMPEXP_ETK e_status_t	etk_get_area_info(void *area, etk_area_info *info);
_IMPEXP_ETK e_status_t	etk_delete_area(void *area);
_IMPEXP_ETK e_status_t	etk_delete_area_etc(void *area, bool no_clone);

/* etk_resize_area:
 * 	Only the original area that created by "etk_create_area" is allowed resizing.
 * 	When it was resized, the clone-area must reclone to get the valid address.
 * */
_IMPEXP_ETK e_status_t	etk_resize_area(void *area, void **start_addr, size_t new_size);
_IMPEXP_ETK e_status_t	etk_set_area_protection(void *area, euint32 new_protection);

/* locker functions */
_IMPEXP_ETK void*	etk_create_locker(void);
_IMPEXP_ETK void*	etk_clone_locker(void* locker);
_IMPEXP_ETK e_status_t	etk_delete_locker(void* locker);

/* after you calling "etk_close_locker":
 * 	1. the next "etk_lock_locker..." function call will be failed
 * */
_IMPEXP_ETK e_status_t	etk_close_locker(void* locker);

_IMPEXP_ETK e_status_t	etk_lock_locker(void *locker);
_IMPEXP_ETK e_status_t	etk_lock_locker_etc(void *locker, euint32 flags, e_bigtime_t timeout);
_IMPEXP_ETK e_status_t	etk_unlock_locker(void *locker);

/* etk_count_locker_locks:
 * 	return count of locks when locked by current thread,
 * 	return less than 0 when locked by other thread or invalid,
 * 	return 0 when it isn't locked or valid.
 * */
_IMPEXP_ETK eint64	etk_count_locker_locks(void *locker);

/* etk_*_simple_locker:
 *	The "simple_locker" DO NOT support nested-locking
 * */
_IMPEXP_ETK void*	etk_create_simple_locker(void);
_IMPEXP_ETK e_status_t	etk_delete_simple_locker(void* slocker);
_IMPEXP_ETK bool	etk_lock_simple_locker(void *slocker);
_IMPEXP_ETK void	etk_unlock_simple_locker(void *slocker);

#ifdef ETK_BUILD_WITH_MEMORY_TRACING
/* etk_memory_tracing_*:
 *	The ETK++ use this to handle synchronization problem.
 * */
_IMPEXP_ETK bool	etk_memory_tracing_lock(void);
_IMPEXP_ETK void	etk_memory_tracing_unlock(void);
#endif

/* semaphore functions */
typedef struct etk_sem_info {
	char		name[E_OS_NAME_LENGTH + 1];
	eint64		latest_holder_team;
	eint64		latest_holder_thread;
	eint64		count;
	bool		closed;
} etk_sem_info;

#ifdef __cplusplus
_IMPEXP_ETK void*	etk_create_sem(eint64 count, const char *name, etk_area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
_IMPEXP_ETK void*	etk_create_sem(eint64 count, const char *name, etk_area_access area_access);
#endif
_IMPEXP_ETK void*	etk_clone_sem(const char *name);
_IMPEXP_ETK void*	etk_clone_sem_by_source(void *sem);
_IMPEXP_ETK e_status_t	etk_get_sem_info(void *sem, etk_sem_info *info);
_IMPEXP_ETK e_status_t	etk_delete_sem(void *sem);
_IMPEXP_ETK e_status_t	etk_delete_sem_etc(void *sem, bool no_clone);

/* after you calling "etk_close_sem()":
 * 	1. the next "etk_release_sem..." function call will be failed
 * 	2. the next "etk_acquire_sem..." function call will be failed when the sem's count <= 0
 * */
_IMPEXP_ETK e_status_t	etk_close_sem(void* sem);

_IMPEXP_ETK e_status_t	etk_acquire_sem(void *sem);
_IMPEXP_ETK e_status_t	etk_release_sem(void *sem);
_IMPEXP_ETK e_status_t	etk_acquire_sem_etc(void *sem, eint64 count, euint32 flags, e_bigtime_t timeout);
_IMPEXP_ETK e_status_t	etk_release_sem_etc(void *sem, eint64 count, euint32 flags);
_IMPEXP_ETK e_status_t	etk_get_sem_count(void *sem, eint64 *count);


/* thread functions */
/* Default stack size of thread: 256KB */
_IMPEXP_ETK e_status_t	etk_snooze(e_bigtime_t microseconds);
_IMPEXP_ETK e_status_t	etk_snooze_until(e_bigtime_t time, int timebase);

_IMPEXP_ETK eint64	etk_get_current_team_id(void);
_IMPEXP_ETK eint64	etk_get_current_thread_id(void);

_IMPEXP_ETK void*	etk_create_thread_by_current_thread(void);
_IMPEXP_ETK void*	etk_create_thread(e_thread_func threadFunction,
					  eint32 priority,
					  void *arg,
					  eint64 *threadId);
_IMPEXP_ETK void*	etk_open_thread(eint64 threadId);
_IMPEXP_ETK e_status_t	etk_delete_thread(void *thread);

/* etk_suspend_thread():
 * 	Be careful please !!!
 * 	In POSIX-Thread implementation only supported to suspend the current thread.
 * 	It return E_OK if successed. */
_IMPEXP_ETK e_status_t	etk_suspend_thread(void *thread);
_IMPEXP_ETK e_status_t	etk_resume_thread(void *thread);

_IMPEXP_ETK e_status_t	etk_on_exit_thread(void (*callback)(void *), void *user_data);

_IMPEXP_ETK eint64	etk_get_thread_id(void *thread);

enum {
	ETK_THREAD_INVALID = 0,
	ETK_THREAD_READY,
	ETK_THREAD_RUNNING,
	ETK_THREAD_EXITED,
	ETK_THREAD_SUSPENDED,
};
_IMPEXP_ETK euint32	etk_get_thread_run_state(void *thread);

_IMPEXP_ETK e_status_t	etk_set_thread_priority(void *thread, eint32 new_priority);
_IMPEXP_ETK eint32	etk_get_thread_priority(void *thread);
_IMPEXP_ETK void	etk_exit_thread(e_status_t status);
_IMPEXP_ETK e_status_t	etk_wait_for_thread(void *thread, e_status_t *thread_return_value);
_IMPEXP_ETK e_status_t	etk_wait_for_thread_etc(void *thread, e_status_t *thread_return_value, euint32 flags, e_bigtime_t timeout);


#define ETK_MAX_PORT_BUFFER_SIZE		((size_t)4096)
#define ETK_VALID_MAX_PORT_QUEUE_LENGTH		((eint32)300)


/* port functions */
#ifdef __cplusplus
_IMPEXP_ETK void*	etk_create_port(eint32 queue_length, const char *name, etk_area_access area_access = ETK_AREA_ACCESS_OWNER);
#else
_IMPEXP_ETK void*	etk_create_port(eint32 queue_length, const char *name, etk_area_access area_access);
#endif
_IMPEXP_ETK void*	etk_open_port(const char *name);
_IMPEXP_ETK void*	etk_open_port_by_source(void *port);
_IMPEXP_ETK e_status_t	etk_delete_port(void *port);

/* after you calling "etk_close_port":
 * 	1. the next "etk_write_port..." function call will be failed
 * 	2. the next "etk_read_port..." function call will be failed when queue is empty
 * */
_IMPEXP_ETK e_status_t	etk_close_port(void *port);

_IMPEXP_ETK e_status_t	etk_write_port(void *port, eint32 code, const void *buf, size_t buf_size);
_IMPEXP_ETK ssize_t	etk_port_buffer_size(void *port);
_IMPEXP_ETK e_status_t	etk_read_port(void *port, eint32 *code, void *buf, size_t buf_size);

_IMPEXP_ETK e_status_t	etk_write_port_etc(void *port, eint32 code, const void *buf, size_t buf_size, euint32 flags, e_bigtime_t timeout);
_IMPEXP_ETK ssize_t	etk_port_buffer_size_etc(void *port, euint32 flags, e_bigtime_t timeout);
_IMPEXP_ETK e_status_t	etk_read_port_etc(void *port, eint32 *code, void *buf, size_t buf_size, euint32 flags, e_bigtime_t timeout);

_IMPEXP_ETK eint32	etk_port_count(void *port);


/* image functions */

_IMPEXP_ETK void*	etk_load_addon(const char* path);
_IMPEXP_ETK e_status_t	etk_unload_addon(void *image);
_IMPEXP_ETK e_status_t	etk_get_image_symbol(void *image, const char *name, void **ptr);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __ETK_KERNEL_H__ */

