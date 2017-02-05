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
#ifdef WIN32

#include "etk-win32gdi.h"

#include "./../../kernel/Kernel.h"
#include "./../../support/Autolock.h"
#include "./../../app/Application.h"
#include "./../../support/String.h"
#include "./../../support/ClassInfo.h"


bool etk_win32_window_convert_to_screen(HWND hWnd, int *x, int *y)
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


bool etk_win32_window_get_rect(HWND hWnd, RECT *r)
{
	if(hWnd == NULL || r == NULL) return false;

	int x = 0, y = 0;

	if(GetClientRect(hWnd, r) == 0) return false;

	if(!etk_win32_window_convert_to_screen(hWnd, &x, &y)) return false;

	r->left += x;
	r->top += y;
	r->right += x;
	r->bottom += y;

	return true;
}


bool etk_win32_window_convert_window_to_client(HWND hWnd, RECT *wr)
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


EWin32GraphicsWindow::EWin32GraphicsWindow(EWin32GraphicsEngine *win32Engine, eint32 x, eint32 y, euint32 w, euint32 h)
	: EGraphicsWindow(), win32Window(NULL),
	  fLook((e_window_look)0), fFeel((e_window_feel)0), fActivateWhenShown(false), hbrBackground(NULL),
	  fEngine(NULL), fRequestWin(NULL), fRequestAsyncWin(NULL), WM_ETK_MESSAGE(0)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	if(win32Engine == NULL) return;

	do {
		EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
		if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK ||
		   win32Engine->WM_ETK_MESSAGE == 0 ||
		   win32Engine->win32RequestWin == NULL || win32Engine->win32RequestAsyncWin == NULL)
		{
			if(fRequestWin != NULL) CloseHandle(fRequestWin);
			if(fRequestAsyncWin != NULL) CloseHandle(fRequestAsyncWin);
			ETK_WARNING("[GRAPHICS]: %s --- Invalid engine or unable to duplicate handle.", __PRETTY_FUNCTION__);
			return;
		}

		WM_ETK_MESSAGE = win32Engine->WM_ETK_MESSAGE;
		fRequestWin = win32Engine->win32RequestWin;
		fRequestAsyncWin = win32Engine->win32RequestAsyncWin;
	} while(false);

	e_rgb_color whiteColor = {255, 255, 255, 255};
	EGraphicsDrawable::SetBackgroundColor(whiteColor);

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_CREATE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = w;
	callback.h = h;
	callback.bkColor = whiteColor;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(successed == false || win32Window == NULL)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Unable to create window: \"%s\".",
			  __PRETTY_FUNCTION__, successed ? "win32Window = NULL" : "SendMessageA failed");
	}
	else
	{
		fEngine = win32Engine;
	}
}


inline LONG _etk_get_window_style_ex(e_window_look look)
{
	LONG style;

	switch(look)
	{
		case E_BORDERED_WINDOW_LOOK:
			style = 0;
			break;

		case E_NO_BORDER_WINDOW_LOOK:
			style = WS_EX_TOOLWINDOW;
			break;

		case E_FLOATING_WINDOW_LOOK:
			style = WS_EX_TOOLWINDOW | WS_EX_APPWINDOW;
			break;

		case E_MODAL_WINDOW_LOOK:
			style = WS_EX_DLGMODALFRAME;
			break;

		default:
			style = WS_EX_WINDOWEDGE;
			break;
	}

	return style;
}


inline LONG _etk_get_window_style(e_window_look look)
{
	LONG style;

	switch(look)
	{
		case E_BORDERED_WINDOW_LOOK:
			style = WS_POPUPWINDOW;
			break;

		case E_NO_BORDER_WINDOW_LOOK:
			style = WS_POPUP;
			break;

		case E_MODAL_WINDOW_LOOK:
//			style = WS_DLGFRAME;
			style = WS_CAPTION;
			break;

		case E_FLOATING_WINDOW_LOOK:
			style = WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
			break;

		default:
			style = WS_OVERLAPPEDWINDOW;
			break;
	}

	return style;
}


