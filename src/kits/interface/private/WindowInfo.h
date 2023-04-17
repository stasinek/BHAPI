/*
 * Copyright 2005-2008, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Axel Dörfler, axeld@pinc-software.de
 */
#ifndef _WINDOW_INFO_H_
#define _WINDOW_INFO_H_


#include <Haiku.h>


struct window_info {
	team_id		team;
    int32_t   	server_token;

	int32_t		thread;
	int32_t		client_token;
	int32_t		client_port;
	uint32_t		workspaces;

	int32_t		layer;	// see ServerWindow::GetInfo()
    uint32_t	  	feel;
    uint32_t      flags;
	int32_t		window_left;
	int32_t		window_top;
	int32_t		window_right;
	int32_t		window_bottom;

	int32_t		show_hide_level;
	bool		is_mini;
} _PACKED;

struct client_window_info : window_info {
	float		tab_height;
	float		border_size;
	char		name[1];
} _PACKED;

enum window_action {
	B_MINIMIZE_WINDOW,
	B_BRING_TO_FRONT
};


// Private BeOS compatible window API

void do_window_action(int32_t window_id, int32_t action, BRect zoomRect, bool zoom);
client_window_info* get_window_info(int32_t token);
int32_t* get_token_list(team_id app, int32_t *count);
void do_bring_to_front_team(BRect zoomRect, team_id app, bool zoom);
void do_minimize_team(BRect zoomRect, team_id app, bool zoom);

// Haiku additions

namespace BPrivate {

status_t get_application_order(int32_t workspace, team_id** _apps, int32_t* _count);
status_t get_window_order(int32_t workspace, int32_t** _tokens, int32_t* _count);

}	// namespace BPrivate


#endif	// _WINDOW_INFO_H_
