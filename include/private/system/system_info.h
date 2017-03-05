/*
 * Copyright 2008-2010, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SYSTEM_INFO_H
#define _SYSTEM_INFO_H


#include <OS.h>


enum {
	// team creation or deletion; object == -1; either one also triggers on
	// exec()
	B_WATCH_SYSTEM_TEAM_CREATION		= 0x01,
	B_WATCH_SYSTEM_TEAM_DELETION		= 0x02,

	// thread creation or deletion or property (name, priority) changes;
	// object == team ID or -1 for all teams
	B_WATCH_SYSTEM_THREAD_CREATION		= 0x04,
	B_WATCH_SYSTEM_THREAD_DELETION		= 0x08,
	B_WATCH_SYSTEM_THREAD_PROPERTIES	= 0x10,

	B_WATCH_SYSTEM_ALL
		= B_WATCH_SYSTEM_TEAM_CREATION
		| B_WATCH_SYSTEM_TEAM_DELETION
		| B_WATCH_SYSTEM_THREAD_CREATION
		| B_WATCH_SYSTEM_THREAD_DELETION
		| B_WATCH_SYSTEM_THREAD_PROPERTIES
};

enum {
	// message what for the notification messages
	B_SYSTEM_OBJECT_UPDATE				= 'SOUP',

	// "opcode" values
	B_TEAM_CREATED						= 0,
	B_TEAM_DELETED						= 1,
	B_TEAM_EXEC							= 2,
	B_THREAD_CREATED					= 3,
	B_THREAD_DELETED					= 4,
	B_THREAD_NAME_CHANGED				= 5
};


#ifdef __cplusplus
extern "C" {
#endif


status_t __get_system_info(system_info* info);
status_t __get_cpu_info(uint32 firstCPU, uint32 cpuCount, cpu_info* info);
status_t __get_cpu_topology_info(cpu_topology_node_info* topologyInfos,
			uint32* topologyInfoCount);

status_t __start_watching_system(int32 object, uint32 flags, port_id port,
			int32 token);
status_t __stop_watching_system(int32 object, uint32 flags, port_id port,
			int32 token);


#ifdef __cplusplus
}
#endif


#endif	/* _SYSTEM_INFO_H */
