﻿/*
 * Copyright 2010 Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_B_URL_PROTOCOL_ASYNCHRONOUS_LISTENER_H
#define BHAPI_B_URL_PROTOCOL_ASYNCHRONOUS_LISTENER_H


#include <Handler.h>
#include <Message.h>
#include <UrlProtocolDispatchingListener.h>


class BUrlProtocolAsynchronousListener : public BHandler,
    public BUrlProtocolListener {
public:
                                BUrlProtocolAsynchronousListener(
                                    bool transparent = false);
    virtual						~BUrlProtocolAsynchronousListener();

    // Synchronous listener access
            BUrlProtocolListener* SynchronousListener();

    // BHandler interface
    virtual void				MessageReceived(BMessage* message);

private:
            BUrlProtocolDispatchingListener*
                                fSynchronousListener;
};

#endif // _B_URL_PROTOCOL_ASYNCHRONOUS_LISTENER_H_

