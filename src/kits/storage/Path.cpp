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
 * File: Path.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Path.h"

#include "../support/String.h"
#include "../support/Errors.h"

#ifdef _WIN32
#include <ctype.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

inline void b_path_prepend_current_directory(BString &path)
{
	char buf[B_MAXPATH + 1];
	bzero(buf, B_MAXPATH + 1);

#ifndef _WIN32
	getcwd(buf, B_MAXPATH);
#else
	_getcwd(buf, B_MAXPATH);

	BString str(buf);
	str.ReplaceAll("\\", "/");
	bzero(buf, B_MAXPATH + 1);
	str.CopyInto(buf, B_MAXPATH, 0, -1);
#endif

	b_int32 len = (b_int32)strlen(buf);
	if(buf[len - 1] != '/')
	{
		buf[len] = '/';
		len++;
	}

	path.Prepend(buf, len);
}


inline bool b_path_do_normalization(BString &path)
{
	if(path.Length() <= 0) return false;

	bool retVal = false;

#ifdef _WIN32
	if(path.Length() < 2 ||
	   !(((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && path[1] == ':'))
	{
		retVal = true;
		b_path_prepend_current_directory(path);
	}
	if(path[0] >= 'a' && path[0] <= 'z')
	{
		char driveNumber = toupper(path[0]);
		path.Remove(0, 1);
		path.Prepend(driveNumber, 1);
	}
#else
	if(path[0] != '/')
	{
		retVal = true;
		b_path_prepend_current_directory(path);
	}
#endif

	if(path.FindFirst("/./") >= 0)
	{
		retVal = true;
		path.ReplaceAll("/./", "/");
	}

	b_int32 uponIndex;
	while((uponIndex = path.FindLast("/../")) >= 0)
	{
		retVal = true;
		b_int32 tmp = path.FindLast("/", uponIndex - 1);
		if(tmp < 0)
			path.MakeEmpty(); // invalid path
		else
			path.Remove(tmp + 1, uponIndex - tmp + 2);
	}

	while(path.FindFirst("//") >= 0)
	{
		retVal = true;
		path.ReplaceAll("//", "/");
	}

#ifndef _WIN32
	if(path.ByteAt(path.Length() - 1) == '/' && path.Length() > 1)
#else
	if(path.ByteAt(path.Length() - 1) == '/' && path.Length() > 3)
#endif
	{
		retVal = true;
		path.Remove(path.Length() - 1, 1);
	}

	return retVal;
}


b_status_t b_path_expound(BString &path, const char *dir, const char *leaf, bool *normalize)
{
	BString str(dir);
	BString str_leaf(leaf);
	if(str.Length() <= 0) return B_BAD_VALUE;

#ifdef _WIN32
	if(str_leaf.FindFirst(":") >= 0) return B_BAD_VALUE;

	str.ReplaceAll("\\", "/");
	str_leaf.ReplaceAll("\\", "/");
#else
	if(!(str_leaf.Length() <= 0 || str_leaf[0] != '/')) return B_BAD_VALUE;
#endif

	if(str_leaf.Length() > 0)
	{
		if(str.ByteAt(str.Length() - 1) != '/')
		{
			str.Append("/");
		}
		else
		{
			if(normalize != NULL) *normalize = true;
		}
		str.Append(str_leaf);
	}

	bool strNormalizeDone = b_path_do_normalization(str);
	if(str.Length() <= 0) return B_BAD_VALUE;
#ifdef _WIN32
	if(str.FindFirst(":", str.FindFirst(":") + 1) >= 0) return B_BAD_VALUE;
#endif
	if(strNormalizeDone && normalize != NULL) *normalize = true;

	path = str;

	return B_OK;
}


b_status_t b_path_get_parent(BString &parent, const char *path)
{
	if(path == NULL || *path == 0) return B_BAD_VALUE;

#ifdef _WIN32
	if(strlen(path) <= 3) return B_ENTRY_NOT_FOUND;
#else
	if(strlen(path) == 1 && *path == '/') return B_ENTRY_NOT_FOUND;
#endif

	parent = path;
	b_int32 slashIndex = parent.FindLast("/");
	parent.Remove(slashIndex, -1);
#ifndef _WIN32
	if(parent.Length() == 0) parent = "/";
#else
	if(parent.Length() < 3) parent.Append("/");
#endif

	return B_OK;
}



BPath::BPath()
	: fPath(NULL)
{
}


BPath::BPath(const char *dir, const char *leaf, bool normalize)
	: fPath(NULL)
{
	SetTo(dir, leaf, normalize);
}


BPath::BPath(const BPath &path)
	: fPath(NULL)
{
	if(path.fPath != NULL) fPath = bhapi::strdup(path.fPath);
}


BPath::~BPath()
{
	if(fPath != NULL) delete[] fPath;
}


b_status_t
BPath::SetTo(const char *dir, const char *leaf, bool normalize)
{
	BString str;
	b_status_t status = b_path_expound(str, dir, leaf, &normalize);
	if(status != B_OK) return status;

	if(normalize)
	{
		BString filename(str);
#ifdef _WIN32
		filename.ReplaceAll("/", "\\");

		struct _stat st;
		if(_stat(filename.String(), &st) != 0) str.MakeEmpty();
#else
		struct stat st;
		if(stat(filename.String(), &st) != 0) str.MakeEmpty();
#endif
	}

	if(str.Length() <= 0) return B_BAD_VALUE;

	char *aPath = bhapi::strdup(str.String());
	if(aPath == NULL) return B_NO_MEMORY;

	if(fPath != NULL) delete[] fPath;
	fPath = aPath;

	return B_OK;
}


b_status_t
BPath::Append(const char *path, bool normalize)
{
	if(fPath == NULL) return B_BAD_VALUE;
	if(path == NULL || *path == 0) return B_BAD_VALUE;

	return SetTo(fPath, path, normalize);
}


void
BPath::Unset()
{
	if(fPath != NULL)
	{
		delete[] fPath;
		fPath = NULL;
	}
}


const char*
BPath::Path() const
{
	return fPath;
}


const char*
BPath::Leaf() const
{
	if(fPath == NULL) return NULL;

#ifdef _WIN32
	if(strlen(fPath) <= 3) return NULL;
#endif

	b_int32 slashIndex = -1;

	for(b_int32 i = (b_int32)strlen(fPath) - 1; i >= 0; i--)
	{
		if(fPath[i] == '/')
		{
			slashIndex = i;
			break;
		}
	}

	if(slashIndex < 0 || slashIndex == (b_int32)strlen(fPath) - 1) return NULL;

	return(fPath + (slashIndex + 1));
}


b_status_t
BPath::GetParent(BPath *parent) const
{
	if(parent == NULL) return B_BAD_VALUE;
	if(fPath == NULL) return B_NO_INIT;

	BString str;
	b_status_t status = b_path_get_parent(str, fPath);
	if(status != B_OK) return status;

	return parent->SetTo(str.String(), NULL, false);
}


bool
BPath::operator==(const BPath &path) const
{
	return operator==(path.fPath);
}


bool
BPath::operator==(const char *path) const
{
	if(fPath == NULL && path == NULL) return true;
	if(fPath == NULL || path == NULL) return false;
	return(!(strlen(fPath) != strlen(path) || strcmp(fPath, path) != 0));
}


bool
BPath::operator!=(const BPath &path) const
{
	return operator!=(path.fPath);
}


bool
BPath::operator!=(const char *path) const
{
	if(fPath == NULL && path == NULL) return false;
	if(fPath == NULL || path == NULL) return true;
	if(strlen(fPath) != strlen(path)) return true;
	return(strcmp(fPath, path) != 0);
}


BPath&
BPath::operator=(const BPath &path)
{
	if(fPath != NULL) delete[] fPath;
	if(path.fPath != NULL)
		fPath = bhapi::strdup(path.fPath);
	else
		fPath = NULL;
	return *this;
}


BPath&
BPath::operator=(const char *path)
{
	if(fPath != NULL) delete[] fPath;
	fPath = NULL;
	SetTo(path, NULL, false);
	return *this;
}

