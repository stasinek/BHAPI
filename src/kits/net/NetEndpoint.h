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
 * File: NetEndpoint.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_NET_ENDPOINT_H
#define BHAPI_NET_ENDPOINT_H
#include <Haiku.h>
#include <kits/net/NetAddress.h>
#include <kits/net/NetBuffer.h>

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BNetEndpoint : public BArchivable {
public:
	BNetEndpoint(int proto = SOCK_STREAM);
	BNetEndpoint(const BNetEndpoint &from);
	virtual ~BNetEndpoint();

	// Archiving
	BNetEndpoint(const BMessage *from);
	virtual status_t Archive(BMessage *into, bool deep = true) const;
	static BArchivable *Instantiate(const BMessage *from);

	status_t		InitCheck() const;

	BNetEndpoint		&operator=(const BNetEndpoint &endpoint);

	status_t		SetProtocol(int proto);

    int			SetSocketOption(int32_t level,  int32_t option, const void *data, size_t data_len);
    int			GetSocketOption(int32_t level,  int32_t option, void *data, size_t *data_len) const;

	int			SetNonBlocking(bool state = true);
	bool			IsNonBlocking() const;

                SetReuseAddr(bool state = true);

	const BNetAddress	&LocalAddr() const;
	const BNetAddress	&RemoteAddr() const;

	virtual void		Close();

	virtual status_t	Bind(const BNetAddress &addr);
    virtual status_t	Bind(uint16 port = 0);

	virtual status_t	Connect(const BNetAddress &addr);
    virtual status_t	Connect(const char *address,  uint16 port);

	virtual status_t	Listen(int backlog = 5);
    virtual BNetEndpoint	*Accept(int32_t timeout_msec = -1);

     int32_t			Error() const;
	const char		*ErrorStr() const;

    virtual  int32_t		Send(const void *buf, size_t len, int flags = 0);
    virtual  int32_t		Send(const BNetBuffer &buf, int flags = 0);
    virtual  int32_t		SendTo(const void *buf, size_t len, const BNetAddress &to, int flags = 0);
    virtual  int32_t		SendTo(const BNetBuffer &buf, const BNetAddress &to, int flags = 0);

    void			SetTimeout(bigtime_t timeout);
    virtual  int32_t		Receive(void *buf, size_t len, int flags = 0);
    virtual  int32_t		Receive(BNetBuffer &buf, size_t len, int flags = 0);
    virtual  int32_t		ReceiveFrom(void *buf, size_t len, const BNetAddress &from, int flags = 0);
    virtual  int32_t		ReceiveFrom(BNetBuffer &buf, size_t len, const BNetAddress &from, int flags = 0);

    virtual bool		IsDataPending(bigtime_t timeout = 0);

protected:
	// Socket(): use it carefully please.
     int32_t			Socket() const;

private:
     int32_t fSocket;
     int32_t fProtocol;
	bool fBind;
	bool fNonBlocking;
	bigtime_t fTimeout;

	BNetAddress fLocalAddr;
	BNetAddress fRemoteAddr;
	void _Close();
};

#ifdef BHAPI_BUILD_LIBRARY

inline int BNetEndpoint::SetReuseAddr(bool state)
{
	int opt = (int)state;
	return SetSocketOption(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}
#endif // BHAPI_BUILD_LIBRARY

#endif /* __cplusplus */

#endif /* BHAPI_NET_ENDPOINT_H */

