/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
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
 * File: DefaultTheme.cpp
 *
 * --------------------------------------------------------------------------*/

#include "ThemeEngine.h"
#include "../../interface/Rect.h"
#include "../../interface/View.h"
#include "../../interface/Region.h"
#include "../../interface/Point.h"

#define MIX_SHADOW_DARKER	80
#define MIX_SHADOW_SHINER	20
#define MIX_BUTTON_PUSHED	5

static void get_border_margins(struct b_theme_engine *engine,
			       const BView *view, float *left, float *top, float *right, float *bottom,
			       b_border_style border, float border_width);
static void draw_border(struct b_theme_engine *engine,
			BView *view, BRect frame,
			b_border_style border, float border_width);

static void get_scrollbar_preferred_size(struct b_theme_engine *engine,
					 const BView *view, float *width, float *height,
					 b_orientation direction);
static void get_scrollbar_respondent_region(struct b_theme_engine *engine,
					    const BView *view, BRect frame,
					    b_orientation direction, float minValue, float maxValue, float curValue, float *ratio,
					    BRegion *drag, BRegion *smallUp, BRegion *smallDown, BRegion *largeUp, BRegion *largeDown);
static void draw_scrollbar(struct b_theme_engine *engine,
			   BView *view, BRect frame,
			   b_orientation direction, float minValue, float maxValue, float curValue,
			   bool mouse_down, BPoint mouse_pos);

static void get_button_preferred_size(struct b_theme_engine *engine,
				      const BView *view, float *width, float *height,
				      const char *button_label);
static b_uint8 should_button_do_focus_flash(struct b_theme_engine *engine, const BView *view);
static void get_button_border_margins(struct b_theme_engine *engine,
				      const BView *view, float *left, float *top, float *right, float *bottom);
static void draw_button_border(struct b_theme_engine *engine,
			       BView *view, BRect frame,
			       bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash);
static void clear_button_content(struct b_theme_engine *engine,
				 BView *view, BRect frame,
				 bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash);
static void draw_button_label(struct b_theme_engine *engine,
			      BView *view, BRect frame,
			      const char *button_label,
			      bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash);
static void draw_button(struct b_theme_engine *engine,
			BView *view, BRect frame,
			const char *button_label,
			bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash);

b_theme_engine bhapi_default_theme_engine = {
	NULL,					// custom data

	get_border_margins,			// get_border_margins
	draw_border,				// draw_border

	get_scrollbar_preferred_size,		// get_scrollbar_preferred_size
	get_scrollbar_respondent_region,	// get_scrollbar_respondent_region
	draw_scrollbar,				// draw_scrollbar

	get_button_preferred_size,		// get_button_preferred_size
	should_button_do_focus_flash,		// should_button_do_focus_flash
	get_button_border_margins,		// get_button_border_margins
	draw_button_border,			// draw_button_border
	clear_button_content,			// clear_button_content
	draw_button_label,			// draw_button_label
	draw_button,				// draw_button

	NULL,					// init
	NULL,					// destroy
};


static void get_button_content_frame(BRect *frame)
{
	if(frame->IsValid())
	{
		frame->left += 3;
		frame->top += 3;
		frame->right -= 5;
		frame->bottom -= 5;
	}
}


static BRect get_button_content_frame(BRect frame)
{
	BRect rect = frame;
	get_button_content_frame(&rect);
	return rect;
}


static void get_border_margins(struct b_theme_engine *engine,
			       const BView *view, float *left, float *top, float *right, float *bottom,
			       b_border_style border, float border_width)
{
	if(engine != &bhapi_default_theme_engine || view == NULL ||
	   left == NULL || top == NULL || right == NULL || bottom == NULL) return;

	if(border == B_NO_BORDER)
	{
		*left = 0; *top = 0; *right = 0; *bottom = 0;
		return;
	}

	if(border == B_FANCY_BORDER && border_width < 2.f * view->UnitsPerPixel()) border_width = 2.f * view->UnitsPerPixel();
	else if(border_width < view->UnitsPerPixel()) border_width = view->UnitsPerPixel();

	*left = border_width;
	*top = border_width;
	*right = border_width;
	*bottom = border_width;
}


