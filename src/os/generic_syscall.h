/*
 * Copyright 2004, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_GENERIC_SYSCALLS_H
#define _KERNEL_GENERIC_SYSCALLS_H


#include <Haiku.h>

#include <generic_syscall_defs.h>


/* If we decide to make this API public, the contents of this file
 * should be moved to KernelExport.h
 */

typedef status_t (*syscall_hook)(const char *subsystem, uint32_t function, void *buffer, size_t bufferSize);

/* syscall flags */
#define B_SYSCALL_NOT_REPLACEABLE	1
#define B_DO_NOT_REPLACE_SYSCALL	2


#ifdef __cplusplus
extern "C" {
#endif

status_t register_generic_syscall(const char *subsystem, syscall_hook hook,
			uint32_t version, uint32_t flags);
status_t unregister_generic_syscall(const char *subsystem, uint32_t version);

#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL_GENERIC_SYSCALLS_H */
