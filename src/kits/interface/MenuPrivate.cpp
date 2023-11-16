/*
 * Copyright 2001-2009, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini (stefano.ceccherini@gmail.com)
 */

#include <MenuPrivate.h>

#include <Bitmap.h>
#include <Menu.h>


const unsigned char kShiftBits[] = {
	0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x17,0x04,0x04,0x17,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x17,0x17,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x04,0x04,0x1a,0x04,0x04,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x17,0x04,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x04,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x17,0x17,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x17,0x04,0x04,0x17,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x14,
	0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14
};


const unsigned char kCtrlBits[] = {
	0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x13,0x04,0x04,0x13,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x04,0x1a,0x04,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x04,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x13,0x04,0x04,0x13,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x13,0x04,0x1a,0x04,0x04,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x14,
	0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14
};


const unsigned char kOptBits[] = {
	0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x13,0x04,0x04,0x13,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x04,0x04,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x13,0x04,0x04,0x13,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x14,
	0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14
};


const unsigned char kAltBits[] = {
	0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x13,0x04,0x04,0x13,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x04,0x04,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x04,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x14,
	0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14
};


const unsigned char kMenuBits[] = {
	0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x04,0x1a,0x04,0x04,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x04,0x04,0x04,0x04,0x1a,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x04,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x04,0x04,0x04,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x04,0x1a,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x04,0x1a,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x1a,0x04,0x1a,0x1a,0x04,0x1a,0x04,0x04,0x04,0x1a,0x17,0x14,
	0x1d,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x1a,0x17,0x14,
	0x1d,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x17,0x14,
	0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14
};


namespace BPrivate {


BBitmap* MenuPrivate::sMenuItemShift;
BBitmap* MenuPrivate::sMenuItemControl;
BBitmap* MenuPrivate::sMenuItemOption;
BBitmap* MenuPrivate::sMenuItemAlt;
BBitmap* MenuPrivate::sMenuItemMenu;


MenuPrivate::MenuPrivate(BMenu* menu)
	:
	fMenu(menu)
{
}


menu_layout
MenuPrivate::Layout() const
{
	return fMenu->Layout();
}


void MenuPrivate::SetLayout(menu_layout layout)
{
	fMenu->fLayout = layout;
}


void MenuPrivate::ItemMarked(BMenuItem *item)
{
	fMenu->_ItemMarked(item);
}


void MenuPrivate::CacheFontInfo()
{
	fMenu->_CacheFontInfo();
}


float
MenuPrivate::FontHeight() const
{
	return fMenu->fFontHeight;
}


float
MenuPrivate::Ascent() const
{
	return fMenu->fAscent;
}


BRect
MenuPrivate::Padding() const
{
	return fMenu->fPad;
}


void MenuPrivate::GetItemMargins(float* left, float* top, float* right,
	float* bottom) const
{
	fMenu->GetItemMargins(left, top, right, bottom);
}


int
MenuPrivate::State(BMenuItem** item) const
{
	return fMenu->_State(item);
}


void MenuPrivate::Install(BWindow* window)
{
	fMenu->_Install(window);
}


void MenuPrivate::Uninstall()
{
	fMenu->_Uninstall();
}


void MenuPrivate::SetSuper(BMenu* menu)
{
	fMenu->fSuper = menu;
}


void MenuPrivate::SetSuperItem(BMenuItem* item)
{
	fMenu->fSuperitem = item;
}


void MenuPrivate::InvokeItem(BMenuItem* item, bool now)
{
	fMenu->_InvokeItem(item, now);
}


void MenuPrivate::QuitTracking(bool thisMenuOnly)
{
	fMenu->_QuitTracking(thisMenuOnly);
}


/* static */
status_t MenuPrivate::CreateBitmaps()
{
	BRect smallRect(0, 0, 16, 10);

	try {
		sMenuItemShift = new BBitmap(BRect(0, 0, 23, 10), B_CMAP8);
		sMenuItemControl = new BBitmap(BRect(0, 0, 21, 10), B_CMAP8);
		sMenuItemOption = new BBitmap(smallRect, B_CMAP8);
		sMenuItemAlt = new BBitmap(smallRect, B_CMAP8);
		sMenuItemMenu = new BBitmap(BRect(0, 0, 22, 10), B_CMAP8);
	} catch (...) {
		return B_NO_MEMORY;
	}

	sMenuItemShift->ImportBits(kShiftBits, sizeof(kShiftBits),
		24, 0, B_CMAP8);
	sMenuItemControl->ImportBits(kCtrlBits, sizeof(kCtrlBits),
		22, 0, B_CMAP8);
	sMenuItemOption->ImportBits(kOptBits, sizeof(kOptBits),
		17, 0, B_CMAP8);
	sMenuItemAlt->ImportBits(kAltBits, sizeof(kAltBits),
		17, 0, B_CMAP8);
	sMenuItemMenu->ImportBits(kMenuBits, sizeof(kMenuBits),
		23, 0, B_CMAP8);
	
	return B_OK;
}


/* static */
void MenuPrivate::DeleteBitmaps()
{
	delete sMenuItemShift;
	delete sMenuItemControl;
	delete sMenuItemOption;
	delete sMenuItemAlt;
	delete sMenuItemMenu;
}


/* static */
const BBitmap*
MenuPrivate::MenuItemShift()
{
	return sMenuItemShift;
}


/* static */
const BBitmap*
MenuPrivate::MenuItemControl()
{
	switch (BMenu::sControlKey) {
		case 0x5d:
		case 0x5f:
			return sMenuItemAlt;
		case 0x66:
		case 0x67:
			return sMenuItemOption;
	}

	return sMenuItemControl;
}


/* static */
const BBitmap*
MenuPrivate::MenuItemOption()
{
	switch (BMenu::sOptionKey) {
		case 0x5c:
		case 0x60:
			return sMenuItemControl;
		case 0x66:
		case 0x67:
			return sMenuItemOption;
	}

	return sMenuItemAlt;
}


/* static */
const BBitmap*
MenuPrivate::MenuItemCommand()
{
	switch (BMenu::sCommandKey) {
		case 0x5c:
		case 0x60:
			return sMenuItemControl;
		case 0x66:
		case 0x67:
			return sMenuItemOption;
	}

	return sMenuItemAlt;
}


/* static */
const BBitmap*
MenuPrivate::MenuItemMenu()
{
	return sMenuItemMenu;
}


}	// namespace BPrivate
