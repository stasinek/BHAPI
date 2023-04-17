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
 * File: InterfaceDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_INTERFACE_DEFS_H
#define BHAPI_INTERFACE_DEFS_H
#include <kits/interface/GraphicsDefs.h>
// some handy UTF-8 characters
#define B_UTF8_BULLET		"\xE2\x80\xA2"
#define B_UTF8_ELLIPSIS		"\xE2\x80\xA6"
#define B_UTF8_OPEN_QUOTE	"\xE2\x80\x9C"
#define B_UTF8_CLOSE_QUOTE	"\xE2\x80\x9D"
#define B_UTF8_COPYRIGHT	"\xC2\xA9"
#define B_UTF8_REGISTERED	"\xC2\xAE"
#define B_UTF8_TRADEMARK	"\xE2\x84\xA2"
#define B_UTF8_SMILING_FACE	"\xE2\x98\xBB"
#define B_UTF8_HIROSHI		"\xE5\xBC\x98"

#ifdef COMPILE_FOR_R5
    #define B_MAX_MOUSE_BUTTONS 3
#else
    #define B_MAX_MOUSE_BUTTONS 16
#endif

enum {
    B_BACKSPACE			= 0x08,
    B_RETURN			= 0x0a,
    B_ENTER				= 0x0a,
    B_SPACE				= 0x20,
    B_TAB				= 0x09,
    B_ESCAPE			= 0x1b,
    B_SUBSTITUTE		= 0x1a,

    B_LEFT_ARROW		= 0x1c,
    B_RIGHT_ARROW		= 0x1d,
    B_UP_ARROW			= 0x1e,
    B_DOWN_ARROW		= 0x1f,

    B_INSERT			= 0x05,
    B_DELETE			= 0x7f,
    B_HOME				= 0x01,
    B_END				= 0x04,
    B_PAGE_UP			= 0x0b,
    B_PAGE_DOWN			= 0x0c,

    B_FUNCTION_KEY		= 0x10,

    // for Japanese keyboards
    B_KATAKANA_HIRAGANA	= 0xf2,
    B_HANKAKU_ZENKAKU	= 0xf3
};

enum {
    B_F1_KEY			= 0x02,
    B_F2_KEY			= 0x03,
    B_F3_KEY			= 0x04,
    B_F4_KEY			= 0x05,
    B_F5_KEY			= 0x06,
    B_F6_KEY			= 0x07,
    B_F7_KEY			= 0x08,
    B_F8_KEY			= 0x09,
    B_F9_KEY			= 0x0a,
    B_F10_KEY			= 0x0b,
    B_F11_KEY			= 0x0c,
    B_F12_KEY			= 0x0d,
    B_PRINT_KEY			= 0x0e,
    B_SCROLL_KEY		= 0x0f,
    B_PAUSE_KEY			= 0x10
};
enum {
    B_CONTROL_TABLE				= 0x00000001,
    B_OPTION_CAPS_SHIFT_TABLE	= 0x00000002,
    B_OPTION_CAPS_TABLE			= 0x00000004,
    B_OPTION_SHIFT_TABLE		= 0x00000008,
    B_OPTION_TABLE				= 0x00000010,
    B_CAPS_SHIFT_TABLE			= 0x00000020,
    B_CAPS_TABLE				= 0x00000040,
    B_SHIFT_TABLE				= 0x00000080,
    B_NORMAL_TABLE				= 0x00000100
};
// modifiers
enum {
    B_SHIFT_KEY			= 0x00000001,
    B_COMMAND_KEY		= 0x00000002,
    B_CONTROL_KEY		= 0x00000004,
    B_CAPS_LOCK			= 0x00000008,
    B_SCROLL_LOCK		= 0x00000010,
    B_NUM_LOCK			= 0x00000020,
    B_OPTION_KEY		= 0x00000040,
    B_MENU_KEY			= 0x00000080,
    B_LEFT_SHIFT_KEY	= 0x00000100,
    B_RIGHT_SHIFT_KEY	= 0x00000200,
    B_LEFT_COMMAND_KEY	= 0x00000400,
    B_RIGHT_COMMAND_KEY	= 0x00000800,
    B_LEFT_CONTROL_KEY	= 0x00001000,
    B_RIGHT_CONTROL_KEY	= 0x00002000,
    B_LEFT_OPTION_KEY	= 0x00004000,
    B_RIGHT_OPTION_KEY	= 0x00008000,
    B_FUNCTIONS_KEY     = 0x00010000
};
enum mode_mouse {
    B_NORMAL_MOUSE 			= 0,
    B_CLICK_TO_FOCUS_MOUSE	= -1,
    B_FOCUS_FOLLOWS_MOUSE	= 1
};

