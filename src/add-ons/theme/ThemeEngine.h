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
 * File: ThemeEngine.h
 * Description: Theme Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_THEME_ENGINE_H__
#define __ETK_THEME_ENGINE_H__

#include "./../../interface/View.h"

#ifdef __cplusplus /* Just for C++ */

enum {
	E_THEME_FOCUS_FLASH_BORDER = 1,
	E_THEME_FOCUS_FLASH_CONTENT = 1 << 1,
};

typedef struct e_theme_engine {
	// custom data
	void *data;

	// border
	void (*get_border_margins)(struct e_theme_engine *engine,
				   const EView *view, float *left, float *top, float *right, float *bottom,
				   e_border_style border, float border_width);
	void (*draw_border)(struct e_theme_engine *engine,
			    EView *view, ERect frame,
			    e_border_style border, float border_width);

	// scrollbar
	void (*get_scrollbar_preferred_size)(struct e_theme_engine *engine,
					     const EView *view, float *width, float *height,
					     e_orientation direction);
	void (*get_scrollbar_respondent_region)(struct e_theme_engine *engine,
						const EView *view, ERect frame,
						e_orientation direction, float minValue, float maxValue, float curValue, float *ratio,
						ERegion *drag, ERegion *smallUp, ERegion *smallDown, ERegion *largeUp, ERegion *largeDown);
	void (*draw_scrollbar)(struct e_theme_engine *engine,
			       EView *view, ERect frame,
			       e_orientation direction, float minValue, float maxValue, float curValue,
			       bool mouse_down, EPoint mouse_pos);

	// button
	void (*get_button_preferred_size)(struct e_theme_engine *engine,
					  const EView *view, float *width, float *height,
					  const char *button_label);
	euint8 (*should_button_do_focus_flash)(struct e_theme_engine *engine, const EView *view);
	void (*get_button_border_margins)(struct e_theme_engine *engine,
					  const EView *view, float *left, float *top, float *right, float *bottom);
	void (*draw_button_border)(struct e_theme_engine *engine,
				   EView *view, ERect frame,
				   bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
	void (*clear_button_content)(struct e_theme_engine *engine,
				     EView *view, ERect frame,
				     bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
	void (*draw_button_label)(struct e_theme_engine *engine,
				  EView *view, ERect frame,
				  const char *button_label,
				  bool button_pushed, bool mouse_inside_button, euint8 focus_flash);
	void (*draw_button)(struct e_theme_engine *engine,
			    EView *view, ERect frame,
			    const char *button_label,
			    bool button_pushed, bool mouse_inside_button, euint8 focus_flash);

	// engine
	bool (*init)(struct e_theme_engine *engine);
	void (*destroy)(struct e_theme_engine *engine);
} e_theme_engine;


_IMPEXP_ETK e_theme_engine *etk_get_current_theme_engine(void);


#endif /* __cplusplus */

#endif /* __ETK_THEME_ENGINE_H__ */

