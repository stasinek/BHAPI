/*
 * Copyright 2005, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler
 */
#ifndef BHAPI_LINK_MESSAGE_H_
#define BHAPI_LINK_MESSAGE_H_

#include <Haiku.h>
#ifdef __cplusplus // C++
namespace bhapi {
#endif // __cplusplus

static const int32 kLinkCode = '_PTL';
static const size_t kInitialBufferSize = 2048;
static const size_t kMaxBufferSize = 65536;
    // anything beyond that should be sent with a different mechanism

struct message_header {
    int32	size;
    uint32	code;
    uint32	flags;
};

static const uint32 kNeedsReply = 0x01;
#ifdef __cplusplus // C++
}
#endif // __cplusplus

#endif	/* _LINK_MESSAGE_H_ */