LRESULT _etk_create_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_CREATE_WINDOW || callback->win == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	LONG style = _etk_get_window_style(E_TITLED_WINDOW_LOOK);
	LONG styleEx = _etk_get_window_style_ex(E_TITLED_WINDOW_LOOK);

	RECT r;
	r.left = callback->x;
	r.top = callback->y;
	r.right = callback->x + (int)callback->w;
	r.bottom = callback->y + (int)callback->h;

	AdjustWindowRectEx(&r, style, FALSE, styleEx);

	if((callback->win->win32Window = CreateWindowEx(styleEx, MAKEINTATOM(win32Engine->win32RegisterClass), "", style,
							r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1,
							NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL) return FALSE;

	callback->win->fLook = E_TITLED_WINDOW_LOOK;

	// FIXME: maybe 64-bit pointer
	SetWindowLong(callback->win->win32Window, 0, reinterpret_cast<long>(win32Engine));
	SetWindowLong(callback->win->win32Window, GWL_USERDATA, reinterpret_cast<long>(callback->win));

	return TRUE;
}


EWin32GraphicsWindow::~EWin32GraphicsWindow()
{
	if(fRequestWin != NULL)
	{
		etk_win32_gdi_callback_t callback;
		callback.command = WM_ETK_MESSAGE_DESTROY_WINDOW;
		callback.win = this;

		bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
					       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

		if(!successed || win32Window != NULL)
			ETK_ERROR("[GRAPHICS]: %s --- Unable to destory window.", __PRETTY_FUNCTION__);
	}
}


LRESULT _etk_destroy_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_DESTROY_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;
 
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


e_status_t
EWin32GraphicsWindow::ContactTo(const EMessenger *msgr)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EWin32GraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	fMsgr = EMessenger();
	if(msgr) fMsgr = *msgr;

	return E_OK;
}


e_status_t
EWin32GraphicsWindow::SetBackgroundColor(e_rgb_color bkColor)
{
	if(bkColor == BackgroundColor()) return E_OK;

	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SET_WINDOW_BACKGROUND;
	callback.win = this;
	callback.bkColor = bkColor;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(!successed) return E_ERROR;

	EGraphicsDrawable::SetBackgroundColor(bkColor);
	return E_OK;
}


