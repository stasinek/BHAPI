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
 * File: Alert.cpp
 * Description: EAlert --- Display a modal window that notifies something
 * 
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h>
#include "./../kernel/Kernel.h>
#include "./../app/Invoker.h>
#include "./../app/Application.h>
#include "./../add-ons/graphics/GraphicsEngine.h>
#include "./../render/Pixmap.h>

#include "Button.h"
#include "TextView.h"
#include "Alert.h"
#include "Bitmap.h"

#define ICON_WIDTH	36
#define ICON_HEIGHT	36

#include "icons/info.xpm"
#include "icons/idea.xpm"
#include "icons/warning.xpm"
#include "icons/stop.xpm"


class EAlertTypeView : public EView {
public:
	EAlertTypeView(ERect frame, e_alert_type type);
	virtual ~EAlertTypeView();

	virtual void SetViewColor(e_rgb_color c);
	virtual void Draw(ERect updateRect);

	virtual void GetPreferredSize(float *width, float *height);
	void InitBitmap();

	e_alert_type fAlertType;
	EBitmap *fBitmap;
	euint8 fState;
	EInvoker *fInvoker;
	void *fSem;
};


class EAlertButton : public EButton {
public:
	EAlertButton(ERect frame, const char *name, const char *label, EMessage *message,
		     euint32 resizeMode, euint32 flags, euint8 index);
	virtual e_status_t Invoke(const EMessage *msg);

private:
	euint8 fIndex;
};


EAlertTypeView::EAlertTypeView(ERect frame, e_alert_type type)
	: EView(frame, "alert_type_view", E_FOLLOW_LEFT | E_FOLLOW_TOP_BOTTOM, E_WILL_DRAW),
	  fAlertType(type), fBitmap(NULL), fState(0), fInvoker(NULL), fSem(NULL)
{
	InitBitmap();
}


EAlertTypeView::~EAlertTypeView()
{
	if(fBitmap) delete fBitmap;
}


void
EAlertTypeView::InitBitmap()
{
	const char **xpm_data = NULL;

	switch(fAlertType)
	{
		case E_INFO_ALERT: xpm_data = (const char**)info_xpm; break;
		case E_IDEA_ALERT: xpm_data = (const char**)idea_xpm; break;
		case E_WARNING_ALERT: xpm_data = (const char**)warning_xpm; break;
		case E_STOP_ALERT: xpm_data = (const char**)stop_xpm; break;
		default: break;
	}

	if(xpm_data != NULL)
	{
#if defined(ETK_OS_BEOS) || defined(ETK_BIG_ENDIAN)
		EPixmap *pix = new EPixmap(ICON_WIDTH, ICON_HEIGHT, E_RGB24_BIG);
#else
		EPixmap *pix = new EPixmap(ICON_WIDTH, ICON_HEIGHT, E_RGB24);
#endif
		pix->SetDrawingMode(E_OP_COPY);
		pix->SetHighColor(ViewColor());
		pix->FillRect(0, 0, ICON_WIDTH, ICON_HEIGHT);
		pix->SetHighColor(200, 200, 200);
		pix->FillRect(0, 0, ICON_WIDTH / 2, ICON_HEIGHT);
		pix->DrawXPM(xpm_data, 0, 0, 0, 0);

		if(fBitmap != NULL) delete fBitmap;
		fBitmap = new EBitmap(pix);
		delete pix;
	}
}


void
EAlertTypeView::SetViewColor(e_rgb_color c)
{
	if(ViewColor() != c)
	{
		EView::SetViewColor(c);
		InitBitmap();
		Invalidate();
	}
}


void
EAlertTypeView::Draw(ERect updateRect)
{
	ERect rect = Bounds();
	rect.right = rect.left + rect.Width() / 2;
	SetHighColor(200, 200, 200);
	FillRect(rect);

	if(fBitmap) DrawBitmap(fBitmap, Bounds().Center() - EPoint(ICON_WIDTH / 2.f - 1.f, Bounds().Height() / 2.f - 10.f));
}


