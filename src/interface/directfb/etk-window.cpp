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
 * File: etk-window.cpp
 * 
 * --------------------------------------------------------------------------*/
#ifdef LINUX
#ifdef DIRECTFB

#include "./../../support/Autolock.h"
#include "./../../support/ClassInfo.h"

#include "etk-dfb.h"


EDFBGraphicsWindow::EDFBGraphicsWindow(EDFBGraphicsEngine *dfbEngine, eint32 x, eint32 y, euint32 w, euint32 h)
	: EGraphicsWindow(), fFlags(0), fEngine(NULL), fTitle(NULL),
	  fHandlingMove(false), fHandlingResize(false)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = dfbEngine;
	if(fEngine == NULL) return;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	fHidden = true;

	fLook = E_TITLED_WINDOW_LOOK;
	fFeel = (e_window_feel)0;

	fMargins.Set(0, 0, 0, 0);
	fOriginX = x;
	fOriginY = y;
	fWidth = w + 1;
	fHeight = h + 1;

	AdjustFrameByDecoration();

	e_rgb_color whiteColor = e_make_rgb_color(255, 255, 255, 255);
	EGraphicsDrawable::SetBackgroundColor(whiteColor);

	DFBWindowDescription desc;
	desc.flags = (DFBWindowDescriptionFlags)(DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS);
	desc.caps = (DFBWindowCapabilities)(DWCAPS_ALPHACHANNEL | DWCAPS_NODECORATION);
	desc.posx = fEngine->dfbDisplayWidth + 100;
	desc.posy = fEngine->dfbDisplayHeight + 100;
	desc.width = (int)fWidth;
	desc.height = (int)fHeight;

	if(fEngine->dfbDisplayLayer->CreateWindow(fEngine->dfbDisplayLayer, &desc, &dfbWindow) != DFB_OK ||
	   dfbWindow->GetSurface(dfbWindow, &dfbSurface) != DFB_OK)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Unable to create window.", __PRETTY_FUNCTION__);

		if(dfbWindow) dfbWindow->Release(dfbWindow);
		fEngine = NULL;
		return;
	}

	DFBWindowOptions options;
	dfbWindow->GetOptions(dfbWindow, &options);
	dfbWindow->SetOptions(dfbWindow, (DFBWindowOptions)(options | DWOP_SHAPED));
	dfbWindow->SetOpacity(dfbWindow, 0xaf);
	dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
				   (int)fWidth - 1 - (int)fMargins.right,
				   (int)fHeight - 1 - (int)fMargins.bottom);
	dfbWindow->SetStackingClass(dfbWindow, DWSC_MIDDLE);

	dfbSurface->Clear(dfbSurface, 255, 255, 255, 255);

	dfbWindow->GetID(dfbWindow, &dfbWindowID);
	dfbWindow->AttachEventBuffer(dfbWindow, fEngine->dfbEventBuffer);
	dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

	fEngine->SetDFBWindowData(dfbWindow, this, NULL);

	RenderDecoration();
}


EDFBGraphicsWindow::~EDFBGraphicsWindow()
{
	if(fEngine != NULL)
	{
		EAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		fEngine->SetDFBWindowData(dfbWindow, NULL, NULL);

		dfbWindow->DisableEvents(dfbWindow, DWET_ALL);
		dfbWindow->Release(dfbWindow);
	}

	if(fTitle) delete[] fTitle;
}


e_status_t
EDFBGraphicsWindow::GetContactor(EMessenger *msgr)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(msgr) *msgr = fMsgr;

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::ContactTo(const EMessenger *msgr)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(msgr) fMsgr = *msgr;
	else fMsgr = EMessenger();

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetBackgroundColor(e_rgb_color bkColor)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	e_rgb_color c = BackgroundColor();
	if(c != bkColor)
	{
		EGraphicsDrawable::SetBackgroundColor(c);
		dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
		RenderDecoration();
#if 0
		// redraw all will process within EWindow
		DFBUserEvent evt;
		evt.clazz = DFEC_USER;
		evt.type = DUET_WINDOWREDRAWALL;
		evt.data = (void*)dfbWindowID;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));
