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
 * File: thread-test.c
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./../kernel/Kernel.h>
#include "./../kernel/Debug.h>

#define TEST_THREAD_NUM 10

static void* threads[TEST_THREAD_NUM];


static void exit_func(void *arg)
{
	eint32_t *index = (eint32_t*)arg;

	ETK_OUTPUT("[Thread %ld][etk_on_exit_thread end]\n", *index);
	free(index);
}


eint32_t test_func(void *arg)
{
	eint32_t index = *((eint32_t*)arg);
	eint32_t count = 0;

	if(etk_on_exit_thread(exit_func, arg) != E_OK)
	{
		free(arg);
		ETK_OUTPUT("[Thread %ld][etk_on_exit_thread failed]\n", index);
	}

	ETK_OUTPUT("[Thread %ld][START]\n", index);

	while(count < 5)
	{
		ETK_OUTPUT("[Thread %ld][count]:%ld\n", index, count);
		e_snooze(100 * (index + 1));
		count++;
	}

	ETK_OUTPUT("[Thread %ld][END]\n", index);

	return 0;
}


int main(int argc, char **argv)
{
	eint32_t i;

	for(i = 0; i < TEST_THREAD_NUM; i++)
	{
		char name[512];
		eint32_t *index;

		sprintf(name, "Thread %d", i);
		index = (eint32_t*)malloc(sizeof(eint32_t));
		*index = i;

		if((threads[i] = etk_create_thread(test_func, E_NORMAL_PRIORITY, (void*)index, NULL)) != NULL)
		{
			etk_resume_thread(threads[i]);
		}
		else
		{
			ETK_OUTPUT("[Thread %ld][NOT CREATE]\n", i);
		}
	}

	for(i = 0; i < TEST_THREAD_NUM; i++)
	{
		e_status_t status;
		etk_wait_for_thread(threads[i], &status);
		etk_delete_thread(threads[i]);
		ETK_OUTPUT("*********** Thread %ld finished *************\n", i);
	}

	etk_snooze(100000);

	return 0;
}


