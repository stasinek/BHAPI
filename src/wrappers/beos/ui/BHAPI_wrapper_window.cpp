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
 * File: BHAPI_wrapper_window.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS

#include <be/BeBuild.h>
#include <be/app/AppDefs.h>
#include <be/interface/View.h>
#include <be/interface/Region.h>

#include <BHAPI_wrapper_beos-graphics.h>

#include <kits/support/ClassInfo.h>
#include <kits/app/Application.h>

#define CLICK_TIMEOUT 200000


class EBePrivateWin : public BWindow
{
public:
	EBePrivateWin(int32_t x,  int32_t y,  int32_t w,  int32_t h);
	virtual ~EBePrivateWin();

	virtual bool QuitRequested();

	virtual void FrameMoved(BPoint new_position);
	virtual void FrameResized(float new_width, float new_height);
	virtual void WorkspacesChanged(uint32_t old_ws, uint32_t new_ws);
	virtual void DispatchMessage(BMessage *bMsg, BHandler *handler);

	BMessenger fContactor;
	BView *fTopView;

private:
	bool doQuit;
	bigtime_t fPrevMouseDownTime;
	int32_t fPrevMouseDownCount;
};


class EBePrivateWinTopView : public BView
{
public:
	EBePrivateWinTopView(BRect frame, const char *name, uint32_t resizeMask, uint32_t flags);
	virtual ~EBePrivateWinTopView();
	virtual void Draw(BRect updateRect);
};


EBePrivateWinTopView::EBePrivateWinTopView(BRect frame, const char *name, uint32_t resizeMask, uint32_t flags)
	: BView(frame, name, resizeMask, flags)
{
}


EBePrivateWinTopView::~EBePrivateWinTopView()
{
}


