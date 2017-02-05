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
 * File: NetEndpoint.cpp
 *
 * --------------------------------------------------------------------------*/

// WARNING: MT-SAFE uncompleted yet !!!

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>

#ifndef _WIN32
#	include <sys/time.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <fcntl.h>

#	ifdef __BEOS__
#		include <sys/socket.h>
#		ifdef BONE_VERSION
#			include <arpa/inet.h>
#		endif
#		ifndef __HAIKU__
#			define socklen_t int
#		endif
#	else
#		include <arpa/inet.h>
#	endif
#else
#	include <winsock2.h>
#	define socklen_t int
#	define EWOULDBLOCK WSAEWOULDBLOCK
#	define ECONNABORTED WSAECONNABORTED
#	undef EINTR
#	define EINTR WSAEINTR
#endif

#include "NetEndpoint.h"


#ifdef SIGPIPE
class _LOCAL ENetEndpointSignalIgnore {
public:
	ENetEndpointSignalIgnore()
	{
		ETK_DEBUG("[NET]: Ignore SIGPIPE.");
		signal(SIGPIPE, SIG_IGN);
	}
};
static ENetEndpointSignalIgnore _ignore;
#endif


ENetEndpoint::ENetEndpoint(int proto)
	: EArchivable(), fProtocol(proto), fBind(false), fNonBlocking(false), fTimeout(0)
{
	fSocket = socket(AF_INET, fProtocol, 0);
}


ENetEndpoint::ENetEndpoint(const ENetEndpoint &from)
	: EArchivable(), fSocket(-1), fBind(false), fNonBlocking(false)
{
	ENetEndpoint::operator=(from);
}


ENetEndpoint::~ENetEndpoint()
{
	_Close();
}


ENetEndpoint::ENetEndpoint(const EMessage *from)
	: EArchivable(from), fSocket(-1), fBind(false), fNonBlocking(false)
{
	// TODO
}


e_status_t
ENetEndpoint::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EArchivable::Archive(into, deep);
	into->AddString("class", "ENetEndpoint");

	// TODO

	return E_OK;
}


EArchivable*
ENetEndpoint::Instantiate(const EMessage *from)
{
	if(e_validate_instantiation(from, "ENetEndpoint"))
		return new ENetEndpoint(from);
	return NULL;
}


e_status_t
ENetEndpoint::InitCheck() const
{
	return(fSocket == -1 ? E_ERROR : E_OK);
}


ENetEndpoint&
ENetEndpoint::operator=(const ENetEndpoint &endpoint)
{
	ENetEndpoint::Close();

	if(endpoint.fSocket != -1)
	{
		if(endpoint.fBind) ENetEndpoint::Bind(endpoint.fLocalAddr);
		ENetEndpoint::Connect(endpoint.fRemoteAddr);
		if(endpoint.fNonBlocking) SetNonBlocking(true);
	}

	return *this;
}


e_status_t
ENetEndpoint::SetProtocol(int proto)
{
	if(fProtocol != proto)
	{
		int s = socket(AF_INET, proto, 0);
		if(s == -1) return E_ERROR;

		_Close();

		fSocket = s;
		fProtocol = proto;
	}

	return E_OK;
}


int
ENetEndpoint::SetSocketOption(eint32 level, eint32 option, const void *data, size_t data_len)
{
	if(fSocket == -1) return -1;

	int retVal = setsockopt(fSocket, level, option, (const char*)data, data_len) < 0 ? -1 : 0;

#ifdef __BEOS__
	if(retVal == 0 && option == SO_NONBLOCK)
	{
		fNonBlocking = false;
		for(const euint8 *tmp = (const euint8*)data; data_len > 0; data_len--, tmp--)
		{
			if(*tmp != 0)
			{
				fNonBlocking = true;
				break;
			}
		}
	}
#endif

	return retVal;
}


int
ENetEndpoint::GetSocketOption(eint32 level, eint32 option, void *data, size_t *data_len) const
{
	if(fSocket == -1) return -1;

#if (defined(__BEOS__) && !defined(BONE_VERSION))
	if(level != SOL_SOCKET || option != SO_NONBLOCK ||
	   data == NULL || data_len == NULL || *data_len == 0) return -1;
	bzero(data, *data_len);
	if(fNonBlocking) *((euint8*)data) = 1;
	return 0;
#else
	socklen_t len = (data_len ? (socklen_t)*data_len : 0);
	if(getsockopt(fSocket, level, option, (char*)data, &len) < 0) return -1;
	if(data_len) *data_len = (size_t)len;
	return 0;
#endif
}


