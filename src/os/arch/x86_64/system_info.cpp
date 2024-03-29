/* 
 * Copyright 2014, Paweł Dziepak, pdziepak@quarnos.org.
 * Copyright 2003-2004, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */


#include <kits/kernel/OS.h>
#include <musl/syscalls.h>


status_t get_cpuid(cpuid_info* info, uint32_t eaxRegister, uint32_t cpuNum)
{
	__asm__("cpuid"
		: "=a" (info->regs.eax), "=b" (info->regs.ebx), "=c" (info->regs.ecx),
			"=d" (info->regs.edx)
		: "a" (eaxRegister), "c" (0));
	return B_OK;
}

