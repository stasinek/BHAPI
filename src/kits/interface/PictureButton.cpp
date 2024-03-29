/*
 * Copyright 2001-2010, Haiku, Inc.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *		Graham MacDonald (macdonag@btopenworld.com)
 */


#include <PictureButton.h>

#include <new>

#include <binary_compatibility/Interface.h>


BPictureButton::BPictureButton(BRect frame, const char* name,
	BPicture* off, BPicture* on, BMessage* message,
	uint32_t behavior, uint32_t resizingMode, uint32_t flags)
	:
	BControl(frame, name, "", message, resizingMode, flags),
	fEnabledOff(new(std::nothrow) BPicture(*off)),
	fEnabledOn(new(std::nothrow) BPicture(*on)),
	fDisabledOff(NULL),
	fDisabledOn(NULL),
	fBehavior(behavior)
{
}


BPictureButton::BPictureButton(BMessage* data)
	:
	BControl(data),
	fEnabledOff(NULL),
	fEnabledOn(NULL),
	fDisabledOff(NULL),
	fDisabledOn(NULL)
{
	BMessage pictureArchive;

	// Default to 1 state button if not here - is this valid?
	if (data->FindInt32("_behave", (int32_t*)&fBehavior) != B_OK)
		fBehavior = B_ONE_STATE_BUTTON;

	// Now expand the pictures:
	if (data->FindMessage("_e_on", &pictureArchive) == B_OK)
		fEnabledOn = new(std::nothrow) BPicture(&pictureArchive);

	if (data->FindMessage("_e_off", &pictureArchive) == B_OK)
		fEnabledOff = new(std::nothrow) BPicture(&pictureArchive);

	if (data->FindMessage("_d_on", &pictureArchive) == B_OK)
		fDisabledOn = new(std::nothrow) BPicture(&pictureArchive);

	if (data->FindMessage("_d_off", &pictureArchive) == B_OK)
		fDisabledOff = new(std::nothrow) BPicture(&pictureArchive);
}


BPictureButton::~BPictureButton()
{
	delete fEnabledOn;
	delete fEnabledOff;
	delete fDisabledOn;
	delete fDisabledOff;
}


BArchivable*
BPictureButton::Instantiate(BMessage* data)
{
	if (validate_instantiation(data, "BPictureButton"))
		return new (std::nothrow) BPictureButton(data);

	return NULL;
}


status_t BPictureButton::Archive(BMessage* data, bool deep) const
{
	status_t err = BControl::Archive(data, deep);
	if (err != B_OK)
		return err;

	// Fill out message, depending on whether a deep copy is required or not.
	if (deep) {
		BMessage pictureArchive;
		if (fEnabledOn->Archive(&pictureArchive, deep) == B_OK) {
			err = data->AddMessage("_e_on", &pictureArchive);
			if (err != B_OK)
				return err;
		}

		pictureArchive.MakeEmpty();
		if (fEnabledOff->Archive(&pictureArchive, deep) == B_OK) {
			err = data->AddMessage("_e_off", &pictureArchive);
			if (err != B_OK)
				return err;
		}

		pictureArchive.MakeEmpty();
		if (fDisabledOn && fDisabledOn->Archive(&pictureArchive, deep) == B_OK) {
			err = data->AddMessage("_d_on", &pictureArchive);
			if (err != B_OK)
				return err;
		}

		pictureArchive.MakeEmpty();
		if (fDisabledOff && fDisabledOff->Archive(&pictureArchive, deep) == B_OK) {
			err = data->AddMessage("_d_off", &pictureArchive);
			if (err != B_OK)
				return err;
		}
	}

	return data->AddInt32("_behave", fBehavior);
}


void BPictureButton::AttachedToWindow()
{
	BControl::AttachedToWindow();
}


void BPictureButton::DetachedFromWindow()
{
	BControl::DetachedFromWindow();
}


void BPictureButton::AllAttached()
{
	BControl::AllAttached();
}


void BPictureButton::AllDetached()
{
	BControl::AllDetached();
}


