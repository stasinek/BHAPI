/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/


#include <DialogPane.h>

#include <ControlLook.h>
#include <LayoutUtils.h>

#include <Thread.h>
#include <Utilities.h>
#include <Window.h>


const uint32_t kValueChanged = 'swch';

const rgb_color kNormalColor = {150, 150, 150, 255};
const rgb_color kHighlightColor = {100, 100, 0, 255};


static void AddSelf(BView* self, BView* to)
{
	to->AddChild(self);
}


void ViewList::RemoveAll(BView*)
{
	EachListItemIgnoreResult(this, &BView::RemoveSelf);
}


void ViewList::AddAll(BView* toParent)
{
	EachListItem(this, &AddSelf, toParent);
}


//	#pragma mark - DialogPane


DialogPane::DialogPane(BRect mode1Frame, BRect mode2Frame, int32_t initialMode,
	const char* name, uint32_t followFlags, uint32_t flags)
	:
	BView(FrameForMode(initialMode, mode1Frame, mode2Frame, mode2Frame),
		name, followFlags, flags),
	fMode(initialMode),
	fMode1Frame(mode1Frame),
	fMode2Frame(mode2Frame),
	fMode3Frame(mode2Frame)
{
	SetMode(fMode, true);
}


DialogPane::DialogPane(BRect mode1Frame, BRect mode2Frame, BRect mode3Frame,
	int32_t initialMode, const char* name, uint32_t followFlags, uint32_t flags)
	:
	BView(FrameForMode(initialMode, mode1Frame, mode2Frame, mode3Frame),
		name, followFlags, flags),
	fMode(initialMode),
	fMode1Frame(mode1Frame),
	fMode2Frame(mode2Frame),
	fMode3Frame(mode3Frame)
{
	SetMode(fMode, true);
}


DialogPane::~DialogPane()
{
	fMode3Items.RemoveAll(this);
	fMode2Items.RemoveAll(this);
}


void DialogPane::SetMode(int32_t mode, bool initialSetup)
{
	ASSERT(mode < 3 && mode >= 0);

	if (!initialSetup && mode == fMode)
		return;

	int32_t oldMode = fMode;
	fMode = mode;

	bool followBottom = (ResizingMode() & B_FOLLOW_BOTTOM) != 0;
	// if we are follow bottom, we will move ourselves, need to place us back
	float bottomOffset = 0;
	if (followBottom && Window() != NULL)
		bottomOffset = Window()->Bounds().bottom - Frame().bottom;

	BRect newBounds(BoundsForMode(fMode));
	if (!initialSetup)
		ResizeParentWindow(fMode, oldMode);

	ResizeTo(newBounds.Width(), newBounds.Height());

	float delta = 0;
	if (followBottom && Window() != NULL)
		delta = (Window()->Bounds().bottom - Frame().bottom) - bottomOffset;

	if (delta != 0) {
		MoveBy(0, delta);
		if (fLatch && (fLatch->ResizingMode() & B_FOLLOW_BOTTOM))
			fLatch->MoveBy(0, delta);
	}

	switch (fMode) {
		case 0:
		{
			if (oldMode > 1)
				fMode3Items.RemoveAll(this);
			if (oldMode > 0)
				fMode2Items.RemoveAll(this);

			BView* separator = FindView("separatorLine");
			if (separator) {
				BRect frame(separator->Frame());
				frame.InsetBy(-1, -1);
				RemoveChild(separator);
				Invalidate();
			}

			AddChild(new SeparatorLine(BPoint(newBounds.left, newBounds.top
				+ newBounds.Height() / 2), newBounds.Width(), false,
				"separatorLine"));
			break;
		}

		case 1:
		{
			if (oldMode > 1)
				fMode3Items.RemoveAll(this);
			else
				fMode2Items.AddAll(this);

			BView* separator = FindView("separatorLine");
			if (separator) {
				BRect frame(separator->Frame());
				frame.InsetBy(-1, -1);
				RemoveChild(separator);
				Invalidate();
			}
			break;
		}

		case 2:
		{
			fMode3Items.AddAll(this);
			if (oldMode < 1)
				fMode2Items.AddAll(this);

			BView* separator = FindView("separatorLine");
			if (separator) {
				BRect frame(separator->Frame());
				frame.InsetBy(-1, -1);
				RemoveChild(separator);
				Invalidate();
			}
			break;
		}
	}
}


