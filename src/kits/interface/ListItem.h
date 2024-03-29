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
 * File: ListItem.h
 * Description: BListItem --- item for BListView/BOutlineListView
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_LIST_ITEM_H
#define BHAPI_LIST_ITEM_H

#ifdef __cplusplus /* Just for C++ */
class BList;
class BListView;
class BOutlineListView;
class BView;
class BRect;
class BFont;
class BPoint;

#include <kits/support/Archivable.h>
class BHAPI_IMPEXP BListItem : public BArchivable {
public:
	BListItem(uint32_t outlineLevel = 0, bool expanded = true,  uint32_t flags = 0);
	virtual ~BListItem();

	// Archiving
	BListItem(BMessage *from);
	virtual status_t Archive(BMessage *into, bool deep = true) const;

	float		Height() const;
	float		Width() const;
	bool		IsSelected() const;
	void		Select();
	void		Deselect();

	virtual void	SetEnabled(bool on);
	bool		IsEnabled() const;

	void		SetHeight(float height);
	void		SetWidth(float width);

	virtual void	SetFlags(uint32_t flags);
	uint32_t		Flags() const;

	void		Invalidate();

	// for item of BOutlineListView
	bool 		IsExpanded() const;
	void 		SetExpanded(bool expanded);
	uint32_t 	OutlineLevel() const;
	bool		IsVisible() const;
	BListItem	*SuperItem() const;
	bool		HasSubitems() const;

protected:
	// for item of BOutlineListView
	void		DrawLeader(BView *owner, BRect *itemRect);
	void		GetLeaderSize(float *width, float *height) const;

private:
	friend class BListView;
	friend class BOutlineListView;

	BListView *fOwner;
	BOutlineListView *fFullOwner;

	uint32_t 	fLevel;
	bool 		fExpanded;
	uint32_t		fFlags;

	float		fWidth;
	float		fHeight;
	bool		fSelected;
	bool		fEnabled;

	virtual void	DrawItem(BView *owner, BRect itemRect, bool drawEverything) = 0;
	virtual void	Update(BView *owner, const BFont *font) = 0;

	virtual void	MouseDown(BView *owner, BPoint where);
	virtual void	MouseUp(BView *owner, BPoint where);
	virtual void	MouseMoved(BView *owner, BPoint where,  uint32_t code, const BMessage *a_message);
	virtual void	KeyDown(BView *owner, const char *bytes,  int32_t numBytes);
	virtual void	KeyUp(BView *owner, const char *bytes,  int32_t numBytes);
};


class BHAPI_IMPEXP StringItem : public BListItem {
public:
	StringItem(const char *text,  uint32_t outlineLevel = 0, bool expanded = true);
	virtual ~StringItem();

	virtual void	SetText(const char *text);
	const char	*Text() const;

private:
	char* fText;

	virtual void	DrawItem(BView *owner, BRect itemRect, bool drawEverything);
	virtual void	Update(BView *owner, const BFont *font);
};


#endif /* __cplusplus */

#endif /* BHAPI_LIST_ITEM_H */

