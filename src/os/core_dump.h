/*
 * Copyright 2016, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 *
 * Core dump support.
 */
#ifndef BHAPI_KERNEL_CORE_DUMP_H
#define BHAPI_KERNEL_CORE_DUMP_H
#include <Haiku.h>

status_t	core_dump_write_core_file(const char* path, bool killTeam);
void		core_dump_trap_thread();

#endif	// BHAPI_KERNEL_CORE_DUMP_H