static void draw_border(struct b_theme_engine *engine,
			BView *view, BRect frame,
			b_border_style border, float border_width)
{
	if(engine != &bhapi_default_theme_engine ||
	   view == NULL || view->Window() == NULL || frame.IsValid() == false ||
	   border == B_NO_BORDER) return;

	BRegion clipping;
	view->GetClippingRegion(&clipping);
	clipping &= frame;
	if(clipping.CountRects() <= 0) return;

	if(border == B_FANCY_BORDER && border_width < 2.f * view->UnitsPerPixel()) border_width = 2.f * view->UnitsPerPixel();
	else if(border_width < view->UnitsPerPixel()) border_width = view->UnitsPerPixel();

	float penSize = (border == B_FANCY_BORDER ? (border_width / 2.f) : border_width);

	view->PushState();

	view->SetDrawingMode(B_OP_COPY);
	view->ConstrainClippingRegion(&clipping);
	view->SetPenSize(penSize);

	b_rgb_color lightColor = view->ViewColor().mix_copy(255, 255, 255, 200);
	b_rgb_color shadowColor = view->ViewColor().mix_copy(0, 0, 0, 100);

	if(border == B_FANCY_BORDER)
	{
		view->SetHighColor(lightColor);
		frame.InsetBy((penSize - view->UnitsPerPixel()) / 2.f, (penSize - view->UnitsPerPixel()) / 2.f);
		frame.left += penSize;
		frame.top += penSize;
		view->StrokeRect(frame);

		view->SetHighColor(shadowColor);
		frame.OffsetBy(-penSize, -penSize);
		view->StrokeRect(frame);
	}
	else
	{
		view->SetHighColor(shadowColor);
		frame.InsetBy((penSize - view->UnitsPerPixel()) / 2.f, (penSize - view->UnitsPerPixel()) / 2.f);
		view->StrokeRect(frame);
	}

	view->PopState();
}


static void get_scrollbar_preferred_size(struct b_theme_engine *engine,
					 const BView *view, float *width, float *height,
					 b_orientation direction)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || (width == NULL && height == NULL)) return;

	if(direction == B_HORIZONTAL)
	{
		if(width) *width = 100;
		if(height) *height = b_ui_get_scrollbar_horizontal_height();
	}
	else
	{
		if(width) *width = b_ui_get_scrollbar_vertical_width();
		if(height) *height = 100;
	}
}


static void get_scrollbar_respondent_region(struct b_theme_engine *engine,
					    const BView *view, BRect frame,
					    b_orientation direction, float minValue, float maxValue, float curValue, float *ratio,
					    BRegion *drag, BRegion *smallUp, BRegion *smallDown, BRegion *largeUp, BRegion *largeDown)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || frame.IsValid() == false ||
	   minValue > maxValue || curValue < minValue || curValue > maxValue ||
	   (ratio == NULL && drag == NULL && smallUp == NULL && smallDown == NULL && largeUp == NULL && largeDown == NULL)) return;

