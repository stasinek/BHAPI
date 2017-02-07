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
 * File: PrivateHandler.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_PRIVATE_HANDLER_H__
#define __BHAPI_PRIVATE_HANDLER_H__

#include "./../support/SupportDefs.h"
#include "./../support/Locker.h"
#include "./../app/Looper.h"

#ifdef __cplusplus /* Just for C++ */

_LOCAL BLocker* bhapi_get_handler_operator_locker();
_LOCAL b_uint64 bhapi_get_handler_token(const BHandler *handler);
_LOCAL b_uint64 bhapi_get_ref_handler_token(const BHandler *handler);
_LOCAL BHandler* bhapi_get_handler(b_uint64 token);
_LOCAL b_bigtime_t bhapi_get_handler_create_time_stamp(b_uint64 token);
_LOCAL BLooper* bhapi_get_handler_looper(b_uint64 token);
_LOCAL b_uint64 bhapi_get_ref_looper_token(b_uint64 token);
_LOCAL b_status_t bhapi_lock_looper_of_handler(b_uint64 token, b_bigtime_t timeout);
_LOCAL bool bhapi_is_current_at_looper_thread(b_uint64 token);
_LOCAL bool bhapi_ref_handler(b_uint64 token);
_LOCAL void bhapi_unref_handler(b_uint64 token);

#endif /* __cplusplus */

#endif /* __BHAPI_PRIVATE_HANDLER_H__ */