void 
EBePrivateWinTopView::Draw(BRect updateRect)
{
	EBePrivateWin *win = cast_as(Window(), EBePrivateWin);
	if(!(win == NULL || win->fContactor.IsValid() == false))
	{
		BMessage message(_UPDATE_);

		message.AddBool("BHAPI:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());

		BRect rect;
		rect.left = updateRect.left;
		rect.top = updateRect.top;
		rect.right = updateRect.right;
		rect.bottom = updateRect.bottom;

		message.AddRect("BHAPI:frame", rect);
		win->fContactor.SendMessage(&message);
	}
}


EBePrivateWin::EBePrivateWin(int32_t x,  int32_t y,  int32_t w,  int32_t h)
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


static bool b_beos_get_byte(int32_t modifiers, int32_t key_code, char *result)
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

	int32_t offset;
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
		for(int32_t i = 0; i < region->CountRects(); i++)
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

	if(bMsg->what == 'b_')
	{
		int32_t what = 0;
		bMsg->FindInt32("BHAPI:what", &what);

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
					ssize_t size = -1;
					if(bMsg->FindData("BHAPI:messenger", B_ANY_TYPE, (const void**)&buffer, &size) != B_OK) break;
					if(buffer == NULL || size <= 0) break;
					fContactor.Unflatten(buffer, (size_t)size);
				}
				break;

			case BHAPI_BEOS_SET_BACKGROUND:
				{
					rgb_color bkColor;
					if(bMsg->FindInt32("background", (int32_t*)&bkColor) != B_OK) break;
					fTopView->SetViewColor(bkColor);
					fTopView->Invalidate();
				}
				break;

			case BHAPI_BEOS_SET_LOOK:
				{
					int8 look;
					if(bMsg->FindInt8("look", &look) != B_OK) break;
					switch((bhapi::window_look)look)
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
					uint32_t workspaces = 0;
					if(bMsg->FindInt32("workspaces", (int32_t*)&workspaces) != B_OK) break;
					if(workspaces == 0) workspaces = current_workspace() + 1;
					SetWorkspaces(workspaces);
				}
				break;

			case BHAPI_BEOS_GET_WORKSPACES:
				{
					uint32_t workspaces = Workspaces();
					bMsg->AddInt32("workspaces", *((int32_t*)&workspaces));
				}
				break;

			case BHAPI_BEOS_ICONIFY:
				if(!IsMinimized()) Minimize(true);
				break;

			case BHAPI_BEOS_SHOW:
				if(IsHidden())
				{
					uint32_t oldFlags = Flags();
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
					uint32_t oldFlags = Flags();
					SetFlags(oldFlags | B_AVOID_FOCUS);
					Activate(true);
					if(Look() != B_NO_BORDER_WINDOW_LOOK) SetFlags(oldFlags);
				}
				break;

			case BHAPI_BEOS_LOWER:
				{
					BHandler *_frontWin = NULL;
					if(bMsg->FindPointer("front", (void**)&_frontWin) != B_OK) break;
					BWindow *frontWin = cast_as(_frontWin, BWindow);
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
					uint32_t options = (what == BHAPI_BEOS_GRAB_MOUSE ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_POINTER_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == BHAPI_BEOS_GRAB_MOUSE);
				}
				break;

			case BHAPI_BEOS_GRAB_KEYBOARD:
			case BHAPI_BEOS_UNGRAB_KEYBOARD:
				{
					uint32_t options = (what == BHAPI_BEOS_GRAB_KEYBOARD ? B_LOCK_WINDOW_FOCUS : 0);
					if(fTopView->SetEventMask(B_KEYBOARD_EVENTS, options) != B_OK) break;
					bMsg->AddBool("state", what == BHAPI_BEOS_GRAB_KEYBOARD);
				}
				break;

			case BHAPI_BEOS_QUERY_MOUSE:
				{
					BPoint pt;
					uint32_t btns = 0;
					fTopView->GetMouse(&pt, &btns, false);
					bMsg->AddInt32("x", (int32_t)pt.x);
					bMsg->AddInt32("y", (int32_t)pt.y);
					bMsg->AddInt32("buttons", (int32_t)btns);
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
				message.AddBool("BHAPI:msg_from_gui", true);
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
				int32_t buttons = 0;

				bMsg->FindPoint("where", &where);
				if(bMsg->what != B_MOUSE_UP) bMsg->FindInt32("buttons", &buttons);

				int32_t clicks = 1;
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

				message.AddBool("BHAPI:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				if(bMsg->what != B_MOUSE_UP) message.AddInt32("buttons", buttons);
				if(bMsg->what == B_MOUSE_DOWN) message.AddInt32("clicks", clicks);
				message.AddPoint("where", BPoint(where.x, where.y));
				ConvertToScreen(&where);
				message.AddPoint("screen_where", BPoint(where.x, where.y));

				// TODO: modifiers

				message.AddMessenger("BHAPI:msg_for_target", fContactor);

				bhapi::__be_clipboard->PostMessage(&message);
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
				int32_t numBytes = 0;
				int32_t key = 0;
				int32_t key_repeat = 0;
				int32_t beModifiers = 0;
				int32_t modifiers = 0;

				bMsg->FindInt32("key", &key);
				bMsg->FindInt32("modifiers", &beModifiers);
				bzero(byte, sizeof(int8) * 4);

				if(bMsg->what == B_KEY_DOWN || bMsg->what == B_KEY_UP)
				{
					for(int32_t i = 0; i < 3; i++) bMsg->FindInt8("byte", i, &byte[i]);
					if(bMsg->FindString("bytes", &bytes) == B_OK) numBytes = strlen(bytes);
//					if(bMsg->what == B_KEY_DOWN) bMsg->FindInt32("be:key_repeat", &key_repeat);
				}
				else
				{
					b_beos_get_byte(beModifiers, key, (char*)byte);
				}

				if(beModifiers & B_SHIFT_KEY) modifiers |= B_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= B_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= B_COMMAND_KEY;

				BMessage message;
				if(bMsg->what == B_KEY_DOWN) message.what = B_KEY_DOWN;
				else if(bMsg->what == B_KEY_UP) message.what = B_KEY_UP;
				else if(bMsg->what == B_UNMAPPED_KEY_DOWN) message.what = B_UNMAPPED_KEY_DOWN;
				else message.what = B_UNMAPPED_KEY_UP;

				message.AddBool("BHAPI:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				message.AddInt32("key", key);
				message.AddInt32("modifiers", modifiers);

				if(bMsg->what == B_KEY_DOWN || bMsg->what == B_KEY_UP)
				{
					if(bMsg->what == B_KEY_DOWN) message.AddInt32("BHAPI:key_repeat", key_repeat);
					for(int32_t i = 0; i < 3; i++) message.AddInt8("byte", byte[i]);
					if(!(numBytes != 1 || *bytes != byte[0]))
					{
						b_beos_get_byte(beModifiers, key, (char*)byte);
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

				message.AddMessenger("BHAPI:msg_for_target", fContactor);

				bhapi::__be_clipboard->PostMessage(&message);
			}
			break;

		case B_MODIFIERS_CHANGED:
			{
				if(fContactor.IsValid() == false) break;

				int32_t modifiers = 0;
				int32_t old_modifiers = 0;
				int32_t beModifiers = 0;
				int32_t old_beModifiers = 0;

				bMsg->FindInt32("modifiers", &beModifiers);
				bMsg->FindInt32("be:old_modifiers", &old_beModifiers);

				if(beModifiers & B_SHIFT_KEY) modifiers |= B_SHIFT_KEY;
				if(beModifiers & B_CONTROL_KEY) modifiers |= B_CONTROL_KEY;
				if(beModifiers & B_COMMAND_KEY) modifiers |= B_COMMAND_KEY;

				if(old_beModifiers & B_SHIFT_KEY) old_modifiers |= B_SHIFT_KEY;
				if(old_beModifiers & B_CONTROL_KEY) old_modifiers |= B_CONTROL_KEY;
				if(old_beModifiers & B_COMMAND_KEY) old_modifiers |= B_COMMAND_KEY;

				BMessage message(B_MODIFIERS_CHANGED);

				message.AddBool("BHAPI:msg_from_gui", true);
				message.AddInt64("when", b_real_time_clock_usecs());
				message.AddInt32("modifiers", modifiers);
				message.AddInt32("BHAPI:old_modifiers", old_modifiers);

				message.AddMessenger("BHAPI:msg_for_target", fContactor);

				bhapi::__be_clipboard->PostMessage(&message);
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
		message.AddBool("BHAPI:msg_from_gui", true);
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
		message.AddBool("BHAPI:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());
		message.AddFloat("width", new_width);
		message.AddFloat("height", new_height);
		fContactor.SendMessage(&message);
	}
}


void 
EBePrivateWin::WorkspacesChanged(uint32_t old_ws, uint32_t new_ws)
{
	BWindow::WorkspacesChanged(old_ws, new_ws);

	if(fContactor.IsValid())
	{
		BMessage message(B_WORKSPACES_CHANGED);
		message.AddBool("BHAPI:msg_from_gui", true);
		message.AddInt64("when", b_real_time_clock_usecs());
		message.AddInt32("new", (int32_t)new_ws);
		fContactor.SendMessage(&message);
	}
}


EBeGraphicsWindow::EBeGraphicsWindow(EBeGraphicsEngine *beEngine,  int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
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

	bhapi::rgb_color whiteColor = {255, 255, 255, 255};
	BGraphicsDrawable::SetBackgroundColor(whiteColor);
}


EBeGraphicsWindow::~EBeGraphicsWindow()
{
	if(beWinMsgr.IsValid())
	{
		BMessage msg('b_');
		msg.AddInt32("BHAPI:what", BHAPI_BEOS_QUIT);
		beWinMsgr.SendMessage(&msg, &msg);
	}
}


status_t 
EBeGraphicsWindow::ContactTo(const BMessenger *msgr)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_CONTACT_TO);
	if(msgr != NULL)
	{
		size_t size = msgr->FlattenedSize();
		if(size == 0) return B_ERROR;
		char *buffer = (char*)malloc(size);
		if(buffer == NULL) return B_NO_MEMORY;
		if(msgr->Flatten(buffer, size) == false) {free(buffer); return B_ERROR;}
		msg.AddData("BHAPI:messenger", B_ANY_TYPE, buffer, (ssize_t)size, true, 1);
		free(buffer);
	}

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SET_BACKGROUND);
	msg.AddInt32("background", *((int32_t*)&bkColor));

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply())
	{
		BGraphicsDrawable::SetBackgroundColor(bkColor);
		return B_OK;
	}
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::SetFlags(uint32_t flags)
{
	// TODO
	return B_OK;
}


status_t 
EBeGraphicsWindow::SetLook(bhapi::window_look look)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SET_LOOK);
	msg.AddInt8("look", (int8)look);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::SetFeel(bhapi::window_feel feel)
{
	// TODO
	return B_OK;
}


status_t 
EBeGraphicsWindow::SetTitle(const char *title)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SET_TITLE);
	msg.AddString("title", title ? title : "");

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::SetWorkspaces(uint32_t workspaces)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SET_WORKSPACES);
	msg.AddInt32("workspaces", *((int32_t*)&workspaces));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::GetWorkspaces(uint32_t *workspaces)
{
	if(beWinMsgr.IsValid() == false || workspaces == NULL) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_GET_WORKSPACES);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;
	return(msg.FindInt32("workspaces", (int32_t*)workspaces) == B_OK ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::Iconify()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_ICONIFY);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::Show()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SHOW);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::Hide()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_HIDE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::Raise()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_RAISE);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::Lower(BGraphicsWindow *_frontWin)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	EBeGraphicsWindow *frontWin = cast_as(_frontWin, EBeGraphicsWindow);
	if(frontWin == NULL) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_LOWER);
	msg.AddPointer("front", (void*)frontWin->beWinMsgr.Target(NULL));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? B_ERROR : B_OK);
}


