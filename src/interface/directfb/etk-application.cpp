/* --------------------------------------------------------------------------
 * 
 * DirectFB Graphics Add-on for ETK++
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

#include <string.h>

#include "etk-dfb.h"

#include "./../kernel/Kernel.h>
#include "./../support/Autolock.h>
#include "./../support/ClassInfo.h>
#include "./../support/StringArray.h>
#include "./../app/Clipboard.h>
#include "./../app/Application.h>

#ifdef ETK_OS_LINUX
extern bool etk_get_prog_argc_argv_linux(EString &progName, EStringArray &progArgv);
#endif // ETK_OS_LINUX


static void etk_dfb_clipboard_changed(EDFBGraphicsEngine *dfbEngine)
{
	EString aStr;

	char *mimetype = NULL;
	void *data = NULL;
	unsigned int dataLen = 0;

	dfbEngine->Lock();
	dfbEngine->dfbDisplay->GetClipboardTimeStamp(dfbEngine->dfbDisplay, &dfbEngine->dfbClipboardTimeStamp);
	dfbEngine->dfbDisplay->GetClipboardData(dfbEngine->dfbDisplay, &mimetype, &data, &dataLen);
	dfbEngine->Unlock();

	if(mimetype == NULL || strcmp(mimetype, "text/plain") != 0 || data == NULL || dataLen == 0)
	{
		if(mimetype) free(mimetype);
		if(data) free(data);
		return;
	}

	aStr.Append((char*)data, (eint32)dataLen);
	free(mimetype);
	free(data);

	EMessage *clipMsg = NULL;
	if(etk_clipboard.Lock())
	{
		if((clipMsg = etk_clipboard.Data()) != NULL)
		{
			const char *text = NULL;
			ssize_t textLen = 0;
			if(clipMsg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen) == false ||
			   text == NULL || textLen != (ssize_t)aStr.Length() || aStr.Compare(text, (eint32)textLen) != 0)
			{
				etk_clipboard.Clear();
				clipMsg->AddBool("etk:msg_from_gui", true);
				clipMsg->AddData("text/plain", E_MIME_TYPE, aStr.String(), aStr.Length());
				etk_clipboard.Commit();
			}
		}
		etk_clipboard.Unlock();
	}
}


class _LOCAL EDFBClipboardMessageFilter : public EMessageFilter {
public:
	EDFBGraphicsEngine *fEngine;

	EDFBClipboardMessageFilter(EDFBGraphicsEngine *dfbEngine)
		: EMessageFilter(E_CLIPBOARD_CHANGED, NULL)
	{
		fEngine = dfbEngine;
	}

	virtual e_filter_result Filter(EMessage *message, EHandler **target)
	{
		if(fEngine == NULL || message->what != E_CLIPBOARD_CHANGED) return E_DISPATCH_MESSAGE;

		do
		{
			const char *text = NULL;
			ssize_t textLen = 0;
			EString aStr;

			EMessage *msg;

			etk_clipboard.Lock();
			if(!((msg = etk_clipboard.Data()) == NULL || msg->HasBool("etk:msg_from_gui")))
			{
				msg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen);
				if(textLen > 0) aStr.Append(text, textLen);
			}
			etk_clipboard.Unlock();

			if(aStr.Length() <= 0) break;

			fEngine->Lock();
			fEngine->dfbDisplay->SetClipboardData(fEngine->dfbDisplay, "text/plain",
							      (void*)aStr.String(), aStr.Length(),
							      &fEngine->dfbClipboardTimeStamp);
			fEngine->Unlock();
		} while(false);

		return E_DISPATCH_MESSAGE;
	}
};


#ifndef ETK_GRAPHICS_DIRECTFB_BUILT_IN
extern "C" {
_EXPORT EGraphicsEngine* instantiate_graphics_engine()
#else
_IMPEXP_ETK EGraphicsEngine* etk_get_build_in_graphics_engine()
#endif
{
#ifndef ETK_GRAPHICS_DIRECTFB_BUILT_IN
	EString useDFB = getenv("ETK_USE_DIRECTFB");
	if(!(useDFB.ICompare("true") == 0 || useDFB == "1")) return NULL;
#endif
	return(new EDFBGraphicsEngine());
}
#ifndef ETK_GRAPHICS_DIRECTFB_BUILT_IN
} // extern "C"
#endif


EDFBGraphicsEngine::EDFBGraphicsEngine()
	: EGraphicsEngine(),
	  dfbDisplay(NULL), dfbDisplayLayer(NULL), dfbEventBuffer(NULL),
	  dfbDisplayWidth(0), dfbDisplayHeight(0), dfbCursor(NULL),
	  dfbDoQuit(false), fDFBThread(NULL), fClipboardFilter(NULL)
{
}


EDFBGraphicsEngine::~EDFBGraphicsEngine()
{
}


bool
EDFBGraphicsEngine::Lock()
{
	return fLocker.Lock();
}


void
EDFBGraphicsEngine::Unlock()
{
	fLocker.Unlock();
}


e_status_t
EDFBGraphicsEngine::InitCheck()
{
	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || dfbDisplay == NULL || dfbDoQuit) return E_NO_INIT;
	return E_OK;
}


static void etk_process_dfb_event(EDFBGraphicsEngine *dfbEngine, DFBEvent *evt)
{
	if(dfbEngine == NULL || evt == NULL) return;

	e_bigtime_t currentTime = e_real_time_clock_usecs();
//	ETK_DEBUG("[GRAPHICS]: %s --- Something DFB Event received.", __PRETTY_FUNCTION__);

	EMessenger etkWinMsgr;
	EMessage message;

	message.AddBool("etk:msg_from_gui", true);
	message.AddInt64("when", currentTime);

	if(evt->clazz == DFEC_WINDOW)
	{
		dfbEngine->Lock();
		DFBWindowEvent *event = &(evt->window);
		EDFBGraphicsWindow *win = (EDFBGraphicsWindow*)dfbEngine->GetDFBWindowData(event->window_id);

		if(win == NULL || win->GetContactor(&etkWinMsgr) != E_OK || etkWinMsgr.IsValid() == false)
		{
//			ETK_DEBUG("[GRAPHICS]: %s --- Can't find window for the event (win:%p).", __PRETTY_FUNCTION__, win);
			dfbEngine->Unlock();
			return;
		}

		IDirectFBWindow *dfbWindow = win->dfbWindow;

		switch(event->type)
		{
			case DWET_SIZE:
				{
					int originX, originY;
					ERect margins = win->fMargins;
					dfbWindow->GetPosition(dfbWindow, &originX, &originY);
					if(win->fWidth != (euint32)event->w || win->fHeight != (euint32)event->h)
					{
						win->fWidth = event->w;
						win->fHeight = event->h;
						dfbWindow->SetOpaqueRegion(dfbWindow, (int)margins.left, (int)margins.top,
									   (int)event->w - 1 - (int)margins.right,
									   (int)event->h - 1 - (int)margins.bottom);
						e_rgb_color c = win->BackgroundColor();
						win->dfbSurface->Clear(win->dfbSurface, c.red, c.green, c.blue, 255);
						win->RenderDecoration();
					}
					if(win->fHidden == false && (win->fOriginX != originX || win->fOriginY != originY))
					{
						win->fOriginX = originX;
						win->fOriginY = originY;
					}
					dfbEngine->Unlock();

					message.AddPoint("where", EPoint((float)originX + margins.left, (float)originY + margins.top));
					message.AddFloat("width", (float)(event->w - 1) - margins.left - margins.right);
					message.AddFloat("height", (float)(event->h - 1) - margins.top - margins.bottom);

					message.what = E_WINDOW_RESIZED;
					etkWinMsgr.SendMessage(&message);
					message.what = E_WINDOW_MOVED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_POSITION:
				{
					ERect margins = win->fMargins;
					if(win->fHidden == false && (win->fOriginX != event->x || win->fOriginY != event->y))
					{
						win->fOriginX = event->x;
						win->fOriginY = event->y;
					}
					dfbEngine->Unlock();

					message.AddPoint("where", EPoint((float)event->x + margins.left, (float)event->y + margins.top));

					message.what = E_WINDOW_MOVED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_POSITION_SIZE:
				{
					ERect margins = win->fMargins;
					if(win->fWidth != (euint32)event->w || win->fHeight != (euint32)event->h)
					{
						win->fWidth = event->w;
						win->fHeight = event->h;
						dfbWindow->SetOpaqueRegion(dfbWindow, (int)margins.left, (int)margins.top,
									   (int)event->w - 1 - (int)margins.right,
									   (int)event->h - 1 - (int)margins.bottom);
						e_rgb_color c = win->BackgroundColor();
						win->dfbSurface->Clear(win->dfbSurface, c.red, c.green, c.blue, 255);
						win->RenderDecoration();
					}
					if(win->fHidden == false && (win->fOriginX != event->x || win->fOriginY != event->y))
					{
						win->fOriginX = event->x;
						win->fOriginY = event->y;
					}
					dfbEngine->Unlock();

					message.AddPoint("where", EPoint((float)event->x + margins.left, (float)event->y + margins.top));
					message.AddFloat("width", (float)(event->w - 1) - margins.left - margins.right);
					message.AddFloat("height", (float)(event->h - 1) - margins.top - margins.bottom);

					message.what = E_WINDOW_RESIZED;
					etkWinMsgr.SendMessage(&message);
					message.what = E_WINDOW_MOVED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_CLOSE:
				{
					if(dfbEngine->dfbCurFocusWin == event->window_id)
						dfbEngine->dfbCurFocusWin = E_MAXUINT;

					if(dfbEngine->dfbCurPointerGrabbed == event->window_id)
						dfbEngine->dfbCurPointerGrabbed = E_MAXUINT;

					dfbEngine->Unlock();

					message.what = E_QUIT_REQUESTED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_GOTFOCUS:
				{
					if(win->fFlags & E_AVOID_FOCUS) break;

					dfbEngine->dfbCurFocusWin = event->window_id;
					dfbWindow->SetOpacity(dfbWindow, 0xff);
					dfbEngine->Unlock();

					message.what = E_WINDOW_ACTIVATED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_LOSTFOCUS:
#if 0
				if(dfbEngine->dfbCurFocusWin == event->window_id)
				{
					dfbEngine->dfbCurFocusWin = E_MAXUINT;
					dfbWindow->SetOpacity(dfbWindow, 0xaf);
					dfbEngine->Unlock();

					message.what = E_WINDOW_ACTIVATED;
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
#else
				dfbWindow->SetOpacity(dfbWindow, 0xaf);
#endif
				break;

			case DWET_WHEEL:
				{
					dfbEngine->Unlock();

					// TODO: delta_x
					message.what = E_MOUSE_WHEEL_CHANGED;
					float delta_x = 0;
					float delta_y = 0;
					delta_y = (event->step < 0 ? -1.0f : 1.0f);

					message.AddFloat("etk:wheel_delta_x", delta_x);
					message.AddFloat("etk:wheel_delta_y", delta_y);

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_BUTTONDOWN:
			case DWET_BUTTONUP:
				{
					if(win->HandleMouseEvent(event)) break;

					int originX, originY;
					ERect margins = win->fMargins;
					dfbWindow->GetPosition(dfbWindow, &originX, &originY);
					dfbEngine->Unlock();

					message.what = (event->type == DWET_BUTTONDOWN ? E_MOUSE_DOWN : E_MOUSE_UP);

					eint32 button = 0;
					if(event->button == DIBI_LEFT) button = 1;
					else if(event->button == DIBI_MIDDLE) button = 2;
					else if(event->button == DIBI_RIGHT) button = 3;

					eint32 buttons = button;
					DFBInputDeviceButtonMask state = event->buttons;
					if((state & DIBM_LEFT) && button != 1) buttons += 1;
					if((state & DIBM_MIDDLE) && button != 2) buttons += 2;
					if((state & DIBM_RIGHT) && button != 3) buttons += 3;
					message.AddInt32("button", button);
					message.AddInt32("buttons", buttons);

					message.AddPoint("where", EPoint((float)(event->cx - originX) - margins.left,
									 (float)(event->cy - originY) - margins.top));
					message.AddPoint("screen_where", EPoint((float)event->cx, (float)event->cy));

					// TODO: modifiers, clicks
					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_MOTION:
				{
					if(win->HandleMouseEvent(event)) break;

					int originX, originY;
					ERect margins = win->fMargins;
					dfbWindow->GetPosition(dfbWindow, &originX, &originY);
					dfbEngine->Unlock();

					message.what = E_MOUSE_MOVED;

					eint32 buttons = 0;
					DFBInputDeviceButtonMask state = event->buttons;
					if(state & DIBM_LEFT) buttons += 1;
					if(state & DIBM_MIDDLE) buttons += 2;
					if(state & DIBM_RIGHT) buttons += 3;
					message.AddInt32("buttons", buttons);

					message.AddPoint("where", EPoint((float)(event->cx - originX) - margins.left,
									 (float)(event->cy - originY) - margins.top));
					message.AddPoint("screen_where", EPoint((float)event->cx, (float)event->cy));

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			case DWET_KEYDOWN:
			case DWET_KEYUP:
				{
					dfbEngine->Unlock();

					message.AddInt32("key", (eint32)(event->key_code));

					// TODO: etk:key_repeat, modifiers, states, raw_char
					if(DFB_KEY_TYPE(event->key_symbol) == DIKT_UNICODE)
					{
						euint16 symbol = (euint16)event->key_symbol;
						if(symbol == DIKS_ENTER) symbol = E_ENTER;
						char *keybuffer = e_unicode_convert_to_utf8((const eunichar*)&symbol, 1);
						eint32 keynum = (keybuffer ? (eint32)strlen(keybuffer) : 0);

						if(keybuffer)
						{
							for(eint32 i = 0; i < keynum; i++) message.AddInt8("byte", (eint8)keybuffer[i]);
							message.AddString("bytes", keybuffer);
							free(keybuffer);
						}
					}
					else
					{
						char byte[2];
						bzero(byte, 2);

						if(event->key_symbol == DIKS_CURSOR_LEFT) byte[0] = E_LEFT_ARROW;
						else if(event->key_symbol == DIKS_CURSOR_RIGHT) byte[0] = E_RIGHT_ARROW;
						else if(event->key_symbol == DIKS_CURSOR_UP) byte[0] = E_UP_ARROW;
						else if(event->key_symbol == DIKS_CURSOR_DOWN) byte[0] = E_DOWN_ARROW;
						else if(event->key_symbol == DIKS_INSERT) byte[0] = E_INSERT;
						else if(event->key_symbol == DIKS_HOME) byte[0] = E_HOME;
						else if(event->key_symbol == DIKS_END) byte[0] = E_END;
						else if(event->key_symbol == DIKS_PAGE_UP) byte[0] = E_PAGE_UP;
						else if(event->key_symbol == DIKS_PAGE_DOWN) byte[0] = E_PAGE_DOWN;

						if(byte[0] != 0)
						{
							message.AddInt8("byte", byte[0]);
							message.AddString("bytes", byte);
						}
					}

					if(message.HasString("bytes"))
						message.what = (event->type == DWET_KEYDOWN ? E_KEY_DOWN : E_KEY_UP);
					else
						message.what = (event->type == DWET_KEYDOWN ? E_UNMAPPED_KEY_DOWN : E_UNMAPPED_KEY_UP);

					eint32 modifiers = 0;

					if(event->modifiers & DIMM_SHIFT) modifiers |= E_SHIFT_KEY;
					if(event->modifiers & DIMM_CONTROL) modifiers |= E_CONTROL_KEY;
					if(event->modifiers & DIMM_ALT) modifiers |= E_COMMAND_KEY;
					if(event->modifiers & DIMM_SUPER) modifiers |= E_MENU_KEY;
					if(event->modifiers & DIMM_HYPER) modifiers |= E_OPTION_KEY;

					if(event->locks & DILS_SCROLL) modifiers |= E_SCROLL_LOCK;
					if(event->locks & DILS_NUM) modifiers |= E_NUM_LOCK;
					if(event->locks & DILS_CAPS) modifiers |= E_CAPS_LOCK;

					message.AddInt32("modifiers", modifiers);

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);

					dfbEngine->Lock();
				}
				break;

			default:
				break;
		}

		dfbEngine->Unlock();
	}
	else if(evt->clazz == DFEC_USER && evt->user.type == DUET_WINDOWREDRAWALL)
	{
		dfbEngine->Lock();
		DFBUserEvent *event = &(evt->user);
		EDFBGraphicsWindow *win = (EDFBGraphicsWindow*)dfbEngine->GetDFBWindowData((DFBWindowID)event->data);

		if(win == NULL || win->GetContactor(&etkWinMsgr) != E_OK || etkWinMsgr.IsValid() == false)
		{
			dfbEngine->Unlock();
			return;
		}

		IDirectFBWindow *dfbWindow = win->dfbWindow;

		int width, height;
		dfbWindow->GetSize(dfbWindow, &width, &height);
		dfbEngine->Unlock();

		message.what = _UPDATE_;

		message.AddRect("etk:frame", ERect(0, 0, (float)width - 1.f, (float)height - 1.f));
		message.AddBool("etk:expose", true);

		etkWinMsgr.SendMessage(&message);
	}
}


static e_status_t etk_dfb_task(void *arg)
{
	EDFBGraphicsEngine *dfbEngine = (EDFBGraphicsEngine*)arg;

	ETK_DEBUG("[GRAPHICS]: Enter DirectFB task...");

	dfbEngine->Lock();

	while(!(dfbEngine->dfbDoQuit || dfbEngine->dfbDisplay == NULL))
	{
		dfbEngine->Unlock();

		if(dfbEngine->dfbEventBuffer->WaitForEvent(dfbEngine->dfbEventBuffer) != DFB_OK)
		{
			ETK_WARNING("[GRAPHICS]: %s --- DirectFB operate error!", __PRETTY_FUNCTION__);
			break;
		}

		dfbEngine->Lock();
		if(dfbEngine->dfbDoQuit) break;
		while(dfbEngine->dfbEventBuffer->HasEvent(dfbEngine->dfbEventBuffer) == DFB_OK)
		{
			/* Read Event */
			DFBEvent evt;
			if(dfbEngine->dfbEventBuffer->GetEvent(dfbEngine->dfbEventBuffer, &evt) != DFB_OK) break;

			dfbEngine->Unlock();
			etk_process_dfb_event(dfbEngine, &evt); // Process DFB Event
			dfbEngine->Lock();
			if(dfbEngine->dfbDoQuit) break;
		}

		struct timeval timestamp;
		bzero(&timestamp, sizeof(struct timeval));
		if(dfbEngine->dfbDisplay->GetClipboardTimeStamp(dfbEngine->dfbDisplay, &timestamp) != DFB_OK) continue;
		if(memcmp((void*)&dfbEngine->dfbClipboardTimeStamp, (void*)&timestamp, sizeof(struct timeval)) != 0)
		{
			dfbEngine->Unlock();
			etk_dfb_clipboard_changed(dfbEngine);
			dfbEngine->Lock();
		}
	}

	/* Do some clean */

	dfbEngine->dfbEventBuffer->Reset(dfbEngine->dfbEventBuffer);
	dfbEngine->dfbEventBuffer->Release(dfbEngine->dfbEventBuffer);
	dfbEngine->dfbEventBuffer= NULL;

	dfbEngine->dfbDisplayLayer = NULL;

	if(dfbEngine->dfbCursor) dfbEngine->dfbCursor->Release(dfbEngine->dfbCursor);
	dfbEngine->dfbCursor = NULL;

	dfbEngine->dfbDisplay->Release(dfbEngine->dfbDisplay);
	dfbEngine->dfbDisplay = NULL;

	dfbEngine->Unlock();

	ETK_DEBUG("[GRAPHICS]: DirectFB task quited.");

	return E_OK;
}


