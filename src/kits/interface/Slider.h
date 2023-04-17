/*
 * Copyright 2001-2013, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SLIDER_H
#define _SLIDER_H

#include <Control.h>
#include <Size.h>

enum hash_mark_location {
    B_HASH_MARKS_NONE = 0,
    B_HASH_MARKS_TOP = 1,
    B_HASH_MARKS_LEFT = 1,
    B_HASH_MARKS_BOTTOM = 2,
    B_HASH_MARKS_RIGHT = 2,
    B_HASH_MARKS_BOTH = 3
};

enum thumb_style {
    B_BLOCK_THUMB,
    B_TRIANGLE_THUMB
};


class BSlider : public BControl {
public:
                                BSlider(BRect frame, const char* name,
                                    const char* label, BMessage* message,
                                    int32_t minValue, int32_t maxValue,
                                    thumb_style thumbType = B_BLOCK_THUMB,
                                    uint32_t resizingMode
                                        = B_FOLLOW_LEFT | B_FOLLOW_TOP,
                                    uint32_t flags = B_NAVIGABLE | B_WILL_DRAW
                                        | B_FRAME_EVENTS);

                                BSlider(BRect frame, const char* name,
                                    const char* label, BMessage* message,
                                    int32_t minValue, int32_t maxValue,
                                    orientation posture,
                                    thumb_style thumbType = B_BLOCK_THUMB,
                                    uint32_t resizingMode
                                        = B_FOLLOW_LEFT | B_FOLLOW_TOP,
                                    uint32_t flags = B_NAVIGABLE | B_WILL_DRAW
                                        | B_FRAME_EVENTS);

                                BSlider(const char* name, const char* label,
                                    BMessage* message, int32_t minValue,
                                    int32_t maxValue, orientation posture,
                                    thumb_style thumbType = B_BLOCK_THUMB,
                                    uint32_t flags = B_NAVIGABLE | B_WILL_DRAW
                                        | B_FRAME_EVENTS);

                                BSlider(BMessage* archive);
    virtual						~BSlider();

    static 	BArchivable*		Instantiate(BMessage* archive);
    virtual	status_t 			Archive(BMessage* archive,
                                    bool deep = true) const;
    virtual status_t			Perform(perform_code code, void* data);

    virtual void				WindowActivated(bool state);
    virtual	void				AttachedToWindow();
    virtual	void				AllAttached();
    virtual	void				AllDetached();
    virtual	void				DetachedFromWindow();

    virtual	void				MessageReceived(BMessage* message);
    virtual void				FrameMoved(BPoint newPosition);
    virtual void				FrameResized(float width, float height);
    virtual void				KeyDown(const char* bytes, int32_t numBytes);
    virtual void				KeyUp(const char* bytes, int32_t numBytes);
    virtual void				MouseDown(BPoint point);
    virtual void				MouseUp(BPoint point);
    virtual void				MouseMoved(BPoint point, uint32_t transit,
                                    const BMessage* dragMessage);
    virtual	void				Pulse();

    virtual void				SetLabel(const char* label);
    virtual	void				SetLimitLabels(const char* minLabel,
                                    const char* maxLabel);
            const char*			MinLimitLabel() const;
            const char*			MaxLimitLabel() const;
    virtual	void				SetValue(int32_t value);
    virtual int32_t				ValueForPoint(BPoint point) const;
    virtual void				SetPosition(float);
            float				Position() const;
    virtual void				SetEnabled(bool on);
            void				GetLimits(int32_t* minimum, int32_t* maximum) const;

    virtual	void				Draw(BRect updateRect);
    virtual void				DrawSlider();
    virtual void				DrawBar();
    virtual void				DrawHashMarks();
    virtual void				DrawThumb();
    virtual void				DrawFocusMark();
    virtual	void				DrawText();
    virtual const char*			UpdateText() const;
            void				UpdateTextChanged();

    virtual BRect				BarFrame() const;
    virtual BRect				HashMarksFrame() const;
    virtual BRect				ThumbFrame() const;

    virtual	void				SetFlags(uint32_t flags);
    virtual	void				SetResizingMode(uint32_t mode);

    virtual void				GetPreferredSize(float* _width,
                                    float* _height);
    virtual void				ResizeToPreferred();

    virtual status_t			Invoke(BMessage* message = NULL);
    virtual BHandler*			ResolveSpecifier(BMessage* message,
                                    int32_t index, BMessage* specifier,
                                    int32_t form, const char* property);
    virtual	status_t			GetSupportedSuites(BMessage* data);

    virtual	void				SetModificationMessage(BMessage* message);
            BMessage*			ModificationMessage() const;

    virtual void				SetSnoozeAmount(int32_t microSeconds);
            int32_t				SnoozeAmount() const;

    virtual	void				SetKeyIncrementValue(int32_t value);
            int32_t				KeyIncrementValue()	const;

    virtual	void				SetHashMarkCount(int32_t count);
            int32_t				HashMarkCount() const;

    virtual	void				SetHashMarks(hash_mark_location where);
            hash_mark_location	HashMarks() const;

    virtual	void				SetStyle(thumb_style style);
            thumb_style			Style() const;

    virtual	void				SetBarColor(rgb_color color);
            rgb_color			BarColor() const;
    virtual	void				UseFillColor(bool useFill,
                                    const rgb_color* color = NULL);
            bool				FillColor(rgb_color* color) const;

            BView*				OffscreenView() const;

            orientation			Orientation() const;
    virtual void				SetOrientation(orientation);

            float				BarThickness() const;
    virtual void				SetBarThickness(float thickness);

    virtual void				SetFont(const BFont* font,
                                    uint32_t properties = B_FONT_ALL);

    virtual void				SetLimits(int32_t minimum, int32_t maximum);

    virtual	float				MaxUpdateTextWidth();

    virtual	BSize				MinSize();
    virtual	BSize				MaxSize();
    virtual	BSize				PreferredSize();

    virtual	status_t			SetIcon(const BBitmap* icon, uint32_t flags = 0);

protected:
    virtual	void				LayoutInvalidated(bool descendants);

private:
            void				_DrawBlockThumb();
            void				_DrawTriangleThumb();

            BPoint				_Location() const;
            void				_SetLocationForValue(int32_t value);

            float				_MinPosition() const;
            float				_MaxPosition() const;
            bool				_ConstrainPoint(BPoint& point,
                                    BPoint compare) const;

            BSize				_ValidateMinSize();

            void				_InitBarColor();
            void				_InitObject();

private:
    // FBC padding and forbidden methods
    virtual	void				_ReservedSlider6();
    virtual	void				_ReservedSlider7();
    virtual	void				_ReservedSlider8();
    virtual	void				_ReservedSlider9();
    virtual	void				_ReservedSlider10();
    virtual	void				_ReservedSlider11();
    virtual	void				_ReservedSlider12();

            BSlider&			operator=(const BSlider& other);

private:
            BMessage*			fModificationMessage;
            int32_t				fSnoozeAmount;

            rgb_color 			fBarColor;
            rgb_color 			fFillColor;
            bool				fUseFillColor;

            char*				fMinLimitLabel;
            char*				fMaxLimitLabel;
            const char*			fUpdateText;

            int32_t 				fMinValue;
            int32_t 				fMaxValue;
            int32_t 				fKeyIncrementValue;

            int32_t 				fHashMarkCount;
            hash_mark_location 	fHashMarks;

            BBitmap*			fOffScreenBits;
            BView*				fOffScreenView;

            thumb_style			fStyle;

            BPoint 				fLocation;
            BPoint				fInitialLocation;

            orientation			fOrientation;
            float				fBarThickness;

            BSize				fMinSize;

            float				fMaxUpdateTextWidth;

            uint32_t				_reserved[4];
};


#endif	// _SLIDER_H
