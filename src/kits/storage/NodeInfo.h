/*
 * Copyright 2002-2010 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_NODE_INFO_H
#define BHAPI_NODE_INFO_H
//-----------------------------------------------------------------------------
#include <Entry.h>
#include <File.h>
#include <Message.h>
#include <Mime.h>
#include <Haiku.h>
//-----------------------------------------------------------------------------
#ifndef BBITMAP_DEF
class BBitmap;
#define BBITMAP_DEF
#endif
//-----------------------------------------------------------------------------
#ifndef BRESOURCES_DEF
class BResources;
#define BRESOURCES_DEF
#endif
//-----------------------------------------------------------------------------
#ifndef BNODE_DEF
class BNode;
#define BNODE_DEF
#endif
//-----------------------------------------------------------------------------
namespace bhapi {
struct entry_ref;
}
//-----------------------------------------------------------------------------
class BNodeInfo {
public:
                                BNodeInfo();
                                BNodeInfo(BNode* node);
    virtual						~BNodeInfo();

            status_t			SetTo(BNode* node);

            status_t			InitCheck() const;

    virtual status_t			GetType(char* type) const;
    virtual status_t			SetType(const char* type);
    virtual status_t			GetIcon(BBitmap* icon,
                                    icon_size which = B_LARGE_ICON) const;
    virtual status_t			SetIcon(const BBitmap* icon,
                                    icon_size which = B_LARGE_ICON);
            status_t			GetIcon(uint8** data, size_t* size,
                                    type_code* type) const;
            status_t			SetIcon(const uint8* data, size_t size);

            status_t			GetPreferredApp(char* signature,
                                    app_verb verb = B_OPEN) const;
            status_t			SetPreferredApp(const char* signature,
                                    app_verb verb = B_OPEN);
            status_t			GetAppHint(bhapi::entry_ref* ref) const;
            status_t			SetAppHint(const bhapi::entry_ref* ref);

            status_t			GetTrackerIcon(BBitmap* icon,
                                    icon_size which = B_LARGE_ICON) const;
    static	status_t			GetTrackerIcon(const bhapi::entry_ref* ref,
                                    BBitmap* icon,
                                    icon_size which = B_LARGE_ICON);
private:
            friend class BAppFileInfo;

    virtual void				_ReservedNodeInfo1();
    virtual void				_ReservedNodeInfo2();
    virtual void				_ReservedNodeInfo3();

                                BNodeInfo &operator=(const BNodeInfo& other);
                                BNodeInfo(const BNodeInfo& other);
                                    // not implemented

private:
            BNode*				fNode;
            uint32_t				_reserved[2];
            status_t			fCStatus;
};
//-----------------------------------------------------------------------------
#define BNODEINFO_DEF
#endif // BHAPI_NODE_INFO_H
//-----------------------------------------------------------------------------
