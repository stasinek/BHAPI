/*
 * Copyright 2006-2011, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef ICON_BUTTON_H
#define ICON_BUTTON_H


//! GUI class that loads an image from disk and shows it as clickable button.


#include <Control.h>
#include <kits/support/String.h>


class BBitmap;
class BMimeType;


namespace BPrivate {


class BIconButton : public BControl {
public:
								BIconButton(const char* name,
									const char* label = NULL,
									BMessage* message = NULL,
									BHandler* target = NULL);
	virtual						~BIconButton();

	// BView interface
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				AttachedToWindow();

	virtual	void				Draw(BRect updateRect);
	virtual	bool				ShouldDrawBorder() const;
	virtual	void				DrawBorder(BRect& frame,
									const BRect& updateRect,
									const rgb_color& backgroundColor,
									uint32_t controlLookFlags);
	virtual	void				DrawBackground(BRect& frame,
									const BRect& updateRect,
									const rgb_color& backgroundColor,
									uint32_t controlLookFlags);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32_t transit,
										   const BMessage* message);
	virtual	void				GetPreferredSize(float* width,
												 float* height);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();


	// BInvoker interface
	virtual	status_t			Invoke(BMessage* message = NULL);

	// BControl interface
	virtual	void				SetValue(int32_t value);
	virtual	void				SetEnabled(bool enable);

	// BIconButton
			bool				IsValid() const;

			void				SetPressed(bool pressed);
			bool				IsPressed() const;

			status_t			SetIcon(int32_t resourceID);
			status_t			SetIcon(const char* pathToBitmap);
	virtual	status_t			SetIcon(const BBitmap* bitmap,
									uint32_t flags = 0);
			status_t			SetIcon(const BMimeType* fileType,
									bool small = true);
			status_t			SetIcon(const unsigned char* bitsFromQuickRes,
									uint32_t width, uint32_t height,
									color_space format,
									bool convertToBW = false);
			void				ClearIcon();
			void				TrimIcon(bool keepAspect = true);

			BBitmap*			Bitmap() const;
									// caller has to delete the returned bitmap

protected:
			bool				IsInside() const;
			void				SetInside(bool inside);

private:
			BBitmap*			_ConvertToRGB32(const BBitmap* bitmap) const;
			status_t			_MakeBitmaps(const BBitmap* bitmap);
			void				_DeleteBitmaps();
			void				_SendMessage() const;
			void				_Update();
			void				_SetTracking(bool state);
			void				_SetFlags(uint32_t flags, bool set);
			bool				_HasFlags(uint32_t flags) const;

private:
			uint32_t				fButtonState;
			BBitmap*			fNormalBitmap;
			BBitmap*			fDisabledBitmap;
			BBitmap*			fClickedBitmap;
			BBitmap*			fDisabledClickedBitmap;

			BHandler*			fTargetCache;
};


} // namespac BPrivate


using BPrivate::BIconButton;


#endif // ICON_BUTTON_H
