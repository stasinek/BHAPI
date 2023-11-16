/*
 * Copyright 2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MULTI_CHANNEL_CONTROL_H
#define _MULTI_CHANNEL_CONTROL_H


#include <Control.h>


class BMultiChannelControl : public BControl {
public:
								BMultiChannelControl(BRect frame,
									const char* name, const char* label,
									BMessage* message, int32_t channelCount = 1,
									uint32_t resize = B_FOLLOW_LEFT
										| B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BMultiChannelControl(BMessage* archive);
	virtual						~BMultiChannelControl();

	virtual	status_t			Archive(BMessage* archive,
									bool deep = true) const;

	virtual	void				Draw(BRect updateRect) = 0;
	virtual	void				MouseDown(BPoint where) = 0;
	virtual	void				KeyDown(const char* bytes, int32_t numBytes) = 0;

	virtual	void				FrameResized(float width, float height);
	virtual	void				SetFont(const BFont* font,
									uint32_t mask = B_FONT_ALL);

	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				ResizeToPreferred();
	virtual	void				GetPreferredSize(float* _width,
									float* _height) = 0;

	virtual	void				MessageReceived(BMessage* message);

	//! SetValueChannel() determines which channel
	virtual	void				SetValue(int32_t value);
	virtual	status_t			SetCurrentChannel(int32_t channel);
			int32_t				CurrentChannel() const;

	virtual	int32_t				CountChannels() const;
	virtual	int32_t				MaxChannelCount() const = 0;
	virtual	status_t			SetChannelCount(int32_t channelCount);
			int32_t				ValueFor(int32_t channel) const;
	virtual	int32_t				GetValues(int32_t* _values, int32_t firstChannel,
									int32_t channelCount) const;
			status_t			SetValueFor(int32_t channel, int32_t value);
	virtual	status_t			SetValues(int32_t firstChannel,
									int32_t channelCount, const int32_t* _values);
			status_t			SetAllValues(int32_t values);
			status_t			SetLimitsFor(int32_t channel, int32_t minimum,
									int32_t maximum);
			status_t			GetLimitsFor(int32_t channel, int32_t* _minimum,
									int32_t* _maximum) const;
	virtual	status_t			SetLimits(int32_t firstChannel,
									int32_t channelCount, const int32_t* minimum,
									const int32_t* maximum);
	virtual	status_t			GetLimits(int32_t firstChannel,
									int32_t channelCount, int32_t* _minimum,
									int32_t* _maximum) const;
			status_t			SetAllLimits(int32_t minimum, int32_t maximum);

	virtual	status_t			SetLimitLabels(const char* minLabel,
									const char* maxLabel);
			const char*			MinLimitLabel() const;
			const char*			MaxLimitLabel() const;

private:
	// FBC padding
	virtual	status_t			_Reserverd_MultiChannelControl_0(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_1(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_2(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_3(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_4(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_5(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_6(void*, ...);
	virtual	status_t			_Reserverd_MultiChannelControl_7(void*, ...);

	// Forbidden
								BMultiChannelControl(
									const BMultiChannelControl&);
			BMultiChannelControl& operator=(const BMultiChannelControl&);


protected:
	inline	int32_t* const&		MinLimitList() const;
	inline	int32_t* const&		MaxLimitList() const;
	inline	int32_t* const&		ValueList() const;

private:
			int32_t				fChannelCount;
			int32_t				fValueChannel;
			int32_t*				fChannelMinima;
			int32_t*				fChannelMaxima;
			int32_t*				fChannelValues;
			char*				fMinLabel;
			char*				fMaxLabel;

			uint32_t				_reserved_[16];
};


inline int32_t* const&
BMultiChannelControl::MinLimitList() const
{
	return fChannelMinima;
}


inline int32_t* const&
BMultiChannelControl::MaxLimitList() const
{
	return fChannelMaxima;
}


inline int32_t* const&
BMultiChannelControl::ValueList() const
{
	return fChannelValues;
}


#endif // _MULTI_CHANNEL_CONTROL_H