enum mode_focus_follows_mouse {
    B_NORMAL_FOCUS_FOLLOWS_MOUSE		= 0,
    B_WARP_FOCUS_FOLLOWS_MOUSE			= 1,
    B_INSTANT_WARP_FOCUS_FOLLOWS_MOUSE	= 2
};
// View orientation/alignment/style
enum border_style {
    B_PLAIN_BORDER,
    B_FANCY_BORDER,
    B_NO_BORDER
};

enum orientation {
    B_HORIZONTAL,
    B_VERTICAL
};

enum button_width {
    B_WIDTH_AS_USUAL,
    B_WIDTH_FROM_WIDEST,
    B_WIDTH_FROM_LABEL
};

enum alignment {
    B_ALIGN_LEFT,
    B_ALIGN_RIGHT,
    B_ALIGN_CENTER,

    B_ALIGN_HORIZONTAL_CENTER	= B_ALIGN_CENTER,

    B_ALIGN_HORIZONTAL_UNSET	= -1L,
    B_ALIGN_USE_FULL_WIDTH		= -2L
};

enum vertical_alignment {
    B_ALIGN_TOP					= 0x10L,
    B_ALIGN_MIDDLE				= 0x20,
    B_ALIGN_BOTTOM				= 0x30,

    B_ALIGN_VERTICAL_CENTER		= B_ALIGN_MIDDLE,

    B_ALIGN_VERTICAL_UNSET		= -1L,
    B_ALIGN_NO_VERTICAL			= B_ALIGN_VERTICAL_UNSET,
    B_ALIGN_USE_FULL_HEIGHT		= -2L
};
// Layout spacing and insets, see BControlLook::ComposeSpacing()
enum {
    B_USE_DEFAULT_SPACING = -1002,
    B_USE_ITEM_SPACING = -1003,
    B_USE_ITEM_INSETS = -1003,
    B_USE_HALF_ITEM_SPACING = -1004,
    B_USE_HALF_ITEM_INSETS = -1004,
    B_USE_WINDOW_INSETS = -1005,
    B_USE_WINDOW_SPACING = -1005,
    B_USE_SMALL_INSETS = -1006,
    B_USE_SMALL_SPACING = -1006,
    B_USE_BIG_INSETS = -1007,
    B_USE_BIG_SPACING = -1007
};
// Line join and cap modes
enum join_mode {
    B_ROUND_JOIN = 0,
    B_MITER_JOIN,
    B_BEVEL_JOIN,
    B_BUTT_JOIN,
    B_SQUARE_JOIN
};

enum cap_mode {
    B_ROUND_CAP		= B_ROUND_JOIN,
    B_BUTT_CAP		= B_BUTT_JOIN,
    B_SQUARE_CAP	= B_SQUARE_JOIN
};
const float B_DEFAULT_MITER_LIMIT = 10.0F;
// Polygon filling rules
enum {
    B_EVEN_ODD = 0,
    B_NONZERO
};
// Bitmap and overlay constants
enum bitmap_tiling {
    B_TILE_BITMAP_X				= 0x00000001,
    B_TILE_BITMAP_Y				= 0x00000002,
    B_TILE_BITMAP				= 0x00000003
};

enum overlay_options {
    B_OVERLAY_FILTER_HORIZONTAL	= 0x00010000,
    B_OVERLAY_FILTER_VERTICAL	= 0x00020000,
    B_OVERLAY_MIRROR			= 0x00040000,
    B_OVERLAY_TRANSFER_CHANNEL	= 0x00080000
};

