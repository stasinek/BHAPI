﻿/*
 * Copyright 2002-2012 Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SYS_TYPES_H
#define BHAPI_SYS_TYPES_H

#include <Haiku.h>

/* BSD compatibility */
typedef unsigned long 		u_long;
typedef unsigned int 		u_int;
typedef unsigned short 		u_short;
typedef unsigned char 		u_char;


/* sysV compatibility */
typedef unsigned long 		ulong;
typedef unsigned short 		ushort;
typedef unsigned int 		uint;
typedef unsigned char		unchar;

typedef __haiku_int64 		blkcnt_t;
typedef __haiku_std_int32	blksize_t;
typedef __haiku_int64		fsblkcnt_t;
typedef __haiku_int64		fsfilcnt_t;
typedef __haiku_std_uint32	umode_t;
typedef __haiku_std_int32	nlink_t;
typedef __haiku_std_int32	cnt_t;
typedef __haiku_int32		id_t;

typedef int _pid_t;
typedef _pid_t              pid_t;
typedef int _uid_t;
typedef _uid_t              uid_t;
typedef int _gid_t;
typedef _gid_t              gid_t;

//typedef __haiku_int64		off_t; // stdio.h
//typedef __haiku_int64		ino_t;
//typedef __haiku_std_uint32  mode_t;
//typedef __haiku_int32		dev_t;

#ifdef __HAIKU_BEOS_COMPATIBLE_TYPES
    typedef int		daddr_t;	/* disk address */
#else
    typedef off_t	daddr_t;	/* disk address */
#endif
typedef char*				caddr_t;

typedef __haiku_addr_t		addr_t;
typedef __haiku_int32 		key_t;

typedef __haiku_std_int32	clockid_t;
typedef struct __timer_t*	timer_t;

/* pthread types */
#if !defined(WINPTHREAD_API)
typedef struct	_pthread_thread		*pthread_t;
typedef struct  _pthread_attr		*pthread_attr_t;
typedef struct  _pthread_mutex		pthread_mutex_t;
typedef struct  _pthread_mutexattr	*pthread_mutexattr_t;
typedef struct  _pthread_cond		pthread_cond_t;
typedef struct  _pthread_condattr	*pthread_condattr_t;
typedef unsigned int				pthread_key_t;
typedef struct  _pthread_once		pthread_once_t;
typedef struct  _pthread_rwlock		pthread_rwlock_t;
typedef struct  _pthread_rwlockattr	*pthread_rwlockattr_t;
typedef struct  _pthread_spinlock	pthread_spinlock_t;
/*
typedef struct  _pthread_barrier	*pthread_barrier_t;
typedef struct  _pthread_barrierattr *pthread_barrierattr_t;
*/

struct _pthread_mutex {
    __haiku_std_uint32	flags;
    __haiku_std_int32	lock;
    __haiku_std_int32	unused;
    __haiku_std_int32	owner;
    __haiku_std_int32	owner_count;
};

struct _pthread_cond {
    __haiku_std_uint32	flags;
    __haiku_std_int32	unused;
    pthread_mutex_t*	mutex;
    __haiku_std_int32	waiter_count;
    __haiku_std_int32	lock;
};

struct _pthread_once {
    __haiku_std_int32	state;
};

struct _pthread_rwlock {
    __haiku_std_uint32	flags;
    __haiku_std_int32	owner;
    union {
        struct {
            __haiku_std_int32	sem;
        } shared;
        struct {
            __haiku_std_int32	lock_sem;
            __haiku_std_int32	lock_count;
            __haiku_std_int32	reader_count;
            __haiku_std_int32	writer_count;
            void*				waiters[2];
        } local;
    } u;
};

struct _pthread_spinlock {
    __haiku_std_int32		lock;
};
#endif

#include <posix/null.h>
#include <posix/size_t.h>
#include <posix/time.h>

#endif
