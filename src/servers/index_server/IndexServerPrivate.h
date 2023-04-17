/*
 * Copyright 2010, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Clemens Zeidler <haiku@clemens-zeidler.de>
 */
#ifndef INDEX_SERVER_PRIVATE_H
#define INDEX_SERVER_PRIVATE_H


#include <kits/support/String.h>


const BString kIndexServerDirectory = ".IndexServer";
const BString kVolumeStatusFileName = "VolumeStatus";

// messages between preferences app
const uint32_t kStopWatching = 'StoW';
const uint32_t kStartWatching = 'StaW';

const uint32_t kRegisterWatcher = 'RegW';
const uint32_t kVolumenAdded = 'VAdd';
const uint32_t kVolumenRemoved = 'VRem';
const uint32_t kAddOnAdded = 'AAdd';
const uint32_t kAddOnRemoved = 'ARem';

const uint32_t kGetVolumenInfos = 'GVIn';
const uint32_t kGetAddOnInfos = 'GAIn';

const uint32_t kEnableAddOn = 'EnaA';
const uint32_t kDisableAddOn = 'DisA';

#endif // INDEX_SERVER_PRIVATE_H
