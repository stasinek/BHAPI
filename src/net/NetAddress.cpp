/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#include "./../config.h>
#include "./../ETKBuild.h>
#include "./../support/ByteOrder.h>

#include "NetAddress.h"


ENetAddress::ENetAddress(const char *hostname, euint16 port)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, port);
}


ENetAddress::ENetAddress(const char *hostname, const char *protocol, const char *service)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(hostname, protocol, service);
}


ENetAddress::ENetAddress(const struct sockaddr_in &sa)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(sa);
}


ENetAddress::ENetAddress(const struct in_addr addr, euint16 port)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


ENetAddress::ENetAddress(euint32 addr, euint16 port)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	SetTo(addr, port);
}


ENetAddress::ENetAddress(const ENetAddress &from)
	: EArchivable(), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	ENetAddress::operator=(from);
}


ENetAddress::~ENetAddress()
{
}


ENetAddress::ENetAddress(const EMessage *from)
	: EArchivable(from), fStatus(E_NO_INIT)
{
	bzero(&fAddr, sizeof(struct sockaddr_in));
	// TODO
}


e_status_t
ENetAddress::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EArchivable::Archive(into, deep);
	into->AddString("class", "ENetAddress");

	// TODO

	return E_OK;
}


EArchivable*
ENetAddress::Instantiate(const EMessage *from)
{
	if(e_validate_instantiation(from, "ENetAddress"))
		return new ENetAddress(from);
	return NULL;
}


e_status_t
ENetAddress::InitCheck() const
{
	return fStatus;
}


ENetAddress&
ENetAddress::operator=(const ENetAddress &addr)
{
	fStatus = addr.fStatus;
	fAddr = addr.fAddr;
	return *this;
}


e_status_t
ENetAddress::SetTo(const char *hostname, euint16 port)
{
	if(hostname == NULL) return E_ERROR;

	struct hostent *ent = NULL;

#ifndef HAVE_GETHOSTBYNAME_R
	ent = gethostbyname(hostname);
#else

#ifdef ETK_OS_SOLARIS
	struct hostent _ent;
	char buf[8192];
	int err;
	ent = gethostbyname_r(hostname, &_ent, buf, sizeof(buf), &err);
#elif defined(ETK_OS_LINUX)
	struct hostent _ent;
	char buf[8192];
	int err;
	gethostbyname_r(hostname, &_ent, buf, sizeof(buf), &ent, &err);
#else
	#error "FIXME: gethostbyname_r"
#endif

#endif

	if(ent == NULL) return E_ERROR;

	e_status_t retVal = E_ERROR;

	switch(ent->h_addrtype)
	{
		case AF_INET:
			fAddr.sin_addr.s_addr = *((euint32*)ent->h_addr);
			fAddr.sin_family = AF_INET;
			fAddr.sin_port = htons(port);
			retVal = fStatus = E_OK;
			break;

		default:
			ETK_DEBUG("[NET]: %s --- unknown address type.", __PRETTY_FUNCTION__);
			break;
	}

	return retVal;
}


e_status_t
ENetAddress::SetTo(const char *hostname, const char *protocol, const char *service)
{
	if(hostname == NULL) return E_ERROR;

	struct servent *ent = NULL;

#ifndef HAVE_GETSERVBYNAME_R
	ent = getservbyname(service, protocol);
#else

#ifdef ETK_OS_SOLARIS
	struct servent _ent;
	char buf[8192];
	ent = getservbyname_r(service, protocol, &_ent, buf, sizeof(buf));
#elif defined(ETK_OS_LINUX)
	struct servent _ent;
	char buf[8192];
	getservbyname_r(service, protocol, &_ent, buf, sizeof(buf), &ent);
#else
	#error "FIXME: getservbyname_r"
#endif

#endif

	if(ent == NULL) return E_ERROR;

	return SetTo(hostname, ntohs(ent->s_port));
}


e_status_t
ENetAddress::SetTo(const struct sockaddr_in &sa)
{
	if(sa.sin_family != AF_INET)
	{
		// TODO
		return E_ERROR;
	}

	fAddr = sa;
	return(fStatus = E_OK);
}


e_status_t
ENetAddress::SetTo(const struct in_addr addr, euint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr = addr;
	return(fStatus = E_OK);
}


e_status_t
ENetAddress::SetTo(euint32 addr, euint16 port)
{
	fAddr.sin_family = AF_INET;
	fAddr.sin_port = htons(port);
	fAddr.sin_addr.s_addr = htonl(addr);
	return(fStatus = E_OK);
}


e_status_t
ENetAddress::GetAddr(char *hostname, size_t hostname_len, euint16 *port) const
{
	if(fStatus != E_OK) return E_ERROR;
	if(!(hostname == NULL || hostname_len == 0))
	{
		struct hostent *ent = NULL;

#ifndef HAVE_GETHOSTBYADDR_R
		ent = gethostbyaddr((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET);
#else

#ifdef ETK_OS_SOLARIS
		struct hostent _ent;
		char buf[8192];
		int err;
		ent = gethostbyaddr_r((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET,
				      &_ent, buf, sizeof(buf), &err);
#elif defined(ETK_OS_LINUX)
		struct hostent _ent;
		char buf[8192];
		int err;
		gethostbyaddr_r((const char*)&fAddr.sin_addr, sizeof(struct in_addr), AF_INET,
				&_ent, buf, sizeof(buf), &ent, &err);
#else
		#error "FIXME: gethostbyaddr_r"
#endif

#endif

		if(ent == NULL) return E_ERROR;

		if(hostname_len > 1)
		{
			hostname_len = min_c(hostname_len, strlen(ent->h_name) + 1);
			memcpy(hostname, ent->h_name, hostname_len - 1);
		}

		*(hostname + hostname_len - 1) = 0;
	}
	if(port) *port = ntohs(fAddr.sin_port);
	return E_OK;
}


e_status_t
ENetAddress::GetAddr(struct sockaddr_in &sa) const
{
	if(fStatus != E_OK) return E_ERROR;
	sa = fAddr;
	return E_OK;
}


e_status_t
ENetAddress::GetAddr(struct in_addr &addr, euint16 *port) const
{
	if(fStatus != E_OK) return E_ERROR;
	addr = fAddr.sin_addr;
	if(port) *port = ntohs(fAddr.sin_port);
	return E_OK;
}

