/*
 * Copyright 2015, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef _MEDIA_CLIENT_DEFS_H
#define _MEDIA_CLIENT_DEFS_H

#include <MediaDefs.h>
#include <MediaNode.h>


namespace BPrivate { namespace media {


typedef int32 media_client_id;
typedef int32 media_client_kind;
typedef int32 media_connection_kind;
typedef int32 media_connection_id;


enum media_client_kinds {
	// The client can receive media data.
	B_MEDIA_RECORDER		= 0x000000001,
	// The client can send media data to another client.
	B_MEDIA_PLAYER			= 0x000000002,
	// The client specify a control GUI which can be used to configure it.
	B_MEDIA_CONTROLLABLE	= 0x000000004
};

enum media_connection_kinds {
	B_MEDIA_INPUT = 0,
	B_MEDIA_OUTPUT = 1
};


typedef struct media_client {
	media_client_id				Id() const;
	media_client_kind			Kind() const;

	BMessage*					ToMessage();

private:
	media_client_kind			kind;

	media_node					node;
	uint32						padding[16];

	friend class BMediaClient;
	friend class BMediaConnection;
	friend class media_connection;
} media_client;


typedef struct media_connection {
	media_connection_id			Id() const;
	media_connection_kind		Kind() const;

	const media_client&			Client() const;

	const char*					Name() const;

	bool						IsInput() const;
	bool						IsOutput() const;

	BMessage*					ToMessage() const;

private:
	media_input					MediaInput() const;
	media_output				MediaOutput() const;

	const media_source&			Source() const;
	const media_destination&	Destination() const;

	media_node					RemoteNode() const;

private:
	media_connection_id			id;

	media_client				client;
	// TODO really needed?
	media_client				remote_client;

    media_source				source;
    media_destination			destination;
    media_format				format;
    char						name[B_MEDIA_NAME_LENGTH];

	media_connection_kind		kind;
	uint32						padding[16];

	friend class BMediaClient;
	friend class BMediaClientNode;
	friend class BMediaConnection;
	friend class BMediaInput;
	friend class BMediaOutput;
} media_connection;


}

}

using namespace BPrivate::media;

#endif