/*
	float l = 0, t = 0, r = 0, b = 0;
	engine->get_border_margins(engine, view, &l, &t, &r, &b, B_PLAIN_BORDER, view->UnitsPerPixel());
	frame.left += l; frame.top += t; frame.right -= r; frame.bottom -= b;
*/

	if(direction == B_HORIZONTAL)
	{
		BRect leftR = frame;
		BRect rightR = frame;
		BRect dragR;

		leftR.right = leftR.left + frame.Height();
		rightR.left = rightR.right - frame.Height();

		if(ratio && maxValue > minValue)
			*ratio = min_c((rightR.left - leftR.right - 2.f * view->UnitsPerPixel()) / (maxValue - minValue), 1.f);
		else if(ratio)
			*ratio = 0;

		if(drag == NULL && smallUp == NULL && smallDown == NULL && largeUp == NULL && largeDown == NULL) return;

		if(maxValue > minValue)
		{
			dragR = frame;
			if(rightR.left - leftR.right - 2.f * view->UnitsPerPixel() - frame.Height() > maxValue - minValue)
			{
				dragR.left = leftR.right + (curValue - minValue) + view->UnitsPerPixel();
				dragR.right = rightR.left - (maxValue - curValue) - view->UnitsPerPixel();
			}
			else
			{
				float x = leftR.right + view->UnitsPerPixel() + (curValue - minValue) *
					  (rightR.left - leftR.right - 2.f * view->UnitsPerPixel()) / (maxValue - minValue);
				dragR.left = x - frame.Height() / 2.f;
				dragR.right = dragR.left + frame.Height();

				if(dragR.left <= leftR.right && dragR.right >= rightR.left)
					dragR = BRect();
				else if(dragR.left <= leftR.right)
					dragR.OffsetTo(leftR.right +  view->UnitsPerPixel(), dragR.top);
				else if(dragR.right >= rightR.left)
					dragR.OffsetTo(rightR.left - view->UnitsPerPixel() - frame.Height(), dragR.top);
			}
		}

		if(drag) drag->Set(dragR);
		if(smallUp) smallUp->Set(leftR);
		if(smallDown) smallDown->Set(rightR);

		if(largeUp == NULL && largeDown == NULL) return;
		if(dragR.IsValid())
		{
			BRect upR = frame;
			BRect downR = frame;
			upR.left = leftR.right + view->UnitsPerPixel();
			upR.right = dragR.left - view->UnitsPerPixel();
			downR.left = dragR.right + view->UnitsPerPixel();
			downR.right = rightR.left - view->UnitsPerPixel();
			if(largeUp) largeUp->Set(upR);
			if(largeDown) largeDown->Set(downR);
		}
		else
		{
			if(largeUp) largeUp->MakeEmpty();
			if(largeDown) largeDown->MakeEmpty();
		}
	}
	else
	{
		BRect topR = frame;
		BRect bottomR = frame;
		BRect dragR;

		topR.bottom = topR.top + frame.Width();
		bottomR.top = bottomR.bottom - frame.Width();

		if(ratio && maxValue > minValue)
			*ratio = min_c((bottomR.top - topR.bottom - 2.f * view->UnitsPerPixel()) / (maxValue - minValue), 1);
		else if(ratio)
			*ratio = 0;

		if(drag == NULL && smallUp == NULL && smallDown == NULL && largeUp == NULL && largeDown == NULL) return;

		if(maxValue > minValue)
		{
			dragR = frame;
			if(bottomR.top - topR.bottom - 2.f * view->UnitsPerPixel() - frame.Width() > maxValue - minValue)
			{
				dragR.top = topR.bottom + (curValue - minValue) + view->UnitsPerPixel();
				dragR.bottom = bottomR.top - (maxValue - curValue) - view->UnitsPerPixel();
			}
			else
			{
				float y = topR.bottom + view->UnitsPerPixel() + (curValue - minValue) *
					  (bottomR.top - topR.bottom - 2.f * view->UnitsPerPixel()) / (maxValue - minValue);
				dragR.top = y - frame.Width() / 2.f;
				dragR.bottom = dragR.top + frame.Width();

				if(dragR.top <= topR.bottom && dragR.bottom >= bottomR.top)
					dragR = BRect();
				else if(dragR.top <= topR.bottom)
					dragR.OffsetTo(dragR.left, topR.bottom +  view->UnitsPerPixel());
				else if(dragR.bottom >= bottomR.top)
					dragR.OffsetTo(dragR.left, bottomR.top - view->UnitsPerPixel() - frame.Width());
			}
		}

		if(drag) drag->Set(dragR);
		if(smallUp) smallUp->Set(topR);
		if(smallDown) smallDown->Set(bottomR);

		if(largeUp == NULL && largeDown == NULL) return;
		if(dragR.IsValid())
		{
			BRect upR = frame;
			BRect downR = frame;
			upR.top = topR.bottom + view->UnitsPerPixel();
			upR.bottom = dragR.top - view->UnitsPerPixel();
			downR.top = dragR.bottom + view->UnitsPerPixel();
			downR.bottom = bottomR.top - view->UnitsPerPixel();
			if(largeUp) largeUp->Set(upR);
			if(largeDown) largeDown->Set(downR);
		}
		else
		{
			if(largeUp) largeUp->MakeEmpty();
			if(largeDown) largeDown->MakeEmpty();
		}
	}
}


static void draw_horizontal_triangle_inside(BView *view, BRect rect, bool left_to_right)
{
	if(view == NULL || rect.IsValid() == false) return;

	BPoint pts[3];

	if(left_to_right)
	{
		pts[0].x = rect.left + rect.Width() / 5.f;
		pts[0].y = rect.top + rect.Height() / 5.f;
		pts[1].x = rect.right - rect.Width() / 5.f;
		pts[1].y = rect.Center().y;
		pts[2].x = pts[0].x;
		pts[2].y = rect.bottom - rect.Height() / 5.f;
	}
	else
	{
		pts[0].x = rect.right - rect.Width() / 5.f;
		pts[0].y = rect.top + rect.Height() / 5.f;
		pts[1].x = rect.left + rect.Width() / 5.f;
		pts[1].y = rect.Center().y;
		pts[2].x = pts[0].x;
		pts[2].y = rect.bottom - rect.Height() / 5.f;
	}

	view->StrokePolygon(pts, 3, false);
}


