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
 * File: ListItem.h
 * Description: EListItem --- item for EListView/EOutlineListView
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LIST_ITEM_H__
#define __ETK_LIST_ITEM_H__

#include "./../support/Archivable.h"
#include "./../support/List.h"
#include "./../interface/View.h"

#ifdef __cplusplus /* Just for C++ */

class EListView;
class EOutlineListView;

class _IMPEXP_ETK EListItem : public EArchivable {
public:
	EListItem(euint32 outlineLevel = 0, bool expanded = true, euint32 flags = 0);
	virtual ~EListItem();

	// Archiving
	EListItem(EMessage *from);
	virtual e_status_t Archive(EMessage *into, bool deep = true) const;

	float		Height() const;
	float		Width() const;
	bool		IsSelected() const;
	void		Select();
	void		Deselect();

	virtual void	SetEnabled(bool on);
	bool		IsEnabled() const;

	void		SetHeight(float height);
	void		SetWidth(float width);

	virtual void	SetFlags(euint32 flags);
	euint32		Flags() const;

	void		Invalidate();

	// for item of EOutlineListView
	bool 		IsExpanded() const;
	void 		SetExpanded(bool expanded);
	euint32 	OutlineLevel() const;
	bool		IsVisible() const;
	EListItem	*SuperItem() const;
	bool		HasSubitems() const;

protected:
	// for item of EOutlineListView
	void		DrawLeader(EView *owner, ERect *itemRect);
	void		GetLeaderSize(float *width, float *height) const;

private:
	friend class EListView;
	friend class EOutlineListView;

	EListView *fOwner;
	EOutlineListView *fFullOwner;

	euint32 	fLevel;
	bool 		fExpanded;
	euint32		fFlags;

	float		fWidth;
	float		fHeight;
	bool		fSelected;
	bool		fEnabled;

	virtual void	DrawItem(EView *owner, ERect itemRect, bool drawEverything) = 0;
	virtual void	Update(EView *owner, const EFont *font) = 0;

	virtual void	MouseDown(EView *owner, EPoint where);
	virtual void	MouseUp(EView *owner, EPoint where);
	virtual void	MouseMoved(EView *owner, EPoint where, euint32 code, const EMessage *a_message);
	virtual void	KeyDown(EView *owner, const char *bytes, eint32 numBytes);
	virtual void	KeyUp(EView *owner, const char *bytes, eint32 numBytes);
};


class _IMPEXP_ETK EStringItem : public EListItem {
public:
	EStringItem(const char *text, euint32 outlineLevel = 0, bool expanded = true);
	virtual ~EStringItem();

	virtual void	SetText(const char *text);
	const char	*Text() const;

private:
	char* fText;

	virtual void	DrawItem(EView *owner, ERect itemRect, bool drawEverything);
	virtual void	Update(EView *owner, const EFont *font);
};


#endif /* __cplusplus */

#endif /* __ETK_LIST_ITEM_H__ */

