/* --------------------------------------------------------------------------
 *
 * BHAPI++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: BHAPI.h
 *
 * --------------------------------------------------------------------------*/

#include "support/SupportDefs.h"

#define B_BEOS_VERSION_4				0x0400
#define B_BEOS_VERSION_4_5				0x0450
#define B_BEOS_VERSION_5				0x0500

#define B_BEOS_VERSION					B_BEOS_VERSION_5
#define B_BEOS_VERSION_MAUI				B_BEOS_VERSION_5

/* Haiku (API) version */
#define B_HAIKU_VERSION_BEOS			0x00000001
#define B_HAIKU_VERSION_BONE			0x00000002
#define B_HAIKU_VERSION_DANO			0x00000003
#define B_HAIKU_VERSION_1_ALPHA_1		0x00000100
#define B_HAIKU_VERSION_1_PRE_ALPHA_2	0x00000101
#define B_HAIKU_VERSION_1_ALPHA_2		0x00000200
#define B_HAIKU_VERSION_1_PRE_ALPHA_3	0x00000201
#define B_HAIKU_VERSION_1_ALPHA_3		0x00000300
#define B_HAIKU_VERSION_1_PRE_ALPHA_4	0x00000301
#define B_HAIKU_VERSION_1_ALPHA_4		0x00000400
#define B_HAIKU_VERSION_1_PRE_BETA_1	0x00000401
#define B_HAIKU_VERSION_1				0x00010000

#define B_HAIKU_VERSION					B_HAIKU_VERSION_1_PRE_ALPHA_4

/* Haiku ABI */
#define B_HAIKU_ABI_MAJOR				0xffff0000
#define B_HAIKU_ABI_GCC_2				0x00020000
#define B_HAIKU_ABI_GCC_4				0x00040000

#define B_HAIKU_ABI_GCC_2_ANCIENT		0x00020000
#define B_HAIKU_ABI_GCC_2_BEOS			0x00020001
#define B_HAIKU_ABI_GCC_2_HAIKU			0x00020002

#define B_HAIKU_ABI_NAME				__HAIKU_ARCH_ABI

#if __GNUC__ == 2
#	define B_HAIKU_ABI					B_HAIKU_ABI_GCC_2_HAIKU
#elif __GNUC__ == 4 || __GNUC__ == 5 || __GNUC__ == 6
#	define B_HAIKU_ABI					B_HAIKU_ABI_GCC_4
#else
#	error Unsupported gcc version!
#endif

#define B_HAIKU_BITS					__HAIKU_ARCH_BITS
#define B_HAIKU_PHYSICAL_BITS			__HAIKU_ARCH_PHYSICAL_BITS

#ifdef __HAIKU_ARCH_64_BIT
#	define B_HAIKU_64_BIT				1
#else
#	define B_HAIKU_32_BIT				1
#endif

#ifdef __HAIKU_ARCH_PHYSICAL_64_BIT
#	define B_HAIKU_PHYSICAL_64_BIT		1
#else
#	define B_HAIKU_PHYSICAL_32_BIT		1
#endif

#ifdef __HAIKU_BEOS_COMPATIBLE
#	define B_HAIKU_BEOS_COMPATIBLE		1
#endif


#define _UNUSED(argument) argument
#define _PACKED __attribute__((packed))
#define _PRINTFLIKE(_format_, _args_) \
    __attribute__((format(__printf__, _format_, _args_)))
#define _EXPORT
#define _IMPORT

#define B_DEFINE_SYMBOL_VERSION(function, versionedSymbol)	\
    __asm__(".symver " function "," versionedSymbol)


#ifdef __cplusplus
#	define B_DEFINE_WEAK_ALIAS(name, alias_name)	\
        extern "C" __typeof(name) alias_name __attribute__((weak, alias(#name)))
#else
#	define B_DEFINE_WEAK_ALIAS(name, alias_name)	\
        __typeof(name) alias_name __attribute__((weak, alias(#name)))
#endif

#define BHAPI_MAJOR_VERSION 0
#define BHAPI_MINOR_VERSION 0
#define BHAPI_MICRO_VERSION 1
#define BHAPI_INTERFACE_AGE 1
#define BHAPI_BINARY_AGE 1

#ifndef BHAPI_BUILD_LIBRARY
#ifdef __cplusplus
extern "C" {
namespace bhapi {
#endif // __cplusplus
extern IMPEXPBHAPI const b_uint8 major_version;
extern IMPEXPBHAPI const b_uint8 minor_version;
extern IMPEXPBHAPI const b_uint8 micro_version;
extern IMPEXPBHAPI const b_uint8 interface_age;
extern IMPEXPBHAPI const b_uint16 binary_age;
#ifdef __cplusplus
} /* namespace */
}
#endif // __cplusplus
#else
#define major_version BHAPI_MAJOR_VERSION
#define minor_version BHAPI_MINOR_VERSION
#define micro_version BHAPI_MICRO_VERSION
#define interface_age BHAPI_INTERFACE_AGE
#define binary_age BHAPI_BINARY_AGE
#endif

#include "AppKit.h"
#include "KernelKit.h"
#include "StorageKit.h"
#include "RenderKit.h"
#include "InterfaceKit.h"
#include "XmlKit.h"
#include "NetKit.h"
#include "SupportKit.h"