#endif
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetFlags(euint32 flags)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fFlags != flags)
	{
		fFlags = flags;
		if(fFlags & E_AVOID_FOCUS)
		{
			if(fEngine->dfbCurFocusWin == dfbWindowID) fEngine->dfbCurFocusWin = E_MAXUINT;
			dfbWindow->SetOpacity(dfbWindow, 0xff);
		}
		else
		{
			dfbWindow->SetOpacity(dfbWindow, (fEngine->dfbCurFocusWin == dfbWindowID ? 0xff : 0xaf));
		}
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetLook(e_window_look look)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fLook != look)
	{
		fLook = look;
		AdjustFrameByDecoration();
		RenderDecoration();

		dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
		dfbWindow->Resize(dfbWindow, (int)fWidth, (int)fHeight);
		dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
					   (int)fWidth - 1 - (int)fMargins.right,
					   (int)fHeight - 1 - (int)fMargins.bottom);
		if(fHidden) dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
		else dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
		dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

		DFBWindowEvent evt;
		evt.clazz = DFEC_WINDOW;
		evt.window_id = dfbWindowID;
		evt.type = DWET_POSITION_SIZE;
		evt.x = fOriginX;
		evt.y = fOriginY;
		evt.w = (int)fWidth;
		evt.h = (int)fHeight;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));

		DFBUserEvent uevt;
		uevt.clazz = DFEC_USER;
		uevt.type = DUET_WINDOWREDRAWALL;
		uevt.data = (void*)dfbWindowID;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&uevt));
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetFeel(e_window_feel feel)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::SetTitle(const char *title)
{
	if(fTitle) delete[] fTitle;
	fTitle = (title == NULL ? NULL : EStrdup(title));

	// TODO

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetWorkspaces(euint32 workspaces)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::GetWorkspaces(euint32 *workspaces)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::Iconify()
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::Show()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fHidden)
	{
		dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
//		dfbWindow->RaiseToTop(dfbWindow);
		fHidden = false;

		if(fEngine->dfbCurFocusWin == E_MAXUINT && !(fFlags & E_AVOID_FOCUS))
		{
			fEngine->dfbCurFocusWin = dfbWindowID;
			dfbWindow->SetOpacity(dfbWindow, 0xff);
		}
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::Hide()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(!fHidden)
	{
		dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
		dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
		dfbWindow->EnableEvents(dfbWindow, DWET_ALL);
		fHidden = true;

		if(fEngine->dfbCurFocusWin == dfbWindowID) fEngine->dfbCurFocusWin = E_MAXUINT;
		if(fEngine->dfbCurPointerGrabbed == dfbWindowID)
		{
			dfbWindow->UngrabPointer(dfbWindow);
			fEngine->dfbCurPointerGrabbed = E_MAXUINT;
			fHandlingMove = false;
			fHandlingResize = false;
		}
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::Raise()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fHidden) return E_ERROR;

	dfbWindow->RaiseToTop(dfbWindow);
	dfbWindow->RequestFocus(dfbWindow);

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::Lower(EGraphicsWindow *_frontWin)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fHidden) return E_ERROR;

	EDFBGraphicsWindow *frontWin = e_cast_as(_frontWin, EDFBGraphicsWindow);

	if(frontWin == NULL)
	{
		dfbWindow->Lower(dfbWindow);
	}
	else if(frontWin->fHidden == false)
	{
		dfbWindow->PutBelow(dfbWindow, frontWin->dfbWindow);
	}
	else return E_ERROR;

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::Activate(bool state)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(state) dfbWindow->RequestFocus(dfbWindow);
	else if(fEngine->dfbCurFocusWin == dfbWindowID)
	{
		DFBWindowEvent evt;
		evt.clazz = DFEC_WINDOW;
		evt.window_id = dfbWindowID;
		evt.type = DWET_LOSTFOCUS;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));
	}

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::GetActivatedState(bool *state) const
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	*state = (fEngine->dfbCurFocusWin == dfbWindowID);

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::MoveTo(eint32 x, eint32 y)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	fOriginX = x - (eint32)fMargins.left;
	fOriginY = y - (eint32)fMargins.top;

	if(!fHidden)
	{
		dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
		dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
		dfbWindow->EnableEvents(dfbWindow, DWET_ALL);
	}

	DFBWindowEvent evt;
	evt.clazz = DFEC_WINDOW;
	evt.window_id = dfbWindowID;
	evt.type = DWET_POSITION;
	evt.x = fOriginX;
	evt.y = fOriginY;
	fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::ResizeTo(euint32 w, euint32 h)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	fWidth = w + 1 + (euint32)fMargins.left + (euint32)fMargins.right;
	fHeight = h + 1 + (euint32)fMargins.top + (euint32)fMargins.bottom;

	dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
	dfbWindow->Resize(dfbWindow, (int)fWidth, (int)fHeight);
	dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
				   (int)fWidth - 1 - (int)fMargins.right,
				   (int)fHeight - 1 - (int)fMargins.bottom);
	if(fHidden) dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
	else dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
	dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

	e_rgb_color c = BackgroundColor();
	dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
	RenderDecoration();

	DFBWindowEvent evt;
	evt.clazz = DFEC_WINDOW;
	evt.window_id = dfbWindowID;
	evt.type = DWET_POSITION_SIZE;
	evt.x = fOriginX;
	evt.y = fOriginY;
	evt.w = (int)fWidth;
	evt.h = (int)fHeight;
	fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	fOriginX = x - (eint32)fMargins.left;
	fOriginY = y - (eint32)fMargins.top;
	fWidth = w + 1 + (euint32)fMargins.left + (euint32)fMargins.right;
	fHeight = h + 1 + (euint32)fMargins.top + (euint32)fMargins.bottom;

	dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
	dfbWindow->Resize(dfbWindow, (int)fWidth, (int)fHeight);
	dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
				   (int)fWidth - 1 - (int)fMargins.right,
				   (int)fHeight - 1 - (int)fMargins.bottom);
	if(fHidden) dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
	else dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
	dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

	e_rgb_color c = BackgroundColor();
	dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
	RenderDecoration();

	DFBWindowEvent evt;
	evt.clazz = DFEC_WINDOW;
	evt.window_id = dfbWindowID;
	evt.type = DWET_POSITION_SIZE;
	evt.x = fOriginX;
	evt.y = fOriginY;
	evt.w = (int)fWidth;
	evt.h = (int)fHeight;
	fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::GrabMouse()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fEngine->dfbCurPointerGrabbed != E_MAXUINT) return E_ERROR;

	if(dfbWindow->GrabPointer(dfbWindow) == DFB_OK)
	{
		fEngine->dfbCurPointerGrabbed = dfbWindowID;
		return E_OK;
	}

	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::UngrabMouse()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	if(fEngine->dfbCurPointerGrabbed != dfbWindowID) return E_ERROR;

	if(dfbWindow->UngrabPointer(dfbWindow) == DFB_OK)
	{
		fEngine->dfbCurPointerGrabbed = E_MAXUINT;
		fHandlingMove = false;
		fHandlingResize = false;
		return E_OK;
	}

	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::GrabKeyboard()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return(dfbWindow->GrabKeyboard(dfbWindow) == DFB_OK ? E_OK : E_ERROR);
}


