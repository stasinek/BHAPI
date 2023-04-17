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
 * File: release_sem.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <etkxx.h>


void show_usage(char *prog)
{
	ETK_OUTPUT("Usage: %s [name] [count] [whether reschedule]\n", prog);
	ETK_OUTPUT("\t[whether reschedule]\t1 means reschedle when semaphore released.\n");
}


int main(int argc, char **argv)
{
	const char *name;
	eint64 count;
	euint32_t flags;
	void *sem;
	e_status_t status = E_ERROR;

	if(argc != 4)
	{
		show_usage(argv[0]);
		exit(1);
	}

	name = argv[1];
	count = (eint64)(atol(argv[2]));
	flags = (atoi(argv[3]) == 1 ? 0 : E_DO_NOT_RESCHEDULE);

	sem = etk_clone_sem(name);

	if(sem == NULL)
	{
		ETK_OUTPUT("Can not open IPC semaphore [%s]!\n", argv[1]);
		exit(1);
	}
	else
	{
		status = etk_release_sem_etc(sem, count, flags);
		switch(status)
		{
			case E_NO_ERROR:
			{
				ETK_OUTPUT("Release semaphore successed.\n");
				if(etk_get_sem_count(sem, &count) == E_NO_ERROR)
					ETK_OUTPUT("Now the semaphore count is: %I64i\n", count);
				else
					ETK_OUTPUT("Get the semaphore count failed.\n");
				break;
			}

			default:
			{
				ETK_OUTPUT("Release semaphore failed!\n");
				break;
			}
		}

		etk_delete_sem(sem);
	}

	return(status == E_NO_ERROR ? 0 : 1);
}

