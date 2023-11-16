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
 * File: helloworld3.cpp
 *
 * --------------------------------------------------------------------------*/

#include <etkxx.h>

class TApplication : public EApplication {
public:
	TApplication();
};


class TWindow : public EWindow {
public:
	TWindow(ERect frame, const char* title, e_window_type type, euint32_t flag);
	virtual bool QuitRequested();
};


class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flag);
	virtual void Draw(ERect updateRect);
};


TApplication::TApplication()
	: EApplication("application/x-vnd.etkxx-helloworld3-app")
{
	TWindow *t_win = new TWindow(ERect(100, 100, 300, 150), "Hello World Sample 3", E_DOCUMENT_WINDOW, 0);

#if 0
	Lock();
	t_win->Lock();
	t_win->ProxyBy(this);
	t_win->Unlock();
	Unlock();
#endif

	t_win->Lock();
	t_win->Show();
	t_win->Unlock();
}


TWindow::TWindow(ERect frame, const char* title, e_window_type type, euint32_t flag)
	: EWindow(frame, title, type, flag)
{
	frame.OffsetTo(0, 0);

	AddChild(new TView(frame, "theView", E_FOLLOW_NONE, E_WILL_DRAW));
}


bool TWindow::QuitRequested()
{
	etk_app->PostMessage(E_QUIT_REQUESTED);
	return true;
}


TView::TView(ERect frame, const char* title, euint32_t resizingMode, euint32_t flags)
	: EView(frame, title, resizingMode, flags)
{
	SetViewColor(200, 200, 200);

	EFont font;
	GetFont(&font);
	font.SetSize(20);

	SetFont(&font, E_FONT_SIZE);
}


void TView::Draw(ERect updateRect)
{
	MovePenTo(20, 20);
	DrawString("Hello World.");
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

