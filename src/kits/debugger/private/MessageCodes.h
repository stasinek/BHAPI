/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2013-2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef MESSAGE_CODES_H
#define MESSAGE_CODES_H


enum {
	MSG_THREAD_RUN								= 'run_',
	MSG_THREAD_SET_ADDRESS						= 'sead',
	MSG_THREAD_STOP								= 'stop',
	MSG_THREAD_STEP_OVER						= 'stov',
	MSG_THREAD_STEP_INTO						= 'stin',
	MSG_THREAD_STEP_OUT							= 'stou',
	MSG_SET_BREAKPOINT							= 'sbrk',
	MSG_CLEAR_BREAKPOINT						= 'cbrk',
	MSG_ENABLE_BREAKPOINT						= 'ebrk',
	MSG_DISABLE_BREAKPOINT						= 'dbrk',
	MSG_SET_BREAKPOINT_CONDITION				= 'sbpc',
	MSG_CLEAR_BREAKPOINT_CONDITION				= 'cbpc',
	MSG_SET_WATCHPOINT							= 'swpt',
	MSG_CLEAR_WATCHPOINT						= 'cwpt',
	MSG_ENABLE_WATCHPOINT						= 'ewpt',
	MSG_DISABLE_WATCHPOINT						= 'dwpt',
	MSG_STOP_ON_IMAGE_LOAD						= 'tsil',
	MSG_ADD_STOP_IMAGE_NAME						= 'asin',
	MSG_REMOVE_STOP_IMAGE_NAME					= 'rsin',
	MSG_SET_DEFAULT_SIGNAL_DISPOSITION			= 'sdsd',
	MSG_SET_CUSTOM_SIGNAL_DISPOSITION			= 'scsd',
	MSG_REMOVE_CUSTOM_SIGNAL_DISPOSITION		= 'rcsd',

	MSG_TEAM_RENAMED							= 'tera',
	MSG_THREAD_STATE_CHANGED					= 'tsch',
	MSG_THREAD_CPU_STATE_CHANGED				= 'tcsc',
	MSG_THREAD_STACK_TRACE_CHANGED				= 'tstc',
	MSG_STACK_FRAME_VALUE_RETRIEVED				= 'sfvr',
	MSG_IMAGE_DEBUG_INFO_CHANGED				= 'idic',
	MSG_STOP_IMAGE_SETTINGS_CHANGED				= 'sisc',
	MSG_STOP_IMAGE_NAME_ADDED					= 'sina',
	MSG_STOP_IMAGE_NAME_REMOVED					= 'sinr',
	MSG_CONSOLE_OUTPUT_RECEIVED					= 'core',
	MSG_IMAGE_FILE_CHANGED						= 'ifch',
	MSG_FUNCTION_SOURCE_CODE_CHANGED			= 'fnsc',
	MSG_USER_BREAKPOINT_CHANGED					= 'ubrc',
	MSG_WATCHPOINT_CHANGED						= 'wapc',
	MSG_MEMORY_DATA_CHANGED						= 'mdac',
	MSG_DEBUGGER_EVENT							= 'dbge',
	MSG_LOAD_SETTINGS							= 'ldst',

	MSG_SETTINGS_MENU_IMPL_ITEM_SELECTED		= 'smii',
	MSG_SETTINGS_MENU_IMPL_OPTION_ITEM_SELECTED	= 'smio',

	MSG_TEXTVIEW_AUTOSCROLL						= 'tvas',

	MSG_VARIABLES_VIEW_CONTEXT_MENU_DONE		= 'ctxd',
	MSG_VARIABLES_VIEW_NODE_SETTINGS_CHANGED	= 'vvns',

	MSG_VALUE_NODE_CHANGED						= 'vnch',
	MSG_VALUE_NODE_CHILDREN_CREATED				= 'vncc',
	MSG_VALUE_NODE_CHILDREN_DELETED				= 'vncd',
	MSG_VALUE_NODE_VALUE_CHANGED				= 'vnvc',

	MSG_TEAM_DEBUGGER_QUIT						= 'dbqt',
	MSG_TEAM_RESTART_REQUESTED					= 'trrq',
	MSG_SHOW_TEAMS_WINDOW						= 'stew',
	MSG_TEAMS_WINDOW_CLOSED						= 'tewc',
	MSG_SHOW_TEAM_SETTINGS_WINDOW				= 'stsw',
	MSG_TEAM_SETTINGS_WINDOW_CLOSED				= 'tswc',
	MSG_SHOW_BREAKPOINT_EDIT_WINDOW				= 'sbew',
	MSG_BREAKPOINT_EDIT_WINDOW_CLOSED			= 'bewc',
	MSG_SHOW_SIGNAL_DISPOSITION_EDIT_WINDOW		= 'sdew',
	MSG_SIGNAL_DISPOSITION_EDIT_WINDOW_CLOSED	= 'sdec',
	MSG_SHOW_START_TEAM_WINDOW					= 'sstw',
	MSG_START_TEAM_WINDOW_CLOSED				= 'stwc',
	MSG_START_NEW_TEAM							= 'sttt',
	MSG_DEBUG_THIS_TEAM							= 'dbtt',
	MSG_LOAD_CORE_TEAM							= 'lcte',
	MSG_SHOW_INSPECTOR_WINDOW					= 'sirw',
	MSG_INSPECTOR_WINDOW_CLOSED					= 'irwc',
	MSG_SHOW_EXPRESSION_WINDOW					= 'seww',
	MSG_EXPRESSION_WINDOW_CLOSED				= 'ewwc',
	MSG_SHOW_EXPRESSION_PROMPT_WINDOW			= 'sepw',
	MSG_ADD_NEW_EXPRESSION						= 'anex',
	MSG_EXPRESSION_PROMPT_WINDOW_CLOSED			= 'epwc',
	MSG_SHOW_VARIABLE_EDIT_WINDOW				= 'svew',
	MSG_VARIABLE_EDIT_WINDOW_CLOSED				= 'vewc',
	MSG_INSPECT_ADDRESS							= 'isad',
	MSG_WRITE_TARGET_MEMORY						= 'wtam',
	MSG_EVALUATE_EXPRESSION						= 'evex',
	MSG_EXPRESSION_EVALUATED					= 'exev',
	MSG_SHOW_TYPECAST_NODE_PROMPT				= 'stnp',
	MSG_TYPECAST_TO_ARRAY						= 'stta',
	MSG_TYPECAST_NODE							= 'tyno',
	MSG_SHOW_WATCH_VARIABLE_PROMPT				= 'swvp',
	MSG_SHOW_CONTAINER_RANGE_PROMPT				= 'scrp',
	MSG_SET_CONTAINER_RANGE						= 'chcr',
	MSG_GENERATE_DEBUG_REPORT					= 'gdrp',
	MSG_WRITE_CORE_FILE							= 'wrcf',
	MSG_WRITE_VARIABLE_VALUE					= 'wrvv',

	MSG_DEBUG_INFO_NEEDS_USER_INPUT				= 'dnui',
	MSG_USER_INTERFACE_FILE_CHOSEN				= 'uifc'
};


#endif	// MESSAGE_CODES_H
