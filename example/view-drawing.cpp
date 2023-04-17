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
 * File: view-drawing.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../app/Application.h>
#include "./../interface/Window.h>
#include "./../interface/View.h>
#include "./../add-ons/font/FontEngine.h>

#define TEST_VIEW_FOLLOW	0

#if TEST_VIEW_FOLLOW == 0
	#define TEST_POINT		1
	#define TEST_SQUARE_POINT	0
	#define TEST_LINE		1
	#define TEST_POLYGON		1
	#define TEST_RECT_AND_REGION	1
	#define TEST_ARC		1

	#define TEST_FONT		1
	#define TEST_FONT_STRING	"Jump over the dog, 跳过那只狗."
	#define TEST_FONT_FAMILY	"AR PL KaitiM GB"
	#define TEST_FONT_STYLE		"Regular"
	#define TEST_FONT_SIZE		24
#endif // TEST_VIEW_FOLLOW


class TView : public EView {
public:
	TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags);
	virtual ~TView();

	virtual void Draw(ERect updateRect);
};


class TWindow : public EWindow {
public:
	TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace = E_CURRENT_WORKSPACE);
	virtual ~TWindow();

	virtual bool QuitRequested();
};


class TApplication : public EApplication {
public:
	TApplication();
	virtual ~TApplication();

	virtual void ReadyToRun();
};


TView::TView(ERect frame, const char *name, euint32_t resizingMode, euint32_t flags)
	: EView(frame, name, resizingMode, flags)
{
}


TView::~TView()
{
}


