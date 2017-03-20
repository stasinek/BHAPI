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
 * File: BHAPI_wrapper_application.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS




#include <be/app/AppDefs.h>
#include <be/app/Application.h>
#include <be/app/Clipboard.h>
#include <be/app/Cursor.h>
#include <be/interface/Screen.h>

#include "BHAPI_wrapper_beos-graphics.h"

#include "../../support/Autolock.h"
#include "../../support/StringClass.h"
#include "../../kernel/Kernel.h"
#include "../../app/Application.h"
#include "../../app/Clipboard.h"


static void b_beos_clipboard_changed()
{
	BString str;

	if(__be_clipboard->Lock())
	{
		BMessage *beClipMsg = NULL;
		if(!((beClipMsg =  __be_clipboard->Data()) == NULL || beClipMsg->HasBool("BHAPI:msg_from_gui")))
		{
			const char *text = NULL;
			ssize_t textLen = 0;
			beClipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
			if(textLen > 0) str.SetTo(text, (__be_int32)textLen);
		}
		__be_clipboard->Unlock();
	}

	if(str.Length() > 0)
	{
		BHAPI_DEBUG("[GRAPHICS]: Clipboard message(\"%s\") sending...", str.String());
		BMessage *clipMsg = NULL;
		if(bhapi::be_clipboard.Lock())
		{
			if((clipMsg = bhapi::be_clipboard.Data()) != NULL)
			{
				const char *text = NULL;
				ssize_t textLen = 0;
				if(clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
				   text == NULL || textLen != (ssize_t)str.Length() || str.Compare(text, (__be_int32)textLen) != 0)
				{
					bhapi::be_clipboard.Clear();
					clipMsg->AddBool("BHAPI:msg_from_gui", true);
					clipMsg->AddData("text/plain", B_MIME_TYPE, str.String(), str.Length());
					bhapi::be_clipboard.Commit();
				}
			}
			bhapi::be_clipboard.Unlock();
		}
	}
}


static bhapi::filter_result b_beos_clipboard_filter(BMessage *message, BHandler **target, BMessageFilter *filter)
{
	if(message->what != B_CLIPBOARD_CHANGED) return B_DISPATCH_MESSAGE;

	do
	{
		const char *text = NULL;
		ssize_t textLen = 0;

		BString str;
		BMessage *msg;

		bhapi::be_clipboard.Lock();
#if defined(BHAPI_ENABLE_DEBUG) && !defined(BHAPI_DISABLE_MORE_CHECKS)
		if((msg = bhapi::be_clipboard.Data()) != NULL)
		{
			msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
			if(msg->HasBool("BHAPI:msg_from_gui"))
			{
				BString aStr(text, textLen);
				BHAPI_DEBUG("[GRAPHICS]: Clipboard message(\"%s\") received.", aStr.String());
				textLen = 0;
			}
		}
#else
		if(!((msg = bhapi::be_clipboard.Data()) == NULL || msg->HasBool("BHAPI:msg_from_gui")))
			msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
#endif
		if(textLen > 0) str.SetTo(text, (__be_int32)textLen);
		bhapi::be_clipboard.Unlock();

		if(str.Length() <= 0) break;

		if(__be_clipboard->Lock())
		{
			BMessage *beClipMsg = NULL;
			__be_clipboard->Clear();
			if((beClipMsg =  __be_clipboard->Data()) != NULL)
			{
				beClipMsg->AddBool("BHAPI:msg_from_gui", true);
				beClipMsg->AddData("text/plain", B_MIME_TYPE, str.String(), str.Length());
				__be_clipboard->Commit();
			}
			__be_clipboard->Unlock();
		}
	} while(false);

	return B_DISPATCH_MESSAGE;
}


#ifndef BHAPI_GRAPHICS_BEOS_BUILT_IN
extern "C" {
namespace bhapi {
EXPORT BGraphicsEngine* instantiate_graphics_engine()
#else
BHAPI_IMPEXP BGraphicsEngine* get_built_in_graphics_engine()
#endif
{
	return(new EBeGraphicsEngine());
}
} /* namespace */
#ifndef BHAPI_GRAPHICS_BEOS_BUILT_IN
} // extern "C"
#endif


class EBePrivateApp : public BApplication
{
public:
	EBePrivateApp(EBeGraphicsEngine *engine);
	virtual ~EBePrivateApp();

	virtual void ReadyToRun();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage *msg);

private:
	EBeGraphicsEngine *fEngine;
};


EBePrivateApp::EBePrivateApp(EBeGraphicsEngine *engine)
	: BApplication(bhapi::be_app->Signature())
{
	fEngine = engine;
}


EBePrivateApp::~EBePrivateApp()
{
	__be_clipboard->StopWatching(BMessenger(this));
}


void
EBePrivateApp::ReadyToRun()
{
	fEngine->Lock();
    bhapi::release_sem_etc(fEngine->fRequestSem, 1, 0);
	fEngine->Unlock();

	__be_clipboard->StartWatching(BMessenger(this));
	b_beos_clipboard_changed();
}


