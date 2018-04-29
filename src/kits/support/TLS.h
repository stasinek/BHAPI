/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku OS source-code, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: TLS.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2003-2007 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_TLS_H
#define BHAPI_TLS_H

#include <kits/support/SupportDefs.h>

/* A maximum of 64 keys is allowed to store in TLS - the key is reserved
 * process-wide. Note that tls_allocate() will return B_NO_MEMORY if you
 * try to exceed this limit.
 */
#define TLS_MAX_KEYS 64

#ifdef __cplusplus
extern "C" {
namespace bhapi {
#endif	/* __cplusplus */

extern int32 tls_allocate(void);

#if !_NO_INLINE_ASM && __INTEL__ && __GNUC__

static inline void *tls_get(int32 index)
{
    void *ret;
    __asm__ __volatile__ (
        "movl	%%fs:(, %1, 4), %0"
        : "=r" (ret) : "r" (index));
    return ret;
}

static inline void **tls_address(int32 index)
{
    void **ret;
    __asm__ __volatile__ (
        "movl	%%fs:0, %0\n\t"
        "leal	(%0, %1, 4), %0\n\t"
        : "=&r" (ret) : "r" (index));
    return ret;
}

static inline void tls_set(int32 index, void *value)
{
    __asm__ __volatile__ (
        "movl	%1, %%fs:(, %0, 4)"
        : : "r" (index), "r" (value));
}

#else	/* !_NO_INLINE_ASM && __INTEL__ && __GNUC__ */

extern void *tls_get(int32 index);
extern void **tls_address(int32 index);
extern void tls_set(int32 index, void *value);

#endif	/* !_NO_INLINE_ASM && __INTEL__ && __GNUC__ */

#ifdef __cplusplus
} /* namespace */
}
#endif	/* __cplusplus */

#endif	/* _TLS_H */
