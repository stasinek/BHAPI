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
 * File: etk-application.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef WIN32

#include "etk-win32gdi.h"

#include "../../kernel/Kernel.h"
#include "../../kernel/Debug.h"
#include "../../support/Locker.h"
#include "../../support/Errors.h"
#include "../../support/Autolock.h"
#include "../../support/String.h"
#include "../../interface/InterfaceDefs.h"
#include "../../app/Application.h"
#include "../../app/AppDefs.h"
#include "../../app/Clipboard.h"
#include "../../app/Message.h"

static void bhapi::win32_clipboard_changed()
{
    char *str = NULL;

    if(OpenClipboard(NULL))
    {
        do
        {
            HANDLE clipText = GetClipboardData(GetVersion() < 0x80000000 ?
                                               CF_UNICODETEXT : CF_TEXT);
            if(clipText == NULL) break;
            if(GetVersion() < 0x80000000) // Windows NT/2000/XP
                str = bhapi::unicode_convert_to_utf8((b_unichar16*)clipText, -1);
            else // Windows 95/98
                str = bhapi::win32_convert_active_to_utf8((char*)clipText, -1);
        }
        while(false);

        CloseClipboard();
    }

    if(str)
    {
        BString aStr(str);
        free(str);
        aStr.ReplaceAll("\r\n", "\n");

        BMessage *clipMsg = NULL;
        if(bhapi::clipboard.Lock())
        {
            if((clipMsg = bhapi::clipboard.Data()) != NULL)
            {
                const char *text = NULL;
                b_size_t textLen = 0;
                if(clipMsg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen) == false ||
                        text == NULL || textLen != (b_size_t)aStr.Length() || aStr.Compare(text, (b_int32)textLen) != 0)
                {
                    bhapi::clipboard.Clear();
                    clipMsg->AddBool("BHAPI:msg_from_gui", true);
                    clipMsg->AddData("text/plain", B_MIME_TYPE, (void*)aStr.String(), (size_t)aStr.Length());
                    bhapi::clipboard.Commit();
                }
            }
            bhapi::clipboard.Unlock();
        }
    }
}


