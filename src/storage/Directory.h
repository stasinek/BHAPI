/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: Directory.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_DIRECTORY__H
#define BHAPI_DIRECTORY__H 

#include "./Entry.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXP_BHAPI BDirectory {
public:
    BDirectory();
    BDirectory(const char *path);
    virtual ~BDirectory();

	b_status_t	InitCheck() const;
	b_status_t	SetTo(const char *path);
	void		Unset();

    b_status_t	GetEntry(BEntry *entry) const;
    b_status_t	GetNextEntry(BEntry *entry, bool traverse = false);
	b_status_t	Rewind();
    b_int32		CountEntries();

	void		DoForEach(bool (*func)(const char *path));
	void		DoForEach(bool (*func)(const char *path, void *user_data), void *user_data);

private:
    friend class BEntry;

	void *fDir;
	char *fName;
};

#endif /* __cplusplus */

#endif /* BHAPI_DIRECTORY__H */

