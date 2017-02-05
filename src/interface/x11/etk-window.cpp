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

#include <stdlib.h>

#include "etk-x11.h"

#include "./../support/Autolock.h>
#include "./../support/String.h>
#include "./../support/ClassInfo.h>
#include "./../app/Application.h>


EXGraphicsWindow::EXGraphicsWindow(EXGraphicsEngine *x11Engine, eint32 x, eint32 y, euint32 w, euint32 h)
	: EGraphicsWindow(), fEngine(NULL), fFlags(0)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = x11Engine;
	if(fEngine == NULL) return;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	e_rgb_color whiteColor = {255, 255, 255, 255};
	EGraphicsDrawable::SetBackgroundColor(whiteColor);
	xBackground = fEngine->xWhitePixel;
	xBackgroundAlloced = false;
	fLook = (e_window_look)0;
	fFeel = (e_window_feel)0;

	XSetWindowAttributes xAttributes;

	xAttributes.background_pixel = xBackground;
	xAttributes.colormap = fEngine->xColormap;
	xAttributes.event_mask = StandardWidgetXEventMask | fEngine->xInputMethodEventMask;
	xAttributes.bit_gravity = NorthWestGravity;
	xAttributes.win_gravity = NorthWestGravity;
	xAttributes.override_redirect = False;
	xAttributes.border_pixel = fEngine->xBlackPixel;

	if((xWindow = XCreateWindow(fEngine->xDisplay, fEngine->xRootWindow,
				    x, y, w + 1, h + 1, 0,
				    fEngine->xDepth, InputOutput, fEngine->xVisual,
				    CWEventMask | CWBackPixel | CWColormap | CWBitGravity | CWWinGravity |
				    CWOverrideRedirect | CWBorderPixel | CWBorderWidth, &xAttributes)) == None)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Unable to create window.", __PRETTY_FUNCTION__);
		fEngine = NULL;
		return;
	}

	XSizeHints size_hints;
	size_hints.flags = PPosition | PSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width =  w + 1;
	size_hints.height = h + 1;

	XWMHints wm_hints;
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;

	XClassHint class_hint;
	class_hint.res_name = NULL;
	class_hint.res_class = (char*)etk_app->Signature();

	XSetWMProperties(fEngine->xDisplay, xWindow, NULL, NULL, NULL, 0, &size_hints, &wm_hints, &class_hint);

	Atom protocols[2];
	protocols[0] = fEngine->atomWMDeleteWindow;
	protocols[1] = fEngine->atomWMFocus;

	XSetWMProtocols(fEngine->xDisplay, xWindow, protocols, 2);

	etk_x11_address_t self_address = reinterpret_cast<etk_x11_address_t>((void*)this);
	XChangeProperty(fEngine->xDisplay, xWindow,
			XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_ADDRESS", False),
			XInternAtom(fEngine->xDisplay, "ATOM_ETK_CAST_ADDRESS", False), ETK_X11_ADDRESS_T_FORMAT,
			PropModeReplace, (const unsigned char*)&self_address, ETK_X11_ADDRESS_T_NELEMENTS);

	euint8 winState = 0; // hidden
	XChangeProperty(fEngine->xDisplay, xWindow,
			XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False),
			XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False), 8,
			PropModeReplace, (const unsigned char*)&winState, 1);

	XGCValues xgcvals;
	xgcvals.function = GXcopy;
	xgcvals.foreground = xBackground;
	xgcvals.line_width = 0;
	xgcvals.line_style = LineSolid;
	xgcvals.fill_style = FillSolid;
	xgcvals.cap_style = CapButt;
	xgcvals.graphics_exposures = False;
	xGC = XCreateGC(fEngine->xDisplay, fEngine->xRootWindow,
			GCFunction | GCForeground | GCLineWidth |
			GCLineStyle | GCFillStyle | GCCapStyle | GCGraphicsExposures, &xgcvals);
}


