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
#ifdef WIN32

#include "BHAPI_wrapper_win32gdi.h"

#include "../../kernel/Kernel.h"
#include "../../kernel/Debug.h"
#include "../../support/Autolock.h"
#include "../../support/Errors.h"
#include "../../app/Application.h"
#include "../../support/StringClass.h"
#include "../../support/ClassInfo.h"


bool bhapi::win32_window_convert_to_screen(HWND hWnd, int *x, int *y)
{
	if(hWnd == NULL || x == NULL || y == NULL) return false;

	POINT pt;
	pt.x = *x;
	pt.y = *y;

	if(ClientToScreen(hWnd, &pt) == 0) return false;

	*x = (int)pt.x;
	*y = (int)pt.y;

	return true;
}


bool bhapi::win32_window_get_rect(HWND hWnd, RECT *r)
{
	if(hWnd == NULL || r == NULL) return false;

	int x = 0, y = 0;

	if(GetClientRect(hWnd, r) == 0) return false;

	if(!bhapi::win32_window_convert_to_screen(hWnd, &x, &y)) return false;

	r->left += x;
	r->top += y;
	r->right += x;
	r->bottom += y;

	return true;
}


bool bhapi::win32_window_convert_window_to_client(HWND hWnd, RECT *wr)
{
	if(hWnd == NULL || wr == NULL) return false;

	RECT r;
	POINT pt;
	pt.x = 0;
	pt.y = 0;

	if(GetWindowRect(hWnd, &r) == 0) return false;
	if(ClientToScreen(hWnd, &pt) == 0) return false;

	int xoffset = pt.x - r.left;
	int yoffset = pt.y - r.top;

	wr->left -= xoffset;
	wr->top -= yoffset;
	wr->right -= xoffset;
	wr->bottom -= yoffset;

	return true;
}


EWin32GraphicsWindow::EWin32GraphicsWindow(EWin32GraphicsEngine *win32Engine,  __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
	: BGraphicsWindow(), win32Window(NULL),
	  fLook((bhapi::window_look)0), fFeel((bhapi::window_feel)0), fActivateWhenShown(false), hbrBackground(NULL),
	  fEngine(NULL), fRequestWin(NULL), fRequestAsyncWin(NULL), WMBHAPI_MESSAGE(0)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	if(win32Engine == NULL) return;

	do {
		BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
		if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK ||
		   win32Engine->WMBHAPI_MESSAGE == 0 ||
		   win32Engine->win32RequestWin == NULL || win32Engine->win32RequestAsyncWin == NULL)
		{
			if(fRequestWin != NULL) CloseHandle(fRequestWin);
			if(fRequestAsyncWin != NULL) CloseHandle(fRequestAsyncWin);
			BHAPI_WARNING("[GRAPHICS]: %s --- Invalid engine or unable to duplicate handle.", __PRETTY_FUNCTION__);
			return;
		}

		WMBHAPI_MESSAGE = win32Engine->WMBHAPI_MESSAGE;
		fRequestWin = win32Engine->win32RequestWin;
		fRequestAsyncWin = win32Engine->win32RequestAsyncWin;
	} while(false);

	bhapi::rgb_color whiteColor = {255, 255, 255, 255};
	BGraphicsDrawable::SetBackgroundColor(whiteColor);

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_CREATE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = w;
	callback.h = h;
	callback.bkColor = whiteColor;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(successed == false || win32Window == NULL)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Unable to create window: \"%s\".",
			  __PRETTY_FUNCTION__, successed ? "win32Window = NULL" : "SendMessageA failed");
	}
	else
	{
		fEngine = win32Engine;
	}
}


inline LONG _bhapi_get_window_style_ex(bhapi::window_look look)
{
	LONG style;

	switch(look)
	{
		case B_BORDERED_WINDOW_LOOK:
			style = 0;
			break;

		case B_NO_BORDER_WINDOW_LOOK:
			style = WS_EX_TOOLWINDOW;
			break;

		case B_FLOATING_WINDOW_LOOK:
			style = WS_EX_TOOLWINDOW | WS_EX_APPWINDOW;
			break;

		case B_MODAL_WINDOW_LOOK:
			style = WS_EX_DLGMODALFRAME;
			break;

		default:
			style = WS_EX_WINDOWEDGE;
			break;
	}

	return style;
}