void BPictureButton::ResizeToPreferred()
{
	BControl::ResizeToPreferred();
}


void BPictureButton::GetPreferredSize(float* _width, float* _height)
{
	BControl::GetPreferredSize(_width, _height);
}


void BPictureButton::FrameMoved(BPoint newPosition)
{
	BControl::FrameMoved(newPosition);
}


void BPictureButton::FrameResized(float newWidth, float newHeight)
{
	BControl::FrameResized(newWidth, newHeight);
}


void BPictureButton::WindowActivated(bool active)
{
	BControl::WindowActivated(active);
}


void BPictureButton::MakeFocus(bool focus)
{
	BControl::MakeFocus(focus);
}


void BPictureButton::Draw(BRect updateRect)
{
	if (IsEnabled()) {
		if (Value() == B_CONTROL_ON)
			DrawPicture(fEnabledOn);
		else
			DrawPicture(fEnabledOff);
	} else {

		if (fDisabledOff == NULL
			|| (fDisabledOn == NULL && fBehavior == B_TWO_STATE_BUTTON))
			debugger("Need to set the 'disabled' pictures for this BPictureButton ");

		if (Value() == B_CONTROL_ON)
			DrawPicture(fDisabledOn);
		else
			DrawPicture(fDisabledOff);
	}

	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(Bounds(), B_SOLID_HIGH);
	}
}


void BPictureButton::MessageReceived(BMessage* message)
{
	BControl::MessageReceived(message);
}


void BPictureButton::KeyDown(const char* bytes, int32_t numBytes)
{
	if (numBytes == 1) {
		switch (bytes[0]) {
			case B_ENTER:
			case B_SPACE:
				if (fBehavior == B_ONE_STATE_BUTTON) {
					SetValue(B_CONTROL_ON);
					snooze(50000);
					SetValue(B_CONTROL_OFF);
				} else {
					if (Value() == B_CONTROL_ON)
						SetValue(B_CONTROL_OFF);
					else
						SetValue(B_CONTROL_ON);
				}
				Invoke();
				return;
		}
	}

	BControl::KeyDown(bytes, numBytes);
}


void BPictureButton::MouseDown(BPoint where)
{
	if (!IsEnabled()) {
		BControl::MouseDown(where);
		return;
	}

	SetMouseEventMask(B_POINTER_EVENTS,
		B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS);

	if (fBehavior == B_ONE_STATE_BUTTON) {
		SetValue(B_CONTROL_ON);
	} else {
		if (Value() == B_CONTROL_ON)
			SetValue(B_CONTROL_OFF);
		else
			SetValue(B_CONTROL_ON);
	}
	SetTracking(true);
}


void BPictureButton::MouseUp(BPoint where)
{
	if (IsEnabled() && IsTracking()) {
		if (Bounds().Contains(where)) {
			if (fBehavior == B_ONE_STATE_BUTTON) {
				if (Value() == B_CONTROL_ON) {
					snooze(75000);
					SetValue(B_CONTROL_OFF);
				}
			}
			Invoke();
		}

		SetTracking(false);
	}
}


void BPictureButton::MouseMoved(BPoint where, uint32_t code,
	const BMessage* dragMessage)
{
	if (IsEnabled() && IsTracking()) {
		if (code == B_EXITED_VIEW)
			SetValue(B_CONTROL_OFF);
		else if (code == B_ENTERED_VIEW)
			SetValue(B_CONTROL_ON);
	} else
		BControl::MouseMoved(where, code, dragMessage);
}


// #pragma mark -


void BPictureButton::SetEnabledOn(BPicture* picture)
{
	delete fEnabledOn;
	fEnabledOn = new (std::nothrow) BPicture(*picture);
}


void BPictureButton::SetEnabledOff(BPicture* picture)
{
	delete fEnabledOff;
	fEnabledOff = new (std::nothrow) BPicture(*picture);
}


void BPictureButton::SetDisabledOn(BPicture* picture)
{
	delete fDisabledOn;
	fDisabledOn = new (std::nothrow) BPicture(*picture);
}