EXGraphicsWindow::~EXGraphicsWindow()
{
	if(fEngine != NULL)
	{
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		XDeleteProperty(fEngine->xDisplay, xWindow, XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_ADDRESS", False));
		XSelectInput(fEngine->xDisplay, xWindow, NoEventMask);

		XClientMessageEvent xevent;

		xevent.type = ClientMessage;
		xevent.window = xWindow;
		xevent.message_type = fEngine->atomProtocols;
		xevent.format = 32;
		xevent.data.l[0] = (long)(fEngine->atomDeleteWindow);
		xevent.data.l[1] = CurrentTime;

		XSendEvent(fEngine->xDisplay, fEngine->xProtocolsWindow, False, 0, (XEvent *)&xevent);

		if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
		fMsgr = EMessenger();

		XFreeGC(fEngine->xDisplay, xGC);

		XFlush(fEngine->xDisplay);
	}
}


e_status_t
EXGraphicsWindow::ContactTo(const EMessenger *msgr)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(msgr) fMsgr = *msgr;
	else fMsgr = EMessenger();

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetBackgroundColor(e_rgb_color bkColor)
{
	if(fEngine == NULL) return E_ERROR;

	e_rgb_color color = BackgroundColor();
	bkColor.alpha = color.alpha = 255;

	if(bkColor == color) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

		if(bkColor.red == 0 && bkColor.green == 0 && bkColor.blue == 0)
		{
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = fEngine->xBlackPixel;
			xBackgroundAlloced = false;
		}
		else if(bkColor.red == 255 && bkColor.green == 255 && bkColor.blue == 255)
		{
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = fEngine->xWhitePixel;
			xBackgroundAlloced = false;
		}
		else
		{
			unsigned long p;
			if(EXGraphicsContext::AllocXColor(fEngine, bkColor, &p) == false) return E_ERROR;
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = p;
			xBackgroundAlloced = true;
		}

		XSetWindowBackground(fEngine->xDisplay, xWindow, xBackground);
		XFlush(fEngine->xDisplay);
	} while(false);

	EGraphicsDrawable::SetBackgroundColor(bkColor);

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetFlags(euint32 flags)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fFlags == flags) return E_OK;

	fFlags = flags;

	// Focus
	XWMHints wm_hints;
	wm_hints.flags = InputHint;
	wm_hints.input = (flags & E_AVOID_FOCUS ? False : True);
	XSetWMHints(fEngine->xDisplay, xWindow, &wm_hints);

	// Window Action
	long data[20];
	int n = 0;
	if(!(flags & E_NOT_MOVABLE)) {data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_MOVE", False); n++;}
	if(!(flags & E_NOT_RESIZABLE)) {data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_RESIZE", False); n++;}
	if(!(flags & E_NOT_MINIMIZABLE)) {data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_MINIMIZE", False); n++;}
	if(!(flags & E_NOT_ZOOMABLE))
	{
		data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_MAXIMIZE_HORZ", False); n++;
		data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_MAXIMIZE_VERT", False); n++;
	}
	if(!(flags & E_NOT_CLOSABLE)) {data[n] = XInternAtom(fEngine->xDisplay, "_NET_WM_ACTION_CLOSE", False); n++;}
	data[n] = 0;
  
	XChangeProperty(fEngine->xDisplay, xWindow,
			XInternAtom(fEngine->xDisplay, "_NET_WM_ALLOWED_ACTIONS", False), XA_ATOM, 32,
			PropModeReplace, (const unsigned char*)data, n);

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetLook(e_window_look look)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fLook == look) return E_OK;
	fLook = look;

	XWindowAttributes xattr;
	XGetWindowAttributes(fEngine->xDisplay, xWindow, &xattr);
	if(xattr.map_state != IsUnmapped)
	{
		XUnmapWindow(fEngine->xDisplay, xWindow);
	}

	Atom wmAtom;
	switch(look)
	{
		case E_TITLED_WINDOW_LOOK:
		case E_DOCUMENT_WINDOW_LOOK:
			wmAtom =  XInternAtom(fEngine->xDisplay, "_NET_WM_WINDOW_TYPE_NORMAL", False);
			break;
		case E_FLOATING_WINDOW_LOOK:
			wmAtom =  XInternAtom(fEngine->xDisplay, "_NET_WM_WINDOW_TYPE_UTILITY", False);
			break;
		case E_MODAL_WINDOW_LOOK:
			wmAtom =  XInternAtom(fEngine->xDisplay, "_NET_WM_WINDOW_TYPE_DIALOG", False);
			break;
		case E_BORDERED_WINDOW_LOOK:
			wmAtom =  XInternAtom(fEngine->xDisplay, "_NET_WM_WINDOW_TYPE_SPLASH", False);
			break;
		default:
			break;
	}

	if(look != E_NO_BORDER_WINDOW_LOOK)
		XChangeProperty(fEngine->xDisplay, xWindow,
				XInternAtom(fEngine->xDisplay, "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32,
				PropModeReplace, (const unsigned char*)&wmAtom, 1);

	if(look == E_BORDERED_WINDOW_LOOK)
	{
		XSetWindowBorder(fEngine->xDisplay, xWindow, fEngine->xBlackPixel);
		XSetWindowBorderWidth(fEngine->xDisplay, xWindow, 1);
	}
	else
	{
		XSetWindowBorderWidth(fEngine->xDisplay, xWindow, 0);
	}

	XSetWindowAttributes xsetattr;
	xsetattr.override_redirect = (look == E_NO_BORDER_WINDOW_LOOK ? True : False);
	XChangeWindowAttributes(fEngine->xDisplay, xWindow, CWOverrideRedirect, &xsetattr);

	if(xattr.map_state != IsUnmapped)
	{
		// FIXME: when change look from E_BORDERED_WINDOW_LOOK to E_NO_BORDER_WINDOW_LOOK, the window disappeared.
		if(look == E_NO_BORDER_WINDOW_LOOK)
			XMapRaised(fEngine->xDisplay, xWindow);
		else
			XMapWindow(fEngine->xDisplay, xWindow);
	}

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetFeel(e_window_feel feel)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fFeel == feel) return E_OK;
	fFeel = feel;

	// TODO

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetTitle(const char *title)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(title == NULL || *title == 0) title = " ";

	XTextProperty prop;
	prop.value = NULL;

	eunichar32 *utf32_title = e_utf8_convert_to_utf32(title, -1);

	if(!utf32_title || XwcTextListToTextProperty(fEngine->xDisplay, (wchar_t**)&utf32_title, 1, XCompoundTextStyle, &prop) != Success)
	{
		prop.value = (unsigned char*)title;
		prop.encoding = XInternAtom(fEngine->xDisplay, "UTF8_STRING", False);
		prop.format = 8;
		prop.nitems = strlen(title);
	}
	if(utf32_title) free(utf32_title);

	XSetWMIconName(fEngine->xDisplay, xWindow, &prop);
	XSetWMName(fEngine->xDisplay, xWindow, &prop);

	XChangeProperty(fEngine->xDisplay, xWindow,
			XInternAtom(fEngine->xDisplay, "_NET_WM_NAME", False), prop.encoding, prop.format,
			PropModeReplace, prop.value, prop.nitems);
	XChangeProperty(fEngine->xDisplay, xWindow,
			XInternAtom(fEngine->xDisplay, "_NET_WM_ICON_NAME", False), prop.encoding, prop.format,
			PropModeReplace, prop.value, prop.nitems);

	if((void*)prop.value != (void*)title && prop.value != NULL) XFree(prop.value);

	XClassHint class_hint;
	class_hint.res_name = (char*)title;
	class_hint.res_class = (char*)etk_app->Signature();

	XSetClassHint(fEngine->xDisplay, xWindow, &class_hint);

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetWorkspaces(euint32 workspaces)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	long maxWorkspace = 0;

	unsigned char *prop = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;
	if(XGetWindowProperty(fEngine->xDisplay, fEngine->xRootWindow,
			      XInternAtom(fEngine->xDisplay, "_NET_NUMBER_OF_DESKTOPS", False), 0, 1,
			      False, AnyPropertyType,
			      &type, &format, &nitems,
			      &bytes_after, &prop) != Success || type == None) return E_ERROR;
	if(format == 32 && nitems == 1 && prop != NULL) maxWorkspace = *((long*)prop);
	if(prop != NULL) XFree(prop);
//	ETK_DEBUG("[GRAPHICS]: Max Workspace is %ld", maxWorkspace);
	if(workspaces != E_ALL_WORKSPACES && workspaces > (euint32)maxWorkspace) return E_ERROR;

	long desktop = (workspaces == E_ALL_WORKSPACES ? (long)0xFFFFFFFF : (long)(workspaces - 1));

	XWindowAttributes xattr;
	XGetWindowAttributes(fEngine->xDisplay, xWindow, &xattr);

	if(xattr.map_state != IsUnmapped)
	{
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.display = fEngine->xDisplay;
		xev.xclient.window = xWindow;
		xev.xclient.message_type = fEngine->atomWMDesktop;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = desktop;
		xev.xclient.data.l[1] = 0;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;
		XSendEvent(fEngine->xDisplay, fEngine->xRootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}
	else
	{
		XChangeProperty(fEngine->xDisplay, xWindow, fEngine->atomWMDesktop,
				XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)&desktop, 1);
	}

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::GetWorkspaces(euint32 *workspaces)
{
	if(fEngine == NULL || workspaces == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	*workspaces = 0;

	unsigned char *prop = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	if(XGetWindowProperty(fEngine->xDisplay, xWindow, fEngine->atomWMDesktop, 0, 1,
			      False, AnyPropertyType,
			      &type, &format, &nitems,
			      &bytes_after, &prop) != Success ||
	   type == None) return E_OK;

	if(format == 32 && nitems == 1 && prop != NULL)
	{
		long desktop = *((long*)prop);
		if((unsigned long)desktop != 0xFFFFFFFF) *workspaces = desktop + 1;
		else *workspaces = E_ALL_WORKSPACES;
	}
	if(prop != NULL) XFree(prop);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Iconify()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	euint8 *_winState = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	XGetWindowProperty(fEngine->xDisplay, xWindow,
			   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
			   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
			   &type, &format, &nitems,
			   &bytes_after, (unsigned char **)&_winState);

	if(_winState == NULL) return E_ERROR;

	if(*_winState != 2) // if not iconed, here we don't check whether the window is hidden, leave it to ETK++
	{
		euint8 winState = 2; // iconed
		XChangeProperty(fEngine->xDisplay, xWindow,
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False),
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False), 8,
				PropModeReplace, (const unsigned char*)&winState, 1);
		XIconifyWindow(fEngine->xDisplay, xWindow, fEngine->xScreen);
		XFlush(fEngine->xDisplay);
	}

	XFree(_winState);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Show()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	euint8 *_winState = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	XGetWindowProperty(fEngine->xDisplay, xWindow,
			   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
			   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
			   &type, &format, &nitems,
			   &bytes_after, (unsigned char **)&_winState);

	if(_winState == NULL) return E_ERROR;

	if(*_winState != 1) // if not shown
	{
		euint8 winState = 1; // shown
		XChangeProperty(fEngine->xDisplay, xWindow,
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False),
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False), 8,
				PropModeReplace, (const unsigned char*)&winState, 1);
		XMapWindow(fEngine->xDisplay, xWindow);
		XFlush(fEngine->xDisplay);
	}

	XFree(_winState);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Hide()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	euint8 *_winState = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;

	XGetWindowProperty(fEngine->xDisplay, xWindow,
			   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
			   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
			   &type, &format, &nitems,
			   &bytes_after, (unsigned char **)&_winState);

	if(_winState == NULL) return E_ERROR;

	if(*_winState != 0) // if not hide
	{
		euint8 winState = 0; // hidden
		XChangeProperty(fEngine->xDisplay, xWindow,
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False),
				XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False), 8,
				PropModeReplace, (const unsigned char*)&winState, 1);
		XUnmapWindow(fEngine->xDisplay, xWindow);
		XFlush(fEngine->xDisplay);
	}

	XFree(_winState);

	return E_OK;
}