enum bitmap_drawing_options {
    B_FILTER_BITMAP_BILINEAR	= 0x00000100,
    B_WAIT_FOR_RETRACE			= 0x00000800
};
// Default UI Colors
enum color_which {
    B_PANEL_BACKGROUND_COLOR = 1,
    B_PANEL_TEXT_COLOR = 10,
    B_DOCUMENT_BACKGROUND_COLOR = 11,
    B_DOCUMENT_TEXT_COLOR = 12,
    B_CONTROL_BACKGROUND_COLOR = 13,
    B_CONTROL_TEXT_COLOR = 14,
    B_CONTROL_BORDER_COLOR = 15,
    B_CONTROL_HIGHLIGHT_COLOR = 16,
    B_CONTROL_MARK_COLOR = 27,
    B_NAVIGATION_BASE_COLOR = 4,
    B_NAVIGATION_PULSE_COLOR = 17,
    // Old name synonyms.
    B_KEYBOARD_NAVIGATION_COLOR = B_NAVIGATION_BASE_COLOR,
    B_SHINE_COLOR = 18,
    B_SHADOW_COLOR = 19,

    B_MENU_BACKGROUND_COLOR = 2,
    B_MENU_SELECTED_BACKGROUND_COLOR = 6,
    B_MENU_ITEM_TEXT_COLOR = 7,
    B_MENU_SELECTED_ITEM_TEXT_COLOR = 8,
    B_MENU_SELECTED_BORDER_COLOR = 9,
    B_MENU_BORDER_COLOR = 27,
    // Old name synonyms.
    B_MENU_SELECTION_BACKGROUND_COLOR = B_MENU_SELECTED_BACKGROUND_COLOR,

    B_LIST_BACKGROUND_COLOR = 28,
    B_LIST_SELECTED_BACKGROUND_COLOR = 29,
    B_LIST_ITEM_TEXT_COLOR = 30,
    B_LIST_SELECTED_ITEM_TEXT_COLOR = 31,

    B_SCROLL_BAR_THUMB_COLOR = 32,

    B_TOOL_TIP_BACKGROUND_COLOR = 20,
    B_TOOL_TIP_TEXT_COLOR = 21,

    B_SUCCESS_COLOR = 100,
    B_FAILURE_COLOR = 101,

    B_WINDOW_TAB_COLOR = 3,
    B_WINDOW_TEXT_COLOR = 22,
    B_WINDOW_INACTIVE_TAB_COLOR = 23,
    B_WINDOW_INACTIVE_TEXT_COLOR = 24,

    B_WINDOW_BORDER_COLOR = 25,
    B_WINDOW_INACTIVE_BORDER_COLOR = 26,

    B_DOCUMENT_HIGHLIGHT_COLOR = 5,
    B_DOCUMENT_CURSOR_COLOR = 6,

    B_BUTTON_BACKGROUND_COLOR = 7,
    B_BUTTON_TEXT_COLOR = 8,
    B_BUTTON_BORDER_COLOR = 9,

    B_TOOLTIP_BACKGROUND_COLOR = 18,
    B_TOOLTIP_TEXT_COLOR = 19,

    B_STATUSBAR_COLOR = 22,
    // The following constants are deprecated, do not use in new code.
    B_DESKTOP_COLOR = 99
        // see BScreen class for B_DESKTOP_COLOR replacement
};
// Color tinting
const float B_LIGHTEN_MAX_TINT	= 0.0f;		// 216 --> 255.0 (255)
const float B_LIGHTEN_2_TINT	= 0.385f;	// 216 --> 240.0 (240)
const float B_LIGHTEN_1_TINT	= 0.590f;	// 216 --> 232.0 (232)

const float B_NO_TINT			= 1.0f;		// 216 --> 216.0 (216)

