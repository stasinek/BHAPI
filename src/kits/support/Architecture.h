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
 * File: Architecture.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2013-2014 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef BHAPI_ARCHITECTURE_H
#define BHAPI_ARCHITECTURE_H

#include <kits/support/SupportDefs.h>
__BEGIN_DECLS
#ifdef __cplusplus
namespace bhapi {
#endif	/* __cplusplus */
const char*	__get_architecture();
const char*	__get_primary_architecture();
size_t		__get_secondary_architectures(const char** a_architectures, size_t a_count);
size_t		__get_architectures(const char** a_architectures, size_t a_count);
const char*	__guess_architecture_for_path(const char* a_path);
#ifdef __cplusplus
}
#endif	/* __cplusplus */
__END_DECLS
/* C++ API */
#ifdef __cplusplus
class BString;
class BStringList;
namespace bhapi {
status_t	__get_secondary_architectures(BStringList& a_architectures);
status_t	__get_architectures(BStringList& a_architectures);
}

#endif	/* __cplusplus */
#endif	/* _ARCHITECTURE_H */