static void draw_vertical_triangle_inside(BView *view, BRect rect, bool top_to_bottom)
{
	if(view == NULL || rect.IsValid() == false) return;

	BPoint pts[3];

	if(top_to_bottom)
	{
		pts[0].x = rect.left + rect.Width() / 5.f;
		pts[0].y = rect.top + rect.Height() / 5.f;
		pts[1].x = rect.Center().x;
		pts[1].y = rect.bottom - rect.Height() / 5.f;
		pts[2].x = rect.right - rect.Width() / 5.f;
		pts[2].y = pts[0].y;
	}
	else
	{
		pts[0].x = rect.left + rect.Width() / 5.f;
		pts[0].y = rect.bottom - rect.Height() / 5.f;
		pts[1].x = rect.Center().x;
		pts[1].y = rect.top + rect.Height() / 5.f;
		pts[2].x = rect.right - rect.Width() / 5.f;
		pts[2].y = pts[0].y;
	}

	view->StrokePolygon(pts, 3, false);
}


static void draw_scrollbar(struct b_theme_engine *engine,
			   BView *view, BRect frame,
			   b_orientation direction, float minValue, float maxValue, float curValue,
			   bool mouse_down, BPoint mouse_pos)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false ||
	   minValue > maxValue || curValue < minValue || curValue > maxValue) return;

