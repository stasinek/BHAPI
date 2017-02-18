/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: OS.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_OS__H
#define BHAPI_OS__H
#include <kits/support/SupportDefs.h>


typedef b_status_t				(*b_thread_func)(void*);

#define B_SYSTEM_TIMEBASE			0
#define B_REAL_TIME_TIMEBASE			1

#define B_OS_NAME_LENGTH			32
#define B_INFINITE_TIMEOUT		    B_MAXINT64

#define B_READ_AREA                     1
#define B_WRITE_AREA                    2

#define B_LOW_PRIORITY                  5
#define B_NORMAL_PRIORITY               10
#define B_DISPLAY_PRIORITY              15
#define B_URGENT_DISPLAY_PRIORITY		20
#define B_REAL_TIME_DISPLAY_PRIORITY	100
#define B_URGENT_PRIORITY               110
#define B_REAL_TIME_PRIORITY            120

#define B_PAGE_SIZE			4096

enum {
    B_CAN_INTERRUPT		= 1,	/* semaphore can be interrupted by a signal */
    B_DO_NOT_RESCHEDULE	= 2,	/* release() without rescheduling */
    B_TIMEOUT						= 0x8,	/* relative timeout */
    B_RELATIVE_TIMEOUT				= 0x8,	/* fails after a relative timeout
                                                with B_TIMED_OUT */
    B_ABSOLUTE_TIMEOUT				= 0x10,	/* fails after an absolute timeout
                                                with B_TIMED_OUT */

    /* experimental Haiku only API */
    B_TIMEOUT_REAL_TIME_BASE		= 0x40,
    B_ABSOLUTE_REAL_TIME_TIMEOUT	= B_ABSOLUTE_TIMEOUT
                                        | B_TIMEOUT_REAL_TIME_BASE
};

#ifndef BHAPI_KERNEL__H
#include "Kernel.h"
#endif /* BHAPI_KERNEL__H */

/* time functions */
#define b_snooze(microseconds)			bhapi_snooze(microseconds)
#define b_snooze_until(time, timebase)		bhapi_snooze_until(time, timebase)
#define b_real_time_clock()			bhapi_real_time_clock()
#define b_real_time_clock_usecs()		bhapi_real_time_clock_usecs()
#define b_system_time()				bhapi_system_time()

#endif /* BHAPI_OS__H */

