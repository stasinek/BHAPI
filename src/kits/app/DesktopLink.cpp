/*
 * Copyright 2006-2009, Haiku Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 */


#include <kits/netDesktopLink.h>

#include <kits/netAppMisc.h>
#include <kits/netServerProtocol.h>


namespace BPrivate {


DesktopLink::DesktopLink()
{
	create_desktop_connection(this, "desktop reply", 1);
}


DesktopLink::~DesktopLink()
{
	delete_port(fReceiver->Port());
}


status_t DesktopLink::InitCheck() const
{
	return fReceiver->Port() < B_OK ? fReceiver->Port() : B_OK;
}


}	// namespace BPrivate