LRESULT _etk_set_window_background(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_SET_WINDOW_BACKGROUND || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	HBRUSH hbrBackground = CreateSolidBrush(RGB(callback->bkColor.red, callback->bkColor.green, callback->bkColor.blue));
	if(hbrBackground == NULL) return FALSE;

	if(callback->win->hbrBackground) DeleteObject(callback->win->hbrBackground);
	callback->win->hbrBackground = hbrBackground;

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::SetLook(e_window_look look)
{
	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SET_WINDOW_LOOK;
	callback.win = this;
	callback.look = look;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_set_window_look(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_SET_WINDOW_LOOK || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	BOOL visible = IsWindowVisible(callback->win->win32Window);

	if(callback->win->fLook != callback->look)
	{
		LONG style = _etk_get_window_style(callback->look);
		LONG styleEx = _etk_get_window_style_ex(callback->look);
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


e_status_t
EWin32GraphicsWindow::SetTitle(const char *title)
{
	if(win32Window == NULL) return E_ERROR;

	e_status_t status;
	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		eunichar *uTitle = e_utf8_convert_to_unicode(title, -1);
		status = (SetWindowTextW(win32Window, (WCHAR*)uTitle) == 0 ? E_ERROR : E_OK);
		if(uTitle) free(uTitle);
	}
	else // Windows 95/98
	{
		char *aTitle = etk_win32_convert_utf8_to_active(title, -1);
		status = (SetWindowTextA(win32Window, aTitle) == 0 ? E_ERROR : E_OK);
		if(aTitle) free(aTitle);
	}

	return status;
}


e_status_t
EWin32GraphicsWindow::SetWorkspaces(euint32 workspaces)
{
	// don't support workspace
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::GetWorkspaces(euint32 *workspaces)
{
	// don't support workspace
	if(workspaces) *workspaces = 0;
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::Iconify()
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_ICONIFY_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_iconify_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_ICONIFY_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, SW_MINIMIZE);
	callback->win->fActivateWhenShown = false;

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::Show()
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SHOW_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_show_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_SHOW_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, callback->win->fActivateWhenShown ? SW_SHOWNORMAL : SW_SHOWNA);
	callback->win->fActivateWhenShown = false;

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::Hide()
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_HIDE_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_hide_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_HIDE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	ShowWindowAsync(callback->win->win32Window, SW_HIDE);

	GUITHREADINFO info;
	info.cbSize = sizeof(GUITHREADINFO);
	GetGUIThreadInfo(win32Engine->win32ThreadID, &info);
	if(info.hwndCapture == callback->win->win32Window) ReleaseCapture();

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::Raise()
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_RAISE_WINDOW;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_raise_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_RAISE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	return(SetWindowPos(callback->win->win32Window, HWND_TOP, 0, 0, 0, 0,
			    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
}


e_status_t
EWin32GraphicsWindow::Lower(EGraphicsWindow *frontWin)
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_LOWER_WINDOW;
	callback.win = this;
	callback.frontWin = e_cast_as(frontWin, EWin32GraphicsWindow);

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_lower_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_LOWER_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	if(callback->frontWin == NULL || callback->frontWin->win32Window == NULL)
		return(SetWindowPos(callback->win->win32Window, HWND_BOTTOM, 0, 0, 0, 0,
				    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
	else
		return(SetWindowPos(callback->win->win32Window, callback->frontWin->win32Window, 0, 0, 0, 0,
				    SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE) == 0 ? FALSE : TRUE);
}


e_status_t
EWin32GraphicsWindow::Activate(bool state)
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_ACTIVATE_WINDOW;
	callback.win = this;
	callback.activate_state = state;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_activate_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_ACTIVATE_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	if(callback->activate_state)
	{
		if(IsWindowVisible(callback->win->win32Window) == 0)
		{
			callback->win->fActivateWhenShown = callback->activate_state;
			return TRUE;
		}
#if 0
		DWORD tidForeground = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
		eint8 otherThread = (tidForeground == win32Engine->win32ThreadID ? 0 : 1);
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


e_status_t
EWin32GraphicsWindow::GetActivatedState(bool *state) const
{
	if(fRequestAsyncWin == NULL || state == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GET_WINDOW_ACTIVATE_STATE;
	callback.win = (EWin32GraphicsWindow*)this;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	if(!successed) return E_ERROR;

	*state = callback.activate_state;

	return E_OK;
}


LRESULT _etk_get_window_activate_state(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_GET_WINDOW_ACTIVATE_STATE || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	bool activate_state = false;

	GUITHREADINFO info;
	info.cbSize = sizeof(GUITHREADINFO);
	if(!(GetGUIThreadInfo(win32Engine->win32ThreadID, &info) == 0 ||
	     info.hwndActive != callback->win->win32Window ||
	     GetForegroundWindow() != callback->win->win32Window)) activate_state = true;

	callback->activate_state = activate_state;

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::MoveTo(eint32 x, eint32 y)
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = E_MAXUINT32;
	callback.h = E_MAXUINT32;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsWindow::ResizeTo(euint32 w, euint32 h)
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_RESIZE_WINDOW;
	callback.win = this;
	callback.w = w;
	callback.h = h;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsWindow::MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(fRequestAsyncWin == NULL) return E_ERROR;

	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW;
	callback.win = this;
	callback.x = x;
	callback.y = y;
	callback.w = w;
	callback.h = h;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_move_resize_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   !(callback->command == WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW || callback->command == WM_ETK_MESSAGE_RESIZE_WINDOW) ||
	   callback->win == NULL || callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	LONG style = _etk_get_window_style(callback->win->fLook);
	LONG styleEx = _etk_get_window_style_ex(callback->win->fLook);

	RECT r;
	r.left = callback->x;
	r.top = callback->y;
	r.right = callback->x + (int)callback->w;
	r.bottom = callback->y + (int)callback->h;

	AdjustWindowRectEx(&r, style, FALSE, styleEx);

	UINT flags = SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOACTIVATE | SWP_NOZORDER;
	if(callback->w == E_MAXUINT32 || callback->h == E_MAXUINT32) flags |= SWP_NOSIZE;
	else if(callback->command == WM_ETK_MESSAGE_RESIZE_WINDOW) flags |= SWP_NOMOVE;

	SetWindowPos(callback->win->win32Window, HWND_TOP, r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1, flags);

	return TRUE;
}


e_status_t
EWin32GraphicsWindow::GrabMouse()
{
	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = true;
	callback.grab_state = true;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsWindow::UngrabMouse()
{
	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = true;
	callback.grab_state = false;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsWindow::GrabKeyboard()
{
	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = false;
	callback.grab_state = true;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsWindow::UngrabKeyboard()
{
	if(fRequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GRAB_WINDOW;
	callback.win = this;
	callback.grab_mouse = false;
	callback.grab_state = false;

	bool successed = (SendMessageA(fRequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
}


LRESULT _etk_grab_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_GRAB_WINDOW || callback->win == NULL ||
	   callback->win->win32Window == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

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


e_status_t
EWin32GraphicsWindow::SetFlags(euint32 flags)
{
	// TODO
	return E_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SET_WINDOW_FLAGS;
	callback.win = this;
	callback.flags = flags;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
#endif
}


e_status_t
EWin32GraphicsWindow::SetFeel(e_window_feel feel)
{
	// TODO
	return E_OK;
#if 0
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SET_WINDOW_FEEL;
	callback.win = this;
	callback.feel = feel;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
#endif
}


e_status_t
EWin32GraphicsWindow::SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h)
{
	// TODO
	return E_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_SET_WINDOW_USIZE;
	callback.win = this;
	callback.min_w = min_w;
	callback.min_h = min_h;
	callback.max_w = max_w;
	callback.max_h = max_h;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);
	return(successed ? E_OK : E_ERROR);
#endif
}


LRESULT _etk_set_window_usize(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	// TODO
	return FALSE;
}


e_status_t
EWin32GraphicsWindow::GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h)
{
	// TODO
	return E_ERROR;
#if 0
	if(fRequestAsyncWin == NULL) return E_ERROR;
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_GET_WINDOW_USIZE;
	callback.win = this;

	bool successed = (SendMessageA(fRequestAsyncWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_WINDOW, (LPARAM)&callback) == (LRESULT)TRUE);

	if(!successed) return E_ERROR;

	*min_w = callback.min_w;
	*min_h = callback.min_h;
	*max_w = callback.max_w;
	*max_h = callback.max_h;

	return E_OK;
#endif
}


LRESULT _etk_get_window_usize(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	// TODO
	return FALSE;
}


e_status_t
EWin32GraphicsWindow::QueryMouse(eint32 *x, eint32 *y, eint32 *buttons)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::CopyTo(EGraphicsContext *dc,
			     EGraphicsDrawable *dstDrawable,
			     eint32 x, eint32 y, euint32 w, euint32 h,
			     eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
				 eint32 x, eint32 y, euint32 w, euint32 h,
				 eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokePoint(EGraphicsContext *dc,
				  eint32 x, eint32 y)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokePoints(EGraphicsContext *dc,
				   const eint32 *pts, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokePoints_Colors(EGraphicsContext *dc,
					  const EList *ptsArrayLists, eint32 arrayCount,
					  const e_rgb_color *highColors)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokePoints_Alphas(EGraphicsContext *dc,
					  const eint32 *pts, const euint8 *alpha, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokeLine(EGraphicsContext *dc,
			         eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokePolygon(EGraphicsContext *dc,
				    const eint32 *pts, eint32 count, bool closed)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillPolygon(EGraphicsContext *dc,
				  const eint32 *pts, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokeRect(EGraphicsContext *dc,
			         eint32 x, eint32 y, euint32 w, euint32 h)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillRect(EGraphicsContext *dc,
			       eint32 x, eint32 y, euint32 w, euint32 h)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokeRects(EGraphicsContext *dc,
				  const eint32 *rects, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillRects(EGraphicsContext *dc,
			        const eint32 *rects, eint32 count)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillRegion(EGraphicsContext *dc,
			         const ERegion &region)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokeRoundRect(EGraphicsContext *dc,
				      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillRoundRect(EGraphicsContext *dc,
				    eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::StrokeArc(EGraphicsContext *dc,
			        eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	// TODO
	return E_ERROR;
}


e_status_t
EWin32GraphicsWindow::FillArc(EGraphicsContext *dc,
			      eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	// TODO
	return E_ERROR;
}

#endif /* WIN32 */
