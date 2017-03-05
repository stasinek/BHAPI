/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku OS source-code, All Rights Reserved
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
 * File: StopWatch.h
 *
 * --------------------------------------------------------------------------*/

/*
 * Copyright 2001-2007, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_STOP_WATCH_H
#define BHAPI_STOP_WATCH_H

#include "SupportDefs.h"

class BStopWatch {
	public:
		BStopWatch(const char* name, bool silent = false);
		virtual ~BStopWatch();

		void		Suspend();
		void		Resume();
		bigtime_t	Lap();
		bigtime_t	ElapsedTime() const;
		void		Reset();
		const char*	Name() const;

	private:
		virtual	void _ReservedStopWatch1();
		virtual	void _ReservedStopWatch2();

		bigtime_t	fStart;
		bigtime_t	fSuspendTime;
		bigtime_t	fLaps[10];
		int32		fLap;
		const char*	fName;
		uint32		_reserved[2];
		bool		fSilent;
};

#endif // BHAPI_STOP_WATCH_H
