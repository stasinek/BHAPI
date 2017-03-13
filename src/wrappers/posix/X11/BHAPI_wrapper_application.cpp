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
#ifdef LINUX

#include <stdlib.h>
#include <locale.h>

#include "BHAPI_wrapper_x11.h"

#include "../../kernel/Kernel.h"
#include "../../support/StringClass.h"
#include "../../support/Locker.h"
#include "../../support/Autolock.h"
#include "../../app/Application.h"
#include "../../app/Clipboard.h"
#include "../../interface/InterfaceDefs.h"

#define CLICK_TIMEOUT 200


static void b_x11_clipboard_changed(const char *aStr)
{
	if(aStr == NULL || *aStr == 0) return;

	BMessage *clipMsg = NULL;
	if(bhapi::clipboard.Lock())
	{
		if((clipMsg = bhapi::clipboard.Data()) != NULL)
		{
			const char *text = NULL;
			__be_size_t textLen = 0;
			if(clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
			   text == NULL || textLen != (__be_size_t)strlen(aStr) || strncmp(text, aStr, (size_t)textLen) != 0)
			{
				bhapi::clipboard.Clear();
				clipMsg->AddBool("BHAPI:msg_from_gui", true);
				clipMsg->AddData("text/plain", B_MIME_TYPE, aStr, strlen(aStr));
				bhapi::clipboard.Commit();
			}
		}
		bhapi::clipboard.Unlock();
	}
}


class BHAPI_LOCAL EX11ClipboardMessageFilter : public BMessageFilter {
public:
	EXGraphicsEngine *fEngine;

	EX11ClipboardMessageFilter(EXGraphicsEngine *x11Engine)
		: BMessageFilter(B_CLIPBOARD_CHANGED, NULL)
	{
		fEngine = x11Engine;
	}

	virtual bhapi::filter_result Filter(BMessage *message, BHandler **target)
	{
		if(fEngine == NULL || message->what != B_CLIPBOARD_CHANGED) return B_DISPATCH_MESSAGE;

		do
		{
			const char *text = NULL;
			__be_size_t textLen = 0;

			BMessage *msg;

			bhapi::clipboard.Lock();
			if(!((msg = bhapi::clipboard.Data()) == NULL || msg->HasBool("BHAPI:msg_from_gui")))
				msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
			bhapi::clipboard.Unlock();

			if(textLen <= 0) break;

			fEngine->Lock();
			XSetSelectionOwner(fEngine->xDisplay, XA_PRIMARY, fEngine->xProtocolsWindow, CurrentTime);
			fEngine->Unlock();
		} while(false);

		return B_DISPATCH_MESSAGE;
	}
};


static int b_x_io_error_handler(Display *display)
{
	BHAPI_ERROR("[GRAPHICS]: X11 IO Error!");
	return 0;
}


static int b_x_error_handler(Display *display, XErrorEvent *event)
{
#ifndef BHAPI_DISABLE_MORE_CHECKS
	char buffer[1024];
	bzero(buffer, sizeof(buffer));

	XGetErrorText(display, event->error_code, buffer, 1024);

	BHAPI_OUTPUT("\n[GRAPHICS]: X11 Error:\n"
		   "            X Error of failed request: %s\n"
		   "            Major opcode of failed request: %d\n"
		   "            Serial number of failed request: %d\n",
		   buffer, event->request_code, event->serial);
#endif

	return 0;
}


#ifndef BHAPI_GRAPHICS_X11_BUILT_IN
extern "C" {
namespace bhapi {
BHAPI_EXPORT BGraphicsEngine* instantiate_graphics_engine()
#else
BHAPI_IMPEXP BGraphicsEngine* get_built_in_graphics_engine()
#endif
{
#if !(defined(BHAPI_GRAPHICS_X11_BUILT_IN) || defined(BHAPI_OS_UNIX) || defined(BHAPI_OS_CYGWIN))
	BString useX11 = getenv("BHAPI_USE_X11");
	if(!(useX11.ICompare("true") == 0 || useX11 == "1")) return NULL;
#endif
	if(getenv("DISPLAY") == NULL) return NULL;
	return(new EXGraphicsEngine());
}
} /* namespace */
#ifndef BHAPI_GRAPHICS_X11_BUILT_IN
} // extern "C"
#endif


EXGraphicsEngine::EXGraphicsEngine()
	: BGraphicsEngine(),
	  xDisplay(NULL), xSupportThreads(false),
	  xVisual(NULL), xInputMethodEventMask(0), xInputMethod(NULL), xInputContext(NULL),
	  xDoQuit(false), xPrevMouseDownSerial(B_MAXULONG), xPrevMouseDownTime(0), xPrevMouseDownCount(0),
	  xLocksCount(B_INT64_CONSTANT(0)), fX11Thread(NULL), fClipboardFilter(NULL)
{
}


EXGraphicsEngine::~EXGraphicsEngine()
{
	DestroyFonts();
	Cancel();
}


bool
EXGraphicsEngine::Lock()
{
	if(!fLocker.Lock()) return false;

	if(xDisplay != NULL && xSupportThreads)
	{
		if(xLocksCount == B_INT64_CONSTANT(0)) XLockDisplay(xDisplay);
		xLocksCount++;
	}

	return true;
}


void
EXGraphicsEngine::Unlock()
{
	if(fLocker.CountLocks() <= B_INT64_CONSTANT(0))
	{
		BHAPI_WARNING("[GRAPHICS]: %s -- Locker didn't locked by current thread.", __PRETTY_FUNCTION__);
		return;
	}

	if(xDisplay != NULL && xLocksCount > B_INT64_CONSTANT(0) && xSupportThreads)
	{
		xLocksCount--;
		if(xLocksCount == B_INT64_CONSTANT(0)) XUnlockDisplay(xDisplay);
	}

	fLocker.Unlock();
}


status_t
EXGraphicsEngine::InitCheck()
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || xDisplay == NULL || xDoQuit) return B_NO_INIT;
	return B_OK;
}


