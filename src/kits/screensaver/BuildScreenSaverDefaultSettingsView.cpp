/*
 * Copyright 2009, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ryan Leavengood, leavengood@gmail.com
 */


#include <BuildScreenSaverDefaultSettingsView.h>

#include <kits/support/StringView.h>
#include <View.h>


namespace BPrivate {

// This is used to provide a consistent look for the settings view for screen 
// savers without any configuration.
void BuildScreenSaverDefaultSettingsView(BView* view, const char* moduleName, const char* info)
{
	BRect rect(15, 15, 20, 20);
	BStringView* stringView = new BStringView(rect, "module", moduleName);
	stringView->SetFont(__be_bold_font);
	stringView->ResizeToPreferred();
	view->AddChild(stringView);

	rect.OffsetBy(0, stringView->Bounds().Height() + 4);
	stringView = new BStringView(rect, "info", info);
	stringView->ResizeToPreferred();
	view->AddChild(stringView);
}

}

