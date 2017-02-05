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
 * File: DefaultTheme.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./ThemeEngine.h"

#define MIX_SHADOW_DARKER	80
#define MIX_SHADOW_SHINER	20
#define MIX_BUTTON_PUSHED	5

static void get_border_margins(struct e_theme_engine *engine,
			       const EView *view, float *left, float *top, float *right, float *bottom,
			       e_border_style border, float border_width);
static void draw_border(struct e_theme_engine *engine,
			EView *view, ERect frame,
			e_border_style border, float border_width);

static void get_scrollbar_preferred_size(struct e_theme_engine *engine,
					 const EView *view, float *width, float *height,
					 e_orientation direction);
static void get_scrollbar_respondent_region(struct e_theme_engine *engine,
					    const EView *view, ERect frame,
					    e_orientation direction, float minValue, float maxValue, float curValue, float *ratio,
					    ERegion *drag, ERegion *smallUp, ERegion *smallDown, ERegion *largeUp, ERegion *largeDown);
static void draw_scrollbar(struct e_theme_engine *engine,
			   EView *view, ERect frame,
			   e_orientation direction, float minValue, float maxValue, float curValue,
			   bool mouse_down, EPoint mouse_pos);

static void get_button_preferred_size(struct e_theme_engine *engine,
				      const EView *view, float *width, float *height,
				      const char *button_label);
static euint8 should_button_do_focus_flash(struct e_theme_engine *engine, const EView *view);
static void get_button_border_margins(struct e_theme_engine *engine,
				      const EView *view, float *left, float *top, float *right, float *bottom);
