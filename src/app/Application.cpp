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
 * File: Application.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/Locker.h>
#include "./../support/Autolock.h>
#include "./../kernel/Kernel.h>
#include "./../interface/Font.h>
#include "./../add-ons/graphics/GraphicsEngine.h>
#include "./../storage/FindDirectory.h>
#include "./../storage/Directory.h>

#include "./../private/PrivateHandler.h>

#include "Application.h"
#include "Clipboard.h"

_LOCAL ECursor _E_CURSOR_SYSTEM_DEFAULT(NULL);

_IMPEXP_ETK EApplication *etk_app = NULL;
_IMPEXP_ETK EMessenger etk_app_messenger;
_IMPEXP_ETK EClipboard etk_clipboard("system");
_IMPEXP_ETK const ECursor *E_CURSOR_SYSTEM_DEFAULT = &_E_CURSOR_SYSTEM_DEFAULT;

EList EApplication::sRunnerList;
e_bigtime_t EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(0);

extern bool etk_font_init(void);
extern void etk_font_cancel(void);
extern bool etk_font_lock(void);
extern void etk_font_unlock(void);

void
EApplication::Init(const char *signature, bool tryInterface)
{
	ELocker *hLocker = etk_get_handler_operator_locker();

	hLocker->Lock();

	if(etk_app != NULL)
		ETK_ERROR("[APP]: %s --- Another application running!", __PRETTY_FUNCTION__);

	if(signature) fSignature = EStrdup(signature);

	EMessenger msgr(this);
	EMessage pulseMsg(E_PULSE);
	fPulseRunner = new EMessageRunner(msgr, &pulseMsg, fPulseRate, 0);
	if(!(fPulseRunner == NULL || fPulseRunner->IsValid())) {delete fPulseRunner; fPulseRunner = NULL;}

	etk_app = this;
	etk_app_messenger = EMessenger(this);

	hLocker->Unlock();

	etk_clipboard.StartWatching(etk_app_messenger);

	if(tryInterface) InitGraphicsEngine();
}


EApplication::EApplication(const char *signature, bool tryInterface)
	: ELooper(signature), fQuit(false), fSignature(NULL),
	  fPulseRate(E_INT64_CONSTANT(500000)), fPulseRunner(NULL),
	  fGraphicsEngine(NULL), fGraphicsEngineAddon(NULL),
	  fCursor(NULL), fCursorHidden(false), fCursorObscure(false)
{
	Init(signature, tryInterface);
}


EApplication::~EApplication()
{
	ELocker *hLocker = etk_get_handler_operator_locker();

	hLocker->Lock();
	if(!(fThread == NULL || (etk_get_current_thread_id() == etk_get_thread_id(fThread) && fQuit == true)))
		ETK_ERROR("[APP]: Task must call \"PostMessage(E_QUIT_REQUESTED)\" instead \"delete\" to quit the application!!!");
	hLocker->Unlock();

	etk_quit_all_loopers(true);

	if(fGraphicsEngine != NULL)
	{
		etk_font_lock();
		fGraphicsEngine->DestroyFonts();
		etk_font_unlock();
	}

	etk_font_cancel();

	if(fGraphicsEngine != NULL)
	{
		fGraphicsEngine->Cancel();
		delete fGraphicsEngine;
	}

	if(fGraphicsEngineAddon != NULL) etk_unload_addon(fGraphicsEngineAddon);

	if(fPulseRunner) delete fPulseRunner;
	if(fSignature) delete[] fSignature;

	hLocker->Lock();
	for(eint32 i = 0; i < fModalWindows.CountItems(); i++)
	{
		EMessenger *tMsgr = (EMessenger*)fModalWindows.ItemAt(i);
		delete tMsgr;
	}
	fModalWindows.MakeEmpty();

	etk_clipboard.StopWatching(etk_app_messenger);
	etk_app_messenger = EMessenger();

	hLocker->Unlock();
}


EApplication::EApplication(const EMessage *from)
	: ELooper(NULL, E_DISPLAY_PRIORITY), fQuit(false), fSignature(NULL),
	  fPulseRate(E_INT64_CONSTANT(500000)), fPulseRunner(NULL),
	  fGraphicsEngine(NULL), fGraphicsEngineAddon(NULL),
	  fCursor(NULL), fCursorHidden(false), fCursorObscure(false)
{
	// TODO
	Init(NULL, !(from == NULL || from->HasBool("etk:has_gui") == false));
}


