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
 * File: etk-window.cpp
 *
 * --------------------------------------------------------------------------*/

#include <be/BeBuild.h>
#include <be/app/AppDefs.h>
#include <be/interface/View.h>
#include <be/interface/Region.h>

#include "etk-beos-graphics.h"

#include "./../support/ClassInfo.h>
#include "./../app/Application.h>

#define CLICK_TIMEOUT 200000


class EBePrivateWin : public BWindow
{
public:
	EBePrivateWin(eint32 x, eint32 y, eint32 w, eint32 h);
	virtual ~EBePrivateWin();

	virtual bool QuitRequested();

	virtual void FrameMoved(BPoint new_position);
	virtual void FrameResized(float new_width, float new_height);
	virtual void WorkspacesChanged(uint32 old_ws, uint32 new_ws);
	virtual void DispatchMessage(BMessage *bMsg, BHandler *handler);

	EMessenger fContactor;
	BView *fTopView;

private:
	bool doQuit;
	bigtime_t fPrevMouseDownTime;
	eint32 fPrevMouseDownCount;
};


class EBePrivateWinTopView : public BView
{
public:
	EBePrivateWinTopView(BRect frame, const char *name, uint32 resizeMask, uint32 flags);
	virtual ~EBePrivateWinTopView();
	virtual void Draw(BRect updateRect);
};


EBePrivateWinTopView::EBePrivateWinTopView(BRect frame, const char *name, uint32 resizeMask, uint32 flags)
	: BView(frame, name, resizeMask, flags)
{
}


EBePrivateWinTopView::~EBePrivateWinTopView()
{
}


void
EBePrivateWinTopView::Draw(BRect updateRect)
{
	EBePrivateWin *win = e_cast_as(Window(), EBePrivateWin);
	if(!(win == NULL || win->fContactor.IsValid() == false))
	{
		EMessage message(_UPDATE_);

		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", e_real_time_clock_usecs());

		ERect rect;
		rect.left = updateRect.left;
		rect.top = updateRect.top;
		rect.right = updateRect.right;
		rect.bottom = updateRect.bottom;

		message.AddRect("etk:frame", rect);
		win->fContactor.SendMessage(&message);
	}
}


