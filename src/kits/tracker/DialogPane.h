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
#ifndef _DIALOG_PANE_H
#define _DIALOG_PANE_H


#include <Control.h>
#include <ObjectList.h>


namespace BPrivate {

class ViewList : public BObjectList<BView> {
public:
								ViewList()
									:
									BObjectList<BView>(5, true)
								{
								}

			void				RemoveAll(BView* fromParent);
			void				AddAll(BView* toParent);
};


class DialogPane : public BView {
	// dialog with collapsible panes
public:
								DialogPane(BRect mode1Frame, BRect mode2Frame,
									int32_t initialMode, const char* name,
									uint32_t followFlags
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);
								DialogPane(BRect mode1Frame, BRect mode2Frame,
									BRect mode3Frame, int32_t initialMode,
									const char* name, uint32_t followFlags
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW);

	virtual						~DialogPane();

			BRect				FrameForMode(int32_t);
			BRect				BoundsForMode(int32_t);

			int32_t				Mode() const;
	virtual	void				SetMode(int32_t, bool initialSetup = false);

			void				AddItem(BView*, int32_t toMode);

			void				SetSwitch(BControl*);

	virtual	void				AttachedToWindow();

protected:
			void				ResizeParentWindow(int32_t from, int32_t to);
	static	BRect				FrameForMode(int32_t, BRect, BRect, BRect);
		// called only by the constructor

	virtual	void				MessageReceived(BMessage* message);

private:
			int32_t				fMode;

			BRect				fMode1Frame;
			BRect				fMode2Frame;
			BRect				fMode3Frame;

			ViewList			fMode2Items;
			ViewList			fMode3Items;
			BControl*			fLatch;

	typedef BView _inherited;
};


inline int32_t DialogPane::Mode() const
{
	return fMode;
}


class PaneSwitch : public BControl {
public:
								PaneSwitch(BRect frame, const char* name,
									bool leftAligned = true,
									uint32_t resizeMask
										= B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32_t flags = B_WILL_DRAW | B_NAVIGABLE);

								PaneSwitch(const char* name,
									bool leftAligned = true,
									uint32_t flags = B_WILL_DRAW | B_NAVIGABLE);

	virtual						~PaneSwitch();

	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);

	virtual	void				GetPreferredSize(float* _width,
									float* _height);

	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			void				SetLabels(const char* labelOn,
									const char* labelOff);

protected:
			void				DoneTracking(BPoint where);
			void				Track(BPoint where, uint32_t);

			enum State {
				kCollapsed,
				kPressed,
				kExpanded
			};

	virtual	void				DrawInState(PaneSwitch::State state);

private:
			bool				fLeftAligned;
			bool				fPressing;

			char*				fLabelOn;
			char*				fLabelOff;

	static	const int32_t			sLatchSize = 11;
};

} // namespace BPrivate

using namespace BPrivate;


#endif	// _DIALOG_PANE_H
