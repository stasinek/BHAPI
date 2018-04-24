/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Looper.h
 * Description: Looper for waiting/dispatching message
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_LOOPER_H
#define BHAPI_LOOPER_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#ifndef BAPPLICATION_DEF
#define BAPPLICATION_DEF
class BApplication;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSENGER_DEF
#define BMESSENGER_DEF
class BMessenger;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEFILTER_DEF
#define BMESSAGEFILTER_DEF
class BMessageFilter;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEQUEUE_DEF
#define BMESSAGEQUEUE_DEF
class BMessageQueue;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEQUEUE_DEF
#define BMESSAGEQUEUE_DEF
class BLoooper;
#endif
//-----------------------------------------------------------------------------
#endif // __cplusplus
//-----------------------------------------------------------------------------
#include <kits/support/List.h>
#include <kits/app/Handler.h>
#include <kits/kernel/OS.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus
class BMessage;
class BMessageQueue;
namespace BPrivate {
    class BDirectMessageTarget;
    class BLooperList;
}

// Port (Message Queue) Capacity
#define B_LOOPER_PORT_DEFAULT_CAPACITY	200
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BLooper : public BHandler {
public:
                            BLooper(const char* name = NULL,
                                int32 priority = B_NORMAL_PRIORITY,
                                int32 portCapacity
                                    = B_LOOPER_PORT_DEFAULT_CAPACITY);
    virtual					~BLooper();

    // Archiving
                            BLooper(const BMessage* data);
    static	BArchivable*	Instantiate(const BMessage* data);
    virtual	status_t		Archive(const BMessage* data, bool deep = true) const;

    // Message transmission
            status_t		PostMessage(uint32 command);
            status_t		PostMessage(BMessage* message);
            status_t		PostMessage(uint32 command, BHandler* handler,
                                BHandler* replyTo = NULL);
            status_t		PostMessage(BMessage* message, BHandler* handler,
                                BHandler* replyTo = NULL);

    virtual	void			DispatchMessage(BMessage* message,
                                BHandler* handler);
    virtual	void			MessageReceived(BMessage* message);
            BMessage*		CurrentMessage() const;
            BMessage*		DetachCurrentMessage();
            void			DispatchExternalMessage(BMessage* message,
                                BHandler* handler, bool& _detached);
            BMessageQueue*	MessageQueue() const;
            bool			IsMessageWaiting() const;

    // Message handlers
            void			AddHandler(BHandler* handler);
            bool			RemoveHandler(BHandler* handler);
            int32			CountHandlers() const;
            BHandler*		HandlerAt(int32 index) const;
            int32			IndexOf(BHandler* handler) const;

            BHandler*		PreferredHandler() const;
            void			SetPreferredHandler(BHandler* handler);

    // Loop control
    virtual	void*   		Run();
    virtual	void			Quit();
    virtual	bool			QuitRequested();
            bool			Lock();
            void			Unlock();
            bool			IsLocked() const;
            status_t		LockWithTimeout(bigtime_t timeout);
            thread_id		Thread() const;
            team_id			Team() const;
    static	BLooper*		LooperForThread(thread_id thread);

    // Loop debugging
            thread_id		LockingThread() const;
            int32			CountLocks() const;
            int32			CountLockRequests() const;
            sem_id			Sem() const;

    // Scripting
    virtual BHandler*		ResolveSpecifier(BMessage* message, int32 index,
                                BMessage* specifier, int32 what,
                                const char* property);
    virtual status_t		GetSupportedSuites(BMessage* data);

    // Message filters (also see BHandler).
    virtual	void			AddCommonFilter(BMessageFilter* filter);
    virtual	bool			RemoveCommonFilter(BMessageFilter* filter);
    virtual	void			SetCommonFilterList(BList* filters);
            BList*			CommonFilterList() const;

    // Private or reserved
    virtual status_t		Perform(perform_code d, void* arg);

protected:
        // called from overridden task_looper
            BMessage*		MessageFromPort(bigtime_t = B_INFINITE_TIMEOUT);

private:
    typedef BHandler _inherited;
    friend class BWindow;
    friend class BApplication;
    friend class BMessenger;
    friend class BView;
    friend class BHandler;
    friend class ::BPrivate::BLooperList;
    friend port_id _get_looper_port_(const BLooper* );

    virtual	void			_ReservedLooper1();
    virtual	void			_ReservedLooper2();
    virtual	void			_ReservedLooper3();
    virtual	void			_ReservedLooper4();
    virtual	void			_ReservedLooper5();
    virtual	void			_ReservedLooper6();

                            BLooper(const BLooper&);
            BLooper&		operator=(const BLooper&);

                            BLooper(int32 priority, port_id port,
                                const char* name);

            status_t		_PostMessage(BMessage* msg, BHandler* handler,
                                BHandler* reply_to);

    static	status_t		_Lock(BLooper* loop, port_id port,
                                bigtime_t timeout);
    static	status_t		_LockComplete(BLooper* loop, int32 old,
                                thread_id this_tid, sem_id sem,
                                bigtime_t timeout);
            void			_InitData(const char* name, int32 priority,
                                port_id port, int32 capacity);
            void			AddMessage(BMessage* msg);
            void			_AddMessagePriv(BMessage* msg);
    static	status_t		_task0_(void* arg);

            void*			ReadRawFromPort(int32* code,
                                bigtime_t timeout = B_INFINITE_TIMEOUT);
            BMessage*		ReadMessageFromPort(
                                bigtime_t timeout = B_INFINITE_TIMEOUT);
    virtual	BMessage*		ConvertToMessage(void* raw, int32 code);
    virtual	void			task_looper();
            void			_QuitRequested(BMessage* msg);
            bool			AssertLocked() const;
            BHandler*		_TopLevelFilter(BMessage* msg, BHandler* target);
            BHandler*		_HandlerFilter(BMessage* msg, BHandler* target);
            BHandler*		_ApplyFilters(BList* list, BMessage* msg,
                                BHandler* target);
            void			check_lock();
            BHandler*		resolve_specifier(BHandler* target, BMessage* msg);
            void			UnlockFully();

            ::BPrivate::BDirectMessageTarget* fDirectTarget;
            BMessage*		fLastMessage;
            port_id			fMsgPort;
            int32			fAtomicCount;
            sem_id			fLockSem;
            int32			fOwnerCount;
            thread_id		fOwner;
            void*		fThread;
            addr_t			fCachedStack;
            int32			fInitPriority;
            BHandler*		fPreferred;
            BList			fHandlers;
            BList*			fCommonFilters;
            bool			fTerminating;
            bool			fRunCalled;
            bool			fOwnsPort;
            uint32			_reserved[11];
public:
    void Init(const char *name,  int32 priority);
    // Archiving
    bool		IsRunning() const;
    BLooper*	Proxy() const;
    bool		ProxyBy(BLooper *proxy);
    bool		IsLockedByCurrentThread() const;

    status_t	PostMessage(const BMessage *message);
    status_t	PostMessage(const BMessage *message,
                    BHandler *handler,
                    BHandler *reply_to = NULL);

    virtual bool	SetCommonFilterList(const BList *filterList);

protected:
    // NextLooperMessage & DispatchLooperMessage: called from task of looper, like below
    //	while(true)
    //	{
    //		...
    //		BMessage *aMsg = NextLooperMessage(B_INFINITE_TIMEOUT);
    //
    //		if(aMsg == NULL) /* after "this->QuitRequested()" return "true" or proxy deconstructing. */
    //		{
    //			...
    //			break;
    //		}
    //		else
    //		{
    //			/* leaked memory unless "DispatchLooperMessage" applied or "delete" instead */
    //			DispatchLooperMessage(aMsg);
    //		}
    //		...
    //	}
//	BMessage	*NextLooperMessage(bigtime_t timeout = B_INFINITE_TIMEOUT);
    BMessage	*NextLooperMessage(bigtime_t timeout);
    void		DispatchLooperMessage(BMessage *msg);

private:
    friend status_t bhapi::lock_looper_of_handler(uint64 token, bigtime_t timeout);

    bool fDeconstructing;
    BLooper *fProxy;
    BList fClients;

     int32 fThreadPriority;

     int32 fHandlersCount;
    BHandler *fPreferredHandler;

    void *fLocker;
     int64 fLocksCount;

    sem_id fSem;

    BMessageQueue *fMessageQueue;
    BMessage *fCurrentMessage;

    static status_t _task(void*);
    static status_t _taskLooper(BLooper*, void*);
    static void _taskError(void*);

    static BList sLooperList;

    BHandler *_MessageTarget(const BMessage *msg, bool *preferred);
    status_t _PostMessage(const BMessage *msg,  uint64 handlerToken,  uint64 replyToken, bigtime_t timeout);

    BLooper *_Proxy() const;
    bool _ProxyBy(BLooper *proxy);
    BLooper *_GetNextClient(BLooper *client) const;

    bool *fThreadExited;
    void _FilterAndDispatchMessage(BMessage *msg, BHandler *target);

    virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#define BLOOPER_DEF
#endif /* BHAPI_LOOPER_H */
//-----------------------------------------------------------------------------
