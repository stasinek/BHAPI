/*
 * Copyright 2002-2006, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef BHAPI_KERNEL_CPU_H
#define BHAPI_KERNEL_CPU_H
#include <arch/cpu.h>
#include <int.h>
#include <smp.h>
#include <timer.h>
#include <setjmp.h>

#include <Scheduler.h>

struct kernel_args;
namespace BKernel {
	struct Thread;
}
using BKernel::Thread;

typedef enum cpu_topology_level {
	CPU_TOPOLOGY_SMT,
	CPU_TOPOLOGY_CORE,
	CPU_TOPOLOGY_PACKAGE,
	//
	CPU_TOPOLOGY_LEVELS
} cpu_topology_level;

typedef struct cpu_topology_node {
	cpu_topology_level	level;
	int					id;
	cpu_topology_node**	children;
	int					children_count;
} cpu_topology_node;

/* CPU local data structure */
typedef struct cpu_ent {
	int				cpu_num;

	// thread.c: used to force a reschedule at quantum expiration time
	bool			preempted;
	timer			quantum_timer;

	// keeping track of CPU activity
	seqlock			active_time_lock;
	bigtime_t		active_time;
	bigtime_t		irq_time;
	bigtime_t		interrupt_time;
	bigtime_t		last_kernel_time;
	bigtime_t		last_user_time;

	int32_t			ici_counter;

	// used in the kernel debugger
	addr_t			fault_handler;
	addr_t			fault_handler_stack_pointer;
	jmp_buf			fault_jump_buffer;

	Thread*			running_thread;
	Thread*			previous_thread;
	bool			invoke_scheduler;
	bool			disabled;

	// CPU topology information
	int				topology_id[CPU_TOPOLOGY_LEVELS];
	int				cache_id[CPU_MAX_CACHE_LEVEL];

	// IRQs assigned to this CPU
	struct list		irqs;
	spinlock		irqs_lock;

	// arch-specific stuff
	arch_cpu_info 	arch;
} cpu_ent CACHE_LINE_ALIGN;

extern cpu_ent gCPU[];
extern uint32_t gCPUCacheLevelCount;

#ifdef __cplusplus
extern "C" {
#endif
status_t cpu_preboot_init_percpu(struct kernel_args *args, int curr_cpu);
status_t cpu_init(struct kernel_args *args);
status_t cpu_init_percpu(struct kernel_args *ka, int curr_cpu);
status_t cpu_init_post_vm(struct kernel_args *args);
status_t cpu_init_post_modules(struct kernel_args *args);
bigtime_t cpu_get_active_time(int32_t cpu);
cpu_ent *get_cpu_struct(void);
extern inline cpu_ent *get_cpu_struct(void) { return &gCPU[smp_get_current_cpu()]; }
status_t cpu_build_topology_tree(void);
const cpu_topology_node* get_cpu_topology(void);
void cpu_set_scheduler_mode(enum scheduler_mode mode);
status_t increase_cpu_performance(int delta);
status_t decrease_cpu_performance(int delta);
void cpu_idle(void);
void cpu_wait(int32_t* variable, int32_t test);

static inline void cpu_pause(void)
{
	arch_cpu_pause();
}

void _user_clear_caches(void *address, size_t length, uint32_t flags);
bool _user_cpu_enabled(int32_t cpu);
status_t _user_set_cpu_enabled(int32_t cpu, bool enabled);

#ifdef __cplusplus
}
#endif
#endif	/* BHAPI_KERNEL_CPU_H */
