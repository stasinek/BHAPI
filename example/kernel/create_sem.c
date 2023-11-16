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
 * File: create_sem.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <etkxx.h>


void show_usage(char *prog)
{
	ETK_OUTPUT("Usage: %s [name] [count]\n", prog);
}


int main(int argc, char **argv)
{
	const char *name;
	eint64 count;
	void *sem;

	if(argc != 3)
	{
		show_usage(argv[0]);
		exit(1);
	}

	name = argv[1];
	count = (eint64)(atol(argv[2]));

	sem = etk_create_sem(count, name, ETK_AREA_ACCESS_OWNER);

	if(sem == NULL)
	{
		ETK_OUTPUT("Create IPC semaphore [%s][%s] failed!\n", argv[1], argv[2]);
		exit(1);
	}
	else
	{
		void *waitSem = etk_create_sem(0, "wait_sem", ETK_AREA_ACCESS_ALL);
		if(waitSem == NULL)
		{
			etk_delete_sem(sem);
			ETK_OUTPUT("Create wait semaphore failed!\n");
			exit(1);
		}
		else
		{
			ETK_OUTPUT("I'll wait until you run \"release_sem wait_sem 1 1\" to end me...\n");
			etk_acquire_sem(waitSem);
			etk_close_sem(sem);
			etk_delete_sem(sem);
			etk_delete_sem(waitSem);
		}
	}

	return 0;
}

