/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for BHAPI++
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
#ifdef LINUX
#ifdef DIRECTFB

#include <kits/support/Autolock.h>
#include <kits/support/ClassInfo.h>

#include <BHAPI_wrapper_dfb.h>


EDFBGraphicsWindow::EDFBGraphicsWindow(EDFBGraphicsEngine *dfbEngine,  int32 x,  int32 y,  uint32 w,  uint32 h)
	: BGraphicsWindow(), fFlags(0), fEngine(NULL), fTitle(NULL),
	  fHandlingMove(false), fHandlingResize(false)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = dfbEngine;
	if(fEngine == NULL) return;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {fEngine = NULL; return;}

	fHidden = true;

	fLook = B_TITLED_WINDOW_LOOK;
	fFeel = (bhapi::window_feel)0;

	fMargins.Set(0, 0, 0, 0);
	fOriginX = x;
	fOriginY = y;
	fWidth = w + 1;
	fHeight = h + 1;

	AdjustFrameByDecoration();

	bhapi::rgb_color whiteColor = bhapi::make_rgb_color(255, 255, 255, 255);
	BGraphicsDrawable::SetBackgroundColor(whiteColor);

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
		BHAPI_DEBUG("[GRAPHICS]: %s --- Unable to create window.", __PRETTY_FUNCTION__);

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
		BAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			BHAPI_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		fEngine->SetDFBWindowData(dfbWindow, NULL, NULL);

		dfbWindow->DisableEvents(dfbWindow, DWET_ALL);
		dfbWindow->Release(dfbWindow);
	}

	if(fTitle) delete[] fTitle;
}


status_t 
EDFBGraphicsWindow::GetContactor(BMessenger *msgr)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(msgr) *msgr = fMsgr;

	return B_OK;
}


status_t 
EDFBGraphicsWindow::ContactTo(const BMessenger *msgr)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(msgr) fMsgr = *msgr;
	else fMsgr = BMessenger();

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	bhapi::rgb_color c = BackgroundColor();
	if(c != bkColor)
	{
		BGraphicsDrawable::SetBackgroundColor(c);
		dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
		RenderDecoration();
#if 0
		// redraw all will process within BWindow
		DFBUserEvent evt;
		evt.clazz = DFEC_USER;
		evt.type = DUET_WINDOWREDRAWALL;
		evt.data = (void*)dfbWindowID;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));
#endif
	}

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetFlags(uint32 flags)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fFlags != flags)
	{
		fFlags = flags;
		if(fFlags & B_AVOID_FOCUS)
		{
			if(fEngine->dfbCurFocusWin == dfbWindowID) fEngine->dfbCurFocusWin = B_MAXUINT;
			dfbWindow->SetOpacity(dfbWindow, 0xff);
		}
		else
		{
			dfbWindow->SetOpacity(dfbWindow, (fEngine->dfbCurFocusWin == dfbWindowID ? 0xff : 0xaf));
		}
	}

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetLook(bhapi::window_look look)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

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

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetFeel(bhapi::window_feel feel)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::SetTitle(const char *title)
{
	if(fTitle) delete[] fTitle;
	fTitle = (title == NULL ? NULL : bhapi::strdup(title));

	// TODO

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetWorkspaces(uint32 workspaces)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::GetWorkspaces(uint32 *workspaces)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::Iconify()
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::Show()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fHidden)
	{
		dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
//		dfbWindow->RaiseToTop(dfbWindow);
		fHidden = false;

		if(fEngine->dfbCurFocusWin == B_MAXUINT && !(fFlags & B_AVOID_FOCUS))
		{
			fEngine->dfbCurFocusWin = dfbWindowID;
			dfbWindow->SetOpacity(dfbWindow, 0xff);
		}
	}

	return B_OK;
}


status_t 
EDFBGraphicsWindow::Hide()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(!fHidden)
	{
		dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
		dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
		dfbWindow->EnableEvents(dfbWindow, DWET_ALL);
		fHidden = true;

		if(fEngine->dfbCurFocusWin == dfbWindowID) fEngine->dfbCurFocusWin = B_MAXUINT;
		if(fEngine->dfbCurPointerGrabbed == dfbWindowID)
		{
			dfbWindow->UngrabPointer(dfbWindow);
			fEngine->dfbCurPointerGrabbed = B_MAXUINT;
			fHandlingMove = false;
			fHandlingResize = false;
		}
	}

	return B_OK;
}


