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
 * File: etk-window.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS

#include <be/BeBuild.h>
#include <be/app/AppDefs.h>
#include <be/interface/View.h>
#include <be/interface/Region.h>

#include "etk-beos-graphics.h"

#include "../../support/ClassInfo.h"
#include "../../app/Application.h"

#define CLICK_TIMEOUT 200000


class EBePrivateWin : public BWindow
{
public:
	EBePrivateWin(b_int32 x, b_int32 y, b_int32 w, b_int32 h);
	virtual ~EBePrivateWin();

	virtual bool QuitRequested();

	virtual void FrameMoved(BPoint new_position);
	virtual void FrameResized(float new_width, float new_height);
	virtual void WorkspacesChanged(uint32 old_ws, uint32 new_ws);
	virtual void DispatchMessage(BMessage *bMsg, BHandler *handler);

	BMessenger fContactor;
	BView *fTopView;

private:
	bool doQuit;
	bigtime_t fPrevMouseDownTime;
	b_int32 fPrevMouseDownCount;
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
	EBePrivateWin *win = b_cast_as(Window(), EBePrivateWin);
	if(!(win == NULL || win->fContactor.IsValid() == false))
	{
		BMessage message(_UPDATE_);

		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());

		BRect rect;
		rect.left = updateRect.left;
		rect.top = updateRect.top;
		rect.right = updateRect.right;
		rect.bottom = updateRect.bottom;

		message.AddRect("etk:frame", rect);
		win->fContactor.SendMessage(&message);
	}
}


EBePrivateWin::EBePrivateWin(b_int32 x, b_int32 y, b_int32 w, b_int32 h)
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

	BMessage message(B_QUIT_REQUESTED);
	fContactor.SendMessage(&message);
	return false;
}


static bool bhapi_beos_get_byte(int32 modifiers, int32 key_code, char *result)
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


