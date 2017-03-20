﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: NetAddress.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_NET_ADDRESS_H
#define BHAPI_NET_ADDRESS_H

#ifndef BHAPI_OS_WIN32
#include <posix/sys/types.h>
#include <posix/sys/socket.h>
#include <posix/netinet/in.h>
#else
#include <winsock2.h>
#endif
#include <Haiku.h>

#ifdef __cplusplus /* Just for C++ */
#include "../support/Archivable.h"
class BHAPI_IMPEXP BNetAddress : public BArchivable {
public:
    BNetAddress(const char *hostname = NULL,  __be_uint16 port = 0);
    BNetAddress(const char *hostname, const char *protocol, const char *service);
    BNetAddress(const struct sockaddr_in &sa);
    BNetAddress(const struct in_addr addr,  __be_uint16 port = 0);
    BNetAddress(__be_uint32 addr,  __be_uint16 port = 0);
    BNetAddress(const BNetAddress &from);
    virtual ~BNetAddress();

    // Archiving
    BNetAddress(const BMessage *from);
    virtual status_t Archive(BMessage *into, bool deep = true) const;
    static BArchivable *Instantiate(const BMessage *from);

    status_t	InitCheck() const;

    BNetAddress	&operator=(const BNetAddress &addr);

    status_t	SetTo(const char *hostname,  __be_uint16 port = 0);
    status_t	SetTo(const char *hostname, const char *protocol, const char *service);
    status_t	SetTo(const struct sockaddr_in &sa);
    status_t	SetTo(const struct in_addr addr,  __be_uint16 port = 0);
    status_t	SetTo(__be_uint32 addr = INADDR_ANY,  __be_uint16 port = 0);

    status_t	GetAddr(char *hostname, size_t hostname_len,  __be_uint16 *port = NULL) const;
    status_t	GetAddr(struct sockaddr_in &sa) const;
    status_t	GetAddr(struct in_addr &addr,  __be_uint16 *port = NULL) const;

private:
    struct sockaddr_in fAddr;
    status_t fStatus;
};

#endif /* __cplusplus */

#endif /* BHAPI_NET_ADDRESS_H */