EBePrivateWin::EBePrivateWin(eint32 x, eint32 y, eint32 w, eint32 h)
	: BWindow(BRect(0, 0, 10, 10), NULL, B_TITLED_WINDOW, 0),
	  fTopView(NULL), doQuit(false),
	  fPrevMouseDownTime(0), fPrevMouseDownCount(0)
{
	fTopView = new EBePrivateWinTopView(Bounds(), NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	fTopView->SetViewColor(255, 255, 255);
	AddChild(fTopView);

	MoveTo((float)x, (float)y);
	ResizeTo((float)w, (float)h);

	Lock();
	Run();
}


EBePrivateWin::~EBePrivateWin()
{
}


bool
EBePrivateWin::QuitRequested()
{
	if(doQuit) return true;

	EMessage message(E_QUIT_REQUESTED);
	fContactor.SendMessage(&message);
	return false;
}


static bool etk_beos_get_byte(int32 modifiers, int32 key_code, char *result)
{
	if(result == NULL || key_code < 0 || key_code >= 128) return false;
	
	key_map *keys = NULL;
	char *chars = NULL;

	get_key_map(&keys, &chars);
	if(keys == NULL || chars == NULL)
	{
		if(keys) free(keys);
		if(chars) free(chars);
		return false;
	}

	int32 offset;
	bool retVal = false;

	if((modifiers & B_SHIFT_KEY) && (modifiers & B_CAPS_LOCK))
			offset = keys->caps_shift_map[key_code];
	else if(modifiers & B_SHIFT_KEY)
		offset = keys->shift_map[key_code];
	else if(modifiers & B_CAPS_LOCK)
		offset = keys->caps_map[key_code];
	else
		offset = keys->normal_map[key_code];

	if(chars[offset] == 1)
	{
		*result = chars[offset + 1];
		retVal = true;
	}

	free(keys);
	free(chars);

	return retVal;
}


static void __etk_convert_region(const ERegion *region, BRegion *beRegion, BRect maxRect)
{
	if(beRegion == NULL) return;

	if(region != NULL)
	{
		for(eint32 i = 0; i < region->CountRects(); i++)
		{
			ERect r = region->RectAt(i);
			BRect bRect(r.left, r.top, r.right, r.bottom);
			beRegion->Include(bRect);
		}
	}
	else
	{
		beRegion->Set(maxRect);
	}
}


void
EBePrivateWin::DispatchMessage(BMessage *bMsg, BHandler *handler)
{
	bool handled = true;

	if(bMsg->what == 'etk_')
	{
		int32 what = 0;
		bMsg->FindInt32("etk:what", &what);

		switch(what)
		{
			case ETK_BEOS_QUIT:
				doQuit = true;
				PostMessage(B_QUIT_REQUESTED);
				break;

			case ETK_BEOS_CONTACT_TO:
				{
					fContactor = EMessenger();
					const char *buffer = NULL;
					ssize_t size = -1;
					if(bMsg->FindData("etk:messenger", B_ANY_TYPE, (const void**)&buffer, &size) != B_OK) break;
					if(buffer == NULL || size <= 0) break;
					fContactor.Unflatten(buffer, (size_t)size);
				}
				break;

			case ETK_BEOS_SET_BACKGROUND:
				{
					rgb_color bkColor;
					if(bMsg->FindInt32("background", (int32*)&bkColor) != B_OK) break;
					fTopView->SetViewColor(bkColor);
					fTopView->Invalidate();
				}
				break;

			case ETK_BEOS_SET_LOOK:
				{
					int8 look;
					if(bMsg->FindInt8("look", &look) != B_OK) break;
					switch((e_window_look)look)
					{
						case E_BORDERED_WINDOW_LOOK:
							SetLook(B_BORDERED_WINDOW_LOOK);
							break;

						case E_NO_BORDER_WINDOW_LOOK:
							SetLook(B_NO_BORDER_WINDOW_LOOK);
							break;

						case E_TITLED_WINDOW_LOOK:
							SetLook(B_TITLED_WINDOW_LOOK);
							break;

						case E_DOCUMENT_WINDOW_LOOK:
							SetLook(B_DOCUMENT_WINDOW_LOOK);
							break;

						case E_MODAL_WINDOW_LOOK:
							SetLook(B_MODAL_WINDOW_LOOK);
							break;

						case E_FLOATING_WINDOW_LOOK:
							SetLook(B_FLOATING_WINDOW_LOOK);
							break;

						default:
							break;
					}
				}
				break;

			case ETK_BEOS_SET_TITLE:
				{
					const char *title = NULL;
					if(bMsg->FindString("title", &title) != B_OK) break;
					SetTitle(title);
				}
				break;

			case ETK_BEOS_SET_WORKSPACES:
				{
					uint32 workspaces = 0;
					if(bMsg->FindInt32("workspaces", (int32*)&workspaces) != B_OK) break;
					if(workspaces == 0) workspaces = current_workspace() + 1;
					SetWorkspaces(workspaces);
				}
				break;

			case ETK_BEOS_GET_WORKSPACES:
				{
					uint32 workspaces = Workspaces();
					bMsg->AddInt32("workspaces", *((int32*)&workspaces));
				}
				break;

			case ETK_BEOS_ICONIFY:
				if(!IsMinimized()) Minimize(true);
				break;

			case ETK_BEOS_SHOW:
				if(IsHidden())
				{
					uint32 oldFlags = Flags();
					SetFlags(oldFlags | B_AVOID_FOCUS);
					Show();
					if(Look() != B_NO_BORDER_WINDOW_LOOK) SetFlags(oldFlags);
				}
				break;

			case ETK_BEOS_HIDE:
				if(!IsHidden()) Hide();
				break;

			case ETK_BEOS_RAISE:
				if(!IsFront())
				{
					uint32 oldFlags = Flags();
					SetFlags(oldFlags | B_AVOID_FOCUS);
					Activate(true);
					if(Look() != B_NO_BORDER_WINDOW_LOOK) SetFlags(oldFlags);
				}
				break;

			case ETK_BEOS_LOWER:
				{
					BHandler *_frontWin = NULL;
					if(bMsg->FindPointer("front", (void**)&_frontWin) != B_OK) break;
					BWindow *frontWin = e_cast_as(_frontWin, BWindow);
					if(frontWin == NULL) break;
					SendBehind(frontWin);
					bMsg->AddBool("done", true);
				}
				break;

			case ETK_BEOS_ACTIVATE:
				{
					bool state;
					if(bMsg->FindBool("state", &state) != B_OK || state == IsActive()) break;
					Activate(state);
				}
				break;

			case ETK_BEOS_GET_ACTIVATED_STATE:
				bMsg->AddBool("state", IsActive());
				break;

			case ETK_BEOS_MOVE_RESIZE:
				{
					if(bMsg->HasPoint("where"))
					{
						BPoint pt;
						if(bMsg->FindPoint("where", &pt) == B_OK) MoveTo(pt);
					}

					if(bMsg->HasFloat("width") && bMsg->HasFloat("height"))
					{
						float w = -1, h = -1;
						bMsg->FindFloat("width", &w);
						bMsg->FindFloat("height", &h);
						if(w < 0 || h < 0) break;
						ResizeTo(w, h);
					}
				}
				break;

			case ETK_BEOS_DRAW_BITMAP:
				{
					BBitmap *bitmap = NULL;
					BRect srcRect, destRect;
					const ERegion *clipping = NULL;

					if(bMsg->FindPointer("bitmap", (void**)&bitmap) != B_OK || bitmap == NULL) break;
					bMsg->FindRect("src", &srcRect);
					bMsg->FindRect("dest", &destRect);
					if(srcRect.IsValid() == false || destRect.IsValid() == false) break;
					bMsg->FindPointer("clipping", (void**)&clipping); 

					BRegion beRegion;
					__etk_convert_region(clipping, &beRegion, fTopView->Bounds());
					fTopView->ConstrainClippingRegion(&beRegion);
					fTopView->DrawBitmap(bitmap, srcRect, destRect);
				}
				break;

			case ETK_BEOS_GRAB_MOUSE:
			case ETK_BEOS_UNGRAB_MOUSE:
				{
					uint32 options = (what == ETK_BEOS_GRAB_MOUSE ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_POINTER_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == ETK_BEOS_GRAB_MOUSE);
				}
				break;

			case ETK_BEOS_GRAB_KEYBOARD:
			case ETK_BEOS_UNGRAB_KEYBOARD:
				{
					uint32 options = (what == ETK_BEOS_GRAB_KEYBOARD ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_KEYBOARD_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == ETK_BEOS_GRAB_KEYBOARD);
				}
				break;

			case ETK_BEOS_QUERY_MOUSE:
				{
					BPoint pt;
					uint32 btns = 0;
					fTopView->GetMouse(&pt, &btns, false);
					bMsg->AddInt32("x", (int32)pt.x);
					bMsg->AddInt32("y", (int32)pt.y);
					bMsg->AddInt32("buttons", (int32)btns);
				}
				break;

			case ETK_BEOS_SET_SIZE_LIMITS:
				{
					BRect r;
					if(bMsg->FindRect("limits", &r) != B_OK) break;
					SetSizeLimits(r.left, r.right, r.top, r.bottom);
					bMsg->AddBool("done", true);
				}
				break;

			case ETK_BEOS_GET_SIZE_LIMITS:
				{
					BRect r(-1, -1, -1, -1);
					GetSizeLimits(&(r.left), &(r.right), &(r.top), &(r.bottom));
					bMsg->AddRect("limits", r);
				}
				break;

			default:
				handled = false;
				break;
		}

		if(handled)
		{
			BMessage aMsg(*bMsg);
			bMsg->SendReply(&aMsg);
			return;
		}
	}

	switch(bMsg->what)
	{
		case B_WINDOW_ACTIVATED:
			{
				handled = false;

				if(fContactor.IsValid() == false) break;
				EMessage message(E_WINDOW_ACTIVATED);
				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", e_real_time_clock_usecs());
				fContactor.SendMessage(&message);
			}
			break;

		case B_MOUSE_DOWN:
		case B_MOUSE_UP:
		case B_MOUSE_MOVED:
			{
				if(fContactor.IsValid() == false) break;

				BPoint where;
				int32 buttons = 0;

				bMsg->FindPoint("where", &where);
				if(bMsg->what != B_MOUSE_UP) bMsg->FindInt32("buttons", &buttons);

				int32 clicks = 1;
				if(bMsg->what == B_MOUSE_DOWN)
				{
#if 0
					bMsg->FindInt32("clicks", &clicks);
#else
					bigtime_t eventTime;
					if(bMsg->FindInt64("when", &eventTime) == B_OK)
					{
						if(eventTime - fPrevMouseDownTime <= CLICK_TIMEOUT)
							clicks = (fPrevMouseDownCount += 1);
						else
							clicks = fPrevMouseDownCount = 1;
						fPrevMouseDownTime = eventTime;
					}
#endif
				}

				EMessage message;
				if(bMsg->what == B_MOUSE_DOWN) message.what = E_MOUSE_DOWN;
				else if(bMsg->what == B_MOUSE_UP) message.what = E_MOUSE_UP;
				else message.what = E_MOUSE_MOVED;

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", e_real_time_clock_usecs());
				if(bMsg->what != B_MOUSE_UP) message.AddInt32("buttons", buttons);
				if(bMsg->what == B_MOUSE_DOWN) message.AddInt32("clicks", clicks);
				message.AddPoint("where", EPoint(where.x, where.y));
				ConvertToScreen(&where);
				message.AddPoint("screen_where", EPoint(where.x, where.y));

				// TODO: modifiers

				message.AddMessenger("etk:msg_for_target", fContactor);

				etk_app->PostMessage(&message);
			}
			break;

		case B_KEY_DOWN:
		case B_KEY_UP:
		case B_UNMAPPED_KEY_DOWN:
		case B_UNMAPPED_KEY_UP:
			{
				if(fContactor.IsValid() == false) break;

				int8 byte[4];
				const char *bytes = NULL;
				int32 numBytes = 0;
				int32 key = 0;
				int32 key_repeat = 0;
				int32 beModifiers = 0;
				eint32 modifiers = 0;

				bMsg->FindInt32("key", &key);
				bMsg->FindInt32("modifiers", &beModifiers);
				bzero(byte, sizeof(int8) * 4);

				if(bMsg->what == B_KEY_DOWN || bMsg->what == B_KEY_UP)
				{
					for(int32 i = 0; i < 3; i++) bMsg->FindInt8("byte", i, &byte[i]);
					if(bMsg->FindString("bytes", &bytes) == B_OK) numBytes = strlen(bytes);
//					if(bMsg->what == B_KEY_DOWN) bMsg->FindInt32("be:key_repeat", &key_repeat);
				}
				else
				{
					etk_beos_get_byte(beModifiers, key, (char*)byte);
				}

				if(beModifiers & B_SHIFT_KEY) modifiers |= E_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= E_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= E_COMMAND_KEY;

				EMessage message;
				if(bMsg->what == B_KEY_DOWN) message.what = E_KEY_DOWN;
				else if(bMsg->what == B_KEY_UP) message.what = E_KEY_UP;
				else if(bMsg->what == B_UNMAPPED_KEY_DOWN) message.what = E_UNMAPPED_KEY_DOWN;
				else message.what = E_UNMAPPED_KEY_UP;

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", e_real_time_clock_usecs());
				message.AddInt32("key", key);
				message.AddInt32("modifiers", modifiers);

				if(bMsg->what == B_KEY_DOWN || bMsg->what == B_KEY_UP)
				{
					if(bMsg->what == B_KEY_DOWN) message.AddInt32("etk:key_repeat", key_repeat);
					for(int32 i = 0; i < 3; i++) message.AddInt8("byte", byte[i]);
					if(!(numBytes != 1 || *bytes != byte[0]))
					{
						etk_beos_get_byte(beModifiers, key, (char*)byte);
						message.AddString("bytes", (char*)byte);
					}
					else if(numBytes > 0)
					{
						message.AddString("bytes", bytes);
					}
				}
				else if(byte[0] != 0)
				{
					message.AddInt8("byte", byte[0]);
					message.AddString("bytes", (char*)byte);
				}

				message.AddMessenger("etk:msg_for_target", fContactor);

				etk_app->PostMessage(&message);
			}
			break;

		case B_MODIFIERS_CHANGED:
			{
				if(fContactor.IsValid() == false) break;

				eint32 modifiers = 0;
				eint32 old_modifiers = 0;
				int32 beModifiers = 0;
				int32 old_beModifiers = 0;

				bMsg->FindInt32("modifiers", &beModifiers);
				bMsg->FindInt32("be:old_modifiers", &old_beModifiers);

				if(beModifiers & B_SHIFT_KEY) modifiers |= E_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= E_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= E_COMMAND_KEY;

				if(old_beModifiers & B_SHIFT_KEY) old_modifiers |= E_SHIFT_KEY;
				if(old_beModifiers & B_CONTROL_KEY) old_modifiers |= E_CONTROL_KEY;
				if(old_beModifiers & B_COMMAND_KEY) old_modifiers |= E_COMMAND_KEY;

				EMessage message(E_MODIFIERS_CHANGED);

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", e_real_time_clock_usecs());
				message.AddInt32("modifiers", modifiers);
				message.AddInt32("etk:old_modifiers", old_modifiers);

				message.AddMessenger("etk:msg_for_target", fContactor);

				etk_app->PostMessage(&message);
			}
			break;

		default:
			handled = false;
			break;
	}

	if(!handled) BWindow::DispatchMessage(bMsg, handler);
}


void
EBePrivateWin::FrameMoved(BPoint new_position)
{
	BWindow::FrameMoved(new_position);

	if(fContactor.IsValid())
	{
		EMessage message(E_WINDOW_MOVED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", e_real_time_clock_usecs());
		message.AddPoint("where", EPoint(new_position.x, new_position.y));
		fContactor.SendMessage(&message);
	}
}


void
EBePrivateWin::FrameResized(float new_width, float new_height)
{
	BWindow::FrameResized(new_width, new_height);

	if(fContactor.IsValid())
	{
		EMessage message(E_WINDOW_RESIZED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", e_real_time_clock_usecs());
		message.AddFloat("width", new_width);
		message.AddFloat("height", new_height);
		fContactor.SendMessage(&message);
	}
}


void
EBePrivateWin::WorkspacesChanged(uint32 old_ws, uint32 new_ws)
{
	BWindow::WorkspacesChanged(old_ws, new_ws);

	if(fContactor.IsValid())
	{
		EMessage message(E_WORKSPACES_CHANGED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", e_real_time_clock_usecs());
		message.AddInt32("new", (eint32)new_ws);
		fContactor.SendMessage(&message);
	}
}


EBeGraphicsWindow::EBeGraphicsWindow(EBeGraphicsEngine *beEngine, eint32 x, eint32 y, euint32 w, euint32 h)
	: EGraphicsWindow(), fEngine(NULL)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = beEngine;
	if(fEngine == NULL) return;

	EBePrivateWin *beWindow = new EBePrivateWin(x, y, w, h);
	beWinMsgr = BMessenger(beWindow);
	if(beWindow == NULL || beWinMsgr.IsValid() == false)
	{
		if(beWindow) beWindow->Quit();
		fEngine = NULL;
		return;
	}
	beWindow->Unlock();

	e_rgb_color whiteColor = {255, 255, 255, 255};
	EGraphicsDrawable::SetBackgroundColor(whiteColor);
}


EBeGraphicsWindow::~EBeGraphicsWindow()
{
	if(beWinMsgr.IsValid())
	{
		BMessage msg('etk_');
		msg.AddInt32("etk:what", ETK_BEOS_QUIT);
		beWinMsgr.SendMessage(&msg, &msg);
	}
}


e_status_t
EBeGraphicsWindow::ContactTo(const EMessenger *msgr)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_CONTACT_TO);
	if(msgr != NULL)
	{
		size_t size = msgr->FlattenedSize();
		if(size == 0) return E_ERROR;
		char *buffer = (char*)malloc(size);
		if(buffer == NULL) return E_NO_MEMORY;
		if(msgr->Flatten(buffer, size) == false) {free(buffer); return E_ERROR;}
		msg.AddData("etk:messenger", B_ANY_TYPE, buffer, (ssize_t)size, true, 1);
		free(buffer);
	}

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::SetBackgroundColor(e_rgb_color bkColor)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SET_BACKGROUND);
	msg.AddInt32("background", *((int32*)&bkColor));

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply())
	{
		EGraphicsDrawable::SetBackgroundColor(bkColor);
		return E_OK;
	}
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::SetFlags(euint32 flags)
{
	// TODO
	return E_OK;
}