/*
	float l = 0, t = 0, r = 0, b = 0;
	engine->get_border_margins(engine, view, &l, &t, &r, &b, B_PLAIN_BORDER, view->UnitsPerPixel());
	engine->draw_border(engine, view, frame, B_PLAIN_BORDER, view->UnitsPerPixel());
	frame.left += l; frame.top += t; frame.right -= r; frame.bottom -= b;
*/

	if(direction == B_HORIZONTAL)
	{
		BRect leftR = frame;
		BRect rightR = frame;
		BRect dragR;

		leftR.right = leftR.left + frame.Height();
		rightR.left = rightR.right - frame.Height();

		if(maxValue > minValue)
		{
			dragR = frame;
			if(rightR.left - leftR.right - 2.f * view->UnitsPerPixel() - frame.Height() > maxValue - minValue)
			{
				dragR.left = leftR.right + (curValue - minValue) + view->UnitsPerPixel();
				dragR.right = rightR.left - (maxValue - curValue) - view->UnitsPerPixel();
			}
			else
			{
				float x = leftR.right + view->UnitsPerPixel() + (curValue - minValue) *
					  (rightR.left - leftR.right - 2.f * view->UnitsPerPixel()) / (maxValue - minValue);
				dragR.left = x - frame.Height() / 2.f;
				dragR.right = dragR.left + frame.Height();

				if(dragR.left <= leftR.right && dragR.right >= rightR.left)
					dragR = BRect();
				else if(dragR.left <= leftR.right)
					dragR.OffsetTo(leftR.right +  view->UnitsPerPixel(), dragR.top);
				else if(dragR.right >= rightR.left)
					dragR.OffsetTo(rightR.left - view->UnitsPerPixel() - frame.Height(), dragR.top);
			}
		}

		engine->draw_button(engine, view, leftR,
				    "", (mouse_down ? leftR.Contains(mouse_pos) : false), leftR.Contains(mouse_pos), 0);
		engine->draw_button(engine, view, rightR,
				    "", (mouse_down ? rightR.Contains(mouse_pos) : false), rightR.Contains(mouse_pos), 0);
		engine->draw_button(engine, view, dragR,
				    "", (mouse_down ? dragR.Contains(mouse_pos) : false), dragR.Contains(mouse_pos), 0);

		BRect iLeftR = get_button_content_frame(leftR);
		BRect iRightR = get_button_content_frame(rightR);
		if(view->IsEnabled() && mouse_down && leftR.Contains(mouse_pos)) iLeftR.OffsetBy(1, 1);
		if(view->IsEnabled() && mouse_down && rightR.Contains(mouse_pos)) iRightR.OffsetBy(1, 1);

		view->PushState();
		view->SetDrawingMode(B_OP_COPY);

		if(view->IsEnabled())
		{
			view->SetHighColor(b_ui_color(B_BUTTON_TEXT_COLOR));
			draw_horizontal_triangle_inside(view, iLeftR, false);
			draw_horizontal_triangle_inside(view, iRightR, true);
		}
		else
		{
			view->SetHighColor(view->ViewColor().mix_copy(255, 255, 255, MIX_SHADOW_DARKER));
			draw_horizontal_triangle_inside(view, iLeftR.OffsetByCopy(1, 1), false);
			draw_horizontal_triangle_inside(view, iRightR.OffsetByCopy(1, 1), true);
			view->SetHighColor(view->ViewColor().mix_copy(0, 0, 0, MIX_SHADOW_DARKER));
			draw_horizontal_triangle_inside(view, iLeftR, false);
			draw_horizontal_triangle_inside(view, iRightR, true);
		}

		view->PopState();
	}
	else
	{
		BRect topR = frame;
		BRect bottomR = frame;
		BRect dragR;

		topR.bottom = topR.top + frame.Width();
		bottomR.top = bottomR.bottom - frame.Width();

		if(maxValue > minValue)
		{
			dragR = frame;
			if(bottomR.top - topR.bottom - 2.f * view->UnitsPerPixel() - frame.Width() > maxValue - minValue)
			{
				dragR.top = topR.bottom + (curValue - minValue) + view->UnitsPerPixel();
				dragR.bottom = bottomR.top - (maxValue - curValue) - view->UnitsPerPixel();
			}
			else
			{
				float y = topR.bottom + view->UnitsPerPixel() + (curValue - minValue) *
					  (bottomR.top - topR.bottom - 2.f * view->UnitsPerPixel()) / (maxValue - minValue);
				dragR.top = y - frame.Width() / 2.f;
				dragR.bottom = dragR.top + frame.Width();

				if(dragR.top <= topR.bottom && dragR.bottom >= bottomR.top)
					dragR = BRect();
				else if(dragR.top <= topR.bottom)
					dragR.OffsetTo(dragR.left, topR.bottom +  view->UnitsPerPixel());
				else if(dragR.bottom >= bottomR.top)
					dragR.OffsetTo(dragR.left, bottomR.top - view->UnitsPerPixel() - frame.Width());
			}
		}

		engine->draw_button(engine, view, topR,
				    "", (mouse_down ? topR.Contains(mouse_pos) : false), topR.Contains(mouse_pos), 0);
		engine->draw_button(engine, view, bottomR,
				    "", (mouse_down ? bottomR.Contains(mouse_pos) : false), bottomR.Contains(mouse_pos), 0);
		engine->draw_button(engine, view, dragR,
				    "", (mouse_down ? dragR.Contains(mouse_pos) : false), dragR.Contains(mouse_pos), 0);

		BRect iTopR = get_button_content_frame(topR);
		BRect iBottomR = get_button_content_frame(bottomR);
		if(view->IsEnabled() && mouse_down && topR.Contains(mouse_pos)) iTopR.OffsetBy(1, 1);
		if(view->IsEnabled() && mouse_down && bottomR.Contains(mouse_pos)) iBottomR.OffsetBy(1, 1);

		view->PushState();
		view->SetDrawingMode(B_OP_COPY);

		if(view->IsEnabled())
		{
			view->SetHighColor(b_ui_color(B_BUTTON_TEXT_COLOR));
			draw_vertical_triangle_inside(view, iTopR, false);
			draw_vertical_triangle_inside(view, iBottomR, true);
		}
		else
		{
			view->SetHighColor(view->ViewColor().mix_copy(255, 255, 255, MIX_SHADOW_DARKER));
			draw_vertical_triangle_inside(view, iTopR.OffsetByCopy(1, 1), false);
			draw_vertical_triangle_inside(view, iBottomR.OffsetByCopy(1, 1), true);
			view->SetHighColor(view->ViewColor().mix_copy(0, 0, 0, MIX_SHADOW_DARKER));
			draw_vertical_triangle_inside(view, iTopR, false);
			draw_vertical_triangle_inside(view, iBottomR, true);
		}

		view->PopState();
	}
}


static void get_button_preferred_size(struct b_theme_engine *engine,
				      const BView *view, float *width, float *height,
				      const char *button_label)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || (width == NULL && height == NULL)) return;

	BFont font;
	view->GetFont(&font);

	b_font_height fontHeight;
	font.GetHeight(&fontHeight);

	if(width != NULL)
	{
		*width = (button_label != NULL ? font.StringWidth(button_label) : 0);
		*width += 20;
	}

	if(height != NULL)
	{
		font.GetHeight(&fontHeight);
		*height = (button_label != NULL ? (fontHeight.ascent + fontHeight.descent) : 0);
		*height += 10;
	}
}


