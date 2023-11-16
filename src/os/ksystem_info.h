/*
 * Copyright 2004-2008, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_SYSTEM_INFO_H
#define _KERNEL_SYSTEM_INFO_H


#include <kits/kernel/OS.h>


struct kernel_args;


#ifdef __cplusplus
extern "C" {
#endif


status_t system_info_init(struct kernel_args *args);
status_t system_notifications_init();
const char* get_haiku_revision(void);

status_t _user_get_system_info(system_info *userInfo);
status_t _user_get_cpu_info(uint32_t firstCPU, uint32_t cpuCount, cpu_info* info);
status_t _user_get_cpu_topology_info(cpu_topology_node_info* topologyInfos,
				uint32_t* topologyInfoCount);

status_t _user_get_system_info_etc(int32_t id, void *buffer,
			size_t bufferSize);

status_t _user_start_watching_system(int32_t object, uint32_t flags, port_id port,
			int32_t token);
status_t _user_stop_watching_system(int32_t object, uint32_t flags, port_id port,
			int32_t token);


#ifdef __cplusplus
}
#endif


#endif	/* _KERNEL_SYSTEM_INFO_H */
