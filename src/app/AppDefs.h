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
 * File: AppDefs.h
 * Description: Definition of Application Kit
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_APPDEFS_H__
#define __ETK_APPDEFS_H__

#include "./../support/SupportDefs.h"

/* System Message Codes */
enum {
	E_ABOUT_REQUESTED			= '_ABR',
	E_WINDOW_ACTIVATED			= '_ACT',
	E_APP_ACTIVATED				= '_ACT', /* Same as E_WINDOW_ACTIVATED */
	E_APP_CURSOR_REQUESTED			= '_CUR',
	E_ARGV_RECEIVED 			= '_ARG',
	E_QUIT_REQUESTED 			= '_QRQ',
	E_CANCEL				= '_CNC',
	E_KEY_DOWN 				= '_KYD',
	E_KEY_UP 				= '_KYU',
	E_UNMAPPED_KEY_DOWN 			= '_UKD',
	E_UNMAPPED_KEY_UP 			= '_UKU',
	E_MODIFIERS_CHANGED			= '_MCH',
	E_MINIMIZE				= '_WMN',
	E_MINIMIZED				= '_WMD',
	E_MOUSE_DOWN 				= '_MDN',
	E_MOUSE_MOVED 				= '_MMV',
	E_MOUSE_ENTER_EXIT			= '_MEX',
	E_MOUSE_UP 				= '_MUP',
	E_MOUSE_WHEEL_CHANGED			= '_MWC',
	E_OPEN_IN_WORKSPACE			= '_OWS',
	E_PRINTER_CHANGED			= '_PCH',
	E_PULSE 				= '_PUL',
	E_READY_TO_RUN 				= '_RTR',
	E_REFS_RECEIVED 			= '_RRC',
	E_SCREEN_CHANGED 			= '_SCH',
	E_VALUE_CHANGED 			= '_VCH',
	E_VIEW_MOVED 				= '_VMV',
	E_VIEW_RESIZED 				= '_VRS',
	E_WINDOW_MOVED 				= '_WMV',
	E_WINDOW_RESIZED 			= '_WRS',
	E_WORKSPACES_CHANGED			= '_WCG',
	E_WORKSPACE_ACTIVATED			= '_WAC',
	E_ZOOM					= '_WZM',
#if !(defined(ETK_OS_BEOS) && defined(_APP_DEFS_H))
	_QUIT_					= '_QIT',
	_EVENTS_PENDING_			= '_EVP',
	_UPDATE_				= '_UPD',
	_UPDATE_IF_NEEDED_			= '_UPN',
	_MENU_EVENT_				= '_MEV'
#endif /* !(ETK_OS_BEOS && _APP_DEFS_H) */
};


/* Other Command */
enum {
	E_UPDATE_STATUS_BAR			= 'SBUP',
	E_RESET_STATUS_BAR			= 'SBRS',
	E_OBSERVER_NOTICE_CHANGE		= 'NTCH',
	E_CONTROL_INVOKED			= 'CIVK',
	E_CONTROL_MODIFIED			= 'CMOD',
	E_CLIPBOARD_CHANGED			= 'CLCH',
	E_SAVE_REQUESTED			= 'SAVE',
	E_NO_REPLY				= 'NONE',
	E_REPLY					= 'RPLY',
	E_SET_PROPERTY				= 'PSET',
	E_GET_PROPERTY				= 'PGET',
	E_CREATE_PROPERTY			= 'PCRT',
	E_DELETE_PROPERTY			= 'PDEL',
	E_COUNT_PROPERTIES			= 'PCNT',
	E_EXECUTE_PROPERTY			= 'PEXE',
	E_GET_SUPPORTED_SUITES			= 'SUIT',
	E_INPUT_METHOD_EVENT			= 'IMEV'
};

#endif /* __ETK_APPDEFS_H__ */

