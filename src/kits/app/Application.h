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
 * File: Application.h
 * Description: Application model to support Looper/Message
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_APPLICATION_H
#define BHAPI_APPLICATION_H
//-----------------------------------------------------------------------------
#include "../app/MessageRunner.h"
#include "../app/Looper.h"
#include "../app/Cursor.h"
#include "../app/Roster.h"
#include "../add-ons/font/FontEngine.h"
//-----------------------------------------------------------------------------
#include <kernel/OS.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
using namespace bhapi;
#ifndef BCLIPBOARD_DEF
#define BCLIPBOARD_DEF
class BClipboard;
#endif
#ifndef BGRAPHICSENGINE_DEF
#define BGRAPHICSENGINE_DEF
class BGraphicsEngine;
#endif

class BCursor;
class BList;
class BLocker;
class BMessageRunner;
class BResources;
class BServer;
class BWindow;
//struct app_info;

namespace BPrivate {
    class PortLink;
    class ServerMemoryAllocator;
}
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BApplication : public BLooper {
//-----------------------------------------------------------------------------
public:
//-----------------------------------------------------------------------------
    BApplication(const char* signature, status_t* error);
    BApplication(const char *signature, bool try_Interface = true);
    virtual ~BApplication();

    // Archiving
    BApplication(void);
    BApplication(const BMessage *from);
    virtual status_t        Archive(BMessage *into, bool deep = true) const;
    static BArchivable*     Instantiate(const BMessage *from);

    const char*             Signature() const;

    virtual void*           Run();
    virtual void            Quit();
    virtual bool            QuitRequested();

    // Empty functions BEGIN --- just for derivative class
    virtual void            ReadyToRun();
    virtual void            Pulse();
    // Empty functions END

    void                    SetPulseRate(bigtime_t rate);
    bigtime_t               PulseRate() const;

    virtual void            MessageReceived(BMessage *msg);
    virtual void            DispatchMessage(BMessage *msg, BHandler *target);

    void                    SetCursor(const void *cursor);
    void                    SetCursor(const BCursor *cursor, bool sync = true);
    void                    HideCursor();
    void                    ShowCursor();
    void                    ObscureCursor();
    bool                    IsCursorHidden() const;
    int32                   CountWindows() const;
    BWindow*                WindowAt(int32 index) const;
    int32                   CountLoopers() const;
    BLooper*                LooperAt(int32 index) const;
    bool                    IsLaunching() const;
    status_t                GetAppInfo(bhapi::app_info* info) const;
    static	BResources*		AppResources();
    // Register a BLooper to be quit before the BApplication
    // object is destroyed.
    status_t                RegisterLooper(BLooper* looper);
    status_t                UnregisterLooper(BLooper* looper);
    // More scripting
    virtual status_t		GetSupportedSuites(BMessage* data);
    // Private or reserved
    virtual status_t		Perform(perform_code d, void* arg);
    class Private;

 //-----------------------------------------------------------------------------
private:
 //-----------------------------------------------------------------------------
    friend class Private;
    friend class BLooper;
    friend class BMessageRunner;
    friend class BWindow;
    friend class BView;
    friend class BBitmap;
    friend class BScreen;
    friend class BServer;

    friend BHAPI_IMPEXP bool bhapi::update_font_families(bool);

    bool fQuit;
    char *fSignature;
    bigtime_t fPulseRate;
    BMessageRunner *fPulseRunner;

    static BList sRunnerList;
    static bigtime_t sRunnerMinimumInterval;
    static void dispatch_message_runners();

    bool quit_all_loopers(bool force);

    BGraphicsEngine *fGraphicsEngine;
    void *fGraphicsEngineAddon;
    void InitGraphicsEngine();

    void Init(const char *signature, bool tryInterface);

    BList fModalWindows;
    bool AddModalWindow(BMessenger &msgr);
    bool RemoveModalWindow(BMessenger &msgr);

    BCursor fCursor;
    bool fCursorHidden;
    bool fCursorObscure;

                                BApplication(const char* signature,
                                    const char* looperName, port_id port,
                                    bool initGUI, status_t* error);
                                BApplication(uint32 signature);
                                BApplication(const BApplication&);
            BApplication&		operator=(const BApplication&);

    virtual	void				_ReservedApplication1();
    virtual	void				_ReservedApplication2();
    virtual	void				_ReservedApplication3();
    virtual	void				_ReservedApplication4();
    virtual	void				_ReservedApplication5();
    virtual	void				_ReservedApplication6();
    virtual	void				_ReservedApplication7();
    virtual	void				_ReservedApplication8();

    virtual	bool				ScriptReceived(BMessage* msg, int32 index,
                                    BMessage* specifier, int32 form,
                                    const char* property);
            void				_InitData(const char* signature, bool initGUI,
                                    status_t* error);
            port_id				_GetPort(const char* signature);
            void				BeginRectTracking(BRect r, bool trackWhole);
            void				EndRectTracking();
            status_t			_SetupServerAllocator();
            status_t			_InitGUIContext();
            status_t			_ConnectToServer();
            void				_ReconnectToServer();
            bool				_QuitAllWindows(bool force);
            bool				_WindowQuitLoop(bool quitFilePanels,
                                    bool force);
            void				_ArgvReceived(BMessage* message);

            uint32				InitialWorkspace();
            int32				_CountWindows(bool includeMenus) const;
            BWindow*			_WindowAt(uint32 index,
                                    bool includeMenus) const;

    static	void				_InitAppResources();

private:
    static	BResources*			sAppResources;
            const char*			fAppName;
            ::BPrivate::PortLink*	fServerLink;
            ::BPrivate::ServerMemoryAllocator* fServerAllocator;
            void*				fCursorData;
            uint32				fInitialWorkspace;
            status_t			fInitError;
            void*				fServerReadOnlyMemory;
            uint32				_reserved[12];
            bool				fReadyToRunCalled;
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
#ifdef BHAPI_BUILD_LIBRARY
inline void BApplication::SetCursor(const void *cursor)
{
    BCursor theCursor(cursor);
    BApplication::SetCursor(&theCursor, true);
}
#endif // BHAPI_BUILD_LIBRARY
namespace bhapi {
extern BHAPI_IMPEXP BApplication __be_app;
extern BHAPI_IMPEXP BMessenger __be_app_messenger;
extern BHAPI_IMPEXP BClipboard __be_clipboard;
extern BHAPI_IMPEXP BApplication* be_app;
extern BHAPI_IMPEXP BMessenger* be_app_messenger;
extern BHAPI_IMPEXP BClipboard* be_clipboard;
}
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
/*#ifdef _WIN32
#ifdef COPYOF_SendMessage
#define SendMessage COPYOF_SendMessage
#endif
#ifdef COPYOF_DispatchMessage
#define DispatchMessage COPYOF_DispatchMessage
#endif
#endif
*/
#define BAPPLICATION_DEF
#endif /* BHAPI_APPLICATION_H */
