/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Layout.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_LAYOUT_H
#define BHAPI_LAYOUT_H

#include <kits/support/List.h>
#include <kits/interface/InterfaceDefs.h>
#include <kits/interface/Region.h>

#ifdef __cplusplus /* Just for C++ */

class BLayoutItem;
class BLayoutForm;


class BHAPI_IMPEXP BLayoutContainer {
public:
	BLayoutContainer();
	virtual ~BLayoutContainer();

	virtual bool		AddItem(BLayoutItem *item,  __be_int32_t index = -1);
	virtual bool		RemoveItem(BLayoutItem *item);
	BLayoutItem		*RemoveItem(__be_int32_t index);

	BLayoutItem		*ItemAt(__be_int32_t index) const;
	__be_int32_t			IndexOf(const BLayoutItem *item) const;
	__be_int32_t			CountItems() const;

	float			UnitsPerPixel() const;
	void			SetUnitsPerPixel(float value, bool deep = true);

	virtual void		Invalidate(BRect rect);

	void			SetPrivateData(void *data, void (*destroy_func)(void*) = NULL);
	void			*PrivateData() const;

private:
	friend class BLayoutItem;

	float fUnitsPerPixel;
	BList fItems;
	void *fPrivate[2];
};


class BHAPI_IMPEXP BLayoutItem : public BLayoutContainer {
public:
	BLayoutItem(BRect frame,  __be_uint32_t resizingMode);
	virtual ~BLayoutItem();

	BLayoutContainer	*Container() const;
	BLayoutContainer	*Ancestor() const;

	bool			RemoveSelf();

	BLayoutItem		*PreviousSibling() const;
	BLayoutItem		*NextSibling() const;

	virtual void		SetResizingMode(__be_uint32_t mode);
	__be_uint32_t			ResizingMode() const;

	virtual void		Show();
	virtual void		Hide();
	bool			IsHidden(bool check_containers = true) const;

	virtual void		SendBehind(BLayoutItem *item);
	virtual void		MoveTo(BPoint where);
	virtual void		ScrollTo(BPoint where);
	virtual void		ResizeTo(float width, float height);
	void			MoveAndResizeTo(BPoint where, float width, float height);

	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		ResizeToPreferred();

	BRect			Bounds() const; // in it's coordinate system
	BRect			Frame() const; // in container's coordinate system
	const BRegion		*VisibleRegion() const; // in it's coordinate system

	BPoint			LeftTop() const;
	float			Width() const;
	float			Height() const;

	void			ConvertToContainer(BPoint *pt) const;
	BPoint			ConvertToContainer(BPoint pt) const;
	void			ConvertFromContainer(BPoint *pt) const;
	BPoint			ConvertFromContainer(BPoint pt) const;

	virtual void		UpdateVisibleRegion();

protected:
	void			GetVisibleRegion(BRegion **region);

private:
	friend class BLayoutContainer;
	friend class BLayoutForm;

	BLayoutContainer *fContainer;
	__be_int32_t fIndex;

	BPoint fLocalOrigin;
	BRegion fVisibleRegion;

	BRect fFrame;
	__be_uint32_t fResizingMode;
	bool fHidden;
	bool fUpdating;
};


class BHAPI_IMPEXP BLayoutForm : public BLayoutItem {
public:
	BLayoutForm(BRect frame,  __be_uint32_t resizingMode,  __be_int32_t rows,  __be_int32_t columns);
	virtual ~BLayoutForm();

private:
	void *fData;
};

#endif /* __cplusplus */

#endif /* BHAPI_LAYOUT_H */