e_status_t
EBeGraphicsWindow::SetLook(e_window_look look)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SET_LOOK);
	msg.AddInt8("look", (int8)look);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::SetFeel(e_window_feel feel)
{
	// TODO
	return E_OK;
}


e_status_t
EBeGraphicsWindow::SetTitle(const char *title)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SET_TITLE);
	msg.AddString("title", title ? title : "");

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::SetWorkspaces(euint32 workspaces)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SET_WORKSPACES);
	msg.AddInt32("workspaces", *((eint32*)&workspaces));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::GetWorkspaces(euint32 *workspaces)
{
	if(beWinMsgr.IsValid() == false || workspaces == NULL) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_GET_WORKSPACES);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;
	return(msg.FindInt32("workspaces", (int32*)workspaces) == B_OK ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::Iconify()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_ICONIFY);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::Show()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SHOW);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::Hide()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_HIDE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::Raise()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_RAISE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::Lower(EGraphicsWindow *_frontWin)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	EBeGraphicsWindow *frontWin = e_cast_as(_frontWin, EBeGraphicsWindow);
	if(frontWin == NULL) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_LOWER);
	msg.AddPointer("front", (void*)frontWin->beWinMsgr.Target(NULL));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? E_ERROR : E_OK);
}


e_status_t
EBeGraphicsWindow::Activate(bool state)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_ACTIVATE);
	msg.AddBool("state", state);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::GetActivatedState(bool *state) const
{
	if(beWinMsgr.IsValid() == false || state == NULL) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_GET_ACTIVATED_STATE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;
	return(msg.FindBool("state", state) == B_OK ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::MoveTo(eint32 x, eint32 y)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::ResizeTo(euint32 w, euint32 h)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_MOVE_RESIZE);
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h)
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_SET_SIZE_LIMITS);
	msg.AddRect("limits", BRect((float)min_w, (float)min_h, (float)max_w, (float)max_h));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? E_ERROR : E_OK);
}


