/*
 * Copyright 2014, Paweł Dziepak, pdziepak@quarnos.org.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_ARCH_GENERIC_USER_MEMORY_H
#define _KERNEL_ARCH_GENERIC_USER_MEMORY_H


#include <atomic>

#include <setjmp.h>
#include <string.h>

#include <thread.h>


namespace {


struct FaultHandlerGuard {
	FaultHandlerGuard()
	{
		ASSERT(thread_get_current_thread()->fault_handler == nullptr);
		thread_get_current_thread()->fault_handler = HandleFault;
		std::atomic_signal_fence(std::memory_order_acq_rel);
	}


	~FaultHandlerGuard()
	{
		std::atomic_signal_fence(std::memory_order_acq_rel);
		thread_get_current_thread()->fault_handler = nullptr;
	}


	[[noreturn]] static void HandleFault()
	{
		longjmp(thread_get_current_thread()->fault_handler_state, 1);
	}
};


template<typename Function>
bool user_access(Function function)
{
	FaultHandlerGuard guard;
	// TODO: try { } catch (...) { } would be much nicer, wouldn't it?
	// And faster... And world wouldn't end in a terrible disaster if function()
	// or anything it calls created on stack an object with non-trivial
	// destructor.
	auto fail = setjmp(thread_get_current_thread()->fault_handler_state);
	if (fail == 0) {
		function();
		return true;
	}
	return false;
}


inline status_t
arch_cpu_user_memcpy(void* src, const void* dst, size_t n)
{
	return user_access([=] { memcpy(src, dst, n); }) ? B_OK : B_ERROR;
}


inline status_t
arch_cpu_user_memset(void* src, char v, size_t n)
{
	return user_access([=] { memset(src, v, n); }) ? B_OK : B_ERROR;
}


inline ssize_t
arch_cpu_user_strlcpy(char* src, const char* dst, size_t n)
{
	ssize_t result;
	return user_access([=, &result] { result = strlcpy(src, dst, n); })
		? result : B_ERROR;
}

}

#endif	// _KERNEL_ARCH_GENERIC_USER_MEMORY_H