void
EAlertTypeView::GetPreferredSize(float *width, float *height)
{
	if(width) *width = 100;
	if(height) *height = 150;
}


EAlertButton::EAlertButton(ERect frame, const char *name, const char *label, EMessage *message,
			   euint32 resizeMode, euint32 flags, euint8 index)
	: EButton(frame, name, label, message, resizeMode, flags)
{
	fIndex = index;
}


e_status_t
EAlertButton::Invoke(const EMessage *msg)
{
	EAlert *alert = e_cast_as(Window(), EAlert);
	EAlertTypeView *alert_view = e_cast_as(Window()->FindView("alert_type_view"), EAlertTypeView);

	if(alert == NULL || alert_view == NULL)
	{
		return EButton::Invoke(msg);
	}

	if(fIndex > 7) return E_ERROR;

	if(alert_view->fState & 0x80) // async
	{
		if(!(alert_view->fInvoker == NULL || alert_view->fInvoker->Message() == NULL))
		{
			EMessage aMsg = *(alert_view->fInvoker->Message());
			aMsg.AddInt32("which", (eint32)fIndex);
			alert_view->fInvoker->Invoke(&aMsg);
		}
	}
	else
	{
		alert_view->fState = (0x01 << fIndex);
		if(alert_view->fSem) etk_release_sem_etc(alert_view->fSem, (eint64)(fIndex + 1), 0);
	}

	alert->PostMessage(E_QUIT_REQUESTED);

	return E_OK;
}


EAlert::EAlert(const char *title,
	       const char *text,
	       const char *button1_label,
	       const char *button2_label,
	       const char *button3_label,
	       e_button_width btnWidth,
	       e_alert_type type)
	: EWindow(ERect(-100, -100, -10, -10), title, E_MODAL_WINDOW, 0)
{
	ERect tmpR(0, 0, 1, 1);

	EView *alert_view = new EAlertTypeView(tmpR, type);
	EView *info_view = new EView(tmpR, NULL, E_FOLLOW_ALL, 0);
	EView *btns_view = new EView(tmpR, NULL, E_FOLLOW_LEFT_RIGHT | E_FOLLOW_BOTTOM, 0);

	fButtons[0] = new EAlertButton(tmpR, NULL, button1_label ? button1_label : "OK", NULL, E_FOLLOW_RIGHT, E_WILL_DRAW, 0);

	if(button2_label) fButtons[1] = new EAlertButton(tmpR, NULL, button2_label, NULL, E_FOLLOW_RIGHT, E_WILL_DRAW, 1);
	else fButtons[1] = NULL;

	if(button3_label) fButtons[2] = new EAlertButton(tmpR, NULL, button3_label, NULL, E_FOLLOW_RIGHT, E_WILL_DRAW, 2);
	else fButtons[2] = NULL;

	float max_w = 0, max_h = 20, all_w = 0;

	for(eint8 i = 0; i < 3; i++)
	{
		if(fButtons[i] == NULL) continue;

		fButtons[i]->ResizeToPreferred();
		if(fButtons[i]->Frame().Width() > max_w) max_w = fButtons[i]->Frame().Width();
		if(fButtons[i]->Frame().Height() > max_h) max_h = fButtons[i]->Frame().Height();
		all_w += fButtons[i]->Frame().Width() + 10;
	}

	btns_view->ResizeTo(max_c((btnWidth == E_WIDTH_AS_USUAL ? (3 * max_w + 20) : all_w), 200), max_h);
	ERect btnR = btns_view->Bounds();

	for(eint8 i = 2; i >= 0; i--)
	{
		if(fButtons[i] == NULL) continue;
		btnR.left = btnR.right - (btnWidth == E_WIDTH_AS_USUAL ? max_w : fButtons[i]->Frame().Width());
		fButtons[i]->ResizeTo(btnR.Width(), btnR.Height());
		fButtons[i]->MoveTo(btnR.LeftTop());
		btns_view->AddChild(fButtons[i]);

		btnR.right -= btnR.Width() + 10;
	}

	fTextView = new ETextView(tmpR, NULL, tmpR, E_FOLLOW_NONE);
	fTextView->SetText(text);
	fTextView->MakeEditable(false);
	fTextView->MakeSelectable(false);
	fTextView->ResizeToPreferred();
	fTextView->MoveTo(5, 5);
	btns_view->MoveTo(fTextView->Frame().LeftBottom() + EPoint(0, 10));

	alert_view->ResizeToPreferred();

	info_view->ResizeTo(max_c(fTextView->Frame().Width(), btns_view->Frame().Width()) + 10,
			    fTextView->Frame().Height() + btns_view->Frame().Height() + 20);
	info_view->MoveTo(alert_view->Frame().Width() + 1, 0);
	info_view->AddChild(fTextView);
	info_view->AddChild(btns_view);
	btns_view->ResizeBy(info_view->Frame().Width() - (btns_view->Frame().Width() + 10), 0);

	alert_view->ResizeBy(0, info_view->Frame().Height() - alert_view->Frame().Height());
	alert_view->MoveTo(0, 0);

	ResizeTo(alert_view->Frame().Width() + info_view->Frame().Width(), info_view->Frame().Height());
	AddChild(alert_view);
	AddChild(info_view);

	fTextView->SetTextBackground(btns_view->ViewColor());

	MoveToCenter();
}