int
ENetEndpoint::SetNonBlocking(bool state)
{
	if(fSocket == -1) return -1;

	if(fNonBlocking == state) return 0;

#ifdef __BEOS__
	return SetSocketOption(SOL_SOCKET, SO_NONBLOCK, &state, sizeof(state));
#elif defined(_WIN32)
	u_long value = (u_long)state;
	if(ioctlsocket(fSocket, FIONBIO, &value) != 0) return -1;
	fNonBlocking = state;
	return 0;
#else
	int flags = fcntl(fSocket, F_GETFL, 0);
	if(state) flags |= O_NONBLOCK;
	else flags &= ~O_NONBLOCK;
	if(fcntl(fSocket, F_SETFL, flags) == -1) return -1;
	fNonBlocking = state;
	return 0;
#endif
}


bool
ENetEndpoint::IsNonBlocking() const
{
	return fNonBlocking;
}


const ENetAddress&
ENetEndpoint::LocalAddr() const
{
	return fLocalAddr;
}


const ENetAddress&
ENetEndpoint::RemoteAddr() const
{
	return fRemoteAddr;
}


void
ENetEndpoint::_Close()
{
	if(fSocket != -1)
	{
#if defined(_WIN32) || (defined(__BEOS__) && !(defined(BONE_VERSION) || defined(__HAIKU__)))
		closesocket(fSocket);
#else
		close(fSocket);
#endif
		fSocket = -1;
	}

	fLocalAddr = ENetAddress();
	fRemoteAddr = ENetAddress();

	fBind = false;
	fNonBlocking = false;
}


void
ENetEndpoint::Close()
{
	_Close();
	fSocket = socket(AF_INET, fProtocol, 0);
}


e_status_t
ENetEndpoint::Bind(const ENetAddress &addr)
{
	if(fSocket == -1) return E_ERROR;

	struct sockaddr_in sa;
	if(addr.GetAddr(sa) != E_OK) return E_ERROR;

	if(bind(fSocket, (struct sockaddr*)&sa, sizeof(sa)) != 0)
	{
		ETK_DEBUG("[NET]: %s --- bind() failed (errno:%d).", __PRETTY_FUNCTION__, errno);
		return E_ERROR;
	}

	socklen_t len = sizeof(sa);
	getsockname(fSocket, (struct sockaddr*)&sa, &len);
	fLocalAddr.SetTo(sa);
	fBind = true;

	return E_OK;
}


e_status_t
ENetEndpoint::Bind(euint16 port)
{
	ENetAddress addr(INADDR_LOOPBACK, port);
	return Bind(addr);
}


e_status_t
ENetEndpoint::Connect(const ENetAddress &addr)
{
	if(fSocket == -1) return E_ERROR;

	struct sockaddr_in sa;
	socklen_t len;

	if(addr.GetAddr(sa) != E_OK) return E_ERROR;

	if(connect(fSocket, (struct sockaddr*)&sa, sizeof(sa)) != 0)
	{
		ETK_DEBUG("[NET]: %s --- connect() failed (errno:%d).", __PRETTY_FUNCTION__, errno);
		return E_ERROR;
	}

	if(fBind == false)
	{
		len = sizeof(sa);
		if(getsockname(fSocket, (struct sockaddr*)&sa, &len) == 0) fLocalAddr.SetTo(sa);
	}

	len = sizeof(sa);
	if(getpeername(fSocket, (struct sockaddr*)&sa, &len) == 0) fRemoteAddr.SetTo(sa);

	return E_OK;
}


e_status_t
ENetEndpoint::Connect(const char *address, euint16 port)
{
	ENetAddress addr(address, port);
	return ENetEndpoint::Connect(addr);
}


e_status_t
ENetEndpoint::Listen(int backlog)
{
	if(fSocket == -1) return E_ERROR;
	return(listen(fSocket, backlog) == 0 ? E_OK : E_ERROR);
}


ENetEndpoint*
ENetEndpoint::Accept(eint32 timeout_msec)
{
	if(fSocket == -1) return NULL;

	struct sockaddr_in sa;
	socklen_t len = sizeof(sa);
	int s = -1;

	if(timeout_msec < 0)
	{
		s = accept(fSocket, (struct sockaddr*)&sa, &len);
	}
	else
	{
		bool saveState = fNonBlocking;
		e_bigtime_t saveTime = e_real_time_clock_usecs();
		SetNonBlocking(true);
		do
		{
			if((s = accept(fSocket, (struct sockaddr*)&sa, &len)) != -1) break;

			int err = Error();
			if(!(err == EWOULDBLOCK ||
			     err == ECONNABORTED ||
			     err == EINTR)) break;
			if(timeout_msec > 0) e_snooze(1000);
		} while(e_real_time_clock_usecs() - saveTime <= timeout_msec * E_INT64_CONSTANT(1000));
		SetNonBlocking(saveState);
	}

	if(s == -1) return NULL;

	ENetEndpoint *endpoint = new ENetEndpoint(fProtocol);
	endpoint->_Close();
	endpoint->fSocket = s;
	endpoint->fLocalAddr = fLocalAddr;
	endpoint->fRemoteAddr.SetTo(sa);

	return endpoint;
}


