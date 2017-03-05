/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: BufferIO.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2004-2009, Axel Dörfler, axeld@pinc-software.de.
 * Copyright 2007, Jérôme Duval. All rights reserved.
 * Distributed under the terms of the Haiku License.
 */

#include "Beep.h"
#include <DataExchange.h>
#include "../media/MediaSounds.h"

status_t bhapi::system_beep(const char* eventName)
{
	BMessenger messenger("application/x-vnd.Be.addon-host");
	if (!messenger.IsValid())
		return B_ERROR;

	BMessage msg(MEDIA_ADD_ON_SERVER_PLAY_MEDIA), reply;
	msg.AddString(MEDIA_NAME_KEY, eventName ? eventName : MEDIA_SOUNDS_BEEP);
	msg.AddString(MEDIA_TYPE_KEY, MEDIA_TYPE_SOUNDS);

	status_t status = messenger.SendMessage(&msg, &reply);
	if (status != B_OK || reply.FindInt32("error", &status) != B_OK)
		status = B_BAD_REPLY;

	return status;
}


status_t bhapi::beep()
{
    return bhapi::system_beep(NULL);
}


status_t bhapi::add_system_beep_event(const char* name, uint32 flags)
{
	BMessenger messenger("application/x-vnd.Be.media-server");
	if (!messenger.IsValid())
		return B_ERROR;

	BMessage msg(MEDIA_SERVER_ADD_SYSTEM_BEEP_EVENT), reply;
	msg.AddString(MEDIA_NAME_KEY, name);
	msg.AddString(MEDIA_TYPE_KEY, MEDIA_TYPE_SOUNDS);
	msg.AddInt32(MEDIA_FLAGS_KEY, flags);

	status_t status = messenger.SendMessage(&msg, &reply);
	if (status != B_OK || reply.FindInt32("error", &status) != B_OK)
		status = B_BAD_REPLY;

	return status;
}
