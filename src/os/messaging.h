/* 
 * Copyright 2005, Ingo Weinhold, bonefish@users.sf.net. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

// kernel-internal interface for the messaging service

#ifndef MESSAGING_H
#define MESSAGING_H

#include <kits/kernel/OS.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct messaging_target {
	port_id	port;
	int32_t	token;
} messaging_target;

status_t init_messaging_service();

status_t send_message(const void *message, int32_t messageSize,
	const messaging_target *targets, int32_t targetCount);

// syscalls
area_id _user_register_messaging_service(sem_id lockingSem, sem_id counterSem);
status_t _user_unregister_messaging_service();

#ifdef __cplusplus
}
#endif


// C++ only

#ifdef __cplusplus

namespace BPrivate {
	class KMessage;
}

status_t send_message(const BPrivate::KMessage *message,
	const messaging_target *targets,
	int32_t targetCount);

#endif	// __cplusplus

#endif	// MESSAGING_H
