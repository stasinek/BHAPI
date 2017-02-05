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
 * File: Render-SDL.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>
#include <SDL.h>
#include <etkxx.h>

#define VIDEO_XRES			640
#define VIDEO_YRES			480
#define VIDEO_DEPTH			32

#define REFRESH_INTERVAL		5000

// the user EMessage::what shouldn't contains any upper character or "_"
#define START_REFRESH_MSG		'ssrm'
#define STOP_REFRESH_MSG		'strm'

#define COUNT_MSG			'cout'

#define BTN_FILL_RECTS_MSG		'btn1'
#define BTN_STROKE_POINTS_MSG		'btn2'
#define BTN_STROKE_LINES_MSG		'btn3'
#define BTN_STROKE_ELLIPSES_MSG		'btn4'
#define BTN_FILL_ELLIPSES_MSG		'btn5'
#define BTN_STROKE_ARCS_MSG		'btn6'
#define BTN_FILL_TRIANGLES_MSG		'btn7'
#define BTN_STROKE_POLYGONS_MSG		'btn8'
#define BTN_FILL_POLYGONS_MSG		'btn9'

#ifdef ETK_OS_WIN32
#define random()			rand()
#endif // ETK_OS_WIN32

#ifdef ETK_THREAD_IMPL_WIN32
	// in order to wake up other thread, like GUI or refreshing-cycle of SDL
	#define SNOOZE()		(void)(e_snooze(0))
#else
	#define SNOOZE()		(void)0
#endif

#ifdef ETK_OS_BEOS

// SDL use BApplication on BeOS, the built-in graphics-engine of ETK++ also use BApplication.
// So we spawn the SDL task before ETK++ initalized in order to use BWindow task in ETK++,
// but the application still be blocked when quit, thus should be fixed yet...
// 
// If you use another graphics-engine (X11,etc.) of ETK++, comment the line below.
#define RUN_SDL_TASK_FIRST

#endif // ETK_OS_BEOS


EMessenger msgrDrawing;


class TRender : public ERender {
public:
	TRender(ELooper *looper);

	void StartFresh() {etk_app->PostMessage(START_REFRESH_MSG);}
	void StopFresh() {etk_app->PostMessage(STOP_REFRESH_MSG);}

	void testFillRects(eint32 count);
	void testStrokePoints();
	void testStrokeLines(eint32 count);
	void testStrokeEllipses(eint32 count);
	void testFillEllipses(eint32 count);
	void testStrokeArcs(eint32 count);
	void testFillTriangles(eint32 count);
	void testStrokePolygons(eint32 count);
	void testFillPolygons(eint32 count);

private:
	e_status_t fStatus;
	SDL_Surface *fScreen;
	ELooper *fLooper;

	virtual e_status_t InitCheck() const;
	virtual void GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const;
	virtual void GetPixel(eint32 x, eint32 y, e_rgb_color &color) const;
	virtual void PutPixel(eint32 x, eint32 y, e_rgb_color color);
	virtual void PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color);
};


class TDrawingLooper : public ELooper {
public:
	TDrawingLooper();
	virtual ~TDrawingLooper();

	virtual void MessageReceived(EMessage *msg);

private:
	TRender *fRender;
	eint32 fCount;
};


class TWindow : public EWindow {
public:
	TWindow();

	virtual void MessageReceived(EMessage *msg);
};


class TApplication : public EApplication {
public:
#ifndef RUN_SDL_TASK_FIRST
	TApplication();
#else
	TApplication(void *thread);
#endif
	virtual ~TApplication();

	virtual void ReadyToRun();
	virtual bool QuitRequested();

	virtual void Pulse();
	virtual void MessageReceived(EMessage *msg);

private:
	friend class TRender;
	void *fSDLThread;
};


TRender::TRender(ELooper *looper)
	: ERender(), fScreen(NULL), fLooper(NULL), fStatus(E_NO_INIT)
{
	while((fScreen = SDL_GetVideoSurface()) == NULL)
	{
		if(etk_get_thread_run_state(((TApplication*)etk_app)->fSDLThread) != ETK_THREAD_RUNNING)
		{
			etk_app->PostMessage(E_QUIT_REQUESTED);
			return;
		}
		e_snooze(1000);
	}

	fLooper = looper;
	fStatus = ((fScreen == NULL || fLooper == NULL) ? E_NO_INIT : E_OK);
}


