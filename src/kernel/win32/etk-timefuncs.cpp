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
 * File: etk-timefuncs.cpp
 *
 * --------------------------------------------------------------------------*/

#include <windows.h>

#include "./../kernel/Kernel.h>

#define SECS_BETWEEN_EPOCHS	E_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS		10000000
#define SECS_TO_US		1000000


// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK e_bigtime_t etk_real_time_clock_usecs(void)
{
	FILETIME CurrentTime;

	// FILETIME represents a 64-bit number of 100-nanoseconds intervals that
	// have passed since 00:00 01 January 1601 UTC (Win32 epoch)
	GetSystemTimeAsFileTime(&CurrentTime);

	// get the full win32 value, in 100-nanoseconds
	eint64 t = ((eint64)CurrentTime.dwHighDateTime << 32) | ((eint64)CurrentTime.dwLowDateTime);
	t -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);
	t /= 10;

	return t;
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK euint32 etk_real_time_clock(void)
{
	return((euint32)(etk_real_time_clock_usecs() / SECS_TO_US));
}


static e_bigtime_t etk_windows_boot_time = E_INT64_CONSTANT(-1);
static LONG etk_windows_boot_time_locker = 0;


_IMPEXP_ETK e_bigtime_t etk_system_boot_time(void)
{
	e_bigtime_t retValue = E_INT64_CONSTANT(-1);

	while(InterlockedExchange(&etk_windows_boot_time_locker, 1) == 1) Sleep(0);

	if(etk_windows_boot_time >= 0)
	{
		retValue = etk_windows_boot_time;
	}
	else
	{
		// TODO: GetTickCount 49.7 days period
		e_bigtime_t CurrentTime = etk_real_time_clock_usecs(); // in microseconds
		e_bigtime_t ElapsedTime = (e_bigtime_t)GetTickCount(); // in milliseconds
		retValue = etk_windows_boot_time = CurrentTime - ElapsedTime * 1000;
	}

	InterlockedExchange(&etk_windows_boot_time_locker, 0);

	return retValue;
}


_IMPEXP_ETK e_bigtime_t etk_system_time(void)
{
	LARGE_INTEGER counter, freq;
	if(QueryPerformanceCounter(&counter) != 0 && QueryPerformanceFrequency(&freq) != 0)
	{
		return((e_bigtime_t)(((double)counter.QuadPart / (double)freq.QuadPart) * 1000000.f));
	}
	else
	{
		// FIXME
		return(etk_real_time_clock_usecs() - etk_system_boot_time());
	}
}