void
TView::Draw(ERect updateRect)
{
	e_pattern pat = E_MIXED_COLORS;

#if TEST_POINT == 1 // Test Point
	{
		PushState();
		SetDrawingMode(E_OP_COPY);
		float point_test_x = 0;
		for(eint32_t i = 0; i < 26; i++)
		{
			SetHighColor(200, 50, 200);
			SetPenSize(i);
			StrokePoint(EPoint(point_test_x, (float)i / 2.f), i < 13 ? E_SOLID_HIGH : pat);

			SetHighColor(0, 0, 0);
			SetPenSize(0);
			StrokePoint(EPoint(point_test_x, (float)i / 2.f));

			point_test_x += (float)(2 * i + 1) / 2.f + 2.f;
		}

		EPoint pts[4] = { EPoint(20, 30), EPoint(40, 30), EPoint(60, 30), EPoint(80, 30) };
		euint8 alpha[4] = {255, 150, 100, 50};

#if TEST_SQUARE_POINT == 1
		SetSquarePointStyle(true);
#endif

		SetDrawingMode(E_OP_COPY);
		SetHighColor(255, 0, 0);
		SetPenSize(17);
		StrokePoints(pts, 4, NULL, pat);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokePoints(pts, 4);

		SetDrawingMode(E_OP_COPY);
		SetHighColor(255, 0, 0);
		SetPenSize(17);
		for(eint32_t i = 0; i < 4; i++) pts[i].x += 100;
		StrokePoints(pts, 4, alpha, pat);

		PopState();
	}
#endif // TEST_POINT
#if TEST_LINE == 1
	{
		PushState();

		SetDrawingMode(E_OP_COPY);

		SetHighColor(255, 0, 0);
		SetPenSize(7);
		EPoint pt(0, 50);
		MovePenTo(pt);
		for(eint32_t i = 0; i < 6; i++) StrokeLine(pt += EPoint(30, (i % 2 == 0 ? 30 : -30)), pat);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		pt.x = 0; pt.y = 50;
		MovePenTo(pt);
		for(eint32_t i = 0; i < 6; i++) StrokeLine(pt += EPoint(30, (i % 2 == 0 ? 30 : -30)));

		SetHighColor(255, 255, 0);
		SetPenSize(0);
		pt.x = 0; pt.y = 50;
		for(eint32_t i = -1; i < 6; i++) StrokePoint(i < 0 ? pt : pt += EPoint(30, (i % 2 == 0 ? 30 : -30)));

		PopState();
	}
#endif // TEST_LINE
#if TEST_POLYGON == 1
	{
		EPolygon poly;
		EPoint pt(220, 50);
		for(eint32_t i = -1; i < 6; i++)
		{
			if(i >= 0) pt += EPoint(30, (i % 2 == 0 ? 30 : -30));
			poly.AddPoints(&pt, 1);
		}

		PushState();

		SetDrawingMode(E_OP_COPY);

		SetHighColor(255, 0, 0);
		SetPenSize(9);
		StrokePolygon(&poly, true, pat);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokePolygon(&poly, true);

		SetHighColor(255, 255, 0);
		SetPenSize(0);
		const EPoint *polyPts = poly.Points();
		for(eint32_t i = 0; i < poly.CountPoints(); i++) StrokePoint(*polyPts++);

		EPolygon aPoly(poly.Points(), 3);
		SetHighColor(0, 255, 0);
		FillPolygon(&aPoly);

		PopState();
	}
#endif // TEST_POLYGON
#if TEST_RECT_AND_REGION == 1
	{
		PushState();

		SetDrawingMode(E_OP_COPY);

		SetHighColor(255, 0, 0);
		SetPenSize(9);
		StrokeRect(ERect(10, 100, 50, 120), pat);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokeRect(ERect(10, 100, 50, 120));

		e_pattern apat = {0xcc, 0x66, 0x33, 0x99, 0xcc, 0x66, 0x33, 0x99};
		SetHighColor(0, 0, 0);
		SetLowColor(255, 255, 255);
		FillRect(ERect(70, 100, 110, 120), apat);

		SetHighColor(0, 0, 0);
		ERect r(150, 100, 200, 150);
		ERect r1(150, 100, 160, 110);
		ERect r2(190, 100, 200, 110);
		ERect r3(150, 140, 160, 150);
		ERect r4(190, 140, 200, 150);
		StrokeRect(r);
		StrokeRect(r1);
		StrokeRect(r2);
		StrokeRect(r3);
		StrokeRect(r4);
		SetHighColor(255, 0, 0);
		SetDrawingMode(E_OP_XOR);
		FillRoundRect(r, 10, 10);
		SetDrawingMode(E_OP_COPY);
		StrokeRoundRect(r, 10, 10);

		ERect rects[3] = {ERect(20, 130, 70, 180), ERect(50, 160, 150, 210), ERect(85, 195, 170, 240)};
		ERegion region; for(eint8 i = 0; i < 3; i++) region.Include(rects[i]); region.OffsetBy(200, 0);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokeRects(rects, 3);
		SetDrawingMode(E_OP_XOR);
		SetHighColor(0, 0, 255);
		FillRects(rects, 3);

		SetHighColor(0, 255, 0);
		FillRegion(&region, apat);

		PopState();
	}
#endif // TEST_RECT_AND_REGION
#if TEST_ARC == 1
	{
		PushState();

		ERect r(10, 260, 60, 310);

		SetDrawingMode(E_OP_COPY);
		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokeRect(r);
		SetHighColor(0, 0, 255);
		StrokeArc(r, 0, 360);
		SetDrawingMode(E_OP_XOR);
		SetHighColor(255, 0, 0);
		FillArc(r, 30, 240);

		r.OffsetBy(70, 0);
		SetDrawingMode(E_OP_COPY);
		SetHighColor(255, 0, 0);
		SetPenSize(9);
		StrokeArc(r, 0, 360);
		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokeArc(r, 0, 360);

		r.OffsetBy(70, 0);
		SetHighColor(0, 0, 0);
		FillArc(r, 0, 360, pat);
		SetHighColor(255, 0, 0);
		SetPenSize(9);
		StrokeArc(r, 0, 360, pat);

		r.OffsetBy(70, 0);
		r.right += 50;
		SetHighColor(0, 0, 0);
		FillEllipse(r, pat);
		SetHighColor(255, 0, 0);
		SetPenSize(9);
		StrokeEllipse(r, pat);

		PopState();
	}
#endif // TEST_ARC
#if TEST_FONT == 1
	{
		PushState();

		EPoint pt(10, 350);

		SetDrawingMode(E_OP_COPY);
		SetHighColor(0, 0, 0);
		ForceFontAliasing(true);
		SetFontSize(TEST_FONT_SIZE);
		DrawString(TEST_FONT_STRING, pt);

		SetDrawingMode(E_OP_COPY);
		SetHighColor(0, 0, 0);
		SetLowColor(ViewColor());
		ForceFontAliasing(false);
		EFont font;
		GetFont(&font);
		font.SetFamilyAndStyle(TEST_FONT_FAMILY, TEST_FONT_STYLE);
		font.SetSize(TEST_FONT_SIZE);
		SetFont(&font);
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		float strHeight = fontHeight.ascent + fontHeight.descent;
		float strWidth = font.StringWidth(TEST_FONT_STRING);

		pt += EPoint(0, 20);

		SetHighColor(0, 0, 0);
		SetPenSize(0);
		StrokeRect(ERect(pt, pt + EPoint(strWidth, strHeight)));

		StrokeLine(pt + EPoint(0, fontHeight.leading),
			   pt + EPoint(strWidth, fontHeight.leading));
		StrokeLine(pt + EPoint(0, fontHeight.ascent),
			   pt + EPoint(strWidth, fontHeight.ascent));

		DrawString(TEST_FONT_STRING, pt + EPoint(0, fontHeight.ascent + 1));

		PopState();
	}
#endif // TEST_FONT
}