const float B_DARKEN_1_TINT		= 1.147f;	// 216 --> 184.2 (184)
const float B_DARKEN_2_TINT		= 1.295f;	// 216 --> 152.3 (152)
const float B_DARKEN_3_TINT		= 1.407f;	// 216 --> 128.1 (128)
const float B_DARKEN_4_TINT		= 1.555f;	// 216 -->  96.1  (96)
const float B_DARKEN_MAX_TINT	= 2.0f;		// 216 -->   0.0   (0)
                                            // effects on standard gray level
const float B_DISABLED_LABEL_TINT		= B_DARKEN_3_TINT;
const float B_HIGHLIGHT_BACKGROUND_TINT	= B_DARKEN_2_TINT;
const float B_DISABLED_MARK_TINT		= B_LIGHTEN_2_TINT;
// Icon related constants
// Values for [Set]IconBitmap() of various view classes. Not all types are
// applicable for all views.
enum {
    B_INACTIVE_ICON_BITMAP					= 0x00,
    B_ACTIVE_ICON_BITMAP					= 0x01,
    B_PARTIALLY_ACTIVATE_ICON_BITMAP		= 0x02,
    // flag, can be combined with any of the above
    B_DISABLED_ICON_BITMAP					= 0x80,
    // disabled version of the specified bitmap
};
// flags for SetIconBitmap() of various view classes
enum {
    B_KEEP_ICON_BITMAP						= 0x0001,
        // transfer bitmap ownership to the view
};
// flags for SetIcon() of various view classes
enum {
    B_TRIM_ICON_BITMAP						= 0x0100,
        // crop the bitmap to the not fully transparent area; may change the
        // icon size
    B_TRIM_ICON_BITMAP_KEEP_ASPECT			= 0x0200,
        // like B_TRIM_BITMAP, but keeps the aspect ratio
    B_CREATE_ACTIVE_ICON_BITMAP				= 0x0400,
    B_CREATE_PARTIALLY_ACTIVE_ICON_BITMAP	= 0x0800,
    B_CREATE_DISABLED_ICON_BITMAPS			= 0x1000,
};
enum {
    B_FOLLOW_NONE			= 0,
    B_FOLLOW_LEFT			= 1,
    B_FOLLOW_RIGHT			= 1 << 1,
    B_FOLLOW_TOP			= 1 << 2,
    B_FOLLOW_BOTTOM			= 1 << 3,
    B_FOLLOW_H_CENTER		= 1 << 4,
    B_FOLLOW_V_CENTER		= 1 << 5,
    B_FOLLOW_ALL			= 0xffff
};
#define B_FOLLOW_LEFT_RIGHT	(B_FOLLOW_LEFT | B_FOLLOW_RIGHT)
#define B_FOLLOW_TOP_BOTTOM	(B_FOLLOW_TOP | B_FOLLOW_BOTTOM)
#define B_FOLLOW_ALL_SIDES    B_FOLLOW_ALL

enum {
    B_PRIMARY_MOUSE_BUTTON = 1,
    B_SECONDARY_MOUSE_BUTTON = 2,
    B_TERTIARY_MOUSE_BUTTON = 3
};

#ifndef BBITMAP_DEF
#define BBITMAP_DEF
class BBitmap;
#endif
#ifndef BPOINT_DEF
#define BPOINT_DEF
class BPoint;
#endif
#ifndef BRECT_DEF
#define BRECT_DEF
class BRect;
#endif

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
// Key definitions
struct key_info {
    uint32_t_t	modifiers;
    uint8_t	key_states[16];
};

