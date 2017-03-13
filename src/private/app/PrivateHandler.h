/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_PRIVATE_HANDLER_H
#define BHAPI_PRIVATE_HANDLER_H

#ifdef __cplusplus /* Just for C++ */
class BHandler;
class BLocker;
class BLooper;
#include "../../kits/support/SupportDefs.h"
namespace bhapi {
BHAPI_LOCAL BLocker* get_handler_operator_locker();
BHAPI_LOCAL  __be_uint64 get_handler_token(const BHandler *handler);
BHAPI_LOCAL  __be_uint64 get_ref_handler_token(const BHandler *handler);
BHAPI_LOCAL BHandler* get_handler(__be_uint64 token);
BHAPI_LOCAL bigtime_t get_handler_create_time_stamp(__be_uint64 token);
BHAPI_LOCAL BLooper* get_handler_looper(__be_uint64 token);
BHAPI_LOCAL  __be_uint64 get_ref_looper_token(__be_uint64 token);
BHAPI_LOCAL status_t lock_looper_of_handler(__be_uint64 token, bigtime_t timeout);
BHAPI_LOCAL bool is_current_at_looper_thread(__be_uint64 token);
BHAPI_LOCAL bool ref_handler(__be_uint64 token);
BHAPI_LOCAL void unref_handler(__be_uint64 token);
} /* namespace bhapi */

#endif /* __cplusplus */
#endif /* BHAPI_PRIVATE_HANDLER_H */