e_status_t
EXGraphicsWindow::MoveTo(eint32 x, eint32 y)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XMoveWindow(fEngine->xDisplay, xWindow, x, y);

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::ResizeTo(euint32 w, euint32 h)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XResizeWindow(fEngine->xDisplay, xWindow, w + 1, h + 1);

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XMoveResizeWindow(fEngine->xDisplay, xWindow, x, y, w + 1, h + 1);

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Raise()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	euint8 *_winState = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;
	bool winShown = false;

	XGetWindowProperty(fEngine->xDisplay, xWindow,
			   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
			   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
			   &type, &format, &nitems,
			   &bytes_after, (unsigned char **)&_winState);

	if(!(_winState == NULL || *_winState != 1)) winShown = true;
	if(_winState) XFree(_winState);

	if(!winShown) return E_ERROR;

	XRaiseWindow(fEngine->xDisplay, xWindow);
	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Lower(EGraphicsWindow *frontW)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	euint8 *_winState = NULL;
	Atom type = None;
	int format;
	unsigned long nitems;
	unsigned long bytes_after;
	bool winShown = false;
	bool frontShown = false;

	EXGraphicsWindow *frontWin = e_cast_as(frontW, EXGraphicsWindow);
	if(!(frontWin == NULL || (fEngine == frontWin->fEngine && xWindow != frontWin->xWindow))) return E_ERROR;

	XGetWindowProperty(fEngine->xDisplay, xWindow,
			   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
			   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
			   &type, &format, &nitems,
			   &bytes_after, (unsigned char **)&_winState);

	if(!(_winState == NULL || *_winState != 1)) winShown = true;
	if(_winState) XFree(_winState);
	_winState = NULL;

	if(!winShown) return E_ERROR;

	if(frontWin != NULL)
	{
		XGetWindowProperty(fEngine->xDisplay, frontWin->xWindow,
				   XInternAtom(fEngine->xDisplay, "ATOM_ETK_WINDOW_STATE", False), 0, 1,
				   False, XInternAtom(fEngine->xDisplay, "ATOM_ETK_BOOL", False),
				   &type, &format, &nitems,
				   &bytes_after, (unsigned char **)&_winState);

		if(!(_winState == NULL || *_winState != 1)) frontShown = true;
		if(_winState) XFree(_winState);
		_winState = NULL;
	}

	if(frontWin == NULL)
	{
		XLowerWindow(fEngine->xDisplay, xWindow);
	}
	else if(frontShown)
	{
		XWindowChanges winChanges;
		winChanges.sibling = xWindow;
		winChanges.stack_mode = Above;

		if(XReconfigureWMWindow(fEngine->xDisplay, frontWin->xWindow, fEngine->xScreen,
					CWSibling | CWStackMode, &winChanges) == 0) return E_ERROR;
	}
	else return E_ERROR;

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::Activate(bool state)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XWindowAttributes xattr;
	XGetWindowAttributes(fEngine->xDisplay, xWindow, &xattr);

	if(xattr.map_state != IsViewable)
	{
		return(state ? E_ERROR : E_OK);
	}

	XWMHints *wm_hints = XGetWMHints(fEngine->xDisplay, xWindow);
	if((wm_hints == NULL || wm_hints->input == False) && state)
	{
		if(wm_hints) XFree(wm_hints);
		return E_ERROR;
	}
	if(wm_hints) XFree(wm_hints);

	Window oldFocusWin;
	int revert;
	XGetInputFocus(fEngine->xDisplay, &oldFocusWin, &revert);

	if(state)
	{
		if(oldFocusWin != xWindow)
		{
			XEvent xev;
			xev.xclient.type = ClientMessage;
			xev.xclient.serial = 0;
			xev.xclient.send_event = True;
			xev.xclient.display = fEngine->xDisplay;
			xev.xclient.window = xWindow;
			xev.xclient.message_type = XInternAtom(fEngine->xDisplay, "_NET_ACTIVE_WINDOW", False);
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 1;
			xev.xclient.data.l[1] = CurrentTime;
			xev.xclient.data.l[2] = 0;
			xev.xclient.data.l[3] = 0;
			xev.xclient.data.l[4] = 0;
			XSendEvent(fEngine->xDisplay, fEngine->xRootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

			XSetInputFocus(fEngine->xDisplay, xWindow, RevertToNone, CurrentTime);
		}
	}
	else
	{
		if(oldFocusWin == xWindow)
		{
			XEvent xev;
			xev.xclient.type = ClientMessage;
			xev.xclient.serial = 0;
			xev.xclient.send_event = True;
			xev.xclient.display = fEngine->xDisplay;
			xev.xclient.window = None;
			xev.xclient.message_type = XInternAtom(fEngine->xDisplay, "_NET_ACTIVE_WINDOW", False);
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 1;
			xev.xclient.data.l[1] = CurrentTime;
			xev.xclient.data.l[2] = xWindow;
			xev.xclient.data.l[3] = 0;
			xev.xclient.data.l[4] = 0;
			XSendEvent(fEngine->xDisplay, fEngine->xRootWindow, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

			XSetInputFocus(fEngine->xDisplay, PointerRoot, RevertToNone, CurrentTime);
		}
	}

	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::GetActivatedState(bool *state) const
{
	if(fEngine == NULL || state == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fFlags & E_AVOID_FOCUS)
	{
		*state = false;
	}
	else
	{
		Window focusWin;
		int revert;
		XGetInputFocus(fEngine->xDisplay, &focusWin, &revert);
		*state = (focusWin == xWindow);
	}

	return E_OK;
}


e_status_t
EXGraphicsWindow::GrabMouse()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	Window focusWin;
	int revert;
	XGetInputFocus(fEngine->xDisplay, &focusWin, &revert);

	if(focusWin != xWindow) return E_ERROR;

	int status = XGrabPointer(fEngine->xDisplay, xWindow, False,
				  ButtonPressMask|ButtonReleaseMask|PointerMotionMask|
				  EnterWindowMask|LeaveWindowMask|KeymapStateMask,
				  GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

	return(status == GrabSuccess ? E_OK : E_ERROR);
}


e_status_t
EXGraphicsWindow::UngrabMouse()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XUngrabPointer(fEngine->xDisplay, CurrentTime);

	return E_OK;
}


e_status_t
EXGraphicsWindow::GrabKeyboard()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	Window focusWin;
	int revert;
	XGetInputFocus(fEngine->xDisplay, &focusWin, &revert);

	if(focusWin != xWindow) return E_ERROR;

	int status = XGrabKeyboard(fEngine->xDisplay, xWindow, False, GrabModeAsync, GrabModeAsync, CurrentTime);

	return(status == GrabSuccess ? E_OK : E_ERROR);
}


