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
 * File: region-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Debug.h>
#include "./../interface/Region.h>

int main()
{
	ERect r1(10, 10, 20, 20);
	ERect r2(15, 12, 30, 39);
	ERect r3(1, 1, 5, 5);
	ERect r4(32, 37, 49, 51);
	ERegion *region = new ERegion();

	ETK_OUTPUT("Region initalize as:");
	r1.PrintToStream();
	ETK_OUTPUT("\n");
	region->Set(r1);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region include:");
	r2.PrintToStream();
	ETK_OUTPUT("\n");
	region->Include(r2);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region include:");
	r3.PrintToStream();
	ETK_OUTPUT("\n");
	region->Include(r3);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region include:");
	r4.PrintToStream();
	ETK_OUTPUT("\n");
	region->Include(r4);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region make empty.\n");
	region->MakeEmpty();
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region set to:");
	r1.PrintToStream();
	ETK_OUTPUT("\n");
	region->Set(r1);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	ETK_OUTPUT("Region exclude:");
	r2.PrintToStream();
	ETK_OUTPUT("\n");
	region->Exclude(r2);
	ETK_OUTPUT("[REGION]: ");
	region->PrintToStream();
	ETK_OUTPUT("\n\n");

	delete region;

	return 0;
}
