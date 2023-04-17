/* 
 * Copyright 2005, Ingo Weinhold, bonefish@users.sf.net. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

// kernel - userland interface definitions

#ifndef MESSAGING_SERVICE_DEFS_H
#define MESSAGING_SERVICE_DEFS_H

#include <kits/kernel/OS.h>

#include <messaging.h>

enum {
	MESSAGING_COMMAND_SEND_MESSAGE	= 0,
};

struct messaging_area_header {
	int32_t	lock_counter;
	int32_t	size;				// set to 0, when area is discarded
	area_id	kernel_area;
	area_id	next_kernel_area;
	int32_t	command_count;
	int32_t	first_command;
	int32_t	last_command;
};

struct messaging_command {
	int32_t	next_command;
	uint32_t	command;
	int32_t	size;			// == sizeof(messaging_command) + dataSize
	char	data[0];
};

struct messaging_command_send_message {
	int32_t				message_size;
	int32_t				target_count;
	messaging_target	targets[0];	// [target_count]
//	char				message[message_size];
};

#endif	// MESSAGING_SERVICE_DEFS_H