e_status_t
EDFBGraphicsEngine::Initalize()
{
	EMessageFilter *clipboardFilter = new EDFBClipboardMessageFilter(this);
	etk_app->Lock();
	etk_app->AddFilter(clipboardFilter);
	etk_app->Unlock();

	Lock();

	if(fDFBThread != NULL)
	{
		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	/* Do DFB Initalize */
	int eArgc = 0;
	char **eArgv = NULL;
	bool argvGotFromSystem = false;

	EString progName;
	EStringArray progArgv;

	DFBDisplayLayerConfig layer_config;

#ifdef ETK_OS_LINUX
	argvGotFromSystem = (etk_get_prog_argc_argv_linux(progName, progArgv) ? progArgv.CountItems() > 0 : false);
#endif // ETK_OS_LINUX

	if(!argvGotFromSystem || progArgv.CountItems() <= 1)
	{
		if(progName.Length() <= 0) progName.SetTo("dfb_app");
		if(progArgv.CountItems() <= 0) progArgv.AddItem(progName);

		char *options = getenv("DFBARGS");
		if(options == NULL)
		{
			if(getenv("DISPLAY") != NULL) // here have X11 and we want SDL only because the X11 module need "depth="
				options = "--dfb:system=SDL,mode=800x600,no-sighandler";
			else
				options = "--dfb:system=FBDev,mode=1024x768,vt-switching,no-sighandler,hardware";
		}
		progArgv.AddItem(options);
	}

	if(progName.Length() > 0 && progArgv.CountItems() > 0)
	{
		if(progArgv.FindString("--dfb-help") >= 0)
		{
			ETK_OUTPUT("%s\n", DirectFBUsageString());

			Unlock();

			etk_app->Lock();
			etk_app->RemoveFilter(clipboardFilter);
			etk_app->Unlock();
			delete clipboardFilter;
			return E_ERROR;
		}

		eArgc = (int)progArgv.CountItems();
		eArgv = new char*[progArgv.CountItems() + 1];
		for(eint32 i = 0; i < progArgv.CountItems(); i++)
			eArgv[i] = (char*)progArgv.ItemAt(i)->String();
		eArgv[progArgv.CountItems()] = NULL;
	}

	if(DirectFBInit(&eArgc, (char***)&eArgv) != DFB_OK || DirectFBCreate(&dfbDisplay) != DFB_OK)
	{
		if(eArgv) delete[] eArgv;
		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (DirectFBCreate) failed!", __PRETTY_FUNCTION__);

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}
	if(eArgv) delete[] eArgv;

	if(dfbDisplay->GetDisplayLayer(dfbDisplay, DLID_PRIMARY, &dfbDisplayLayer) != DFB_OK)
	{
		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (GetDisplayLayer) failed!", __PRETTY_FUNCTION__);

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	dfbDisplayLayer->GetConfiguration(dfbDisplayLayer, &layer_config);
	dfbDisplayLayer->SetCooperativeLevel(dfbDisplayLayer, DLSCL_ADMINISTRATIVE);
	dfbDisplayLayer->EnableCursor(dfbDisplayLayer, 1);

	dfbDisplayWidth = layer_config.width;
	dfbDisplayHeight = layer_config.height;

	if(dfbDisplay->CreateEventBuffer(dfbDisplay, &dfbEventBuffer) != DFB_OK)
	{
		dfbDisplayLayer = NULL;

		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		ETK_WARNING("[GRAPHICS]: %s --- Initalize DirectFB (CreateEventBuffer) failed!", __PRETTY_FUNCTION__);

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	dfbDoQuit = false;
	dfbCurFocusWin = E_MAXUINT;
	dfbCurPointerGrabbed = E_MAXUINT;
	bzero(&dfbClipboardTimeStamp, sizeof(struct timeval));
	dfbDisplay->GetClipboardTimeStamp(dfbDisplay, &dfbClipboardTimeStamp);

	if((fDFBThread = etk_create_thread(etk_dfb_task, E_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
	   etk_resume_thread(fDFBThread) != E_OK)
	{
		if(fDFBThread)
		{
			etk_delete_thread(fDFBThread);
			fDFBThread = NULL;
		}

		dfbEventBuffer->Release(dfbEventBuffer);
		dfbEventBuffer= NULL;

		dfbDisplayLayer = NULL;

		dfbDisplay->Release(dfbDisplay);
		dfbDisplay = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	etk_dfb_clipboard_changed(this);

	return E_OK;
}


void
EDFBGraphicsEngine::Cancel()
{
	EMessageFilter *clipboardFilter = NULL;

	Lock();

	if(fDFBThread != NULL)
	{
		void *dfbThread = etk_open_thread(etk_get_thread_id(fDFBThread));
		if(dfbThread == NULL)
		{
			Unlock();
			return;
		}

		dfbDoQuit = true;

		/* Send a pending event to EventLoop in order to quit */
		DFBUserEvent evt;
		evt.clazz = DFEC_USER;
		evt.type = DUET_EVENTPENDING;
		evt.data = NULL;

		dfbEventBuffer->PostEvent(dfbEventBuffer, DFB_EVENT(&evt));

		Unlock();

		e_status_t status;
		etk_wait_for_thread(dfbThread, &status);

		Lock();

		if(fDFBThread != NULL && etk_get_thread_id(fDFBThread) == etk_get_thread_id(dfbThread))
		{
			etk_delete_thread(fDFBThread);
			fDFBThread = NULL;

			struct etk_dfb_data *item;
			while((item = (struct etk_dfb_data*)fDFBDataList.RemoveItem((eint32)0)) != NULL) free(item);
		}

		etk_delete_thread(dfbThread);

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;
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
EDFBGraphicsEngine::CreateContext()
{
	return(new EGraphicsContext());
}


EGraphicsDrawable*
EDFBGraphicsEngine::CreatePixmap(euint32 w, euint32 h)
{
	return(new EDFBGraphicsDrawable(this, w, h));
}


EGraphicsWindow*
EDFBGraphicsEngine::CreateWindow(eint32 x, eint32 y, euint32 w, euint32 h)
{
	return(new EDFBGraphicsWindow(this, x, y, w, h));
}


e_status_t
EDFBGraphicsEngine::GetDesktopBounds(euint32 *w, euint32 *h)
{
	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return E_ERROR;

	if(w) *w = dfbDisplayWidth;
	if(h) *h = dfbDisplayHeight;

	return E_OK;
}


e_status_t
EDFBGraphicsEngine::GetCurrentWorkspace(euint32 *workspace)
{
	// don't support workspace
	if(workspace != NULL) *workspace = 0;
	return E_ERROR;
}


e_status_t
EDFBGraphicsEngine::SetCursor(const void *cursor_data)
{
	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return E_ERROR;

	if(cursor_data)
	{
		ECursor cursor(cursor_data);
		if(cursor.ColorDepth() != 1) return E_ERROR;

		DFBSurfaceDescription desc;
		desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
		desc.caps = DSCAPS_SYSTEMONLY;
		desc.pixelformat = DSPF_ARGB;
		desc.width = (int)cursor.Width();
		desc.height = (int)cursor.Height();

		IDirectFBSurface *newCursor;
		if(dfbDisplay->CreateSurface(dfbDisplay, &desc, &newCursor) != DFB_OK) return E_ERROR;

		const euint8 *bits = (const euint8*)cursor.Bits();
		const euint8 *mask = (const euint8*)cursor.Mask();

		newCursor->SetDrawingFlags(newCursor, DSDRAW_NOFX);
		newCursor->SetColor(newCursor, 0, 0, 0, 0);
		newCursor->FillRectangle(newCursor, 0, 0, desc.width, desc.height);

		for(euint8 j = 0; j < cursor.Height(); j++)
			for(euint8 i = 0; i < cursor.Width(); i += 8, bits++, mask++)
				for(euint8 k = 0; k < 8 && k + i < cursor.Width(); k++)
				{
					if(!(*mask & (1 << (7 - k)))) continue;
					if(*bits & (1 << (7 - k))) newCursor->SetColor(newCursor, 0, 0, 0, 255);
					else newCursor->SetColor(newCursor, 255, 255, 255, 255);
					newCursor->FillRectangle(newCursor, k + i, j, 1, 1);
				}

		dfbDisplayLayer->SetCursorShape(dfbDisplayLayer, newCursor, (int)cursor.SpotX(), (int)cursor.SpotY());
		dfbDisplayLayer->SetCursorOpacity(dfbDisplayLayer, 255);
		dfbDisplayLayer->WaitForSync(dfbDisplayLayer);

		if(dfbCursor) dfbCursor->Release(dfbCursor);
		dfbCursor = newCursor;
	}
	else
	{
		dfbDisplayLayer->SetCursorOpacity(dfbDisplayLayer, 0);
	}

	return E_OK;
}


e_status_t
EDFBGraphicsEngine::GetDefaultCursor(ECursor *cursor)
{
	return E_ERROR;
}


bool
EDFBGraphicsEngine::SetDFBWindowData(IDirectFBWindow *dfbWin, void *data, void **old_data)
{
	if(dfbWin == NULL) return false;

	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return false;

	bool found = false;
	for(eint32 i = 0; i < fDFBDataList.CountItems(); i++)
	{
		struct etk_dfb_data *item = (struct etk_dfb_data*)fDFBDataList.ItemAt(i);
		if(item->win == dfbWin)
		{
			if(old_data) *old_data = item->data;

			if(data)
			{
				item->data = data;
			}
			else
			{
				fDFBDataList.RemoveItem(item);
				free(item);
			}

			found = true;
			break;
		}
	}

	if(!found && data)
	{
		struct etk_dfb_data *item = (struct etk_dfb_data*)malloc(sizeof(struct etk_dfb_data));
		if(!(item == NULL || fDFBDataList.AddItem(item) == false))
		{
			item->win = dfbWin;
			item->data = data;
			if(old_data) *old_data = NULL;
			return true;
		}
		if(item) free(item);
	}

	return found;
}


void*
EDFBGraphicsEngine::GetDFBWindowData(IDirectFBWindow *dfbWin)
{
	if(dfbWin == NULL) return NULL;

	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return NULL;

	for(eint32 i = 0; i < fDFBDataList.CountItems(); i++)
	{
		struct etk_dfb_data *item = (struct etk_dfb_data*)fDFBDataList.ItemAt(i);
		if(item->win == dfbWin) return item->data;
	}

	return NULL;
}


void*
EDFBGraphicsEngine::GetDFBWindowData(DFBWindowID dfbWinID)
{
	if(dfbWinID == E_MAXUINT) return NULL;

	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return NULL;

	for(eint32 i = 0; i < fDFBDataList.CountItems(); i++)
	{
		struct etk_dfb_data *item = (struct etk_dfb_data*)fDFBDataList.ItemAt(i);
		DFBWindowID id;
		if(item->win->GetID(item->win, &id) != DFB_OK) continue;
		if(id == dfbWinID) return item->data;
	}

	return NULL;
}


bool
EDFBGraphicsEngine::ConvertRegion(const ERegion *region, DFBRegion **dfbRegions, int *nRegions)
{
	if(dfbRegions == NULL || nRegions == NULL) return false;

	eint32 nrectsNeeded = max_c((region ? region->CountRects() : 0), 1);
	if((*dfbRegions = (DFBRegion*)malloc(sizeof(DFBRegion) * (size_t)nrectsNeeded)) == NULL) return false;
	*nRegions = 0;

	if(region != NULL)
	{
		for(eint32 i = 0; i < region->CountRects(); i++)
		{
			ERect r = region->RectAt(i).FloorCopy();

			(*dfbRegions)[*nRegions].x1 = (int)r.left;
			(*dfbRegions)[*nRegions].y1 = (int)r.top;
			(*dfbRegions)[*nRegions].x2 = (int)r.right;
			(*dfbRegions)[*nRegions].y2 = (int)r.bottom;

			*nRegions += 1;
		}
	}

	if(*nRegions == 0)
	{
		if(region == NULL)
		{
			(*dfbRegions)->x1 = 0;
			(*dfbRegions)->y1 = 0;
			(*dfbRegions)->x2 = E_MAXINT;
			(*dfbRegions)->y2 = E_MAXINT;
			*nRegions = 1;
		}
		else
		{
			free(*dfbRegions);
			*dfbRegions = NULL;
		}
	}

	return(*nRegions != 0);
}

