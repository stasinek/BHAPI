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
 * File: texteditable-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/Box.h>
#include "./../interface/TextEditable.h>

#include "./../kernel/Debug.h>

#define BTN_EDITABLE_ENTER	'edt1'

class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32 resizingMode, euint32 flags);
	virtual ~TView();
};


TView::TView(ERect frame, const char *name, euint32 resizingMode, euint32 flags)
	: EView(frame, name, resizingMode, flags)
{
	ETextEditable *edt = new ETextEditable(ERect(10, 10, 150, 60), NULL, "Test Something", new EMessage(BTN_EDITABLE_ENTER), E_FOLLOW_ALL, E_WILL_DRAW | E_FRAME_EVENTS | E_NAVIGABLE);
//	edt->SetAlignment(E_ALIGN_CENTER);
	AddChild(edt);

	edt = new ETextEditable(ERect(10, 100, 250, 130), NULL, "这个世界日新月异", new EMessage(BTN_EDITABLE_ENTER), E_FOLLOW_NONE, E_WILL_DRAW | E_FRAME_EVENTS | E_NAVIGABLE);
	AddChild(edt);
//	edt->MakeEditable(false);
	edt->Select(2, 5);
//	edt->ResizeToPreferred();

	edt = new ETextEditable(ERect(10, 150, 40, 180), NULL, "失效的编辑栏", NULL, E_FOLLOW_NONE, E_WILL_DRAW | E_FRAME_EVENTS);
	AddChild(edt);
	edt->SetEnabled(false);
	edt->ResizeToPreferred();
}


TView::~TView()
{
}


class TWindow : public EWindow {
public:
	TWindow(ERect frame,
		const char *title,
		e_window_type type,
		euint32 flags,
		euint32 workspace = E_CURRENT_WORKSPACE);
	virtual ~TWindow();

	virtual void WindowActivated(bool state);
	virtual bool QuitRequested();

	virtual void MessageReceived(EMessage *msg);

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
//	SetBackgroundColor(213, 213, 213);

	EView *view_top = new EView(frame.OffsetToCopy(E_ORIGIN), NULL, E_FOLLOW_ALL, 0);
	EBox *box = new EBox(ERect(100, 100, 300, 300), NULL, E_FOLLOW_ALL);
	box->SetLabelAlignment(E_ALIGN_CENTER);
	box->SetLabel("Just for test");
//	box->SetPenSize(5);
	view_top->AddChild(box);

	EView *tview = new TView(box->ContentBounds(), NULL, E_FOLLOW_ALL, E_WILL_DRAW | E_FRAME_EVENTS);
	box->AddChild(tview);

	AddChild(view_top);
}


TWindow::~TWindow()
{
}

void
TWindow::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case BTN_EDITABLE_ENTER:
			{
				const char *str = NULL;
				msg->FindString("etk:texteditable-content", &str);
				ETK_OUTPUT("Enter pressed in TextEditable -- %s.\n", str ? str : "[EMPTY]");
			}
			break;

		default:
			EWindow::MessageReceived(msg);
	}
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
	TWindow *win = new TWindow(ERect(100, 100, 500, 500), "TextEditable Test 1", E_TITLED_WINDOW, 0);
	win->Show();
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	return 0;
}