static b_filter_result bhapi::win32_clipboard_filter(BMessage *message, BHandler **target, BMessageFilter *filter)
{
    if(message->what != B_CLIPBOARD_CHANGED) return B_DISPATCH_MESSAGE;

    do
    {
        const char *text = NULL;
        b_size_t textLen = 0;

        BString str;
        BMessage *msg;

        bhapi::clipboard.Lock();
        if(!((msg = bhapi::clipboard.Data()) == NULL || msg->HasBool("BHAPI:msg_from_gui")))
            msg->FindData("text/plain", B_MIME_TYPE, (const void**)&text, &textLen);
        if(textLen > 0) str.SetTo(text, (b_int32)textLen);
        bhapi::clipboard.Unlock();

        if(str.Length() <= 0) break;
        str.ReplaceAll("\n", "\r\n");

        UINT uFormat = CF_TEXT;
        HGLOBAL hMem = NULL;

        if(GetVersion() < 0x80000000) // Windows NT/2000/XP
        {
            b_unichar16*wStr = bhapi::utf8_convert_to_unicode(str.String(), -1);
            if(wStr)
            {
                b_int32 len = 0;
                for(b_unichar16*tmp = wStr; *tmp != 0; tmp++) len += 2;
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
            char *aStr = bhapi::win32_convert_utf8_to_active(str.String(), -1);
            if(aStr)
            {
                b_int32 len = strlen(aStr);
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
    }
    while(false);

    return B_DISPATCH_MESSAGE;
}


#ifndef BHAPI_GRAPHICS_WIN32_BUILT_IN
extern "C" {
namespace bhapi {
EXPORT_BHAPI BGraphicsEngine* instantiate_graphics_engine()
#else
IMPEXP_BHAPI BGraphicsEngine* bhapi::get_built_in_graphics_engine()
#endif
    {
        return(new EWin32GraphicsEngine());
    }
} /* namespace */
#ifndef BHAPI_GRAPHICS_WIN32_BUILT_IN
} // extern "C"
#endif


b_status_t
EWin32GraphicsEngine::InitCheck()
{
    BAutolock <EWin32GraphicsEngine> autolock(this);
    if(autolock.IsLocked() == false ||
            win32RegisterClass == 0 || win32ScreenHDC == NULL ||
            win32ThreadID == 0 || win32RequestWin == NULL || win32RequestAsyncWin == NULL ||
            WM_BHAPI_MESSAGE == 0 || win32DoQuit) return B_NO_INIT;
    return B_OK;
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
EWin32GraphicsEngine::GetContactor(HWND hWnd, BMessenger *msgr)
{
    if(hWnd == NULL || msgr == NULL) return false;

    BAutolock <EWin32GraphicsEngine> autolock(this);

    // FIXME: maybe 64-bit pointer
    LONG data = GetWindowLong(hWnd, GWL_USERDATA);
    EWin32GraphicsWindow *win = (data != 0 ? reinterpret_cast<EWin32GraphicsWindow*>(data) : NULL);
    if(win == NULL)
    {
//      BHAPI_WARNING("[GRAPHICS]: %s --- Unable to get data with \"GetWindowLong\"!", __PRETTY_FUNCTION__);
        return false;
    }

    *msgr = win->fMsgr;

    return true;
}


EWin32GraphicsWindow*
EWin32GraphicsEngine::GetWin32Window(HWND hWnd)
{
    if(hWnd == NULL) return NULL;

    BAutolock <EWin32GraphicsEngine> autolock(this);

    LONG data = GetWindowLong(hWnd, GWL_USERDATA);
    EWin32GraphicsWindow *win = (data != 0 ? reinterpret_cast<EWin32GraphicsWindow*>(data) : NULL);
    return win;
}


LRESULT _bhapi_set_app_cursor(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
    if(win32Engine == NULL || callback == NULL || callback->command != WM_BHAPI_MESSAGE_CHANGE_APP_CURSOR) return FALSE;

    BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
    if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

    HCURSOR newCursor = NULL;

    if(callback->data)
    {
        BCursor cursor(callback->data);
        if(cursor.ColorDepth() != 1) return FALSE;

        if(GetVersion() < 0x80000000) // Windows NT/2000/XP
        {
            const b_uint8 *bits = (const b_uint8*)cursor.Bits();
            const b_uint8 *mask = (const b_uint8*)cursor.Mask();

            b_uint8 *ANDmaskCursor = (b_uint8*)malloc((size_t)(mask - bits));
            b_uint8 *XORmaskCursor = (b_uint8*)malloc((size_t)(mask - bits));

            if(ANDmaskCursor != NULL && XORmaskCursor != NULL)
            {
                b_uint8 *ANDmask = ANDmaskCursor;
                b_uint8 *XORmask = XORmaskCursor;
                const b_uint8 *tmp = mask;
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

            b_uint8 *ANDmaskCursor = (b_uint8*)malloc(w * h / 8);
            b_uint8 *XORmaskCursor = (b_uint8*)malloc(w * h / 8);

            if(w % 8 == 0 && cursor.Width() % 8 == 0 && ANDmaskCursor != NULL && XORmaskCursor != NULL)
            {
                memset(ANDmaskCursor, ~0, w * h / 8);
                memset(XORmaskCursor, 0, w * h / 8);

                b_int32 n = 0;
                for(b_int32 j = (h - (int)cursor.Height()) / 2; j < h && n < (b_int32)cursor.Height(); j++, n++)
                {
                    if(j < 0) continue;

                    const b_uint8 *__bits = (const b_uint8*)cursor.Bits() + n * (b_int32)cursor.Width() / 8;
                    const b_uint8 *__mask = (const b_uint8*)cursor.Mask() + n * (b_int32)cursor.Width() / 8;
                    b_uint8 *__ANDmask = ANDmaskCursor + j * w / 8;
                    b_uint8 *__XORmask = XORmaskCursor + j * w / 8;

                    b_int32 m = 0;

                    for(b_int32 i = (w - (int)cursor.Width()) / 2; i < w && m < (b_int32)cursor.Width(); i += 8, m += 8)
                    {
                        if(i < 0) continue;

                        const b_uint8 *bits = __bits + m / 8;
                        const b_uint8 *mask = __mask + m / 8;
                        b_uint8 *ANDmask = __ANDmask + i / 8;
                        b_uint8 *XORmask = __XORmask + i / 8;

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

//      BHAPI_DEBUG("[GRAPHICS]: SetCursor");
    }
    else
    {
//      BHAPI_DEBUG("[GRAPHICS]: HideCursor");
    }

    SetCursor(newCursor);
    if(win32Engine->win32Cursor) DestroyCursor(win32Engine->win32Cursor);
    win32Engine->win32Cursor = newCursor;

    return TRUE;
}


extern LRESULT _bhapi_create_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_destroy_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_iconify_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_show_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_hide_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_move_resize_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_set_window_look(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_activate_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_get_window_activate_state(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_raise_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_lower_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_set_window_background(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_set_window_usize(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_get_window_usize(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_grab_window(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);

extern LRESULT _bhapi_create_pixmap(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_destroy_pixmap(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_resize_pixmap(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_draw_pixmap(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_draw_epixmap(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);

extern LRESULT _bhapi_stroke_point(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_points(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_points_color(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_points_alpha(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_line(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_rect(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_rects(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_round_rect(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_arc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_stroke_polygon(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_rect(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_rects(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_round_rect(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_arc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_polygon(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_fill_region(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);

extern LRESULT _bhapi_create_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_destroy_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_font_string_width(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_font_get_height(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_font_render_string(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_creatbhapi::font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);
extern LRESULT _bhapi_destroy_font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback);

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(a) ((int)((short)LOWORD(a)))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(a) ((int)((short)HIWORD(a)))
#endif

#undef BHAPI_GDI32_REQUEST_DEBUG
#define CLICK_TIMEOUT 200000


static bool b_process_win32_event(EWin32GraphicsEngine *win32Engine, MSG *winMsg, LRESULT *retResult = NULL)
{
    if(win32Engine == NULL) return FALSE;

    bool handled = true;
    b_bigtime_t currentTime = b_real_time_clock_usecs();
    BMessenger etkWinMsgr;

//  BHAPI_DEBUG("[GRAPHICS]: Window Event Received.");

    if(winMsg->message == win32Engine->WM_BHAPI_MESSAGE &&
            (winMsg->hwnd == win32Engine->win32RequestWin || winMsg->hwnd == win32Engine->win32RequestAsyncWin))
    {
//      BHAPI_DEBUG("[GRAPHICS]: Receive a BHAPI request");

        LRESULT result = FALSE;

        bhapi::win32_gdi_callback_t *callback = (bhapi::win32_gdi_callback_t*)winMsg->lParam;

        if(!(callback == NULL || winMsg->wParam != WM_BHAPI_MESSAGE_APP))
        {
            switch(callback->command)
            {
            case WM_BHAPI_MESSAGE_CHANGE_APP_CURSOR:
                result = _bhapi_set_app_cursor(win32Engine, callback);
                break;

            default:
                break;
            }
        }
        else
            if(!(callback == NULL || winMsg->wParam != WM_BHAPI_MESSAGE_WINDOW))
            {
                switch(callback->command)
                {
                case WM_BHAPI_MESSAGE_CREATE_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_CREATE_WINDOW");
#endif
                    result = _bhapi_create_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_DESTROY_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DESTROY_WINDOW");
#endif
                    result = _bhapi_destroy_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_ICONIFY_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_ICONIFY_WINDOW");
#endif
                    result = _bhapi_iconify_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_SHOW_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_SHOW_WINDOW");
#endif
                    result = _bhapi_show_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_HIDE_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_HIDE_WINDOW");
#endif
                    result = _bhapi_hide_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_MOVE_RESIZE_WINDOW:
                case WM_BHAPI_MESSAGE_RESIZE_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_MOVE_RESIZE_WINDOW");
#endif
                    result = _bhapi_move_resize_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_SET_WINDOW_LOOK:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_SET_WINDOW_LOOK");
#endif
                    result = _bhapi_set_window_look(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_ACTIVATE_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_ACTIVATE_WINDOW");
#endif
                    result = _bhapi_activate_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_GET_WINDOW_ACTIVATE_STATE:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_WINDOW_ACTIVATE_STATE");
#endif
                    result = _bhapi_get_window_activate_state(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_RAISE_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_RAISE_WINDOW");
#endif
                    result = _bhapi_raise_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_LOWER_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_LOWER_WINDOW");
#endif
                    result = _bhapi_lower_window(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_SET_WINDOW_BACKGROUND:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_SET_WINDOW_BACKGROUND");
#endif
                    result = _bhapi_set_window_background(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_SET_WINDOW_USIZE:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_SET_WINDOW_USIZE");
#endif
                    result = _bhapi_set_window_usize(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_GET_WINDOW_USIZE:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_GET_WINDOW_USIZE");
#endif
                    result = _bhapi_get_window_usize(win32Engine, callback);
                }
                break;

                case WM_BHAPI_MESSAGE_GRAB_WINDOW:
                {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                    BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_GRAB_WINDOW");
#endif
                    result = _bhapi_grab_window(win32Engine, callback);
                }
                break;

                default:
                    break;
                }
            }
            else
                if(!(callback == NULL || winMsg->wParam != WM_BHAPI_MESSAGE_PIXMAP))
                {
                    switch(callback->command)
                    {
                    case WM_BHAPI_MESSAGE_CREATE_PIXMAP:
                    {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                        BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_CREATE_PIXMAP");
#endif
                        result = _bhapi_create_pixmap(win32Engine, callback);
                    }
                    break;

                    case WM_BHAPI_MESSAGE_DESTROY_PIXMAP:
                    {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                        BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DESTROY_PIXMAP");
#endif
                        result = _bhapi_destroy_pixmap(win32Engine, callback);
                    }
                    break;

                    case WM_BHAPI_MESSAGE_RESIZE_PIXMAP:
                    {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                        BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_RESIZE_PIXMAP");
#endif
                        result = _bhapi_resize_pixmap(win32Engine, callback);
                    }
                    break;

                    case WM_BHAPI_MESSAGE_DRAW_PIXMAP:
                    {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                        BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DRAW_PIXMAP");
#endif
                        result = _bhapi_draw_pixmap(win32Engine, callback);
                    }
                    break;

                    case WM_BHAPI_MESSAGE_DRAW_EPIXMAP:
                    {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                        BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DRAW_EPIXMAP");
#endif
                        result = _bhapi_draw_epixmap(win32Engine, callback);
                    }
                    break;

                    default:
                        break;
                    }
                }
                else
                    if(!(callback == NULL || winMsg->wParam != WM_BHAPI_MESSAGB_FONT))
                    {
                        switch(callback->command)
                        {
                        case WM_BHAPI_MESSAGE_CREATB_FONT:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_CREATB_FONT");
#endif
                            result = _bhapi_create_font(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGE_DESTROY_FONT:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DESTROY_FONT");
#endif
                            result = _bhapi_destroy_font(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGB_FONT_STRING_WIDTH:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGB_FONT_STRING_WIDTH");
#endif
                            result = _bhapi_font_string_width(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGB_FONT_GET_HEIGHT:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGB_FONT_GET_HEIGHT");
#endif
                            result = _bhapi_font_get_height(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGB_FONT_RENDER_STRING:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGB_FONT_RENDER_STRING");
#endif
                            result = _bhapi_font_render_string(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGE_CREATB_FONT_TMP_DC:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_CREATB_FONT_TMP_DC");
#endif
                            result = _bhapi_creatbhapi::font_tmp_dc(win32Engine, callback);
                        }
                        break;

                        case WM_BHAPI_MESSAGE_DESTROY_FONT_TMP_DC:
                        {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                            BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_DESTROY_FONT_TMP_DC");
#endif
                            result = _bhapi_destroy_font_tmp_dc(win32Engine, callback);
                        }
                        break;

                        default:
                            break;
                        }
                    }
                    else
                        if(!(callback == NULL || winMsg->wParam != WM_BHAPI_MESSAGE_DRAWING))
                        {
                            switch(callback->command)
                            {
                            case WM_BHAPI_MESSAGE_STROKE_POINT:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_POINT");
#endif
                                result = _bhapi_stroke_point(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_POINTS:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_POINTS");
#endif
                                result = _bhapi_stroke_points(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_POINTS_COLOR:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_POINTS_COLOR");
#endif
                                result = _bhapi_stroke_points_color(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_POINTS_ALPHA:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_POINTS_ALPHA");
#endif
                                result = _bhapi_stroke_points_alpha(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_LINE:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_LINE");
#endif
                                result = _bhapi_stroke_line(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_RECT:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_RECT");
#endif
                                result = _bhapi_stroke_rect(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_RECTS:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_RECTS");
#endif
                                result = _bhapi_stroke_rects(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_ROUND_RECT:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_ROUND_RECT");
#endif
                                result = _bhapi_stroke_round_rect(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_ARC:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_ARC");
#endif
                                result = _bhapi_stroke_arc(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_STROKE_POLYGON:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_STROKE_POLYGON");
#endif
                                result = _bhapi_stroke_polygon(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_RECT:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_RECT");
#endif
                                result = _bhapi_fill_rect(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_RECTS:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_RECTS");
#endif
                                result = _bhapi_fill_rects(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_ROUND_RECT:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_ROUND_RECT");
#endif
                                result = _bhapi_fill_round_rect(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_ARC:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_ARC");
#endif
                                result = _bhapi_fill_arc(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_POLYGON:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_POLYGON");
#endif
                                result = _bhapi_fill_polygon(win32Engine, callback);
                            }
                            break;

                            case WM_BHAPI_MESSAGE_FILL_REGION:
                            {
#ifdef BHAPI_GDI32_REQUEST_DEBUG
                                BHAPI_DEBUG("[GRAPHICS]: Receive a WM_BHAPI_MESSAGE_FILL_REGION");
#endif
                                result = _bhapi_fill_region(win32Engine, callback);
                            }
                            break;

                            default:
                                break;
                            }
                        }

        if(retResult) *retResult = result;
    }
    else
        if(winMsg->message == WM_QUIT)
        {
            bhapi::app->PostMessage(B_QUIT_REQUESTED);
        }
        else
            if(win32Engine->GetContactor(winMsg->hwnd, &etkWinMsgr) == false || etkWinMsgr.IsValid() == false)
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
                    else
                        if(win32Engine->win32NextClipboardViewer != NULL)
                            SendMessageA(win32Engine->win32NextClipboardViewer,
                                         winMsg->message, winMsg->wParam, winMsg->lParam);
                }
                break;

                case WM_DRAWCLIPBOARD:
                {
                    if(winMsg->hwnd != win32Engine->win32RequestAsyncWin) break;

                    handled = true;
                    bhapi::win32_clipboard_changed();
                }
                break;

                default:
                    break;
                }
            }
            else
            {
                if(retResult) *retResult = 0;

                BMessage message;
                message.AddBool("BHAPI:msg_from_gui", true);
                message.AddInt64("when", currentTime);

                switch(winMsg->message)
                {
                case WM_CLOSE:
                {
                    message.what = B_QUIT_REQUESTED;
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
                    BRect rect;
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
                    message.AddRect("BHAPI:frame", rect);

                    etkWinMsgr.SendMessage(&message);
                }
                break;

                case WM_SHOWWINDOW:
                {
                    // TODO: minimize send when needed
                    handled = false;

                    message.what = B_MINIMIZED;
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
                        message.what = B_WINDOW_RESIZED;
                        message.AddFloat("width", (float)((short)LOWORD(winMsg->lParam) - 1));
                        message.AddFloat("height", (float)((short)HIWORD(winMsg->lParam) - 1));
                    }
                    else
                        if(winMsg->message == WM_MOVE)
                        {
                            float xPos = (float)((short)LOWORD(winMsg->lParam));
                            float yPos = (float)((short)HIWORD(winMsg->lParam));

                            message.what = B_WINDOW_MOVED;
                            message.AddPoint("where", BPoint(xPos, yPos));

                            handled = false;
                        }
                        else
                            if(winMsg->message == WM_MOVING)
                            {
                                if(retResult) *retResult = TRUE;

                                RECT *r = (RECT*)(winMsg->lParam);
                                message.what = B_WINDOW_MOVED;

                                RECT wr;
                                int left = 0, top = 0;

                                win32Engine->Lock();
                                GetWindowRect(winMsg->hwnd, &wr);
                                bhapi::win32_window_convert_to_screen(winMsg->hwnd, &left, &top);
                                win32Engine->Unlock();

                                left += r->left - wr.left;
                                top += r->top - wr.top;
                                message.AddPoint("where", BPoint((float)left, (float)top));
                            }
                            else
                                if(winMsg->message == WM_SIZING)
                                {
                                    if(retResult) *retResult = TRUE;

                                    RECT *r = (RECT*)(winMsg->lParam);
                                    message.what = B_WINDOW_RESIZED;

                                    RECT wr, cr;
                                    int left = 0, top = 0, width, height;

                                    win32Engine->Lock();
                                    GetWindowRect(winMsg->hwnd, &wr);
                                    GetClientRect(winMsg->hwnd, &cr);
                                    bhapi::win32_window_convert_to_screen(winMsg->hwnd, &left, &top);
                                    win32Engine->Unlock();

                                    width = cr.right;
                                    height = cr.bottom;
                                    left += r->left - wr.left;
                                    top += r->top - wr.top;
                                    width += (r->left - r->right) - (wr.left - wr.right);
                                    height += (r->bottom - r->top) - (wr.bottom - wr.top);

                                    message.AddPoint("where", BPoint((float)left, (float)top));
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
                                win->fLook != B_NO_BORDER_WINDOW_LOOK))
                        {
                            win32Engine->Unlock();
                            handled = true;
                            if(retResult) *retResult = MA_NOACTIVATE;
                            break;
                        }
                        win32Engine->Unlock();
                    }

                    message.what = B_WINDOW_ACTIVATED;
                    etkWinMsgr.SendMessage(&message);

                    if(winMsg->message == WM_ACTIVATE)
                    {
                        message.what = B_MINIMIZED;
                        message.AddBool("minimize", (HIWORD(winMsg->wParam) != 0 ? true : false));
                        etkWinMsgr.SendMessage(&message);
                    }
                }
                break;

#ifdef _WIN32_WINNT
                case WM_MOUSEWHEEL:
                {
                    short zDelta = GET_WHEEL_DELTA_WPARAM(winMsg->wParam);

                    message.what = B_MOUSE_WHEEL_CHANGED;
                    message.AddFloat("BHAPI:wheel_delta_y", zDelta > 0 ? -1.f : 1.f);

                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
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
                    bhapi::win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
                    win32Engine->Unlock();

                    b_int32 buttons = 0;
                    if(winMsg->wParam & MK_LBUTTON) buttons += 1;
                    if(winMsg->wParam & MK_RBUTTON) buttons += 2;
                    if(winMsg->wParam & MK_MBUTTON) buttons += 3;

                    message.what = B_MOUSE_MOVED;
                    message.AddInt32("buttons", buttons);
                    message.AddPoint("where", BPoint((float)xPos, (float)yPos));
                    message.AddPoint("screen_where", BPoint((float)xScreenPos, (float)yScreenPos));

                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
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
                    b_int32 clicks;

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
                    bhapi::win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
                    win32Engine->Unlock();

                    b_int32 button = 0;
                    if(winMsg->message == WM_LBUTTONDOWN) button = 1;
                    else
                        if(winMsg->message == WM_MBUTTONDOWN) button = 2;
                        else
                            if(winMsg->message == WM_RBUTTONDOWN) button = 3;

                    b_int32 buttons = 0;
                    if(winMsg->wParam & MK_LBUTTON) buttons += 1;
                    if(winMsg->wParam & MK_MBUTTON) buttons += 2;
                    if(winMsg->wParam & MK_RBUTTON) buttons += 3;

                    message.what = B_MOUSE_DOWN;
                    message.AddInt32("button", button);
                    message.AddInt32("buttons", buttons);
                    message.AddInt32("clicks", clicks);
                    message.AddPoint("where", BPoint((float)xPos, (float)yPos));
                    message.AddPoint("screen_where", BPoint((float)xScreenPos, (float)yScreenPos));

                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
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
                    bhapi::win32_window_convert_to_screen(winMsg->hwnd, &xScreenPos, &yScreenPos);
                    win32Engine->Unlock();

                    b_int32 button = 0;
                    if(winMsg->message == WM_LBUTTONUP) button = 1;
                    else
                        if(winMsg->message == WM_MBUTTONUP) button = 2;
                        else
                            if(winMsg->message == WM_RBUTTONUP) button = 3;

                    b_int32 buttons = 0;
                    if(winMsg->wParam & MK_LBUTTON) buttons += 1;
                    if(winMsg->wParam & MK_MBUTTON) buttons += 2;
                    if(winMsg->wParam & MK_RBUTTON) buttons += 3;

                    message.what = B_MOUSE_UP;
                    message.AddInt32("button", button);
                    message.AddInt32("buttons", buttons);
                    message.AddPoint("where", BPoint((float)xPos, (float)yPos));
                    message.AddPoint("screen_where", BPoint((float)xScreenPos, (float)yScreenPos));

                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
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

                    message.AddInt32("key", (b_int32)scanCode);
                    if(winMsg->message == WM_KEYDOWN)
                        message.AddInt32("BHAPI:key_repeat", (b_int32)(winMsg->lParam & 0xffff));

//                  BHAPI_DEBUG("[GRAPHICS]: %s: %d",
//                        winMsg->message == WM_KEYDOWN ? "KEYDOWN" : "KEYUP",
//                        (b_int32)winMsg->wParam);

                    b_int32 modifiers = 0;
                    if((keyState[VK_SHIFT] >> 4) > 0) modifiers |= B_SHIFT_KEY;
                    if((keyState[VK_MENU] >> 4) > 0) modifiers |= B_COMMAND_KEY;
                    if((keyState[VK_CONTROL] >> 4) > 0) modifiers |= B_CONTROL_KEY;
                    if((keyState[VK_LSHIFT] >> 4) > 0) modifiers |= B_LEFT_SHIFT_KEY | B_SHIFT_KEY;
                    if((keyState[VK_RSHIFT] >> 4) > 0) modifiers |= B_RIGHT_SHIFT_KEY | B_SHIFT_KEY;
                    if((keyState[VK_LMENU] >> 4) > 0) modifiers |= B_LEFT_COMMAND_KEY | B_COMMAND_KEY;
                    if((keyState[VK_RMENU] >> 4) > 0) modifiers |= B_RIGHT_COMMAND_KEY | B_COMMAND_KEY;
                    if((keyState[VK_LCONTROL] >> 4) > 0) modifiers |= B_LEFT_CONTROL_KEY | B_CONTROL_KEY;
                    if((keyState[VK_RCONTROL] >> 4) > 0) modifiers |= B_RIGHT_CONTROL_KEY | B_CONTROL_KEY;
                    if((keyState[VK_LWIN] >> 4) > 0) modifiers |= B_LEFT_OPTION_KEY | B_OPTION_KEY;
                    if((keyState[VK_RWIN] >> 4) > 0) modifiers |= B_RIGHT_OPTION_KEY | B_OPTION_KEY;
                    if((keyState[VK_APPS] >> 4) > 0) modifiers |= B_MENU_KEY;
                    if((keyState[VK_CAPITAL] & 0x0f) > 0) modifiers |= B_CAPS_LOCK;
                    if((keyState[VK_SCROLL] & 0x0f) > 0) modifiers |= B_SCROLL_LOCK;
                    if((keyState[VK_NUMLOCK] & 0x0f) > 0) modifiers |= B_NUM_LOCK;
                    if(winMsg->wParam >= VK_F1 && winMsg->wParam <= VK_F12)
                    {
                        message.ReplaceInt32("key", winMsg->wParam - VK_F1 + B_F1_KEY);
                        modifiers |= B_FUNCTIONS_KEY;
                    }
                    else
                        if(winMsg->wParam == VK_PRINT)
                        {
                            message.ReplaceInt32("key", B_PRINT_KEY);
                            modifiers |= B_FUNCTIONS_KEY;
                        }
                        else
                            if(winMsg->wParam == VK_SCROLL)
                            {
                                message.ReplaceInt32("key", B_SCROLL_KEY);
                                modifiers |= B_FUNCTIONS_KEY;
                            }
                            else
                                if(winMsg->wParam == VK_PAUSE)
                                {
                                    message.ReplaceInt32("key", B_PAUSE_KEY);
                                    modifiers |= B_FUNCTIONS_KEY;
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
                        keybuffer[0] = B_TAB;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_LEFT:
                        keybuffer[0] = B_LEFT_ARROW;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_RIGHT:
                        keybuffer[0] = B_RIGHT_ARROW;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_UP:
                        keybuffer[0] = B_UP_ARROW;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_DOWN:
                        keybuffer[0] = B_DOWN_ARROW;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_PRIOR:
                        keybuffer[0] = B_PAGE_UP;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_NEXT:
                        keybuffer[0] = B_PAGE_DOWN;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_HOME:
                        keybuffer[0] = B_HOME;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_END:
                        keybuffer[0] = B_END;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_INSERT:
                        keybuffer[0] = B_INSERT;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_DELETE:
                        keybuffer[0] = B_DELETE;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_BACK:
                        keybuffer[0] = B_BACKSPACE;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_ESCAPE:
                        keybuffer[0] = B_ESCAPE;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_RETURN:
                        keybuffer[0] = B_ENTER;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    case VK_SPACE:
                        keybuffer[0] = B_SPACE;
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
                        keybuffer[0] = B_FUNCTION_KEY;
                        keybuffer[1] = '\0';
                        keynum = 1;
                        break;

                    default:
                        break;
                    }

                    message.what = (winMsg->message == WM_KEYDOWN ? B_KEY_DOWN : B_KEY_UP);
                    if(keynum > 0)
                    {
                        if(keynum == 1) message.AddInt8("byte", keybuffer[0]);
                        message.AddString("bytes", keybuffer);
                    }

                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
                    etkWinMsgr.SendMessage(&message);

                    bool dealed = true;

                    switch(winMsg->wParam)
                    {
                    case VK_SHIFT:
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~B_SHIFT_KEY;
                        else
                            modifiers |= B_SHIFT_KEY;
                        break;
                    case VK_MENU:
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~B_COMMAND_KEY;
                        else
                            modifiers |= B_COMMAND_KEY;
                        break;
                    case VK_CONTROL:
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~B_CONTROL_KEY;
                        else
                            modifiers |= B_CONTROL_KEY;
                        break;
                    case VK_LWIN:
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~(B_LEFT_OPTION_KEY | B_OPTION_KEY);
                        else
                            modifiers |= (B_LEFT_OPTION_KEY | B_OPTION_KEY);
                        break;
                    case VK_RWIN:
                        modifiers |= B_RIGHT_OPTION_KEY | B_OPTION_KEY;
                        break;
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~(B_RIGHT_OPTION_KEY | B_OPTION_KEY);
                        else
                            modifiers |= (B_RIGHT_OPTION_KEY | B_OPTION_KEY);
                        break;
                    case VK_APPS:
                        if(winMsg->message == WM_KEYDOWN)
                            modifiers &= ~B_MENU_KEY;
                        else
                            modifiers |= B_MENU_KEY;
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
                            modifiers &= ~B_FUNCTIONS_KEY;
                        else
                            modifiers |= B_FUNCTIONS_KEY;
                        break;
                    case VK_CAPITAL:
                        if(modifiers & B_CAPS_LOCK) modifiers &= ~B_CAPS_LOCK;
                        else modifiers |= B_CAPS_LOCK;
                        break;
                    case VK_SCROLL:
                        if(modifiers & B_SCROLL_LOCK) modifiers &= ~B_SCROLL_LOCK;
                        else modifiers |= B_SCROLL_LOCK;
                        break;
                    case VK_NUMLOCK:
                        if(modifiers & B_NUM_LOCK) modifiers &= ~B_NUM_LOCK;
                        else modifiers |= B_NUM_LOCK;
                        break;

                    default:
                        dealed = false;
                        break;
                    }

                    if(dealed)
                    {
                        if((modifiers & B_LEFT_SHIFT_KEY) ||
                                (modifiers & B_RIGHT_SHIFT_KEY)) modifiers |= B_SHIFT_KEY;
                        if((modifiers & B_LEFT_CONTROL_KEY) ||
                                (modifiers & B_RIGHT_CONTROL_KEY)) modifiers |= B_CONTROL_KEY;
                        if((modifiers & B_LEFT_COMMAND_KEY) ||
                                (modifiers & B_RIGHT_COMMAND_KEY)) modifiers |= B_COMMAND_KEY;
                        if((modifiers & B_LEFT_OPTION_KEY) ||
                                (modifiers & B_RIGHT_OPTION_KEY)) modifiers |= B_OPTION_KEY;

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
                        char *uStr = bhapi::win32_convert_active_to_utf8(str, -1);
                        free(str);
                        str = uStr;
                    }

                    if(str == NULL || *str == 0)
                    {
                        if(str) free(str);
                        ImmReleaseContext(winMsg->hwnd, imc);
                        break;
                    }

                    BHAPI_DEBUG("[GRAPHICS]: %s --- received input method(%s).", __PRETTY_FUNCTION__, str);
                    message.what = B_KEY_DOWN;
                    message.AddString("bytes", str);
                    message.AddMessenger("BHAPI:msg_for_target", etkWinMsgr);
                    etkWinMsgr = BMessenger(bhapi::app);
                    etkWinMsgr.SendMessage(&message);
                    message.what = B_KEY_UP;
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
                            !bhapi::win32_window_get_rect(winMsg->hwnd, &r) ||
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

        handled = b_process_win32_event(win32Engine, &winMsg, &result); // Process Win32 GDI32 Event
    }

    if(!handled) result = DefWindowProc(hWnd, umsg, wParam, lParam);

    return result;
}


static b_status_t b_graphics_request_async_task(void *arg)
{
    EWin32GraphicsEngine *win32Engine = (EWin32GraphicsEngine*)arg;

    win32Engine->Lock();

    if(win32Engine->win32Hinstance == NULL || win32Engine->win32RegisterClass == 0 ||
            (win32Engine->win32RequestAsyncWin = ::CreateWindowExW(0, MAKEINTATOM(win32Engine->win32RegisterClass),
                    TEXT("b_gdi32_request_async_win"), WS_DISABLED, 0, 0, 1, 1,
                    NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL)
    {
        if(win32Engine->win32RequestAsyncWin != NULL) DestroyWindow(win32Engine->win32RequestAsyncWin);
        win32Engine->win32RequestAsyncWin = NULL;

        bhapi::release_sem_etc(win32Engine->fRequestAsyncSem, 2, 0);

        win32Engine->Unlock();

        return B_ERROR;
    }

    SetWindowLong(win32Engine->win32RequestAsyncWin, 0, reinterpret_cast<long>(win32Engine));
    win32Engine->win32NextClipboardViewer = SetClipboardViewer(win32Engine->win32RequestAsyncWin);

    bhapi::release_sem(win32Engine->fRequestAsyncSem);

    win32Engine->Unlock();

    MSG winMsg;

    BHAPI_DEBUG("[GRAPHICS]: Enter Win32 GDI32 request-async task...");

    bhapi::win32_clipboard_changed();

    while(true)
    {
        if(GetMessage(&winMsg, NULL, 0, 0) == -1)
            BHAPI_ERROR("[GRAPHICS]: %s --- Win32 GDI32 operate error!", __PRETTY_FUNCTION__);

        win32Engine->Lock();
        if(win32Engine->win32DoQuit)
        {
            win32Engine->Unlock();
            break;
        }
        win32Engine->Unlock(); // for not dead lock within processing

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

    BHAPI_DEBUG("[GRAPHICS]: Win32 GDI32 request-async task quit.");

    return B_OK;
}


static b_status_t b_graphics_request_task(void *arg)
{
    EWin32GraphicsEngine *win32Engine = (EWin32GraphicsEngine*)arg;

    win32Engine->Lock();

    win32Engine->win32ThreadID = GetCurrentThreadId();

    WNDCLASSEXA wcApp;
    wcApp.lpszClassName = bhapi::app->Name();
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

    if((win32Engine->win32RegisterClass = RegisterClassExA(&wcApp)) == 0 ||
            (win32Engine->WM_BHAPI_MESSAGE = RegisterWindowMessageA("WM_BHAPI_MESSAGE")) == 0 ||
            (win32Engine->win32RequestWin = CreateWindowExW(0, MAKEINTATOM(win32Engine->win32RegisterClass),
                                            TEXT("b_gdi32_request_win"), WS_DISABLED, 0, 0, 1, 1,
                                            NULL, NULL, win32Engine->win32Hinstance, NULL)) == NULL ||
            (win32Engine->win32ScreenHDC = CreateDCA("DISPLAY", NULL, NULL, NULL)) == NULL)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- Unable to initialize the GDI32(\"%s\" failed, error code: %u).",
                      __PRETTY_FUNCTION__,
                      win32Engine->win32RegisterClass == 0 ? "RegisterClassEx" : (
                          win32Engine->WM_BHAPI_MESSAGE == 0 ? "RegisterWindowMessage" : (
                              win32Engine->win32RequestWin == NULL ? "CreateWindowEx" : "CreateDC")), GetLastError());
        if(win32Engine->win32RequestWin != NULL) DestroyWindow(win32Engine->win32RequestWin);
        if(win32Engine->win32RegisterClass != 0)
            UnregisterClass(MAKEINTATOM(win32Engine->win32RegisterClass), win32Engine->win32Hinstance);

        win32Engine->win32RequestWin = NULL;
        win32Engine->win32RegisterClass = 0;
        win32Engine->win32ThreadID = 0;

        bhapi::release_sem_etc(win32Engine->fRequestSem, 2, 0);

        win32Engine->Unlock();

        return B_ERROR;
    }

    SetWindowLong(win32Engine->win32RequestWin, 0, reinterpret_cast<long>(win32Engine));

    bhapi::release_sem(win32Engine->fRequestSem);

    win32Engine->Unlock();

    MSG winMsg;

    BHAPI_DEBUG("[GRAPHICS]: Enter Win32 GDI32 main task...");

    while(true)
    {
        if(GetMessage(&winMsg, NULL, 0, 0) == -1)
            BHAPI_ERROR("[GRAPHICS]: %s --- Win32 GDI32 operate error!", __PRETTY_FUNCTION__);

        win32Engine->Lock();
        if(win32Engine->win32DoQuit)
        {
            win32Engine->Unlock();
            break;
        }
        win32Engine->Unlock(); // for not dead lock within processing

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

    BHAPI_DEBUG("[GRAPHICS]: Win32 GDI32 request task quited.");

    return B_OK;
}


EWin32GraphicsEngine::EWin32GraphicsEngine()
    : BGraphicsEngine(),
      win32Hinstance(NULL), win32RegisterClass(0), win32ScreenHDC(NULL),
      win32ThreadID(0), win32RequestWin(NULL), win32RequestAsyncWin(NULL), WM_BHAPI_MESSAGE(0),
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


b_status_t
EWin32GraphicsEngine::Initalize()
{
    BMessageFilter *clipboardFilter = new BMessageFilter(B_CLIPBOARD_CHANGED, bhapi::win32_clipboard_filter);
    bhapi::app->Lock();
    bhapi::app->AddFilter(clipboardFilter);
    bhapi::app->Unlock();

    Lock();

    if(win32Hinstance == NULL)
    {
        Unlock();
        BHAPI_WARNING("[GRAPHICS]: %s --- win32Hinstance == NULL", __PRETTY_FUNCTION__);

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    if(InitCheck() == B_OK)
    {
        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    if((fRequestSem = bhapi::create_sem(0, NULL)) == NULL || (fRequestAsyncSem = bhapi::create_sem(0, NULL)) == NULL)
    {
        if(fRequestSem) bhapi::delete_sem(fRequestSem);
        if(fRequestAsyncSem) bhapi::delete_sem(fRequestAsyncSem);
        fRequestSem = NULL;
        fRequestAsyncSem = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    win32DoQuit = false;

    if((fRequestThread = bhapi::create_thread(b_graphics_request_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL ||
            (fRequestAsyncThread = bhapi::create_thread(b_graphics_request_async_task, B_URGENT_DISPLAY_PRIORITY, this, NULL)) == NULL)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- Unable to create thread for GDI32.", __PRETTY_FUNCTION__);

        bhapi::delete_sem(fRequestSem);
        bhapi::delete_sem(fRequestAsyncSem);
        fRequestSem = NULL;
        fRequestAsyncSem = NULL;

        if(fRequestThread) bhapi::delete_thread(fRequestThread);
        if(fRequestAsyncThread) bhapi::delete_thread(fRequestAsyncThread);
        fRequestThread = NULL;
        fRequestAsyncThread = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    if(bhapi::resume_thread(fRequestThread) != B_OK)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- Unable to resume GDI32 requst task.", __PRETTY_FUNCTION__);
        bhapi::delete_sem(fRequestSem);
        bhapi::delete_sem(fRequestAsyncSem);
        fRequestSem = NULL;
        fRequestAsyncSem = NULL;

        bhapi::delete_thread(fRequestThread);
        bhapi::delete_thread(fRequestAsyncThread);
        fRequestThread = NULL;
        fRequestAsyncThread = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    Unlock();

    b_int64 count = 0;

    bhapi::acquire_sem(fRequestSem);
    if(bhapi::get_sem_count(fRequestSem, &count) != B_OK || count > 0)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- GDI32 requst task return a error.", __PRETTY_FUNCTION__);

        Lock();

        bhapi::delete_sem(fRequestSem);
        bhapi::delete_sem(fRequestAsyncSem);
        fRequestSem = NULL;
        fRequestAsyncSem = NULL;

        bhapi::delete_thread(fRequestThread);
        bhapi::delete_thread(fRequestAsyncThread);
        fRequestThread = NULL;
        fRequestAsyncThread = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    Lock();

    bhapi::delete_sem(fRequestSem);
    fRequestSem = NULL;

    if(bhapi::resume_thread(fRequestAsyncThread) != B_OK)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- Unable to resume GDI32 requst-async task.", __PRETTY_FUNCTION__);
        bhapi::delete_sem(fRequestAsyncSem);
        fRequestAsyncSem = NULL;

        bhapi::delete_thread(fRequestAsyncThread);
        fRequestAsyncThread = NULL;

        PostMessageA(win32RequestWin, WM_QUIT, 0, 0);

        Unlock();

        b_status_t status;
        bhapi::wait_for_thread(fRequestThread, &status);

        Lock();

        bhapi::delete_thread(fRequestThread);
        fRequestThread = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    fClipboardFilter = clipboardFilter;

    Unlock();

    bhapi::acquire_sem(fRequestAsyncSem);
    if(bhapi::get_sem_count(fRequestAsyncSem, &count) != B_OK || count > 0)
    {
        BHAPI_WARNING("[GRAPHICS]: %s --- GDI32 requst-async task return a error.", __PRETTY_FUNCTION__);

        Lock();

        bhapi::delete_sem(fRequestAsyncSem);
        fRequestAsyncSem = NULL;

        PostMessageA(win32RequestWin, WM_QUIT, 0, 0);

        fClipboardFilter = NULL;

        Unlock();

        b_status_t status;
        bhapi::wait_for_thread(fRequestThread, &status);

        Lock();

        bhapi::delete_thread(fRequestThread);
        bhapi::delete_thread(fRequestAsyncThread);
        fRequestThread = NULL;
        fRequestAsyncThread = NULL;

        Unlock();

        bhapi::app->Lock();
        bhapi::app->RemoveFilter(clipboardFilter);
        bhapi::app->Unlock();
        delete clipboardFilter;
        return B_ERROR;
    }

    Lock();
    bhapi::delete_sem(fRequestAsyncSem);
    fRequestAsyncSem = NULL;
    Unlock();

    BHAPI_DEBUG("[GRAPHICS]: %s --- GDI32 Initalized.", __PRETTY_FUNCTION__);

    return B_OK;
}


void
EWin32GraphicsEngine::Cancel()
{
    BMessageFilter *clipboardFilter = NULL;

    Lock();

    if(InitCheck() == B_OK)
    {
        win32DoQuit = true;

        // Send a pending event to EventLoop in order to quit
        if(win32RequestWin) PostMessageA(win32RequestWin, WM_QUIT, 0, 0);
        if(win32RequestAsyncWin) PostMessageA(win32RequestAsyncWin, WM_QUIT, 0, 0);

        Unlock();

        b_status_t status;
        bhapi::wait_for_thread(fRequestThread, &status);
        bhapi::wait_for_thread(fRequestAsyncThread, &status);

        Lock();

        bhapi::delete_thread(fRequestThread);
        bhapi::delete_thread(fRequestAsyncThread);
        fRequestThread = NULL;
        fRequestAsyncThread = NULL;

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
EWin32GraphicsEngine::CreateContext()
{
    return(new BGraphicsContext());
}


BGraphicsDrawable*
EWin32GraphicsEngine::CreatePixmap(b_uint32 w, b_uint32 h)
{
    return(new EWin32GraphicsDrawable(this, w, h));
}


BGraphicsWindow*
EWin32GraphicsEngine::CreateWindow(b_int32 x, b_int32 y, b_uint32 w, b_uint32 h)
{
    return(new EWin32GraphicsWindow(this, x, y, w, h));
}


b_status_t
EWin32GraphicsEngine::GetDesktopBounds(b_uint32 *w, b_uint32 *h)
{
    BAutolock <EWin32GraphicsEngine> autolock(this);

    if(InitCheck() != B_OK) return B_ERROR;

    if(w) *w = GetDeviceCaps(win32ScreenHDC, HORZRES);
    if(h) *h = GetDeviceCaps(win32ScreenHDC, VERTRES);

    return B_OK;
}


b_status_t
EWin32GraphicsEngine::GetCurrentWorkspace(b_uint32 *workspace)
{
    // don't support workspace
    if(workspace != NULL) *workspace = 0;
    return B_ERROR;
}


b_status_t
EWin32GraphicsEngine::SetCursor(const void *cursor_data)
{
    if(win32RequestWin == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
    callback.command = WM_BHAPI_MESSAGE_CHANGE_APP_CURSOR;
    callback.data = cursor_data;

    bool successed = (SendMessageA(win32RequestWin, WM_BHAPI_MESSAGE,
                                   WM_BHAPI_MESSAGE_APP, (LPARAM)&callback) == (LRESULT)TRUE);

    return(successed ? B_OK : B_ERROR);
}


b_status_t
EWin32GraphicsEngine::GetDefaultCursor(BCursor *cursor)
{
    return B_ERROR;
}

#endif /* WIN32 */
