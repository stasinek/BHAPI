#ifndef __ASM_SMPLOCK_H
#define __ASM_SMPLOCK_H

//#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <asm/current.h>



#define lock_kernel() spin_lock(&kernel_flag)
#define unlock_kernel() spin_unlock(&kernel_flag)

extern spinlock_t kernel_flag;

#endif /* __ASM_SMPLOCK_H */
