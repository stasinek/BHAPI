/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: ThemeEngine.h
 * Description: Theme Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_THEME_ENGINE_H
#define BHAPI_THEME_ENGINE_H
#include <kits/support/SupportDefs.h>
#include <kits/interface/InterfaceDefs.h>

#ifdef __cplusplus /* Just for C++ */
class BView;
class BRect;
class BRegion;
class BPoint;

namespace bhapi {
enum {
    B_THEME_FOCUS_FLASH_BORDER = 1,
    B_THEME_FOCUS_FLASH_CONTENT = 1 << 1,
};

typedef struct b_theme_engine {
	// custom data
	void *data;

	// border
	void (*get_border_margins)(struct b_theme_engine *engine,
                   const BView *view, float *left, float *top, float *right, float *bottom,
				   bhapi::border_style border, float border_width);
	void (*draw_border)(struct b_theme_engine *engine,
                BView *view, BRect frame,
			    bhapi::border_style border, float border_width);

	// scrollbar
	void (*get_scrollbar_preferred_size)(struct b_theme_engine *engine,
                         const BView *view, float *width, float *height,
					     bhapi::orientation direction);
	void (*get_scrollbar_respondent_region)(struct b_theme_engine *engine,
                        const BView *view, BRect frame,
						bhapi::orientation direction, float minValue, float maxValue, float curValue, float *ratio,
                        BRegion *drag, BRegion *smallUp, BRegion *smallDown, BRegion *largeUp, BRegion *largeDown);
	void (*draw_scrollbar)(struct b_theme_engine *engine,
                   BView *view, BRect frame,
			       bhapi::orientation direction, float minValue, float maxValue, float curValue,
                   bool mouse_down, BPoint mouse_pos);

	// button
	void (*get_button_preferred_size)(struct b_theme_engine *engine,
                      const BView *view, float *width, float *height,
					  const char *button_label);
     uint8 (*should_button_do_focus_flash)(struct b_theme_engine *engine, const BView *view);
	void (*get_button_border_margins)(struct b_theme_engine *engine,
                      const BView *view, float *left, float *top, float *right, float *bottom);
	void (*draw_button_border)(struct b_theme_engine *engine,
                   BView *view, BRect frame,
                   bool button_pushed, bool mouse_inside_button,  uint8 focus_flash);
	void (*clear_button_content)(struct b_theme_engine *engine,
                     BView *view, BRect frame,
                     bool button_pushed, bool mouse_inside_button,  uint8 focus_flash);
	void (*draw_button_label)(struct b_theme_engine *engine,
                  BView *view, BRect frame,
				  const char *button_label,
                  bool button_pushed, bool mouse_inside_button,  uint8 focus_flash);
	void (*draw_button)(struct b_theme_engine *engine,
                BView *view, BRect frame,
			    const char *button_label,
                bool button_pushed, bool mouse_inside_button,  uint8 focus_flash);

	// engine
	bool (*init)(struct b_theme_engine *engine);
	void (*destroy)(struct b_theme_engine *engine);
} b_theme_engine;

BHAPI_IMPEXP b_theme_engine *get_current_theme_engine(void);
} /* namespace bhapi */
#endif /* __cplusplus */
#endif /* BHAPI_THEME_ENGINE_H */

