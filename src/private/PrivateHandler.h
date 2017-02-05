/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: PrivateHandler.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_PRIVATE_HANDLER_H__
#define __ETK_PRIVATE_HANDLER_H__

#include "./../support/SupportDefs.h"
#include "./../support/Locker.h>
#include "./../app/Looper.h>

#ifdef __cplusplus /* Just for C++ */

_LOCAL ELocker* etk_get_handler_operator_locker();
_LOCAL euint64 etk_get_handler_token(const EHandler *handler);
_LOCAL euint64 etk_get_ref_handler_token(const EHandler *handler);
_LOCAL EHandler* etk_get_handler(euint64 token);
_LOCAL e_bigtime_t etk_get_handler_create_time_stamp(euint64 token);
_LOCAL ELooper* etk_get_handler_looper(euint64 token);
_LOCAL euint64 etk_get_ref_looper_token(euint64 token);
_LOCAL e_status_t etk_lock_looper_of_handler(euint64 token, e_bigtime_t timeout);
_LOCAL bool etk_is_current_at_looper_thread(euint64 token);
_LOCAL bool etk_ref_handler(euint64 token);
_LOCAL void etk_unref_handler(euint64 token);

#endif /* __cplusplus */

#endif /* __ETK_PRIVATE_HANDLER_H__ */