e_status_t
EDFBGraphicsWindow::UngrabKeyboard()
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return(dfbWindow->UngrabKeyboard(dfbWindow) == DFB_OK ? E_OK : E_ERROR);
}


e_status_t
EDFBGraphicsWindow::QueryMouse(eint32 *x, eint32 *y, eint32 *buttons)
{
	return E_ERROR;
}


e_status_t
EDFBGraphicsWindow::CopyTo(EGraphicsContext *dc,
		           EGraphicsDrawable *dstDrawable,
			   eint32 x, eint32 y, euint32 w, euint32 h,
			   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32 || dstW >= E_MAXINT32 || dstH >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL || dc == NULL || dstDrawable == NULL) return E_ERROR;

	if(dc->DrawingMode() != E_OP_COPY)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	EDFBGraphicsWindow *win = NULL;
	EDFBGraphicsDrawable *pix = NULL;
	IDirectFBSurface *destSurface = NULL;
	ERect margins(0, 0, 0, 0);

	if((win = e_cast_as(dstDrawable, EDFBGraphicsWindow)) != NULL) {destSurface = win->dfbSurface; margins = win->fMargins;}
	else if((pix = e_cast_as(dstDrawable, EDFBGraphicsDrawable)) != NULL) destSurface = pix->dfbSurface;

	if(destSurface == NULL) return E_ERROR;

	DFBRegion *dfbRegions = NULL;
	int nRegions = 0;

	if(fEngine->ConvertRegion(dc->Clipping(), &dfbRegions, &nRegions) == false) return E_ERROR;

	for(int i = 0; i < nRegions; i++)
	{
		DFBRectangle srcRect, destRect;
		srcRect.x = (int)x + (int)fMargins.left;
		srcRect.y = (int)y + (int)fMargins.top;
		srcRect.w = (int)w + 1;
		srcRect.h = (int)h + 1;
		destRect.x = (int)dstX + (int)margins.left;
		destRect.y = (int)dstY + (int)margins.top;
		destRect.w = (int)dstW + 1;
		destRect.h = (int)dstH + 1;

		destSurface->SetBlittingFlags(destSurface, DSBLIT_NOFX);
		if(dstW == w && dstH == h)
			destSurface->Blit(destSurface, dfbSurface, &srcRect, destRect.x, destRect.y);
		else
			destSurface->StretchBlit(destSurface, dfbSurface, &srcRect, &destRect);
	}

	if(win != NULL) destSurface->Flip(destSurface, NULL, DSFLIP_WAITFORSYNC);

	free(dfbRegions);

	return E_OK;
}


