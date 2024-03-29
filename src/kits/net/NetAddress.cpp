/* --------------------------------------------------------------------------
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
 * File: NetAddress.cpp
 *
 * --------------------------------------------------------------------------*/

#include <NetAddress.h>

#include <kits/app/Message.h>
#include <os/debug.h>
#include <kits/debug/Errors.h>
#include <kits/support/ByteOrder.h>

#ifndef _WIN32
#	include <netdb.h>
#	include <netinet/in.h>

#	ifdef __BEOS__
#		include <sys/socket.h>
#		ifdef BONE_VERSION
#			include <arpa/inet.h>
#		endif
#	else
#		include <arpa/inet.h>
#	endif
#else
#	include <winsock2.h>
#endif

BNetAddress::BNetAddress(const char *hostname,  __be_uint16 port)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, port);
}


BNetAddress::BNetAddress(const char *hostname, const char *protocol, const char *service)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, protocol, service);
}


BNetAddress::BNetAddress(const struct sockaddr_in &sa)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(sa);
}


BNetAddress::BNetAddress(const struct in_addr addr,  __be_uint16 port)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


BNetAddress::BNetAddress(__be_uint32_t addr,  __be_uint16 port)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


BNetAddress::BNetAddress(const BNetAddress &from)
	: BArchivable(), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	BNetAddress::operator=(from);
}


BNetAddress::~BNetAddress()
{
}


BNetAddress::BNetAddress(const BMessage *from)
	: BArchivable(from), fStatus(B_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	// TODO
}


status_t BNetAddress::Archive(BMessage *into, bool deep) const
{
	if(!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BNetAddress");

	// TODO

	return B_OK;
}


BArchivable*
BNetAddress::Instantiate(const BMessage *from)
{
	if(bhapi::validate_instantiation(from, "BNetAddress"))
		return new BNetAddress(from);
	return NULL;
}


status_t BNetAddress::InitCheck() const
{
	return fStatus;
}


BNetAddress&
BNetAddress::operator=(const BNetAddress &addr)
{
	fStatus = addr.fStatus;
	fAddr = addr.fAddr;
	return *this;
}


status_t BNetAddress::SetTo(const char *hostname,  __be_uint16 port)
{
	if(hostname == NULL) return B_ERROR;

	struct hostent *ent = NULL;

#ifndef HAVE_GETHOSTBYNAME_R
	ent = gethostbyname(hostname);
#else

#ifdef BHAPI_OS_SOLARIS
	struct hostent _ent;
	char buf[8192];
	int err;
	ent = gethostbyname_r(hostname, &_ent, buf, sizeof(buf), &err);
#elif defined(BHAPI_OS_LINUX)
	struct hostent _ent;
	char buf[8192];
	int err;
	gethostbyname_r(hostname, &_ent, buf, sizeof(buf), &ent, &err);
#else
	#error "FIXME: gethostbyname_r"
#endif

#endif

	if(ent == NULL) return B_ERROR;

	status_t retVal = B_ERROR;

	switch(ent->h_addrtype)
	{
		case AF_INET:
			fAddr.sin_addr.s_addr = *((__be_uint32_t*)ent->h_addr);
			fAddr.sin_family = AF_INET;
			fAddr.sin_port = htons(port);
			retVal = fStatus = B_OK;
			break;

		default:
			BHAPI_DEBUG("[NET]: %s --- unknown address type.", __PRETTY_FUNCTION__);
			break;
	}

	return retVal;
}


status_t BNetAddress::SetTo(const char *hostname, const char *protocol, const char *service)
{
	if(hostname == NULL) return B_ERROR;

	struct servent *ent = NULL;

#ifndef HAVE_GETSERVBYNAME_R
	ent = getservbyname(service, protocol);
#else

#ifdef BHAPI_OS_SOLARIS
	struct servent _ent;
	char buf[8192];
	ent = getservbyname_r(service, protocol, &_ent, buf, sizeof(buf));
#elif defined(BHAPI_OS_LINUX)
	struct servent _ent;
	char buf[8192];
	getservbyname_r(service, protocol, &_ent, buf, sizeof(buf), &ent);
#else
	#error "FIXME: getservbyname_r"
#endif

#endif

	if(ent == NULL) return B_ERROR;

	return SetTo(hostname, ntohs(ent->s_port));
}


status_t BNetAddress::SetTo(const struct sockaddr_in &sa)
{
	if(sa.sin_family != AF_INET)
	{
		// TODO
		return B_ERROR;
	}

	fAddr = sa;
	return(fStatus = B_OK);
}


status_t BNetAddress::SetTo(const struct in_addr addr,  __be_uint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr = addr;
	return(fStatus = B_OK);
}


status_t BNetAddress::SetTo(__be_uint32_t addr,  __be_uint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr.s_addr = htonl(addr);
	return(fStatus = B_OK);
}


status_t BNetAddress::GetAddr(char *hostname, size_t hostname_len,  __be_uint16 *port) const
{
	if(fStatus != B_OK) return B_ERROR;
	if(!(hostname == NULL || hostname_len == 0))
	{
		struct hostent *ent = NULL;

#ifndef HAVE_GETHOSTBYADDR_R
		ent = gethostbyaddr((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET);
#else

#ifdef BHAPI_OS_SOLARIS
		struct hostent _ent;
		char buf[8192];
		int err;
		ent = gethostbyaddr_r((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET,
				      &_ent, buf, sizeof(buf), &err);
#elif defined(BHAPI_OS_LINUX)
		struct hostent _ent;
		char buf[8192];
		int err;
		gethostbyaddr_r((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET,
				&_ent, buf, sizeof(buf), &ent, &err);
#else
		#error "FIXME: gethostbyaddr_r"
#endif

#endif

		if(ent == NULL) return B_ERROR;

		if(hostname_len > 1)
		{
			hostname_len = min_c(hostname_len, strlen(ent->h_name) + 1);
			memcpy(hostname, ent->h_name, hostname_len - 1);
		}

		*(hostname + hostname_len - 1) = 0;
	}
	if(port) *port = ntohs(fAddr.sin_port);
	return B_OK;
}


status_t BNetAddress::GetAddr(struct sockaddr_in &sa) const
{
	if(fStatus != B_OK) return B_ERROR;
	sa = fAddr;
	return B_OK;
}


status_t BNetAddress::GetAddr(struct in_addr &addr,  __be_uint16 *port) const
{
	if(fStatus != B_OK) return B_ERROR;
	addr = fAddr.sin_addr;
	if(port) *port = ntohs(fAddr.sin_port);
	return B_OK;
}

