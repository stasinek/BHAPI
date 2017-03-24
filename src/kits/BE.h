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
#ifndef BHAPI_BE_H
#define BHAPI_BE_H
//-----------------------------------------------------------------------------
#include "AppKit.h"
//#include "DeviceKit.h"
//#include "GameKit.h"
//#include "InterfaceKit.h"
#include "KernelKit.h"
//#include "LocaleKit.h"
//#include "MailKit.h"
//#include "MediaKit.h"
//#include "MidiKit.h"
//#include "NetKit.h"
//#include "NetworkKit.h"
//#include "RenderKit.h"
#include "SupportKit.h"
//#include "StorageKit.h"
//#include "TranslationKit.h"
//#include "XmlKit.h"
//-----------------------------------------------------------------------------
#include <Haiku.h>
//-----------------------------------------------------------------------------
#ifndef BHAPI_BUILD_LIBRARY
#ifdef __cplusplus
extern "C" {
namespace bhapi {
#endif // __cplusplus
extern BHAPI_IMPEXP const  uint8 be_major_version;
extern BHAPI_IMPEXP const  uint8 be_minor_version;
extern BHAPI_IMPEXP const  uint8 be_micro_version;
extern BHAPI_IMPEXP const  uint8 be_interface_age;
extern BHAPI_IMPEXP const  uint16 be_binary_age;
#ifdef __cplusplus
} /* namespace */
}
#endif // __cplusplus
#else
extern const  uint8 be_major_version;
extern const  uint8 be_minor_version;
extern const  uint8 be_micro_version;
extern const  uint8 be_interface_age;
extern const  uint16 be_binary_age;
#endif
//-----------------------------------------------------------------------------
#endif // BHAPI_BE_H
//-----------------------------------------------------------------------------
