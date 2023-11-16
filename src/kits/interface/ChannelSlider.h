/*
 * Copyright 2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHANNEL_SLIDER_H
#define _CHANNEL_SLIDER_H


#include <ChannelControl.h>


class BChannelSlider : public BChannelControl {
public:
								BChannelSlider(BRect area, const char* name,
									const char* label, BMessage* message,
									int32_t channels = 1,
									uint32_t resizeMode
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BChannelSlider(BRect area, const char* name,
									const char* label, BMessage* message,
									orientation orientation,
									int32_t channels = 1,
									uint32_t resizeMode
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BChannelSlider(const char* name,
									const char* label, BMessage* message,
									orientation orientation,
									int32_t channels = 1,
									uint32_t flags = B_WILL_DRAW);
								BChannelSlider(BMessage* archive);
	virtual						~BChannelSlider();

	static	BArchivable*		Instantiate(BMessage* from);
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

	virtual	void				AttachedToWindow();
	virtual	void				AllAttached();
	virtual	void				DetachedFromWindow();
	virtual	void				AllDetached();

	virtual	void				MessageReceived(BMessage* message);

	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32_t transit,
									const BMessage* dragMessage);
	virtual	void				WindowActivated(bool state);
	virtual	void				KeyDown(const char* bytes, int32_t numBytes);
	virtual	void				KeyUp(const char* bytes, int32_t numBytes);
	virtual	void				FrameResized(float width, float height);

	virtual	void				SetFont(const BFont* font,
									uint32_t mask = B_FONT_ALL);
	virtual	void				MakeFocus(bool focusState = true);

	virtual	void				GetPreferredSize(float* _width, float* _height);

	virtual	BHandler*			ResolveSpecifier(BMessage* message,
									int32_t index, BMessage* specifier,
									int32_t form, const char* property);
	virtual	status_t			GetSupportedSuites(BMessage* data);

	virtual	void				SetEnabled(bool on);

	virtual	orientation			Orientation() const;
			void				SetOrientation(orientation orientation);

	virtual	int32_t				MaxChannelCount() const;
	virtual	bool				SupportsIndividualLimits() const;

	virtual	void				DrawChannel(BView* into, int32_t channel,
									BRect area, bool pressed);

	virtual	void				DrawGroove(BView* into, int32_t channel,
									BPoint leftTop, BPoint rightBottom);

	virtual	void				DrawThumb(BView* into, int32_t channel,
									BPoint where, bool pressed);

	virtual	const BBitmap*		ThumbFor(int32_t channel, bool pressed);
	virtual	BRect				ThumbFrameFor(int32_t channel);
	virtual	float				ThumbDeltaFor(int32_t channel);
	virtual	float				ThumbRangeFor(int32_t channel);

private:
	// FBC padding
								BChannelSlider(const BChannelSlider&);
			BChannelSlider&		operator=(const BChannelSlider&);


	virtual	void				_Reserved_BChannelSlider_0(void*, ...);
	virtual	void				_Reserved_BChannelSlider_1(void*, ...);
	virtual	void				_Reserved_BChannelSlider_2(void*, ...);
	virtual	void				_Reserved_BChannelSlider_3(void*, ...);
	virtual	void				_Reserved_BChannelSlider_4(void*, ...);
	virtual	void				_Reserved_BChannelSlider_5(void*, ...);
	virtual	void				_Reserved_BChannelSlider_6(void*, ...);
	virtual	void				_Reserved_BChannelSlider_7(void*, ...);

private:
			void				_InitData();
			void				_FinishChange(bool update = false);
			void				_UpdateFontDimens();
			void				_DrawThumbs();
			void				_DrawGrooveFrame(BView* where,
									const BRect& area);
			void				_MouseMovedCommon(BPoint point, BPoint point2);

private:
			float				fBaseLine;
			float				fLineFeed;
			BBitmap*			fLeftKnob;
			BBitmap*			fMidKnob;
			BBitmap*			fRightKnob;
			BBitmap*			fBacking;
			BView*				fBackingView;
			bool				fIsVertical;
			bool				_padding_[3];
			BPoint				fClickDelta;

			int32_t				fCurrentChannel;
			bool				fAllChannels;
			int32_t*				fInitialValues;
			float				fMinPoint;
			int32_t				fFocusChannel;

			uint32_t				_reserved_[12];
};


#endif // _CHANNEL_SLIDER_H
