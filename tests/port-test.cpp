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
 * File: port-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./../kernel/Kernel.h>
#include "./../kernel/Debug.h>

int main()
{
	void *fPort = etk_create_port(10, "Test Port", ETK_AREA_ACCESS_OWNER);
	if(fPort == NULL)
	{
		ETK_OUTPUT("Creat port failed!\n");
		exit(1);
	}

	const char *msg = "That's a port test for the ETK library.";
	etk_write_port(fPort, 'poTe', (void*)msg, (size_t)strlen(msg));

	ssize_t len = etk_port_buffer_size(fPort);
	if(len > 0)
	{
		ETK_OUTPUT("Write Success: Len(%ld)\n", len);
		char *buffer = new char[len + 1];
		bzero(buffer, len + 1);
		if(buffer)
		{
			eint32_t code;
			if(etk_read_port(fPort, &code, buffer, len + 1) == E_NO_ERROR)
				ETK_OUTPUT("Read[%ld]: \"%s\"\n", code, buffer);

			delete[] buffer;
		}
	}

	etk_delete_port(fPort);
}


