#ifndef BHAPI_H
#define BHAPI_H

/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: bhapi.h
 *
 * --------------------------------------------------------------------------*/

#if defined(BHAPI_LIBRARY)
#  define BHAPISHARED_EXPORT Q_DECL_EXPORT
#else
#  define BHAPISHARED_EXPORT Q_DECL_IMPORT
#endif
#include <src/Haiku.h>

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
//#include <kits/AppKit.h>
//#include <kits/DeviceKit.h>
//#include <kits/GameKit.h>
//#include <kits/InterfaceKit.h>
//#include <kits/KernelKit.h>
//#include <kits/LocaleKit.h>
//#include <kits/MailKit.h>
//#include <kits/MediaKit.h>
//#include <kits/MidiKit.h>
//#include <kits/NetKit.h>
//#include <kits/NetworkKit.h>
//#include <kits/RenderKit.h>
#include <kits/SupportKit.h>
//#include <kits/StorageKit.h>
//#include <kits/TranslationKit.h>
//#include <kits/XmlKit.h>
//-----------------------------------------------------------------------------
#ifndef BHAPI_BUILD_LIBRARY
#ifdef __cplusplus
extern "C" {
namespace bhapi {
#endif // __cplusplus
extern BHAPI_IMPEXP const  uint8_t be_major_version;
extern BHAPI_IMPEXP const  uint8_t be_minor_version;
extern BHAPI_IMPEXP const  uint8_t be_micro_version;
extern BHAPI_IMPEXP const  uint8_t be_interface_age;
extern BHAPI_IMPEXP const  uint16_t be_binary_age;
#ifdef __cplusplus
} /* namespace */
}
#endif // __cplusplus
#else
extern const  uint8_t be_major_version;
extern const  uint8_t be_minor_version;
extern const  uint8_t be_micro_version;
extern const  uint8_t be_interface_age;
extern const  uint16_t be_binary_age;
#endif
//-----------------------------------------------------------------------------
class BHAPI
{
public:
      BHAPI();
     ~BHAPI();
};
//-----------------------------------------------------------------------------
#endif // BHAPI_H
