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
 * File: stringview-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/StringView.h>

#include "./../kernel/Debug.h>

class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags);
	virtual ~TView();
};


TView::TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags)
	: EView(frame, name, resizingMode, flags)
{
	EStringView *strv = new EStringView(ERect(10, 10, 150, 150), NULL, "在这里我们测试字符串，\n还有多行文字，\n这又是一行。", E_FOLLOW_ALL);
	strv->SetAlignment(E_ALIGN_RIGHT);
	strv->SetVerticalAlignment(E_ALIGN_MIDDLE);
	AddChild(strv);

	strv = new EStringView(ERect(10, 190, 40, 220), NULL, "失效的字符串");
	AddChild(strv);
	strv->SetEnabled(false);
	strv->ResizeToPreferred();
}


TView::~TView()
{
}


class TWindow : public EWindow {
public:
	TWindow(ERect frame,
		const char *title,
		e_window_type type,
		euint32_t flags,
		euint32_t workspace = E_CURRENT_WORKSPACE);
	virtual ~TWindow();

	virtual void WindowActivated(bool state);
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

TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace)
	: EWindow(frame, title, type, flags, workspace), quited(false)
{
//	SetBackgroundColor(0, 255, 255);

	EView *view_top = new TView(frame.OffsetToCopy(E_ORIGIN), NULL, E_FOLLOW_ALL, E_WILL_DRAW | E_FRAME_EVENTS);
	AddChild(view_top);
}


TWindow::~TWindow()
{
}


void
TWindow::WindowActivated(bool state)
{
	ETK_OUTPUT("Window activated %s.\n", state ? "true" : "false");
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
	TWindow *win = new TWindow(ERect(100, 100, 500, 500), "StringView Test", E_TITLED_WINDOW, 0);
	win->Show();
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	return 0;
}

