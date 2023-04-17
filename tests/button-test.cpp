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
 * File: button-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/Button.h>

#include "./../kernel/Debug.h>

#define BTN_HELLO_WORLD_EN_MSG	'btn1'
#define BTN_HELLO_WORLD_CN_MSG	'btn2'
#define BTN_NOT_ENABLED_MSG	'btn3'
#define BTN_FOCUS_MSG		'btn4'

class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags);
	virtual ~TView();
};


TView::TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags)
	: EView(frame, name, resizingMode, flags)
{
	EFont font;

	EButton *btn = new EButton(ERect(10, 10, 150, 50), NULL, "Hello World", new EMessage(BTN_HELLO_WORLD_EN_MSG));
	btn->ForceFontAliasing(true);
	if(font.SetFamilyAndStyle("SimSun", "Regular") == E_OK) btn->SetFont(&font, E_FONT_FAMILY_AND_STYLE);
	btn->SetFontSize(20);
	AddChild(btn);

	btn = new EButton(ERect(10, 100, 50, 120), NULL, "哈啰，这个世界很美妙", new EMessage(BTN_HELLO_WORLD_CN_MSG));
	btn->ForceFontAliasing(true);
	if(font.SetFamilyAndStyle("SimHei", "Regular") == E_OK)
	{
		btn->SetFont(&font, E_FONT_FAMILY_AND_STYLE);
		btn->SetFontSize(24);
	}
	AddChild(btn);
	btn->ResizeToPreferred();

	btn = new EButton(ERect(10, 150, 40, 180), NULL, "失效按钮", new EMessage(BTN_NOT_ENABLED_MSG));
	btn->SetEnabled(false);
	AddChild(btn);
	btn->ResizeToPreferred();
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

	virtual void MessageReceived(EMessage *msg);

private:
	bool quited;
};


class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual void ReadyToRun();
};

TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace)
	: EWindow(frame, title, type, flags, workspace), quited(false)
{
//	SetBackgroundColor(0, 255, 255);

	EButton *btn = new EButton(ERect(10, 200, 40, 230), NULL, "Focus Button", new EMessage(BTN_FOCUS_MSG));
	AddChild(btn);
	btn->ResizeToPreferred();
	btn->MakeFocus(true);

	EView *view = new TView(frame.OffsetToCopy(E_ORIGIN), NULL, E_FOLLOW_ALL, E_WILL_DRAW | E_FRAME_EVENTS);
	AddChild(view);
}


TWindow::~TWindow()
{
}

void
TWindow::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case BTN_HELLO_WORLD_EN_MSG:
			ETK_OUTPUT("Hello world button is pressed.\n");
			break;

		case BTN_HELLO_WORLD_CN_MSG:
			ETK_OUTPUT("测试按钮 button is pressed.\n");
			break;

		case BTN_NOT_ENABLED_MSG:
			ETK_OUTPUT("Not enabled button is pressed, it must be something error!\n");
			break;

		case BTN_FOCUS_MSG:
			ETK_OUTPUT("Focus button is pressed.\n");
			SetFlags((Flags() & E_AVOID_FOCUS) ? Flags() & ~E_AVOID_FOCUS : Flags() | E_AVOID_FOCUS);
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
	TWindow *win = new TWindow(ERect(100, 100, 500, 500), "Button Test 按钮测试", E_TITLED_WINDOW, 0);
	win->Show();
}


int main(int argc, char **argv)
{
	TApplication app;
	app.Run();

	return 0;
}