e_status_t
EDFBGraphicsWindow::DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
			       eint32 x, eint32 y, euint32 w, euint32 h,
			       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_draw_epixmap(dfbSurface, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokePoint(EGraphicsContext *dc,
				eint32 x, eint32 y)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_point(dfbSurface, dc, x, y, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokePoints(EGraphicsContext *dc,
				 const eint32 *pts, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points(dfbSurface, dc, pts, count, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokePoints_Colors(EGraphicsContext *dc,
					const EList *ptsArrayLists, eint32 arrayCount,
					const e_rgb_color *highColors)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points_color(dfbSurface, dc, ptsArrayLists, arrayCount, highColors, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokePoints_Alphas(EGraphicsContext *dc,
					const eint32 *pts, const euint8 *alpha, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points_alphas(dfbSurface, dc, pts, alpha, count, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokeLine(EGraphicsContext *dc,
			       eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_line(dfbSurface, dc, x0, y0, x1, y1, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokePolygon(EGraphicsContext *dc,
				  const eint32 *pts, eint32 count, bool closed)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_polygon(dfbSurface, dc, pts, count, closed, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillPolygon(EGraphicsContext *dc,
				const eint32 *pts, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_polygon(dfbSurface, dc, pts, count, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokeRect(EGraphicsContext *dc,
			       eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_rect(dfbSurface, dc, x, y, w, h, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillRect(EGraphicsContext *dc,
			     eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_rect(dfbSurface, dc, x, y, w, h, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokeRects(EGraphicsContext *dc,
				const eint32 *rects, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_rects(dfbSurface, dc, rects, count, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillRects(EGraphicsContext *dc,
			      const eint32 *rects, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_rects(dfbSurface, dc, rects, count, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillRegion(EGraphicsContext *dc,
			       const ERegion &region)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_region(dfbSurface, dc, region, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokeRoundRect(EGraphicsContext *dc,
				    eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillRoundRect(EGraphicsContext *dc,
				  eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius, &fMargins);
}


e_status_t
EDFBGraphicsWindow::StrokeArc(EGraphicsContext *dc,
			      eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle, &fMargins);
}


e_status_t
EDFBGraphicsWindow::FillArc(EGraphicsContext *dc,
			    eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle, &fMargins);
}

#endif /* DIRECTFB */
#endif /* LINUX */
