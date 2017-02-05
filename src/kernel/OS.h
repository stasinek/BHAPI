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
 * File: OS.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_OS_H__
#define __ETK_OS_H__

#include "./../support/SupportDefs.h"

typedef e_status_t				(*e_thread_func)(void*);

#define E_SYSTEM_TIMEBASE			0
#define E_REAL_TIME_TIMEBASE			1

#define E_OS_NAME_LENGTH			32
#define E_INFINITE_TIMEOUT			E_MAXINT64

#define E_READ_AREA				1
#define E_WRITE_AREA				2

#define E_LOW_PRIORITY				5
#define E_NORMAL_PRIORITY			10
#define E_DISPLAY_PRIORITY			15
#define	E_URGENT_DISPLAY_PRIORITY		20
#define	E_REAL_TIME_DISPLAY_PRIORITY		100
#define	E_URGENT_PRIORITY			110
#define E_REAL_TIME_PRIORITY			120

/* flags for semaphore control */
enum {
	E_CAN_INTERRUPT		= 1,	/* semaphore can be interrupted by a signal */
	E_DO_NOT_RESCHEDULE	= 2,	/* release() without rescheduling */
	E_TIMEOUT		= 8,	/* honor the (relative) timeout parameter */
	E_RELATIVE_TIMEOUT	= 8,
	E_ABSOLUTE_TIMEOUT	= 16	/* honor the (absolute) timeout parameter */
};

#ifndef __ETK_KERNEL_H__
#include "Kernel.h"
#endif /* __ETK_KERNEL_H__ */

/* time functions */
#define e_snooze(microseconds)			etk_snooze(microseconds)
#define e_snooze_until(time, timebase)		etk_snooze_until(time, timebase)
#define e_real_time_clock()			etk_real_time_clock()
#define e_real_time_clock_usecs()		etk_real_time_clock_usecs()
#define e_system_time()				etk_system_time()

#endif /* __ETK_OS_H__ */

