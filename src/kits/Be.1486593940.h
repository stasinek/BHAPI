/* --------------------------------------------------------------------------
 *
 * BHAPI++ --- The Easy Toolkit for C++ programing
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
 * File: BHAPI.h
 *
 * --------------------------------------------------------------------------*/

#include "./support/SupportDefs.h"
#define BHAPI_MAJOR_VERSION 0
#define BHAPI_MINOR_VERSION 0
#define BHAPI_MICRO_VERSION 1
#define BHAPI_INTERFACE_AGE 1
#define BHAPI_BINARY_AGE 1

extern "C" {

extern _IMPEXP_BHAPI const b_uint8 bhapi_major_version = BHAPI_MAJOR_VERSION;
extern _IMPEXP_BHAPI const b_uint8 bhapi_minor_version = BHAPI_MINOR_VERSION;
extern _IMPEXP_BHAPI const b_uint8 bhapi_micro_version = BHAPI_MICRO_VERSION;
extern _IMPEXP_BHAPI const b_uint8 bhapi_interface_age = BHAPI_INTERFACE_AGE;
extern _IMPEXP_BHAPI const b_uint16 bhapi_binary_age = BHAPI_BINARY_AGE;

} // extern "C"

#include "SupportKit.h"
#include "KernelKit.h"
#include "StorageKit.h"
#include "AppKit.h"
#include "RenderKit.h"
#include "InterfaceKit.h"
#include "XmlKit.h"
#include "NetKit.h"
