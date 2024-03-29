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
 * File: Control.cpp
 *
 * --------------------------------------------------------------------------*/

#include <Control.h>
#include <Window.h>

#include <kits/support/String.h>
#include <kits/debug/Errors.h>
#include <os/kernel.h>
#include <kits/app/Looper.h>
#include <kits/app/Message.h>
#include <kits/interface/Rect.h>
#include <kits/interface/View.h>

BControl::BControl(BRect frame, const char *name, const char *label,
		   BMessage *message,  __be_uint32_t resizeMode,  __be_uint32_t flags)
	: BView(frame, name, resizeMode, flags), BInvoker(message, NULL, NULL),
	  fLabel(NULL), fValue(B_CONTROL_OFF), fFocusChanging(false)
{
	if(label) fLabel = bhapi::strdup(label);
}


BControl::~BControl()
{
	if(fLabel) delete[] fLabel;
}


void BControl::SetLabel(const char *label)
{
	if(fLabel) delete[] fLabel;
	if(label)
		fLabel = bhapi::strdup(label);
	else
		fLabel = NULL;
}


const char*  BControl::Label() const
{
	return fLabel;
}


void BControl::SetValue(__be_int32_t value)
{
	if(fValue != value)
	{
		fValue = value;
		if((Flags() & B_WILL_DRAW) && Window() != NULL) Draw(Bounds());
	}
}


__be_int32_t BControl::Value() const
{
	return fValue;
}


status_t BControl::Invoke(const BMessage *aMsg)
{
	bool IsNotify = false;
	__be_uint32_t kind = InvokeKind(&IsNotify);
      
	BMessage msg(kind);
	status_t status = B_BAD_VALUE;
      
	if(!aMsg && !IsNotify) aMsg = Message();

	if(!aMsg)
	{
		if(!IsNotify || !IsWatched()) return status;
	}
	else
	{
		msg = *aMsg;
	}

	msg.AddInt64("when", b_real_time_clock_usecs());
	msg.AddPointer("source", this);
	if(aMsg) status = BInvoker::Invoke(&msg);

	if(IsNotify) SendNotices(kind, &msg);

	return status;
}


void BControl::AttachedToWindow()
{
	if(Target() == NULL) SetTarget(Window());
}


void BControl::DetachedFromWindow()
{
	if(Target() == Window()) SetTarget(NULL);
}


void BControl::MakeFocus(bool focusState)
{
	if(IsFocus() != focusState)
	{
		BView::MakeFocus(focusState);

		if(IsVisible() && (Flags() & B_WILL_DRAW))
		{
			BRegion aRegion = VisibleBoundsRegion();
			if(aRegion.CountRects() <= 0) return;

			fFocusChanging = true;
			if(Flags() & B_UPDATE_WITH_REGION)
				for(__be_int32_t i = 0; i < aRegion.CountRects(); i++) Draw(aRegion.RectAt(i));
			else
				Draw(aRegion.Frame());
			fFocusChanging = false;
		}
	}
}


bool BControl::IsFocusChanging() const
{
	return fFocusChanging;
}


void BControl::SetValueNoUpdate(__be_int32_t value)
{
	fValue = value;
}


