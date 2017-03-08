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
 * File: SupportDefs.cpp
 * Description: Body of support functions
 *
 * --------------------------------------------------------------------------*/
#include "SupportDefs.h"

#ifdef BHAPI_BUILD_LIBRARY
namespace bhapi {
extern EXPORTBHAPI const  __be_uint8 major_version;
extern EXPORTBHAPI const  __be_uint8 minor_version;
extern EXPORTBHAPI const  __be_uint8 micro_version;
extern EXPORTBHAPI const  __be_uint8 interface_age;
extern EXPORTBHAPI const  __be_uint16 binary_age;
}
#endif

EXPORTBHAPI int32 	bhapi::atomic_add (int32 *value, int32 addValue) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_add64 (int64 *value, int64 addValue) { return 0; }
EXPORTBHAPI int32 	bhapi::atomic_and (int32 *value, int32 andValue) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_and64 (int64 *value, int64 andValue) { return 0; }
EXPORTBHAPI int32 	bhapi::atomic_get (int32 *value) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_get64 (int64 *value) { return 0;}
EXPORTBHAPI int32 	bhapi::atomic_get_and_set (int32 *value, int32 newValue) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_get_and_set64 (int64 *value, int64 newValue) { return 0; }
EXPORTBHAPI int32 	bhapi::atomic_or (int32 *value, int32 orValue) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_or64 (int64 *value, int64 orValue) { return 0; }
EXPORTBHAPI void 	bhapi::atomic_set (int32 *value, int32 newValue) {}
EXPORTBHAPI void 	bhapi::atomic_set64 (int64 *value, int64 newValue) {}
EXPORTBHAPI int32 	bhapi::atomic_test_and_set (int32 *value, int32 newValue, int32 testAgains) { return 0; }
EXPORTBHAPI int64 	bhapi::atomic_test_and_set64 (int64 *value, int64 newValue, int64 testAgainst) { return 0; }
EXPORTBHAPI void * 	bhapi::get_stack_frame (void) { return NULL; }
EXPORTBHAPI const char* B_EMPTY_STRING = "";
