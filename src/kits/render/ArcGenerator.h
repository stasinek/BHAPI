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
#include <Haiku.h>
#include <kits/interface/Rect.h>

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BArcGenerator {
public:
    BArcGenerator(BPoint center, float xRadius, float yRadius, BPoint start, BPoint end);

    bool Start(int32_t &x,  int32_t &y,  int32_t &step,  int32_t &pixels, bool &both, bool isLoopX = true, float pixel_size = 1);
    bool Next(int32_t &next,  int32_t &pixels, bool &both);

private:
    BPoint fCenter;
    BPoint fRadius;
    BPoint fStart;
    BPoint fEnd;

     int32_t fStep;
	bool fIsLoopX;

    BPoint fRadius2;
    BPoint _fRadius;
    BPoint _fStart;
    BPoint _fEnd;
	float fDeltaNext;

     int32_t _fCenterX;
     int32_t _fCenterY;
     int32_t _fRadiusX;
     int32_t _fStartX;
     int32_t _fStartY;
     int32_t _fEndX;
     int32_t _fEndY;
     int32_t _X;
     int32_t _Y;
};

#endif /* __cplusplus */

#endif /* BHAPI_ARC_GENERATOR_H */

