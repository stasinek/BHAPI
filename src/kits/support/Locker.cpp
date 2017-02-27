/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: Locker.cpp
 * Description: BLocker --- locker support nested-locking enough times
 *
 * --------------------------------------------------------------------------*/

#include "Locker.h"

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/Errors.h"

BLocker::BLocker()
{
	fLocker = b_create_locker();
}


BLocker::~BLocker()
{
	if(fLocker != NULL) b_delete_locker(fLocker);
}


bool
BLocker::Lock()
{
	return(b_lock_locker(fLocker) == B_OK);
}


void
BLocker::Unlock()
{
	if(b_count_locker_locks(fLocker) <= 0)
	{
		BHAPI_WARNING("[SUPPORT]: %s -- Locker didn't locked by current thread.", __PRETTY_FUNCTION__);
		return;
	}

	b_unlock_locker(fLocker);
}


b_status_t
BLocker::LockWithTimeout(b_bigtime_t microseconds)
{
	return b_lock_locker_etc(fLocker, B_TIMEOUT, microseconds);
}


b_int64
BLocker::CountLocks() const
{
	return b_count_locker_locks(fLocker);
}


bool
BLocker::IsLockedByCurrentThread() const
{
	return(b_count_locker_locks(fLocker) > 0);
}

