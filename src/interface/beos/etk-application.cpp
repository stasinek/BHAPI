/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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
#ifdef BEOS

#include <stdlib.h>

#include <be/app/AppDefs.h>
#include <be/app/Application.h>
#include <be/app/Clipboard.h>
#include <be/app/Cursor.h>
#include <be/interface/Screen.h>

#include "etk-beos-graphics.h"

#include "./../../support/Autolock.h"
#include "./../../support/String.h"
#include "./../../kernel/Kernel.h"
#include "./../../app/Application.h"
#include "./../../app/Clipboard.h"


static void etk_beos_clipboard_changed()
{
	EString str;

	if(be_clipboard->Lock())
	{
		BMessage *beClipMsg = NULL;
		if(!((beClipMsg = be_clipboard->Data()) == NULL || beClipMsg->HasBool("etk:msg_from_gui")))
		{
			const char *text = NULL;
			ssize_t textLen = 0;
			beClipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
			if(textLen > 0) str.SetTo(text, (eint32)textLen);
		}
		be_clipboard->Unlock();
	}

	if(str.Length() > 0)
	{
		ETK_DEBUG("[GRAPHICS]: Clipboard message(\"%s\") sending...", str.String());
		EMessage *clipMsg = NULL;
		if(etk_clipboard.Lock())
		{
			if((clipMsg = etk_clipboard.Data()) != NULL)
			{
				const char *text = NULL;
				ssize_t textLen = 0;
				if(clipMsg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen) == false ||
				   text == NULL || textLen != (ssize_t)str.Length() || str.Compare(text, (eint32)textLen) != 0)
				{
					etk_clipboard.Clear();
					clipMsg->AddBool("etk:msg_from_gui", true);
					clipMsg->AddData("text/plain", E_MIME_TYPE, str.String(), str.Length());
					etk_clipboard.Commit();
				}
			}
			etk_clipboard.Unlock();
		}
	}
}


static e_filter_result etk_beos_clipboard_filter(EMessage *message, EHandler **target, EMessageFilter *filter)
{
	if(message->what != E_CLIPBOARD_CHANGED) return E_DISPATCH_MESSAGE;

	do
	{
		const char *text = NULL;
		ssize_t textLen = 0;

		EString str;
		EMessage *msg;

		etk_clipboard.Lock();
#if defined(ETK_ENABLE_DEBUG) && !defined(ETK_DISABLE_MORE_CHECKS)
		if((msg = etk_clipboard.Data()) != NULL)
		{
			msg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen);
			if(msg->HasBool("etk:msg_from_gui"))
			{
				EString aStr(text, textLen);
				ETK_DEBUG("[GRAPHICS]: Clipboard message(\"%s\") received.", aStr.String());
				textLen = 0;
			}
		}
#else
		if(!((msg = etk_clipboard.Data()) == NULL || msg->HasBool("etk:msg_from_gui")))
			msg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen);
#endif
		if(textLen > 0) str.SetTo(text, (eint32)textLen);
		etk_clipboard.Unlock();

		if(str.Length() <= 0) break;

		if(be_clipboard->Lock())
		{
			BMessage *beClipMsg = NULL;
			be_clipboard->Clear();
			if((beClipMsg = be_clipboard->Data()) != NULL)
			{
				beClipMsg->AddBool("etk:msg_from_gui", true);
				beClipMsg->AddData("text/plain", B_MIME_TYPE, str.String(), str.Length());
				be_clipboard->Commit();
			}
			be_clipboard->Unlock();
		}
	} while(false);

	return E_DISPATCH_MESSAGE;
}


#ifndef ETK_GRAPHICS_BEOS_BUILT_IN
extern "C" {
_EXPORT EGraphicsEngine* instantiate_graphics_engine()
#else
_IMPEXP_ETK EGraphicsEngine* etk_get_built_in_graphics_engine()
#endif
{
	return(new EBeGraphicsEngine());
}
#ifndef ETK_GRAPHICS_BEOS_BUILT_IN
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
	: BApplication(etk_app->Signature())
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
	etk_release_sem_etc(fEngine->fRequestSem, 1, 0);
	fEngine->Unlock();

	be_clipboard->StartWatching(BMessenger(this));
	etk_beos_clipboard_changed();
}


bool
EBePrivateApp::QuitRequested()
{
	bool retVal = false;

	fEngine->Lock();
	if(fEngine->beDoQuit) retVal = true;
	fEngine->Unlock();

	if(!retVal) etk_app->PostMessage(E_QUIT_REQUESTED);

	return retVal;
}


void
EBePrivateApp::MessageReceived(BMessage *msg)
{
	if(msg->what == B_CLIPBOARD_CHANGED) etk_beos_clipboard_changed();
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
		etk_release_sem_etc(fEngine->fRequestSem, 1, 0);
		fEngine->Unlock();

		etk_beos_clipboard_changed();
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

	if(!retVal) etk_app->PostMessage(E_QUIT_REQUESTED);

	return retVal;
}


void
EBePrivateAppWin::MessageReceived(BMessage *msg)
{
	if(msg->what == B_CLIPBOARD_CHANGED) etk_beos_clipboard_changed();
	BWindow::MessageReceived(msg);
}


EBeGraphicsEngine::EBeGraphicsEngine()
	: EGraphicsEngine(), fRequestSem(NULL), beDoQuit(false), fBeThread(NULL), fClipboardFilter(NULL)
{
}


EBeGraphicsEngine::~EBeGraphicsEngine()
{
}


e_status_t
EBeGraphicsEngine::InitCheck()
{
	EAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || fBeThread == NULL || beDoQuit) return E_NO_INIT;
	return E_OK;
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


