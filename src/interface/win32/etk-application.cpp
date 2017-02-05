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

#include "etk-win32gdi.h"

#include "./../kernel/Kernel.h>
#include "./../support/Locker.h>
#include "./../support/Autolock.h>
#include "./../support/String.h>
#include "./../interface/InterfaceDefs.h>
#include "./../app/Application.h>
#include "./../app/Clipboard.h>


static void etk_win32_clipboard_changed()
{
	char *str = NULL;

	if(OpenClipboard(NULL))
	{
		do {
			HANDLE clipText = GetClipboardData(GetVersion() < 0x80000000 ?
								CF_UNICODETEXT : CF_TEXT);
			if(clipText == NULL) break;
			if(GetVersion() < 0x80000000) // Windows NT/2000/XP
				str = e_unicode_convert_to_utf8((eunichar*)clipText, -1);
			else // Windows 95/98
				str = etk_win32_convert_active_to_utf8((char*)clipText, -1);
		} while(false);

		CloseClipboard();
	}

	if(str)
	{
		EString aStr(str);
		free(str);
		aStr.ReplaceAll("\r\n", "\n");

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
					clipMsg->AddData("text/plain", E_MIME_TYPE, (void*)aStr.String(), (size_t)aStr.Length());
					etk_clipboard.Commit();
				}
			}
			etk_clipboard.Unlock();
		}
	}
}


static e_filter_result etk_win32_clipboard_filter(EMessage *message, EHandler **target, EMessageFilter *filter)
{
	if(message->what != E_CLIPBOARD_CHANGED) return E_DISPATCH_MESSAGE;

	do
	{
		const char *text = NULL;
		ssize_t textLen = 0;

		EString str;
		EMessage *msg;

		etk_clipboard.Lock();
		if(!((msg = etk_clipboard.Data()) == NULL || msg->HasBool("etk:msg_from_gui")))
			msg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen);
		if(textLen > 0) str.SetTo(text, (eint32)textLen);
		etk_clipboard.Unlock();

		if(str.Length() <= 0) break;
		str.ReplaceAll("\n", "\r\n");

		UINT uFormat = CF_TEXT;
		HGLOBAL hMem = NULL;

		if(GetVersion() < 0x80000000) // Windows NT/2000/XP
		{
			eunichar *wStr = e_utf8_convert_to_unicode(str.String(), -1);
			if(wStr)
			{
				eint32 len = 0;
				for(eunichar *tmp = wStr; *tmp != 0; tmp++) len += 2;
				if((hMem = GlobalAlloc(GMEM_MOVEABLE, (size_t)len + 2)) != NULL)
				{
					void *addr = GlobalLock(hMem);
					bzero(addr, (size_t)len + 2);
					memcpy(addr, wStr, (size_t)len);
					GlobalUnlock(hMem);
					uFormat = CF_UNICODETEXT;
				}
				free(wStr);
			}
		}
		else // Windows 95/98
		{
			char *aStr = etk_win32_convert_utf8_to_active(str.String(), -1);
			if(aStr)
			{
				eint32 len = strlen(aStr);
				if((hMem = GlobalAlloc(GMEM_MOVEABLE, (size_t)len + 1)) != NULL)
				{
					void *addr = GlobalLock(hMem);
					bzero(addr, (size_t)len + 1);
					memcpy(addr, aStr, (size_t)len);
					GlobalUnlock(hMem);
				}
				free(aStr);
			}
		}

		if(hMem == NULL) break;

		if(OpenClipboard(NULL))
		{
			EmptyClipboard();
			SetClipboardData(uFormat, hMem);
			CloseClipboard();
		}

		GlobalFree(hMem);
	} while(false);

	return E_DISPATCH_MESSAGE;
}


#ifndef ETK_GRAPHICS_WIN32_BUILT_IN
extern "C" {
_EXPORT EGraphicsEngine* instantiate_graphics_engine()
#else
_IMPEXP_ETK EGraphicsEngine* etk_get_built_in_graphics_engine()
#endif
{
	return(new EWin32GraphicsEngine());
}
#ifndef ETK_GRAPHICS_WIN32_BUILT_IN
} // extern "C"
#endif


e_status_t
EWin32GraphicsEngine::InitCheck()
{
	EAutolock <EWin32GraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false ||
	   win32RegisterClass == 0 || win32ScreenHDC == NULL ||
	   win32ThreadID == 0 || win32RequestWin == NULL || win32RequestAsyncWin == NULL ||
	   WM_ETK_MESSAGE == 0 || win32DoQuit) return E_NO_INIT;
	return E_OK;
}


bool
EWin32GraphicsEngine::Lock()
{
	return fLocker.Lock();
}


void
EWin32GraphicsEngine::Unlock()
{
	fLocker.Unlock();
}


bool
EWin32GraphicsEngine::GetContactor(HWND hWnd, EMessenger *msgr)
{
	if(hWnd == NULL || msgr == NULL) return false;

	EAutolock <EWin32GraphicsEngine> autolock(this);

	// FIXME: maybe 64-bit pointer
	LONG data = GetWindowLong(hWnd, GWL_USERDATA);
	EWin32GraphicsWindow *win = (data != 0 ? reinterpret_cast<EWin32GraphicsWindow*>(data) : NULL);
	if(win == NULL)
	{
//		ETK_WARNING("[GRAPHICS]: %s --- Unable to get data with \"GetWindowLong\"!", __PRETTY_FUNCTION__);
		return false;
	}

	*msgr = win->fMsgr;

	return true;
}


EWin32GraphicsWindow*
EWin32GraphicsEngine::GetWin32Window(HWND hWnd)
{
	if(hWnd == NULL) return NULL;

	EAutolock <EWin32GraphicsEngine> autolock(this);

	LONG data = GetWindowLong(hWnd, GWL_USERDATA);
	EWin32GraphicsWindow *win = (data != 0 ? reinterpret_cast<EWin32GraphicsWindow*>(data) : NULL);
	return win;
}


