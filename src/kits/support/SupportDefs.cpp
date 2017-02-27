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
 * File: SupportDefs.cpp
 * Description: Body of support functions
 *
 * --------------------------------------------------------------------------*/


#include "SupportDefs.h"
#ifdef BHAPI_BUILD_LIBRARY
extern EXPORT_BHAPI const b_uint8 bhapi::major_version;
extern EXPORT_BHAPI const b_uint8 bhapi::minor_version;
extern EXPORT_BHAPI const b_uint8 bhapi::micro_version;
extern EXPORT_BHAPI const b_uint8 bhapi::interface_age;
extern EXPORT_BHAPI const b_uint16 bhapi::binary_age;
#endif

EXPORT_BHAPI int32 	atomic_add (int32 *value, int32 addValue) {}
EXPORT_BHAPI int64 	atomic_add64 (int64 *value, int64 addValue) {}
EXPORT_BHAPI int32 	atomic_and (int32 *value, int32 andValue) {}
EXPORT_BHAPI int64 	atomic_and64 (int64 *value, int64 andValue) {}
EXPORT_BHAPI int32 	atomic_get (int32 *value) {}
EXPORT_BHAPI int64 	atomic_get64 (int64 *value) {}
EXPORT_BHAPI int32 	atomic_get_and_set (int32 *value, int32 newValue) {}
EXPORT_BHAPI int64 	atomic_get_and_set64 (int64 *value, int64 newValue) {}
EXPORT_BHAPI int32 	atomic_or (int32 *value, int32 orValue) {}
EXPORT_BHAPI int64 	atomic_or64 (int64 *value, int64 orValue) {}
EXPORT_BHAPI void 	atomic_set (int32 *value, int32 newValue) {}
EXPORT_BHAPI void 	atomic_set64 (int64 *value, int64 newValue) {}
EXPORT_BHAPI int32 	atomic_test_and_set (int32 *value, int32 newValue, int32 testAgains) {}
EXPORT_BHAPI int64 	atomic_test_and_set64 (int64 *value, int64 newValue, int64 testAgainst) {}
EXPORT_BHAPI void * 	get_stack_frame (void) {}
EXPORT_BHAPI const char * B_EMPTY_STRING = "";

#endif /* BHAPI_SUPPORT_DEFS__H */