void DialogPane::AttachedToWindow()
{
	BView* parent = Parent();
	if (parent != NULL) {
		SetViewColor(parent->ViewColor());
		SetLowColor(parent->LowColor());
	}
}


void DialogPane::ResizeParentWindow(int32_t from, int32_t to)
{
	if (Window() == NULL)
		return;

	BRect oldBounds = BoundsForMode(from);
	BRect newBounds = BoundsForMode(to);

	BPoint by = oldBounds.RightBottom() - newBounds.RightBottom();
	if (by != BPoint(0, 0))
		Window()->ResizeBy(by.x, by.y);
}


void DialogPane::AddItem(BView* view, int32_t toMode)
{
	if (toMode == 1)
		fMode2Items.AddItem(view);
	else if (toMode == 2)
		fMode3Items.AddItem(view);

	if (fMode >= toMode)
		AddChild(view);
}


BRect
DialogPane::FrameForMode(int32_t mode)
{
	switch (mode) {
		case 0:
			return fMode1Frame;

		case 1:
			return fMode2Frame;

		case 2:
			return fMode3Frame;
	}

	return fMode1Frame;
}


BRect
DialogPane::BoundsForMode(int32_t mode)
{
	BRect result;
	switch (mode) {
		case 0:
			result = fMode1Frame;
			break;

		case 1:
			result = fMode2Frame;
			break;

		case 2:
			result = fMode3Frame;
			break;
	}
	result.OffsetTo(0, 0);

	return result;
}


BRect
DialogPane::FrameForMode(int32_t mode, BRect mode1Frame, BRect mode2Frame,
	BRect mode3Frame)
{
	switch (mode) {
		case 0:
			return mode1Frame;

		case 1:
			return mode2Frame;

		case 2:
			return mode3Frame;
	}

	return mode1Frame;
}


void DialogPane::SetSwitch(BControl* control)
{
	fLatch = control;
	control->SetMessage(new BMessage(kValueChanged));
	control->SetTarget(this);
}


void DialogPane::MessageReceived(BMessage* message)
{
	if (message->what == kValueChanged) {
		int32_t value;
		if (message->FindInt32("be:value", &value) == B_OK)
			SetMode(value);
	} else
		_inherited::MessageReceived(message);
}


//	#pragma mark - PaneSwitch


PaneSwitch::PaneSwitch(BRect frame, const char* name, bool leftAligned,
		uint32_t resizeMask, uint32_t flags)
	:
	BControl(frame, name, "", 0, resizeMask, flags),
	fLeftAligned(leftAligned),
	fPressing(false),
	fLabelOn(NULL),
	fLabelOff(NULL)
{
}


PaneSwitch::PaneSwitch(const char* name, bool leftAligned, uint32_t flags)
	:
	BControl(name, "", 0, flags),
	fLeftAligned(leftAligned),
	fPressing(false),
	fLabelOn(NULL),
	fLabelOff(NULL)
{
}


PaneSwitch::~PaneSwitch()
{
	free(fLabelOn);
	free(fLabelOff);
}


void PaneSwitch::Draw(BRect)
{
	BRect bounds(Bounds());

	// Draw the label, if any
	const char* label = fLabelOff;
	if (fLabelOn != NULL && Value() == B_CONTROL_ON)
		label = fLabelOn;

	if (label != NULL) {
		BPoint point;
		float labelDist = sLatchSize + ceilf(sLatchSize / 2.0);
		if (fLeftAligned)
			point.x = labelDist;
		else
			point.x = bounds.right - labelDist - StringWidth(label);

		font_height fontHeight;
		GetFontHeight(&fontHeight);
		point.y = (bounds.top + bounds.bottom
			- ceilf(fontHeight.ascent) - ceilf(fontHeight.descent)) / 2
			+ ceilf(fontHeight.ascent);

		DrawString(label, point);
	}

	// draw the latch
	if (fPressing)
		DrawInState(kPressed);
	else if (Value())
		DrawInState(kExpanded);
	else
		DrawInState(kCollapsed);

	// ...and the focus indication
	if (!IsFocus() || !Window()->IsActive())
		return;

	rgb_color markColor = ui_color(B_KEYBOARD_NAVIGATION_COLOR);

	BeginLineArray(2);
	AddLine(BPoint(bounds.left + 2, bounds.bottom - 1),
		BPoint(bounds.right - 2, bounds.bottom - 1), markColor);
	AddLine(BPoint(bounds.left + 2, bounds.bottom),
		BPoint(bounds.right - 2, bounds.bottom), kWhite);
	EndLineArray();
}