LRESULT _etk_set_app_cursor(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL || callback->command != WM_ETK_MESSAGE_CHANGE_APP_CURSOR) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	HCURSOR newCursor = NULL;

	if(callback->data)
	{
		ECursor cursor(callback->data);
		if(cursor.ColorDepth() != 1) return FALSE;

		if(GetVersion() < 0x80000000) // Windows NT/2000/XP
		{
			const euint8 *bits = (const euint8*)cursor.Bits();
			const euint8 *mask = (const euint8*)cursor.Mask();

			euint8 *ANDmaskCursor = (euint8*)malloc((size_t)(mask - bits));
			euint8 *XORmaskCursor = (euint8*)malloc((size_t)(mask - bits));

			if(ANDmaskCursor != NULL && XORmaskCursor != NULL)
			{
				euint8 *ANDmask = ANDmaskCursor;
				euint8 *XORmask = XORmaskCursor;
				const euint8 *tmp = mask;
				for(; bits != tmp; bits++, mask++, ANDmask++, XORmask++)
				{
					*XORmask = ~(*bits) & (*mask);
					*ANDmask = ~(*mask);
				}

				newCursor = CreateCursor(win32Engine->win32Hinstance,
							 (int)cursor.SpotX(), (int)cursor.SpotY(),
							 (int)cursor.Width(), (int)cursor.Height(),
							 ANDmaskCursor, XORmaskCursor);
			}

			if(ANDmaskCursor) free(ANDmaskCursor);
			if(XORmaskCursor) free(XORmaskCursor);
		}
		else // Windows 95/98
		{
			int w = GetSystemMetrics(SM_CXCURSOR), h = GetSystemMetrics(SM_CYCURSOR);

			euint8 *ANDmaskCursor = (euint8*)malloc(w * h / 8);
			euint8 *XORmaskCursor = (euint8*)malloc(w * h / 8);

			if(w % 8 == 0 && cursor.Width() % 8 == 0 && ANDmaskCursor != NULL && XORmaskCursor != NULL)
			{
				memset(ANDmaskCursor, ~0, w * h / 8);
				memset(XORmaskCursor, 0, w * h / 8);

				eint32 n = 0;
				for(eint32 j = (h - (int)cursor.Height()) / 2; j < h && n < (eint32)cursor.Height(); j++, n++)
				{
					if(j < 0) continue;

					const euint8 *__bits = (const euint8*)cursor.Bits() + n * (eint32)cursor.Width() / 8;
					const euint8 *__mask = (const euint8*)cursor.Mask() + n * (eint32)cursor.Width() / 8;
					euint8 *__ANDmask = ANDmaskCursor + j * w / 8;
					euint8 *__XORmask = XORmaskCursor + j * w / 8;

					eint32 m = 0;

					for(eint32 i = (w - (int)cursor.Width()) / 2; i < w && m < (eint32)cursor.Width(); i += 8, m += 8)
					{
						if(i < 0) continue;

						const euint8 *bits = __bits + m / 8;
						const euint8 *mask = __mask + m / 8;
						euint8 *ANDmask = __ANDmask + i / 8;
						euint8 *XORmask = __XORmask + i / 8;

						*XORmask = ~(*bits) & (*mask);
						*ANDmask = ~(*mask);
					}
				}

				newCursor = CreateCursor(win32Engine->win32Hinstance,
							 (w - (int)cursor.Width()) / 2 + (int)cursor.SpotX(),
							 (h - (int)cursor.Height()) / 2 + (int)cursor.SpotY(),
							 w, h, ANDmaskCursor, XORmaskCursor);
			}

			if(ANDmaskCursor) free(ANDmaskCursor);
			if(XORmaskCursor) free(XORmaskCursor);
		}

		if(newCursor == NULL) return FALSE;

//		ETK_DEBUG("[GRAPHICS]: SetCursor");
	}
	else
	{
//		ETK_DEBUG("[GRAPHICS]: HideCursor");
	}

	SetCursor(newCursor);
	if(win32Engine->win32Cursor) DestroyCursor(win32Engine->win32Cursor);
	win32Engine->win32Cursor = newCursor;

	return TRUE;
}