inline LONG _bhapi_get_window_style(bhapi::window_look look)
{
	LONG style;

	switch(look)
	{
		case B_BORDERED_WINDOW_LOOK:
			style = WS_POPUPWINDOW;
			break;

		case B_NO_BORDER_WINDOW_LOOK:
			style = WS_POPUP;
			break;

		case B_MODAL_WINDOW_LOOK:
//			style = WS_DLGFRAME;
			style = WS_CAPTION;
			break;

		case B_FLOATING_WINDOW_LOOK:
			style = WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
			break;

		default:
			style = WS_OVERLAPPEDWINDOW;
			break;
	}

	return style;
}


LRESULT _bhapi_create_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_CREATE_WINDOW || callback->win == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	LONG style = _bhapi_get_window_style(B_TITLED_WINDOW_LOOK);
	LONG styleEx = _bhapi_get_window_style_ex(B_TITLED_WINDOW_LOOK);

	RECT r;
	r.left = callback->x;
	r.top = callback->y;
	r.right = callback->x + (int)callback->w;
	r.bottom = callback->y + (int)callback->h;

	AdjustWindowRectEx(&r, style, FALSE, styleEx);

    if((callback->win->win32Window = CreateWindowExW(styleEx, MAKEINTATOM(win32Engine->win32RegisterClass), TEXT(""), style,
							r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1,
							NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL) return FALSE;

	callback->win->fLook = B_TITLED_WINDOW_LOOK;

	// FIXME: maybe 64-bit pointer
	SetWindowLong(callback->win->win32Window, 0, reinterpret_cast<long>(win32Engine));
	SetWindowLong(callback->win->win32Window, GWL_USERDATA, reinterpret_cast<long>(callback->win));

	return TRUE;
}


EWin32GraphicsWindow::~EWin32GraphicsWindow()
{
	if(fRequestWin != NULL)
	{
		bhapi::win32_gdi_callback_t callback;
		callback.command = WMBHAPI_MESSAGE_DESTROY_WINDOW;
		callback.win = this;

		bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
					       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

		if(!successed || win32Window != NULL)
			BHAPI_ERROR("[GRAPHICS]: %s --- Unable to destory window.", __PRETTY_FUNCTION__);
	}
}


LRESULT _bhapi_destroy_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_DESTROY_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	// FIXME: maybe 64-bit pointer
	SetWindowLong(callback->win->win32Window, GWL_USERDATA, 0);
	DestroyWindow(callback->win->win32Window);
	callback->win->win32Window = NULL;

	if(callback->win->hbrBackground)
	{
		DeleteObject(callback->win->hbrBackground);
		callback->win->hbrBackground = NULL;
	}

	return TRUE;
}


status_t 
EWin32GraphicsWindow::ContactTo(const BMessenger *msgr)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EWin32GraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	fMsgr = BMessenger();
	if(msgr) fMsgr = *msgr;

	return B_OK;
}


status_t 
EWin32GraphicsWindow::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	if(bkColor == BackgroundColor()) return B_OK;

	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SET_WINDOW_BACKGROUND;
	callback.win = this;
	callback.bkColor = bkColor;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(!successed) return B_ERROR;

	BGraphicsDrawable::SetBackgroundColor(bkColor);
	return B_OK;
}


LRESULT _bhapi_set_window_background(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_SET_WINDOW_BACKGROUND || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	HBRUSH hbrBackground = CreateSolidBrush(RGB(callback->bkColor.red, callback->bkColor.green, callback->bkColor.blue));
	if(hbrBackground == NULL) return FALSE;

	if(callback->win->hbrBackground) DeleteObject(callback->win->hbrBackground);
	callback->win->hbrBackground = hbrBackground;

	return TRUE;
}


