/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: etk-application.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS


#include <stdlib.h>

#include <be/app/AppDefs.h>
#include <be/app/Application.h>
#include <be/app/Clipboard.h>
#include <be/app/Cursor.h>
#include <be/interface/Screen.h>

#include "etk-beos-graphics.h"

#include "../../support/Autolock.h"
#include "../../support/String.h"
#include "../../kernel/Kernel.h"
#include "../../app/Application.h"
#include "../../app/Clipboard.h"


static void b_beos_clipboard_changed()
{
	BString str;

	if(be_clipboard->Lock())
	{
		BMessage *beClipMsg = NULL;
		if(!((beClipMsg = be_clipboard->Data()) == NULL || beClipMsg->HasBool("BHAPI:msg_from_gui")))
		{
			const char *text = NULL;
			b_size_t textLen = 0;
			beClipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
			if(textLen > 0) str.SetTo(text, (b_int32)textLen);
		}
		be_clipboard->Unlock();
	}

	if(str.Length() > 0)
	{
		BHAPI_DEBUG("[GRAPHICS]: Clipboard message(\"%s\") sending...", str.String());
		BMessage *clipMsg = NULL;
		if(b_clipboard.Lock())
		{
			if((clipMsg = b_clipboard.Data()) != NULL)
			{
				const char *text = NULL;
				b_size_t textLen = 0;
				if(clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
				   text == NULL || textLen != (b_size_t)str.Length() || str.Compare(text, (b_int32)textLen) != 0)
				{
					b_clipboard.Clear();
					clipMsg->AddBool("BHAPI:msg_from_gui", true);
					clipMsg->AddData("text/plain", B_MIME_TYPE, str.String(), str.Length());
					b_clipboard.Commit();
				}
			}
			b_clipboard.Unlock();
		}
	}
}


static b_filter_result b_beos_clipboard_filter(BMessage *message, BHandler **target, BMessageFilter *filter)
{
	if(message->what != B_CLIPBOARD_CHANGED) return B_DISPATCH_MESSAGE;

	do
	{
		const char *text = NULL;
		b_size_t textLen = 0;

		BString str;
		BMessage *msg;

		b_clipboard.Lock();
#if defined(BHAPI_ENABLE_DEBUG) && !defined(BHAPI_DISABLE_MORE_CHECKS)
		if((msg = b_clipboard.Data()) != NULL)
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
		if(!((msg = b_clipboard.Data()) == NULL || msg->HasBool("BHAPI:msg_from_gui")))
			msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
#endif
		if(textLen > 0) str.SetTo(text, (b_int32)textLen);
		b_clipboard.Unlock();

		if(str.Length() <= 0) break;

		if(be_clipboard->Lock())
		{
			BMessage *beClipMsg = NULL;
			be_clipboard->Clear();
			if((beClipMsg = be_clipboard->Data()) != NULL)
			{
				beClipMsg->AddBool("BHAPI:msg_from_gui", true);
				beClipMsg->AddData("text/plain", B_MIME_TYPE, str.String(), str.Length());
				be_clipboard->Commit();
			}
			be_clipboard->Unlock();
		}
	} while(false);

	return B_DISPATCH_MESSAGE;
}


