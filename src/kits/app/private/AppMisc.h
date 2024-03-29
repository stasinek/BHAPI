﻿/*
 * Copyright 2002-2009, Ingo Weinhold, bonefish@users.sf.net.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_APP_MISC_H
#define BHAPI_APP_MISC_H

#include <kits/app/Handler.h>
#include <kits/netLocker.h>
#include <kits/kernel/OS.h>
#include <Haiku.h>

//struct entry_ref;

namespace BPrivate {

class ServerLink;
status_t get_app_path(team_id team, char *buffer);
status_t get_app_path(char *buffer);
status_t get_app_ref(team_id team, bhapi::entry_ref *ref, bool traverse = true);
status_t get_app_ref(bhapi::entry_ref *ref, bool traverse = true);
team_id current_team();
void init_team_after_fork();
thread_id main_thread_for(team_id team);
bool is_app_showing_modal_window(team_id team);
status_t create_desktop_connection(ServerLink* link, const char* name,int32_t capacity);

} // namespace BPrivate

// _get_object_token_
/*!	Return the token of a BHandler.
    \param handler The BHandler.
    \return the token.
*/
inline int32_t _get_object_token_(const BHandler* object)
    { return bhapi::get_handler_token(object); }
#endif	// _APP_MISC_H
