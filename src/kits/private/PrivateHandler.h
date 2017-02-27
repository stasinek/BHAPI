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

#ifndef BHAPI_PRIVATE_HANDLER__H
#define BHAPI_PRIVATE_HANDLER__H
#include "../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */
class BHandler;
class BLocker;
class BLooper;
namespace bhapi {
LOCAL_BHAPI BLocker* get_handler_operator_locker();
LOCAL_BHAPI b_uint64 get_handler_token(const BHandler *handler);
LOCAL_BHAPI b_uint64 get_ref_handler_token(const BHandler *handler);
LOCAL_BHAPI BHandler* get_handler(b_uint64 token);
LOCAL_BHAPI b_bigtime_t get_handler_create_time_stamp(b_uint64 token);
LOCAL_BHAPI BLooper* get_handler_looper(b_uint64 token);
LOCAL_BHAPI b_uint64 get_ref_looper_token(b_uint64 token);
LOCAL_BHAPI b_status_t lock_looper_of_handler(b_uint64 token, b_bigtime_t timeout);
LOCAL_BHAPI bool is_current_at_looper_thread(b_uint64 token);
LOCAL_BHAPI bool ref_handler(b_uint64 token);
LOCAL_BHAPI void unref_handler(b_uint64 token);
} /* namespace bhapi */

#endif /* __cplusplus */
#endif /* BHAPI_PRIVATE_HANDLER__H */