status_t 
EDFBGraphicsWindow::Raise()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fHidden) return B_ERROR;

	dfbWindow->RaiseToTop(dfbWindow);
	dfbWindow->RequestFocus(dfbWindow);

	return B_OK;
}


status_t 
EDFBGraphicsWindow::Lower(BGraphicsWindow *_frontWin)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fHidden) return B_ERROR;

	EDFBGraphicsWindow *frontWin = cast_as(_frontWin, EDFBGraphicsWindow);

	if(frontWin == NULL)
	{
		dfbWindow->Lower(dfbWindow);
	}
	else if(frontWin->fHidden == false)
	{
		dfbWindow->PutBelow(dfbWindow, frontWin->dfbWindow);
	}
	else return B_ERROR;

	return B_OK;
}


status_t 
EDFBGraphicsWindow::Activate(bool state)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(state) dfbWindow->RequestFocus(dfbWindow);
	else if(fEngine->dfbCurFocusWin == dfbWindowID)
	{
		DFBWindowEvent evt;
		evt.clazz = DFEC_WINDOW;
		evt.window_id = dfbWindowID;
		evt.type = DWET_LOSTFOCUS;
		fEngine->dfbEventBuffer->PostEvent(fEngine->dfbEventBuffer, DFB_EVENT(&evt));
	}

	return B_OK;
}


status_t 
EDFBGraphicsWindow::GetActivatedState(bool *state) const
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	*state = (fEngine->dfbCurFocusWin == dfbWindowID);

	return B_OK;
}


status_t 
EDFBGraphicsWindow::MoveTo(int32 x,  int32 y)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	fOriginX = x - (int32)fMargins.left;
	fOriginY = y - (int32)fMargins.top;

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

	return B_OK;
}


status_t 
EDFBGraphicsWindow::ResizeTo(uint32 w,  uint32 h)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	fWidth = w + 1 + (uint32)fMargins.left + (uint32)fMargins.right;
	fHeight = h + 1 + (uint32)fMargins.top + (uint32)fMargins.bottom;

	dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
	dfbWindow->Resize(dfbWindow, (int)fWidth, (int)fHeight);
	dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
				   (int)fWidth - 1 - (int)fMargins.right,
				   (int)fHeight - 1 - (int)fMargins.bottom);
	if(fHidden) dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
	else dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
	dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

	bhapi::rgb_color c = BackgroundColor();
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

	return B_OK;
}


status_t 
EDFBGraphicsWindow::MoveAndResizeTo(int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	fOriginX = x - (int32)fMargins.left;
	fOriginY = y - (int32)fMargins.top;
	fWidth = w + 1 + (uint32)fMargins.left + (uint32)fMargins.right;
	fHeight = h + 1 + (uint32)fMargins.top + (uint32)fMargins.bottom;

	dfbWindow->DisableEvents(dfbWindow, DWET_POSITION_SIZE);
	dfbWindow->Resize(dfbWindow, (int)fWidth, (int)fHeight);
	dfbWindow->SetOpaqueRegion(dfbWindow, (int)fMargins.left, (int)fMargins.top,
				   (int)fWidth - 1 - (int)fMargins.right,
				   (int)fHeight - 1 - (int)fMargins.bottom);
	if(fHidden) dfbWindow->MoveTo(dfbWindow, fEngine->dfbDisplayWidth + 100, fEngine->dfbDisplayHeight + 100);
	else dfbWindow->MoveTo(dfbWindow, fOriginX, fOriginY);
	dfbWindow->EnableEvents(dfbWindow, DWET_ALL);

	bhapi::rgb_color c = BackgroundColor();
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

	return B_OK;
}


status_t 
EDFBGraphicsWindow::SetSizeLimits(uint32 min_w,  uint32 max_w,  uint32 min_h,  uint32 max_h)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::GetSizeLimits(uint32 *min_w,  uint32 *max_w,  uint32 *min_h,  uint32 *max_h)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::GrabMouse()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fEngine->dfbCurPointerGrabbed != B_MAXUINT) return B_ERROR;

	if(dfbWindow->GrabPointer(dfbWindow) == DFB_OK)
	{
		fEngine->dfbCurPointerGrabbed = dfbWindowID;
		return B_OK;
	}

	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::UngrabMouse()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(fEngine->dfbCurPointerGrabbed != dfbWindowID) return B_ERROR;

	if(dfbWindow->UngrabPointer(dfbWindow) == DFB_OK)
	{
		fEngine->dfbCurPointerGrabbed = B_MAXUINT;
		fHandlingMove = false;
		fHandlingResize = false;
		return B_OK;
	}

	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::GrabKeyboard()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return(dfbWindow->GrabKeyboard(dfbWindow) == DFB_OK ? B_OK : B_ERROR);
}


