﻿/*
 * Copyright 2002-2007 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SYM_LINK_H
#define BHAPI_SYM_LINK_H

#include <Node.h>
#include <StorageDefs.h>

namespace bhapi {
struct entry_ref;
}

class BDirectory;
class BPath;
class BSymLink : public BNode {
public:
                                BSymLink();
                                BSymLink(const BSymLink& other);
                                BSymLink(const bhapi::entry_ref* ref);
                                BSymLink(const BEntry* entry);
                                BSymLink(const char* path);
                                BSymLink(const BDirectory* dir,
                                         const char* path);
    virtual						~BSymLink();

            ssize_t				ReadLink(char* buf, size_t size);

            ssize_t				MakeLinkedPath(const char* dirPath,
                                    BPath* path);
            ssize_t				MakeLinkedPath(const BDirectory* dir,
                                    BPath* path);

            bool				IsAbsolute();

private:
    virtual	void				_MissingSymLink1();
    virtual	void				_MissingSymLink2();
    virtual	void				_MissingSymLink3();
    virtual	void				_MissingSymLink4();
    virtual	void				_MissingSymLink5();
    virtual	void				_MissingSymLink6();

            uint32_t				_reservedData[4];
            BEntry*				fSecretEntry;

private:
            int					get_fd() const;
};
#define BSYMLINK_DEF
#endif	// BHAPI_SYM_LINK_H