e_status_t
TRender::InitCheck() const
{
	if(fStatus != E_OK) return fStatus;
	if(fScreen == NULL) return E_ERROR;
	return E_OK;
}


void
TRender::GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const
{
	if(originX) *originX = 0;
	if(originY) *originY = 0;
	if(width) *width = VIDEO_XRES;
	if(height) *height = VIDEO_YRES;
}


void
TRender::GetPixel(eint32 x, eint32 y, e_rgb_color &color) const
{
	if(fScreen == NULL || SDL_LockSurface(fScreen) < 0) return;

	Uint32 sdlColor = 0;
	Uint8 *p = (Uint8*)fScreen->pixels + y * fScreen->pitch + x * fScreen->format->BytesPerPixel;

	switch(fScreen->format->BytesPerPixel)
	{
		case 1: // 8-bpp
			sdlColor = *p;
			break;

		case 2: // 15-bpp or 16-bpp
			sdlColor = *((Uint16*)p);
			break;

		case 3: // 24-bpp
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				sdlColor = (p[0] << 16) | (p[1] << 8) | p[2];
			else
				sdlColor = p[0] | (p[1] << 8) | (p[2] << 16);
			break;

		case 4: // 32-bpp
			sdlColor = *((Uint32*)p);
			break;

		default: break;
	}

	SDL_GetRGB(sdlColor, fScreen->format, &(color.red), &(color.green), &(color.blue));

	SDL_UnlockSurface(fScreen);
}


void
TRender::PutPixel(eint32 x, eint32 y, e_rgb_color color)
{
	if(fScreen == NULL || SDL_LockSurface(fScreen) < 0) return;

	Uint32 sdlColor = SDL_MapRGB(fScreen->format, color.red, color.green, color.blue);
	Uint8 *p = (Uint8*)fScreen->pixels + y * fScreen->pitch + x * fScreen->format->BytesPerPixel;

	switch(fScreen->format->BytesPerPixel)
	{
		case 1: // 8-bpp
			*p = sdlColor;
			break;

		case 2: // 15-bpp or 16-bpp
			*((Uint16*)p) = sdlColor;
			break;

		case 3: // 24-bpp
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (sdlColor >> 16) & 0xff;
				p[1] = (sdlColor >> 8) & 0xff;
				p[2] = sdlColor & 0xff;
			}
			else
			{
				p[0] = sdlColor & 0xff;
				p[1] = (sdlColor >> 8) & 0xff;
				p[2] = (sdlColor >> 16) & 0xff;
			}
			break;

		case 4: // 32-bpp
			*((Uint32*)p) = sdlColor;
			break;

		default: break;
	}

	SDL_UnlockSurface(fScreen);
}


void
TRender::PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color)
{
	if(fScreen == NULL || width == 0 || height == 0 ||
	   x > E_MAXINT16 || y > E_MAXINT16 || width >= E_MAXUINT16 || height >= E_MAXUINT16 ||
	   SDL_LockSurface(fScreen) < 0) return;
	
	Uint32 sdlColor = SDL_MapRGB(fScreen->format, color.red, color.green, color.blue);

	SDL_UnlockSurface(fScreen);

	SDL_Rect sdlRect;

	sdlRect.x = (Sint16)x;
	sdlRect.y = (Sint16)y;
	sdlRect.w = (Uint16)width;
	sdlRect.h = (Uint16)height;

	SDL_FillRect(fScreen, &sdlRect, sdlColor);
}