static void __bhapi_convert_region(const BRegion *region, BRegion *beRegion, BRect maxRect)
{
	if(beRegion == NULL) return;

	if(region != NULL)
	{
		for(b_int32 i = 0; i < region->CountRects(); i++)
		{
			BRect r = region->RectAt(i);
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

	if(bMsg->what == 'bhapi_')
	{
		int32 what = 0;
		bMsg->FindInt32("etk:what", &what);

		switch(what)
		{
			case BHAPI_BEOS_QUIT:
				doQuit = true;
				PostMessage(B_QUIT_REQUESTED);
				break;

			case BHAPI_BEOS_CONTACT_TO:
				{
					fContactor = BMessenger();
					const char *buffer = NULL;
					b_size_t size = -1;
					if(bMsg->FindData("etk:messenger", B_ANY_TYPE, (const void**)&buffer, &size) != B_OK) break;
					if(buffer == NULL || size <= 0) break;
					fContactor.Unflatten(buffer, (size_t)size);
				}
				break;

			case BHAPI_BEOS_SET_BACKGROUND:
				{
					rgb_color bkColor;
					if(bMsg->FindInt32("background", (int32*)&bkColor) != B_OK) break;
					fTopView->SetViewColor(bkColor);
					fTopView->Invalidate();
				}
				break;

			case BHAPI_BEOS_SET_LOOK:
				{
					int8 look;
					if(bMsg->FindInt8("look", &look) != B_OK) break;
					switch((b_window_look)look)
					{
						case B_BORDERED_WINDOW_LOOK:
							SetLook(B_BORDERED_WINDOW_LOOK);
							break;

						case B_NO_BORDER_WINDOW_LOOK:
							SetLook(B_NO_BORDER_WINDOW_LOOK);
							break;

						case B_TITLED_WINDOW_LOOK:
							SetLook(B_TITLED_WINDOW_LOOK);
							break;

						case B_DOCUMENT_WINDOW_LOOK:
							SetLook(B_DOCUMENT_WINDOW_LOOK);
							break;

						case B_MODAL_WINDOW_LOOK:
							SetLook(B_MODAL_WINDOW_LOOK);
							break;

						case B_FLOATING_WINDOW_LOOK:
							SetLook(B_FLOATING_WINDOW_LOOK);
							break;

						default:
							break;
					}
				}
				break;

			case BHAPI_BEOS_SET_TITLE:
				{
					const char *title = NULL;
					if(bMsg->FindString("title", &title) != B_OK) break;
					SetTitle(title);
				}
				break;

			case BHAPI_BEOS_SET_WORKSPACES:
				{
					uint32 workspaces = 0;
					if(bMsg->FindInt32("workspaces", (int32*)&workspaces) != B_OK) break;
					if(workspaces == 0) workspaces = current_workspace() + 1;
					SetWorkspaces(workspaces);
				}
				break;

			case BHAPI_BEOS_GET_WORKSPACES:
				{
					uint32 workspaces = Workspaces();
					bMsg->AddInt32("workspaces", *((int32*)&workspaces));
				}
				break;

			case BHAPI_BEOS_ICONIFY:
				if(!IsMinimized()) Minimize(true);
				break;

			case BHAPI_BEOS_SHOW:
				if(IsHidden())
				{
					uint32 oldFlags = Flags();
					SetFlags(oldFlags | B_AVOID_FOCUS);
					Show();
					if(Look() != B_NO_BORDER_WINDOW_LOOK) SetFlags(oldFlags);
				}
				break;

			case BHAPI_BEOS_HIDE:
				if(!IsHidden()) Hide();
				break;

			case BHAPI_BEOS_RAISE:
				if(!IsFront())
				{
					uint32 oldFlags = Flags();
					SetFlags(oldFlags | B_AVOID_FOCUS);
					Activate(true);
					if(Look() != B_NO_BORDER_WINDOW_LOOK) SetFlags(oldFlags);
				}
				break;

			case BHAPI_BEOS_LOWER:
				{
					BHandler *_frontWin = NULL;
					if(bMsg->FindPointer("front", (void**)&_frontWin) != B_OK) break;
					BWindow *frontWin = b_cast_as(_frontWin, BWindow);
					if(frontWin == NULL) break;
					SendBehind(frontWin);
					bMsg->AddBool("done", true);
				}
				break;

			case BHAPI_BEOS_ACTIVATE:
				{
					bool state;
					if(bMsg->FindBool("state", &state) != B_OK || state == IsActive()) break;
					Activate(state);
				}
				break;

			case BHAPI_BEOS_GET_ACTIVATED_STATE:
				bMsg->AddBool("state", IsActive());
				break;

			case BHAPI_BEOS_MOVE_RESIZE:
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

			case BHAPI_BEOS_DRAW_BITMAP:
				{
					BBitmap *bitmap = NULL;
					BRect srcRect, destRect;
					const BRegion *clipping = NULL;

					if(bMsg->FindPointer("bitmap", (void**)&bitmap) != B_OK || bitmap == NULL) break;
					bMsg->FindRect("src", &srcRect);
					bMsg->FindRect("dest", &destRect);
					if(srcRect.IsValid() == false || destRect.IsValid() == false) break;
					bMsg->FindPointer("clipping", (void**)&clipping);

					BRegion beRegion;
					__bhapi_convert_region(clipping, &beRegion, fTopView->Bounds());
					fTopView->ConstrainClippingRegion(&beRegion);
					fTopView->DrawBitmap(bitmap, srcRect, destRect);
				}
				break;

			case BHAPI_BEOS_GRAB_MOUSE:
			case BHAPI_BEOS_UNGRAB_MOUSE:
				{
					uint32 options = (what == BHAPI_BEOS_GRAB_MOUSE ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_POINTER_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == BHAPI_BEOS_GRAB_MOUSE);
				}
				break;

			case BHAPI_BEOS_GRAB_KEYBOARD:
			case BHAPI_BEOS_UNGRAB_KEYBOARD:
				{
					uint32 options = (what == BHAPI_BEOS_GRAB_KEYBOARD ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_KEYBOARD_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == BHAPI_BEOS_GRAB_KEYBOARD);
				}
				break;

			case BHAPI_BEOS_QUERY_MOUSE:
				{
					BPoint pt;
					uint32 btns = 0;
					fTopView->GetMouse(&pt, &btns, false);
					bMsg->AddInt32("x", (int32)pt.x);
					bMsg->AddInt32("y", (int32)pt.y);
					bMsg->AddInt32("buttons", (int32)btns);
				}
				break;

			case BHAPI_BEOS_SET_SIZE_LIMITS:
				{
					BRect r;
					if(bMsg->FindRect("limits", &r) != B_OK) break;
					SetSizeLimits(r.left, r.right, r.top, r.bottom);
					bMsg->AddBool("done", true);
				}
				break;

			case BHAPI_BEOS_GET_SIZE_LIMITS:
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
				BMessage message(B_WINDOW_ACTIVATED);
				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
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

				BMessage message;
				if(bMsg->what == B_MOUSE_DOWN) message.what = B_MOUSE_DOWN;
				else if(bMsg->what == B_MOUSE_UP) message.what = B_MOUSE_UP;
				else message.what = B_MOUSE_MOVED;

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				if(bMsg->what != B_MOUSE_UP) message.AddInt32("buttons", buttons);
				if(bMsg->what == B_MOUSE_DOWN) message.AddInt32("clicks", clicks);
				message.AddPoint("where", BPoint(where.x, where.y));
				ConvertToScreen(&where);
				message.AddPoint("screen_where", BPoint(where.x, where.y));

				// TODO: modifiers

				message.AddMessenger("etk:msg_for_target", fContactor);

				bhapi_app->PostMessage(&message);
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
				b_int32 modifiers = 0;

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
					bhapi_beos_get_byte(beModifiers, key, (char*)byte);
				}

				if(beModifiers & B_SHIFT_KEY) modifiers |= B_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= B_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= B_COMMAND_KEY;

				BMessage message;
				if(bMsg->what == B_KEY_DOWN) message.what = B_KEY_DOWN;
				else if(bMsg->what == B_KEY_UP) message.what = B_KEY_UP;
				else if(bMsg->what == B_UNMAPPED_KEY_DOWN) message.what = B_UNMAPPED_KEY_DOWN;
				else message.what = B_UNMAPPED_KEY_UP;

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				message.AddInt32("key", key);
				message.AddInt32("modifiers", modifiers);

				if(bMsg->what == B_KEY_DOWN || bMsg->what == B_KEY_UP)
				{
					if(bMsg->what == B_KEY_DOWN) message.AddInt32("etk:key_repeat", key_repeat);
					for(int32 i = 0; i < 3; i++) message.AddInt8("byte", byte[i]);
					if(!(numBytes != 1 || *bytes != byte[0]))
					{
						bhapi_beos_get_byte(beModifiers, key, (char*)byte);
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

				bhapi_app->PostMessage(&message);
			}
			break;

		case B_MODIFIERS_CHANGED:
			{
				if(fContactor.IsValid() == false) break;

				b_int32 modifiers = 0;
				b_int32 old_modifiers = 0;
				int32 beModifiers = 0;
				int32 old_beModifiers = 0;

				bMsg->FindInt32("modifiers", &beModifiers);
				bMsg->FindInt32("be:old_modifiers", &old_beModifiers);

				if(beModifiers & B_SHIFT_KEY) modifiers |= B_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= B_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= B_COMMAND_KEY;

				if(old_beModifiers & B_SHIFT_KEY) old_modifiers |= B_SHIFT_KEY;
				if(old_beModifiers & B_CONTROL_KEY) old_modifiers |= B_CONTROL_KEY;
				if(old_beModifiers & B_COMMAND_KEY) old_modifiers |= B_COMMAND_KEY;

				BMessage message(B_MODIFIERS_CHANGED);

				message.AddBool("etk:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				message.AddInt32("modifiers", modifiers);
				message.AddInt32("etk:old_modifiers", old_modifiers);

				message.AddMessenger("etk:msg_for_target", fContactor);

				bhapi_app->PostMessage(&message);
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
		BMessage message(B_WINDOW_MOVED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());
		message.AddPoint("where", BPoint(new_position.x, new_position.y));
		fContactor.SendMessage(&message);
	}
}


void
EBePrivateWin::FrameResized(float new_width, float new_height)
{
	BWindow::FrameResized(new_width, new_height);

	if(fContactor.IsValid())
	{
		BMessage message(B_WINDOW_RESIZED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());
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
		BMessage message(B_WORKSPACES_CHANGED);
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());
		message.AddInt32("new", (b_int32)new_ws);
		fContactor.SendMessage(&message);
	}
}


EBeGraphicsWindow::EBeGraphicsWindow(EBeGraphicsEngine *beEngine, b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
	: BGraphicsWindow(), fEngine(NULL)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
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

	b_rgb_color whiteColor = {255, 255, 255, 255};
	BGraphicsDrawable::SetBackgroundColor(whiteColor);
}


EBeGraphicsWindow::~EBeGraphicsWindow()
{
	if(beWinMsgr.IsValid())
	{
		BMessage msg('bhapi_');
		msg.AddInt32("etk:what", BHAPI_BEOS_QUIT);
		beWinMsgr.SendMessage(&msg, &msg);
	}
}


b_status_t
EBeGraphicsWindow::ContactTo(const BMessenger *msgr)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_CONTACT_TO);
	if(msgr != NULL)
	{
		size_t size = msgr->FlattenedSize();
		if(size == 0) return B_ERROR;
		char *buffer = (char*)malloc(size);
		if(buffer == NULL) return B_NO_MEMORY;
		if(msgr->Flatten(buffer, size) == false) {free(buffer); return B_ERROR;}
		msg.AddData("etk:messenger", B_ANY_TYPE, buffer, (b_size_t)size, true, 1);
		free(buffer);
	}

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::SetBackgroundColor(b_rgb_color bkColor)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SET_BACKGROUND);
	msg.AddInt32("background", *((int32*)&bkColor));

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply())
	{
		BGraphicsDrawable::SetBackgroundColor(bkColor);
		return B_OK;
	}
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::SetFlags(b_uint32 flags)
{
	// TODO
	return B_OK;
}


b_status_t
EBeGraphicsWindow::SetLook(b_window_look look)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SET_LOOK);
	msg.AddInt8("look", (int8)look);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::SetFeel(b_window_feel feel)
{
	// TODO
	return B_OK;
}


