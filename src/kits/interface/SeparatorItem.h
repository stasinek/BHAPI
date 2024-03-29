/*
 * Copyright 2001-2014 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SEPARATOR_ITEM_H
#define _SEPARATOR_ITEM_H


#include <MenuItem.h>

class BMessage;


class BSeparatorItem : public BMenuItem {
public:
								BSeparatorItem();
								BSeparatorItem(BMessage* data);
	virtual						~BSeparatorItem();

	static	BArchivable*		Instantiate(BMessage* data);
	virtual	status_t			Archive(BMessage* data,
									bool deep = true) const;

	virtual	void				SetEnabled(bool enable);

protected:
	virtual	void				GetContentSize(float* _width, float* _height);
	virtual	void				Draw();

private:
	// FBC padding, reserved and forbidden
	virtual	void				_ReservedSeparatorItem1();
	virtual	void				_ReservedSeparatorItem2();

			BSeparatorItem&		operator=(const BSeparatorItem& other);

			uint32_t				_reserved[1];
};


#endif // _SEPARATOR_ITEM_H
