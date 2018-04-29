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
#ifdef BHAPI_OS_BEOS

#include <be/kernel/image.h>

#include <os/kernel.h>
#include <kits/storage/Path.h>


BHAPI_IMPEXP void*
bhapi::load_addon(const char* path)
{
	BPath aPath(path, NULL, true);
	if(aPath.Path() == NULL || strlen(aPath.Path()) > B_MAXPATH) return NULL;

	image_id *image = (image_id*)malloc(sizeof(image_id));
	if(image == NULL) return NULL;

	if((*image = load_add_on(aPath.Path())) < 0)
	{
		free(image);
		return NULL;
	}

	return((void*)image);
}


BHAPI_IMPEXP status_t 
bhapi::unload_addon(void *data)
{
	if(data == NULL) return B_ERROR;
	if(unload_add_on(*((image_id*)data)) != B_OK) return B_ERROR;
	free(data);
	return B_OK;
}


BHAPI_IMPEXP status_t 
bhapi::get_image_symbol(void *data, const char *name, void **ptr)
{
	if(data == NULL || name == NULL || *name == 0 || ptr == NULL) return B_BAD_VALUE;

	void *aPtr = NULL;
	if(get_image_symbol(*((image_id*)data), name, B_SYMBOL_TYPE_ANY, &aPtr) != B_OK) return B_ERROR;
	if(aPtr == NULL) return B_ERROR;

	*ptr = aPtr;

	return B_OK;
}
#endif // BHAPI_OS_BEOS