b_status_t
EBeGraphicsWindow::SetTitle(const char *title)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SET_TITLE);
	msg.AddString("title", title ? title : "");

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::SetWorkspaces(b_uint32 workspaces)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SET_WORKSPACES);
	msg.AddInt32("workspaces", *((b_int32*)&workspaces));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::GetWorkspaces(b_uint32 *workspaces)
{
	if(beWinMsgr.IsValid() == false || workspaces == NULL) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_GET_WORKSPACES);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;
	return(msg.FindInt32("workspaces", (int32*)workspaces) == B_OK ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::Iconify()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_ICONIFY);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::Show()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SHOW);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::Hide()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_HIDE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::Raise()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_RAISE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::Lower(BGraphicsWindow *_frontWin)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	EBeGraphicsWindow *frontWin = b_cast_as(_frontWin, EBeGraphicsWindow);
	if(frontWin == NULL) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_LOWER);
	msg.AddPointer("front", (void*)frontWin->beWinMsgr.Target(NULL));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? B_ERROR : B_OK);
}


b_status_t
EBeGraphicsWindow::Activate(bool state)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_ACTIVATE);
	msg.AddBool("state", state);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::GetActivatedState(bool *state) const
{
	if(beWinMsgr.IsValid() == false || state == NULL) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_GET_ACTIVATED_STATE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;
	return(msg.FindBool("state", state) == B_OK ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::MoveTo(b_int32 x, b_int32 y)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::ResizeTo(b_uint32 w, b_uint32 h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::MoveAndResizeTo(b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::SetSizeLimits(b_uint32 min_w, b_uint32 max_w, b_uint32 min_h, b_uint32 max_h)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_SET_SIZE_LIMITS);
	msg.AddRect("limits", BRect((float)min_w, (float)min_h, (float)max_w, (float)max_h));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? B_ERROR : B_OK);
}


b_status_t
EBeGraphicsWindow::GetSizeLimits(b_uint32 *min_w, b_uint32 *max_w, b_uint32 *min_h, b_uint32 *max_h)
{
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return B_ERROR;

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	*min_w = B_MAXUINT32;
	*max_w = B_MAXUINT32;
	*min_h = B_MAXUINT32;
	*max_h = B_MAXUINT32;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_GET_SIZE_LIMITS);

	beWinMsgr.SendMessage(&msg, &msg);

	BRect r;
	if(msg.IsReply() == false || msg.FindRect("limits", &r) != B_OK) return B_ERROR;

	if(r.left >= 0) *min_w = (b_uint32)r.left;
	if(r.right >= 0) *max_w = (b_uint32)r.right;
	if(r.top >= 0) *min_h = (b_uint32)r.top;
	if(r.bottom >= 0) *max_h = (b_uint32)r.bottom;

	return B_OK;
}