struct key_map {
    uint32_t_t	version;
    uint32_t_t	caps_key;
    uint32_t_t	scroll_key;
    uint32_t_t	num_key;
    uint32_t_t	left_shift_key;
    uint32_t_t	right_shift_key;
    uint32_t_t	left_command_key;
    uint32_t_t	right_command_key;
    uint32_t_t	left_control_key;
    uint32_t_t	right_control_key;
    uint32_t_t	left_option_key;
    uint32_t_t	right_option_key;
    uint32_t_t	menu_key;
    uint32_t_t	lock_settings;
    int32_t_t	control_map[128];
    int32_t_t	option_caps_shift_map[128];
    int32_t_t	option_caps_map[128];
    int32_t_t	option_shift_map[128];
    int32_t_t	option_map[128];
    int32_t_t	caps_shift_map[128];
    int32_t_t	caps_map[128];
    int32_t_t	shift_map[128];
    int32_t_t	normal_map[128];
    int32_t_t	acute_dead_key[32];
    int32_t_t	grave_dead_key[32];
    int32_t_t	circumflex_dead_key[32];
    int32_t_t	dieresis_dead_key[32];
    int32_t_t	tilde_dead_key[32];
    uint32_t_t	acute_tables;
    uint32_t_t	grave_tables;
    uint32_t_t	circumflex_tables;
    uint32_t_t	dieresis_tables;
    uint32_t_t	tilde_tables;
};
// Mouse definitions
struct mouse_map {
    uint32_t_t	button[B_MAX_MOUSE_BUTTONS];
};

struct scroll_bar_info {
    bool	proportional;
    bool	double_arrows;
    int32_t_t	knob;
    int32_t_t	min_knob_size;
};

const color_map* system_colors();
status_t		get_deskbar_frame(BRect* frame);
status_t		set_screen_space(int32_t_t index, uint32_t_t resolution,bool save = true);
status_t		get_scroll_bar_info(scroll_bar_info* info);
status_t		set_scroll_bar_info(scroll_bar_info* info);
status_t		get_mouse_type(int32_t_t* type);
status_t		set_mouse_type(int32_t_t type);
status_t		get_mouse_map(mouse_map* map);
status_t		set_mouse_map(mouse_map* map);
status_t		get_mouse_speed(int32_t_t* speed);
status_t		set_mouse_speed(int32_t_t speed);
status_t		get_mouse_acceleration(int32_t_t* speed);
status_t		set_mouse_acceleration(int32_t_t speed);
status_t		get_click_speed(bigtime_t* speed);
status_t		set_click_speed(bigtime_t speed);

status_t		get_key_repeat_rate(int32_t_t* rate);
status_t		set_key_repeat_rate(int32_t_t rate);
status_t		get_key_repeat_delay(bigtime_t* delay);
status_t		set_key_repeat_delay(bigtime_t delay);
status_t		get_key_info(key_info* info);
void			get_key_map(key_map** _map, char** _keyBuffer);
status_t		get_keyboard_id(uint16_t* _id);
void			set_keyboard_locks(uint32_t_t modifiers);
uint32_t_t			modifiers();
status_t		get_modifier_key(uint32_t_t modifier, uint32_t_t *key);
void			set_modifier_key(uint32_t_t modifier, uint32_t_t key);
rgb_color		keyboard_navigation_color();

int32_t_t			count_workspaces();
void			set_workspace_count(int32_t_t count);
int32_t_t			current_workspace();
void			activate_workspace(int32_t_t workspace);

bigtime_t		idle_time();

void			run_select_printer_panel();
void			run_add_printer_panel();
void			run_be_about();

void			set_focus_follows_mouse(bool follow);
bool			focus_follows_mouse();

void			set_mouse_mode(mode_mouse mode);
mode_mouse		mouse_mode();

void			set_focus_follows_mouse_mode(mode_focus_follows_mouse mode);
mode_focus_follows_mouse	focus_follows_mouse_mode();

status_t		get_mouse(BPoint* screenWhere, uint32_t_t* buttons);
status_t		get_mouse_bitmap(BBitmap** bitmap, BPoint* hotspot);

void			set_accept_first_click(bool acceptFirstClick);
bool			accept_first_click();

rgb_color		ui_color(color_which which);
void			set_ui_color(const color_which& which, const rgb_color& color);
rgb_color		tint_color(rgb_color color, float tint);

extern "C" status_t __init_interface_kit_();
    // for convenience, should be removed including the friend declarations
    // in Menu.h, ...
BHAPI_IMPEXP rgb_color ui_color(color_which which);
BHAPI_IMPEXP float ui_get_scrollbar_horizontal_height();
BHAPI_IMPEXP float ui_get_scrollbar_vertical_width();
#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif
#endif /* BHAPI_INTERFACE_DEFS_H */