bool
EXGraphicsEngine::GetContactor(Window w, BMessenger *msgr)
{
	if(w == None || msgr == NULL) return false;

	b_x11_address_t *self_address = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return false;

	if(XGetWindowProperty(xDisplay, w,
			      XInternAtom(xDisplay, "ATOMBHAPI_WINDOW_ADDRESS", False), 0, BHAPI_X11_ADDRESS_T_NELEMENTS,
			      False, XInternAtom(xDisplay, "ATOMBHAPI_CAST_ADDRESS", False),
			      &type, &format, &nitems,
			      &bytes_after, (unsigned char **)&self_address) != Success) return false;

	if(!self_address || format != BHAPI_X11_ADDRESS_T_FORMAT || nitems != BHAPI_X11_ADDRESS_T_NELEMENTS)
	{
//		BHAPI_DEBUG("[GRAPHICS]: %s --- Can't get address for XWindow!", __PRETTY_FUNCTION__);
		if(self_address) XFree(self_address);
		return false;
	}

	EXGraphicsWindow *win = (EXGraphicsWindow*)reinterpret_cast<void*>(*self_address);
	if(!win)
	{
//		BHAPI_DEBUG("[GRAPHICS]: %s --- Can't get address for XWindow!", __PRETTY_FUNCTION__);
		XFree(self_address);
		return false;
	}

	XFree(self_address);

	*msgr = win->fMsgr;

	return true;
}


