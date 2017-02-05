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
 * File: Path.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef _WIN32
#include <ctype.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "./../support/String.h>

#include "Path.h"

inline void etk_path_prepend_current_directory(EString &path)
{
	char buf[E_MAXPATH + 1];
	bzero(buf, E_MAXPATH + 1);

#ifndef _WIN32
	getcwd(buf, E_MAXPATH);
#else
	_getcwd(buf, E_MAXPATH);

	EString str(buf);
	str.ReplaceAll("\\", "/");
	bzero(buf, E_MAXPATH + 1);
	str.CopyInto(buf, E_MAXPATH, 0, -1);
#endif

	eint32 len = (eint32)strlen(buf);
	if(buf[len - 1] != '/')
	{
		buf[len] = '/';
		len++;
	}

	path.Prepend(buf, len);
}


inline bool etk_path_do_normalization(EString &path)
{
	if(path.Length() <= 0) return false;

	bool retVal = false;

#ifdef _WIN32
	if(path.Length() < 2 ||
	   !(((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z')) && path[1] == ':'))
	{
		retVal = true;
		etk_path_prepend_current_directory(path);
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
		etk_path_prepend_current_directory(path);
	}
#endif

	if(path.FindFirst("/./") >= 0)
	{
		retVal = true;
		path.ReplaceAll("/./", "/");
	}

	eint32 uponIndex;
	while((uponIndex = path.FindLast("/../")) >= 0)
	{
		retVal = true;
		eint32 tmp = path.FindLast("/", uponIndex - 1);
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


e_status_t etk_path_expound(EString &path, const char *dir, const char *leaf, bool *normalize)
{
	EString str(dir);
	EString str_leaf(leaf);
	if(str.Length() <= 0) return E_BAD_VALUE;

#ifdef _WIN32
	if(str_leaf.FindFirst(":") >= 0) return E_BAD_VALUE;

	str.ReplaceAll("\\", "/");
	str_leaf.ReplaceAll("\\", "/");
#else
	if(!(str_leaf.Length() <= 0 || str_leaf[0] != '/')) return E_BAD_VALUE;
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

	bool strNormalizeDone = etk_path_do_normalization(str);
	if(str.Length() <= 0) return E_BAD_VALUE;
#ifdef _WIN32
	if(str.FindFirst(":", str.FindFirst(":") + 1) >= 0) return E_BAD_VALUE;
#endif
	if(strNormalizeDone && normalize != NULL) *normalize = true;

	path = str;

	return E_OK;
}


e_status_t etk_path_get_parent(EString &parent, const char *path)
{
	if(path == NULL || *path == 0) return E_BAD_VALUE;

#ifdef _WIN32
	if(strlen(path) <= 3) return E_ENTRY_NOT_FOUND;
#else
	if(strlen(path) == 1 && *path == '/') return E_ENTRY_NOT_FOUND;
#endif

	parent = path;
	eint32 slashIndex = parent.FindLast("/");
	parent.Remove(slashIndex, -1);
#ifndef _WIN32
	if(parent.Length() == 0) parent = "/";
#else
	if(parent.Length() < 3) parent.Append("/");
#endif

	return E_OK;
}



EPath::EPath()
	: fPath(NULL)
{
}


EPath::EPath(const char *dir, const char *leaf, bool normalize)
	: fPath(NULL)
{
	SetTo(dir, leaf, normalize);
}


EPath::EPath(const EPath &path)
	: fPath(NULL)
{
	if(path.fPath != NULL) fPath = EStrdup(path.fPath);
}


EPath::~EPath()
{
	if(fPath != NULL) delete[] fPath;
}


e_status_t
EPath::SetTo(const char *dir, const char *leaf, bool normalize)
{
	EString str;
	e_status_t status = etk_path_expound(str, dir, leaf, &normalize);
	if(status != E_OK) return status;

	if(normalize)
	{
		EString filename(str);
#ifdef _WIN32
		filename.ReplaceAll("/", "\\");

		struct _stat st;
		if(_stat(filename.String(), &st) != 0) str.MakeEmpty();
#else
		struct stat st;
		if(stat(filename.String(), &st) != 0) str.MakeEmpty();
#endif
	}

	if(str.Length() <= 0) return E_BAD_VALUE;

	char *aPath = EStrdup(str.String());
	if(aPath == NULL) return E_NO_MEMORY;

	if(fPath != NULL) delete[] fPath;
	fPath = aPath;

	return E_OK;
}


e_status_t
EPath::Append(const char *path, bool normalize)
{
	if(fPath == NULL) return E_BAD_VALUE;
	if(path == NULL || *path == 0) return E_BAD_VALUE;

	return SetTo(fPath, path, normalize);
}


void
EPath::Unset()
{
	if(fPath != NULL)
	{
		delete[] fPath;
		fPath = NULL;
	}
}


const char*
EPath::Path() const
{
	return fPath;
}


const char*
EPath::Leaf() const
{
	if(fPath == NULL) return NULL;

#ifdef _WIN32
	if(strlen(fPath) <= 3) return NULL;
#endif

	eint32 slashIndex = -1;

	for(eint32 i = (eint32)strlen(fPath) - 1; i >= 0; i--)
	{
		if(fPath[i] == '/')
		{
			slashIndex = i;
			break;
		}
	}

	if(slashIndex < 0 || slashIndex == (eint32)strlen(fPath) - 1) return NULL;

	return(fPath + (slashIndex + 1));
}


e_status_t
EPath::GetParent(EPath *parent) const
{
	if(parent == NULL) return E_BAD_VALUE;
	if(fPath == NULL) return E_NO_INIT;

	EString str;
	e_status_t status = etk_path_get_parent(str, fPath);
	if(status != E_OK) return status;

	return parent->SetTo(str.String(), NULL, false);
}


bool
EPath::operator==(const EPath &path) const
{
	return operator==(path.fPath);
}


bool
EPath::operator==(const char *path) const
{
	if(fPath == NULL && path == NULL) return true;
	if(fPath == NULL || path == NULL) return false;
	return(!(strlen(fPath) != strlen(path) || strcmp(fPath, path) != 0));
}


bool
EPath::operator!=(const EPath &path) const
{
	return operator!=(path.fPath);
}


bool
EPath::operator!=(const char *path) const
{
	if(fPath == NULL && path == NULL) return false;
	if(fPath == NULL || path == NULL) return true;
	if(strlen(fPath) != strlen(path)) return true;
	return(strcmp(fPath, path) != 0);
}


EPath&
EPath::operator=(const EPath &path)
{
	if(fPath != NULL) delete[] fPath;
	if(path.fPath != NULL)
		fPath = EStrdup(path.fPath);
	else
		fPath = NULL;
	return *this;
}


EPath&
EPath::operator=(const char *path)
{
	if(fPath != NULL) delete[] fPath;
	fPath = NULL;
	SetTo(path, NULL, false);
	return *this;
}

