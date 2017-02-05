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
 * File: Polygon.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_POLYGON_H__
#define __ETK_POLYGON_H__

#include "./../interface/Rect.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EPolygon {
public:
	EPolygon(const EPoint *pts, eint32 nPts);
	EPolygon();
	EPolygon(const EPolygon *poly);
	virtual ~EPolygon();

	EPolygon	&operator=(const EPolygon &poly);
	ERect		Frame() const;

	bool		AddPoints(const EPoint *pts, eint32 nPts, bool updateFrame = true);
	void		RemovePoints(eint32 fromIndex, eint32 toIndex, bool updateFrame = true);

	bool		AddPoint(const EPoint &aPt, bool updateFrame = true);
	void		RemovePoint(eint32 index, bool updateFrame = true);
	void		UpdateFrame();

	const EPoint	&operator[](eint32 index) const; // none checking
	eint32		CountPoints() const;

	bool		MapTo(ERect srcRect, ERect dstRect);

	const EPoint	*Points() const;
	void		PrintToStream() const;

private:
	ERect fFrame;
	eint32 fCount;
	EPoint *fPts;
	bool fNeededToUpdateFrame;
};


inline bool EPolygon::AddPoint(const EPoint &aPt, bool updateFrame)
{
	return AddPoints(&aPt, 1, updateFrame);
}


inline void EPolygon::RemovePoint(eint32 index, bool updateFrame)
{
	RemovePoints(index, index, updateFrame);
}


#endif /* __cplusplus */

#endif /* __ETK_POLYGON_H__ */

