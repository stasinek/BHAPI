/*
 * Copyright 2004-2006, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#include <fs_volume.h>
#include <syscalls.h>

#include <kits/support/String.h>


dev_t
fs_mount_volume(const char *where, const char *device,
	const char *fileSystem, uint32_t flags, const char *parameters)
{
	return _kern_mount(where, device, fileSystem, flags, (void *)parameters,
		parameters ? strlen(parameters) : 0);
}


status_t
fs_unmount_volume(const char *path, uint32_t flags)
{
	return _kern_unmount(path, flags);
}