void
TRender::testFillRects(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = random() % VIDEO_XRES;
		eint32 y = random() % VIDEO_YRES;
		SetHighColor(x & 0xff, y & 0xff, (x * y) & 0xff, x & 0xff);
		FillRect(x, y, VIDEO_XRES - min_c(2 * x, VIDEO_XRES), VIDEO_YRES - min_c(2 * y, VIDEO_YRES));

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testStrokePoints()
{
	euint8 alpha = 128;
	SetDrawingMode(alpha < 255 ? E_OP_ALPHA : E_OP_COPY);

	StartFresh();

	for(eint32 y = 0; y < VIDEO_YRES; y++)
	{
		for(eint32 x = 0; x < 128; x++)
		{
			SetHighColor(x << 1, 0, 0, alpha);
			StrokePoint(x, y);
			SetHighColor(0, x << 1, 0, alpha);
			StrokePoint(128 + x, y);
			if(128 * 2 + x >= VIDEO_XRES) continue;
			SetHighColor(0, 0, x << 1, alpha);
			StrokePoint(128 * 2 + x, y);
			if(128 * 3 + x >= VIDEO_XRES) continue;
			SetHighColor(x << 1, x << 1, 0, alpha);
			StrokePoint(128 * 3 + x, y);
			if(128 * 4 + x >= VIDEO_XRES) continue;
			SetHighColor(x << 1, 0, x << 1, alpha);
			StrokePoint(128 * 4 + x, y);
			if(128 * 5 + x >= VIDEO_XRES) continue;
			SetHighColor(0, x << 1, x << 1, alpha);
			StrokePoint(128 * 5 + x, y);
			if(128 * 6 + x >= VIDEO_XRES) continue;
			SetHighColor(x << 1, x << 1, x << 1, alpha);
			StrokePoint(128 * 6 + x, y);
		}

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testStrokeLines(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = random() % VIDEO_XRES;
		eint32 y = random() % VIDEO_YRES;
		SetHighColor(x & 0xff, y & 0xff, (x * y) & 0xff, x & 0xff);
		StrokeLine(x, y, VIDEO_XRES - x - 1, VIDEO_YRES - y - 1);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testStrokeEllipses(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = random() % VIDEO_XRES;
		eint32 y = random() % VIDEO_YRES;
		eint32 xr = x % 120;
		eint32 yr = y % 120;
		SetHighColor(x & 0xff, y & 0xff, (x * y) & 0xff, x & 0xff);
		StrokeEllipse(x, y, xr, yr);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testFillEllipses(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = random() % VIDEO_XRES;
		eint32 y = random() % VIDEO_YRES;
		eint32 xr = x % 120;
		eint32 yr = y % 120;
		SetHighColor(x & 0xff, y & 0xff, (x * y) & 0xff, x & 0xff);
		FillEllipse(x, y, xr, yr);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testStrokeArcs(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = random() % VIDEO_XRES;
		eint32 y = random() % VIDEO_YRES;
		eint32 xr = x % 120;
		eint32 yr = y % 120;
		SetHighColor(x & 0xff, y & 0xff, (x * y) & 0xff, x & 0xff);
		StrokeArc(ERect(x, y, x + xr, y + yr), (x + y) % 360, (x - y) % 360);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testFillTriangles(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x0 = random() % VIDEO_XRES;
		eint32 y0 = random() % VIDEO_YRES;

		eint32 x1 = random() % VIDEO_XRES;
		eint32 y1 = random() % VIDEO_YRES;

		eint32 x2 = random() % VIDEO_XRES;
		eint32 y2 = random() % VIDEO_YRES;

		SetHighColor(x0 & 0xff, y0 & 0xff, x1 & 0xff, y1 & 0xff);
		FillTriangle(x0, y0, x1, y1, x2, y2);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testStrokePolygons(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 numPts = max_c(random() % 15, 3);

		EPolygon aPolygon;
		EPoint pt;

		for(eint32 k = 0; k < numPts; k++)
		{
			eint32 x = random() % VIDEO_XRES;
			eint32 y = random() % VIDEO_YRES;
			pt.Set((float)x + 0.5f, (float)y + 0.5f);
			aPolygon.AddPoints(&pt, 1);
		}

		SetHighColor((eint32)pt.x & 0xff, (eint32)pt.y & 0xff, numPts << 5, (eint32)pt.y & 0xff);
		StrokePolygon(&aPolygon);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


void
TRender::testFillPolygons(eint32 count)
{
#if 1
	SetDrawingMode(E_OP_COPY);
	SetHighColor(0, 0, 0);
	FillRect(0, 0, VIDEO_XRES, VIDEO_YRES);
#else
	SetDrawingMode(E_OP_ALPHA);
#endif

	StartFresh();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 numPts = max_c(random() % 15, 3);

		EPolygon aPolygon;
		EPoint pt;

		for(eint32 k = 0; k < numPts; k++)
		{
			eint32 x = random() % VIDEO_XRES;
			eint32 y = random() % VIDEO_YRES;
			pt.Set((float)x + 0.5f, (float)y + 0.5f);
			aPolygon.AddPoints(&pt, 1);
		}

		SetHighColor((eint32)pt.x & 0xff, (eint32)pt.y & 0xff, numPts << 5, (eint32)pt.y & 0xff);
		FillPolygon(&aPolygon);

		SNOOZE(); // for user could interrupt this

		if(!fLooper) continue;
		if(fLooper->MessageQueue()->IsEmpty() == false) break;
	}

	StopFresh();
}


TDrawingLooper::TDrawingLooper()
	: ELooper(), fCount(2500)
{
	fRender = new TRender(this);
}


TDrawingLooper::~TDrawingLooper()
{
	if(fRender) delete fRender;
}


void
TDrawingLooper::MessageReceived(EMessage *msg)
{
	if(!fRender) return;

	if(msg->what == COUNT_MSG)
	{
		msg->FindInt32("count", &fCount);
		return;
	}

	if(fCount <= 0) return;
	if(msg->what == BTN_STROKE_POINTS_MSG) fRender->testStrokePoints();
	else if(msg->what == BTN_STROKE_LINES_MSG) fRender->testStrokeLines(fCount);
	else if(msg->what == BTN_STROKE_ELLIPSES_MSG) fRender->testStrokeEllipses(fCount);
	else if(msg->what == BTN_FILL_ELLIPSES_MSG) fRender->testFillEllipses(fCount);
	else if(msg->what == BTN_STROKE_ARCS_MSG) fRender->testStrokeArcs(fCount);
	else if(msg->what == BTN_FILL_RECTS_MSG) fRender->testFillRects(fCount);
	else if(msg->what == BTN_FILL_TRIANGLES_MSG) fRender->testFillTriangles(fCount);
	else if(msg->what == BTN_STROKE_POLYGONS_MSG) fRender->testStrokePolygons(fCount);
	else if(msg->what == BTN_FILL_POLYGONS_MSG) fRender->testFillPolygons(fCount);
}


TWindow::TWindow()
	: EWindow(ERect(300, 100, 600, 600), "ERender Sample", E_TITLED_WINDOW, E_QUIT_ON_WINDOW_CLOSE)
{
	ETextControl *tctrl = new ETextControl(ERect(50, 5, 250, 25), NULL, "Count: ", "2500", NULL);
	tctrl->SetModificationMessage(new EMessage(COUNT_MSG));
	tctrl->ResizeToPreferred();
	AddChild(tctrl);

	ERect rect(50, 40, 250, 70);

	EButton *btn = new EButton(rect, NULL, "Fill Rectangles", new EMessage(BTN_FILL_RECTS_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Stroke Points", new EMessage(BTN_STROKE_POINTS_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Stroke Lines", new EMessage(BTN_STROKE_LINES_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Stroke Ellipses", new EMessage(BTN_STROKE_ELLIPSES_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Fill Ellipses", new EMessage(BTN_FILL_ELLIPSES_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Stroke Arcs", new EMessage(BTN_STROKE_ARCS_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Fill Triangles", new EMessage(BTN_FILL_TRIANGLES_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Stroke Polygons", new EMessage(BTN_STROKE_POLYGONS_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);

	rect.OffsetBy(0, 50);
	btn = new EButton(rect, NULL, "Fill Polygons", new EMessage(BTN_FILL_POLYGONS_MSG));
	btn->SetTarget(msgrDrawing);
	AddChild(btn);
}


void
TWindow::MessageReceived(EMessage *msg)
{
	if(msg->what == COUNT_MSG)
	{
		ETextControl *tctrl = NULL;
		msg->FindPointer("source", (void**)&tctrl);

		if(tctrl == NULL) return;

		EString str(tctrl->Text());
		eint32 count;
		if(str.GetInteger(&count) == false) return;

		msg->AddInt32("count", count);
		msgrDrawing.SendMessage(msg);
	}

	EWindow::MessageReceived(msg);
}


static e_status_t sdl_task(void *arg)
{
	SDL_Surface *screen;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ETK_WARNING("%s --- Couldn't initialize SDL: %s", __PRETTY_FUNCTION__, SDL_GetError());
		etk_app->PostMessage(E_QUIT_REQUESTED);
		return E_ERROR;
	}

	if((screen = SDL_SetVideoMode(VIDEO_XRES, VIDEO_YRES, VIDEO_DEPTH, SDL_SWSURFACE | SDL_ASYNCBLIT | SDL_ANYFORMAT)) == NULL)
	{
		ETK_WARNING("%s --- Couldn't set video mode: %s", __PRETTY_FUNCTION__, SDL_GetError());
		etk_app->PostMessage(E_QUIT_REQUESTED);
		return E_ERROR;
	}

	SDL_WM_SetCaption("SDL area", NULL);

	SDL_Event event;
	do {
		SDL_WaitEvent(&event);
		if(event.type == SDL_VIDEOEXPOSE) SDL_UpdateRect(screen, 0, 0, 0, 0);
	} while(event.type != SDL_QUIT);

	etk_app->PostMessage(E_QUIT_REQUESTED);

	return E_OK;
}


#ifndef RUN_SDL_TASK_FIRST
TApplication::TApplication()
	: EApplication("application/x-vnd.etkxx-render_sample-app")
{
	fSDLThread = etk_create_thread(sdl_task, E_NORMAL_PRIORITY, NULL, NULL);
	if(fSDLThread == NULL || etk_resume_thread(fSDLThread) != E_OK)
	{
		if(fSDLThread != NULL) etk_delete_thread(fSDLThread);
		fSDLThread = NULL;
		ETK_WARNING("%s --- Couldn't create thread for spawning \"sdl_task\"!", __PRETTY_FUNCTION__);
	}
}
#else
TApplication::TApplication(void *thread)
	: EApplication("application/x-vnd.etkxx-render_sample-app")
{
	fSDLThread = thread;
}
#endif


TApplication::~TApplication()
{
	if(fSDLThread != NULL)
	{
		// post the event in order to quit the "sdl_task"
		SDL_Event sdlEvent;
		sdlEvent.type = SDL_QUIT;
		SDL_PushEvent(&sdlEvent);

		e_status_t status;
		etk_wait_for_thread(fSDLThread, &status);
		etk_delete_thread(fSDLThread);

		SDL_Quit();
	}
}


void
TApplication::ReadyToRun()
{
	if(fSDLThread != NULL)
	{
		TDrawingLooper *drawingLooper = new TDrawingLooper();
		msgrDrawing = EMessenger(drawingLooper);
		drawingLooper->Run();

		TWindow *win = new TWindow();

#if 0
		// let "TApplication" handle the events of "TWindow"
		Lock();
		win->Lock();
		win->ProxyBy(this);
		win->Unlock();
		Unlock();
#endif

		// "Show()" isn't MT-Safe, so we call Lock/Unlock
		win->Lock();
		win->Show();
		win->Unlock();
	}
	else
	{
		Quit();
	}
}


bool
TApplication::QuitRequested()
{
	while(msgrDrawing.SendMessage(E_QUIT_REQUESTED) == E_OK) e_snooze(100000);
	return true;
}


void
TApplication::Pulse()
{
	SDL_Event sdlEvent;
	sdlEvent.type = SDL_VIDEOEXPOSE;
	SDL_PushEvent(&sdlEvent);
}


void
TApplication::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case START_REFRESH_MSG:
		case STOP_REFRESH_MSG:
			SetPulseRate(msg->what == START_REFRESH_MSG ? REFRESH_INTERVAL : 0);
			if(msg->what == STOP_REFRESH_MSG) Pulse();
			break;

		default:
			EApplication::MessageReceived(msg);
	}
}


int main(int argc, char **argv)
{
#ifndef RUN_SDL_TASK_FIRST
	TApplication *myapp = new TApplication();
#else
	void *thread = etk_create_thread(sdl_task, E_NORMAL_PRIORITY, NULL, NULL);
	if(thread == NULL || etk_resume_thread(thread) != E_OK)
	{
		if(thread != NULL) etk_delete_thread(thread);
		ETK_ERROR("%s --- Couldn't create thread for spawning \"sdl_task\"!", __PRETTY_FUNCTION__);
	}
	e_snooze(500000);
	TApplication *myapp = new TApplication(thread);
#endif

	myapp->Run();
	delete myapp;

	return 0;
}


#if defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))
#include <windows.h>
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif // defined(_WIN32) && !(defined(_MSC_VER) && defined(_DEBUG))

