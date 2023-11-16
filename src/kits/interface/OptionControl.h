/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _OPTION_CONTROL_H
#define _OPTION_CONTROL_H


#include <Control.h>


enum {
	B_OPTION_CONTROL_VALUE = '_BMV'
};


class BOptionControl : public BControl {
public:
								BOptionControl(BRect frame, const char* name,
									const char* label, BMessage* message,
									uint32_t resizeMask = B_FOLLOW_LEFT
										| B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BOptionControl(const char* name,
									const char* label, BMessage* message,
									uint32_t flags = B_WILL_DRAW);

	virtual						~BOptionControl();

	virtual	void				MessageReceived(BMessage* message);

			status_t			AddOption(const char* name, int32_t value);
	virtual	bool				GetOptionAt(int32_t index, const char** _name,
									int32_t* _value) = 0;
	virtual	void				RemoveOptionAt(int32_t index) = 0;
	virtual	int32_t				CountOptions() const = 0;
	virtual	status_t			AddOptionAt(const char* name, int32_t value,
									int32_t index) = 0;
	virtual	int32_t				SelectedOption(const char** name = NULL,
									int32_t* outValue = NULL) const = 0;

	virtual	status_t			SelectOptionFor(int32_t value);
	virtual	status_t			SelectOptionFor(const char* name);

protected:
			BMessage*			MakeValueMessage(int32_t value);

private:
	// FBC padding and forbidden methods
								BOptionControl();
								BOptionControl(const BOptionControl& other);
			BOptionControl&		operator=(const BOptionControl& other);

	virtual	status_t			_Reserved_OptionControl_0(void*, ...);
	virtual	status_t			_Reserved_OptionControl_1(void*, ...);
	virtual	status_t			_Reserved_OptionControl_2(void*, ...);
	virtual	status_t			_Reserved_OptionControl_3(void*, ...);
	virtual	status_t			_Reserved_OptionControl_4(void*, ...);
	virtual	status_t			_Reserved_OptionControl_5(void*, ...);
	virtual	status_t			_Reserved_OptionControl_6(void*, ...);
	virtual	status_t			_Reserved_OptionControl_7(void*, ...);
	virtual	status_t			_Reserved_OptionControl_8(void*, ...);
	virtual	status_t			_Reserved_OptionControl_9(void*, ...);
	virtual	status_t			_Reserved_OptionControl_10(void*, ...);
	virtual	status_t			_Reserved_OptionControl_11(void*, ...);

private:
			uint32_t				_reserved[8];
};


#endif // _OPTION_CONTROL_H
