﻿/*
 * Copyright 2008-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2002-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef _KERNEL_LOCK_H
#define _KERNEL_LOCK_H


#include <kits/kernel/OS.h>

#include <arch/atomic.h>
#include <kits/debug/Debug.h>


struct mutex_waiter;

typedef struct mutex {
    const char*				name;
    struct mutex_waiter*	waiters;
    spinlock				lock;
#if KDEBUG
    thread_id				holder;
#else
    int32_t					count;
    uint16					ignore_unlock_count;
#endif
    uint8					flags;
} mutex;

#define MUTEX_FLAG_CLONE_NAME	0x1


typedef struct recursive_lock {
    mutex		lock;
#if !KDEBUG
    thread_id	holder;
#endif
    int			recursion;
} recursive_lock;


struct rw_lock_waiter;

typedef struct rw_lock {
    const char*				name;
    struct rw_lock_waiter*	waiters;
    spinlock				lock;
    thread_id				holder;
    int32_t					count;
    int32_t					owner_count;
    int16					active_readers;
                                // Only > 0 while a writer is waiting: number
                                // of active readers when the first waiting
                                // writer started waiting.
    int16					pending_readers;
                                // Number of readers that have already
                                // incremented "count", but have not yet started
                                // to wait at the time the last writer unlocked.
    uint32_t					flags;
} rw_lock;

#define RW_LOCK_WRITER_COUNT_BASE	0x10000

#define RW_LOCK_FLAG_CLONE_NAME	0x1


#if KDEBUG
#	define KDEBUG_RW_LOCK_DEBUG 0
        // Define to 1 if you want to use ASSERT_READ_LOCKED_RW_LOCK().
        // The rw_lock will just behave like a recursive locker then.
#	define ASSERT_LOCKED_RECURSIVE(r) \
        { ASSERT(find_thread(NULL) == (r)->lock.holder); }
#	define ASSERT_LOCKED_MUTEX(m) { ASSERT(find_thread(NULL) == (m)->holder); }
#	define ASSERT_WRITE_LOCKED_RW_LOCK(l) \
        { ASSERT(find_thread(NULL) == (l)->holder); }
#	if KDEBUG_RW_LOCK_DEBUG
#		define ASSERT_READ_LOCKED_RW_LOCK(l) \
            { ASSERT(find_thread(NULL) == (l)->holder); }
#	else
#		define ASSERT_READ_LOCKED_RW_LOCK(l) do {} while (false)
#	endif
#else
#	define ASSERT_LOCKED_RECURSIVE(r)		do {} while (false)
#	define ASSERT_LOCKED_MUTEX(m)			do {} while (false)
#	define ASSERT_WRITE_LOCKED_RW_LOCK(m)	do {} while (false)
#	define ASSERT_READ_LOCKED_RW_LOCK(l)	do {} while (false)
#endif


// static initializers
#if KDEBUG
#	define MUTEX_INITIALIZER(name) \
    { name, NULL, B_SPINLOCK_INITIALIZER, -1, 0 }
#	define RECURSIVE_LOCK_INITIALIZER(name)	{ MUTEX_INITIALIZER(name), 0 }
#else
#	define MUTEX_INITIALIZER(name) \
    { name, NULL, B_SPINLOCK_INITIALIZER, 0, 0, 0 }
#	define RECURSIVE_LOCK_INITIALIZER(name)	{ MUTEX_INITIALIZER(name), -1, 0 }
#endif

#define RW_LOCK_INITIALIZER(name) \
    { name, NULL, B_SPINLOCK_INITIALIZER, -1, 0, 0, 0 }


#if KDEBUG
#	define RECURSIVE_LOCK_HOLDER(recursiveLock)	((recursiveLock)->lock.holder)
#else
#	define RECURSIVE_LOCK_HOLDER(recursiveLock)	((recursiveLock)->holder)
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
namespace bhapi {
#endif

extern void	recursive_lock_init(recursive_lock *lock, const char *name);
    // name is *not* cloned nor freed in recursive_lock_destroy()
extern void recursive_lock_init_etc(recursive_lock *lock, const char *name,
    uint32_t flags);
extern void recursive_lock_destroy(recursive_lock *lock);
extern status_t recursive_lock_lock(recursive_lock *lock);
extern status_t recursive_lock_trylock(recursive_lock *lock);
extern void recursive_lock_unlock(recursive_lock *lock);
extern int32_t recursive_lock_get_recursion(recursive_lock *lock);

extern void rw_lock_init(rw_lock* lock, const char* name);
    // name is *not* cloned nor freed in rw_lock_destroy()
extern void rw_lock_init_etc(rw_lock* lock, const char* name, uint32_t flags);
extern void rw_lock_destroy(rw_lock* lock);
extern status_t rw_lock_write_lock(rw_lock* lock);

extern void mutex_init(mutex* lock, const char* name);
    // name is *not* cloned nor freed in mutex_destroy()
extern void mutex_init_etc(mutex* lock, const char* name, uint32_t flags);
extern void mutex_destroy(mutex* lock);
extern status_t mutex_switch_lock(mutex* from, mutex* to);
    // Unlocks "from" and locks "to" such that unlocking and starting to wait
    // for the lock is atomically. I.e. if "from" guards the object "to" belongs
    // to, the operation is safe as long as "from" is held while destroying
    // "to".
extern status_t mutex_switch_from_read_lock(rw_lock* from, mutex* to);
    // Like mutex_switch_lock(), just for a switching from a read-locked
    // rw_lock.


// implementation private:

extern status_t _rw_lock_read_lock(rw_lock* lock);
extern status_t _rw_lock_read_lock_with_timeout(rw_lock* lock,
    uint32_t timeoutFlags, bigtime_t timeout);
extern void _rw_lock_read_unlock(rw_lock* lock);
extern void _rw_lock_write_unlock(rw_lock* lock);

extern status_t _mutex_lock(mutex* lock, void* locker);
extern void _mutex_unlock(mutex* lock);
extern status_t _mutex_trylock(mutex* lock);
extern status_t _mutex_lock_with_timeout(mutex* lock, uint32_t timeoutFlags,
    bigtime_t timeout);


static inline status_t rw_lock_read_lock(rw_lock* lock)
{
#if KDEBUG_RW_LOCK_DEBUG
    return rw_lock_write_lock(lock);
#else
    int32_t oldCount = atomic_add(&lock->count, 1);
    if (oldCount >= RW_LOCK_WRITER_COUNT_BASE)
        return _rw_lock_read_lock(lock);
    return B_OK;
#endif
}


static inline status_t rw_lock_read_lock_with_timeout(rw_lock* lock, uint32_t timeoutFlags,
    bigtime_t timeout)
{
#if KDEBUG_RW_LOCK_DEBUG
    return mutex_lock_with_timeout(lock, timeoutFlags, timeout);
#else
    int32_t oldCount = atomic_add(&lock->count, 1);
    if (oldCount >= RW_LOCK_WRITER_COUNT_BASE)
        return _rw_lock_read_lock_with_timeout(lock, timeoutFlags, timeout);
    return B_OK;
#endif
}


static inline void rw_lock_read_unlock(rw_lock* lock)
{
#if KDEBUG_RW_LOCK_DEBUG
    rw_lock_write_unlock(lock);
#else
    int32_t oldCount = atomic_add(&lock->count, -1);
    if (oldCount >= RW_LOCK_WRITER_COUNT_BASE)
        _rw_lock_read_unlock(lock);
#endif
}


static inline void rw_lock_write_unlock(rw_lock* lock)
{
    _rw_lock_write_unlock(lock);
}


static inline status_t mutex_lock(mutex* lock)
{
#if KDEBUG
    return _mutex_lock(lock, NULL);
#else
    if (atomic_add(&lock->count, -1) < 0)
        return _mutex_lock(lock, NULL);
    return B_OK;
#endif
}


static inline status_t mutex_trylock(mutex* lock)
{
#if KDEBUG
    return _mutex_trylock(lock);
#else
    if (atomic_test_and_set(&lock->count, -1, 0) != 0)
        return B_WOULD_BLOCK;
    return B_OK;
#endif
}


static inline status_t mutex_lock_with_timeout(mutex* lock, uint32_t timeoutFlags, bigtime_t timeout)
{
#if KDEBUG
    return _mutex_lock_with_timeout(lock, timeoutFlags, timeout);
#else
    if (atomic_add(&lock->count, -1) < 0)
        return _mutex_lock_with_timeout(lock, timeoutFlags, timeout);
    return B_OK;
#endif
}


static inline void mutex_unlock(mutex* lock)
{
#if !KDEBUG
    if (atomic_add(&lock->count, 1) < -1)
#endif
        _mutex_unlock(lock);
}


static inline void mutex_transfer_lock(mutex* lock, thread_id thread)
{
#if KDEBUG
    lock->holder = thread;
#endif
}


static inline void recursive_lock_transfer_lock(recursive_lock* lock, thread_id thread)
{
    if (lock->recursion != 1)
        panic("invalid recursion level for lock transfer!");

#if KDEBUG
    lock->lock.holder = thread;
#else
    lock->holder = thread;
#endif
}
extern void lock_debug_init();

#ifdef __cplusplus
} // C++
#endif
#ifdef __cplusplus
} // namespace bhapi
#endif

#endif	/* _KERNEL_LOCK_H */
