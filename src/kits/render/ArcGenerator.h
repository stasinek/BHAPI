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
 * File: ArcGenerator.h
 * Description: BArcGenerator --- Pixel generator for zero-width-arc-drawing
 * 
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_ARC_GENERATOR_H
#define BHAPI_ARC_GENERATOR_H
#include "../support/SupportDefs.h"
#include "../interface/Rect.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXPBHAPI BArcGenerator {
public:
    BArcGenerator(BPoint center, float xRadius, float yRadius, BPoint start, BPoint end);

    bool Start(__be_int32 &x,  __be_int32 &y,  __be_int32 &step,  __be_int32 &pixels, bool &both, bool isLoopX = true, float pixel_size = 1);
    bool Next(__be_int32 &next,  __be_int32 &pixels, bool &both);

private:
    BPoint fCenter;
    BPoint fRadius;
    BPoint fStart;
    BPoint fEnd;

     __be_int32 fStep;
	bool fIsLoopX;

    BPoint fRadius2;
    BPoint _fRadius;
    BPoint _fStart;
    BPoint _fEnd;
	float fDeltaNext;

     __be_int32 _fCenterX;
     __be_int32 _fCenterY;
     __be_int32 _fRadiusX;
     __be_int32 _fStartX;
     __be_int32 _fStartY;
     __be_int32 _fEndX;
     __be_int32 _fEndY;
     __be_int32 _X;
     __be_int32 _Y;
};

#endif /* __cplusplus */

#endif /* BHAPI_ARC_GENERATOR_H */

