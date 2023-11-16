/*
 * Copyright 2008-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHANNEL_CONTROL_H
#define _CHANNEL_CONTROL_H


//! BChannelControl is the base class for controls that have several
// independent values, with minima and maxima.

#include <Control.h>
#include <kits/support/String.h>


class BChannelControl : public BControl {
public:
								BChannelControl(BRect frame, const char* name,
									const char* label, BMessage* model,
									int32_t channelCount = 1,
									uint32_t resizingMode
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BChannelControl(const char* name,
									const char* label, BMessage* model,
									int32_t channelCount = 1,
									uint32_t flags = B_WILL_DRAW);
								BChannelControl(BMessage* archive);
	virtual						~BChannelControl();

	virtual	status_t			Archive(BMessage* data, bool deep = true) const;

	virtual	void				Draw(BRect updateRect) = 0;
	virtual	void				MouseDown(BPoint where) = 0;
	virtual	void				KeyDown(const char* bytes, int32_t numBytes) = 0;

	virtual	void				FrameResized(float newWidth, float newHeight);
	virtual	void				SetFont(const BFont* font,
									uint32_t mask = B_FONT_ALL);

	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				ResizeToPreferred();
	virtual	void				GetPreferredSize(float* width,
									float* height) = 0;

	virtual	void				MessageReceived(BMessage* message);

	virtual	BHandler*			ResolveSpecifier(BMessage* message, int32_t index,
									BMessage* specifier, int32_t what,
									const char* property);
	virtual	status_t			GetSupportedSuites(BMessage* data);

	virtual	void				SetModificationMessage(BMessage* message);
			BMessage*			ModificationMessage() const;

	virtual	status_t			Invoke(BMessage* message = NULL);

	//! These methods are similar to Invoke() Invoke() and InvokeNotify(), but
	// include additional information about all of the channels in the control.
	virtual	status_t			InvokeChannel(BMessage* message = NULL,
									int32_t fromChannel = 0,
									int32_t channelCount = -1,
									const bool* _mask = NULL);
			status_t			InvokeNotifyChannel(BMessage* message = NULL,
									uint32_t kind = B_CONTROL_INVOKED,
									int32_t fromChannel = 0,
									int32_t channelCount = -1,
									const bool* _mask = NULL);

	virtual	void				SetValue(int32_t value);
		// SetCurrentChannel() determines which channel
	virtual	status_t			SetCurrentChannel(int32_t index);
			int32_t				CurrentChannel() const;

	virtual	int32_t				CountChannels() const;
	virtual	int32_t				MaxChannelCount() const = 0;
	virtual	status_t			SetChannelCount(int32_t count);
			int32_t				ValueFor(int32_t channel) const;
	virtual	int32_t				GetValue(int32_t* _values, int32_t fromChannel,
									int32_t channelCount) const;
			status_t			SetValueFor(int32_t channel, int32_t value);
	virtual	status_t			SetValue(int32_t fromChannel, int32_t channelCount,
									const int32_t* values);
			status_t			SetAllValue(int32_t values);
			status_t			SetLimitsFor(int32_t channel, int32_t minimum,
									int32_t maximum);
			status_t			GetLimitsFor(int32_t channel, int32_t* _minimum,
									int32_t* _maximum) const ;
	virtual	status_t			SetLimitsFor(int32_t fromChannel,
									int32_t channelCount, const int32_t* minima,
									const int32_t* maxima);
	virtual	status_t			GetLimitsFor(int32_t fromChannel,
									int32_t channelCount, int32_t* minima,
									int32_t* maxima) const;
			status_t			SetLimits(int32_t minimum, int32_t maximum);
			status_t			GetLimits(int32_t* _minimum,
									int32_t* _maximum) const;

	virtual	bool				SupportsIndividualLimits() const = 0;
	virtual	status_t			SetLimitLabels(const char* minLabel,
									const char* maxLabel);
			const char* 		MinLimitLabel() const;
			const char* 		MaxLimitLabel() const;
	virtual	status_t			SetLimitLabelsFor(int32_t channel,
									const char* minLabel, const char* maxLabel);
	virtual	status_t			SetLimitLabelsFor(int32_t fromChannel,
									int32_t channelCount, const char* minLabel,
									const char* maxLabel);
			const char*			MinLimitLabelFor(int32_t channel) const;
			const char*			MaxLimitLabelFor(int32_t channel) const;
private:
	// Forbidden (and unimplemented)
								BChannelControl(const BChannelControl& other);
			BChannelControl&	operator=(const BChannelControl& other);

	virtual	void				_Reserverd_ChannelControl_0(void*, ...);
	virtual	void				_Reserverd_ChannelControl_1(void*, ...);
	virtual	void				_Reserverd_ChannelControl_2(void*, ...);
	virtual	void				_Reserverd_ChannelControl_3(void*, ...);
	virtual	void				_Reserverd_ChannelControl_4(void*, ...);
	virtual	void				_Reserverd_ChannelControl_5(void*, ...);
	virtual	void				_Reserverd_ChannelControl_6(void*, ...);
	virtual	void				_Reserverd_ChannelControl_7(void*, ...);
	virtual	void				_Reserverd_ChannelControl_8(void*, ...);
	virtual	void				_Reserverd_ChannelControl_9(void*, ...);
	virtual	void				_Reserverd_ChannelControl_10(void*, ...);
	virtual	void				_Reserverd_ChannelControl_11(void*, ...);

protected:
	inline	int32_t* const&		MinLimitList() const;
	inline	int32_t* const&		MaxLimitList() const;
	inline	int32_t* const&		ValueList() const;

private:
			int32_t				fChannelCount;
			int32_t				fCurrentChannel;
			int32_t*				fChannelMin;
			int32_t*				fChannelMax;
			int32_t*				fChannelValues;

			BString				fMinLabel;
			BString				fMaxLabel;

			void*				fMultiLabels;

			BMessage*			fModificationMsg;

			uint32_t				_reserved_[15];

			status_t			StuffValues(int32_t fromChannel,
									int32_t channelCount, const int32_t* values);
};


inline int32_t* const&
BChannelControl::MinLimitList() const
{
	return fChannelMin;
}


inline int32_t* const&
BChannelControl::MaxLimitList() const
{
	return fChannelMax;
}


inline int32_t* const&
BChannelControl::ValueList() const
{
	return fChannelValues;
}


#endif // _CHANNEL_CONTROL_H