static b_uint8 should_button_do_focus_flash(struct b_theme_engine *engine, const BView *view)
{
	if(engine != &bhapi_default_theme_engine || view == NULL) return 0;
	if(view->IsFocus() == false || view->IsEnabled() == false) return 0;
	return B_THEME_FOCUS_FLASH_BORDER;
}


static void get_button_border_margins(struct b_theme_engine *engine,
				      const BView *view, float *left, float *top, float *right, float *bottom)
{
	if(engine != &bhapi_default_theme_engine || view == NULL ||
	   left == NULL || top == NULL || right == NULL || bottom == NULL) return;

	*left = 3; *top = 3; *right = 5; *bottom = 5;
}


static void draw_button_border(struct b_theme_engine *engine,
			       BView *view, BRect frame,
			       bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false) return;

	b_rgb_color viewColor = view->ViewColor();
	b_rgb_color backgroundColor = b_ui_color(B_BUTTON_BACKGROUND_COLOR);
	b_rgb_color borderColor = b_ui_color(B_BUTTON_BORDER_COLOR);

	bool button_enabled = view->IsEnabled();
	bool button_is_focus = view->IsFocus();

	if(button_enabled == false)
	{
		borderColor = backgroundColor = viewColor;
		backgroundColor.mix(0, 0, 0, MIX_SHADOW_SHINER);
		borderColor.mix(0, 0, 0, MIX_SHADOW_DARKER);
	}
	else
	{
		if(button_pushed)
			backgroundColor.mix(0, 0, 0, MIX_BUTTON_PUSHED);
		if(button_is_focus)
			borderColor = b_ui_color((focus_flash % 2 == 0 ? B_NAVIGATION_BASE_COLOR : B_NAVIGATION_PULSE_COLOR));
	}

	view->PushState();

	view->SetDrawingMode(B_OP_COPY);
	view->SetPenSize(1);
	view->SetSquarePointStyle(true);

	// Clear border
	view->SetHighColor(viewColor);
	BRect contentFrame = get_button_content_frame(frame);
	contentFrame.InsetBy(-1, -1);
	BRegion clearRegion(frame);
	clearRegion.Exclude(contentFrame);
	view->FillRegion(&clearRegion, B_SOLID_HIGH);

	// Draw background of border
	view->SetHighColor(backgroundColor);
	BRect rects[4];
	BRect rect = frame;
	rect.left += 1;
	rect.right = rect.left + 1;
	rect.top += 1;
	rect.bottom -= 3;
	rects[0] = rect;
	rect.right = frame.right - 3;
	rect.left = rect.right - 1;
	rects[1] = rect;
	rect.right = frame.right - 5;
	rect.left = frame.left + 3;
	rect.bottom = rect.top + 1;
	rects[2] = rect;
	rect.bottom = frame.bottom - 3;
	rect.top = rect.bottom - 1;
	rects[3] = rect;
	view->FillRects(rects, 4, B_SOLID_HIGH);

	// Draw shadow
	if(button_enabled == true && button_pushed == false)
	{
		b_rgb_color color1, color2;
		color1 = color2 = viewColor;
		color1.mix(0, 0, 0, MIX_SHADOW_DARKER);
		color2.mix(0, 0, 0, MIX_SHADOW_SHINER);

		BPoint pt1, pt2;

		view->SetHighColor(color1);
		pt1.x = frame.left + 4;
		pt1.y = frame.bottom - 1;
		pt2.x = frame.right - 3;
		pt2.y = pt1.y;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		view->SetHighColor(color2);
		pt1.x += 1; pt1.y += 1; pt2.x -= 1; pt2.y += 1;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		view->SetHighColor(color1);
		pt1.x = frame.right - 1;
		pt1.y = frame.top + 3;
		pt2.x = pt1.x;
		pt2.y = frame.bottom - 3;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		BPoint pt3 = pt2;

		view->SetHighColor(color2);
		pt1.x += 1; pt1.y += 1; pt2.x += 1; pt2.y -= 1;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		BPoint pts[3];

		view->SetHighColor(color1);
		pts[0] = pt3 + BPoint(-1, 0);
		pts[1] = pt3 + BPoint(-1, 1);
		pts[2] = pt3 + BPoint(-2, 1);
		view->StrokePoints(pts, 3, NULL, B_SOLID_HIGH);

		view->SetHighColor(color2);
		pts[0] = pt3 + BPoint(0, 1);
		pts[1] = pt3 + BPoint(-1, 2);
		view->StrokePoints(pts, 2, NULL, B_SOLID_HIGH);
	}
	else if(button_enabled == false)
	{
		b_rgb_color color = borderColor.mix_copy(255, 255, 255, 127);

		view->SetHighColor(color);

		BPoint pt1, pt2;

		pt1.x = frame.left + 4;
		pt1.y = frame.bottom - 1;
		pt2.x = frame.right - 3;
		pt2.y = pt1.y;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		pt1.x = frame.right - 1;
		pt1.y = frame.top + 3;
		pt2.x = pt1.x;
		pt2.y = frame.bottom - 3;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		BPoint pt3 = pt2;

		BPoint pts[3];

		pts[0] = pt3 + BPoint(-1, 0);
		pts[1] = pt3 + BPoint(-1, 1);
		pts[2] = pt3 + BPoint(-2, 1);
		view->StrokePoints(pts, 3, NULL, B_SOLID_HIGH);
	}

	BPoint pt1, pt2;
	BPoint pts[4];

	// Draw single border
	view->SetHighColor(borderColor);
	pt1 = frame.LeftTop(); pt2 = frame.RightTop();
	pt1.x += 2; pt2.x -= 4;
	view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
	pt1.y = pt2.y = frame.bottom - 2;
	view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
	pt1 = frame.LeftTop(); pt2 = frame.LeftBottom();
	pt1.y += 2; pt2.y -= 4;
	view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
	pt1.x = pt2.x = frame.right - 2;
	view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
	pts[0] = frame.LeftTop() + BPoint(1, 1);
	pts[1] = frame.RightTop() + BPoint(-3, 1);
	pts[2] = frame.LeftBottom() + BPoint(1, -3);
	pts[3] = frame.RightBottom() + BPoint(-3, -3);
	view->StrokePoints(pts, 4, NULL, B_SOLID_HIGH);

	if(button_enabled == false)
	{
		view->PopState();
		return;
	}

	if(button_pushed == true)
	{
		// Draw push shadow
		b_rgb_color color1, color2;
		color1 = color2 = backgroundColor;
		color1.mix(0, 0, 0, MIX_SHADOW_DARKER);
		color2.mix(0, 0, 0, MIX_SHADOW_SHINER);

		view->SetHighColor(color1);
		pt1 = frame.LeftTop() + BPoint(2, 1);
		pt2 = frame.RightTop() + BPoint(-4, 1);
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
		pt1 = frame.LeftTop() + BPoint(1, 2);
		pt2 = frame.LeftBottom() + BPoint(1, -4);
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		view->SetHighColor(color2);
		pt1 = frame.LeftTop() + BPoint(2, 2);
		pt2 = frame.RightTop() + BPoint(-3, 2);
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
		pt1 = frame.LeftTop() + BPoint(2, 3);
		pt2 = frame.LeftBottom() + BPoint(2, -3);
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		view->PopState();
		return;
	}

