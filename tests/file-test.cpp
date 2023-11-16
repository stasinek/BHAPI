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
 * File: file-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <etkxx.h>


static void show_usage(const char *prog)
{
	ETK_OUTPUT("%s: filename filesize\n", prog);
}


int main(int argc, char **argv)
{
	if(argc != 3)
	{
		show_usage(argv[0]);
		exit(1);
	}

	eint64 size;

	EString str(argv[2]);
	if(str.GetInteger(&size) == false)
	{
		show_usage(argv[0]);
		exit(1);
	}

	EFile f(argv[1], E_CREATE_FILE | E_FAIL_IF_EXISTS | E_READ_WRITE);
	EFile another(f);
	if(another.InitCheck() != E_OK)
	{
		ETK_WARNING("Can't create \"%s\".\n", argv[1]);
		exit(1);
	}

	ETK_OUTPUT("Create \"%s\" with %I64i bytes", argv[1], size);

	another.SetSize(size);

	return 0;
}

