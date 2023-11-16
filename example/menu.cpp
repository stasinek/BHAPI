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
 * File: menu.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/PopUpMenu.h>
#include "./../interface/MenuBar.h>

#define MENU_TEST_OPEN_MSG	'mtop'
#define MENU_TEST_QUIT_MSG	'mtqi'
#define MENU_TEST_MARK_MSG	'mtma'
#define MENU_TEST_SUBM_MSG	'mtsu'


class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags);
	virtual ~TView();

	virtual void	AllAttached();
	virtual void	MouseDown(EPoint where);

private:
	EMenu *fMenu;
	EPopUpMenu *fPopUp;
};


class TWindow : public EWindow {
public:
	TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace = E_CURRENT_WORKSPACE);
	virtual ~TWindow();

	virtual bool QuitRequested();
	virtual void MessageReceived(EMessage *msg);
};


class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual void ReadyToRun();
};


inline void setMenuTarget(EMenu *menu, EMessenger msgr)
{
	if(!menu) return;

	EMenuItem *item;
	for(eint32_t i = 0; (item = menu->ItemAt(i)) != NULL; i++)
	{
		if(item->Submenu() != NULL) setMenuTarget(item->Submenu(), msgr);
		else item->SetTarget(msgr);
	}
}


TView::TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags)
	: EView(frame, name, resizingMode, flags), fMenu(NULL), fPopUp(NULL)
{
	fMenu = new EMenu("Test Menu", E_ITEMS_IN_COLUMN);
//	fMenu->SetEventMask(E_KEYBOARD_EVENTS);

	fMenu->AddItem(new EMenuItem("Open...", new EMessage(MENU_TEST_OPEN_MSG)));
	fMenu->AddSeparatorItem();
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new EMenuItem("Quit", new EMessage(MENU_TEST_QUIT_MSG), 'q', E_COMMAND_KEY));
	fMenu->AddItem(new EMenuItem("Just A Test", NULL), 2);

	EMenuItem *item = new EMenuItem("Disabled menuitem", NULL);
	item->SetEnabled(false);
	fMenu->AddItem(item, 3);

	item = new EMenuItem("标志菜单", new EMessage(MENU_TEST_MARK_MSG));
	item->SetMarked(true);
	fMenu->AddItem(item, 3);

	item = new EMenuItem("Shortcut Test", NULL, E_F12_KEY, E_FUNCTIONS_KEY|E_CONTROL_KEY);
	fMenu->AddItem(item, 3);

	fMenu->AddItem(new EMenuItem("Shortcut Test", NULL, 'e', E_SHIFT_KEY|E_CONTROL_KEY));
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new EMenuItem("Shortcut Test", NULL, E_HOME, E_SHIFT_KEY|E_CONTROL_KEY|E_COMMAND_KEY));

	EMenu *submenu1 = new EMenu("Sub Menu 1", E_ITEMS_IN_COLUMN);
	submenu1->AddItem(new EMenuItem("Test 1", NULL));
	submenu1->AddSeparatorItem();
	submenu1->AddItem(new EMenuItem("Test 2", NULL));

	EMenu *submenu2 = new EMenu("Sub Menu 2", E_ITEMS_IN_COLUMN);
	submenu2->AddItem(new EMenuItem("Test Message", new EMessage(MENU_TEST_SUBM_MSG)));
	submenu2->AddSeparatorItem();
	submenu2->AddItem(new EMenuItem("Test", NULL));

	submenu1->AddItem(submenu2);

	fMenu->AddItem(submenu1, 3);

	fMenu->ResizeToPreferred();
	fMenu->MoveTo(50, 50);

	AddChild(fMenu);

	fPopUp = new EPopUpMenu("Test PopUp Menu");

	fPopUp->AddItem(new EMenuItem("Open...", new EMessage(MENU_TEST_OPEN_MSG)));
	fPopUp->AddSeparatorItem();
	fPopUp->AddSeparatorItem();
	fPopUp->AddItem(new EMenuItem("Quit", new EMessage(MENU_TEST_QUIT_MSG), 'q', E_COMMAND_KEY));
	fPopUp->AddItem(new EMenuItem("Just A Test", NULL), 2);

	EMenu *submenu3 = new EMenu("Sub Menu 1", E_ITEMS_IN_COLUMN);
	submenu3->AddItem(new EMenuItem("Test 1", NULL));
	submenu3->AddSeparatorItem();
	submenu3->AddItem(new EMenuItem("Test 2", NULL));
	fPopUp->AddItem(submenu3, 3);

	EMenu *submenu4 = new EMenu("Sub Menu 2", E_ITEMS_IN_COLUMN);
	submenu4->AddItem(new EMenuItem("Test Message", new EMessage(MENU_TEST_SUBM_MSG)));
	submenu4->AddSeparatorItem();
	submenu4->AddItem(new EMenuItem("Test", NULL));
	fPopUp->AddItem(submenu4, 3);
}


