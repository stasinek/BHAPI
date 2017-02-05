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
 * File: looper-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "./../kernel/Kernel.h>
#include "./../kernel/OS.h>
#include "./../app/Looper.h>
#include "./../kernel/Debug.h>
#include "./../app/Messenger.h>


class TLooper : public ELooper {
public:
	TLooper();
	virtual ~TLooper();

	virtual void MessageReceived(EMessage *msg);
};


TLooper::TLooper()
	: ELooper()
{
}


TLooper::~TLooper()
{
}


void
TLooper::MessageReceived(EMessage *msg)
{
	ETK_OUTPUT(".");
	if(msg->IsSourceWaiting())
	{
		ETK_OUTPUT("\nReplying message...\n");
		EMessage aMsg(E_REPLY);
		aMsg.AddString("reply", "This a reply-message...");
		msg->SendReply(&aMsg);
	}
	e_snooze(1000);
}


e_status_t testTask(void *arg)
{
	TLooper *looper = (TLooper*)arg;

	if(looper)
	{
		EMessenger msgr(looper);

		ETK_OUTPUT("\nI'll send message and waiting for reply...\n");
		EMessage msg('tsuc');
		if(msgr.SendMessage(&msg, &msg) != E_OK)
			ETK_WARNING("\nNo reply!!!");
		else
			msg.PrintToStream();


		ETK_OUTPUT("\nI'll kill the looper...\n");

		looper->Lock();
		ETK_OUTPUT("\nkilling...\n");
		looper->Quit();
		ETK_OUTPUT("\nkilled.\n");
	}

	return E_OK;
}


int main(int argc, char **argv)
{
	TLooper *looper = new TLooper();

	looper->Lock();
	looper->Run();
	looper->Unlock();

	void *thread = etk_create_thread(testTask, E_NORMAL_PRIORITY, (void*)looper, NULL);
	if(!thread)
	{
		looper->Lock();
		looper->Quit();
		ETK_WARNING("Create thread error!");
		return 1;
	}

	eint32 counter = 0;
	while(counter < 300)
	{
		EMessage message('cust');

		char *buffer = e_strdup_printf("%s: counter -- %d", __FUNCTION__, counter + 1);

		if(buffer)
		{
			message.AddString("counter_info", buffer);
			free(buffer);
		}

//		looper->Lock();
		looper->PostMessage(&message);
//		looper->Unlock();

		e_snooze(1000);

		counter++;
	}

	e_status_t status;
	etk_wait_for_thread(thread, &status);

	etk_delete_thread(thread);

	return 0;
}

