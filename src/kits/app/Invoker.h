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
 * File: Invoker.h
 * Description: Invoke message to any target
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_INVOKER_H
#define BHAPI_INVOKER_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
//-----------------------------------------------------------------------------
#ifndef BLOOPER_DEF
#define BLOOPER_DEF
class BLooper;
#endif
//-----------------------------------------------------------------------------
#ifndef BHANDLER_DEF
#define BHANDLER_DEF
class BHandler;
#endif
//-----------------------------------------------------------------------------
#include <kits/app/Messenger.h>
#include <kits/support/List.h>
#include <Haiku.h>
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
//-----------------------------------------------------------------------------
typedef struct invoker_notify_state {
     uint32 kind;
    bool called;
} invoker_notify_state;
#ifdef __cplusplus /* Just for C++ */
}
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BInvoker {
public:
    BInvoker();
    BInvoker(BMessage *message,
         const BHandler *handler,
         const BLooper *looper = NULL);
    BInvoker(BMessage *message, BMessenger target);

    virtual ~BInvoker();

    virtual status_t	SetMessage(BMessage *message);
    BMessage*		Message() const;
     uint32			Command() const;

    virtual status_t	SetTarget(const BHandler *handler, const BLooper *looper = NULL);
    virtual status_t	SetTarget(BMessenger messenger);

    bool			IsTargetLocal() const;
    BHandler*		Target(BLooper **looper = NULL) const;
    BMessenger		Messenger() const;

    virtual status_t	SetHandlerForReply(const BHandler *handler);
    BHandler*		HandlerForReply() const;

    virtual status_t	Invoke(const BMessage *msg = NULL);
    //    status_t		InvokeNotify(const BMessage *msg,  uint32 kind = B_CONTROL_INVOKED);
    status_t		InvokeNotify(const BMessage *msg);
    status_t		InvokeNotify(const BMessage *msg,  uint32 kind);

    status_t		SetTimeout(bigtime_t timeout);
    bigtime_t		Timeout() const;

protected:
    /* Return the change code for a notification.  This is either
       B_CONTROL_INVOKED for raw Invoke() calls, or the kind
       supplied to InvokeNotify().  In addition, 'notify' will be
       set to true if this was an InvokeNotify() call, else false. */
     uint32			InvokeKind(bool* notify = NULL);

    /* Start and end an InvokeNotify context around an Invoke() call.
       These are only needed for writing custom methods that
       emulate the standard InvokeNotify() call. */
//    void			BeginInvokeNotify(uint32 kind = B_CONTROL_INVOKED);
    void			BeginInvokeNotify(void);
    void			BeginInvokeNotify(uint32 kind);
    void			EndInvokeNotify();

private:
    BMessage *fMessage;
    BMessenger fMessenger;
     uint64 fReplyHandlerToken;

    bigtime_t fTimeout;

     uint32 fNotifyKind;
    bool fNotifyCalled;

    BList fNotifyStatesList;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#endif /* BHAPI_INVOKER_H */
//-----------------------------------------------------------------------------
