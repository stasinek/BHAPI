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

#include <sys/time.h>
#include <time.h>

#include "./../config.h>
#include "./../kernel/Kernel.h>
#include "./../support/SimpleLocker.h>

#define SECS_TO_US		E_INT64_CONSTANT(1000000)


// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK e_bigtime_t etk_real_time_clock_usecs(void)
{
	eint64 current_time = E_INT64_CONSTANT(-1);
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	struct timespec ts;

	if(clock_gettime(CLOCK_REALTIME, &ts) == 0)
		current_time = (eint64)ts.tv_sec * SECS_TO_US + (eint64)(ts.tv_nsec + 500) / E_INT64_CONSTANT(1000);
#else
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;

	if(gettimeofday(&tv, NULL) == 0)
		current_time = (eint64)tv.tv_sec * SECS_TO_US + (eint64)tv.tv_usec;
#else
	#error "no time function implement etk_real_time_clock_usec!"
#endif
#endif
	return current_time;
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK euint32 etk_real_time_clock(void)
{
	euint32 current_time = 0;
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == 0) current_time = (euint32)ts.tv_sec;
#else
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	if(gettimeofday(&tv, NULL) == 0) current_time = (euint32)tv.tv_sec;
#else
	#error "no time function implement etk_real_time_clock!"
#endif
#endif
	return current_time;
}


static eint64 etk_unix_boot_time = E_INT64_CONSTANT(-1);
static ESimpleLocker etk_unix_boot_time_locker(true);


_IMPEXP_ETK e_bigtime_t etk_system_boot_time(void)
{
	e_bigtime_t retValue = E_INT64_CONSTANT(-1);

	etk_unix_boot_time_locker.Lock();

	if(etk_unix_boot_time >= E_INT64_CONSTANT(0))
	{
		retValue = etk_unix_boot_time;
	}
	else
	{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
		struct timespec ts;

		if(clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		{
			e_bigtime_t up_time = (eint64)ts.tv_sec * SECS_TO_US + (eint64)(ts.tv_nsec + 500) / E_INT64_CONSTANT(1000);
			retValue = etk_unix_boot_time = etk_real_time_clock_usecs() - up_time;
		}
#else
		#warning "fixme: no time function implement etk_system_boot_time!"
		retValue = etk_unix_boot_time = E_INT64_CONSTANT(0);
#endif
	}

	etk_unix_boot_time_locker.Unlock();

	return retValue;
}


_IMPEXP_ETK e_bigtime_t etk_system_time(void)
{
	// FIXME
	return(etk_real_time_clock_usecs() - etk_system_boot_time());
}