static void b_process_x_event(EXGraphicsEngine *x11Engine, XEvent *event)
{
	if(x11Engine == NULL || event == NULL) return;

	bigtime_t currentTime = b_real_time_clock_usecs();
//	BHAPI_DEBUG("[GRAPHICS]: Event->Type = %ld", event->type);

	bool handled = false;
	BMessenger etkWinMsgr;
	BMessage message;

	message.AddBool("BHAPI:msg_from_gui", true);
	message.AddInt64("when", currentTime);

	switch(event->type)
	{
		case SelectionClear: // FIXME: to be watching...
			{
				if(event->xselectionclear.selection != XA_PRIMARY) break;

				x11Engine->Lock();
				XConvertSelection(x11Engine->xDisplay,
						  XA_PRIMARY, x11Engine->atomCompoundText, x11Engine->atomClipboard,
						  x11Engine->xProtocolsWindow, CurrentTime);
				x11Engine->Unlock();
			}
			break;

		case SelectionRequest:
			{
				// TODO: pixmap, etc...
				XEvent respond;
				respond.xselection.property = None;

				x11Engine->Lock();
				if(event->xselectionrequest.selection == XA_PRIMARY)
				{
					BString aStr;

					if(bhapi::clipboard.Lock())
					{
						BMessage *clipMsg = NULL;
						if((clipMsg = bhapi::clipboard.Data()) != NULL)
						{
							const char *text = NULL;
							__be_size_t textLen = 0;
							clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
							if(textLen > 0) aStr.SetTo(text, (__be_int32)textLen);
						}
					}
					bhapi::clipboard.Unlock();

					if(aStr.Length() > 0 && event->xselectionrequest.target == x11Engine->atomCompoundText)
					{
						unichar32 *wStr = bhapi::utf8_convert_to_utf32(aStr.String(), -1);
						XTextProperty prop;
						prop.value = NULL;
						if(!(wStr == NULL ||
						     XwcTextListToTextProperty(x11Engine->xDisplay,
							   		     (wchar_t**)&wStr, 1, XCompoundTextStyle, &prop) != Success))
						{
							XChangeProperty(x11Engine->xDisplay,
									event->xselectionrequest.requestor,
									event->xselectionrequest.property,
									x11Engine->atomCompoundText,
									prop.format, PropModeReplace, prop.value, prop.nitems);
							respond.xselection.property = event->xselectionrequest.property;
						}
						if(wStr) free(wStr);
						if(prop.value) XFree(prop.value);
					}
					else if(aStr.Length() > 0 && event->xselectionrequest.target == XA_STRING)
					{
						XChangeProperty(x11Engine->xDisplay,
								event->xselectionrequest.requestor,
								event->xselectionrequest.property,
								XA_STRING,
								8,
								PropModeReplace,
								(const unsigned char*)aStr.String(), aStr.Length());
						respond.xselection.property = event->xselectionrequest.property;
					}
				}
				respond.xselection.type = SelectionNotify;
				respond.xselection.display = x11Engine->xDisplay;
				respond.xselection.requestor = event->xselectionrequest.requestor;
				respond.xselection.selection = event->xselectionrequest.selection;
				respond.xselection.target = event->xselectionrequest.target;
				respond.xselection.time = event->xselectionrequest.time;
				XSendEvent(x11Engine->xDisplay, event->xselectionrequest.requestor, 0, 0, &respond);
				XFlush(x11Engine->xDisplay);
				x11Engine->Unlock();
			}
			break;

		case SelectionNotify:
			{
				// TODO: pixmap, etc...
				if(event->xselection.selection != XA_PRIMARY || event->xselection.property == None) break;

				char *aStr = NULL;

				x11Engine->Lock();

				Atom type = None;
				int format;
				unsigned long nitems;
				unsigned long bytes_after;
				unsigned char *prop = NULL;

				if(!(XGetWindowProperty(x11Engine->xDisplay,
						        x11Engine->xProtocolsWindow, x11Engine->atomClipboard,
						        0, 1,
						        False, AnyPropertyType,
						        &type, &format, &nitems,
						        &bytes_after, &prop) != Success || prop == NULL))
				{
					long len = 1;

					if(bytes_after > 0)
					{
						XFree(prop);
						prop = NULL;

						len = (long)bytes_after + 1;
						if(XGetWindowProperty(x11Engine->xDisplay,
								      x11Engine->xProtocolsWindow, x11Engine->atomClipboard,
								      0, len,
								      False, AnyPropertyType,
								      &type, &format, &nitems,
								      &bytes_after, &prop) != Success || prop == NULL) len = 0;
					}
					else if(*prop == 0)
					{
						len = 0;
					}

					if(len > 0)
					{
						if(event->xselection.target == x11Engine->atomCompoundText)
						{
							XTextProperty tProp;
							tProp.value = prop;
							tProp.format = format;
							tProp.nitems = nitems;
							tProp.encoding = x11Engine->atomCompoundText;

							wchar_t **wStrList = NULL;
							int wCount = 0;

							if(XwcTextPropertyToTextList(x11Engine->xDisplay,
										     &tProp, &wStrList, &wCount) == Success)
							{
								BString tmpStr;
								const wchar_t **tmp = (const wchar_t **)wStrList;
								for(int i = 0; i < wCount && *tmp != NULL; i++, tmp++)
								{
									char *wStr = bhapi::utf32_convert_to_utf8((const unichar32*)(*tmp), -1);
									if(wStr == NULL) continue;
									tmpStr.Append(wStr);
									free(wStr);
								}
								if(tmpStr.Length() > 0) aStr = bhapi::strdup(tmpStr.String());
								XwcFreStringList(wStrList);
							}
						}
						else if(event->xselection.target == XA_STRING)
						{
							aStr = bhapi::strndup((const char*)prop, (__be_int32)len);
						}
					}
				}

				if(prop != NULL) XFree(prop);

				x11Engine->Unlock();

				if(aStr != NULL)
				{
					b_x11_clipboard_changed(aStr);
					free(aStr);
				}
			}
			break;

		case ConfigureNotify:
			{
				Window xid_owner = event->xconfigure.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				BRect rect;

				Window child;
				int x = 0, y = 0;

				x11Engine->Lock();
				if(XTranslateCoordinates(x11Engine->xDisplay, xid_owner, x11Engine->xRootWindow, 0, 0, &x, &y, &child))
				{
					rect.left = (float)x;
					rect.top = (float)y;
				}
				else
				{
					rect.left = (float)event->xconfigure.x;
					rect.top = (float)event->xconfigure.y;
				}
				x11Engine->Unlock();

				rect.right = rect.left + (float)event->xconfigure.width - 1.f;
				rect.bottom = rect.top + (float)event->xconfigure.height - 1.f;

				message.AddPoint("where", rect.LeftTop());
				message.AddFloat("width", rect.Width());
				message.AddFloat("height", rect.Height());

				message.what = B_WINDOW_RESIZED;
				etkWinMsgr.SendMessage(&message);
				message.what = B_WINDOW_MOVED;
				etkWinMsgr.SendMessage(&message);
			}
			break;

		case Expose:
			{
				Window xid_owner = event->xexpose.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				BRect rect;

				rect.left = (float)event->xexpose.x;
				rect.top = (float)event->xexpose.y;
				rect.right = (float)(event->xexpose.x + event->xexpose.width) - 1.f;
				rect.bottom = (float)(event->xexpose.y + event->xexpose.height) - 1.f;

				message.what = _UPDATE_;
				message.AddRect("BHAPI:frame", rect);

				etkWinMsgr.SendMessage(&message);
			}
			break;

		case PropertyNotify:
			{
				if(event->xproperty.atom != x11Engine->atomWMDesktop) break;

				Window xid_owner = event->xproperty.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				__be_uint32 winWorkspace = 0;
				Atom type = None;
				int format;
				unsigned long nitems;
				unsigned long bytes_after;

				x11Engine->Lock();
				unsigned char *prop = NULL;
				if(XGetWindowProperty(x11Engine->xDisplay, xid_owner, x11Engine->atomWMDesktop, 0, 1,
						      False, AnyPropertyType,
						      &type, &format, &nitems,
						      &bytes_after, &prop) == Success)
				{
					if(type != None && format == 32 && nitems == 1 && prop != NULL)
					{
						long desktop = *((long*)prop);
						winWorkspace = ((unsigned long)desktop == 0xFFFFFFFF ? B_ALL_WORKSPACES : desktop + 1);
					}
				}
				if(prop != NULL) XFree(prop);
				x11Engine->Unlock();

				if(winWorkspace != 0)
				{
					message.what = B_WORKSPACES_CHANGED;
					message.AddInt32("new", (__be_int32)winWorkspace);
					etkWinMsgr.SendMessage(&message);
				}
			}
			break;

		case MapNotify:
			{
				Window xid_owner = event->xmap.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				__be_uint8 winState = 0;
				Atom type = None;
				int format;
				unsigned long nitems;
				unsigned long bytes_after;
				XWindowAttributes xattr;

				x11Engine->Lock();
				unsigned char *prop = NULL;
				XGetWindowProperty(x11Engine->xDisplay, xid_owner,
						   XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_WINDOW_STATE", False), 0, 1,
						   False, XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_BOOL", False),
						   &type, &format, &nitems,
						   &bytes_after, &prop);
				XGetWindowAttributes(x11Engine->xDisplay, xid_owner, &xattr);
				if(!prop)
				{
					x11Engine->Unlock();
					break;
				}
				winState = (__be_uint8)*prop;
				XFree(prop);
				x11Engine->Unlock();

				switch(winState)
				{
					case 0: // hidden
						{
							x11Engine->Lock();
							XUnmapWindow(x11Engine->xDisplay, xid_owner); // remain hidden, should not happen.
							x11Engine->Unlock();
						}
						break;

					case 1: // shown
						{
							message.what = _UPDATE_;
							message.AddRect("BHAPI:frame",
									BRect(0, 0, (float)xattr.width - 1.f, (float)xattr.height - 1.f));
							etkWinMsgr.SendMessage(&message);
						}
						break;

					case 2: // iconed
						{
							x11Engine->Lock();
							winState = 1; // shown
							XChangeProperty(x11Engine->xDisplay, xid_owner,
									XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_WINDOW_STATE", False),
									XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_BOOL", False), 8,
									PropModeReplace, (const unsigned char*)&winState, 1);
							x11Engine->Unlock();

							message.what = B_MINIMIZED;
							message.AddBool("minimize", false);
							etkWinMsgr.SendMessage(&message);

							message.what = _UPDATE_;
							message.AddRect("BHAPI:frame",
									BRect(0, 0, (float)xattr.width - 1.f, (float)xattr.height - 1.f));
							etkWinMsgr.SendMessage(&message);
						}
						break;

					default: // reserved
						break;
				}
			}
			break;

		case UnmapNotify:
			{
				Window xid_owner = event->xunmap.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				__be_uint8 winState = 0;
				Atom type = None;
				int format;
				unsigned long nitems;
				unsigned long bytes_after;

				x11Engine->Lock();
				unsigned char *prop = NULL;
				XGetWindowProperty(x11Engine->xDisplay, xid_owner,
						   XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_WINDOW_STATE", False), 0, 1,
						   False, XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_BOOL", False),
						   &type, &format, &nitems,
						   &bytes_after, &prop);
				if(!prop)
				{
					x11Engine->Unlock();
					break;
				}
				winState = (__be_uint8)*prop;
				XFree(prop);
				x11Engine->Unlock();

				if(winState == 1) // if shown, it must be WM iconed or change workspace
				{
					x11Engine->Lock();
					winState = 2; // iconed
					XChangeProperty(x11Engine->xDisplay, xid_owner,
							XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_WINDOW_STATE", False),
							XInternAtom(x11Engine->xDisplay, "ATOMBHAPI_BOOL", False), 8,
							PropModeReplace, (const unsigned char*)&winState, 1);
					x11Engine->Unlock();

					message.what = B_MINIMIZED;
					message.AddBool("minimize", true);
					etkWinMsgr.SendMessage(&message);
				}
			}
			break;

		case ButtonPress:
			{
				Window xid_owner = event->xbutton.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				message.what = B_MOUSE_DOWN;

				__be_int32 button = (__be_int32)(event->xbutton.button);
				if(button >= 4 && button <= 7)
				{
					message.what = B_MOUSE_WHEEL_CHANGED;
					float delta_x = 0;
					float delta_y = 0;
					if(button <= 5) delta_y = (button == 4 ? -1.0f : 1.0f);
					else delta_x = (button == 6 ? -1.0f : 1.0f);

					message.AddFloat("BHAPI:wheel_delta_x", delta_x);
					message.AddFloat("BHAPI:wheel_delta_y", delta_y);
				}
				else
				{
					__be_int32 buttons = button;
					__be_int32 clicks = 1;

					unsigned int state = event->xbutton.state;
					if((state & Button1Mask) && button != 1) buttons += 1;
					if((state & Button2Mask) && button != 2) buttons += 2;
					if((state & Button3Mask) && button != 3) buttons += 3;

					x11Engine->Lock();
					if(event->xbutton.serial != x11Engine->xPrevMouseDownSerial)
					{
						event->xbutton.serial = x11Engine->xPrevMouseDownSerial;
						if(x11Engine->xPrevMouseDownTime > event->xbutton.time ||
						   event->xbutton.time - x11Engine->xPrevMouseDownTime > CLICK_TIMEOUT)
							clicks = x11Engine->xPrevMouseDownCount = 1;
						else
							clicks = (x11Engine->xPrevMouseDownCount += 1);
						x11Engine->xPrevMouseDownTime = event->xbutton.time;
					}
					x11Engine->Unlock();

					message.AddInt32("button", button);
					message.AddInt32("buttons", buttons);
					message.AddInt32("clicks", clicks);

					message.AddPoint("where", BPoint((float)event->xbutton.x, (float)event->xbutton.y));
					message.AddPoint("screen_where", BPoint((float)event->xbutton.x_root, (float)event->xbutton.y_root));
				}

				// TODO: modifiers, clicks

				message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(bhapi::app);
				etkWinMsgr.SendMessage(&message);
			}
			break;

		case ButtonRelease:
			{
				Window xid_owner = event->xbutton.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				__be_int32 button = (__be_int32)(event->xbutton.button);
				if(button >= 4 && button <= 7) break;

				message.what = B_MOUSE_UP;

				message.AddPoint("where", BPoint((float)event->xbutton.x, (float)event->xbutton.y));
				message.AddPoint("screen_where", BPoint((float)event->xbutton.x_root, (float)event->xbutton.y_root));

				__be_int32 buttons = 0;
				unsigned int state = event->xbutton.state;
				if((state & Button1Mask) && button != 1) buttons += 1;
				if((state & Button2Mask) && button != 2) buttons += 2;
				if((state & Button3Mask) && button != 3) buttons += 3;
				message.AddInt32("button", button);
				message.AddInt32("buttons", buttons);

				// TODO: modifiers

				message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(bhapi::app);
				etkWinMsgr.SendMessage(&message);
			}
			break;

		case MotionNotify:
			{
				Window xid_owner = event->xmotion.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				message.what = B_MOUSE_MOVED;

				message.AddPoint("where", BPoint((float)event->xmotion.x, (float)event->xmotion.y));
				message.AddPoint("screen_where", BPoint((float)event->xmotion.x_root, (float)event->xmotion.y_root));

				__be_int32 buttons = 0;
				if(event->xmotion.state & Button1Mask) buttons += 1;
				if(event->xmotion.state & Button2Mask) buttons += 2;
				if(event->xmotion.state & Button3Mask) buttons += 3;
				message.AddInt32("buttons", buttons);

				message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(bhapi::app);
				etkWinMsgr.SendMessage(&message);
			}
			break;

		case EnterNotify:
		case LeaveNotify:
			{
				handled = true;

				Window xid_owner = event->xcrossing.window;

				x11Engine->Lock();
				if(event->type == EnterNotify)
					XDefineCursor(x11Engine->xDisplay, xid_owner, x11Engine->xCursor);
				else
					XUndefineCursor(x11Engine->xDisplay, xid_owner);
				x11Engine->Unlock();
			}
			break;

		case FocusIn:
		case FocusOut:
			{
				Window xid_owner = event->xfocus.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				message.what = B_WINDOW_ACTIVATED;
				etkWinMsgr.SendMessage(&message);

				x11Engine->Lock();
				if(x11Engine->xInputContext != NULL && event->type == FocusIn)
				{
					XSetICValues(x11Engine->xInputContext, XNFocusWindow, event->xfocus.window, NULL);
				}
				x11Engine->Unlock();
			}
			break;

		case KeyPress:
		case KeyRelease:
			{
				Window xid_owner = event->xkey.window;
				if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
				handled = true;
				if(etkWinMsgr.IsValid() == false) break;

				message.what = (event->type == KeyPress ? B_KEY_DOWN : B_KEY_UP);

				message.AddInt32("key", (__be_int32)(event->xkey.keycode));

				// TODO: BHAPI:key_repeat, modifiers, states, raw_char

				XComposeStatus status;
				KeySym keysym;
				char keybuffer[17];
				char *utf8_keybuffer = NULL;
				int keynum;

				bzero(keybuffer, sizeof(keybuffer));

				__be_int32 modifiers = 0;

				x11Engine->Lock();
				keynum = XLookupString(&event->xkey, keybuffer, 16, &keysym, &status);
				if(event->xkey.state & (ControlMask | Mod1Mask))
				{
					char *st_buf = XKeysymToString(keysym);
					if(!(st_buf == NULL || *st_buf == 0 || st_buf[1] != 0))
					{
						keybuffer[0] = *st_buf;
						keybuffer[1] = '\0';
						keynum = 1;
					}
				}
#if 0
				if(event->type == KeyRelease)
					BHAPI_DEBUG("event->xkey.state & ShiftMask --- %s",
						  (event->xkey.state & ShiftMask) ? "TRUE" : "FALSE");
#endif
				if((keysym >= XK_KP_F1 && keysym <= XK_KP_F4) || (keysym >= XK_F1 && keysym <= XK_F12) ||
				    keysym == XK_Pause || keysym == XK_Scroll_Lock || keysym == XK_Print)
				{
					if(keysym >= XK_KP_F1 && keysym <= XK_KP_F4)
						message.ReplaceInt32("key", keysym - XK_KP_F1 + B_F1_KEY);
					else if(keysym >= XK_F1 && keysym <= XK_F12)
						message.ReplaceInt32("key", keysym - XK_F1 + B_F1_KEY);
					else if(keysym == XK_Pause)
						message.ReplaceInt32("key", B_PAUSE_KEY);
					else if(keysym == XK_Scroll_Lock)
						message.ReplaceInt32("key", B_SCROLL_KEY);
					else if(keysym == XK_Print)
						message.ReplaceInt32("key", B_PRINT_KEY);
					modifiers |= B_FUNCTIONS_KEY;
					keybuffer[0] = B_FUNCTION_KEY;
					keybuffer[1] = 0;
					keynum = 1;
				}

				if(event->xkey.state & ShiftMask) modifiers |= B_SHIFT_KEY;
				if(event->xkey.state & ControlMask) modifiers |= B_CONTROL_KEY;
				if(event->xkey.state & Mod1Mask) modifiers |= B_COMMAND_KEY;
				if(event->type == KeyRelease) // WHY!!!: when XK_Shift_* released, but still ShiftMask
				{
					if(keysym == XK_Shift_L || keysym == XK_Shift_R) modifiers &= ~B_SHIFT_KEY;
					if(keysym == XK_Control_L || keysym == XK_Control_R) modifiers &= ~B_CONTROL_KEY;
					if(keysym == XK_Alt_L || keysym == XK_Alt_R) modifiers &= ~B_COMMAND_KEY;
				}
				message.AddInt32("modifiers", modifiers);

				event->xkey.state &= ~(ControlMask | Mod1Mask);

				while(x11Engine->xInputContext != NULL && event->type == KeyPress)
				{
					wchar_t *im_buf = (wchar_t*)malloc(sizeof(wchar_t) * 20);
					Status im_status = XLookupNone;

					if(im_buf == NULL) break;
					bzero(im_buf, sizeof(wchar_t) * 20);

					int len = XwcLookupString(x11Engine->xInputContext, &(event->xkey), im_buf, 20, NULL, &im_status);
					if(im_status == XBufferOverflow && len > 0)
					{
						BHAPI_WARNING("[GRAPHICS]: XIM --- im_status == XBufferOverflow, len: %d", len);
						wchar_t *newBuf = (wchar_t*)realloc(im_buf, sizeof(wchar_t) * (size_t)(len + 1));
						if(newBuf != NULL)
						{
							im_buf = newBuf;
							bzero(im_buf, sizeof(wchar_t) * (size_t)(len + 1));
							len = min_c(XwcLookupString(x11Engine->xInputContext, &(event->xkey),
										    im_buf, len, NULL, &im_status), len);
						}
						else
						{
							len = 0;
						}
					}
					else if(len > 20)
					{
						BHAPI_WARNING("[GRAPHICS]: XIM --- \"XwcLookupString\" has BUG");
						len = 0;
					}
					if(len > 0 && (len != 1 || *im_buf != 0x0000000d))
						utf8_keybuffer = bhapi::utf32_convert_to_utf8((const unichar32*)im_buf, len);
					free(im_buf);
					break;
				}
				x11Engine->Unlock();

				if(keysym == XK_KP_Enter) keysym = XK_Return;
				else if(keysym == XK_KP_Insert) keysym = XK_Insert;
				else if(keysym == XK_KP_Delete) keysym = XK_Delete;
				else if(keysym == XK_KP_Home) keysym = XK_Home;
				else if(keysym == XK_KP_End) keysym = XK_End;
				else if(keysym == XK_KP_Page_Up) keysym = XK_Page_Up;
				else if(keysym == XK_KP_Page_Down) keysym = XK_Page_Down;
				else if(keysym == XK_KP_Left) keysym = XK_Left;
				else if(keysym == XK_KP_Right) keysym = XK_Right;
				else if(keysym == XK_KP_Up) keysym = XK_Up;
				else if(keysym == XK_KP_Down) keysym = XK_Down;

				if(keynum > 0 && keybuffer[keynum - 1] == '\0') keynum--;
				else if(keynum > 0) keybuffer[keynum] = '\0';
				else bzero(keybuffer, sizeof(keybuffer));

				if(keysym == XK_BackSpace)
				{
					keybuffer[0] = B_BACKSPACE;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Return)
				{
					keybuffer[0] = B_ENTER;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Tab)
				{
					keybuffer[0] = B_TAB;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Left)
				{
					keybuffer[0] = B_LEFT_ARROW;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Right)
				{
					keybuffer[0] = B_RIGHT_ARROW;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Up)
				{
					keybuffer[0] = B_UP_ARROW;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Down)
				{
					keybuffer[0] = B_DOWN_ARROW;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Home)
				{
					keybuffer[0] = B_HOME;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_End)
				{
					keybuffer[0] = B_END;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Insert)
				{
					keybuffer[0] = B_INSERT;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Delete)
				{
					keybuffer[0] = B_DELETE;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Page_Up)
				{
					keybuffer[0] = B_PAGE_UP;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Page_Down)
				{
					keybuffer[0] = B_PAGE_DOWN;
					keybuffer[1] = '\0';
					keynum = 1;
				}
				else if(keysym == XK_Escape)
				{
					keybuffer[0] = B_ESCAPE;
					keybuffer[1] = '\0';
					keynum = 1;
				}

				if(keynum > 0)
				{
					if(keynum == 1) message.AddInt8("byte", (__be_int8)keybuffer[0]);
					if(utf8_keybuffer == NULL) message.AddString("bytes", keybuffer);
				}
				if(utf8_keybuffer != NULL)
				{
					message.AddString("bytes", utf8_keybuffer);
					free(utf8_keybuffer);
				}

				message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
				etkWinMsgr = BMessenger(bhapi::app);
				etkWinMsgr.SendMessage(&message);

				bool dealed = true;
				switch(keysym)
				{
					case XK_Shift_L:
						if(event->type == KeyPress)
							modifiers &= ~(B_LEFT_SHIFT_KEY | B_SHIFT_KEY);
						else
							modifiers |= (B_LEFT_SHIFT_KEY | B_SHIFT_KEY);
						break;
					case XK_Shift_R:
						if(event->type == KeyPress)
							modifiers &= ~(B_RIGHT_SHIFT_KEY | B_SHIFT_KEY);
						else
							modifiers |= (B_RIGHT_SHIFT_KEY | B_SHIFT_KEY);
						break;
					case XK_Control_L:
						if(event->type == KeyPress)
							modifiers &= ~(B_LEFT_SHIFT_KEY | B_CONTROL_KEY);
						else
							modifiers |= (B_LEFT_SHIFT_KEY | B_CONTROL_KEY);
						break;
					case XK_Control_R:
						if(event->type == KeyPress)
							modifiers &= ~(B_RIGHT_SHIFT_KEY | B_CONTROL_KEY);
						else
							modifiers |= (B_RIGHT_SHIFT_KEY | B_CONTROL_KEY);
						break;
					case XK_Alt_L:
						if(event->type == KeyPress)
							modifiers &= ~(B_LEFT_SHIFT_KEY | B_COMMAND_KEY);
						else
							modifiers |= (B_LEFT_SHIFT_KEY | B_COMMAND_KEY);
						break;
					case XK_Alt_R:
						if(event->type == KeyPress)
							modifiers &= ~(B_RIGHT_SHIFT_KEY | B_COMMAND_KEY);
						else
							modifiers |= (B_RIGHT_SHIFT_KEY | B_COMMAND_KEY);
						break;
					default:
						dealed = false;
						break;
				}

				if(dealed)
				{
					if((modifiers & B_LEFT_SHIFT_KEY) || (modifiers & B_RIGHT_SHIFT_KEY)) modifiers |= B_SHIFT_KEY;
					if((modifiers & B_LEFT_CONTROL_KEY) || (modifiers & B_RIGHT_CONTROL_KEY)) modifiers |= B_CONTROL_KEY;
					if((modifiers & B_LEFT_COMMAND_KEY) || (modifiers & B_RIGHT_COMMAND_KEY)) modifiers |= B_COMMAND_KEY;
					if((modifiers & B_LEFT_OPTION_KEY) || (modifiers & B_RIGHT_OPTION_KEY)) modifiers |= B_OPTION_KEY;
					message.what = B_MODIFIERS_CHANGED;
					message.RemoveInt32("key");
					message.RemoveInt32("BHAPI:key_repeat");
					message.RemoveInt8("byte");
					message.RemovString("bytes");
					message.RemoveInt32("raw_char");
					message.AddInt32("BHAPI:old_modifiers", modifiers);
					etkWinMsgr.SendMessage(&message);
				}
			}
			break;

		case ClientMessage:
			{
				if((Atom)event->xclient.data.l[0] == x11Engine->atomWMDeleteWindow)
				{
					Window xid_owner = event->xclient.window;
					if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
					handled = true;
					if(etkWinMsgr.IsValid() == false) break;

					message.what = B_QUIT_REQUESTED;
					etkWinMsgr.SendMessage(&message);
				}
				else if((Atom)event->xclient.data.l[0] == x11Engine->atomDeleteWindow)
				{
					handled = true;
					Window xid_owner = event->xclient.window;
					x11Engine->Lock();
					XDestroyWindow(x11Engine->xDisplay, xid_owner);
					XFlush(x11Engine->xDisplay);
					x11Engine->Unlock();
				}
				else if((Atom)event->xclient.data.l[0] == x11Engine->atomWMFocus)
				{
					Window xid_owner = event->xclient.window;
					if(x11Engine->GetContactor(xid_owner, &etkWinMsgr) == false) break;
					handled = true;
				}
			}
			break;

		default:
			break;
	}
}