e_status_t
EApplication::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	ELooper::Archive(into, deep);
	into->AddString("class", "EApplication");

	// TODO

	return E_OK;
}


EArchivable*
EApplication::Instantiate(const EMessage *from)
{
	if(e_validate_instantiation(from, "EApplication"))
		return new EApplication(from);
	return NULL;
}


void*
EApplication::Run()
{
	ELocker *hLocker = etk_get_handler_operator_locker();

	hLocker->Lock();

	if(fThread)
		ETK_ERROR("[APP]: %s --- Thread must run only one time!", __PRETTY_FUNCTION__);

	if((fThread = etk_open_thread(etk_get_current_thread_id())) == NULL)
		fThread = etk_create_thread_by_current_thread();

	if(fThread == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create thread!", __PRETTY_FUNCTION__);

	hLocker->Unlock();

	Lock();

	ReadyToRun();

	EMessage *aMsg = NULL;
	while(!fQuit)
	{
		if((aMsg = NextLooperMessage(E_INFINITE_TIMEOUT)) != NULL) DispatchLooperMessage(aMsg);
		if(!fQuit)
		{
			MessageQueue()->Lock();
			aMsg = MessageQueue()->FindMessage((eint32)0);
			if(!(aMsg == NULL || aMsg->what != _QUIT_)) fQuit = true;
			MessageQueue()->Unlock();
		}
	}

	fQuit = true;

	Unlock();

	return NULL;
}


void
EApplication::etk_dispatch_message_runners()
{
	ELocker *hLocker = etk_get_handler_operator_locker();

	hLocker->Lock();

	if(sRunnerMinimumInterval == E_INT64_CONSTANT(0))
	{
		hLocker->Unlock();
		return;
	}

	sRunnerMinimumInterval = E_INT64_CONSTANT(0);
	e_bigtime_t curTime = etk_real_time_clock_usecs();
	for(eint32 i = 0; i < sRunnerList.CountItems(); i++)
	{
		EMessageRunner *runner = (EMessageRunner*)sRunnerList.ItemAt(i);
		if(runner == NULL || runner->IsValid() == false || runner->fCount == 0 || runner->fInterval <= E_INT64_CONSTANT(0) ||
		   runner->fTarget == NULL || runner->fTarget->IsValid() == false || runner->fMessage == NULL) continue;

		if(runner->fPrevSendTime < E_INT64_CONSTANT(0) || curTime - runner->fPrevSendTime >= runner->fInterval)
		{
			// TODO: replyTo
			runner->fPrevSendTime = curTime;
			bool send = (runner->fTarget->SendMessage(runner->fMessage, (EHandler*)NULL, E_INT64_CONSTANT(50000)) == E_OK);

			if(sRunnerList.ItemAt(i) != (void*)runner || sRunnerMinimumInterval < E_INT64_CONSTANT(0)) {i = 0; continue;}
			if(send && runner->fCount > 0) runner->fCount -= 1;
			if(runner->IsValid() == false || runner->fCount == 0 || runner->fInterval <= E_INT64_CONSTANT(0) ||
			   runner->fTarget == NULL || runner->fTarget->IsValid() == false || runner->fMessage == NULL) continue;
			if(sRunnerMinimumInterval == E_INT64_CONSTANT(0) ||
			   runner->fInterval < sRunnerMinimumInterval) sRunnerMinimumInterval = runner->fInterval;
		}
		else if(sRunnerMinimumInterval == E_INT64_CONSTANT(0) ||
			runner->fInterval - (curTime - runner->fPrevSendTime) <  sRunnerMinimumInterval)
		{
			sRunnerMinimumInterval = runner->fInterval - (curTime - runner->fPrevSendTime);
		}
	}

	hLocker->Unlock();
}


bool
EApplication::QuitRequested()
{
	return(etk_quit_all_loopers(false));
}


void
EApplication::DispatchMessage(EMessage *msg, EHandler *target)
{
	if(fQuit) return;

	if(target == NULL) target = fPreferredHandler;
	if(!target || target->Looper() != this) return;

	if(msg->what == E_QUIT_REQUESTED && target == this)
	{
		if(QuitRequested()) Quit();
	}
	else if(msg->what == E_APP_CURSOR_REQUESTED && target == this)
	{
		const void *cursor_data = NULL;
		ssize_t len;
		bool show_cursor;

		if(msg->FindData("etk:cursor_data", E_ANY_TYPE, &cursor_data, &len))
		{
			if(len > 0)
			{
				ECursor newCursor(cursor_data);
				SetCursor(&newCursor);
			}
		}
		else if(msg->FindBool("etk:show_cursor", &show_cursor))
		{
			if(show_cursor) ShowCursor();
			else HideCursor();
		}
		else if(msg->HasBool("etk:obscure_cursor"))
		{
			ObscureCursor();
		}
	}
	else
	{
		ELooper::DispatchMessage(msg, target);
	}
}


void
EApplication::Quit()
{
	if(!IsLockedByCurrentThread())
		ETK_ERROR("[APP]: %s --- Application must LOCKED before this call!", __PRETTY_FUNCTION__);
	else if(fThread == NULL)
		ETK_ERROR("[APP]: %s --- Application isn't running!", __PRETTY_FUNCTION__);
	else if(etk_get_thread_id(fThread) != etk_get_current_thread_id())
		ETK_ERROR("\n\
**************************************************************************\n\
*                           [APP]: EApplication                          *\n\
*                                                                        *\n\
*      Task must call \"PostMessage(E_QUIT_REQUESTED)\" instead of         *\n\
*      \"Quit()\" outside the looper!!!                                    *\n\
**************************************************************************\n\n");

	etk_close_sem(fSem);

	fQuit = true;

	ETK_DEBUG("[APP]: Application Quit.");
}


void
EApplication::ReadyToRun()
{
}


void
EApplication::Pulse()
{
}


void
EApplication::SetPulseRate(e_bigtime_t rate)
{
	if(fPulseRunner == NULL)
	{
		ETK_DEBUG("[APP]: %s --- No message runner.", __PRETTY_FUNCTION__);
		return;
	}

	if(fPulseRunner->SetInterval(rate) == E_OK)
	{
		fPulseRate = rate;
		fPulseRunner->SetCount(rate > E_INT64_CONSTANT(0) ? -1 : 0);
	}
	else
	{
		ETK_DEBUG("[APP]: %s --- Unable to set pulse rate.", __PRETTY_FUNCTION__);
	}
}


e_bigtime_t
EApplication::PulseRate() const
{
	if(fPulseRunner == NULL) return E_INT64_CONSTANT(-1);
	return fPulseRate;
}


void
EApplication::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case E_PULSE:
			Pulse();
			break;

		case E_MOUSE_DOWN:
		case E_MOUSE_UP:
		case E_MOUSE_MOVED:
		case E_MOUSE_WHEEL_CHANGED:
		case E_KEY_DOWN:
		case E_KEY_UP:
		case E_MODIFIERS_CHANGED:
			{
				if(msg->what == E_MOUSE_MOVED && !fCursorHidden && fCursorObscure) ShowCursor();

				EMessenger msgr;
				if(msg->FindMessenger("etk:msg_for_target", &msgr))
				{
					// TODO: Floating window shouldt receive the MOUSE event
					ELocker *hLocker = etk_get_handler_operator_locker();
					hLocker->Lock();
					EMessenger *tMsgr = (EMessenger*)fModalWindows.ItemAt(0);
					if(tMsgr != NULL) msgr = *tMsgr;
					hLocker->Unlock();

					if(msgr.IsValid() == false)
					{
						ETK_DEBUG("[APP]: %s --- Invalid messenger.", __PRETTY_FUNCTION__);
						break;
					}

					msg->RemoveMessenger("etk:msg_for_target");
					if(tMsgr != NULL) msg->RemovePoint("where");
					msgr.SendMessage(msg);
				}
				else
				{
					ELooper::MessageReceived(msg);
				}
			}
			break;

		default:
			ELooper::MessageReceived(msg);
	}
}


