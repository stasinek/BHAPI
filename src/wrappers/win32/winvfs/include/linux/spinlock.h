#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

#include <ntifs.h>
#include <asm/atomic.h>

typedef struct {
	KSPIN_LOCK lock;
	KIRQL irql;
} spinlock_t;

typedef struct {
	ERESOURCE rwlock;
} rwlock_t;

#define write_lock(lock) ExAcquireResourceExclusiveLite(&((lock)->rwlock), TRUE)
#define write_unlock(lock) ExReleaseResourceForThreadLite(&((lock)->rwlock), ExGetCurrentResourceThread())
#define read_lock(lock)		ExAcquireResourceSharedLite(&((lock)->rwlock), TRUE)
#define read_unlock(lock)	write_unlock(lock)

#define spin_lock(spin) 
//KeAcquireSpinLock(&((spin)->lock), &((spin)->irql))
#define spin_unlock(spin) 
//KeReleaseSpinLock(&((spin)->lock), (spin)->irql)

#define init_spinlock(spin) KeInitializeSpinLock(&((spin)->lock));

#define SPIN_LOCK_UNLOCKED { 0 , 0 }
#define RW_LOCK_UNLOCKED { 0 }

// the "+" is just a hack
#define atomic_dec_and_lock(atomic, lock) (spin_lock(lock) + atomic_dec_and_test(atomic))
#define ATOMIC_DEC_AND_LOCK

#define rwlock_init(lock)	do { } while(0)
#define spin_lock_init(x)	//do { *(x) = SPIN_LOCK_UNLOCKED; } while(0)

#endif