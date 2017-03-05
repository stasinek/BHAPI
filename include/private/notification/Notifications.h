/*
 * Copyright 2010-2014, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NOTIFICATIONS_H
#define _NOTIFICATIONS_H

#include <Mime.h>

#define kNotificationServerSignature "application/x-vnd.Haiku-notification_server"

// Messages
const uint32 kNotificationMessage = 'nssm';

// Settings constants
extern const char* kSettingsFile;

// General settings
extern const char* kAutoStartName;
extern const char* kTimeoutName;

// General default settings
const bool kDefaultAutoStart = true;
const int32 kDefaultTimeout = 10;

// Display settings
extern const char* kWidthName;
extern const char* kIconSizeName;
extern const char* kLayoutName;

// Display default settings
const float kDefaultWidth = 300.0f;
const icon_size kDefaultIconSize = B_LARGE_ICON;

#endif	// _NOTIFICATIONS_H
