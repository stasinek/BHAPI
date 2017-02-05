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
 * File: Archivable.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_ARCHIVABLE_H__
#define __ETK_ARCHIVABLE_H__

#include "SupportDefs.h"
#include "./../app/Message.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EArchivable {
public:
	EArchivable();
	EArchivable(const EMessage *from);
	virtual ~EArchivable();

	virtual e_status_t Archive(EMessage *into, bool deep = true) const;
	static EArchivable *Instantiate(const EMessage *from);
};


typedef EArchivable *(*e_instantiation_func)(const EMessage*);

_IMPEXP_ETK bool			e_validate_instantiation(const EMessage *from, const char *class_name);
_IMPEXP_ETK e_instantiation_func	e_find_instantiation_func(const char *class_name);
_IMPEXP_ETK e_instantiation_func	e_find_instantiation_func(const EMessage *archive_data);

#endif /* __cplusplus */

#endif /* __ETK_ARCHIVABLE_H__ */