static e_status_t etk_beos_graphics_task(void *arg)
{
	EBeGraphicsEngine *engine = (EBeGraphicsEngine*)arg;

	if(engine)
	{
		if(be_app == NULL)
		{
			EBePrivateApp *privApp = new EBePrivateApp((EBeGraphicsEngine*)arg);
			if(privApp != NULL)
			{
				ETK_DEBUG("[GRAPHICS]: %s --- BApplication task spawned.", __PRETTY_FUNCTION__);
				privApp->Run();
				delete privApp;
				ETK_DEBUG("[GRAPHICS]: %s --- BApplication task quit.", __PRETTY_FUNCTION__);
			}
		}
		else
		{
			EBePrivateAppWin *privAppWin = new EBePrivateAppWin((EBeGraphicsEngine*)arg);
			if(privAppWin != NULL)
			{
				ETK_WARNING("[GRAPHICS]: %s --- Another BApplication running!!! Spawned a BWindow task instead.", __PRETTY_FUNCTION__);
				privAppWin->Lock();
				thread_id tid = privAppWin->Run();
				privAppWin->Unlock();
				status_t status;
				wait_for_thread(tid, &status);
				ETK_DEBUG("[GRAPHICS]: %s --- BWindow task quit.", __PRETTY_FUNCTION__);
			}
		}

		engine->Lock();
		if(engine->fRequestSem != NULL) etk_release_sem_etc(engine->fRequestSem, 2, 0);
		engine->Unlock();
	}

	return E_OK;
}


e_status_t
EBeGraphicsEngine::Initalize()
{
	EMessageFilter *clipboardFilter = new EMessageFilter(E_CLIPBOARD_CHANGED, etk_beos_clipboard_filter);
	etk_app->Lock();
	etk_app->AddFilter(clipboardFilter);
	etk_app->Unlock();

	Lock();

	if(fBeThread != NULL)
	{
		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_OK;
	}

	fRequestSem = etk_create_sem(0, NULL);
	if(fRequestSem == NULL)
	{
		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	if((fBeThread = etk_create_thread(etk_beos_graphics_task, E_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
	    etk_resume_thread(fBeThread) != E_OK)
	{
		ETK_WARNING("[GRAPHICS]: %s --- Unable to resume graphics thread!", __PRETTY_FUNCTION__);

		if(fBeThread != NULL) etk_delete_thread(fBeThread);
		fBeThread = NULL;

		etk_delete_sem(fRequestSem);
		fRequestSem = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	eint64 count = 0;

	e_status_t status = etk_acquire_sem(fRequestSem);
	if(status == E_OK) status = etk_get_sem_count(fRequestSem, &count);

	if(status != E_OK || count > 0)
	{
		ETK_WARNING("[GRAPHICS]: %s --- BApplication task run failed!", __PRETTY_FUNCTION__);

		etk_wait_for_thread(fBeThread, &status);

		Lock();

		etk_delete_thread(fBeThread);
		fBeThread = NULL;

		etk_delete_sem(fRequestSem);
		fRequestSem = NULL;

		fClipboardFilter = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	Lock();
	if(fRequestSem != NULL) etk_delete_sem(fRequestSem);
	fRequestSem = NULL;
	Unlock();

	return E_OK;
}


void
EBeGraphicsEngine::Cancel()
{
	EMessageFilter *clipboardFilter = NULL;

	Lock();

	if(fBeThread != NULL)
	{
		void *beThread = etk_open_thread(etk_get_thread_id(fBeThread));
		if(beThread == NULL)
		{
			ETK_DEBUG("[GRAPHICS]: %s --- Unable to duplicate thread handle.", __PRETTY_FUNCTION__);
			Unlock();
			return;
		}

		beDoQuit = true;

		Unlock();

		e_status_t status;
		do
		{
			ETK_DEBUG("[GRAPHICS]: %s --- sending B_QUIT_REQUESTED", __PRETTY_FUNCTION__);
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		}while(etk_wait_for_thread_etc(beThread, &status, E_TIMEOUT, 1000000) == E_TIMED_OUT);

		Lock();

		if(fBeThread != NULL)
		{
			etk_delete_thread(fBeThread);
			fBeThread = NULL;

			if(fRequestSem != NULL) etk_delete_sem(fRequestSem);
			fRequestSem = NULL;
		}

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;

		Unlock();
		etk_delete_thread(beThread);
		Lock();
	}

	Unlock();

	if(clipboardFilter != NULL)
	{
		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
	}
}


EGraphicsContext*
EBeGraphicsEngine::CreateContext()
{
	return(new EGraphicsContext());
}


EGraphicsDrawable*
EBeGraphicsEngine::CreatePixmap(euint32 w, euint32 h)
{
	return(new EBeGraphicsDrawable(this, w, h));
}


EGraphicsWindow*
EBeGraphicsEngine::CreateWindow(eint32 x, eint32 y, euint32 w, euint32 h)
{
	return(new EBeGraphicsWindow(this, x, y, w, h));
}


e_status_t
EBeGraphicsEngine::GetDesktopBounds(euint32 *w, euint32 *h)
{
	BScreen screen(B_MAIN_SCREEN_ID);
	if(w) *w = (euint32)screen.Frame().Width() + 1;
	if(h) *h = (euint32)screen.Frame().Height() + 1;
	return E_OK;
}


e_status_t
EBeGraphicsEngine::GetCurrentWorkspace(euint32 *workspace)
{
	if(workspace) *workspace = (euint32)current_workspace() + 1;
	return E_OK;
}


e_status_t
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

	return E_OK;
}


e_status_t
EBeGraphicsEngine::GetDefaultCursor(ECursor *cursor)
{
	if(cursor == NULL) return E_ERROR;
	*cursor = ECursor((const void*)B_HAND_CURSOR);
	return E_OK;
}

#endif /* BEOS */