extern LRESULT _etk_create_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_destroy_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_iconify_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_show_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_hide_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_move_resize_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_set_window_look(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_activate_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_get_window_activate_state(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_raise_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_lower_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_set_window_background(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_set_window_usize(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_get_window_usize(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_grab_window(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);

extern LRESULT _etk_create_pixmap(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_destroy_pixmap(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_resize_pixmap(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_draw_pixmap(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_draw_epixmap(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);

extern LRESULT _etk_stroke_point(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_points(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_points_color(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_points_alpha(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_line(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_rect(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_rects(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_round_rect(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_arc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_stroke_polygon(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_rect(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_rects(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_round_rect(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_arc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_polygon(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_fill_region(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);

extern LRESULT _etk_create_font(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_destroy_font(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_font_string_width(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_font_get_height(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_font_render_string(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_create_font_tmp_dc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);
extern LRESULT _etk_destroy_font_tmp_dc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback);

#ifndef GET_X_LPARAM
	#define GET_X_LPARAM(a)	((int)((short)LOWORD(a)))
#endif

#ifndef GET_Y_LPARAM
	#define GET_Y_LPARAM(a)	((int)((short)HIWORD(a)))
#endif

#undef ETK_GDI32_REQUEST_DEBUG
#define CLICK_TIMEOUT 200000


static bool etk_process_win32_event(EWin32GraphicsEngine *win32Engine, MSG *winMsg, LRESULT *retResult = NULL)
{
	if(win32Engine == NULL) return FALSE;

	bool handled = true;
	e_bigtime_t currentTime = e_real_time_clock_usecs();
	EMessenger etkWinMsgr;

//	ETK_DEBUG("[GRAPHICS]: Window Event Received.");

	if(winMsg->message == win32Engine->WM_ETK_MESSAGE &&
	   (winMsg->hwnd == win32Engine->win32RequestWin || winMsg->hwnd == win32Engine->win32RequestAsyncWin))
	{
//		ETK_DEBUG("[GRAPHICS]: Receive a ETK request");

		LRESULT result = FALSE;

		etk_win32_gdi_callback_t *callback = (etk_win32_gdi_callback_t*)winMsg->lParam;

		if(!(callback == NULL || winMsg->wParam != WM_ETK_MESSAGE_APP))
		{
			switch(callback->command)
			{
				case WM_ETK_MESSAGE_CHANGE_APP_CURSOR:
					result = _etk_set_app_cursor(win32Engine, callback);
					break;

				default:
					break;
			}
		}
		else if(!(callback == NULL || winMsg->wParam != WM_ETK_MESSAGE_WINDOW))
		{
			switch(callback->command)
			{
				case WM_ETK_MESSAGE_CREATE_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_CREATE_WINDOW");
#endif
						result = _etk_create_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DESTROY_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DESTROY_WINDOW");
#endif
						result = _etk_destroy_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_ICONIFY_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_ICONIFY_WINDOW");
#endif
						result = _etk_iconify_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_SHOW_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_SHOW_WINDOW");
#endif
						result = _etk_show_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_HIDE_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_HIDE_WINDOW");
#endif
						result = _etk_hide_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW:
				case WM_ETK_MESSAGE_RESIZE_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW");
#endif
						result = _etk_move_resize_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_SET_WINDOW_LOOK:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_SET_WINDOW_LOOK");
#endif
						result = _etk_set_window_look(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_ACTIVATE_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_ACTIVATE_WINDOW");
#endif
						result = _etk_activate_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_GET_WINDOW_ACTIVATE_STATE:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_WINDOW_ACTIVATE_STATE");
#endif
						result = _etk_get_window_activate_state(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_RAISE_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_RAISE_WINDOW");
#endif
						result = _etk_raise_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_LOWER_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_LOWER_WINDOW");
#endif
						result = _etk_lower_window(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_SET_WINDOW_BACKGROUND:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_SET_WINDOW_BACKGROUND");
#endif
						result = _etk_set_window_background(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_SET_WINDOW_USIZE:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_SET_WINDOW_USIZE");
#endif
						result = _etk_set_window_usize(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_GET_WINDOW_USIZE:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_GET_WINDOW_USIZE");
#endif
						result = _etk_get_window_usize(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_GRAB_WINDOW:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_GRAB_WINDOW");
#endif
						result = _etk_grab_window(win32Engine, callback);
					}
					break;

				default:
					break;
			}
		}
		else if(!(callback == NULL || winMsg->wParam != WM_ETK_MESSAGE_PIXMAP))
		{
			switch(callback->command)
			{
				case WM_ETK_MESSAGE_CREATE_PIXMAP:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_CREATE_PIXMAP");
#endif
						result = _etk_create_pixmap(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DESTROY_PIXMAP:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DESTROY_PIXMAP");
#endif
						result = _etk_destroy_pixmap(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_RESIZE_PIXMAP:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_RESIZE_PIXMAP");
#endif
						result = _etk_resize_pixmap(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DRAW_PIXMAP:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DRAW_PIXMAP");
#endif
						result = _etk_draw_pixmap(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DRAW_EPIXMAP:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DRAW_EPIXMAP");
#endif
						result = _etk_draw_epixmap(win32Engine, callback);
					}
					break;

				default:
					break;
			}
		}
		else if(!(callback == NULL || winMsg->wParam != WM_ETK_MESSAGE_FONT))
		{
			switch(callback->command)
			{
				case WM_ETK_MESSAGE_CREATE_FONT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_CREATE_FONT");
#endif
						result = _etk_create_font(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DESTROY_FONT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DESTROY_FONT");
#endif
						result = _etk_destroy_font(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FONT_STRING_WIDTH:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FONT_STRING_WIDTH");
#endif
						result = _etk_font_string_width(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FONT_GET_HEIGHT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FONT_GET_HEIGHT");
#endif
						result = _etk_font_get_height(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FONT_RENDER_STRING:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FONT_RENDER_STRING");
#endif
						result = _etk_font_render_string(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_CREATE_FONT_TMP_DC:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_CREATE_FONT_TMP_DC");
#endif
						result = _etk_create_font_tmp_dc(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_DESTROY_FONT_TMP_DC:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_DESTROY_FONT_TMP_DC");
#endif
						result = _etk_destroy_font_tmp_dc(win32Engine, callback);
					}
					break;

				default:
					break;
			}
		}
		else if(!(callback == NULL || winMsg->wParam != WM_ETK_MESSAGE_DRAWING))
		{
			switch(callback->command)
			{
				case WM_ETK_MESSAGE_STROKE_POINT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_POINT");
#endif
						result = _etk_stroke_point(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_POINTS:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_POINTS");
#endif
						result = _etk_stroke_points(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_POINTS_COLOR:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_POINTS_COLOR");
#endif
						result = _etk_stroke_points_color(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_POINTS_ALPHA:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_POINTS_ALPHA");
#endif
						result = _etk_stroke_points_alpha(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_LINE:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_LINE");
#endif
						result = _etk_stroke_line(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_RECT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_RECT");
#endif
						result = _etk_stroke_rect(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_RECTS:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_RECTS");
#endif
						result = _etk_stroke_rects(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_ROUND_RECT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_ROUND_RECT");
#endif
						result = _etk_stroke_round_rect(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_ARC:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_ARC");
#endif
						result = _etk_stroke_arc(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_STROKE_POLYGON:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_STROKE_POLYGON");
#endif
						result = _etk_stroke_polygon(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_RECT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_RECT");
#endif
						result = _etk_fill_rect(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_RECTS:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_RECTS");
#endif
						result = _etk_fill_rects(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_ROUND_RECT:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_ROUND_RECT");
#endif
						result = _etk_fill_round_rect(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_ARC:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_ARC");
#endif
						result = _etk_fill_arc(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_POLYGON:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_POLYGON");
#endif
						result = _etk_fill_polygon(win32Engine, callback);
					}
					break;

				case WM_ETK_MESSAGE_FILL_REGION:
					{
#ifdef ETK_GDI32_REQUEST_DEBUG
						ETK_DEBUG("[GRAPHICS]: Receive a WM_ETK_MESSAGE_FILL_REGION");
#endif
						result = _etk_fill_region(win32Engine, callback);
					}
					break;

				default:
					break;
			}
		}

		if(retResult) *retResult = result;
	}
	else if(winMsg->message == WM_QUIT)
	{
		etk_app->PostMessage(E_QUIT_REQUESTED);
	}
	else if(win32Engine->GetContactor(winMsg->hwnd, &etkWinMsgr) == false || etkWinMsgr.IsValid() == false)
	{
		handled = false;

		switch(winMsg->message)
		{
			case WM_DESTROY:
				{
					if(winMsg->hwnd != win32Engine->win32RequestAsyncWin) break;
					ChangeClipboardChain(win32Engine->win32RequestAsyncWin, win32Engine->win32NextClipboardViewer);
					win32Engine->win32NextClipboardViewer = NULL;
				}
				break;

			case WM_CHANGECBCHAIN:
				{
					if(winMsg->hwnd != win32Engine->win32RequestAsyncWin) break;

					handled = true;
					if(retResult) *retResult = 0;

					if((HWND)winMsg->wParam == win32Engine->win32NextClipboardViewer)
						win32Engine->win32NextClipboardViewer = (HWND)winMsg->lParam;
					else if(win32Engine->win32NextClipboardViewer != NULL)
						SendMessageA(win32Engine->win32NextClipboardViewer,
							     winMsg->message, winMsg->wParam, winMsg->lParam);
				}
				break;

			case WM_DRAWCLIPBOARD:
				{
					if(winMsg->hwnd != win32Engine->win32RequestAsyncWin) break;

					handled = true;
					etk_win32_clipboard_changed();
				}
				break;

			default:
				break;
		}
	}
	else
	{
		if(retResult) *retResult = 0;

		EMessage message;
		message.AddBool("etk:msg_from_gui", true);
		message.AddInt64("when", currentTime);

		switch(winMsg->message)
		{
			case WM_CLOSE:
				{
					message.what = E_QUIT_REQUESTED;
					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_ERASEBKGND:
				{
					win32Engine->Lock();

					EWin32GraphicsWindow *win = win32Engine->GetWin32Window(winMsg->hwnd);
					if(win == NULL)
					{
						win32Engine->Unlock();
						break;
					}

					HRGN hrgn = CreateRectRgn(0, 0, 1, 1);
					if(hrgn == NULL)
					{
						win32Engine->Unlock();
						break;
					}
					handled = true;
					if(retResult) *retResult = 1;

					int status = GetUpdateRgn(winMsg->hwnd, hrgn, TRUE);
					if(!(status == NULLREGION || status == ERROR))
					{
						HDC hdc = (HDC)winMsg->wParam;
						HBRUSH hbrs = win->hbrBackground;
						if(hbrs == NULL) hbrs = (HBRUSH)GetStockObject(WHITE_BRUSH);
						FillRgn(hdc, hrgn, hbrs);
					}

					DeleteObject(hrgn);
					win32Engine->Unlock();
				}
				break;

			case WM_PAINT:
				{
					ERect rect;
					RECT r;

					win32Engine->Lock();
					if(GetUpdateRect(winMsg->hwnd, &r, FALSE) != 0)
					{
						rect.left = (float)r.left;
						rect.top = (float)r.top;
						rect.right = (float)r.right;
						rect.bottom = (float)r.bottom;
					}
					ValidateRect(winMsg->hwnd, NULL);
					win32Engine->Unlock();

					if(!rect.IsValid()) break;

					message.what = _UPDATE_;
					message.AddRect("etk:frame", rect);

					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_SHOWWINDOW:
				{
					// TODO: minimize send when needed
					handled = false;

					message.what = E_MINIMIZED;
					message.AddBool("minimize", (winMsg->wParam == TRUE ? false : true));

					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_MOVING:
			case WM_MOVE:
			case WM_SIZING:
			case WM_SIZE:
				{
					if(winMsg->message == WM_SIZE)
					{
						handled = false;

						if(winMsg->wParam == SIZE_MINIMIZED) break;
						message.what = E_WINDOW_RESIZED;
						message.AddFloat("width", (float)((short)LOWORD(winMsg->lParam) - 1));
						message.AddFloat("height", (float)((short)HIWORD(winMsg->lParam) - 1));
					}
					else if(winMsg->message == WM_MOVE)
					{
						float xPos = (float)((short)LOWORD(winMsg->lParam));
						float yPos = (float)((short)HIWORD(winMsg->lParam));

						message.what = E_WINDOW_MOVED;
						message.AddPoint("where", EPoint(xPos, yPos));

						handled = false;
					}
					else if(winMsg->message == WM_MOVING)
					{
						if(retResult) *retResult = TRUE;

						RECT *r = (RECT*)(winMsg->lParam);
						message.what = E_WINDOW_MOVED;

						RECT wr;
						int left = 0, top = 0;

						win32Engine->Lock();
						GetWindowRect(winMsg->hwnd, &wr);
						etk_win32_window_convert_to_screen(winMsg->hwnd, &left, &top);
						win32Engine->Unlock();

						left += r->left - wr.left;
						top += r->top - wr.top;
						message.AddPoint("where", EPoint((float)left, (float)top));
					}
					else if(winMsg->message == WM_SIZING)
					{
						if(retResult) *retResult = TRUE;

						RECT *r = (RECT*)(winMsg->lParam);
						message.what = E_WINDOW_RESIZED;

						RECT wr, cr;
						int left = 0, top = 0, width, height;

						win32Engine->Lock();
						GetWindowRect(winMsg->hwnd, &wr);
						GetClientRect(winMsg->hwnd, &cr);
						etk_win32_window_convert_to_screen(winMsg->hwnd, &left, &top);
						win32Engine->Unlock();

						width = cr.right; height = cr.bottom;
						left += r->left - wr.left;
						top += r->top - wr.top;
						width += (r->left - r->right) - (wr.left - wr.right);
						height += (r->bottom - r->top) - (wr.bottom - wr.top);

						message.AddPoint("where", EPoint((float)left, (float)top));
						message.AddFloat("width", (float)width);
						message.AddFloat("height", (float)height);
					}

					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_MOUSEACTIVATE:
			case WM_NCACTIVATE:
			case WM_ACTIVATE:
				{
					handled = false;

					if(winMsg->message == WM_MOUSEACTIVATE)
					{
						EWin32GraphicsWindow *win = NULL;
						win32Engine->Lock();
						if(!((win = win32Engine->GetWin32Window(winMsg->hwnd)) == NULL ||
						      win->fLook != E_NO_BORDER_WINDOW_LOOK))
						{
							win32Engine->Unlock();
							handled = true;
							if(retResult) *retResult = MA_NOACTIVATE;
							break;
						}
						win32Engine->Unlock();
					}

					message.what = E_WINDOW_ACTIVATED;
					etkWinMsgr.SendMessage(&message);

					if(winMsg->message == WM_ACTIVATE)
					{
						message.what = E_MINIMIZED;
						message.AddBool("minimize", (HIWORD(winMsg->wParam) != 0 ? true : false));
						etkWinMsgr.SendMessage(&message);
					}
				}
				break;

#ifdef _WIN32_WINNT
			case WM_MOUSEWHEEL:
				{
					short zDelta = GET_WHEEL_DELTA_WPARAM(winMsg->wParam);

					message.what = E_MOUSE_WHEEL_CHANGED;
					message.AddFloat("etk:wheel_delta_y", zDelta > 0 ? -1.f : 1.f);

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);
				}
				break;
#endif

			case WM_MOUSEMOVE:
				{
					int xPos = GET_X_LPARAM(winMsg->lParam);
					int yPos = GET_Y_LPARAM(winMsg->lParam);
					int xScreenPos = xPos;
					int yScreenPos = yPos;

					win32Engine->Lock();
					if(win32Engine->win32PrevMouseMovedWin == etkWinMsgr &&
					   win32Engine->win32PrevMouseMovedX == xPos &&
					   win32Engine->win32PrevMouseMovedY == yPos)
					{
						win32Engine->Unlock();
						break;
					}
					win32Engine->win32PrevMouseMovedWin = etkWinMsgr;
					win32Engine->win32PrevMouseMovedX = xPos;
					win32Engine->win32PrevMouseMovedY = yPos;
					etk_win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
					win32Engine->Unlock();

					eint32 buttons = 0;
					if(winMsg->wParam & MK_LBUTTON) buttons += 1;
					if(winMsg->wParam & MK_RBUTTON) buttons += 2;
					if(winMsg->wParam & MK_MBUTTON) buttons += 3;

					message.what = E_MOUSE_MOVED;
					message.AddInt32("buttons", buttons);
					message.AddPoint("where", EPoint((float)xPos, (float)yPos));
					message.AddPoint("screen_where", EPoint((float)xScreenPos, (float)yScreenPos));

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
				{
					int xPos = GET_X_LPARAM(winMsg->lParam);
					int yPos = GET_Y_LPARAM(winMsg->lParam);
					int xScreenPos = xPos;
					int yScreenPos = yPos;
					eint32 clicks;

					win32Engine->Lock();
					if(win32Engine->win32PrevMouseMovedWin == etkWinMsgr)
					{
						win32Engine->win32PrevMouseMovedX = -1;
						win32Engine->win32PrevMouseMovedY = -1;
					}
					if(currentTime - win32Engine->win32PrevMouseDownTime <= CLICK_TIMEOUT)
						clicks = (win32Engine->win32PrevMouseDownCount += 1);
					else
						clicks = win32Engine->win32PrevMouseDownCount = 1;
					win32Engine->win32PrevMouseDownTime = currentTime;
					etk_win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
					win32Engine->Unlock();

					eint32 button = 0;
					if(winMsg->message == WM_LBUTTONDOWN) button = 1;
					else if(winMsg->message == WM_MBUTTONDOWN) button = 2;
					else if(winMsg->message == WM_RBUTTONDOWN) button = 3;

					eint32 buttons = 0;
					if(winMsg->wParam & MK_LBUTTON) buttons += 1;
					if(winMsg->wParam & MK_MBUTTON) buttons += 2;
					if(winMsg->wParam & MK_RBUTTON) buttons += 3;

					message.what = E_MOUSE_DOWN;
					message.AddInt32("button", button);
					message.AddInt32("buttons", buttons);
					message.AddInt32("clicks", clicks);
					message.AddPoint("where", EPoint((float)xPos, (float)yPos));
					message.AddPoint("screen_where", EPoint((float)xScreenPos, (float)yScreenPos));

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
				{
					int xPos = GET_X_LPARAM(winMsg->lParam);
					int yPos = GET_Y_LPARAM(winMsg->lParam);

					int xScreenPos = xPos;
					int yScreenPos = yPos;

					win32Engine->Lock();
					etk_win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
					win32Engine->Unlock();

					eint32 button = 0;
					if(winMsg->message == WM_LBUTTONUP) button = 1;
					else if(winMsg->message == WM_MBUTTONUP) button = 2;
					else if(winMsg->message == WM_RBUTTONUP) button = 3;

					eint32 buttons = 0;
					if(winMsg->wParam & MK_LBUTTON) buttons += 1;
					if(winMsg->wParam & MK_MBUTTON) buttons += 2;
					if(winMsg->wParam & MK_RBUTTON) buttons += 3;

					message.what = E_MOUSE_UP;
					message.AddInt32("button", button);
					message.AddInt32("buttons", buttons);
					message.AddPoint("where", EPoint((float)xPos, (float)yPos));
					message.AddPoint("screen_where", EPoint((float)xScreenPos, (float)yScreenPos));

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);
				}
				break;

			case WM_KEYDOWN:
			case WM_KEYUP:
				{
					// TODO: other keys
					BYTE keyState[256];
					if(GetKeyboardState(keyState) == 0) {handled = false; break;}
					UINT scanCode = (UINT)((winMsg->lParam >> 16) & 0xff);

					message.AddInt32("key", (eint32)scanCode);
					if(winMsg->message == WM_KEYDOWN)
						message.AddInt32("etk:key_repeat", (eint32)(winMsg->lParam & 0xffff));

//					ETK_DEBUG("[GRAPHICS]: %s: %d",
//						  winMsg->message == WM_KEYDOWN ? "KEYDOWN" : "KEYUP",
//						  (eint32)winMsg->wParam);

					eint32 modifiers = 0;
					if((keyState[VK_SHIFT] >> 4) > 0) modifiers |= E_SHIFT_KEY;
					if((keyState[VK_MENU] >> 4) > 0) modifiers |= E_COMMAND_KEY;
					if((keyState[VK_CONTROL] >> 4) > 0) modifiers |= E_CONTROL_KEY;
					if((keyState[VK_LSHIFT] >> 4) > 0) modifiers |= E_LEFT_SHIFT_KEY | E_SHIFT_KEY;
					if((keyState[VK_RSHIFT] >> 4) > 0) modifiers |= E_RIGHT_SHIFT_KEY | E_SHIFT_KEY;
					if((keyState[VK_LMENU] >> 4) > 0) modifiers |= E_LEFT_COMMAND_KEY | E_COMMAND_KEY;
					if((keyState[VK_RMENU] >> 4) > 0) modifiers |= E_RIGHT_COMMAND_KEY | E_COMMAND_KEY;
					if((keyState[VK_LCONTROL] >> 4) > 0) modifiers |= E_LEFT_CONTROL_KEY | E_CONTROL_KEY;
					if((keyState[VK_RCONTROL] >> 4) > 0) modifiers |= E_RIGHT_CONTROL_KEY | E_CONTROL_KEY;
					if((keyState[VK_LWIN] >> 4) > 0) modifiers |= E_LEFT_OPTION_KEY | E_OPTION_KEY;
					if((keyState[VK_RWIN] >> 4) > 0) modifiers |= E_RIGHT_OPTION_KEY | E_OPTION_KEY;
					if((keyState[VK_APPS] >> 4) > 0) modifiers |= E_MENU_KEY;
					if((keyState[VK_CAPITAL] & 0x0f) > 0) modifiers |= E_CAPS_LOCK;
					if((keyState[VK_SCROLL] & 0x0f) > 0) modifiers |= E_SCROLL_LOCK;
					if((keyState[VK_NUMLOCK] & 0x0f) > 0) modifiers |= E_NUM_LOCK;
					if(winMsg->wParam >= VK_F1 && winMsg->wParam <= VK_F12)
					{
						message.ReplaceInt32("key", winMsg->wParam - VK_F1 + E_F1_KEY);
						modifiers |= E_FUNCTIONS_KEY;
					}
					else if(winMsg->wParam == VK_PRINT)
					{
						message.ReplaceInt32("key", E_PRINT_KEY);
						modifiers |= E_FUNCTIONS_KEY;
					}
					else if(winMsg->wParam == VK_SCROLL)
					{
						message.ReplaceInt32("key", E_SCROLL_KEY);
						modifiers |= E_FUNCTIONS_KEY;
					}
					else if(winMsg->wParam == VK_PAUSE)
					{
						message.ReplaceInt32("key", E_PAUSE_KEY);
						modifiers |= E_FUNCTIONS_KEY;
					}
					message.AddInt32("modifiers", modifiers);

					char keybuffer[3];
					int keynum = 0;

					bzero(keybuffer, sizeof(keybuffer));
					keyState[VK_CONTROL] = keyState[VK_LCONTROL] = keyState[VK_RCONTROL] = 0x0f;
					keyState[VK_MENU] = keyState[VK_LMENU] = keyState[VK_RMENU] = 0x0f;
					keyState[VK_LWIN] = keyState[VK_RWIN] = keyState[VK_APPS] = 0x0f;
					keynum = ToAscii(winMsg->wParam, scanCode, keyState, (WORD*)keybuffer, 0);
					if(keynum < 0) keynum = 0;

					switch(winMsg->wParam)
					{
						case VK_TAB:
							keybuffer[0] = E_TAB;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_LEFT:
							keybuffer[0] = E_LEFT_ARROW;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_RIGHT:
							keybuffer[0] = E_RIGHT_ARROW;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_UP:
							keybuffer[0] = E_UP_ARROW;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_DOWN:
							keybuffer[0] = E_DOWN_ARROW;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_PRIOR:
							keybuffer[0] = E_PAGE_UP;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_NEXT:
							keybuffer[0] = E_PAGE_DOWN;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_HOME:
							keybuffer[0] = E_HOME;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_END:
							keybuffer[0] = E_END;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_INSERT:
							keybuffer[0] = E_INSERT;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_DELETE:
							keybuffer[0] = E_DELETE;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_BACK:
							keybuffer[0] = E_BACKSPACE;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_ESCAPE:
							keybuffer[0] = E_ESCAPE;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_RETURN:
							keybuffer[0] = E_ENTER;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_SPACE:
							keybuffer[0] = E_SPACE;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						case VK_F1:
						case VK_F2:
						case VK_F3:
						case VK_F4:
						case VK_F5:
						case VK_F6:
						case VK_F7:
						case VK_F8:
						case VK_F9:
						case VK_F10:
						case VK_F11:
						case VK_F12:
						case VK_PRINT:
						case VK_SCROLL:
						case VK_PAUSE:
							keybuffer[0] = E_FUNCTION_KEY;
							keybuffer[1] = '\0';
							keynum = 1;
							break;

						default:
							break;
					}

					message.what = (winMsg->message == WM_KEYDOWN ? E_KEY_DOWN : E_KEY_UP);
					if(keynum > 0)
					{
						if(keynum == 1) message.AddInt8("byte", keybuffer[0]);
						message.AddString("bytes", keybuffer);
					}

					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);

					bool dealed = true;

					switch(winMsg->wParam)
					{
						case VK_SHIFT:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~E_SHIFT_KEY;
							else
								modifiers |= E_SHIFT_KEY;
							break;
						case VK_MENU:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~E_COMMAND_KEY;
							else
								modifiers |= E_COMMAND_KEY;
							break;
						case VK_CONTROL:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~E_CONTROL_KEY;
							else
								modifiers |= E_CONTROL_KEY;
							break;
						case VK_LWIN:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~(E_LEFT_OPTION_KEY | E_OPTION_KEY);
							else
								modifiers |= (E_LEFT_OPTION_KEY | E_OPTION_KEY);
							break;
						case VK_RWIN: modifiers |= E_RIGHT_OPTION_KEY | E_OPTION_KEY; break;
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~(E_RIGHT_OPTION_KEY | E_OPTION_KEY);
							else
								modifiers |= (E_RIGHT_OPTION_KEY | E_OPTION_KEY);
							break;
						case VK_APPS:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~E_MENU_KEY;
							else
								modifiers |= E_MENU_KEY;
							break;
						case VK_F1:
						case VK_F2:
						case VK_F3:
						case VK_F4:
						case VK_F5:
						case VK_F6:
						case VK_F7:
						case VK_F8:
						case VK_F9:
						case VK_F10:
						case VK_F11:
						case VK_F12:
						case VK_PRINT:
						case VK_PAUSE:
							if(winMsg->message == WM_KEYDOWN)
								modifiers &= ~E_FUNCTIONS_KEY;
							else
								modifiers |= E_FUNCTIONS_KEY;
							break;
						case VK_CAPITAL:
							if(modifiers & E_CAPS_LOCK) modifiers &= ~E_CAPS_LOCK;
							else modifiers |= E_CAPS_LOCK;
							break;
						case VK_SCROLL:
							if(modifiers & E_SCROLL_LOCK) modifiers &= ~E_SCROLL_LOCK;
							else modifiers |= E_SCROLL_LOCK;
							break;
						case VK_NUMLOCK:
							if(modifiers & E_NUM_LOCK) modifiers &= ~E_NUM_LOCK;
							else modifiers |= E_NUM_LOCK;
							break;

						default:
							dealed = false;
							break;
					}

					if(dealed)
					{
						if((modifiers & E_LEFT_SHIFT_KEY) ||
						   (modifiers & E_RIGHT_SHIFT_KEY)) modifiers |= E_SHIFT_KEY;
						if((modifiers & E_LEFT_CONTROL_KEY) ||
						   (modifiers & E_RIGHT_CONTROL_KEY)) modifiers |= E_CONTROL_KEY;
						if((modifiers & E_LEFT_COMMAND_KEY) ||
						   (modifiers & E_RIGHT_COMMAND_KEY)) modifiers |= E_COMMAND_KEY;
						if((modifiers & E_LEFT_OPTION_KEY) ||
						   (modifiers & E_RIGHT_OPTION_KEY)) modifiers |= E_OPTION_KEY;

						message.what = E_MODIFIERS_CHANGED;
						message.RemoveInt32("key");
						message.RemoveInt32("etk:key_repeat");
						message.RemoveInt8("byte");
						message.RemoveString("bytes");
						message.RemoveInt32("raw_char");
						message.AddInt32("etk:old_modifiers", modifiers);

						etkWinMsgr.SendMessage(&message);
					}
				}
				break;

			case WM_IME_COMPOSITION:
				{
					handled = false;
					if(!(winMsg->lParam & GCS_RESULTSTR)) break;

					HIMC imc = ImmGetContext(winMsg->hwnd);
					if(!imc) break;

					DWORD nChars = ImmGetCompositionString(imc, GCS_RESULTSTR, NULL, 0);
					char *str = (nChars > 0 ? (char*)malloc(nChars + 1) : NULL);

					if(str != NULL)
					{
						bzero(str, nChars + 1);
						ImmGetCompositionString(imc, GCS_RESULTSTR, str, nChars + 1);
						char *uStr = etk_win32_convert_active_to_utf8(str, -1);
						free(str);
						str = uStr;
					}

					if(str == NULL || *str == 0)
					{
						if(str) free(str);
						ImmReleaseContext(winMsg->hwnd, imc);
						break;
					}

					ETK_DEBUG("[GRAPHICS]: %s --- received input method(%s).", __PRETTY_FUNCTION__, str);
					message.what = E_KEY_DOWN;
					message.AddString("bytes", str);
					message.AddMessenger("etk:msg_for_target", etkWinMsgr);
					etkWinMsgr = EMessenger(etk_app);
					etkWinMsgr.SendMessage(&message);
					message.what = E_KEY_UP;
					etkWinMsgr.SendMessage(&message);

					free(str);
					ImmReleaseContext(winMsg->hwnd, imc);
				}
				break;

			case WM_SETCURSOR:
				{
					POINT pt;
					RECT r;

					win32Engine->Lock();
					if(!(!etkWinMsgr.IsValid() ||
					     !GetCursorPos(&pt) ||
					     !etk_win32_window_get_rect(winMsg->hwnd, &r) ||
					     !PtInRect(&r, pt)))
					{
						if(retResult) *retResult = TRUE;
						SetCursor(win32Engine->win32Cursor);
					}
					else
					{
						handled = false;
					}
					win32Engine->Unlock();
				}
				break;

			default:
				handled = false;
		}
	}

	return handled;
}


static LRESULT CALLBACK _win32_WndProc_(HWND hWnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	bool handled = false;
	LRESULT result = 0;

	LONG data = GetWindowLong(hWnd, 0);
	EWin32GraphicsEngine *win32Engine = (data != 0 ? reinterpret_cast<EWin32GraphicsEngine*>(data) : NULL);

	if(win32Engine != NULL)
	{
		MSG winMsg;
		winMsg.hwnd = hWnd;
		winMsg.message = umsg;
		winMsg.wParam = wParam;
		winMsg.lParam = lParam;

		handled = etk_process_win32_event(win32Engine, &winMsg, &result); // Process Win32 GDI32 Event
	}

	if(!handled) result = DefWindowProc(hWnd, umsg, wParam, lParam);

	return result;
}


static e_status_t etk_graphics_request_async_task(void *arg)
{
	EWin32GraphicsEngine *win32Engine = (EWin32GraphicsEngine*)arg;

	win32Engine->Lock();

	if(win32Engine->win32Hinstance == NULL || win32Engine->win32RegisterClass == 0 ||
	   (win32Engine->win32RequestAsyncWin = CreateWindowEx(0, MAKEINTATOM(win32Engine->win32RegisterClass),
							       "etk_gdi32_request_async_win", WS_DISABLED, 0, 0, 1, 1,
							       NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL)
	{
		if(win32Engine->win32RequestAsyncWin != NULL) DestroyWindow(win32Engine->win32RequestAsyncWin);
		win32Engine->win32RequestAsyncWin = NULL;

		etk_release_sem_etc(win32Engine->fRequestAsyncSem, 2, 0);

		win32Engine->Unlock();

		return E_ERROR;
	}

	SetWindowLong(win32Engine->win32RequestAsyncWin, 0, reinterpret_cast<long>(win32Engine));
	win32Engine->win32NextClipboardViewer = SetClipboardViewer(win32Engine->win32RequestAsyncWin);

	etk_release_sem(win32Engine->fRequestAsyncSem);

	win32Engine->Unlock();

	MSG winMsg;

	ETK_DEBUG("[GRAPHICS]: Enter Win32 GDI32 request-async task...");

	etk_win32_clipboard_changed();

	while(true)
	{
		if(GetMessage(&winMsg, NULL, 0, 0) == -1)
			ETK_ERROR("[GRAPHICS]: %s --- Win32 GDI32 operate error!", __PRETTY_FUNCTION__);

		win32Engine->Lock();
		if(win32Engine->win32DoQuit)
		{
			win32Engine->Unlock();
			break;
		}
		win32Engine->Unlock(); // for not dead lock within proccessing

		DispatchMessageA(&winMsg);

		win32Engine->Lock();
		if(win32Engine->win32DoQuit)
		{
			win32Engine->Unlock();
			break;
		}
		win32Engine->Unlock();
	}

	win32Engine->Lock();

	// Do some clean...
	DestroyWindow(win32Engine->win32RequestAsyncWin);
	win32Engine->win32RequestAsyncWin = NULL;

	win32Engine->Unlock();

	ETK_DEBUG("[GRAPHICS]: Win32 GDI32 request-async task quited.");

	return E_OK;
}


static e_status_t etk_graphics_request_task(void *arg)
{
	EWin32GraphicsEngine *win32Engine = (EWin32GraphicsEngine*)arg;

	win32Engine->Lock();

	win32Engine->win32ThreadID = GetCurrentThreadId();

	WNDCLASSEX wcApp;
	wcApp.lpszClassName = etk_app->Name();
	wcApp.hInstance = win32Engine->win32Hinstance;
	wcApp.lpfnWndProc = _win32_WndProc_;
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.hIcon = 0;
	wcApp.hIconSm = 0;
	wcApp.lpszMenuName = 0;
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcApp.style = CS_CLASSDC;
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 4;
	wcApp.cbSize = sizeof(wcApp);

	if((win32Engine->win32RegisterClass = RegisterClassEx(&wcApp)) == 0 ||
	   (win32Engine->WM_ETK_MESSAGE = RegisterWindowMessage("WM_ETK_MESSAGE")) == 0 ||
	   (win32Engine->win32RequestWin = CreateWindowEx(0, MAKEINTATOM(win32Engine->win32RegisterClass),
							  "etk_gdi32_request_win", WS_DISABLED, 0, 0, 1, 1,
							  NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL ||
	   (win32Engine->win32ScreenHDC = CreateDC("DISPLAY", NULL, NULL, NULL)) == NULL)
	{
		ETK_WARNING("[GRAPHICS]: %s --- Unable to initalize the GDI32(\"%s\" failed, error code: %u).",
			    __PRETTY_FUNCTION__,
			    win32Engine->win32RegisterClass == 0 ? "RegisterClassEx" : (
			    win32Engine->WM_ETK_MESSAGE == 0 ? "RegisterWindowMessage" : (
			    win32Engine->win32RequestWin == NULL ? "CreateWindowEx" : "CreateDC")), GetLastError());
		if(win32Engine->win32RequestWin != NULL) DestroyWindow(win32Engine->win32RequestWin);
		if(win32Engine->win32RegisterClass != 0)
			UnregisterClass(MAKEINTATOM(win32Engine->win32RegisterClass), win32Engine->win32Hinstance);

		win32Engine->win32RequestWin = NULL;
		win32Engine->win32RegisterClass = 0;
		win32Engine->win32ThreadID = 0;

		etk_release_sem_etc(win32Engine->fRequestSem, 2, 0);

		win32Engine->Unlock();

		return E_ERROR;
	}

	SetWindowLong(win32Engine->win32RequestWin, 0, reinterpret_cast<long>(win32Engine));

	etk_release_sem(win32Engine->fRequestSem);

	win32Engine->Unlock();

	MSG winMsg;

	ETK_DEBUG("[GRAPHICS]: Enter Win32 GDI32 main task...");

	while(true)
	{
		if(GetMessage(&winMsg, NULL, 0, 0) == -1)
			ETK_ERROR("[GRAPHICS]: %s --- Win32 GDI32 operate error!", __PRETTY_FUNCTION__);

		win32Engine->Lock();
		if(win32Engine->win32DoQuit)
		{
			win32Engine->Unlock();
			break;
		}
		win32Engine->Unlock(); // for not dead lock within proccessing

		TranslateMessage(&winMsg); // for input method
		DispatchMessageA(&winMsg);

		win32Engine->Lock();
		if(win32Engine->win32DoQuit)
		{
			win32Engine->Unlock();
			break;
		}
		win32Engine->Unlock();
	}

	win32Engine->Lock();

	// Do some clean, like unregister wcApp etc...

	GUITHREADINFO info;
	info.cbSize = sizeof(GUITHREADINFO);
	GetGUIThreadInfo(win32Engine->win32ThreadID, &info);
	if(GetWindowThreadProcessId(info.hwndCapture, NULL) == GetCurrentThreadId()) ReleaseCapture();

	DeleteDC(win32Engine->win32ScreenHDC);
	win32Engine->win32ScreenHDC = NULL;

	DestroyWindow(win32Engine->win32RequestWin);
	win32Engine->win32RequestWin = NULL;

	if(win32Engine->win32Cursor != NULL)
	{
		DestroyCursor(win32Engine->win32Cursor);
		win32Engine->win32Cursor = NULL;
	}

	UnregisterClass(MAKEINTATOM(win32Engine->win32RegisterClass), win32Engine->win32Hinstance);
	win32Engine->win32RegisterClass = 0;

	win32Engine->win32ThreadID = 0;

	win32Engine->Unlock();

	ETK_DEBUG("[GRAPHICS]: Win32 GDI32 request task quited.");

	return E_OK;
}


EWin32GraphicsEngine::EWin32GraphicsEngine()
	: EGraphicsEngine(),
	  win32Hinstance(NULL), win32RegisterClass(0), win32ScreenHDC(NULL),
	  win32ThreadID(0), win32RequestWin(NULL), win32RequestAsyncWin(NULL), WM_ETK_MESSAGE(0),
	  win32NextClipboardViewer(NULL), win32Cursor(NULL),
	  win32PrevMouseDownTime(0), win32PrevMouseDownCount(0),
	  win32DoQuit(false),
	  fRequestSem(NULL), fRequestAsyncSem(NULL),
	  fRequestThread(NULL), fRequestAsyncThread(NULL), fClipboardFilter(NULL)
{
	win32Hinstance = (HINSTANCE)GetModuleHandle(NULL);
}


EWin32GraphicsEngine::~EWin32GraphicsEngine()
{
	DestroyFonts();
	Cancel();
}


e_status_t
EWin32GraphicsEngine::Initalize()
{
	EMessageFilter *clipboardFilter = new EMessageFilter(E_CLIPBOARD_CHANGED, etk_win32_clipboard_filter);
	etk_app->Lock();
	etk_app->AddFilter(clipboardFilter);
	etk_app->Unlock();

	Lock();

	if(win32Hinstance == NULL)
	{
		Unlock();
		ETK_WARNING("[GRAPHICS]: %s --- win32Hinstance == NULL", __PRETTY_FUNCTION__);

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	if(InitCheck() == E_OK)
	{
		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	if((fRequestSem = etk_create_sem(0, NULL)) == NULL || (fRequestAsyncSem = etk_create_sem(0, NULL)) == NULL)
	{
		if(fRequestSem) etk_delete_sem(fRequestSem);
		if(fRequestAsyncSem) etk_delete_sem(fRequestAsyncSem);
		fRequestSem = NULL; fRequestAsyncSem = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	win32DoQuit = false;

	if((fRequestThread = etk_create_thread(etk_graphics_request_task, E_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
	   (fRequestAsyncThread = etk_create_thread(etk_graphics_request_async_task, E_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL)
	{
		ETK_WARNING("[GRAPHICS]: %s --- Unable to create thread for GDI32.", __PRETTY_FUNCTION__);

		etk_delete_sem(fRequestSem);
		etk_delete_sem(fRequestAsyncSem);
		fRequestSem = NULL; fRequestAsyncSem = NULL;

		if(fRequestThread) etk_delete_thread(fRequestThread);
		if(fRequestAsyncThread) etk_delete_thread(fRequestAsyncThread);
		fRequestThread = NULL; fRequestAsyncThread = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	if(etk_resume_thread(fRequestThread) != E_OK)
	{
		ETK_WARNING("[GRAPHICS]: %s --- Unable to resume GDI32 requst task.", __PRETTY_FUNCTION__);
		etk_delete_sem(fRequestSem);
		etk_delete_sem(fRequestAsyncSem);
		fRequestSem = NULL; fRequestAsyncSem = NULL;

		etk_delete_thread(fRequestThread);
		etk_delete_thread(fRequestAsyncThread);
		fRequestThread = NULL; fRequestAsyncThread = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	Unlock();

	eint64 count = 0;

	etk_acquire_sem(fRequestSem);
	if(etk_get_sem_count(fRequestSem, &count) != E_OK || count > 0)
	{
		ETK_WARNING("[GRAPHICS]: %s --- GDI32 requst task return a error.", __PRETTY_FUNCTION__);

		Lock();

		etk_delete_sem(fRequestSem);
		etk_delete_sem(fRequestAsyncSem);
		fRequestSem = NULL; fRequestAsyncSem = NULL;

		etk_delete_thread(fRequestThread);
		etk_delete_thread(fRequestAsyncThread);
		fRequestThread = NULL; fRequestAsyncThread = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	Lock();

	etk_delete_sem(fRequestSem); fRequestSem = NULL;

	if(etk_resume_thread(fRequestAsyncThread) != E_OK)
	{
		ETK_WARNING("[GRAPHICS]: %s --- Unable to resume GDI32 requst-async task.", __PRETTY_FUNCTION__);
		etk_delete_sem(fRequestAsyncSem);
		fRequestAsyncSem = NULL;

		etk_delete_thread(fRequestAsyncThread);
		fRequestAsyncThread = NULL;

		PostMessageA(win32RequestWin, WM_QUIT, 0, 0);

		Unlock();

		e_status_t status;
		etk_wait_for_thread(fRequestThread, &status);

		Lock();

		etk_delete_thread(fRequestThread);
		fRequestThread = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	fClipboardFilter = clipboardFilter;

	Unlock();

	etk_acquire_sem(fRequestAsyncSem);
	if(etk_get_sem_count(fRequestAsyncSem, &count) != E_OK || count > 0)
	{
		ETK_WARNING("[GRAPHICS]: %s --- GDI32 requst-async task return a error.", __PRETTY_FUNCTION__);

		Lock();

		etk_delete_sem(fRequestAsyncSem);
		fRequestAsyncSem = NULL;

		PostMessageA(win32RequestWin, WM_QUIT, 0, 0);

		fClipboardFilter = NULL;

		Unlock();

		e_status_t status;
		etk_wait_for_thread(fRequestThread, &status);

		Lock();

		etk_delete_thread(fRequestThread);
		etk_delete_thread(fRequestAsyncThread);
		fRequestThread = NULL; fRequestAsyncThread = NULL;

		Unlock();

		etk_app->Lock();
		etk_app->RemoveFilter(clipboardFilter);
		etk_app->Unlock();
		delete clipboardFilter;
		return E_ERROR;
	}

	Lock();
	etk_delete_sem(fRequestAsyncSem);
	fRequestAsyncSem = NULL;
	Unlock();

	ETK_DEBUG("[GRAPHICS]: %s --- GDI32 Initalized.", __PRETTY_FUNCTION__);

	return E_OK;
}


void
EWin32GraphicsEngine::Cancel()
{
	EMessageFilter *clipboardFilter = NULL;

	Lock();

	if(InitCheck() == E_OK)
	{
		win32DoQuit = true;

		// Send a pending event to EventLoop in order to quit
		if(win32RequestWin) PostMessageA(win32RequestWin, WM_QUIT, 0, 0);
		if(win32RequestAsyncWin) PostMessageA(win32RequestAsyncWin, WM_QUIT, 0, 0);

		Unlock();

		e_status_t status;
		etk_wait_for_thread(fRequestThread, &status);
		etk_wait_for_thread(fRequestAsyncThread, &status);

		Lock();

		etk_delete_thread(fRequestThread);
		etk_delete_thread(fRequestAsyncThread);
		fRequestThread = NULL; fRequestAsyncThread = NULL;

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
EWin32GraphicsEngine::CreateContext()
{
	return(new EGraphicsContext());
}


EGraphicsDrawable*
EWin32GraphicsEngine::CreatePixmap(euint32 w, euint32 h)
{
	return(new EWin32GraphicsDrawable(this, w, h));
}


EGraphicsWindow*
EWin32GraphicsEngine::CreateWindow(eint32 x, eint32 y, euint32 w, euint32 h)
{
	return(new EWin32GraphicsWindow(this, x, y, w, h));
}


e_status_t
EWin32GraphicsEngine::GetDesktopBounds(euint32 *w, euint32 *h)
{
	EAutolock <EWin32GraphicsEngine> autolock(this);

	if(InitCheck() != E_OK) return E_ERROR;

	if(w) *w = GetDeviceCaps(win32ScreenHDC, HORZRES);
	if(h) *h = GetDeviceCaps(win32ScreenHDC, VERTRES);

	return E_OK;
}


e_status_t
EWin32GraphicsEngine::GetCurrentWorkspace(euint32 *workspace)
{
	// don't support workspace
	if(workspace != NULL) *workspace = 0;
	return E_ERROR;
}


e_status_t
EWin32GraphicsEngine::SetCursor(const void *cursor_data)
{
	if(win32RequestWin == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_CHANGE_APP_CURSOR;
	callback.data = cursor_data;

	bool successed = (SendMessageA(win32RequestWin, WM_ETK_MESSAGE,
				       WM_ETK_MESSAGE_APP, (LPARAM)&callback) == (LRESULT)TRUE);

	return(successed ? E_OK : E_ERROR);
}


e_status_t
EWin32GraphicsEngine::GetDefaultCursor(ECursor *cursor)
{
	return E_ERROR;
}

