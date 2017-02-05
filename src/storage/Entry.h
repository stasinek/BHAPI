/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Entry.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_ENTRY_H__
#define __ETK_ENTRY_H__ 

#include "./Path.h"

#ifdef __cplusplus /* Just for C++ */

class EDirectory;

class _IMPEXP_ETK EEntry {
public:
	EEntry();
	EEntry(const char *dir, const char *leaf, bool traverse = false);
	EEntry(const EDirectory *dir, const char *leaf, bool traverse = false);
	EEntry(const char *path, bool traverse = false);
	EEntry(const EEntry &entry);
	virtual ~EEntry();

	e_status_t	SetTo(const char *dir, const char *leaf, bool traverse = false);
	e_status_t	SetTo(const EDirectory *dir, const char *leaf, bool traverse = false);
	e_status_t	SetTo(const char *path, bool traverse = false);
	void		Unset();

	e_status_t	InitCheck() const;

	bool		Exists() const;
	bool		IsHidden() const;

	bool		IsFile() const;
	bool		IsDirectory() const;
	bool		IsSymLink() const;

	e_status_t	GetSize(eint64 *file_size) const;
	e_status_t	GetModificationTime(e_bigtime_t *time) const;
	e_status_t	GetCreationTime(e_bigtime_t *time) const;
	e_status_t	GetAccessTime(e_bigtime_t *time) const;

	const char	*Name() const;
	e_status_t	GetName(char *buffer, size_t bufferSize) const;

	const char	*Path() const;
	e_status_t	GetPath(EPath *path) const;

	e_status_t	GetParent(EEntry *entry) const;
	e_status_t	GetParent(EPath *path) const;
	e_status_t	GetParent(EDirectory *dir) const;

	bool		operator==(const EEntry &entry) const;
	bool		operator!=(const EEntry &entry) const;
	EEntry&		operator=(const EEntry &entry);

private:
	friend class EDirectory;

	char *fName;
};

#endif /* __cplusplus */

#endif /* __ETK_ENTRY_H__ */

