/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _OPTION_POP_UP_H
#define _OPTION_POP_UP_H


#include <OptionControl.h>


class BMenuField;


class BOptionPopUp : public BOptionControl {
public:
								BOptionPopUp(BRect frame, const char* name,
									const char* label, BMessage* message,
									uint32_t resizeMask = B_FOLLOW_LEFT
										| B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BOptionPopUp(BRect frame, const char* name,
									const char* label,  BMessage* message,
									bool fixed, uint32_t resizeMask
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								BOptionPopUp(const char* name,
									const char* label, BMessage* message,
									uint32_t flags = B_WILL_DRAW);

	virtual						~BOptionPopUp();

			BMenuField*			MenuField();

	virtual	bool				GetOptionAt(int32_t index, const char** _name,
									int32_t* _value);
	virtual	void				RemoveOptionAt(int32_t index);
	virtual	int32_t				CountOptions() const;
	virtual	status_t 			AddOptionAt(const char* name, int32_t value,
									int32_t index);

	virtual	void				AllAttached();
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				SetLabel(const char* text);
	virtual	void				SetValue(int32_t value);
	virtual	void				SetEnabled(bool enabled);
	virtual	void				GetPreferredSize(float* _width,
									float* _height);
	virtual	void				ResizeToPreferred();

	virtual	int32_t				SelectedOption(const char** _name = 0,
									int32_t* _value = 0) const;
private:
	// Forbidden and FBC padding
								BOptionPopUp();
								BOptionPopUp(const BOptionPopUp& other);
			BOptionPopUp&		operator=(const BOptionPopUp& other);


	virtual	status_t		 	_Reserved_OptionControl_0(void*, ...);
	virtual	status_t 		 	_Reserved_OptionControl_1(void*, ...);
	virtual	status_t 		 	_Reserved_OptionControl_2(void*, ...);
	virtual	status_t 		 	_Reserved_OptionControl_3(void*, ...);

	virtual	status_t 		 	_Reserved_OptionPopUp_0(void*, ...);
	virtual	status_t 		 	_Reserved_OptionPopUp_1(void*, ...);
	virtual	status_t 		 	_Reserved_OptionPopUp_2(void*, ...);
	virtual	status_t 		 	_Reserved_OptionPopUp_3(void*, ...);
	virtual	status_t 		 	_Reserved_OptionPopUp_4(void*, ...);
	virtual	status_t 		 	_Reserved_OptionPopUp_5(void*, ...);
	virtual	status_t		 	_Reserved_OptionPopUp_6(void*, ...);
	virtual	status_t		 	_Reserved_OptionPopUp_7(void*, ...);

private:
			BMenuField*			fMenuField;

			uint32_t				_reserved[8];
};

#endif // _OPTION_POP_UP_H
