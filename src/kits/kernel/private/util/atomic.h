/*
 * Copyright 2008, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_UTIL_ATOMIC_H
#define _KERNEL_UTIL_ATOMIC_H


#include <limits.h>

#include <Haiku.h>

#include <kits/debug/Debug.h>


#ifdef __cplusplus

template<typename PointerType> PointerType*
atomic_pointer_test_and_set(PointerType** _pointer, const PointerType* set,
	const PointerType* test)
{
#if LONG_MAX == INT_MAX
	return (PointerType*)atomic_test_and_set((int32_t*)_pointer, (int32_t)set,
		(int32_t)test);
#else
	return (PointerType*)atomic_test_and_set64((int64*)_pointer, (int64)set,
		(int64)test);
#endif
}


template<typename PointerType> PointerType*
atomic_pointer_get_and_set(PointerType** _pointer, const PointerType* set)
{
#if LONG_MAX == INT_MAX
	return (PointerType*)atomic_get_and_set((int32_t*)_pointer, (int32_t)set);
#else
	return (PointerType*)atomic_get_and_set64((int64*)_pointer, (int64)set);
#endif
}


template<typename PointerType> void atomic_pointer_set(PointerType** _pointer, const PointerType* set)
{
	ASSERT((addr_t(_pointer) & (sizeof(PointerType*) - 1)) == 0);
#if LONG_MAX == INT_MAX
	atomic_set((int32_t*)_pointer, (int32_t)set);
#else
	atomic_set64((int64*)_pointer, (int64)set);
#endif
}


template<typename PointerType> PointerType*
atomic_pointer_get(PointerType** _pointer)
{
	ASSERT((addr_t(_pointer) & (sizeof(PointerType*) - 1)) == 0);
#if LONG_MAX == INT_MAX
	return (PointerType*)atomic_get((int32_t*)_pointer);
#else
	return (PointerType*)atomic_get64((int64*)_pointer);
#endif
}

#endif	// __cplusplus

#endif	/* _KERNEL_UTIL_ATOMIC_H */