EAlert::~EAlert()
{
}


bool
EAlert::QuitRequested()
{
	return true;
}


eint32
EAlert::Go(bool could_proxy)
{
	if(IsRunning() || Proxy() != this || IsLockedByCurrentThread())
	{
		ETK_WARNING("[INTERFACE]: %s --- IsRunning() || Proxy() != this || IsLockedByCurrentThread()", __PRETTY_FUNCTION__);
		return -1;
	}

	Lock();

	EAlertTypeView *alert_view = e_cast_as(FindView("alert_type_view"), EAlertTypeView);
	alert_view->fState = 0x40;
	alert_view->fInvoker = NULL;

	if(could_proxy)
	{
		ELooper *looper = ELooper::LooperForThread(etk_get_current_thread_id());
		if(looper)
		{
			looper->Lock();
			ProxyBy(looper);
			looper->Unlock();
		}
	}

	Show();
	SendBehind(NULL);
	Activate();

	eint32 retVal = -1;

	if(Proxy() != this)
	{
		while(true)
		{
			EMessage *aMsg = NextLooperMessage(E_INFINITE_TIMEOUT);
			DispatchLooperMessage(aMsg);
			if(aMsg == NULL) break;
		}

		if(!(alert_view->fState & 0x40))
		{
			if(alert_view->fState & 0x01) retVal = 0;
			else if(alert_view->fState & 0x02) retVal = 1;
			else if(alert_view->fState & 0x04) retVal = 2;
		}

		Quit();
	}
	else
	{
		void *trackingSem = etk_create_sem(0, NULL);
		alert_view->fSem = trackingSem;

		Unlock();

		eint64 count = 0;
		if(!(etk_acquire_sem(trackingSem) != E_OK ||
		     etk_get_sem_count(trackingSem, &count) != E_OK ||
		     count < 0 || count > 2))
		{
			retVal = (eint32)count;
		}
		etk_delete_sem(trackingSem);
	}

	return retVal;
}


e_status_t
EAlert::Go(EInvoker *invoker)
{
	if(IsRunning() || Proxy() != this || IsLockedByCurrentThread())
	{
		ETK_WARNING("[INTERFACE]: %s --- IsRunning() || Proxy() != this || IsLockedByCurrentThread()", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	Lock();

	EAlertTypeView *alert_view = e_cast_as(FindView("alert_type_view"), EAlertTypeView);
	alert_view->fState = 0x80;
	alert_view->fInvoker = invoker;

	Show();
	SendBehind(NULL);

	Unlock();

	return E_OK;
}


EButton*
EAlert::ButtonAt(eint32 index) const
{
	if(index < 0 || index > 2) return NULL;
	return fButtons[index];
}


ETextView*
EAlert::TextView() const
{
	return fTextView;
}

