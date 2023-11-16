#ifndef _I386_SEMAPHORE_H
#define _I386_SEMAPHORE_H

//
//	/include/lonix/semaphore.h
//
// rewritten
//

//#include <asm/system.h>
#include <asm/atomic.h>
//#include <linux/wait.h>
#include <linux/rwsem.h>

struct semaphore {
	PRKSEMAPHORE sema;
};
// -- danny --- temporar -- asta cu semafoarele ar trebui neaparat rezolvat
// trebuie apelat KeInitializeSemaphore
// deci ar trebui cautat peste tot unde se intializeaza o structura care contine un semafor
// si acolo initializat si semaforul de ex inode->i_sem dar mai sunt si altele
#define down(s) 
//KeWaitForSingleObject(&((s)->sema), Executive, KernelMode, FALSE, 0)
#define up(s) 
//KeReleaseSemaphore((s)->sema, 0, 1, FALSE)
// --- end

static __inline void sema_init (struct semaphore *sem, int val)
{
	/*
	atomic_set(&sem->count, val);
	sem->sleepers = 0;
	init_waitqueue_head(&sem->wait);
	*/
}

static inline void init_MUTEX (struct semaphore *sem)
{
	sema_init(sem, 1);
}

#endif
