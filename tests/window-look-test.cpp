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
 * File: window-look-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../kernel/Debug.h>

class TWindow : public EWindow {
public:
	TWindow(ERect frame,
		const char *title,
		e_window_type type,
		euint32 flags,
		euint32 workspace = E_CURRENT_WORKSPACE);
	TWindow(ERect frame,
		const char *title,
		e_window_look look,
		e_window_feel feel,
		euint32 flags,
		euint32 workspace = E_CURRENT_WORKSPACE);
	virtual ~TWindow();

	virtual bool QuitRequested();

private:
	bool quited;
};


class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual void	ReadyToRun();
};


TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32 flags, euint32 workspace)
	: EWindow(frame, title, type, flags, workspace), quited(false)
{
}


TWindow::TWindow(ERect frame, const char *title, e_window_look look, e_window_feel feel, euint32 flags, euint32 workspace)
	: EWindow(frame, title, look, feel, flags, workspace), quited(false)
{
}


TWindow::~TWindow()
{
}


bool
TWindow::QuitRequested()
{
	if(quited) return true;
	etk_app->PostMessage(E_QUIT_REQUESTED);
	quited = true;
	return false;
}


TApplication::TApplication()
	: EApplication("application/x-vnd.lee-test-app")
{
}


TApplication::~TApplication()
{
}


void
TApplication::ReadyToRun()
{
	TWindow *win = new TWindow(ERect(20, 20, 400, 400), "Window Test -- Modal Window", E_MODAL_WINDOW, 0);
	win->Show();

	win = new TWindow(ERect(50, 50, 450, 450), "Window Test -- Document Window", E_DOCUMENT_WINDOW, 0);
	win->Show();

	win = new TWindow(ERect(100, 100, 500, 500), "Window Test -- Floating Window", E_FLOATING_WINDOW, 0);
	win->Show();

	win = new TWindow(ERect(150, 150, 550, 550), "Window Test -- Bordered Window", E_BORDERED_WINDOW, 0);
	win->Show();

	win = new TWindow(ERect(200, 200, 600, 600), "Window Test -- Titled Window", E_TITLED_WINDOW, E_NOT_ZOOMABLE|E_NOT_CLOSABLE);
	win->Show();

	win = new TWindow(ERect(250, 250, 650, 650), "Window Test -- No Border Look", E_NO_BORDER_WINDOW_LOOK, E_NORMAL_WINDOW_FEEL, 0);
	win->Show();

	win->Lock();
	win->Activate(false);
	win->Unlock();
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	return 0;
}