#if 0
eint32
EApplication::CountLoopers()
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	return ELooper::sLooperList.CountItems();
}


ELooper*
EApplication::LooperAt(eint32 index)
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	return (ELooper*)(ELooper::sLooperList.ItemAt(index));
}
#endif


bool
EApplication::etk_quit_all_loopers(bool force)
{
	eint32 index;
	ELooper *looper = NULL;
	ELocker *hLocker = etk_get_handler_operator_locker();

	while(true)
	{
		hLocker->Lock();

		if(etk_app != this) {hLocker->Unlock(); return false;}

		looper = NULL;
		for(index = 0; index < ELooper::sLooperList.CountItems(); index++)
		{
			looper = (ELooper*)(ELooper::sLooperList.ItemAt(index));
			if(looper == etk_app) looper = NULL; // ignore etk_app
			if(looper != NULL) break;
		}

		if(looper == NULL)
		{
			hLocker->Unlock();
			break;
		}

		if(looper->IsDependsOnOthersWhenQuitRequested())
		{
			ELooper::sLooperList.MoveItem(index, ELooper::sLooperList.CountItems() - 1);
			hLocker->Unlock();
			ETK_DEBUG("[APP]: %s --- Looper depends on others, retry again...", __PRETTY_FUNCTION__);
			continue;
		}

		if(looper->Lock() == false)
		{
			ELooper::sLooperList.MoveItem(index, ELooper::sLooperList.CountItems() - 1);
			hLocker->Unlock();
			ETK_DEBUG("[APP]: %s --- Lock looper failed, retry again...", __PRETTY_FUNCTION__);
			e_snooze(5000);
			continue;
		}

		if(!force)
		{
			if(looper->QuitRequested() == false)
			{
				looper->Unlock();
				hLocker->Unlock();
				return false;
			}
		}

		ELooper::sLooperList.RemoveItem(looper);

		hLocker->Unlock();

		looper->Quit();
	}

	return true;
}