static status_t b_x11_task(void *arg)
{
	EXGraphicsEngine *x11Engine = (EXGraphicsEngine*)arg;

	XEvent event;

	BHAPI_DEBUG("[GRAPHICS]: Enter X11 task...");

	x11Engine->Lock();

	while(!(x11Engine->xDoQuit || x11Engine->xDisplay == NULL))
	{
		if(x11Engine->xSupportThreads)
		{
			x11Engine->Unlock();
			XNextEvent(x11Engine->xDisplay, &event);
			if(x11Engine->xInputMethod == NULL)
			{
				b_process_x_event(x11Engine, &event);
			}
			else
			{
				x11Engine->Lock();
				XUnlockDisplay(x11Engine->xDisplay);

				Bool status = XFilterEvent(&event, None);

				XLockDisplay(x11Engine->xDisplay);
				x11Engine->Unlock();
				if(status != True) b_process_x_event(x11Engine, &event);
			}
			x11Engine->Lock();
		}
		else
		{
			if(XEventsQueued(x11Engine->xDisplay, QueuedAfterFlush) <= 0)
			{
				x11Engine->Unlock();
				fd_set rset;
				FD_ZERO(&rset);
				FD_SET(x11Engine->xSocket, &rset);
#if defined(HAVE_X11_XPOLL_H) && defined(Select)
				Select(x11Engine->xSocket + 1, &rset, NULL, NULL, NULL);
#else
				select(x11Engine->xSocket + 1, &rset, NULL, NULL, NULL);
#endif
				x11Engine->Lock();
				continue;
			}

			XNextEvent(x11Engine->xDisplay, &event);

			if(x11Engine->xInputMethod == NULL || XFilterEvent(&event, None) != True)
			{
				x11Engine->Unlock();
				b_process_x_event(x11Engine, &event);
				x11Engine->Lock();
			}
		}
	}

	if(x11Engine->xDisplay != NULL)
	{
		if(x11Engine->xInputContext != NULL) {XDestroyIC(x11Engine->xInputContext); x11Engine->xInputContext = NULL;}
		if(x11Engine->xInputMethod != NULL) {XCloseIM(x11Engine->xInputMethod); x11Engine->xInputMethod = NULL;}
		XDestroyWindow(x11Engine->xDisplay, x11Engine->xProtocolsWindow);
		if(x11Engine->xCursor != None) {XFreeCursor(x11Engine->xDisplay, x11Engine->xCursor); x11Engine->xCursor = None;}
		XCloseDisplay(x11Engine->xDisplay);
		x11Engine->xDisplay = NULL;
	}

	x11Engine->Unlock();

	BHAPI_DEBUG("[GRAPHICS]: X11 task quited.");

	return B_OK;
}