static void draw_button_border(struct e_theme_engine *engine,
			       EView *view, ERect frame,
			       bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
static void clear_button_content(struct e_theme_engine *engine,
				 EView *view, ERect frame,
				 bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
static void draw_button_label(struct e_theme_engine *engine,
			      EView *view, ERect frame,
			      const char *button_label,
			      bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
static void draw_button(struct e_theme_engine *engine,
			EView *view, ERect frame,
			const char *button_label,
			bool button_pushed, bool mouse_inside_button, euint8 focus_flash);

e_theme_engine etk_default_theme_engine = {
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


static void get_button_content_frame(ERect *frame)
{
	if(frame->IsValid())
	{
		frame->left += 3;
		frame->top += 3;
		frame->right -= 5;
		frame->bottom -= 5;
	}
}


static ERect get_button_content_frame(ERect frame)
{
	ERect rect = frame;
	get_button_content_frame(&rect);
	return rect;
}


static void get_border_margins(struct e_theme_engine *engine,
			       const EView *view, float *left, float *top, float *right, float *bottom,
			       e_border_style border, float border_width)
{
	if(engine != &etk_default_theme_engine || view == NULL ||
	   left == NULL || top == NULL || right == NULL || bottom == NULL) return;

	if(border == E_NO_BORDER)
	{
		*left = 0; *top = 0; *right = 0; *bottom = 0;
		return;
	}

	if(border == E_FANCY_BORDER && border_width < 2.f * view->UnitsPerPixel()) border_width = 2.f * view->UnitsPerPixel();
	else if(border_width < view->UnitsPerPixel()) border_width = view->UnitsPerPixel();

	*left = border_width;
	*top = border_width;
	*right = border_width;
	*bottom = border_width;
}


static void draw_border(struct e_theme_engine *engine,
			EView *view, ERect frame,
			e_border_style border, float border_width)
{
	if(engine != &etk_default_theme_engine ||
	   view == NULL || view->Window() == NULL || frame.IsValid() == false ||
	   border == E_NO_BORDER) return;

	ERegion clipping;
	view->GetClippingRegion(&clipping);
	clipping &= frame;
	if(clipping.CountRects() <= 0) return;

	if(border == E_FANCY_BORDER && border_width < 2.f * view->UnitsPerPixel()) border_width = 2.f * view->UnitsPerPixel();
	else if(border_width < view->UnitsPerPixel()) border_width = view->UnitsPerPixel();

	float penSize = (border == E_FANCY_BORDER ? (border_width / 2.f) : border_width);

	view->PushState();

	view->SetDrawingMode(E_OP_COPY);
	view->ConstrainClippingRegion(&clipping);
	view->SetPenSize(penSize);

	e_rgb_color lightColor = view->ViewColor().mix_copy(255, 255, 255, 200);
	e_rgb_color shadowColor = view->ViewColor().mix_copy(0, 0, 0, 100);

	if(border == E_FANCY_BORDER)
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


static void get_scrollbar_preferred_size(struct e_theme_engine *engine,
					 const EView *view, float *width, float *height,
					 e_orientation direction)
{
	if(engine != &etk_default_theme_engine || view == NULL || (width == NULL && height == NULL)) return;

	if(direction == E_HORIZONTAL)
	{
		if(width) *width = 100;
		if(height) *height = e_ui_get_scrollbar_horizontal_height();
	}
	else
	{
		if(width) *width = e_ui_get_scrollbar_vertical_width();
		if(height) *height = 100;
	}
}


static void get_scrollbar_respondent_region(struct e_theme_engine *engine,
					    const EView *view, ERect frame,
					    e_orientation direction, float minValue, float maxValue, float curValue, float *ratio,
					    ERegion *drag, ERegion *smallUp, ERegion *smallDown, ERegion *largeUp, ERegion *largeDown)
{
	if(engine != &etk_default_theme_engine || view == NULL || frame.IsValid() == false ||
	   minValue > maxValue || curValue < minValue || curValue > maxValue ||
	   (ratio == NULL && drag == NULL && smallUp == NULL && smallDown == NULL && largeUp == NULL && largeDown == NULL)) return;

/*
	float l = 0, t = 0, r = 0, b = 0;
	engine->get_border_margins(engine, view, &l, &t, &r, &b, E_PLAIN_BORDER, view->UnitsPerPixel());
	frame.left += l; frame.top += t; frame.right -= r; frame.bottom -= b;
*/

	if(direction == E_HORIZONTAL)
	{
		ERect leftR = frame;
		ERect rightR = frame;
		ERect dragR;

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
					dragR = ERect();
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
			ERect upR = frame;
			ERect downR = frame;
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
		ERect topR = frame;
		ERect bottomR = frame;
		ERect dragR;

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
					dragR = ERect();
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
			ERect upR = frame;
			ERect downR = frame;
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


static void draw_horizontal_triangle_inside(EView *view, ERect rect, bool left_to_right)
{
	if(view == NULL || rect.IsValid() == false) return;

	EPoint pts[3];

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


static void draw_vertical_triangle_inside(EView *view, ERect rect, bool top_to_bottom)
{
	if(view == NULL || rect.IsValid() == false) return;

	EPoint pts[3];

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


static void draw_scrollbar(struct e_theme_engine *engine,
			   EView *view, ERect frame,
			   e_orientation direction, float minValue, float maxValue, float curValue,
			   bool mouse_down, EPoint mouse_pos)
{
	if(engine != &etk_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false ||
	   minValue > maxValue || curValue < minValue || curValue > maxValue) return;

/*
	float l = 0, t = 0, r = 0, b = 0;
	engine->get_border_margins(engine, view, &l, &t, &r, &b, E_PLAIN_BORDER, view->UnitsPerPixel());
	engine->draw_border(engine, view, frame, E_PLAIN_BORDER, view->UnitsPerPixel());
	frame.left += l; frame.top += t; frame.right -= r; frame.bottom -= b;
*/

	if(direction == E_HORIZONTAL)
	{
		ERect leftR = frame;
		ERect rightR = frame;
		ERect dragR;

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
					dragR = ERect();
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

		ERect iLeftR = get_button_content_frame(leftR);
		ERect iRightR = get_button_content_frame(rightR);
		if(view->IsEnabled() && mouse_down && leftR.Contains(mouse_pos)) iLeftR.OffsetBy(1, 1);
		if(view->IsEnabled() && mouse_down && rightR.Contains(mouse_pos)) iRightR.OffsetBy(1, 1);

		view->PushState();
		view->SetDrawingMode(E_OP_COPY);

		if(view->IsEnabled())
		{
			view->SetHighColor(e_ui_color(E_BUTTON_TEXT_COLOR));
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
		ERect topR = frame;
		ERect bottomR = frame;
		ERect dragR;

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
					dragR = ERect();
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

		ERect iTopR = get_button_content_frame(topR);
		ERect iBottomR = get_button_content_frame(bottomR);
		if(view->IsEnabled() && mouse_down && topR.Contains(mouse_pos)) iTopR.OffsetBy(1, 1);
		if(view->IsEnabled() && mouse_down && bottomR.Contains(mouse_pos)) iBottomR.OffsetBy(1, 1);

		view->PushState();
		view->SetDrawingMode(E_OP_COPY);

		if(view->IsEnabled())
		{
			view->SetHighColor(e_ui_color(E_BUTTON_TEXT_COLOR));
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


static void get_button_preferred_size(struct e_theme_engine *engine,
				      const EView *view, float *width, float *height,
				      const char *button_label)
{
	if(engine != &etk_default_theme_engine || view == NULL || (width == NULL && height == NULL)) return;

	EFont font;
	view->GetFont(&font);

	e_font_height fontHeight;
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


static euint8 should_button_do_focus_flash(struct e_theme_engine *engine, const EView *view)
{
	if(engine != &etk_default_theme_engine || view == NULL) return 0;
	if(view->IsFocus() == false || view->IsEnabled() == false) return 0;
	return E_THEME_FOCUS_FLASH_BORDER;
}


static void get_button_border_margins(struct e_theme_engine *engine,
				      const EView *view, float *left, float *top, float *right, float *bottom)
{
	if(engine != &etk_default_theme_engine || view == NULL ||
	   left == NULL || top == NULL || right == NULL || bottom == NULL) return;

	*left = 3; *top = 3; *right = 5; *bottom = 5;
}


static void draw_button_border(struct e_theme_engine *engine,
			       EView *view, ERect frame,
			       bool button_pushed, bool mouse_inside_button, euint8 focus_flash)
{
	if(engine != &etk_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false) return;

	e_rgb_color viewColor = view->ViewColor();
	e_rgb_color backgroundColor = e_ui_color(E_BUTTON_BACKGROUND_COLOR);
	e_rgb_color borderColor = e_ui_color(E_BUTTON_BORDER_COLOR);

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
			borderColor = e_ui_color((focus_flash % 2 == 0 ? E_NAVIGATION_BASE_COLOR : E_NAVIGATION_PULSE_COLOR));
	}

	view->PushState();

	view->SetDrawingMode(E_OP_COPY);
	view->SetPenSize(1);
	view->SetSquarePointStyle(true);

	// Clear border
	view->SetHighColor(viewColor);
	ERect contentFrame = get_button_content_frame(frame);
	contentFrame.InsetBy(-1, -1);
	ERegion clearRegion(frame);
	clearRegion.Exclude(contentFrame);
	view->FillRegion(&clearRegion, E_SOLID_HIGH);

	// Draw background of border
	view->SetHighColor(backgroundColor);
	ERect rects[4];
	ERect rect = frame;
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
	view->FillRects(rects, 4, E_SOLID_HIGH);

	// Draw shadow
	if(button_enabled == true && button_pushed == false)
	{
		e_rgb_color color1, color2;
		color1 = color2 = viewColor;
		color1.mix(0, 0, 0, MIX_SHADOW_DARKER);
		color2.mix(0, 0, 0, MIX_SHADOW_SHINER);

		EPoint pt1, pt2;

		view->SetHighColor(color1);
		pt1.x = frame.left + 4;
		pt1.y = frame.bottom - 1;
		pt2.x = frame.right - 3;
		pt2.y = pt1.y;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		view->SetHighColor(color2);
		pt1.x += 1; pt1.y += 1; pt2.x -= 1; pt2.y += 1;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		view->SetHighColor(color1);
		pt1.x = frame.right - 1;
		pt1.y = frame.top + 3;
		pt2.x = pt1.x;
		pt2.y = frame.bottom - 3;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		EPoint pt3 = pt2;

		view->SetHighColor(color2);
		pt1.x += 1; pt1.y += 1; pt2.x += 1; pt2.y -= 1;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		EPoint pts[3];

		view->SetHighColor(color1);
		pts[0] = pt3 + EPoint(-1, 0);
		pts[1] = pt3 + EPoint(-1, 1);
		pts[2] = pt3 + EPoint(-2, 1);
		view->StrokePoints(pts, 3, NULL, E_SOLID_HIGH);

		view->SetHighColor(color2);
		pts[0] = pt3 + EPoint(0, 1);
		pts[1] = pt3 + EPoint(-1, 2);
		view->StrokePoints(pts, 2, NULL, E_SOLID_HIGH);
	}
	else if(button_enabled == false)
	{
		e_rgb_color color = borderColor.mix_copy(255, 255, 255, 127);

		view->SetHighColor(color);

		EPoint pt1, pt2;

		pt1.x = frame.left + 4;
		pt1.y = frame.bottom - 1;
		pt2.x = frame.right - 3;
		pt2.y = pt1.y;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		pt1.x = frame.right - 1;
		pt1.y = frame.top + 3;
		pt2.x = pt1.x;
		pt2.y = frame.bottom - 3;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		EPoint pt3 = pt2;

		EPoint pts[3];

		pts[0] = pt3 + EPoint(-1, 0);
		pts[1] = pt3 + EPoint(-1, 1);
		pts[2] = pt3 + EPoint(-2, 1);
		view->StrokePoints(pts, 3, NULL, E_SOLID_HIGH);
	}

	EPoint pt1, pt2;
	EPoint pts[4];

	// Draw single border
	view->SetHighColor(borderColor);
	pt1 = frame.LeftTop(); pt2 = frame.RightTop();
	pt1.x += 2; pt2.x -= 4;
	view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
	pt1.y = pt2.y = frame.bottom - 2;
	view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
	pt1 = frame.LeftTop(); pt2 = frame.LeftBottom();
	pt1.y += 2; pt2.y -= 4;
	view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
	pt1.x = pt2.x = frame.right - 2;
	view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
	pts[0] = frame.LeftTop() + EPoint(1, 1);
	pts[1] = frame.RightTop() + EPoint(-3, 1);
	pts[2] = frame.LeftBottom() + EPoint(1, -3);
	pts[3] = frame.RightBottom() + EPoint(-3, -3);
	view->StrokePoints(pts, 4, NULL, E_SOLID_HIGH);

	if(button_enabled == false)
	{
		view->PopState();
		return;
	}

	if(button_pushed == true)
	{
		// Draw push shadow
		e_rgb_color color1, color2;
		color1 = color2 = backgroundColor;
		color1.mix(0, 0, 0, MIX_SHADOW_DARKER);
		color2.mix(0, 0, 0, MIX_SHADOW_SHINER);

		view->SetHighColor(color1);
		pt1 = frame.LeftTop() + EPoint(2, 1);
		pt2 = frame.RightTop() + EPoint(-4, 1);
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
		pt1 = frame.LeftTop() + EPoint(1, 2);
		pt2 = frame.LeftBottom() + EPoint(1, -4);
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		view->SetHighColor(color2);
		pt1 = frame.LeftTop() + EPoint(2, 2);
		pt2 = frame.RightTop() + EPoint(-3, 2);
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
		pt1 = frame.LeftTop() + EPoint(2, 3);
		pt2 = frame.LeftBottom() + EPoint(2, -3);
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		view->PopState();
		return;
	}

#if 0
	if(mouse_inside_button == true)
	{
		// draw over feel
		view->SetHighColor(borderColor);

		pt1 = frame.LeftTop() + EPoint(1, 2);
		pt2 = frame.LeftBottom() + EPoint(1, -4);
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
		pt1.x = pt2.x = frame.right - 3;
		view->StrokeLine(pt1, pt2, E_SOLID_HIGH);

		if(button_is_focus)
		{
			pt1 = frame.LeftTop() + EPoint(2, 1);
			pt2 = frame.LeftBottom() + EPoint(2, -3);
			view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
			pt1.x = pt2.x = frame.right - 4;
			view->StrokeLine(pt1, pt2, E_SOLID_HIGH);
		}
		else
		{
			EPoint Pts[12];

			pt1 = frame.LeftTop() + EPoint(2, 1);
			pt2 = frame.LeftBottom() + EPoint(2, -3);

			Pts[0] = pt1; Pts[1] = pt1 + EPoint(0, 1); Pts[2] = pt1 + EPoint(0, 2);
			Pts[3] = pt2; Pts[4] = pt2 + EPoint(0, -1); Pts[5] = pt2 + EPoint(0, -2);

			pt1.x = pt2.x = frame.right - 4;

			Pts[6] = pt1; Pts[7] = pt1 + EPoint(0, 1); Pts[8] = pt1 + EPoint(0, 2);
			Pts[9] = pt2; Pts[10] = pt2 + EPoint(0, -1); Pts[11] = pt2 + EPoint(0, -2);

			view->StrokePoints(Pts, 12, NULL, E_SOLID_HIGH);
		}
	}
	else
#endif
	if(button_is_focus)
	{
		view->SetHighColor(borderColor);
		pt1 = frame.LeftTop() + EPoint(1, 1);
		pt2 = frame.RightBottom() + EPoint(-3, -3);
		rect = ERect(pt1, pt2);
		view->StrokeRect(rect, E_SOLID_HIGH);
		rect.InsetBy(1, 1);
		view->StrokePoint(rect.LeftTop());
		view->StrokePoint(rect.RightTop());
		view->StrokePoint(rect.LeftBottom());
		view->StrokePoint(rect.RightBottom());
	}

	view->PopState();
}


static void clear_button_content(struct e_theme_engine *engine,
				 EView *view, ERect frame,
				 bool button_pushed, bool mouse_inside_button, euint8 focus_flash)
{
	if(engine != &etk_default_theme_engine || view == NULL || view->Window() == NULL || frame.IsValid() == false) return;

	ERect contentFrame = get_button_content_frame(frame);
	bool button_enabled = view->IsEnabled();

	view->PushState();

	e_rgb_color color = e_ui_color(E_BUTTON_BACKGROUND_COLOR);
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

	view->SetDrawingMode(E_OP_COPY);
	view->SetHighColor(color);
	view->SetPenSize(0);
	view->FillRect(contentFrame, E_SOLID_HIGH);

	view->PopState();
}


static void draw_button_label(struct e_theme_engine *engine,
			      EView *view, ERect frame,
			      const char *button_label,
			      bool button_pushed, bool mouse_inside_button, euint8 focus_flash)
{
	if(engine != &etk_default_theme_engine || view == NULL || view->Window() == NULL ||
	   frame.IsValid() == false || button_label == NULL || *button_label == 0) return;

	bool button_enabled = view->IsEnabled();

	view->PushState();

	EFont font;
	view->GetFont(&font);

	e_font_height fontHeight;
	font.GetHeight(&fontHeight);

	ERect rect = get_button_content_frame(frame);
	EPoint penLocation = view->PenLocation();
	if(button_pushed) penLocation += EPoint(2, 2);

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
		e_rgb_color backgroundColor = view->ViewColor();
		backgroundColor.mix(0, 0, 0, MIX_SHADOW_SHINER);

		e_rgb_color textColor = view->ViewColor();
		textColor.mix(255, 255, 255, MIX_SHADOW_DARKER);

		view->SetDrawingMode(E_OP_COPY);

		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation + EPoint(1, 1));

		backgroundColor = textColor;
		textColor = view->ViewColor();
		textColor.mix(0, 0, 0, MIX_SHADOW_DARKER);
		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation);
	}
	else
	{
		e_rgb_color backgroundColor = e_ui_color(E_BUTTON_BACKGROUND_COLOR);
		e_rgb_color textColor = e_ui_color(E_BUTTON_TEXT_COLOR);

		view->SetDrawingMode(E_OP_COPY);
		view->SetHighColor(textColor);
		view->SetLowColor(backgroundColor);
		view->DrawString(button_label, penLocation);
	}

	view->PopState();
}


static void draw_button(struct e_theme_engine *engine,
			EView *view, ERect frame,
			const char *button_label,
			bool button_pushed, bool mouse_inside_button, euint8 focus_flash)
{
	if(engine != &etk_default_theme_engine || view == NULL || view->Window() == NULL) return;

	ERegion clipping;
	view->GetClippingRegion(&clipping);
	clipping &= frame;
	if(clipping.CountRects() <= 0) return;

	view->PushState();

	view->ConstrainClippingRegion(&clipping);

	draw_button_border(engine, view, frame, button_pushed, mouse_inside_button, focus_flash);
	clear_button_content(engine, view, frame, button_pushed, mouse_inside_button, focus_flash);

	ERect contentFrame = get_button_content_frame(frame);
	clipping &= contentFrame;
	if(clipping.CountRects() > 0)
	{
		view->ConstrainClippingRegion(&clipping);
		view->MovePenTo(contentFrame.LeftTop());
		draw_button_label(engine, view, frame, button_label, button_pushed, mouse_inside_button, focus_flash);
	}

	view->PopState();
}

