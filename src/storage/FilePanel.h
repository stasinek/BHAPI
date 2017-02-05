/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#ifndef __ETK_FILE_PANEL_H__
#define __ETK_FILE_PANEL_H__

#include "./../interface/Window.h>
#include "./../storage/Directory.h>

typedef enum e_file_panel_mode {
	E_OPEN_PANEL,
	E_SAVE_PANEL
} e_file_panel_mode;

typedef enum e_file_panel_button {
	E_CANCEL_BUTTON,
	E_DEFAULT_BUTTON
} e_file_panel_button;

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EFilePanelFilter {
public:
	virtual ~EFilePanelFilter();

	virtual bool		Filter(const EEntry *entry) = 0;
};


class _IMPEXP_ETK EFilePanel {
public:
	EFilePanel(e_file_panel_mode mode = E_OPEN_PANEL,
		   const EMessenger *target = NULL,
		   const char *panel_directory = NULL,
		   euint32 node_flavors = 0,
		   bool allow_multiple_selection = true,
		   const EMessage *message = NULL,
		   EFilePanelFilter *filter = NULL,
		   bool modal = false,
		   bool hide_when_done = true);
	virtual ~EFilePanel();

	void			Show();
	void			Hide();
	bool			IsShowing() const;

	// Empty functions BEGIN --- just for derivative class
	virtual void		WasHidden();
	virtual void		SelectionChanged();
	// Empty functions END

	virtual void		SendMessage(const EMessenger *msgr, EMessage *msg);

	EWindow			*Window() const;
	EMessenger		*Target() const;
	EFilePanelFilter	*Filter() const;

	e_file_panel_mode	PanelMode() const;

	void			SetTarget(const EMessenger *target);
	void			SetMessage(const EMessage *msg);

	void			SetFilter(EFilePanelFilter *filter);
	void			SetSaveText(const char *text);
	void			SetButtonLabel(e_file_panel_button btn, const char *label);

	void			SetHideWhenDone(bool state);
	bool			HidesWhenDone() const;

	void			GetPanelDirectory(EEntry *entry) const;
	void			GetPanelDirectory(EPath *path) const;
	void			GetPanelDirectory(EDirectory *directory) const;

	void			SetPanelDirectory(const EEntry *entry);
	void			SetPanelDirectory(const EDirectory *directory);
	void			SetPanelDirectory(const char *directory);

	void			Refresh();
	void			Rewind();
	e_status_t		GetNextSelected(EEntry *entry);

private:
	EWindow *fWindow;
};

#endif /* __cplusplus */

#endif /* __ETK_FILE_PANEL_H__ */

