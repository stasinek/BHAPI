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
#ifdef BHAPI_OS_LINUX

#include <sys/time.h>
#include <time.h>

#include "../support/SupportDefs.h"
#include "../kernel/Kernel.h"
#include "../support/SimpleLocker.h"

#define SECS_TO_US	    B_INT64_CONSTANT(1000000)


// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
IMPEXPBHAPI b_bigtime_t b_real_time_clock_usecs(void)
{
	b_int64 current_time = B_INT64_CONSTANT(-1);
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	struct timespec ts;

	if(clock_gettime(CLOCK_REALTIME, &ts) == 0)
		current_time = (b_int64)ts.tv_sec * SECS_TO_US + (b_int64)(ts.tv_nsec + 500) / B_INT64_CONSTANT(1000);
#else
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;

	if(gettimeofday(&tv, NULL) == 0)
		current_time = (b_int64)tv.tv_sec * SECS_TO_US + (b_int64)tv.tv_usec;
#else
	#error "no time function implement b_real_time_clock_usec!"
#endif
#endif
	return current_time;
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
IMPEXPBHAPI b_uint32 b_real_time_clock(void)
{
	b_uint32 current_time = 0;
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == 0) current_time = (b_uint32)ts.tv_sec;
#else
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == 0) current_time = (b_uint32)tv.tv_sec;
#else
	#error "no time function implement b_real_time_clock!"
#endif
#endif
	return current_time;
}


static b_int64 b_unix_boot_time = B_INT64_CONSTANT(-1);
static BSimpleLocker b_unix_boot_time_locker(true);


IMPEXPBHAPI b_bigtime_t b_system_boot_time(void)
{
	b_bigtime_t retValue = B_INT64_CONSTANT(-1);

	b_unix_boot_time_locker.Lock();

	if(b_unix_boot_time >= B_INT64_CONSTANT(0))
	{
		retValue = b_unix_boot_time;
	}
	else
	{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
		struct timespec ts;

		if(clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		{
			b_bigtime_t up_time = (b_int64)ts.tv_sec * SECS_TO_US + (b_int64)(ts.tv_nsec + 500) / B_INT64_CONSTANT(1000);
			retValue = b_unix_boot_time = b_real_time_clock_usecs() - up_time;
		}
#else
		#warning "fixme: no time function implement b_system_boot_time!"
		retValue = b_unix_boot_time = B_INT64_CONSTANT(0);
#endif
	}

	b_unix_boot_time_locker.Unlock();

	return retValue;
}


IMPEXPBHAPI b_bigtime_t b_system_time(void)
{
	// FIXME
	return(b_real_time_clock_usecs() - b_system_boot_time());
}
#endif // BHAPI_OS_LINUX