status_t 
EDFBGraphicsWindow::UngrabKeyboard()
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return(dfbWindow->UngrabKeyboard(dfbWindow) == DFB_OK ? B_OK : B_ERROR);
}


status_t 
EDFBGraphicsWindow::QueryMouse(int32 *x,  int32 *y,  int32 *buttons)
{
	return B_ERROR;
}


status_t 
EDFBGraphicsWindow::CopyTo(BGraphicsContext *dc,
		           BGraphicsDrawable *dstDrawable,
			    int32 x,  int32 y,  uint32 w,  uint32 h,
			    int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32 || dstW >= B_MAXINT32 || dstH >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL || dc == NULL || dstDrawable == NULL) return B_ERROR;

	if(dc->DrawingMode() != B_OP_COPY)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	EDFBGraphicsWindow *win = NULL;
	EDFBGraphicsDrawable *pix = NULL;
	IDirectFBSurface *destSurface = NULL;
	BRect margins(0, 0, 0, 0);

	if((win = cast_as(dstDrawable, EDFBGraphicsWindow)) != NULL) {destSurface = win->dfbSurface; margins = win->fMargins;}
	else if((pix = cast_as(dstDrawable, EDFBGraphicsDrawable)) != NULL) destSurface = pix->dfbSurface;

	if(destSurface == NULL) return B_ERROR;

	DFBRegion *dfbRegions = NULL;
	int nRegions = 0;

	if(fEngine->ConvertRegion(dc->Clipping(), &dfbRegions, &nRegions) == false) return B_ERROR;

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

	return B_OK;
}


status_t 
EDFBGraphicsWindow::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
			        int32 x,  int32 y,  uint32 w,  uint32 h,
			        int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_draw_epixmap(dfbSurface, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokePoint(BGraphicsContext *dc,
				int32 x,  int32 y)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_point(dfbSurface, dc, x, y, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokePoints(BGraphicsContext *dc,
				 const  int32 *pts,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points(dfbSurface, dc, pts, count, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokePoints_Colors(BGraphicsContext *dc,
					const BList *ptsArrayLists,  int32 arrayCount,
					const bhapi::rgb_color *highColors)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points_color(dfbSurface, dc, ptsArrayLists, arrayCount, highColors, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokePoints_Alphas(BGraphicsContext *dc,
					const  int32 *pts, const  uint8 *alpha,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points_alphas(dfbSurface, dc, pts, alpha, count, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokeLine(BGraphicsContext *dc,
			        int32 x0,  int32 y0,  int32 x1,  int32 y1)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_line(dfbSurface, dc, x0, y0, x1, y1, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokePolygon(BGraphicsContext *dc,
				  const  int32 *pts,  int32 count, bool closed)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_polygon(dfbSurface, dc, pts, count, closed, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillPolygon(BGraphicsContext *dc,
				const  int32 *pts,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_polygon(dfbSurface, dc, pts, count, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokeRect(BGraphicsContext *dc,
			        int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_rect(dfbSurface, dc, x, y, w, h, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillRect(BGraphicsContext *dc,
			      int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_rect(dfbSurface, dc, x, y, w, h, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokeRects(BGraphicsContext *dc,
				const  int32 *rects,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_rects(dfbSurface, dc, rects, count, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillRects(BGraphicsContext *dc,
			      const  int32 *rects,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_rects(dfbSurface, dc, rects, count, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillRegion(BGraphicsContext *dc,
			       const BRegion &region)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_region(dfbSurface, dc, region, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokeRoundRect(BGraphicsContext *dc,
				     int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillRoundRect(BGraphicsContext *dc,
				   int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius, &fMargins);
}


status_t 
EDFBGraphicsWindow::StrokeArc(BGraphicsContext *dc,
			       int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle, &fMargins);
}


status_t 
EDFBGraphicsWindow::FillArc(BGraphicsContext *dc,
			     int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle, &fMargins);
}

#endif /* DIRECTFB */
#endif /* LINUX */
