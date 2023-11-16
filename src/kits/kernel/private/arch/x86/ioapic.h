/*
 * Copyright 2011, Michael Lotz, mmlr@mlotz.ch.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_x86_IOAPIC_H
#define _KERNEL_ARCH_x86_IOAPIC_H

#include <Haiku.h>

struct kernel_args;

bool ioapic_is_interrupt_available(int32_t gsi);

void ioapic_init(kernel_args* args);

#endif // _KERNEL_ARCH_x86_IOAPIC_H
