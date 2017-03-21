/*
 * Copyright 2001-2006, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold <bonefish@cs.tu-berlin.de>
 */
#ifndef BHAPI_APPLICATION_PRIVATE_H
#define BHAPI_APPLICATION_PRIVATE_H

#include <Application.h>
struct server_read_only_memory;

class BApplication::Private {
    public:
        static inline BPrivate::PortLink *ServerLink()
            { return  __be_app->fServerLink; }

        static inline BPrivate::ServerMemoryAllocator* ServerAllocator()
            { return  __be_app->fServerAllocator; }

        static inline server_read_only_memory* ServerReadOnlyMemory()
            { return (server_read_only_memory*)be_app->fServerReadOnlyMemory; }
};

#endif	// _APPLICATION_PRIVATE_H
