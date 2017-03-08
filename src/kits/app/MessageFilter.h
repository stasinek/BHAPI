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
 * File: MessageFilter.h
 * Description: Filter message before BLooper::DispatchMessage
 * Warning: ignore _QUIT_
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MESSAGE_FILTER_H
#define BHAPI_MESSAGE_FILTER_H

#include "../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
typedef enum filter_result {
    B_SKIP_MESSAGE,
    B_DISPATCH_MESSAGE
} filter_result;

typedef enum message_delivery {
    B_DROPPED_DELIVERY	=	0x1,
    B_PROGRAMMED_DELIVERY	=	0x2,
    B_ANY_DELIVERY		=	0xff
} message_delivery;

typedef enum message_source {
    BLOCALBHAPI_SOURCE		=	0x1,
    B_REMOTE_SOURCE		=	0x2,
    B_ANY_SOURCE		=	0xff
} message_source;
#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif

#ifdef __cplusplus /* Just for C++ */
class BLooper;
class BMessage;
class BMessageFilter;
class BHandler;
namespace bhapi {
typedef filter_result (*filter_hook)(BMessage *message, BHandler **target, BMessageFilter *filter);
}
class IMPEXPBHAPI BMessageFilter {
public:
    BMessageFilter(bhapi::message_delivery delivery, bhapi::message_source source,
                __be_uint32 command, bhapi::filter_hook filter = NULL);
    BMessageFilter(bhapi::message_delivery delivery, bhapi::message_source source,
               bhapi::filter_hook filter = NULL);
    BMessageFilter(__be_uint32 command, bhapi::filter_hook filter = NULL);
    BMessageFilter(const BMessageFilter &filter);
    BMessageFilter(const BMessageFilter *filter);
    virtual ~BMessageFilter();

    BMessageFilter &operator=(const BMessageFilter &from);

    virtual bhapi::filter_result		Filter(BMessage *message, BHandler **target);

    bhapi::message_delivery		MessageDelivery() const;
    bhapi::message_source		MessageSource() const;
     __be_uint32				Command() const;
    bool				FiltersAnyCommand() const;
    BLooper				*Looper() const;

private:
    friend class BLooper;
    friend class BHandler;

     __be_uint32 fCommand;
    bool fFiltersAny;
    bhapi::message_delivery fDelivery;
    bhapi::message_source fSource;
    bhapi::filter_hook fFilterHook;

    BHandler *fHandler;

    bhapi::filter_result doFilter(BMessage *message, BHandler **target);
};

#endif /* __cplusplus */

#endif /* BHAPI_MESSAGE_FILTER_H */

