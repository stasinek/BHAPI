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
 * File: Bitmap.cpp
 * Description: EBitmap --- a rectangular image for drawing
 * Warning: Unfinished.
 *
 * --------------------------------------------------------------------------*/


#include "./../add-ons/graphics/GraphicsEngine.h"
#include "./../app/Application.h"
#include "./../render/Pixmap.h"

#include "Bitmap.h"

class _LOCAL EBitmapWindow : public EWindow {
public:
	EBitmapWindow(ERect frame);
	virtual ~EBitmapWindow();

private:
	virtual bool IsDependsOnOthersWhenQuitRequested() const;
};


EBitmapWindow::EBitmapWindow(ERect frame)
	: EWindow(frame, NULL, E_NO_BORDER_WINDOW_LOOK, E_NORMAL_WINDOW_FEEL, 0)
{
}


EBitmapWindow::~EBitmapWindow()
{
}


bool
EBitmapWindow::IsDependsOnOthersWhenQuitRequested() const
{
	return true;
}


void
EBitmap::InitSelf(ERect bounds, bool acceptsViews)
{
	fRows = 0;
	fColumns = 0;
	fPixmap = NULL;
	fWindow = NULL;

	if(bounds.IsValid() == false) return;

	if(etk_app == NULL || etk_app->fGraphicsEngine == NULL)
	{
		ETK_WARNING("[INTERFACE]: %s --- You should create EBitmap within graphics engine.", __PRETTY_FUNCTION__);
		return;
	}

	fColumns = (euint32)bounds.IntegerWidth() + 1;
	fRows = (euint32)bounds.IntegerHeight() + 1;

	if(acceptsViews == false)
	{
		fPixmap = etk_app->fGraphicsEngine->CreatePixmap(fColumns - 1, fRows - 1);
	}
	else
	{
		fWindow = new EBitmapWindow(bounds);
		fWindow->Lock();
		delete fWindow->fWindow;
		fWindow->fWindow = NULL;
		fPixmap = fWindow->fPixmap;
		fWindow->Show();
		fWindow->Unlock();
	}
}


EBitmap::EBitmap(ERect bounds, bool acceptsViews)
	: EArchivable()
{
	InitSelf(bounds, acceptsViews);
}


EBitmap::EBitmap(const EBitmap *bitmap, bool acceptsViews)
	: EArchivable()
{
	InitSelf(bitmap == NULL ? ERect() : bitmap->Bounds(), acceptsViews);
	if(bitmap->fPixmap != NULL)
	{
		EGraphicsContext *dc = etk_app->fGraphicsEngine->CreateContext();
		if(dc)
		{
			bitmap->fPixmap->CopyTo(dc, fPixmap, 0, 0, fColumns - 1, fRows - 1, 0, 0, fColumns - 1, fRows - 1);
			delete dc;
		}
	}
}


EBitmap::EBitmap(const EPixmap *pixmap, bool acceptsViews)
	: EArchivable()
{
	InitSelf((pixmap == NULL || pixmap->IsValid() == false) ? ERect() : pixmap->Bounds(), acceptsViews);
	if(fPixmap != NULL)
	{
		EGraphicsContext *dc = etk_app->fGraphicsEngine->CreateContext();
		if(dc)
		{
			dc->SetDrawingMode(E_OP_COPY);
			dc->SetHighColor(0, 0, 0, 255);
			dc->SetClipping(ERegion(pixmap->Bounds()));
			fPixmap->DrawPixmap(dc, pixmap, 0, 0, fColumns - 1, fRows - 1, 0, 0, fColumns - 1, fRows - 1);
			delete dc;
		}
	}
}


EBitmap::~EBitmap()
{
	if(fWindow)
	{
		fWindow->Lock();
		fWindow->Quit();
	}
	else if(fPixmap)
	{
		delete fPixmap;
	}
}


e_status_t
EBitmap::InitCheck() const
{
	return(fPixmap != NULL ? E_OK : E_ERROR);
}


bool
EBitmap::IsValid() const
{
	return(fPixmap != NULL);
}


ERect
EBitmap::Bounds() const
{
	if(fPixmap == NULL) return ERect();
	return ERect(0, 0, (float)(fColumns - 1), (float)(fRows - 1));
}


void
EBitmap::AddChild(EView *view)
{
	if(fWindow != NULL) fWindow->AddChild(view);
}


bool
EBitmap::RemoveChild(EView *view)
{
	return(fWindow != NULL ? fWindow->RemoveChild(view) : false);
}


eint32
EBitmap::CountChildren() const
{
	return(fWindow != NULL ? fWindow->CountChildren() : 0);
}


EView*
EBitmap::ChildAt(eint32 index) const
{
	return(fWindow != NULL ? fWindow->ChildAt(index) : NULL);
}


EView*
EBitmap::FindView(const char *name) const
{
	return(fWindow != NULL ? fWindow->FindView(name) : NULL);
}


EView*
EBitmap::FindView(EPoint where) const
{
	return(fWindow != NULL ? fWindow->FindView(where) : NULL);
}


bool
EBitmap::Lock()
{
	return(fWindow != NULL ? fWindow->Lock() : false);
}


void
EBitmap::Unlock()
{
	if(fWindow != NULL) fWindow->Unlock();
}