const char*
EApplication::Signature() const
{
	return fSignature;
}


#ifndef ETK_GRAPHICS_NONE_BUILT_IN
extern EGraphicsEngine* etk_get_built_in_graphics_engine();
#endif

void
EApplication::InitGraphicsEngine()
{
	bool hasEngine = false;

	do {
		EAutolock <ELooper>autolock(this);

		if(fGraphicsEngine != NULL) ETK_ERROR("[APP]: %s --- This function must run only one time!", __PRETTY_FUNCTION__);

		EPath aPath;

		for(eint8 i = 0; i < 3; i++)
		{
			if(i < 2)
			{
				if(e_find_directory(i == 0 ? E_USER_ADDONS_DIRECTORY : E_ADDONS_DIRECTORY, &aPath) != E_OK)
				{
					ETK_DEBUG("[APP]: Unable to find %s.", i == 0 ? "E_USER_ADDONS_DIRECTORY" : "E_ADDONS_DIRECTORY");
					continue;
				}

				aPath.Append("etkxx/graphics");
				EDirectory directory(aPath.Path());
				if(directory.InitCheck() != E_OK)
				{
					ETK_DEBUG("[APP]: Unable to read directory(%s).", aPath.Path());
					continue;
				}

				EEntry aEntry;
				while(directory.GetNextEntry(&aEntry, false) == E_OK)
				{
					if(aEntry.GetPath(&aPath) != E_OK) continue;
					void *addon = etk_load_addon(aPath.Path());
					if(addon == NULL)
					{
						ETK_WARNING("[APP]: Unable to load addon(%s).", aPath.Path());
						continue;
					}

					EGraphicsEngine* (*instantiate_func)() = NULL;
					if(etk_get_image_symbol(addon, "instantiate_graphics_engine", (void**)&instantiate_func) != E_OK)
					{
						ETK_WARNING("[APP]: Unable to get symbol of image(%s).", aPath.Path());
						etk_unload_addon(addon);
						continue;
					}

					EGraphicsEngine *engine = (*instantiate_func)();
					if(engine == NULL || engine->Initalize() != E_OK)
					{
						ETK_DEBUG("[APP]: Unable to initalize engine(%s).", aPath.Path());
						etk_unload_addon(addon);
						continue;
					}

					fGraphicsEngine = engine;
					fGraphicsEngineAddon = addon;
					break;
				}

				if(fGraphicsEngine != NULL) break;
			}
			else
			{
#ifndef ETK_GRAPHICS_NONE_BUILT_IN
				EGraphicsEngine *engine = etk_get_built_in_graphics_engine();
				if(engine == NULL || engine->Initalize() != E_OK)
				{
					ETK_WARNING("[APP]: Unable to initalize built-in engine.");
					break;
				}

				fGraphicsEngine = engine;
				fGraphicsEngineAddon = NULL;
#else
				break;
#endif
			}
		}

		if(fGraphicsEngine != NULL)
		{
			hasEngine = true;
			if(fGraphicsEngine->GetDefaultCursor(&_E_CURSOR_SYSTEM_DEFAULT) != E_OK)
				_E_CURSOR_SYSTEM_DEFAULT = *E_CURSOR_HAND;
			fCursor = _E_CURSOR_SYSTEM_DEFAULT;
			fGraphicsEngine->SetCursor(fCursorHidden ? NULL : fCursor.Data());
		}
	} while(false);

	if(hasEngine)
	{
		etk_font_lock();
		fGraphicsEngine->InitalizeFonts();
		etk_font_unlock();
		etk_font_init();
	}
	else
	{
		ETK_WARNING("[APP]: No graphics engine found.");
	}
}


