/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_NOTIFICATION_H
#define BHAPI_NOTIFICATION_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus
//-----------------------------------------------------------------------------
// notification types
enum notification_type {
    B_INFORMATION_NOTIFICATION,
    B_IMPORTANT_NOTIFICATION,
    B_ERROR_NOTIFICATION,
    B_PROGRESS_NOTIFICATION
};
//-----------------------------------------------------------------------------
#ifndef BBITMAP_I
#define BBITMAP_I
class BBitmap;
#endif
//-----------------------------------------------------------------------------
#include <kits/netArchivable.h>
#include <kits/storage/Entry.h>
#include <kits/netList.h>
#include <kits/netStringClass.h>
#include <Haiku.h>
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BNotification : public BArchivable {
public:
                                BNotification(notification_type type);
                                BNotification(BMessage* archive);
    virtual						~BNotification();

            status_t			InitCheck() const;

    static	BArchivable*		Instantiate(BMessage* archive);
    virtual	status_t			Archive(BMessage* archive, bool deep = true) const;

            notification_type	Type() const;

            const char*			Group() const;
            void				SetGroup(const BString& group);

            const char*			Title() const;
            void				SetTitle(const BString& title);

            const char*			Content() const;
            void				SetContent(const BString& content);

            const char*			MessageID() const;
            void				SetMessageID(const BString& id);

            float				Progress() const;
            void				SetProgress(float progress);

            const char*			OnClickApp() const;
            void				SetOnClickApp(const BString& app);

            const entry_ref*	OnClickFile() const;
            status_t			SetOnClickFile(const entry_ref* file);

            status_t			AddOnClickRef(const entry_ref* ref);
            int32_t				CountOnClickRefs() const;
            const entry_ref*	OnClickRefAt(int32_t index) const;

            status_t			AddOnClickArg(const BString& arg);
            int32_t				CountOnClickArgs() const;
            const char*			OnClickArgAt(int32_t index) const;

            const BBitmap*		Icon() const;
            status_t			SetIcon(const BBitmap* icon);

            status_t			Send(bigtime_t timeout = -1);

private:
            status_t			fInitStatus;

            notification_type	fType;
            BString				fGroup;
            BString				fTitle;
            BString				fContent;
            BString				fID;
            float				fProgress;

            BString				fApp;
            entry_ref*			fFile;
            BList				fRefs;
            BList				fArgv;
            BBitmap*			fBitmap;
};
//-----------------------------------------------------------------------------
#endif // __cplusplus
//-----------------------------------------------------------------------------
#define BNOTIFICATION_I
#endif	// BHAPI_NOTIFICATION_H
//-----------------------------------------------------------------------------