bool
EBePrivateApp::QuitRequested()
{
	bool retVal = false;

	fEngine->Lock();
	if(fEngine->beDoQuit) retVal = true;
	fEngine->Unlock();

	if(!retVal) bhapi::be_app->PostMessage(B_QUIT_REQUESTED);

	return retVal;
}


void
EBePrivateApp::MessageReceived(BMessage *msg)
{
	if(msg->what == B_CLIPBOARD_CHANGED) b_beos_clipboard_changed();
	BApplication::MessageReceived(msg);
}


class EBePrivateAppWin : public BWindow
{
public:
	EBePrivateAppWin(EBeGraphicsEngine *engine);
	virtual ~EBePrivateAppWin();

	virtual thread_id Run();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage *msg);

private:
	EBeGraphicsEngine *fEngine;
};


EBePrivateAppWin::EBePrivateAppWin(EBeGraphicsEngine *engine)
	: BWindow(BRect(-100, -100, -90, -90), "BHAPI_wrapper_graphics", B_UNTYPED_WINDOW, B_AVOID_FRONT|B_AVOID_FOCUS|B_NO_WORKSPACE_ACTIVATION)
{
	fEngine = engine;
}


EBePrivateAppWin::~EBePrivateAppWin()
{
	__be_clipboard->StopWatching(BMessenger(this));
}


thread_id
EBePrivateAppWin::Run()
{
	thread_id retVal = BWindow::Run();

	if(retVal > 0)
	{
		fEngine->Lock();
        bhapi::release_sem_etc(fEngine->fRequestSem, 1, 0);
		fEngine->Unlock();

		b_beos_clipboard_changed();
		__be_clipboard->StartWatching(BMessenger(this));
	}

	return retVal;
}


bool
EBePrivateAppWin::QuitRequested()
{
	bool retVal = false;

	fEngine->Lock();
	if(fEngine->beDoQuit) retVal = true;
	fEngine->Unlock();

	if(!retVal) bhapi::be_app->PostMessage(B_QUIT_REQUESTED);

	return retVal;
}


void
EBePrivateAppWin::MessageReceived(BMessage *msg)
{
	if(msg->what == B_CLIPBOARD_CHANGED) b_beos_clipboard_changed();
	BWindow::MessageReceived(msg);
}


EBeGraphicsEngine::EBeGraphicsEngine()
	: BGraphicsEngine(), fRequestSem(NULL), beDoQuit(false), fBeThread(NULL), fClipboardFilter(NULL)
{
}


EBeGraphicsEngine::~EBeGraphicsEngine()
{
}


status_t
EBeGraphicsEngine::InitCheck()
{
	BAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || fBeThread == NULL || beDoQuit) return B_NO_INIT;
	return B_OK;
}


bool
EBeGraphicsEngine::Lock()
{
	return fLocker.Lock();
}


void
EBeGraphicsEngine::Unlock()
{
	fLocker.Unlock();
}


static status_t b_beos_graphics_task(void *arg)
{
	EBeGraphicsEngine *engine = (EBeGraphicsEngine*)arg;

	if(engine)
	{
		if(__be_app == NULL)
		{
			EBePrivateApp *privApp = new EBePrivateApp((EBeGraphicsEngine*)arg);
			if(privApp != NULL)
			{
				BHAPI_DEBUG("[GRAPHICS]: %s --- BApplication task spawned.", __PRETTY_FUNCTION__);
				privApp->Run();
				delete privApp;
				BHAPI_DEBUG("[GRAPHICS]: %s --- BApplication task quit.", __PRETTY_FUNCTION__);
			}
		}
		else
		{
			EBePrivateAppWin *privAppWin = new EBePrivateAppWin((EBeGraphicsEngine*)arg);
			if(privAppWin != NULL)
			{
				BHAPI_WARNING("[GRAPHICS]: %s --- Another BApplication running!!! Spawned a BWindow task instead.", __PRETTY_FUNCTION__);
				privAppWin->Lock();
				thread_id tid = privAppWin->Run();
				privAppWin->Unlock();
				status_t status;
				wait_for_thread(tid, &status);
				BHAPI_DEBUG("[GRAPHICS]: %s --- BWindow task quit.", __PRETTY_FUNCTION__);
			}
		}

		engine->Lock();
        if(engine->fRequestSem != NULL) bhapi::release_sem_etc(engine->fRequestSem, 2, 0);
		engine->Unlock();
	}

	return B_OK;
}