e_status_t
EBeGraphicsWindow::GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h)
{
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return E_ERROR;

	if(beWinMsgr.IsValid() == false) return E_ERROR;

	*min_w = E_MAXUINT32;
	*max_w = E_MAXUINT32;
	*min_h = E_MAXUINT32;
	*max_h = E_MAXUINT32;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_GET_SIZE_LIMITS);

	beWinMsgr.SendMessage(&msg, &msg);

	BRect r;
	if(msg.IsReply() == false || msg.FindRect("limits", &r) != B_OK) return E_ERROR;

	if(r.left >= 0) *min_w = (euint32)r.left;
	if(r.right >= 0) *max_w = (euint32)r.right;
	if(r.top >= 0) *min_h = (euint32)r.top;
	if(r.bottom >= 0) *max_h = (euint32)r.bottom;

	return E_OK;
}


e_status_t
EBeGraphicsWindow::GrabMouse()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_GRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::UngrabMouse()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_UNGRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::GrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_GRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::UngrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_UNGRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? E_OK : E_ERROR);
}


e_status_t
EBeGraphicsWindow::QueryMouse(eint32 *x, eint32 *y, eint32 *buttons)
{
	if(x == NULL && y == NULL && buttons == NULL) return E_ERROR;
	if(beWinMsgr.IsValid() == false) return E_ERROR;

	BMessage msg('etk_');
	msg.AddInt32("etk:what", ETK_BEOS_QUERY_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return E_ERROR;

	if(x) if(msg.FindInt32("x", (int32*)x) != B_OK) return E_ERROR;
	if(y) if(msg.FindInt32("y", (int32*)y) != B_OK) return E_ERROR;
	if(buttons) if(msg.FindInt32("buttons", (int32*)buttons) != B_OK) return E_ERROR;

	return E_OK;
}


e_status_t
EBeGraphicsWindow::CopyTo(EGraphicsContext *dc,
			  EGraphicsDrawable *dstDrawable,
			  eint32 x, eint32 y, euint32 w, euint32 h,
			  eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
			      eint32 x, eint32 y, euint32 w, euint32 h,
			      eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokePoint(EGraphicsContext *dc,
			       eint32 x, eint32 y)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokePoints(EGraphicsContext *dc,
				const eint32 *pts, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokePoints_Colors(EGraphicsContext *dc,
				       const EList *ptsArrayLists, eint32 arrayCount,
				       const e_rgb_color *highColors)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokePoints_Alphas(EGraphicsContext *dc,
				       const eint32 *pts, const euint8 *alpha, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokeLine(EGraphicsContext *dc,
			      eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokePolygon(EGraphicsContext *dc,
				 const eint32 *pts, eint32 count, bool closed)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillPolygon(EGraphicsContext *dc,
			       const eint32 *pts, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokeRect(EGraphicsContext *dc,
			      eint32 x, eint32 y, euint32 w, euint32 h)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillRect(EGraphicsContext *dc,
			    eint32 x, eint32 y, euint32 w, euint32 h)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokeRects(EGraphicsContext *dc,
			       const eint32 *rects, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillRects(EGraphicsContext *dc,
			     const eint32 *rects, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillRegion(EGraphicsContext *dc,
			      const ERegion &region)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokeRoundRect(EGraphicsContext *dc,
				   eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillRoundRect(EGraphicsContext *dc,
				 eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::StrokeArc(EGraphicsContext *dc,
			     eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	// TODO
	return E_ERROR;
}


e_status_t
EBeGraphicsWindow::FillArc(EGraphicsContext *dc,
			   eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	// TODO
	return E_ERROR;
}

