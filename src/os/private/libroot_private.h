/*
 * Copyright 2004-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef LIBROOT_PRIVATE_H
#define LIBROOT_PRIVATE_H


#include <Haiku.h>
#include <image.h>


struct user_space_program_args;
struct real_time_data;


#ifdef __cplusplus
extern "C" {
#endif

extern int __gABIVersion;

extern char _single_threaded;
	/* This determines if a process runs single threaded or not */

status_t __parse_invoke_line(char *invoker, char ***_newArgs,
			char * const **_oldArgs, int32_t *_argCount, const char *arg0);
status_t __get_next_image_dependency(image_id id, uint32_t *cookie,
			const char **_name);
status_t __test_executable(const char *path, char *invoker);
status_t __flatten_process_args(const char* const* args, int32_t argCount,
			const char* const* env, int32_t* envCount, const char* executablePath,
			char*** _flatArgs, size_t* _flatSize);
void _call_atexit_hooks_for_range(addr_t start, addr_t size);
void __init_env(const struct user_space_program_args *args);
void __init_env_post_heap(void);
status_t __init_heap(void);
void __heap_terminate_after(void);

void __init_time(addr_t commPageTable);
void __arch_init_time(struct real_time_data *data, bool setDefaults);
bigtime_t __arch_get_system_time_offset(struct real_time_data *data);
bigtime_t __get_system_time_offset();
void __init_pwd_backend(void);
void __reinit_pwd_backend_after_fork(void);
void* __arch_get_caller(void);
int32_t __arch_get_stack_trace(addr_t* returnAddresses, int32_t maxCount,
	int32_t skipFrames, addr_t stackBase, addr_t stackEnd);

void __set_stack_protection(void);


#ifdef __cplusplus
}
#endif

#endif	/* LIBROOT_PRIVATE_H */
