/*
 * Copyright 2002-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
 * Distributed under the terms of the NewOS License.
 */
#ifndef KERNEL_SEM_H
#define KERNEL_SEM_H


#include <kits/kernel/OS.h>
#include <thread.h>


struct kernel_args;
struct select_info;


#ifdef __cplusplus
extern "C" {
#endif

extern status_t haiku_sem_init(struct kernel_args *args);
extern void sem_delete_owned_sems(Team* team);
extern int32_t sem_used_sems(void);
extern int32_t sem_max_sems(void);

extern status_t select_sem(int32_t object, struct select_info *info, bool kernel);
extern status_t deselect_sem(int32_t object, struct select_info *info,
			bool kernel);

extern sem_id create_sem_etc(int32_t count, const char *name, team_id owner);

extern const char* sem_get_name_unsafe(sem_id id);

/* user calls */
sem_id _user_create_sem(int32_t count, const char *name);
status_t _user_delete_sem(sem_id id);
status_t _user_acquire_sem(sem_id id);
status_t _user_acquire_sem_etc(sem_id id, int32_t count, uint32_t flags, bigtime_t timeout);
status_t _user_switch_sem(sem_id releaseSem, sem_id id);
status_t _user_switch_sem_etc(sem_id releaseSem, sem_id id, int32_t count, uint32_t flags, bigtime_t timeout);
status_t _user_release_sem(sem_id id);
status_t _user_release_sem_etc(sem_id id, int32_t count, uint32_t flags);
status_t _user_get_sem_count(sem_id id, int32_t* thread_count);
status_t _user_get_sem_info(sem_id, struct sem_info *, size_t);
status_t _user_get_next_sem_info(team_id, int32_t *, struct sem_info *, size_t);
status_t _user_set_sem_owner(sem_id id, team_id team);

#ifdef __cplusplus
}
#endif

#endif	/* KERNEL_SEM_H */
