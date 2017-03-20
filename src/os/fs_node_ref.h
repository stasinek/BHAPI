/*
 * Copyright 2005-2008, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef NODE_REF_H
#define NODE_REF_H

#include <../include/sys/stat.h>

namespace BPrivate {
struct node_ref {
    dev_t	device;
    ino_t	node;

    node_ref(dev_t device = 0, ino_t node = 0)
        : device(device),
          node(node)
    {
    }

    node_ref(const struct stat &st)
        : device(st.st_dev),
          node(st.st_ino)
    {
    }

    node_ref(const node_ref &other)
    {
        device = other.device;
        node = other.node;
    }

    node_ref &operator=(const node_ref &other)
    {
        device = other.device;
        node = other.node;
        return *this;
    }

    bool operator==(const node_ref &other) const
    {
        return (device == other.device && node == other.node);
    }

    bool operator!=(const node_ref &other) const
    {
        return !(*this == other);
    }

    bool operator<(const node_ref &other) const
    {
        return (device < other.device
            || (device == other.device && node < other.node));
    }

};
} // namespace BPrivate

#endif	// NODE_REF_H