status_t 
EBeGraphicsWindow::Activate(bool state)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_ACTIVATE);
	msg.AddBool("state", state);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::GetActivatedState(bool *state) const
{
	if(beWinMsgr.IsValid() == false || state == NULL) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_GET_ACTIVATED_STATE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;
	return(msg.FindBool("state", state) == B_OK ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::MoveTo(int32_t x,  int32_t y)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::ResizeTo(uint32_t w,  uint32_t h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::MoveAndResizeTo(int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_MOVE_RESIZE);
	msg.AddPoint("where", BPoint(x, y));
	msg.AddFloat("width", (float)w);
	msg.AddFloat("height", (float)h);

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::SetSizeLimits(uint32_t min_w,  uint32_t max_w,  uint32_t min_h,  uint32_t max_h)
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_SET_SIZE_LIMITS);
	msg.AddRect("limits", BRect((float)min_w, (float)min_h, (float)max_w, (float)max_h));

	beWinMsgr.SendMessage(&msg, &msg);
	return(msg.IsReply() == false || msg.HasBool("done") == false ? B_ERROR : B_OK);
}


status_t 
EBeGraphicsWindow::GetSizeLimits(uint32_t *min_w,  uint32_t *max_w,  uint32_t *min_h,  uint32_t *max_h)
{
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return B_ERROR;

	if(beWinMsgr.IsValid() == false) return B_ERROR;

	*min_w = B_MAXUINT32;
	*max_w = B_MAXUINT32;
	*min_h = B_MAXUINT32;
	*max_h = B_MAXUINT32;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_GET_SIZE_LIMITS);

	beWinMsgr.SendMessage(&msg, &msg);

	BRect r;
	if(msg.IsReply() == false || msg.FindRect("limits", &r) != B_OK) return B_ERROR;

	if(r.left >= 0) *min_w = (uint32_t)r.left;
	if(r.right >= 0) *max_w = (uint32_t)r.right;
	if(r.top >= 0) *min_h = (uint32_t)r.top;
	if(r.bottom >= 0) *max_h = (uint32_t)r.bottom;

	return B_OK;
}


