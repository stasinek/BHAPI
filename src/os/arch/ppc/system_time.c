/*
 * Copyright 2006, Ingo Weinhold <bonefish@cs.tu-berlin.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <OS.h>

#include <arch_cpu.h>
#include <libroot_private.h>
#include <real_time_data.h>


static vint32_t *sConversionFactor;

void
__ppc_setup_system_time(vint32_t *cvFactor)
{
	sConversionFactor = cvFactor;
}


bigtime_t
system_time(void)
{
	uint64 timeBase = __ppc_get_time_base();

	uint32_t cv = *sConversionFactor;
	return (timeBase >> 32) * cv + (((timeBase & 0xffffffff) * cv) >> 32);
}