status_t 
EWin32GraphicsWindow::SetLook(bhapi::window_look look)
{
	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SET_WINDOW_LOOK;
	callback.win = this;
	callback.look = look;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_set_window_look(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_SET_WINDOW_LOOK || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	BOOL visible = IsWindowVisible(callback->win->win32Window);

	if(callback->win->fLook != callback->look)
	{
		LONG style = _bhapi_get_window_style(callback->look);
		LONG styleEx = _bhapi_get_window_style_ex(callback->look);
		SetWindowLong(callback->win->win32Window, GWL_EXSTYLE, styleEx);
		SetWindowLong(callback->win->win32Window, GWL_STYLE, style);
		SetWindowPos(callback->win->win32Window, NULL, 0, 0, 0, 0,
			     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		callback->win->fLook = callback->look;

		if(visible)
		{
			SetWindowPos(callback->win->win32Window, NULL, 0, 0, 0, 0,
				     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
			SetWindowPos(callback->win->win32Window, NULL, 0, 0, 0, 0,
				     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
		}
	}

	return TRUE;
}


status_t 
EWin32GraphicsWindow::SetTitle(const char *title)
{
	if(win32Window == NULL) return B_ERROR;

	status_t status;
	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		unichar16*uTitle = bhapi::utf8_convert_to_unicode(title, -1);
		status = (SetWindowTextW(win32Window, (WCHAR*)uTitle) == 0 ? B_ERROR : B_OK);
		if(uTitle) free(uTitle);
	}
	else // Windows 95/98
	{
		char *aTitle = bhapi::win32_convert_utf8_to_active(title, -1);
		status = (SetWindowTextA(win32Window, aTitle) == 0 ? B_ERROR : B_OK);
		if(aTitle) free(aTitle);
	}

	return status;
}


status_t 
EWin32GraphicsWindow::SetWorkspaces(__be_uint32 workspaces)
{
	// don't support workspace
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::GetWorkspaces(__be_uint32 *workspaces)
{
	// don't support workspace
	if(workspaces) *workspaces = 0;
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::Iconify()
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_ICONIFY_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_iconify_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_ICONIFY_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, SW_MINIMIZE);
	callback->win->fActivateWhenShown = false;

	return TRUE;
}


status_t 
EWin32GraphicsWindow::Show()
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SHOW_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_show_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_SHOW_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, callback->win->fActivateWhenShown ? SW_SHOWNORMAL : SW_SHOWNA);
	callback->win->fActivateWhenShown = false;

	return TRUE;
}


status_t 
EWin32GraphicsWindow::Hide()
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_HIDE_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_hide_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_HIDE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, SW_HIDE);

	GUITHREADINFO info;
	info.cbSize = sizeof(GUITHREADINFO);
	GetGUIThreadInfo(win32Engine->win32ThreadID, &info);
	if(info.hwndCapture == callback->win->win32Window) ReleaseCapture();

	return TRUE;
}