status_t
EXGraphicsEngine::Initalize()
{
	BMessageFilter *clipboardFilter = new EX11ClipboardMessageFilter(this);
	bhapi::app->Lock();
	bhapi::app->AddFilter(clipboardFilter);
	bhapi::app->Unlock();

	Lock();
	if(fX11Thread != NULL)
	{
		Unlock();

		bhapi::app->Lock();
		bhapi::app->RemoveFilter(clipboardFilter);
		bhapi::app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	setlocale(LC_ALL, "");

	// if XFilterEvent block, comment the line below.
	// XFilterEvent deadlock patch : http://people.freedesktop.org/~alanc/thread-fixes/4041914.txt
	xSupportThreads = (XInitThreads() != 0);

	xDisplay = XOpenDisplay(NULL);
	if(xDisplay == NULL)
	{
		Unlock();
		BHAPI_WARNING("[GRAPHICS]: %s --- Unable to open X11 display!", __PRETTY_FUNCTION__);

		bhapi::app->Lock();
		bhapi::app->RemoveFilter(clipboardFilter);
		bhapi::app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	Lock();

	XSetIOErrorHandler(b_x_io_error_handler);
	XSetErrorHandler(b_x_error_handler);
	xSocket = ConnectionNumber(xDisplay);
	xScreen = DefaultScreen(xDisplay);
	xRootWindow = RootWindow(xDisplay, xScreen);
	xDisplayWidth = DisplayWidth(xDisplay, xScreen);
	xDisplayHeight = DisplayHeight(xDisplay, xScreen);
	xVisual = DefaultVisual(xDisplay, xScreen);
	xDepth = DefaultDepth(xDisplay, xScreen);
	xColormap = DefaultColormap(xDisplay, xScreen);
	xProtocolsWindow = XCreateWindow(xDisplay, xRootWindow,
					 0, 0, 10, 10, 0, CopyFromParent,
					 InputOnly, NULL, 0, NULL);

	atomProtocols = XInternAtom(xDisplay, "ATOMBHAPI_PROTOCOLS", False);
	atomDeleteWindow = XInternAtom(xDisplay, "ATOMBHAPI_DELETE_WINDOW", False);
	atomWMDeleteWindow = XInternAtom(xDisplay, "WM_DELETE_WINDOW", False);
	atomWMFocus = XInternAtom(xDisplay, "WM_TAKE_FOCUS", False);
	atomWMDesktop =  XInternAtom(xDisplay, "_NET_WM_DESKTOP", False);
	atomCurrentDesktop = XInternAtom(xDisplay, "_NET_CURRENT_DESKTOP", False);
	atomEventPending = XInternAtom(xDisplay, "ATOMBHAPI_EVENT_PENDING", False);
	atomClipboard = XInternAtom(xDisplay, "ATOMBHAPI_CLIPBOARD", False);
	atomCompoundText = XInternAtom(xDisplay, "COMPOUND_TEXT", False);

	// TODO: atom for workspace, iconified etc.

	xBlackPixel = BlackPixel(xDisplay, xScreen);
	xWhitePixel = WhitePixel(xDisplay, xScreen);

#ifdef BHAPI_OS_LINUX
	if(getenv("BHAPI_USE_XIM") == NULL)
	{
		BHAPI_OUTPUT("[GRAPHICS]: X11 maybe has a bug when \"XFilterEvent\" processing as \"XInitThreads\" done.\n\tYou could find out how to fix it\n\tby visiting \"http://people.freedesktop.org/~alanc/thread-fixes/4041914.txt\".\n\tThen set the environment \"BHAPI_USE_XIM\" when you really want XIM.\n");
	}
	else
	{
#endif
	if(!(!XSupportsLocale() || XSetLocaleModifiers("") == NULL))
	{
		if((xInputMethod = XOpenIM(xDisplay, NULL, NULL, NULL)) != NULL)
		{
			xInputContext = XCreateIC(xInputMethod,
						  XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
						  XNClientWindow, xProtocolsWindow,
						  NULL);

			if(xInputContext == NULL)
			{
				BHAPI_WARNING("[GRAPHICS]: %s --- \"XIMPreeditNothing|XIMStatusNothing\" unsupported.", __PRETTY_FUNCTION__);
				XCloseIM(xInputMethod);
				xInputMethod = NULL;
			}
			else
			{
				XGetICValues(xInputContext, XNFilterEvents, &xInputMethodEventMask, NULL);
				XSelectInput(xDisplay, xProtocolsWindow, KeyPressMask | xInputMethodEventMask);
				XSetICFocus(xInputContext);
			}
		}
	}
	if(xInputMethod == NULL) BHAPI_WARNING("[GRAPHICS]: %s --- Unable to initalize XIM.", __PRETTY_FUNCTION__);
#ifdef BHAPI_OS_LINUX
	}
#endif

	xCursor = None;
	xDoQuit = false;

	if((fX11Thread = bhapi::create_thread(b_x11_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
       bhapi::resume_thread(fX11Thread) != B_OK)
	{
		if(fX11Thread)
		{
			bhapi::delete_thread(fX11Thread);
			fX11Thread = NULL;
		}

		if(xInputContext != NULL) {XDestroyIC(xInputContext); xInputContext = NULL;}
		if(xInputMethod != NULL) {XCloseIM(xInputMethod); xInputMethod = NULL;}
		XDestroyWindow(xDisplay, xProtocolsWindow);
		XCloseDisplay(xDisplay);
		xDisplay = NULL;

		Unlock();
		Unlock();

		BHAPI_WARNING("[GRAPHICS]: %s --- Unable to spawn graphics thread!", __PRETTY_FUNCTION__);

		bhapi::app->Lock();
		bhapi::app->RemoveFilter(clipboardFilter);
		bhapi::app->Unlock();
		delete clipboardFilter;
		return B_ERROR;
	}

	fClipboardFilter = clipboardFilter;
	XConvertSelection(xDisplay, XA_PRIMARY, atomCompoundText, atomClipboard, xProtocolsWindow, CurrentTime);

	Unlock();
	Unlock();

	return B_OK;
}


void
EXGraphicsEngine::Cancel()
{
	BMessageFilter *clipboardFilter = NULL;

	Lock();

	if(fX11Thread != NULL)
	{
		void *x11Thread = bhapi::open_thread(bhapi::get_thread_id(fX11Thread));
		if(x11Thread == NULL)
		{
			Unlock();
			return;
		}

		xDoQuit = true;

		if(xDisplay != NULL)
		{
			XClientMessageEvent xevent;

			xevent.type = ClientMessage;
			xevent.window = xProtocolsWindow;
			xevent.message_type = atomProtocols;
			xevent.format = 32;
			xevent.data.l[0] = (long)atomEventPending;
			xevent.data.l[1] = CurrentTime;

			XSendEvent(xDisplay, xProtocolsWindow, False, 0, (XEvent *)&xevent);
			XFlush(xDisplay);
		}

		Unlock();

		status_t status;
        bhapi::wait_for_thread(x11Thread, &status);

		Lock();

		if(fX11Thread != NULL && bhapi::get_thread_id(fX11Thread) == bhapi::get_thread_id(x11Thread))
		{
			bhapi::delete_thread(fX11Thread);
			fX11Thread = NULL;
		}

		bhapi::delete_thread(x11Thread);

		clipboardFilter = fClipboardFilter;
		fClipboardFilter = NULL;
	}

	Unlock();

	if(clipboardFilter != NULL)
	{
		bhapi::app->Lock();
		bhapi::app->RemoveFilter(clipboardFilter);
		bhapi::app->Unlock();
		delete clipboardFilter;
	}
}


BGraphicsContext*
EXGraphicsEngine::CreateContext()
{
	return(new EXGraphicsContext(this));
}


BGraphicsDrawable*
EXGraphicsEngine::CreatePixmap(__be_uint32 w,  __be_uint32 h)
{
	return(new EXGraphicsDrawable(this, w, h));
}


BGraphicsWindow*
EXGraphicsEngine::CreateWindow(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
{
	return(new EXGraphicsWindow(this, x, y, w, h));
}


status_t
EXGraphicsEngine::GetDesktopBounds(__be_uint32 *w,  __be_uint32 *h)
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if(w) *w = xDisplayWidth;
	if(h) *h = xDisplayHeight;

	return B_OK;
}


status_t
EXGraphicsEngine::GetCurrentWorkspace(__be_uint32 *workspace)
{
	if(workspace == NULL) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	*workspace = 0;

	unsigned char *prop = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	if(XGetWindowProperty(xDisplay, xRootWindow, atomCurrentDesktop, 0, 1,
			      False, AnyPropertyType,
			      &type, &format, &nitems,
			      &bytes_after, &prop) != Success ||
	   type == None) return B_OK;

	if(format == 32 && nitems == 1 && prop != NULL)
	{
		long desktop = *((long*)prop);
		if((unsigned long)desktop != 0xFFFFFFFF) *workspace = desktop + 1;
	}
	if(prop != NULL) XFree(prop);

	return B_OK;
}


status_t
EXGraphicsEngine::SetCursor(const void *cursor_data)
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	Cursor newCursor = None;

	if(cursor_data)
	{
		BCursor cursor(cursor_data);
		if(cursor.ColorDepth() != 1) return B_ERROR;

		const  __be_uint8 *bits = (const  __be_uint8*)cursor.Bits();
		const  __be_uint8 *mask = (const  __be_uint8*)cursor.Mask();

		__be_uint8 *source_data = (__be_uint8*)malloc((size_t)(mask - bits));
		__be_uint8 *mask_data = (__be_uint8*)malloc((size_t)(mask - bits));

		if(source_data != NULL && mask_data != NULL)
		{
			__be_uint8 *s = source_data;
			__be_uint8 *m = mask_data;
			const  __be_uint8 *tmp = mask;
			for(; bits != tmp; bits++, mask++, s++, m++)
			{
				// convert left to right
				*s = *m = 0;
				for(__be_uint8 i = 0; i < 8; i++)
				{
					*s |= ((*bits >> i) & 0x01) << (7 - i);
					*m |= ((*mask >> i) & 0x01) << (7 - i);
				}
			}

			Pixmap source_pix = XCreateBitmapFromData(xDisplay, xRootWindow,
								  (const char*)source_data, cursor.Width(), cursor.Height());
			Pixmap mask_pix = XCreateBitmapFromData(xDisplay, xRootWindow,
								(const char*)mask_data, cursor.Width(), cursor.Height());

			XColor fg_color;
			XColor bg_color;
			fg_color.pixel = xBlackPixel; fg_color.red = fg_color.green = fg_color.blue = 0;
			bg_color.pixel = xWhitePixel; bg_color.red = bg_color.green = bg_color.blue = 65535;

			newCursor = XCreatePixmapCursor(xDisplay, source_pix, mask_pix,
							&fg_color, &bg_color,
							cursor.SpotX(), cursor.SpotY());

			XFreePixmap(xDisplay, source_pix);
			XFreePixmap(xDisplay, mask_pix);
		}

		if(source_data) free(source_data);
		if(mask_data) free(mask_data);

		if(newCursor == None) return B_ERROR;

//		BHAPI_DEBUG("[GRAPHICS]: SetCursor");
	}
	else
	{
		__be_uint8 tmp = 0;

		Pixmap source_pix = XCreateBitmapFromData(xDisplay, xRootWindow, (const char*)&tmp, 1, 1);
		Pixmap mask_pix = XCreateBitmapFromData(xDisplay, xRootWindow, (const char*)&tmp, 1, 1);

		XColor fg_color;
		XColor bg_color;
		fg_color.pixel = xBlackPixel; fg_color.red = fg_color.green = fg_color.blue = 0;
		bg_color.pixel = xWhitePixel; bg_color.red = bg_color.green = bg_color.blue = 65535;

		newCursor = XCreatePixmapCursor(xDisplay, source_pix, mask_pix, &fg_color, &bg_color, 0, 0);

		XFreePixmap(xDisplay, source_pix);
		XFreePixmap(xDisplay, mask_pix);

//		BHAPI_DEBUG("[GRAPHICS]: HideCursor");
	}

	Window focusWin = None;
	int revert;

	XGetInputFocus(xDisplay, &focusWin, &revert);
	if(!(focusWin == None || focusWin == PointerRoot)) XDefineCursor(xDisplay, focusWin, newCursor);

	if(xCursor != None) XFreeCursor(xDisplay, xCursor);
	xCursor = newCursor;

	return B_OK;
}


status_t
EXGraphicsEngine::GetDefaultCursor(BCursor *cursor)
{
	return B_ERROR;
}


bool
EXGraphicsEngine::ConvertRegion(const BRegion *region, Region *xRegion)
{
	if(xRegion == NULL) return false;

	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return false;

	if((*xRegion = XCreateRegion()) == NULL) return false;

	if(region != NULL)
	{
		for(__be_int32 i = 0; i < region->CountRects(); i++)
		{
			BRect r = region->RectAt(i).FloorCopy();

			XRectangle xRect;
			xRect.x = (short)r.left;
			xRect.y = (short)r.top;
			xRect.width = (unsigned short)r.Width() + 1;
			xRect.height = (unsigned short)r.Height() + 1;

			XUnionRectWithRegion(&xRect, *xRegion, *xRegion);
		}
	}
	else
	{
		XRectangle xRect;
		xRect.x = 0;
		xRect.y = 0;
		xRect.width = B_MAXUSHORT;
		xRect.height = B_MAXUSHORT;

		XUnionRectWithRegion(&xRect, *xRegion, *xRegion);
	}

	return true;
}


bool
EXGraphicsEngine::ConvertRegion(const BRegion *region, XRectangle **xRects, int *nrects)
{
	if(xRects == NULL || nrects == NULL) return false;

	__be_int32 nrectsNeeded = max_c((region ? region->CountRects() : 0), 1);
	if((*xRects = (XRectangle*)malloc(sizeof(XRectangle) * (size_t)nrectsNeeded)) == NULL) return false;
	*nrects = 0;

	if(region != NULL)
	{
		for(__be_int32 i = 0; i < region->CountRects(); i++)
		{
			BRect r = region->RectAt(i).FloorCopy();

			(*xRects)[*nrects].x = (short)r.left;
			(*xRects)[*nrects].y = (short)r.top;
			(*xRects)[*nrects].width = (unsigned short)r.Width() + 1;
			(*xRects)[*nrects].height = (unsigned short)r.Height() + 1;

			*nrects += 1;
		}
	}

	if(*nrects == 0)
	{
		(*xRects)->x = 0;
		(*xRects)->y = 0;
		(*xRects)->width = (region ? 0 : B_MAXUSHORT);
		(*xRects)->height = (region ? 0 : B_MAXUSHORT);

		*nrects = 1;
	}

	return true;
}

#endif /* LINUX */

