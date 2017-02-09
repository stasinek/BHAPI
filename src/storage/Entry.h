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
 * File: Entry.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_ENTRY__H
#define BHAPI_ENTRY__H 

#include "./Path.h"

#ifdef __cplusplus /* Just for C++ */

class BDirectory;

class IMPEXP_BHAPI BEntry {
public:
    BEntry();
    BEntry(const char *dir, const char *leaf, bool traverse = false);
    BEntry(const BDirectory *dir, const char *leaf, bool traverse = false);
    BEntry(const char *path, bool traverse = false);
    BEntry(const BEntry &entry);
    virtual ~BEntry();

	b_status_t	SetTo(const char *dir, const char *leaf, bool traverse = false);
	b_status_t	SetTo(const BDirectory *dir, const char *leaf, bool traverse = false);
	b_status_t	SetTo(const char *path, bool traverse = false);
	void		Unset();

	b_status_t	InitCheck() const;

	bool		Exists() const;
	bool		IsHidden() const;

	bool		IsFile() const;
	bool		IsDirectory() const;
	bool		IsSymLink() const;

    b_status_t	GetSize(b_int64 *file_size) const;
    b_status_t	GetModificationTime(b_bigtime_t *time) const;
    b_status_t	GetCreationTime(b_bigtime_t *time) const;
    b_status_t	GetAccessTime(b_bigtime_t *time) const;

	const char	*Name() const;
	b_status_t	GetName(char *buffer, size_t bufferSize) const;

	const char	*Path() const;
    b_status_t	GetPath(BPath *path) const;

    b_status_t	GetParent(BEntry *entry) const;
    b_status_t	GetParent(BPath *path) const;
	b_status_t	GetParent(BDirectory *dir) const;

    bool		operator==(const BEntry &entry) const;
    bool		operator!=(const BEntry &entry) const;
    BEntry&		operator=(const BEntry &entry);

private:
	friend class BDirectory;

	char *fName;
};

#endif /* __cplusplus */

#endif /* BHAPI_ENTRY__H */

