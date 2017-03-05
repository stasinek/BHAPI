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
 * File: AppDefs.h
 * Description: Definition of Application Kit
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_APPDEFS_H
#define BHAPI_APPDEFS_H
#include "../support/SupportDefs.h"

/* System Message Codes */
enum {
    B_ABOUT_REQUESTED			= '_ABR',
    B_WINDOW_ACTIVATED			= '_ACT',
    B_APP_ACTIVATED				= '_ACT', /* Same as B_WINDOW_ACTIVATED */
    B_APP_CURSOR_REQUESTED			= '_CUR',
    B_ARGV_RECEIVED 			= '_ARG',
    B_QUIT_REQUESTED 			= '_QRQ',
    B_CANCEL				= '_CNC',
    B_KEY_DOWN 				= '_KYD',
    B_KEY_UP 				= '_KYU',
    B_UNMAPPED_KEY_DOWN 			= '_UKD',
    B_UNMAPPED_KEY_UP 			= '_UKU',
    B_MODIFIERS_CHANGED			= '_MCH',
    B_MINIMIZE				= '_WMN',
    B_MINIMIZED				= '_WMD',
    B_MOUSE_DOWN 				= '_MDN',
    B_MOUSE_MOVED 				= '_MMV',
    B_MOUSE_ENTER_EXIT			= '_MEX',
    B_MOUSE_UP 				= '_MUP',
    B_MOUSE_WHEEL_CHANGED			= '_MWC',
    B_OPEN_IN_WORKSPACE			= '_OWS',
    B_PRINTER_CHANGED			= '_PCH',
    B_PULSE 				= '_PUL',
    B_READY_TO_RUN 				= '_RTR',
    B_REFS_RECEIVED 			= '_RRC',
    B_SCREEN_CHANGED 			= '_SCH',
    B_VALUE_CHANGED 			= '_VCH',
    B_VIEW_MOVED 				= '_VMV',
    B_VIEW_RESIZED 				= '_VRS',
    B_WINDOW_MOVED 				= '_WMV',
    B_WINDOW_RESIZED 			= '_WRS',
    B_WORKSPACES_CHANGED			= '_WCG',
    B_WORKSPACE_ACTIVATED			= '_WAC',
    B_ZOOM					= '_WZM',
#if !(defined(BHAPI_OS_BEOS) && defined(_APP_DEFS_H))
	_QUIT_					= '_QIT',
	_EVENTS_PENDING_			= '_EVP',
	_UPDATE_				= '_UPD',
	_UPDATE_IF_NEEDED_			= '_UPN',
	_MENU_EVENT_				= '_MEV'
#endif /* !(BHAPI_OS_BEOS && _APP_DEFS_H) */
};


/* Other Command */
enum {
    B_UPDATE_STATUS_BAR			= 'SBUP',
    B_RESET_STATUS_BAR			= 'SBRS',
    B_OBSERVER_NOTICE_CHANGE		= 'NTCH',
    B_CONTROL_INVOKED			= 'CIVK',
    B_CONTROL_MODIFIED			= 'CMOD',
    B_CLIPBOARD_CHANGED			= 'CLCH',
    B_SAVE_REQUESTED			= 'SAVE',
    B_NO_REPLY				= 'NONE',
    B_REPLY					= 'RPLY',
    B_SET_PROPERTY				= 'PSET',
    B_GET_PROPERTY				= 'PGET',
    B_CREATE_PROPERTY			= 'PCRT',
    B_DELETE_PROPERTY			= 'PDEL',
    B_COUNT_PROPERTIES			= 'PCNT',
    B_EXECUTE_PROPERTY			= 'PEXE',
    B_GET_SUPPORTED_SUITES			= 'SUIT',
    B_INPUT_METHOD_EVENT			= 'IMEV'
};

#endif /* BHAPI_APPDEFS_H */

