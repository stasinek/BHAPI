﻿/* --------------------------------------------------------------------------
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
 * File: Polygon.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_POLYGON_H
#define BHAPI_POLYGON_H

#ifdef __cplusplus /* Just for C++ */
#include "../interface/Rect.h"
class IMPEXPBHAPI BPolygon {
public:
    BPolygon(const BPoint *pts,  __be_int32 nPts);
    BPolygon();
    BPolygon(const BPolygon *poly);
    virtual ~BPolygon();

    BPolygon	&operator=(const BPolygon &poly);
    BRect		Frame() const;

    bool		AddPoints(const BPoint *pts,  __be_int32 nPts, bool updateFrame = true);
    void		RemovePoints(__be_int32 fromIndex,  __be_int32 toIndex, bool updateFrame = true);

    bool		AddPoint(const BPoint &aPt, bool updateFrame = true);
    void		RemovePoint(__be_int32 index, bool updateFrame = true);
    void		UpdateFrame();

    const BPoint	&operator[](__be_int32 index) const; // none checking
     __be_int32		CountPoints() const;

    bool		MapTo(BRect srcRect, BRect dstRect);

    const BPoint	*Points() const;
    void		PrintToStream() const;

private:
    BRect fFrame;
     __be_int32 fCount;
    BPoint *fPts;
    bool fNeededToUpdateFrame;
};

#ifdef BHAPI_BUILD_LIBRARY
inline bool BPolygon::AddPoint(const BPoint &aPt, bool updateFrame)
{
    return AddPoints(&aPt, 1, updateFrame);
}


inline void BPolygon::RemovePoint(__be_int32 index, bool updateFrame)
{
    RemovePoints(index, index, updateFrame);
}
#endif // BHAPI_BUILD_LIBRARY

#endif /* __cplusplus */

#endif /* BHAPI_POLYGON_H */

