/*
 * Copyright 2011, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_DATAGRAM_SOCKET_H
#define BHAPI_DATAGRAM_SOCKET_H

#ifdef __cplusplus // C++
#include <AbstractSocket.h>
#include <Haiku.h>
class BHAPI_IMPEXP BDatagramSocket : public BAbstractSocket {
public:
                                BDatagramSocket();
                                BDatagramSocket(const BNetworkAddress& peer,
                                    bigtime_t timeout = -1);
                                BDatagramSocket(const BDatagramSocket& other);
    virtual						~BDatagramSocket();

    virtual	status_t			Bind(const BNetworkAddress& peer);
    virtual	status_t			Connect(const BNetworkAddress& peer,
                                    bigtime_t timeout = B_INFINITE_TIMEOUT);

            status_t			SetBroadcast(bool broadcast);
            void				SetPeer(const BNetworkAddress& peer);

    virtual	size_t				MaxTransmissionSize() const;

    virtual	ssize_t				SendTo(const BNetworkAddress& address,
                                    const void* buffer, size_t size);
    virtual	ssize_t				ReceiveFrom(void* buffer, size_t bufferSize,
                                    BNetworkAddress& from);

    // BDataIO implementation

    virtual ssize_t				Read(void* buffer, size_t size);
    virtual ssize_t				Write(const void* buffer, size_t size);
};
#endif  // C++
#define BDATAGRAMSOCKET_I
#endif	// BHAPI_DATAGRAM_SOCKET_H
