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

#include <be/kernel/OS.h>

#include "./../kernel/Kernel.h>

// return the number of microseconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK e_bigtime_t etk_real_time_clock_usecs(void)
{
	return (e_bigtime_t)real_time_clock_usecs();
}


// return the number of seconds elapsed since 00:00 01 January 1970 UTC (Unix epoch)
_IMPEXP_ETK euint32 etk_real_time_clock(void)
{
	return (euint32)real_time_clock();
}


_IMPEXP_ETK e_bigtime_t etk_system_boot_time(void)
{
	system_info sysInfo;
	get_system_info(&sysInfo);
	return (e_bigtime_t)sysInfo.boot_time;
}


_IMPEXP_ETK e_bigtime_t etk_system_time(void)
{
	return (e_bigtime_t)system_time();
}

