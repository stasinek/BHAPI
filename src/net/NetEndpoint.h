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
 * File: NetEndpoint.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_NET_ENDPOINT_H__
#define __ETK_NET_ENDPOINT_H__

#include "./../net/NetAddress.h"
#include "./../net/NetBuffer.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ENetEndpoint : public EArchivable {
public:
	ENetEndpoint(int proto = SOCK_STREAM);
	ENetEndpoint(const ENetEndpoint &from);
	virtual ~ENetEndpoint();

	// Archiving
	ENetEndpoint(const EMessage *from);
	virtual e_status_t Archive(EMessage *into, bool deep = true) const;
	static EArchivable *Instantiate(const EMessage *from);

	e_status_t		InitCheck() const;

	ENetEndpoint		&operator=(const ENetEndpoint &endpoint);

	e_status_t		SetProtocol(int proto);

	int			SetSocketOption(eint32 level, eint32 option, const void *data, size_t data_len);
	int			GetSocketOption(eint32 level, eint32 option, void *data, size_t *data_len) const;

	int			SetNonBlocking(bool state = true);
	bool			IsNonBlocking() const;

	int			SetReuseAddr(bool state = true);

	const ENetAddress	&LocalAddr() const;
	const ENetAddress	&RemoteAddr() const;

	virtual void		Close();

	virtual e_status_t	Bind(const ENetAddress &addr);
	virtual e_status_t	Bind(euint16 port = 0);

	virtual e_status_t	Connect(const ENetAddress &addr);
	virtual e_status_t	Connect(const char *address, euint16 port);

	virtual e_status_t	Listen(int backlog = 5);
	virtual ENetEndpoint	*Accept(eint32 timeout_msec = -1);

	int			Error() const;
	const char		*ErrorStr() const;

	virtual eint32		Send(const void *buf, size_t len, int flags = 0);
	virtual eint32		Send(const ENetBuffer &buf, int flags = 0);
	virtual eint32		SendTo(const void *buf, size_t len, const ENetAddress &to, int flags = 0);
	virtual eint32		SendTo(const ENetBuffer &buf, const ENetAddress &to, int flags = 0);

	void			SetTimeout(e_bigtime_t timeout);
	virtual eint32		Receive(void *buf, size_t len, int flags = 0);
	virtual eint32		Receive(ENetBuffer &buf, size_t len, int flags = 0);
	virtual eint32		ReceiveFrom(void *buf, size_t len, const ENetAddress &from, int flags = 0);
	virtual eint32		ReceiveFrom(ENetBuffer &buf, size_t len, const ENetAddress &from, int flags = 0);

	virtual bool		IsDataPending(e_bigtime_t timeout = 0);

protected:
	// Socket(): use it carefully please.
	int			Socket() const;

private:
	int fSocket;
	int fProtocol;
	bool fBind;
	bool fNonBlocking;
	e_bigtime_t fTimeout;

	ENetAddress fLocalAddr;
	ENetAddress fRemoteAddr;
	void _Close();
};


inline int
ENetEndpoint::SetReuseAddr(bool state)
{
	int opt = (int)state;
	return SetSocketOption(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}


#endif /* __cplusplus */

#endif /* __ETK_NET_ENDPOINT_H__ */

