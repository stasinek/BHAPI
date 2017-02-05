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
 * File: window-wmaction.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/Button.h>

#define WIN_MINIMIZE_MSG		'tst0'
#define WIN_INACTIVATE_MSG		'tst1'
#define WIN_NEXT_WORKSPACE_MSG		'tst2'
#define WIN_PREV_WORKSPACE_MSG		'tst3'
#define WIN_ALL_WORKSPACE_MSG		'tst4'
#define WIN_BORDERED_LOOK_MSG		'tst5'
#define WIN_NO_BORDER_LOOK_MSG		'tst6'
#define WIN_TITLED_LOOK_MSG		'tst7'
#define WIN_DOCUMENT_LOOK_MSG		'tst8'
#define WIN_MODAL_LOOK_MSG		'tst9'
#define WIN_FLOATING_LOOK_MSG		'tsta'


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

	virtual void WindowActivated(bool state);
	virtual void WorkspacesChanged(euint32 old_ws, euint32 new_ws);
	virtual bool QuitRequested();

	virtual void Minimize(bool minimize);

	virtual void MessageReceived(EMessage *msg);
};


class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual void ReadyToRun();
};


TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32 flags, euint32 workspace)
	: EWindow(frame, title, type, flags, workspace)
{
	SetSizeLimits(10, 800, 10, 600);
}


TWindow::TWindow(ERect frame, const char *title, e_window_look look, e_window_feel feel, euint32 flags, euint32 workspace)
	: EWindow(frame, title, look, feel, flags, workspace)
{
}


TWindow::~TWindow()
{
}


void
TWindow::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case WIN_MINIMIZE_MSG:
			Minimize(true);
			break;

		case WIN_INACTIVATE_MSG:
			Activate(false);
			break;

		case WIN_PREV_WORKSPACE_MSG:
			SetWorkspaces((Workspaces() == E_ALL_WORKSPACES ? 0 : Workspaces() - 1));
			break;

		case WIN_NEXT_WORKSPACE_MSG:
			SetWorkspaces((Workspaces() == E_ALL_WORKSPACES ? 0 : Workspaces() + 1));
			break;

		case WIN_ALL_WORKSPACE_MSG:
			SetWorkspaces(E_ALL_WORKSPACES);
			break;

		case WIN_BORDERED_LOOK_MSG:
		case WIN_NO_BORDER_LOOK_MSG:
		case WIN_TITLED_LOOK_MSG:
		case WIN_DOCUMENT_LOOK_MSG:
		case WIN_MODAL_LOOK_MSG:
		case WIN_FLOATING_LOOK_MSG:
			SetLook(msg->what == WIN_BORDERED_LOOK_MSG ? E_BORDERED_WINDOW_LOOK : (
				msg->what == WIN_NO_BORDER_LOOK_MSG ? E_NO_BORDER_WINDOW_LOOK : (
				msg->what == WIN_TITLED_LOOK_MSG ? E_TITLED_WINDOW_LOOK : (
				msg->what == WIN_DOCUMENT_LOOK_MSG ? E_DOCUMENT_WINDOW_LOOK : (
				msg->what == WIN_MODAL_LOOK_MSG ? E_MODAL_WINDOW_LOOK : E_FLOATING_WINDOW_LOOK)))));
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


void
TWindow::WorkspacesChanged(euint32 old_ws, euint32 new_ws)
{
	ETK_OUTPUT("Window workspaces changed: old - %u, new - %u.\n", old_ws, new_ws);
}


void
TWindow::Minimize(bool minimize)
{
	EWindow::Minimize(minimize);
	ETK_OUTPUT("Window minimized %s.\n", minimize ? "true" : "false");
}


bool
TWindow::QuitRequested()
{
	etk_app->PostMessage(E_QUIT_REQUESTED);
	return true;
}


TApplication::TApplication()
	: EApplication("application/x-vnd.lee-example-app")
{
}


TApplication::~TApplication()
{
}


void
TApplication::ReadyToRun()
{
	TWindow *win = new TWindow(ERect(20, 20, 400, 500), "Window Example: WM Action", E_TITLED_WINDOW, 0);

	win->Lock();

	ERect btnRect(10, 10, win->Bounds().Width() - 10, 35);
	EButton *btn = new EButton(btnRect, NULL, "Minimize me",
				   new EMessage(WIN_MINIMIZE_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Inactivate me",
			  new EMessage(WIN_INACTIVATE_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 60);
	btn = new EButton(btnRect, NULL, "Send me to the next workspace",
			  new EMessage(WIN_NEXT_WORKSPACE_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Send me to the previous workspace",
			  new EMessage(WIN_PREV_WORKSPACE_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let me stay on all workspaces",
			  new EMessage(WIN_ALL_WORKSPACE_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 60);
	btn = new EButton(btnRect, NULL, "Let my look to be E_BORDERED_WINDOW_LOOK",
			  new EMessage(WIN_BORDERED_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let my look to be E_NO_BORDER_WINDOW_LOOK",
			  new EMessage(WIN_NO_BORDER_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let my look to be E_TITLED_WINDOW_LOOK",
			  new EMessage(WIN_TITLED_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let my look to be E_DOCUMENT_WINDOW_LOOK",
			  new EMessage(WIN_DOCUMENT_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let my look to be E_MODAL_WINDOW_LOOK",
			  new EMessage(WIN_MODAL_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	btnRect.OffsetBy(0, 30);
	btn = new EButton(btnRect, NULL, "Let my look to be E_FLOATING_WINDOW_LOOK",
			  new EMessage(WIN_FLOATING_LOOK_MSG), E_FOLLOW_LEFT_RIGHT);
	win->AddChild(btn);

	win->Show();

	win->Activate(false);

	win->Unlock();
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	return 0;
}


#if defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))
#include <windows.h>
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif // defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))

