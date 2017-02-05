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
 * File: LineGenerator.h
 * Description: ELineGenerator --- Pixel generator for zero-width-line-drawing
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LINE_GENERATOR_H__
#define __ETK_LINE_GENERATOR_H__

#include "./../interface/Rect.h"
#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ELineGenerator {
public:
	ELineGenerator(EPoint start, EPoint end);

	bool Start(eint32 &x, eint32 &y, eint32 &step, eint32 &pixels, bool isLoopX = true, float pixel_size = 1);
	bool Next(eint32 &next, eint32 &pixels);

private:
	EPoint fStart;
	EPoint fEnd;

	euint8 fDirection;
	eint32 fStep;
	eint32 fStepOther;
	bool fIsLoopX;

	float fEltaNext;
	float fEltaNextAbs;
	float fDeltaElta;

	void InitDirection();
};

#endif /* __cplusplus */

#endif /* __ETK_LINE_GENERATOR_H__ */

