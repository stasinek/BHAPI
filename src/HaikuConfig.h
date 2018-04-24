/*
 * Copyright 2009-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_HAIKU_CONFIG_H
#define BHAPI_HAIKU_CONFIG_H

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

/* Determine the architecture and define macros for some fundamental
   properties:
   __HAIKU_ARCH					- short name of the architecture (used in paths)
   __HAIKU_ARCH_ABI				- name of ABI (as in package architecture)
   __HAIKU_ARCH_<arch>			- defined to 1 for the respective architecture
   __HAIKU_ARCH_BITS			- defined to 32/64 on 32/64 bit architectures
                                  (defaults to 32)
   __HAIKU_ARCH_PHYSICAL_BITS	- defined to 32/64 on architectures with 32/64
                                  (defaults to __HAIKU_ARCH_BITS)
   __HAIKU_BIG_ENDIAN			- defined to 1 on big endian architectures
                                  (defaults to undefined)
*/
#ifdef __INTEL__
#	define __HAIKU_ARCH					x86
#	if __GNUC__ == 2
#		define __HAIKU_ARCH_ABI			"x86_gcc2"
#	else
#		define __HAIKU_ARCH_ABI			"x86"
#	endif
#	define __HAIKU_ARCH_X86				1
#	define __HAIKU_ARCH_PHYSICAL_BITS	32
#elif __x86_64__
#	define __HAIKU_ARCH					x86_64
#	define __HAIKU_ARCH_ABI				"x86_64"
#	define __HAIKU_ARCH_X86_64			1
#	define __HAIKU_ARCH_BITS			64
#elif __POWERPC__
#	define __HAIKU_ARCH					ppc
#	define __HAIKU_ARCH_ABI				"ppc"
#	define __HAIKU_ARCH_PPC				1
#	define __HAIKU_ARCH_PHYSICAL_BITS	64
#	define __HAIKU_BIG_ENDIAN			1
#elif __M68K__
#	define __HAIKU_ARCH					m68k
#	define __HAIKU_ARCH_ABI				"m68k"
#	define __HAIKU_ARCH_M68K			1
#	define __HAIKU_BIG_ENDIAN			1
#elif __MIPSEL__
#	define __HAIKU_ARCH					mipsel
#	define __HAIKU_ARCH_ABI				"mipsel"
#	define __HAIKU_ARCH_MIPSEL			1
#elif __ARM__ || __ARMEL__
#	define __HAIKU_ARCH					arm
#	define __HAIKU_ARCH_ABI				"arm"
#	define __HAIKU_ARCH_ARM				1
#elif __ARMEB__
#	define __HAIKU_ARCH					armeb
#	define __HAIKU_ARCH_ABI				"armeb"
#	define __HAIKU_ARCH_ARM				1
#	define __HAIKU_BIG_ENDIAN			1
#else
#	error Unsupported architecture!
#endif

/* implied properties:
   __HAIKU_ARCH_{32,64}_BIT		- defined to 1 on 32/64 bit architectures, i.e.
                                  using 32/64 bit virtual addresses
   __HAIKU_ARCH_PHYSICAL_BITS	- defined to 32/64 on architectures with 32/64
                                  bit physical addresses
   __HAIKU_ARCH_PHYSICAL_{32,64}_BIT - defined to 1 on architectures using 64
                                  bit physical addresses
   __HAIKU_BIG_ENDIAN			- defined to 1 on big endian architectures
*/

/* bitness */
#ifndef __HAIKU_ARCH_BITS
#	define __HAIKU_ARCH_BITS		32
#endif

#if __HAIKU_ARCH_BITS == 32
#	define __HAIKU_ARCH_32_BIT		1
#elif __HAIKU_ARCH_BITS == 64
#	define __HAIKU_ARCH_64_BIT		1
#else
#	error Unsupported bitness!
#endif

/* physical bitness */
#ifndef __HAIKU_ARCH_PHYSICAL_BITS
#	define __HAIKU_ARCH_PHYSICAL_BITS	__HAIKU_ARCH_BITS
#endif

#if __HAIKU_ARCH_PHYSICAL_BITS == 32
#	define __HAIKU_ARCH_PHYSICAL_32_BIT	1
#elif __HAIKU_ARCH_PHYSICAL_BITS == 64
#	define __HAIKU_ARCH_PHYSICAL_64_BIT	1
#else
#	error Unsupported physical bitness!
#endif

/* endianess */
#ifndef __HAIKU_BIG_ENDIAN
#	define	__HAIKU_LITTLE_ENDIAN	1
#endif

/* architecture specific include macros */
#define __HAIKU_ARCH_HEADER(header)					<arch/__HAIKU_ARCH/header>
#define __HAIKU_SUBDIR_ARCH_HEADER(subdir, header)	\
    <subdir/arch/__HAIKU_ARCH/header>

/* BeOS R5 binary compatibility (gcc 2 on x86) */
#if defined(__HAIKU_ARCH_X86) && __GNUC__ == 2
#	define __HAIKU_BEOS_COMPATIBLE	1
#endif

/* BeOS R5 compatible types */
#ifndef __HAIKU_ARCH_64_BIT
/*#ifdef __HAIKU_ARCH_X86*/
    /* TODO: This should be "#ifdef __HAIKU_BEOS_COMPATIBLE", but this will
       break all gcc 4 C++ optional packages. I.e. switch that at a suitable
       time.
    */
#	define __HAIKU_BEOS_COMPATIBLE_TYPES	1
#endif

#endif	/* BHAPI_HAIKU_CONFIG_H */
