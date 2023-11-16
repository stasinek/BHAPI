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
 * File: helloworld2.cpp
 *
 * --------------------------------------------------------------------------*/

#include <etkxx.h>

#define MSG_HELLO_WORLD		'helo'


static e_filter_result hello_filter_hook(EMessage *message, EHandler **target, EMessageFilter *filter)
{
	etk_app->PostMessage(E_QUIT_REQUESTED);
	return E_SKIP_MESSAGE;
}


int main(int argc, char **argv)
{
	new EApplication("application/x-vnd.etkxx-helloworld2-app");

	EWindow *win = new EWindow(ERect(100, 100, 300, 170),
				   "Hello World Sample 2",
				   E_TITLED_WINDOW, 0);
	EButton *btn = new EButton(ERect(20, 20, 180, 50), NULL,
				   "Hello World",
				   new EMessage(MSG_HELLO_WORLD),
				   E_FOLLOW_ALL);

	win->Lock();
	win->AddChild(btn);
	win->Show();
	win->AddCommonFilter(new EMessageFilter(MSG_HELLO_WORLD, hello_filter_hook));
	win->Unlock();

	etk_app->Run();

	delete etk_app;

	return 0;
}


#if defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))
#include <windows.h>
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif // defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))