void BPictureButton::SetDisabledOff(BPicture* picture)
{
	delete fDisabledOff;
	fDisabledOff = new (std::nothrow) BPicture(*picture);
}


BPicture*
BPictureButton::EnabledOn() const
{
	return fEnabledOn;
}


BPicture*
BPictureButton::EnabledOff() const
{
	return fEnabledOff;
}


BPicture*
BPictureButton::DisabledOn() const
{
	return fDisabledOn;
}


BPicture*
BPictureButton::DisabledOff() const
{
	return fDisabledOff;
}


void BPictureButton::SetBehavior(uint32_t behavior)
{
	fBehavior = behavior;
}


uint32_t BPictureButton::Behavior() const
{
	return fBehavior;
}


void BPictureButton::SetValue(int32_t value)
{
	BControl::SetValue(value);
}


status_t BPictureButton::Invoke(BMessage* message)
{
	return BControl::Invoke(message);
}


BHandler*
BPictureButton::ResolveSpecifier(BMessage* message, int32_t index,
	BMessage* specifier, int32_t what, const char* property)
{
	return BControl::ResolveSpecifier(message, index, specifier,
		what, property);
}


status_t BPictureButton::GetSupportedSuites(BMessage* data)
{
	return BControl::GetSupportedSuites(data);
}


status_t BPictureButton::Perform(perform_code code, void* _data)
{
	switch (code) {
		case PERFORM_CODE_MIN_SIZE:
			((perform_data_min_size*)_data)->return_value
				= BPictureButton::MinSize();
			return B_OK;
		case PERFORM_CODE_MAX_SIZE:
			((perform_data_max_size*)_data)->return_value
				= BPictureButton::MaxSize();
			return B_OK;
		case PERFORM_CODE_PREFERRED_SIZE:
			((perform_data_preferred_size*)_data)->return_value
				= BPictureButton::PreferredSize();
			return B_OK;
		case PERFORM_CODE_LAYOUT_ALIGNMENT:
			((perform_data_layout_alignment*)_data)->return_value
				= BPictureButton::LayoutAlignment();
			return B_OK;
		case PERFORM_CODE_HAS_HEIGHT_FOR_WIDTH:
			((perform_data_has_height_for_width*)_data)->return_value
				= BPictureButton::HasHeightForWidth();
			return B_OK;
		case PERFORM_CODE_GET_HEIGHT_FOR_WIDTH:
		{
			perform_data_get_height_for_width* data
				= (perform_data_get_height_for_width*)_data;
			BPictureButton::GetHeightForWidth(data->width, &data->min, &data->max,
				&data->preferred);
			return B_OK;
		}
		case PERFORM_CODE_SET_LAYOUT:
		{
			perform_data_set_layout* data = (perform_data_set_layout*)_data;
			BPictureButton::SetLayout(data->layout);
			return B_OK;
		}
		case PERFORM_CODE_LAYOUT_INVALIDATED:
		{
			perform_data_layout_invalidated* data
				= (perform_data_layout_invalidated*)_data;
			BPictureButton::LayoutInvalidated(data->descendants);
			return B_OK;
		}
		case PERFORM_CODE_DO_LAYOUT:
		{
			BPictureButton::DoLayout();
			return B_OK;
		}
		case PERFORM_CODE_SET_ICON:
		{
			perform_data_set_icon* data = (perform_data_set_icon*)_data;
			return BPictureButton::SetIcon(data->icon, data->flags);
		}
	}

	return BControl::Perform(code, _data);
}


status_t BPictureButton::SetIcon(const BBitmap* icon, uint32_t flags)
{
	return BControl::SetIcon(icon, flags);
}


// #pragma mark - BPictureButton private methods


void BPictureButton::_ReservedPictureButton1() {}
void BPictureButton::_ReservedPictureButton2() {}
void BPictureButton::_ReservedPictureButton3() {}


BPictureButton&
BPictureButton::operator=(const BPictureButton &button)
{
	return *this;
}