int
ENetEndpoint::Error() const
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}


const char*
ENetEndpoint::ErrorStr() const
{
	// FIXME: wrong on Win32
	return strerror(Error());
}


eint32
ENetEndpoint::Send(const void *buf, size_t len, int flags)
{
	if(fSocket == -1 || fLocalAddr.InitCheck() != E_OK) return -1;

	if(fProtocol == SOCK_DGRAM)
	{
		struct sockaddr_in sa;
		if(fRemoteAddr.GetAddr(sa) != E_OK) return -1;
		return sendto(fSocket, (const char*)buf, len, flags, (struct sockaddr*)&sa, sizeof(sa));
	}
	else
	{
		return send(fSocket, (const char*)buf, len, flags);
	}
}


eint32
ENetEndpoint::Send(const ENetBuffer &buf, int flags)
{
	return ENetEndpoint::Send(buf.Data(), buf.Size(), flags);
}


eint32
ENetEndpoint::SendTo(const void *buf, size_t len, const ENetAddress &to, int flags)
{
	if(fSocket == -1 || fLocalAddr.InitCheck() != E_OK) return -1;
	if(fProtocol != SOCK_DGRAM) return -1;

	struct sockaddr_in sa;
	if(fRemoteAddr.GetAddr(sa) != E_OK) return -1;
	return sendto(fSocket, (const char*)buf, len, flags, (struct sockaddr*)&sa, sizeof(sa));
}


eint32
ENetEndpoint::SendTo(const ENetBuffer &buf, const ENetAddress &to, int flags)
{
	return ENetEndpoint::SendTo(buf.Data(), buf.Size(), to, flags);
}


void
ENetEndpoint::SetTimeout(e_bigtime_t timeout)
{
	if(timeout < 0) timeout = 0;
	fTimeout = timeout;
}


eint32
ENetEndpoint::Receive(void *buf, size_t len, int flags)
{
	if(fSocket == -1 || fLocalAddr.InitCheck() != E_OK) return -1;
	if(!ENetEndpoint::IsDataPending(fTimeout)) return -1;
	return recv(fSocket, (char*)buf, len, flags);
}


eint32
ENetEndpoint::Receive(ENetBuffer &buf, size_t len, int flags)
{
	void *data = (len != 0 ? malloc(len) : NULL);
	if(data == NULL) return -1;

	eint32 bytes = ENetEndpoint::Receive(data, len, flags);
	if(bytes < 0)
	{
		free(data);
		return -1;
	}

	buf = ENetBuffer(bytes);
	buf.AppendData(data, (size_t)bytes);
	free(data);

	return bytes;
}


eint32
ENetEndpoint::ReceiveFrom(void *buf, size_t len, const ENetAddress &from, int flags)
{
	if(fSocket == -1 || fLocalAddr.InitCheck() != E_OK) return -1;
	if(fProtocol != SOCK_DGRAM) return -1;

	struct sockaddr_in sa;
	if(from.GetAddr(sa) != E_OK) return -1;

	if(!ENetEndpoint::IsDataPending(fTimeout)) return -1;

	socklen_t _len = sizeof(sa);
	return recvfrom(fSocket, (char*)buf, len, flags, (struct sockaddr*)&sa, &_len);
}


eint32
ENetEndpoint::ReceiveFrom(ENetBuffer &buf, size_t len, const ENetAddress &from, int flags)
{
	void *data = (len != 0 ? malloc(len) : NULL);
	if(data == NULL) return -1;

	eint32 bytes = ENetEndpoint::ReceiveFrom(data, len, from, flags);
	if(bytes < 0)
	{
		free(data);
		return -1;
	}

	buf = ENetBuffer(bytes);
	buf.AppendData(data, (size_t)bytes);
	free(data);

	return bytes;
}


bool
ENetEndpoint::IsDataPending(e_bigtime_t _timeout)
{
	if(fSocket == -1) return false;

	struct timeval timeout;
	if(fNonBlocking)
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
	}
	else if(_timeout != E_INFINITE_TIMEOUT)
	{
		timeout.tv_sec = (long)(_timeout / E_INT64_CONSTANT(1000000));
		timeout.tv_usec = (long)(_timeout % E_INT64_CONSTANT(1000000));
	}

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(fSocket, &rset);

	int status = select(fSocket + 1, &rset, NULL, NULL,
			    (fNonBlocking || _timeout != E_INFINITE_TIMEOUT) ? &timeout : NULL);
	return(status > 0 && FD_ISSET(fSocket, &rset));
}


int
ENetEndpoint::Socket() const
{
	return fSocket;
}