e_status_t
EXGraphicsWindow::UngrabKeyboard()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XUngrabKeyboard(fEngine->xDisplay, CurrentTime);

	return E_OK;
}


e_status_t
EXGraphicsWindow::SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XSizeHints size_hints;
	size_hints.flags = PMinSize | PMaxSize;
	size_hints.min_width = (min_w == E_MAXUINT32 ? 0 : (min_w + 1));
	size_hints.min_height = (min_h == E_MAXUINT32 ? 0 : (min_h + 1));
	size_hints.max_width = (max_w == E_MAXUINT32 ? 0 : (max_w + 1));
	size_hints.max_height = (max_h == E_MAXUINT32 ? 0 : (max_h + 1));

	XSetWMNormalHints(fEngine->xDisplay, xWindow, &size_hints);
	XFlush(fEngine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsWindow::GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h)
{
	if(min_w == NULL || max_w == NULL || min_h == NULL || max_h == NULL) return E_ERROR;

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	XSizeHints size_hints;
	long supplied_return = 0;

	if(!XGetWMNormalHints(fEngine->xDisplay, xWindow, &size_hints, &supplied_return)) return E_ERROR;

	*min_w = E_MAXUINT32;
	*max_w = E_MAXUINT32;
	*min_h = E_MAXUINT32;
	*max_h = E_MAXUINT32;

	if(size_hints.flags & PMinSize)
	{
		*min_w = (size_hints.min_width > 0 ? (size_hints.min_width - 1) : E_MAXUINT32);
		*min_h = (size_hints.min_height > 0 ? (size_hints.min_height - 1) : E_MAXUINT32);
	}

	if(size_hints.flags & PMaxSize)
	{
		*max_w = (size_hints.max_width > 0 ? (size_hints.max_width - 1) : E_MAXUINT32);
		*max_h = (size_hints.max_height > 0 ? (size_hints.max_height - 1) : E_MAXUINT32);
	}

	return E_OK;
}


