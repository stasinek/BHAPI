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
 * File: SimpleLocker.h
 * Description: BSimpleLocker --- locker DO NOT support nested-locking
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"

#include "SimpleLocker.h"


BSimpleLocker::BSimpleLocker(bool assert)
{
	fLocker = bhapi_create_simple_locker();
	if(fLocker == NULL && assert) BHAPI_ERROR("[SUPPORT]: %s --- Create simple locker failed!", __PRETTY_FUNCTION__);
}


BSimpleLocker::~BSimpleLocker()
{
	if(fLocker != NULL) bhapi_delete_simple_locker(fLocker);
}


bool
BSimpleLocker::IsValid() const
{
	return(fLocker != NULL);
}


bool
BSimpleLocker::Lock()
{
	return(fLocker != NULL ? bhapi_lock_simple_locker(fLocker) : false);
}


void
BSimpleLocker::Unlock()
{
	if(fLocker != NULL) bhapi_unlock_simple_locker(fLocker);
}