#if 0
	if(mouse_inside_button == true)
	{
		// draw over feel
		view->SetHighColor(borderColor);

		pt1 = frame.LeftTop() + BPoint(1, 2);
		pt2 = frame.LeftBottom() + BPoint(1, -4);
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
		pt1.x = pt2.x = frame.right - 3;
		view->StrokeLine(pt1, pt2, B_SOLID_HIGH);

		if(button_is_focus)
		{
			pt1 = frame.LeftTop() + BPoint(2, 1);
			pt2 = frame.LeftBottom() + BPoint(2, -3);
			view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
			pt1.x = pt2.x = frame.right - 4;
			view->StrokeLine(pt1, pt2, B_SOLID_HIGH);
		}
		else
		{
			BPoint Pts[12];

			pt1 = frame.LeftTop() + BPoint(2, 1);
			pt2 = frame.LeftBottom() + BPoint(2, -3);

			Pts[0] = pt1; Pts[1] = pt1 + BPoint(0, 1); Pts[2] = pt1 + BPoint(0, 2);
			Pts[3] = pt2; Pts[4] = pt2 + BPoint(0, -1); Pts[5] = pt2 + BPoint(0, -2);

			pt1.x = pt2.x = frame.right - 4;

			Pts[6] = pt1; Pts[7] = pt1 + BPoint(0, 1); Pts[8] = pt1 + BPoint(0, 2);
			Pts[9] = pt2; Pts[10] = pt2 + BPoint(0, -1); Pts[11] = pt2 + BPoint(0, -2);

			view->StrokePoints(Pts, 12, NULL, B_SOLID_HIGH);
		}
	}
	else
