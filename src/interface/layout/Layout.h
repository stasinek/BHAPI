/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Layout.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LAYOUT_H__
#define __ETK_LAYOUT_H__

#include "./../../support/List.h"
#include "./../../interface/InterfaceDefs.h"
#include "./../../interface/Region.h"

#ifdef __cplusplus /* Just for C++ */

class ELayoutItem;
class ELayoutForm;


class _IMPEXP_ETK ELayoutContainer {
public:
	ELayoutContainer();
	virtual ~ELayoutContainer();

	virtual bool		AddItem(ELayoutItem *item, eint32 index = -1);
	virtual bool		RemoveItem(ELayoutItem *item);
	ELayoutItem		*RemoveItem(eint32 index);

	ELayoutItem		*ItemAt(eint32 index) const;
	eint32			IndexOf(const ELayoutItem *item) const;
	eint32			CountItems() const;

	float			UnitsPerPixel() const;
	void			SetUnitsPerPixel(float value, bool deep = true);

	virtual void		Invalidate(ERect rect);

	void			SetPrivateData(void *data, void (*destroy_func)(void*) = NULL);
	void			*PrivateData() const;

private:
	friend class ELayoutItem;

	float fUnitsPerPixel;
	EList fItems;
	void *fPrivate[2];
};


class _IMPEXP_ETK ELayoutItem : public ELayoutContainer {
public:
	ELayoutItem(ERect frame, euint32 resizingMode);
	virtual ~ELayoutItem();

	ELayoutContainer	*Container() const;
	ELayoutContainer	*Ancestor() const;

	bool			RemoveSelf();

	ELayoutItem		*PreviousSibling() const;
	ELayoutItem		*NextSibling() const;

	virtual void		SetResizingMode(euint32 mode);
	euint32			ResizingMode() const;

	virtual void		Show();
	virtual void		Hide();
	bool			IsHidden(bool check_containers = true) const;

	virtual void		SendBehind(ELayoutItem *item);
	virtual void		MoveTo(EPoint where);
	virtual void		ScrollTo(EPoint where);
	virtual void		ResizeTo(float width, float height);
	void			MoveAndResizeTo(EPoint where, float width, float height);

	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		ResizeToPreferred();

	ERect			Bounds() const; // in it's coordinate system
	ERect			Frame() const; // in container's coordinate system
	const ERegion		*VisibleRegion() const; // in it's coordinate system

	EPoint			LeftTop() const;
	float			Width() const;
	float			Height() const;

	void			ConvertToContainer(EPoint *pt) const;
	EPoint			ConvertToContainer(EPoint pt) const;
	void			ConvertFromContainer(EPoint *pt) const;
	EPoint			ConvertFromContainer(EPoint pt) const;

	virtual void		UpdateVisibleRegion();

protected:
	void			GetVisibleRegion(ERegion **region);

private:
	friend class ELayoutContainer;
	friend class ELayoutForm;

	ELayoutContainer *fContainer;
	eint32 fIndex;

	EPoint fLocalOrigin;
	ERegion fVisibleRegion;

	ERect fFrame;
	euint32 fResizingMode;
	bool fHidden;
	bool fUpdating;
};


class _IMPEXP_ETK ELayoutForm : public ELayoutItem {
public:
	ELayoutForm(ERect frame, euint32 resizingMode, eint32 rows, eint32 columns);
	virtual ~ELayoutForm();

private:
	void *fData;
};

#endif /* __cplusplus */

#endif /* __ETK_LAYOUT_H__ */

