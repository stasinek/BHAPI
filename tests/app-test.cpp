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
 * File: app-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "./../app/Application.h>
#include "./../kernel/Kernel.h>
#include "./../kernel/OS.h>
#include "./../kernel/Debug.h>

//#define TEST_EXIT_THREAD

class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual bool	QuitRequested();
	virtual void	ReadyToRun();
	virtual void	Pulse();

private:
	void *thread;
	int countSecond;
};


void
TApplication::Pulse()
{
	ETK_OUTPUT("%s: %d seconds past.\n", __PRETTY_FUNCTION__, ++countSecond);
}


TApplication::TApplication()
	: EApplication("application/x-vnd.lee-test-app", false), thread(NULL), countSecond(0)
{
	SetPulseRate(E_INT64_CONSTANT(1000000));
}


TApplication::~TApplication()
{
	if(thread) etk_delete_thread(thread);
}


bool
TApplication::QuitRequested()
{
#ifdef TEST_EXIT_THREAD
	etk_exit_thread(E_OK);
#endif

	return true;
}


static e_status_t testTask(void *arg)
{
	ETK_OUTPUT("\nI'll quit the application after 10 seconds...\n");
	e_snooze(10000000);
	if(etk_app)
	{
		ETK_OUTPUT("Send Quit...\n");
		etk_app->PostMessage(E_QUIT_REQUESTED);
		ETK_OUTPUT("Send.\n");
	}
	else
	{
		ETK_WARNING("NO APP!");
	}
	return E_OK;
}


void
TApplication::ReadyToRun()
{
	thread = etk_create_thread(testTask, E_NORMAL_PRIORITY, NULL, NULL);
	if(!thread || etk_resume_thread(thread) != E_OK)
	{
		ETK_WARNING("Create thread error! I'll Quit.");
		Quit();
	}
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	e_snooze(100);

	return 0;
}

