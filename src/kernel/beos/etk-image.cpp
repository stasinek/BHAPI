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
 * File: etk-image.cpp
 *
 * --------------------------------------------------------------------------*/

#include <be/kernel/image.h>

#include "./../kernel/Kernel.h>
#include "./../storage/Path.h>


_IMPEXP_ETK void*
etk_load_addon(const char* path)
{
	EPath aPath(path, NULL, true);
	if(aPath.Path() == NULL || strlen(aPath.Path()) > E_MAXPATH) return NULL;

	image_id *image = (image_id*)malloc(sizeof(image_id));
	if(image == NULL) return NULL;

	if((*image = load_add_on(aPath.Path())) < 0)
	{
		free(image);
		return NULL;
	}

	return((void*)image);
}


_IMPEXP_ETK e_status_t
etk_unload_addon(void *data)
{
	if(data == NULL) return E_ERROR;
	if(unload_add_on(*((image_id*)data)) != B_OK) return E_ERROR;
	free(data);
	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_get_image_symbol(void *data, const char *name, void **ptr)
{
	if(data == NULL || name == NULL || *name == 0 || ptr == NULL) return E_BAD_VALUE;

	void *aPtr = NULL;
	if(get_image_symbol(*((image_id*)data), name, B_SYMBOL_TYPE_ANY, &aPtr) != B_OK) return E_ERROR;
	if(aPtr == NULL) return E_ERROR;

	*ptr = aPtr;

	return E_OK;
}