TWindow::TWindow(ERect frame, const char *title, e_window_type type, euint32_t flags, euint32_t workspace)
	: EWindow(frame, title, type, flags, workspace)
{
	EView *view_top = new TView(frame.OffsetToCopy(E_ORIGIN), NULL, E_FOLLOW_ALL, E_WILL_DRAW);
	AddChild(view_top);

#if TEST_VIEW_FOLLOW == 1
	EView *view = new EView(ERect(50, 50, 100, 100), NULL, E_FOLLOW_NONE, 0);
	view->SetViewColor(255, 0, 0);
	view_top->AddChild(view);

	view = new EView(ERect(50, 150, 100, 200), NULL, E_FOLLOW_RIGHT, 0);
	view->SetViewColor(0, 255, 0);
	view_top->AddChild(view);

	view = new EView(ERect(150, 250, 200, 300), NULL, E_FOLLOW_H_CENTER | E_FOLLOW_RIGHT, 0);
	view->SetViewColor(0, 0, 155);
	view_top->AddChild(view);
#endif // TEST_VIEW_FOLLOW
}


TWindow::~TWindow()
{
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
	TWindow *win = new TWindow(ERect(100, 100, 550, 500), "View Example: Drawing", E_TITLED_WINDOW, 0);
	win->Show();

#if TEST_FONT == 1
	for(eint32_t i = 0; i < etk_count_font_families(); i++)
	{
		const char *family = NULL;
		if(!(etk_get_font_family(i, &family) != E_OK || family == NULL))
		{
			ETK_OUTPUT("Font[%d]:(%s)", i, family);
			for(eint32_t j = 0; j < etk_count_font_styles(i); j++)
			{
				const char *style = NULL;
				if(etk_get_font_style(family, j, &style) == E_OK)
				{
					EFontEngine *engine = etk_get_font_engine(i, j);
					if(engine)
					{
						ETK_OUTPUT(" (%s[%s]", style, engine->IsScalable() ? "Scalable" : "Not Scalable");
						eint32_t fixedCount = 0;
						if(engine->HasFixedSize(&fixedCount))
						{
							ETK_OUTPUT(":");
							float size = 0;
							for(eint32_t k = 0; k < fixedCount; k++)
							{
								engine->GetFixedSize(&size, k);
								ETK_OUTPUT("%s%g", (k == 0 ? "" : "/"), size);
							}
						}
						ETK_OUTPUT(")");
					}
				}
			}
			ETK_OUTPUT("\n");
		}
	}

	if(etk_plain_font) etk_plain_font->PrintToStream();
	if(etk_bold_font) etk_bold_font->PrintToStream();
	if(etk_fixed_font) etk_fixed_font->PrintToStream();
#endif // TEST_FONT
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