void PaneSwitch::MouseDown(BPoint)
{
	if (!IsEnabled())
		return;

	fPressing = true;
	MouseDownThread<PaneSwitch>::TrackMouse(this, &PaneSwitch::DoneTracking,
		&PaneSwitch::Track);
	Invalidate();
}


void PaneSwitch::GetPreferredSize(float* _width, float* _height)
{
	BSize size = MinSize();
	if (_width != NULL)
		*_width = size.width;

	if (_height != NULL)
		*_height = size.height;
}


BSize
PaneSwitch::MinSize()
{
	BSize size;
	float onLabelWidth = StringWidth(fLabelOn);
	float offLabelWidth = StringWidth(fLabelOff);
	float labelWidth = max_c(onLabelWidth, offLabelWidth);
	size.width = sLatchSize;
	if (labelWidth > 0.0)
		size.width += ceilf(sLatchSize / 2.0) + labelWidth;

	font_height fontHeight;
	GetFontHeight(&fontHeight);
	size.height = ceilf(fontHeight.ascent) + ceilf(fontHeight.descent);
	size.height = max_c(size.height, sLatchSize);

	return BLayoutUtils::ComposeSize(ExplicitMinSize(), size);
}


BSize
PaneSwitch::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), MinSize());
}


BSize
PaneSwitch::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), MinSize());
}


void PaneSwitch::SetLabels(const char* labelOn, const char* labelOff)
{
	free(fLabelOn);
	free(fLabelOff);

	if (labelOn != NULL)
		fLabelOn = strdup(labelOn);
	else
		fLabelOn = NULL;

	if (labelOff != NULL)
		fLabelOff = strdup(labelOff);
	else
		fLabelOff = NULL;

	Invalidate();
	InvalidateLayout();
}


void PaneSwitch::DoneTracking(BPoint point)
{
	BRect bounds(Bounds());
	bounds.InsetBy(-3, -3);

	fPressing = false;
	Invalidate();
	if (bounds.Contains(point)) {
		SetValue(!Value());
		Invoke();
	}
}


void PaneSwitch::Track(BPoint point, uint32_t)
{
	BRect bounds(Bounds());
	bounds.InsetBy(-3, -3);

	bool newPressing = bounds.Contains(point);
	if (newPressing != fPressing) {
		fPressing = newPressing;
		Invalidate();
	}
}


void PaneSwitch::DrawInState(PaneSwitch::State state)
{
	BRect rect(0, 0,  __be_plain_font->Size(),  __be_plain_font->Size());
	rect.OffsetBy(1, 1);

	rgb_color arrowColor = state == kPressed ? kHighlightColor : kNormalColor;
	int32_t arrowDirection = BControlLook::B_RIGHT_ARROW;
	float tint = IsEnabled() && Window()->IsActive() ? B_DARKEN_3_TINT
		: B_DARKEN_1_TINT;

	switch (state) {
		case kCollapsed:
			arrowDirection = BControlLook::B_RIGHT_ARROW;
			break;

		case kPressed:
			arrowDirection = BControlLook::B_RIGHT_DOWN_ARROW;
			break;

		case kExpanded:
			arrowDirection = BControlLook::B_DOWN_ARROW;
			break;
	}

	SetDrawingMode(B_OP_COPY);
	be_control_look->DrawArrowShape(this, rect, rect, arrowColor,
		arrowDirection, 0, tint);
}
