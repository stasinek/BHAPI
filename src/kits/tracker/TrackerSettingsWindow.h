/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/
#ifndef _TRACKER_SETTINGS_WINDOW_H
#define _TRACKER_SETTINGS_WINDOW_H


#include <Box.h>
#include <Button.h>
#include <ListView.h>
#include <ListItem.h>
#include <Window.h>

#include <SettingsViews.h>


namespace BPrivate {

class TrackerSettingsWindow : public BWindow {
public:
	TrackerSettingsWindow();

	bool QuitRequested();
	void MessageReceived(BMessage* message);
	void Show();

	enum SettingsPage {
		kDesktopSettings,
		kWindowsSettings,
		kSpaceBarSettings,
		kAutomountSettings
	};

	void ShowPage(SettingsPage page);

private:
	SettingsView* _ViewAt(int32_t i);

	void _HandleChangedContents();
	void _HandlePressedDefaultsButton();
	void _HandlePressedRevertButton();
	void _HandleChangedSettingsView();
	void _UpdateButtons();

	BListView*	fSettingsTypeListView;
	BBox*		fSettingsContainerBox;
	BButton*	fDefaultsButton;
	BButton*	fRevertButton;

	typedef BWindow _inherited;
};

} // namespace BPrivate

using namespace BPrivate;


#endif	// _TRACKER_SETTINGS_WINDOW_H
