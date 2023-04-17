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
 * File: BHAPI_wrapper_timefuncs.cpp
 *
 * --------------------------------------------------------------------------*/

#include <os/kernel.h>

#include <winsock2.h>
#include <windows.h>

#define SECS_BETWEEN_EPOCHS    B_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS		10000000
#define SECS_TO_US		1000000

// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
BHAPI_EXPORT bigtime_t b_real_time_clock_usecs(void)
{
	FILETIME CurrentTime;

	// FILETIME represents a 64-bit number of 100-nanoseconds intervals that
	// have passed since 00:00 01 January 1601 UTC (Win32 epoch)
	GetSystemTimeAsFileTime(&CurrentTime);

	// get the full win32 value, in 100-nanoseconds
	int64 t = ((int64)CurrentTime.dwHighDateTime << 32) | ((int64)CurrentTime.dwLowDateTime);
	t -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);
	t /= 10;

	return t;
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
BHAPI_EXPORT  uint32_t b_real_time_clock(void)
{
	return((uint32_t)(b_real_time_clock_usecs() / SECS_TO_US));
}


static bigtime_t b_windows_boot_time = B_INT64_CONSTANT(-1);
static LONG b_windows_boot_time_locker = 0;


BHAPI_EXPORT bigtime_t b_system_boot_time(void)
{
	bigtime_t retValue = B_INT64_CONSTANT(-1);

	while(InterlockedExchange(&b_windows_boot_time_locker, 1) == 1) Sleep(0);

	if(b_windows_boot_time >= 0)
	{
		retValue = b_windows_boot_time;
	}
	else
	{
		// TODO: GetTickCount 49.7 days period
		bigtime_t CurrentTime = b_real_time_clock_usecs(); // in microseconds
		bigtime_t ElapsedTime = (bigtime_t)GetTickCount(); // in milliseconds
		retValue = b_windows_boot_time = CurrentTime - ElapsedTime * 1000;
	}

	InterlockedExchange(&b_windows_boot_time_locker, 0);

	return retValue;
}


BHAPI_EXPORT bigtime_t b_system_time(void)
{
	LARGE_INTEGER counter, freq;
	if(QueryPerformanceCounter(&counter) != 0 && QueryPerformanceFrequency(&freq) != 0)
	{
		return((bigtime_t)(((double)counter.QuadPart / (double)freq.QuadPart) * 1000000.f));
	}
	else
	{
		// FIXME
		return(b_real_time_clock_usecs() - b_system_boot_time());
	}
}

