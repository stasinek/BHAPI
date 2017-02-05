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
 * File: InterfaceDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_INTERFACE_DEFS_H__
#define __ETK_INTERFACE_DEFS_H__

#include "./../interface/GraphicsDefs.h>

enum {
	E_FOLLOW_NONE			= 0,
	E_FOLLOW_LEFT			= 1,
	E_FOLLOW_RIGHT			= 1 << 1,
	E_FOLLOW_TOP			= 1 << 2,
	E_FOLLOW_BOTTOM			= 1 << 3,
	E_FOLLOW_H_CENTER		= 1 << 4,
	E_FOLLOW_V_CENTER		= 1 << 5,
	E_FOLLOW_ALL			= 0xffff
};

#define E_FOLLOW_LEFT_RIGHT	(E_FOLLOW_LEFT | E_FOLLOW_RIGHT)
#define E_FOLLOW_TOP_BOTTOM	(E_FOLLOW_TOP | E_FOLLOW_BOTTOM)
#define E_FOLLOW_ALL_SIDES	E_FOLLOW_ALL

enum {
	E_BACKSPACE	= 0x08,
	E_RETURN	= 0x0a,
	E_ENTER		= 0x0a,
	E_SPACE		= 0x20,
	E_TAB		= 0x09,
	E_ESCAPE	= 0x1b,

	E_LEFT_ARROW	= 0x1c,
	E_RIGHT_ARROW	= 0x1d,
	E_UP_ARROW	= 0x1e,
	E_DOWN_ARROW	= 0x1f,

	E_INSERT	= 0x05,
	E_DELETE	= 0x7f,
	E_HOME		= 0x01,
	E_END		= 0x04,
	E_PAGE_UP	= 0x0b,
	E_PAGE_DOWN	= 0x0c,

	E_FUNCTION_KEY	= 0x10
};

enum {
	E_F1_KEY	= 0x02,
	E_F2_KEY	= 0x03,
	E_F3_KEY	= 0x04,
	E_F4_KEY	= 0x05,
	E_F5_KEY	= 0x06,
	E_F6_KEY	= 0x07,
	E_F7_KEY	= 0x08,
	E_F8_KEY	= 0x09,
	E_F9_KEY	= 0x0a,
	E_F10_KEY	= 0x0b,
	E_F11_KEY	= 0x0c,
	E_F12_KEY	= 0x0d,
	E_PRINT_KEY	= 0x0e,
	E_SCROLL_KEY	= 0x0f,
	E_PAUSE_KEY	= 0x10
};

enum e_border_style {
	E_PLAIN_BORDER,
	E_FANCY_BORDER,
	E_NO_BORDER
};

enum e_orientation {
	E_HORIZONTAL,
	E_VERTICAL
};

enum e_join_mode {
	E_ROUND_JOIN = 0,
	E_MITER_JOIN,
	E_BEVEL_JOIN,
	E_BUTT_JOIN,
	E_SQUARE_JOIN
};

enum e_cap_mode {
	E_ROUND_CAP = E_ROUND_JOIN,
	E_BUTT_CAP = E_BUTT_JOIN,
	E_SQUARE_CAP = E_SQUARE_JOIN
};

enum e_alignment {
	E_ALIGN_LEFT,
	E_ALIGN_RIGHT,
	E_ALIGN_CENTER
};

enum e_vertical_alignment {
	E_ALIGN_TOP,
	E_ALIGN_BOTTOM,
	E_ALIGN_MIDDLE
};

enum {
	E_SHIFT_KEY		= 1 << 1,
	E_COMMAND_KEY		= 1 << 2,
	E_CONTROL_KEY		= 1 << 3,
	E_CAPS_LOCK		= 1 << 4,
	E_SCROLL_LOCK		= 1 << 5,
	E_NUM_LOCK		= 1 << 6,
	E_OPTION_KEY		= 1 << 7,
	E_MENU_KEY		= 1 << 8,
	E_LEFT_SHIFT_KEY	= 1 << 9,
	E_RIGHT_SHIFT_KEY	= 1 << 10,
	E_LEFT_COMMAND_KEY	= 1 << 11,
	E_RIGHT_COMMAND_KEY	= 1 << 12,
	E_LEFT_CONTROL_KEY	= 1 << 13,
	E_RIGHT_CONTROL_KEY	= 1 << 14,
	E_LEFT_OPTION_KEY	= 1 << 15,
	E_RIGHT_OPTION_KEY	= 1 << 16,
	E_FUNCTIONS_KEY		= 1 << 17
};

enum {
	E_PRIMARY_MOUSE_BUTTON = 1,
	E_SECONDARY_MOUSE_BUTTON = 2,
	E_TERTIARY_MOUSE_BUTTON = 3
};

typedef enum e_color_which {
	E_DESKTOP_COLOR = 0,

	E_PANEL_BACKGROUND_COLOR = 1,
	E_PANEL_TEXT_COLOR = 2,

	E_DOCUMENT_BACKGROUND_COLOR = 3,
	E_DOCUMENT_TEXT_COLOR = 4,
	E_DOCUMENT_HIGHLIGHT_COLOR = 5,
	E_DOCUMENT_CURSOR_COLOR = 6,

	E_BUTTON_BACKGROUND_COLOR = 7,
	E_BUTTON_TEXT_COLOR = 8,
	E_BUTTON_BORDER_COLOR = 9,

	E_NAVIGATION_BASE_COLOR = 10,
	E_NAVIGATION_PULSE_COLOR = 11,

	E_MENU_BACKGROUND_COLOR = 12,
	E_MENU_BORDER_COLOR = 13,
	E_MENU_SELECTED_BACKGROUND_COLOR = 14,
	E_MENU_ITEM_TEXT_COLOR = 15,
	E_MENU_SELECTED_ITEM_TEXT_COLOR = 16,
	E_MENU_SELECTED_BORDER_COLOR = 17,

	E_TOOLTIP_BACKGROUND_COLOR = 18,
	E_TOOLTIP_TEXT_COLOR = 19,

	E_SHINE_COLOR = 20,
	E_SHADOW_COLOR = 21,

	E_STATUSBAR_COLOR = 22,

} e_color_which;


enum e_button_width {
	E_WIDTH_AS_USUAL,
	E_WIDTH_FROM_LABEL
};


#ifdef __cplusplus /* Just for C++ */
extern "C" {
#endif

_IMPEXP_ETK e_rgb_color e_ui_color(e_color_which which);
_IMPEXP_ETK float e_ui_get_scrollbar_horizontal_height();
_IMPEXP_ETK float e_ui_get_scrollbar_vertical_width();

#ifdef __cplusplus /* Just for C++ */
} /* extern "C" */
#endif

#endif /* __ETK_INTERFACE_DEFS_H__ */

