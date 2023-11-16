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
#include <kits/app/MessageRunner.h>
#include <kits/app/Looper.h>
#include <kits/app/Cursor.h>
#include <kits/app/Roster.h>
#include <kits/add-ons/font/FontEngine.h>
//-----------------------------------------------------------------------------
#include <kits/kernel/OS.h>
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
public:
                                BApplication(const char* signature);
                                BApplication(const char* signature,
                                    status_t* error);
    virtual						~BApplication();

    // Archiving
                                BApplication(BMessage* data);
    static	BArchivable*		Instantiate(BMessage* data);
    virtual	status_t			Archive(BMessage* data, bool deep = true) const;

            status_t			InitCheck() const;

    // App control and System Message handling
    virtual	void *			Run();
    virtual	void				Quit();
    virtual bool				QuitRequested();
    virtual	void				Pulse();
    virtual	void				ReadyToRun();
    virtual	void				MessageReceived(BMessage* message);
    virtual	void				ArgvReceived(int32_t argc, char** argv);
    virtual	void				AppActivated(bool active);
    virtual	void				RefsReceived(BMessage* message);
    virtual	void				AboutRequested();

    // Scripting
    virtual BHandler*			ResolveSpecifier(BMessage* message, int32_t index,
                                    BMessage* specifier, int32_t form,
                                    const char* property);

    // Cursor control, window/looper list, and app info
            void				ShowCursor();
            void				HideCursor();
            void				ObscureCursor();
            bool				IsCursorHidden() const;
            void				SetCursor(const void* cursor);
            void				SetCursor(const BCursor* cursor,
                                    bool sync = true);

            int32_t				CountWindows() const;
            BWindow*			WindowAt(int32_t index) const;

            int32_t				CountLoopers() const;
            BLooper*			LooperAt(int32_t index) const;
            bool				IsLaunching() const;
            const char*			Signature() const;
            status_t			GetAppInfo(app_info* info) const;
    static	BResources*			AppResources();

    virtual	void				DispatchMessage(BMessage* message,
                                    BHandler* handler);
            void				SetPulseRate(bigtime_t rate);

    // More scripting
    virtual status_t			GetSupportedSuites(BMessage* data);


    // Private or reserved
    virtual status_t			Perform(perform_code d, void* arg);

    class Private;

private:
    typedef BLooper _inherited;

    friend class Private;
    friend class BServer;

                                BApplication(const char* signature,
                                    const char* looperName, port_id port,
                                    bool initGUI, status_t* error);
                                BApplication(uint32_t signature);
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

    virtual	bool				ScriptReceived(BMessage* msg, int32_t index,
                                    BMessage* specifier, int32_t form,
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

            uint32_t				InitialWorkspace();
            int32_t				_CountWindows(bool includeMenus) const;
            BWindow*			_WindowAt(uint32_t index,
                                    bool includeMenus) const;

    static	void				_InitAppResources();

private:
    static	BResources*			sAppResources;

            const char*			fAppName;
            ::BPrivate::PortLink*	fServerLink;
            ::BPrivate::ServerMemoryAllocator* fServerAllocator;

            void*				fCursorData;
            bigtime_t			fPulseRate;
            uint32_t				fInitialWorkspace;
            BMessageRunner*		fPulseRunner;
            status_t			fInitError;
            void*				fServerReadOnlyMemory;
            uint32_t				_reserved[12];

            bool				fReadyToRunCalled;
//-----------------------------------------------------------------------------
public:
            BApplication(const char *signature, bool try_Interface = true);

    // Archiving
    BApplication(void);
    BApplication(const BMessage *from);
    static BArchivable*     Instantiate(const BMessage *from);



    bigtime_t               PulseRate() const;

    // Register a BLooper to be quit before the BApplication
    // object is destroyed.
    status_t                RegisterLooper(BLooper* looper);
    status_t                UnregisterLooper(BLooper* looper);
    // More scripting
 //-----------------------------------------------------------------------------
private:
 //-----------------------------------------------------------------------------
    friend class BLooper;
    friend class BMessageRunner;
    friend class BWindow;
    friend class BView;
    friend class BBitmap;
    friend class BScreen;

    friend BHAPI_IMPEXP bool bhapi::update_font_families(bool);

    bool fQuit;
    char *fSignature;

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