void
TView::AllAttached()
{
	setMenuTarget(fMenu, EMessenger(Window()));
	setMenuTarget(fPopUp, EMessenger(Window()));
}


TView::~TView()
{
	if(fPopUp) delete fPopUp;
}


void
TView::MouseDown(EPoint where)
{
	if(fPopUp && (!fMenu || fMenu->Frame().Contains(where) == false))
	{
		ConvertToScreen(&where);
		EMenuItem *item = fPopUp->Go(where, false, false, false, true);
		if(item)
		{
			EMessage *msg = item->Message();
			if(msg) msg->PrintToStream();
			item->Invoke();
		}
		else
		{
			ETK_OUTPUT("None selected.\n");
		}
	}
}


TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace)
	: EWindow(frame, title, type, flags, workspace)
{
	ERect rect = frame;
	rect.bottom = rect.top + 20;
	rect.OffsetTo(E_ORIGIN);

	EMenuBar *menubar = new EMenuBar(rect, "Test Menu Bar");

	EMenu *menu = new EMenu("File", E_ITEMS_IN_COLUMN);
	menu->AddItem(new EMenuItem("Open...", new EMessage(MENU_TEST_OPEN_MSG)));
	menu->AddSeparatorItem();
	menu->AddSeparatorItem();
	menu->AddItem(new EMenuItem("Quit", new EMessage(MENU_TEST_QUIT_MSG), 'q', E_COMMAND_KEY));

	EMenuItem *item = new EMenuItem("Disabled menuitem", NULL);
	item->SetEnabled(false);
	menu->AddItem(item, 2);

	item = new EMenuItem("标志菜单", new EMessage(MENU_TEST_MARK_MSG));
	item->SetMarked(true);
	menu->AddItem(item, 2);

	menubar->AddItem(menu);

	menu = new EMenu("Edit", E_ITEMS_IN_COLUMN);
	menu->AddItem(new EMenuItem("Copy", NULL));
	menu->AddItem(new EMenuItem("Cut", NULL));
	menu->AddItem(new EMenuItem("Paste", NULL));
	menubar->AddItem(menu);

	menubar->AddItem(new EMenuItem("Help", NULL));

	EMenu *submenu1 = new EMenu("Test", E_ITEMS_IN_COLUMN);
	submenu1->AddItem(new EMenuItem("Test 1", NULL));
	submenu1->AddSeparatorItem();
	submenu1->AddItem(new EMenuItem("Test 2", NULL));

	EMenu *submenu2 = new EMenu("Sub Menu 2", E_ITEMS_IN_COLUMN);
	submenu2->AddItem(new EMenuItem("Test Message", new EMessage(MENU_TEST_SUBM_MSG)));
	submenu2->AddSeparatorItem();
	submenu2->AddItem(new EMenuItem("Test", NULL));

	submenu1->AddItem(submenu2);

	menubar->AddItem(submenu1, 2);

	AddChild(menubar);
	setMenuTarget(menubar, EMessenger(this));

	rect = frame;
	rect.OffsetTo(E_ORIGIN);
	rect.top += 20;
	EView *menu_view = new TView(rect, NULL, E_FOLLOW_ALL, 0);
//	menu_view->SetViewColor(233, 233, 200);
	AddChild(menu_view);
}


TWindow::~TWindow()
{
}


void
TWindow::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case MENU_TEST_OPEN_MSG:
			ETK_OUTPUT("Open menu selected.\n");
			break;

		case MENU_TEST_QUIT_MSG:
			ETK_OUTPUT("Quit menu selected.\n");
			etk_app->PostMessage(E_QUIT_REQUESTED);
			break;

		case MENU_TEST_SUBM_MSG:
			ETK_OUTPUT("Sub menu selected.\n");
			break;

		case MENU_TEST_MARK_MSG:
			{
				EMenuItem *item = NULL;
				if(msg->FindPointer("source", (void**)&item) == false || item == NULL) break;
				item->SetMarked(item->IsMarked() ? false : true);
			}
			break;

		default:
			EWindow::MessageReceived(msg);
	}
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
	TWindow *win = new TWindow(ERect(100, 100, 500, 500), "Menu Example", E_TITLED_WINDOW, 0);
	win->Show();
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