b_status_t
EBeGraphicsWindow::GrabMouse()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_GRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::UngrabMouse()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_UNGRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::GrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_GRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::UngrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_UNGRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? B_OK : B_ERROR);
}


b_status_t
EBeGraphicsWindow::QueryMouse(b_int32 *x, b_int32 *y, b_int32 *buttons)
{
	if(x == NULL && y == NULL && buttons == NULL) return B_ERROR;
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('bhapi_');
	msg.AddInt32("etk:what", BHAPI_BEOS_QUERY_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	if(x) if(msg.FindInt32("x", (int32*)x) != B_OK) return B_ERROR;
	if(y) if(msg.FindInt32("y", (int32*)y) != B_OK) return B_ERROR;
	if(buttons) if(msg.FindInt32("buttons", (int32*)buttons) != B_OK) return B_ERROR;

	return B_OK;
}


b_status_t
EBeGraphicsWindow::CopyTo(BGraphicsContext *dc,
			  BGraphicsDrawable *dstDrawable,
			  b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
			  b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
			      b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
			      b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokePoint(BGraphicsContext *dc,
			       b_int32 x, b_int32 y)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokePoints(BGraphicsContext *dc,
				const b_int32 *pts, b_int32 count)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokePoints_Colors(BGraphicsContext *dc,
				       const BList *ptsArrayLists, b_int32 arrayCount,
				       const b_rgb_color *highColors)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokePoints_Alphas(BGraphicsContext *dc,
				       const b_int32 *pts, const b_uint8 *alpha, b_int32 count)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokeLine(BGraphicsContext *dc,
			      b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokePolygon(BGraphicsContext *dc,
				 const b_int32 *pts, b_int32 count, bool closed)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillPolygon(BGraphicsContext *dc,
			       const b_int32 *pts, b_int32 count)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokeRect(BGraphicsContext *dc,
			      b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillRect(BGraphicsContext *dc,
			    b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokeRects(BGraphicsContext *dc,
			       const b_int32 *rects, b_int32 count)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillRects(BGraphicsContext *dc,
			     const b_int32 *rects, b_int32 count)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillRegion(BGraphicsContext *dc,
			      const BRegion &region)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokeRoundRect(BGraphicsContext *dc,
				   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillRoundRect(BGraphicsContext *dc,
				 b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::StrokeArc(BGraphicsContext *dc,
			     b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}


b_status_t
EBeGraphicsWindow::FillArc(BGraphicsContext *dc,
			   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}

#endif /* BEOS */
