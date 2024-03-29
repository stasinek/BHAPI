/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: FilePanel.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_FILE_PANEL_H
#define BHAPI_FILE_PANEL_H

#include <Haiku.h>

namespace bhapi {
typedef enum file_panel_mode {
    B_OPEN_PANEL,
    B_SAVE_PANEL
} file_panel_mode;

typedef enum file_panel_button {
    B_CANCEL_BUTTON,
    B_DEFAULT_BUTTON
} file_panel_button;
} /* namespace */

#ifdef __cplusplus /* Just for C++ */

class BPath;
class BEntry;
class BDirectory;
class BWindow;
class BMessenger;
class BMessage;
class BHAPI_IMPEXP BFilePanelFilter {
public:
	virtual ~BFilePanelFilter();

	virtual bool		Filter(const BEntry *entry) = 0;
};


class BHAPI_IMPEXP BFilePanel {
public:
    BFilePanel(bhapi::file_panel_mode mode = bhapi::B_OPEN_PANEL,
           const BMessenger *target = NULL,
		   const char *panel_directory = NULL,
            uint32_t node_flavors = 0,
		   bool allow_multiple_selection = true,
           const BMessage *message = NULL,
		   BFilePanelFilter *filter = NULL,
		   bool modal = false,
		   bool hide_when_done = true);
	virtual ~BFilePanel();

	void			Show();
	void			Hide();
	bool			IsShowing() const;

	// Empty functions BEGIN --- just for derivative class
	virtual void		WasHidden();
	virtual void		SelectionChanged();
	// Empty functions END

	#undef SendMessage
    virtual void		SendMessage(const BMessenger *msgr, BMessage *msg);

    BWindow			*Window() const;
    BMessenger		*Target() const;
	BFilePanelFilter	*Filter() const;

    bhapi::file_panel_mode	PanelMode() const;

    void			SetTarget(const BMessenger *target);
    void			SetMessage(const BMessage *msg);

	void			SetFilter(BFilePanelFilter *filter);
	void			SetSaveText(const char *text);
    void			SetButtonLabel(bhapi::file_panel_button btn, const char *label);

	void			SetHideWhenDone(bool state);
	bool			HidesWhenDone() const;

	void			GetPanelDirectory(BEntry *entry) const;
    void			GetPanelDirectory(BPath *path) const;
	void			GetPanelDirectory(BDirectory *directory) const;

	void			SetPanelDirectory(const BEntry *entry);
	void			SetPanelDirectory(const BDirectory *directory);
	void			SetPanelDirectory(const char *directory);

	void			Refresh();
	void			Rewind();
	status_t		GetNextSelected(BEntry *entry);

private:
    BWindow *fWindow;
};

#endif /* __cplusplus */

#endif /* BHAPI_FILE_PANEL_H */