status_t 
EBeGraphicsWindow::GrabMouse()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_GRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::UngrabMouse()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_UNGRAB_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::GrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_GRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = false;
	msg.FindBool("state", &state);
	return(state ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::UngrabKeyboard()
{
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_UNGRAB_KEYBOARD);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	bool state = true;
	msg.FindBool("state", &state);
	return(!state ? B_OK : B_ERROR);
}


status_t 
EBeGraphicsWindow::QueryMouse(int32_t *x,  int32_t *y,  int32_t *buttons)
{
	if(x == NULL && y == NULL && buttons == NULL) return B_ERROR;
	if(beWinMsgr.IsValid() == false) return B_ERROR;

	BMessage msg('b_');
	msg.AddInt32("BHAPI:what", BHAPI_BEOS_QUERY_MOUSE);

	beWinMsgr.SendMessage(&msg, &msg);
	if(msg.IsReply() == false) return B_ERROR;

	if(x) if(msg.FindInt32("x", (int32_t*)x) != B_OK) return B_ERROR;
	if(y) if(msg.FindInt32("y", (int32_t*)y) != B_OK) return B_ERROR;
	if(buttons) if(msg.FindInt32("buttons", (int32_t*)buttons) != B_OK) return B_ERROR;

	return B_OK;
}


status_t 
EBeGraphicsWindow::CopyTo(BGraphicsContext *dc,
			  BGraphicsDrawable *dstDrawable,
			   int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
			   int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
			       int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
			       int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokePoint(BGraphicsContext *dc,
			        int32_t x,  int32_t y)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokePoints(BGraphicsContext *dc,
				const  int32_t *pts,  int32_t count)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokePoints_Colors(BGraphicsContext *dc,
				       const BList *ptsArrayLists,  int32_t arrayCount,
				       const bhapi::rgb_color *highColors)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokePoints_Alphas(BGraphicsContext *dc,
				       const  int32_t *pts, const  uint8 *alpha,  int32_t count)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokeLine(BGraphicsContext *dc,
			       int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokePolygon(BGraphicsContext *dc,
				 const  int32_t *pts,  int32_t count, bool closed)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillPolygon(BGraphicsContext *dc,
			       const  int32_t *pts,  int32_t count)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokeRect(BGraphicsContext *dc,
			       int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillRect(BGraphicsContext *dc,
			     int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokeRects(BGraphicsContext *dc,
			       const  int32_t *rects,  int32_t count)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillRects(BGraphicsContext *dc,
			     const  int32_t *rects,  int32_t count)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillRegion(BGraphicsContext *dc,
			      const BRegion &region)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokeRoundRect(BGraphicsContext *dc,
				    int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillRoundRect(BGraphicsContext *dc,
				  int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::StrokeArc(BGraphicsContext *dc,
			      int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}


status_t 
EBeGraphicsWindow::FillArc(BGraphicsContext *dc,
			    int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}

#endif /* BEOS */