#ifndef BHAPI_GRAPHICS_BEOS_BUILT_IN
extern "C" {
_EXPORT BGraphicsEngine* instantiate_graphics_engine()
#else
IMPEXP_BHAPI BGraphicsEngine* b_get_built_in_graphics_engine()
#endif
{
	return(new EBeGraphicsEngine());
}
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
	: BApplication(b_app->Signature())
{
	fEngine = engine;
}


EBePrivateApp::~EBePrivateApp()
{
	be_clipboard->StopWatching(BMessenger(this));
}


void
EBePrivateApp::ReadyToRun()
{
	fEngine->Lock();
	b_release_sem_etc(fEngine->fRequestSem, 1, 0);
	fEngine->Unlock();

	be_clipboard->StartWatching(BMessenger(this));
	b_beos_clipboard_changed();
}


bool
EBePrivateApp::QuitRequested()
{
	bool retVal = false;

	fEngine->Lock();
	if(fEngine->beDoQuit) retVal = true;
	fEngine->Unlock();

	if(!retVal) b_app->PostMessage(B_QUIT_REQUESTED);

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
	: BWindow(BRect(-100, -100, -90, -90), "etk-graphics", B_UNTYPED_WINDOW, B_AVOID_FRONT|B_AVOID_FOCUS|B_NO_WORKSPACE_ACTIVATION)
{
	fEngine = engine;
}


EBePrivateAppWin::~EBePrivateAppWin()
{
	be_clipboard->StopWatching(BMessenger(this));
}


thread_id
EBePrivateAppWin::Run()
{
	thread_id retVal = BWindow::Run();

	if(retVal > 0)
	{
		fEngine->Lock();
		b_release_sem_etc(fEngine->fRequestSem, 1, 0);
		fEngine->Unlock();

		b_beos_clipboard_changed();
		be_clipboard->StartWatching(BMessenger(this));
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

	if(!retVal) b_app->PostMessage(B_QUIT_REQUESTED);

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


b_status_t
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


static b_status_t b_beos_graphics_task(void *arg)
{
	EBeGraphicsEngine *engine = (EBeGraphicsEngine*)arg;

	if(engine)
	{
		if(be_app == NULL)
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
		if(engine->fRequestSem != NULL) b_release_sem_etc(engine->fRequestSem, 2, 0);
		engine->Unlock();
	}

	return B_OK;
}


b_status_t
EBeGraphicsEngine::Initalize()
{
	BMessageFilter *clipboardFilter = new BMessageFilter(B_CLIPBOARD_CHANGED, b_beos_clipboard_filter);
	b_app->Lock();
	b_app->AddFilter(clipboardFilter);
	b_app->Unlock();

	Lock();

	if(fBeThread != NULL)
	{
		Unlock();

		b_app->Lock();
		b_app->RemoveFilter(clipboardFilter);
		b_app->Unlock();
		delete clipboardFilter;
		return B_OK;
	}

	fRequestSem = b_create_sem(0, NULL);
	if(fRequestSem == NULL)
	{
		Unlock();

		b_app->Lock();
		b_app->RemoveFilter(clipboardFilter);
		b_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	if((fBeThread = b_create_thread(b_beos_graphics_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
	    b_resume_thread(fBeThread) != B_OK)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Unable to resume graphics thread!", __PRETTY_FUNCTION__);

		if(fBeThread != NULL) b_delete_thread(fBeThread);
		fBeThread = NULL;

		b_delete_sem(fRequestSem);
		fRequestSem = NULL;

		Unlock();

		b_app->Lock();
		b_app->RemoveFilter(clipboardFilter);
		b_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	b_int64 count = 0;

	b_status_t status = b_acquire_sem(fRequestSem);
	if(status == B_OK) status = b_get_sem_count(fRequestSem, &count);

	if(status != B_OK || count > 0)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- BApplication task run failed!", __PRETTY_FUNCTION__);

		b_wait_for_thread(fBeThread, &status);

		Lock();

		b_delete_thread(fBeThread);
		fBeThread = NULL;

		b_delete_sem(fRequestSem);
		fRequestSem = NULL;

		fClipboardFilter = NULL;

		Unlock();

		b_app->Lock();
		b_app->RemoveFilter(clipboardFilter);
		b_app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	Lock();
	if(fRequestSem != NULL) b_delete_sem(fRequestSem);
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
		void *beThread = b_open_thread(b_get_thread_id(fBeThread));
		if(beThread == NULL)
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- Unable to duplicate thread handle.", __PRETTY_FUNCTION__);
			Unlock();
			return;
		}

		beDoQuit = true;

		Unlock();

		b_status_t status;
		do
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- sending B_QUIT_REQUESTED", __PRETTY_FUNCTION__);
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		}while(b_wait_for_thread_etc(beThread, &status, B_TIMEOUT, 1000000) == B_TIMED_OUT);

		Lock();

		if(fBeThread != NULL)
		{
			b_delete_thread(fBeThread);
			fBeThread = NULL;

			if(fRequestSem != NULL) b_delete_sem(fRequestSem);
			fRequestSem = NULL;
		}

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;

		Unlock();
		b_delete_thread(beThread);
		Lock();
	}

	Unlock();

	if(clipboardFilter != NULL)
	{
		b_app->Lock();
		b_app->RemoveFilter(clipboardFilter);
		b_app->Unlock();
		delete clipboardFilter;
	}
}


BGraphicsContext*
EBeGraphicsEngine::CreateContext()
{
	return(new BGraphicsContext());
}


BGraphicsDrawable*
EBeGraphicsEngine::CreatePixmap(b_uint32 w, b_uint32 h)
{
	return(new EBeGraphicsDrawable(this, w, h));
}


BGraphicsWindow*
EBeGraphicsEngine::CreateWindow(b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
{
	return(new EBeGraphicsWindow(this, x, y, w, h));
}


b_status_t
EBeGraphicsEngine::GetDesktopBounds(b_uint32 *w, b_uint32 *h)
{
	BScreen screen(B_MAIN_SCREEN_ID);
	if(w) *w = (b_uint32)screen.Frame().Width() + 1;
	if(h) *h = (b_uint32)screen.Frame().Height() + 1;
	return B_OK;
}


b_status_t
EBeGraphicsEngine::GetCurrentWorkspace(b_uint32 *workspace)
{
	if(workspace) *workspace = (b_uint32)current_workspace() + 1;
	return B_OK;
}


b_status_t
EBeGraphicsEngine::SetCursor(const void *cursor_data)
{
	if(cursor_data)
	{
		be_app->ShowCursor();
		be_app->SetCursor(cursor_data);
	}
	else
	{
		be_app->HideCursor();
	}

	return B_OK;
}


b_status_t
EBeGraphicsEngine::GetDefaultCursor(BCursor *cursor)
{
	if(cursor == NULL) return B_ERROR;
	*cursor = BCursor((const void*)B_HAND_CURSOR);
	return B_OK;
}

#endif /* BEOS */