status_t
EBeGraphicsEngine::Initalize()
{
	BMessageFilter *clipboardFilter = new BMessageFilter(B_CLIPBOARD_CHANGED, b_beos_clipboard_filter);
	bhapi::be_app->Lock();
	bhapi::be_app->AddFilter(clipboardFilter);
	bhapi::be_app->Unlock();

	Lock();

	if(fBeThread != NULL)
	{
		Unlock();

		bhapi::be_app->Lock();
		bhapi::be_app->RemoveFilter(clipboardFilter);
		bhapi::be_app->Unlock();
		delete clipboardFilter;
		return B_OK;
	}

	fRequestSem = bhapi::create_sem(0, NULL);
	if(fRequestSem == NULL)
	{
		Unlock();

		bhapi::be_app->Lock();
		bhapi::be_app->RemoveFilter(clipboardFilter);
		bhapi::be_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	if((fBeThread = bhapi::create_thread(b_beos_graphics_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
        bhapi::resume_thread(fBeThread) != B_OK)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Unable to resume graphics thread!", __PRETTY_FUNCTION__);

		if(fBeThread != NULL) bhapi::delete_thread(fBeThread);
		fBeThread = NULL;

		bhapi::delete_sem(fRequestSem);
		fRequestSem = NULL;

		Unlock();

		bhapi::be_app->Lock();
		bhapi::be_app->RemoveFilter(clipboardFilter);
		bhapi::be_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	__be_int64 count = 0;

    status_t status = bhapi::acquire_sem(fRequestSem);
	if(status == B_OK) status = bhapi::get_sem_count(fRequestSem, &count);

	if(status != B_OK || count > 0)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- BApplication task run failed!", __PRETTY_FUNCTION__);

        bhapi::wait_for_thread(fBeThread, &status);

		Lock();

		bhapi::delete_thread(fBeThread);
		fBeThread = NULL;

		bhapi::delete_sem(fRequestSem);
		fRequestSem = NULL;

		fClipboardFilter = NULL;

		Unlock();

		bhapi::be_app->Lock();
		bhapi::be_app->RemoveFilter(clipboardFilter);
		bhapi::be_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	Lock();
	if(fRequestSem != NULL) bhapi::delete_sem(fRequestSem);
	fRequestSem = NULL;
	Unlock();

	return B_OK;
}


void
EBeGraphicsEngine::Cancel()
{
	BMessageFilter *clipboardFilter = NULL;

	Lock();

	if(fBeThread != NULL)
	{
		void *beThread = bhapi::open_thread(bhapi::get_thread_id(fBeThread));
		if(beThread == NULL)
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- Unable to duplicate thread handle.", __PRETTY_FUNCTION__);
			Unlock();
			return;
		}

		beDoQuit = true;

		Unlock();

		status_t status;
		do
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- sending B_QUIT_REQUESTED", __PRETTY_FUNCTION__);
			__be_app_messenger.SendMessage(B_QUIT_REQUESTED);
        }while(bhapi::wait_for_thread_etc(beThread, &status, B_TIMEOUT, 1000000) == B_TIMED_OUT);

		Lock();

		if(fBeThread != NULL)
		{
			bhapi::delete_thread(fBeThread);
			fBeThread = NULL;

			if(fRequestSem != NULL) bhapi::delete_sem(fRequestSem);
			fRequestSem = NULL;
		}

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;

		Unlock();
		bhapi::delete_thread(beThread);
		Lock();
	}

	Unlock();

	if(clipboardFilter != NULL)
	{
		bhapi::be_app->Lock();
		bhapi::be_app->RemoveFilter(clipboardFilter);
		bhapi::be_app->Unlock();
		delete clipboardFilter;
	}
}


BGraphicsContext*
EBeGraphicsEngine::CreateContext()
{
	return(new BGraphicsContext());
}


BGraphicsDrawable*
EBeGraphicsEngine::CreatePixmap(__be_uint32 w,  __be_uint32 h)
{
	return(new EBeGraphicsDrawable(this, w, h));
}


BGraphicsWindow*
EBeGraphicsEngine::CreateWindow(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
{
	return(new EBeGraphicsWindow(this, x, y, w, h));
}


status_t
EBeGraphicsEngine::GetDesktopBounds(__be_uint32 *w,  __be_uint32 *h)
{
	BScreen screen(B_MAIN_SCREEN_ID);
	if(w) *w = (__be_uint32)screen.Frame().Width() + 1;
	if(h) *h = (__be_uint32)screen.Frame().Height() + 1;
	return B_OK;
}


status_t
EBeGraphicsEngine::GetCurrentWorkspace(__be_uint32 *workspace)
{
	if(workspace) *workspace = (__be_uint32)current_workspace() + 1;
	return B_OK;
}


status_t
EBeGraphicsEngine::SetCursor(const void *cursor_data)
{
	if(cursor_data)
	{
		__be_app->ShowCursor();
		__be_app->SetCursor(cursor_data);
	}
	else
	{
		__be_app->HideCursor();
	}

	return B_OK;
}


status_t
EBeGraphicsEngine::GetDefaultCursor(BCursor *cursor)
{
	if(cursor == NULL) return B_ERROR;
	*cursor = BCursor((const void*)B_HAND_CURSOR);
	return B_OK;
}

#endif /* BEOS */
