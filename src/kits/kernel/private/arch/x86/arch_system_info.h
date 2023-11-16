/*
 * Copyright 2004-2005, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_x86_SYSTEM_INFO_H
#define _KERNEL_ARCH_x86_SYSTEM_INFO_H


#include <kits/kernel/OS.h>

#ifdef __cplusplus
extern "C" {
#endif

status_t get_current_cpuid(cpuid_info* info, uint32_t eax, uint32_t ecx);
uint32_t get_eflags(void);
void set_eflags(uint32_t value);

status_t _user_get_cpuid(cpuid_info* info, uint32_t eax, uint32_t cpu);

#ifdef __cplusplus
}
#endif

#endif	/* _KRENEL_ARCH_x86_SYSTEM_INFO_H */
