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
 * File: area-test.c
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#include "./../kernel/Kernel.h>

int main()
{
	char name[E_OS_NAME_LENGTH + 1];
	void *address = NULL;
	void *area = NULL;

	ETK_OUTPUT("Checking whether area support name which length is E_OS_NAME_LENGTH ...");

	bzero(name, E_OS_NAME_LENGTH + 1);
	memset(name, '1', E_OS_NAME_LENGTH);

	area = etk_create_area(name, &address, 1048576, E_WRITE_AREA, ETK_AREA_USER_DOMAIN, ETK_AREA_ACCESS_ALL);

	if(area == NULL || address == NULL)
		ETK_OUTPUT("\t[Failed]\n");
	else
		ETK_OUTPUT("\t[  OK  ]\n");

	if(area) etk_delete_area(area);

	return 0;
}


