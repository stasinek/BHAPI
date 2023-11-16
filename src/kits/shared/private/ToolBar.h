/*
 * Copyright 2011-2015, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include <GroupView.h>


class BButton;

namespace BPrivate {

class BToolBar : public BGroupView {
public:
								BToolBar(BRect frame,
										 orientation ont = B_HORIZONTAL);
								BToolBar(orientation ont = B_HORIZONTAL);
	virtual						~BToolBar();

	virtual	void				Hide();

			void				AddAction(uint32_t command, BHandler* target,
									const BBitmap* icon,
									const char* toolTipText = NULL,
									const char* text = NULL,
									bool lockable = false);
			void				AddAction(BMessage* message, BHandler* target,
									const BBitmap* icon,
									const char* toolTipText = NULL,
									const char* text = NULL,
									bool lockable = false);
			void				AddSeparator();
			void				AddGlue();
			void				AddView(BView* view);

			void				SetActionEnabled(uint32_t command, bool enabled);
			void				SetActionPressed(uint32_t command, bool pressed);
			void				SetActionVisible(uint32_t command, bool visible);

			BButton*			FindButton(uint32_t command) const;

private:
	virtual	void				Pulse();
	virtual	void				FrameResized(float width, float height);

			void				_Init();
			void				_HideToolTips() const;

			orientation			fOrientation;
};

} // namespace BPrivate

using BPrivate::BToolBar;

#endif // _TOOLBAR_H
