/*
 * Copyright 2008-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SYSTEM_USER_THREAD_DEFS_H
#define _SYSTEM_USER_THREAD_DEFS_H


#include <pthread.h>
#include <signal.h>

#include <Haiku.h>


struct user_thread {
	pthread_t		pthread;			// pthread pointer
	uint32_t			flags;
	status_t		wait_status;		// wait status for thread blocking
	int32_t			defer_signals;		// counter; 0 == signals allowed
	sigset_t		pending_signals;	// signals that are pending, when
										// signals are deferred
};


#endif	/* _SYSTEM_USER_THREAD_DEFS_H */
