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
 * File: Errors.h
 *
 * --------------------------------------------------------------------------*/
#ifndef __ETK_ERRORS_H__
#define __ETK_ERRORS_H__

#include <limits.h>

/* Error baselines */
#define E_GENERAL_ERROR_BASE	INT_MIN
#define E_OS_ERROR_BASE		E_GENERAL_ERROR_BASE + 0x1000
#define E_APP_ERROR_BASE	E_GENERAL_ERROR_BASE + 0x2000
#define E_STORAGE_ERROR_BASE	E_GENERAL_ERROR_BASE + 0x3000

/* General Errors */
enum {
	E_NO_MEMORY = E_GENERAL_ERROR_BASE,
	E_IO_ERROR,
	E_PERMISSION_DENIED,
	E_BAD_INDEX,
	E_BAD_TYPE,
	E_BAD_VALUE,
	E_MISMATCHED_VALUES,
	E_NAME_NOT_FOUND,
	E_NAME_IN_USE,
	E_TIMED_OUT,
	E_INTERRUPTED,
	E_WOULD_BLOCK,
	E_CANCELED,
	E_NO_INIT,
	E_BUSY,
	E_NOT_ALLOWED,

	E_ERROR = -1,
	E_OK = 1,
	E_NO_ERROR = 1
};

/* Kernel Kit Errors */
enum {
	E_BAD_SEM_ID = E_OS_ERROR_BASE,
	E_NO_MORE_SEMS,

	E_BAD_THREAD_ID = E_OS_ERROR_BASE + 0x100,
	E_NO_MORE_THREADS,
	E_BAD_THREAD_STATE,
	E_BAD_TEAM_ID,
	E_NO_MORE_TEAMS,

	E_BAD_PORT_ID = E_OS_ERROR_BASE + 0x200,
	E_NO_MORE_PORTS,

	E_BAD_IMAGE_ID = E_OS_ERROR_BASE + 0x300,
	E_BAD_ADDRESS,
	E_NOT_AN_EXECUTABLE,
	E_MISSING_LIBRARY,
	E_MISSING_SYMBOL,

	E_DEBUGGER_ALREADY_INSTALLED = E_OS_ERROR_BASE + 0x400
};

/* Application Kit Errors */
enum {
	E_BAD_REPLY = E_APP_ERROR_BASE,
	E_DUPLICATE_REPLY,
	E_MESSAGE_TO_SELF,
	E_BAD_HANDLER,
	E_ALREADY_RUNNING,
};

/* Storage Kit Errors */
enum {
	E_FILE_ERROR = E_STORAGE_ERROR_BASE,
	E_ENTRY_NOT_FOUND,
	E_LINK_LIMIT,
	E_NAME_TOO_LONG,
};

#endif /* __ETK_ERRORS_H__ */