e_status_t
EXGraphicsWindow::QueryMouse(eint32 *x, eint32 *y, eint32 *buttons)
{
	if(x == NULL && y == NULL && buttons == NULL) return E_ERROR;

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	int rx, ry, wx, wy;
	unsigned int state;
	Window rootReturn, childReturn;
	if(XQueryPointer(fEngine->xDisplay, xWindow, &rootReturn, &childReturn, &rx, &ry, &wx, &wy, &state) == False) return E_ERROR;

	if(x) *x = wx;
	if(y) *y = wy;
	if(buttons)
	{
		*buttons = 0;
		if(state & Button1Mask) *buttons += 1;
		if(state & Button2Mask) *buttons += 2;
		if(state & Button3Mask) *buttons += 3;
	}

	return E_OK;
}


e_status_t
EXGraphicsWindow::CopyTo(EGraphicsContext *_dc_,
			 EGraphicsDrawable *dstDrawable,
			 eint32 x, eint32 y, euint32 w, euint32 h,
			 eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(w != dstW || h != dstH)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(w == E_MAXUINT32 || h == E_MAXUINT32 || dstW == E_MAXUINT32 || dstH == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL || dstDrawable == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(dc == NULL || dc->fEngine != fEngine) return E_ERROR;
	if(dc->DrawingMode() != E_OP_COPY)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	EXGraphicsWindow *win = NULL;
	EXGraphicsDrawable *pix = NULL;

	e_status_t retVal = E_OK;

	if((win = e_cast_as(dstDrawable, EXGraphicsWindow)) != NULL)
		XCopyArea(fEngine->xDisplay, xWindow, win->xWindow, DefaultGC(fEngine->xDisplay, fEngine->xScreen),
			  x, y, w + 1, h + 1, dstX, dstY);
	else if((pix = e_cast_as(dstDrawable, EXGraphicsDrawable)) != NULL)
		XCopyArea(fEngine->xDisplay, xWindow, pix->xPixmap, DefaultGC(fEngine->xDisplay, fEngine->xScreen),
			  x, y, w + 1, h + 1, dstX, dstY);
	else
		retVal = E_ERROR;

	XFlush(fEngine->xDisplay);

	return retVal;
}