bool
EApplication::AddModalWindow(EMessenger &msgr)
{
	EMessenger *aMsgr = new EMessenger(msgr);
	if(aMsgr == NULL || aMsgr->IsValid() == false)
	{
		if(aMsgr) delete aMsgr;
		return false;
	}

	ELocker *hLocker = etk_get_handler_operator_locker();
	hLocker->Lock();
	for(eint32 i = 0; i < fModalWindows.CountItems(); i++)
	{
		EMessenger *tMsgr = (EMessenger*)fModalWindows.ItemAt(i);
		if(*tMsgr == *aMsgr)
		{
			hLocker->Unlock();
			delete aMsgr;
			return false;
		}
	}
	if(fModalWindows.AddItem(aMsgr, 0) == false)
	{
		hLocker->Unlock();
		delete aMsgr;
		return false;
	}
	hLocker->Unlock();

	return true;
}


bool
EApplication::RemoveModalWindow(EMessenger &msgr)
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	hLocker->Lock();
	for(eint32 i = 0; i < fModalWindows.CountItems(); i++)
	{
		EMessenger *tMsgr = (EMessenger*)fModalWindows.ItemAt(i);
		if(*tMsgr == msgr)
		{
			if(fModalWindows.RemoveItem(tMsgr) == false) break;
			hLocker->Unlock();
			delete tMsgr;
			return true;
		}
	}
	hLocker->Unlock();

	return false;
}


void
EApplication::SetCursor(const ECursor *cursor, bool sync)
{
	if(cursor == NULL || cursor->DataLength() == 0) return;

	if(etk_get_current_thread_id() != Thread())
	{
		EMessage msg(E_APP_CURSOR_REQUESTED);
		msg.AddData("etk:cursor_data", E_ANY_TYPE, cursor->Data(), (size_t)cursor->DataLength(), true);
		if(sync == false) etk_app_messenger.SendMessage(&msg);
		else etk_app_messenger.SendMessage(&msg, &msg);
	}
	else if(fCursor != *cursor)
	{
		fCursor = *cursor;
		if(fGraphicsEngine && !fCursorHidden) fGraphicsEngine->SetCursor(fCursor.Data());
	}
}


void
EApplication::HideCursor()
{
	if(etk_get_current_thread_id() != Thread())
	{
		EMessage msg(E_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:show_cursor", false);
		etk_app_messenger.SendMessage(&msg);
	}
	else if(!fCursorHidden)
	{
		fCursorHidden = true;
		fCursorObscure = false;
		if(fGraphicsEngine) fGraphicsEngine->SetCursor(NULL);
	}
}


void
EApplication::ShowCursor()
{
	if(etk_get_current_thread_id() != Thread())
	{
		EMessage msg(E_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:show_cursor", true);
		etk_app_messenger.SendMessage(&msg);
	}
	else if(fCursorHidden || fCursorObscure)
	{
		fCursorHidden = false;
		fCursorObscure = false;
		if(fGraphicsEngine) fGraphicsEngine->SetCursor(fCursor.Data());
	}
}


void
EApplication::ObscureCursor()
{
	if(etk_get_current_thread_id() != Thread())
	{
		EMessage msg(E_APP_CURSOR_REQUESTED);
		msg.AddBool("etk:obscure_cursor", true);
		etk_app_messenger.SendMessage(&msg);
	}
	else if(!fCursorHidden && !fCursorObscure)
	{
		fCursorObscure = true;
		if(fGraphicsEngine) fGraphicsEngine->SetCursor(NULL);
	}
}


bool
EApplication::IsCursorHidden() const
{
	return fCursorHidden;
}