status_t 
EWin32GraphicsWindow::Raise()
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_RAISE_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_raise_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_RAISE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	return(SetWindowPos(callback->win->win32Window, HWND_TOP, 0, 0, 0, 0,
			    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
}


status_t 
EWin32GraphicsWindow::Lower(BGraphicsWindow *frontWin)
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_LOWER_WINDOW;
	callback.win = this;
	callback.frontWin = cast_as(frontWin, EWin32GraphicsWindow);

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_lower_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_LOWER_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	if(callback->frontWin == NULL || callback->frontWin->win32Window == NULL)
		return(SetWindowPos(callback->win->win32Window, HWND_BOTTOM, 0, 0, 0, 0,
				    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
	else
		return(SetWindowPos(callback->win->win32Window, callback->frontWin->win32Window, 0, 0, 0, 0,
				    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
}


status_t 
EWin32GraphicsWindow::Activate(bool state)
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_ACTIVATE_WINDOW;
	callback.win = this;
	callback.activate_state = state;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_activate_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_ACTIVATE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	if(callback->activate_state)
	{
		if(IsWindowVisible(callback->win->win32Window) == 0)
		{
			callback->win->fActivateWhenShown = callback->activate_state;
			return TRUE;
		}
#if 0
		DWORD tidForeground = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
		__be_int8 otherThread = (tidForeground == win32Engine->win32ThreadID ? 0 : 1);
		BOOL retVal = FALSE;
		if(otherThread == 1) otherThread = (AttachThreadInput(tidForeground, win32Engine->win32ThreadID, TRUE) == 0 ? 2 : 1);
		if(otherThread <= 1) retVal = (SetForegroundWindow(callback->win->win32Window) == 0 ? FALSE : TRUE);
		if(otherThread == 1) AttachThreadInput(tidForeground, win32Engine->win32ThreadID, FALSE);
		if(retVal) retVal = (SetActiveWindow(callback->win->win32Window) == NULL ? (GetLastError() == 0) : TRUE);
		return retVal;
#endif
		return(SetActiveWindow(callback->win->win32Window) == NULL ? (GetLastError() == 0) : TRUE);
	}
	else
	{
		GUITHREADINFO info;
		info.cbSize = sizeof(GUITHREADINFO);
		GetGUIThreadInfo(win32Engine->win32ThreadID, &info);
		if(info.hwndCapture == callback->win->win32Window) ReleaseCapture();
		PostMessageA(callback->win->win32Window, WM_NCACTIVATE, FALSE, 0);
	}

	return TRUE;
}


status_t 
EWin32GraphicsWindow::GetActivatedState(bool *state) const
{
	if(fRequestAsyncWin == NULL || state == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GET_WINDOW_ACTIVATE_STATE;
	callback.win = (EWin32GraphicsWindow*)this;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	if(!successed) return B_ERROR;

	*state = callback.activate_state;

	return B_OK;
}


LRESULT _bhapi_get_window_activate_state(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_GET_WINDOW_ACTIVATE_STATE || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	bool activate_state = false;

	GUITHREADINFO info;
	info.cbSize = sizeof(GUITHREADINFO);
	if(!(GetGUIThreadInfo(win32Engine->win32ThreadID, &info) == 0 ||
	     info.hwndActive != callback->win->win32Window ||
	     GetForegroundWindow() != callback->win->win32Window)) activate_state = true;

	callback->activate_state = activate_state;

	return TRUE;
}


status_t 
EWin32GraphicsWindow::MoveTo(__be_int32 x,  __be_int32 y)
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_MOVE_RESIZE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = B_MAXUINT32;
	callback.h = B_MAXUINT32;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


status_t 
EWin32GraphicsWindow::ResizeTo(__be_uint32 w,  __be_uint32 h)
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_RESIZE_WINDOW;
	callback.win = this;
	callback.w = w;
	callback.h = h;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


status_t 
EWin32GraphicsWindow::MoveAndResizeTo(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
{
	if(fRequestAsyncWin == NULL) return B_ERROR;

	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_MOVE_RESIZE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = w;
	callback.h = h;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_move_resize_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   !(callback->command == WMBHAPI_MESSAGE_MOVE_RESIZE_WINDOW || callback->command == WMBHAPI_MESSAGE_RESIZE_WINDOW) ||
	   callback->win == NULL || callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	LONG style = _bhapi_get_window_style(callback->win->fLook);
	LONG styleEx = _bhapi_get_window_style_ex(callback->win->fLook);

	RECT r;
	r.left = callback->x;
	r.top = callback->y;
	r.right = callback->x + (int)callback->w;
	r.bottom = callback->y + (int)callback->h;

	AdjustWindowRectEx(&r, style, FALSE, styleEx);

	UINT flags = SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOZORDER;
	if(callback->w == B_MAXUINT32 || callback->h == B_MAXUINT32) flags |= SWP_NOSIZE;
	else if(callback->command == WMBHAPI_MESSAGE_RESIZE_WINDOW) flags |= SWP_NOMOVE;

	SetWindowPos(callback->win->win32Window, HWND_TOP, r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1, flags);

	return TRUE;
}


status_t 
EWin32GraphicsWindow::GrabMouse()
{
	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = true;
	callback.grab_state = true;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


status_t 
EWin32GraphicsWindow::UngrabMouse()
{
	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = true;
	callback.grab_state = false;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


status_t 
EWin32GraphicsWindow::GrabKeyboard()
{
	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = false;
	callback.grab_state = true;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


status_t 
EWin32GraphicsWindow::UngrabKeyboard()
{
	if(fRequestWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = false;
	callback.grab_state = false;

	bool successed = (SendMessageA(fRequestWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
}


LRESULT _bhapi_grab_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_GRAB_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	// TODO: keyboard grab
	if(callback->grab_mouse == false) return FALSE;

	if(callback->grab_state)
	{
		SetCapture(callback->win->win32Window);

		GUITHREADINFO info;
		info.cbSize = sizeof(GUITHREADINFO);
		GetGUIThreadInfo(win32Engine->win32ThreadID, &info);

		if(info.hwndCapture != callback->win->win32Window) return FALSE;
	}
	else
	{
		GUITHREADINFO info;
		info.cbSize = sizeof(GUITHREADINFO);
		GetGUIThreadInfo(win32Engine->win32ThreadID, &info);

		if(info.hwndCapture != callback->win->win32Window) return FALSE;

		if(ReleaseCapture() == 0) return FALSE;
	}

	return TRUE;
}


status_t 
EWin32GraphicsWindow::SetFlags(__be_uint32 flags)
{
	// TODO
	return B_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SET_WINDOW_FLAGS;
	callback.win = this;
	callback.flags = flags;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
#endif
}


status_t 
EWin32GraphicsWindow::SetFeel(bhapi::window_feel feel)
{
	// TODO
	return B_OK;
#if 0
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SET_WINDOW_FEEL;
	callback.win = this;
	callback.feel = feel;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
#endif
}


status_t 
EWin32GraphicsWindow::SetSizeLimits(__be_uint32 min_w,  __be_uint32 max_w,  __be_uint32 min_h,  __be_uint32 max_h)
{
	// TODO
	return B_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_SET_WINDOW_USIZE;
	callback.win = this;
	callback.min_w = min_w;
	callback.min_h = min_h;
	callback.max_w = max_w;
	callback.max_h = max_h;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? B_OK : B_ERROR);
#endif
}


LRESULT _bhapi_set_window_usize(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	// TODO
	return FALSE;
}


status_t 
EWin32GraphicsWindow::GetSizeLimits(__be_uint32 *min_w,  __be_uint32 *max_w,  __be_uint32 *min_h,  __be_uint32 *max_h)
{
	// TODO
	return B_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return B_ERROR;
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return B_ERROR;

	bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_GET_WINDOW_USIZE;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WMBHAPI_MESSAGE,
				       WMBHAPI_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(!successed) return B_ERROR;

	*min_w = callback.min_w;
	*min_h = callback.min_h;
	*max_w = callback.max_w;
	*max_h = callback.max_h;

	return B_OK;
#endif
}


LRESULT _bhapi_get_window_usize(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	// TODO
	return FALSE;
}


status_t 
EWin32GraphicsWindow::QueryMouse(__be_int32 *x,  __be_int32 *y,  __be_int32 *buttons)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::CopyTo(BGraphicsContext *dc,
			     BGraphicsDrawable *dstDrawable,
			      __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
			      __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
				  __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
				  __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokePoint(BGraphicsContext *dc,
				   __be_int32 x,  __be_int32 y)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokePoints(BGraphicsContext *dc,
				   const  __be_int32 *pts,  __be_int32 count)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokePoints_Colors(BGraphicsContext *dc,
					  const BList *ptsArrayLists,  __be_int32 arrayCount,
					  const bhapi::rgb_color *highColors)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokePoints_Alphas(BGraphicsContext *dc,
					  const  __be_int32 *pts, const  __be_uint8 *alpha,  __be_int32 count)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokeLine(BGraphicsContext *dc,
			          __be_int32 x0,  __be_int32 y0,  __be_int32 x1,  __be_int32 y1)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokePolygon(BGraphicsContext *dc,
				    const  __be_int32 *pts,  __be_int32 count, bool closed)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillPolygon(BGraphicsContext *dc,
				  const  __be_int32 *pts,  __be_int32 count)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokeRect(BGraphicsContext *dc,
			          __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillRect(BGraphicsContext *dc,
			        __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokeRects(BGraphicsContext *dc,
				  const  __be_int32 *rects,  __be_int32 count)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillRects(BGraphicsContext *dc,
			        const  __be_int32 *rects,  __be_int32 count)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillRegion(BGraphicsContext *dc,
			         const BRegion &region)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokeRoundRect(BGraphicsContext *dc,
				       __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillRoundRect(BGraphicsContext *dc,
				     __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::StrokeArc(BGraphicsContext *dc,
			         __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}


status_t 
EWin32GraphicsWindow::FillArc(BGraphicsContext *dc,
			       __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle)
{
	// TODO
	return B_ERROR;
}

#endif /* WIN32 */
