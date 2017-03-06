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
 * File: BHAPI_wrapper_image.cpp
 *
 * --------------------------------------------------------------------------*/

#include "../../kernel/Kernel.h"
#include "../../storage/Path.h"
#include "../../support/StringClass.h"
#include "../../support/Errors.h"

#include <winsock2.h>
#include <windows.h>
#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif

EXPORTBHAPI void*
bhapi::load_addon(const char* path)
{
	BPath aPath(path, NULL, true);
	BString filename = aPath.Path();

#ifdef __CYGWIN__
	char buf[B_MAXPATH + 1];
	bzero(buf, B_MAXPATH + 1);
	cygwin_conv_to_full_win32_path(filename.String(), buf);
	filename.SetTo(buf);
#endif
	filename.ReplaceAll("/", "\\");
	if(filename.Length() <= 0 || filename.Length() > B_MAXPATH) return NULL;

    return((void*)LoadLibraryA(filename.String()));
}


EXPORTBHAPI b_status_t
bhapi::unload_addon(void *data)
{
	if(data == NULL) return B_ERROR;
	if(FreeLibrary((HMODULE)data) == 0) return B_ERROR;
	return B_OK;
}


EXPORTBHAPI b_status_t
bhapi::get_image_symbol(void *data, const char *name, void **ptr)
{
	if(data == NULL || name == NULL || *name == 0 || ptr == NULL) return B_BAD_VALUE;

	FARPROC aProc = GetProcAddress((HMODULE)data, name);
	if(aProc == NULL) return B_ERROR;

	*ptr = (void*)aProc;

	return B_OK;
}

