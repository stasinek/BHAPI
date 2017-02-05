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
 * File: Control.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/String.h"
#include "./../kernel/OS.h"
#include "./../app/Looper.h"

#include "Window.h"
#include "Control.h"


EControl::EControl(ERect frame, const char *name, const char *label,
		   EMessage *message, euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags), EInvoker(message, NULL, NULL),
	  fLabel(NULL), fValue(E_CONTROL_OFF), fFocusChanging(false)
{
	if(label) fLabel = EStrdup(label);
}


EControl::~EControl()
{
	if(fLabel) delete[] fLabel;
}


void
EControl::SetLabel(const char *label)
{
	if(fLabel) delete[] fLabel;
	if(label)
		fLabel = EStrdup(label);
	else
		fLabel = NULL;
}


const char*
EControl::Label() const
{
	return fLabel;
}


void
EControl::SetValue(eint32 value)
{
	if(fValue != value)
	{
		fValue = value;
		if((Flags() & E_WILL_DRAW) && Window() != NULL) Draw(Bounds());
	}
}


eint32
EControl::Value() const
{
	return fValue;
}


e_status_t
EControl::Invoke(const EMessage *aMsg)
{
	bool IsNotify = false;
	euint32 kind = InvokeKind(&IsNotify);
      
	EMessage msg(kind);
	e_status_t status = E_BAD_VALUE;
      
	if(!aMsg && !IsNotify) aMsg = Message();

	if(!aMsg)
	{
		if(!IsNotify || !IsWatched()) return status;
	}
	else
	{
		msg = *aMsg;
	}

	msg.AddInt64("when", e_real_time_clock_usecs());
	msg.AddPointer("source", this);
	if(aMsg) status = EInvoker::Invoke(&msg);

	if(IsNotify) SendNotices(kind, &msg);

	return status;
}


void
EControl::AttachedToWindow()
{
	if(Target() == NULL) SetTarget(Window());
}


void
EControl::DetachedFromWindow()
{
	if(Target() == Window()) SetTarget(NULL);
}


void
EControl::MakeFocus(bool focusState)
{
	if(IsFocus() != focusState)
	{
		EView::MakeFocus(focusState);

		if(IsVisible() && (Flags() & E_WILL_DRAW))
		{
			ERegion aRegion = VisibleBoundsRegion();
			if(aRegion.CountRects() <= 0) return;

			fFocusChanging = true;
			if(Flags() & E_UPDATE_WITH_REGION)
				for(eint32 i = 0; i < aRegion.CountRects(); i++) Draw(aRegion.RectAt(i));
			else
				Draw(aRegion.Frame());
			fFocusChanging = false;
		}
	}
}


bool
EControl::IsFocusChanging() const
{
	return fFocusChanging;
}


void
EControl::SetValueNoUpdate(eint32 value)
{
	fValue = value;
}


