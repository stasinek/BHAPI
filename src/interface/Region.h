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
 * File: Region.h
 * Description: ERegion --- Combination of rectangles to describe region
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_REGION_H__
#define __ETK_REGION_H__

#include "./../support/List.h>
#include "./../interface/Rect.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ERegion {
public:
	ERegion();
	ERegion(const ERegion &region);
	ERegion(const ERect &rect);
	virtual ~ERegion();

	ERegion &operator=(const ERegion &from);

	ERegion operator&(ERect r) const;
	ERegion operator|(ERect r) const;

	ERegion& operator&=(ERect r);
	ERegion& operator|=(ERect r);

	ERegion operator&(const ERegion &region) const;
	ERegion operator|(const ERegion &region) const;

	ERegion& operator&=(const ERegion &region);
	ERegion& operator|=(const ERegion &region);

	ERect Frame() const;
	ERect RectAt(eint32 index) const;
	eint32 CountRects() const;

	void Set(ERect singleBound);
	void MakeEmpty();

	bool Include(ERect r);
	bool Include(const ERegion *region);

	bool Exclude(ERect r);
	bool Exclude(const ERegion *region);

	void OffsetBy(float dx, float dy);
	void OffsetBy(EPoint pt);
	ERegion& OffsetBySelf(float dx, float dy);
	ERegion& OffsetBySelf(EPoint pt);
	ERegion OffsetByCopy(float dx, float dy);
	ERegion OffsetByCopy(EPoint pt);

	void Scale(float scaling);
	ERegion& ScaleSelf(float scaling);
	ERegion ScaleCopy(float scaling);

	bool Intersects(ERect r) const;
	bool Intersects(float l, float t, float r, float b) const;
	bool Intersects(const ERegion *region) const;

	bool Contains(EPoint pt) const;
	bool Contains(float x, float y) const;
	bool Contains(ERect r) const;
	bool Contains(float l, float t, float r, float b) const;
	bool Contains(const ERegion *region) const;

	void PrintToStream() const;

private:
	EList fRects;
	ERect fFrame;
};

#endif /* __cplusplus */

#endif /* __ETK_REGION_H__ */