#endif
	if(button_is_focus)
	{
		view->SetHighColor(borderColor);
		pt1 = frame.LeftTop() + BPoint(1, 1);
		pt2 = frame.RightBottom() + BPoint(-3, -3);
		rect = BRect(pt1, pt2);
		view->StrokeRect(rect, B_SOLID_HIGH);
		rect.InsetBy(1, 1);
		view->StrokePoint(rect.LeftTop());
		view->StrokePoint(rect.RightTop());
		view->StrokePoint(rect.LeftBottom());
		view->StrokePoint(rect.RightBottom());
	}

	view->PopState();
}


static void clear_button_content(struct b_theme_engine *engine,
				 BView *view, BRect frame,
				 bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false) return;

	BRect contentFrame = get_button_content_frame(frame);
	bool button_enabled = view->IsEnabled();

	view->PushState();

	b_rgb_color color = b_ui_color(B_BUTTON_BACKGROUND_COLOR);
	if(button_enabled == false)
	{
		color = view->ViewColor();
		color.mix(0, 0, 0, MIX_SHADOW_SHINER);
	}
	else
	{
		if(button_pushed)
			color.mix(0, 0, 0, MIX_BUTTON_PUSHED);
		else if(mouse_inside_button)
			color.mix(255, 255, 255, 150);
	}

	view->SetDrawingMode(B_OP_COPY);
	view->SetHighColor(color);
	view->SetPenSize(0);
	view->FillRect(contentFrame, B_SOLID_HIGH);

	view->PopState();
}


static void draw_button_label(struct b_theme_engine *engine,
			      BView *view, BRect frame,
			      const char *button_label,
			      bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || view->Window() == NULL ||
	   frame.IsValid() == false || button_label == NULL || *button_label == 0) return;

	bool button_enabled = view->IsEnabled();

	view->PushState();

	BFont font;
	view->GetFont(&font);

	b_font_height fontHeight;
	font.GetHeight(&fontHeight);

	BRect rect = get_button_content_frame(frame);
	BPoint penLocation = view->PenLocation();
	if(button_pushed) penLocation += BPoint(2, 2);

	if(rect.Contains(penLocation))
	{
		rect.left = max_c(rect.left, penLocation.x);
		rect.top = max_c(rect.top, penLocation.y);
	}

	float sWidth = font.StringWidth(button_label);
	float sHeight = fontHeight.ascent + fontHeight.descent;
	penLocation.x = rect.Center().x - sWidth / 2.f;
	penLocation.y = rect.Center().y - sHeight / 2.f;
	penLocation.y += fontHeight.ascent + 1;

	if(button_enabled == false)
	{
		b_rgb_color backgroundColor = view->ViewColor();
		backgroundColor.mix(0, 0, 0, MIX_SHADOW_SHINER);

		b_rgb_color textColor = view->ViewColor();
		textColor.mix(255, 255, 255, MIX_SHADOW_DARKER);

		view->SetDrawingMode(B_OP_COPY);

		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation + BPoint(1, 1));

		backgroundColor = textColor;
		textColor = view->ViewColor();
		textColor.mix(0, 0, 0, MIX_SHADOW_DARKER);
		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation);
	}
	else
	{
		b_rgb_color backgroundColor = b_ui_color(B_BUTTON_BACKGROUND_COLOR);
		b_rgb_color textColor = b_ui_color(B_BUTTON_TEXT_COLOR);

		view->SetDrawingMode(B_OP_COPY);
		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation);
	}

	view->PopState();
}


static void draw_button(struct b_theme_engine *engine,
			BView *view, BRect frame,
			const char *button_label,
			bool button_pushed, bool mouse_inside_button, b_uint8 focus_flash)
{
	if(engine != &bhapi_default_theme_engine || view == NULL || view->Window() == NULL) return;

	BRegion clipping;
	view->GetClippingRegion(&clipping);
	clipping &= frame;
	if(clipping.CountRects() <= 0) return;

	view->PushState();

	view->ConstrainClippingRegion(&clipping);

	draw_button_border(engine, view, frame, button_pushed, mouse_inside_button, focus_flash);
	clear_button_content(engine, view, frame, button_pushed, mouse_inside_button, focus_flash);

	BRect contentFrame = get_button_content_frame(frame);
	clipping &= contentFrame;
	if(clipping.CountRects() > 0)
	{
		view->ConstrainClippingRegion(&clipping);
		view->MovePenTo(contentFrame.LeftTop());
		draw_button_label(engine, view, frame, button_label, button_pushed, mouse_inside_button, focus_flash);
	}

	view->PopState();
}

