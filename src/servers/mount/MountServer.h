/*
 * Copyright 2007-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MOUNT_SERVER_H
#define _MOUNT_SERVER_H

#include <Haiku.h>


const uint32_t kMountVolume 			= 'mntv';
const uint32_t kMountAllNow			= 'mntn';
const uint32_t kSetAutomounterParams 	= 'pmst';
const uint32_t kGetAutomounterParams 	= 'gpms';
const uint32_t kVolumeMounted			= 'vmtd';
const uint32_t kUnmountVolume			= 'umnt';

#define kMountServerSignature "application/x-vnd.Haiku-mount_server"


#endif // _MOUNT_SERVER_H
